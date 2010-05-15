
#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <gui/sdl.h>

#include <gui/line.h>
#include <gui/log.h>
#include <gui/types.h>

static EG_BOOL EG_DrawString_Initialise(void);
static void EG_DrawString_Free(void);

/* Rendering scale 1 = 100%, 0.5 = 50%
 */
static float scale = 1;
static EG_BOOL no_update = EG_FALSE;

/* Initialize GUI. Must be called before any other EG function call.
 */
EG_BOOL EG_Initialize(void)
{	
//	/* Initialize event handler:
//	 */
//	if (EG_Event_InitializeQueue() == EG_FALSE)
//		return(EG_FALSE);
//
//	/* Load label fonts:
//	 */

	EG_Draw_FlushEventQueue();

	if (EG_DrawString_Initialise() == EG_FALSE)
		return(EG_FALSE);


	return(EG_TRUE);
}

void EG_Quit(void)
{
//	EG_Event_FreeQueue();
	EG_DrawString_Free();
}


/* Rendering scale (1 = 100%, 0.5 = 50%)
 */
void EG_Draw_SetToLowResolution(void)
{
        scale=0.5;
}

void EG_Draw_SetToHighResolution(void)
{
        scale=1.0;
}

/* Get scale (divide coordinates by this value to calculate the actual size
 * of anything physically drawn on the surface).
 */
float EG_Draw_GetScale(void)
{
        return(scale);
}


Uint32 EG_Draw_GetCurrentTime(void)
{
	return(SDL_GetTicks());
}

Uint32 EG_Draw_GetTimePassed(Uint32 time_start)
{
	Uint32 time_end = SDL_GetTicks();

        if (time_end < time_start)
                return(time_end + ( ((Uint32) 0xffffffff) - time_start) );
        else
                return(time_end - time_start);
}

void EG_Draw_FlushEventQueue(void)
{
	SDL_Event event;
	while ( SDL_PollEvent(&event) ){}
}


/* Wrapper for SDL surface update
 *
 * Should call a user callback so program knows the GUI has updated something
 */
static void EG_Draw_UpdateSurfaceAbsolute(SDL_Surface *surface, Sint32 x, Sint32 y, Sint32 w
 , Sint32 h)
{
	if (no_update == EG_FALSE /* || scale != 1 */)
		SDL_UpdateRect(surface, x, y, w, h);
}

void EG_Draw_UpdateSurface(SDL_Surface *surface, SDL_Rect area)
{
	area.x = (int) (area.x * EG_Draw_GetScale() );
	area.y = (int) (area.y * EG_Draw_GetScale() );
	area.w = (int) (area.w * EG_Draw_GetScale() );
	area.h = (int) (area.h * EG_Draw_GetScale() );

	EG_Draw_UpdateSurfaceAbsolute(surface, area.x, area.y, area.w, area.h);
}


void EG_Draw_Disable_Update(void)
{
	no_update = EG_TRUE;
}

void EG_Draw_Enable_Update(void)
{
	no_update = EG_FALSE;
}

/* Text functions */
static SDL_Surface *label_low = NULL, *label_high = NULL;

static EG_BOOL EG_DrawString_Initialise(void)
{	//int i;
	char largefont[1024];
        char smallfont[1024];

        char cpath[]={DATA_DIR};

        strcpy(largefont, cpath);
        if (cpath[strlen(cpath)] != '/')
                        strcat(largefont, "/");

        strcpy(smallfont, largefont);

        strcat(largefont, "resources/font10x16.bmp");
        strcat(smallfont, "resources/font5x8.bmp");

        label_low = SDL_LoadBMP(smallfont);
        label_high = SDL_LoadBMP(largefont);

        if (label_low == NULL || label_high == NULL){
                pFATAL(dL"Failed to load fonts.. (paths are: [%s] and [%s])\n"
		 , dR , largefont, smallfont);
                return(EG_FALSE);
        }

        // Set color key
	SDL_SetColorKey(label_low, SDL_SRCCOLORKEY
	 , SDL_MapRGB(label_low->format, 255,255,255));
	SDL_SetColorKey(label_high, SDL_SRCCOLORKEY
	 , SDL_MapRGB(label_high->format, 255,255,255));

	return(EG_TRUE);
}

