/* START OF progressbar.c --------------------------------------------------------------
 *
 * 	A simple progress bar widget.
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

#include <gui/progressbar.h>
#include <gui/progressbar_private.h>

#include <gui/widget_shared.h>

#include <gui/window.h>
#include <gui/window_private.h>

#include <gui/widget.h>
#include <gui/widget_private.h>


#include <SDL.h>


/* Callbacks:
 */

static EG_BOOL Callback_SDL_Event(EG_Widget *widget_ptr, SDL_Event *event_ptr)
{
        EG_ProgressBar *progressbar_ptr;

        EG_PROGRESSBAR_GET_STRUCT_PTR(widget_ptr, progressbar_ptr, EG_FALSE);

	/* Process User Events:
	 */

	if (event_ptr->type == SDL_USEREVENT){

		/* Is this the 10ms poll event?
		 */
		if (event_ptr->user.code == EG_USER_SDL_EVENT_POLL){

			/* Approx. every half second render.
			 */
			if ( EG_Draw_GetTimePassed(progressbar_ptr->last_called) >= 100){
				EG_Widget_RepaintLot(widget_ptr);
				progressbar_ptr->last_called = EG_Draw_GetCurrentTime();
			}
		}
	}

	return(EG_FALSE);
}

static EG_BOOL Callback_Paint(EG_Widget *widget_ptr, SDL_Rect area)
{
	EG_ProgressBar *progressbar_ptr;
	EG_Window *window_ptr;

	SDL_Rect loc, tmp;

	SDL_Color color;
	SDL_Surface *surface_ptr;

	if ( EG_Shared_GetRenderingDetails(widget_ptr, area, 
	 (void*) &progressbar_ptr, &window_ptr, &surface_ptr, &color, &loc) != EG_TRUE )
		return(EG_TRUE);

//#ifdef EG_DEBUG
//	printf("SOMEONE CALLED PAINT FOR '%s' [repaint area (window)="
//	 "{%d, %d, %d, %d}:widget area (SDL_Surface)={%d, %d, %d, %d}]\n"
//	 , EG_Widget_GetName(widget_ptr), area.x, area.y, area.w, area.h
//	 , loc.x, loc.y, loc.w, loc.h);
//#endif

	/* Render the background.
	 */
	EG_Draw_Disable_Update();

	if (progressbar_ptr->unknown_position == EG_FALSE){

//		if (progressbar_ptr->position != progressbar_ptr->previous_position){		
//			progressbar_ptr->previous_position = progressbar_ptr->position;
			tmp.x = loc.x + 2;
			tmp.y = loc.y + 2;
			tmp.w = ( (loc.w - 4) * progressbar_ptr->position);
			tmp.h = loc.h - 4;

//		if (progressbar_ptr->previous_w != tmp.w || progressbar_ptr->previous_w <= 0){
//			progressbar_ptr->previous_w = tmp.w;

//			printf("---> %ld\n", progressbar_ptr->previous_w);

			EG_Draw_Box(surface_ptr, &loc, &color);
			EG_Draw_Border(surface_ptr, &loc, &color, EG_Draw_Border_BoxLow);

			if (tmp.w >= 2){
				EG_Draw_Box(surface_ptr, &tmp, &progressbar_ptr->bar_color);
				EG_Draw_Border(surface_ptr, &tmp, &progressbar_ptr->bar_color
				 , EG_Draw_Border_BoxHigh);	
			}
//		}
	}else{

		EG_Draw_Box(surface_ptr, &loc, &color);
		EG_Draw_Border(surface_ptr, &loc, &color, EG_Draw_Border_BoxLow);

		tmp.x = loc.x + 2 + ( (loc.w -4) * progressbar_ptr->position);
		tmp.y = loc.y + 2;
		tmp.w = ( (loc.w - 4) * 0.1);
		tmp.h = loc.h - 4;

		EG_Draw_Box(surface_ptr, &tmp, &progressbar_ptr->bar_color);
		EG_Draw_Border(surface_ptr, &tmp, &progressbar_ptr->bar_color, EG_Draw_Border_BoxHigh);
	
		progressbar_ptr->position +=  progressbar_ptr->inc;
		if (progressbar_ptr->position >= 0.9){
			progressbar_ptr->position = 0.9;
			progressbar_ptr->inc = -0.1;
		}
		if (progressbar_ptr->position < 0.0){
			progressbar_ptr->position = 0.0;
			progressbar_ptr->inc = 0.1;
		}
	}

	EG_Draw_Enable_Update();
	EG_Draw_UpdateSurface(surface_ptr, loc);


	/* Paint succeeded so return true.
	 */
	return(EG_TRUE);
}


