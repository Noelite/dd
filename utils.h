#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

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

bool isPathValid(LPCSTR lpPath);
int reste(int a, int b);
bool stringToBool(LPCSTR);
void boolToString(const bool, LPSTR);
void boolToAnswer(const bool, LPSTR);
bool answerToBool(LPCSTR);
bool isLetter(const char ch, bool upper = false);
bool IsProgramRunFromCommandPrompt();
void Pause(bool = false);
bool CheckValidFileName(LPCSTR str);
bool FileExist(LPCSTR lpFileName);

bool OverflowDirectory(register LPSTR lpFullName, register ULONG64 nFiles, register PULONG64 lpFiles,
	register ULONG64 nResume, register bool bWriteContent, register LPCSTR content, register bool* stop, register bool* pause);
bool OverflowDirectoryFrom(register LPSTR fullName, register LPSTR dest, register ULONG64 nFiles,
	register PULONG64 lpFiles, register ULONG64 nResume, register bool* stop, register bool* pause);

void clear_screen();
void ShowLastError(LPCSTR lpCaption);
DWORD CopyLargeFile(HANDLE hSrcFile, HANDLE hDestFile, QWORD qwBufferSize, QWORD qwFileSize);
DWORD FillFile(HANDLE hFile, QWORD qwSize, QWORD qwBufferSize, BYTE data);
DWORD GetDriveSize(HANDLE hDrive, QWORD* lpQwSize);
bool LockDriveVolumes(DWORD dwDriveNumber, bool bDeleteVolumes);

#endif