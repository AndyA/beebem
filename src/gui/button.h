/* START OF button.h -----------------------------------------------------------
 *
 * 	A simple click-able button widget.
 *
 *	---
 *	THIS GUI IS TOTALLY *BROKEN*! PLEASE DO NOT USE IT!
 *	---
 */


#ifndef _DW_BUTTON_H_
#define _DW_BUTTON_H_


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
#define EG_Widget_Type_Button 0x6BE48939


/* Alignment:
 */
#define EG_BUTTON_ALIGN_LEFT 		-1
#define EG_BUTTON_ALIGN_RIGHT 		1
#define EG_BUTTON_ALIGN_CENTER 		0

/* Caption length:
 */
#define MAX_BUTTON_CAPTIONSIZE 		256

/* Public functions:
 */

#ifdef __cplusplus
extern "C"{
#endif
	EG_Widget*	EG_Button_Create(const char *name_ptr, SDL_Color color, int alignment, const char *caption_ptr, SDL_Rect dimension);

	EG_BOOL 	EG_Button_SetCaption(EG_Widget *widget_ptr, const char *caption_ptr);
	const char* 	EG_Button_GetCaption(EG_Widget *widget_ptr);

	EG_BOOL 	EG_Button_SetAlignment(EG_Widget *widget_ptr, int alignment);
	int 		EG_Button_GetAlignment(EG_Widget *widget_ptr);
#ifdef __cplusplus
}
#endif


#define EG_Button_Destroy			EG_Widget_CallDestroy

#define EG_Button_SetMyCallback_OnEvent		EG_Widget_SetUserCallback_OnEvent
#define EG_Button_ClearMyCallback_OnEvent	EG_Widget_ClearUserCallback_OnEvent

#define EG_Button_SetMyCallback_OnClick		EG_Widget_SetUserCallback_OnClick
#define EG_Button_ClearMyCallback_OnClick	EG_Widget_ClearUserCallback_OnClick

#define EG_Button_SetBackgroundColor		EG_Widget_SetBackgroundColor
#define EG_Button_GetBackgroundColor		EG_Widget_GetBackgroundColor

#define EG_Button_GetFocus			EG_Window_SetFocusToThisWidget

#define EG_Button_Repaint			EG_Widget_Repaint
#define EG_Button_RepaintLot			EG_Widget_RepaintLot

#define EG_Button_Hide				EG_Window_HideWidget
#define EG_Button_Show				EG_Window_ShowWidget

#define EG_Button_Enable			EG_Window_EnableWidget
#define EG_Button_Disable			EG_Window_DisableWidget

#define EG_Button_IsVisible			EG_Widget_IsVisible
#define EG_Button_IsEnabled			EG_Widget_IsEnabled

#define EG_Button_GetID				EG_Widget_GetID
#define EG_Button_GetName			EG_Widget_GetName

/* END OF button.h -------------------------------------------------------------
 */
#endif



