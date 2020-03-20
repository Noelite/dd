#include <Windows.h>
#include "..\..\..\..\Desktop\Mathis\Code\headers\utils.h"


#define EXIT_REASON_NO_REASON 0U
#define EXIT_REASON_INVALID_ARGUMENT 1U
#define EXIT_REASON_PREVENT_CRASH 2U
#define EXIT_REASON_NOT_ENOUGH_ARGUMENTS 3U
#define EXIT_REASON_VOLUME_LOCK_ERROR 4U
#define EXIT_REASON_VOLUME_UNLOCK_ERROR 5U

void Exit(DWORD dwExitReason, DWORD dwExitCode);
DWORD GetDriveSize(HANDLE hDrive, QWORD* lpQwSize);

bool bIfSet = false, bOfSet = false, bSizeSet = false, bBsSet = false, bDataSet = false, bIsPhysicalDrive = false;
HANDLE hSrcFile = 0, hDestFile = 0;

int main(int argc, char* argv[]) {

	if (argc < 3) {
		int index = FindLastChar(argv[0], '.');
		if (index != ERROR_CHAR_NOT_FOUND && memcmp(argv[0] + index, "exe", 3) == 0) {
			argv[index] = 0;

		}
		printf("Arguments pour %s:\n"
			   "if=\"fichier source\"\n"
			   "of=\"fichier de destination\"\n"
			   "bs=taille du buffer de copie (65536 par défaut)\n"
			   "size=taille à copier (en octets)\n"
			   "data=nombre (de 0 à 255, optionnel si if est renseigné)\n", argv[0]);
		
		Exit(EXIT_REASON_NO_REASON, 1);
	}
	
	byte data = 0;
	char szInputFile[FILENAME_MAX];
	char szOutputFile[FILENAME_MAX];
	DWORD dwBufferSize = 65536;
	QWORD qwWriteSize = 0;
	

	

	for (int i = 1; i < argc; i++) {
		if (argv[i] == NULL) {
			Exit(EXIT_REASON_PREVENT_CRASH, 2);
		}
		if (memcmp(argv[i], "if=", 3) == 0) {
			strcpy(szInputFile, argv[i] + 3);
			hSrcFile = CreateFileA(szInputFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
			bIfSet = true;
			continue;

		}

		if (memcmp(argv[i], "of=", 3) == 0) {
			strcpy(szOutputFile, argv[i] + 3);
			hDestFile = CreateFileA(szOutputFile, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
			if (GetLastError() == ERROR_FILE_NOT_FOUND) {
				CloseHandle(hDestFile);
				hDestFile = CreateFileA(szOutputFile, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, CREATE_ALWAYS, NULL, NULL);
			}
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
			dwBufferSize = atol(tmp);
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
			qwWriteSize = atoll(tmp);
			bSizeSet = true;
			continue;
		}
		printf("i = %d\nargc = %d\nargv[%d] = %s", i, argc, i, argv[i]);
		Exit(EXIT_REASON_INVALID_ARGUMENT, 6);
	}
	
	LARGE_INTEGER li;
	DWORD ret = 0;
	/*if (memcmp(szOutputFile + 3, "\\\\.\\", 4) == 0 && szOutputFile[7] == ':') {
		if (szOutputFile[7] >= 'a' || szOutputFile[7] <= 'z')
			szOutputFile[7] += 32;
		if (isLetter(szOutputFile[7], true)) {
			if (!DeviceIoControl(hDestFile, FSCTL_LOCK_VOLUME, 0, 0, 0, 0, &ret, 0)) {
				printf("DeviceIoControl(hDestFile, FSCTL_LOCK_VOLUME, ...)\nCode d'erreur : %d\n", GetLastError());
				Exit(EXIT_REASON_VOLUME_LOCK_ERROR, 7);

			}
			bIsPhysicalDrive = true;
		}
		else {
			Exit(EXIT_REASON_INVALID_ARGUMENT, 8);
		}
	}*/

	
	if (!bSizeSet) {
		GetFileSizeEx(hSrcFile, &li);
		qwWriteSize = li.QuadPart;
	}
	printf("szOf: %s\n", szOutputFile);
	if (memcmp(szOutputFile, "\\\\.\\PhysicalDrive", 17) == 0) {
		bIsPhysicalDrive = true;
		if (!DeviceIoControl(hDestFile, FSCTL_LOCK_VOLUME, 0, 0, 0, 0, &ret, 0)) {
			printf("DeviceIoControl(hDestFile, FSCTL_LOCK_VOLUME, ...)\nCode d'erreur : %d\n", GetLastError());
			Exit(EXIT_REASON_VOLUME_LOCK_ERROR, 7);

		}
		GetDriveSize(hDestFile, &qwWriteSize);
		printf("GetDriveSize %llu\n", qwWriteSize);
	}

	if (!bIfSet && !bDataSet) {
		Exit(EXIT_REASON_NOT_ENOUGH_ARGUMENTS, 9);
	}
	if (!bIfSet) {
		if (!bSizeSet && !bIsPhysicalDrive) {
			GetFileSizeEx(hDestFile, &li);
			qwWriteSize = li.QuadPart;

		}
		printf("call FillFile(%p, %llu, %lu, %d)\n", hDestFile, qwWriteSize, dwBufferSize, data);
		ret = FillFile(hDestFile, qwWriteSize, dwBufferSize, data);
	}
	else {
		printf("call CopyLargeFile(%p, %p, %d, %llu)\n", hSrcFile, hDestFile, dwBufferSize, qwWriteSize);
		ret = CopyLargeFile(hSrcFile, hDestFile, dwBufferSize, qwWriteSize);
	}
	if (ret) {
		printf("Error\nret = %d\n", ret);
	}
	if (bIsPhysicalDrive) {
		if (!DeviceIoControl(hDestFile, FSCTL_UNLOCK_VOLUME, 0, 0, 0, 0, &ret, 0)) {
			printf("DeviceIoControl(hDestFile, FSCTL_UNLOCK_VOLUME, ...)\nCode d'erreur : %d\n", GetLastError());
			Exit(EXIT_REASON_VOLUME_UNLOCK_ERROR, 7);
		}
	}
	Exit(EXIT_REASON_NO_REASON, 0);
}

void Exit(DWORD dwExitReason, DWORD dwExitCode) {
	if (bIfSet)
		CloseHandle(hSrcFile);

	if (bOfSet)
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
	case EXIT_REASON_VOLUME_LOCK_ERROR: {
		printf("Impossible de verouiler le volume. %d\n", dwExitCode);
		ExitProcess(dwExitCode);
	}
	case EXIT_REASON_VOLUME_UNLOCK_ERROR: {
		printf("Impossible de déverouiler le volume. %d\n", dwExitCode);
		ExitProcess(dwExitCode);
	}
	default:
		break;
	}
}