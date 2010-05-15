/* START OF widget.c -----------------------------------------------------------
 *
 *	All actual widgets will share this file.
 *
 * 	---
 *	THIS GUI IS TOTALLY *BROKEN*! PLEASE DO NOT USE IT!
 *	---
 */

#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include <gui/log.h>

#include <gui/functions.h>

#include <gui/widget.h>
#include <gui/widget_private.h>

#include <SDL.h>

static EG_BOOL InitializeWidgetStruct(EG_Widget *widget_ptr,const char *name_ptr, EG_StringHash type);

/* Callbacks:
 *
 * These are generic callbacks.  You should really replace them for each
 * widget.
 *
 * Don't forget to free all resources allocated by the widget in the
 * Callback_Destroy function below.
 */

/* The destroy callback is called by any function that wants to totally
 * delete the widget.  It should do everything required to delete resources
 * allocated to the widget.  It does not need to unregister the widget as
 * a window child widget.  It's upto the caller to take care of the details
 */
static void Callback_Destroy(EG_Widget *widget_ptr)
{
	/* If the widget sets payload then it should replace this callback.
	 * We'll probably never call this actual callback to warn user.
	 */
	EG_Log(EG_LOG_WARNING, dL"EG_Widget Callback_Destroy function called"
	 , dR);


	/* If this was a real widget destroy callback, you'd free all the
	 * resources you allocated for the widget here!
	 */

	/* . . . . */


	/* Free heap alloc of this widgets EG_Widget structure
	 */
	EG_Widget_Free(widget_ptr);
}

/* Return true if you don't want to share this event with other widgets.
 */
static EG_BOOL Callback_SDL_Event(EG_Widget *widget_ptr, SDL_Event *event_ptr)
{
	EG_Widget *tmp;
	SDL_Event *tmp_e_ptr;

	tmp = widget_ptr; tmp_e_ptr = event_ptr;

	EG_Log(EG_LOG_WARNING, dL"EG_Widget Callback_Event function called", dR);

	return(EG_FALSE);
}


/* Repaint the whole widget, or part of it.
 */
static EG_BOOL Callback_Paint(EG_Widget *widget_ptr, SDL_Rect area)
{
	/* Loose compiler warnings
	 */
	EG_Widget *tmp;
	SDL_Rect *area_ptr;
	tmp = widget_ptr; area_ptr = &area;

	EG_Log(EG_LOG_WARNING, dL"EG_Widget Callback_Paint function called"
	 , dR);

	return(EG_FALSE);
}

static EG_BOOL Callback_Visible(EG_Widget *widget_ptr, EG_BOOL visible)
{
	/* Loose compiler warnings
	 */
	EG_Widget *tmp;
	EG_BOOL tmp_v;
	tmp = widget_ptr; tmp_v = visible;

	EG_Log(EG_LOG_WARNING, dL"EG_Widget Callback_Visible called", dR);

	return(EG_FALSE);
}

static EG_BOOL Callback_Enabled(EG_Widget *widget_ptr, EG_BOOL enabled)
{
	/* Loose compiler warnings
	 */
	EG_Widget *tmp;
	EG_BOOL tmp_e;
	tmp = widget_ptr; tmp_e = enabled;

	EG_Log(EG_LOG_WARNING, dL"EG_Widget Callback_Enabled called", dR);

	return(EG_FALSE);
}

static EG_BOOL Callback_GotFocus(EG_Widget *widget_ptr)
{
	EG_Widget *tmp;
	tmp = widget_ptr;

	EG_Log(EG_LOG_WARNING, dL"EG_Widget Callback_GotFocus called", dR);

	return(EG_FALSE);
}

static void Callback_LostFocus(EG_Widget *widget_ptr)
{
	EG_Widget *tmp;
	tmp = widget_ptr;

	EG_Log(EG_LOG_WARNING, dL"EG_Widget Callback_LostFocus called", dR);
}

static EG_BOOL Callback_Stopped(EG_Widget *widget_ptr, EG_BOOL stopped)
{
	EG_Widget *tmp;
	EG_BOOL tmp_e;
	tmp = widget_ptr;
	tmp_e = stopped;

	EG_Log(EG_LOG_WARNING, dL"EG_Widget Callback_Stopped called", dR);
	
	return(EG_FALSE);	
}

/* If a widget is a composition widget, then it must handle being added or
 * removed to a parent window correctly. Most widgets wont need to use this,
 * but some like the tab group for instance will benift from executing their
 * own callback when attached to a window.
 */
