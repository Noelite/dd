#include <Windows.h>
#include "diskutils.h"
#include "bases.h"

#define EXIT_REASON_NO_REASON 0U
#define EXIT_REASON_INVALID_ARGUMENT 1U
#define EXIT_REASON_PREVENT_CRASH 2U
#define EXIT_REASON_NOT_ENOUGH_ARGUMENTS 3U
#define EXIT_REASON_ERROR_OTHER 4U
#define EXIT_REASON_TOO_MUCH_ARGUMENTS 5U

void PrintHelp(char* lpProgramName);
void Exit(DWORD dwExitReason, DWORD dwExitCode);
bool StrToQword(LPCSTR lpStr, QWORD* lpQwDest);
bool GetSizeAsString(QWORD qwSize, LPSTR lpDest);

bool bDestHandleOpen = false, bSrcHandleOpen = false, bDebugMode = false;
HANDLE hSrcFile = 0, hDestFile = 0;


int main(int argc, char* argv[]) {
	SetConsoleOutputCP(CP_UTF7);

	if (argc == 1) {
		PrintHelp(argv[0]);
		Exit(EXIT_REASON_NO_REASON, 0);
	}

	if (argc == 2) {
		char szBuffer[10];
		
		bool bGetVolEx = false;
		DWORD len = strlen(argv[1]);
		memcpy(szBuffer, argv[1], len < 9 ? len : 9);
		szBuffer[len < 9 ? len : 9] = 0;
		ToLower(szBuffer);

		if (Equal(szBuffer, "version")) {
			puts(__DATE__);
			Exit(EXIT_REASON_NO_REASON, 0);
		}
		else if (Equal(szBuffer, "help")) {
			PrintHelp(argv[0]);
			Exit(EXIT_REASON_NO_REASON, 0);
		}

		if (Equal(szBuffer, "getvol", 6)) {
			if (strlen(szBuffer) > 6) {
				if (Equal(szBuffer + 6, "ex")) {
					bGetVolEx = true;
				}
				else {
					Exit(EXIT_REASON_INVALID_ARGUMENT, 1);
				}
			}


			DWORD driveNumber[26];
			DWORD volumesCount = 0, drivesCount = 0, dw;
			VOLUME_DISK_EXTENTS vde;
			DWORD dwDriveType;
			QWORD qwVolSize, qwVolOffset;
			HANDLE hVolume = 0, hVolumeSearch = 0;
			char szVolumeGUID[49];
			char szVolumeGUIDRoot[50];
			char szVolumeName[PATH_BUFFER_SIZE];
			char szVolumeFS[PATH_BUFFER_SIZE];
			szVolumeGUID[48] = 0;
			

			puts("");
			if (!(hVolumeSearch = FindFirstVolumeA(szVolumeGUIDRoot, sizeof szVolumeGUIDRoot))) {
				GetErrorString(szVolumeName, sizeof szVolumeName);
				printf("FindFirstVolumeA(%s) failed.\n%s", szVolumeGUIDRoot, szVolumeName);
				Exit(EXIT_REASON_ERROR_OTHER, 2);
			}
			goto inLoop;
			while (FindNextVolumeA(hVolumeSearch, szVolumeGUIDRoot, sizeof szVolumeGUIDRoot)) {
			inLoop:


				memcpy(szVolumeGUID, szVolumeGUIDRoot, 48);
				

				hVolume = CreateFileA(szVolumeGUID, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
				if (hVolume == INVALID_HANDLE_VALUE) {
					if (GetLastError() == ERROR_FILE_NOT_FOUND)
						continue;
					GetErrorString(szVolumeName, sizeof szVolumeName);
					printf("CreateFile(%s) failed.\n%s", szVolumeGUID, szVolumeName);
					CloseHandle(hVolume);
					continue;
				}
				dwDriveType = GetDriveTypeA(szVolumeGUIDRoot);
				if (!DeviceIoControl(hVolume, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL, NULL, &vde, sizeof vde, &dw, NULL)) {
					DWORD dwError = GetLastError();
					if ((dwDriveType != DRIVE_CDROM && dwError != ERROR_INVALID_FUNCTION) && dwError != ERROR_MORE_DATA) {	// Si le disque est un CDROM vide ou un RAID sur plusieurs disques
						GetErrorString(szVolumeName, sizeof szVolumeName);
						printf("DeviceIoControl(IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS) failed.\n%s", szVolumeName);
					}
					CloseHandle(hVolume);
					continue;
				}
				driveNumber[drivesCount] = vde.Extents[0].DiskNumber;
				*szVolumeFS = 0;
				*szVolumeName = 0;
				GetVolumeInformationA(szVolumeGUIDRoot, szVolumeName, sizeof szVolumeName, 0, 0, 0, szVolumeFS, sizeof szVolumeFS);
				



				printf("Volume %lu  %s\n", volumesCount, szVolumeGUID);

				if (driveNumber[drivesCount] != -1 && bGetVolEx)
					printf("Disk: %lu\n", driveNumber[drivesCount]);
				
				qwVolSize = vde.Extents[0].ExtentLength.QuadPart;
				qwVolOffset = vde.Extents[0].StartingOffset.QuadPart;
				if (qwVolSize == 0)
					puts("No media");
				else {
					char szSizeString[20];
					GetSizeAsString(qwVolSize, szSizeString);
					
					if (bGetVolEx) {

						printf("Size: %llu (%s)\n", qwVolSize, szSizeString);
						if (bGetVolEx && (qwVolOffset != 0 || driveNumber[drivesCount] != -1)) {
							char szOffsetString[20];
							GetSizeAsString(qwVolOffset, szOffsetString);
							printf("Offset: %llu (%s)\n", qwVolOffset, szOffsetString);
						}
					}
					else printf("Size: %s\n", szSizeString);
				}

				printf("File system: %s\n", *szVolumeFS == 0 ? "RAW" : szVolumeFS);

				
					
				if (bGetVolEx) {
					printf("Type: ");

					switch (dwDriveType) {
					case DRIVE_UNKNOWN: {
						puts("Unknown");
						break;
					}
					case DRIVE_NO_ROOT_DIR: {
						puts("Bad volume");
						break;
					}
					case DRIVE_REMOVABLE: {
						puts("Removable storage");
						break;
					}
					case DRIVE_FIXED: {
						puts(driveNumber[drivesCount] != -1 ? "Fixed storage" : "RAM Disk");
						break;
					}
					case DRIVE_REMOTE: {
						puts("Network storage");
						break;
					}
					case DRIVE_CDROM: {
						puts("CDROM reader");
						break;
					}
					case DRIVE_RAMDISK: {
						puts("RAM Disk");
						break;
					}
					default: {
						printf("dwDriveType: %lu\n", dwDriveType);
					}
					}
				}

				if (*szVolumeName != 0)
					printf("Name: %s\n", szVolumeName);

				puts("");

				volumesCount++;
				drivesCount++;

				CloseHandle(hVolume);
			}


			Exit(EXIT_REASON_NO_REASON, 0);
		}

		if (Equal(szBuffer, "getdisk")) {
			char szDriveNumber[11];
			char szPhysicalDriveString[38] = "\\\\.\\PhysicalDrive";

			puts("");
			for (DWORD i = 0; i < 1024; i++) {

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
				printf("Disk %lu", i);
				QWORD qwDriveSize = 0;
				if (GetDriveSize(hDrive, &qwDriveSize) == NO_ERROR) {
					printf("   %lu %s",
						(qwDriveSize > 10737418240) ? (qwDriveSize / 1073741824) : (qwDriveSize > 10485760 ? qwDriveSize / 1048576 : qwDriveSize / 1024),
						(qwDriveSize > 10737418240) ? "Gb" : (qwDriveSize > 10485760 ? "Mb" : "Kb"));
				}
				puts("");
			}
			
			Exit(EXIT_REASON_NO_REASON, 0);
		}
		Exit(EXIT_REASON_INVALID_ARGUMENT, 3);
	}

	byte data = 0;
	byte bIfIsVolume = 0, bOfIsVolume = 0;	// 0 = no, 1 = volume letter, 2 = volume GUID
	char szInputFile[PATH_BUFFER_SIZE];
	char szOutputFile[PATH_BUFFER_SIZE];
	char szUnit[3] = { 0,'b',0 };
	QWORD qwBufferSize = 1 * 1024 * 1024;
	QWORD qwWriteSize = 0;
	QWORD qwIfSp = 0, qwOfSp = 0;	// File pointers
	QWORD qwSizeDivisor = 0;
	
	bool bIfSet = false, bOfSet = false, bSizeSet = false, bBsSet = false, bDataSet = false, bIfIsPhysicalDrive = false,
		bOfIsPhysicalDrive = false, bDeleteIf = false, bDeleteOf = false, bSetFileSize = false, bEndFile = false,
		bDeleteVolumes = false, bSetIfPtr = false, bSetOfPtr = false, bPrealloc = false, bSetOfPtrEnd = false, bUnitSet = false;
	
	
	for (int i = 1; i < argc; i++) {		// Command line parsing loop
		if (argv[i] == NULL) {
			Exit(EXIT_REASON_PREVENT_CRASH, 6);
		}
		char szCmdLineTempBuffer[9];
		szCmdLineTempBuffer[8] = 0;
		memcpy(szCmdLineTempBuffer, argv[i], 8);
		ToLower(szCmdLineTempBuffer);	// Disable case sensitive
		

		if (Equal(szCmdLineTempBuffer, "if=", 3)) {
			if (bIfSet)
				Exit(EXIT_REASON_TOO_MUCH_ARGUMENTS, 7);

			strcpy(szInputFile, argv[i] + 3);
			bIfSet = true;
			continue;

		}

		if (Equal(szCmdLineTempBuffer, "of=", 3)) {
			if (bOfSet)
				Exit(EXIT_REASON_TOO_MUCH_ARGUMENTS, 8);

			strcpy(szOutputFile, argv[i] + 3);
			bOfSet = true;
			continue;

		}

		if (Equal(szCmdLineTempBuffer, "data=", 5)) {
			if (bDataSet)
				Exit(EXIT_REASON_TOO_MUCH_ARGUMENTS, 9);

			data = atoi(argv[i] + 5);
			bDataSet = true;
			continue;

		}
		
		if (Equal(szCmdLineTempBuffer, "bs=", 3)) {
			if (bBsSet)
				Exit(EXIT_REASON_TOO_MUCH_ARGUMENTS, 10);

			if (!StrToQword(argv[i] + 3, &qwBufferSize)) {
				Exit(EXIT_REASON_INVALID_ARGUMENT, 11);
			}
			if (qwBufferSize == 0) {
				Exit(EXIT_REASON_INVALID_ARGUMENT, 12);
			}

			if (Remainder(qwBufferSize, 512) != 0) {
				puts("Buffer size must be a multiple of the sector size (512 bytes).");
				Exit(EXIT_REASON_ERROR_OTHER, 13);
			}
			bBsSet = true;
			continue;

		}

		if (Equal(szCmdLineTempBuffer, "size=", 5)) {
			if (bSizeSet)
				Exit(EXIT_REASON_TOO_MUCH_ARGUMENTS, 14);

			if (!StrToQword(argv[i] + 5, &qwWriteSize)) {
				Exit(EXIT_REASON_INVALID_ARGUMENT, 15);
			}
			bSizeSet = true;
			continue;
		}

		if (Equal(szCmdLineTempBuffer, "ifsp=", 5)) {
			if (bSetIfPtr)
				Exit(EXIT_REASON_TOO_MUCH_ARGUMENTS, 16);

			if (!StrToQword(argv[i] + 5, &qwIfSp)) {
				Exit(EXIT_REASON_INVALID_ARGUMENT, 17);
			}
			bSetIfPtr = true;
			continue;
		}
		
		if (Equal(szCmdLineTempBuffer, "ofsp=", 5)) {
			if (bSetOfPtr)
				Exit(EXIT_REASON_TOO_MUCH_ARGUMENTS, 18);

			if (!StrToQword(argv[i] + 5, &qwOfSp)) {
				if (!Equal(szCmdLineTempBuffer + 5, "app")) {
					Exit(EXIT_REASON_INVALID_ARGUMENT, 19);
				}
				bSetOfPtrEnd = true;
			}
			bSetOfPtr = true;
			continue;
		}

		if (Equal(szCmdLineTempBuffer, "sp=", 3)) {
			if (bSetIfPtr && bSetOfPtr)
				Exit(EXIT_REASON_TOO_MUCH_ARGUMENTS, 20);

			if (!StrToQword(argv[i] + 3, &qwIfSp)) {
				Exit(EXIT_REASON_INVALID_ARGUMENT, 21);
			}
			qwOfSp = qwIfSp;
			bSetIfPtr = true;
			bSetOfPtr = true;
			continue;
		}

		if (Equal(szCmdLineTempBuffer, "unit=", 3)) {
			if (bUnitSet)
				Exit(EXIT_REASON_TOO_MUCH_ARGUMENTS, 20);

			if (szCmdLineTempBuffer[6] != 0) {
				puts("Bad unit.");
				Exit(EXIT_REASON_INVALID_ARGUMENT, 21);
			}


			const char suffix[] = { 'k', 'm', 'g', 't' };
			const QWORD qwSuffixSize[] = { 1024, 1048576, 1073741824, 1099511627776 };
			const char unit = szCmdLineTempBuffer[5];
			for (byte i = 0; i < sizeof(suffix); i++) {
				if (unit == suffix[i]) {
					qwSizeDivisor = qwSuffixSize[i];
					*szUnit = suffix[i] - 32;
					break;
				}
				else if (i == sizeof suffix - 1) {
					puts("Bad unit.");
					Exit(EXIT_REASON_INVALID_ARGUMENT, 22);
				}
			}

			bUnitSet = true;
			continue;
		}

		if (Equal(szCmdLineTempBuffer, "delete", 6)) {
			if (Equal(szCmdLineTempBuffer + 6, "if")) {
				if (bDeleteIf)
					Exit(EXIT_REASON_TOO_MUCH_ARGUMENTS, 22);
				
				bDeleteIf = true;
			}
			else if (Equal(szCmdLineTempBuffer + 6, "of")) {
				if (bDeleteOf)
					Exit(EXIT_REASON_TOO_MUCH_ARGUMENTS, 23);

				bDeleteOf = true;
			}
			else {
				Exit(EXIT_REASON_INVALID_ARGUMENT, 24);
			}
			continue;
		}
		if (Equal(szCmdLineTempBuffer, "delvol")) {
			if (bDeleteVolumes)
				Exit(EXIT_REASON_TOO_MUCH_ARGUMENTS, 25);

			bDeleteVolumes = true;
			continue;
		}
		if (Equal(szCmdLineTempBuffer, "endfile")) {
			if (bEndFile)
				Exit(EXIT_REASON_TOO_MUCH_ARGUMENTS, 26);

			bEndFile = true;
			continue;
		}
		if (Equal(szCmdLineTempBuffer, "prealloc")) {
			if (bPrealloc)
				Exit(EXIT_REASON_TOO_MUCH_ARGUMENTS, 27);

			bPrealloc = true;
			continue;
		}
		if (Equal(szCmdLineTempBuffer, "debug")) {
			if (bDebugMode)
				Exit(EXIT_REASON_TOO_MUCH_ARGUMENTS, 271);

			bDebugMode = true;
			continue;
		}

		printf("Unknown argument \"%s\"\n", argv[i]);
		Exit(EXIT_REASON_INVALID_ARGUMENT, 28);
	}
	
	if (!bOfSet) {
		Exit(EXIT_REASON_NOT_ENOUGH_ARGUMENTS, 29);
	}

	LARGE_INTEGER li;
	char szBuffer[50];
	szBuffer[17] = 0;

	if (bIfSet) {

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
		
		memcpy(szBuffer, szInputFile, 17);
		ToLower(szBuffer);
		if (Equal(szBuffer, "\\\\.\\physicaldrive") || Equal(szBuffer, "\\\\?\\physicaldrive")) {
			if (!IsDecimalString(szInputFile + 17)) {
				puts("Source disk is invalid.");
				Exit(EXIT_REASON_INVALID_ARGUMENT, 381);
			}
			bIfIsPhysicalDrive = true;
		}
		


		hSrcFile = CreateFileA(szInputFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		bSrcHandleOpen = true;
		if (hSrcFile == INVALID_HANDLE_VALUE) {
			GetErrorString(szInputFile, sizeof szInputFile);
			printf("Error opening source file.\n%s", szInputFile);
			Exit(EXIT_REASON_NO_REASON, 0);
		}
		
		if (bSetIfPtr) {
			if (qwIfSp == 0) {
				Exit(EXIT_REASON_INVALID_ARGUMENT, 32);
			}
			li.QuadPart = qwIfSp;
			if (!SetFilePointerEx(hSrcFile, li, 0, FILE_BEGIN)) {
				GetErrorString(szInputFile, sizeof szInputFile);
				printf("SetFilePointerEx(hSrcFile, %llu) failed.\n%s", li.QuadPart, szInputFile);
				Exit(EXIT_REASON_NO_REASON, 33);
			}
		}

	}


	char volumeLetter = szOutputFile[0];
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

	DWORD dwOutputFileDriveNumber, dwSystemDrive;

	memcpy(szBuffer, szOutputFile, 17);
	ToLower(szBuffer);
	if (Equal(szBuffer, "\\\\.\\physicaldrive") || Equal(szBuffer, "\\\\?\\physicaldrive")) {
		if (!IsDecimalString(szOutputFile + 17)) {
			puts("Destination disk is invalid.");
			Exit(EXIT_REASON_INVALID_ARGUMENT, 371);
		}
		bOfIsPhysicalDrive = true;

		if (!GetDrive("\\\\.\\C:", &dwSystemDrive)) {
			GetErrorString(szInputFile, sizeof szInputFile);
			printf("Error detecting system drive.\n%s", szInputFile);

		}
		else if (IsDecimalString(szOutputFile + 17)) {

			char tmp[10];
			strcpy(tmp, szOutputFile + 17);
			dwOutputFileDriveNumber = strtoul(tmp, NULL, 10);
			if (dwOutputFileDriveNumber == dwSystemDrive) {
			a:

				printf("\nThe selected disk is system drive. Still continue ? [y/n] ");
				char response;
				scanf("%c", &response);

				if (response == 'n' && response == 'N') {
					Exit(EXIT_REASON_NO_REASON, 0);
				}
				else if (response != 'y' && response != 'Y') {
					goto a;
				}

			}
		}
	}




	
	hDestFile = CreateFileA(szOutputFile, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH | FILE_ATTRIBUTE_NORMAL, NULL);
	bDestHandleOpen = true;
	if (hDestFile == INVALID_HANDLE_VALUE) {
		DWORD dwLastError = GetLastError();
		if (dwLastError == ERROR_FILE_NOT_FOUND) {
			CloseHandle(hDestFile);
			hDestFile = CreateFileA(szOutputFile, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH | FILE_ATTRIBUTE_NORMAL, NULL);
			if (hDestFile == INVALID_HANDLE_VALUE) {
				GetErrorString(szInputFile, sizeof szInputFile);
				printf("Error creating output file.\n%s", szInputFile);
				Exit(EXIT_REASON_NO_REASON, 0);

			}
		}
		else if (dwLastError != NO_ERROR) {
			GetErrorString(szInputFile, sizeof szInputFile);
			printf("Error opening output file.\n%s", szInputFile);
			Exit(EXIT_REASON_NO_REASON, 0);
		

		}
	}


	
	DWORD ret;

	const char szPtrPosError[] = "File pointer must be a multiple of 512 bytes on disks/volumes.";
	if ((bOfIsPhysicalDrive || bOfIsVolume) && bSetOfPtr) {
		if (Remainder(qwOfSp, 512) != 0) {
			puts(szPtrPosError);
			Exit(EXIT_REASON_ERROR_OTHER, 39);
		}
	}
	if ((bIfIsPhysicalDrive || bIfIsVolume) && bSetIfPtr) {
		if (Remainder(qwIfSp, 512) != 0) {
			puts(szPtrPosError);
			Exit(EXIT_REASON_ERROR_OTHER, 40);
		}
	}
	
	QWORD qwDestSize = 0, qwSrcSize = 0;
	if (!bSizeSet || bSetOfPtrEnd) {
		if (bIfIsPhysicalDrive) {
			GetDriveSize(hSrcFile, &qwSrcSize);
			if (bDebugMode)
				printf("GetDriveSize(hSrcFile) %llu\n", qwSrcSize);
		}
		if (bOfIsPhysicalDrive) {
			GetDriveSize(hDestFile, &qwDestSize);
			if (bDebugMode)
				printf("GetDriveSize(hDestFile) %llu\n", qwDestSize);
		}
		if (bIfSet && !bIfIsPhysicalDrive && !bIfIsVolume) {
			GetFileSizeEx(hSrcFile, &li);
			qwSrcSize = li.QuadPart;
			if (bDebugMode)
				printf("GetFileSizeEx(hSrcFile) %llu\n", qwSrcSize);
		}
		if (bOfSet && !bOfIsPhysicalDrive && !bOfIsVolume) {
			GetFileSizeEx(hDestFile, &li);
			qwDestSize = li.QuadPart;
			if (bDebugMode)
				printf("GetFileSizeEx(hDestFile) %llu\n", qwDestSize);
		}
		if (bIfIsVolume) {
			GetVolumeSize(hSrcFile, &qwSrcSize);
			if (bDebugMode)
				printf("GetVolumeSize(hSrcFile) %llu\n", qwSrcSize);
		}
		if (bOfIsVolume) {
			GetVolumeSize(hDestFile, &qwDestSize);
			if (bDebugMode)
				printf("GetVolumeSize(hDestFile) %llu\n", qwDestSize);
		}
		if (bSetIfPtr)
			qwSrcSize = qwSrcSize - qwIfSp;

		if (bSetOfPtr)
			qwDestSize = qwDestSize - qwOfSp;

		if (bSetOfPtrEnd)
			qwOfSp = qwDestSize;

	}


	
	if (!bIfSet && !bDataSet) {
		if (!bSizeSet)
			Exit(EXIT_REASON_NOT_ENOUGH_ARGUMENTS, 41);

		if (bOfIsPhysicalDrive) {
			Exit(EXIT_REASON_INVALID_ARGUMENT, 42);
		}
		bSetFileSize = true;
		
	}
	if (bOfIsPhysicalDrive) {
		LockDriveVolumes(dwOutputFileDriveNumber, (!bSizeSet && (qwSrcSize == qwDestSize)) || bDeleteVolumes, !bDebugMode);
	}
	else if (bOfIsVolume) {
		LockVolume(hDestFile, bDeleteVolumes);
	}


	if (!bSizeSet) {
		bool tinner = qwSrcSize < qwDestSize; // false = qwSrcSize, true = qwDestSize

		if (qwSrcSize != 0 && qwDestSize != 0) {
			if ((bIfIsPhysicalDrive || bIfIsVolume) && (bOfIsPhysicalDrive || bOfIsVolume)) {	// If both input and output file are disks/volumes, the size will be the smallest

				qwWriteSize = (tinner) ? qwSrcSize : qwDestSize;
				goto startOperation;
			}

			if (bIfIsPhysicalDrive || bIfIsVolume) {
				qwWriteSize = qwSrcSize;
				goto startOperation;
			}

			if (bOfIsPhysicalDrive || bOfIsVolume) {
				qwWriteSize = (bIfSet && tinner) ? qwSrcSize : qwDestSize;
				goto startOperation;
			}

			qwWriteSize = qwSrcSize;
		}
		else {
			if (qwSrcSize == 0) qwWriteSize = qwDestSize;
			else if (qwDestSize == 0) qwWriteSize = qwSrcSize;
		}
		
	}

startOperation:

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	if (bPrealloc && !bOfIsPhysicalDrive && !bOfIsVolume && !bSetFileSize) {
		if ((ret = SetFileSize(hDestFile, qwWriteSize + qwOfSp)) != NO_ERROR) {
			printf("[1] SetFileSize(hDestFile, %llu) failed.\n", qwWriteSize + qwOfSp);
			Exit(EXIT_REASON_ERROR_OTHER, 43);
		}
		li.QuadPart = 0;
		if (!SetFilePointerEx(hDestFile, li, NULL, FILE_BEGIN)) {
			printf("[2] SetFilePointerEx(hDestFile, %llu) failed.\n", li.QuadPart);
			goto end;
		}
	}

	if (bSetOfPtr) {
		if (qwOfSp == 0 && !bSetOfPtrEnd) {
			Exit(EXIT_REASON_INVALID_ARGUMENT, 36);
		}
		li.QuadPart = qwOfSp;
		if (!SetFilePointerEx(hDestFile, li, NULL, FILE_BEGIN)) {
			GetErrorString(szInputFile, sizeof szInputFile);
			printf("[1] SetFilePointerEx(hDestFile, %llu) failed.\n%s", li.QuadPart, szInputFile);
			Exit(EXIT_REASON_NO_REASON, 0);
		}
	}

	COORD coord;

	if (bSetFileSize && !bIfSet) {
		if ((ret = SetFileSize(hDestFile, qwWriteSize + qwOfSp)) != NO_ERROR) {
			printf("[2] SetFileSize(hDestFile, %llu) failed.\n", qwWriteSize + qwOfSp);
			goto end;
		}
		
	}
	else {

		if (qwBufferSize > qwWriteSize && Remainder(qwWriteSize, 512) == 0) {
			qwBufferSize = qwWriteSize;
		}
		char szProgress[3] = { 0, 0, 0 };
		byte currentProgress = 0, oldProgress = 0;
		QWORD qwRemainderSize = qwWriteSize < 512 ? qwWriteSize : Remainder(qwWriteSize, qwBufferSize);
		QWORD qwWrites = qwWriteSize < 512 ? 0 : qwWriteSize / qwBufferSize;
		LPBYTE lpBuffer = (LPBYTE)malloc(qwBufferSize);
		if (lpBuffer == NULL) {
			printf("malloc (%llu) failed for r/w buffer.\n", qwBufferSize);
			Exit(EXIT_REASON_PREVENT_CRASH, 431);
		}

		if (!bUnitSet) {
			qwSizeDivisor = (qwWriteSize >= 107374182400) ? 1073741824 : (qwWriteSize >= 104857600) ? 1048576 : 1024;
			strcpy(szUnit, (qwWriteSize >= 107374182400) ? "Gb" : (qwWriteSize >= 104857600) ? "Mb" : "Kb");
		}
		char szPercent[4] = { 0,0,0 };
		char szTotalSizeString[11], szCurrentSizeString[11];
		COORD coord = { 0,0 };
		COORD coord_percent = { 0,0 };
		CONSOLE_SCREEN_BUFFER_INFO consoleScreenInfo;
		GetConsoleScreenBufferInfo(hConsole, &consoleScreenInfo);
		coord.Y = consoleScreenInfo.dwCursorPosition.Y;
		byte totalSizeTextLength;


		
		
		_ui64toa(qwWriteSize / qwSizeDivisor, szTotalSizeString, 10);
		DWORD len = strlen(szTotalSizeString);
		szTotalSizeString[len] = ' ';
		strcpy(szTotalSizeString + len + 1, szUnit);

		totalSizeTextLength = strlen(szTotalSizeString);
		ZeroMemory(szCurrentSizeString, totalSizeTextLength);
		printf("%.*s%s/%s (   %%)\n", totalSizeTextLength - 2, "           ", szUnit, szTotalSizeString);
		coord_percent.X = coord.X + totalSizeTextLength * 2 + 3;
		coord_percent.Y = coord.Y;
		

		byte percent = 0xFF;

		DWORD dwSectorWriteSize = Remainder(qwRemainderSize, 512);
		WORD wInSectorMiddleOffset = (qwRemainderSize >= 512) ? (512 - dwSectorWriteSize) : 512 - qwRemainderSize;
		DWORD dwNumberOfSectors = qwWriteSize / 512;
		bool bInSizeMiddleSector = bIfIsPhysicalDrive || bIfIsVolume ? (dwSectorWriteSize == 0 ? false : true) : false;
		bool bOutSizeMiddleSector = bOfIsPhysicalDrive || bOfIsVolume ? (dwSectorWriteSize == 0 ? false : true) : false;

		if (bDebugMode) {
			if (bIfSet)
				printf("Input filename: %s\nOutput filename: %s\nBuffer size: %llu\nTotal bytes: %llu\n", szInputFile, szOutputFile, qwBufferSize, qwWriteSize);
			else
				printf("Output filename: %s\nBuffer size: %llu\nbyte: %lu\nTotal bytes: %llu\n", szOutputFile, qwBufferSize, data, qwWriteSize);

		}


		if (coord.Y >= consoleScreenInfo.dwSize.Y - 1)
			coord.Y--;

		

		if (bIfSet) {

			for (QWORD i = 0; i < qwWrites; i++) {
				if (!ReadFile(hSrcFile, lpBuffer, qwBufferSize, &ret, NULL)) {
					puts("[1] Read error!");
					ret = GetLastError();
					goto end;
				}

				if (!WriteFile(hDestFile, lpBuffer, qwBufferSize, &ret, NULL)) {
					puts("[1] Write error!");
					ret = GetLastError();
					goto end;
				}

				_ultoa(((i + 1) * qwBufferSize) / qwSizeDivisor, szCurrentSizeString, 10);
				WriteConsoleOutputCharacterA(hConsole, szCurrentSizeString, totalSizeTextLength - 3, coord, &ret);

				register byte reg_percent_temp = (i + 1) * 100 / qwWrites;
				if (percent != reg_percent_temp) {
					percent = reg_percent_temp;
					_ultoa(reg_percent_temp, szPercent, 10);
					WriteConsoleOutputCharacterA(hConsole, szPercent, 3, coord_percent, &ret);
				}

			}
			if (qwRemainderSize == 0) {
				ret = NO_ERROR;
				goto end;
			}

			
			if (!ReadFile(hSrcFile, lpBuffer, bInSizeMiddleSector ? qwRemainderSize + wInSectorMiddleOffset : qwRemainderSize, &ret, NULL)) {
				puts("[2] Read error!");
				ret = GetLastError();
				goto end;
			}

			if (bOutSizeMiddleSector) {
				byte sectorDataTemp[512];
				LPBYTE psectorDataTemp = sectorDataTemp;
				li.QuadPart = (QWORD)dwNumberOfSectors * 512 + qwOfSp;
				SetFilePointerEx(hDestFile, li, NULL, FILE_BEGIN);

				if (!ReadFile(hDestFile, sectorDataTemp, 512, &ret, NULL)) {
					puts("[3] Read error!");
					ret = GetLastError();
					goto end;
				}
				psectorDataTemp += dwSectorWriteSize;
				memcpy(lpBuffer + qwRemainderSize, psectorDataTemp, wInSectorMiddleOffset);
				li.QuadPart = qwWriteSize - qwRemainderSize + qwOfSp;
				SetFilePointerEx(hDestFile, li, NULL, FILE_BEGIN);
			}

			if (!WriteFile(hDestFile, lpBuffer, bOutSizeMiddleSector ? qwRemainderSize + wInSectorMiddleOffset : qwRemainderSize, &ret, NULL)) {
				puts("[2] Write error!");
				ret = GetLastError();
				goto end;
			}

			free(lpBuffer);
			ret = NO_ERROR;
			goto end;
		}

		memset(lpBuffer, data, qwBufferSize);

		for (QWORD i = 0; i < qwWrites; i++) {
			if (!WriteFile(hDestFile, lpBuffer, qwBufferSize, &ret, NULL)) {
				puts("[3] Write error!");
				ret = GetLastError();
				goto end;
			}

			_ultoa(((i + 1) * qwBufferSize) / qwSizeDivisor, szCurrentSizeString, 10);
			WriteConsoleOutputCharacterA(hConsole, szCurrentSizeString, totalSizeTextLength - 3, coord, &ret);

			register byte reg_percent_temp = (i + 1) * 100 / qwWrites;
			if (percent != reg_percent_temp) {	// Write progress percentage ONLY if changed (performances)
				percent = reg_percent_temp;
				_ultoa(reg_percent_temp, szPercent, 10);
				WriteConsoleOutputCharacterA(hConsole, szPercent, 3, coord_percent, &ret);
			}

		}
		if (qwRemainderSize == 0) {
			ret = NO_ERROR;
			goto end;
		}

		if (bOutSizeMiddleSector) {
			byte sectorDataTemp[512];
			LPBYTE psectorDataTemp = sectorDataTemp;
			li.QuadPart = (QWORD)dwNumberOfSectors * 512 + qwOfSp;
			SetFilePointerEx(hDestFile, li, NULL, FILE_BEGIN);

			if (!ReadFile(hDestFile, sectorDataTemp, 512, &ret, NULL)) {
				puts("[4] Read error!");
				ret = GetLastError();
				goto end;
			}
			psectorDataTemp += dwSectorWriteSize;
			memcpy(lpBuffer + qwRemainderSize, psectorDataTemp, wInSectorMiddleOffset);
			li.QuadPart = qwWriteSize - qwRemainderSize + qwOfSp;
			SetFilePointerEx(hDestFile, li, NULL, FILE_BEGIN);
		}

		if (!WriteFile(hDestFile, lpBuffer, bOutSizeMiddleSector ? qwRemainderSize + wInSectorMiddleOffset : qwRemainderSize, &ret, NULL)) {
			puts("[4] Write error!");
			ret = GetLastError();
			goto end;
		}

		free(lpBuffer);
		ret = NO_ERROR;
		goto end;

	}
	

	
end:
	
	if (ret != NO_ERROR) {
		if (!FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, ret, LANG_USER_DEFAULT, szInputFile, 261, NULL)) {
			_ultoa(ret, szInputFile, 10);
		}
		printf("%s", szInputFile);
		Exit(EXIT_REASON_NO_REASON, 0);
	}


	
	if (bEndFile && !bOfIsVolume && !bOfIsPhysicalDrive) {
		DWORD dwLastError;
		if (dwLastError = SetFileSize(hDestFile, qwWriteSize + qwOfSp)) {
			GetErrorString(szInputFile, sizeof szInputFile);
			printf("[3] SetFileSize(hDestFile, %llu) failed.\n%s", qwWriteSize + qwOfSp, szInputFile);
		}
	}

	if (bDeleteIf && !bIfIsVolume && !bIfIsPhysicalDrive) {
		if (bSrcHandleOpen) {
			CloseHandle(hSrcFile);
			bSrcHandleOpen = false;
		}
		
		if (!DeleteFileA(szInputFile)) {
			GetErrorString(szInputFile, sizeof szInputFile);
			printf("Error deleting source file.\n%s", szInputFile);
		}
	}

	if (bDeleteOf && !bOfIsVolume && !bOfIsPhysicalDrive) {
		if (bDestHandleOpen) {
			CloseHandle(hDestFile);
			bDestHandleOpen = false;
		}

		if (!DeleteFileA(szOutputFile)) {
			GetErrorString(szInputFile, sizeof szInputFile);
			printf("Error deleting output file.\n%s", szInputFile);
		}
	}
	
	Exit(EXIT_REASON_NO_REASON, 0);
}

bool StrToQword(LPCSTR lpStr, QWORD* lpQwDest) {
	const char suffix[] = { 's', 'k', 'm', 'g', 't' }; // (s)ector, (k)ilobyte...
	
	const QWORD qwSuffixSize[] = { 512, 1024, 1048576, 1073741824, 1099511627776 };
	DWORD dwStrLength = 0;
	byte lastDecimal = 0;
	while (lpStr[dwStrLength] != 0) {
		if (lpStr[dwStrLength] >= '0' && lpStr[dwStrLength] <= '9')
			lastDecimal++;
		dwStrLength++;
	}
	if (dwStrLength > lastDecimal + 1)	// Error if suffix is larger than 1 character
		return false;

	if (lastDecimal == dwStrLength) {	// If no suffix, send back the number
		*lpQwDest = strtoull(lpStr, 0, 10);
		return true;
	}

	for (byte i = 0; i < sizeof(suffix); i++) {
		if (lpStr[lastDecimal] == suffix[i] || lpStr[lastDecimal] == (suffix[i] - 32)) {
			char tmp[20];
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

bool GetSizeAsString(QWORD qwSize, LPSTR lpDest) {

	_ui64toa((qwSize > 10737418240) ? (qwSize / 1073741824) : (qwSize > 10485760) ?(qwSize / 1048576) : (qwSize / 1024), lpDest, 10);
	DWORD len = strlen(lpDest);
	lpDest[len] = ' ';
	strcpy(lpDest + len + 1, (qwSize > 10737418240) ? "Gb" : (qwSize > 10485760) ? "Mb" : "Kb");

	return true;
}

void Exit(DWORD dwExitReason, DWORD dwExitCode) {
	
	if (bSrcHandleOpen) {
		if (bDebugMode) printf("CloseHandle(hSrcFile)...");
		if (!CloseHandle(hSrcFile)) {
			printf(" GetLastError(): %lu\n", GetLastError());
		}
		
	}
	
	if (bDestHandleOpen) {
		if (bDebugMode) printf("\nCloseHandle(hDestFile)...");
		if (!CloseHandle(hDestFile)) {
			printf(" GetLastError(): %lu\n", GetLastError());
		}
		
	}
	

	switch (dwExitReason) {
	case EXIT_REASON_NO_REASON: {
		goto exit;

	}
	case EXIT_REASON_INVALID_ARGUMENT: {
		puts("\nInvalid argument.");
		goto exit;
	}
	case EXIT_REASON_PREVENT_CRASH: {
		puts("\nApplication error.");
		goto exit;
	}
	case EXIT_REASON_NOT_ENOUGH_ARGUMENTS: {
		puts("\nNot enouth arguments.");
		goto exit;
	}
	case EXIT_REASON_ERROR_OTHER: {
		puts("\nAn error occured.");
		goto exit;
	}
	case EXIT_REASON_TOO_MUCH_ARGUMENTS: {
		puts("\nToo much arguments.");
		goto exit;
	}
	default:
		break;
	}

exit:
	if (dwExitCode) 
		printf("Stop code: %lu\n", dwExitCode);
	ExitProcess(dwExitCode);
}

void PrintHelp(char* lpProgramName) {
	printf("\n  [Usage]\n"
		"    if=\"source file\".\n"
		"    of=\"destination file\".\n\n"
		"  [Options]\n"
		"    bs=n          : Copy by blocks of n bytes (1m by default)\n"
		"    size=n        : Copy only n bytes.\n"
		"    data=byte     : Fills destination file with byte.\n"
		"    unit=[k|m|g|t]: Set the display unit to u\n"
		"    delvol        : Deletes volumes on output file if it's a drive (\\\\.\\PhysicalDrive...). Use this setting if you writing a smaller image than the disk.\n"
		"    getdisk       : List local drives and their capacity.\n"
		"    getvol        : List volumes and some informations.\n"
		"    getvolex      : Same as getvol but display more info.\n"
		"    version       : Display build date.\n\n"
		"  [Example]\n"
		"    %s if=\\\\.\\PhysicalDrive2 of=backup.img bs=4M\n"
	, lpProgramName
	);

	
}
