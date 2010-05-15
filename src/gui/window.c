/* START OF window.c -----------------------------------------------------------
 *
 *	Window API.
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

#include <gui/sdl.h>

#include <gui/window.h>
#include <gui/window_private.h>

#include <gui/widget.h>
#include <gui/widget_private.h>

#include <SDL.h>


/* Private static functions:
 */

static EG_BOOL InitializeWindowStruct(EG_Window *window_ptr,const char *name_ptr);
static EG_BOOL Process_SDL_Event(EG_Window *window_ptr, SDL_Event *event_ptr);
static EG_BOOL MoveFocus(EG_Window *window_ptr, int direction);


/* Initialize new EG_Window struct to default values.
 */
static EG_BOOL InitializeWindowStruct(EG_Window *window_ptr,const char *name_ptr)
{
	static unsigned long unique_ID = 0;
	Uint16 i;

	CHECK_EG_WINDOW_IS_VALID(window_ptr, EG_FALSE);

	/* Give the new struct a unique ID.  (Useful for comparing two struct
	 * pointers).
	 *
	 * If after increment the value wraps to zero, add one, as we want
	 * zero as an ID to signify an error.  This shouldn't happen unless the
	 * GUI is running for ages, or creates and destroys *LOTS* of windows.
	 */
	if (++unique_ID == 0){
		EG_Log(EG_LOG_INFO, dL"EG_Window struct ID counter wrapped."
		 , dR);
		unique_ID++;
	}
	window_ptr->ID = unique_ID;

	/* Initialize child widget pointer list.
	 */
	window_ptr->count = 0;
	for(i=0; i<EG_WINDOW_MAX_CHILD_WIDGETS; i++){
		window_ptr->widget_ptr[i] = NULL;
	}
	/* Window has no child widgets yet, so fix widget with focus.
	 */
	window_ptr->widget_with_focus = -1;

	/* Window is in 'hidden' state.
	 */
	window_ptr->hidden = EG_TRUE;

	/* Set name, fail if cannot, EG_Window_SetName will log failure.
	 */
	if(EG_Window_SetName(window_ptr, name_ptr)== EG_FALSE) return(EG_FALSE);

	/* Set to a default background color of gray.
	 */
	window_ptr->background_color = ( (SDL_Color) {127, 127, 127, 0} );

	window_ptr->window_background_surface_ptr = NULL;

	/* Set everything else.
	 */
	EG_Window_SetSurface(window_ptr, NULL);
	EG_Window_SetDimension(window_ptr, ( (SDL_Rect) {0,0,0,0}) );
	EG_Window_SetHoldValue(window_ptr, NULL);

	return(EG_TRUE);
}

/* Alloc new EG_Window struct and then initialize it to default values.
 *
 * (use this in the 'Create' function for each widget).
 */
EG_Window* EG_Window_Alloc(const char *name_ptr)
{
	EG_Window *window_ptr = NULL;

	if ( (window_ptr = EG_AsWindow(EG_Malloc(sizeof(EG_Window))) ) == NULL){
		EG_Log(EG_LOG_WARNING, dL"Unable to malloc EG_Window struct"
		 , dR);
		return(NULL);
	}

	if(InitializeWindowStruct(window_ptr, name_ptr) == EG_FALSE){
		/* InitializeWindowStruct will log the reason for failure.
		 *
		 * We handle freeing the struct ourselves rather than call
		 * EG_Window_Free.  If EG_Window_Free changes, then we don't
		 * care, as we've not created any buffers anyway.
		 */

		EG_Free(window_ptr);
		return(NULL);
	}

	return(window_ptr);
}

/* Destroy all child widgets - useful!
 *
 * We don't have to worry about focus as we are going to destroy all widgets
 * associated with the window.  We just need to clear the child array at the
 * end.
 *
 * Some widgets may delete others, so rather than using a for/next loop to
 * iterate through the child widgets.  We should continuely delete the first
 * one until none are left.
 */
void EG_Window_DestroyAllChildWidgets(EG_Window *window_ptr)
{
        EG_Widget *tmp_widget_ptr;

	CHECK_EG_WINDOW_IS_VALID_VOID(window_ptr);

	while ( EG_Window_Child_GetCount(window_ptr) != 0 ){
		/* Find EG_Widget instance and remove it from the window child
		 * list.
	 	 */

		tmp_widget_ptr = EG_Window_Child_Get(window_ptr, 0);
		EG_Window_Child_Remove(window_ptr, 0);

		if(tmp_widget_ptr != NULL){

			/* Inform widget it's been removed from the window via
			 * callback.
		 	 * (It doesn't know it's going to be physically
			 * destroyed though.)
		 	 */

			EG_Widget_CallAttach(tmp_widget_ptr
			 , EG_Window_Type_Window, (void*) window_ptr, EG_FALSE);

			/* Physically destroy the child widget.
			 */
			EG_Widget_CallDestroy(tmp_widget_ptr);
		}

	}

	/* No widget has focus now.
	 */
	window_ptr->widget_with_focus = -1;
}


/* Free EG_Window struct, call EG_Window_Destroy to delete child widgets too.
 */
void EG_Window_Free(EG_Window *window_ptr)
{
	CHECK_EG_WINDOW_IS_VALID_VOID(window_ptr);

	/* Check that Window has no children. If it does we need to
	 * delete it anyway, but log the fact, as this could create a memory
	 * leak.
	 */
	if (window_ptr->count > 0)
		EG_Log(EG_LOG_WARNING, dL"Freeing window when children exist"
		 , dR);

	EG_Free(window_ptr);
}


/* Will force a window to clear the focused widget. I.E. After a call to this,
 * no widget on the parent window will have focus.
 *
 * Some of the focus switching code will call this function prior to setting
 * focus to another widget (see EG_Window_SetFocusToThisWidget below), so please
 * don't slow down this function too much.  If you want a slow but sure focus
 * *reset* function that's only used in emergencies, then please add one!  But
 * don't slow this down.
 */
