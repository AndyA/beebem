/* Convenient macros:
 *
 * __L__		Prints the filename + line no.
 * __F__		same as __PRETTY_FUNCTION__
 * __S__		Spacer
 *
 * Adding something like:
 *
 * fprintf(stderr, "%s%s%sGot this far!\n", __L__, __F__, __S__);
 *
 * Will print something like:
 *
 * "beebwin.cc:313, void BeebWin::Initialise(): Got this far!"
 *
 * See ../log.c for additional convenient functions/macros for logging
 *
 * If this is a bit too evil for your taste, then
 * WITH_NO_LINE_SHENANIGANS will disable the macros..
 *
 * In which case the above example would output:
 *
 * "Got this far!"
 * 
 * instead.
 *
 *
 * David Eggleston 18/10/05
 */

#ifndef LINE_H
#define LINE_H

#if HAVE_CONFIG_H
# include <config.h>
#endif

/* This uses a little trick to allow __LINE__ to be
 * stringified.  It then adds __FILE__ nicely formatted
 * as a string literal.
 */

#ifndef WITHOUT_SHENANIGANS
#       ifndef __HERE__
#               define _STR1_(s)                #s
#               define _STR2_(s)                _STR1_(s)
#               define __STRLINE__              _STR2_(__LINE__)
#               define __L__			__FILE__": "__STRLINE__": "
#       endif
#else
#       define __L__				""
#endif


/* As __PRETTY_FUNCTION__ may be a variable, I cannot mix it
 * with the string literal generated above.
 *
 * So I'm going to rename it to something shorter.
 */

#ifndef WITHOUT_SHENANIGANS
#	define  __F__                           __PRETTY_FUNCTION__
#else
#       define  __F__                           ""
#endif


/* Nice spacer
 */

#ifndef WITHOUT_SHENANIGANS
#	define	__S__				": "
#else
#	define	__S__				""
#endif


#endif  /* LINE_H */
