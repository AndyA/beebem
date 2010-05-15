/* START OF tabpage.c -----------------------------------------------------------
 *
 *	A Tab Page widget.
 *
 *	---
 *	THIS GUI IS TOTALLY *BROKEN*! PLEASE DO NOT USE IT!
 *	---
 */


#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include <gui/functions.h>
#include <gui/log.h>

#include <gui/sdl.h>

#include <gui/window.h>
#include <gui/window_private.h>

#include <gui/widget.h>
#include <gui/widget_private.h>

#include <gui/widget_shared.h>

#include <gui/tabgroup.h>
#include <gui/tabgroup_private.h>

#include <gui/tabpage.h>
#include <gui/tabpage_private.h>

#include <SDL.h>


/* Callbacks:
 */

static void Callback_Destroy(EG_Widget *widget_ptr)
{
        EG_TabPage *tabpage_ptr;
        int i;

        /* If widget is actually just a NULL, then do nothing but log error.
         */
        if (widget_ptr==NULL){
                EG_Log(EG_LOG_ERROR, dL"Tried to Destroy EG_TabPage with"
                 " NULL widget pointer.", dR);
                return;
        }

        /* Child widgets become unlinked to a EG_TabPage when the 
         * tab page is deleted.
         */
        EG_TABPAGE_GET_STRUCT_PTR_VOID(widget_ptr, tabpage_ptr);

        for(i=0; i<tabpage_ptr->count; i++){
		if (EG_Widget_GetStoppedToggle(widget_ptr) == EG_TRUE)
			(void) EG_Window_StartWidget(tabpage_ptr->child[i].widget_ptr);
        }

        /* Free EG_Widget payload (the instance of this widget class).
         */
        if (tabpage_ptr != NULL) EG_Free(tabpage_ptr);

        /* Free EG_Widget struct.
         */
        EG_Widget_Free(widget_ptr);
}

static EG_BOOL Callback_Paint(EG_Widget *widget_ptr, SDL_Rect area)
{
	EG_TabPage *tabpage_ptr;
        EG_Window *window_ptr;
	
	SDL_Rect loc;
        SDL_Color color;
        SDL_Surface *surface_ptr;

	int border=EG_Draw_Border_BoxLow;
	EG_BOOL bold;


        /* If widget falls outside area we've been asked to paint, returns
         * success.  Otherwise, function loads all the variables we'll need to
         * render the widget and returns false.
         */
        if ( EG_Shared_GetRenderingDetails(widget_ptr, area,
         (void*) &tabpage_ptr,&window_ptr,&surface_ptr,&color,&loc) != EG_TRUE )
                return(EG_TRUE);


#ifdef EG_DEBUG
        printf("SOMEONE CALLED PAINT FOR '%s' [repaint area (window)="
         "{%d, %d, %d, %d}:widget area (SDL_Surface)={%d, %d, %d, %d}]\n"
        , EG_Widget_GetName(widget_ptr), area.x, area.y, area.w, area.h
        , loc.x, loc.y, loc.w, loc.h);
#endif


	/* Dull down the button a bit when disabled.
         */
        if (EG_Widget_IsEnabled(widget_ptr) != EG_TRUE){
                color.r *= 0.7; color.g *= 0.7; color.b *= 0.7;
        }

	/* If page is selected use high border, otherwise low border,
	 * but if this page is depressed, always show low border.
	 */
	if (EG_TabPage_IsShown(widget_ptr) == EG_TRUE){
		if (tabpage_ptr->depressed == EG_TRUE)
			border = EG_Draw_Border_BoxLow;
		else
			border = EG_Draw_Border_BoxHigh;
	}else{
		if (tabpage_ptr->depressed == EG_TRUE)
			border = EG_Draw_Border_BoxHigh;
		else
			border = EG_Draw_Border_BoxLow;
	}

	/* Draw the tab page button.
	 */
	EG_Draw_Box(surface_ptr, &loc, &color);
	EG_Draw_TabBorder(surface_ptr, &loc, &color, border);

	if ( EG_Window_ThisWidgetHasFocus(widget_ptr) )
		bold = EG_TRUE;
	else
		bold = EG_FALSE;

	EG_Draw_String(surface_ptr, &color, bold, &loc, 0, tabpage_ptr->caption );

//	if (EG_Window_ThisWidgetHasFocus(widget_ptr) ){
//		loc.x++;
//		EG_Draw_String(surface_ptr, &color, EG_FALSE, &loc, 0
//		 , tabpage_ptr->caption );
//		loc.x--;
//	}

	/* Remove line from tab group box border if this tab page selected.
	 */
	loc.x +=1; loc.w -=2; loc.y += EG_TabGroup_PageButtonHeight -1;
	if (EG_TabPage_IsShown(widget_ptr) == EG_TRUE){
		loc.h = 2;
	}else{
		loc.h = 1;
	}
	EG_Draw_Box(surface_ptr, &loc, &color);


        return(EG_TRUE);
}