void EG_Window_ClearFocus(EG_Window *window_ptr)
{
	EG_Widget *widget_ptr;

	CHECK_EG_WINDOW_IS_VALID_VOID(window_ptr);

	widget_ptr = EG_Window_Child_GetFocusedWidget(window_ptr);
	window_ptr->widget_with_focus = -1;
	if (widget_ptr != NULL){
		EG_Widget_CallLostFocus(widget_ptr);
		EG_Widget_RepaintLot(widget_ptr);
	}
}

void EG_Window_SetDimension(EG_Window *window_ptr, SDL_Rect dimension)
{
	CHECK_EG_WINDOW_IS_VALID_VOID(window_ptr);
	window_ptr->dimension = dimension;
}

EG_BOOL EG_Window_SetName(EG_Window *window_ptr, const char *name_ptr)
{
	CHECK_EG_WINDOW_IS_VALID(window_ptr, EG_FALSE);

	/* Check name string is not too long.
         */
        if (name_ptr == NULL || strlen(name_ptr) > EG_WINDOW_MAX_NAME_SIZE){
                EG_Log(EG_LOG_ERROR, dL"name_ptr string too long or NULL.", dR);
		return(EG_FALSE);
        }

        /* Copy name string.
         */
        strcpy(window_ptr->name, name_ptr);

	return(EG_TRUE);
}

void	EG_Window_SetSurface(EG_Window *window_ptr, SDL_Surface *surface_ptr)
{
	CHECK_EG_WINDOW_IS_VALID_VOID(window_ptr);
	window_ptr->surface_ptr = surface_ptr;
}

void	EG_Window_SetHoldValue(EG_Window *window_ptr, void *hold_value_ptr)
{
	CHECK_EG_WINDOW_IS_VALID_VOID(window_ptr);
	window_ptr->hold_value_ptr = hold_value_ptr;
}

void* 	EG_Window_GetHoldValue(EG_Window *window_ptr)
{
	CHECK_EG_WINDOW_IS_VALID(window_ptr, NULL);
	return(window_ptr->hold_value_ptr);
}


unsigned long EG_Window_GetID(EG_Window *window_ptr)
{
	CHECK_EG_WINDOW_IS_VALID(window_ptr, 0);
	return(window_ptr->ID);
}


/* Diagnostics:
 */


void EG_Window_Diagnositc_DumpChildren(EG_Window *window_ptr)
{
	SDL_Rect clipping;
	long i;
	EG_Widget *tmp_widget_ptr;;

      CHECK_EG_WINDOW_IS_VALID_VOID(window_ptr);

	printf("Window '%s' child widgets:\n", EG_Window_GetName(window_ptr));


	for (i=0; i<window_ptr->count; i++){
		tmp_widget_ptr = window_ptr->widget_ptr[i];

		clipping = EG_Widget_GetDimension(tmp_widget_ptr);

//		printf("%8X|%10s|%d|%d|(%03d %03d %03d %03d)\n"
//		 , tmp_widget_ptr, EG_Widget_GetName(tmp_widget_ptr), (unsigned int) EG_Widget_GetCanGetFocusToggle(tmp_widget_ptr)
//		 , EG_Window_ThisWidgetHasFocus(tmp_widget_ptr) 
//		 , clipping.x, clipping.y, clipping.w, clipping.h
//		);
        }
}



/* ---------------------------------------------------------- */


long EG_Window_Child_GetCount(EG_Window *window_ptr)
{
	CHECK_EG_WINDOW_IS_VALID(window_ptr, 0);
	return(window_ptr->count);
}

EG_BOOL EG_Window_Child_SetFocusedIndex(EG_Window *window_ptr, long new_index)
{
	CHECK_EG_WINDOW_IS_VALID(window_ptr, EG_FALSE);
	
	window_ptr->widget_with_focus = new_index;
	return(EG_TRUE);
}

EG_BOOL EG_Window_Child_SetFocusedIndexByWidget(EG_Window *window_ptr, EG_Widget *widget_ptr)
{
	EG_Window *widget_window_ptr;
	long new_index;

	CHECK_EG_WINDOW_IS_VALID(window_ptr, EG_FALSE);
	CHECK_EG_WIDGET_IS_VALID(window_ptr, EG_FALSE);

        // Check widget has same window as window.
        widget_window_ptr = (EG_Window*) EG_Widget_GetWindow(widget_ptr);
        if ( EG_Window_GetID(window_ptr)
         != EG_Window_GetID(widget_window_ptr) ){
                EG_Log(EG_LOG_ERROR, dL"Referenced widget is not on this"
		 " window!", dR);
                return(-1);
        }

	// Get the index of this widget.	
	new_index = EG_Window_Child_GetIndexFromWidget(window_ptr, widget_ptr);
	if (new_index < 0) return(EG_FALSE);

	// Set focused index.
	return( EG_Window_Child_SetFocusedIndex(window_ptr, new_index) );
}


/* Returns pointer to widget with focus
 */
EG_Widget* EG_Window_Child_GetFocusedWidget(EG_Window *window_ptr)
{
	CHECK_EG_WINDOW_IS_VALID(window_ptr, NULL);

	if (window_ptr->widget_with_focus == -1)
		return(NULL);

	return( EG_Window_Child_Get(window_ptr
	 , window_ptr->widget_with_focus) );
}

/* Converts widget_ptr into child array index.
 * -1 = error or not assigned.
 */
long EG_Window_Child_GetIndexFromWidget(EG_Window *window_ptr
 , EG_Widget *widget_ptr)
{
	EG_Window *widget_window_ptr;

	CHECK_EG_WINDOW_IS_VALID(window_ptr, -1);

	// Check widget has same window as window.
	widget_window_ptr = (EG_Window*) EG_Widget_GetWindow(widget_ptr);	
	if ( EG_Window_GetID(window_ptr) 
	 != EG_Window_GetID(widget_window_ptr) ){
		EG_Log(EG_LOG_ERROR, dL"Referenced widget is not on this"
		 " window!", dR);
		return(-1);
	}

	return(EG_Widget_GetHoldValue(widget_ptr));
}

