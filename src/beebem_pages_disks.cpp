/* START OF beebem_pages_disks.cpp --------------------------------------------
 *
 */


#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include "log.h"

#include "sdl.h"
//#include "include/gui_functions.h"

#include <gui.h>

#include <SDL.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 512

static char *about2[1000];

static int return_value;

static long about_lines;
static EG_Window *window_ptr;
static int d;


static EG_Widget *widget_about[25];
static SDL_Rect widget_loc[25];

static SDL_Rect scrolling_area;

static EG_Widget *v_slider;

#define MENU_COLORS CalcColor(127+64, 127+64, 127+64)
#define TITLE_COLORS CalcColor(255,0,0)




/* END OF beebem_pages_disks.cpp ----------------------------------------------
 */
