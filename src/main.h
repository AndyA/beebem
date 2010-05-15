/****************************************************************************/
/*                               Beebem                                     */
/*                               ------                                     */
/* This program may be distributed freely within the following restrictions:*/
/*                                                                          */
/* 1) You may not charge for this program or for any part of it.            */
/* 2) This copyright message must be distributed with all copies.           */
/* 3) This program must be distributed complete with source code.  Binary   */
/*    only distribution is not permitted.                                   */
/* 4) The author offers no warrenties, or guarentees etc. - you use it at   */
/*    your own risk.  If it messes something up or destroys your computer   */
/*    thats YOUR problem.                                                   */
/* 5) You may use small sections of code from this program in your own      */
/*    applications - but you must acknowledge its use.  If you plan to use  */
/*    large sections then please ask the author.                            */
/*                                                                          */
/* If you do not agree with any of the above then please do not use this    */
/* program.                                                                 */
/****************************************************************************/
/* Mike Wyatt and NRM's port to win32 - 7/6/97 */

#ifndef MAIN_HEADER
#define MAIN_HEADER
#ifdef MULTITHREAD
#undef MULTITHREAD
#endif
//#define MULTITHREAD

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include "windows.h"
#include "beebwin.h"

#include <functions.h>
#include "sdl.h"

#include <gui.h>

/* Fudge for command line --------
 */
extern int Tmp_Command_Line_Fullscreen;
/* -------------------------------
 */


#include "cregistry.h"
extern cRegistry SysReg;
extern char FDCDLL[256];

extern const char *CFG_REG_KEY;

//extern int fullscreen;
extern int done;

extern unsigned char MachineType;
extern BeebWin *mainWin;
//-- extern HINSTANCE hInst;
void WriteLog(char *fmt, ...);
extern int trace_186;



void Quit(void);
void SetActiveWindow(EG_Window *window_ptr);
int GetFullscreenState(void);
int ToggleFullscreen(void);
void ShowingMenu(void);
void NoMenuShown(void);




#endif