EG_Widget* EG_Window_Child_Get(EG_Window *window_ptr, long from_index)
{
	CHECK_EG_WINDOW_IS_VALID(window_ptr, NULL);

	// Check for index under/over run
	if (from_index < 0 || from_index >= window_ptr->count){
		EG_Log(EG_LOG_ERROR, dL"Tried to access array index out of"
		 " range.", dR);
		return(NULL);
	}

	// Return widget
        return(window_ptr->widget_ptr[from_index]);
}

EG_BOOL EG_Window_Child_Remove(EG_Window *window_ptr, long from_index)
{
	long i;
	EG_Widget *tmp_widget_ptr;;

	CHECK_EG_WINDOW_IS_VALID(window_ptr, EG_FALSE);

	// Check for index under/over run
	if (from_index < 0 || from_index >= window_ptr->count){
		EG_Log(EG_LOG_ERROR, dL"Tried to delete array index out of"
		 " range.", dR);
		return(EG_FALSE);
	}

	// Clear link to widget we are deleting from window.
	tmp_widget_ptr = window_ptr->widget_ptr[from_index];

	(void) EG_Widget_SetWindow(tmp_widget_ptr, NULL);
	(void) EG_Widget_SetHoldValue(tmp_widget_ptr, -1);

	// If deleting last item, simply clear it.
	if (from_index == window_ptr->count - 1){
		window_ptr->widget_ptr[from_index] = NULL;
		// [Clear widget]
	}else{
	// If deleting from anywhere else, move all higher indexes down one.
		for (i=from_index + 1; i<window_ptr->count; i++){
			tmp_widget_ptr = window_ptr->widget_ptr[i];
			(void) EG_Widget_SetHoldValue(tmp_widget_ptr, i-1);

			window_ptr->widget_ptr[i-1] = window_ptr->widget_ptr[i];
		}
		window_ptr->widget_ptr[window_ptr->count-1] = NULL;
	}

	// Dec count
	window_ptr->count--;
	
	return(EG_TRUE);
}

EG_BOOL EG_Window_Child_Add(EG_Window *window_ptr, EG_Widget *widget_ptr)
{
	CHECK_EG_WINDOW_IS_VALID(window_ptr, EG_FALSE);
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	// Check that more children can be added (-1 as indexed from zero).
	if (window_ptr->count>EG_WINDOW_MAX_CHILD_WIDGETS-1){
		EG_Log(EG_LOG_ERROR, dL"Cannot add more child widgets to window"
		 " '%s', max count met."
		 , dR, EG_Window_GetName(window_ptr));
		return(EG_FALSE);
	}

	// Inc child widgets count
	window_ptr->count++;

	// Set new array index
	window_ptr->widget_ptr[window_ptr->count-1]=widget_ptr;

	// Set window and hold-value of widget.
	(void) EG_Widget_SetWindow(widget_ptr, window_ptr);
        (void) EG_Widget_SetHoldValue(widget_ptr, window_ptr->count-1);

	return(EG_TRUE);
}

EG_BOOL EG_Window_AddWidget(EG_Window *window_ptr, void *widget_ptr)
{
//[HERE]       CHECK_EG_WINDOW_IS_VALID(window_ptr, EG_FALSE);
//	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	if( EG_Window_Child_Add(window_ptr, (EG_Widget*) widget_ptr) == EG_TRUE ){
		/* Inform widget it's been attached to a window via callback.
		 */
		EG_Widget_CallAttach( (EG_Widget*) widget_ptr, EG_Window_Type_Window
		 , (void*) window_ptr, EG_TRUE);

		/* If window is shown, render the widget.
		 */
		EG_Widget_RepaintLot( (EG_Widget*) widget_ptr);

		return(EG_TRUE);
	}else{
		return(EG_FALSE);
	}
}

EG_BOOL EG_Window_RemoveWidget(EG_Window *window_ptr, void *widget_ptr)
{
	long from_index;
	SDL_Rect size;

	CHECK_EG_WINDOW_IS_VALID(window_ptr, EG_FALSE);
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	/* If this widget has focus, move focus to the next widget.  If this is
	 * the only widget that can receive focus, then force loss of focus to
	 * all widgets on this window.
 	 */
	if (EG_Window_ThisWidgetHasFocus( (EG_Widget*) widget_ptr) == EG_TRUE){

		(void) EG_Window_MoveFocusForward(window_ptr);

		/* If widget to be deleted STILL has focus after moveing focus
	 	 * to the next widget (ie. this is the only widget that can have
		 * focus), force window to lose this widgets focus.
		 */
		if (EG_Window_ThisWidgetHasFocus( (EG_Widget*) widget_ptr) == EG_TRUE)
			EG_Window_ClearFocus(window_ptr);
	}

	/* Find widget in windows child list.
	 */
	from_index = EG_Window_Child_GetIndexFromWidget(window_ptr, (EG_Widget*) widget_ptr);
	if (from_index == -1){
		EG_Log(EG_LOG_ERROR, dL"Cannot remove widget from window, not"
		 " on window", dR);
		return(EG_FALSE);
	}

	/* Remove widget from windows child list.
	 */
	(void) EG_Window_Child_Remove(window_ptr, from_index);

	/* Inform widget it's been removed from the window via callback.
	 */
	EG_Widget_CallAttach( (EG_Widget*) widget_ptr, EG_Window_Type_Window
                 , (void*) window_ptr, EG_FALSE);

	/* Repaint part of window widget has vacated.
	 */
	size = EG_Widget_GetDimension( (EG_Widget*) widget_ptr);
	EG_Window_Repaint(window_ptr, size);

	return(EG_TRUE);
}

/* Wrappers for MoveFocus below.
 */
EG_BOOL EG_Window_MoveFocusForward(EG_Window *window_ptr)
{
	return( MoveFocus(window_ptr, 1) );
}

EG_BOOL EG_Window_MoveFocusBackward(EG_Window *window_ptr)
{
	return( MoveFocus(window_ptr, -1) );
}

/* Move focus to next widget (in child list) that can receive focus.
 *
 * If no widget can receive focus returns EG_FALSE, otherwise EG_TRUE is
 * returned (even if the same widget gets focus again).
 *
 * This function is private, it should only be called by the SDL Event handler
 * or a widget implementations LoseFocus callback.
 *
 * Note: Make sure hidden windows can still call this function.
 *       Otherwise when a child widget with focus is deleted, it will not
 *       lose focus!
 */
