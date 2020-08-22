#pragma once
#include <Windows.h>
#include "utils.h"

#define ERROR_CHAR_NOT_FOUND -1

typedef unsigned long long QWORD;
void BuildEnvString(LPCSTR, LPSTR);
bool Equal(LPCSTR lpStr1, LPCSTR lpStr2);
bool Equal(LPCSTR lpBuf1, LPCSTR lpBuf2, QWORD qwSize);
int FindCharIndex(LPSTR lpsz, const char ch, int index);
void InvertCase(LPSTR);
DWORD GetChars(LPCSTR, const char);
void ToLower(LPSTR lpszSource);
void ToLower(LPSTR lpszSource, DWORD qwSize);
void ToUpper(LPSTR lpszSource);
void ToUpper(LPSTR lpszSource, DWORD qwSize);
LPSTR GetFileName(LPSTR);
bool Contains(LPCSTR lpsz, const char ch);
bool GetFileExt(LPCSTR lpszFileName, LPSTR lpszDest);
DWORD FindFirstChar(LPCSTR lpStr, const char);
DWORD FindLastChar(LPCSTR lpStr, const char ch);
void ShiftLeft(LPSTR lpsz, DWORD dwLength);
void ShiftRight(LPSTR lpsz, DWORD dwLength);
void RemoveEndSpaces(LPSTR lpsz);
void RemoveEndSpaces(LPSTR lpsz, DWORD dwLength);
void ReplaceAllChars(LPSTR lpsz, const char _old, const char _new);