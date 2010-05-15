/* START OF tabgroup.h ----------------------------------------------------------
 *
 *	A Tab Group widget.
 *
 *	---
 *	THIS GUI IS TOTALLY *BROKEN*! PLEASE DO NOT USE IT!
 *	---
 */


#ifndef _DW_TABGROUP_H_
#define _DW_TABGROUP_H_

#if HAVE_CONFIG_H
#	include <config.h>
#endif

#include <gui/types.h>

#include <gui/widget.h>
#include <gui/window.h>

#include <SDL.h>


/* Widget type:
 */
#define EG_Widget_Type_TabGroup 0xF8FB831


/* Tab Page button height:
 */
#define EG_TabGroup_PageButtonHeight		21


/* Public functions:
 */
#ifdef __cplusplus
extern "C" {
#endif

	EG_Widget*	EG_TabGroup_Create(const char *name_ptr, SDL_Color color, SDL_Rect dimension);
	EG_BOOL		EG_TabGroup_AddPage(EG_Widget *group_widget_ptr, EG_Widget *page_widget_ptr);

	/* Deletes the page widget, but leaves it's child widget intact (they are placed
	 * directly onto the window instead).
	 */
	void 		EG_TabGroup_RemovePage(EG_Widget *group_widget_ptr, EG_Widget *page_widget_ptr);

	/* Deletes the page widget and all of it's child widgets.
	 */
	void		EG_TabGroup_DeletePage(EG_Widget *group_widget_ptr, EG_Widget *page_widget_ptr);

	EG_Widget* 	EG_TabGroup_GetSelectedPageWidget(EG_Widget *group_widget_ptr);

#ifdef __cplusplus
}
#endif

#define EG_TabGroup_Destroy			EG_Widget_CallDestroy

#define EG_TabGroup_GetID			EG_Widget_GetID
#define EG_TabGroup_GetName			EG_Widget_GetName


/* END OF tabgroup.h ------------------------------------------------------------
 */
#endif

