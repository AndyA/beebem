/* START OF slidebar.h ----------------------------------------------------------
 *
 *	A slide bar widget.
 *
 *	---
 *	THIS GUI IS TOTALLY *BROKEN*! PLEASE DO NOT USE IT!
 *	---
 */


#ifndef _DW_SLIDEBAR_H_
#define _DW_SLIDEBAR_H_


#if HAVE_CONFIG_H
#	include <config.h>
#endif

#include <gui/types.h>

#include <gui/widget.h>
#include <gui/window.h>

#include <SDL.h>


/* Widget type:
 */
#define EG_Widget_Type_SlideBar 0xFB1718E9

#define EG_SlideBar_Vertical 		0
#define EG_SlideBar_Horizontal 		1


/* Public functions:
 */

#ifdef __cplusplus
extern "C"{
#endif
	EG_Widget* EG_SlideBar_Create(const char *name_ptr, SDL_Color color, int orientation, SDL_Rect area);

	EG_BOOL EG_SlideBar_SetLength(EG_Widget *widget_ptr, long length);
	long EG_SlideBar_GetLength(EG_Widget *widget_ptr);

	EG_BOOL EG_SlideBar_SetPosition(EG_Widget *widget_ptr, long position);
	long EG_SlideBar_GetPosition(EG_Widget *widget_ptr);

	EG_BOOL EG_SlideBar_SetStep(EG_Widget *widget_ptr, long step);
	long EG_SlideBar_GetStep(EG_Widget *widget_ptr);

	EG_BOOL EG_SlideBar_SetSnap(EG_Widget *widget_ptr, EG_BOOL snap);
	EG_BOOL EG_SlideBar_GetSnap(EG_Widget *widget_ptr);

	EG_BOOL EG_SlideBar_Resize(EG_Widget *widget_ptr, SDL_Rect area);
	
#ifdef __cplusplus
}
#endif

#define EG_SlideBar_Destroy				EG_Widget_CallDestroy
#define EG_SlideBar_GetID				EG_Widget_GetID
#define EG_SlideBar_GetName				EG_Widget_GetName

#define EG_SlideBar_SetMyCallback_OnChange		EG_Widget_SetUserCallback_OnChange
#define EG_SlideBar_ClearMyCallback_OnChange		EG_Widget_ClearUserCallback_OnChange

#define EG_SlideBar_SetMyCallback_OnLostFocus		EG_Widget_SetUserCallback_OnLostFocus
#define EG_SlideBar_ClearMyCallback_OnLostFocus		EG_Widget_ClearUserCallback_OnLostFocus

#define EG_SlideBar_SetMyCallback_OnGotFocus		EG_Widget_SetUserCallback_OnGotFocus
#define EG_SlideBar_ClearMyCallback_OnGotFocus		EG_Widget_ClearUserCallback_OnGotFocus

#define EG_SlideBar_Enable				EG_Window_EnableWidget
#define EG_SlideBar_Disable				EG_Window_DisableWidget

#define EG_SlideBar_Repaint				EG_Widget_Repaint
#define EG_SlideBar_RepaintLot				EG_Widget_RepaintLot

#define EG_SlideBar_IsEnabled				EG_Widget_IsEnabled


/* END OF slidebar.h ------------------------------------------------------------
 */
#endif