static void Callback_Attach(EG_Widget *widget_ptr
 , EG_StringHash attach_to_type, void *attach_to_ptr, EG_BOOL attached)
{
	EG_Widget *tmp_widget_ptr;
	EG_StringHash tmp_attach_to_type;
	void *tmp_attach_to_ptr;
	EG_BOOL tmp_attached;

	tmp_widget_ptr = widget_ptr;
	tmp_attach_to_type = attach_to_type;
	tmp_attach_to_ptr = attach_to_ptr;
	tmp_attached = attached;

	EG_Log(EG_LOG_WARNING, dL"EG_Widget Callback_Attached called", dR);
}

/* Initialize new EG_Window struct to default values.
 */
static EG_BOOL InitializeWidgetStruct(EG_Widget *widget_ptr
 , const char *name_ptr, EG_StringHash type)
{
        static unsigned long unique_ID = 0;
        Uint16 i;

        CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

        /* Give the new struct a unique ID.  (Useful for comparing two struct
         * pointers).
         *
         * If after increment the value wraps to zero, add one, as we want
         * zero as an ID to signify an error.  This shouldn't happen unless the
         * GUI is running or ages, or creates and destroys *LOTS* of widgets.
         */
        if (++unique_ID == 0){
                EG_Log(EG_LOG_INFO, dL"EG_Widget struct ID counter wrapped."
                 , dR);
                unique_ID++;
        }
        widget_ptr->ID = unique_ID;

        /* Set name, fail if cannot, EG_Widget_SetName will log failure.
         */
        if(EG_Widget_SetName(widget_ptr, name_ptr) == EG_FALSE) return(EG_FALSE);

        /* Set to a default background color of gray.
         */
        widget_ptr->background_color = ( (SDL_Color) {127, 127, 127, 0} );

	/* Set type.
	 */
	widget_ptr->type = type;

        /* Set pointers.
         */
	widget_ptr->payload_ptr = NULL;
	widget_ptr->window_ptr = NULL;
	widget_ptr->hold_value = -1;

	/* Set callbacks.
	 */
	(void) EG_Widget_SetCallback_Destroy(widget_ptr, Callback_Destroy);

	(void) EG_Widget_SetCallback_Paint(widget_ptr, Callback_Paint);
	(void) EG_Widget_SetCallback_SDL_Event(widget_ptr, Callback_SDL_Event);
	(void) EG_Widget_SetCallback_Visible(widget_ptr, Callback_Visible);
	(void) EG_Widget_SetCallback_Enabled(widget_ptr, Callback_Enabled);
	(void) EG_Widget_SetCallback_GotFocus(widget_ptr, Callback_GotFocus);
	(void) EG_Widget_SetCallback_LostFocus(widget_ptr, Callback_LostFocus);
	(void) EG_Widget_SetCallback_Stopped(widget_ptr, Callback_Stopped);
	(void) EG_Widget_SetCallback_Attach(widget_ptr, Callback_Attach);

	(void) EG_Widget_SetUserCallback_OnChange(widget_ptr, NULL, NULL);
	(void) EG_Widget_SetUserCallback_OnClick(widget_ptr, NULL, NULL);
	(void) EG_Widget_SetUserCallback_OnEvent(widget_ptr, NULL, NULL);
	(void) EG_Widget_SetUserCallback_OnLostFocus(widget_ptr, NULL, NULL);
	(void) EG_Widget_SetUserCallback_OnGotFocus(widget_ptr, NULL, NULL);

	widget_ptr->dimension = ( (SDL_Rect) {0,0,0,0} );
	
	widget_ptr->numeric_value = 0.0;

	/* Clear string (+1 for paranoids like me.. {see definition of struct} )
 	 */
	for(i=0; i<EG_WIDGET_MAX_STRING_VALUE_SIZE +1; i++)
		widget_ptr->string_value[i] = '\0';

	widget_ptr->visible = EG_TRUE;
	widget_ptr->enabled = EG_TRUE;
	widget_ptr->can_get_focus = EG_TRUE;
	widget_ptr->stopped = EG_FALSE;

        return(EG_TRUE);
}


/* Alloc/Free new Widget instance (EG_Widget struct, and EG_Widget->payload_ptr)
 */
