#include <Windows.h>
#include "diskutils.h"
#include "bases.h"

#define EXIT_REASON_NO_REASON 0U
#define EXIT_REASON_INVALID_ARGUMENT 1U
#define EXIT_REASON_PREVENT_CRASH 2U
#define EXIT_REASON_NOT_ENOUGH_ARGUMENTS 3U
#define EXIT_REASON_ERROR_OTHER 4U
#define EXIT_REASON_TOO_MUCH_ARGUMENTS 5U

void Exit(DWORD dwExitReason, DWORD dwExitCode);
bool StrToQword(LPCSTR lpStr, QWORD* lpQwDest);
bool GetSizeAsString(QWORD qwSize, LPSTR lpDest);


bool bDestHandleOpen = false, bSrcHandleOpen = false, bDebugMode = false;
HANDLE hSrcFile = 0, hDestFile = 0;


int main(int argc, char* argv[]) {
	SetConsoleOutputCP(CP_UTF7);

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
			char szVolumeName[MAX_PATH];
			char szVolumeFS[MAX_PATH];
			szVolumeGUID[48] = 0;
			

			puts("");
			if (!(hVolumeSearch = FindFirstVolumeA(szVolumeGUIDRoot, sizeof szVolumeGUIDRoot))) {
				printf("Impossible de trouver le premier volume. %lu\n", GetLastError());
				Exit(EXIT_REASON_ERROR_OTHER, 2);
			}
			goto inLoop;
			while (FindNextVolumeA(hVolumeSearch, szVolumeGUIDRoot, sizeof szVolumeGUIDRoot)) {
			inLoop:


				memcpy(szVolumeGUID, szVolumeGUIDRoot, 48);
				

				hVolume = CreateFileA(szVolumeGUID, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
				if (hVolume == INVALID_HANDLE_VALUE) {
					DWORD dwLastError = GetLastError();
					if (dwLastError == ERROR_FILE_NOT_FOUND)
						continue;
					printf("CreateFile(%s) err: %lu\n\n", szVolumeGUID, dwLastError);
					CloseHandle(hVolume);
					continue;
				}
				dwDriveType = GetDriveTypeA(szVolumeGUIDRoot);
				if (!DeviceIoControl(hVolume, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL, NULL, &vde, sizeof vde, &dw, NULL)) {
					DWORD dwError = GetLastError();
					if ((dwDriveType != DRIVE_CDROM && dwError != ERROR_INVALID_FUNCTION) && dwError != ERROR_MORE_DATA) {	// Si le disque est un CDROM vide ou un RAID sur plusieurs disques
						printf("DeviceIoControl err %lu\n\n", dwError);
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
					printf("Disque: %lu\n", driveNumber[drivesCount]);
				
				qwVolSize = vde.Extents[0].ExtentLength.QuadPart;
				qwVolOffset = vde.Extents[0].StartingOffset.QuadPart;
				if (qwVolSize == 0)
					puts("Aucun média");
				else {
					char szSizeString[20];
					GetSizeAsString(qwVolSize, szSizeString);
					
					if (bGetVolEx) {

						printf("Taille: %llu (%s)\n", qwVolSize, szSizeString);
						if (bGetVolEx && (qwVolOffset != 0 || driveNumber[drivesCount] != -1)) {
							char szOffsetString[20];
							GetSizeAsString(qwVolOffset, szOffsetString);
							printf("Offset: %llu (%s)\n", qwVolOffset, szOffsetString);
						}
					}
					else printf("Taille: %s\n", szSizeString);
				}

				printf("Système de fichiers: %s\n", *szVolumeFS == 0 ? "RAW" : szVolumeFS);

				
					
				if (bGetVolEx) {
					printf("Type: ");

					switch (dwDriveType) {
					case DRIVE_UNKNOWN: {
						puts("Inconnu");
						break;
					}
					case DRIVE_NO_ROOT_DIR: {
						puts("Mauvais volume");
						break;
					}
					case DRIVE_REMOVABLE: {
						puts("Stockage amovible");
						break;
					}
					case DRIVE_FIXED: {
						puts(driveNumber[drivesCount] != -1 ? "Stockage fixe" : "Volume RAM Virtuel");
						break;
					}
					case DRIVE_REMOTE: {
						puts("Stockage réseau");
						break;
					}
					case DRIVE_CDROM: {
						puts("Lecteur de CDROM");
						break;
					}
					case DRIVE_RAMDISK: {
						puts("Volume RAM Virtuel");
						break;
					}
					default: {
						printf("%llu\n", dwDriveType);
					}
					}
				}

				if (*szVolumeName != 0)
					printf("Nom: %s\n", szVolumeName);

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
			
			Exit(EXIT_REASON_NO_REASON, 0);
		}
		Exit(EXIT_REASON_INVALID_ARGUMENT, 3);
	}

	else if (argc < 3) {

		puts("\n"
				"if=\"fichier source\".\n"
				"of=\"fichier de destination\".\n"
				"bs=n       : Copie par blocs de n octets (1m par défaut)\n"
				"size=n     : Copie seulement n octets.\n"
				"data=octet : Remplit le fichier de destination avec la valeur donnée.\n"
				"delvol     : Supprime les volumes sur le fichier de destination si c'est un disque physique (\\\\.\\PhysicalDrive...). Utilisez ce paramètres si vous appliquez une image plus petite que le disque de destination.\n"
				"getdisk    : Liste tous les disques trouvés sur cet ordinateur et leur capacité.\n"
				"getvol     : Liste les volumes et les informations de base.\n"
				"getvolex   : Pareil que getvol mais affiche plus d'informations.\n"
				"version    : Affiche la date de build."
		);
		
		Exit(EXIT_REASON_NO_REASON, 0);
	}

	byte data = 0;
	byte bIfIsVolume = 0, bOfIsVolume = 0;	// 0 = non, 1 = lettre de volume, 2 = GUID du volume
	char szInputFile[PATH_BUFFER_SIZE];
	char szOutputFile[PATH_BUFFER_SIZE];
	QWORD qwBufferSize = 1 * 1024 * 1024;
	QWORD qwWriteSize = 0;
	QWORD qwIfSp = 0, qwOfSp = 0;	// Pointeurs de fichiers
	
	bool bIfSet = false, bOfSet = false, bSizeSet = false, bBsSet = false, bDataSet = false, bIfIsPhysicalDrive = false,
		bOfIsPhysicalDrive = false, bDeleteIf = false, bDeleteOf = false, bSetFileSize = false, bEndFile = false,
		bDeleteVolumes = false, bSetIfPtr = false, bSetOfPtr = false, bPrealloc = false;
	
	
	for (int i = 1; i < argc; i++) {		// Boucle principale pour parser la ligne de commande
		if (argv[i] == NULL) {
			Exit(EXIT_REASON_PREVENT_CRASH, 6);
		}
		char szCmdLineTempBuffer[9];
		szCmdLineTempBuffer[8] = 0;
		memcpy(szCmdLineTempBuffer, argv[i], 8);
		ToLower(szCmdLineTempBuffer);	// Permet de supporter les options même si il y a des majuscules
		

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
				puts("La taille du buffer doit être un multiple de la taille d'un secteur (512).");
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
				Exit(EXIT_REASON_INVALID_ARGUMENT, 19);
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

		printf("argc = %d\nargv[%d] = %s\n", argc, i, argv[i]);
		Exit(EXIT_REASON_INVALID_ARGUMENT, 28);
	}
	
	if (!bOfSet) {
		Exit(EXIT_REASON_NOT_ENOUGH_ARGUMENTS, 29);
	}
	for (WORD i = 0; i < sizeof szInputFile; i++) {
		if (szInputFile[i] == '/') szInputFile[i] = '\\';
		if (szOutputFile[i] == '/') szOutputFile[i] = '\\';
	}

	LARGE_INTEGER li;
	char szBuffer[50];
	szBuffer[17] = 0;

	if (bIfSet) {
		char szInputFileTemp[PATH_BUFFER_SIZE];
		char szOutputFileTemp[PATH_BUFFER_SIZE];
		strcpy(szInputFileTemp, szInputFile);
		strcpy(szOutputFileTemp, szOutputFile);
		ToLower(szInputFileTemp);
		ToLower(szOutputFileTemp);
		if (Equal(szInputFileTemp, szOutputFileTemp)) {
			puts("Les fichiers d'entrée et de sortie ne peuvent pas être les mêmes.");
			Exit(EXIT_REASON_ERROR_OTHER, 30);
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
		
		memcpy(szBuffer, szInputFile, 17);
		ToLower(szBuffer);
		if (Equal(szBuffer, "\\\\.\\physicaldrive") || Equal(szBuffer, "\\\\?\\physicaldrive")) {
			if (!IsDecimalString(szInputFile + 17)) {
				puts("Le disque source n'est pas valide.");
				Exit(EXIT_REASON_INVALID_ARGUMENT, 381);
			}
			bIfIsPhysicalDrive = true;
		}
		


		hSrcFile = CreateFileA(szInputFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		bSrcHandleOpen = true;
		if (hSrcFile == INVALID_HANDLE_VALUE) {
			printf("Impossible d'ouvrir le fichier source. Code d'erreur : %lu\n", GetLastError());
			Exit(EXIT_REASON_NO_REASON, 31);
		}
		
		if (bSetIfPtr) {
			if (qwIfSp == 0) {
				Exit(EXIT_REASON_INVALID_ARGUMENT, 32);
			}
			li.QuadPart = qwIfSp;
			if (!SetFilePointerEx(hSrcFile, li, 0, FILE_BEGIN)) {
				printf("Impossible de déplacer le pointeur du fichier source: %lu\n", GetLastError());
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
			puts("Le disque de destination n'est pas valide.");
			Exit(EXIT_REASON_INVALID_ARGUMENT, 371);
		}
		bOfIsPhysicalDrive = true;

	}

	if (!GetDrive("\\\\.\\C:", &dwSystemDrive)) {
		printf("Erreur lors de l'obtention du disque système: %lu\n", GetLastError());

	}
	else {
		
		char tmp[8];
		strcpy(tmp, szOutputFile + 17);
		dwOutputFileDriveNumber = atoi(tmp);
		if (dwOutputFileDriveNumber == dwSystemDrive) {
			printf("Le disque sélectionné est le disque système.\nContinuer tout de même ? ");
			char response[256];
			scanf("%s", response);
			ToLower(response);

			if (!Equal(response, "oui")) {
				Exit(EXIT_REASON_NO_REASON, 0);
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
				printf("Impossible de créer le fichier de destination. Code d'erreur : %lu\n", GetLastError());
				Exit(EXIT_REASON_NO_REASON, 34);

			}
		}
		else if (dwLastError != NO_ERROR) {
			printf("Impossible d'ouvrir le fichier de destination. Code d'erreur : %lu\n", dwLastError);
			Exit(EXIT_REASON_NO_REASON, 35);
		

		}
	}

	if (bSetOfPtr) {
		if (qwOfSp == 0) {
			Exit(EXIT_REASON_INVALID_ARGUMENT, 36);
		}
		li.QuadPart = qwOfSp;
		if (!SetFilePointerEx(hDestFile, li, NULL, FILE_BEGIN)) {
			printf("Impossible de déplacer le pointeur du fichier de destination: %lu\n", GetLastError());
			Exit(EXIT_REASON_NO_REASON, 37);
		}
		
	}
	
	DWORD ret;

	const char szPtrPosError[] = "La position du pointeur de fichier doit être un multiple de 512 sur les disques/volumes.";
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
	if (!bSizeSet) {
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
			if ((bIfIsPhysicalDrive || bIfIsVolume) && (bOfIsPhysicalDrive || bOfIsVolume)) {	// Si les deux fichiers sont des périphériques / volumes la taille sera la plus petite

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

	if (bPrealloc && !bOfIsPhysicalDrive && !bOfIsVolume && !bSetFileSize) {
		if ((ret = SetFileSize(hDestFile, qwWriteSize + qwOfSp)) != NO_ERROR) {
			printf("Impossible de préallouer la taille du fichier: %lu\n", ret);
			Exit(EXIT_REASON_ERROR_OTHER, 43);
		}
		li.QuadPart = 0;
		SetFilePointerEx(hDestFile, li, NULL, FILE_BEGIN);
	}

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD coord;

	if (bSetFileSize && !bIfSet) {
		ret = SetFileSize(hDestFile, qwWriteSize + qwOfSp);
	}
	else {

		if (qwBufferSize > qwWriteSize && Remainder(qwWriteSize, 512) == 0) {
			qwBufferSize = qwWriteSize;
		}
		char szProgress[3] = { 0, 0, 0 };
		byte currentProgress = 0, oldProgress = 0;
		QWORD qwRemainderSize = qwWriteSize < 512 ? qwWriteSize : Remainder(qwWriteSize, qwBufferSize);
		QWORD qwPasses = qwWriteSize < 512 ? 0 : qwWriteSize / qwBufferSize;
		LPBYTE lpBuffer = (LPBYTE)malloc(qwBufferSize);
		if (lpBuffer == NULL) {
			puts("Impossible d'allouer la mémoire pour la copie.");
			Exit(EXIT_REASON_PREVENT_CRASH, 431);
		}

		DWORD dwSectorWriteSize = Remainder(qwRemainderSize, 512);
		WORD wInSectorMiddleOffset = (qwRemainderSize >= 512) ? (512 - dwSectorWriteSize) : 512 - qwRemainderSize;
		DWORD dwNumberOfSectors = qwWriteSize / 512;
		bool bInSizeMiddleSector = bIfIsPhysicalDrive || bIfIsVolume ? (dwSectorWriteSize == 0 ? false : true) : false;
		bool bOutSizeMiddleSector = bOfIsPhysicalDrive || bOfIsVolume ? (dwSectorWriteSize == 0 ? false : true) : false;

		if (bDebugMode) {
			if (bIfSet)
				printf("CopyLargeFile(%s, %s, %llu, %llu)\n", szInputFile, szOutputFile, qwBufferSize, qwWriteSize);
			else
				printf("SeqWrite(%s, %llu, %llu, %d)\n", szOutputFile, qwWriteSize, qwBufferSize, data);

		}

		CONSOLE_SCREEN_BUFFER_INFO consoleScreenInfo;
		GetConsoleScreenBufferInfo(hConsole, &consoleScreenInfo);
		coord = consoleScreenInfo.dwCursorPosition;
		coord.X = 0;

		puts("");
		if (coord.Y >= consoleScreenInfo.dwSize.Y - 1)
			coord.Y--;
		
		WriteConsoleOutputCharacterA(hConsole, "0 %", 3, coord, &ret);
		

		if (bIfSet) {

			for (QWORD i = 0; i < qwPasses; i++) {
				if (!ReadFile(hSrcFile, lpBuffer, qwBufferSize, &ret, NULL)) {
					puts("Erreur de lecture!");
					ret = GetLastError();
					goto end;
				}

				if (!WriteFile(hDestFile, lpBuffer, qwBufferSize, &ret, NULL)) {
					puts("Erreur d'écriture!");
					ret = GetLastError();
					goto end;
				}
				currentProgress = i * 100 / qwPasses;
				if (currentProgress != oldProgress) {
					oldProgress = currentProgress;
					_itoa(currentProgress, szProgress, 10);
					WriteConsoleOutputCharacterA(hConsole, szProgress, 2, coord, &ret);
				}


			}
			if (qwRemainderSize == 0) {
				ret = NO_ERROR;
				goto end;
			}

			
			if (!ReadFile(hSrcFile, lpBuffer, bInSizeMiddleSector ? qwRemainderSize + wInSectorMiddleOffset : qwRemainderSize, &ret, NULL)) {
				puts("Erreur de lecture!");
				ret = GetLastError();
				goto end;
			}

			if (bOutSizeMiddleSector) {
				byte sectorDataTemp[512];
				LPBYTE psectorDataTemp = sectorDataTemp;
				li.QuadPart = (QWORD)dwNumberOfSectors * 512 + qwOfSp;
				SetFilePointerEx(hDestFile, li, NULL, FILE_BEGIN);

				if (!ReadFile(hDestFile, sectorDataTemp, 512, &ret, NULL)) {
					puts("Erreur de lecture sur le disque de destination!");
					ret = GetLastError();
					goto end;
				}
				psectorDataTemp += dwSectorWriteSize;
				memcpy(lpBuffer + qwRemainderSize, psectorDataTemp, wInSectorMiddleOffset);
				li.QuadPart = qwWriteSize - qwRemainderSize + qwOfSp;
				SetFilePointerEx(hDestFile, li, NULL, FILE_BEGIN);
			}

			if (!WriteFile(hDestFile, lpBuffer, bOutSizeMiddleSector ? qwRemainderSize + wInSectorMiddleOffset : qwRemainderSize, &ret, NULL)) {
				puts("Erreur d'écriture!");
				ret = GetLastError();
				goto end;
			}

			free(lpBuffer);
			ret = NO_ERROR;
			goto end;
		}

		memset(lpBuffer, data, qwBufferSize);

		for (QWORD i = 0; i < qwPasses; i++) {
			if (!WriteFile(hDestFile, lpBuffer, qwBufferSize, &ret, NULL)) {
				puts("Erreur d'écriture!");
				ret = GetLastError();
				goto end;
			}

			currentProgress = i * 100 / qwPasses;
			if (currentProgress != oldProgress) {
				oldProgress = currentProgress;
				_itoa(currentProgress, szProgress, 10);
				WriteConsoleOutputCharacterA(hConsole, szProgress, 2, coord, &ret);
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
				puts("Erreur de lecture sur le disque de destination!");
				ret = GetLastError();
				goto end;
			}
			psectorDataTemp += dwSectorWriteSize;
			memcpy(lpBuffer + qwRemainderSize, psectorDataTemp, wInSectorMiddleOffset);
			li.QuadPart = qwWriteSize - qwRemainderSize + qwOfSp;
			SetFilePointerEx(hDestFile, li, NULL, FILE_BEGIN);
		}

		if (!WriteFile(hDestFile, lpBuffer, bOutSizeMiddleSector ? qwRemainderSize + wInSectorMiddleOffset : qwRemainderSize, &ret, NULL)) {
			puts("Erreur d'écriture!");
			ret = GetLastError();
			goto end;
		}

		free(lpBuffer);
		ret = NO_ERROR;
		goto end;

	}
	

	
end:
	
	if (ret != NO_ERROR) {
		printf("Code d'erreur: %lu\n", ret);
		Exit(EXIT_REASON_NO_REASON, 0);
	}

	if (!bSetFileSize) {
		
		WriteConsoleOutputCharacterA(hConsole, "100%", 4, coord, &ret);
	}

	
	if (bEndFile && !bOfIsVolume && !bOfIsPhysicalDrive) {
		DWORD dwLastError;
		if (dwLastError = SetFileSize(hDestFile, qwWriteSize + qwOfSp)) {
			printf("SetFileSizeEnd(%s) err: %lu\n", szOutputFile, dwLastError);
		}
	}

	if (bDeleteIf && !bIfIsVolume && !bIfIsPhysicalDrive) {
		if (bSrcHandleOpen) {
			CloseHandle(hSrcFile);
			bSrcHandleOpen = false;
		}
		
		if (!DeleteFileA(szInputFile))
			printf("DeleteFileA(%s) err: %lu\n", szInputFile, GetLastError());
	}

	if (bDeleteOf && !bOfIsVolume && !bOfIsPhysicalDrive) {
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
	const char suffix[] = { 's', 'k', 'm', 'g' }; // (s)ector, (k)ilobyte...
	
	const QWORD qwSuffixSize[] = { 512, 1024, 1048576, 1073741824 };
	DWORD dwStrLength = 0;
	byte lastDecimal = 0;
	while (lpStr[dwStrLength] != 0) {
		if (lpStr[dwStrLength] >= '0' && lpStr[dwStrLength] <= '9')
			lastDecimal++;
		dwStrLength++;
	}
	if (dwStrLength > lastDecimal + 1)	// Erreur si le suffixe est supérieur à 1 caratère
		return false;

	if (lastDecimal == dwStrLength) {	// Si y'a pas de suffixe, la fonction renvoie le chiffre comme il est
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

	_ui64toa((qwSize > 10737418240) ? (qwSize / 1073741824) : (qwSize > 10485760 ? qwSize / 1048576 : qwSize / 1024), lpDest, 10);
	DWORD len = strlen(lpDest);
	lpDest[len] = ' ';
	strcpy(lpDest + len + 1, (qwSize > 10737418240) ? "Go" : (qwSize > 10485760 ? "Mo" : "Ko"));

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
		if (bDebugMode) printf("CloseHandle(hDestFile)...");
		if (!CloseHandle(hDestFile)) {
			printf(" GetLastError(): %lu\n", GetLastError());
		}
		
	}
	

	switch (dwExitReason) {
	case EXIT_REASON_NO_REASON: {
		goto exit;

	}
	case EXIT_REASON_INVALID_ARGUMENT: {
		puts("\nArgument invalide.");
		goto exit;
	}
	case EXIT_REASON_PREVENT_CRASH: {
		puts("\nErreur d'application.");
		goto exit;
	}
	case EXIT_REASON_NOT_ENOUGH_ARGUMENTS: {
		puts("\nPas assez d'arguments spécifiés.");
		goto exit;
	}
	case EXIT_REASON_ERROR_OTHER: {
		puts("\nUne erreur s'est produite.");
		goto exit;
	}
	case EXIT_REASON_TOO_MUCH_ARGUMENTS: {
		puts("\nTrop d'arguments spécifiés.");
		goto exit;
	}
	default:
		break;
	}

exit:
	if (dwExitCode) 
		printf("Code d'arrêt: %lu\n", dwExitCode);
	ExitProcess(dwExitCode);
}