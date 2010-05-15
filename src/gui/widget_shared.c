/* START OF widget_shared.c ----------------------------------------------------
 *
 *	Useful helper functions for implementing widgets in EG.  window.c,
 *	widget.c do not use these functions, and never should.  Only actual
 *	widget implementations should.
 *
 *	All functions are public, there is no widget_shared_private.h.  Only
 *	include it for widget implementations.  Users should not have access to
 *	these functions.
 *
 *	---
 *	THIS GUI IS TOTALLY *BROKEN*! PLEASE DO NOT USE IT!
 *	---
 */






#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include <gui/widget_shared.h>

/* Needed for additional debugging code below.
 */
#ifdef EG_DEBUG
#	include <gui/sdl.h>
#endif

/* Generic callback implementations:
 *
 * All do the absolute minimum (but are still a good place to start for
 * implementing more complex callbacks).
 *
 * (THIS IS IN THE PROCESS OF MOVING INTO THE UNIFIED window.c/widget.c EG2)
 */


/* D E S T R O Y:
 */
void EG_Callback_Generic_Destroy(EG_Widget *widget_ptr)
{
        void *payload_ptr;

        if (widget_ptr==NULL){
                EG_Log(EG_LOG_ERROR, dL"Tried to Destroy Box with"
		 " widget_ptr=NULL", dR);
                return;
        }

	/* 
	 *
 	 * Freeing child widgets from a composite widget is a bit of a black
	 * art I'm afraid!
	 *
	 * But, so long as you use the window.c function
	 * 'EG_Window_RemoveWidget' to free up your child widgets you should
	 * be OK.
	 *
	 */

        /* Free payload.
         */
	if ( (payload_ptr=EG_Widget_GetPayload(widget_ptr)) != NULL)
		EG_Free(payload_ptr);

        /* Free widget.
         */
        EG_Widget_Free(widget_ptr);
}

/* S D L   E V E N T:
 *
 * Do not free widgets (even if you do call the window.c function 
 * 'EG_Window_RemoveWidget') within this callback.
 *
 * The window.c code that handles the conversion and passing of SDL_Event
 * structures to your widget here uses a for/next loop, so if you kill a
 * widget (even if you do it cleanly as per Destroy instructions above), it
 * still may be processed within that darn window.c loop!
 *
 * So please, never delete widgets here.
 *
 * Returning EG_FALSE tells window.c to pass this event on to other widgets
 * too.
 */
EG_BOOL EG_Callback_Generic_SDL_Event(EG_Widget *widget_ptr, SDL_Event *event_ptr)
{
	/* Loose GNU CC -Wall -W compiler warnings.
	 */

	EG_Widget *tmp_widget;
	SDL_Event *tmp_event;

	tmp_widget = widget_ptr;
	tmp_event = event_ptr;

	/* Call users event callback.
	 */
	if (EG_Shared_IsMouseOverWidget(widget_ptr, event_ptr) == EG_TRUE)
		EG_Widget_CallUserOnEvent(widget_ptr, event_ptr);
	
	return(EG_FALSE);
}

/* V I S I B L E:
 *
 * When visibility state changes, the callback will re-render the widget or screen
 * itself.
 */
EG_BOOL EG_Callback_Generic_Visible_NoSupport(EG_Widget *widget_ptr
 , EG_BOOL is_visible)
{
	/* Loose GNU CC -Wall -W compiler warnings.
	 */

	EG_Widget *tmp_widget_ptr;
	EG_BOOL tmp_is_visible;

	tmp_widget_ptr = widget_ptr;
	tmp_is_visible = is_visible;

	return(EG_FALSE);
}

EG_BOOL EG_Callback_Generic_Visible(EG_Widget *widget_ptr, EG_BOOL is_visible)
{

	return( EG_Shared_UpdateVisibleState(widget_ptr, is_visible) );
}

/* S T O P P E D:
 *
 * Note: Anything smart enough to 'stop' a widget, is smart enough to update the
 *       screen for you!  So we don't remove/render the widget when this state
 *       changes.
 *
 * Always succeeds if the widget pointer is OK.  Don't use this for composite
 * widgets (those that micro-manage other widgets, the tabber widgets and the
 * radio group widget for instance), for composite widgets you'll need to
 * rewrite this (the tabgorup.c implementation makes a good example).
 */
