// This class is an easier way of accessing the registry from a c++ application.
// By David Overton (david@overton.org.uk (NO SPAM)).
// See cRegistry.cpp for more comments and stuff.

#if !defined(_cRegistry_H_)
#define _cRegistry_H_

#if HAVE_CONFIG_H
#	include <config.h>
#endif

#include "windows.h"
#define MAX_BUFF_LENGTH 1024 // maximum length of data (in bytes) that you may read in.

class cRegistry {
public:
	bool CreateKey(HKEY hKeyRoot, LPCTSTR lpSubKey);
	bool DeleteKey(HKEY hKeyRoot, LPCTSTR lpSubKey);

	bool DeleteValue(HKEY hKeyRoot, LPCTSTR lpSubKey, LPCTSTR lpValueName);

	bool GetBinaryValue(HKEY hKeyRoot, LPCTSTR lpSubKey, LPCTSTR lpValue, PVOID pData, int* pnSize);
	bool GetDWORDValue(HKEY hKeyRoot, LPCTSTR lpSubKey, LPCTSTR lpValue, DWORD &dwBuffer);
	bool GetStringValue(HKEY hKeyRoot, LPCTSTR lpSubKey, LPCTSTR lpValue, LPSTR lpBuffer);

	bool SetBinaryValue(HKEY hKeyRoot, LPCTSTR lpSubKey, LPCTSTR lpValue, const PVOID pData, int* pnSize);
	bool SetDWORDValue(HKEY hKeyRoot, LPCTSTR lpSubKey, LPCTSTR lpValue, DWORD dwValue);
	bool SetStringValue(HKEY hKeyRoot, LPCTSTR lpSubKey, LPCTSTR lpValue, LPCTSTR lpData);
};


#endif
