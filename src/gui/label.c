/* START OF label.c ------------------------------------------------------------
 *
 *	Label widget.
 *
 *	---
 *	THIS GUI IS TOTALLY *BROKEN*! PLEASE DO NOT USE IT!
 *	---
 */


#if HAVE_CONFIG_H
#       include <config.h>
#endif


#include <gui/log.h>

#include <gui/sdl.h>

#include <gui/functions.h>

#include <gui/label.h>
#include <gui/label_private.h>

#include <gui/window.h>
#include <gui/window_private.h>

#include <gui/widget_shared.h>

#include <gui/widget.h>
#include <gui/widget_private.h>

#include <SDL.h>


/* Callbacks:
 */

static EG_BOOL Callback_Paint(EG_Widget *widget_ptr, SDL_Rect area)
{
        EG_Label *label_ptr;
        EG_Window *window_ptr;
        
        SDL_Rect loc;

        SDL_Color color;
        SDL_Surface *surface_ptr;


        if ( EG_Shared_GetRenderingDetails(widget_ptr, area,
         (void*) &label_ptr, &window_ptr, &surface_ptr, &color
	 , &loc) != EG_TRUE )
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
	EG_Draw_String(surface_ptr, &color, EG_FALSE, &loc, label_ptr->alignment
	 , label_ptr->caption);

	/* Paint succeeded so return true.
	 */
	return(EG_TRUE);
}


/* Private functions:
 */

static void InitializeWidget(EG_Widget *widget_ptr, SDL_Color color
 , SDL_Rect dimension, int alignment, const char *caption_ptr)
{
	/* Initialize callbacks:
	 */
	(void) EG_Widget_SetCallback_Destroy(widget_ptr
	 , EG_Callback_Generic_Destroy);

	(void) EG_Widget_SetCallback_Paint(widget_ptr, Callback_Paint);

	(void) EG_Widget_SetCallback_SDL_Event(widget_ptr
	 , EG_Callback_Generic_SDL_Event);

	(void) EG_Widget_SetCallback_Visible(widget_ptr
	 , EG_Callback_Generic_Visible);

	(void) EG_Widget_SetCallback_Stopped(widget_ptr
	 , EG_Callback_Generic_Stopped);

	(void) EG_Widget_SetCallback_Enabled(widget_ptr
	 , EG_Callback_Generic_Enabled_NoSupport);

	(void) EG_Widget_SetCallback_GotFocus(widget_ptr
	 , EG_Callback_Generic_GotFocus_NoSupport);

	(void) EG_Widget_SetCallback_LostFocus(widget_ptr
	 , EG_Callback_Generic_LostFocus_NoSupport);

	(void) EG_Widget_SetCallback_Attach(widget_ptr
	 , EG_Callback_Generic_Attach);

	/* Initialize state:
	 */
	(void) EG_Widget_SetCanGetFocusToggle(widget_ptr, EG_FALSE);

	(void) EG_Widget_SetDimension(widget_ptr, dimension);
	(void) EG_Widget_SetBackgroundColor(widget_ptr, color);

	(void) EG_Label_SetCaption(widget_ptr, caption_ptr);
	(void) EG_Label_SetAlignment(widget_ptr, alignment);
}


/* Public functions:
 */

EG_Widget* EG_Label_Create(const char *name_ptr, SDL_Color color, int alignment
 , const char *caption_ptr, SDL_Rect dimension)
{
	EG_Label *label_ptr;
	EG_Widget *widget_ptr;
	void *ptr;

	/* Create the new widget.
	 */
	SHARED__ALLOC_PAYLOAD_STRUCT( ptr, EG_Label
	 , "Unable to malloc EG_Label struct");
	label_ptr = (EG_Label*) ptr;

	SHARED__CREATE_NEW_EG_WIDGET(widget_ptr, name_ptr, EG_Widget_Type_Label
	 , label_ptr);

	SHARED__ATTACH_PAYLOAD_TO_WIDGET(widget_ptr, label_ptr);

	InitializeWidget(widget_ptr, color, dimension, alignment, caption_ptr);
	return(widget_ptr);
}

EG_BOOL EG_Label_SetCaption(EG_Widget *widget_ptr, const char *caption_ptr)
{
	int i;
	EG_Label *label_ptr;

	EG_LABEL_GET_STRUCT_PTR(widget_ptr, label_ptr, EG_FALSE);
	
	/* Clear caption (not really needed but nice).
	 */
        for(i=0; i<MAX_LABEL_CAPTIONSIZE+1; i++)
                label_ptr->caption[i]='\0';

        /* Set the new caption. If the supplied caption is too long, then
	 * truncate it.
	 *
	 * (And yes I know that I could use strncpy on it's own here, but I
	 * don't want to do that.  I think I'd find it too easy to forget that
	 * '\0' is not included at the end).
         */
        if (caption_ptr != NULL){
                if (strlen(caption_ptr) <= MAX_LABEL_CAPTIONSIZE)
                        strcpy(label_ptr->caption, caption_ptr);
                else
                        strncpy(label_ptr->caption, caption_ptr
			 , MAX_LABEL_CAPTIONSIZE);
	}

	(void) EG_Widget_RepaintLot(widget_ptr);

	return(EG_TRUE);
}

const char* EG_Label_GetCaption(EG_Widget *widget_ptr)
{
	EG_Label *label_ptr;

	EG_LABEL_GET_STRUCT_PTR(widget_ptr, label_ptr, NULL);
	return(label_ptr->caption);
}

int EG_Label_GetAlignment(EG_Widget *widget_ptr)
{
	EG_Label *label_ptr;

	EG_LABEL_GET_STRUCT_PTR(widget_ptr, label_ptr, EG_LABEL_ALIGN_CENTER);
	return(label_ptr->alignment);
}

EG_BOOL EG_Label_SetAlignment(EG_Widget *widget_ptr, int alignment)
{
        EG_Label *label_ptr;

	EG_LABEL_GET_STRUCT_PTR(widget_ptr, label_ptr, EG_FALSE);
	label_ptr->alignment = alignment;

	(void) EG_Widget_RepaintLot(widget_ptr);

	return(EG_TRUE);
}



