/* START OF togglebutton_private.h --------------------------------------------------
 *
 *	A simple toggle button widget.
 *
 *	---
 *	THIS GUI IS TOTALLY *BROKEN*! PLEASE DO NOT USE IT!
 *	---
 */


#ifndef _DW_TOGGLEBUTTON_PRIVATE_H_
#define _DW_TOGGLEBUTTON_PRIVATE_H_


#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include <gui/types.h>

#include <gui/window.h>
#include <gui/widget.h>

#include <gui/radiogroup.h>

#include <gui/togglebutton.h>

#include <SDL.h>


/* Get EG_ToggleButton struct.
 */

#define EG_TOGGLEBUTTON_GET_STRUCT_PTR(w, v, r)  \
	if (w == NULL ){ \
		EG_Log(EG_LOG_ERROR, dL"EG_Widget is NULL.", dR); \
		return( r ); \
	} \
	if (EG_Widget_GetType( w ) != EG_Widget_Type_ToggleButton){ \
		EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is not of type EG_ToggleButton.", dR); \
		return( r ); \
	} \
	if ( (v = (EG_ToggleButton*) EG_Widget_GetPayload( w )) == NULL){ \
		EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is NULL.", dR); \
		return(EG_FALSE); \
	} \
	EG_ASSERT_MALLOC( v );


/* EG_Widget payload:
 */

typedef struct{
	char caption[MAX_TOGGLEBUTTON_CAPTIONSIZE+1];

	EG_Widget *parent_group_widget;

//	char set_value;
//	char unset_value;

	EG_BOOL is_selected;

	EG_BOOL depressed;
}EG_ToggleButton;
#define EG_AsToggleButton(n) ((EG_ToggleButton*) n)


/* Private and friend functions:
 */

#ifdef __cplusplus
extern "C"{
#endif
	EG_BOOL 	EG_ToggleButton_SetGroupParent(EG_Widget *togglebutton_widget_ptr, EG_Widget *group_parent_widget_ptr);
#ifdef __cplusplus
}
#endif


/* END OF togglebutton_private.h ----------------------------------------------------
 */
#endif