static EG_BOOL MoveFocus(EG_Window *window_ptr, int direction)
{
	EG_Widget *widget_ptr;
	long current_index, start_index;

	CHECK_EG_WINDOW_IS_VALID(window_ptr, EG_FALSE);

	/* If window has no child widgets, cannot move focus.  This is 
	 * considered successful.
	 */
	if (EG_Window_Child_GetCount(window_ptr) == 0)
		return(EG_TRUE);

	/* If a widget currently has focus, store that widgets index +
	 * direction (1 or -1) (wrap round index if required) and lose focus).
	 *
	 * If the window does not have a focused widget, then store index 0.
	 */
	if ( (widget_ptr=EG_Window_Child_GetFocusedWidget(window_ptr)) != NULL){
		current_index = EG_Window_Child_GetIndexFromWidget(window_ptr
		 , widget_ptr);

		current_index += direction;
		if (current_index >= EG_Window_Child_GetCount(window_ptr) )
		 current_index = 0;
		if (current_index < 0) current_index = EG_Window_Child_GetCount(
		 window_ptr) - 1;

		EG_Widget_CallLostFocus(widget_ptr);

		/* Clear focus so widget will be repainted without focus.
		 */
		window_ptr->widget_with_focus = -1;

		EG_Widget_RepaintLot(widget_ptr);
	}else{
		current_index = 0;
	}

	/* Remember the index of the starting widget, when we iterate through
	 * the widgets, if we get back here, then we know we've completed a
	 * cycle of the windows child widgets and our search condition was
	 * never met.
	 */
	start_index = current_index;

	/* Iterate through child widgets. End iteration after full loop of
	 * child widgets.
	 */
	do{
		/* Try and switch focus to next widget at index.  If success
		 * then return true.
		 */
		widget_ptr = EG_Window_Child_Get(window_ptr, current_index);

		if ( EG_Widget_CallGotFocus(widget_ptr) == EG_TRUE ){
			if (EG_Window_Child_SetFocusedIndex(window_ptr
			 , current_index)
			 != EG_TRUE){
				EG_Log(EG_LOG_ERROR, dL"Window failed to switch"
				 " focus index to new widget. This is a bug :-("
				 , dR);
			}else{
				EG_Widget_RepaintLot(widget_ptr);
			}
			return(EG_TRUE);
		}

		/* Otherwise, inc. index (with wrap to zero if required) and
		 * continue iterating through the windows child widgets.
		 */
		current_index += direction;
		if (current_index >= EG_Window_Child_GetCount(window_ptr) )
		 current_index = 0;

		if (current_index < 0) current_index =
		 EG_Window_Child_GetCount(window_ptr) -1;

	}while(current_index != start_index);

	return(EG_TRUE);
}


/* Public functions:
 */

EG_Window* EG_Window_Create(char *name_ptr, SDL_Surface *surface_ptr
 , SDL_Color background_color, SDL_Rect dimension)
{
	EG_Window *window_ptr = NULL;

	if ( (window_ptr=EG_Window_Alloc(name_ptr)) == NULL){
		/* EG_Window_Alloc will log failure.
	 	 */
		return(NULL);
	}

	EG_Window_SetSurface(window_ptr, surface_ptr);

	/* Set the dimension, if the user sends {0,0,0,0} for dimension, make
	 * same size as SDL_Surface.
	 */
	if (dimension.x + dimension.y + dimension.w + dimension.h == 0
	 && surface_ptr != NULL){
		dimension.x = 0;
		dimension.y = 0;
		dimension.w = surface_ptr->w;
		dimension.h = surface_ptr->h;
	}
	EG_Window_SetDimension(window_ptr, dimension);	

	/* Set background color.
	 */
	window_ptr->background_color = background_color;

	return(window_ptr);
}

void EG_Window_Destroy(EG_Window *window_ptr)
{
	/* Restore background if window was shown etc.
	 */
	(void) EG_Window_Hide(window_ptr);

	/* Now free the EG_Window struct.
	 */
	EG_Window_Free(window_ptr);
}

/* Accessor Wrappers:
 */

const char* EG_Window_GetName(EG_Window *window_ptr)
{
	CHECK_EG_WINDOW_IS_VALID(window_ptr,NULL);
	return(window_ptr->name);
}

SDL_Surface* EG_Window_GetSurface(EG_Window *window_ptr)
{
	CHECK_EG_WINDOW_IS_VALID(window_ptr,NULL);
	return(window_ptr->surface_ptr);
}

SDL_Rect EG_Window_GetDimension(EG_Window *window_ptr)
{
	CHECK_EG_WINDOW_IS_VALID(window_ptr, ((SDL_Rect) {0,0,0,0}) );
	return(window_ptr->dimension);	
}

SDL_Color EG_Window_GetBackgroundColor(EG_Window *window_ptr)
{
	CHECK_EG_WINDOW_IS_VALID(window_ptr, ((SDL_Color) {0,0,0,0}) );
	return(window_ptr->background_color);
}

EG_BOOL EG_Window_IsHidden(EG_Window *window_ptr)
{
	CHECK_EG_WINDOW_IS_VALID(window_ptr, EG_FALSE);
	return(window_ptr->hidden);
}

// [TODO] This should be removed.
/* Returns the drawing area on the source page SDL_Surface.
 */
//SDL_Rect EG_Window_GetWidgetDrawingArea(EG_Window *window_ptr
// , EG_Widget *widget_ptr)
//{
//	SDL_Rect tmp = ( (SDL_Rect) {0,0,0,0});
//
//	CHECK_EG_WINDOW_IS_VALID(window_ptr, tmp);
//	CHECK_EG_WIDGET_IS_VALID(widget_ptr, tmp);
//
////	tmp = EG_Widget_GetDimension(widget_ptr);
//
//	tmp.x += EG_Window_GetXDisplacement(window_ptr);
//	tmp.y += EG_Window_GetYDisplacement(window_ptr);
//
//	return(tmp);
//}

