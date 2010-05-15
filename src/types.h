/*
 */


#ifndef _EG_TYPES_H_
#define _EG_TYPES_H_

#if HAVE_CONFIG_H
# include <config.h>
#endif

/* START OF types.h ------------------------------------------------------------
 */


/* exit codes:
 */
#define	EG_EXITCODE_MALLOCASSERTION 10



/* GUI Error codes:
 */
typedef enum{
  EG_SUCCESS,				// Generic Success
  EG_FAILURE,				// Generic Fail

  EG_ERROR_ISNULL,			// Pointer passed is NULL
  EG_ERROR_HASPAYLOAD,		// Widget for del. has Graphic payload
  EG_ERROR_ADDNODEFAILED,	// Add Node to List failed
	
  EG_ERROR_SURFACE_ALREADY_EXISTS,
  EG_ERROR_CREATE_SURFACE_FAILED
}EG_Error;



typedef int EG_BOOL;

#ifndef EG_FALSE
#define EG_FALSE 0
#define EG_TRUE (1)
#endif

typedef unsigned long EG_StringHash;
#define EG_AsStringHash(n) ((EG_StringHash) n)

/* END OF types.h --------------------------------------------------------------
 */

#endif
