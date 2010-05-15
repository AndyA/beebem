/* START OF togglebutton.c ----------------------------------------------------------
 *
 *	A simple togglebutton widget.
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

#include <gui/togglebutton.h>
#include <gui/togglebutton_private.h>

#include <gui/window.h>
#include <gui/window_private.h>

#include <gui/widget.h>
#include <gui/widget_private.h>

#include <gui/widget_shared.h>

#include <SDL.h>



/* Callbacks:
 */

static EG_BOOL Callback_Paint(EG_Widget *widget_ptr, SDL_Rect area)
{
	EG_ToggleButton *togglebutton_ptr;
	EG_Window *window_ptr;
	SDL_Rect loc;
	SDL_Color color;
	SDL_Surface *surface_ptr;

	EG_BOOL bold;


	if ( EG_Shared_GetRenderingDetails(widget_ptr, area,
	 (void*) &togglebutton_ptr, &window_ptr, &surface_ptr, &color, &loc) != EG_TRUE )
		return(EG_TRUE);


#ifdef EG_DEBUG
	printf("SOMEONE CALLED PAINT FOR '%s' [repaint area (window)="
	 "{%d, %d, %d, %d}:widget area (SDL_Surface)={%d, %d, %d, %d}]\n"
	, EG_Widget_GetName(widget_ptr), area.x, area.y, area.w, area.h
	, loc.x, loc.y, loc.w, loc.h);
#endif


	/* Paint the widget:
	 */
	EG_Draw_Disable_Update();

	/* Dull down the button a bit when disabled.
	 */
	if (EG_Widget_IsEnabled(widget_ptr) != EG_TRUE){
		color.r = (int) ( color.r * 0.9); 
		color.g = (int) ( color.g * 0.9); 
		color.b = (int) ( color.b * 0.9);
	}

//	EG_Draw_Box(surface_ptr, &loc, &color);

        if (togglebutton_ptr->is_selected == EG_TRUE){
                EG_Draw_Toggle(surface_ptr,&loc, &color
		 , EG_Draw_Border_BoxLow);
        }else{
                EG_Draw_Toggle(surface_ptr,&loc,&color
		 , EG_Draw_Border_BoxHigh);
        }

	loc.x +=2;
	loc.w -=4;

	/* Caption
	 */
	if ( EG_Window_ThisWidgetHasFocus(widget_ptr) == EG_TRUE)
		bold = EG_TRUE;
	else
		bold = EG_FALSE;

	EG_Draw_String(surface_ptr, &color, bold, &loc, 0, togglebutton_ptr->caption);

//	if ( EG_Window_ThisWidgetHasFocus(widget_ptr) ){
//		loc.x++;
//		EG_Draw_String(surface_ptr, &color, EG_FALSE, &loc, -1, tickbox_ptr->caption);
//		loc.x--;
//	}

	EG_Draw_Enable_Update();
	loc.x -=2; loc.w +=4;
	EG_Draw_UpdateSurface(surface_ptr, loc);

	/* Paint succeeded so return true.
	 */
	return(EG_TRUE);
}

/* SDL Events.
 */

// [TODO] This little lot is not nice, does it really need that many if's?