/* Return the X offset for the top left of the window including insets, relative
 * to the windows source SDL_Surface.
 * 
 * The +2 is the current hardwired left window inset.
 */
Uint16 EG_Window_GetXDisplacement(EG_Window *window_ptr)
{
	CHECK_EG_WINDOW_IS_VALID(window_ptr, 0);
	return(window_ptr->dimension.x +2);	
}

/* Return the Y offset for the top left of the window including insets, relative
 * to the windows source SDL_Surface.
 *
 * The +2 is the current hardwired top window inset.
 */
Uint16 EG_Window_GetYDisplacement(EG_Window *window_ptr)
{
	CHECK_EG_WINDOW_IS_VALID(window_ptr, 0);
	return(window_ptr->dimension.y +2);
}

/* Return the Width of a window (with insets taken into account), relative to
 * the X offset and Y offset functions above.
 *
 * The 2+2 is the current hardwired left and right window insets.
 */
Uint16 EG_Window_GetWidth(EG_Window *window_ptr)
{
	CHECK_EG_WINDOW_IS_VALID(window_ptr, 0);
	return( window_ptr->dimension.w - (2+2) );
}

/* Return the Heigh of a window (with insets taken into account), relative to
 * the X offset and Y offset functions above.
 *
 * The 2+2 is the current hardwired top and bottom window insets.
 */
Uint16 EG_Window_GetHeight(EG_Window *window_ptr)
{
	CHECK_EG_WINDOW_IS_VALID(window_ptr, 0);
	return( window_ptr->dimension.h - (2+2) );
}

/* Window actions:
 */

/* Repaint whole window without need of SDL_Rect arg
 */
EG_BOOL EG_Window_RepaintLot(EG_Window *window_ptr)
{
	CHECK_EG_WINDOW_IS_VALID(window_ptr, EG_FALSE);

	SDL_Rect size = EG_Window_GetDimension(window_ptr);

	// [TODO] Shouldn't x and y be 0 anyway?

	// We only want the width and hieght, window top left = 0,0
	size.x = 0; size.y = 0;

	return(EG_Window_Repaint(window_ptr, size));
}

/* Repaints part of the window.
 */

/*
 * area is relative to the window not the SDL_Surface.
 */
EG_BOOL EG_Window_Repaint(EG_Window *window_ptr, SDL_Rect area)
{
	long i;
	EG_Widget *tmp_widget_ptr;
	SDL_Color color;
	SDL_Surface *surface_ptr;
	SDL_Rect size;

	//SDL_Delay(3000);

	CHECK_EG_WINDOW_IS_VALID(window_ptr, EG_FALSE);

	/* Don't do anything if window not visible.
	 */
	if (window_ptr->hidden != EG_FALSE)
		return(EG_TRUE);

	/* Get the windows actual dimension
	 */
	size = EG_Window_GetDimension(window_ptr);

	/* Clip width and height to fit (as widget may go off the page).
	 * Do not use size.x, size.y here.
	 */
//	if (area.x+area.w > size.w)
//		area.w = size.w - area.x;
//	if (area.y+area.h > size.h)
//		area.h = size.h - area.y;

	/* Convert window relative area coordinates into
	 * the SDL_Surface specific absolute coordinates the
	 * window will expect.
	 */
//	area.x += EG_Window_GetXDisplacement(window_ptr);
//	area.y += EG_Window_GetYDisplacement(window_ptr);

	/* Area shouldn't be negative (after insets are taken into account).
	 */
//	if (area.x < 0){
//		EG_Log(EG_LOG_WARNING, dL"Window area to repaint has"
//		 " negative X coordinate.", dR);
//		area.w -= (0 - area.x);
//		area.x = 0;
//	}
//	if (area.y <0){
//		EG_Log(EG_LOG_WARNING, dL"Window area to repaint has"
//		 " negative Y coordinate.", dR);
//		area.h -= (0 - area.y);
//		area.y = 0;
//	}

	/* Get surface and color details.
	 */
	surface_ptr = EG_Window_GetSurface(window_ptr);
	color = EG_Window_GetBackgroundColor(window_ptr);

	//printf("area = %d %d %d %d, size = %d %d %d %d\n", area.x, area.y, area.w, area.h, size.x, size.y, size.w, size.h);

	/* Convert coordinates so they are relative to the SDL_Surface.
	 * (do not send size to widgets.)
	 */

	//printf("area = %d %d %d %d, size = %d %d %d %d\n", area.x, area.y, area.w, area.h, size.x, size.y, size.w, size.h);

	/* If not repainting the whole window, only fill the exposed part.
	 */
	if (area.x==0 && area.y==0 && area.w==size.w
	 && area.h==size.h){
		/* Convert coordinates so they are relative to the SDL_Surface.
	 	 */
//
//
//
//		size.x = EG_Window_GetXDisplacement(window_ptr);
//		size.y = EG_Window_GetYDisplacement(window_ptr);
//
//
//
//
		EG_Draw_Box(surface_ptr, &size, &color);
		EG_Draw_Border(surface_ptr, &size, &color
		 , EG_Draw_Border_BoxHigh);
		//printf("---> FILLING ***WHOLE*** WINDOW\n");
		//SDL_Delay(3000);
	}else{
		//color = ( (SDL_Color) {0, 255,255,0} );

		size = area;
		size.x += EG_Window_GetXDisplacement(window_ptr);
		size.y += EG_Window_GetYDisplacement(window_ptr);

		//printf("area = %d %d %d %d, size = %d %d %d %d\n", area.x, area.y, area.w, area.h, size.x, size.y, size.w, size.h);

		EG_Draw_Box(surface_ptr, &size, &color);

		//printf("---> FILLING ONLY A PART OF THE WINDOW\n");
		//SDL_Delay(3000);
	}

	/* Repaint child widgets.
	 */
	for(i=0; i<EG_Window_Child_GetCount(window_ptr); i++){
		tmp_widget_ptr = EG_Window_Child_Get(window_ptr, i);

		if(tmp_widget_ptr != NULL){
			if (EG_Widget_IsVisible(tmp_widget_ptr) == EG_TRUE){
				if (EG_Widget_CallPaint(tmp_widget_ptr, area)
				 ==EG_FALSE){
					EG_Log(EG_LOG_ERROR, dL"Repainting"
					 " widget '%s' failed (returned false)."
					 , dR
					 , EG_Widget_GetName(tmp_widget_ptr));
				}
			}
		}else{
			EG_Log(EG_LOG_ERROR, dL"Repainting widget index %d"
			 " failed (is NULL)", dR, (int) i);
		}
	}

	return(EG_TRUE);
}

