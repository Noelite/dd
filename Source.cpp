#include <Windows.h>
#include "utils.h"
#include "diskutils.h"

#define EXIT_REASON_NO_REASON 0U
#define EXIT_REASON_INVALID_ARGUMENT 1U
#define EXIT_REASON_PREVENT_CRASH 2U
#define EXIT_REASON_NOT_ENOUGH_ARGUMENTS 3U

void Exit(DWORD dwExitReason, DWORD dwExitCode);
bool StrToQword(LPCSTR lpStr, QWORD* lpQwDest);


bool bIfSet = false, bOfSet = false, bSizeSet = false, bBsSet = false, bDataSet = false, bIfIsPhysicalDrive = false,
bOfIsPhysicalDrive = false, bDeleteFile = false, bDestHandleOpen = false, bSrcHandleOpen = false, bIfIsVolume = false, bOfIsVolume = false;
HANDLE hSrcFile = 0, hDestFile = 0;


int main(int argc, char* argv[]) {
	SetConsoleOutputCP(CP_UTF7);
	
	if (argc == 2 && Equal(argv[1], "getvol")) {
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
			Exit(EXIT_REASON_NO_REASON, 0);
		}
		goto inLoop;
		while (FindNextVolumeA(hVolumeSearch, szVolumeGUID, sizeof szVolumeGUID)) {
			inLoop:
					
			
			memcpy(szVolumeGUIDRoot, szVolumeGUID, 48);
			szVolumeGUIDRoot[48] = 0;

			hVolume = CreateFileA(szVolumeGUIDRoot, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
			if (hVolume == INVALID_HANDLE_VALUE) {
				printf("CreateFile(%s) err: %lu\n", szVolumeGUIDRoot, GetLastError());
				CloseHandle(hVolume);
				continue;
			}
			if (!DeviceIoControl(hVolume, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL, NULL, &vde, sizeof vde, &dw, NULL)) {
				printf("DeviceIoControl err %lu\n", GetLastError());
				CloseHandle(hVolume);
				continue;
			}
			driveNumber[drivesCount] = vde.Extents[0].DiskNumber;
			*szVolumeFS = 0;
			*szVolumeName = 0;
			GetVolumeInformationA(szVolumeGUID, szVolumeName, sizeof szVolumeName, 0, 0, 0, szVolumeFS, sizeof szVolumeFS);

			printf("Volume %lu (%s)\nDisque: %lu\nTaille: %llu Mo\nSystème de fichiers: %s%s",
				volumesCount, szVolumeGUIDRoot, driveNumber[drivesCount], vde.Extents[0].ExtentLength.QuadPart / 1048576,
				*szVolumeFS == 0 ? "RAW" : szVolumeFS, *szVolumeName != 0 ? "\n" : "\n\n");
			
			if (*szVolumeName != 0) {
				printf("Nom: %s\n\n", szVolumeName);
			}

			volumesCount++;
			drivesCount++;
			
			CloseHandle(hVolume);
		}
		

		Exit(EXIT_REASON_NO_REASON, 0);
	}

	else if (argc < 3) {
		int index = FindLastChar(argv[0], '.');
		if (index != ERROR_CHAR_NOT_FOUND && memcmp(argv[0] + index, "exe", 3) == 0) {
			argv[index] = 0;

		}
		printf("Arguments pour %s:\n"
			   "if=\"fichier source\"\n"
			   "of=\"fichier de destination\"\n"
			   "bs=taille du buffer d'écriture (1m par défaut)\n"
			   "size=taille à copier (en octets)\n"
			   "data=nombre (de 0 à 255, optionnel si if est renseigné)\n", argv[0]);
		
		Exit(EXIT_REASON_NOT_ENOUGH_ARGUMENTS, 1);
	}

	byte data = 0;
	char szInputFile[FILENAME_MAX];
	char szOutputFile[FILENAME_MAX];
	QWORD qwBufferSize = 1 * 1024 * 1024;
	QWORD qwWriteSize = 0;
	bool bSetFileSize = false;

	

	for (int i = 1; i < argc; i++) {
		if (argv[i] == NULL) {
			Exit(EXIT_REASON_PREVENT_CRASH, 2);
		}
		if (memcmp(argv[i], "if=", 3) == 0) {
			strcpy(szInputFile, argv[i] + 3);
			bIfSet = true;
			continue;

		}

		if (memcmp(argv[i], "of=", 3) == 0) {
			strcpy(szOutputFile, argv[i] + 3);
			bOfSet = true;
			continue;

		}

		if (memcmp(argv[i], "data=", 5) == 0) {
			data = atoi(argv[i] + 5);
			bDataSet = true;
			continue;

		}
		
		if (memcmp(argv[i], "bs=", 3) == 0) {
			if (!StrToQword(argv[i] + 3, &qwBufferSize)) {
				Exit(EXIT_REASON_INVALID_ARGUMENT, 4);
			}
			bBsSet = true;
			continue;

		}

		if (memcmp(argv[i], "size=", 5) == 0) {
			if (!StrToQword(argv[i] + 5, &qwWriteSize)) {
				Exit(EXIT_REASON_INVALID_ARGUMENT, 5);
			}
			bSizeSet = true;
			continue;
		}
		if (memcmp(argv[i], "delete", 7) == 0) {
			bDeleteFile = true;
			continue;
		}
		printf("i = %d\nargc = %d\nargv[%d] = %s", i, argc, i, argv[i]);
		Exit(EXIT_REASON_INVALID_ARGUMENT, 6);
	}
	
	for (WORD i = 0; i < sizeof szInputFile; i++) {
		if (szInputFile[i] == '/') szInputFile[i] = '\\';
		if (szOutputFile[i] == '/') szOutputFile[i] = '\\';
	}
	if (bIfSet && bOfSet) {
		char szInputFileTemp[FILENAME_MAX];
		char szOutputFileTemp[FILENAME_MAX];
		strcpy(szInputFileTemp, szInputFile);
		strcpy(szOutputFileTemp, szOutputFile);
		ToLower(szInputFileTemp);
		ToLower(szOutputFileTemp);
		if (Equal(szInputFileTemp, szOutputFileTemp)) {
			printf("Les fichiers d'entrée et de sortie ne peuvent pas être les mêmes.\n");
			Exit(EXIT_REASON_NO_REASON, 7);
		}
	}

	if (bIfSet) {
		hSrcFile = CreateFileA(szInputFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
		bSrcHandleOpen = true;
		if (hSrcFile == INVALID_HANDLE_VALUE) {
			printf("Impossible d'ouvrir le fichier source. Code d'erreur : %lu\n", GetLastError());
			Exit(EXIT_REASON_NO_REASON, 8);
		}
	}
	if (bOfSet) {
		hDestFile = CreateFileA(szOutputFile, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH, NULL);
		bDestHandleOpen = true;
		DWORD dwLastError = GetLastError();
		if (dwLastError == ERROR_FILE_NOT_FOUND) {
			CloseHandle(hDestFile);
			hDestFile = CreateFileA(szOutputFile, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH, NULL);
			if (hDestFile == INVALID_HANDLE_VALUE) {
				printf("Impossible de créer le fichier de destination. Code d'erreur : %lu\n", GetLastError());
				Exit(EXIT_REASON_NO_REASON, 9);
			}
		}
		else {
			if (dwLastError != NO_ERROR) {
				printf("Impossible d'ouvrir le fichier de destination. Code d'erreur : %lu\n", dwLastError);
				Exit(EXIT_REASON_NO_REASON, 10);
			}

		}
		
	}

	LARGE_INTEGER li;
	DWORD ret;
	DWORD dwOutputFileDriveNumber;

	char szBuffer[18];
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
				Exit(EXIT_REASON_NO_REASON, 11);
			}
			
		}

	}


	memcpy(szBuffer, szInputFile, 17);
	ToLower(szBuffer);
	if (Equal(szBuffer, "\\\\.\\physicaldrive") || Equal(szBuffer, "\\\\?\\physicaldrive")) {
		bIfIsPhysicalDrive = true;
		
	}

	QWORD qwDestSize = 0, qwSrcSize = 0;
	if (!bSizeSet && (bOfIsPhysicalDrive || bIfIsPhysicalDrive)) {
		GetDriveSize(bIfIsPhysicalDrive ? hSrcFile : hDestFile, bIfIsPhysicalDrive ? &qwSrcSize : &qwDestSize);
		printf("GetDriveSize(%s) %llu\n", bIfIsPhysicalDrive ? "hSrcFile" : "hDestFile", bIfSet ? qwSrcSize : qwDestSize);
	}
	if (!bSizeSet && bIfSet && !bIfIsPhysicalDrive) {
		GetFileSizeEx(hSrcFile, &li);
		qwSrcSize = li.QuadPart;
		printf("GetFileSizeEx(hSrcFile) %llu\n", qwSrcSize);
	}
	if (!bSizeSet && !bOfIsPhysicalDrive) {
		GetFileSizeEx(hDestFile, &li);
		qwDestSize = li.QuadPart;

	}
	if (!bIfSet && !bDataSet) {
		if (!bSizeSet)
			Exit(EXIT_REASON_NOT_ENOUGH_ARGUMENTS, 12);

		if (bOfIsPhysicalDrive) {
			Exit(EXIT_REASON_INVALID_ARGUMENT, 13);
		}
		bSetFileSize = true;
		
	}
	if (bOfIsPhysicalDrive)
		LockDriveVolumes(dwOutputFileDriveNumber, !bSizeSet && (qwSrcSize == qwDestSize));

	printf("bIfPhysDrv: %d\nbOfPhysDrv: %d\n", bIfIsPhysicalDrive, bOfIsPhysicalDrive);
	printf("qwSrcSize:%llu, qwDestSize:%llu\n", qwSrcSize, qwDestSize);
	if (!bSizeSet) {
		if (qwSrcSize != 0 && qwDestSize != 0) {
			if (bIfIsPhysicalDrive) {
				qwWriteSize = qwSrcSize;
			}
			else {
				qwWriteSize = (qwSrcSize < qwDestSize) ? qwSrcSize : qwDestSize;
			}
		}
		else {
			if (qwSrcSize == 0) qwWriteSize = qwDestSize;
			else if (qwDestSize == 0) qwWriteSize = qwSrcSize;
		}
		

	}

	if (!bIfSet) {
		printf("bSetFileSize: %d\n", bSetFileSize);
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
	
	if (bDeleteFile) {
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
	printf("StrToQword(%s, %llu)\n", lpStr, *lpQwDest);
	char suffix[] = { 's', 'k', 'm', 'g' }; // (s)ector, (k)ilobyte...
	
	QWORD qwSuffixSize[] = { 512, 1024, 1048576, 1073741824 };
	DWORD dwStrLength = 0;
	while (lpStr[dwStrLength] >= '0' && lpStr[dwStrLength] <= '9') {
		dwStrLength++;
	}
	dwStrLength++;
	
	if (lpStr[dwStrLength - 1] >= '0' && lpStr[dwStrLength - 1] <= '9') {	// Si y'a pas de suffixe, la fonction renvoie le chiffre comme il est
		*lpQwDest = strtoull(lpStr, 0, 10);
		printf("StrToQword(%s, %llu)\n", lpStr, *lpQwDest);
		return true;
	}

	for (byte i = 0; i < sizeof(suffix); i++) {
		if (lpStr[dwStrLength - 1] == suffix[i] || lpStr[dwStrLength - 1] == suffix[i] + 32) {
			char tmp[12];
			memcpy(tmp, lpStr, dwStrLength - 1);
			DWORD temp = strtoul(lpStr, 0, 10);
			*lpQwDest = temp * qwSuffixSize[i];
			printf("StrToQword(%s, %llu)\n", lpStr, *lpQwDest);
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
		printf("Argument invalide.\n");
		goto exit;
	}
	case EXIT_REASON_PREVENT_CRASH: {
		printf("Erreur d'application.\n");
		goto exit;
	}
	case EXIT_REASON_NOT_ENOUGH_ARGUMENTS: {
		printf("Pas assez d'arguments spécifiés.\n");
		goto exit;
	}
	default:
		break;
	}
exit:
	printf("Code d'arrêt : %lu\n", dwExitCode);
	ExitProcess(dwExitCode);
}