#include <Windows.h>
#include "..\..\..\..\Desktop\Mathis\Code\headers\utils.h"


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
	if (argc < 3) {
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
			hSrcFile = CreateFileA(szInputFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
			bSrcHandleOpen = true;
			bIfSet = true;
			continue;

		}

		if (memcmp(argv[i], "of=", 3) == 0) {
			strcpy(szOutputFile, argv[i] + 3);
			hDestFile = CreateFileA(szOutputFile, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH, NULL);
			if (GetLastError() == ERROR_FILE_NOT_FOUND) {
				CloseHandle(hDestFile);
				hDestFile = CreateFileA(szOutputFile, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH, NULL);
			}
			bDestHandleOpen = true;
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
	
	LARGE_INTEGER li;
	DWORD ret = 0;


	if (memcmp(szOutputFile, "\\\\.\\PhysicalDrive", 17) == 0) {
		bOfIsPhysicalDrive = true;
		char tmp[4];
		strcpy(tmp, szOutputFile + 17);
		DWORD dwOutputFileDriveNumber = atoi(tmp);
		LockDriveVolumes(dwOutputFileDriveNumber);

	}
	if (memcmp(szInputFile, "\\\\.\\PhysicalDrive", 17) == 0) {
		bIfIsPhysicalDrive = true;

		
	}
	if (!bSizeSet && (bOfIsPhysicalDrive || bIfIsPhysicalDrive)) {
		GetDriveSize(bIfSet ? hSrcFile : hDestFile, &qwWriteSize);
		printf("GetDriveSize(%s) %llu\n", bIfSet ? "hSrcFile" : "hDestFile", qwWriteSize);
	}
	if (!bSizeSet && bIfSet && !bIfIsPhysicalDrive) {
		GetFileSizeEx(hSrcFile, &li);
		qwWriteSize = li.QuadPart;
		printf("GetFileSizeEx(hSrcFile) %llu\n", qwWriteSize);
	}
	if (!bIfSet && !bDataSet) {
		Exit(EXIT_REASON_NOT_ENOUGH_ARGUMENTS, 9);
	}
	if (!bIfSet) {
		if (!bSizeSet && !bOfIsPhysicalDrive) {
			GetFileSizeEx(hDestFile, &li);
			qwWriteSize = li.QuadPart;

		}
		ret = FillFile(hDestFile, qwWriteSize, qwBufferSize, data);
		bOfSet = false;
	}
	else {
		ret = CopyLargeFile(hSrcFile, hDestFile, qwBufferSize, qwWriteSize);
	}
	
	printf("\nret = %d\n", ret);
	
	
	if (bDeleteFile) {
		if (bDestHandleOpen)
			CloseHandle(hDestFile);
		bool a = DeleteFileA(szOutputFile);
		if (!a)
			printf("DeleteFileA err %d\n", GetLastError());
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
	case EXIT_REASON_INVALID_ARGUMENT: {
		printf("Argument invalide. %d\n", dwExitCode);
		ExitProcess(dwExitCode);
	}
	case EXIT_REASON_PREVENT_CRASH: {
		printf("Erreur d'application. %d\n", dwExitCode);
		ExitProcess(dwExitCode);
	}
	case EXIT_REASON_NOT_ENOUGH_ARGUMENTS: {
		printf("Pas assez d'arguments spécifiés. %d\n", dwExitCode);
		ExitProcess(dwExitCode);
	}
	default:
		break;
	}
}