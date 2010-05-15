#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

#include <gui/log.h>

// Verbosity levels 0 to 4.
// (normal value would be around eLOG_WARN)
typedef enum
{	eLOG_FATAL,
	eLOG_INFO,
	eLOG_ERROR,
	eLOG_WARN,
	eLOG_DEBUG,
	eLOG_DISABLED,
	eLOG_TRACKER_E,
	eLOG_TRACKER_L
} eLOGTYPE;

#ifndef WITHOUT_LEVEL_INDICATOR
char *error_descriptions[9]={"[FATAL] ", "[INFO]  ", "[ERROR] ", "[WARN]  ", "[DEBUG] ", "      X ", "   ---> ", "   <--- "};
#endif

static FILE *hLogFile = NULL;

void VLog_Printf(eLOGTYPE, const char*, va_list);
void VLog_AlertDlg(const char*, va_list);

void pDIALOG(const char *psFormat, ...)
{
	va_list argptr;
	va_start(argptr, psFormat);
	VLog_AlertDlg(psFormat, argptr);
}

// Append to log:
#define CALL_VLOG_PRINTF(t)   va_list argptr; va_start(argptr, psFormat);   VLog_Printf(t, psFormat, argptr);

// (see includes/log.h for some convenient but limited macros you can also call)
void pFATAL(const char *psFormat, ...){ CALL_VLOG_PRINTF(eLOG_FATAL); }
void pERROR(const char *psFormat, ...){ CALL_VLOG_PRINTF(eLOG_ERROR); }
void pWARN (const char *psFormat, ...){ CALL_VLOG_PRINTF( eLOG_WARN); }
void pINFO (const char *psFormat, ...){ CALL_VLOG_PRINTF( eLOG_INFO); }

#ifndef WITH_DEBUG_OUTPUT
	void pDEBUG(const char *psFormat, ...){}
	void pDISABLED(const char *psFormat, ...){}
	void pTRACKER_E(const char *psFormat, ...){}
	void pTRACKER_L(const char *psFormat, ...){}
#else
	void pDEBUG(const char *psFormat, ...){ CALL_VLOG_PRINTF(eLOG_DEBUG); }
	void pDISABLED(const char *psFormat, ...){ CALL_VLOG_PRINTF( eLOG_DISABLED); }
	void pTRACKER_E(const char *psFormat, ...){ CALL_VLOG_PRINTF( eLOG_TRACKER_E); }
	void pTRACKER_L(const char *psFormat, ...){ CALL_VLOG_PRINTF( eLOG_TRACKER_L); }
#endif

void Log_Init(void)
{
		hLogFile = stderr;
}

void Log_UnInit(void)
{
	if (hLogFile && hLogFile != stdout && hLogFile != stderr)
	{
		fclose(hLogFile);
	}
	hLogFile = NULL;
}

void VLog_Printf(eLOGTYPE nType, const char *psFormat, va_list argptr)
{
#ifndef WITHOUT_LEVEL_INDICATOR
		fprintf(hLogFile, "%s", error_descriptions[ (int) nType]);
#endif
		vfprintf(hLogFile, psFormat, argptr);
		va_end(argptr);

		if (psFormat[strlen(psFormat)-1] != '\n')
			fputs("\n", hLogFile);
}

void VLog_AlertDlg(const char *psFormat, va_list argptr)
{
	// GUI not finished yet..

	va_end(argptr);
}




/* vfprintf wrapper, used internally instead of printf, fprintf.
 */
#define EG_OUTPUT_STREAM_HARDWARED stdout
 
void EG_vprintf(const char *format, va_list argptr)
{	
	vfprintf(EG_OUTPUT_STREAM_HARDWARED, format, argptr);
	va_end(argptr);

	if (format[strlen(format)-1] != '\n')
		fputs("\n", EG_OUTPUT_STREAM_HARDWARED);
}

/* Used throughout EG for generating output.
 */
void EG_Log(EG_LOGTYPE type, const char *format, ...)
{
	va_list argptr;

	va_start(argptr, format);

	/* EG_LOGTYPE is a placeholder for a more advanced
	 * logging system.  Currently it does nothing.
	 */
	type = EG_LOG_ERROR;

	EG_vprintf(format, argptr);
}