static EG_BOOL Process_SDL_Event(EG_Window *window_ptr, SDL_Event *event_ptr)
{
	/* Process TAB key press:
	 */

	// [TODO] only checks shift state, maybe should check no meta keys are
	//        pressed to allow a TAB.
	if(event_ptr->key.keysym.sym==SDLK_TAB && event_ptr->type==SDL_KEYDOWN){
		if ( (SDL_GetModState() & KMOD_SHIFT) == 0){
			//printf("**** TAB ****\n");
			EG_Window_MoveFocusForward(window_ptr);
		}else{
			//printf("**** SHIFT TAB ****\n");
			EG_Window_MoveFocusBackward(window_ptr);
		}

		/* Don't pass TAB keypress event to children (will need to
		 * rethink this should 'complete' text boxes be implemented.
		 */
		return(EG_TRUE);
	}

	/* Window doesn't take event, so allow children to process it too.
	 */
	return(EG_FALSE);
}

/* x/y_displacement: If the event originates from a surface other than the on
 * the user must specify the amount of displacement to take into consideration.
 *
 * For instance:  The EG window renders to an off-screen surface.  This surface
 * is overlayed the actual visible screen by the user.  If the user blits the
 * window surface at anywhere other than the top left hand corner, then they
 * must also specify the displacement.  This displacement is subtracted from
 * mouse events so the coordinates fit the EG windows surface.
 *
 * When waiting and no events are available call this function with
 * event_ptr = NULL.  It's nice to keep calling this function at least every
 * 10ms so it can perform house keeping (timers) and finish events (such as
 * mouse movement grouping, animating, keyboard auto repeat etc).
 */
EG_BOOL EG_Window_ProcessEvent(EG_Window *window_ptr, SDL_Event *event_ptr
 , int x_displacement, int y_displacement)
{
	long i;
	EG_Widget *tmp_widget_ptr;

	CHECK_EG_WINDOW_IS_VALID(window_ptr, EG_FALSE);

	if (event_ptr == NULL){
		/* If no event passed, then call the 'polling' user
		 * event, this is executed (in beebem) every 10ms,
		 * and allows the slide bar to group mouse motion and
		 * auto repeat etc.
		 */

		SDL_Event event;

		memset(&event, 0, sizeof(event));

		event.type = SDL_USEREVENT;
		event.user.code = EG_USER_SDL_EVENT_POLL;
		event.user.data1 = 0;
		event.user.data2 = 0;
		SDL_PushEvent(&event);

		return EG_TRUE;
	}

	/* Transform event so that it's relative to scale:
	 */
	if (event_ptr->type != SDL_USEREVENT){
		event_ptr->motion.x = (int) (event_ptr->motion.x / EG_Draw_GetScale() );
		event_ptr->motion.y = (int) (event_ptr->motion.y / EG_Draw_GetScale() );

		/* You don't need to update event_ptr->button.x/y, it's the same value.
		 */
		event_ptr->motion.x-= x_displacement;
		event_ptr->motion.y-= y_displacement;
	}

	// [TODO] Once all widgets converted to widget_shared.c code, make mouse coordinates relative to window coordinates so widget_shared.c code does not have to add the window x,y ofset within the surface. (See EG_Shared_GetEventDetails)

	if (Process_SDL_Event(window_ptr, event_ptr) != EG_TRUE){

		// [TODO] What happens if an event courses a widget to delete itself?
		for(i=0; i<EG_Window_Child_GetCount(window_ptr); i++){
			tmp_widget_ptr = EG_Window_Child_Get(window_ptr, i);

			/* Pass event to widget, if widget returns EG_TRUE then
			 * dont pass this event to other widgets.
			 */
			if(tmp_widget_ptr != NULL){
				if( EG_Widget_CallSDLEvent(tmp_widget_ptr, event_ptr) == EG_TRUE)
					break;
			}else{
				EG_Log(EG_LOG_ERROR, dL"widget index %d failed (is NULL)"
				 " while sending SDL Event", dR, (int) i);
			}
		}
	}

	/* Restore event to previous value:
	 */
	event_ptr->motion.x += x_displacement;
	event_ptr->motion.y += y_displacement;

	event_ptr->motion.x = (int) ( event_ptr->motion.x * EG_Draw_GetScale() );
	event_ptr->motion.y = (int) ( event_ptr->motion.y * EG_Draw_GetScale() );

	return(EG_TRUE);
}

EG_BOOL EG_Window_SetBackgroundColor(EG_Window *window_ptr, SDL_Color color)
{
	CHECK_EG_WINDOW_IS_VALID(window_ptr, EG_FALSE);
	window_ptr->background_color = color;

	return( EG_Window_RepaintLot(window_ptr) );
}

EG_BOOL EG_Window_Hide(EG_Window *window_ptr)
{
	CHECK_EG_WINDOW_IS_VALID(window_ptr, EG_FALSE);


	//printf("CALLED WINDOW HIDE\n");

	/* Do nothing if already hidden.
	 */
	if (EG_Window_IsHidden(window_ptr) == EG_TRUE)
		return(EG_TRUE);

	/* Set window state to hidden.
	 */
	window_ptr->hidden = EG_TRUE;

	/* Restore SDL Surface background (remove window).
	 */
	if (window_ptr->window_background_surface_ptr != NULL){
		if (SDL_BlitSurface(window_ptr->window_background_surface_ptr
		 , NULL, window_ptr->surface_ptr, NULL) != 0)
			EG_Log(EG_LOG_ERROR, dL"Window failed to re-store"
			 " background", dR);
		else
	        	SDL_UpdateRect(window_ptr->surface_ptr, 0, 0
			 , window_ptr->surface_ptr->w
			 , window_ptr->surface_ptr->h);

		SDL_FreeSurface(window_ptr->window_background_surface_ptr);
		window_ptr->window_background_surface_ptr = NULL;
	}else{
	/* Otherwise clear background.
	 */
		SDL_Rect r = EG_Window_GetDimension(window_ptr);

		SDL_FillRect(window_ptr->surface_ptr, &r
		 , SDL_MapRGB(window_ptr->surface_ptr->format, 0, 0, 0));

		SDL_UpdateRect(window_ptr->surface_ptr, 0, 0
		 , window_ptr->surface_ptr->w
		 , window_ptr->surface_ptr->h);
	}

	return(EG_TRUE);
}

