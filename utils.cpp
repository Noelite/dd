#include "utils.h"

void clear_screen() {
	HANDLE hCons = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwConSize, nCharsWritten;
	_CONSOLE_SCREEN_BUFFER_INFO cbinfo;
	COORD coord_screen = { 0,0 };
	GetConsoleScreenBufferInfo(hCons, &cbinfo);
	dwConSize = cbinfo.dwSize.X * cbinfo.dwSize.Y;
	FillConsoleOutputCharacter(hCons, (CHAR)' ', dwConSize, coord_screen, &nCharsWritten);
	FillConsoleOutputAttribute(hCons, cbinfo.wAttributes, dwConSize, coord_screen, &nCharsWritten);
	SetConsoleCursorPosition(hCons, coord_screen);

}
bool FileExist(LPCSTR lpFileName)
{
	HANDLE hFile = CreateFile(lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	bool exist = GetLastError() != ERROR_FILE_NOT_FOUND;
	CloseHandle(hFile);
	return exist;
}

bool OverflowDirectory(register LPSTR lpFullName, register ULONG64 nFiles, register PULONG64 lpFiles,
	register ULONG64 nResume, register bool bWriteContent, register LPCSTR content, register bool* stop, register bool* pause)
{
	//Optimisation maximale. Ce code pourra être éxécuté des milliards de fois (nofake)
	register char lpFileName[FILENAME_MAX];
	strcpy(lpFileName, lpFullName);
	register bool isExtValid = false,
		isFileNameValid = true;
	register unsigned int fullName_lenght;
	register char countIndex[21];		
	char ext[20];
	
	ZeroMemory(countIndex, sizeof countIndex);
	register DWORD dw, size;
	register HANDLE file;
	if (bWriteContent) {
		size = strlen(content);
	}
	
	GetFileExt(lpFullName, ext);
	if (ext != nullptr) {
		
		isExtValid = true;
		
		MakeFileNameValid(ext);
		
		RemoveFileExt(lpFileName);
		
	}
	bool filenameContainsDriveLetter = 0;
	if (isLetter(lpFileName[0], true) && lpFileName[1] == ':')
		filenameContainsDriveLetter = true;
	if (isLetter(lpFileName[0], false) && lpFileName[1] == ':') { //si le nom de fichier commence par une lettre de lecteur
		*lpFileName -= 32;
		filenameContainsDriveLetter = true;
	}
	MakeFileNameValid(filenameContainsDriveLetter ? lpFileName + 2 : lpFileName);
	
	strcpy(lpFullName, lpFileName);
	fullName_lenght = strlen(lpFullName);
	register char endl = '\n';
	register ULONG64 i = 0;
	lpFiles = &i;
	for (i = nResume; i < nFiles; i++) {
		
		if (isFileNameValid)
			lpFullName[fullName_lenght] = 0;
			
		
		_ui64toa(i, countIndex, 10);
		strcpy(lpFullName + fullName_lenght, countIndex);

		if (isExtValid) 
			strcat(lpFullName, ext);
		
		
		file = CreateFileA(lpFullName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, NULL, NULL);
		if (bWriteContent) 
			WriteFile(file, content, size, &dw, NULL);
		while (*pause) Sleep(100);
		if (*stop)
			return false;
	}
	return true;

}
bool OverflowDirectoryFrom(register LPSTR fullName, register LPSTR dest, register ULONG64 nFiles, register PULONG64 lpFiles,
	register ULONG64 nResume, register bool* stop, register bool* pause)
{
	
	register char fileName[FILENAME_MAX - 20];
	strcpy(fileName, fullName);
	register bool isExtValid = false,
		isFileNameValid = true;
	DWORD fullName_lenght;
	char countIndex[20];
	char ext[20];

	ZeroMemory(countIndex, sizeof countIndex);

	GetFileExt(fullName, ext);
	if (ext != nullptr) {

		isExtValid = true;

		MakeFileNameValid(ext);

		RemoveFileExt(fileName);

	}
	bool filenameContainsDriveLetter = 0;
	if (isLetter(fileName[0], false) && fileName[1] == ':') { //si le nom de fichier commence par une lettre de lecteur
		*fileName -= 32;
		filenameContainsDriveLetter = true;
	}
	MakeFileNameValid(filenameContainsDriveLetter ? fileName + 2 : fileName);

	strcpy(fullName, fileName);
	fullName_lenght = strlen(fullName);
	register ULONG64 i = 0;
	lpFiles = &i;
	for (i = nResume; i < nFiles; i++) {

		if (isFileNameValid)
			fullName[fullName_lenght] = 0;

		_ui64toa(i, countIndex, 10);
		strcat(fullName, countIndex);

		if (isExtValid)
			strcat(fullName, ext);

		CopyFileA(fileName, fullName, 0);
		
		while (*pause) Sleep(100);
		if (*stop)
			return false;

	}
	return true;
	
}
bool IsProgramRunFromCommandPrompt() {

	HWND hwndCons = GetConsoleWindow();
	DWORD pid;
	GetWindowThreadProcessId(hwndCons, &pid);
	return GetCurrentProcessId() == pid;

	
}

void Pause(bool state) {
	if (state)
		std::cout << "Appuyez sur une touche pour continuer...";
	_getch();
}

bool isLetter(const char _char, bool upper) {
	if (upper) {
		return _char >= 'A' && _char <= 'Z';
	}
	return _char >= 'a' && _char <= 'z';
}
void boolToString(const bool bl, LPSTR lpDest) {
	if (bl) {
		strcpy(lpDest, "true");
		return;
	}
	strcpy(lpDest, "false");
}
bool stringToBool(LPCSTR string) {
	if (strlen(string) > 4) {
		return false;
	}
	char _str[5];
	strcpy(_str, string);
	ToLower(_str);
	if (strcmp(_str, "true") == 0) {
		return true;
	}
	return false;
}
void boolToAnswer(const bool bl, LPSTR lpDest) {
	bl ? strcpy(lpDest, "Oui") : strcpy(lpDest, "Non");
}
bool answerToBool(LPCSTR ans) {
	if (strlen(ans) > 3)
		return false;

	char _ans[4];
	strcpy(_ans, ans);
	ToLower(_ans);

	if (strcmp(_ans, "oui") == 0)
		return true;
	return false;
}
int reste(int a, int b) {
	int _reste = (int)a / b;
	return a - _reste * b;
}
bool isPathValid(LPCSTR lpPath) {
	if (HowMany(lpPath, ':') > 1 &&
		FindFirstChar(lpPath, ':') != 1 &&
		strlen(lpPath) > _MAX_PATH) {
		return false;
	}
	return true;
}
void ShowLastError(LPCSTR lpCaption) {
	char errString[10] = "0x";
	_itoa(GetLastError(), errString + 2, 16);
	ToUpper(errString + 2);
	char lpFullMessage[54] = "Une erreur s'est produite.\nCode d'erreur : ";
	strcat(lpFullMessage, errString);
	MessageBox(NULL, lpFullMessage, lpCaption, MB_OK | MB_ICONERROR);
}
bool LockDriveVolumes(DWORD dwDriveNumber, bool bDeleteVolumes) {
	DWORD dwVolumes = GetLogicalDrives();
	VOLUME_DISK_EXTENTS vde;
	HANDLE hVolToLock[25];
	char volumeLetterToLock[25];
	byte volumeIndex = 0;
	DWORD dw;
	printf("Préparation du verouillage des systèmes de fichiers sur \\\\.\\PhysicalDrive%d...\n", dwDriveNumber);
	for (byte i = 0; i < 26; i++) {

		if (GETBIT(dwVolumes, i)) {
			char szVol[7] = "\\\\.\\ :";
			szVol[4] = i + 'A';
			HANDLE hVolume = CreateFileA(szVol, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
			if (hVolume == INVALID_HANDLE_VALUE) {
				printf("Impossible d'ouvrir le volume %s GetLastError() %d\n", szVol, GetLastError());
				continue;
			}

			if (!DeviceIoControl(hVolume, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, 0, 0, &vde, sizeof vde, &dw, 0)) {
				if (GetDriveTypeA(szVol) != DRIVE_CDROM) {
					printf("DeviceIoControl volume %s GetLastError %d\n", szVol, GetLastError());
				}

				continue;
			}
			if (vde.Extents[0].DiskNumber == dwDriveNumber) {
				volumeLetterToLock[volumeIndex] = i + 'A';
				hVolToLock[volumeIndex++] = hVolume;
				
			}
			else CloseHandle(hVolume);
		}
	}
	for (byte i = 0; i < volumeIndex; i++) {
		printf("Verouillage du volume %c: (%d/%d)...\n", volumeLetterToLock[i], i + 1, volumeIndex);

		byte volData[4096];
		ZeroMemory(volData, sizeof volData);
		if (!WriteFile(hVolToLock[i], volData, sizeof volData, &dw, 0)) {
			printf("WriteFile(hVolToLock[%d]) GetLastError() %d\n", i, GetLastError());
		}
		if (!DeviceIoControl(hVolToLock[i], FSCTL_LOCK_VOLUME, 0, 0, 0, 0, &dw, 0)) {
			printf("DeviceIoControl(FSCTL_LOCK_VOLUME) GetLastError() %d\n", GetLastError());

		}

	}
	return true;
}
DWORD CopyLargeFile(HANDLE hSrcFile, HANDLE hDestFile, QWORD qwBufferSize, QWORD qwFileSize) {
	
	
	DWORD dwRemainderSize = reste(qwFileSize, qwBufferSize);
	DWORD dwPasses = qwFileSize / qwBufferSize;
	LPBYTE lpBuffer = new BYTE[qwBufferSize];
	DWORD dw;
	printf("CopyLargeFile(%p, %p, %d, %llu)\n\n", hSrcFile, hDestFile, qwBufferSize, qwFileSize);

	char szProgress[3] = {0, 0, 0};
	HANDLE hCons = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD coord;
	CONSOLE_SCREEN_BUFFER_INFO consoleScreenInfo;
	GetConsoleScreenBufferInfo(hCons, &consoleScreenInfo);
	coord = consoleScreenInfo.dwCursorPosition;
	coord.Y--;
	WriteConsoleOutputCharacterA(hCons, "  %", 3, coord, &dw);

	for (DWORD i = 0; i < dwPasses; i++) {
		if (!ReadFile(hSrcFile, lpBuffer, qwBufferSize, &dw, NULL)) {
			delete[] lpBuffer;
			printf("Erreur de lecture :\nPasse: %d\nPosition: %d\n", i, SetFilePointer(hSrcFile, 0, 0, FILE_CURRENT));
			return GetLastError();
		}
		if (!WriteFile(hDestFile, lpBuffer, qwBufferSize, &dw, NULL)) {
			delete[] lpBuffer;
			printf("Erreur d'écriture :\nPasse: %d\nPosition: %d\n", i, SetFilePointer(hDestFile, 0, 0, FILE_CURRENT));
			return GetLastError();
		}
		
		_itoa(i * 100 / dwPasses, szProgress, 10);
		WriteConsoleOutputCharacterA(hCons, szProgress, 2, coord, &dw);
		
	}
	if (dwRemainderSize == 0) return NO_ERROR;
	
	ZeroMemory(lpBuffer, qwBufferSize);
	if (!ReadFile(hSrcFile, lpBuffer, dwRemainderSize, &dw, NULL)) {
		delete[] lpBuffer;
		return GetLastError();
	}
	
	if (!WriteFile(hDestFile, lpBuffer, dwRemainderSize, &dw, NULL)) {
		
		delete[] lpBuffer;
		return GetLastError();
	}
	WriteConsoleOutputCharacterA(hCons, "100%", 4, coord, &dw);
	delete[] lpBuffer;
	return NO_ERROR;
}
DWORD FillFile(HANDLE hFile, QWORD qwSize, QWORD qwBufferSize, BYTE data) {
	DWORD dwRemainderSize = reste(qwSize, qwBufferSize);
	DWORD dwPasses = qwSize / qwBufferSize;
	LPBYTE lpBuffer = new BYTE[qwBufferSize];
	memset(lpBuffer, data, qwBufferSize);
	DWORD dw;
	printf("FillFile(%p, %llu, %llu, %d)\n\n", hFile, qwSize, qwBufferSize, data);

	char szProgress[3] = { 0, 0, 0 };
	HANDLE hCons = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD coord;
	CONSOLE_SCREEN_BUFFER_INFO consoleScreenInfo;
	GetConsoleScreenBufferInfo(hCons, &consoleScreenInfo);
	coord = consoleScreenInfo.dwCursorPosition;
	coord.Y--;
	WriteConsoleOutputCharacterA(hCons, "  %", 3, coord, &dw);

	for (DWORD i = 0; i < dwPasses; i++) {
		if (!WriteFile(hFile, lpBuffer, qwBufferSize, &dw, NULL)) {
			delete[] lpBuffer;
			printf("Erreur d'écriture :\nPasse: %d\nPosition: %d\n", i, SetFilePointer(hFile, 0, 0, FILE_CURRENT));
			return GetLastError();
		}

		_itoa(i * 100 / dwPasses, szProgress, 10);
		WriteConsoleOutputCharacterA(hCons, szProgress, 2, coord, &dw);
	}
	if (dwRemainderSize == 0) return NO_ERROR;

	if (!WriteFile(hFile, lpBuffer, dwRemainderSize, &dw, NULL)) {
		delete[] lpBuffer;
		printf("Erreur d'écriture :\nPosition: %d\n", SetFilePointer(hFile, 0, 0, FILE_CURRENT));
		return GetLastError();
	}
	WriteConsoleOutputCharacterA(hCons, "100%", 4, coord, &dw);
	delete[] lpBuffer;
	return NO_ERROR;
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