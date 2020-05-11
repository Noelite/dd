#pragma once
#include <Windows.h>
#include <stdio.h>
#include "..\..\..\..\Desktop\Mathis\Code\headers\utils.h"

bool LockDriveVolumes(DWORD dwDriveNumber, bool bDeleteVolumes, bool bQuiet = false);
bool LockVolume(HANDLE hVolume, bool bDelete);
DWORD GetDriveSize(HANDLE hDrive, QWORD* lpQwSize);
DWORD GetVolumeSize(HANDLE hVolume, QWORD* lpQwSize);
void GetPhysicalDrives(bool* lpbAvailablesDrives, DWORD* lpdwErrors, DWORD dwScanRange);