EG_BOOL EG_Callback_Generic_Stopped(EG_Widget *widget_ptr, EG_BOOL is_stopped)
{
        CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

        widget_ptr->stopped = is_stopped;
        return(EG_TRUE);
}

/* E N A B L E D:
 *
 * Note: Never repaint the widget when enabling and disabling a widget.  Leave
 *       that to the callback wrappers 'EG_Window_DisableWidget' and 
 *       'EG_Window_EnableWidget' to handle.
 *
 * NONFUNCTIONAL:	Always fails, never changes state in EG_Widget struct
 * FUNCTIONAL:		Will try to change state in EG_Widget struct.
 */
EG_BOOL EG_Callback_Generic_Enabled_NoSupport(EG_Widget *widget_ptr, EG_BOOL is_enabled)
{

	/* Loose GNU CC -Wall -W compiler warnings.
	 */

	EG_Widget *tmp_widget_ptr;
	EG_BOOL tmp;

	tmp_widget_ptr = widget_ptr;
	tmp = is_enabled;

        return(EG_FALSE);
}
EG_BOOL EG_Callback_Generic_Enabled(EG_Widget *widget_ptr, EG_BOOL is_enabled)
{
	return( EG_Shared_UpdateEnabledState(widget_ptr, is_enabled) );
}

/* G O T   F O C U S:
 * 
 * Note: The caller
 *       must repaint the widget, as if the window has not set this widget to
 * 	 the focused widget internally, then this widget will be painted
 *	 without focus.
 */
EG_BOOL EG_Callback_Generic_GotFocus_NoSupport(EG_Widget *widget_ptr)
{
	/* Loose GNU CC -Wall -W compiler warnings.
	 */

	EG_Widget *tmp_widget_ptr;

	tmp_widget_ptr = widget_ptr;

	return(EG_FALSE);
}
EG_BOOL EG_Callback_Generic_GotFocus(EG_Widget *widget_ptr)
{
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	/* If the widget is stopped, non-visible or disabled, then the caller
	 * shouldn't have called this callback.  So we'll handle it, but log
	 * a warning to that effect.
	 */
	if (EG_Widget_IsEnabled(widget_ptr) == EG_FALSE ||
	 EG_Widget_IsVisible(widget_ptr) == EG_FALSE){
		EG_Log( EG_LOG_WARNING, dL"Tried to set focus to widget '%s'"
		 "and it's either disabled, stopped or non-visible."
		 , dR, EG_Widget_GetName(widget_ptr) );
		
		return(EG_FALSE);
	}

	/* Call users event callback.
	 */
	EG_Widget_CallUserOnGotFocus(widget_ptr);

	/* If this widget already has focus, then do nothing and return
	 * success.
	 */
	if ( EG_Window_ThisWidgetHasFocus(widget_ptr) == EG_TRUE)
		return(EG_TRUE);

	return(EG_TRUE);
}

/* L O S T   F O C U S:
 *
 * Note: The caller must repaint the widget (see GotFocus above for reason).
 *
 * It should generally be considered safe to call this callback when the
 * widget in question does not have focus.  It should just do nothing.
 *
 * These two functions are exactly the same for now!
 */
void EG_Callback_Generic_LostFocus_NoSupport(EG_Widget *widget_ptr)
{
	/* Loose GNU CC -Wall -W compiler warnings.
	 */

	EG_Widget *tmp_ptr;
	tmp_ptr = widget_ptr;
}
void EG_Callback_Generic_LostFocus(EG_Widget *widget_ptr)
{
	/* There is not much we can do if this fails.  I'll log it for now,
	 * but this may become annoying!
	 */
//	if ( EG_Widget_RepaintLot(widget_ptr) != EG_TRUE)
//		EG_Log( EG_LOG_WARNING, dL"Failed to render widget '%s'\n"
//		 , dR, EG_Widget_GetName(widget_ptr) );

	EG_Widget *tmp_ptr;
	tmp_ptr = widget_ptr;

	/* Call users event callback.
	 */
	EG_Widget_CallUserOnGotFocus(widget_ptr);
}

