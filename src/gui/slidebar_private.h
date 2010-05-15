/* START OF slidebar_private.h --------------------------------------------------
 *
 *	A slide bar widget.
 * 
 *	---
 *	THIS GUI IS TOTALLY *BROKEN*! PLEASE DO NOT USE IT!
 *	---
 */


#ifndef _DW_SLIDEBAR_PRIVATE_H_
#define _DW_SLIDEBAR_PRIVATE_H_


#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include <gui/types.h>

#include <gui/widget.h>
#include <gui/window.h>

#include <SDL.h>


/* Get EG_SlideBar struct.
 */

#define EG_SLIDEBAR_GET_STRUCT_PTR(w, v, r)  \
        if (w == NULL ){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget is NULL.", dR); \
                return( r ); \
        } \
        if (EG_Widget_GetType( w ) != EG_Widget_Type_SlideBar){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is not of type EG_SlideBar.", dR); \
                return( r ); \
        } \
        if ( (v = (EG_SlideBar*) EG_Widget_GetPayload( w )) == NULL){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is NULL.", dR); \
                return(r); \
        } \
        EG_ASSERT_MALLOC( v );

#define EG_SLIDEBAR_GET_STRUCT_PTR_VOID(w, v)  \
        if (w == NULL ){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget is NULL.", dR); \
                return; \
        } \
        if (EG_Widget_GetType( w ) != EG_Widget_Type_SlideBar){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is not of type EG_SlideBar.", dR); \
                return; \
        } \
        if ( (v = (EG_SlideBar*) EG_Widget_GetPayload( w )) == NULL){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is NULL.", dR); \
                return; \
        } \
        EG_ASSERT_MALLOC( v );

typedef struct{
	SDL_Rect button1_area, button2_area;
	char button1_label[2], button2_label[2], slider_label[2];
	int orientation;
	long virtual_length;

	long position, previous_position;

	Uint32 last_called;

	long step;				// stepping amount.
	SDL_Rect background_area, slider_area;

#ifdef WITHOUT_REALTIME_SLIDER
	SDL_Rect slider_area_previous;
#endif

	EG_BOOL snap;
	EG_BOOL button1_depressed;
	EG_BOOL button2_depressed;
	EG_BOOL slider_depressed;
	int mouse_x, mouse_y;
}EG_SlideBar;
#define EG_AsSlideBar(n) ((EG_SlideBar*) n)

/* Private and friend functions:
 */

/* END OF slidebar_private.h ----------------------------------------------------
 */
#endif