static EG_BOOL Callback_SDL_Event(EG_Widget *widget_ptr, SDL_Event *event_ptr)
{
        EG_TabPage *tabpage_ptr;
        EG_Window *window_ptr;
        EG_BOOL mouse_over_widget;

	EG_BOOL return_value = EG_FALSE;
        
	/* Populates variables needed to process the event.  If the event
         * shouldn't have been passed to us, then logs the fact (for bug
         * reporting) and returns false.  Quit the callback if this returns
         * false.
         */
        if (EG_Shared_GetEventDetails(widget_ptr, event_ptr
         , (void*) &tabpage_ptr, &window_ptr, &mouse_over_widget) != EG_TRUE)
                return(return_value);

        /* Process SDL Event:
         */

        if (event_ptr->type == SDL_MOUSEBUTTONDOWN || event_ptr->type
         == SDL_MOUSEBUTTONUP){

                /* If depressed = false and left mouse button is pressed and
                 * mouse pointer is within area of widget.
                 */
                if (tabpage_ptr->depressed ==EG_FALSE && event_ptr->button.state
                 == SDL_PRESSED && event_ptr->button.button == SDL_BUTTON_LEFT
                 && mouse_over_widget == EG_TRUE){

                        /* Set depressed = true, repaint widget.
                         */
                        tabpage_ptr->depressed=EG_TRUE;
                        (void) EG_Widget_RepaintLot(widget_ptr);
                }

                /* If depressed = true and left mouse button is released.
                 */
                if (tabpage_ptr->depressed == EG_TRUE && event_ptr->button.state
                 == SDL_RELEASED && event_ptr->button.button ==SDL_BUTTON_LEFT){

                        /* If released with mouse pointer within area of widget
                         * and --------the minimum click-time has been meet. 
                         */
                        if (mouse_over_widget == EG_TRUE){

                                /* Select this page.
                                 */
	                        if (EG_TabGroup_SelectPage(widget_ptr)!=EG_TRUE)
        	                        EG_Log(EG_LOG_WARNING, dL"Could not"
					 " select EG_TabPage.", dR);

				/* Call users on-click event.
				 */
				EG_Widget_CallUserOnClick(widget_ptr);
                        }

                        /* Regardless of mouse pointers location, set
                         * depressed = false; repaint widget.
                         */
                        tabpage_ptr->depressed=EG_FALSE;
                        (void) EG_Widget_RepaintLot(widget_ptr);
                }
        }

        /* If widget has focus and 'Enter' key is pressed.
         */
        if (event_ptr->type == SDL_KEYUP
         && event_ptr->key.keysym.sym == SDLK_RETURN){

                /* If this widget currently has focus on the window.
                 */
		if (EG_Window_ThisWidgetHasFocus(widget_ptr) == EG_TRUE){
			/* Select this page.
			 */

			return_value = EG_TRUE;

			if (EG_TabGroup_SelectPage(widget_ptr) != EG_TRUE)
				EG_Log(EG_LOG_WARNING, dL"Could not select"
				 " EG_TabPage.", dR);

			EG_Widget_CallUserOnClick(widget_ptr);
                }
        }

        /* If mouse over widget, and users event callback is set, call users
         * callback.             
         */
        if (mouse_over_widget == EG_TRUE)
                EG_Widget_CallUserOnEvent(widget_ptr, event_ptr);
	
	return(return_value);
}