/* Private functions:
 */

static EG_BOOL InitializeWidget(EG_Widget *widget_ptr, SDL_Color color, SDL_Color bar_color , SDL_Rect dimension)
{
	/* Initialize callbacks:
	 */
	(void) EG_Widget_SetCallback_Destroy(widget_ptr,
	 EG_Callback_Generic_Destroy);

	(void) EG_Widget_SetCallback_Paint(widget_ptr, 
	 Callback_Paint);

	(void) EG_Widget_SetCallback_SDL_Event(widget_ptr,
	 Callback_SDL_Event);

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

	(void) EG_ProgressBar_SetPosition(widget_ptr, 0.0);
	(void) EG_ProgressBar_SetBarColor(widget_ptr, bar_color);

	return(EG_TRUE);
}


/* Public functions:
 */

EG_Widget* EG_ProgressBar_Create(const char *name_ptr, SDL_Color color, SDL_Color bar_color, SDL_Rect dimension)
{
	EG_ProgressBar *progressbar_ptr;
	EG_Widget *widget_ptr;
	void *ptr;

	/* Create the new widget.
	 */
	SHARED__ALLOC_PAYLOAD_STRUCT(ptr, EG_ProgressBar
	 , "Unable to malloc EG_ProgressBar struct");
	progressbar_ptr = (EG_ProgressBar*) ptr;

	SHARED__CREATE_NEW_EG_WIDGET(widget_ptr, name_ptr, EG_Widget_Type_ProgressBar
	 , progressbar_ptr);

	SHARED__ATTACH_PAYLOAD_TO_WIDGET(widget_ptr, progressbar_ptr);

	progressbar_ptr->last_called = 0;
	progressbar_ptr->previous_w = 0;
	progressbar_ptr->inc = 0.1;

	InitializeWidget(widget_ptr, color, bar_color, dimension);
	return(widget_ptr);
}

EG_BOOL EG_ProgressBar_Resize(EG_Widget *widget_ptr, SDL_Rect area)
{
	if (EG_Widget_SetDimension(widget_ptr, area) == EG_TRUE){
		EG_Widget_RepaintLot(widget_ptr);
		return(EG_TRUE);
	}
	
	return(EG_FALSE);
}

EG_BOOL EG_ProgressBar_PositionIsUnknown(EG_Widget *widget_ptr)
{
	EG_ProgressBar *progressbar_ptr;
	EG_PROGRESSBAR_GET_STRUCT_PTR(widget_ptr, progressbar_ptr, EG_FALSE);
	
	progressbar_ptr->unknown_position = EG_TRUE;
	progressbar_ptr->position = 0.0;

	EG_Widget_RepaintLot(widget_ptr);
	return EG_TRUE;
}

EG_BOOL EG_ProgressBar_SetBarColor(EG_Widget *widget_ptr, SDL_Color color)
{
	EG_ProgressBar *progressbar_ptr;

	EG_PROGRESSBAR_GET_STRUCT_PTR(widget_ptr, progressbar_ptr, EG_FALSE);

	progressbar_ptr->bar_color = color;
	EG_Widget_RepaintLot(widget_ptr);

	return EG_TRUE;
}

float	EG_ProgressBar_GetPosition(EG_Widget *widget_ptr)
{
	EG_ProgressBar *progressbar_ptr;
	EG_PROGRESSBAR_GET_STRUCT_PTR(widget_ptr, progressbar_ptr, 0.0);
	return progressbar_ptr->position;
}

EG_BOOL EG_ProgressBar_SetPosition(EG_Widget *widget_ptr, float position)
{
	EG_ProgressBar *progressbar_ptr; 

	EG_PROGRESSBAR_GET_STRUCT_PTR(widget_ptr, progressbar_ptr, EG_FALSE);

	if (position<0.0) position = 0.0;
	if (position>1.0) position = 1.0;

	progressbar_ptr->position = position;
	progressbar_ptr->unknown_position = EG_FALSE;

//	EG_Widget_RepaintLot(widget_ptr);
	return EG_TRUE;
}

/* END OF progressbar.c ----------------------------------------------------------------
 */



