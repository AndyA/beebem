#if HAVE_CONFIG_H
#	include <config.h>
#endif
#include "fatal.h"
#include <stdarg.h>
#include <stdlib.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
    void
fatal( const char *msg, ... ) {
  va_list ap;
  va_start( ap, msg );
  fprintf( stderr, "Fatal error: " );
  vfprintf( stderr, msg, ap );
  fprintf( stderr, "\n" );
  va_end( ap );
  exit( 1 );
}

void
fatal_fread( void *ptr, size_t size, size_t nmemb, FILE * stream ) {
  size_t got = fread( ptr, size, nmemb, stream );
  if ( got != nmemb ) {
    fatal( "Short read (expected %" PRIuPTR ", got %" PRIuPTR ")", nmemb,
           got );
  }
}

void
fatal_fgets( char *s, int size, FILE * stream ) {
  if ( NULL == fgets( s, size, stream ) ) {
    fatal( "Read failed" );
  }
}