void EG_Window_ClearBackgroundCache(EG_Window *window_ptr)
{
	CHECK_EG_WINDOW_IS_VALID_VOID(window_ptr);

	if (window_ptr->window_background_surface_ptr != NULL){
		SDL_FreeSurface(window_ptr->window_background_surface_ptr);
		window_ptr->window_background_surface_ptr = NULL;
	}
}

EG_BOOL EG_Window_Show(EG_Window *window_ptr)
{
	CHECK_EG_WINDOW_IS_VALID(window_ptr, EG_FALSE);


	//printf("CALLED WINDOW SHOW\n");

	/* Do nothing if already shown.
	 */
	if (EG_Window_IsHidden(window_ptr) != EG_TRUE)
		return(EG_TRUE);

	/* Store background of window area on SDL_Surface
	 */
	if (window_ptr->window_background_surface_ptr != NULL)
		SDL_FreeSurface(window_ptr->window_background_surface_ptr);

	window_ptr->window_background_surface_ptr =
	 SDL_ConvertSurface(window_ptr->surface_ptr
	 , window_ptr->surface_ptr->format, SDL_SWSURFACE);

        if (window_ptr->window_background_surface_ptr == NULL){
		EG_Log(EG_LOG_ERROR, dL"Window failed to store background", dR);
        }       

	/* Set window state to none hidden.
	 */
	window_ptr->hidden = EG_FALSE;
	
	/* If no child widget has focus, then try and
	 * set focus to first child widget that can
	 * get focus.
	 */
	if (EG_Window_Child_GetFocusedWidget(window_ptr) == NULL)
		(void) EG_Window_MoveFocusForward(window_ptr);

	(void) EG_Window_RepaintLot(window_ptr);

	return(EG_TRUE);
}


/* Actions to control the presentation of child widgets:
 */

/* Sets focus to the specified widget (if it can).
 * Warning: Other actions below call this too.
 */
EG_BOOL EG_Window_SetFocusToThisWidget(EG_Widget *widget_ptr)
{
	EG_Widget *focused_widget_ptr = NULL;
	EG_Window *window_ptr = NULL;


	/* If widget is not attached to a window then cannot change focus, so
	 * fail.
	 */
	if ( (window_ptr= (EG_Window*) EG_Widget_GetWindow(widget_ptr)) == NULL )
		return(EG_FALSE);

	/* If another widget on the parent window already has focus, then tell
	 * that widget to loose focus (calling the private clear focus
	 * function).
	 */
	if ( (focused_widget_ptr=EG_Window_Child_GetFocusedWidget(window_ptr))
	 != NULL) EG_Window_ClearFocus(window_ptr);

	/* Try and switch focus to the new widget.
	 */
	if ( EG_Widget_CallGotFocus(widget_ptr) == EG_TRUE ){
		/* If widget excepted focus, then update windows focus index.
		 *
		 * (if this fails then it's a bug..)
		 */
		if (EG_Window_Child_SetFocusedIndexByWidget(window_ptr
		 , widget_ptr) != EG_TRUE)
			EG_Log(EG_LOG_ERROR, dL"Window failed to switch focus"
			 " index to new child widget. This is a bug :-(", dR);
 
		EG_Widget_RepaintLot(widget_ptr);

		/* New widget has excepted focus, so loose focus for old widget
	 	 * and return success.
		 */
		if (focused_widget_ptr != NULL){
			EG_Widget_CallLostFocus(focused_widget_ptr);
			EG_Widget_RepaintLot(focused_widget_ptr);
		}

		return(EG_TRUE);

	/* New widget did not except focus, return failure.
	 */
	}else{

		/* Make sure old widget gets focus again if new widget
		 * did not except it.
		 */
		if (focused_widget_ptr != NULL){
			if (EG_Window_Child_SetFocusedIndexByWidget(window_ptr
        	         , focused_widget_ptr) != EG_TRUE)
                	        EG_Log(EG_LOG_ERROR, dL"Window failed to switch focus"
                        	 " index to new child widget. This is a bug :-(", dR);
		}

		return(EG_FALSE);
	}
}

EG_BOOL EG_Window_StopWidget(EG_Widget *widget_ptr)
{
        EG_Window *window_ptr;

	printf("got in stop 1\n");

        /* If already stopped, do nothing and return true.
         */
        if (EG_Widget_GetStoppedToggle(widget_ptr) == EG_TRUE)
                return(EG_TRUE);

        /* If widget is attached to a window, and has focus on that window, then
         * we must move focus before hiding it.
         */
        if ( (window_ptr=EG_Widget_GetWindow(widget_ptr))
         != NULL ){
                CHECK_EG_WINDOW_IS_VALID(window_ptr, EG_FALSE);

                /* If have focus, then lose focus.  If STILL have
                 * focus, then force loss of focus.
                 */
		if (EG_Window_ThisWidgetHasFocus(widget_ptr) == EG_TRUE)
			EG_Window_ClearFocus(window_ptr);
        }

        /* Call the widgets stopped callback.
         */
	printf("got in stop to calling callback 2\n");
        return( EG_Widget_CallStopped(widget_ptr, EG_TRUE) );
}


/* This is a low level function, users should never call this.  See above
 * stop function for more details.
 *
 * If starting a widget was successful, then focus is NOT changed, it's up to
 * you to handle it, see above stop function for more details.
 */
