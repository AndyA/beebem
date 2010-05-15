/* START OF button_private.h ---------------------------------------------------
 *
 *      A simple click-able button widget.
 *
 *	---
 *	THIS GUI IS TOTALLY *BROKEN*! PLEASE DO NOT USE IT!
 *	---
 */


#ifndef _DW_BUTTON_PRIVATE_H_
#define _DW_BUTTON_PRIVATE_H_


#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include <gui/types.h>

#include <SDL.h>

#include <gui/window.h>
#include <gui/widget.h>

#include <gui/button.h>


/* Get EG_Button struct.
 */

#define EG_BUTTON_GET_STRUCT_PTR(w, v, r)  \
        if (w == NULL ){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget is NULL.", dR); \
                return( r ); \
        } \
        if (EG_Widget_GetType( w ) != EG_Widget_Type_Button){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is not of type EG_Button.", dR); \
                return( r ); \
        } \
        if ( (v = (EG_Button*) EG_Widget_GetPayload( w )) == NULL){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is NULL.", dR); \
                return(EG_FALSE); \
        } \
        EG_ASSERT_MALLOC( v );


/* EG_Widget payload:
 */
typedef struct{
        int alignment;
	char caption[MAX_BUTTON_CAPTIONSIZE+1];

	EG_BOOL depressed;
}EG_Button;
#define EG_AsButton(n) ((EG_Button*) n)


/* Private and friend functions:
 */

/* END OF button_private.h ------------------------------------------------------
 */
#endif                                                                           
