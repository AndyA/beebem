/* START OF togglebutton.h ----------------------------------------------------------
 *
 *	A simple toggle button widget.
 *
 *	---
 *	THIS GUI IS TOTALLY *BROKEN*! PLEASE DO NOT USE IT!
 *	---
 */


#ifndef _DW_TOGGLEBUTTON_H_
#define _DW_TOGGLEBUTTON_H_


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
#define EG_Widget_Type_ToggleButton 0x6F55806D


/* Caption size.
 */
#define MAX_TOGGLEBUTTON_CAPTIONSIZE 64




/* Public functions:
 */

#ifdef __cplusplus
extern "C"{
#endif
	EG_Widget* 	EG_ToggleButton_Create(const char *name_ptr, SDL_Color color, const char *caption_ptr, SDL_Rect dimention);

	EG_BOOL         EG_ToggleButton_SetCaption(EG_Widget *widget_ptr, const char *caption_ptr);
	const char*     EG_ToggleButton_GetCaption(EG_Widget *widget_ptr);

	EG_Widget*      EG_ToggleButton_GetGroupParent(EG_Widget *togglebutton_widget_ptr);

	EG_BOOL 	EG_ToggleButton_IsSelected(EG_Widget *widget_ptr);

	EG_BOOL 	EG_ToggleButton_SetSelected(EG_Widget *widget_ptr);
	EG_BOOL 	EG_ToggleButton_SetUnselected(EG_Widget *widget_ptr);
#ifdef __cplusplus
}
#endif

#define EG_ToggleButton_Destroy			EG_Widget_CallDestroy

#define EG_ToggleButton_SetMyCallback_OnEvent	EG_Widget_SetUserCallback_OnEvent
#define EG_ToggleButton_ClearMyCallback_OnEvent	EG_Widget_ClearUserCallback_OnEvent

#define EG_ToggleButton_SetMyCallback_OnClick	EG_Widget_SetUserCallback_OnClick
#define EG_ToggleButton_ClearMyCallback_OnClick	EG_Widget_ClearUserCallback_OnClick

#define EG_ToggleButton_SetBackgroundColor	EG_Widget_SetBackgroundColor
#define EG_ToggleButton_GetBackgroundColor	EG_Widget_GetBackgroundColor

#define EG_ToggleButton_Repaint			EG_Widget_Repaint
#define EG_ToggleButton_RepaintLot		EG_Widget_RepaintLot

#define EG_ToggleButton_Hide			EG_Window_HideWidget
#define EG_ToggleButton_Show			EG_Window_ShowWidget

#define EG_ToggleButton_Enable			EG_Window_EnableWidget
#define EG_ToggleButton_Disable			EG_Window_DisableWidget

#define EG_ToggleButton_IsVisible		EG_Widget_IsVisible
#define EG_ToggleButton_IsHidden		EG_Widget_IsHidden
#define EG_ToggleButton_IsEnabled		EG_Widget_IsEnabled

#define EG_ToggleButton_GetID			EG_Widget_GetID
#define EG_ToggleButton_GetName			EG_Widget_GetName

/* END OF togglebutton.h ------------------------------------------------------------
 */
#endif