static EG_BOOL Callback_Visible(EG_Widget *widget_ptr, EG_BOOL is_visible)
{
        EG_TabPage *tabpage_ptr;
        int i;

        /* Set the state of the tab page widget (us).
         */
        if ( EG_Widget_SetVisibleToggle(widget_ptr, is_visible) != EG_TRUE){
                EG_Log( EG_LOG_ERROR, dL"Changing widget '%s' visible state"
                 " failed (returned false).", dR
                 , EG_Widget_GetName(widget_ptr) );

                return(EG_FALSE);
        }

        /* Repaint the widget in it's new state. If not attached to a window
         * yet, then don't bother to render.
         */
        if ( EG_Widget_GetWindow(widget_ptr) != NULL )
                (void) EG_Widget_RepaintLot(widget_ptr);

        /* Change state of child widgets too by passes this state change
         * to the child widgets.
         */
        EG_TABPAGE_GET_STRUCT_PTR(widget_ptr, tabpage_ptr, EG_FALSE);

        for(i=0; i<tabpage_ptr->count; i++){
                (void) EG_Widget_CallVisible(tabpage_ptr->child[i].widget_ptr
                 , is_visible);
        }

        return(EG_TRUE);
}

static EG_BOOL Callback_Enabled(EG_Widget *widget_ptr, EG_BOOL is_enabled)
{
        EG_TabPage *tabpage_ptr;
        int i;

        /* Set the state of the tab page widget (us).
         */
        if ( EG_Widget_SetEnabledToggle(widget_ptr, is_enabled) != EG_TRUE){
                EG_Log( EG_LOG_ERROR, dL"Changing widget '%s' visible state"
                 " failed (returned false).", dR
                 , EG_Widget_GetName(widget_ptr) );

                return(EG_FALSE);
        }

        /* Repaint the widget in it's new state. If not attached to a window
         * yet, then don't bother to render.
         */
        if ( EG_Widget_GetWindow(widget_ptr) != NULL )
                (void) EG_Widget_RepaintLot(widget_ptr);

        /* Change state of child widgets too by passes this state change
         * to the child widgets.
         */
        EG_TABPAGE_GET_STRUCT_PTR(widget_ptr, tabpage_ptr, EG_FALSE);

        for(i=0; i<tabpage_ptr->count; i++){
                (void) EG_Widget_CallEnabled(tabpage_ptr->child[i].widget_ptr
                 , is_enabled);
        }

        return(EG_TRUE);
}

/* If a tab page is stopped or started it needs to pass this state change onto
 * it's children.
 */
static EG_BOOL Callback_Stopped(EG_Widget *widget_ptr, EG_BOOL is_stopped)
{
        EG_TabPage *tabpage_ptr;
	EG_BOOL page_is_shown;
        int i;

        CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);
        EG_TABPAGE_GET_STRUCT_PTR(widget_ptr, tabpage_ptr, EG_FALSE);

	/* Always honor state change for tab page widget itself
	 */
	EG_Widget_SetStoppedToggle(widget_ptr, is_stopped);

        /* Pass state change on to child widgets.  (Start will only be honored
	 * when the tab page is shown.
         */
	page_is_shown = EG_TabPage_IsShown(widget_ptr);

        for(i=0; i<tabpage_ptr->count; i++){
		if (is_stopped == EG_TRUE){
			EG_Window_StopWidget(tabpage_ptr->child[i].widget_ptr);
		}else{
			if (page_is_shown == EG_TRUE){
				EG_Window_StartWidget(tabpage_ptr->child[i].widget_ptr);
			}
		}

//		if (page_is_shown == EG_TRUE && is_stopped == EG_FALSE)
//			EG_Widget_SetStoppedToggle(tabpage_ptr->child[i].widget_ptr, is_stopped);
	}
	
	return(EG_TRUE);
}

// [TODO] This is fixed now, (forgot to do nothing when detaching {we don't need to do anything for that in this version of EG as the window
// micro-manages everything, in the final version where parent widgets manage child widgets [thus keeping their implementation secret from
// the parent], so they will need to detatch their children from window resources themselves).
static void Callback_Attach(EG_Widget *widget_ptr, EG_StringHash attach_to_type, void *attach_to_ptr, EG_BOOL attached)
{
        EG_TabPage *tabpage_ptr;
        EG_Window *window_ptr;
        int i;

        /* We only need to add the pages to the window when attaching, we do
         * nothing when detatching.
         */
        if (attached != EG_TRUE)
                return;

        CHECK_EG_WIDGET_IS_VALID_VOID(widget_ptr);
	EG_TABPAGE_GET_STRUCT_PTR_VOID(widget_ptr, tabpage_ptr);

        /* Get parent window.
         */
	window_ptr = (EG_Window*) attach_to_ptr;

        /* Add the page widgets to the parent window.
         */
        for(i=0; i<tabpage_ptr->count; i++){
                EG_Window_AddWidget(window_ptr, tabpage_ptr->child[i].widget_ptr);
        }
}


