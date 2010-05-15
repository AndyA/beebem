/* START OF functions.h --------------------------------------------------------
 *
 *	Memory function wrappers mostly.
 *
 *	---
 *	Written by David Eggleston (2006) <deggleston@users.sourceforge.net>
 *	for the 'BeebEm' Acorn BBC Model B, Integra-B, Model B Plus and
 *	Master 128 emulator.
 *
 *	This file is part of 'Economy GUI' and may be copied only under the
 *	terms of either the GNU General Public License (GPL) or Dr. David
 *	Alan Gilbert's BeebEm license.
 *
 *	For more details please visit:
 *
 *	http://www.gnu.org/copyleft/gpl.html
 *	---
 */

#ifndef _EG_FUNCTIONS_H_
#define _EG_FUNCTIONS_H_

#if HAVE_CONFIG_H
# include <config.h>
#endif

/* START OF functions.h --------------------------------------------------------
 */



#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <gui/types.h>
#include <gui/line.h>
#include <gui/log.h>



/* The following three definitions exist because I don't know what pointers
 * on 64 bit machines look like.
 *
 * EG_PTR_AS_INT is used whenever I want an integer that contains a
 * pointer address.  I never reference with this!  I use it for testing that
 * writes to malloc'd blocks are within bounds and for printing addresses, both
 * parts of the optional additonal debugging code enabled by EG_DEBUG.
 *
 * EG_CAST_PTR_TO_INT(n) Should cast a pointer into an integer, it should be
 * the same datatype as EG_PTR_AS_INT above.
 *
 * EG_PTRFMT is used for formatting the integer version of the pointer
 * within printf type functions.
 *
 * These defs. are mainly used for the malloc and free wrappers.  The casting is
 * needed for the pointer assertion code (see EG_Malloc definition)
 *
 * (If this is lame, please tell me how to do it for all machines properly).
 */
#define EG_PTR_AS_INT unsigned long
#define EG_CAST_PTR_TO_INT(n) ((unsigned long) n)
#define EG_PTRFMT "0x%08X"



/* Pubic functions:
 */
#ifdef __cplusplus
extern "C"{
#endif
	void* EG_Malloc(size_t);
	void EG_Free(void*);

	EG_BOOL EG_Malloc_CanFindAllocationInList(void*);
	unsigned long EG_MakeStringHash(char *p);
#ifdef __cplusplus
}
#endif

/* A suspicious value EG_Malloc and EG_Free should use when initialising
 * or freeing blocks of memory while in EG_DEBUG mode.
 */
#define EG_MALLOCFILLER 'X'
#define EG_FREEFILLER   'Z'



/* A simple pointer assertion.  Only functional in debug mode, but doesn't
 * need to be wrapped in EG_DEBUG ifdefs itself at a user level.
 */
#ifdef EG_DEBUG
#	ifdef EG_ASSERTIONSAREFATAL
#		define EG_ASSERT_MALLOC(n) \
		{ \
			EG_BOOL b; \
			b = EG_Malloc_CanFindAllocationInList(n); \
			\
			if (!b){ \
			EG_Log(EG_LOG_DEBUG5 \
			 , dL" Assertion FAILED for "EG_PTRFMT"" ,dR \
		 	, EG_CAST_PTR_TO_INT(n)); \
			\
			exit(EG_EXITCODE_MALLOCASSERTION); \
			} \
		}
#	else
#		define EG_ASSERT_MALLOC(n) \
		{ \
			EG_BOOL b; \
			b = EG_Malloc_CanFindAllocationInList(n); \
			\
			if (!b){ \
			EG_Log(EG_LOG_DEBUG5 \
			 , dL"Assertion FAILED for "EG_PTRFMT"",dR \
			 ,(b?"PASSED":"FAILED"), EG_CAST_PTR_TO_INT(n)); \
			} \
		}
#	endif
#else
#	define EG_ASSERT_MALLOC(n) {}
#endif

/* A more verbose version of above.  'message' is printed to the log as well.
 */

#ifdef EG_DEBUG
#	ifdef EG_ASSERTIONSAREFATAL
#		define EG_VASSERT_MALLOC(n, s) \
		{ \
			EG_BOOL b; \
			b = EG_Malloc_CanFindAllocationInList(n); \
			\
			if (!b){ \
			EG_Log(EG_LOG_DEBUG5 \
			 , dL" Assertion FAILED for "EG_PTRFMT": %s" ,dR \
		 	, EG_CAST_PTR_TO_INT(n), message); \
			\
			exit(EG_EXITCODE_MALLOCASSERTION); \
			} \
		}
#	else
#		define EG_VASSERT_MALLOC(n, s) \
		{ \
			EG_BOOL b; \
			b = EG_Malloc_CanFindAllocationInList(n); \
			\
			if (!b){ \
			EG_Log(EG_LOG_DEBUG5 \
			 , dL"Assertion FAILED for "EG_PTRFMT": %s",dR \
			 ,(b?"PASSED":"FAILED"), EG_CAST_PTR_TO_INT(n) \
			 , message); \
			} \
		}
#	endif
#else
#	define EG_ASSERT_MALLOC(n) {}
#endif


#ifndef EG_DEBUG
#	define EG_Malloc malloc
#	define EG_Free free
#endif

/* END OF functions.h ----------------------------------------------------------
 */

#endif
