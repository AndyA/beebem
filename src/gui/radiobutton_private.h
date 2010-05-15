/* START OF radiobutton_private.h ----------------------------------------------
 *
 *	A simple Radio Button (wraps the TickBox widget really).
 *
 *	---
 *	THIS GUI IS TOTALLY *BROKEN*! PLEASE DO NOT USE IT!
 *	---
 */


#ifndef _DW_RADIOBUTTON_PRIVATE_H_
#define _DW_RADIOBUTTON_PRIVATE_H_


#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include <gui/types.h>

#include <gui/tickbox_private.h>

#include <gui/window.h>
#include <gui/widget.h>

#include <SDL.h>


/* Get EG_TickBox struct.
 */
#define EG_RADIOBUTTON_GET_STRUCT_PTR	EG_TICKBOX_GET_STRUCT_PTR

/* Don't really need this.
 */
#define EG_RadioButton EG_TickBox


/* Private and friend functions:
 */

#define EG_RadioButton_SetGroupParent	EG_TickBox_SetGroupParent
#define EG_RadioButton_GetGroupParent	EG_TickBox_GetGroupParent

#define EG_RadioButton_Tick		EG_TickBox_Tick
#define EG_RadioButton_Untick		EG_TickBox_Untick

/* END OF tickbox_private.h ----------------------------------------------------
 */
#endif                                                                           
