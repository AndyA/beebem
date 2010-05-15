/* START OF radiogroup.h -------------------------------------------------------
 *
 *	A Radio Button Group widget.
 *
 *	---
 *	THIS GUI IS TOTALLY *BROKEN*! PLEASE DO NOT USE IT!
 *	---
 */


#ifndef _DW_RADIOGROUP_H_
#define _DW_RADIOGROUP_H_


#if HAVE_CONFIG_H
#	include <config.h>
#endif

#include <gui/types.h>

#include <gui/widget.h>
#include <gui/window.h>

#include <SDL.h>


/* Widget type:
 */
#define EG_Widget_Type_RadioGroup 0x50F23DCB


/* Public functions:
 */

#ifdef __cplusplus
extern "C"{
#endif
	EG_Widget*	EG_RadioGroup_Create(const char *name_ptr);
	EG_BOOL		EG_RadioGroup_AddButton(EG_Widget *group_widget_ptr, EG_Widget *button_widget_ptr);

	void 		EG_RadioGroup_RemoveButton(EG_Widget *group_widget_ptr, EG_Widget *button_widget_ptr);

	void		EG_RadioGroup_DeleteButton(EG_Widget *group_widget_ptr, EG_Widget *button_widget_ptr);

	EG_Widget* 	EG_RadioGroup_GetSelected(EG_Widget *group_widget_ptr);

	/* Note the widget here is the button widget not the group widget
	 */
	EG_BOOL		EG_RadioGroup_Select(EG_Widget *button_widget_ptr);
#ifdef __cplusplus
}
#endif

#define EG_RadioGroup_Destroy			EG_Widget_CallDestroy

/* These are not implemented yet:
 * -----------------------------
 *
 * #define EG_RadioGroup_SetMyCallback_OnEvent		EG_Widget_SetUserCallback_OnEvent
 * #define EG_RadioGroup_ClearMyCallback_OnEvent	EG_Widget_ClearUserCallback_OnEvent
 * 
 * #define EG_RadioGroup_Repaint			EG_Widget_Repaint
 * #define EG_RadioGroup_RepaintLot			EG_Widget_RepaintLot
 * 
 * #define EG_RadioGroup_Hide				EG_Window_HideWidget
 * #define EG_RadioGroup_Show				EG_Window_ShowWidget
 * 
 * #define EG_RadioGroup_Enable				EG_Window_EnableWidget
 * #define EG_RadioGroup_Disable			EG_Window_DisableWidget
 *
 *  #define EG_RadioGroup_IsVisible			EG_Widget_IsVisible
 *  #define EG_RadioGroup_IsHidden			EG_Widget_IsHidden
 *  #define EG_RadioGroup_IsEnabled			EG_Widget_IsEnabled
 */

#define EG_RadioGroup_GetID			EG_Widget_GetID
#define EG_RadioGroup_GetName			EG_Widget_GetName

/* END OF radiogroup.h ---------------------------------------------------------
 */
#endif

