#pragma once
#ifndef STRINGUTILS_H_INCLUDED
#define STRINGUTILS_H_INCLUDED
#include <Windows.h>
#include "utils.h"
#include "bases.h"

#define ERROR_CHAR_NOT_FOUND -1


void BuildEnvString(LPCSTR, LPSTR);
bool Equal(LPCSTR lpStr1, LPCSTR lpStr2);
int FindCharIndex(LPSTR string, const char charToFind, int indexInString);
void RawStrCpy(LPCSTR lpSrc, LPSTR lpDest);
void InvertCharsUpperLower(LPSTR);
DWORD HowMany(LPCSTR, const char);
void ToLower(LPSTR);
void ToUpper(LPSTR);
void MakeFileNameValid(LPSTR str);
LPSTR GetFileName(LPSTR);
bool Contains(LPCSTR lpStr, const char ch);
bool Contains(LPCWSTR, const wchar_t);
DWORD FindFirstChar(LPCSTR, const char);
void GetFileExt(LPCSTR lpFileName, LPSTR lpDest);
bool RemoveFileExt(LPSTR);
int FindLastChar(LPCSTR lpStr, const char ch);
void ShiftLeft(LPSTR lpStr, DWORD lenght);
void ShiftRight(LPSTR lpStr, DWORD lenght);

#endif