/* START OF slidebar.c -------------------------------------------------------
 *
 *	A slide bar widget.
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

#include <gui/slidebar.h>
#include <gui/slidebar_private.h>

#include <gui/window.h>
#include <gui/window_private.h>

#include <gui/widget.h>
#include <gui/widget_private.h>

#include <gui/widget_shared.h>

#include <gui/sdl.h>

#include <SDL.h>

#define BUTTON_WIDTH	16
#define BUTTON_HEIGHT	16


/* Private functions:
 */
static void 	Calc_Slider_Area(EG_Widget *widget_ptr);
static long 	CalcVisibleLength(EG_Widget *widget_ptr);
static float 	GetPositionAsFloat(EG_Widget *widget_ptr);
static void 	Scroll(EG_Widget *widget_ptr, long amount);
static void 	Calc_Background_Area(EG_Widget *widget_ptr);
static void 	Move_Slider(EG_Widget *widget_ptr);


/* Calculates the visible length of the scrollbar.
 */
static long CalcVisibleLength(EG_Widget *widget_ptr)
{
	EG_SlideBar *slidebar_ptr;
	SDL_Rect area; 

	EG_SLIDEBAR_GET_STRUCT_PTR(widget_ptr, slidebar_ptr, 0);

	area = EG_Widget_GetDimension(widget_ptr);

	switch(slidebar_ptr->orientation){
        case EG_SlideBar_Vertical:
		return area.h;
        	break;

        case EG_SlideBar_Horizontal:
		return area.w;
		break;
	}

	return 0;
}


/* Returns the position of the scroll bar as a float between 0 and 1
 */
static float GetPositionAsFloat(EG_Widget *widget_ptr)
{
	EG_SlideBar *slidebar_ptr;
	EG_SLIDEBAR_GET_STRUCT_PTR(widget_ptr, slidebar_ptr, 0.0);

	return (float) slidebar_ptr->position / (float) ( 
	 slidebar_ptr->virtual_length - CalcVisibleLength(widget_ptr) );
}


/* Scroll scroll bar by amount
 */
static void Scroll(EG_Widget *widget_ptr, long amount)
{
	EG_SlideBar *slidebar_ptr;
	EG_SLIDEBAR_GET_STRUCT_PTR_VOID(widget_ptr, slidebar_ptr);

	slidebar_ptr->position += amount;

	if ( slidebar_ptr->position > slidebar_ptr->virtual_length - CalcVisibleLength(widget_ptr) )
		slidebar_ptr->position = slidebar_ptr->virtual_length - CalcVisibleLength(widget_ptr);

	if ( slidebar_ptr->position < 0 )
		slidebar_ptr->position = 0;

	Calc_Background_Area(widget_ptr);
	Calc_Slider_Area(widget_ptr);
	EG_Widget_RepaintLot(widget_ptr);


	slidebar_ptr->last_called = EG_Draw_GetCurrentTime();					

#ifdef WITHOUT_REALTIME_SLIDER
	EG_Widget_CallUserOnChange(widget_ptr);
#endif
}


/* Calculates the area of the scroll bar troff.
 */
static void Calc_Background_Area(EG_Widget *widget_ptr)
{
	EG_SlideBar *slidebar_ptr;

	/* gET Slidebar (quit on error)
	 */
	EG_SLIDEBAR_GET_STRUCT_PTR_VOID(widget_ptr, slidebar_ptr);

	/* Set slidebar background area.
	 */
        slidebar_ptr->background_area = EG_Widget_GetDimension(widget_ptr);

        switch(slidebar_ptr->orientation){
        case EG_SlideBar_Vertical:
                slidebar_ptr->background_area.y +=BUTTON_HEIGHT;
		slidebar_ptr->background_area.h -=(BUTTON_HEIGHT*2);
                break;

        case EG_SlideBar_Horizontal:
                slidebar_ptr->background_area.x +=BUTTON_WIDTH;
		slidebar_ptr->background_area.w -=(BUTTON_WIDTH*2);
                break;
        }
}


/* Calculates the area of the slider.
 */
