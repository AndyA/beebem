#ifndef _FATAL_H
#define _FATAL_H

#if HAVE_CONFIG_H
#	include <config.h>
#endif

#include <stddef.h>
#include <stdio.h>

void fatal( const char *msg, ... );

void fatal_fread( void *ptr, size_t size, size_t nmemb, FILE * stream );
void fatal_fgets( char *s, int size, FILE * stream );

#endif