/* Private:
 */

static void InitializePayload(EG_TabPage *tabpage_ptr, const char *caption_ptr)
{
        int i;

	/* Clear link to Tab Page.
	 */
	tabpage_ptr->tabgroup_widget_ptr = NULL;

	/* Initialize child array.
	 */
        for(i=0; i<MAX_TABPAGE_CHILDREN; i++)
                tabpage_ptr->child[i].widget_ptr = NULL;
	tabpage_ptr->count = 0;

	/* Clear caption.
	 * (String size declared MAX_TABPAGE_CAPTIONSIZE+1 see tabpage.h.)
	 */
	for(i=0; i<MAX_TABPAGE_CAPTIONSIZE+1; i++)
		tabpage_ptr->caption[i]='\0';

	/* Set new caption.
	 */
	if (caption_ptr != NULL){
		if (strlen(caption_ptr) <= MAX_TABPAGE_CAPTIONSIZE)
			strcpy(tabpage_ptr->caption, caption_ptr);
		else
			strncpy(tabpage_ptr->caption, caption_ptr
			 , MAX_TABPAGE_CAPTIONSIZE);
	}

	tabpage_ptr->depressed = EG_FALSE;
}

static void InitializeWidget(EG_Widget *widget_ptr)
{
        /* Initialize callbacks:
         */
        (void) EG_Widget_SetCallback_Destroy(widget_ptr, Callback_Destroy);
        (void) EG_Widget_SetCallback_Paint(widget_ptr, Callback_Paint);
        (void) EG_Widget_SetCallback_SDL_Event(widget_ptr, Callback_SDL_Event);
        (void) EG_Widget_SetCallback_Visible(widget_ptr, Callback_Visible);
        (void) EG_Widget_SetCallback_Stopped(widget_ptr, Callback_Stopped);
        (void) EG_Widget_SetCallback_Enabled(widget_ptr, Callback_Enabled);

        (void) EG_Widget_SetCallback_GotFocus(widget_ptr, EG_Callback_Generic_GotFocus);
        (void) EG_Widget_SetCallback_LostFocus(widget_ptr, EG_Callback_Generic_LostFocus);

        (void) EG_Widget_SetCallback_Attach(widget_ptr, Callback_Attach);

	/* Initialize state:
	 */
}

// [TODO] Don't delete this yet, it can be easily called from tab groups attach
// callback, but should now be replaced with tab pages attach callback.
//
//
//
//EG_BOOL EG_TabPage_AddChildWidgetsToWindow(EG_Widget *widget_ptr)
//{
//        EG_TabPage *tabpage_ptr;
//        EG_Window *window_ptr;
//        int i;
//
//        CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);
//
//        /* Check this is an EG_TabPage widget.
//         */
//        if (EG_Widget_GetType(widget_ptr) != EG_Widget_Type_TabPage){
//                EG_Log(EG_LOG_ERROR, dL"Widget is not an EG_TabPage widget."
//                , dR);
//                return(EG_FALSE);
//        }
//
//        /* Get parent window.
//         */
//        window_ptr = EG_Widget_GetWindow(widget_ptr);
//        CHECK_EG_WINDOW_IS_VALID(window_ptr, EG_FALSE);
//
//        /* Get EG_TabPage struct.
//         */
//        if ( (tabpage_ptr=EG_Widget_GetPayload(widget_ptr)) == NULL){
//                EG_Log(EG_LOG_ERROR, dL"EG_Widget contains NULL payload.", dR);
//                return(EG_FALSE);
//        }
//
//	/* Add the page widgets to the parent window.
//	 */
//	for(i=0; i<tabpage_ptr->count; i++){
//		EG_Window_AddWidget(window_ptr, tabpage_ptr->child[i].widget_ptr);
//	}
//
//	return(EG_TRUE);
//}

/* Public functions:
 */

EG_Widget* EG_TabPage_Create(const char *name_ptr, const char *caption_ptr)
{
        EG_TabPage *tabpage_ptr;
        EG_Widget *widget_ptr;

        SHARED__ALLOC_PAYLOAD_STRUCT(tabpage_ptr, EG_TabPage
         , "Unable to malloc EG_TabPage struct");

        SHARED__CREATE_NEW_EG_WIDGET(widget_ptr, name_ptr
         , EG_Widget_Type_TabPage, tabpage_ptr);

        SHARED__ATTACH_PAYLOAD_TO_WIDGET(widget_ptr, tabpage_ptr);

        InitializePayload(tabpage_ptr, caption_ptr);
        InitializeWidget(widget_ptr);

        return(widget_ptr);
}