static EG_BOOL Callback_SDL_Event(EG_Widget *widget_ptr, SDL_Event *event_ptr)
{
	EG_ToggleButton *togglebutton_ptr;
	EG_Window *window_ptr;
	EG_BOOL mouse_over_widget;

	EG_BOOL return_value = EG_FALSE;

        /* Populates variables needed to process the event.  If the event
         * shouldn't have been passed to us, then logs the fact (for bug
         * reporting) and returns false.  Quit the callback if this returns
         * false.
         */
	if (EG_Shared_GetEventDetails(widget_ptr, event_ptr
	 , (void*) &togglebutton_ptr, &window_ptr, &mouse_over_widget) != EG_TRUE)
		return(return_value);

	/* Process SDL Event:
	 */


	/* Is a mouse button event?
	 */
	if (event_ptr->type == SDL_MOUSEBUTTONDOWN || event_ptr->type
	 == SDL_MOUSEBUTTONUP){

		/* If depressed = false and left mouse button is pressed and
		 * mouse pointer is within area of widget.
		 */
		if (togglebutton_ptr->depressed ==EG_FALSE && event_ptr->button.state
		 == SDL_PRESSED && event_ptr->button.button == SDL_BUTTON_LEFT
		 && mouse_over_widget == EG_TRUE){

			/* Set depressed = true, repaint widget.
			 */
			togglebutton_ptr->depressed=EG_TRUE;
			(void) EG_Widget_RepaintLot(widget_ptr);
		}

		/* If depressed = true and left mouse button is released.
		 */
		if (togglebutton_ptr->depressed == EG_TRUE && event_ptr->button.state
		 == SDL_RELEASED && event_ptr->button.button ==SDL_BUTTON_LEFT){
	
			/* If released with mouse pointer within area of widget
			 * and -------- the minimum click-time has been met. 
			 */
			if (mouse_over_widget == EG_TRUE &&
			 EG_Widget_GetVisibleToggle(widget_ptr) == EG_TRUE
                         && EG_Widget_GetStoppedToggle(widget_ptr) == EG_FALSE){
				
				/* If this toggle button has a group widget value,
				 * then it's part of a group (a radio button),
				 * so process it in that mode instead.
				 */

				if (togglebutton_ptr->parent_group_widget != NULL){
					/* Process as group member.
					 */

					/* If this toggle button is not already
					 * selected, select it.
					 */
					if (EG_ToggleButton_IsSelected(widget_ptr) != EG_TRUE)
						if(EG_RadioGroup_Select(widget_ptr) != EG_TRUE)
							EG_Log(EG_LOG_ERROR, dL"Failed to select button", dR);
				}else{
					/* Process as normal tickbox.
					 */
	
					/* Toggle value.
				 	 */
					if (togglebutton_ptr->is_selected == EG_TRUE){
						togglebutton_ptr->is_selected = EG_FALSE;
//						togglebutton_ptr->caption[0]= togglebutton_ptr->unset_value;
					}else{
						togglebutton_ptr->is_selected = EG_TRUE;
//						togglebutton_ptr->caption[0] = togglebutton_ptr->set_value;
					}
					if (EG_Window_SetFocusToThisWidget(widget_ptr) != EG_TRUE)
						EG_Log(EG_LOG_WARNING, dL"Could net"
						 " move focus to pressed EG_TickBox."
						 , dR);
				}
					
                                	/* Call users 'OnClick' event.
                                 	 */
					EG_Widget_CallUserOnClick(widget_ptr);
			}
	
			/* Regardless of mouse pointers location, set
			 * depressed = false; repaint widget.
			 */
			togglebutton_ptr->depressed=EG_FALSE;
			(void) EG_Widget_RepaintLot(widget_ptr);
		}
	}

	/* If widget has focus and 'Enter' key is pressed.
	 */

	if (event_ptr->type == SDL_KEYDOWN
	 && event_ptr->key.keysym.sym == SDLK_RETURN){

		/* If this widget currently has focus on the window.
		 */
		if (EG_Window_ThisWidgetHasFocus(widget_ptr) == EG_TRUE){
			togglebutton_ptr->depressed=EG_TRUE;
			(void) EG_Widget_RepaintLot(widget_ptr);
		}
	}

	if (event_ptr->type == SDL_KEYUP
	 && event_ptr->key.keysym.sym == SDLK_RETURN){

		/* If this widget currently has focus on the window.
		 */
		if (EG_Window_ThisWidgetHasFocus(widget_ptr) == EG_TRUE){

				if (togglebutton_ptr->parent_group_widget != NULL){
					/* Process as group member (radio button).
					 */

					if(EG_RadioGroup_Select(widget_ptr) != EG_TRUE)
						EG_Log(EG_LOG_ERROR, dL"Failed to select radio button", dR);

				}else{
					/* Process as normal tickbox.
					 */

					/* Toggle value.
				 	 */
					if (togglebutton_ptr->is_selected == EG_TRUE){
						togglebutton_ptr->is_selected = EG_FALSE;
//						togglebutton_ptr->caption[0] = togglebutton_ptr->unset_value;
					}else{
						togglebutton_ptr->is_selected = EG_TRUE;
//						togglebutton_ptr->caption[0] = togglebutton_ptr->set_value;
					}
	
					if (EG_Window_SetFocusToThisWidget(widget_ptr)
					 != EG_TRUE)
					 	EG_Log(EG_LOG_WARNING, dL"Could net"
						 " move focus to pressed EG_ToggleButton."
						 , dR);
				}

			return_value = EG_TRUE;

			EG_Widget_CallUserOnClick(widget_ptr);

			togglebutton_ptr->depressed=EG_FALSE;
			(void) EG_Widget_RepaintLot(widget_ptr);
		}
	}

        /* If mouse over widget, and users event callback is set, call users
         * callback.             
         */                             
        if (mouse_over_widget == EG_TRUE)
                EG_Widget_CallUserOnEvent(widget_ptr, event_ptr);

	return(return_value);
}


