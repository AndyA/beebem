#ifndef _FAKE_REGISTRY_H_
#define _FAKE_REGISTRY_H_

#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include "types.h"
#include "windows.h"

#ifdef __cplusplus
extern "C"{
#endif
	void InitializeFakeRegistry(void);
	BOOL SaveFakeRegistry(void);

	BOOL GetFakeRegistryItem_String(const char *key_ptr, char **value_ptr);
	BOOL SetFakeRegistryItem_String(const char *key_ptr, const char *value_ptr);

	BOOL GetFakeRegistryItem_Double(const char *key_ptr, double *double_ptr);
	BOOL SetFakeRegistryItem_Double(const char *key_ptr, double double_val);

	BOOL GetFakeRegistryItem_Long(const char *key_ptr, long *long_val);
	BOOL SetFakeRegistryItem_Long(const char *key_ptr, long long_val);

	void RemoveFakeRegistryItem(const char *key_ptr);

	void DestroyFakeRegistry(void);

	void DumpFakeRegistry(void);
#ifdef __cplusplus
}
#endif

#endif
