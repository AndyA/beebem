/* START OF tickbox.c ----------------------------------------------------------
 *
 *	A Radio Button (is actually the TickBox really).
 *
 *	---
 *	THIS GUI IS TOTALLY *BROKEN*! PLEASE DO NOT USE IT!
 *	---
 */

#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include <gui/log.h>

#include <gui/functions.h>

#include <gui/radiobutton.h>
#include <gui/radiobutton_private.h>

#include <gui/window.h>
#include <gui/window_private.h>

#include <gui/widget.h>
#include <gui/widget_private.h>

#include <SDL.h>

EG_Widget* EG_RadioButton_Create(const char *name_ptr, SDL_Color color
 , const char *caption_ptr, SDL_Rect dimension)
{
	EG_Widget *widget_ptr;

	if ( (widget_ptr = EG_TickBox_Create(name_ptr, color, caption_ptr
	 , dimension)) != NULL)
		EG_TickBox_SetIcons(widget_ptr, 13, 12);

	return(widget_ptr);
}