static void Calc_Slider_Area(EG_Widget *widget_ptr)
{
	EG_SlideBar *slidebar_ptr;

	Calc_Background_Area(widget_ptr);

	/* Get slidebar (quit on error)
	 */
	EG_SLIDEBAR_GET_STRUCT_PTR_VOID(widget_ptr, slidebar_ptr);

	/* Calc. max size.
	 */
	Calc_Background_Area(widget_ptr);
	slidebar_ptr->slider_area = slidebar_ptr->background_area;

	/* Short circuit if visible length and virtual length are
	 * the same (will use whole troff area).
	 */
	if ( slidebar_ptr->virtual_length == CalcVisibleLength(widget_ptr) )
		return;

	/* Calc. actual size.
	 */
	switch (slidebar_ptr->orientation){
	case EG_SlideBar_Vertical:

		/* Calc length of slider.
		 */
		slidebar_ptr->slider_area.h = (float) slidebar_ptr->slider_area.h
		 * ( (float) CalcVisibleLength(widget_ptr)
		     / (float) slidebar_ptr->virtual_length
		   );
		if (slidebar_ptr->slider_area.h < 30)
			slidebar_ptr->slider_area.h = 30;

		/* Calc position of slider.
		 */
		slidebar_ptr->slider_area.y += (slidebar_ptr->background_area.h
		 - slidebar_ptr->slider_area.h) * GetPositionAsFloat(widget_ptr);
		break;

	case EG_SlideBar_Horizontal:

		/* Calc length of slider.
		 */
		slidebar_ptr->slider_area.w = (float) slidebar_ptr->slider_area.w
		 * ( (float) CalcVisibleLength(widget_ptr)
		     / (float) slidebar_ptr->virtual_length
		   );
		if (slidebar_ptr->slider_area.w < 30)
			slidebar_ptr->slider_area.w = 30;

		/* Calc position of slider.
		 */
		slidebar_ptr->slider_area.x += (slidebar_ptr->background_area.w
		 - slidebar_ptr->slider_area.w) * GetPositionAsFloat(widget_ptr);
		break;
	}
}


/* Convert visible slider graphic into the slider location variables
 */
static void Move_Slider(EG_Widget *widget_ptr)
{
	EG_SlideBar *slidebar_ptr;
	float new_slider_position, scrollable_area_length;

	/* Get slidebar (quit on error)
	 */
	EG_SLIDEBAR_GET_STRUCT_PTR_VOID(widget_ptr, slidebar_ptr);

        /* Short circuit if visible length and virtual length are
         * the same.
         */
        if ( slidebar_ptr->virtual_length == CalcVisibleLength(widget_ptr) )
                return;

	switch (slidebar_ptr->orientation){
	case EG_SlideBar_Vertical:

		/* Calc scrollable area within troff.
		 */
		scrollable_area_length = slidebar_ptr->background_area.h - slidebar_ptr->slider_area.h;

		/* Calc movement of slidebar within troff.
		 */
		new_slider_position = (float) (slidebar_ptr->slider_area.y
		  - slidebar_ptr->background_area.y) + slidebar_ptr->mouse_y;

		/* Clip it.
		 */
		if (new_slider_position < 0)
			new_slider_position = 0;
		if (new_slider_position > scrollable_area_length)
			new_slider_position = scrollable_area_length;

		/* Set position to it.
		 */
		slidebar_ptr->position = (
		 ((float) (slidebar_ptr->virtual_length - CalcVisibleLength(widget_ptr))) 
		 * ((float) new_slider_position / (float) scrollable_area_length) );
		
		slidebar_ptr->slider_area.y = new_slider_position + slidebar_ptr->background_area.y;
		break;

	case EG_SlideBar_Horizontal:
		/* Calc scrollable area within troff.
		 */
		scrollable_area_length = slidebar_ptr->background_area.w - slidebar_ptr->slider_area.w;

		/* Calc movement of slidebar within troff.
		 */
		new_slider_position = (float) (slidebar_ptr->slider_area.x
		  - slidebar_ptr->background_area.x) + slidebar_ptr->mouse_x;

		/* Clip it.
		 */
		if (new_slider_position < 0)
			new_slider_position = 0;
		if (new_slider_position > scrollable_area_length)
			new_slider_position = scrollable_area_length;

		/* Set position to it.
		 */
		slidebar_ptr->position = (
		 ((float) (slidebar_ptr->virtual_length - CalcVisibleLength(widget_ptr))) 
		 * ((float) new_slider_position / (float) scrollable_area_length) );
		
		slidebar_ptr->slider_area.x = new_slider_position + slidebar_ptr->background_area.x;
		break;
	}

	slidebar_ptr->mouse_x = 0;
	slidebar_ptr->mouse_y = 0;

	EG_Widget_RepaintLot(widget_ptr);
}


