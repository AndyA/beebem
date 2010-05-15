/* START OF box.h --------------------------------------------------------------
 *
 *	A simple box widget.
 *
 *	---
 *	THIS GUI IS TOTALLY *BROKEN*! PLEASE DO NOT USE IT!
 *	---
 */


#ifndef _DW_BOX_H_
#define _DW_BOX_H_


#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include <gui/types.h>
#include <gui/sdl.h>
#include <gui/window.h>
#include <gui/widget.h>

#include <SDL.h>


/* Widget type:
 *
 * Use functions.c 'EG_MakeStringHash' function to calculate this. The
 * string required is the definition name.
 *
 * For example below:- "EG_Widget_Type_Box" = 0xD136B124 (using printf "0x%X").
 */
#define EG_Widget_Type_Box 0xD136B124


/* Border type:
 */
//#define EG_Box_Border_None 	EG_Draw_Border_Normal
//#define EG_Box_Border_Raised	EG_Draw_Border_BoxHigh
//#define EG_Box_Border_Sunk	EG_Draw_Border_BoxLow

#define EG_BOX_BORDER_NONE 	EG_Draw_Border_Normal
#define EG_BOX_BORDER_RAISED	EG_Draw_Border_BoxHigh
#define EG_BOX_BORDER_SUNK	EG_Draw_Border_BoxLow



/* Public functions:
 */
#ifdef __cplusplus
extern "C"{
#endif
	EG_Widget* 	EG_Box_Create(const char *name_ptr, int type, SDL_Color color, SDL_Rect dimension);
	EG_BOOL 	EG_Box_SetBorderType(EG_Widget *widget_ptr, int type);
	int 		EG_Box_GetBorderType(EG_Widget *widget_ptr);
	EG_BOOL		EG_Box_Resize(EG_Widget *widget_ptr, SDL_Rect area);
#ifdef __cplusplus
}
#endif

#define EG_Box_Destroy			EG_Widget_CallDestroy

#define EG_Box_SetMyCallback_OnEvent	EG_Widget_SetUserCallback_OnEvent
#define EG_Box_ClearMyCallback_OnEvent	EG_Widget_ClearUserCallback_OnEvent

#define EG_Box_Hide			EG_Window_HideWidget
#define EG_Box_Show			EG_Window_ShowWidget

#define EG_Box_SetBackgroundColor	EG_Widget_SetBackgroundColor
#define EG_Box_GetBackgroundColor	EG_Widget_GetBackgroundColor

#define EG_Box_Repaint			EG_Widget_Repaint
#define EG_Box_RepaintLot		EG_Widget_RepaintLot

#define EG_Box_IsVisible		EG_Widget_IsVisible

#define EG_Box_GetID			EG_Widget_GetID
#define EG_Box_GetName			EG_Widget_GetName

/* END OF box.h ----------------------------------------------------------------
 */
#endif