/* A T T A C H:
 *
 * Only composite widgets (those that micro-manage other widgets) need handle
 * this.  It's called whenever the widget is attached to a window (see
 * EG_Window_AddWidget), and gives the widget the opportunity to add more
 * widgets.  For instance:- the tab group widget uses this to attach it's own
 * tab page widgets to the window (when the window calls this for the tab group
 * widget).  The tab page widgets then do the same for their own children. Thus
 * adding all widgets on the tabber to the window when you: 
 *
 * (void) EG_Window_AddWidget(my_window, my_tab_group);
 *
 * This is basically a fudge to add a tabber widget. If you find yourself using
 * this for all your widgets, junk EG and rewrite it! EG only works when the
 * root window can micro-manage all widgets displayed upon it.
 */
void EG_Callback_Generic_Attach(EG_Widget *widget_ptr
 , EG_StringHash attach_to_type, void *attach_to_ptr, EG_BOOL attached)
{

	/* Loose GNU CC -Wall -W compiler warnings.
	 */

	EG_Widget *tmp_widget_ptr;
	EG_StringHash tmp_attach_to_type;
	void *tmp_attach_to_ptr;
	EG_BOOL tmp_attached;

	tmp_widget_ptr = widget_ptr;
	tmp_attach_to_type = attach_to_type;
	tmp_attach_to_ptr = attach_to_ptr;
	tmp_attached = attached;
}


/* Functions to help with physically rendering widgets:
 */


/* Checks, assigns and returns all variables needed to start painting the
 * widget.  The return value determines whether you actually need to repaint
 * the widget or not.
 *
 * This feels so wrong..
 *
 * Return value:
 *
 * If EG_TRUE:  Widget needs to be rendered, and 'color', 'window_ptr_ptr',
 *              'color_ptr', 'surface_ptr_ptr' and 'loc_ptr' are populated with
 *              the relevant values.
 * 
 * If EG_FALSE: Widget does not need to be rendered at all. You may return from
 *              your paint callback and report success. 
 *              -
 *              All passed pointers are returned in an undetermined state. Do
 *              not use them
 *
 *
 *
 * Use like this (if you dare):
 *
 *      -       -       -       -       -
 *      
 *      EG_Box          *box_ptr;       // Widget payload (EG_Box for instance)
 *      EG_Window       *window_ptr;    // Window pointer
 *      SDL_Rect        loc;            // Location of widget on SDL_Surface
 *      SDL_Color       color;          // Background color.
 *      SDL_Surface     *surface_ptr;   // SDL_Surface to render to.
 *
 *      if (EG_Shared_GetRenderingDetails(widget_ptr, area
 *       , (void*) &box_ptr, &window_ptr, &color, &surface_ptr, &loc)
 *        == EG_FALSE)
 *              return(EG_TRUE);
 *
 *      // Now box_ptr, window_ptr, loc, color, and surface_ptr all contain
 *      // values I'd expect.
 *      -       -       -       -       -
 *
 */
