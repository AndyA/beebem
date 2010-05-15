/* START OF messagebox.h --------------------------------------------------------------
 *
 *	A simple message box.
 *
 *	---
 *	THIS GUI IS TOTALLY *BROKEN*! PLEASE DO NOT USE IT!
 *	---
 */

#ifndef _DW_MESSAGEBOX_H_
#define _DW_MESSAGEBOX_H_

#if HAVE_CONFIG_H
#       include <config.h>
#endif

#define EG_MESSAGEBOX_STOP		0
#define EG_MESSAGEBOX_QUESTION		1
#define EG_MESSAGEBOX_INFORMATION 	2

//#include "include/gui.h"

#include <gui/types.h>

#include <gui/widget.h>
#include <gui/window.h>
#include <gui/box.h>
#include <gui/label.h>
#include <gui/button.h>

#include <gui/widget_private.h>

#ifdef __cplusplus
extern "C" {
#endif
	int EG_MessageBox(SDL_Surface *surface_ptr, int type
	 , const char *title_ptr, const char *text_ptr, const char *button1_ptr
	 , const char *button2_ptr, const char *button3_ptr
	 , const char *button4_ptr, int has_focus);
#ifdef __cplusplus
}
#endif

/* END OF messagebox.h ----------------------------------------------------------------
 */
#endif



