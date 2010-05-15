/* START OF widget_private.h ---------------------------------------------------
 *
 *	Base widget API.
 *
 *	---
 *    THIS GUI IS TOTALLY *BROKEN*! PLEASE DO NOT USE IT!
 *	---
 */


#ifndef _DW_WIDGET_PRIVATE_H_
#define _DW_WIDGET_PRIVATE_H_

#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include <gui/types.h>
#include <SDL.h>

#include <gui/widget.h>


/* Convenient macro to check for more obvious EG_Widget struct pointer errors.
 */

// [TODO] Probably a bit late now, but add the type as an argument so it can test that it is the correct type of widget too.
#ifdef EG_DEBUG                 
#       define CHECK_EG_WIDGET_IS_VALID(o, r) \
                if (o == NULL){ \
                        EG_Log(EG_LOG_WARNING, dL"EG_Widget pointer is NULL." \
                         , dR); \
                        return(r); \
                } \
                EG_ASSERT_MALLOC(o);

#       define CHECK_EG_WIDGET_IS_VALID_VOID(o) \
                if (o == NULL){ \
                        EG_Log(EG_LOG_WARNING, dL"EG_Widget pointer is NULL." \
                        , dR); \
                        return; \
                } \
                EG_ASSERT_MALLOC(o);
#else
#       define CHECK_EG_WIDGET_IS_VALID(o, r) \
                if (o == NULL){ \
                        EG_Log(EG_LOG_WARNING, dL"EG_Widget pointer is NULL." \
                         , dR); \
                        return(r); \
                }

#       define CHECK_EG_WIDGET_IS_VALID_VOID(o) \
                if (o == NULL){ \
                        EG_Log(EG_LOG_WARNING, dL"EG_Widget pointer is NULL." \
                        , dR); \
                        return; \
                }
#endif


/* Private functions:
 */
