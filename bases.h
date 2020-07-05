#pragma once
#ifndef BASES_H_INCLUDED
#define BASES_H_INCLUDED

#include <Windows.h>
#include "..\..\..\..\Desktop\Mathis\Code\headers\utils.h"


#define BIN 0
#define OCTAL 1
#define DECIMAL 2
#define HEXADECIMAL 3	//numerotation de 0 a 3 pour correspondre avec les combobox
#define IsHexSymb(symbol) (symbol >= 'A' && symbol <= 'F' && symbol <= '9' && symbol >= '0')
#define IsHexAszha(symbol) (symbol >= 'A' && symbol <= 'F')

bool IsBinaryString(LPCSTR szStr);
bool IsOctalString(LPCSTR szStr);
bool IsHexString(LPCSTR szStr);
bool IsDecimalString(LPCSTR szStr);

void BinToOct(LPSTR szSrc, LPSTR szDest);
void BinToDecimal(LPSTR szSrc, LPSTR szDest);
void BinToHex(LPSTR szSrc, LPSTR szDest);

void OctToBin(LPSTR szSrc, LPSTR szDest);
void OctToDecimal(LPSTR szSrc, LPSTR szDest);
void OctToHex(LPSTR szSrc, LPSTR szDest);

void DecToBin(LPSTR szSrc, LPSTR szDest);
void DecToOct(LPSTR szSrc, LPSTR szDest);
void DecToHex(LPSTR szSrc, LPSTR szDest);

void HexToBin(LPSTR szSrc, LPSTR szDest);
void HexToOct(LPSTR szSrc, LPSTR szDest);
void HexToDecimal(LPSTR szSrc, LPSTR szDest);
#endif