EG_BOOL EG_TabPage_AddWidget(EG_Widget *page_widget_ptr, EG_Widget *child_widget_ptr)
{
	EG_TabPage *tabpage_ptr;

	CHECK_EG_WIDGET_IS_VALID(page_widget_ptr, EG_FALSE);

	printf("1\n");

	if (child_widget_ptr == NULL){
		EG_Log(EG_LOG_ERROR, dL"EG_Widget is NULL, cannot add it to"
		 " EG_TabPage.", dR);
		return(EG_FALSE);
	}

	printf("2\n");

	EG_TABPAGE_GET_STRUCT_PTR(page_widget_ptr, tabpage_ptr, EG_FALSE);

	if ( (tabpage_ptr->count > MAX_TABPAGE_CHILDREN) ){
		EG_Log(EG_LOG_ERROR, dL"Can't add EG_Widget to EG_TabPage: "
                 "Would exceed max widget count.", dR);
		return(EG_FALSE);
	}

	printf("3\n");

	/* Add widget to tab page.
	 */
	tabpage_ptr->child[tabpage_ptr->count].widget_ptr = child_widget_ptr;
	tabpage_ptr->count++;

	printf("4\n");

	/* If this tab page is stopped, then all new child widgets must also be
	 * stopped.
	 */
	if (EG_Widget_GetStoppedToggle(page_widget_ptr) == EG_TRUE){
		printf("passed first if 5\n");
		if ( EG_Window_StopWidget(child_widget_ptr) != EG_TRUE)
			EG_Log(EG_LOG_ERROR, dL"Could not stop child widget."
			 , dR);
	}else{
		printf("passed first if (else) 5\n");
		/* Otherwise, if the page is shown than make sure this child
		 * widget is also started.
	 	 */
		if (EG_TabPage_IsShown(page_widget_ptr) == EG_TRUE){
			printf("6\n");
			if ( EG_Window_StartWidget(child_widget_ptr) != EG_TRUE)
				EG_Log(EG_LOG_ERROR, dL"Could not start child widget."
				 , dR);
		}else{
		/* Otherwise, stop this child widget.
		 */
			printf("7\n");
			if ( EG_Window_StopWidget(child_widget_ptr) != EG_TRUE)
				EG_Log(EG_LOG_ERROR, dL"Could not stop child widget."
				 , dR);
		}
	}

	printf("5\n");

	return(EG_TRUE);
}

/* Removes the widget from the tab page.
 */

// Think about a 'I'm registered with callback', that way stock functions can delete widgets and who they are registered
// with can be notified, so they can remove them from their list.  eg.  EG_Widget_Remove(whatever) -> registered with
// tabpage so tabpage is notified when widget is destroyed, so it knows to remove it from it's list.

void EG_TabPage_RemoveWidget(EG_Widget *page_widget_ptr, EG_Widget *child_widget_ptr)
{
	// [TODO] If this widget has focus, movefocus, if this widget still has focus, clear focus.

	// [TODO] DONT FORGET TO CLEAR ALL ORIGIN RELATED STUFF.
}

/* Switch focus to the first widget in this tab page (if does not have a
 * focusable widget, then the page button itself).
 * 
 * If the tab page has focus and you press enter, it'll do this anyway.
 *
 * Private and does not paint.
 */
EG_BOOL EG_TabPage_Show(EG_Widget *widget_ptr)
{
	EG_TabPage *tabpage_ptr;
	int i;	

	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);
	EG_TABPAGE_GET_STRUCT_PTR(widget_ptr, tabpage_ptr, EG_FALSE);

	/* Start this tab pages child widgets.
	 */
	for(i=0; i<tabpage_ptr->count; i++)
		if (EG_Window_StartWidget(tabpage_ptr->child[i].widget_ptr)
		 != EG_TRUE)
			EG_Log(EG_LOG_ERROR, dL"Failed to start widget.", dR);

	/* Give the first active widget in the tab page focus.
	 */
	// [TODO] This doesn't always work.
	printf("Setting focus to this widget\n");
	EG_Window_SetFocusToThisWidget(widget_ptr);

	return(EG_TRUE);
}

