// Implementation of cRegistry class (I like lower case c's).
// This class provides easier way of accessing the registry from a c++ application.
// Written by David Overton (david@overton.org.uk -- NO SPAM).
// Use this code if you want to, i found it much easier to use the registry with it.
// Please don't remove this section from here, or the section in the header.
// 
// This code is not commented (as you can see) but it works, and it is pretty 
// self-evident anyway.
//
// Problems/Improvements: 
//
// * Not really a problem, but this code could be modified to return a more
//   meaningful value than true or false for each function.
// 
// * The string/binary functions need to be modified to allow more than MAX_BUFFER_SIZE
//   bytes to be read in at any one time (possibly pass buffer size as function param).
//
// * Doesn't provide network registry support.

#if HAVE_CONFIG_H
#	include <config.h>
#endif

#include <ctype.h>

#include "cregistry.h"

//+>
#include "windows.h"
#include "fake_registry.h"
//<-

// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/sysinfo/base/regcreatekey.asp
bool cRegistry::CreateKey(HKEY hKeyRoot, LPSTR lpSubKey)
{
//+>
	// We don't need this as our config file will contain only BeebEm
	// stuff, so assume success.

	// hKeyRoot = User
	// lpSubKey = Application ("Software\BeebEm")

	return true;
//<+
	

//--	HKEY hKeyResult;
//--	if(RegCreateKey(hKeyRoot, lpSubKey, &hKeyResult)==ERROR_SUCCESS)
//--		return true;
//--	return false;
}

bool cRegistry::DeleteKey(HKEY hKeyRoot, LPSTR lpSubKey)
{
//+>
	// The Windows version of Beebem does not call this function.

	// hKeyRoot = User
	// lpSubKey = Application ("Software\BeebEm")

	//printf("DeleteKey\n");
	return true;
//<+


//--	if(RegDeleteKey(hKeyRoot,lpSubKey)==ERROR_SUCCESS)
//--		return true;	
//--	return false;
}

bool cRegistry::DeleteValue(HKEY hKeyRoot, LPSTR lpSubKey, LPSTR lpValueName)
{
//+>
//## The Windows version of Beebem does not call this function.
//## hKeyRoot = User
//## lpSubKey = Application ("Software\BeebEm")
//## lpValueName = value (all we need to concern ourselves with)

	//printf("DeleteValue\n");
	return false;
//<+
}

bool cRegistry::GetBinaryValue(HKEY hKeyRoot, LPSTR lpSubKey, LPSTR lpValue, PVOID pData, int *pnSize)
{

//+>
	// hKeyRoot = User
	// lpSubKey = Application ("Software\BeebEm")
	// lpValue = value (all we need to concern ourselves with - string ptr).
	// pData = pointer to callers buffer.
	// pnSize = size in bytes.

	char *fake_registry_ptr;
	unsigned char *callers_buffer_ptr = (unsigned char*) pData;
	BOOL ret;
	unsigned int i, val;

	ret = GetFakeRegistryItem_String( (char*) lpValue, &fake_registry_ptr);

	if (ret == TRUE) {
		for (i=0; i< (unsigned int) (*pnSize); i++)
			callers_buffer_ptr[i] = 0;

		for (i=0; i< strlen(fake_registry_ptr)/2; i++){

			sscanf(fake_registry_ptr, "%02X", &val);

			*callers_buffer_ptr++ = (unsigned char) val;
		}
	}

	return(ret);
//<+


//--	HKEY hKeyResult;
//--	DWORD dwType	= REG_BINARY;
//--	DWORD dwSize	= *pnSize;
//--	LONG  lRes		= 0;
//--	if((RegOpenKeyEx(hKeyRoot, lpSubKey, 0, KEY_ALL_ACCESS, &hKeyResult))==ERROR_SUCCESS) {
//--		if((lRes=RegQueryValueEx(hKeyResult,lpValue,NULL,&dwType,(LPBYTE)pData,&dwSize))==ERROR_SUCCESS) {
//--			if(dwType==REG_BINARY) {
//--				*pnSize=dwSize;
//--				RegCloseKey(hKeyResult);
//--				return true;
//--			}
//--		}
//--		RegCloseKey(hKeyResult);
//--	}
//--	return false;
}