/* Event processing callback:
 */
#define OVER_NOTHING 				0
#define OVER_BUTTON1 				1
#define OVER_TROFF 				2
#define OVER_SLIDER				3
#define OVER_BUTTON2				4
#define OVER_TROFF_AND_ABOVE_SLIDER		5
#define OVER_TROFF_AND_BELOW_SLIDER		6
static EG_BOOL Callback_SDL_Event(EG_Widget *widget_ptr, SDL_Event *event_ptr)
{
        EG_SlideBar *slidebar_ptr;
        EG_Window *window_ptr;
        EG_BOOL mouse_over_widget;
	int mouse_is_over;
        EG_BOOL return_value = EG_FALSE;

        /* Populates variables needed to process the event.  If the event
         * shouldn't have been passed to us, then logs the fact (for bug
         * reporting) and returns false.  Quit the callback if this returns
         * false.
         */
        if (EG_Shared_GetEventDetails(widget_ptr, event_ptr
         , (void*) &slidebar_ptr, &window_ptr, &mouse_over_widget) != EG_TRUE)
                return(return_value);

        /* Process User Events:
         */

	if (event_ptr->type == SDL_USEREVENT){

		/* Is this the 10ms poll event?
	 	 */
		if (event_ptr->user.code == EG_USER_SDL_EVENT_POLL){

#ifndef WITHOUT_REALTIME_SLIDER
			if (slidebar_ptr->previous_position != slidebar_ptr->position){
//				if ( EG_Draw_GetTimePassed(slidebar_ptr->last_called) >= 100){
					EG_Widget_CallUserOnChange(widget_ptr);
					slidebar_ptr->previous_position = slidebar_ptr->position;
//					slidebar_ptr->last_called = EG_Draw_GetCurrentTime();
//				}
			}
#endif


			if ( EG_Draw_GetTimePassed(slidebar_ptr->last_called) >= 100){
				if (slidebar_ptr->button1_depressed == EG_TRUE)
					Scroll(widget_ptr, 0.0 - slidebar_ptr->step);
				if (slidebar_ptr->button2_depressed == EG_TRUE)
					Scroll(widget_ptr, slidebar_ptr->step);
			}
		}

		return EG_TRUE;
	}
	
        /* Process SDL Events:
         */

	/* Determine which part of the slide bar widget the mouse is over (if any).
	 */
	mouse_is_over = OVER_NOTHING;
	if (EG_Shared_IsMouseOverArea(widget_ptr, slidebar_ptr->button1_area, event_ptr) == EG_TRUE)
		mouse_is_over = OVER_BUTTON1;
	if (EG_Shared_IsMouseOverArea(widget_ptr, slidebar_ptr->background_area, event_ptr) == EG_TRUE)
		mouse_is_over = OVER_TROFF;
	if (EG_Shared_IsMouseOverArea(widget_ptr, slidebar_ptr->slider_area, event_ptr) == EG_TRUE)
		mouse_is_over = OVER_SLIDER;
	if (EG_Shared_IsMouseOverArea(widget_ptr, slidebar_ptr->button2_area, event_ptr) == EG_TRUE)
		mouse_is_over = OVER_BUTTON2;
	if (mouse_is_over == OVER_TROFF){
		/* If the mouse pointer is over the troff (background) of the
		 * widget then determine whether the mouse pointer is above
		 * or below the slider.
		 */
		switch (slidebar_ptr->orientation){
		case EG_SlideBar_Vertical:
			if (event_ptr->motion.y < slidebar_ptr->slider_area.y
			 + EG_Window_GetYDisplacement(window_ptr) )
				mouse_is_over = OVER_TROFF_AND_ABOVE_SLIDER;
			else
				mouse_is_over = OVER_TROFF_AND_BELOW_SLIDER;
	                break;
	
		case EG_SlideBar_Horizontal:
			if (event_ptr->motion.x < slidebar_ptr->slider_area.x
			 + EG_Window_GetXDisplacement(window_ptr) )
				mouse_is_over = OVER_TROFF_AND_ABOVE_SLIDER;
			else
				mouse_is_over = OVER_TROFF_AND_BELOW_SLIDER;
			break;
		}
	}


	/* Handle scrolling with buttons 1 and 2:
	 */

	/* Is this a mouse event?
	 * Yes,
	 */
	if (event_ptr->type == SDL_MOUSEBUTTONDOWN 
	 || event_ptr->type == SDL_MOUSEBUTTONUP){
		/* Is mouse button 1 physically depressed?
		 * Yes,
		 */
		if (event_ptr->button.state == SDL_PRESSED
		 && event_ptr->button.button == SDL_BUTTON_LEFT){

 			/* Is mouse pointer over button1 area?
			 * Yes, does 'button1_depressed' equal false?
			 */
			if (mouse_is_over == OVER_BUTTON1 
			 && slidebar_ptr->button1_depressed == EG_FALSE){
 				/* 	Yes, set 'button1_depressed' to true, scroll by step amount, 
 				 *	repaint widget.
 				 */
				slidebar_ptr->button1_depressed = EG_TRUE;
				(void)EG_Window_SetFocusToThisWidget(widget_ptr);
				Scroll(widget_ptr, 0.0 - slidebar_ptr->step);
			}

			/* Is mouse pointer over button2 area?
	 		 * Yes, does button2_depressed equal false?
			 */
			if (mouse_is_over == OVER_BUTTON2
			 && slidebar_ptr->button2_depressed == EG_FALSE){
	
		 		/*	Yes, set 'button2_depressed' to true, scroll by minus step amount,
		 		 *	repaint widget.
		 		 */
				slidebar_ptr->button2_depressed = EG_TRUE;
				(void)EG_Window_SetFocusToThisWidget(widget_ptr);
				Scroll(widget_ptr, slidebar_ptr->step);
			}

		/* No,
		 */
		}else{

			/* Does 'button1_depressed' equal true?
		 	 *	Yes, set 'button1_depressed' to false, repaint widget.
		 	 */
			if (slidebar_ptr->button1_depressed == EG_TRUE){
				slidebar_ptr->button1_depressed = EG_FALSE;
				EG_Widget_RepaintLot(widget_ptr);
			}

			/* Does 'button2_depressed' equal true?
	 		 *	Yes, set 'button2_depressed' to false, repaint widget.
	 		 */
			if (slidebar_ptr->button2_depressed == EG_TRUE){
				slidebar_ptr->button2_depressed = EG_FALSE;
				EG_Widget_RepaintLot(widget_ptr);
			}
		}
	}


	/* [TODO] Add support to this for the forward/backward keys too.
	 *
	 * Is this the 'time up' timer event?
	 *	Yes,
	 */

		/* Is 'button1_depressed' equal true?
		 *	Yes, scroll by step amount.
		 */

		/* Is 'button2_depressed' equal true?
		 *	Yes, scroll by minus step amount.
		 */


	/* Is this a mouse event?
	 */
	if (event_ptr->type == SDL_MOUSEBUTTONDOWN
	 || event_ptr->type == SDL_MOUSEBUTTONUP){
		/* Yes, Is mouse button 1 physically depressed?
		 */
		if (event_ptr->button.state == SDL_PRESSED
		 && event_ptr->button.button == SDL_BUTTON_LEFT){
			/* Yes,
			 */
			
			/* Is the slider button not depressed and
		 	 * the mouse pointer within the slider area?
			 */
			if (slidebar_ptr->slider_depressed == EG_FALSE
			 && mouse_is_over == OVER_SLIDER){
			 	/* Yes, set 'slider_depressed' to true
				 * and clear movement.
			 	 */
				slidebar_ptr->slider_depressed = EG_TRUE;
				slidebar_ptr->mouse_x = 0;
				slidebar_ptr->mouse_y = 0;
#ifdef WITHOUT_REALTIME_SLIDER
				slidebar_ptr->slider_area_previous = slidebar_ptr->slider_area;
#endif
				(void)EG_Window_SetFocusToThisWidget(widget_ptr);
				EG_Widget_RepaintLot(widget_ptr);
			}

			/* Was the mouse button pressed while on troff up/down?
			 * Yes, page up/page down.
			 */
			if (event_ptr->type == SDL_MOUSEBUTTONDOWN){
				if (mouse_is_over == OVER_TROFF_AND_BELOW_SLIDER){
					(void)EG_Window_SetFocusToThisWidget(widget_ptr);
					Scroll(widget_ptr, CalcVisibleLength(widget_ptr));
				}

				if (mouse_is_over == OVER_TROFF_AND_ABOVE_SLIDER){
					(void)EG_Window_SetFocusToThisWidget(widget_ptr);
					Scroll(widget_ptr, 0 - CalcVisibleLength(widget_ptr));
				}
			}
		}else{
			/* No, is 'slider_depressed' equal to true?
			 */
			if (slidebar_ptr->slider_depressed == EG_TRUE){
				/* Yes, set 'slider_depressed' to false.
				 */
				slidebar_ptr->slider_depressed = EG_FALSE;
				EG_Widget_RepaintLot(widget_ptr);
#ifdef WITHOUT_REALTIME_SLIDER
				EG_Widget_CallUserOnChange(widget_ptr);
#endif
			}
		}
	}


//[HERE]

	/* Did the mouse move and is the slider button depressed?
	 */
	if (event_ptr->type == SDL_MOUSEMOTION
	 && slidebar_ptr->slider_depressed == EG_TRUE){
		/* Yes, calc. amount, clip it and update slider position.
		 */
		switch (slidebar_ptr->orientation){
			case EG_SlideBar_Vertical:
			slidebar_ptr->mouse_y += event_ptr->motion.yrel* (1/EG_Draw_GetScale());
                	break;

			case EG_SlideBar_Horizontal:
			slidebar_ptr->mouse_x += event_ptr->motion.xrel* (1/EG_Draw_GetScale());
			break;
		}

		Move_Slider(widget_ptr);
		EG_Widget_RepaintLot(widget_ptr);
	}


	/* [TODO] If have focus and 'Forward' or 'Backward' key pressed then process it as if button1/2
	 * were pressed.
	 */

        return(return_value);
}


