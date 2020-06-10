#include "bases.h"

const char hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
const char oct[8] = { '0', '1', '2', '3', '4', '5', '6', '7'};

bool isBinaryString(LPCSTR szSrc) {
	USHORT i = 0;
	while (szSrc[i] != 0) {
		if (szSrc[i] != '0' && szSrc[i] != '1') {
			return false;
		}
		i++;
	}
	return true;
}
bool isOctalString(LPCSTR szSrc) {
	USHORT i = 0;
	while (szSrc[i] != 0) {
		if (szSrc[i] < '0' || szSrc[i] > '7') {
			return false;
		}
		i++;
	}
	return true;
}
bool isDecimalString(LPCSTR szSrc) {
	USHORT i = 0;
	while (szSrc[i] != 0) {
		if (szSrc[i] < '0' || szSrc[i] > '9') {
			return false;
		}
		i++;
	}
	return true;
}
bool isHexString(LPCSTR szSrc) {
	USHORT i = 0;
	while (szSrc[i] != 0) {
		if (szSrc[i] < '0' || szSrc[i] > '9' && (szSrc[i] < 'A' || szSrc[i] > 'F')) {
			return false;
		}
		i++;
	}
	return true;
}

void BinToOct(LPSTR szSrc, LPSTR szDest)
{
	int nSymboles = strlen(szSrc) / 3;	//nombre de symboles hexadecimaux
	nSymboles += (strlen(szSrc) % 3) == 0 ? 0 : 1;

	int b = strlen(szSrc);
	register char somme_bits = 0;

	for (int i = nSymboles; i > 0; i--) {

		if (--b >= 0)
			somme_bits += (szSrc[b] - '0');
		if (--b >= 0)
			somme_bits += (szSrc[b] - '0') * 2;
		if (--b >= 0)
			somme_bits += (szSrc[b] - '0') * 4;


		szDest[i - 1] = oct[somme_bits];
		somme_bits = 0;
	}
}

void BinToDecimal(LPSTR szSrc, LPSTR szDest)
{
	
}

void BinToHex(LPSTR szSrc, LPSTR szDest)
{
	int nSymboles = strlen(szSrc) / 4;	//nombre de symboles hexadecimaux
	nSymboles += (bool)(strlen(szSrc) % 4);
	
	int b = strlen(szSrc);
	register char somme_bits = 0;

	for (int i = nSymboles; i > 0; i--) {
		
		if (--b >= 0)
		somme_bits += (szSrc[b] - '0');
		if (--b >= 0)
		somme_bits += (szSrc[b] - '0') * 2;
		if (--b >= 0)
		somme_bits += (szSrc[b] - '0') * 4;
		if (--b >= 0)
		somme_bits += (szSrc[b] - '0') * 8;
		
		
		szDest[i - 1] = hex[somme_bits];
		somme_bits = 0;
	}
}

void OctToBin(LPSTR szSrc, LPSTR szDest)
{
	USHORT nSymboles = strlen(szSrc), nBits = 0;
	char symb = 0;
	char bits[4] = {0, 0, 0, 0};
	for (int i = 0; i < nSymboles; i++) {
		symb = szSrc[i] - '0';	//convertit le symbole en chiffre décimal
		memset(bits, '0', 3);
		for (char o = 2; o >= 0; o--) {
			if (symb == 0) {
				break;
			}
			
			bits[o] = Remainder(symb, 2) + '0';
			symb /= 2;
		}
		
		strcpy(szDest + nBits, bits);
		nBits += 3;
		
	}
}

void OctToDecimal(LPSTR szSrc, LPSTR szDest)
{

}

void OctToHex(LPSTR szSrc, LPSTR szDest)
{


}

void DecToBin(LPSTR szSrc, LPSTR szDest)
{
	
}

void DecToOct(LPSTR szSrc, LPSTR szDest)
{

}

void DecToHex(LPSTR szSrc, LPSTR szDest)
{

}
void HexToBin(LPSTR szSrc, LPSTR szDest)
{
	USHORT nSymboles = strlen(szSrc), nBits = nSymboles * 4;
	DWORD bitsIndex = 0;
	byte somme_bits;
	byte _reste;
	bool isAszhaChar;
	for (USHORT i = 0; i < nSymboles; i++) {
		somme_bits = 0;
		for (byte a = 0; a < 4; a++) {
			isAszhaChar = IsHexAszha(szSrc[nSymboles]);
			_reste = Remainder(isAszhaChar ? szSrc[nSymboles] - 'A' : szSrc[nSymboles] - '0', 2);
			if (isAszhaChar) { if (szSrc[nSymboles] - 'A' == 0) break; }

			szDest[bitsIndex] = _reste;
			bitsIndex++;
		}
	}
	
}

void HexToOct(LPSTR szSrc, LPSTR szDest)
{

}

void HexToDecimal(LPSTR szSrc, LPSTR szDest)
{

}