static void EG_DrawString_Free(void)
{
        SDL_FreeSurface(label_low);
        SDL_FreeSurface(label_high);
}

// 0 = center, -1 = left. 1 = right
void EG_Draw_String(SDL_Surface *surface, SDL_Color *color, EG_BOOL bold, SDL_Rect *area_ptr
 , int justify, char *string)
{
	SDL_Rect drawing_area; // SDL_DW_Draw_CalcDrawingArea(surface, &area);

        int i, w, h, x, y, len;

	drawing_area = EG_Draw_CalcDrawingArea(surface, area_ptr);

	w = (int) (10 * EG_Draw_GetScale() );
	h = (int) (16 * EG_Draw_GetScale() );

	drawing_area.x = (int) (drawing_area.x * EG_Draw_GetScale() );
	drawing_area.y = (int) (drawing_area.y * EG_Draw_GetScale() );
	drawing_area.w = (int) (drawing_area.w * EG_Draw_GetScale() );
	drawing_area.h = (int) (drawing_area.h * EG_Draw_GetScale() );

        // Calc. number of letters we can actually draw.
        len = strlen(string);

        //SDL_FillRect(surface, &drawing_area, SDL_MapRGB(surface->format, 255
        // , 255, 255));

        x = drawing_area.x;
        y = drawing_area.y + ((drawing_area.h - h)/2);

        // Don't bother rendering anything if area width is less than one
	// character
        if ( w > drawing_area.w)
                return;

        // We will need to clip it:
        if ( (len * w) > drawing_area.w){
                len = (drawing_area.w / w) -1;
                x += (drawing_area.w % w) /2;

                for(i=0; i<len;i++){
                        EG_Draw_Char(surface, color, bold, x, y, string[i]);
                                x += w;
                }
                EG_Draw_Char(surface, color, bold, x, y, (char) 8);

        }else{
        // We don't need to clip it. But may need to center it:
                if (justify == 0)
                        x += (drawing_area.w - len * w) /2;
                else if (justify == 1)
                        x += (drawing_area.w - len * w);

                for(i=0; i<len;i++){
                        EG_Draw_Char(surface, color, bold, x, y, string[i]);
                                x += w;
                }
        }
        EG_Draw_UpdateSurfaceAbsolute(surface, drawing_area.x, drawing_area.y
	 , drawing_area.w, drawing_area.h);
}

void EG_Draw_Char(SDL_Surface *surface, SDL_Color *color, EG_BOOL bold, Uint16 x, Uint16 y
 , char c)
{
        SDL_Rect src, dst;
        SDL_Surface *source_surface;

	SDL_Color *tmp;		// Dump compiler warning
	tmp = color;

	src.x = c & 15;
	src.y = ((unsigned char) c) >> 4;

        if (EG_Draw_GetScale() >= 1){
                source_surface = label_high;
                src.w = 10; src.h = 16;
        }else{
                source_surface = label_low;
                src.w = 5; src.h = 8;
        }
        src.x *= src.w; src.y *= src.h;


        dst.x = x; dst.y = y;

        SDL_BlitSurface(source_surface, &src, surface, &dst);
	if (bold == EG_TRUE){
		dst.x++;
		SDL_BlitSurface(source_surface, &src, surface, &dst);
	}
}

/* If surface == NULL, returns an SDL_Rect of 0,0,0,0.  If update == NULL,
 * returns whole area of surface.  Otherwise returns update.
 */
SDL_Rect EG_Draw_CalcDrawingArea(SDL_Surface *surface, SDL_Rect *update)
{
        SDL_Rect area = {0,0,0,0};

        if (surface == NULL)
                return(area);

        if (update == NULL){
                area.w = surface->w;
                area.h = surface->h;
                area.x = 0;
                area.y = 0;
        }else{
                area = *(update);
        }

        return(area);
}

