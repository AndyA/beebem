/* START OF progressbar_private.h ------------------------------------------------------
 *
 *	A simple progress bar widget.
 *
 *	---
 *	THIS GUI IS TOTALLY *BROKEN*! PLEASE DO NOT USE IT!
 *	---
 */


#ifndef _DW_PROGRESSBAR_PRIVATE_H_
#define _DW_PROGRESSBAR_PRIVATE_H_


#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include <gui/types.h>

#include <SDL.h>

#include <gui/window.h>
#include <gui/widget.h>


/* Get EG_ProgressBar struct.
 */

#define EG_PROGRESSBAR_GET_STRUCT_PTR(w, v, r)  \
        if (w == NULL ){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget is NULL.", dR); \
                return( r ); \
        } \
        if (EG_Widget_GetType( w ) != EG_Widget_Type_ProgressBar){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is not of type EG_ProgressBar.", dR); \
                return( r ); \
        } \
        if ( (v = (EG_ProgressBar*) EG_Widget_GetPayload( w )) == NULL){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is NULL.", dR); \
                return(EG_FALSE); \
        } \
        EG_ASSERT_MALLOC( v );


/* EG_Widget payload:
 */
typedef struct{
	SDL_Color bar_color;
	float position;
	Sint16 previous_w;

	Uint32  last_called;
	float inc;

	EG_BOOL unknown_position;
}EG_ProgressBar;
#define EG_AsProgressBar(n) ((EG_ProgressBar*) n)


/* END OF progressbar_private.h --------------------------------------------------------
 */
#endif                 
