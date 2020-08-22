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

bool Equal(LPCSTR lpBuf1, LPCSTR lpBuf2, QWORD qwSize) {
	
	for (QWORD i = 0; i < qwSize; i++) {
		if (lpBuf1[i] != lpBuf2[i])
			return false;
	}
	return true;
}

void ToLower(LPSTR lpszSource) {
	QWORD i = 0;
	while (lpszSource[i] != 0) {
		if (lpszSource[i] >= 65 && lpszSource[i] <= 90) {
			lpszSource[i] += 32;
		}
		i++;
	}
}

void ToLower(LPSTR lpszSource, DWORD dwSize) {
	for (DWORD i = 0; i < dwSize; i++) {
		if (lpszSource[i] >= 65 && lpszSource[i] <= 90) {
			lpszSource[i] += 32;
		}
	}
}

void ToUpper(LPSTR lpszSource) {
	QWORD i = 0;
	while (lpszSource[i] != 0) {
		if (lpszSource[i] >= 97 && lpszSource[i] <= 122) {
			lpszSource[i] -= 32;
		}
		i++;
	}
}

void ToUpper(LPSTR lpszSource, DWORD dwSize) {
	for (DWORD i = 0; i < dwSize; i++) {
		if (lpszSource[i] >= 97 && lpszSource[i] <= 122) {
			lpszSource[i] -= 32;
		}
	}
}

DWORD GetChars(LPCSTR str, const char ch) {
	DWORD count = 0;
	LPSTR _str = (LPSTR)str;
	while (*_str != 0) {
		if (*_str == ch) {
			count++;
		}
		_str++;
	}
	return count;
}

LPSTR GetFileName(LPSTR fullName) {
	int index = FindLastChar(fullName, '\\');
	if (index == -1) {
		return fullName;
	}
	int len = strlen(fullName);
	char buffer[PATH_BUFFER_SIZE];
	ZeroMemory(buffer, len);
	if (len - index < 260) {
		return fullName;
	}
	for (int i = index; i < len; i++) {
		buffer[i] = fullName[i];
	}
	return buffer;

}

bool GetFileExt(LPCSTR lpStr, LPSTR lpDest) {
	DWORD dwLastDot = FindLastChar(lpStr, '.');
	if (dwLastDot == ERROR_CHAR_NOT_FOUND)
		return false;
	strcpy(lpDest, lpStr + dwLastDot);
	return true;
}

bool Contains(LPCSTR str, const char ch) {
	LPSTR _str = (LPSTR)str;
	while (*_str != 0) {
		if (*_str++ == ch)
			return true;
	}
	return false;
}

DWORD FindLastChar(LPCSTR str, const char ch) {

	DWORD i = 0, last = ERROR_CHAR_NOT_FOUND;
	while (str[i] != 0) {
		if (str[i] == ch)
			last = i;

		i++;
	}
	return last;
}

DWORD FindFirstChar(LPCSTR str, const char ch) {
	DWORD index = 0;
	while (str[index] != 0) {
		if (str[index] == ch) {
			return index;
		}
		index++;
	}
	return ERROR_CHAR_NOT_FOUND;

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

int FindCharIndex(LPSTR lpStr, const char ch, int index) {
	int cnt = 0;
	int i = 0;
	while (lpStr[i] != 0) {
		if (lpStr[i] == ch) {
			cnt++;
		}
		if (cnt == index)
			return i;
		i++;
	}
	return -1;
}

void RemoveEndSpaces(LPSTR lpsz) {
	
	DWORD dwLength = strlen(lpsz);
	if (*lpsz == 0)
		return;
	while (lpsz[dwLength] == ' ') {
		lpsz[dwLength--] = 0;
	}
}

void RemoveEndSpaces(LPSTR lpsz, DWORD dwLength) {

	if (*lpsz == 0)
		return;
	while (lpsz[dwLength] == ' ') {
		lpsz[dwLength--] = 0;
	}
}

void ReplaceAllChars(LPSTR lpsz, const char _old, const char _new) {
	while (*lpsz != 0) {
		if (*lpsz == _old)
			*lpsz = _new;
		lpsz++;
	}
}