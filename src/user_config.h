#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_

#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include "types.h"
#include "windows.h"

#ifdef __cplusplus
extern "C" {
#endif
	char* GetUserConfigPath(char *path, size_t len);

	char* GetLocation_roms_cfg(char *buffer, size_t length);
	char* GetLocation_econet_cfg(char *buffer, size_t length);
	char* GetLocation_phroms_cfg(char *buffer, size_t length);
	char* GetLocation_cmos_ram(char *buffer, size_t length);
	char* GetLocation_roms(char *buffer, size_t length);
	char* GetLocation_scsi(char *buffer, size_t length);
	char* GetLocation_tube(char *buffer, size_t length);
	char* GetLocation_teletext(char *buffer, size_t length);

#ifdef __cplusplus
}
#endif


#endif