EG_Widget* EG_Widget_Alloc(const char *name_ptr, EG_StringHash type)
{
        EG_Widget *widget_ptr = NULL;

        if ( (widget_ptr = EG_AsWidget(EG_Malloc(sizeof(EG_Widget))) ) == NULL){
                EG_Log(EG_LOG_WARNING, dL"Unable to malloc EG_Widget struct"
                 , dR);
                return(NULL);
        }        
                
        if(InitializeWidgetStruct(widget_ptr, name_ptr, type) == EG_FALSE){
                EG_Free(widget_ptr);
                return(NULL);
        }

        return(widget_ptr);
}

void EG_Widget_Free(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID_VOID(widget_ptr);
	EG_Free(widget_ptr);
}

/* 'Call' widget callbacks:
 */

void EG_Widget_CallDestroy(EG_Widget *widget_ptr)
{
	EG_Callback_Destroy func_ptr;

	CHECK_EG_WIDGET_IS_VALID_VOID(widget_ptr);

	if (widget_ptr->func_destroy_ptr == NULL){
		EG_Log(EG_LOG_ERROR, dL"EG_Widget.func_destroy_ptr"
		 " = NULL, widget not freed!", dR);
	}else{
		func_ptr 
		 = (EG_Callback_Destroy) widget_ptr->func_destroy_ptr;

		func_ptr(widget_ptr);
	}
}

EG_BOOL EG_Widget_CallPaint(EG_Widget *widget_ptr, SDL_Rect rect)
{
	EG_Callback_Paint func_ptr;

	//SDL_Delay(1000);

	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	/* Only paint widgets that are visible, if the widget is not visible
	 * (EG_Widget.visible = EG_FALSE or EG_Widget.stopped = EG_TRUE) then
	 * quit and return success (as the paint code did not actually fail).
	 */
	if (EG_Widget_IsVisible(widget_ptr) != EG_TRUE)
		return(EG_TRUE);

	if (widget_ptr->func_paint_ptr != NULL){
		func_ptr 
		 = (EG_Callback_Paint) widget_ptr->func_paint_ptr;

		return( func_ptr(widget_ptr, rect) );
	}
	return(EG_FALSE);
}

EG_BOOL EG_Widget_CallSDLEvent(EG_Widget *widget_ptr, SDL_Event *event_ptr)
{
	EG_Callback_SDL_Event func_ptr;

	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	/* Only send events to widgets that are enabled.
	 */
	if (EG_Widget_IsEnabled(widget_ptr) != EG_TRUE)
		return(EG_FALSE);

	if (widget_ptr->func_sdl_event_ptr != NULL){
		func_ptr
		 = (EG_Callback_SDL_Event) widget_ptr->func_sdl_event_ptr;

		return( func_ptr(widget_ptr, event_ptr) );
	}

	return(EG_FALSE);
}

EG_BOOL EG_Widget_CallVisible(EG_Widget *widget_ptr, EG_BOOL visible)
{
	EG_Callback_Visible func_ptr;

	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	if (widget_ptr->func_visible_ptr != NULL){
		func_ptr
		 = (EG_Callback_Visible) widget_ptr->func_visible_ptr;

		return( func_ptr(widget_ptr, visible) );
	}

	return(EG_FALSE);
}

EG_BOOL EG_Widget_CallEnabled(EG_Widget *widget_ptr, EG_BOOL enabled)
{
	EG_Callback_Enabled func_ptr;

	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	if (widget_ptr->func_enabled_ptr != NULL){
		func_ptr
		 = (EG_Callback_Enabled) widget_ptr->func_enabled_ptr;

		return( func_ptr(widget_ptr, enabled) );
	}
	return(EG_FALSE);
}

EG_BOOL EG_Widget_CallGotFocus(EG_Widget *widget_ptr)
{
	EG_Callback_GotFocus func_ptr;

	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	if (widget_ptr->func_got_focus_ptr != NULL){
		func_ptr
		 = (EG_Callback_GotFocus) widget_ptr->func_got_focus_ptr;

		return( func_ptr(widget_ptr) );
	}
	return(EG_FALSE);
}

void EG_Widget_CallLostFocus(EG_Widget *widget_ptr)
{
	EG_Callback_LostFocus func_ptr;

	CHECK_EG_WIDGET_IS_VALID_VOID(widget_ptr);

	if (widget_ptr->func_lost_focus_ptr != NULL){
		func_ptr
		 = (EG_Callback_LostFocus) widget_ptr->func_lost_focus_ptr;

		func_ptr(widget_ptr);
	}
}

