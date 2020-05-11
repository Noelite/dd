#include "diskutils.h"

bool LockDriveVolumes(DWORD dwDriveNumber, bool bDeleteVolumes, bool bQuiet) {
	DWORD dwVolumes = GetLogicalDrives();
	VOLUME_DISK_EXTENTS vde;
	HANDLE hVolToLock[128];		// 128 = Maximum de partitions sur un disque GPT

	WORD volumeIndex = 0;
	DWORD dw;
	HANDLE hVolumeSearch;
	char szVolumeGUID[49];
	char szVolumeGUIDRoot[50];
	szVolumeGUID[48] = 0;

	if (!(hVolumeSearch = FindFirstVolumeA(szVolumeGUIDRoot, sizeof szVolumeGUIDRoot))) {
		printf("FindFirstVolume err: %lu\n", GetLastError());
		return false;
	}
	goto inLoop;
	while (FindNextVolumeA(hVolumeSearch, szVolumeGUIDRoot, sizeof szVolumeGUIDRoot)) {
	inLoop:

		memcpy(szVolumeGUID, szVolumeGUIDRoot, 48);
		
		
		HANDLE hVolume = CreateFileA(szVolumeGUID, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (hVolume == INVALID_HANDLE_VALUE) {
			printf("Impossible d'ouvrir le volume %s err: %lu\n", szVolumeGUID, GetLastError());
			continue;
		}

		if (!DeviceIoControl(hVolume, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, 0, 0, &vde, sizeof vde, &dw, 0)) {
			DWORD dwError = GetLastError();
			if (dwError == ERROR_MORE_DATA)
				continue;

			if (GetDriveTypeA(szVolumeGUIDRoot) != DRIVE_CDROM) {
				printf("DeviceIoControl volume %s err: %lu\n", szVolumeGUID, dwError);
			}
			

			continue;
		}
		if (vde.Extents[0].DiskNumber == dwDriveNumber) {

			hVolToLock[volumeIndex++] = hVolume;

		}
		else CloseHandle(hVolume);

	}
	for (byte i = 0; i < volumeIndex; i++) {
		
		QWORD qwVolSize;
		if (GetVolumeSize(hVolToLock[i], &qwVolSize) != NO_ERROR || qwVolSize == 0) {
			continue;
		}

		if (!bQuiet)
			printf("Verouillage du volume %d/%d...\n", i + 1, volumeIndex);
		
			if (bDeleteVolumes) {
			byte volData[4096];
			ZeroMemory(volData, sizeof volData);
			if (!WriteFile(hVolToLock[i], volData, sizeof volData, &dw, 0)) {
				printf("WriteFile(hVolToLock[%d]) err: %lu\n", i, GetLastError());
			}
			if (!DeviceIoControl(hVolToLock[i], FSCTL_DISMOUNT_VOLUME, 0, 0, 0, 0, &dw, 0)) {
				printf("DeviceIoControl(FSCTL_DISMOUNT_VOLUME) err: %lu\n", GetLastError());
			}
		}

		if (!DeviceIoControl(hVolToLock[i], FSCTL_LOCK_VOLUME, 0, 0, 0, 0, &dw, 0)) {
			printf("DeviceIoControl(FSCTL_LOCK_VOLUME) err: %lu\n", GetLastError());

		}

	}
	return true;
}

bool LockVolume(HANDLE hVolume, bool bDelete) {

	DWORD dw;

	if (bDelete) {
		byte volData[4096];
		ZeroMemory(volData, sizeof volData);
		if (!WriteFile(hVolume, volData, sizeof volData, &dw, 0)) {
			printf("WriteFile(hVolume) err: %lu\n", GetLastError());
			return false;
		}
	}
	if (!DeviceIoControl(hVolume, FSCTL_LOCK_VOLUME, 0, 0, 0, 0, &dw, 0)) {
		printf("DeviceIoControl(FSCTL_LOCK_VOLUME) err: %lu\n", GetLastError());
		return false;
	}
	return true;
	
}

DWORD GetDriveSize(HANDLE hDrive, QWORD* lpQwSize) {
	DWORD dw;
	GET_LENGTH_INFORMATION diskLengthInfo;

	if (!DeviceIoControl(hDrive, IOCTL_DISK_GET_LENGTH_INFO, 0, 0, &diskLengthInfo, sizeof GET_LENGTH_INFORMATION, &dw, NULL)) {
		return GetLastError();
	}

	*lpQwSize = diskLengthInfo.Length.QuadPart;
	return NO_ERROR;
}

DWORD GetVolumeSize(HANDLE hVolume, QWORD* lpQwSize) {
	DWORD dw;
	VOLUME_DISK_EXTENTS vde;
	if (!DeviceIoControl(hVolume, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, 0, 0, &vde, sizeof vde, &dw, 0)) {
		return GetLastError();
	}
	*lpQwSize = vde.Extents[0].ExtentLength.QuadPart;
	return NO_ERROR;
}

void GetPhysicalDrives(bool* lpbAvailablesDrives, DWORD* lpdwErrors, DWORD dwScanRange) {
	char szDriveNumber[11];
	char szPhysicalDriveString[38] = "\\\\.\\PhysicalDrive";
	
	for (DWORD i = 0; i < dwScanRange; i++) {

		_ultoa(i, szDriveNumber, 10);
		strcpy(szPhysicalDriveString + 17, szDriveNumber);
		HANDLE hDrive = CreateFileA(szPhysicalDriveString, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
		if (hDrive == INVALID_HANDLE_VALUE) {
			DWORD dwLastError = GetLastError();
			if (dwLastError == ERROR_FILE_NOT_FOUND) {
				lpbAvailablesDrives[i] = false;
				continue;
			}
			else {
				lpbAvailablesDrives[i] = true;
				lpdwErrors[i] = dwLastError;
				continue;
			}
		}
		else {
			lpbAvailablesDrives[i] = true;
		}

	}
}