EG_BOOL EG_Shared_GetRenderingDetails(EG_Widget *widget_ptr, SDL_Rect area
 , void         **payload_ptr_ptr       // EG_Box *box_ptr;
 , EG_Window    **window_ptr_ptr        // EG_Window *window_ptr;
 , SDL_Surface  **surface_ptr_ptr       // SDL_Surface *surface_ptr;
 , SDL_Color    *color_ptr              // SDL_Color color;
 , SDL_Rect     *loc_ptr)               // SDL_Rect loc;
{

	/* Make sure widget_ptr actually is a widget.
	 */
	if (widget_ptr == NULL){
		EG_Log( EG_LOG_WARNING, dL"Tried to render NULL widget.", dR);
		return(EG_FALSE);
	}
        /* We should not get as far as calling a paint callback if the widget
         * is not even visible (I.e.: visible = false, or stopped = true).
         *
         * So log a warning.
         */
        if (EG_Widget_IsVisible(widget_ptr) == EG_FALSE){
                EG_Log( EG_LOG_WARNING, dL"Tried to render '%s', even though"
                 " EG_Widget_IsVisible == EG_FALSE.", dR
                 , EG_Widget_GetName(widget_ptr) );
                return(EG_FALSE);
        }

        /* Does a part of the window, that the widget occupies need repainting?
         */
        *loc_ptr = EG_Widget_GetDimension(widget_ptr);

        /* Get pointers to EG_Window SDL_Surface and EG_Box:
         */
        *payload_ptr_ptr = EG_Widget_GetPayload(widget_ptr);

        /* If widget is not connected to a window yet, don't bother painting.
         */
        *window_ptr_ptr = (EG_Window*) EG_Widget_GetWindow(widget_ptr);
        if (*window_ptr_ptr == NULL)
                return(EG_FALSE);

        /* Only paint if the window is visible.
         */
        if (EG_Window_IsHidden( *window_ptr_ptr ) == EG_TRUE)
                return(EG_FALSE);

        /* Calculate absolute position of widget on SDL_Surface.
         */
        loc_ptr->x += EG_Window_GetXDisplacement( *window_ptr_ptr );
        loc_ptr->y += EG_Window_GetYDisplacement( *window_ptr_ptr );

        *color_ptr = EG_Widget_GetBackgroundColor(widget_ptr);
        *surface_ptr_ptr = EG_Window_GetSurface( *window_ptr_ptr );


//	//------------------------------------------------------------------------------------------------------------------
//
//	goto end_of_func;
//
	SDL_Rect a, b; //, p;
//	SDL_Colour col1, col2;
//	SDL_Surface *s, *ss;
//
//	s = *surface_ptr_ptr;
//
//	//printf("========================================================\n");
//
//	ss = SDL_ConvertSurface(s, s->format, SDL_SWSURFACE);
//	if (ss == NULL){
//		printf("Did not make surface.\n");
//	}
//
//
//	// ==========================================
//
//	p.x = 0; p.y=0; p.w = s->w; p.h = s->h;
////	col1 = ( (SDL_Color) {255,0,255,0} );
////	EG_Draw_Box(*surface_ptr_ptr, &p, &col1);
//
//
//	col1 = ( (SDL_Color) {255,0,0,0} );
//	col2 = ( (SDL_Color) {0,255,0,0} );
//
//	a = area;
//	b.x = loc_ptr->x; b.y = loc_ptr->y; b.w = loc_ptr->w; b.h = loc_ptr->h;
//
//	// convert a (area) to SDL_Surface.
//	a.x += EG_Window_GetXDisplacement( *window_ptr_ptr );
//	a.y += EG_Window_GetYDisplacement( *window_ptr_ptr );
//	
//	EG_Draw_Box(*surface_ptr_ptr, &a, &col1);
//
//	SDL_Delay(5);
//
//	EG_Draw_Box(*surface_ptr_ptr, &b, &col2);
//
//
#define overlap(a, b) ( ( ( ((a.y - (b.y+b.h) ) ^ ( (a.y+a.h) - b.y)) & ((a.x - (b.x+b.w) ) ^ ( (a.x+a.w) - b.x)) ) < 0) ? 1 : 0)
//
//	if ( overlap(a,b) ){
////		printf("OVERLAP\n");
//		EG_Draw_String(s, NULL, EG_TRUE, &p ,0, "Overlaps.");
//		
//	}else{
////		printf("DONT OVERLAP\n");
//		EG_Draw_String(s, NULL, EG_TRUE, &p ,0, "Does not overlap.");
//	}	
//
//	SDL_Delay(5);
//
//
//	// ====================================================================================================
//
////	if (
//SDL_BlitSurface(ss, NULL, *surface_ptr_ptr, NULL);
//// != 0)
////	{ printf("BLIT TO SCREEN FAILED\n");
////	}else{printf("BLIT TO SCREEN SUCCESS\n");}
//
//	SDL_UpdateRect(s, 0, 0, s->w, s->h);
//
//
////	printf("DELAY\n");
////	printf("===============================================================\n");
//
////	SDL_BlitSurface(ss, NULL, s, NULL);
//	SDL_FreeSurface(ss);
//
//
//
//	//=================================================================
//
//
//end_of_func:


        a = area;
        b.x = loc_ptr->x; b.y = loc_ptr->y; b.w = loc_ptr->w; b.h = loc_ptr->h;

        // convert a (area) to SDL_Surface.
        a.x += EG_Window_GetXDisplacement( *window_ptr_ptr );
        a.y += EG_Window_GetYDisplacement( *window_ptr_ptr );

	if (! overlap(a,b) )
		return(EG_FALSE);
	



















//	a.x = 0; a.y=0; a.w = s->w; a.h = s->h;
//	col1 = ( (SDL_Color) {0,0,126,0} );
//	EG_Draw_Box(*surface_ptr_ptr, &a, &col1);
//
//
//	col1 = ( (SDL_Color) {255,0,0,0} );
//	col2 = ( (SDL_Color) {0,255,0,0} );
//
//	a = area;
//	b.x = loc_ptr->x; b.y = loc_ptr->y; b.w = loc_ptr->w; b.h = loc_ptr->h;
//	
//	EG_Draw_Box(*surface_ptr_ptr, &a, &col1);
//	EG_Draw_Box(*surface_ptr_ptr, &b, &col2);
//
//
//#define overlap(a, b) ( ( ( ((a.y - (b.y+b.h) ) ^ ( (a.y+a.h) - b.y)) & ((a.x - (b.x+b.w) ) ^ ( (a.x+a.w) - b.x)) ) < 0) ? 1 : 0)
//
//	if ( overlap(a,b) )
//		printf("OVERLAP\n");
//	else
//		printf("DONT OVERLAP\n");
//
//	SDL_Delay(3000);

	
        return(EG_TRUE);
}





