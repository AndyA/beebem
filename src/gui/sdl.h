#ifndef GUI_SDL_HEADER
#define GUI_SDL_HEADER

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <SDL.h>

#include <gui/types.h>

#define EG_Draw_Border_Normal	0
#define EG_Draw_Border_BoxHigh 	1
#define EG_Draw_Border_BoxLow 	2
#define EG_Draw_Border_Focused	3

/* Screen size (in virtual resolution - all may not be usable):
 */
#define SCREEN_WIDTH 		640
#define SCREEN_HEIGHT 		512

/* GUI colors.
 */

#define EG_COLOR_NORMAL		( (SDL_Color) {191, 191, 191, 0} )
#define EG_COLOR_ALT		( (SDL_Color) {191, 0, 0, 0} )

/* GUI Window max size (in virtual resolution):
 */
#define EG_WINDOW_MAX_WIDTH	480
#define EG_WINDOW_MAX_HEIGHT	480

/* Default GUI colors
 */
#define MENU_COLORS CalcColor(127+64, 127+64, 127+64)
#define TITLE_COLORS CalcColor(255,0,0)


/* Convert SDL_Rect to text character positions (in virtual resolution):
 */
#define LOC(c, r, w, h) ( (SDL_Rect) {(c)*10, (r)*16, (w)*10, (h)*16} )

#ifdef __cplusplus
extern "C" {
#endif

EG_BOOL EG_Initialize(void);
void EG_Quit(void);

Uint32 EG_Draw_GetCurrentTime(void);
Uint32 EG_Draw_GetTimePassed(Uint32 time_start);

void EG_Draw_FlushEventQueue(void);

float EG_Draw_GetScale(void);
void EG_Draw_SetToLowResolution(void);
void EG_Draw_SetToHighResolution(void);
//void EG_Draw_UpdateSurface(SDL_Surface *surface, Sint32 x, Sint32 y, Sint32 w
// , Sint32 h);
void EG_Draw_UpdateSurface(SDL_Surface *surface, SDL_Rect area);


void EG_Draw_Char(SDL_Surface *surface, SDL_Color *color, EG_BOOL bold, Uint16 x, Uint16 y
 , char c);

void EG_Draw_String(SDL_Surface *surface, SDL_Color *colour, EG_BOOL bold, SDL_Rect *area_ptr
 , int justify, char* string);

void EG_Draw_Border(SDL_Surface *surface, SDL_Rect *area, SDL_Color *color
 , int type);

void EG_Draw_Toggle(SDL_Surface *surface, SDL_Rect *area, SDL_Color *color
 , int type);

void EG_Draw_TabBorder(SDL_Surface *surface, SDL_Rect *area, SDL_Color *color
 , int type);

SDL_Rect EG_Draw_CalcDrawingArea(SDL_Surface *surface, SDL_Rect *update);
void EG_Draw_Box(SDL_Surface *surface, SDL_Rect *area, SDL_Color *color);
Uint32 EG_Draw_CalcTimePassed(Uint32 starttime, Uint32 endtime);

void EG_Draw_Disable_Update(void);
void EG_Draw_Enable_Update(void);

SDL_Rect CalcRectCentered(int width, int height);
SDL_Rect CalcRect(int x, int y, int w, int h);
SDL_Color CalcColor(int r, int g, int b);

void PutPixel(SDL_Surface *surface, Uint16 x, Uint16 y, Uint32 color);

#ifdef __cplusplus
}
#endif



#endif
