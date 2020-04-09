#include "stringUtils.h"


void BuildEnvString(LPCSTR lpSrc, LPSTR lpDest) {
	DWORD dwDestPos = 0;
	DWORD dwSrcPos = 0;
	*lpDest = 0;
	while (lpSrc[dwSrcPos] != 0) {
		if (lpSrc[dwSrcPos++] == '%') {
			char env[256];		//variable temporaire pour le nom de la variable d'environnement
			byte tmp = 0;
			while (lpSrc[dwSrcPos] != '%') {
				if (lpSrc[dwSrcPos] == 0) {
					lpDest[dwDestPos] = 0;
					return;
				}
				env[tmp++] = lpSrc[dwSrcPos];
				dwSrcPos++;
			}
			env[tmp] = 0;
			char env_value[1024];
			dwDestPos += GetEnvironmentVariableA(env, env_value, sizeof env_value);
			strcat(lpDest, env_value);
			dwSrcPos++;
		}
		else {
			lpDest[dwDestPos++] = lpSrc[dwSrcPos++ - 1];
		}
		
	}
	lpDest[dwDestPos] = lpSrc[dwSrcPos];
	lpDest[dwDestPos] = 0;
}
bool Equal(LPCSTR lpStr1, LPCSTR lpStr2) {
	DWORD index = 0;
	while (lpStr1[index] != 0 && lpStr2[index] != 0) {
		if (lpStr1[index] != lpStr2[index]) {
			return false;
		}
		index++;
	}
	return lpStr1[index] == lpStr2[index] ? true : false;
}
void RawStrCpy(LPCSTR lpSrc, LPSTR lpDest) {
	DWORD i = 0;
	while (lpSrc[i] != 0) {
		lpDest[i] = lpSrc[i];
		i++;
	}
}
void InvertCharsUpperLower(LPSTR str) {
	DWORD i = 0;
	while (str[i] != 0) {
		if (str[i] <= 90 && str[i] >= 65) {
			str[i] += 32;
			continue;
		}
		if (str[i] <= 122 && str[i] >= 97) {
			str[i] -= 32;

		}
		i++;
	}
}
void ToLower(LPSTR str) {
	DWORD i = 0;
	while (str[i] != 0) {
		if (str[i] >= 65 && str[i] <= 90) {
			str[i] += 32;
		}
		i++;
	}
}
void ToUpper(LPSTR str) {
	DWORD i = 0;
	while (str[i] != 0) {
		if (str[i] >= 97 && str[i] <= 122) {
			str[i] -= 32;
		}
		i++;
	}
}

DWORD HowMany(LPCSTR str, const char ch) {
	DWORD i = 0, count = 0;
	while (str[i] != 0) {
		if (str[i] == ch) {
			count++;
		}
		i++;
	}
	return count;
}
LPSTR GetFileName(LPSTR fullName) {
	int index = FindLastChar(fullName, '\\');
	if (index == -1) {
		return fullName;
	}
	int len = strlen(fullName);
	char buffer[FILENAME_MAX];
	ZeroMemory(buffer, len);
	if (len - index < 260) {
		return fullName;
	}
	for (int i = index; i < len; i++) {
		buffer[i] = fullName[i];
	}
	return buffer;

}
void GetFileExt(LPCSTR lpStr, LPSTR lpDest) {
	DWORD lastCharIndex = 0;
	lastCharIndex = FindLastChar(lpStr, '.');
	if (lastCharIndex == 0) {
		strcpy(lpDest, lpStr);
		return;
	}
	if (lastCharIndex == -1) {
		return;
	}

	DWORD lenght = strlen(lpStr);
	if (lenght == 0) return;
	


	if (Contains(lpStr + lastCharIndex, ' ')) {
		*lpDest = 0;
	}
}
bool RemoveFileExt(LPSTR str) {

	USHORT index = FindLastChar(str, '.');

	if (index == -1) {
		return false;
	}
	USHORT len = strlen(str);

	short i = 0;
	for (i = index; i < len; i++) {
		str[i] = 0;
	}

	return true;
}

bool Contains(LPCSTR str, const char ch) {
	DWORD cnt = 0;
	while (str[cnt] != '\0') {
		if (str[cnt] == ch)
			return true;
		cnt++;
	}
	return false;
}
bool Contains(LPCWSTR str, const wchar_t ch) {
	DWORD cnt = 0;
	while (str[cnt] != L'\0') {
		if (str[cnt++] == ch)
			return true;
	}
	return false;
}
bool StrContainsASCII(LPCSTR str) {

	DWORD index = 0;
	while (str[index++] != '\0') {
		if (!isascii(str[index]))
			return false;
	}
	return true;
}

int FindLastChar(LPCSTR str, const char ch) {

	int i = 0, last = -1;
	while (str[i] != '\0') {
		if (str[i] == ch)
			last = i;

		i++;
	}
	return last;
}
void RemoveEnd(LPSTR str, DWORD _where, DWORD size) {
	ZeroMemory(str + _where, size - _where);
}
DWORD FindFirstChar(LPCSTR str, const char ch) {
	DWORD index = 0;
	while (str[index++] != ch);
	return index;

}
bool CheckValidFileName(LPCSTR str)
{
	short lenght = strlen(str);
	for (short i = 0; i < lenght; i++) {
		if (str[i] == '"' || str[i] == '?' || str[i] == ':' || str[i] == '*' || str[i] == '<' || str[i] == '>' || str[i] == '|') {
			return false;
		}
	}
	return true;
}
void MakeFileNameValid(LPSTR str) {
	short lenght = strlen(str);
	LPSTR buffer = new char[FILENAME_MAX];
	ZeroMemory(buffer, sizeof buffer);
	short cnt = 0;
	for (short i = 0; i < lenght; i++) {
		if (!(str[i] == '"' || str[i] == '?' || str[i] == ':' || str[i] == '*' || str[i] == '<' || str[i] == '>' || str[i] == '|')) {
			buffer[cnt++] = str[i];
		}
	}
	RemoveEnd(buffer, cnt, sizeof buffer);
	strcpy(str, buffer);
}
void ShiftLeft(LPSTR lpSrc, DWORD count) {
	DWORD len = strlen(lpSrc);
	DWORD base = 0;
	for (DWORD i = 0; i < (len - count); i++) {
		lpSrc[base] = lpSrc[base + count];
		base++;
	}
	ZeroMemory(lpSrc + len - count, count);
}
void ShiftRight(LPSTR lpSrc, DWORD count) {
	DWORD i = strlen(lpSrc);
	while (lpSrc[i] != 0) {
		lpSrc[i + count] = lpSrc[i];
		i--;
	}
}
int FindCharIndex(LPSTR lpSrc, const char char_to_find, int index) {
	int cnt = 0;
	int i = 0;
	while (lpSrc[i] != 0) {
		if (lpSrc[i] == char_to_find) {
			cnt++;
		}
		if (cnt == index)
			return i;
		i++;
	}
	return -1;
}