EG_BOOL EG_Shared_GetEventDetails(EG_Widget *widget_ptr, SDL_Event *event_ptr
 , void         **payload_ptr_ptr       // EG_Box *button_ptr;
 , EG_Window    **window_ptr_ptr        // EG_Window *window_ptr;
 , EG_BOOL      *mouse_over_widget)     // EG_BOOL mouse_over_widget;
{
	//SDL_Rect wa;
	//int x, y;

	/* Make sure SDL_Event is not just a NULL pointer (never should be so
	 * log Error).
	 */
	if (event_ptr == NULL){
		EG_Log( EG_LOG_WARNING, dL"Tried to process a NULL SDL Event"
		 "'%s'", dR, EG_Widget_GetName(widget_ptr) );
		return(EG_FALSE);
        }

        /* Make sure widget_ptr is actually a widget.
         */
        if (widget_ptr == NULL){
                EG_Log( EG_LOG_WARNING, dL"Tried to process SDL Event for a"
		 " NULL widget.", dR);
                return(EG_FALSE);
        }	

	/* Get pointers to EG_Window SDL_Surface and EG_Box:
	 */
	*payload_ptr_ptr = EG_Widget_GetPayload(widget_ptr);

	/* If widget is not connected to a window yet, don't bother processing
	 * events.  (But if it's NOT, where are the events coming from?  Is this
	 * code just a bit TOO paranoid?)
	 */
	*window_ptr_ptr = (EG_Window*) EG_Widget_GetWindow(widget_ptr);
	if (*window_ptr_ptr == NULL){
		EG_Log( EG_LOG_WARNING, dL"Tried to process an SDL event for"
		 " '%s', even though it has no window (!), riddle me this"
		 " batman!", dR, EG_Widget_GetName(widget_ptr) );
		return(EG_FALSE);
	}

	/* Is the mouse pointer over the widget in question?
	 */

//	// [TODO] Try converting SDL_Events so they are relative to the window coordinates not the SDL_Surface ones.
//	wa = EG_Widget_GetDimension(widget_ptr);
//	wa.x += EG_Window_GetXDisplacement(*window_ptr_ptr);
//	wa.y += EG_Window_GetYDisplacement(*window_ptr_ptr);
//
//	x = event_ptr->button.x;
//	y = event_ptr->button.y;
//
//	// [TODO] Use a #define like overlap for this, you'll need it for the
//	// user SDL_Event support later - don't fail me now!
//	if (x>=wa.x && x<wa.x+wa.w && y>=wa.y && y<wa.y+wa.h){
//		*mouse_over_widget = EG_TRUE;
//	}else{
//		*mouse_over_widget = EG_FALSE;
//	}

	*mouse_over_widget = EG_Shared_IsMouseOverWidget(widget_ptr, event_ptr);

	return(EG_TRUE);
}


EG_BOOL EG_Shared_IsMouseOverArea(EG_Widget *widget_ptr, SDL_Rect wa, SDL_Event *event_ptr)
{
        EG_Window *window_ptr;
//        SDL_Rect wa;
        int x, y;

        if ( (window_ptr= (EG_Window*) EG_Widget_GetWindow(widget_ptr) ) == NULL)
                return(EG_FALSE);

//        wa = EG_Widget_GetDimension(widget_ptr);
        wa.x += EG_Window_GetXDisplacement(window_ptr);
        wa.y += EG_Window_GetYDisplacement(window_ptr);

        x = event_ptr->button.x;
        y = event_ptr->button.y;

        if (x>=wa.x && x<wa.x+wa.w && y>=wa.y && y<wa.y+wa.h)
                return(EG_TRUE);
        else
                return(EG_FALSE);
}



