/* START OF box.c --------------------------------------------------------------
 *
 * 	A simple box widget.
 *
 *	---
 *	THIS GUI IS TOTALLY *BROKEN*! PLEASE DO NOT USE IT!
 *	---
 */


#if HAVE_CONFIG_H
#       include <config.h>
#endif


#include <gui/types.h>
#include <gui/log.h>

#include <gui/functions.h>

#include <gui/box.h>
#include <gui/box_private.h>

#include <gui/widget_shared.h>

#include <gui/window.h>
#include <gui/window_private.h>

#include <gui/widget.h>
#include <gui/widget_private.h>

#include <SDL.h>


/* Callbacks:
 */

static EG_BOOL Callback_Paint(EG_Widget *widget_ptr, SDL_Rect area)
{
	EG_Box *box_ptr;
	EG_Window *window_ptr;

	SDL_Rect loc;

	SDL_Color color;
	SDL_Surface *surface_ptr;

	if ( EG_Shared_GetRenderingDetails(widget_ptr, area, 
	 (void*) &box_ptr, &window_ptr, &surface_ptr, &color, &loc) != EG_TRUE )
		return(EG_TRUE);

#ifdef EG_DEBUG
	printf("SOMEONE CALLED PAINT FOR '%s' [repaint area (window)="
	 "{%d, %d, %d, %d}:widget area (SDL_Surface)={%d, %d, %d, %d}]\n"
	 , EG_Widget_GetName(widget_ptr), area.x, area.y, area.w, area.h
	 , loc.x, loc.y, loc.w, loc.h);
#endif

	/* Render the widget.
	 */
	EG_Draw_Box(surface_ptr, &loc, &color);
	EG_Draw_Border(surface_ptr, &loc, &color, box_ptr->type);

	/* Paint succeeded so return true.
	 */
	return(EG_TRUE);
}


/* Private functions:
 */

static EG_BOOL InitializeWidget(EG_Widget *widget_ptr, SDL_Color color
 , SDL_Rect dimension, int type)
{
	/* Initialize callbacks:
	 */
	(void) EG_Widget_SetCallback_Destroy(widget_ptr,
	 EG_Callback_Generic_Destroy);

	(void) EG_Widget_SetCallback_Paint(widget_ptr, 
	 Callback_Paint);

	(void) EG_Widget_SetCallback_SDL_Event(widget_ptr,
	 EG_Callback_Generic_SDL_Event);

	(void) EG_Widget_SetCallback_Visible(widget_ptr,
	 EG_Callback_Generic_Visible);

	(void) EG_Widget_SetCallback_Stopped(widget_ptr,
	 EG_Callback_Generic_Stopped);

	(void) EG_Widget_SetCallback_Enabled(widget_ptr,
	 EG_Callback_Generic_Enabled_NoSupport);

	(void) EG_Widget_SetCallback_GotFocus(widget_ptr,
	 EG_Callback_Generic_GotFocus_NoSupport);

	(void) EG_Widget_SetCallback_LostFocus(widget_ptr,
	 EG_Callback_Generic_LostFocus_NoSupport);

	(void) EG_Widget_SetCallback_Attach(widget_ptr,
	 EG_Callback_Generic_Attach);

	/* Initialize state:
	 */
	(void) EG_Widget_SetCanGetFocusToggle(widget_ptr, EG_FALSE);

	(void) EG_Widget_SetDimension(widget_ptr, dimension);
	(void) EG_Widget_SetBackgroundColor(widget_ptr, color);

	(void) EG_Box_SetBorderType(widget_ptr, type);

	return(EG_TRUE);
}


/* Public functions:
 */

EG_Widget* EG_Box_Create(const char *name_ptr, int type, SDL_Color color
 , SDL_Rect dimension)
{
	EG_Box *box_ptr;
	EG_Widget *widget_ptr;
	void *ptr;

	/* Create the new widget.
	 */
	SHARED__ALLOC_PAYLOAD_STRUCT(ptr, EG_Box
	 , "Unable to malloc EG_Box struct");
	box_ptr = (EG_Box*) ptr;

	SHARED__CREATE_NEW_EG_WIDGET(widget_ptr, name_ptr, EG_Widget_Type_Box
	 , box_ptr);

	SHARED__ATTACH_PAYLOAD_TO_WIDGET(widget_ptr, box_ptr);

	InitializeWidget(widget_ptr, color, dimension, type);
	return(widget_ptr);
}

EG_BOOL EG_Box_SetBorderType(EG_Widget *widget_ptr, int type)
{
	EG_Box *box_ptr;

	EG_BOX_GET_STRUCT_PTR(widget_ptr, box_ptr, EG_FALSE);
	box_ptr->type = type;
	return(EG_TRUE);
}

int EG_Box_GetBorderType(EG_Widget *widget_ptr)
{
	EG_Box *box_ptr;
	EG_BOX_GET_STRUCT_PTR(widget_ptr, box_ptr, EG_FALSE);
	return(box_ptr->type);
}

EG_BOOL EG_Box_Resize(EG_Widget *widget_ptr, SDL_Rect area)
{
	if (EG_Widget_SetDimension(widget_ptr, area) == EG_TRUE){
		EG_Widget_RepaintLot(widget_ptr);
		return(EG_TRUE);
	}
	
	return(EG_FALSE);
}

/* END OF box.c ----------------------------------------------------------------
 */