/* Callbacks:
 */

/* Paint the widget
 */
static EG_BOOL Callback_Paint(EG_Widget *widget_ptr, SDL_Rect area)
{
	EG_SlideBar *slidebar_ptr;
        EG_Window *window_ptr;
	SDL_Rect tmp;
        SDL_Rect loc;
        SDL_Color color;
        SDL_Surface *surface_ptr = NULL;
	EG_BOOL bold;
	int style;
	SDL_Color color_bg;

        if ( EG_Shared_GetRenderingDetails(widget_ptr, area,
         (void*) &slidebar_ptr, &window_ptr, &surface_ptr, &color, &loc) != EG_TRUE )
		return(EG_TRUE);

#ifdef EG_DEBUG
        printf("SOMEONE CALLED PAINT FOR '%s' [repaint area (window)="
         "{%d, %d, %d, %d}:widget area (SDL_Surface)={%d, %d, %d, %d}]\n"
         , EG_Widget_GetName(widget_ptr), area.x, area.y, area.w, area.h
         , loc.x, loc.y, loc.w, loc.h);
#endif

	EG_Draw_Disable_Update();

        if (EG_Window_ThisWidgetHasFocus(widget_ptr) == EG_TRUE)
                bold = EG_TRUE;
        else
                bold = EG_FALSE;

	/* Draw the troff background:
	 */
	color_bg.r = (int) ( color.r * 0.7);
	color_bg.g = (int) ( color.g * 0.7);
	color_bg.b = (int) ( color.b * 0.7);
	color_bg.unused = 0;

	/* If disabled render everything in troff color
	 */
	if ( EG_Widget_IsEnabled(widget_ptr) == EG_FALSE)
		color = color_bg;

	/* Draw button1.
	 */
	tmp = slidebar_ptr->button1_area;
	tmp.x += EG_Window_GetXDisplacement( window_ptr );
	tmp.y += EG_Window_GetYDisplacement( window_ptr );

	if (slidebar_ptr->button1_depressed == EG_TRUE)
		style = EG_Draw_Border_BoxLow;
	else
		style = EG_Draw_Border_BoxHigh;

//	EG_Draw_Box(surface_ptr, &tmp, &color_bg);
//	tmp.x++; tmp.y++; tmp.w-=2; tmp.h--;
	EG_Draw_Box(surface_ptr, &tmp, &color);
	EG_Draw_Border(surface_ptr,&tmp, &color, style);
	EG_Draw_String(surface_ptr, &color, bold, &tmp, 0, slidebar_ptr->button1_label );

	/* Draw troff
	 */
	tmp = slidebar_ptr->background_area;
	tmp.x += EG_Window_GetXDisplacement( window_ptr );
	tmp.y += EG_Window_GetYDisplacement( window_ptr );

	EG_Draw_Box(surface_ptr, &tmp, &color_bg);

	/* Draw previous position
	 */
#ifdef WITHOUT_REALTIME_SLIDER
	if (slidebar_ptr->slider_depressed == EG_TRUE){
		tmp = slidebar_ptr->slider_area_previous;
		tmp.x += EG_Window_GetXDisplacement( window_ptr );
		tmp.y += EG_Window_GetYDisplacement( window_ptr );

		SDL_Color tmp_col = (SDL_Color) {0,0,0,0};
		EG_Draw_Box(surface_ptr, &tmp, &tmp_col);
	}
#endif

	/* Draw slide bar
	 */
	tmp = slidebar_ptr->slider_area;

        tmp.x += EG_Window_GetXDisplacement( window_ptr );
        tmp.y += EG_Window_GetYDisplacement( window_ptr );

//	if (slidebar_ptr->slider_depressed == EG_TRUE)
//		style = EG_Draw_Border_BoxLow;
//	else
		style = EG_Draw_Border_BoxHigh;

//	tmp.x++; tmp.w-=2;
//	tmp.y++; tmp.h-=2;
	EG_Draw_Box(surface_ptr, &tmp, &color);
	EG_Draw_Border(surface_ptr,&tmp, &color, style);
	EG_Draw_String(surface_ptr, &color, bold, &tmp, 0, slidebar_ptr->slider_label);

	/* Draw button2.
	 */
	tmp = slidebar_ptr->button2_area;
	tmp.x += EG_Window_GetXDisplacement( window_ptr );
	tmp.y += EG_Window_GetYDisplacement( window_ptr );

	if (slidebar_ptr->button2_depressed == EG_TRUE)
		style = EG_Draw_Border_BoxLow;
	else
		style = EG_Draw_Border_BoxHigh;

//	EG_Draw_Box(surface_ptr, &tmp, &color_bg);
//	tmp.x++; tmp.w-=2; tmp.h--;
	EG_Draw_Box(surface_ptr, &tmp, &color);
	EG_Draw_Border(surface_ptr,&tmp, &color, style);
	EG_Draw_String(surface_ptr, &color, bold, &tmp, 0, slidebar_ptr->button2_label );

	EG_Draw_Enable_Update();

	/* Update the widget on the SDL surface.  We do this all in
	 * one go last to reduce flicker.
	 */
	area.x += EG_Window_GetXDisplacement( window_ptr );
	area.y += EG_Window_GetYDisplacement( window_ptr );
	EG_Draw_UpdateSurface(surface_ptr, area);

	/* Return success - was painted.
	 */
	return(EG_TRUE);
}