EG_BOOL EG_Widget_CallStopped(EG_Widget *widget_ptr, EG_BOOL stopped)
{
	EG_Callback_Stopped func_ptr;

	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	if (widget_ptr->func_stopped_ptr != NULL){
		func_ptr
		 = (EG_Callback_Stopped) widget_ptr->func_stopped_ptr;

		return( func_ptr(widget_ptr, stopped) );
	}
	return(EG_FALSE);
}

void EG_Widget_CallAttach(EG_Widget *widget_ptr, EG_StringHash attach_to_type
 , void *attach_to_ptr, EG_BOOL attached)
{       
        EG_Callback_Attach func_ptr;
        
        CHECK_EG_WIDGET_IS_VALID_VOID(widget_ptr);
        
        if (widget_ptr->func_attach_ptr != NULL){
                func_ptr
                 = (EG_Callback_Attach) widget_ptr->func_attach_ptr;
                
                func_ptr(widget_ptr, attach_to_type, attach_to_ptr, attached);
        }
}

/* 'Call' user callbacks:
 */

void EG_Widget_CallUserOnChange(EG_Widget *widget_ptr)
{
	EG_Callback_User_OnChange func_ptr;

	CHECK_EG_WIDGET_IS_VALID_VOID(widget_ptr);

	if (widget_ptr->func_user_change_ptr != NULL){
		func_ptr
		 = (EG_Callback_User_OnChange) widget_ptr->func_user_change_ptr;
		
		func_ptr(widget_ptr, widget_ptr->user_change_value_ptr);
	}
}

void EG_Widget_CallUserOnClick(EG_Widget *widget_ptr)
{
	EG_Callback_User_OnClick func_ptr;

	CHECK_EG_WIDGET_IS_VALID_VOID(widget_ptr);

	if (widget_ptr->func_user_click_ptr != NULL){
		func_ptr
		 = (EG_Callback_User_OnClick) widget_ptr->func_user_click_ptr;

		func_ptr(widget_ptr, widget_ptr->user_click_value_ptr);
	}
}

void EG_Widget_CallUserOnEvent(EG_Widget *widget_ptr, SDL_Event *event_ptr)
{
	EG_Callback_User_OnEvent func_ptr;

	CHECK_EG_WIDGET_IS_VALID_VOID(widget_ptr);

	if (widget_ptr->func_user_event_ptr != NULL){
		func_ptr
		 = (EG_Callback_User_OnEvent) widget_ptr->func_user_event_ptr;

		func_ptr(widget_ptr, event_ptr, widget_ptr->user_event_value_ptr);
	}
}

void EG_Widget_CallUserOnLostFocus(EG_Widget *widget_ptr)
{
	EG_Callback_User_OnLostFocus func_ptr;

	CHECK_EG_WIDGET_IS_VALID_VOID(widget_ptr);

	if (widget_ptr->func_user_lostfocus_ptr != NULL){
		func_ptr
		 = (EG_Callback_User_OnLostFocus) widget_ptr->func_user_lostfocus_ptr;

		func_ptr(widget_ptr, widget_ptr->user_lostfocus_value_ptr);
	}
}

void EG_Widget_CallUserOnGotFocus(EG_Widget *widget_ptr)
{
	EG_Callback_User_OnGotFocus func_ptr;

	CHECK_EG_WIDGET_IS_VALID_VOID(widget_ptr);

	if (widget_ptr->func_user_gotfocus_ptr != NULL){
		func_ptr
		 = (EG_Callback_User_OnGotFocus) widget_ptr->func_user_gotfocus_ptr;

		func_ptr(widget_ptr, widget_ptr->user_gotfocus_value_ptr);
	}
}


/* 'Set' widget callbacks:
 */

EG_BOOL EG_Widget_SetCallback_Paint(EG_Widget *widget_ptr
 , EG_Callback_Paint paint_func_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	widget_ptr->func_paint_ptr = (void*) paint_func_ptr;
	return(EG_TRUE);
}

EG_BOOL EG_Widget_SetCallback_SDL_Event(EG_Widget *widget_ptr
 , EG_Callback_SDL_Event sdl_event_func_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	widget_ptr->func_sdl_event_ptr = (void*) sdl_event_func_ptr;
	return(EG_TRUE);
}

