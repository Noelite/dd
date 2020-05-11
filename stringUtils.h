#pragma once
#include <Windows.h>
#include "utils.h"
#include "bases.h"

#define ERROR_CHAR_NOT_FOUND -1

typedef unsigned long long QWORD;
void BuildEnvString(LPCSTR, LPSTR);
bool Equal(LPCSTR lpStr1, LPCSTR lpStr2);
bool Equal(LPCSTR lpBuf1, LPCSTR lpBuf2, QWORD qwSize);
int FindCharIndex(LPSTR string, const char charToFind, int indexInString);
void InvertCharsUpperLower(LPSTR);
DWORD HowMany(LPCSTR, const char);
void ToLower(LPSTR lpszSource);
void ToLower(LPSTR lpszSource, QWORD qwSize);
void ToUpper(LPSTR lpszSource);
void ToUpper(LPSTR lpszSource, QWORD qwSize);
void MakeFileNameValid(LPSTR str);
LPSTR GetFileName(LPSTR);
bool Contains(LPCSTR lpStr, const char ch);
bool Contains(LPCWSTR, const wchar_t);
void GetFileExt(LPCSTR lpFileName, LPSTR lpDest);
bool RemoveFileExt(LPSTR);
DWORD FindFirstChar(LPCSTR lpStr, const char);
DWORD FindLastChar(LPCSTR lpStr, const char ch);
void ShiftLeft(LPSTR lpStr, DWORD lenght);
void ShiftRight(LPSTR lpStr, DWORD lenght);