/* START OF tickbox.h ----------------------------------------------------------
 *
 *	A simple tickbox widget.
 *
 *	---
 *	THIS GUI IS TOTALLY *BROKEN*! PLEASE DO NOT USE IT!
 *	---
 */


#ifndef _DW_TICKBOX_H_
#define _DW_TICKBOX_H_


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
#define EG_Widget_Type_TickBox 0xB36EC9E7


/* Caption size.
 */
#define MAX_TICKBOX_CAPTIONSIZE 64




/* Public functions:
 */

#ifdef __cplusplus
extern "C"{
#endif
	EG_Widget* 	EG_TickBox_Create(const char *name_ptr, SDL_Color color, const char *caption_ptr, SDL_Rect dimention);

	EG_BOOL         EG_TickBox_SetCaption(EG_Widget *widget_ptr, const char *caption_ptr);
	const char*     EG_TickBox_GetCaption(EG_Widget *widget_ptr);

	EG_Widget*      EG_TickBox_GetGroupParent(EG_Widget *tickbox_widget_ptr);

	EG_BOOL 	EG_TickBox_IsTicked(EG_Widget *widget_ptr);
	EG_BOOL 	EG_TickBox_Tick(EG_Widget *widget_ptr);
	EG_BOOL 	EG_TickBox_Untick(EG_Widget *widget_ptr);

        EG_BOOL         EG_TickBox_SetIcons(EG_Widget *widget_ptr, char set_icon, char unset_icon);
#ifdef __cplusplus
}
#endif

#define EG_TickBox_Destroy			EG_Widget_CallDestroy

#define EG_TickBox_SetMyCallback_OnEvent	EG_Widget_SetUserCallback_OnEvent
#define EG_TickBox_ClearMyCallback_OnEvent	EG_Widget_ClearUserCallback_OnEvent

#define EG_TickBox_SetMyCallback_OnClick	EG_Widget_SetUserCallback_OnClick
#define EG_TickBox_ClearMyCallback_OnClick	EG_Widget_ClearUserCallback_OnClick

#define EG_TickBox_SetBackgroundColor		EG_Widget_SetBackgroundColor
#define EG_TickBox_GetBackgroundColor		EG_Widget_GetBackgroundColor

#define EG_TickBox_Repaint			EG_Widget_Repaint
#define EG_TickBox_RepaintLot			EG_Widget_RepaintLot

#define EG_TickBox_Hide				EG_Window_HideWidget
#define EG_TickBox_Show				EG_Window_ShowWidget

#define EG_TickBox_Enable			EG_Window_EnableWidget
#define EG_TickBox_Disable			EG_Window_DisableWidget

#define EG_TickBox_IsVisible			EG_Widget_IsVisible
#define EG_TickBox_IsHidden			EG_Widget_IsHidden
#define EG_TickBox_IsEnabled			EG_Widget_IsEnabled

#define EG_TickBox_GetID			EG_Widget_GetID
#define EG_TickBox_GetName			EG_Widget_GetName

/* END OF tickbox.h ------------------------------------------------------------
 */
#endif