void EG_Draw_Box(SDL_Surface *surface, SDL_Rect *area, SDL_Color *color)
{       
	SDL_Rect drawing_area = EG_Draw_CalcDrawingArea(surface, area);
      
        if (surface == NULL)
                return;
        
	drawing_area.x = (int) (drawing_area.x * EG_Draw_GetScale() );
	drawing_area.y = (int) (drawing_area.y * EG_Draw_GetScale() );
	drawing_area.w = (int) (drawing_area.w * EG_Draw_GetScale() );
	drawing_area.h = (int) (drawing_area.h * EG_Draw_GetScale() );

        SDL_FillRect(surface, &drawing_area, SDL_MapRGB(surface->format
	 , color->r, color->g, color->b));

	EG_Draw_UpdateSurfaceAbsolute(surface, drawing_area.x, drawing_area.y
	 , drawing_area.w, drawing_area.h);
}


void EG_Draw_TabBorder(SDL_Surface *surface, SDL_Rect *area, SDL_Color *color
 , int type)
{
	SDL_Rect drawing_area = EG_Draw_CalcDrawingArea(surface, area);
        SDL_Rect line = {0,0,0,0};
        Uint32 bright_col = 0, dull_col = 0;

        if (surface == NULL)
                return;

        drawing_area.x = (int) (drawing_area.x * EG_Draw_GetScale() );
        drawing_area.y = (int) (drawing_area.y * EG_Draw_GetScale() );
        drawing_area.w = (int) (drawing_area.w * EG_Draw_GetScale() );
        drawing_area.h = (int) (drawing_area.h * EG_Draw_GetScale() );

	if (drawing_area.w <2) return;
	if (drawing_area.h <2) return;

	switch (type){

		case EG_Draw_Border_Normal:
			bright_col = SDL_MapRGB(surface->format
			 , (Uint8) color->r
			 , (Uint8) color->g
			 , (Uint8) color->b );
			
			dull_col = bright_col;	
		break;

		case EG_Draw_Border_BoxHigh:
			bright_col = SDL_MapRGB(surface->format
			 , (int) (1.3333*color->r >255.0 ? 255.0 : 1.3333*color->r)
			 , (int) (1.3333*color->g >255.0 ? 255.0 : 1.3333*color->g)
			 , (int) (1.3333*color->b >255.0 ? 255.0 : 1.3333*color->b) );
	
			dull_col = SDL_MapRGB(surface->format
			 , (int) (color->r * 0.6666)
			 , (int) (color->g * 0.6666)
			 , (int) (color->b * 0.6666) );
		break;

		case EG_Draw_Border_BoxLow:
//			dull_col = SDL_MapRGB(surface->format
//			 , (int) (1.3333*color->r >255.0 ? 255.0 : 1.3333*color->r)
//			 , (int) (1.3333*color->g >255.0 ? 255.0 : 1.3333*color->g)
//			 , (int) (1.3333*color->b >255.0 ? 255.0 : 1.3333*color->b) );
//	
			bright_col = SDL_MapRGB(surface->format
			 , (int) (color->r * 0.6666)
			 , (int) (color->g * 0.6666)
			 , (int) (color->b * 0.6666) );

			dull_col = bright_col;
		break;

		case EG_Draw_Border_Focused:

                        dull_col = SDL_MapRGB(surface->format
                         , (int) (color->r * 0.4)
                         , (int) (color->g * 0.4)
                         , (int) (color->b * 0.4) );

			bright_col = dull_col;
		break;

	}

        // Top line:
        line.x = drawing_area.x  +1  ; line.y = drawing_area.y; 
	line.w = drawing_area.w  -2  ; line.h = 1;
        SDL_FillRect(surface, &line, bright_col);

        // Bottom line:
        line.x = drawing_area.x  +1  ; line.y = drawing_area.y + drawing_area.h-1;
	line.w = drawing_area.w  -2  ; 
	line.h = 1;
 //       SDL_FillRect(surface, &line, dull_col);

        // Left line:
        line.x=drawing_area.x; line.y=drawing_area.y  +1  ; line.h=drawing_area.h  -2;
	line.w = 1;
  	line.h+=1;
        SDL_FillRect(surface, &line, bright_col);
	line.h-=1;

        // Right line:
        line.x=drawing_area.x+drawing_area.w-1; line.y=drawing_area.y  +1  ;
	line.w=1; line.h=drawing_area.h  -2  ;

//	line.h++;
        SDL_FillRect(surface, &line, dull_col);
//	line.h--;

	// Update: (if I was smart, I'd only draw the actual lines..)
        EG_Draw_UpdateSurfaceAbsolute(surface, drawing_area.x, drawing_area.y
         , drawing_area.w, drawing_area.h+2);
}




