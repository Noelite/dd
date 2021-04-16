#pragma once
#include <Windows.h>
#include <stdio.h>
#include "utils.h"

bool LockDriveVolumes(DWORD dwDriveNumber, bool bDeleteVolumes, bool bQuiet = false);
bool LockVolume(HANDLE hVolume, bool bDelete);
DWORD GetDriveSize(HANDLE hDrive, QWORD* lpqwSize);
DWORD GetVolumeSize(HANDLE hVolume, QWORD* lpqwSize);
void GetPhysicalDrives(bool* lpbAvailablesDrives, DWORD* lpdwErrors, DWORD dwScanRange);
DWORD GetDrive(LPCSTR lpszVolume, LPDWORD lpdwDrive);
