#ifndef UEFSTATE_HEADER
#define UEFSTATE_HEADER

#if HAVE_CONFIG_H
#	include <config.h>
#endif

void fput32(unsigned int word32,FILE *fileptr);
void fput16(unsigned int word16,FILE *fileptr);
unsigned int fget32(FILE *fileptr);
unsigned int fget16(FILE *fileptr);
void SaveUEFState(char *StateName);
void LoadUEFState(char *StateName);
#endif