EG_BOOL EG_Shared_IsMouseOverWidget(EG_Widget *widget_ptr, SDL_Event *event_ptr)
{
	EG_Window *window_ptr;
	SDL_Rect wa;
	int x, y;

	if ( (window_ptr= (EG_Window*) EG_Widget_GetWindow(widget_ptr) ) == NULL)
		return(EG_FALSE);

	wa = EG_Widget_GetDimension(widget_ptr);
	wa.x += EG_Window_GetXDisplacement(window_ptr);
	wa.y += EG_Window_GetYDisplacement(window_ptr);

	x = event_ptr->button.x;
	y = event_ptr->button.y;

	if (x>=wa.x && x<wa.x+wa.w && y>=wa.y && y<wa.y+wa.h)
		return(EG_TRUE);
	else
		return(EG_FALSE);
}

/* Will update visible state and repaint a part of a window when a widget
 * becomes visible or invisible.
 *
 * Will update the state of a widgets visible toggle and update the widgets
 * window accordingly.  If the widget becomes invisible then the window is
 * redrawn.  If the widget becomes visible then the widget is drawn.
 *
 * Returns EG_TRUE on success, the widget could change the visibility state and
 * EG_FALSE if the widget could not change it's visibility state.
 *
 * There are no actual errors returned, so it's safe to return the result from
 * this function within a visible callback (see 'EG_Callback_Generic_Visible'
 * above).
 */
EG_BOOL EG_Shared_UpdateVisibleState(EG_Widget *widget_ptr, EG_BOOL is_visible)
{
        EG_Window *window_ptr;
        SDL_Rect size;

        CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

        if ( EG_Widget_SetVisibleToggle(widget_ptr, is_visible) != EG_TRUE){
                EG_Log( EG_LOG_ERROR, dL"Changing widget '%s' visible state"
                 " failed (returned false).", dR
                 , EG_Widget_GetName(widget_ptr) );

                return(EG_FALSE);
        }

        /* If not attached to a window yet, then don't bother to render.
         */
        if ( (window_ptr=(EG_Widget_GetWindow(widget_ptr))) == NULL)
                return(EG_TRUE);


        /* If widget has become none visible, repaint the part of window
         * now vacated by the widget.
         */
        if ( is_visible == EG_FALSE ){
        	size = EG_Widget_GetDimension(widget_ptr);
		//printf("DIMENSION IS: %d %d %d %d\n", size.x, size.y, size.w, size.h);
		//SDL_Delay(2000);
                (void) EG_Window_Repaint(window_ptr, size);
        }else{
         /* Otherwise, repaint the now visible widget. (If it's stopped then
	  * the repainting will not be honored, but our state has changed,
	  * which is what we wanted anyway).
          */
                (void) EG_Widget_RepaintLot(widget_ptr);
        }

        /* Return true, change of state succeeded.
         */
        return(EG_TRUE);
}

/* Will update enabled state and repaint a widget 
 *
 * Returns EG_TRUE on success, the widget could change the enabled state and
 * EG_FALSE if the widget could not change state.
 *
 * There are no actual errors returned, so it's safe to return the result from
 * this function within a visible callback (see 'EG_Callback_Generic_Enabled'
 * above).
 */
EG_BOOL EG_Shared_UpdateEnabledState(EG_Widget *widget_ptr, EG_BOOL is_enabled)
{
        EG_Window *window_ptr;
        
        CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);
                 
        if ( EG_Widget_SetEnabledToggle(widget_ptr, is_enabled) != EG_TRUE){
                EG_Log( EG_LOG_ERROR, dL"Changing widget '%s' enabled state"
                 " failed (returned false).", dR
                 , EG_Widget_GetName(widget_ptr) );

                return(EG_FALSE);
        }
        
        /* If not attached to a window yet, then don't bother to render.
         */
        if ( (window_ptr=( EG_Window*) (EG_Widget_GetWindow(widget_ptr))) == NULL)
                return(EG_TRUE);

         /* Repaint the widget in it's new state.
         */
	(void) EG_Widget_RepaintLot(widget_ptr);

        /* Return true, change of state succeeded.
         */
        return(EG_TRUE);
}

