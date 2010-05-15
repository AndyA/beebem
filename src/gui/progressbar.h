/* START OF progressbar.h --------------------------------------------------------------
 *
 *	A simple progress bar widget.
 *
 *	---
 *	THIS GUI IS TOTALLY *BROKEN*! PLEASE DO NOT USE IT!
 *	---
 */


#ifndef _DW_PROGRESSBAR_H_
#define _DW_PROGRESSBAR_H_


#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include <gui/types.h>
#include <gui/sdl.h>

#include <gui/window.h>
#include <gui/widget.h>

#include <SDL.h>


/* Widget type:
 */
#define EG_Widget_Type_ProgressBar 0x3D8E40FF

/* Public functions:
 */
#ifdef __cplusplus
extern "C"{
#endif
	EG_Widget* 	EG_ProgressBar_Create(const char *name_ptr, SDL_Color color, SDL_Color bar_color, SDL_Rect dimension);

	EG_BOOL		EG_ProgressBar_Resize(EG_Widget *widget_ptr, SDL_Rect area);

	EG_BOOL 	EG_ProgressBar_SetPosition(EG_Widget *widget_ptr, float position);
	float  	 	EG_ProgressBar_GetPosition(EG_Widget *widget_ptr);

	EG_BOOL		EG_ProgressBar_SetBarColor(EG_Widget *widget_ptr, SDL_Color color);

	EG_BOOL		EG_ProgressBar_PositionIsUnknown(EG_Widget *widget_ptr);

#ifdef __cplusplus
}
#endif

#define EG_ProgressBar_Destroy			EG_Widget_CallDestroy

#define EG_ProgressBar_SetMyCallback_OnEvent	EG_Widget_SetUserCallback_OnEvent
#define EG_ProgressBar_ClearMyCallback_OnEvent	EG_Widget_ClearUserCallback_OnEvent

#define EG_ProgressBar_Hide			EG_Window_HideWidget
#define EG_ProgressBar_Show			EG_Window_ShowWidget

#define EG_ProgressBar_SetBackgroundColor	EG_Widget_SetBackgroundColor
#define EG_ProgressBar_GetBackgroundColor	EG_Widget_GetBackgroundColor

#define EG_ProgressBar_Repaint			EG_Widget_Repaint
#define EG_ProgressBar_RepaintLot		EG_Widget_RepaintLot

#define EG_ProgressBar_IsVisible		EG_Widget_IsVisible

#define EG_ProgressBar_GetID			EG_Widget_GetID
#define EG_ProgressBar_GetName			EG_Widget_GetName

/* END OF progressbar.h ----------------------------------------------------------------
 */
#endif