EG_BOOL EG_Window_StartWidget(EG_Widget *widget_ptr)
{
        /* Do nothing and return true if not stopped.
         */
        if (EG_Widget_GetStoppedToggle(widget_ptr) == EG_FALSE)
                return(EG_TRUE);

	/* Try and start the widget, return result to caller. Does not change
	 * windows focus - and it never should!
	 */
	return(EG_Widget_CallStopped(widget_ptr, EG_FALSE));
}

EG_BOOL EG_Window_ShowWidget(EG_Widget *widget_ptr)
{
	EG_Window *window_ptr;

	/* Do nothing and return true if already enabled.
	 */
	if (EG_Widget_IsVisible(widget_ptr) == EG_TRUE)
		return(EG_TRUE);

	/* Call widget implementations visible callback.  If widget becomes
	 * visible and no other widgets currently have focus on the window, then
	 * this widget should try to take focus.
	 */
	if (EG_Widget_CallVisible(widget_ptr, EG_TRUE) != EG_TRUE)
		return(EG_FALSE);

	/* If not attached to a window then dont bother with focus checks!
 	 */
	if ( (window_ptr=EG_Widget_GetWindow(widget_ptr)) == NULL )
		return(EG_TRUE);
	
	/* Does a widget on the window already have focus?  If not, try
	 * and set this one to it.
	 */
	if (EG_Window_Child_GetFocusedWidget(window_ptr) == NULL)
		EG_Window_SetFocusToThisWidget(widget_ptr);

	/* Return success.
	 */
	return(EG_TRUE);
}

EG_BOOL EG_Window_HideWidget(EG_Widget *widget_ptr)
{
	EG_Window *window_ptr;

	/* If already hidden do nothing and return success.
	 */
	if (EG_Widget_IsVisible(widget_ptr) == EG_FALSE)
		return(EG_TRUE);

	/* Ask widget to change state, if failed return false.
	 */
	if (  EG_Widget_CallVisible(widget_ptr, EG_FALSE) != EG_TRUE )
		return(EG_FALSE);

	/* If not attached to a window, don't worry about focus.
	 */
	if ( (window_ptr=EG_Widget_GetWindow(widget_ptr)) == NULL )
		return(EG_TRUE);

	/* If success and this widget has focus, move focus.
	 */
	if (EG_Window_ThisWidgetHasFocus(widget_ptr) == EG_TRUE){
		EG_Window_MoveFocusForward(window_ptr);
		if (EG_Window_ThisWidgetHasFocus(widget_ptr) == EG_TRUE)
			EG_Window_ClearFocus(window_ptr);
	}
	
	/* Return success.
	 */
	return(EG_TRUE);
}


EG_BOOL EG_Window_EnableWidget(EG_Widget *widget_ptr)
{
	EG_Window *window_ptr;

	/* Do nothing and return true if already enabled.
	 */
	if (EG_Widget_IsEnabled(widget_ptr) == EG_TRUE)
		return(EG_TRUE);

	/* Call widget implementations enable callback.  If widget becomes
	 * enabled and no other widgets currently have focus on the window, then
	 * this widget needs to take focus.
	 */
	if (EG_Widget_CallEnabled(widget_ptr, EG_TRUE) == EG_TRUE){
		/* If not attached to window then dont bother with focus checks!
	 	 */
		if ( (window_ptr= (EG_Window*) EG_Widget_GetWindow(widget_ptr)) == NULL )
			return(EG_TRUE);
	
		/* Does a widget on the window already have focus?  If not, set
		 * this one to it.
		 */
		if (EG_Window_Child_GetFocusedWidget(window_ptr) == NULL)
			EG_Window_SetFocusToThisWidget(widget_ptr);
	}else{
		return(EG_FALSE);
	}
	
	return(EG_TRUE);
}

EG_BOOL EG_Window_DisableWidget(EG_Widget *widget_ptr)
{
	EG_Window *window_ptr;

	/* If already disabled, do nothing and return true.
	 */
	if (EG_Widget_IsEnabled(widget_ptr) == EG_FALSE)
		return(EG_TRUE);
	
	/* Disable the widget.  If disabling the widget succeeded (as not all
	 * widgets have to support this).  Then control focus and repaint the
	 * widget in it's new state.
	 */
	if ( EG_Widget_CallEnabled(widget_ptr, EG_FALSE) == EG_TRUE){
		/* If widget is attached to a window, and has focus on that window, then
		 * we must move focus.
		 */
		if ( (window_ptr= (EG_Window*) EG_Widget_GetWindow(widget_ptr)) != NULL ){
			CHECK_EG_WINDOW_IS_VALID(window_ptr, EG_FALSE);
	
			/* If have focus, then lose focus. 
			 */
			if (EG_Window_ThisWidgetHasFocus(widget_ptr) == EG_TRUE){
				EG_Window_MoveFocusForward(window_ptr);

				/* If still have focus, then force window to clear focus.
			  	 */
				if (EG_Window_ThisWidgetHasFocus(widget_ptr) == EG_TRUE)
					EG_Window_ClearFocus(window_ptr);		
        		}
		}
		return(EG_TRUE);
	}else{
		return(EG_FALSE);
	}
}

/* Test whether a widget has focus on the window.
 */
EG_BOOL EG_Window_ThisWidgetHasFocus(EG_Widget *widget_ptr)
{
	EG_Window *window_ptr;
	EG_Widget *focused_widget_ptr;
	/* Widget must be attached to a window to get focus.
	 */
	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	if ( (window_ptr= (EG_Window*) EG_Widget_GetWindow(widget_ptr) ) == NULL)
		return(EG_FALSE);

	/* Get windows focused widget, if none exit with false.
	 */
	if ( (focused_widget_ptr=EG_Window_Child_GetFocusedWidget(window_ptr) ) == NULL)
		return(EG_FALSE);

	/* Compare widgets, if the same then return true.
	 */
	if ( EG_Widget_GetID(widget_ptr) == EG_Widget_GetID(focused_widget_ptr) )
		return(EG_TRUE);
	else
		return(EG_FALSE);
}