/* Private functions:
 */
static void InitializePayload(EG_ToggleButton *togglebutton_ptr)
{
	togglebutton_ptr->depressed = EG_FALSE;
}

static void InitializeWidget(EG_Widget *widget_ptr, SDL_Color color, SDL_Rect dimension, const char *caption_ptr)
{
	/* Initialize callbacks:
	 */
	(void) EG_Widget_SetCallback_Destroy(widget_ptr
	 , EG_Callback_Generic_Destroy);

	(void) EG_Widget_SetCallback_Paint(widget_ptr, Callback_Paint);

	(void) EG_Widget_SetCallback_SDL_Event(widget_ptr, Callback_SDL_Event);

	(void) EG_Widget_SetCallback_Visible(widget_ptr
	 , EG_Callback_Generic_Visible);

	(void) EG_Widget_SetCallback_Stopped(widget_ptr
	 , EG_Callback_Generic_Stopped);

	(void) EG_Widget_SetCallback_Enabled(widget_ptr
	 , EG_Callback_Generic_Enabled);

	(void) EG_Widget_SetCallback_GotFocus(widget_ptr
	 , EG_Callback_Generic_GotFocus);

	(void) EG_Widget_SetCallback_LostFocus(widget_ptr
	 , EG_Callback_Generic_LostFocus);

	(void) EG_Widget_SetCallback_Attach(widget_ptr
	 , EG_Callback_Generic_Attach);

	/* Initialize state:
	 */
	(void) EG_Widget_SetDimension(widget_ptr, dimension);
	(void) EG_Widget_SetBackgroundColor(widget_ptr, color);

	(void) EG_ToggleButton_SetGroupParent(widget_ptr, NULL);
	(void) EG_ToggleButton_SetUnselected(widget_ptr);
	(void) EG_ToggleButton_SetCaption(widget_ptr, caption_ptr);
}


/* Public functions:
 */

EG_Widget* EG_ToggleButton_Create(const char *name_ptr, SDL_Color color
 , const char *caption_ptr, SDL_Rect dimension)
{
	EG_ToggleButton *togglebutton_ptr;
	EG_Widget *widget_ptr;
	void *ptr;

	SHARED__ALLOC_PAYLOAD_STRUCT(ptr, EG_ToggleButton
	 , "Unable to malloc EG_ToggleButton struct");
	togglebutton_ptr = (EG_ToggleButton*) ptr;

	SHARED__CREATE_NEW_EG_WIDGET(widget_ptr, name_ptr, EG_Widget_Type_ToggleButton
	 , togglebutton_ptr);

	SHARED__ATTACH_PAYLOAD_TO_WIDGET(widget_ptr, togglebutton_ptr);

	InitializePayload(togglebutton_ptr);
        InitializeWidget(widget_ptr, color, dimension, caption_ptr);

	return(widget_ptr);
}

