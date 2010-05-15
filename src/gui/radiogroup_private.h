/* START OF radiogroup_private.h -----------------------------------------------
 *
 *	A Radio Button Group widget.
 * 
 *	---
 *	THIS GUI IS TOTALLY *BROKEN*! PLEASE DO NOT USE IT!
 *	---
 */


#ifndef _DW_RADIOGROUP_PRIVATE_H_
#define _DW_RADIOGROUP_PRIVATE_H_


#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include <gui/types.h>

#include <gui/widget.h>
#include <gui/window.h>

#include <SDL.h>


/* Get EG_RadioGroup struct.
 */

#define EG_RADIOGROUP_GET_STRUCT_PTR(w, v, r)  \
        if (w == NULL ){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget is NULL.", dR); \
                return( r ); \
        } \
        if (EG_Widget_GetType( w ) != EG_Widget_Type_RadioGroup){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is not of type EG_RadioGroup.", dR); \
                return( r ); \
        } \
        if ( (v = (EG_RadioGroup*) EG_Widget_GetPayload( w )) == NULL){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is NULL.", dR); \
                return(r); \
        } \
        EG_ASSERT_MALLOC( v );

#define EG_RADIOGORUP_GET_STRUCT_PTR_VOID(w, v)  \
        if (w == NULL ){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget is NULL.", dR); \
                return; \
        } \
        if (EG_Widget_GetType( w ) != EG_Widget_Type_RadioGroup){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is not of type EG_RadioGroup.", dR); \
                return; \
        } \
        if ( (v = (EG_RadioGroup*) EG_Widget_GetPayload( w )) == NULL){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is NULL.", dR); \
                return; \
        } \
        EG_ASSERT_MALLOC( v );



/* EG_Widget payload:
 */

#define MAX_RADIOGROUP_BUTTONS 256
typedef struct{
        EG_Widget       *button_widget_ptr[MAX_RADIOGROUP_BUTTONS];
        long            count;

	long		selected;
}EG_RadioGroup;
#define EG_AsRadioGroup(n) ((EG_RadioGroup*) n)


/* Private and friend functions:
 */



/* END OF radiogroup_private.h --------------------------------------------------
 */
#endif