EG_BOOL EG_Widget_SetCallback_Visible(EG_Widget *widget_ptr
 , EG_Callback_Visible visible_func_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	widget_ptr->func_visible_ptr = (void*) visible_func_ptr;
	return(EG_TRUE);
}

EG_BOOL EG_Widget_SetCallback_Enabled(EG_Widget *widget_ptr
 , EG_Callback_Enabled enabled_func_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	widget_ptr->func_enabled_ptr = (void*) enabled_func_ptr;
	return(EG_TRUE);
}

EG_BOOL EG_Widget_SetCallback_GotFocus(EG_Widget *widget_ptr
 , EG_Callback_GotFocus got_focus_func_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	widget_ptr->func_got_focus_ptr = (void*) got_focus_func_ptr;
	return(EG_TRUE);
}

EG_BOOL EG_Widget_SetCallback_LostFocus(EG_Widget *widget_ptr
 , EG_Callback_LostFocus lost_focus_func_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	widget_ptr->func_lost_focus_ptr = (void*) lost_focus_func_ptr;
	return(EG_TRUE);
}

EG_BOOL EG_Widget_SetCallback_Destroy(EG_Widget *widget_ptr
 , EG_Callback_Destroy destroy_func_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	widget_ptr->func_destroy_ptr = (void*) destroy_func_ptr;
	return(EG_TRUE);
}

EG_BOOL EG_Widget_SetCallback_Stopped(EG_Widget *widget_ptr
 , EG_Callback_Stopped stopped_func_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	widget_ptr->func_stopped_ptr = (void*) stopped_func_ptr;
	return(EG_TRUE);
}

EG_BOOL EG_Widget_SetCallback_Attach(EG_Widget *widget_ptr
 , EG_Callback_Attach attach_func_ptr)
{
        CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

        widget_ptr->func_attach_ptr = (void*) attach_func_ptr;
        return(EG_TRUE);
}

/* 'Clear' widget callbacks:
 */

void EG_Widget_ClearCallback_Paint(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID_VOID(widget_ptr);
	widget_ptr->func_paint_ptr = NULL;
}

void EG_Widget_ClearCallback_SDL_Event(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID_VOID(widget_ptr);
	widget_ptr->func_sdl_event_ptr = NULL;
}

void EG_Widget_ClearCallback_Visible(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID_VOID(widget_ptr);
	widget_ptr->func_visible_ptr = NULL;
}

void EG_Widget_ClearCallback_Enabled(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID_VOID(widget_ptr);
	widget_ptr->func_enabled_ptr = NULL;
}

void EG_Widget_ClearCallback_GotFocus(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID_VOID(widget_ptr);
	widget_ptr->func_got_focus_ptr = NULL;
}

void EG_Widget_ClearCallback_LostFocus(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID_VOID(widget_ptr);
	widget_ptr->func_lost_focus_ptr = NULL;
}

void EG_Widget_ClearCallback_Stopped(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID_VOID(widget_ptr);
	widget_ptr->func_stopped_ptr = NULL;
}

void EG_Widget_ClearCallback_Attach(EG_Widget *widget_ptr)
{
        CHECK_EG_WIDGET_IS_VALID_VOID(widget_ptr);
        widget_ptr->func_attach_ptr = NULL;
}

void EG_Widget_ClearCallback_Destroy(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID_VOID(widget_ptr);
	widget_ptr->func_destroy_ptr = NULL;
}

/* EG_Widget struct wrappers:
 */

/* If you pass NULL as the name, then a unique name is assigned based on the
 * widgets ID number.  It's an eight byte hex number prefixed with zeros where
 * required, and has a leading '0x' to signify that it's a hex value.
 */
EG_BOOL EG_Widget_SetName(EG_Widget *widget_ptr, const char *name_ptr)
{
        CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	/* If NULL is passed then we use the ID value instead.
	 */
	if (name_ptr == NULL){
		sprintf( widget_ptr->name, "0x%08lX", EG_Widget_GetID(widget_ptr) );
		return(EG_TRUE);
	}

        /* Check name string is not too long.
         */
        if (strlen(name_ptr) > EG_WIDGET_MAX_NAME_SIZE){
                EG_Log(EG_LOG_ERROR, dL"name_ptr string too long or NULL.", dR);
                return(EG_FALSE);
        }

        /* Copy name string.
         */
        strcpy(widget_ptr->name, name_ptr);

        return(EG_TRUE);
}

