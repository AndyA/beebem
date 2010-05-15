/* START OF tabpage.h -----------------------------------------------------------
 *
 *	A Tab Page widget.
 *
 *	---
 *	THIS GUI IS TOTALLY *BROKEN*! PLEASE DO NOT USE IT!
 *	---
 */     


#ifndef _DW_TABPAGE_H_
#define _DW_TABPAGE_H_

#if HAVE_CONFIG_H               
#       include <config.h>      
#endif

#include <gui/types.h>

#include <gui/widget.h>
#include <gui/window.h>

#include <SDL.h>                


/* Widget type:
 */
#define EG_Widget_Type_TabPage 0xA5ADA29D

/* Public functions:
 */

#ifdef __cplusplus
extern "C" {
#endif

	EG_Widget*	EG_TabPage_Create(const char *name_ptr, const char *caption_ptr);

	EG_BOOL		EG_TabPage_AddWidget(EG_Widget *page_widget_ptr, EG_Widget *child_widget_ptr);

	/* Removes the widget from the tab page.
	 */
	void            EG_TabPage_RemoveWidget(EG_Widget *page_widget_ptr, EG_Widget *child_widget_ptr);

	/* Switch focus to the first widget in this tab page (if does not have a
	 * focusable widget, then the page button itself).
	 * 
	 * If the tab page has focus and you press enter, it'll do this anyway.
	 */
	EG_BOOL		EG_TabPage_Show(EG_Widget *widget_ptr);

	/* Returns true if this page is currently shown, false otherwise.
	 */
	EG_BOOL         EG_TabPage_IsShown(EG_Widget *widget_ptr);

	const char* 	EG_TabPage_GetCaption(EG_Widget *widget_ptr);

#ifdef __cplusplus
}
#endif


#define EG_TabPage_Destroy			EG_Widget_CallDestroy

#define EG_TabPage_GetID			EG_Widget_GetID
#define EG_TabPage_GetName			EG_Widget_GetName


/* END OF tabpage.h -------------------------------------------------------------
 */
#endif