/* Private functions:
 */

static void InitializePayload(EG_SlideBar *slidebar_ptr, SDL_Color color, int orientation, SDL_Rect area)
{
	switch(orientation){
		case EG_SlideBar_Vertical:

		slidebar_ptr->button1_label[0] = 1;
		slidebar_ptr->button1_label[1] = 0;

		slidebar_ptr->button2_label[0] = 2;
		slidebar_ptr->button2_label[1] = 0;
		break;

		case EG_SlideBar_Horizontal:

		slidebar_ptr->button1_label[0] = 4;
		slidebar_ptr->button1_label[1] = 0;

		slidebar_ptr->button2_label[0] = 3;
		slidebar_ptr->button2_label[1] = 0;
		break;
	}

	slidebar_ptr->slider_label[0] = 9;
	slidebar_ptr->slider_label[1] = 0;

	slidebar_ptr->orientation = orientation;
	slidebar_ptr->position = 0;
	slidebar_ptr->previous_position = 0;

	slidebar_ptr->mouse_x = 0;
	slidebar_ptr->mouse_y = 0;

	slidebar_ptr->button1_depressed = EG_FALSE;
	slidebar_ptr->button2_depressed = EG_FALSE;
	slidebar_ptr->slider_depressed = EG_FALSE;

	slidebar_ptr->last_called = 0;

#ifdef WITHOUT_REALTIME_SLIDER
	slidebar_ptr->slider_area_previous = (SDL_Rect) {0,0,0,0};
#endif
}