bool cRegistry::GetDWORDValue(HKEY hKeyRoot, LPSTR lpSubKey, LPSTR lpValue, DWORD &dwBuffer)
{
//+>
	// hKeyRoot = User
	// lpSubKey = Application ("Software\BeebEm")
	// lpValue = value (all we need to concern ourselves with - string ptr).
	// dwBuffer = pointer to DWORD for answer.

//	long val = 0;
//	bool ret = false;
//	DWORD *p = &dwBuffer;
//
//	ret = GetFakeRegistryItem_Long( (char*) lpValue, &val);
//	printf("---> [%s] %ld\n", (char*) lpValue, val);
//
//	*p = (DWORD) val;
//	return ret;

	return(GetFakeRegistryItem_Long( (char*) lpValue, (long*) &dwBuffer));
//<-


//--	HKEY hKeyResult;
//--	DWORD dwType = REG_DWORD;
//--	DWORD dwBufferSize = MAX_BUFF_LENGTH;
//--	LONG  lRes;
//--	BYTE* pBytes = new BYTE[MAX_BUFF_LENGTH];
//--	ZeroMemory((void*)pBytes, MAX_BUFF_LENGTH );
//--	if((RegOpenKeyEx(hKeyRoot, lpSubKey, 0, KEY_ALL_ACCESS, &hKeyResult))==ERROR_SUCCESS) {
//--		if((lRes=RegQueryValueEx(hKeyResult,lpValue, NULL, &dwType, pBytes, &dwBufferSize))==ERROR_SUCCESS) {
//--			if(dwType==REG_DWORD) {
//--				DWORD* pDW = reinterpret_cast<DWORD*>(pBytes);
//--				dwBuffer = (*pDW);
//--				RegCloseKey(hKeyResult);
//--				delete[] pBytes;
//--				return 1;
//--			}
//--		}
//--		RegCloseKey(hKeyResult);
//--	}
//--	delete[] pBytes;
//--	return false;
}

bool cRegistry::GetStringValue(HKEY hKeyRoot, LPSTR lpSubKey, LPSTR lpValue, LPSTR lpBuffer)
{
//+>
	// hKeyRoot = User
	// lpSubKey = Application ("Software\BeebEm")
	// lpValue = value (all we need to concern ourselves with - string ptr).
	// lpBuffer = pointer to where string should be copied to (does not seem to specify a length).

	char *fake_registry_string_ptr;
	bool ret;

	ret = GetFakeRegistryItem_String( (char*) lpValue, &fake_registry_string_ptr);

	if (ret == TRUE)
		strcpy(lpBuffer,fake_registry_string_ptr);

	return(ret);
//<-


//--	HKEY hKeyResult;
//--	DWORD dwType = REG_SZ;
//--	DWORD dwBufferSize = MAX_BUFF_LENGTH;
//--	LONG  lRes;
//--	BYTE* pBytes = new BYTE[MAX_BUFF_LENGTH];
//--	ZeroMemory((void*)pBytes, MAX_BUFF_LENGTH );
//--	if((RegOpenKeyEx(hKeyRoot, lpSubKey, 0, KEY_ALL_ACCESS, &hKeyResult))==ERROR_SUCCESS) {
//--		if((lRes=RegQueryValueEx(hKeyResult,lpValue, NULL, &dwType, pBytes, &dwBufferSize))==ERROR_SUCCESS) {
//--			if(dwType==REG_SZ) {
//--				strcpy(lpBuffer,(char*)pBytes);
//--				RegCloseKey(hKeyResult);
//--				delete[] pBytes;
//--				return 1;
//--			}
//--		}
//--		RegCloseKey(hKeyResult);
//--	}
//--	delete[] pBytes;
//--	return false;
}