EG_BOOL EG_Widget_SetType(EG_Widget *widget_ptr, EG_StringHash type)
{
        CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	widget_ptr->type = type;
        return(EG_TRUE);
}

EG_BOOL EG_Widget_SetHoldValue(EG_Widget *widget_ptr, long hold_value)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	widget_ptr->hold_value = hold_value;
	return(EG_TRUE);
}

long EG_Widget_GetHoldValue(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, -1);
	return(widget_ptr->hold_value);
}

void* EG_Widget_GetPayload(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, NULL);
	return(widget_ptr->payload_ptr);
}

EG_BOOL EG_Widget_GetStoppedToggle(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);
	return(widget_ptr->stopped);
}

EG_BOOL EG_Widget_SetStoppedToggle(EG_Widget *widget_ptr, EG_BOOL stopped)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);
	
	widget_ptr->stopped = stopped;
	return(EG_TRUE);
}

EG_BOOL EG_Widget_GetVisibleToggle(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);
	return(widget_ptr->visible);
}

EG_BOOL EG_Widget_SetVisibleToggle(EG_Widget *widget_ptr, EG_BOOL visible)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);
	
	widget_ptr->visible = visible;
	return(EG_TRUE);
}

EG_BOOL EG_Widget_GetEnabledToggle(EG_Widget *widget_ptr)
{
        CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);
        return(widget_ptr->enabled);
}

EG_BOOL EG_Widget_SetEnabledToggle(EG_Widget *widget_ptr, EG_BOOL enabled)
{
        CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

        widget_ptr->enabled = enabled;
        return(EG_TRUE);
}

EG_BOOL EG_Widget_SetPayload(EG_Widget *widget_ptr, void *payload_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	widget_ptr->payload_ptr = payload_ptr;
	return(EG_TRUE);
}

/* Be careful with this as it does not resize the SDL_Surface!
 */
EG_BOOL EG_Widget_SetDimension(EG_Widget *widget_ptr, SDL_Rect dimension)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	widget_ptr->dimension = dimension;
	return(EG_TRUE);
}

/* Some widget will not allow focus (Labels for instance), so these
 * private functions set the state.
 */
EG_BOOL EG_Widget_SetCanGetFocusToggle(EG_Widget *widget_ptr
, EG_BOOL can_get_focus)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	widget_ptr->can_get_focus = can_get_focus;
	return(EG_TRUE);
}

EG_BOOL EG_Widget_GetCanGetFocusToggle(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);
	return(widget_ptr->can_get_focus);
}

void* EG_Widget_GetWindow(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, NULL);

	return(widget_ptr->window_ptr);
}

EG_BOOL EG_Widget_SetWindow(EG_Widget *widget_ptr, void* window_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	widget_ptr->window_ptr = window_ptr;
	return(EG_TRUE);
}

/* Public functions:
 * actual widget type, rather than a generic 'paint' function.
 */

/* Set user callback pointers:
 */

EG_BOOL EG_Widget_SetUserCallback_OnChange(EG_Widget *widget_ptr
 , EG_Callback_User_OnChange user_callback_ptr, void *user_change_value_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	widget_ptr->func_user_change_ptr = (void*) user_callback_ptr;
	widget_ptr->user_change_value_ptr = user_change_value_ptr;
	return(EG_TRUE);
}

EG_BOOL EG_Widget_SetUserCallback_OnClick(EG_Widget *widget_ptr
 , EG_Callback_User_OnClick user_callback_ptr, void *user_click_value_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	widget_ptr->func_user_click_ptr = (void*) user_callback_ptr;
	widget_ptr->user_click_value_ptr = user_click_value_ptr;
	return(EG_TRUE);
}

EG_BOOL EG_Widget_SetUserCallback_OnEvent(EG_Widget *widget_ptr
 , EG_Callback_User_OnEvent user_callback_ptr, void *user_event_value_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	widget_ptr->func_user_event_ptr = (void*) user_callback_ptr;
	widget_ptr->user_event_value_ptr = user_event_value_ptr;
	return(EG_TRUE);
}

EG_BOOL EG_Widget_SetUserCallback_OnLostFocus(EG_Widget *widget_ptr
 , EG_Callback_User_OnLostFocus user_callback_ptr, void *user_event_value_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	widget_ptr->func_user_lostfocus_ptr = (void*) user_callback_ptr;
	widget_ptr->user_lostfocus_value_ptr = user_event_value_ptr;
	return(EG_TRUE);
}

