/* START OF window_private.h ---------------------------------------------------
 *
 *	Window API.
 *
 *	---
 *	THIS GUI IS TOTALLY *BROKEN*! PLEASE DO NOT USE IT!
 *	---
 */


#ifndef _DW_WINDOW_PRIVATE_H_
#define _DW_WINDOW_PRIVATE_H_

#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include <gui/types.h>
#include <SDL.h>

#include <gui/window.h>
#include <gui/widget.h>


/* Convenient macro to check for more obvious EG_Window struct pointer errors.
 */
#ifdef EG_DEBUG
#       define CHECK_EG_WINDOW_IS_VALID(o, r) \
                if (o == NULL){ \
                        EG_Log(EG_LOG_WARNING, dL"EG_Window pointer is NULL." \
                         , dR); \
                        return(r); \
                } \
                EG_ASSERT_MALLOC(o);

#       define CHECK_EG_WINDOW_IS_VALID_VOID(o) \
                if (o == NULL){ \
                        EG_Log(EG_LOG_WARNING, dL"EG_Window pointer is NULL." \
                        , dR); \
                        return; \
                } \
                EG_ASSERT_MALLOC(o);
#else
#       define CHECK_EG_WINDOW_IS_VALID(o, r) \
                if (o == NULL){ \
                        EG_Log(EG_LOG_WARNING, dL"EG_Window pointer is NULL." \
                         , dR); \
                        return(r); \
                }

#       define CHECK_EG_WINDOW_IS_VALID_VOID(o) \
                if (o == NULL){ \
                        EG_Log(EG_LOG_WARNING, dL"EG_Window pointer is NULL." \
                        , dR); \
                        return; \
                }
#endif


/* Private and Friend functions:
 */

#ifdef __cplusplus
extern "C"{
#endif
	long 		EG_Window_Child_GetIndexFromWidget(EG_Window *window_ptr, EG_Widget *widget_ptr);

	EG_Widget* 	EG_Window_Child_GetFocusedWidget(EG_Window *window_ptr);
	EG_BOOL 	EG_Window_Child_SetFocusedIndex(EG_Window *window_ptr, long index);
	EG_BOOL 	EG_Window_Child_SetFocusedWidget(EG_Window *window_ptr, EG_Widget *widget_ptr);

	long 		EG_Window_Child_GetCount(EG_Window *window_ptr);

	EG_Widget* 	EG_Window_Child_Get(EG_Window *window_ptr, long index);

	EG_BOOL	 	EG_Window_Child_Remove(EG_Window *window_ptr, long index);
	EG_BOOL 	EG_Window_Child_Add(EG_Window *window_ptr, EG_Widget *widget_ptr);


	EG_Window* 	EG_Window_Alloc(const char *name_ptr);
	void 		EG_Window_Free(EG_Window *window_ptr);

	void 		EG_Window_ClearFocus(EG_Window *window_ptr);
	EG_BOOL 	EG_Window_MoveFocusForward(EG_Window *window_ptr);
	EG_BOOL 	EG_Window_MoveFocusBackward(EG_Window *window_ptr);

	void 		EG_Window_SetDimension(EG_Window *window_ptr, SDL_Rect dimension);
	EG_BOOL 	EG_Window_SetName(EG_Window *window_ptr, const char *name_ptr);

	void    	EG_Window_SetHoldValue(EG_Window *window_ptr, void *hold_value_ptr);
	void*   	EG_Window_GetHoldValue(EG_Window *window_ptr);

	EG_BOOL 	EG_Window_StopWidget(EG_Widget *widget_ptr);
	EG_BOOL 	EG_Window_StartWidget(EG_Widget *widget_ptr);

#ifdef __cplusplus
}
#endif


/* END OF window_private.h -----------------------------------------------------
 */
#endif
