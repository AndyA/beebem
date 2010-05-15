/* START OF box_private.h ------------------------------------------------------
 *
 *	A simple box widget.
 *
 *	---
 *	THIS GUI IS TOTALLY *BROKEN*! PLEASE DO NOT USE IT!
 *	---
 */


#ifndef _DW_BOX_PRIVATE_H_
#define _DW_BOX_PRIVATE_H_


#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include <gui/types.h>

#include <SDL.h>

#include <gui/window.h>
#include <gui/widget.h>

#include <gui/box.h>


/* Get EG_Box struct.
 */

#define EG_BOX_GET_STRUCT_PTR(w, v, r)  \
        if (w == NULL ){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget is NULL.", dR); \
                return( r ); \
        } \
        if (EG_Widget_GetType( w ) != EG_Widget_Type_Box){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is not of type EG_Box.", dR); \
                return( r ); \
        } \
        if ( (v = (EG_Box*) EG_Widget_GetPayload( w )) == NULL){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is NULL.", dR); \
                return(EG_FALSE); \
        } \
        EG_ASSERT_MALLOC( v );


/* EG_Widget payload:
 */
typedef struct{
        int type;
}EG_Box;
#define EG_AsBox(n) ((EG_Box*) n)


/* END OF box_private.h --------------------------------------------------------
 */
#endif                 