void EG_Draw_Border(SDL_Surface *surface, SDL_Rect *area, SDL_Color *color
 , int type)
{
	SDL_Rect drawing_area = EG_Draw_CalcDrawingArea(surface, area);
        SDL_Rect line = {0,0,0,0};
        Uint32 bright_col = 0, dull_col = 0;

        if (surface == NULL)
                return;

        drawing_area.x = (int) ( drawing_area.x * EG_Draw_GetScale() );
        drawing_area.y = (int) ( drawing_area.y * EG_Draw_GetScale() );
        drawing_area.w = (int) ( drawing_area.w * EG_Draw_GetScale() );
        drawing_area.h = (int) ( drawing_area.h * EG_Draw_GetScale() );

	if (drawing_area.w <2) return;
	if (drawing_area.h <2) return;

	switch (type){

		case EG_Draw_Border_Normal:
			bright_col = SDL_MapRGB(surface->format
			 , (Uint8) color->r
			 , (Uint8) color->g
			 , (Uint8) color->b );
			
			dull_col = bright_col;	
		break;

		case EG_Draw_Border_BoxHigh:
			bright_col = SDL_MapRGB(surface->format
			 , (int) (1.3333*color->r >255.0 ? 255.0 : 1.3333*color->r)
			 , (int) (1.3333*color->g >255.0 ? 255.0 : 1.3333*color->g)
			 , (int) (1.3333*color->b >255.0 ? 255.0 : 1.3333*color->b) );
	
			dull_col = SDL_MapRGB(surface->format
			 , (int) (color->r * 0.6666)
			 , (int) (color->g * 0.6666)
			 , (int) (color->b * 0.6666) );
		break;

		case EG_Draw_Border_BoxLow:
			dull_col = SDL_MapRGB(surface->format
			 , (int) (1.3333*color->r >255.0 ? 255.0 : 1.3333*color->r)
			 , (int) (1.3333*color->g >255.0 ? 255.0 : 1.3333*color->g)
			 , (int) (1.3333*color->b >255.0 ? 255.0 : 1.3333*color->b) );
	
			bright_col = SDL_MapRGB(surface->format
			 , (int) (color->r * 0.6666)
			 , (int) (color->g * 0.6666)
			 , (int) (color->b * 0.6666) );
		break;

		case EG_Draw_Border_Focused:
                        dull_col = SDL_MapRGB(surface->format
                         , (int) (color->r * 0.7)
                         , (int) (color->g * 0.7)
                         , (int) (color->b * 0.7) );

                        bright_col = dull_col;
		break;

	}

        // Top line:
        line.x = drawing_area.x  +1  ; line.y = drawing_area.y; 
	line.w = drawing_area.w  -2  ; line.h = 1;
        SDL_FillRect(surface, &line, bright_col);

        // Bottom line:
        line.x = drawing_area.x  +1  ; line.y = drawing_area.y + drawing_area.h-1;
	line.w = drawing_area.w  -2  ; 
	line.h = 1;
        SDL_FillRect(surface, &line, dull_col);

        // Left line:
        line.x=drawing_area.x; line.y=drawing_area.y  +1  ; line.h=drawing_area.h  -2  ;
	line.w = 1;
        SDL_FillRect(surface, &line, bright_col);

        // Right line:
        line.x=drawing_area.x+drawing_area.w-1; line.y=drawing_area.y  +1  ;
	line.w=1; line.h=drawing_area.h  -2  ;
        SDL_FillRect(surface, &line, dull_col);

	// Update: (if I was smart, I'd only draw the actual lines..)
        EG_Draw_UpdateSurfaceAbsolute(surface, drawing_area.x, drawing_area.y
         , drawing_area.w, drawing_area.h);
}