bool cRegistry::SetBinaryValue(HKEY hKeyRoot, LPSTR lpSubKey, LPSTR lpValue, PVOID pData, int* pnSize)
{
//+>
	// hKeyRoot = User
	// lpSubKey = Application ("Software\BeebEm")
	// lpValue = value (all we need to concern ourselves with - string ptr).
	// pData = pointer to data.
	// pnSize = size of data.

	char buffer[1024*4];
	char *buffer_ptr = buffer;
	unsigned char *callers_buffer_ptr = (unsigned char*) pData;
	int i, val;

#ifdef WITH_DEBUG_OUTPUT
	for(i=0; i< 1024*4; i++){
		buffer[i]='X';
	}
#endif

	for(i=0; i< *pnSize; i++){
		val = (int) *callers_buffer_ptr++;
		sprintf(buffer_ptr, "%02X", val); buffer_ptr+=2;
	}
	*buffer_ptr=0;

	return(SetFakeRegistryItem_String((char*) lpValue, buffer) );
//<-
	

//--	HKEY hKeyResult;
//--	DWORD dwType	= REG_BINARY;
//--	DWORD dwSize	= *pnSize;
//--	LONG  lRes		= 0;
//--	if((RegOpenKeyEx(hKeyRoot, lpSubKey, 0, KEY_ALL_ACCESS, &hKeyResult))==ERROR_SUCCESS) {
//--		if((lRes=RegSetValueEx(hKeyResult,lpValue,0,REG_BINARY,reinterpret_cast<BYTE*>(pData),dwSize))==ERROR_SUCCESS) {
//--			RegCloseKey(hKeyResult);
//--			*pnSize = dwSize;
//--			return true;
//--		}
//--		RegCloseKey(hKeyResult);
//--	}
//--	return false;
}

bool cRegistry::SetDWORDValue(HKEY hKeyRoot, LPSTR lpSubKey, LPSTR lpValue, DWORD dwValue)
{
//+>
	// hKeyRoot = User
	// lpSubKey = Application ("Software\BeebEm")
	// lpValue = value (all we need to concern ourselves with - string ptr).
	// dwValue = word value.

	return(SetFakeRegistryItem_Long((char*) lpValue, (long) dwValue));
//<-


//--	HKEY hKeyResult;
//--	DWORD dwType = REG_DWORD;
//--	DWORD dwBufferSize = MAX_BUFF_LENGTH;
//--	LONG  lRes;
//--	if((RegOpenKeyEx(hKeyRoot, lpSubKey, 0, KEY_ALL_ACCESS, &hKeyResult))==ERROR_SUCCESS) {
//--		if((lRes=RegSetValueEx(hKeyResult,lpValue,0,REG_DWORD,reinterpret_cast<BYTE*>(&dwValue),sizeof(DWORD)))==ERROR_SUCCESS) {
//--			RegCloseKey(hKeyResult);
//--			return true;
//--		}
//--		RegCloseKey(hKeyResult);
//--	}
//--	return false;
}

bool cRegistry::SetStringValue(HKEY hKeyRoot, LPSTR lpSubKey, LPSTR lpValue, LPSTR lpData)
{
//+>
	// hKeyRoot = User
	// lpSubKey = Application ("Software\BeebEm")
	// lpValue = value (all we need to concern ourselves with - string ptr).
	// lpData = pointer to the string.

	return(SetFakeRegistryItem_String((char*) lpValue, (char*) lpData) );
//<-


//--	HKEY hKeyResult;
//--	DWORD dwLength = strlen(lpData) * sizeof(char);
//--	DWORD dwType = REG_SZ;
//--	DWORD dwBufferSize = MAX_BUFF_LENGTH;
//--	LONG  lRes;
//--	if((RegOpenKeyEx(hKeyRoot, lpSubKey, 0, KEY_ALL_ACCESS, &hKeyResult))==ERROR_SUCCESS) {
//--		if((lRes=RegSetValueEx(hKeyResult,lpValue,0,REG_SZ,reinterpret_cast<BYTE*>(lpData),dwLength))==ERROR_SUCCESS) {
//--			RegCloseKey(hKeyResult);
//--			return true;
//--		}
//--	}
//--	return false;
}