#ifdef __cplusplus
extern "C"{
#endif
	EG_Widget* 	EG_Widget_Alloc(const char *name_ptr, EG_StringHash type);
	void 		EG_Widget_Free(EG_Widget *widget_ptr);

	// This is now public.
	//void 		EG_Widget_CallDestroy(EG_Widget *widget_ptr);

	EG_BOOL 	EG_Widget_CallPaint(EG_Widget *widget_ptr, SDL_Rect rect);
	EG_BOOL		EG_Widget_CallSDLEvent(EG_Widget *widget_ptr, SDL_Event *event_ptr);
	EG_BOOL 	EG_Widget_CallVisible(EG_Widget *widget_ptr, EG_BOOL visible);
	EG_BOOL 	EG_Widget_CallEnabled(EG_Widget *widget_ptr, EG_BOOL enabled);
	EG_BOOL 	EG_Widget_CallGotFocus(EG_Widget *widget_ptr);
	void 		EG_Widget_CallLostFocus(EG_Widget *widget_ptr);
	EG_BOOL 	EG_Widget_CallStopped(EG_Widget *widget_ptr, EG_BOOL stopped);
	void		EG_Widget_CallAttach(EG_Widget *widget_ptr, EG_StringHash attach_to_type, void *attach_to_ptr, EG_BOOL attached);

	void 		EG_Widget_CallUserOnChange(EG_Widget *widget_ptr);
	void 		EG_Widget_CallUserOnClick(EG_Widget *widget_ptr);
	void 		EG_Widget_CallUserOnEvent(EG_Widget *widget_ptr, SDL_Event *event_ptr);
	void		EG_Widget_CallUserOnLostFocus(EG_Widget *widget_ptr);
	void		EG_Widget_CallUserOnGotFocus(EG_Widget *widget_ptr);

	EG_BOOL 	EG_Widget_SetCallback_Destroy(EG_Widget *widget_ptr, EG_Callback_Destroy destroy_func_ptr);
	EG_BOOL 	EG_Widget_SetCallback_Paint(EG_Widget *widget_ptr, EG_Callback_Paint paint_func_ptr);
	EG_BOOL 	EG_Widget_SetCallback_SDL_Event(EG_Widget *widget_ptr, EG_Callback_SDL_Event sdl_event_func_ptr);
	EG_BOOL 	EG_Widget_SetCallback_Visible(EG_Widget *widget_ptr, EG_Callback_Visible visible_func_ptr);
	EG_BOOL 	EG_Widget_SetCallback_Enabled(EG_Widget *widget_ptr, EG_Callback_Enabled enabled_func_ptr);
	EG_BOOL 	EG_Widget_SetCallback_GotFocus(EG_Widget *widget_ptr, EG_Callback_GotFocus got_focus_func_ptr);
	EG_BOOL 	EG_Widget_SetCallback_LostFocus(EG_Widget *widget_ptr, EG_Callback_LostFocus lost_focus_func_ptr);
	EG_BOOL 	EG_Widget_SetCallback_Stopped(EG_Widget *widget_ptr, EG_Callback_Stopped stopped_func_ptr);
	EG_BOOL 	EG_Widget_SetCallback_Attach(EG_Widget *widget_ptr, EG_Callback_Attach attach_func_ptr);

	void 		EG_Widget_ClearCallback_Destroy(EG_Widget *widget_ptr);
	void 		EG_Widget_ClearCallback_Paint(EG_Widget *widget_ptr);
	void 		EG_Widget_ClearCallback_SDL_Event(EG_Widget *widget_ptr);
	void 		EG_Widget_ClearCallback_Visible(EG_Widget *widget_ptr);
	void 		EG_Widget_ClearCallback_Enabled(EG_Widget *widget_ptr);
	void 		EG_Widget_ClearCallback_GotFocus(EG_Widget *widget_ptr);
	void 		EG_Widget_ClearCallback_LostFocus(EG_Widget *widget_ptr);
	void 		EG_Widget_ClearCallback_Stopped(EG_Widget *widget_ptr);
	void 		EG_Widget_ClearCallback_Attach(EG_Widget *widget_ptr);

	EG_BOOL 	EG_Widget_SetName(EG_Widget *widget_ptr, const char *name_ptr);
	EG_BOOL 	EG_Widget_SetType(EG_Widget *widget_ptr, EG_StringHash type);

	EG_BOOL 	EG_Widget_SetHoldValue(EG_Widget *widget_ptr, long hold_value);
	long 		EG_Widget_GetHoldValue(EG_Widget *widget_ptr);

	void* 		EG_Widget_GetPayload(EG_Widget *widget_ptr);
	EG_BOOL 	EG_Widget_SetPayload(EG_Widget *widget_ptr, void *payload_ptr);

	EG_BOOL 	EG_Widget_SetDrawingArea(EG_Widget *widget_ptr, SDL_Rect dimension);

	EG_BOOL 	EG_Widget_SetDimension(EG_Widget *widget_ptr, SDL_Rect dimension);

	EG_BOOL 	EG_Widget_GetStoppedToggle(EG_Widget *widget_ptr);
	EG_BOOL 	EG_Widget_SetStoppedToggle(EG_Widget *widget_ptr, EG_BOOL stopped);

	EG_BOOL 	EG_Widget_SetCanGetFocusToggle(EG_Widget *widget_ptr, EG_BOOL can_get_focus);
	EG_BOOL 	EG_Widget_GetCanGetFocusToggle(EG_Widget *widget_ptr);

	EG_BOOL		EG_Widget_GetVisibleToggle(EG_Widget *widget_ptr);
	EG_BOOL		EG_Widget_SetVisibleToggle(EG_Widget *widget_ptr, EG_BOOL visible);

	EG_BOOL		EG_Widget_GetEnabledToggle(EG_Widget *widget_ptr);
	EG_BOOL		EG_Widget_SetEnabledToggle(EG_Widget *widget_ptr, EG_BOOL enabled);

	void* 		EG_Widget_GetWindow(EG_Widget *widget_ptr);
	EG_BOOL 	EG_Widget_SetWindow(EG_Widget *widget_ptr, void* window_ptr);
#ifdef __cplusplus
}
#endif

/* END OF widget_private.h -----------------------------------------------------
 */
#endif
