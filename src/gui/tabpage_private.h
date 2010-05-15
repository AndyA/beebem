/* START OF tabpage_private.h ---------------------------------------------------
 *
 *	A Tab Page widget.
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


#ifndef _DW_TABPAGE_PRIVATE_H_
#define _DW_TABPAGE_PRIVATE_H_

#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include <gui/types.h>

#include <gui/widget.h>
#include <gui/window.h>

#include <SDL.h>


/* Get EG_TabPage struct.
 */

#define EG_TABPAGE_GET_STRUCT_PTR(w, v, r)  \
        if (w == NULL ){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget is NULL.", dR); \
                return( r ); \
        } \
        if (EG_Widget_GetType( w ) != EG_Widget_Type_TabPage){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is not of type EG_TabPage.", dR); \
                return( r ); \
        } \
        if ( (v =EG_Widget_GetPayload( w )) == NULL){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is NULL.", dR); \
                return(r); \
        } \
        EG_ASSERT_MALLOC( v );

#define EG_TABPAGE_GET_STRUCT_PTR_VOID(w, v)  \
        if (w == NULL ){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget is NULL.", dR); \
                return; \
        } \
        if (EG_Widget_GetType( w ) != EG_Widget_Type_TabPage){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is not of type EG_TabPage.", dR); \
                return; \
        } \
        if ( (v =EG_Widget_GetPayload( w )) == NULL){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is NULL.", dR); \
                return; \
        } \
        EG_ASSERT_MALLOC( v );


#define EG_TABPAGE_GET_STRUCT_PTR_VOID(w, v)  \
        if (w == NULL ){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget is NULL.", dR); \
                return; \
        } \
        if (EG_Widget_GetType( w ) != EG_Widget_Type_TabPage){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is not of type EG_TabPage.", dR); \
                return; \
        } \
        if ( (v =EG_Widget_GetPayload( w )) == NULL){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is NULL.", dR); \
                return; \
        } \
        EG_ASSERT_MALLOC( v );

#define EG_TABPAGE_GET_STRUCT_PTR_VOID(w, v)  \
        if (w == NULL ){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget is NULL.", dR); \
                return; \
        } \
        if (EG_Widget_GetType( w ) != EG_Widget_Type_TabPage){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is not of type EG_TabPage.", dR); \
                return; \
        } \
        if ( (v =EG_Widget_GetPayload( w )) == NULL){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is NULL.", dR); \
                return; \
        } \
        EG_ASSERT_MALLOC( v );



/* EG_Widget payload:
 */

#define MAX_TABPAGE_CHILDREN            64
#define MAX_TABPAGE_CAPTIONSIZE         32

typedef struct{
        EG_Widget       *widget_ptr;
}EG_TabPage_Child;

typedef struct{
        char                    caption[MAX_TABPAGE_CAPTIONSIZE+1];
        EG_Widget               *tabgroup_widget_ptr;
        EG_TabPage_Child        child[MAX_TABPAGE_CHILDREN];

	int 			count;
	EG_BOOL			depressed;
}EG_TabPage;
#define EG_AsTabPage(n) ((EG_TabPage*) n)


/* Private and friend functions:
 */

#ifdef __cplusplus
extern "C" {
#endif

	EG_BOOL EG_TabPage_SetTabGroup(EG_Widget *page_widget_ptr, EG_Widget *group_widget_ptr);
	EG_Widget* EG_TabPage_GetTabGroup(EG_Widget *page_widget_ptr);

	EG_BOOL EG_TabPage_Show(EG_Widget *widget_ptr);
	EG_BOOL EG_TabPage_Hide(EG_Widget *widget_ptr);
	EG_BOOL EG_TabPage_RePaintChildren(EG_Widget *widget_ptr);

	EG_BOOL EG_TabPage_AddChildWidgetsToWindow(EG_Widget *widget_ptr);

#ifdef __cplusplus
}
#endif


/* END OF tabpage_private.h -----------------------------------------------------
 */
#endif



