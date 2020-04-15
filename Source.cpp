#include <Windows.h>
#include "..\..\..\..\Desktop\Mathis\Code\headers\utils.h"
#include "..\..\..\..\Desktop\Mathis\Code\headers\diskutils.h"

#define EXIT_REASON_NO_REASON 0U
#define EXIT_REASON_INVALID_ARGUMENT 1U
#define EXIT_REASON_PREVENT_CRASH 2U
#define EXIT_REASON_NOT_ENOUGH_ARGUMENTS 3U

void Exit(DWORD dwExitReason, DWORD dwExitCode);
bool StrToQword(LPCSTR lpStr, QWORD* lpQwDest);

bool bDestHandleOpen = false, bSrcHandleOpen = false;
HANDLE hSrcFile = 0, hDestFile = 0;


int main(int argc, char* argv[]) {
	SetConsoleOutputCP(CP_UTF7);
	
	if (argc == 2) {
		char szBuffer[8];
		memcpy(szBuffer, argv[1], 8);
		ToLower(szBuffer);

		if (Equal(szBuffer, "getvol")) {
			DWORD driveNumber[26];
			DWORD volumesCount = 0, drivesCount = 0;
			VOLUME_DISK_EXTENTS vde;
			DWORD dw;
			HANDLE hVolume = 0, hVolumeSearch = 0;
			char szVolumeGUID[50];
			char szVolumeGUIDRoot[49];
			char szVolumeName[MAX_PATH];
			char szVolumeFS[MAX_PATH];


			puts("");
			if (!(hVolumeSearch = FindFirstVolumeA(szVolumeGUID, sizeof szVolumeGUID))) {
				printf("Impossible de trouver le premier volume. %lu\n", GetLastError());
				Exit(EXIT_REASON_NO_REASON, 1);
			}
			goto inLoop;
			while (FindNextVolumeA(hVolumeSearch, szVolumeGUID, sizeof szVolumeGUID)) {
			inLoop:


				memcpy(szVolumeGUIDRoot, szVolumeGUID, 48);
				szVolumeGUIDRoot[48] = 0;

				hVolume = CreateFileA(szVolumeGUIDRoot, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
				if (hVolume == INVALID_HANDLE_VALUE) {
					printf("CreateFile(%s) err: %lu\n\n", szVolumeGUIDRoot, GetLastError());
					CloseHandle(hVolume);
					continue;
				}
				if (!DeviceIoControl(hVolume, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL, NULL, &vde, sizeof vde, &dw, NULL)) {
					printf("DeviceIoControl err %lu\n\n", GetLastError());
					CloseHandle(hVolume);
					continue;
				}
				driveNumber[drivesCount] = vde.Extents[0].DiskNumber;
				*szVolumeFS = 0;
				*szVolumeName = 0;
				GetVolumeInformationA(szVolumeGUID, szVolumeName, sizeof szVolumeName, 0, 0, 0, szVolumeFS, sizeof szVolumeFS);

				printf("Volume %lu (%s)\n", volumesCount, szVolumeGUIDRoot);

				if (driveNumber[drivesCount] != -1)
					printf("Disque: %lu\n", driveNumber[drivesCount]);
				
				printf("Taille: %llu %s\n",
					(vde.Extents[0].ExtentLength.QuadPart > 10737418240) ? (vde.Extents[0].ExtentLength.QuadPart / 1073741824) : (vde.Extents[0].ExtentLength.QuadPart > 10485760 ? vde.Extents[0].ExtentLength.QuadPart / 1048576 : vde.Extents[0].ExtentLength.QuadPart / 1024),
					(vde.Extents[0].ExtentLength.QuadPart > 10737418240) ? "Go" : (vde.Extents[0].ExtentLength.QuadPart > 10485760 ? "Mo" : "Ko"));
				printf("Système de fichiers: %s\n", *szVolumeFS == 0 ? "RAW" : szVolumeFS);

				if (*szVolumeName != 0)
					printf("Nom: %s\n", szVolumeName);

				puts("");

				volumesCount++;
				drivesCount++;

				CloseHandle(hVolume);
			}


			Exit(EXIT_REASON_NO_REASON, 2);
		}
		if (Equal(szBuffer, "getdisk")) {
			bool bAvailablesDrives[64];
			char szDriveNumber[11];
			char szPhysicalDriveString[38] = "\\\\.\\PhysicalDrive";

			puts("");
			for (DWORD i = 0; i < 64; i++) {

				_ultoa(i, szDriveNumber, 10);
				strcpy(szPhysicalDriveString + 17, szDriveNumber);
				HANDLE hDrive = CreateFileA(szPhysicalDriveString, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
				if (hDrive == INVALID_HANDLE_VALUE) {
					DWORD dwLastError = GetLastError();
					if (dwLastError == ERROR_FILE_NOT_FOUND) {
						continue;
					}
					else {
						goto goodDrive;
						
					}

				}
			goodDrive:
				printf("Disque %lu", i);
				QWORD qwDriveSize = 0;
				if (GetDriveSize(hDrive, &qwDriveSize) == NO_ERROR) {
					printf("   %lu %s",
						(qwDriveSize > 10737418240) ? (qwDriveSize / 1073741824) : (qwDriveSize > 10485760 ? qwDriveSize / 1048576 : qwDriveSize / 1024),
						(qwDriveSize > 10737418240) ? "Go" : (qwDriveSize > 10485760 ? "Mo" : "Ko"));
				}
				puts("");
			}
			
			Exit(EXIT_REASON_NO_REASON, 21);
		}
		Exit(EXIT_REASON_INVALID_ARGUMENT, 22);
	}

	else if (argc < 3) {
		int index = FindLastChar(argv[0], '.');
		if (index != ERROR_CHAR_NOT_FOUND && Equal(argv[0] + index, ".exe")) {
			argv[0][index] = 0;
			
		}
		printf("Arguments pour %s:\n"
			   "if=\"fichier source\".\n"
			   "of=\"fichier de destination\".\n"
			   "bs=taille du buffer d'écriture (1m par défaut).\n"
			   "size=nombre d'octets à écrire.\n"
			   "data=valeur (de 0 à 255, optionnel si if est spécifié). Ce paramètre remplit le fichier spécifié par of avec la valeur.\n"
			   "delvol: supprime les volumes sur of si le fichier spécifié est un disque physique (\\\\.\\PhysicalDrive...). Utilisez ce paramètres si vous appliquez une image plus petite que le disque de destination pour éviter des effets innatendus.\n",
			argv[0]);
		
		Exit(EXIT_REASON_NOT_ENOUGH_ARGUMENTS, 3);
	}

	byte data = 0;
	byte bIfIsVolume = 0, bOfIsVolume = 0;	// 0 = non, 1 = lettre de volume, 2 = GUID du volume
	char szInputFile[FILENAME_MAX];
	char szOutputFile[FILENAME_MAX];
	QWORD qwBufferSize = 1 * 1024 * 1024;
	QWORD qwWriteSize = 0;
	QWORD qwIfSp = 0, qwOfSp = 0;	// Pointeurs de fichiers
	
	bool bIfSet = false, bOfSet = false, bSizeSet = false, bBsSet = false, bDataSet = false, bIfIsPhysicalDrive = false,
		bOfIsPhysicalDrive = false, bDeleteFile = false, bSetFileSize = false, bEndFile = false,
		bDeleteVolumes = false, bSetIfPtr = false, bSetOfPtr = false;
	
	
	for (int i = 1; i < argc; i++) {		// Boucle principale pour parser la ligne de commande
		if (argv[i] == NULL) {
			Exit(EXIT_REASON_PREVENT_CRASH, 4);
		}
		char szCmdLineTempBuffer[8];
		szCmdLineTempBuffer[7] = 0;
		memcpy(szCmdLineTempBuffer, argv[i], 7);
		ToLower(szCmdLineTempBuffer);	// Permet de supporter les options même si il y a des majuscules
		

		if (Equal(szCmdLineTempBuffer, "if=", 3)) {
			strcpy(szInputFile, argv[i] + 3);
			bIfSet = true;
			continue;

		}

		if (Equal(szCmdLineTempBuffer, "of=", 3)) {
			strcpy(szOutputFile, argv[i] + 3);
			bOfSet = true;
			continue;

		}

		if (Equal(szCmdLineTempBuffer, "data=", 5)) {
			data = atoi(argv[i] + 5);
			bDataSet = true;
			continue;

		}
		
		if (Equal(szCmdLineTempBuffer, "bs=", 3)) {
			if (!StrToQword(argv[i] + 3, &qwBufferSize)) {
				Exit(EXIT_REASON_INVALID_ARGUMENT, 5);
			}
			if (qwBufferSize == 0) {
				Exit(EXIT_REASON_INVALID_ARGUMENT, 51);
			}
			bBsSet = true;
			continue;

		}

		if (Equal(szCmdLineTempBuffer, "size=", 5)) {
			if (!StrToQword(argv[i] + 5, &qwWriteSize)) {
				Exit(EXIT_REASON_INVALID_ARGUMENT, 6);
			}
			bSizeSet = true;
			continue;
		}

		if (Equal(szCmdLineTempBuffer, "ifsp=", 5)) {
			if (!StrToQword(argv[i] + 5, &qwIfSp)) {
				Exit(EXIT_REASON_INVALID_ARGUMENT, 7);
			}
			bSetIfPtr = true;
			continue;
		}
		
		if (Equal(szCmdLineTempBuffer, "ofsp=", 5)) {
			if (!StrToQword(argv[i] + 5, &qwOfSp)) {
				Exit(EXIT_REASON_INVALID_ARGUMENT, 8);
			}
			bSetOfPtr = true;
			continue;
		}
		if (Equal(szCmdLineTempBuffer, "sp=", 3)) {
			if (!StrToQword(argv[i] + 3, &qwIfSp)) {
				Exit(EXIT_REASON_INVALID_ARGUMENT, 9);
			}
			qwOfSp = qwIfSp;
			bSetIfPtr = true;
			bSetOfPtr = true;
			continue;
		}

		if (Equal(szCmdLineTempBuffer, "delete")) {
			bDeleteFile = true;
			continue;
		}
		if (Equal(szCmdLineTempBuffer, "delvol")) {
			bDeleteVolumes = true;
			continue;
		}
		if (Equal(szCmdLineTempBuffer, "endfile")) {
			bEndFile = true;
			continue;
		}
		printf("i = %d\nargc = %d\nargv[%d] = %s\n", i, argc, i, argv[i]);
		Exit(EXIT_REASON_INVALID_ARGUMENT, 10);
	}
	
	if (!bOfSet) {
		Exit(EXIT_REASON_NOT_ENOUGH_ARGUMENTS, 11);
	}
	for (WORD i = 0; i < sizeof szInputFile; i++) {
		if (szInputFile[i] == '/') szInputFile[i] = '\\';
		if (szOutputFile[i] == '/') szOutputFile[i] = '\\';
	}

	LARGE_INTEGER li;

	if (bIfSet) {
		char szInputFileTemp[FILENAME_MAX];
		char szOutputFileTemp[FILENAME_MAX];
		strcpy(szInputFileTemp, szInputFile);
		strcpy(szOutputFileTemp, szOutputFile);
		ToLower(szInputFileTemp);
		ToLower(szOutputFileTemp);
		if (Equal(szInputFileTemp, szOutputFileTemp)) {
			printf("Les fichiers d'entrée et de sortie ne peuvent pas être les mêmes.\n");
			Exit(EXIT_REASON_NO_REASON, 12);
		}

		char volumeLetter = szInputFile[0];
		if (szInputFile[1] == ':' && IsLetter(volumeLetter) && szInputFile[2] == 0) {
			strcpy(szInputFile, "\\\\.\\ :");
			szInputFile[4] = (volumeLetter >= 'A' && volumeLetter <= 'Z') ? volumeLetter : volumeLetter - 32;
			bIfIsVolume = 1;
			
		}
		else if (((Equal(szInputFile, "\\\\.\\", 4)) || (Equal(szInputFile, "\\\\?\\", 4))) && IsLetter(szInputFile[4]) && szInputFile[5] == ':' && szInputFile[6] == 0) {
			bIfIsVolume = 1;
			
		}
		else {
			if (strlen(szInputFile) == 48) {
				char szBuffer[11];
				memcpy(szBuffer, szInputFile, sizeof szBuffer);
				ToLower(szBuffer, sizeof szBuffer);
				if (Equal(szBuffer, "\\\\.\\volume{", 11) || Equal(szBuffer, "\\\\?\\volume{", 11)) {
					bIfIsVolume = 2;
				}
			}
		}
		volumeLetter = szOutputFile[0];
		if (szOutputFile[1] == ':' && IsLetter(volumeLetter) && szOutputFile[2] == 0) {
			strcpy(szOutputFile, "\\\\.\\ :");
			szOutputFile[4] = (volumeLetter >= 'A' && volumeLetter <= 'Z') ? volumeLetter : volumeLetter - 32;
			bOfIsVolume = 1;

		}
		else if (((Equal(szOutputFile, "\\\\.\\", 4)) || (Equal(szOutputFile, "\\\\?\\", 4))) && IsLetter(szOutputFile[4]) && szOutputFile[5] == ':' && szOutputFile[6] == 0) {
			bOfIsVolume = 1;

		}
		else {
			if (strlen(szOutputFile) == 48) {
				char szBuffer[11];
				memcpy(szBuffer, szOutputFile, sizeof szBuffer);
				ToLower(szBuffer, sizeof szBuffer);
				if (Equal(szBuffer, "\\\\.\\volume{", 11) || Equal(szBuffer, "\\\\?\\volume{", 11)) {
					bOfIsVolume = 2;
				}
			}
		}

		hSrcFile = CreateFileA(szInputFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
		bSrcHandleOpen = true;
		if (hSrcFile == INVALID_HANDLE_VALUE) {
			printf("Impossible d'ouvrir le fichier source. Code d'erreur : %lu\n", GetLastError());
			Exit(EXIT_REASON_NO_REASON, 13);
		}
		
		if (bSetIfPtr) {
			if (qwIfSp == 0) {
				Exit(EXIT_REASON_INVALID_ARGUMENT, 14);
			}
			li.QuadPart = qwIfSp;
			if (!SetFilePointerEx(hSrcFile, li, 0, FILE_BEGIN)) {
				printf("Impossible de déplacer le pointeur du fichier source: %lu\n", GetLastError());
				Exit(EXIT_REASON_NO_REASON, 15);
			}
		}

	}



	hDestFile = CreateFileA(szOutputFile, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH, NULL);
	bDestHandleOpen = true;
	DWORD dwLastError = GetLastError();
	if (dwLastError == ERROR_FILE_NOT_FOUND) {
		CloseHandle(hDestFile);
		hDestFile = CreateFileA(szOutputFile, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH, NULL);
		if (hDestFile == INVALID_HANDLE_VALUE) {
			printf("Impossible de créer le fichier de destination. Code d'erreur : %lu\n", GetLastError());
			Exit(EXIT_REASON_NO_REASON, 16);

		}
	}
	else {
		if (dwLastError != NO_ERROR) {
			printf("Impossible d'ouvrir le fichier de destination. Code d'erreur : %lu\n", dwLastError);
			Exit(EXIT_REASON_NO_REASON, 17);
		}

	}
	if (bSetOfPtr) {
		if (qwOfSp == 0) {
			Exit(EXIT_REASON_INVALID_ARGUMENT, 171);
		}
		li.QuadPart = qwOfSp;
		if (!SetFilePointerEx(hDestFile, li, NULL, FILE_BEGIN)) {
			printf("Impossible de déplacer le pointeur du fichier de destination: %lu\n", GetLastError());
			Exit(EXIT_REASON_NO_REASON, 172);
		}
		
	}
	
	DWORD ret;
	DWORD dwOutputFileDriveNumber;

	char szBuffer[50];
	szBuffer[17] = 0;
	memcpy(szBuffer, szOutputFile, 17);
	ToLower(szBuffer);
	if (Equal(szBuffer, "\\\\.\\physicaldrive") || Equal(szBuffer, "\\\\?\\physicaldrive")) {
		bOfIsPhysicalDrive = true;
		char tmp[8];
		strcpy(tmp, szOutputFile + 17);
		dwOutputFileDriveNumber = atoi(tmp);
		if (dwOutputFileDriveNumber == 0) {
			printf("Le disque sélectionné est le disque système.\nContinuer tout de même ? ");
			char response[256];
			scanf("%s", response);
			ToLower(response);

			if (!Equal(response, "oui")) {
				Exit(EXIT_REASON_NO_REASON, 18);
			}
			
		}

	}

	if (bIfSet) {
		memcpy(szBuffer, szInputFile, 17);
		ToLower(szBuffer);
		if (Equal(szBuffer, "\\\\.\\physicaldrive") || Equal(szBuffer, "\\\\?\\physicaldrive")) {
			bIfIsPhysicalDrive = true;
		
		}
	}
	

	QWORD qwDestSize = 0, qwSrcSize = 0;
	if (bIfIsPhysicalDrive) {
		GetDriveSize(hSrcFile, &qwSrcSize);
		printf("GetDriveSize(hSrcFile) %llu\n", qwSrcSize);
	}
	if (bOfIsPhysicalDrive) {
		GetDriveSize(hDestFile, &qwDestSize);
		printf("GetDriveSize(hDestFile) %llu\n", qwDestSize);
	}
	if (bIfSet && !bIfIsPhysicalDrive && !bIfIsVolume) {
		GetFileSizeEx(hSrcFile, &li);
		qwSrcSize = li.QuadPart;
		printf("GetFileSizeEx(hSrcFile) %llu\n", qwSrcSize);
	}
	if (bOfSet && !bOfIsPhysicalDrive && !bOfIsVolume) {
		GetFileSizeEx(hDestFile, &li);
		qwDestSize = li.QuadPart;
		printf("GetFileSizeEx(hDestFile) %llu\n", qwDestSize);
	}
	if (bIfIsVolume) {
		GetVolumeSize(hSrcFile, &qwSrcSize);
		printf("GetVolumeSize(hSrcFile) %llu\n", qwSrcSize);
	}
	if (bOfIsVolume) {
		GetVolumeSize(hDestFile, &qwDestSize);
		printf("GetVolumeSize(hDestFile) %llu\n", qwDestSize);
	}
	
	if (!bIfSet && !bDataSet) {
		if (!bSizeSet)
			Exit(EXIT_REASON_NOT_ENOUGH_ARGUMENTS, 19);

		if (bOfIsPhysicalDrive) {
			Exit(EXIT_REASON_INVALID_ARGUMENT, 20);
		}
		bSetFileSize = true;
		
	}
	if (bOfIsPhysicalDrive) {
		LockDriveVolumes(dwOutputFileDriveNumber, (!bSizeSet && (qwSrcSize == qwDestSize)) || bDeleteVolumes);
	}

	if (bOfIsVolume) {
		LockVolume(hDestFile, bDeleteVolumes);
	}

	printf("qwSrcSize:%llu, qwDestSize:%llu\n", qwSrcSize, qwDestSize);
	printf("ifIsVolume: %d, ofIsVolume: %d\n", bIfIsVolume, bOfIsVolume);

	if (!bSizeSet) {
		if (qwSrcSize != 0 && qwDestSize != 0) {
			if (bIfIsPhysicalDrive) {
				qwWriteSize = bSetIfPtr ? qwSrcSize - qwIfSp : qwSrcSize;
				
			}
			else {
				if (qwSrcSize < qwDestSize || bIfSet) {
					qwWriteSize = bSetIfPtr ? qwSrcSize - qwIfSp : qwSrcSize;
				}
				else {
					qwWriteSize = bSetOfPtr ? qwDestSize - qwOfSp : qwDestSize;

				}
			}
		}
		else {
			if (qwSrcSize == 0) qwWriteSize = qwDestSize;
			else if (qwDestSize == 0) qwWriteSize = bSetIfPtr ? qwSrcSize - qwIfSp : qwSrcSize;
		}
		

	}
	printf("qwWriteSize: %llu\n", qwWriteSize);
	if (!bIfSet) {
		
		if (bSetFileSize) {
			ret = SetFileSize(hDestFile, qwWriteSize);
		}
		else {
			ret = FillFile(hDestFile, qwWriteSize, qwBufferSize, data);
		}
		
	}
	else {
		ret = CopyLargeFile(hSrcFile, hDestFile, qwBufferSize, qwWriteSize);
	}
	
	printf("\nret = %lu\n", ret);
	if (bEndFile && !bOfIsVolume && !bOfIsPhysicalDrive) {
		if (!(dwLastError = SetFileSize(hDestFile, bSetOfPtr ? qwWriteSize + qwOfSp : qwWriteSize))) {
			printf("SetFileSizeEnd(%s) err: %lu\n", szOutputFile, dwLastError);
		}
	}
	if (bDeleteFile && !bOfIsVolume && !bOfIsPhysicalDrive) {
		if (bDestHandleOpen) {
			CloseHandle(hDestFile);
			bDestHandleOpen = false;
		}
		
		if (!DeleteFileA(szOutputFile))
			printf("DeleteFileA(%s) err: %lu\n", szOutputFile, GetLastError());
	}

	Exit(EXIT_REASON_NO_REASON, 0);
}
bool StrToQword(LPCSTR lpStr, QWORD* lpQwDest) {
	char suffix[] = { 's', 'k', 'm', 'g' }; // (s)ector, (k)ilobyte...
	
	QWORD qwSuffixSize[] = { 512, 1024, 1048576, 1073741824 };
	DWORD dwStrLength = 0;
	byte lastDecimal = 0;
	while (lpStr[dwStrLength] != 0) {
		if (lpStr[dwStrLength] >= '0' && lpStr[dwStrLength] <= '9')
			lastDecimal++;
		dwStrLength++;
	}
	if (dwStrLength > lastDecimal + 1)
		return false;

	if (lastDecimal == dwStrLength) {	// Si y'a pas de suffixe, la fonction renvoie le chiffre comme il est
		*lpQwDest = strtoull(lpStr, 0, 10);
		return true;
	}

	for (byte i = 0; i < sizeof(suffix); i++) {
		if (lpStr[lastDecimal] == suffix[i] || lpStr[lastDecimal] == suffix[i] + 32) {
			char tmp[18];
			memcpy(tmp, lpStr, lastDecimal);
			DWORD temp = strtoul(lpStr, 0, 10);
			*lpQwDest = temp * qwSuffixSize[i];
			return true;
		}
		else if (i == sizeof suffix - 1) {
			return false;
		}
	}
	return true;
}

void Exit(DWORD dwExitReason, DWORD dwExitCode) {
	if (bSrcHandleOpen)
		CloseHandle(hSrcFile);

	if (bDestHandleOpen)
		CloseHandle(hDestFile);

	switch (dwExitReason) {
	case EXIT_REASON_NO_REASON: {
		goto exit;

	}
	case EXIT_REASON_INVALID_ARGUMENT: {
		printf("\nArgument invalide.");
		goto exit;
	}
	case EXIT_REASON_PREVENT_CRASH: {
		printf("\nErreur d'application.");
		goto exit;
	}
	case EXIT_REASON_NOT_ENOUGH_ARGUMENTS: {
		printf("\nPas assez d'arguments spécifiés.");
		goto exit;
	}
	default:
		break;
	}
exit:
	printf("\nCode d'arrêt : %lu\n", dwExitCode);
	ExitProcess(dwExitCode);
}