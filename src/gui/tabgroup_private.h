/* START OF tabgroup_private.h --------------------------------------------------
 *
 *	A Tab Group widget.
 *
 *	The design of this GUI cannot really support Tabbers, but I've 'fudged'
 *	it with the 'stopped' / 'start' callbacks so basic none nested tabbers
 *	can be easily implemented.
 *
 *	The design does not really support it, but I think tabbers are just too
 *	nice to live without.
 *
 *	---
 *	THIS GUI IS TOTALLY *BROKEN*! PLEASE DO NOT USE IT!
 *	---
 */


#ifndef _DW_TABGROUP_PRIVATE_H_
#define _DW_TABGROUP_PRIVATE_H_

#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include <gui/types.h>

#include <gui/widget.h>
#include <gui/window.h>

#include <SDL.h>

/*
#ifdef EG_DEBUG
#       define CHECK_EG_TABGROUP_IS_VALID(o, r) \
                if (o == NULL){ \
                        EG_Log(EG_LOG_WARNING, dL"EG_TabGroup pointer is NULL." \
                         , dR); \
                        return(r); \
                } \
                EG_ASSERT_MALLOC(o);

#       define CHECK_EG_TABGROUP_IS_VALID_VOID(o) \
                if (o == NULL){ \
                        EG_Log(EG_LOG_WARNING, dL"EG_TabGroup pointer is NULL." \
                        , dR); \
                        return; \
                } \
                EG_ASSERT_MALLOC(o);
#else
#       define CHECK_EG_TABGROUP_IS_VALID(o, r) \
                if (o == NULL){ \
                        EG_Log(EG_LOG_WARNING, dL"EG_TabGroup pointer is NULL." \
                         , dR); \
                        return(r); \
                }

#       define CHECK_EG_TABGROUP_IS_VALID_VOID(o) \
                if (o == NULL){ \
                        EG_Log(EG_LOG_WARNING, dL"EG_TabGroup pointer is NULL." \
                        , dR); \
                        return; \
                }
#endif
*/

/* Get EG_TabGroup struct.
 */

#define EG_TABGROUP_GET_STRUCT_PTR(w, v, r)  \
        if (w == NULL ){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget is NULL.", dR); \
                return( r ); \
        } \
        if (EG_Widget_GetType( w ) != EG_Widget_Type_TabGroup){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is not of type EG_TabGroup.", dR); \
                return( r ); \
        } \
        if ( (v =EG_Widget_GetPayload( w )) == NULL){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is NULL.", dR); \
                return(r); \
        } \
        EG_ASSERT_MALLOC( v );

#define EG_TABGORUP_GET_STRUCT_PTR_VOID(w, v)  \
        if (w == NULL ){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget is NULL.", dR); \
                return; \
        } \
        if (EG_Widget_GetType( w ) != EG_Widget_Type_TabGroup){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is not of type EG_TabGroup.", dR); \
                return; \
        } \
        if ( (v =EG_Widget_GetPayload( w )) == NULL){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is NULL.", dR); \
                return; \
        } \
        EG_ASSERT_MALLOC( v );

#define EG_TABGROUP_GET_STRUCT_PTR_VOID(w, v)  \
        if (w == NULL ){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget is NULL.", dR); \
                return; \
        } \
        if (EG_Widget_GetType( w ) != EG_Widget_Type_TabGroup){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is not of type EG_TabGroup.", dR); \
                return; \
        } \
        if ( (v =EG_Widget_GetPayload( w )) == NULL){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is NULL.", dR); \
                return; \
        } \
        EG_ASSERT_MALLOC( v );

#define EG_TABGORUP_GET_STRUCT_PTR_VOID(w, v)  \
        if (w == NULL ){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget is NULL.", dR); \
                return; \
        } \
        if (EG_Widget_GetType( w ) != EG_Widget_Type_TabGroup){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is not of type EG_TabGroup.", dR); \
                return; \
        } \
        if ( (v =EG_Widget_GetPayload( w )) == NULL){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is NULL.", dR); \
                return; \
        } \
        EG_ASSERT_MALLOC( v );



/* EG_Widget payload:
 */

#define MAX_TABGROUP_PAGES 16

typedef struct{
        EG_Widget       *page_widget_ptr[MAX_TABGROUP_PAGES];
        long            count;

	long		selected;

	int		tab_bar_width;
}EG_TabGroup;
#define EG_AsTabGroup(n) ((EG_TabGroup*) n)


/* Private and Friend Functions:
 */

#ifdef __cplusplus
extern "C" {
#endif

	EG_BOOL EG_TabGroup_SelectPage(EG_Widget *page_widget);
	EG_BOOL EG_TabGroup_SelectPageByIndex(EG_Widget *widget_ptr, int page);

#ifdef __cplusplus
}
#endif


/* END OF tabgroup_private.h ----------------------------------------------------
 */
#endif

