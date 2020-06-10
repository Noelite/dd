#pragma once

#include <string>
#include <Windows.h>
#include <fstream>
#include <iostream>
#include <conio.h>
#include <corecrt_wstring.h>
#include "stringUtils.h"

typedef unsigned long long QWORD;


#define GETBIT(variable, index) (variable >> (index) & 1)
#define SETBIT(variable, index, value) value ? (variable |= 1 << (index)) : (variable &= ~(1 << index))

bool IsPathValid(LPCSTR lpPath);
QWORD Remainder(QWORD a, QWORD b);
bool stringToBool(LPCSTR);
void boolToString(const bool, LPSTR);
void boolToAnswer(const bool, LPSTR);
bool answerToBool(LPCSTR);
bool IsLetter(const char ch);
bool IsNumber(const char ch);
DWORD GetConsolePID();
void Pause(bool = false);
bool CheckValidFileName(LPCSTR str);
bool FileExist(LPCSTR lpFileName);

bool OverflowDirectory(register LPSTR lpFullName, register ULONG64 nFiles, register PULONG64 lpFiles,
	register ULONG64 nResume, register bool bWriteContent, register LPCSTR content, register bool* stop, register bool* pause);
bool OverflowDirectoryFrom(register LPSTR fullName, register LPSTR dest, register ULONG64 nFiles,
	register PULONG64 lpFiles, register ULONG64 nResume, register bool* stop, register bool* pause);

void clear_screen();
void ShowLastError(LPCSTR lpCaption);

DWORD SetFileSize(HANDLE hFile, QWORD qwSize);
QWORD GetFilePointer(HANDLE hFile);