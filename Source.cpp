#include <Windows.h>
#include "utils.h"


#define EXIT_REASON_NO_REASON 0U
#define EXIT_REASON_INVALID_ARGUMENT 1U
#define EXIT_REASON_PREVENT_CRASH 2U
#define EXIT_REASON_NOT_ENOUGH_ARGUMENTS 3U

void Exit(DWORD dwExitReason, DWORD dwExitCode);
void StrToQword(LPCSTR lpStr, QWORD* lpQwDest);


bool bIfSet = false, bOfSet = false, bSizeSet = false, bBsSet = false, bDataSet = false, bIfIsPhysicalDrive = false, bOfIsPhysicalDrive = false, bDeleteFile = false,
bDestHandleOpen, bSrcHandleOpen;
HANDLE hSrcFile = 0, hDestFile = 0;

int main(int argc, char* argv[]) {
	SetConsoleOutputCP(CP_UTF7);
	
	if (argc == 2 && Equal(argv[1], "getvol")) {
		DWORD dwVolumesLetters = GetLogicalDrives();
		char volumeLetter[26];
		byte driveNumber[26];
		bool bVolumeMounted[26];
		char szVolumeLetter[7] = "\\\\.\\ :";
		byte volumesCount = 0;
		VOLUME_DISK_EXTENTS vde;
		DWORD dw;

		for (byte i = 0; i < 26; i++) {
			if (GETBIT(dwVolumesLetters, i)) {
				bVolumeMounted[i] = true;
				szVolumeLetter[4] = i + 'A';
				HANDLE hVolume = CreateFileA(szVolumeLetter, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
				printf("%s %lu\n", szVolumeLetter, GetLastError());
				if (!DeviceIoControl(hVolume, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL, NULL, &vde, sizeof vde, &dw, NULL)) {
					printf("DeviceIoControl err %lu\n", GetLastError());
					continue;
				}
				volumesCount++;
			}
			else {
				bVolumeMounted[i] = false;
			}

			for (byte i = 0; i < volumesCount; i++) {
				printf("PhysicalDrive%d:\n", driveNumber[i]);
			}
		}
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
			char tmp[4];
			byte cnt = 0;
			while (argv[i][cnt + 5] != 0) {
				if (cnt == sizeof tmp) {
					Exit(EXIT_REASON_PREVENT_CRASH, 3);
				}
				tmp[cnt] = argv[i][cnt + 5];
				cnt++;
			}
			tmp[cnt] = 0;
			data = atoi(tmp);
			bDataSet = true;
			continue;

		}
		
		if (memcmp(argv[i], "bs=", 3) == 0) {
			char tmp[11];
			byte cnt = 0;
			while (argv[i][cnt + 3] != 0) {
				if (cnt == sizeof tmp) {
					Exit(EXIT_REASON_PREVENT_CRASH, 4);
				}
				tmp[cnt] = argv[i][cnt + 3];
				cnt++;
			}
			tmp[cnt] = 0;
			StrToQword(tmp, &qwBufferSize);
			bBsSet = true;
			continue;

		}

		if (memcmp(argv[i], "size=", 5) == 0) {
			char tmp[20];
			byte cnt = 0;
			while (argv[i][cnt + 5] != 0) {
				if (cnt == sizeof tmp) {
					Exit(EXIT_REASON_PREVENT_CRASH, 5);
				}
				tmp[cnt] = argv[i][cnt + 5];
				cnt++;
			}
			tmp[cnt] = 0;
			StrToQword(tmp, &qwWriteSize);
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
		hSrcFile = CreateFileA(szInputFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
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

	char szBuffer[17];
	memcpy(szBuffer, szOutputFile, 17);
	ToLower(szBuffer);
	if (memcmp(szBuffer, "\\\\.\\physicaldrive", 17) == 0) {
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
	if (memcmp(szInputFile, "\\\\.\\physicaldrive", 17) == 0) {
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
		Exit(EXIT_REASON_NOT_ENOUGH_ARGUMENTS, 12);
	}
	if (bOfIsPhysicalDrive)
		LockDriveVolumes(dwOutputFileDriveNumber, !bSizeSet && (qwSrcSize == qwDestSize));

	printf("qwSrcSize:%llu, qwDestSize:%llu\n", qwSrcSize, qwDestSize);
	if (!bSizeSet) {
		if (qwSrcSize != 0 && qwDestSize != 0) {
			qwWriteSize = (qwSrcSize < qwDestSize) ? qwSrcSize : qwDestSize;
		}
		else {
			if (qwSrcSize == 0) qwWriteSize = qwDestSize;
			else if (qwDestSize == 0) qwWriteSize = qwSrcSize;
		}
		

	}

	if (!bIfSet) {

		ret = FillFile(hDestFile, qwWriteSize, qwBufferSize, data);
		
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
			printf("DeleteFileA err %lu\n", GetLastError());
	}
	Exit(EXIT_REASON_NO_REASON, 0);
}
void StrToQword(LPCSTR lpStr, QWORD* lpQwDest) {
	printf("StrToQword(%s, %llu)\n", lpStr, *lpQwDest);
	char suffixLow[] = { 's', 'k', 'm', 'g' }; // (s)ector, (k)ilobyte...
	char suffixUpper[] = { 'S', 'K', 'M', 'G' };
	QWORD qwSuffixSize[] = { 512, 1024, 1048576, 1073741824 };
	DWORD dwStrLength = strlen(lpStr);
	if (lpStr[dwStrLength - 1] >= '0' && lpStr[dwStrLength - 1] <= '9') {
		*lpQwDest = strtoul(lpStr, 0, 10);
		return;
	}
	for (byte i = 0; i < sizeof(suffixLow); i++) {
		if (lpStr[dwStrLength - 1] == suffixLow[i] || lpStr[dwStrLength - 1] == suffixUpper[i]) {
			char tmp[12];
			memcpy(tmp, lpStr, dwStrLength - 1);
			DWORD temp = strtoul(tmp, 0, 10);
			*lpQwDest = temp * qwSuffixSize[i];
		}
	}
	printf("StrToQword(%s, %llu)\n", lpStr, *lpQwDest);
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