static void InitializeWidget(EG_Widget *widget_ptr, SDL_Color color, SDL_Rect area)
{
	/* Initialize callbacks:
	 */
	(void) EG_Widget_SetCallback_Destroy(widget_ptr
	 , EG_Callback_Generic_Destroy);

        (void) EG_Widget_SetCallback_Paint(widget_ptr
	 , Callback_Paint);

        (void) EG_Widget_SetCallback_SDL_Event(widget_ptr
	 , Callback_SDL_Event);

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
	(void) EG_Widget_SetCanGetFocusToggle(widget_ptr, EG_FALSE);
	(void) EG_Widget_SetDimension(widget_ptr, area);
	(void) EG_Widget_SetBackgroundColor(widget_ptr, color);

	/* Initialize internal variables:
	 */
	EG_SlideBar_Resize(widget_ptr, area);

	EG_SlideBar_SetSnap(widget_ptr, EG_FALSE);
	EG_SlideBar_SetLength(widget_ptr, CalcVisibleLength(widget_ptr));
	EG_SlideBar_SetStep(widget_ptr, 1);
}

/* Public functions:
 */

EG_Widget* EG_SlideBar_Create(const char *name_ptr, SDL_Color color, int orientation, SDL_Rect area)
{
        EG_SlideBar *slidebar_ptr;
        EG_Widget *widget_ptr;
	void *ptr;

        SHARED__ALLOC_PAYLOAD_STRUCT(ptr, EG_SlideBar
         , "Unable to malloc EG_SlideBar struct");
	slidebar_ptr = (EG_SlideBar*) ptr;

        SHARED__CREATE_NEW_EG_WIDGET(widget_ptr, name_ptr
	 , EG_Widget_Type_SlideBar, slidebar_ptr);

        SHARED__ATTACH_PAYLOAD_TO_WIDGET(widget_ptr, slidebar_ptr);

	InitializePayload(slidebar_ptr, color, orientation, area);
	InitializeWidget(widget_ptr, color, area);

        return widget_ptr;
}


