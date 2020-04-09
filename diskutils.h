#pragma once
#include <Windows.h>
#include <stdio.h>
#include "..\..\..\..\Desktop\Mathis\Code\headers\utils.h"

bool LockDriveVolumes(DWORD dwDriveNumber, bool bDeleteVolumes);
bool LockVolume(LPCSTR szVolumePath, bool bDelete);
DWORD GetDriveSize(HANDLE hDrive, QWORD* lpQwSize);
DWORD GetVolumeSize(HANDLE hVolume, QWORD* lpQwSize);