EG_BOOL EG_Widget_SetUserCallback_OnGotFocus(EG_Widget *widget_ptr
 , EG_Callback_User_OnGotFocus user_callback_ptr, void *user_event_value_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	widget_ptr->func_user_gotfocus_ptr = (void*) user_callback_ptr;
	widget_ptr->user_gotfocus_value_ptr = user_event_value_ptr;
	return(EG_TRUE);
}

/* Clear user callback pointers:
 */

void EG_Widget_ClearUserCallback_OnChange(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID_VOID(widget_ptr);
	widget_ptr->func_user_change_ptr = NULL;
	widget_ptr->user_change_value_ptr = NULL;
}

void EG_Widget_ClearUserCallback_OnClick(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID_VOID(widget_ptr);
	widget_ptr->func_user_click_ptr = NULL;	
	widget_ptr->user_click_value_ptr = NULL;
}

void EG_Widget_ClearUserCallback_OnEvent(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID_VOID(widget_ptr);
	widget_ptr->func_user_event_ptr = NULL;
	widget_ptr->user_event_value_ptr = NULL;
}

void EG_Widget_ClearUserCallback_OnLostFocus(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID_VOID(widget_ptr);
	widget_ptr->func_user_lostfocus_ptr = NULL;
	widget_ptr->user_lostfocus_value_ptr = NULL;
}

void EG_Widget_ClearUserCallback_OnGotFocus(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID_VOID(widget_ptr);
	widget_ptr->func_user_gotfocus_ptr = NULL;
	widget_ptr->user_gotfocus_value_ptr = NULL;
}

/* EG_Widget struct access wrappers:
 */

EG_StringHash EG_Widget_GetType(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, (EG_StringHash) 0);
	return(widget_ptr->type);
}

unsigned long EG_Widget_GetID(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, 0);
	return(widget_ptr->ID);
}

const char* EG_Widget_GetName(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, NULL);
	return(widget_ptr->name);
}

double EG_Widget_GetValue_Numeric(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, 0.0);
	return(widget_ptr->numeric_value);
}

const char* EG_Widget_GetValue_String(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, NULL);
	return(widget_ptr->string_value);
}

SDL_Rect EG_Widget_GetDimension(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, ( (SDL_Rect) {0,0,0,0} ) );
	return(widget_ptr->dimension);
}

SDL_Color EG_Widget_GetBackgroundColor(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, ( (SDL_Color) {0,0,0,0} ) );
	return(widget_ptr->background_color);
}

EG_BOOL EG_Widget_IsFocusAllowed(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	if (widget_ptr->stopped == EG_TRUE)
		return(EG_FALSE);
	else
		return(widget_ptr->can_get_focus);
}

/* 'Set' widget state:
 */

EG_BOOL EG_Widget_SetBackgroundColor(EG_Widget *widget_ptr, SDL_Color color)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);
	widget_ptr->background_color = color;

	EG_Widget_RepaintLot(widget_ptr);
	return(EG_TRUE);
}

EG_BOOL EG_Widget_SetValue_Numeric(EG_Widget *widget_ptr, double value)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);
	widget_ptr->numeric_value = value;
	return(EG_TRUE);
}

EG_BOOL EG_Widget_SetValue_String(EG_Widget *widget_ptr, const char *string_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	/* Check length of string.
	 */
	if (string_ptr == NULL
	 || strlen(string_ptr) > EG_WIDGET_MAX_STRING_VALUE_SIZE){
		EG_Log(EG_LOG_ERROR, dL"New widget string value is too long", dR);
		return(EG_FALSE);
	}

	strcpy(widget_ptr->string_value, string_ptr);
	return(EG_TRUE);
}

EG_BOOL EG_Widget_RepaintLot(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	return( EG_Widget_CallPaint(widget_ptr
	 , EG_Widget_GetDimension(widget_ptr)) );
}

EG_BOOL EG_Widget_Repaint(EG_Widget *widget_ptr, SDL_Rect area)
{
	return( EG_Widget_CallPaint(widget_ptr, area) );
}

EG_BOOL EG_Widget_IsVisible(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	if (widget_ptr->stopped == EG_TRUE)
		return(EG_FALSE);
	else
		return(widget_ptr->visible);
}

EG_BOOL EG_Widget_IsEnabled(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	if (widget_ptr->stopped == EG_TRUE)
		return(EG_FALSE);
	else
		return(widget_ptr->enabled);
}