void EG_Draw_Toggle(SDL_Surface *surface, SDL_Rect *area, SDL_Color *color
 , int type)
{
	SDL_Rect drawing_area = EG_Draw_CalcDrawingArea(surface, area);
        SDL_Rect line = {0,0,0,0};
        Uint32 normal_col = 0, bright_col = 0, dull_col = 0;

        if (surface == NULL)
                return;

        drawing_area.x = (int) ( drawing_area.x * EG_Draw_GetScale() );
        drawing_area.y = (int) ( drawing_area.y * EG_Draw_GetScale() );
        drawing_area.w = (int) ( drawing_area.w * EG_Draw_GetScale() );
        drawing_area.h = (int) ( drawing_area.h * EG_Draw_GetScale() );

	if (drawing_area.w <2) return;
	if (drawing_area.h <2) return;

	normal_col = SDL_MapRGB(surface->format, color->r, color->g, color->b);

	switch (type){

		case EG_Draw_Border_Normal:
			bright_col = SDL_MapRGB(surface->format
			 , (Uint8) color->r
			 , (Uint8) color->g
			 , (Uint8) color->b );
			
			dull_col = bright_col;	
		break;

		case EG_Draw_Border_BoxHigh:
			bright_col = SDL_MapRGB(surface->format
			 , (int) (1.3333*color->r >255.0 ? 255.0 : 1.3333*color->r)
			 , (int) (1.3333*color->g >255.0 ? 255.0 : 1.3333*color->g)
			 , (int) (1.3333*color->b >255.0 ? 255.0 : 1.3333*color->b) );
	
			dull_col = SDL_MapRGB(surface->format
			 , (int) (color->r * 0.6666)
			 , (int) (color->g * 0.6666)
			 , (int) (color->b * 0.6666) );
		break;

		case EG_Draw_Border_BoxLow:
			dull_col = SDL_MapRGB(surface->format
			 , (int) (1.3333*color->r >255.0 ? 255.0 : 1.3333*color->r)
			 , (int) (1.3333*color->g >255.0 ? 255.0 : 1.3333*color->g)
			 , (int) (1.3333*color->b >255.0 ? 255.0 : 1.3333*color->b) );
	
			bright_col = SDL_MapRGB(surface->format
			 , (int) (color->r * 0.6666)
			 , (int) (color->g * 0.6666)
			 , (int) (color->b * 0.6666) );
		break;

		case EG_Draw_Border_Focused:
                        dull_col = SDL_MapRGB(surface->format
                         , (int) (color->r * 0.7)
                         , (int) (color->g * 0.7)
                         , (int) (color->b * 0.7) );

                        bright_col = dull_col;
		break;

	}

	int corner;
	corner = 4 * EG_Draw_GetScale();

	Uint32 fill_col;

//	if (type == EG_Draw_Border_BoxLow)
//		fill_col = SDL_MapRGB(surface->format,127+64, 127+64-32, 127+64-32);
//	else
		fill_col = normal_col;

	// Fill
	line.x = drawing_area.x; line.y = drawing_area.y;
	line.w = drawing_area.w; line.h = drawing_area.h - corner;
	SDL_FillRect(surface, &line, fill_col);

	line.x = drawing_area.x; line.y=drawing_area.y+drawing_area.h-corner;
	line.w = drawing_area.w; line.h=1;

	line.x++; line.w-=2;
	SDL_FillRect(surface, &line, fill_col);

	line.x++; line.w-=2; line.y++;
	SDL_FillRect(surface, &line, fill_col);

	line.x++; line.w-=2; line.y++;
	SDL_FillRect(surface, &line, fill_col);



        // Top line:
        line.x = drawing_area.x  +1  ; line.y = drawing_area.y; 
	line.w = drawing_area.w  -2  ; line.h = 1;
        SDL_FillRect(surface, &line, bright_col);

        // Bottom line:
        line.x = drawing_area.x  +1+corner  ; line.y = drawing_area.y + drawing_area.h-1;
	line.w = drawing_area.w  -2-(corner*2)  ; 
	line.h = 1;
        SDL_FillRect(surface, &line, dull_col);

        // Left line:
        line.x=drawing_area.x; line.y=drawing_area.y  +1  ; line.h=drawing_area.h  -2-corner  ;
	line.w = 1;
        SDL_FillRect(surface, &line, bright_col);

        // Right line:
        line.x=drawing_area.x+drawing_area.w-1; line.y=drawing_area.y  +1  ;
	line.w=1; line.h=drawing_area.h  -2-corner;
        SDL_FillRect(surface, &line, dull_col);


	Uint16 x, y;
	x = drawing_area.x;
	y = drawing_area.y+drawing_area.h-1;

	float s = EG_Draw_GetScale();

	PutPixel(surface,x,y-4*s,bright_col);
	PutPixel(surface,x+1*s, y-3*s, bright_col);
	PutPixel(surface,x+2*s, y-2*s, bright_col);
	PutPixel(surface,x+3*s, y-1*s, bright_col);
	PutPixel(surface,x+4*s, y, bright_col);

	x = drawing_area.x + drawing_area.w-1;
	
        PutPixel(surface,x,   y-4*s, dull_col);
        PutPixel(surface,x-1*s, y-3*s, dull_col);
        PutPixel(surface,x-2*s, y-2*s, dull_col);
        PutPixel(surface,x-3*s, y-1*s, dull_col);
        PutPixel(surface,x-4*s, y, dull_col);


        EG_Draw_UpdateSurfaceAbsolute(surface, drawing_area.x, drawing_area.y
         , drawing_area.w, drawing_area.h);
}