EG_BOOL EG_ToggleButton_SetCaption(EG_Widget *widget_ptr, const char *caption_ptr)
{
        int i;
        EG_ToggleButton *togglebutton_ptr;

        EG_TOGGLEBUTTON_GET_STRUCT_PTR(widget_ptr, togglebutton_ptr, EG_FALSE);

        for(i=1; i<MAX_TOGGLEBUTTON_CAPTIONSIZE+1; i++)
                togglebutton_ptr->caption[i]='\0';

        /* Set the new caption. If the supplied caption is too long, then
         * truncate it.
         */
        if (caption_ptr != NULL){
                if (strlen(caption_ptr) <= MAX_TOGGLEBUTTON_CAPTIONSIZE)
                        strcpy(togglebutton_ptr->caption, caption_ptr);
                else
                        strncpy(togglebutton_ptr->caption, caption_ptr
                         , MAX_TOGGLEBUTTON_CAPTIONSIZE);
        }

        return(EG_TRUE);
}

const char* EG_ToggleButton_GetCaption(EG_Widget *widget_ptr)
{
        EG_ToggleButton *togglebutton_ptr;

        EG_TOGGLEBUTTON_GET_STRUCT_PTR(widget_ptr, togglebutton_ptr, NULL);
        return(togglebutton_ptr->caption);
}

EG_BOOL EG_ToggleButton_SetGroupParent(EG_Widget *togglebutton_widget_ptr
 , EG_Widget *group_parent_widget_ptr)
{
	EG_ToggleButton *togglebutton_ptr;
	
	EG_TOGGLEBUTTON_GET_STRUCT_PTR(togglebutton_widget_ptr, togglebutton_ptr, EG_FALSE);
	togglebutton_ptr->parent_group_widget = group_parent_widget_ptr;
	return(EG_TRUE);
}

EG_Widget* EG_ToggleButton_GetGroupParent(EG_Widget *togglebutton_widget_ptr)
{
        EG_ToggleButton *togglebutton_ptr;

        EG_TOGGLEBUTTON_GET_STRUCT_PTR(togglebutton_widget_ptr, togglebutton_ptr, NULL);
	return(togglebutton_ptr->parent_group_widget);
}

/*
EG_BOOL EG_ToggleButton_SetIcons(EG_Widget *widget_ptr, char set_icon
 , char unset_icon)
{
        EG_TickBox *tickbox_ptr;

	EG_TICKBOX_GET_STRUCT_PTR(widget_ptr, tickbox_ptr, EG_FALSE);

	tickbox_ptr->set_value = set_icon;
	tickbox_ptr->unset_value = unset_icon;

	if (tickbox_ptr->is_ticked == EG_TRUE)
		tickbox_ptr->caption[0] = set_icon;
	else
		tickbox_ptr->caption[0] = unset_icon;

	return(EG_TRUE);
}
*/

EG_BOOL EG_ToggleButton_IsSelected(EG_Widget *widget_ptr)
{
	EG_ToggleButton *togglebutton_ptr;

	EG_TOGGLEBUTTON_GET_STRUCT_PTR(widget_ptr, togglebutton_ptr, EG_FALSE);
	return(togglebutton_ptr->is_selected);
}

EG_BOOL EG_ToggleButton_SetSelected(EG_Widget *widget_ptr)
{
	EG_ToggleButton *togglebutton_ptr;

	EG_TOGGLEBUTTON_GET_STRUCT_PTR(widget_ptr, togglebutton_ptr, EG_FALSE);

	togglebutton_ptr->is_selected = EG_TRUE;
//	togglebutton_ptr->caption[0] = togglebutton_ptr->set_value;

	return(EG_TRUE);
}

EG_BOOL EG_ToggleButton_SetUnselected(EG_Widget *widget_ptr)
{
        EG_ToggleButton *togglebutton_ptr;

	EG_TOGGLEBUTTON_GET_STRUCT_PTR(widget_ptr, togglebutton_ptr, EG_FALSE);

        togglebutton_ptr->is_selected = EG_FALSE;
//	tickbox_ptr->caption[0] = tickbox_ptr->unset_value;

	return(EG_TRUE);
}
