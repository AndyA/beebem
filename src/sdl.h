#ifndef SDL_HEADER
#define SDL_HEADER

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <SDL.h>

#include <stdlib.h>
#include "sdl.h"
#include "types.h"


#define BEEBEM_VIDEO_CORE_SCREEN_WIDTH		800
#define BEEBEM_VIDEO_CORE_SCREEN_HEIGHT		600

#define SDL_WINDOW_WIDTH			640
#define SDL_WINDOW_HEIGHT			512

#define LOCK(s)   {if(SDL_MUSTLOCK(s))(void)SDL_LockSurface(s);}
#define UNLOCK(s) {if(SDL_MUSTLOCK(s))(void)SDL_UnlockSurface(s);}


extern SDL_Surface *video_output;
extern SDL_Surface *screen_ptr;

/* Global configuration options:
 */

extern int cfg_HaveX11;

#define OPT_EMULATECRTGRAPHICS 30001
#define CFG_EMULATECRTGRAPHICS "EmulateCrtGraphics"
extern int cfg_EmulateCrtGraphics;

#define OPT_EMUALTECRTTELETEXT 30002
#define CFG_EMUALTECRTTELETEXT "EmulateCrtTeletext"
extern int cfg_EmulateCrtTeletext;

#define OPT_WANTLOWLATENCYSOUND 30003
#define CFG_WANTLOWLATENCYSOUND "WantLowLatencySound"
extern int cfg_WantLowLatencySound;

#define RESOLUTION_640X512      0
#define RESOLUTION_640X480_S    1
#define RESOLUTION_640X480_V    2
#define RESOLUTION_320X240_S    3
#define RESOLUTION_320X240_V    4
#define RESOLUTION_320X256	5

#define CFG_WINDOWEDRESOLUTION	"WindowedResolution"
extern int cfg_Windowed_Resolution;

#define CFG_FULLSCREENRESOLUTION	"FullscreenResolution"
extern int cfg_Fullscreen_Resolution;

extern int cfg_VerticalOffset;

/* Timing:
 *
 * The functions below replace the Windows 'sleep' command.  It's a bit more
 * involved here and hopefully this approach will give varied systems more of
 * a chance to execute the emulator properly.
 *
 * The definitions below set the type of wait to use when the emulator wants to
 * sleep.  It's set via the user interface on the 'Screen' page.
 */
#define OPT_SLEEP_OS	30005
#define OPT_SLEEP_F1	30006
#define OPT_SLEEP_F2	30007
#define OPT_SLEEP_F3	30008
#define OPT_SLEEP_BW	30009
#define CFG_SLEEP_TYPE	"SleepType"
extern int cfg_WaitType;

/*	-	-	-	-
 */

extern int InitialiseSDL(int argc, char *argv[]);
extern void UninitialiseSDL(void);
extern void RenderLine(int, int, int);
extern void SaferSleep(unsigned int);
extern unsigned char* GetSDLScreenLinePtr(int);
extern void SetWindowTitle(char*);
extern int ConvertSDLKeyToBBCKey(SDL_keysym, int*, /* int*, */ int*);
extern void SetBeebEmEmulatorCoresPalette(unsigned char*, int);

extern void AddBytesToSDLSoundBuffer(void*, int);

extern void CatchupSound(void);
extern void ClearVideoWindow(void);
extern void RenderFullscreenFPS(const char *str, int y);

extern void Destroy_Screen(void);
extern int Create_Screen(void);

/* Delta time to realtime sync:
 */

#define MINIMUMDELAY            10      // Min. amount of time OS will can sleep
                                        // in milliseconds.

#define WAIT_IS_NICE		0	// Will free-up max amount of time to OS
#define WAIT_IS_OPTIMISED	1	// Will free-up min amount of time to OS
#define WAIT_IS_NASTY 		2	// Will free-up NO time to the OS..



/* If you mess with the above, you may
 * get better performance.  Also I'm worried that some PDA's and other systems
 * may have a problem with sleeping for n milliseconds etc, and of course the
 * first thing bad timing will effect is the sound..
 */

#define FRAMESPERSECOND 	50	// Number of times per second 'screen'
					// will update.



/* Prototypes
 */

#define LOCK(s)   {if(SDL_MUSTLOCK(s))(void)SDL_LockSurface(s);}
#define UNLOCK(s) {if(SDL_MUSTLOCK(s))(void)SDL_UnlockSurface(s);}



/*
EG_BOOL EG_Initialize(void);
void EG_Quit(void);

Uint32 EG_Draw_GetCurrentTime(void);
Uint32 EG_Draw_GetTimePassed(Uint32 time_start);

void EG_Draw_FlushEventQueue(void);

float EG_Draw_GetScale(void);
void EG_Draw_SetToLowResolution(void);
void EG_Draw_SetToHighResolution(void);
void EG_Draw_UpdateSurface(SDL_Surface *surface, Sint32 x, Sint32 y, Sint32 w
 , Sint32 h);

void EG_Draw_Char(SDL_Surface *surface, SDL_Color *color, EG_BOOL bold, Uint16 x, Uint16 y
 , char c);

void EG_Draw_String(SDL_Surface *surface, SDL_Color *colour, EG_BOOL bold, SDL_Rect *area_ptr
 , int justify, char* string);

#define EG_Draw_Border_Normal	0
#define EG_Draw_Border_BoxHigh 	1
#define EG_Draw_Border_BoxLow 	2
#define EG_Draw_Border_Focused	3
void EG_Draw_Border(SDL_Surface *surface, SDL_Rect *area, SDL_Color *color
 , int type);

void EG_Draw_TabBorder(SDL_Surface *surface, SDL_Rect *area, SDL_Color *color
 , int type);

SDL_Rect EG_Draw_CalcDrawingArea(SDL_Surface *surface, SDL_Rect *update);
void EG_Draw_Box(SDL_Surface *surface, SDL_Rect *area, SDL_Color *color);
Uint32 EG_Draw_CalcTimePassed(Uint32 starttime, Uint32 endtime);
*/

int InitializeSDLSound(int soundfrequency);
void FreeSDLSound(void);


//static EG_BOOL EG_DrawString_Initialise(void);
//static void EG_DrawString_Free(void);


#endif