Uint32 EG_Draw_CalcTimePassed(Uint32 starttime, Uint32 endtime)
{
        if (starttime > endtime)
                return(0xffffffff - starttime + endtime);
        else
                return(endtime - starttime);
}




SDL_Rect CalcRectCentered(int width, int height)
{
        SDL_Rect tmp;

        tmp.x = (SCREEN_WIDTH-width)/2;
        tmp.y = (SCREEN_HEIGHT-height)/2;

        tmp.w = width;
        tmp.h = height;

        return(tmp);
}

SDL_Rect CalcRect(int x, int y, int w, int h)
{
        SDL_Rect tmp;
        tmp.x = x; tmp.y = y; tmp.w = w; tmp.h = h;
        return(tmp);
}

SDL_Color CalcColor(int r, int g, int b)
{
        SDL_Color tmp;
        tmp.r = r; tmp.g = g; tmp.b = b;
        tmp.unused = 0;
        return(tmp);
}


void PutPixel(SDL_Surface *surface, Uint16 x, Uint16 y, Uint32 color)
{
     //   Uint32 color = SDL_MapRGB(surface->format, col.r, col.g, col.b);

//        x *= EG_Draw_GetScale();
//        y *= EG_Draw_GetScale();

        if (x > surface->w-1) return;
        if (y > surface->h-1) return;

        switch (surface->format->BytesPerPixel){

        case 1: /* 8-bpp */
        {
        Uint8 *bufp;

        bufp = (Uint8 *)surface->pixels + y*surface->pitch + x;
        *bufp = color;
        }
        break;

    case 2: // Probably 15-bpp or 16-bpp
      {
        Uint16 *bufp;

        bufp = (Uint16 *)surface->pixels + y*surface->pitch/2 + x;
        *bufp = color;
      }
      break;
    case 3: // Slow 24-bpp mode, usually not used
      {
        Uint8 *bufp;

        bufp = (Uint8 *)surface->pixels + y*surface->pitch + x * 3;
        if(SDL_BYTEORDER == SDL_LIL_ENDIAN)
        {
          bufp[0] = color;
          bufp[1] = color >> 8;
          bufp[2] = color >> 16;
        } else {
          bufp[2] = color;
          bufp[1] = color >> 8;
          bufp[0] = color >> 16;
        }
      }
      break;
    case 4: // Probably 32-bpp
      {
        Uint32 *bufp;

        bufp = (Uint32 *)surface->pixels + y*surface->pitch/4 + x;
        *bufp = color;
      }
      break;
  }
}

