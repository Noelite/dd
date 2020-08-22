#include "utils.h"

void ClearConsole() {
	HANDLE hCons = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwConSize, nCharsWritten;
	_CONSOLE_SCREEN_BUFFER_INFO cbinfo;
	COORD coord_screen = { 0,0 };
	GetConsoleScreenBufferInfo(hCons, &cbinfo);
	dwConSize = cbinfo.dwSize.X * cbinfo.dwSize.Y;
	FillConsoleOutputCharacter(hCons, (CHAR)'\0', dwConSize, coord_screen, &nCharsWritten);
	FillConsoleOutputAttribute(hCons, cbinfo.wAttributes, dwConSize, coord_screen, &nCharsWritten);
	SetConsoleCursorPosition(hCons, coord_screen);
}

bool FileExist(LPCSTR lpFileName)
{
	HANDLE hFile = CreateFile(lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	DWORD dwLastError = GetLastError();
	bool exist = dwLastError != ERROR_FILE_NOT_FOUND && dwLastError != ERROR_INVALID_NAME && dwLastError != ERROR_PATH_NOT_FOUND;
	CloseHandle(hFile);
	return exist;
}

DWORD GetConsolePID() {

	HWND hwndCons = GetConsoleWindow();
	DWORD pid;
	GetWindowThreadProcessId(hwndCons, &pid);
	return pid;
	
}

bool IsLetter(const char ch) {
	return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
}

bool IsNumber(const char ch) {
	return ch <= '9' && ch >= '0';
}

QWORD Remainder(QWORD a, QWORD b) {
	QWORD qwRemainder = a / b;
	return a - qwRemainder * b;
}

bool IsPathValid(LPCSTR lpPath) {
	if (GetChars(lpPath, ':') > 1 &&
		FindFirstChar(lpPath, ':') != 1 &&
		strlen(lpPath) > MAX_PATH) {
		return false;
	}
	return true;
}

void ShowLastError(LPCSTR lpCaption) {
	char errString[10] = "0x";
	_ultoa(GetLastError(), errString + 2, 16);
	ToUpper(errString + 2);
	char lpFullMessage[54] = "Une erreur s'est produite.\nCode d'erreur : ";
	strcat(lpFullMessage, errString);
	MessageBoxA(NULL, lpFullMessage, lpCaption, MB_OK | MB_ICONERROR);
}

DWORD SetFileSize(HANDLE hFile, QWORD qwSize) {
	LARGE_INTEGER li;
	li.QuadPart = qwSize;
	if (!SetFilePointerEx(hFile, li, NULL, FILE_BEGIN)) {
		return GetLastError();
	}
	if (!SetEndOfFile(hFile)) {
		return GetLastError();
	}
	return NO_ERROR;
}

QWORD GetFilePointer(HANDLE hFile) {
	LARGE_INTEGER li, _li = { 0 };
	SetFilePointerEx(hFile, _li, &li, FILE_CURRENT);
	return li.QuadPart;
}