/* private and does not paint.
 *
 * Will clear focus if a widget on this tab page has focus.
 */
EG_BOOL EG_TabPage_Hide(EG_Widget *widget_ptr)
{
	EG_Widget *focused_widget_ptr;
        EG_TabPage *tabpage_ptr;
	EG_Window *window_ptr;
        int i;

        CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);
	EG_TABPAGE_GET_STRUCT_PTR(widget_ptr, tabpage_ptr, EG_FALSE);

	/* Get parent window pointer.
	 */
	window_ptr = EG_Widget_GetWindow(widget_ptr);
	if (window_ptr != NULL)
		focused_widget_ptr =
		 EG_Window_Child_GetFocusedWidget(window_ptr);
	else
		focused_widget_ptr = NULL;

	/* Stop this tab pages child widgets.
	 */
        for(i=0; i<tabpage_ptr->count; i++){
		/* If the widget we are about to stop has focus, then clear
		 * focus.  We cannot be sure that another widget will
		 * take focus later, so we must call this widgets LostFocus
 		 * callback now.  (This will render the widget.)
	 	 */
		if (focused_widget_ptr == tabpage_ptr->child[i].widget_ptr)
			EG_Window_ClearFocus(window_ptr);

                if (EG_Window_StopWidget(tabpage_ptr->child[i].widget_ptr)
                 != EG_TRUE)
                        EG_Log(EG_LOG_ERROR, dL"Failed to stop widget.", dR);
	}

	return(EG_TRUE);
}


/* Private functions:
 */

EG_BOOL EG_TabPage_RePaintChildren(EG_Widget *widget_ptr)
{
	EG_TabPage *tabpage_ptr;
	int i;

	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);
	EG_TABPAGE_GET_STRUCT_PTR(widget_ptr, tabpage_ptr, EG_FALSE);

	/* Repaint this tab pages child widgets.
	 */
	for(i=0; i<tabpage_ptr->count; i++)
		if (EG_Widget_RepaintLot(tabpage_ptr->child[i].widget_ptr)
		 != EG_TRUE)
			EG_Log(EG_LOG_ERROR, dL"Failed to repaint tab page"
			 " child widget.", dR);

	return(EG_TRUE);
}

/* Returns true if this page is currently shown, false otherwise.
 */
EG_BOOL EG_TabPage_IsShown(EG_Widget *widget_ptr)
{
	EG_TabPage *tabpage_ptr;
	EG_Widget *selected_widget_ptr;

	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);
	EG_TABPAGE_GET_STRUCT_PTR(widget_ptr, tabpage_ptr, EG_FALSE);

	/* If not associated with a tab group yet return false.
	 */
	if (tabpage_ptr->tabgroup_widget_ptr == NULL)
		return(EG_FALSE);

	selected_widget_ptr =
	 EG_TabGroup_GetSelectedPageWidget(tabpage_ptr->tabgroup_widget_ptr);

	if ( EG_Widget_GetID(selected_widget_ptr)
	 == EG_Widget_GetID(widget_ptr) )
		return(EG_TRUE);
	else
		return(EG_FALSE);
}

const char* EG_TabPage_GetCaption(EG_Widget *widget_ptr)
{
	EG_TabPage *tabpage_ptr;

	CHECK_EG_WIDGET_IS_VALID(widget_ptr, NULL);
	EG_TABPAGE_GET_STRUCT_PTR(widget_ptr, tabpage_ptr, NULL);

	return( (const char*) tabpage_ptr->caption);
}

EG_BOOL EG_TabPage_SetTabGroup(EG_Widget *page_widget_ptr, EG_Widget *group_widget_ptr)
{
	EG_TabPage *tabpage_ptr;

	CHECK_EG_WIDGET_IS_VALID(page_widget_ptr, EG_FALSE);
	EG_TABPAGE_GET_STRUCT_PTR(page_widget_ptr, tabpage_ptr, EG_FALSE);

	tabpage_ptr->tabgroup_widget_ptr = group_widget_ptr;

	return(EG_TRUE);
}

EG_Widget* EG_TabPage_GetTabGroup(EG_Widget *page_widget_ptr)
{
        EG_TabPage *tabpage_ptr;

        CHECK_EG_WIDGET_IS_VALID(page_widget_ptr, NULL);
	EG_TABPAGE_GET_STRUCT_PTR(page_widget_ptr, tabpage_ptr, NULL);

        return(tabpage_ptr->tabgroup_widget_ptr);
}

