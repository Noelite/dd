#pragma once

#include <string>
#include <Windows.h>
#include <fstream>
#include <iostream>
#include "stringUtils.h"

typedef unsigned long long QWORD;

#define GETBIT(variable, index) (variable >> (index) & 1)
#define SETBIT(variable, index, value) value ? (variable |= 1 << (index)) : (variable &= ~(1 << index))

#define PATH_BUFFER_SIZE 261

bool IsPathValid(LPCSTR lpPath);
QWORD Remainder(QWORD a, QWORD b);
bool IsLetter(const char ch);
bool IsNumber(const char ch);
DWORD GetConsolePID();
bool FileExist(LPCSTR lpFileName);

void ClearConsole();
void ShowLastError(LPCSTR lpCaption);

DWORD SetFileSize(HANDLE hFile, QWORD qwSize);
QWORD GetFilePointer(HANDLE hFile);