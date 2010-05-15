/* START OF label_private.h ----------------------------------------------------
 *
 *	Simple label widget
 *
 *	---
 *	THIS GUI IS TOTALLY *BROKEN*! PLEASE DO NOT USE IT!
 *	---
 */


#ifndef _DW_LABEL_PRIVATE_H_
#define _DW_LABEL_PRIVATE_H_

#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include <gui/types.h>
#include <SDL.h>

#include <gui/label.h>

#include <gui/window.h>
#include <gui/widget.h>


/* Get EG_Label struct.
 */

#define EG_LABEL_GET_STRUCT_PTR(w, v, r)  \
	if (w == NULL ){ \
		EG_Log(EG_LOG_ERROR, dL"EG_Widget is NULL.", dR); \
		return( r ); \
	} \
	if (EG_Widget_GetType( w ) != EG_Widget_Type_Label){ \
		EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is not of type EG_Label.", dR); \
		return( r ); \
	} \
	if ( (v = (EG_Label*) EG_Widget_GetPayload( w )) == NULL){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is NULL.", dR); \
                return(EG_FALSE); \
        } \
        EG_ASSERT_MALLOC( (void*) v );


/* Maximum size a label caption can be (minus the '\0', I add one for that).
 */
#define MAX_LABEL_CAPTIONSIZE         256

/* EG_Widget payload:
 */
typedef struct{
	int alignment;
        char caption[MAX_LABEL_CAPTIONSIZE+1];
}EG_Label;
#define EG_AsLabel(n) ((EG_Label*) n)


/* Private and Friends Functions:
 */


/* END OF label_private.h ------------------------------------------------------
 */
#endif