/* Set the visible length of the scrolling area (and also how fat the widget is).
 */
EG_BOOL EG_SlideBar_Resize(EG_Widget *widget_ptr, SDL_Rect area)
{
        EG_SlideBar *slidebar_ptr;
        EG_SLIDEBAR_GET_STRUCT_PTR(widget_ptr, slidebar_ptr, EG_FALSE);

	/* Change button size + positions
	 */
	switch(slidebar_ptr->orientation){
		case EG_SlideBar_Vertical:

		slidebar_ptr->button1_area.x = area.x;
		slidebar_ptr->button1_area.y = area.y;
		slidebar_ptr->button1_area.w = area.w;
		slidebar_ptr->button1_area.h = BUTTON_HEIGHT;

		slidebar_ptr->button2_area.x = area.x;
		slidebar_ptr->button2_area.y = area.y + area.h - BUTTON_HEIGHT;
		slidebar_ptr->button2_area.w = area.w;
		slidebar_ptr->button2_area.h = BUTTON_HEIGHT;
		break;

		case EG_SlideBar_Horizontal:

		slidebar_ptr->button1_area.x = area.x;
		slidebar_ptr->button1_area.y = area.y;
		slidebar_ptr->button1_area.w = BUTTON_WIDTH;
		slidebar_ptr->button1_area.h = area.h;

		slidebar_ptr->button2_area.x = area.x + area.w - BUTTON_WIDTH;
		slidebar_ptr->button2_area.y = area.y;
		slidebar_ptr->button2_area.w = BUTTON_WIDTH;
		slidebar_ptr->button2_area.h = area.h;
		break;
	}

	/* Change widget size.
	 */
	(void) EG_Widget_SetDimension(widget_ptr, area);


	/* Make sure position is still valid, truncate if not
	 */
        if (slidebar_ptr->position > slidebar_ptr->virtual_length
         - CalcVisibleLength(widget_ptr) )
                slidebar_ptr->position = slidebar_ptr->virtual_length
                 - CalcVisibleLength(widget_ptr);

	/* Recalc scroll bar variables
	 */
	Calc_Background_Area(widget_ptr);
	Calc_Slider_Area(widget_ptr);

	(void) EG_Widget_RepaintLot(widget_ptr);

	return EG_TRUE;
}


/* Set the virtual length of the scrolling area. If the new length is less than the visible length, then
 * the virtual length is set to the visible length instead.  So length=0 will reset the scroll bar to a 1:1
 * ratio.
 */
