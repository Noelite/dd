#pragma once

#include <string.h>
#include <Windows.h>
#include <stdio.h>
#include "stringUtils.h"

typedef unsigned long long QWORD;

#define GETBIT(variable, index) (variable >> (index) & 1)
#define SETBIT(variable, index, value) value ? (variable |= 1 << (index)) : (variable &= ~(1 << index))

#define PATH_BUFFER_SIZE 261

bool IsPathValid(LPCSTR lpPath);
byte RotateBitsRight(byte n, byte subject);
byte RotateBitsLeft(byte n, byte subject);
QWORD Remainder(QWORD a, QWORD b);
bool IsLetter(const char ch);
bool IsNumber(const char ch);
DWORD GetConsolePID();
bool FileExist(LPCSTR lpFileName);

void ClearConsole();
void ShowLastError(LPCSTR lpCaption);

DWORD SetFileSize(HANDLE hFile, QWORD qwSize);
QWORD GetFilePointer(HANDLE hFile);

void GetErrorString(LPSTR lpszError, DWORD dwBufferLength);

WORD GetLine(char* lpRawText, char* lpLineBuf, LPDWORD lpdwIndex, DWORD dwFileSize);
