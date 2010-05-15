#ifndef _FATAL_H
#define _FATAL_H

#if HAVE_CONFIG_H
#	include <config.h>
#endif

#include <stddef.h>
#include <stdio.h>

#define _LOC const char *file, int line

void fatal( const char *msg, ... );

void fatal__fread( _LOC, void *ptr, size_t size, size_t nmemb,
                   FILE * stream );
void fatal__fgets( _LOC, char *s, int size, FILE * stream );

#define fatal_fread(ptr, size, nmemb, stream) \
  fatal__fread(__FILE__, __LINE__, ptr, size, nmemb, stream)

#define fatal_fgets(s, size, stream) \
  fatal__fgets(__FILE__, __LINE__, s, size, stream)

#endif