EG_BOOL EG_SlideBar_SetLength(EG_Widget *widget_ptr, long length)
{
	EG_SlideBar *slidebar_ptr;

	EG_SLIDEBAR_GET_STRUCT_PTR(widget_ptr, slidebar_ptr, EG_FALSE);

	if ( length < CalcVisibleLength(widget_ptr) )
		slidebar_ptr->virtual_length = CalcVisibleLength(widget_ptr);
	else
		slidebar_ptr->virtual_length = length;

	if (slidebar_ptr->position > slidebar_ptr->virtual_length - CalcVisibleLength(widget_ptr) ){
		slidebar_ptr->position = slidebar_ptr->virtual_length - CalcVisibleLength(widget_ptr);
		slidebar_ptr->previous_position = slidebar_ptr->position;
		
		slidebar_ptr->previous_position = slidebar_ptr->position;
		EG_Widget_CallUserOnChange(widget_ptr);
	}

	/* Update variables and repaint widget
	 */
	Calc_Background_Area(widget_ptr);
	Calc_Slider_Area(widget_ptr);
	EG_Widget_RepaintLot(widget_ptr);

	return EG_TRUE;
}

/* Get length.
 */
long EG_SlideBar_GetLength(EG_Widget *widget_ptr)
{
	EG_SlideBar *slidebar_ptr;

	EG_SLIDEBAR_GET_STRUCT_PTR(widget_ptr, slidebar_ptr, 0);

	return slidebar_ptr->virtual_length;
}



/* position is 0 to length - widget_length (if horizontal this is width, if vertical this is height).
 */
EG_BOOL EG_SlideBar_SetPosition(EG_Widget *widget_ptr, long position)
{
	EG_SlideBar *slidebar_ptr;

	EG_SLIDEBAR_GET_STRUCT_PTR(widget_ptr, slidebar_ptr, EG_FALSE);

	if (position<0)
		position = 0;

	if (position > slidebar_ptr->virtual_length 
	 - CalcVisibleLength(widget_ptr) )
		position = slidebar_ptr->virtual_length
	 	 - CalcVisibleLength(widget_ptr);

	slidebar_ptr->position = position;
	slidebar_ptr->previous_position = position;

	Calc_Background_Area(widget_ptr);
	Calc_Slider_Area(widget_ptr);
	EG_Widget_RepaintLot(widget_ptr);

	return EG_TRUE;
}


/* get the start position within the virtual scrolling area.  Read this in an on-change event.
 */
long EG_SlideBar_GetPosition(EG_Widget *widget_ptr)
{
	long position;
	EG_SlideBar *slidebar_ptr;

	EG_SLIDEBAR_GET_STRUCT_PTR(widget_ptr, slidebar_ptr, 0);

	position = slidebar_ptr->position;

	if (slidebar_ptr->snap == EG_TRUE)
		position = position / slidebar_ptr->step
		 * slidebar_ptr->step;

	return position;
}

/* Sets the stepping value in pixels, button1 and button2 will scroll by this amount.
 */
EG_BOOL EG_SlideBar_SetStep(EG_Widget *widget_ptr, long step)
{
	EG_SlideBar *slidebar_ptr;

	EG_SLIDEBAR_GET_STRUCT_PTR(widget_ptr, slidebar_ptr, EG_FALSE);
	slidebar_ptr->step = step;
	return EG_TRUE;
}

/* Returns the step (see above)
 */
long EG_SlideBar_GetStep(EG_Widget *widget_ptr)
{
	EG_SlideBar *slidebar_ptr;

	EG_SLIDEBAR_GET_STRUCT_PTR(widget_ptr, slidebar_ptr, 0);
	return slidebar_ptr->step;
}

/* Set snap
 */
EG_BOOL EG_SlideBar_SetSnap(EG_Widget *widget_ptr, EG_BOOL snap)
{
	EG_SlideBar *slidebar_ptr;

	EG_SLIDEBAR_GET_STRUCT_PTR(widget_ptr, slidebar_ptr, EG_FALSE);
	slidebar_ptr->snap = snap;
	return EG_TRUE;
}

/* Get snap.
 */
EG_BOOL EG_SlideBar_GetSnap(EG_Widget *widget_ptr)
{
	EG_SlideBar *slidebar_ptr;

	EG_SLIDEBAR_GET_STRUCT_PTR(widget_ptr, slidebar_ptr, EG_FALSE);
	return slidebar_ptr->snap;
}
