/* START OF widget_shared.h ----------------------------------------------------
 *
 *	Header for widget_shared.c, shared functions and definitions to make
 *	implementing widgets in EG easier.
 *
 *	---
 *	THIS GUI IS TOTALLY *BROKEN*! PLEASE DO NOT USE IT!
 *	---
 */


#ifndef _DW_WIDGET_SHARED_H_
#define _DW_WIDGET_SHARED_H_


/* 	Note: Includes window_private.h and widget_private.h so for widget
 * 	implementation headers only.
 */

#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include <gui/types.h>
#include <gui/log.h>

#include <SDL.h>

#include <gui/functions.h>

#include <gui/widget.h>
#include <gui/widget_private.h>

#include <gui/window.h>
#include <gui/window_private.h>


/* Special stuff to make your code look more complicated than it needs to be.
 *
 * Not that EG isn't over complicated already.. I'm so lame..
 */

/* Create new widget payload struct (see box.c for example):
 * 
 * ALLOC_PAYLOAD_STRUCT(box_ptr, EG_Box, "Unable to malloc EG_Box struct");
 */
#define SHARED__ALLOC_PAYLOAD_STRUCT(v, s, e) \
	if ( ( v = EG_Malloc(sizeof( s )) ) == NULL ){ \
		EG_Log(EG_LOG_WARNING, dL"" e ,dR); \
		return(NULL); \
	}


/* Create new EG_Widget for our new widgets payload struct (see box.c for
 * example).
 *
 * CREATE_NEW_EG_WIDGET(widget_ptr, name_ptr, EG_Widget_Type_Box, box_ptr);
 */
#define SHARED__CREATE_NEW_EG_WIDGET(v1, n, t, v2) \
	if ( (v1 =EG_Widget_Alloc( n , t)) == NULL){ \
                EG_Log(EG_LOG_WARNING, dL"Unable to create EG_Box widget.", dR); \
                EG_Free( v2 ); \
                return(NULL); \
        }


/* Attach payload struct to an EG_Widget (see box.c for example).
 *
 * 
 */
#define SHARED__ATTACH_PAYLOAD_TO_WIDGET(v1, v2) \
        if(EG_Widget_SetPayload( v1, (void*) v2) == EG_FALSE){ \
                EG_Log(EG_LOG_WARNING,dL"Unable to attach payload to" \
		 " EG_Widget.", dR); \
                EG_Free( v2 ); \
                EG_Widget_Free( v1 ); \
                return(NULL); \
        }



/* Functions:
 */
#ifdef __cplusplus
extern "C"{
#endif
	void		EG_Callback_Generic_Destroy(EG_Widget *widget_ptr);
	EG_BOOL		EG_Callback_Generic_SDL_Event(EG_Widget *widget_ptr, SDL_Event *event_ptr);
	EG_BOOL 	EG_Callback_Generic_Visible(EG_Widget *widget_ptr, EG_BOOL is_visible);
	EG_BOOL 	EG_Callback_Generic_Stopped(EG_Widget *widget_ptr, EG_BOOL is_stopped);

	EG_BOOL 	EG_Callback_Generic_Enabled_NoSupport(EG_Widget *widget_ptr, EG_BOOL is_enabled);
	EG_BOOL 	EG_Callback_Generic_Enabled(EG_Widget *widget_ptr, EG_BOOL is_enabled);

	EG_BOOL 	EG_Callback_Generic_GotFocus_NoSupport(EG_Widget *widget_ptr);
	EG_BOOL 	EG_Callback_Generic_GotFocus(EG_Widget *widget_ptr);

	void 		EG_Callback_Generic_LostFocus_NoSupport(EG_Widget *widget_ptr);
	void		EG_Callback_Generic_LostFocus(EG_Widget *widget_ptr);

	void		EG_Callback_Generic_Attach(EG_Widget *widget_ptr, EG_StringHash attach_to_type, void *attach_to_ptr, EG_BOOL attached);

	EG_BOOL 	EG_Shared_GetRenderingDetails(EG_Widget *widget_ptr, SDL_Rect area
			 , void         **payload_ptr_ptr       // EG_Box *box_ptr;
			 , EG_Window    **window_ptr_ptr        // EG_Window *window_ptr;
			 , SDL_Surface  **surface_ptr_ptr       // SDL_Surface *surface_ptr;
			 , SDL_Color    *color_ptr              // SDL_Color color;
			 , SDL_Rect     *loc_ptr);              // SDL_Rect loc;

	EG_BOOL 	EG_Shared_GetEventDetails(EG_Widget *widget_ptr, SDL_Event *event_ptr
			 , void         **payload_ptr_ptr       // EG_Box *button_ptr;
			 , EG_Window    **window_ptr_ptr        // EG_Window *window_ptr;
			 , EG_BOOL      *mouse_over_widget);     // EG_BOOL mouse_over_widget;

	EG_BOOL 	EG_Shared_UpdateVisibleState(EG_Widget *widget_ptr, EG_BOOL is_visible);
	EG_BOOL	 	EG_Shared_UpdateEnabledState(EG_Widget *widget_ptr, EG_BOOL is_enabled);
	EG_BOOL 	EG_Shared_IsMouseOverWidget(EG_Widget *widget_ptr, SDL_Event *event_ptr);

	EG_BOOL 	EG_Shared_IsMouseOverArea(EG_Widget *widget_ptr, SDL_Rect wa, SDL_Event *event_ptr);
#ifdef __cplusplus
}
#endif

/* END OF widget_shared.h ------------------------------------------------------
 */
#endif
