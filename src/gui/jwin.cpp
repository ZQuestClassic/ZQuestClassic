#include <ctype.h>
#include <cstring>
#include "base/zc_alleg.h"
#include <allegro/internal/aintern.h>
#include "gui/jwin.h"
#include "gui/editbox.h"
#include <iostream>
#include <sstream>
#include "base/zsys.h"
#include <stdio.h>
#include "base/util.h"
#include "pal.h"
#include "gui/tabpanel.h"
#include "gui/text_field.h"
#include "dialog/info.h"
#include "drawing.h"
using namespace util;
using std::string;
using std::istringstream;

void update_hw_screen(bool force);
extern int32_t zq_screen_w, zq_screen_h;
extern int32_t joystick_index;
int CheckerCol1 = 7, CheckerCol2 = 8;

extern bool is_editor();

int32_t abc_patternmatch = 1;

char abc_keypresses[1024] = {0};
void wipe_abc_keypresses() { memset(abc_keypresses, 0, 1024); }

/* these are provided for external use */
int32_t jwin_colors[jcMAX] =
{
    0xC0C0C0,0xF0F0F0,0xD0D0D0,0x808080,0x404040,0x000000,
    0x000080,0x00F0F0,0xFFFFFF,0xFFFFFF,0x000000,0x000080,0xFFFFFF
};

int32_t scheme[jcMAX] =
{
    0xC0C0C0,0xF0F0F0,0xD0D0D0,0x808080,0x404040,0x000000,
    0x000080,0x00F0F0,0xFFFFFF,0xFFFFFF,0x000000,0x000080,0xFFFFFF
};

int32_t jwin_pal[jcMAX] = {0};

// A pointer to this variable is used to identify the DIALOG belonging to
// the DialogRunner. It isn't used for anything else.
char newGuiMarker;

int32_t new_gui_event(DIALOG* d, guiEvent event)
{
	for(int32_t i = 0; true; --d, ++i)
	{
		if(d->dp3 == &newGuiMarker)
		{
			d->d1 = i;
			return d->proc(MSG_GUI_EVENT, d, event);
		}
	}

	return -1;
}

void close_new_gui_dlg(DIALOG* d);

int32_t bound(int32_t x,int32_t low,int32_t high)
{
    if(x<low) x=low;
    
    if(x>high) x=high;
    
    return x;
}
/*
  float bound(float x,float low,float high)
  {
  if(x<low) x=low;
  if(x>high) x=high;
  return x;
  }
  */

int32_t get_selected_tab(TABPANEL* panel)
{
	for(int32_t i=0; panel[i].text; ++i)
	{
		if((panel[i].flags&D_SELECTED)!=0)
			return i;
	}
	return -1;
}

/*  jwin_set_colors:
 *   Loads a set of colors in 0xRRGGBB or 256-color-indexed format
 *   into the current color scheme using the appropriate color depth
 *   conversions.
 */
void jwin_set_colors(int32_t *colors)
{
    int32_t i = 0;
    
    if(bitmap_color_depth(screen) == 8)
    {
        // use color indices
        for(; i<jcMAX; i++)
            scheme[i] = colors[i];
    }
    else
    {
        // 0xRRGGBB format
        for(; i<jcMAX; i++)
            scheme[i] = makecol(colors[i]>>16, (colors[i]>>8)&0xFF, colors[i]&0xFF);
    }
}

/* jwin_set_dialog_color:
  *  Sets the foreground and background colors of all the objects in a dialog.
  *
  *  Needs work!
  */
void jwin_set_dialog_color(DIALOG *dialog)
{
    int32_t c;
    
    for(c=0; dialog[c].proc; c++)
    {
        dialog[c].fg = scheme[jcMEDDARK];
        dialog[c].bg = scheme[jcBOX];
    }
}

/*  jwin_draw_frame:
  *   Draws a frame using the specified style.
  */
void jwin_draw_frame(BITMAP *dest,int32_t x,int32_t y,int32_t w,int32_t h,int32_t style)
{
	optional<int> c1,c2,c3,c4;
	
	switch(style)
	{
		case FR_INVIS:
			return;
		case FR_BOX:
			c1 = jcLIGHT;
			c2 = jcMEDLT;
			c3 = jcMEDDARK;
			c4 = jcDARK;
			break;
			
		case FR_INV:
			c1 = jcDARK;
			c2 = jcMEDDARK;
			c3 = jcMEDLT;
			c4 = jcLIGHT;
			break;
			
		case FR_DEEP:
			c1 = jcMEDDARK;
			c2 = jcDARK;
			c3 = jcMEDLT;
			c4 = jcLIGHT;
			break;
			
		case FR_DARK:
			c1 = jcDARK;
			c2 = jcMEDDARK;
			c3 = jcMEDDARK;
			c4 = jcDARK;
			break;
			
		case FR_ETCHED:
			c1 = jcMEDDARK;
			c2 = jcLIGHT;
			c3 = jcMEDDARK;
			c4 = jcLIGHT;
			break;
			
		case FR_MEDDARK:
			c1 = jcMEDDARK;
			c2 = jcBOX;
			c3 = jcBOX;
			c4 = jcMEDDARK;
			break;
		
		case FR_MENU:
			c1 = jcLIGHT;
			c4 = jcMEDDARK;
			break;
		case FR_MENU_INV:
			c1 = jcMEDDARK;
			c2 = jcMEDDARK;
			c3 = jcLIGHT;
			c4 = jcLIGHT;
			break;
			
		case FR_WIN:
		default:
			c1 = jcMEDLT;
			c2 = jcLIGHT;
			c3 = jcMEDDARK;
			c4 = jcDARK;
			break;
	}
	
	if(c1) c1 = scheme[*c1];
	if(c2) c2 = scheme[*c2];
	if(c3) c3 = scheme[*c3];
	if(c4) c4 = scheme[*c4];
	switch (style)
	{
		case FR_RED:
			c1 = 0xE4;
			c2 = 0xEC;
			c3 = 0xE4;
			c4 = 0xEC;
			break;
		case FR_GREEN:
			c1 = 0xE2;
			c2 = 0xEA;
			c3 = 0xE2;
			c4 = 0xEA;
			break;
	}
	if(c1)
	{
		_allegro_hline(dest, vbound(x,0,dest->w-1), vbound(y,0,dest->h-1)  , vbound(x+w-2, 0,dest->w-1), *c1);
		_allegro_vline(dest, vbound(x,0,dest->w-1), vbound(y+1,0,dest->h-1), vbound(y+h-2, 0, dest->h-1), *c1);
	}
	if(c2)
	{
		_allegro_hline(dest, vbound(x+1,0,dest->w-1), vbound(y+1,0,dest->h-1), vbound(x+w-3,0,dest->w-1), *c2);
		_allegro_vline(dest, vbound(x+1,0,dest->w-1), vbound(y+2,0,dest->h-1), vbound(y+h-3,0,dest->h-1), *c2);
	}
	if(c3)
	{
		_allegro_hline(dest, vbound(x+1,0,dest->w-1), vbound(y+h-2,0,dest->h-1), vbound(x+w-2,0,dest->w-1), *c3);
		_allegro_vline(dest, vbound(x+w-2,0,dest->w-1), vbound(y+1,0,dest->h-1), vbound(y+h-3,0,dest->h-1), *c3);
	}
	if(c4)
	{
	
		_allegro_hline(dest, vbound(x,0,dest->w-1), vbound(y+h-1,0,dest->h-1), vbound(x+w-1,0, dest->w-1), *c4);
		_allegro_vline(dest, vbound(x+w-1,0,dest->w-1), vbound(y,0,dest->h-1), vbound(y+h-2,0,dest->h-1), *c4);
	}
}
void jwin_draw_frag_frame(BITMAP* dest, int x1, int y1, int w, int h, int fw, int fh, int style)
{
    int c1,c2,c3,c4;
    
    switch(style)
    {
    case FR_BOX:
        c1 = jcLIGHT;
        c2 = jcMEDLT;
        c3 = jcMEDDARK;
        c4 = jcDARK;
        break;
        
    case FR_INV:
        c1 = jcDARK;
        c2 = jcMEDDARK;
        c3 = jcMEDLT;
        c4 = jcLIGHT;
        break;
        
    case FR_DEEP:
        c1 = jcMEDDARK;
        c2 = jcDARK;
        c3 = jcMEDLT;
        c4 = jcLIGHT;
        break;
        
    case FR_DARK:
        c1 = jcDARK;
        c2 = jcMEDDARK;
        c3 = jcMEDDARK;
        c4 = jcDARK;
        break;
        
    case FR_ETCHED:
        c1 = jcMEDDARK;
        c2 = jcLIGHT;
        c3 = jcMEDDARK;
        c4 = jcLIGHT;
        break;
        
    case FR_MEDDARK:
        c1 = jcMEDDARK;
        c2 = jcBOX;
        c3 = jcBOX;
        c4 = jcMEDDARK;
        break;
        
    case FR_WIN:
    default:
        c1 = jcMEDLT;
        c2 = jcLIGHT;
        c3 = jcMEDDARK;
        c4 = jcDARK;
        break;
    }
    
	int xc = x1+fw-1;
	int yc = y1+fh-1;
	int x2 = x1+w-1;
	int y2 = y1+h-1;
	
	rectfill(dest, x1, y1, x2, yc, vc(0));
	rectfill(dest, x1, yc, xc, y2, vc(0));
	
    _allegro_hline(dest, x1-2, y1-2, x2+2, scheme[c1]);
    _allegro_hline(dest, x1-1, y1-1, x2+1, scheme[c2]);
    
    _allegro_vline(dest, x1-2, y1-2, y2+2, scheme[c1]);
    _allegro_vline(dest, x1-1, y1-1, y2+1, scheme[c2]);
    
    _allegro_hline(dest, x1-2, y2+2, xc+2, scheme[c3]);
    _allegro_hline(dest, x1-1, y2+1, xc+1, scheme[c4]);
	
    _allegro_vline(dest, x2+2, y1-2, yc+2, scheme[c3]);
    _allegro_vline(dest, x2+1, y1-1, yc+1, scheme[c4]);
	
    _allegro_hline(dest, xc+2, yc+2, x2+2, scheme[c3]);
    _allegro_hline(dest, xc+1, yc+1, x2+1, scheme[c4]);
	
    _allegro_vline(dest, xc+2, yc+2, y2+2, scheme[c3]);
    _allegro_vline(dest, xc+1, yc+1, y2+1, scheme[c4]);
}
void jwin_draw_minimap_frame(BITMAP *dest,int x,int y,int w,int h,int scrsz,int style)
{
	jwin_draw_frag_frame(dest,x,y,w,h,scrsz*8,scrsz*8,style);
}

/*  jwin_draw_win:
  *   Draws a window -- a box with a frame.
  */
void jwin_draw_win(BITMAP *dest,int32_t x,int32_t y,int32_t w,int32_t h,int32_t frame)
{
	rectfill(dest,zc_max(x,0),zc_max(y,0),zc_min(x+w-1, dest->w-1),zc_min(y+h-1, dest->h-1),scheme[jcBOX]);
	jwin_draw_frame(dest, x, y, w, h, frame);
}

/*  jwin_draw_button:
  *   Helper function for buttons.
  *   Draws a box with a frame that depends on "state":
  *     0: normal border (slightly different than window border)
  *     1: inverted border
  *     2: dark border
  *     3: medium dark border
  */
void jwin_draw_button(BITMAP *dest,int32_t x,int32_t y,int32_t w,int32_t h,int32_t state,int32_t type)
{
    int32_t frame = FR_BOX;
    
    if(type==1)
    {
        frame=FR_WIN;
    }
    
    switch(state)
    {
    case 1:
        frame = FR_INV;
        break;
        
    case 2:
        frame = FR_DARK;
        break;
        
    case 3:
        frame = FR_MEDDARK;
        break;
    }
    
    jwin_draw_win(dest, x, y, w, h, frame);
}

/*  mix_value:
  *   Returns a mix of the values c1 and c2 with pos==0 being c1,
  *   pos==max being c2, pos==max/2 being half way between c1 and c2, etc.
  */
int32_t mix_value(int32_t c1,int32_t c2,int32_t pos,int32_t max)
{
    if(max<=0)
        return c1;
        
    return (c2 - c1) * pos / max + c1;
}

/*  mix_color:
  *   Returns a mix of the colors c1 and c2 with pos==0 being c1,
  *   pos==max being c2, pos==max/2 being half way between c1 and c2, etc.
  *
  static int32_t mix_color(int32_t c1,int32_t c2,int32_t pos,int32_t max)
  {
  int32_t c;

  if(bitmap_color_depth(screen) == 8)
  c = mix_value(c1, c2, pos, max);
  else
  {
  int32_t r = mix_value(getr(c1), getr(c2), pos, max);
  int32_t g = mix_value(getg(c1), getg(c2), pos, max);
  int32_t b = mix_value(getb(c1), getb(c2), pos, max);
  c = makecol(r,g,b);
  }

  return c;
  }
  */

char *shorten_string(char *dest, char const* src, FONT *usefont, int32_t maxchars, int32_t maxwidth)
{
    strncpy(dest,src,maxchars);
    dest[maxchars-1]='\0';
    int32_t len=(int32_t)strlen(dest);
    int32_t width=text_length(usefont, dest);
    dest[len]=0;
    
    while(width>maxwidth && len>4)
    {
        dest[len-4] = '.';
        dest[len-3] = '.';
        dest[len-2] = '.';
        dest[len-1] = 0;
        len--;
        width=text_length(usefont, dest);
    }
    
    return dest;
}

void jwin_draw_titlebar(BITMAP *dest, int32_t x, int32_t y, int32_t w, int32_t h, const char *str, bool draw_button, bool helpbtn)
{
    char buf[512];
    int32_t len = (int32_t)strlen(str);
    int32_t length = text_length(font,str);
    int32_t height = text_height(font);
    
    int32_t tx = x + 2;
    int32_t ty = y + (h-height)/2;
    PALETTE temp_pal;
    get_palette(temp_pal);
    dither_rect(dest, &temp_pal, x, y, x+w-1, y+h-1,
                makecol15(temp_pal[scheme[jcTITLEL]].r*255/63,
                          temp_pal[scheme[jcTITLEL]].g*255/63,
                          temp_pal[scheme[jcTITLEL]].b*255/63),
                makecol15(temp_pal[scheme[jcTITLER]].r*255/63,
                          temp_pal[scheme[jcTITLER]].g*255/63,
                          temp_pal[scheme[jcTITLER]].b*255/63),
                scheme[jcTITLEL], scheme[jcTITLER]);
                
                
    if(len>509)
        len=509;
        
    strncpy(buf,str,len);
    buf[len]=0;
    
    // this part needs work
    
    if(length>w-20)
    {
        while(length>w-20 && len>1)
        {
            buf[len-4] = '.';
            buf[len-3] = '.';
            buf[len-2] = '.';
            buf[len-1] = 0;
            len--;
            length = text_length(font,buf);
        }
    }
    
    textout_ex(dest,font,buf,tx,ty,scheme[jcTITLEFG],-1);
    
    if(draw_button)
    {
        draw_x_button(dest, x + w - 18, y+2, 0);
    }
	
	if(helpbtn)
	{
		draw_question_button(dest, x + w - (draw_button ? 36 : 18), y+2, 0);
	}
    
}

void draw_question_button(BITMAP* dest, int32_t x, int32_t y, int32_t state)
{
    int32_t c = scheme[jcBOXFG];
    
    jwin_draw_button(dest,x,y,16,14,state,0);
    x += 4 + (state?1:0);
    y += 3 + (state?1:0);
    
    line(dest, x+2, y+0, x+5, y+0, c);
    line(dest, x+1, y+1, x+2, y+1, c);
    line(dest, x+5, y+1, x+6, y+1, c);
    line(dest, x+4, y+2, x+5, y+2, c);
    line(dest, x+3, y+3, x+4, y+3, c);
    line(dest, x+3, y+4, x+4, y+4, c);
    line(dest, x+3, y+6, x+4, y+6, c);
    line(dest, x+3, y+7, x+4, y+7, c);
}

void draw_x_button(BITMAP *dest, int32_t x, int32_t y, int32_t state)
{
    int32_t c = scheme[jcBOXFG];
    
    jwin_draw_button(dest,x,y,16,14,state,0);
    x += 4 + (state?1:0);
    y += 3 + (state?1:0);
    
    line(dest,x,  y,  x+6,y+6,c);
    line(dest,x+1,y,  x+7,y+6,c);
    line(dest,x,  y+6,x+6,y,  c);
    line(dest,x+1,y+6,x+7,y,  c);
}

void draw_arrow(BITMAP *dest, int c, int x, int y, int h, bool up, bool center)
{
	if(!center)
		x += h-1;
	for(int i = 0; i<h; i++)
		_allegro_hline(dest, x-(up?i:h-i-1), y+i, x+(up?i:h-i-1), c);
}
void draw_arrow_horz(BITMAP *dest, int c, int x, int y, int w, bool left, bool center)
{
	if(!center)
		y += w-1;
	for(int i = 0; i<w; i++)
		_allegro_vline(dest, x+i, y-(left?i:w-i-1), y+(left?i:w-i-1), c);
}
void draw_arrow_button(BITMAP *dest, int32_t x, int32_t y, int32_t w, int32_t h, int32_t up, int32_t state)
{
    int32_t c = scheme[jcDARK];
    int32_t ah = zc_min(h/3, 5);
    int32_t i = 0;
    
    jwin_draw_button(dest,x,y,w,h,state,1);
    x += w/2 - (state?0:1);
    y += (h-ah)/2 + (state?1:0);
    
    for(; i<ah; i++)
    {
        _allegro_hline(dest, x-(up?i:ah-i-1), y+i, x+(up?i:ah-i-1), c);
    }
}

void draw_arrow_button_horiz(BITMAP *dest, int32_t x, int32_t y, int32_t w, int32_t h, int32_t up, int32_t state)
{
    int32_t c = scheme[jcDARK];
    int32_t aw = zc_min(w/3, 5);
    int32_t i = 0;
    
    jwin_draw_button(dest,x,y,w,h,state,1);
    y += h/2 - (state?0:1);
    x += (w-aw)/2 + (state?1:0);
    
    for(; i<aw; i++)
    {
        _allegro_vline(dest, x+i,y-(up?i:aw-i-1), y+(up?i:aw-i-1), c);
    }
}

int32_t mouse_in_rect(int32_t x,int32_t y,int32_t w,int32_t h)
{
    return ((gui_mouse_x() >= x) && (gui_mouse_y() >= y) &&
            (gui_mouse_x() < x + w) && (gui_mouse_y() < y + h));
}

static int32_t jwin_do_x_button(BITMAP *dest, int32_t x, int32_t y)
{
    int32_t down=0, last_draw = 0;
    
    while(gui_mouse_b())
    {
        down = mouse_in_rect(x,y,16,14);
        
        if(down!=last_draw)
        {
            draw_x_button(dest,x,y,down);
            last_draw = down;
        }
        
        /* let other objects continue to animate */
        broadcast_dialog_message(MSG_IDLE, 0);
        rest(1);
    }
    
    if(down)
    {
        draw_x_button(dest,x,y,0);
    }
    
    return down;
}

/* dotted_rect:
  *  Draws a dotted rectangle, for showing an object has the input focus.
  */
void dotted_rect(BITMAP *dest, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t fg, int32_t bg)
{
    int32_t x = ((x1+y1) & 1) ? 1 : 0;
    int32_t c;
    
    //acquire the bitmap ourselves, because locking a surface within each call is *SLOW AS BALLS* -DD
    acquire_bitmap(dest);
    
    for(c=x1; c<=x2; c++)
    {
        putpixel(dest, c, y1, (((c+y1) & 1) == x) ? fg : bg);
        putpixel(dest, c, y2, (((c+y2) & 1) == x) ? fg : bg);
    }
    
    for(c=y1+1; c<y2; c++)
    {
        putpixel(dest, x1, c, (((c+x1) & 1) == x) ? fg : bg);
        putpixel(dest, x2, c, (((c+x2) & 1) == x) ? fg : bg);
    }
    
    release_bitmap(dest);
    
}

static void _dotted_rect(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t fg, int32_t bg)
{
    dotted_rect(screen, x1, y1, x2, y2, fg, bg);
}

static bool no_hline = false;
/* gui_textout_ln:
  *  Wrapper function for drawing text to the screen, which interprets the
  *  & character as an underbar for displaying keyboard shortcuts. Returns
  *  the width of the output string in pixels.
  *
  *  Handles '\n' characters.
  */
int32_t gui_textout_ln(BITMAP *bmp, FONT *f, unsigned const char *s, int32_t x, int32_t y, int32_t color, int32_t bg, int32_t pos)
{
    char tmp[1024];
    int32_t c = 0;
    int32_t len;
    int32_t pix_len = 0;
    int32_t max_len = 0;
    int32_t hline_pos;
    int32_t xx = x;
	bool is_scr = bmp == screen;
    
	while(s[c])
    {
        len = 0;
        hline_pos = -1;
        
        for(; (s[c]) && (len<(int32_t)(sizeof(tmp)-1)); c++)
        {
            if(s[c] == '\n')
            {
                c++;
                break;
            }
            else if(!no_hline && s[c] == '&')
            {
                if(s[c+1] != '&')
                    hline_pos = len;
                else
                {
                    tmp[len++] = '&';
                    c++;
                }
            }
            else
                tmp[len++] = s[c];
        }
        
        tmp[len] = 0;
        pix_len = text_length(f, tmp);
        if (pix_len > max_len) max_len = pix_len;
        x = xx;
        
        if(pos==1)  //center
        {
            x -= pix_len / 2;
        }
        else if(pos==2)  //right
        {
            x -= pix_len;
        }
        
        if(bmp)
        {
            textout_ex(bmp, f, tmp, x, y, color,bg);
            
            if(hline_pos >= 0)
            {
                int32_t i;
                i = tmp[hline_pos];
                tmp[hline_pos] = 0;
                hline_pos = text_length(f, tmp);
                tmp[0] = i;
                tmp[1] = 0;
                i = text_length(f, tmp);
                _allegro_hline(bmp, x+hline_pos, y+text_height(f)-gui_font_baseline, x+hline_pos+i-1, color);
            }
        }
        
        y += text_height(f);
    }
    return max_len;
}

int32_t gui_textout_ln(BITMAP *bmp, unsigned const char *s, int32_t x, int32_t y, int32_t color, int32_t bg, int32_t pos)
{
    return gui_textout_ln(bmp, font, s, x, y, color, bg, pos);
}

int32_t gui_text_width(FONT *f, const char *s)
{
	return gui_textout_ln(NULL, f, (uint8_t*)s, 0, 0, 0, 0, 0);
}

int32_t count_newline(uint8_t *s)
{
	int32_t cnt = 0;
	for(int32_t q = 0; s[q] != 0; ++q)
	{
		if(s[q] == '\n') ++cnt;
	}
	return cnt;
}

int32_t gui_textheight(FONT* f, uint8_t *s)
{
	return text_height(f) * (count_newline(s) + 1);
}

int32_t gui_textheight(uint8_t* s)
{
	return gui_textheight(font, s);
}

/*******************************/
/*****  Misc Dialog Procs  *****/
/*******************************/

/* typedef for the listbox callback functions */
typedef char *(*getfuncptr)(int32_t, int32_t *);

/* event handler that closes a dialog */
int32_t close_dlg()
{
    return D_CLOSE;
}

int32_t jwin_frame_proc(int32_t msg, DIALOG *d, int32_t c)
{
    //these are here to bypass compiler warnings about unused arguments
    c=c;
    
    if(msg == MSG_DRAW)
    {
        jwin_draw_frame(screen, d->x, d->y, d->w, d->h, d->d1);
    }
    
    return D_O_K;
}

int32_t jwin_guitest_proc(int32_t msg, DIALOG *d, int32_t c)
{
    //these are here to bypass compiler warnings about unused arguments
    c=c;
    
    if(msg == MSG_DRAW)
    {
        jwin_draw_frame(screen, d->x, d->y, d->w, d->h, d->d1);
    }
    
    return D_O_K;
}

/* jwin_win_proc:
  *  Draws a box with a frame. Use dp for a title string. If dp is NULL,
  *  it won't draw a title bar. If the D_EXIT flag is set, it will also
  *  draw an "X" button on the title bar that can be used to close the
  *  dialog.
  *  If d->dp3 is non-null, it will be read as a help text string, and
  *  a ? button will be drawn, that upon clicking will display the helptext.
  */
int32_t jwin_win_proc(int32_t msg, DIALOG *d, int32_t c)
{
	//these are here to bypass compiler warnings about unused arguments
	c=c;
	
	rest(1);
	static bool skipredraw = false;
	
	switch(msg)
	{
		case MSG_DRAW:
			if(skipredraw)
			{
				skipredraw = false;
				break;
			}
			jwin_draw_win(screen, d->x, d->y, d->w, d->h, FR_WIN);
			
			if(d->dp)
			{
				FONT *oldfont = font;
				
				if(d->dp2)
				{
					font = (FONT*)d->dp2;
				}
				
				jwin_draw_titlebar(screen, d->x+3, d->y+3, d->w-6, 18, (char*)d->dp, d->flags & D_EXIT, d->dp3!=NULL);
				font = oldfont;
			}
			break;
		
		case MSG_WANTFOCUS:
			if(gui_mouse_b())
				return D_WANTFOCUS|D_REDRAW;
			else return D_O_K;
		case MSG_GOTFOCUS:
		case MSG_LOSTFOCUS:
			skipredraw = true;
			return D_O_K;
		
		case MSG_CLICK:
		{
			if((d->flags & D_EXIT) && mouse_in_rect(d->x+d->w-21, d->y+5, 16, 14))
			{
				if(jwin_do_x_button(screen, d->x+d->w-21, d->y+5))
				{
					GUI_EVENT(d, geCLOSE);
					return D_CLOSE;
				}
			}
			if(char const* helpstr = (char const*)d->dp3)
			{
				if(mouse_in_rect(d->x+d->w-((d->flags&D_EXIT)?39:21), d->y+5, 16, 14))
				{
					broadcast_dialog_message(MSG_DRAW,0);
					InfoDialog("Info", helpstr).show();
				}
			}
		}
		break;
	}
	
	return D_O_K;
}

/* jwin_text_proc:
  *  Simple dialog procedure: draws the text string which is pointed to by dp.
  *
  *  Handles '\n' characters.
  */
int32_t jwin_text_proc(int32_t msg, DIALOG *d, int32_t)
{
	ASSERT(d);
	static BITMAP *dummy=create_bitmap_ex(8, 1, 1);
	
	switch(msg)
	{
		case MSG_START:
		{
			FONT *oldfont = font;
			
			if(d->dp2)
			{
				font = (FONT*)d->dp2;
			}
			
			d->w=gui_textout_ln(dummy, (uint8_t *)d->dp, 0, 0, scheme[jcMEDDARK], -1, 0);
			d->h=gui_textheight((uint8_t *)d->dp);
			
			font = oldfont;
			break;
		}
		case MSG_DRAW:
		{
			FONT *oldfont = font;
			
			if(d->dp2)
			{
				font = (FONT*)d->dp2;
			}
			
			if(d->flags & D_DISABLED)
			{
				gui_textout_ln(screen, (uint8_t*)d->dp, d->x+1, d->y+1, scheme[jcLIGHT], scheme[jcBOX], 0);
				d->w=gui_textout_ln(screen, (uint8_t*)d->dp, d->x, d->y, scheme[jcDISABLED_FG], -1, 0);
			}
			else
			{
				d->w=gui_textout_ln(screen, (uint8_t*)d->dp, d->x, d->y, scheme[jcBOXFG], scheme[jcBOX], 0);
			}
			
			font = oldfont;
			break;
		}
	}
	
	return D_O_K;
}

int32_t jwin_ctext_proc(int32_t msg, DIALOG *d, int32_t)
{
    ASSERT(d);
    static BITMAP *dummy=create_bitmap_ex(8, 320, 240);
    
    switch(msg)
    {
    case MSG_START:
        d->w=gui_textout_ln(dummy, (uint8_t *)d->dp, 0, 0, scheme[jcMEDDARK], -1, 0);
        break;
        
    case MSG_DRAW:
        FONT *oldfont = font;
        
        if(d->dp2)
        {
            font = (FONT*)d->dp2;
        }
        
        if(d->flags & D_DISABLED)
        {
            gui_textout_ln(screen, (uint8_t*)d->dp, d->x+1, d->y+1, scheme[jcLIGHT], scheme[jcBOX], 1);
            gui_textout_ln(screen, (uint8_t*)d->dp, d->x, d->y, scheme[jcDISABLED_FG], -1, 1);
        }
        else
        {
            gui_textout_ln(screen, (uint8_t*)d->dp, d->x, d->y, scheme[jcBOXFG], scheme[jcBOX], 1);
        }
        
        font = oldfont;
        break;
    }
    
    return D_O_K;
}

int32_t jwin_rtext_proc(int32_t msg, DIALOG *d, int32_t)
{
    ASSERT(d);
    static BITMAP *dummy=create_bitmap_ex(8, 1, 1);
    
    switch(msg)
    {
    case MSG_START:
        d->w=gui_textout_ln(dummy, (uint8_t *)d->dp, 0, 0, scheme[jcMEDDARK], -1, 2);
        break;
        
    case MSG_DRAW:
        FONT *oldfont = font;
        
        if(d->dp2)
        {
            font = (FONT*)d->dp2;
        }
        
        if(d->flags & D_DISABLED)
        {
            gui_textout_ln(screen, (uint8_t*)d->dp, d->x+1, d->y+1, scheme[jcLIGHT], scheme[jcBOX], 2);
            gui_textout_ln(screen, (uint8_t*)d->dp, d->x, d->y, scheme[jcDISABLED_FG], -1, 2);
        }
        else
        {
            gui_textout_ln(screen, (uint8_t*)d->dp, d->x, d->y, scheme[jcBOXFG], scheme[jcBOX], 2);
        }
        
        font = oldfont;
        break;
    }
    
    return D_O_K;
}

int32_t d_ctext2_proc(int32_t msg, DIALOG *d, int32_t c)
{
	auto ret = d_ctext_proc(msg, d, c);
	return ret;
}

int32_t new_text_proc(int32_t msg, DIALOG *d, int32_t c)
{
	BITMAP* oldscreen = screen;
	if(msg==MSG_DRAW)
	{
		if(d->flags & D_HIDDEN) return D_O_K;
		screen = create_bitmap_ex(8,oldscreen->w,oldscreen->h);
		clear_bitmap(screen);
		set_clip_rect(screen, d->x, d->y, d->x+d->w-1, d->y+d->h-1);
	}
	int32_t ret = D_O_K;
	int32_t w = d->w, h = d->h, x = d->x, y = d->y;
	if(d->d2) no_hline = true;
	switch(d->d1)
	{
		case 0:
			ret = jwin_text_proc(msg, d, c);
			break;
		case 1:
			d->x += d->w/2;
			ret = jwin_ctext_proc(msg, d, c);
			break;
		case 2:
            d->x += d->w - 1;
			ret = jwin_rtext_proc(msg, d, c);
			break;
	}
	no_hline = false;
	d->w = w;
	d->h = h;
	d->x = x;
	d->y = y;
	if(msg==MSG_DRAW)
	{
		masked_blit(screen, oldscreen, d->x, d->y, d->x, d->y, d->w, d->h);
		destroy_bitmap(screen);
		screen = oldscreen;
	}
	if(msg==MSG_WANTFOCUS && gui_mouse_b())
		ret |= D_WANTFOCUS|D_REDRAW;
	return ret;
}

/* draw_text_button:
  *  Helper for jwin_button_proc.
  */
void jwin_draw_text_button(BITMAP *dest, int32_t x, int32_t y, int32_t w, int32_t h, const char *str, int32_t flags, bool show_dotted_rect)
{
    int32_t g = (flags & D_SELECTED) ? 1 : 0;
    
    if(flags & D_SELECTED)
	{
        jwin_draw_button(dest, x, y, w, h, 2, 0);
		flags &= ~D_DISABLED;
	}
    else if(!(flags & D_GOTFOCUS))
        jwin_draw_button(dest, x, y, w, h, 0, 0);
    else
    {
        rect(dest, x, y, x+w-1, y+h-1, scheme[jcDARK]);
        jwin_draw_button(dest, x+1, y+1, w-2, h-2, 0, 0);
    }
    
	bool drawstring = true;
	if(str[1]==0 && byte(str[0]) >= 0x80)
	{
		drawstring = false;
		int col = jwin_pal[(flags & D_DISABLED) ? jcLIGHT : jcBOXFG];
		int aw = w/4, ah = h/4;
		int woff = (aw/2)+1, hoff = (ah/2)+1;
		int x1 = x+w/2, x2 = x+(w-aw)/2;
		int y1 = y+(h-aw)/2, y2 = y+h/2;
		switch(byte(str[0]))
		{
			case 0x88:
				draw_arrow(dest, col, x1, y1, ah, true, true);
				break;
			case 0x89:
				draw_arrow(dest, col, x1, y1, ah, false, true);
				break;
			case 0x8A:
				draw_arrow_horz(dest, col, x2, y2, aw, true, true);
				break;
			case 0x8B:
				draw_arrow_horz(dest, col, x2, y2, aw, false, true);
				break;
			case 0x98:
				draw_arrow(dest, col, x1, y1-hoff, ah, false, true);
				draw_arrow(dest, col, x1, y1+hoff, ah, true, true);
				break;
			case 0x99:
				draw_arrow(dest, col, x1, y1-hoff, ah, true, true);
				draw_arrow(dest, col, x1, y1+hoff, ah, false, true);
				break;
			case 0x9A:
				draw_arrow_horz(dest, col, x2-woff, y2, aw, false, true);
				draw_arrow_horz(dest, col, x2+woff, y2, aw, true, true);
				break;
			case 0x9B:
				draw_arrow_horz(dest, col, x2-woff, y2, aw, true, true);
				draw_arrow_horz(dest, col, x2+woff, y2, aw, false, true);
				break;
			default: drawstring = true;
		}
	}
	if(drawstring)
	{
		if(!(flags & D_DISABLED))
			gui_textout_ex(dest, str, x+w/2+g, y+(h-text_height(font))/2+g, scheme[jcBOXFG], -1, TRUE);
		else
		{
			gui_textout_ex(dest, str, x+w/2+1,y+(h-text_height(font))/2+1, scheme[jcLIGHT], -1, TRUE);
			gui_textout_ex(dest, str, x+w/2,  y+(h-text_height(font))/2, scheme[jcDISABLED_FG], -1, TRUE);
		}
	}
    
    if(show_dotted_rect&&(flags & D_GOTFOCUS))
        dotted_rect(dest, x+4, y+4, x+w-5, y+h-5, scheme[jcDARK], scheme[jcBOX]);
}

int icon_proportion(int icon,int s1,int s2)
{
	int sz = round(sqrt(zc_min(s1,s2))*1.25);
	switch(icon)
	{
		case BTNICON_STOPSQUARE:
			sz += 4;
			break;
		case BTNICON_PLUS:
		case BTNICON_MINUS:
			sz += 4;
			break;
	}
	return sz;
}
void jwin_draw_icon_button(BITMAP *dest, int32_t x, int32_t y, int32_t w, int32_t h, int icon, int32_t flags, bool show_dotted_rect)
{
    int32_t g = (flags & D_SELECTED) ? 1 : 0;
    
    if(flags & D_SELECTED)
        jwin_draw_button(dest, x, y, w, h, 2, 0);
    else if(!(flags & D_GOTFOCUS))
        jwin_draw_button(dest, x, y, w, h, 0, 0);
    else
    {
        rect(dest, x, y, x+w-1, y+h-1, scheme[jcDARK]);
        jwin_draw_button(dest, x+1, y+1, w-2, h-2, 0, 0);
    }
    
	int col = jwin_pal[(flags & D_DISABLED) ? jcLIGHT : jcBOXFG];
	jwin_draw_icon(dest,x+w/2,y+h/2,col,icon,icon_proportion(icon,w,h),true);
	
    if(show_dotted_rect&&(flags & D_GOTFOCUS))
        dotted_rect(dest, x+4, y+4, x+w-5, y+h-5, scheme[jcDARK], scheme[jcBOX]);
}
void jwin_draw_icon(BITMAP *dest, int x, int y, int col, int icon, int asz, bool center)
{
	jwin_draw_icon(dest,x,y,col,icon,asz,asz,center);
}
void jwin_draw_icon(BITMAP *dest, int x, int y, int col, int icon, int aw, int ah, bool center)
{
	int w2 = aw, h2 = ah;
	int sz = zc_min(aw,ah);
	switch(icon)
	{
		case BTNICON_ARROW_LEFT2:
		case BTNICON_ARROW_RIGHT2:
			aw *= 2;
			ah = aw*2-1;
			break;
		case BTNICON_ARROW_LEFT3:
		case BTNICON_ARROW_RIGHT3:
			aw *= 3;
			ah = aw*2-1;
			break;
		case BTNICON_ARROW_UP:
		case BTNICON_ARROW_DOWN:
		case BTNICON_CONTRACT_VERT:
		case BTNICON_EXPAND_VERT:
			aw = ah*2-1;
			break;
		case BTNICON_ARROW_LEFT:
		case BTNICON_ARROW_RIGHT:
		case BTNICON_CONTRACT_HORZ:
		case BTNICON_EXPAND_HORZ:
			ah = aw*2-1;
			break;
		case BTNICON_STOPSQUARE:
			aw = ah = sz;
			break;
		case BTNICON_PLUS:
			if(!(sz%2)) ++sz;
			aw = ah = w2 = h2 = sz;
			w2 /= 3;
			h2 /= 3;
			if(!(h2%2)) ++h2;
			if(!(w2%2)) ++w2;
			break;
		case BTNICON_MINUS:
			if(!(sz%2)) ++sz;
			aw = ah = w2 = h2 = sz;
			h2 /= 3;
			if(!(h2%2)) ++h2;
			break;
	}
	int woff = (aw/2)+1, hoff = (ah/2)+1;
	int cx = center ? (x-aw/2) : x,
		cy = center ? (y-ah/2) : y;
	switch(icon)
	{
		case BTNICON_ARROW_UP:
			draw_arrow(dest, col, x, cy, ah, true, center);
			break;
		case BTNICON_ARROW_DOWN:
			draw_arrow(dest, col, x, cy, ah, false, center);
			break;
		case BTNICON_ARROW_LEFT:
			draw_arrow_horz(dest, col, cx, y, aw, true, center);
			break;
		case BTNICON_ARROW_RIGHT:
			draw_arrow_horz(dest, col, cx, y, aw, false, center);
			break;
		case BTNICON_CONTRACT_VERT:
			draw_arrow(dest, col, x, cy-hoff, ah, false, center);
			draw_arrow(dest, col, x, cy+hoff, ah, true, center);
			break;
		case BTNICON_EXPAND_VERT:
			draw_arrow(dest, col, x, cy-hoff, ah, true, center);
			draw_arrow(dest, col, x, cy+hoff, ah, false, center);
			break;
		case BTNICON_CONTRACT_HORZ:
			draw_arrow_horz(dest, col, cx-woff, y, aw, false, center);
			draw_arrow_horz(dest, col, cx+woff, y, aw, true, center);
			break;
		case BTNICON_EXPAND_HORZ:
			draw_arrow_horz(dest, col, cx-woff, y, aw, true, center);
			draw_arrow_horz(dest, col, cx+woff, y, aw, false, center);
			break;
		case BTNICON_ARROW_LEFT2:
			draw_arrow_horz(dest, col, cx, y, w2, true, center);
			draw_arrow_horz(dest, col, cx+w2, y, w2, true, center);
			break;
		case BTNICON_ARROW_LEFT3:
			draw_arrow_horz(dest, col, cx, y, w2, true, center);
			draw_arrow_horz(dest, col, cx+w2, y, w2, true, center);
			draw_arrow_horz(dest, col, cx+w2*2, y, w2, true, center);
			break;
		case BTNICON_ARROW_RIGHT2:
			draw_arrow_horz(dest, col, cx, y, w2, false, center);
			draw_arrow_horz(dest, col, cx+w2, y, w2, false, center);
			break;
		case BTNICON_ARROW_RIGHT3:
			draw_arrow_horz(dest, col, cx, y, w2, false, center);
			draw_arrow_horz(dest, col, cx+w2, y, w2, false, center);
			draw_arrow_horz(dest, col, cx+w2*2, y, w2, false, center);
			break;
		case BTNICON_STOPSQUARE:
			rectfill(dest, cx, cy, cx+aw-1, cy+ah-1, col);
			break;
		case BTNICON_MINUS:
			rectfill(dest, cx, cy+(ah/2)-(h2/2), cx+aw-1, cy+(ah/2)+(h2/2), col);
			break;
		case BTNICON_PLUS:
			rectfill(dest, cx, cy+(ah/2)-(h2/2), cx+aw-1, cy+(ah/2)+(h2/2), col);
			rectfill(dest, cx+(aw/2)-(w2/2), cy, cx+(aw/2)+(w2/2), cy+ah-1, col);
			break;
	}
}
/* draw_graphics_button:
  *  Helper for jwin_button_proc.
  */
void jwin_draw_graphics_button(BITMAP *dest, int32_t x, int32_t y, int32_t w, int32_t h, BITMAP *bmp, BITMAP *bmp2, int32_t flags, bool show_dotted_rect, bool overlay)
{
    int32_t g = (flags & D_SELECTED) ? 1 : 0;
    
    if(flags & D_SELECTED)
        jwin_draw_button(dest, x, y, w, h, 2, 0);
    else if(!(flags & D_GOTFOCUS))
        jwin_draw_button(dest, x, y, w, h, 0, 0);
    else
    {
        rect(dest, x, y, x+w-1, y+h-1, scheme[jcDARK]);
        jwin_draw_button(dest, x+1, y+1, w-2, h-2, 0, 0);
    }
    
    if(!(flags & D_DISABLED))
    {
//    gui_textout_ex(dest, str, x+w/2+g, y+h/2-text_height(font)/2+g, scheme[jcBOXFG], -1, TRUE);
        if(bmp!=NULL)
        {
            if(overlay)
            {
                masked_blit(bmp, dest, 0, 0, x+w/2-bmp->w/2+g, y+h/2-bmp->h/2+g, bmp->h, bmp->w);
            }
            else
            {
                blit(bmp, dest, 0, 0, x+w/2-bmp->w/2+g, y+h/2-bmp->h/2+g, bmp->h, bmp->w);
            }
        }
    }
    else
    {
//    gui_textout_ex(dest, str, x+w/2+1,y+h/2-text_height(font)/2+1, scheme[jcLIGHT], -1, TRUE);
//    gui_textout_ex(dest, str, x+w/2,  y+h/2-text_height(font)/2, scheme[jcMEDDARK], -1, TRUE);
        if(bmp2!=NULL)
        {
            if(overlay)
            {
                masked_blit(bmp2, dest, 0, 0, x+w/2-bmp2->w/2+g, y+h/2-bmp2->h/2+g, bmp2->h, bmp2->w);
            }
            else
            {
                blit(bmp2, dest, 0, 0, x+w/2-bmp2->w/2+g, y+h/2-bmp2->h/2+g, bmp2->h, bmp2->w);
            }
        }
    }
    
    if(show_dotted_rect&&(flags & D_GOTFOCUS))
        dotted_rect(dest, x+4, y+4, x+w-5, y+h-5, scheme[jcDARK], scheme[jcBOX]);
}

/* jwin_button_proc:
  *  A button object (the dp field points to the text string). This object
  *  can be selected by clicking on it with the mouse or by pressing its
  *  keyboard shortcut. If the D_EXIT flag is set, selecting it will close
  *  the dialog, otherwise it will toggle on and off.
  */
int32_t jwin_button_proc(int32_t msg, DIALOG *d, int32_t)
{
    int32_t down=0;
    int32_t selected=(d->flags&D_SELECTED)?1:0;
    int32_t disabled=(d->flags&D_DISABLED)?1:0;
    int32_t last_draw;
    
    switch(msg)
    {
		case MSG_DRAW:
		{
			FONT *oldfont = font;
			
			if(d->dp2)
			{
				font = (FONT*)d->dp2;
			}
			
			jwin_draw_text_button(screen, d->x, d->y, d->w, d->h, (char*)d->dp, d->flags, true);
			font = oldfont;
		}
		break;
		
		case MSG_WANTFOCUS:
			return D_WANTFOCUS;
			
		case MSG_KEY:
			if(disabled) break;
			/* close dialog? */
			if(d->flags & D_EXIT)
			{
				return D_CLOSE;
			}
			if(d->d2 == 1) //Insta-button
			{
				GUI_EVENT(d, geCLICK);
				break;
			}
			/* or just toggle */
			d->flags ^= D_SELECTED;
			GUI_EVENT(d, geCLICK);
			object_message(d, MSG_DRAW, 0);
			break;
			
		case MSG_CLICK:
		{
			if(disabled) break;
			if(d->d2 == 1) //Insta-button
			{
				if(mouse_in_rect(d->x, d->y, d->w, d->h))
				{
					GUI_EVENT(d, geCLICK);
					if(d->flags & D_EXIT)
						return D_CLOSE;
				}
			}
			else
			{
				last_draw = 0;
				
				/* track the mouse until it is released */
				while(gui_mouse_b())
				{
					down = mouse_in_rect(d->x, d->y, d->w, d->h);
					
					/* redraw? */
					bool should_redraw = false;
					if(last_draw != down)
					{
						if(down != selected)
							d->flags |= D_SELECTED;
						else
							d->flags &= ~D_SELECTED;
							
						object_message(d, MSG_DRAW, 0);
						last_draw = down;
						should_redraw = true;
					}
					
					/* let other objects continue to animate */
					int r = broadcast_dialog_message(MSG_IDLE, 0);
					if (r & D_REDRAWME) should_redraw = true;

					if (should_redraw)
					{
						update_hw_screen();
					}
				}
				
				/* redraw in normal state */
				if(down)
				{
					GUI_EVENT(d, geCLICK);
					if(d->flags&D_EXIT)
					{
						d->flags &= ~D_SELECTED;
						object_message(d, MSG_DRAW, 0);
					}
				}
				
				/* should we close the dialog? */
				if(down && (d->flags & D_EXIT))
					return D_CLOSE;
			}
		}
		break;
	}
	return D_O_K;
}
int32_t jwin_iconbutton_proc(int32_t msg, DIALOG *d, int32_t)
{
    int32_t down=0;
    int32_t selected=(d->flags&D_SELECTED)?1:0;
    int32_t last_draw;
    
    switch(msg)
    {
		case MSG_DRAW:
		{
			jwin_draw_icon_button(screen, d->x, d->y, d->w, d->h, d->d1, d->flags, true);
		}
		break;
		
		case MSG_WANTFOCUS:
			return D_WANTFOCUS;
			
		case MSG_KEY:
			/* close dialog? */
			if(d->flags & D_EXIT)
			{
				return D_CLOSE;
			}
			if(d->d2 == 1) //Insta-button
			{
				GUI_EVENT(d, geCLICK);
				break;
			}
			/* or just toggle */
			d->flags ^= D_SELECTED;
			GUI_EVENT(d, geCLICK);
			object_message(d, MSG_DRAW, 0);
			break;
			
		case MSG_CLICK:
		{
			if(d->d2 == 1) //Insta-button
			{
				if(mouse_in_rect(d->x, d->y, d->w, d->h))
				{
					GUI_EVENT(d, geCLICK);
					if(d->flags & D_EXIT)
						return D_CLOSE;
				}
			}
			else
			{
				last_draw = 0;
				
				/* track the mouse until it is released */
				while(gui_mouse_b())
				{
					down = mouse_in_rect(d->x, d->y, d->w, d->h);
					
					/* redraw? */
					bool should_redraw = false;
					if(last_draw != down)
					{
						if(down != selected)
							d->flags |= D_SELECTED;
						else
							d->flags &= ~D_SELECTED;
							
						object_message(d, MSG_DRAW, 0);
						last_draw = down;
						should_redraw = true;
					}
					
					/* let other objects continue to animate */
					int r = broadcast_dialog_message(MSG_IDLE, 0);
					if (r & D_REDRAWME) should_redraw = true;

					if (should_redraw)
					{
						update_hw_screen();
					}
				}
				
				/* redraw in normal state */
				if(down)
				{
					GUI_EVENT(d, geCLICK);
					if(d->flags&D_EXIT)
					{
						d->flags &= ~D_SELECTED;
						object_message(d, MSG_DRAW, 0);
					}
				}
				
				/* should we close the dialog? */
				if(down && (d->flags & D_EXIT))
					return D_CLOSE;
			}
		}
		break;
	}
	return D_O_K;
}
int32_t jwin_infobtn_proc(int32_t msg, DIALOG *d, int32_t)
{
    int32_t down=0;
    int32_t selected=(d->flags&D_SELECTED)?1:0;
    int32_t last_draw;
	std::string* str = (std::string*)d->dp;
	bool dis = (d->flags & D_DISABLED) || !str || !((*str)[0]) || str->find_first_not_of(" \t")==std::string::npos;
    int flags = d->flags | (dis?D_DISABLED:0);
	bool show = false;
    switch(msg)
    {
		case MSG_DRAW:
		{
			FONT *oldfont = font;
			
			if(d->dp2)
				font = (FONT*)d->dp2;
			
			jwin_draw_text_button(screen, d->x, d->y, d->w, d->h, "?", flags, true);
			font = oldfont;
		}
		break;
		
		case MSG_WANTFOCUS:
			if(dis) break;
			return D_WANTFOCUS;
			
		case MSG_KEY:
			if(dis) break;
			show = true;
			break;
			
		case MSG_CLICK:
		{
			if(dis) break;
			if(d->d2 == 1) //Insta-button
			{
				if(mouse_in_rect(d->x, d->y, d->w, d->h))
				{
					show = true;
					break;
				}
			}
			else
			{
				last_draw = 0;
				
				/* track the mouse until it is released */
				while(gui_mouse_b())
				{
					down = mouse_in_rect(d->x, d->y, d->w, d->h);
					
					/* redraw? */
					bool should_redraw = false;
					if(last_draw != down)
					{
						if(down != selected)
							d->flags |= D_SELECTED;
						else
							d->flags &= ~D_SELECTED;
							
						object_message(d, MSG_DRAW, 0);
						last_draw = down;
						should_redraw = true;
					}
					
					/* let other objects continue to animate */
					int r = broadcast_dialog_message(MSG_IDLE, 0);
					if (r & D_REDRAWME) should_redraw = true;

					if (should_redraw)
					{
						update_hw_screen();
					}
				}
				
				/* redraw in normal state */
				if(down)
					show = true;
			}
		}
		break;
	}
	if(show)
	{
		d->flags &= ~D_SELECTED;
		object_message(d, MSG_DRAW, 0);
		InfoDialog("Info",*str).show();
		GUI_EVENT(d, geCLICK);
	}
	return D_O_K;
}

/* jwin_func_button_proc:
  *  A button that runs a void() function when clicked.
  *  dp: Button text
  *  dp2: Function to run
  */
int32_t jwin_func_button_proc(int32_t msg, DIALOG *d, int32_t c)
{
    int32_t down=0;
    int32_t selected=(d->flags&D_SELECTED)?1:0;
    int32_t last_draw;
    
    if(msg==MSG_CLICK || msg==MSG_KEY)
    {
        last_draw = 0;
        
        /* track the mouse until it is released */
        while(gui_mouse_b())
        {
            down = mouse_in_rect(d->x, d->y, d->w, d->h);
            
            /* redraw? */
            bool should_redraw = false;
            if(last_draw != down)
            {
                if(down != selected)
                    d->flags |= D_SELECTED;
                else
                    d->flags &= ~D_SELECTED;
                    
                object_message(d, MSG_DRAW, 0);
                last_draw = down;
                should_redraw = true;
            }
            
            /* let other objects continue to animate */
            int r = broadcast_dialog_message(MSG_IDLE, 0);
            if (r & D_REDRAWME) should_redraw = true;

            if (should_redraw)
            {
                update_hw_screen();
            }
        }
        
        /* redraw in normal state */
        if(down)
        {
            if(d->flags&D_EXIT)
            {
                d->flags &= ~D_SELECTED;
                object_message(d, MSG_DRAW, 0);
            }
        }
        
        /* pop up and call the function */
        if(down)
        {
            d->flags &= ~D_SELECTED;
            object_message(d, MSG_DRAW, 0);
            typedef void (*funcType)(void);
            funcType func=reinterpret_cast<funcType>(d->dp3);
            func();
        }
            
        return D_O_K;
    }
    
    return jwin_button_proc(msg, d, c);
}

/*(int32_t x = atoi(d->dp);
if ( x > 256 )
d->dp = (char*)"255";
elseif (x < 0 ) d->dp = (char*)"0";
*/

int32_t jwin_vedit_proc(int32_t msg, DIALOG *d, int32_t c)
{
	if(d->flags & D_HIDDEN)
	{
		switch(msg)
		{
			case MSG_CHAR: case MSG_UCHAR: case MSG_XCHAR: case MSG_DRAW: case MSG_CLICK: case MSG_DCLICK: case MSG_KEY: case MSG_WANTFOCUS:
				return D_O_K;
		}
	}
	if(d->h < 2+((text_height(d->dp2 ? (FONT*)d->dp2 : font)+2)*2))
		return jwin_edit_proc(msg, d, c);
	static char nullbuf[2];
	sprintf(nullbuf, " ");
	int32_t f, l, p, w, x, y, fg, bg;
	int32_t lastSpace = -1;
	char *s;
	char buf[2] = {0,0};
	
	if(d->dp==NULL)
	{
		d->dp=(void *)nullbuf;
	}
	
	s = (char*)d->dp;
	l = (int32_t)strlen(s);
	
	int32_t cursor_start = d->d2 & 0x0000FFFF;
	int32_t cursor_end = int32_t((d->d2 & 0xFFFF0000) >> 16);
	// This was previously doing bitshifts on -1. There wasn't enough space so I cannibalized 0xFFFF instead. -Moosh
	if (cursor_start == 0xFFFF)
		cursor_start = -1;
	if (cursor_end == 0xFFFF)
		cursor_end = -1;
	
	if(cursor_start > l)
		cursor_start = l;
	if(cursor_end > l)
		cursor_end = l;
	auto low_cursor = cursor_start<0 ? cursor_end : (cursor_end<0 ? cursor_start : (zc_min(cursor_start, cursor_end)));
	auto high_cursor = zc_max(cursor_start,cursor_end);
	bool multiselect = cursor_end > -1;
	
	FONT *oldfont = font;
	if(d->dp2)
		font = (FONT*)d->dp2;
	
	auto* char_length = font->vtable->char_length;
	std::vector<size_t> lines;
	x = 0;
	
	y = d->y + 2;
	size_t ind = 0;
	int32_t yinc = text_height(font)+2;
	int32_t maxy = y;
	size_t maxlines = 1;
	while(maxy+yinc < d->y+d->h-3)
	{
		maxy += yinc;
		++maxlines;
	}
	size_t half_width = (maxlines-1)/2;
	size_t line_scroll = 0;
	size_t focused_line = size_t(-1);
	size_t focused_line2 = size_t(-1);
	switch(msg)
	{
		//Only these messages need these calculations, so save processing.
		case MSG_DRAW:
		case MSG_CLICK:
		case MSG_CHAR:
		{
			for(auto q = 0; q <= l; ++q)
			{
				char c = s[q] ? s[q] : ' ';
				x += char_length(font, c);
				if(x > d->w - 6)
				{
					// Line's too long, break
					if(lastSpace >= 0)
					{
						q = lastSpace+1;
						lines.push_back(q);
						lastSpace = -1;
					}
					else
					{
						lines.push_back(q);
					}
					x = 0;
					--q; //counteract increment
				}
				else if(c == ' ')
					lastSpace = q;
			}
			if(lines.empty() || lines.back() != l+1)
				lines.push_back(l+1);
			for(size_t line = 0; line < lines.size(); ++line)
			{
				if(size_t(multiselect ? cursor_end : cursor_start) < lines[line]) //should ALWAYS execute once
				{
					focused_line = line;
					break;
				}
			}
			if(!multiselect)
			{
				focused_line2 = -1;
			}
			else for(size_t line = 0; line < lines.size(); ++line)
			{
				if(size_t(cursor_start) < lines[line]) //should ALWAYS execute once
				{
					focused_line2 = line;
					break;
				}
			}
			if (focused_line >= lines.size())
				focused_line = lines.size() - 1;
			if (focused_line2 >= lines.size())
				focused_line2 = lines.size() - 1;
			if(maxlines >= lines.size() || focused_line <= half_width)
				line_scroll = 0;
			else if(lines.size()-maxlines+half_width < focused_line)
				line_scroll = lines.size()-maxlines+half_width-1;
			else
				line_scroll = focused_line - half_width;
		}
	}
	font = oldfont; //in case of early return, need to reset here
	static bool dclick = false;
	switch(msg)
	{
		case MSG_START:
			dclick = false;
			cursor_start = (int32_t)strlen((char*)d->dp);
			cursor_end = -1;
			d->d2 = cursor_start | (((cursor_end == -1 ? 0xFFFF : cursor_end) & 0xFFFF) << 16);
			break;
			
		case MSG_DRAW:
		{
			if(d->dp2)
				font = (FONT*)d->dp2;
			if(d->flags & D_DISABLED)
			{
				fg = scheme[jcDISABLED_FG];
				bg = scheme[jcDISABLED_BG];
			}
			else if(d->flags & D_READONLY)
			{
				fg = scheme[jcALT_TEXTFG];
				bg = scheme[jcALT_TEXTBG];
			}
			else
			{
				fg = scheme[jcTEXTFG];
				bg = scheme[jcTEXTBG];
			}
			
			//Fill the BG
			rectfill(screen, d->x+2, d->y+2, d->x+d->w-3, d->y+d->h-3, bg);
			
			//Now the text
			size_t m = zc_min(line_scroll + maxlines, lines.size());
			jwin_draw_frame(screen, d->x, d->y, d->w, d->h, FR_DEEP);
			for(size_t line = line_scroll; line < m; ++line, y+=yinc)
			{
				x = 3;
				for(ind = line ? lines[line-1] : 0; ind < lines[line]; ++ind)
				{
					char c = s[ind] ? s[ind] : ' ';
					w = char_length(font, c);
					bool focused = multiselect
						? (ind >= low_cursor && ind <= high_cursor)
						: (ind == cursor_start);
					f = (focused && (d->flags & D_GOTFOCUS));
					buf[0] = c;
					textout_ex(screen, font, buf, d->x+x, y, f ? bg : fg,f ? fg : bg);
					x += w;
				}
			}
				
			font = oldfont;
			break;
		}

		case MSG_DCLICK:
			if ((gui_mouse_b() & 2) != 0)
				break;
			if (d->flags & (D_DISABLED | D_READONLY))
				break;
			dclick = true;
			break;
		case MSG_CLICK:
		{
			if(d->flags & (D_DISABLED|D_READONLY))
				break;
			if(d->dp2)
				font = (FONT*)d->dp2;
			
			bool found = false;
			for(size_t line = line_scroll; line < lines.size() && line < (line_scroll + maxlines); ++line, y+=yinc)
			{
				if(gui_mouse_y() >= y+yinc)
					continue;
				x = d->x+3;
				for(ind = line ? lines[line-1] : 0; ind < lines[line]; ++ind)
				{
					x += char_length(font, s[ind]);
					if(x >= gui_mouse_x())
					{
						if(key_shifts&KB_SHIFT_FLAG)
							cursor_end = ind;
						else
						{
							cursor_start = ind;
							cursor_end = -1;
							if (dclick)
								cursor_end = cursor_start;
						}
						found = true;
						break;
					}
				}
				break;
			}
			if(!found)
			{
				if(key_shifts&KB_SHIFT_FLAG)
					cursor_end = l;
				else
				{
					cursor_start = l;
					cursor_end = -1;
					if (dclick)
						cursor_end = cursor_start;
				}
			}

			if (dclick)
			{
				while (cursor_start > 0 && cursor_start < l)
				{
					if (s[cursor_start] == ' ')
					{
						if (cursor_start <= cursor_end)
							++cursor_start;
						else
							--cursor_start;
						break;
					}
					if (cursor_start <= cursor_end)
						--cursor_start;
					else
						++cursor_start;
				}
				while (cursor_end > 0 && cursor_end < l)
				{
					if (s[cursor_end] == ' ')
					{
						if (cursor_end >= cursor_start)
							--cursor_end;
						else
							++cursor_end;
						break;
					}
					if (cursor_end >= cursor_start)
						++cursor_end;
					else
						--cursor_end;
				}
				d->d2 = cursor_start | (((cursor_end == -1 ? 0xFFFF : cursor_end) & 0xFFFF) << 16);
				d->flags |= D_DIRTY;
			}
			else
			{
				if (cursor_end == cursor_start) cursor_end = -1;
				else d->flags |= D_DIRTY;
				d->d2 = cursor_start | (((cursor_end == -1 ? 0xFFFF : cursor_end) & 0xFFFF) << 16);
			}
			
			object_message(d, MSG_DRAW, 0);
			font = oldfont;
			dclick = false;
			break;
		}

		case MSG_WANTFOCUS:
		case MSG_LOSTFOCUS:
		case MSG_KEY:
			if(d->flags & (D_DISABLED|D_READONLY))
				break;
			return D_WANTFOCUS;
			
		case MSG_CHAR:
		{
			if(d->flags & (D_DISABLED|D_READONLY))
				break;
			bool shifted = key_shifts & KB_SHIFT_FLAG;
			bool ctrl = key_shifts & KB_CTRL_FLAG;
			bool change_cursor = true;
			int32_t scursor = cursor_start, ecursor = cursor_end;
			char upper_c = c>>8;
			char lower_c = c&255;
			if(shifted)
			{
				if(ecursor < 0)
				{
					ecursor = scursor;
					focused_line2 = focused_line;
				}
			}
			if(upper_c == KEY_LEFT)
			{
				if(shifted)
				{
					if(ecursor>0)
						--ecursor;
				}
				else
				{
					ecursor = -1;
					if(scursor > 0)
						--scursor;
				}
			}
			else if(upper_c == KEY_RIGHT)
			{
				if(shifted)
				{
					if(ecursor < l)
						++ecursor;
				}
				else
				{
					ecursor = -1;
					if(scursor < l)
						++scursor;
				}
			}
			else if(upper_c == KEY_UP || upper_c == KEY_DOWN)
			{
				if(shifted)
				{
					size_t line = focused_line2 + (upper_c == KEY_UP ? -1 : 1);
					if(!focused_line2 && upper_c == KEY_UP)
						ecursor = 0;
					else if(line >= lines.size())
						ecursor = l;
					else
					{
						if(d->dp2)
							font = (FONT*)d->dp2;
						x = d->x + 3;
						for(ind = focused_line2 ? lines[focused_line2-1] : 0; ind < lines[focused_line2]; ++ind)
						{
							w = char_length(font, s[ind]);
							if(ind < size_t(ecursor))
								x += w;
							else
							{
								x += w / 2;
								break;
							}
						}
						
						int32_t tx = d->x+3;
						bool done = false;
						for(ind = line ? lines[line-1] : 0; ind < lines[line]; ++ind)
						{
							tx += char_length(font, s[ind] ? s[ind] : ' ');
							if(tx < x)
								continue;
							ecursor = ind;
							done = true;
							break;
						}
						font = oldfont;
						if(!done)
						{
							ecursor = (upper_c == KEY_UP) ? 0 : lines[line]-1;
						}
					}
				}
				else
				{
					ecursor = -1;
					if(multiselect)
					{
						focused_line = focused_line2;
						scursor = ecursor;
					}
					size_t line = focused_line + (upper_c == KEY_UP ? -1 : 1);
					if(!focused_line && upper_c == KEY_UP)
						scursor = 0;
					else if(line >= lines.size())
						scursor = l;
					else
					{
						if(d->dp2)
							font = (FONT*)d->dp2;
						x = d->x + 3;
						for(ind = focused_line ? lines[focused_line-1] : 0; ind < lines[focused_line]; ++ind)
						{
							w = char_length(font, s[ind]);
							if(ind < size_t(scursor))
								x += w;
							else
							{
								x += w / 2;
								break;
							}
						}
						
						int32_t tx = d->x+3;
						bool done = false;
						for(ind = line ? lines[line-1] : 0; ind < lines[line]; ++ind)
						{
							tx += char_length(font, s[ind] ? s[ind] : ' ');
							if(tx < x)
								continue;
							scursor = ind;
							done = true;
							break;
						}
						font = oldfont;
						if(!done)
						{
							scursor = (upper_c == KEY_UP) ? 0 : lines[line]-1;
						}
					}
				}
			}
			else if(upper_c == KEY_HOME)
			{
				if(shifted)
					ecursor = 0;
				else
				{
					ecursor = -1;
					scursor = 0;
				}
			}
			else if(upper_c == KEY_END)
			{
				if(shifted)
					ecursor = l;
				else
				{
					ecursor = -1;
					scursor = l;
				}
			}
			else if(upper_c == KEY_DEL)
			{
				if(ctrl)
				{
					s[0] = 0;
					scursor = 0;
					ecursor = -1;
					GUI_EVENT(d, geCHANGE_VALUE);
				}
				else if(multiselect)
				{
					ecursor = -1;
					scursor = low_cursor;
					size_t ind = low_cursor, ind2 = high_cursor+1;
					while(s[ind2])
						s[ind++] = s[ind2++];
					while(s[ind])
						s[ind++] = 0;
					GUI_EVENT(d, geCHANGE_VALUE);
				}
				else if(scursor < l)
				{
					for(p=scursor; s[p]; p++)
						s[p] = s[p+1];
					GUI_EVENT(d, geCHANGE_VALUE);
				}
			}
			else if(upper_c == KEY_BACKSPACE)
			{
				if(ctrl)
				{
					s[0] = 0;
					scursor = 0;
					ecursor = -1;
					GUI_EVENT(d, geCHANGE_VALUE);
				}
				else if(multiselect)
				{
					ecursor = -1;
					scursor = low_cursor;
					size_t ind = low_cursor, ind2 = high_cursor+1;
					while(s[ind2])
						s[ind++] = s[ind2++];
					while(s[ind])
						s[ind++] = 0;
					GUI_EVENT(d, geCHANGE_VALUE);
				}
				else if(scursor > 0)
				{
					--scursor;
					for(p=scursor; s[p]; p++)
						s[p] = s[p+1];
					GUI_EVENT(d, geCHANGE_VALUE);
				}
			}
			else if(upper_c == KEY_ENTER)
			{
				change_cursor = false;
				GUI_EVENT(d, geENTER);
				if(d->flags & D_EXIT)
				{
					object_message(d, MSG_DRAW, 0);
					return D_CLOSE;
				}
				else
					return D_O_K;
			}
			else if(upper_c == KEY_TAB)
			{
				change_cursor = false;
				return D_O_K;
			}
			else if(ctrl && (lower_c=='c' || lower_c=='C'))
			{
				change_cursor = false;
				std::ostringstream oss;
				if(multiselect)
				{
					for(size_t ind = low_cursor; ind <= high_cursor; ++ind)
					{
						if(s[ind])
							oss << s[ind];
					}
				}
				else
				{
					if(s[scursor])
						oss << s[scursor];
				}
				set_al_clipboard(oss.str());
			}
			else if(clipboard_has_text() && ctrl && (lower_c=='v' || lower_c=='V'))
			{
				std::string cb;
				if(get_al_clipboard(cb))
				{
					int ind = low_cursor, ind2 = high_cursor + 1;
					if (multiselect)
					{
						//Delete selected text
						ecursor = -1;
						scursor = low_cursor;
						while (s[ind2] && ind2 < l)
							s[ind++] = s[ind2++];
						while (s[ind])
							s[ind++] = 0;
						l = (int32_t)strlen(s);
					}
					//Move the text out of the way of the pasting
					int paste_len = cb.size();
					int paste_start = scursor;
					int paste_end = paste_start+paste_len;
					ind = strlen(s);
					ind2 = ind+paste_len;
					while(ind2 > d->d1)
					{
						--ind;
						--ind2;
					}
					size_t new_l = ind2;
					while(ind >= paste_start)
					{
						if(s[ind] || (ind&&s[ind-1]))
						{
							s[ind2] = s[ind];
						}
						--ind2; --ind;
					}
					for(auto q = 0; q < paste_len && paste_start+q < new_l; ++q)
					{
						s[paste_start+q] = cb.at(q);
					}
					s[new_l] = 0;
					scursor = paste_start + paste_len;
					ecursor = -1;
					GUI_EVENT(d, geCHANGE_VALUE);
				}
			}
			else if(ctrl && (lower_c=='a' || lower_c=='A'))
			{
				cursor_start = 0;
				cursor_end = (int16_t)strlen((char*)d->dp) - 1;
				d->d2 = cursor_start | (((cursor_end == -1 ? 0xFFFF : cursor_end) & 0xFFFF) << 16);
				d->flags |= D_DIRTY;
				break;
			}
			else if(lower_c >= 32)
			{
				if(multiselect)
				{
					//Delete selected text
					ecursor = -1;
					scursor = low_cursor;
					size_t ind = low_cursor, ind2 = high_cursor+1;
					while(s[ind2] && ind2 < l)
						s[ind++] = s[ind2++];
					while(s[ind])
						s[ind++] = 0;
					l = (int32_t)strlen(s);
					//Type the character in its' place
					//(fallthrough)
				}
				if(l < d->d1)
				{
					ecursor = -1;
					s[l+1] = 0;
					size_t ind = l;
					while(ind >= scursor)
					{
						s[ind+1] = s[ind];
						if (!ind) break;
						--ind;
					}
					
					s[scursor++] = lower_c;
					
					GUI_EVENT(d, geCHANGE_VALUE);
				}
			}
			else
				return D_O_K;
			
			if(change_cursor)
			{
				if (cursor_start != scursor)
					d->flags |= D_DIRTY;

				cursor_end = ecursor; cursor_start = scursor;
				if (cursor_end == cursor_start) cursor_end = -1;
				d->d2 = cursor_start | (((cursor_end == -1 ? 0xFFFF : cursor_end) & 0xFFFF) << 16);
			}
			
			/* if we changed something, better redraw... */
			object_message(d, MSG_DRAW, 0);
			return D_USED_CHAR;
		}
	}
	
	return D_O_K;
}

/* jwin_edit_proc:
  *  An editable text object (the dp field points to the string). When it
  *  has the input focus (obtained by clicking on it with the mouse), text
  *  can be typed into this object. The d1 field specifies the maximum
  *  number of characters that it will accept, and d2 is the text cursor
  *  position within the string.
  */
int32_t jwin_edit_proc(int32_t msg, DIALOG *d, int32_t c)
{
	if(d->flags & D_HIDDEN)
	{
		switch(msg)
		{
			case MSG_CHAR: case MSG_UCHAR: case MSG_XCHAR: case MSG_DRAW: case MSG_CLICK: case MSG_DCLICK: case MSG_KEY: case MSG_WANTFOCUS:
				return D_O_K;
		}
	}
	if(d->h >= 2+((text_height(d->dp2 ? (FONT*)d->dp2 : font)+2)*2))
		return jwin_vedit_proc(msg, d, c);
	int32_t f, l, p, w, x, y, fg, bg, fg2 = -1, bg2 = -1, fg3, bg3;
	int32_t b;
	int32_t scroll;
	char *s;
	char buf[2];
	static char nullbuf[2];
	sprintf(nullbuf, " ");
	
	if(d->dp==NULL)
	{
		d->dp=(void *)nullbuf;
	}
	
	s = (char*)d->dp;
	l = (int32_t)strlen(s);
	
	int32_t cursor_start = d->d2 & 0x0000FFFF;
	int32_t cursor_end = int32_t((d->d2 & 0xFFFF0000) >> 16);
	// This was previously doing bitshifts on -1. There wasn't enough space so I cannibalized 0xFFFF instead. -Moosh
	if (cursor_start == 0xFFFF)
		cursor_start = -1;
	if (cursor_end == 0xFFFF)
		cursor_end = -1;
	
	if(cursor_start > l)
		cursor_start = l;
	if(cursor_end > l)
		cursor_end = l;
	auto low_cursor = cursor_start<0 ? cursor_end : (cursor_end<0 ? cursor_start : (zc_min(cursor_start, cursor_end)));
	auto high_cursor = zc_max(cursor_start,cursor_end);
		
	/* calculate maximal number of displayable characters */
	b = x = 0;
	
	if(cursor_start == l)
	{
		buf[0] = ' ';
		buf[1] = 0;
		
		if(d->dp2)
			x = text_length((FONT*)d->dp2, buf);
		else
			x = text_length(font, buf);
	}
	
	buf[1] = 0;
	
	for(p=cursor_start; p>=0; p--)
	{
		buf[0] = s[p];
		b++;
		
		if(d->dp2)
			x += text_length((FONT*)d->dp2, buf);
		else
			x += text_length(font, buf);
			
		if(x > d->w-6)
			break;
	}
	
	if(x <= d->w-6)
	{
		b = l;
		scroll = FALSE;
	}
	else
	{
		b--;
		scroll = TRUE;
	}
	
	FONT *oldfont = font;
	static bool dclick = false;
	switch(msg)
	{
		case MSG_START:
			dclick = false;
			cursor_start = (int32_t)strlen((char*)d->dp);
			cursor_end = -1;
			d->d2 = cursor_start | (((cursor_end == -1 ? 0xFFFF : cursor_end) & 0xFFFF) << 16);
			break;
			
		case MSG_DRAW:
		{
			if(d->dp2)
			{
				font = (FONT*)d->dp2;
			}
			if(d->flags & D_DISABLED)
			{
				fg2 = scheme[jcLIGHT];
				bg2 = scheme[jcDISABLED_BG];
				fg = scheme[jcDISABLED_FG];
				bg = -1;
				fg3 = fg;
				bg3 = bg2;
			}
			else if(d->flags & D_READONLY)
			{
				fg = scheme[jcALT_TEXTFG];
				bg = scheme[jcALT_TEXTBG];
				fg3 = fg;
				bg3 = bg;
			}
			else
			{
				fg = scheme[jcTEXTFG];
				bg = scheme[jcTEXTBG];
				fg3 = fg;
				bg3 = bg;
			}
			
			x = 3;
			y = (d->h - text_height(font)) / 2 + d->y;
			
			/* first fill in the edges */
			
			rectfill(screen, d->x+2, d->y+2, d->x+d->w-3, d->y+d->h-3, bg3);
			
			_allegro_vline(screen, d->x+2, d->y+2, d->y+d->h-3, bg3);
			
			/* now the text */
			
			if(scroll)
			{
				p = cursor_start-b+1;
				b = cursor_start;
			}
			else
				p = 0;
			for(; p<=b; p++)
			{
				buf[0] = s[p] ? s[p] : ' ';
				w = text_length(font, buf);
				
				if(x+w > d->w)
					break;
				bool focused = (cursor_end>-1)
					? (p >= low_cursor && p <= high_cursor)
					: (p == cursor_start);
				f = fg2 < 0 && (focused && (d->flags & D_GOTFOCUS));
				if(fg2 > -1)
				{
					textout_ex(screen, font, buf, d->x+x+1, y+1, fg2, bg2);
				}
				textout_ex(screen, font, buf, d->x+x, y, f ? bg : fg,f ? fg : bg);
				x += w;
			}
			
			if(x < d->w-2)
				rectfill(screen, d->x+x, y, d->x+d->w-3, y+text_height(font)-1, bg3);
				
			jwin_draw_frame(screen, d->x, d->y, d->w, d->h, FR_DEEP);
			font = oldfont;
			break;
		}

		case MSG_DCLICK:
			if ((gui_mouse_b() & 2) != 0)
				break;
			if (d->flags & (D_DISABLED | D_READONLY))
				break;
			dclick = true;
			break;
		case MSG_CLICK:
		{
			if(d->flags & (D_DISABLED|D_READONLY))
				break;
			x = d->x+3;
			
			if(scroll)
			{
				p = cursor_start-b+1;
				b = cursor_start;
			}
			else
				p = 0;
				
			for(; p<b; p++)
			{
				buf[0] = s[p];
				x += text_length((d->dp2) ? (FONT*)d->dp2 : font, buf);
				
				if(x > gui_mouse_x())
					break;
			}
			
			if(key_shifts&KB_SHIFT_FLAG)
				cursor_end = MID(0, p, l);
			else
			{
				cursor_end = -1;
				cursor_start = MID(0, p, l);
				if (dclick)
					cursor_end = cursor_start;
			}

			if (dclick)
			{
				while (cursor_start > 0 && cursor_start < l)
				{
					if (s[cursor_start] == ' ')
					{
						if (cursor_start <= cursor_end)
							++cursor_start;
						else
							--cursor_start;
						break;
					}
					if (cursor_start <= cursor_end)
						--cursor_start;
					else
						++cursor_start;
				}
				while (cursor_end > 0 && cursor_end < l)
				{
					if (s[cursor_end] == ' ')
					{
						if (cursor_end >= cursor_start)
							--cursor_end;
						else
							++cursor_end;
						break;
					}
					if (cursor_end >= cursor_start)
						++cursor_end;
					else
						--cursor_end;
				}
				d->d2 = cursor_start | (((cursor_end == -1 ? 0xFFFF : cursor_end) & 0xFFFF) << 16);
				d->flags |= D_DIRTY;
			}
			else
			{
				if (cursor_end == cursor_start) cursor_end = -1;
				d->d2 = cursor_start | (((cursor_end == -1 ? 0xFFFF : cursor_end) & 0xFFFF) << 16);
			}
			d->flags |= D_DIRTY;
			dclick = false;
			break;
		}

		case MSG_WANTFOCUS:
		case MSG_LOSTFOCUS:
		case MSG_KEY:
			if(d->flags & (D_DISABLED|D_READONLY))
				break;
			return D_WANTFOCUS;
			
		case MSG_CHAR:
		{
			if(d->flags & (D_DISABLED|D_READONLY))
				break;
			bool shifted = key_shifts & KB_SHIFT_FLAG;
			bool ctrl = key_shifts & KB_CTRL_FLAG;
			bool change_cursor = true;
			bool change_value = false;
			int16_t scursor = cursor_start, ecursor = cursor_end;
			bool multiselect = cursor_end > -1;
			auto upper_c = c>>8;
			auto lower_c = c&0xFF;
			if(shifted)
			{
				if(ecursor < 0)
					ecursor = scursor;
			}
			if(upper_c == KEY_LEFT)
			{
				if(shifted)
				{
					if(ecursor>0)
						--ecursor;
				}
				else
				{
					ecursor = -1;
					if(scursor > 0)
						--scursor;
				}
			}
			else if(upper_c == KEY_RIGHT)
			{
				if(shifted)
				{
					if(ecursor < l)
						++ecursor;
				}
				else
				{
					ecursor = -1;
					if(scursor < l)
						++scursor;
				}
			}
			else if(upper_c == KEY_HOME)
			{
				if(shifted)
					ecursor = 0;
				else
				{
					ecursor = -1;
					scursor = 0;
				}
			}
			else if(upper_c == KEY_END)
			{
				if(shifted)
					ecursor = l;
				else
				{
					ecursor = -1;
					scursor = l;
				}
			}
			else if(upper_c == KEY_DEL)
			{
				if(ctrl)
				{
					s[0] = 0;
					scursor = 0;
					ecursor = -1;
					GUI_EVENT(d, geCHANGE_VALUE);
					change_value = true;
				}
				else if(multiselect)
				{
					ecursor = -1;
					scursor = low_cursor;
					size_t ind = low_cursor, ind2 = high_cursor+1;
					while(s[ind2])
						s[ind++] = s[ind2++];
					while(s[ind])
						s[ind++] = 0;
					GUI_EVENT(d, geCHANGE_VALUE);
					change_value = true;
				}
				else if(scursor < l)
				{
					for(p=scursor; s[p]; p++)
						s[p] = s[p+1];
					GUI_EVENT(d, geCHANGE_VALUE);
					change_value = true;
				}
			}
			else if(upper_c == KEY_BACKSPACE)
			{
				if(ctrl)
				{
					s[0] = 0;
					scursor = 0;
					ecursor = -1;
					GUI_EVENT(d, geCHANGE_VALUE);
					change_value = true;
				}
				else if(multiselect)
				{
					ecursor = -1;
					scursor = low_cursor;
					size_t ind = low_cursor, ind2 = high_cursor+1;
					while(s[ind2])
						s[ind++] = s[ind2++];
					while(s[ind])
						s[ind++] = 0;
					GUI_EVENT(d, geCHANGE_VALUE);
					change_value = true;
				}
				else if(scursor > 0)
				{
					--scursor;
					for(p=scursor; s[p]; p++)
						s[p] = s[p+1];
					GUI_EVENT(d, geCHANGE_VALUE);
					change_value = true;
				}
			}
			else if(upper_c == KEY_ENTER)
			{
				change_cursor = false;
				GUI_EVENT(d, geENTER);
				if(d->flags & D_EXIT)
				{
					object_message(d, MSG_DRAW, 0);
					return D_CLOSE;
				}
				else
					return D_O_K;
			}
			else if(upper_c == KEY_TAB)
			{
				change_cursor = false;
				return D_O_K;
			}
			else if(ctrl && (lower_c=='c' || lower_c=='C'))
			{
				change_cursor = false;
				std::ostringstream oss;
				if(multiselect)
				{
					for(size_t ind = low_cursor; ind <= high_cursor; ++ind)
					{
						if(s[ind])
							oss << s[ind];
					}
				}
				else
				{
					if(s[scursor])
						oss << s[scursor];
				}
				set_al_clipboard(oss.str());
			}
			else if(clipboard_has_text() && ctrl && (lower_c=='v' || lower_c=='V'))
			{
				std::string cb;
				if(get_al_clipboard(cb))
				{
					int ind = low_cursor, ind2 = high_cursor + 1;
					if (multiselect)
					{
						//Delete selected text
						ecursor = -1;
						scursor = low_cursor;
						while (s[ind2] && ind2 < l)
							s[ind++] = s[ind2++];
						while (s[ind])
							s[ind++] = 0;
						l = (int32_t)strlen(s);
					}
					//Move the text out of the way of the pasting
					int paste_len = cb.size();
					int paste_start = scursor;
					int paste_end = paste_start+paste_len;
					ind = strlen(s);
					ind2 = ind+paste_len;
					while(ind2 > d->d1)
					{
						--ind;
						--ind2;
					}
					size_t new_l = ind2;
					while(ind >= paste_start)
					{
						if(s[ind] || (ind&&s[ind-1]))
						{
							s[ind2] = s[ind];
						}
						--ind2; --ind;
					}
					for(auto q = 0; q < paste_len && paste_start+q < new_l; ++q)
					{
						s[paste_start+q] = cb.at(q);
					}
					s[new_l] = 0;
					scursor = paste_start + paste_len;
					ecursor = -1;
					GUI_EVENT(d, geCHANGE_VALUE);
					change_value = true;
				}
			}
			else if(ctrl && (lower_c=='a' || lower_c=='A'))
			{
				cursor_start = 0;
				cursor_end = (int16_t)strlen((char*)d->dp) - 1;
				d->d2 = cursor_start | (((cursor_end == -1 ? 0xFFFF : cursor_end) & 0xFFFF) << 16);
				d->flags |= D_DIRTY;
				break;
			}
			else if(lower_c >= 32)
			{
				if(multiselect)
				{
					//Delete selected text
					ecursor = -1;
					scursor = low_cursor;
					size_t ind = low_cursor, ind2 = high_cursor+1;
					while(s[ind2] && ind2 < l)
						s[ind++] = s[ind2++];
					while(s[ind])
						s[ind++] = 0;
					l = (int32_t)strlen(s);
					//Type the character in its' place
					//(fallthrough)
				}
				if(l < d->d1)
				{
					ecursor = -1;
					s[l+1] = 0;
					size_t ind = l;
					while(ind >= scursor)
					{
						s[ind+1] = s[ind];
						if (!ind) break;
						--ind;
					}
					
					s[scursor++] = lower_c;

					GUI_EVENT(d, geCHANGE_VALUE);
					change_value = true;
				}
			}
			else
				return D_O_K;
			if(change_cursor)
			{
				cursor_end = ecursor; cursor_start = scursor;
				if (cursor_end == cursor_start) cursor_end = -1;
				d->d2 = cursor_start | (((cursor_end == -1 ? 0xFFFF : cursor_end) & 0xFFFF) << 16);
			}
			/* if we changed something, better redraw... */
			// Note: this still redraws when not necessary.
			if (change_value || change_cursor)
				d->flags |= D_DIRTY;
			return D_USED_CHAR;
		}
	}
	return D_O_K;
}

int32_t jwin_hexedit_proc_old(int32_t msg,DIALOG *d,int32_t c)
{
    if(msg==MSG_CHAR)
        if(((isalpha(c&255) && !isxdigit(c&255))) || ispunct(c&255))
            return D_USED_CHAR;
            
    return jwin_edit_proc(msg,d,isalpha(c&255)?c&0xDF:c);
}

bool editproc_special_key(int32_t c)
{
	switch(c>>8)
	{
		case KEY_LEFT: case KEY_RIGHT:
		case KEY_HOME: case KEY_END:
		case KEY_DEL: case KEY_BACKSPACE:
		case KEY_ENTER: case KEY_TAB:
			return true;
	}
	if(key_shifts & KB_CTRL_FLAG)
		switch(c&255)
		{
			case 'c': case 'C':
				return true;
			case 'v': case 'V':
				return clipboard_has_text();
		}
	return false;
}
bool editproc_combined_key(int32_t c)
{
	if(key_shifts & KB_CTRL_FLAG)
		switch(c&255)
		{
			case 'c': case 'C':
				return true;
			case 'v': case 'V':
				return clipboard_has_text();
		}
	return false;
}
int32_t jwin_hexedit_proc(int32_t msg,DIALOG *d,int32_t c)
{
	bool caps_paste = false;
	if(msg==MSG_CHAR)
	{
		if(key_shifts & KB_CTRL_FLAG)
		{
			if(clipboard_has_text() && ((c&255)=='v' || (c&255)=='V'))
			{
				std::string cb;
				if(get_al_clipboard(cb))
				{
					if(cb.find_first_not_of("-.0123456789ABCDEFabcdef") != std::string::npos)
						return D_USED_CHAR;
					if(cb.find_first_of("abcdef") != std::string::npos)
						caps_paste = true;
				}
				else return D_USED_CHAR;
			}
		}
		switch(c&255)
		{
			case '-': case '.':
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
			case 'A': case 'B': case 'C':
			case 'D': case 'E': case 'F':
				break;
			case 'a': case 'b': case 'c':
			case 'd': case 'e': case 'f':
				c = (c&~255)|toupper(c&255);
				break;
			default:
				if(!editproc_special_key(c))
					return D_O_K;
				else if(!editproc_combined_key(c))
					c&=~255;
		}
	}
	
	auto ret = jwin_edit_proc(msg,d,c);
	if(caps_paste)
	{
		char* s = (char*)d->dp;
		caps_paste = false;
		for(auto q = strlen(s)-1; q >= 0; --q)
		{
			switch(s[q])
			{
				case 'a': case 'b': case 'c':
				case 'd': case 'e': case 'f':
					s[q] = toupper(s[q]);
					caps_paste = true;
					break;
			}
		}
		if(caps_paste)
		{
			jwin_edit_proc(MSG_DRAW,d,0);
		}
	}
	return ret;
}
int32_t jwin_numedit_proc(int32_t msg,DIALOG *d,int32_t c)
{
	if(msg==MSG_CHAR)
	{
		if(key_shifts & KB_CTRL_FLAG)
		{
			if(clipboard_has_text() && ((c&255)=='v' || (c&255)=='V'))
			{
				std::string cb;
				if(get_al_clipboard(cb))
				{
					if(cb.find_first_not_of("-.0123456789") != std::string::npos)
						return D_USED_CHAR;
				}
				else return D_USED_CHAR;
			}
		}
		switch(c&255)
		{
			case '-': case '.':
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				break;
			default:
				if(!editproc_special_key(c))
					return D_O_K;
				else if(!editproc_combined_key(c))
					c&=~255;
		}
	}
			
	return jwin_edit_proc(msg,d,c);
}

int32_t jwin_numedit_byte_proc(int32_t msg,DIALOG *d,int32_t c)
{
	if ( (atoi((char*)d->dp)) > 255 )
	{
		strcpy((char*)d->dp,"255\0");
		return jwin_numedit_proc(msg,d,c);
	}
	else if ( (atoi((char*)d->dp)) < 0 ) 
	{
		strcpy((char*)d->dp,"0\0");
		return jwin_numedit_proc(msg,d,c);
	}
            
    return jwin_numedit_proc(msg,d,c);
}

int32_t jwin_numedit_short_proc(int32_t msg,DIALOG *d,int32_t c)
{
	if ( (atoi((char*)d->dp)) > 65535 )
	{
		strcpy((char*)d->dp,"65535\0");
		return jwin_numedit_proc(msg,d,c);
	}
	else if ( (atoi((char*)d->dp)) < 0 ) 
	{
		strcpy((char*)d->dp,"0\0");
		return jwin_numedit_proc(msg,d,c);
	}
            
    return jwin_numedit_proc(msg,d,c);
}

int32_t jwin_numedit_zscriptint_proc(int32_t msg,DIALOG *d,int32_t c)
{
	if ( (atoi((char*)d->dp)) > 214748 )
	{
		strcpy((char*)d->dp,"214748\0");
		return jwin_numedit_proc(msg,d,c);
	}
	else if ( (atoi((char*)d->dp)) < -214748 ) 
	{
		strcpy((char*)d->dp,"-214748\0");
		return jwin_numedit_proc(msg,d,c);
	}
            
    return jwin_numedit_proc(msg,d,c);
}

int32_t jwin_numedit_sshort_proc(int32_t msg,DIALOG *d,int32_t c)
{
	if ( (atoi((char*)d->dp)) > 32767 )
	{
		strcpy((char*)d->dp,"32767\0");
		return jwin_numedit_proc(msg,d,c);
	}
	else if ( (atoi((char*)d->dp)) < -32768 ) 
	{
		strcpy((char*)d->dp,"-32768\0");
		return jwin_numedit_proc(msg,d,c);
	}
            
    return jwin_numedit_proc(msg,d,c);
}

int32_t jwin_numedit_sbyte_proc(int32_t msg,DIALOG *d,int32_t c)
{
	if ( (atoi((char*)d->dp)) > 127 )
	{
		strcpy((char*)d->dp,"127\0");
		return jwin_numedit_proc(msg,d,c);
	}
	else if ( (atoi((char*)d->dp)) < -128 ) 
	{
		strcpy((char*)d->dp,"-128\0");
		return jwin_numedit_proc(msg,d,c);
	}
            
    return jwin_numedit_proc(msg,d,c);
}

// Special numedit procs

void trim_trailing_0s(char* str, bool leaveDec = false)
{
	bool foundDec = false;
	for(int32_t q = 0; str[q]; ++q)
	{
		if(str[q] == '.')
		{
			foundDec = true;
			break;
		}
	}
	if(!foundDec) return; //No decimal place, thus no trailing 0's.
	for(int32_t q = strlen(str)-1; q > 0; --q)
	{
		if(str[q] == '0' && (!leaveDec || str[q-1] != '.'))
		{
			str[q] = 0;
		}
		else if(str[q] == '.')
		{
			str[q] = 0;
			return;
		}
		else return;
	}
}
int32_t jwin_swapbtn_proc(int32_t msg, DIALOG* d, int32_t c)
{
	static const char* swp[nswapMAX] = {"D", "H", "LD", "LH", "B"};
	d->dp = (void*)swp[d->d1&0xF];
	//d1 is (0xF0 = old val, 0x0F = new val)
	//d2 is max val
	if(d->d2 < 2 || d->d2 > nswapMAX) return D_O_K; //Not setup yet, or bad value
	DIALOG* relproc = (DIALOG*)d->dp3;
	GUI::TextField *tf_obj = nullptr;
	if(d->d2 > nswapBOOL) tf_obj = (GUI::TextField*)relproc->dp3;
	int32_t ret = jwin_button_proc(msg, d, c);
	if(d->flags & D_SELECTED) //On selection
	{
		d->d1 = ((d->d1&0x0F)<<4) | (((d->d1&0x0F)+1)%d->d2);
		d->dp = (void*)swp[d->d1&0xF];
		d->flags &= ~D_SELECTED;
		if(tf_obj) tf_obj->refresh_cb_swap();
		if(relproc)
		{
			object_message(relproc, MSG_DRAW, 0);
		}
		object_message(d, MSG_DRAW, 0);
	}
	return ret;
}
int32_t jwin_numedit_swap_byte_proc(int32_t msg, DIALOG *d, int32_t c)
{
	DIALOG* swapbtn;
	if(d->flags&D_NEW_GUI)
	{
		swapbtn = d+1;
	}
	else swapbtn = (DIALOG*)d->dp3;
	if(!swapbtn) return D_O_K;
	if(msg==MSG_START) //Setup the swapbtn
	{
		d->bg = 0;
		swapbtn->d2 = 2; //Max states
		auto ty = swapbtn->d1&0xF;
		if(unsigned(ty) > swapbtn->d2)
			swapbtn->d1 &= ~0xF;
		swapbtn->dp3 = (void*)d;
	}
	int32_t ret = D_O_K;
	int32_t ntype = swapbtn->d1&0xF,
	    otype = swapbtn->d1>>4;
	
	char* str = (char*)d->dp;
	int32_t v = 0;
	if(msg == MSG_START)
		v = d->fg;
	else switch(otype)
	{
		case nswapDEC:
			v = atoi(str);
			break;
		case nswapHEX:
			v = zc_xtoi(str);
			break;
	}
	byte b;
	if ( v > 255 )
		b=255;
	else if ( v < 0 )
		b=0;
	else b = (byte)v;
	if(msg==MSG_CHAR && ((c&255)=='-'))
	{
		//unsigned//b = -b;
		c &= ~255;
	}
	if(unsigned(v) != b || otype != ntype || msg == MSG_START)
	{
		switch(ntype)
		{
			case nswapDEC:
				sprintf(str, "%d", b);
				break;
			case nswapHEX:
				sprintf(str, "%X", b);
				break;
		}
		if(msg != MSG_DRAW) ret |= D_REDRAWME;
	}
	
	if(d->fg != b)
	{
		d->fg = b; //Store numeric data
		GUI_EVENT(d, geUPDATE_SWAP);
	}
	switch(ntype)
	{
		case nswapDEC:
			d->d1 = 3; //3 digits max
			ret |= jwin_numedit_proc(msg, d, c);
			break;
		case nswapHEX:
			d->d1 = 2; //2 digits max
			if(msg == MSG_CHAR && isalpha(c&255)) //always capitalize
				c = (c&~255) | (toupper(c&255));
			ret |= jwin_hexedit_proc(msg, d, c);
			break;
	}
	
	swapbtn->d1 = (ntype<<4)|ntype; //Mark the type change processed
	
	return ret;
}
#define INC_TF_CURSORS(val,inc,max) \
do \
{ \
	int32_t scursor = (val & 0xFFFF)+inc; \
	int32_t ecursor = (val & 0xFFFF0000) >> 16; \
	bool valid_ecursor = ecursor != 0xFFFF; \
	if(valid_ecursor) ecursor += inc; \
	if(inc < 0) \
	{ \
		if(scursor < 0) scursor = 0; \
		if(valid_ecursor && ecursor < 0) ecursor = 0; \
	} \
	else \
	{ \
		if(scursor > max) scursor = max; \
		if(valid_ecursor && ecursor > max) ecursor = max; \
	} \
	val = scursor | (ecursor<<16); \
} while(false)
int32_t jwin_numedit_swap_sshort_proc(int32_t msg, DIALOG *d, int32_t c)
{
	const size_t maxlen = 7;
	DIALOG* swapbtn;
	if(d->flags&D_NEW_GUI)
	{
		swapbtn = d+1;
	}
	else swapbtn = (DIALOG*)d->dp3;
	if(!swapbtn) return D_O_K;
	if(msg==MSG_START) //Setup the swapbtn
	{
		d->bg = 0;
		swapbtn->d2 = 2; //Max states
		auto ty = swapbtn->d1&0xF;
		if(unsigned(ty) > swapbtn->d2)
			swapbtn->d1 &= ~0xF;
		swapbtn->dp3 = (void*)d;
	}
	int32_t ret = D_O_K;
	int32_t ntype = swapbtn->d1&0xF,
	    otype = swapbtn->d1>>4;
	
	char* str = (char*)d->dp;
	int32_t v = 0;
	if(msg == MSG_START)
		v = d->fg;
	else switch(otype)
	{
		case nswapDEC:
			v = atoi(str);
			break;
		case nswapHEX:
			v = zc_xtoi(str);
			break;
	}
	int16_t b;
	if ( v > 32767 )
		b=32767;
	else if ( v < -32768 )
		b=-32768;
	else b = (int16_t)v;
	bool queued_neg = d->bg;
	if(msg==MSG_CHAR && ((c&255)=='-'))
	{
		if(b)
		{
			b = -b;
			v = b;
			if(b<0)
			{
				if(str[0] != '-')
				{
					char buf[16] = {0};
					strcpy(buf, str);
					sprintf(str, "-%s", buf);
					INC_TF_CURSORS(d->d2,1,strlen(str));
				}
			}
			else if(str[0] == '-')
			{
				char buf[16] = {0};
				strcpy(buf, str);
				sprintf(str, "%s", buf+1);
				INC_TF_CURSORS(d->d2,-1,strlen(str));
			}
			if(msg != MSG_DRAW) ret |= D_REDRAWME;
		}
		else queued_neg = !queued_neg; //queue the negative
		c &= ~255;
		ret |= D_USED_CHAR;
	}
	if(b && queued_neg)
	{
		//b = -b; //actually, 'atoi' handles it for us.....
		queued_neg = false;
	}
	if(bool(d->bg) != queued_neg)
	{
		d->bg = queued_neg;
		if(queued_neg)
		{
			if(str[0] != '-')
			{
				char buf[16] = {0};
				strcpy(buf, str);
				sprintf(str, "-%s", buf);
				INC_TF_CURSORS(d->d2,1,strlen(str));
			}
		}
		else if(!b && str[0] == '-')
		{
			char buf[16] = {0};
			strcpy(buf, str);
			sprintf(str, "%s", buf+1);
			INC_TF_CURSORS(d->d2,-1,strlen(str));
		}
		if(msg != MSG_DRAW) ret |= D_REDRAWME;
	}
	if(v != b || otype != ntype || msg == MSG_START)
	{
		switch(ntype)
		{
			case nswapDEC:
				sprintf(str, "%d", b);
				break;
			case nswapHEX:
				if(b<0)
					sprintf(str, "-%X", -b);
				else sprintf(str, "%X", b);
				break;
		}
		d->d2 = 0xFFFF0000|strlen(str);
		if(msg != MSG_DRAW) ret |= D_REDRAWME;
	}
	
	if(d->fg != b)
	{
		d->fg = b; //Store numeric data
		GUI_EVENT(d, geUPDATE_SWAP);
	}
	bool rev_d2 = false;
	int32_t old_d2 = d->d2;
	int32_t ref_d2;
	if(msg == MSG_CHAR && queued_neg)
	{
		auto scursor = d->d2 & 0xFFFF;
		auto ecursor = (d->d2 & 0xFFFF0000) >> 16;
		if(!scursor)
		{
			rev_d2 = true;
			INC_TF_CURSORS(d->d2,1,strlen(str));
			ref_d2 = d->d2;
		}
	}
	switch(ntype)
	{
		case nswapDEC:
			d->d1 = 6; //6 digits max (incl '-')
			ret |= jwin_numedit_proc(msg, d, c);
			break;
		case nswapHEX:
			d->d1 = 5; //5 digits max (incl '-')
			if(msg == MSG_CHAR && !editproc_special_key(c) && isalpha(c&255)) //always capitalize
				c = (c&~255) | (toupper(c&255));
			ret |= jwin_hexedit_proc(msg, d, c);
			break;
	}
	if(rev_d2 && ref_d2 == d->d2)
	{
		d->d2 = old_d2;
	}
	
	swapbtn->d1 = (ntype<<4)|ntype; //Mark the type change processed
	
	return ret;
}
int32_t jwin_numedit_swap_zsint_proc(int32_t msg, DIALOG *d, int32_t c)
{
	const size_t maxlen = 13;
	DIALOG* swapbtn;
	if(d->flags&D_NEW_GUI)
	{
		swapbtn = d+1;
	}
	else swapbtn = (DIALOG*)d->dp3;
	if(!swapbtn) return D_O_K;
	if(msg==MSG_START) //Setup the swapbtn
	{
		d->bg = 0;
		swapbtn->d2 = 4; //Max states
		auto ty = swapbtn->d1&0xF;
		if(unsigned(ty) > swapbtn->d2)
			swapbtn->d1 &= ~0xF;
		swapbtn->dp3 = (void*)d;
	}
	int32_t ret = D_O_K;
	int32_t ntype = swapbtn->d1&0xF,
	    otype = swapbtn->d1>>4;
	
	char* str = (char*)d->dp;
	int64_t v = 0;
	if(msg == MSG_START)
		v = d->fg;
	else switch(otype)
	{
		case nswapDEC:
			if(char *ptr = strchr(str, '.'))
			{
				char tempstr[32] = {0};
				strcpy(tempstr, str);
				for(int32_t q = 0; q < 4; ++q)
					tempstr[strlen(str)+q]='0';
				ptr = strchr(tempstr, '.');
				*ptr=0;++ptr;*(ptr+4)=0; //Nullchar at 2 positions to limit strings
				v = atoi(tempstr);
				v *= 10000;
				if(tempstr[0] == '-')
					v -= atoi(ptr);
				else v += atoi(ptr);
			}
			else
			{
				v = atoi(str);
				v *= 10000;
			}
			break;
		case nswapHEX:
			if(char *ptr = strchr(str, '.'))
			{
				char tempstr[32] = {0};
				strcpy(tempstr, str);
				for(int32_t q = 0; q < 4; ++q)
					tempstr[strlen(str)+q]='0';
				ptr = strchr(tempstr, '.');
				*ptr=0;++ptr;*(ptr+4)=0; //Nullchar at 2 positions to limit strings
				v = zc_xtoi(tempstr);
				v *= 10000;
				if(tempstr[0] == '-')
					v -= atoi(ptr);
				else v += atoi(ptr);
			}
			else
			{
				v = zc_xtoi(str);
				v *= 10000;
			}
			break;
		case nswapLDEC:
			v = zc_atoi64(str);
			break;
		case nswapLHEX:
			v = zc_xtoi64(str);
			break;
	}
	int32_t b;
	if ( v > 2147483647 )
		b=2147483647;
	else if ( v < INT_MIN )
		b=INT_MIN;
	else b = (int32_t)v;
	bool queued_neg = d->bg;
	if(msg==MSG_CHAR && ((c&255)=='-'))
	{
		if(b)
		{
			if(b==INT_MIN)
				++b;
			b = -b;
			v = b;
			if(b<0)
			{
				if(str[0] != '-')
				{
					char buf[16] = {0};
					strcpy(buf, str);
					sprintf(str, "-%s", buf);
					INC_TF_CURSORS(d->d2,1,strlen(str));
				}
			}
			else if(str[0] == '-')
			{
				char buf[16] = {0};
				strcpy(buf, str);
				sprintf(str, "%s", buf+1);
				INC_TF_CURSORS(d->d2,-1,strlen(str));
			}
			if(msg != MSG_DRAW) ret |= D_REDRAWME;
		}
		else queued_neg = !queued_neg; //queue negative
		c &= ~255;
		ret |= D_USED_CHAR;
	}
	if(b && queued_neg)
	{
		//b = -b; //actually, 'atoi' handles it for us.....
		queued_neg = false;
	}
	if(bool(d->bg) != queued_neg)
	{
		d->bg = queued_neg;
		if(queued_neg)
		{
			if(str[0] != '-')
			{
				char buf[16] = {0};
				strcpy(buf, str);
				sprintf(str, "-%s", buf);
				INC_TF_CURSORS(d->d2,1,strlen(str));
			}
		}
		else if(!b && str[0] == '-')
		{
			char buf[16] = {0};
			strcpy(buf, str);
			sprintf(str, "%s", buf+1);
			INC_TF_CURSORS(d->d2,-1,strlen(str));
		}
		if(msg != MSG_DRAW) ret |= D_REDRAWME;
	}
	if(v != b || otype != ntype || msg == MSG_START)
	{
		switch(ntype)
		{
			case nswapDEC:
				if(b < 0)
					sprintf(str, "-%ld.%04ld", abs(b/10000L), abs(b%10000L));
				else sprintf(str, "%ld.%04ld", b/10000L, b%10000L);
				trim_trailing_0s(str);
				break;
			case nswapHEX:
				if(b<0)
					sprintf(str, "-%lX.%04ld", abs(b/10000L), abs(b%10000L));
				else sprintf(str, "%lX.%04ld", b/10000L, abs(b%10000L));
				trim_trailing_0s(str);
				break;
			case nswapLDEC:
				sprintf(str, "%d", b);
				break;
			case nswapLHEX:
				if(b<0)
					sprintf(str, "-%X", -b);
				else sprintf(str, "%X", b);
				break;
		}
		d->d2 = 0xFFFF0000|strlen(str);
		if(msg != MSG_DRAW) ret |= D_REDRAWME;
	}
	if(d->fg != b)
	{
		d->fg = b; //Store numeric data
		GUI_EVENT(d, geUPDATE_SWAP);
	}
	if(msg==MSG_CHAR && ((c&255)=='.'))
	{
		if(ntype >= nswapLDEC) //No '.' in long modes
			c&=~255;
		else
		{
			for(int32_t q = 0; str[q]; ++q)
			{
				if(str[q] == '.') //Only one '.'
				{
					c&=~255;
					break;
				}
			}
		}
	}
	bool rev_d2 = false;
	int32_t old_d2 = d->d2;
	int32_t ref_d2;
	if(msg == MSG_CHAR && queued_neg)
	{
		auto scursor = d->d2 & 0xFFFF;
		auto ecursor = (d->d2 & 0xFFFF0000) >> 16;
		if(!scursor)
		{
			rev_d2 = true;
			INC_TF_CURSORS(d->d2,1,strlen(str));
			ref_d2 = d->d2;
		}
	}
	bool areaselect = (d->d2 & 0xFFFF0000) != 0xFFFF0000;
	switch(ntype)
	{
		case nswapDEC:
			d->d1 = 12; //12 digits max (incl '-', '.')
			if(msg==MSG_CHAR && !editproc_special_key(c) && !areaselect)
			{
				int32_t p = 0;
				for(int32_t q = 0; str[q]; ++q)
				{
					if(str[q]=='.')
					{
						if((d->d2&0x0000FFFF) <= q)
							break; //typing before the '.'
						++p;
					}
					else if(p) ++p;
				}
				if(p>=5) //too many chars after '.'
					c&=~255;
			}
			ret |= jwin_numedit_proc(msg, d, c);
			break;
		case nswapHEX:
			d->d1 = 11; //11 digits max (incl '-', '.')
			if(msg==MSG_CHAR && !editproc_special_key(c))
			{
				if(!((c&255)=='.'||isxdigit(c&255)))
					c&=~255;
				else if(isxdigit(c&255) && !isdigit(c&255))
					for(int32_t q = 0; q < (d->d2&0x0000FFFF) && str[q]; ++q)
					{
						if(str[q] == '.') //No hex digits to the right of the '.'
						{
							c&=~255;
							break;
						}
					}
				if((c&255) && !areaselect)
				{
					int32_t p = 0;
					for(int32_t q = 0; str[q]; ++q)
					{
						if(str[q]=='.')
						{
							if((d->d2&0x0000FFFF) <= q)
								break; //typing before the '.'
							++p;
						}
						else if(p) ++p;
					}
					if(p>=5) //too many chars after '.'
						c&=~255;
				}
				if(isalpha(c&255)) //always capitalize
					c = (c&~255) | (toupper(c&255));
			}
			ret |= jwin_hexedit_proc(msg, d, c);
			break;
		case nswapLDEC:
			d->d1 = 11; //11 digits max (incl '-')
			ret |= jwin_numedit_proc(msg, d, c);
			break;
		case nswapLHEX:
			d->d1 = 9; //9 digits max (incl '-')
			if(msg == MSG_CHAR && !editproc_special_key(c) && isalpha(c&255)) //always capitalize
				c = (c&~255) | (toupper(c&255));
			ret |= jwin_hexedit_proc(msg, d, c);
			break;
	}
	if(rev_d2 && ref_d2 == d->d2)
	{
		d->d2 = old_d2;
	}
	
	swapbtn->d1 = (ntype<<4)|ntype; //Mark the type change processed
	
	return ret;
}
int32_t jwin_numedit_swap_zsint_nodec_proc(int32_t msg, DIALOG *d, int32_t c)
{
	const size_t maxlen = 7;
	DIALOG* swapbtn;
	if(d->flags&D_NEW_GUI)
	{
		swapbtn = d+1;
	}
	else swapbtn = (DIALOG*)d->dp3;
	if(!swapbtn) return D_O_K;
	if(msg==MSG_START) //Setup the swapbtn
	{
		d->bg = 0;
		swapbtn->d2 = 2; //Max states
		auto ty = swapbtn->d1&0xF;
		if(unsigned(ty) > swapbtn->d2)
			swapbtn->d1 &= ~0xF;
		swapbtn->dp3 = (void*)d;
	}
	int32_t ret = D_O_K;
	int32_t ntype = swapbtn->d1&0xF,
	    otype = swapbtn->d1>>4;
	
	char* str = (char*)d->dp;
	int64_t v = 0;
	if(msg == MSG_START)
		v = d->fg;
	else switch(otype)
	{
		case nswapDEC:
			v = atoi(str);
			v *= 10000;
			break;
        case nswapHEX:
			v = zc_xtoi(str);
			v *= 10000;
			break;
	}
	int32_t b;
	if ( v > 2147480000 )
		b=2147480000;
	else if ( v < -2147480000 )
		b=-2147480000;
	else b = (int32_t)v;
	bool queued_neg = d->bg;
	if(msg==MSG_CHAR && ((c&255)=='-'))
	{
		if(b)
		{
			if(b==INT_MIN)
				++b;
			b = -b;
			v = b;
			if(b<0)
			{
				if(str[0] != '-')
				{
					char buf[16] = {0};
					strcpy(buf, str);
					sprintf(str, "-%s", buf);
					INC_TF_CURSORS(d->d2,1,strlen(str));
				}
			}
			else if(str[0] == '-')
			{
				char buf[16] = {0};
				strcpy(buf, str);
				sprintf(str, "%s", buf+1);
				INC_TF_CURSORS(d->d2,-1,strlen(str));
			}
			if(msg != MSG_DRAW) ret |= D_REDRAWME;
		}
		else queued_neg = !queued_neg; //queue negative
		c &= ~255;
		ret |= D_USED_CHAR;
	}
	if(b && queued_neg)
	{
		//b = -b; //actually, 'atoi' handles it for us.....
		queued_neg = false;
	}
	if(bool(d->bg) != queued_neg)
	{
		d->bg = queued_neg;
		if(queued_neg)
		{
			if(str[0] != '-')
			{
				char buf[16] = {0};
				strcpy(buf, str);
				sprintf(str, "-%s", buf);
				INC_TF_CURSORS(d->d2,1,strlen(str));
			}
		}
		else if(!b && str[0] == '-')
		{
			char buf[16] = {0};
			strcpy(buf, str);
			sprintf(str, "%s", buf+1);
			INC_TF_CURSORS(d->d2,-1,strlen(str));
		}
		if(msg != MSG_DRAW) ret |= D_REDRAWME;
	}
	if(v != b || otype != ntype || msg == MSG_START)
	{
		switch(ntype)
		{
			case nswapDEC:
				if(b < 0)
					sprintf(str, "-%ld", abs(b/10000L));
				else sprintf(str, "%ld", b/10000L);
				break;
			case nswapHEX:
				if(b<0)
					sprintf(str, "-%lX", abs(b/10000L));
				else sprintf(str, "%lX", b/10000L);
				break;
		}
		d->d2 = 0xFFFF0000|strlen(str);
		if(msg != MSG_DRAW) ret |= D_REDRAWME;
	}
	if(d->fg != b)
	{
		d->fg = b; //Store numeric data
		GUI_EVENT(d, geUPDATE_SWAP);
	}
	if(msg==MSG_CHAR && ((c&255)=='.'))
	{
		c&=~255; //no '.' in nodec version
	}
	bool rev_d2 = false;
	int32_t old_d2 = d->d2;
	int32_t ref_d2;
	if(msg == MSG_CHAR && queued_neg)
	{
		auto scursor = d->d2 & 0xFFFF;
		auto ecursor = (d->d2 & 0xFFFF0000) >> 16;
		if(!scursor)
		{
			rev_d2 = true;
			INC_TF_CURSORS(d->d2,1,strlen(str));
			ref_d2 = d->d2;
		}
	}
	bool areaselect = (d->d2 & 0xFFFF0000) != 0xFFFF0000;
	switch(ntype)
	{
		case nswapDEC:
			d->d1 = 7; //7 digits max (incl '-')
			ret |= jwin_numedit_proc(msg, d, c);
			break;
		case nswapHEX:
			d->d1 = 6; //6 digits max (incl '-')
			if(msg==MSG_CHAR && !editproc_special_key(c))
			{
				if(!isxdigit(c&255))
					c&=~255;
				if(isalpha(c&255)) //always capitalize
					c = (c&~255) | (toupper(c&255));
			}
			ret |= jwin_hexedit_proc(msg, d, c);
			break;
	}
	if(rev_d2 && ref_d2 == d->d2)
	{
		d->d2 = old_d2;
	}
	
	swapbtn->d1 = (ntype<<4)|ntype; //Mark the type change processed
	
	return ret;
}
int32_t jwin_numedit_swap_zsint2_proc(int32_t msg, DIALOG *d, int32_t c)
{
	const size_t maxlen = 13;
	DIALOG* swapbtn;
	ASSERT(d->flags&D_NEW_GUI);
	swapbtn = d+1;
	if(!swapbtn) return D_O_K;
	GUI::TextField *tf_obj = (GUI::TextField*)d->dp3;
	if(!tf_obj) return D_O_K;
	if(msg==MSG_START) //Setup the swapbtn
	{
		d->bg = 0;
		swapbtn->d2 = 5; //Max states
		auto ty = swapbtn->d1&0xF;
		if(unsigned(ty) > swapbtn->d2)
			swapbtn->d1 &= ~0xF;
		swapbtn->dp3 = (void*)d;
	}
	int32_t ret = D_O_K;
	int32_t ntype = swapbtn->d1&0xF,
	    otype = swapbtn->d1>>4;
    if(otype==nswapBOOL || ntype == nswapBOOL)
	{
		if(otype != ntype)
		{
			tf_obj->refresh_cb_swap();
		}
		if(ntype == nswapBOOL)
		{
			swapbtn->d1 = (ntype<<4)|ntype;
			return D_O_K;
		}
	}
	
	char* str = (char*)d->dp;
	int64_t v = 0;
	if(msg == MSG_START)
		v = d->fg;
	else switch(otype)
	{
		case nswapDEC:
			if(char *ptr = strchr(str, '.'))
			{
				char tempstr[32] = {0};
				strcpy(tempstr, str);
				for(int32_t q = 0; q < 4; ++q)
					tempstr[strlen(str)+q]='0';
				ptr = strchr(tempstr, '.');
				*ptr=0;++ptr;*(ptr+4)=0; //Nullchar at 2 positions to limit strings
				v = atoi(tempstr);
				v *= 10000;
				if(tempstr[0] == '-')
					v -= atoi(ptr);
				else v += atoi(ptr);
			}
			else
			{
				v = atoi(str);
				v *= 10000;
			}
			break;
		case nswapHEX:
			if(char *ptr = strchr(str, '.'))
			{
				char tempstr[32] = {0};
				strcpy(tempstr, str);
				for(int32_t q = 0; q < 4; ++q)
					tempstr[strlen(str)+q]='0';
				ptr = strchr(tempstr, '.');
				*ptr=0;++ptr;*(ptr+4)=0; //Nullchar at 2 positions to limit strings
				v = zc_xtoi(tempstr);
				v *= 10000;
				if(tempstr[0] == '-')
					v -= atoi(ptr);
				else v += atoi(ptr);
			}
			else
			{
				v = zc_xtoi(str);
				v *= 10000;
			}
			break;
		case nswapLDEC:
			v = zc_atoi64(str);
			break;
		case nswapLHEX:
			v = zc_xtoi64(str);
			break;
		case nswapBOOL:
			v = d->fg;
			break;
	}
	int32_t b;
	if ( v > 2147483647 )
		b=2147483647;
	else if ( v < INT_MIN )
		b=INT_MIN;
	else b = (int32_t)v;
	bool queued_neg = d->bg;
	if(msg==MSG_CHAR && ((c&255)=='-'))
	{
		if(b)
		{
			if(b==INT_MIN)
				++b;
			b = -b;
			v = b;
			if(b<0)
			{
				if(str[0] != '-')
				{
					char buf[16] = {0};
					strcpy(buf, str);
					sprintf(str, "-%s", buf);
					INC_TF_CURSORS(d->d2,1,strlen(str));
				}
			}
			else if(str[0] == '-')
			{
				char buf[16] = {0};
				strcpy(buf, str);
				sprintf(str, "%s", buf+1);
				INC_TF_CURSORS(d->d2,-1,strlen(str));
			}
			if(msg != MSG_DRAW) ret |= D_REDRAWME;
		}
		else queued_neg = !queued_neg; //queue negative
		c &= ~255;
		ret |= D_USED_CHAR;
	}
	if(b && queued_neg)
	{
		//b = -b; //actually, 'atoi' handles it for us.....
		queued_neg = false;
	}
	if(bool(d->bg) != queued_neg)
	{
		d->bg = queued_neg;
		if(queued_neg)
		{
			if(str[0] != '-')
			{
				char buf[16] = {0};
				strcpy(buf, str);
				sprintf(str, "-%s", buf);
				INC_TF_CURSORS(d->d2,1,strlen(str));
			}
		}
		else if(!b && str[0] == '-')
		{
			char buf[16] = {0};
			strcpy(buf, str);
			sprintf(str, "%s", buf+1);
			INC_TF_CURSORS(d->d2,-1,strlen(str));
		}
		if(msg != MSG_DRAW) ret |= D_REDRAWME;
	}
	if(v != b || otype != ntype || msg == MSG_START)
	{
		switch(ntype)
		{
			case nswapDEC:
				if(b < 0)
					sprintf(str, "-%ld.%04ld", abs(b/10000L), abs(b%10000L));
				else sprintf(str, "%ld.%04ld", b/10000L, b%10000L);
				trim_trailing_0s(str);
				break;
			case nswapHEX:
				if(b<0)
					sprintf(str, "-%lX.%04ld", abs(b/10000L), abs(b%10000L));
				else sprintf(str, "%lX.%04ld", b/10000L, abs(b%10000L));
				trim_trailing_0s(str);
				break;
			case nswapLDEC:
				sprintf(str, "%d", b);
				break;
			case nswapLHEX:
				if(b<0)
					sprintf(str, "-%X", -b);
				else sprintf(str, "%X", b);
				break;
		}
		d->d2 = 0xFFFF0000|strlen(str);
		if(msg != MSG_DRAW) ret |= D_REDRAWME;
	}
	if(d->fg != b)
	{
		d->fg = b; //Store numeric data
		GUI_EVENT(d, geUPDATE_SWAP);
	}
	if(msg==MSG_CHAR && ((c&255)=='.'))
	{
		if(ntype >= nswapLDEC) //No '.' in long modes
			c&=~255;
		else
		{
			for(int32_t q = 0; str[q]; ++q)
			{
				if(str[q] == '.') //Only one '.'
				{
					c&=~255;
					break;
				}
			}
		}
	}
	bool rev_d2 = false;
	int32_t old_d2 = d->d2;
	int32_t ref_d2;
	if(msg == MSG_CHAR && queued_neg)
	{
		auto scursor = d->d2 & 0xFFFF;
		auto ecursor = (d->d2 & 0xFFFF0000) >> 16;
		if(!scursor)
		{
			rev_d2 = true;
			INC_TF_CURSORS(d->d2,1,strlen(str));
			ref_d2 = d->d2;
		}
	}
	bool areaselect = (d->d2 & 0xFFFF0000) != 0xFFFF0000;
	switch(ntype)
	{
		case nswapDEC:
			d->d1 = 12; //12 digits max (incl '-', '.')
			if(msg==MSG_CHAR && !editproc_special_key(c) && !areaselect)
			{
				int32_t p = 0;
				for(int32_t q = 0; str[q]; ++q)
				{
					if(str[q]=='.')
					{
						if((d->d2&0x0000FFFF) <= q)
							break; //typing before the '.'
						++p;
					}
					else if(p) ++p;
				}
				if(p>=5) //too many chars after '.'
					c&=~255;
			}
			ret |= jwin_numedit_proc(msg, d, c);
			break;
		case nswapHEX:
			d->d1 = 11; //11 digits max (incl '-', '.')
			if(msg==MSG_CHAR && !editproc_special_key(c))
			{
				if(!((c&255)=='.'||isxdigit(c&255)))
					c&=~255;
				else if(isxdigit(c&255) && !isdigit(c&255))
					for(int32_t q = 0; q < (d->d2&0x0000FFFF) && str[q]; ++q)
					{
						if(str[q] == '.') //No hex digits to the right of the '.'
						{
							c&=~255;
							break;
						}
					}
				if((c&255) && !areaselect)
				{
					int32_t p = 0;
					for(int32_t q = 0; str[q]; ++q)
					{
						if(str[q]=='.')
						{
							if((d->d2&0x0000FFFF) <= q)
								break; //typing before the '.'
							++p;
						}
						else if(p) ++p;
					}
					if(p>=5) //too many chars after '.'
						c&=~255;
				}
				if(isalpha(c&255)) //always capitalize
					c = (c&~255) | (toupper(c&255));
			}
			ret |= jwin_hexedit_proc(msg, d, c);
			break;
		case nswapLDEC:
			d->d1 = 11; //11 digits max (incl '-')
			ret |= jwin_numedit_proc(msg, d, c);
			break;
		case nswapLHEX:
			d->d1 = 9; //9 digits max (incl '-')
			if(msg == MSG_CHAR && !editproc_special_key(c) && isalpha(c&255)) //always capitalize
				c = (c&~255) | (toupper(c&255));
			ret |= jwin_hexedit_proc(msg, d, c);
			break;
	}
	if(rev_d2 && ref_d2 == d->d2)
	{
		d->d2 = old_d2;
	}
	
	swapbtn->d1 = (ntype<<4)|ntype; //Mark the type change processed
	
	if(msg==MSG_START)
		tf_obj->refresh_cb_swap();
	
	return ret;
}
int32_t jwin_numedit_noswap_zsint_proc(int32_t msg, DIALOG *d, int32_t c)
{
	const size_t maxlen = 13;
	ASSERT(d->flags&D_NEW_GUI);
	GUI::TextField *tf_obj = (GUI::TextField*)d->dp3;
	if(!tf_obj) return D_O_K;
	int32_t ret = D_O_K;
	int32_t type = tf_obj->getSwapType();
	
	char* str = (char*)d->dp;
	int64_t v = 0;
	if(msg == MSG_START)
		v = d->fg;
	else switch(type)
	{
		case nswapDEC:
			if(char *ptr = strchr(str, '.'))
			{
				char tempstr[32] = {0};
				strcpy(tempstr, str);
				for(int32_t q = 0; q < 4; ++q)
					tempstr[strlen(str)+q]='0';
				ptr = strchr(tempstr, '.');
				*ptr=0;++ptr;*(ptr+4)=0; //Nullchar at 2 positions to limit strings
				v = atoi(tempstr);
				v *= 10000;
				if(tempstr[0] == '-')
					v -= atoi(ptr);
				else v += atoi(ptr);
			}
			else
			{
				v = atoi(str);
				v *= 10000;
			}
			break;
		case nswapHEX:
			if(char *ptr = strchr(str, '.'))
			{
				char tempstr[32] = {0};
				strcpy(tempstr, str);
				for(int32_t q = 0; q < 4; ++q)
					tempstr[strlen(str)+q]='0';
				ptr = strchr(tempstr, '.');
				*ptr=0;++ptr;*(ptr+4)=0; //Nullchar at 2 positions to limit strings
				v = zc_xtoi(tempstr);
				v *= 10000;
				if(tempstr[0] == '-')
					v -= atoi(ptr);
				else v += atoi(ptr);
			}
			else
			{
				v = zc_xtoi(str);
				v *= 10000;
			}
			break;
		case nswapLDEC:
			v = zc_atoi64(str);
			break;
		case nswapLHEX:
			v = zc_xtoi64(str);
			break;
		case nswapBOOL:
			v = d->fg;
			break;
	}
	int32_t b;
	if ( v > 2147483647 )
		b=2147483647;
	else if ( v < INT_MIN )
		b=INT_MIN;
	else b = (int32_t)v;
	bool queued_neg = d->bg;
	if(msg==MSG_CHAR && ((c&255)=='-'))
	{
		if(b)
		{
			if(b==INT_MIN)
				++b;
			b = -b;
			v = b;
			if(b<0)
			{
				if(str[0] != '-')
				{
					char buf[16] = {0};
					strcpy(buf, str);
					sprintf(str, "-%s", buf);
					INC_TF_CURSORS(d->d2,1,strlen(str));
				}
			}
			else if(str[0] == '-')
			{
				char buf[16] = {0};
				strcpy(buf, str);
				sprintf(str, "%s", buf+1);
				INC_TF_CURSORS(d->d2,-1,strlen(str));
			}
			if(msg != MSG_DRAW) ret |= D_REDRAWME;
		}
		else queued_neg = !queued_neg; //queue negative
		c &= ~255;
		ret |= D_USED_CHAR;
	}
	if(b && queued_neg)
	{
		//b = -b; //actually, 'atoi' handles it for us.....
		queued_neg = false;
	}
	if(bool(d->bg) != queued_neg)
	{
		d->bg = queued_neg;
		if(queued_neg)
		{
			if(str[0] != '-')
			{
				char buf[16] = {0};
				strcpy(buf, str);
				sprintf(str, "-%s", buf);
				INC_TF_CURSORS(d->d2,1,strlen(str));
			}
		}
		else if(!b && str[0] == '-')
		{
			char buf[16] = {0};
			strcpy(buf, str);
			sprintf(str, "%s", buf+1);
			INC_TF_CURSORS(d->d2,-1,strlen(str));
		}
		if(msg != MSG_DRAW) ret |= D_REDRAWME;
	}
	if(v != b || msg == MSG_START)
	{
		switch(type)
		{
			case nswapDEC:
				if(b < 0)
					sprintf(str, "-%ld.%04ld", abs(b/10000L), abs(b%10000L));
				else sprintf(str, "%ld.%04ld", b/10000L, b%10000L);
				trim_trailing_0s(str);
				break;
			case nswapHEX:
				if(b<0)
					sprintf(str, "-%lX.%04ld", abs(b/10000L), abs(b%10000L));
				else sprintf(str, "%lX.%04ld", b/10000L, abs(b%10000L));
				trim_trailing_0s(str);
				break;
			case nswapLDEC:
				sprintf(str, "%d", b);
				break;
			case nswapLHEX:
				if(b<0)
					sprintf(str, "-%X", -b);
				else sprintf(str, "%X", b);
				break;
		}
		d->d2 = 0xFFFF0000|strlen(str);
		if(msg != MSG_DRAW) ret |= D_REDRAWME;
	}
	if(d->fg != b)
	{
		d->fg = b; //Store numeric data
		GUI_EVENT(d, geUPDATE_SWAP);
	}
	if(msg==MSG_CHAR && ((c&255)=='.'))
	{
		if(type >= nswapLDEC) //No '.' in long modes
			c&=~255;
		else
		{
			for(int32_t q = 0; str[q]; ++q)
			{
				if(str[q] == '.') //Only one '.'
				{
					c&=~255;
					break;
				}
			}
		}
	}
	bool rev_d2 = false;
	int32_t old_d2 = d->d2;
	int32_t ref_d2;
	if(msg == MSG_CHAR && queued_neg)
	{
		auto scursor = d->d2 & 0xFFFF;
		auto ecursor = (d->d2 & 0xFFFF0000) >> 16;
		if(!scursor)
		{
			rev_d2 = true;
			INC_TF_CURSORS(d->d2,1,strlen(str));
			ref_d2 = d->d2;
		}
	}
	bool areaselect = (d->d2 & 0xFFFF0000) != 0xFFFF0000;
	switch(type)
	{
		case nswapDEC:
			d->d1 = 12; //12 digits max (incl '-', '.')
			if(msg==MSG_CHAR && !editproc_special_key(c) && !areaselect)
			{
				int32_t p = 0;
				for(int32_t q = 0; str[q]; ++q)
				{
					if(str[q]=='.')
					{
						if((d->d2&0x0000FFFF) <= q)
							break; //typing before the '.'
						++p;
					}
					else if(p) ++p;
				}
				if(p>=5) //too many chars after '.'
					c&=~255;
			}
			ret |= jwin_numedit_proc(msg, d, c);
			break;
		case nswapHEX:
			d->d1 = 11; //11 digits max (incl '-', '.')
			if(msg==MSG_CHAR && !editproc_special_key(c))
			{
				if(!((c&255)=='.'||isxdigit(c&255)))
					c&=~255;
				else if(isxdigit(c&255) && !isdigit(c&255))
					for(int32_t q = 0; q < (d->d2&0x0000FFFF) && str[q]; ++q)
					{
						if(str[q] == '.') //No hex digits to the right of the '.'
						{
							c&=~255;
							break;
						}
					}
				if((c&255) && !areaselect)
				{
					int32_t p = 0;
					for(int32_t q = 0; str[q]; ++q)
					{
						if(str[q]=='.')
						{
							if((d->d2&0x0000FFFF) <= q)
								break; //typing before the '.'
							++p;
						}
						else if(p) ++p;
					}
					if(p>=5) //too many chars after '.'
						c&=~255;
				}
				if(isalpha(c&255)) //always capitalize
					c = (c&~255) | (toupper(c&255));
			}
			ret |= jwin_hexedit_proc(msg, d, c);
			break;
		case nswapLDEC:
			d->d1 = 11; //11 digits max (incl '-')
			ret |= jwin_numedit_proc(msg, d, c);
			break;
		case nswapLHEX:
			d->d1 = 9; //9 digits max (incl '-')
			if(msg == MSG_CHAR && !editproc_special_key(c) && isalpha(c&255)) //always capitalize
				c = (c&~255) | (toupper(c&255));
			ret |= jwin_hexedit_proc(msg, d, c);
			break;
	}
	if(rev_d2 && ref_d2 == d->d2)
	{
		d->d2 = old_d2;
	}
	
	if(msg==MSG_START)
		tf_obj->refresh_cb_swap();
	
	return ret;
}

/*  _calc_scroll_bar:
  *   Helps find positions of buttons on the scroll bar.
  */
void _calc_scroll_bar(int32_t h, int32_t height, int32_t listsize, int32_t offset,
                      int32_t *bh, int32_t *len, int32_t *pos)
{
    *bh = zc_max(zc_min((h-4)/2, 14), 0);
    *len = zc_max(((h - 32) * height + listsize/2) / listsize , 6);
    *pos = ((h - 32 - *len) * offset) / (listsize-height);
}

/* _handle_scrollable_click:
  *  Helper to process a click on a scrollable object.
  */

void _handle_jwin_scrollable_scroll_click(DIALOG *d, int32_t listsize, int32_t *offset, FONT *fnt)
{
    enum { top_btn, bottom_btn, bar, top_bar, bottom_bar };
    
    int32_t xx, yy;
    int32_t height = (d->h-3) / (fnt ? text_height(fnt) : 1);
    int32_t hh = d->h - 32;
    int32_t obj = bar;
    int32_t bh, len, pos;
    int32_t down = 1, last_draw = 0;
    int32_t redraw = 0, mouse_delay = 0;
    
    _calc_scroll_bar(d->h, height, listsize, *offset, &bh, &len, &pos);
    
    xx = d->x + d->w - 18;
    
    // find out which object is being clicked
    
    yy = gui_mouse_y();
    
    if(yy <= d->y+2+bh)
    {
        obj = top_btn;
        yy = d->y+2;
    }
    else if(yy >= d->y+d->h-2-bh)
    {
        obj = bottom_btn;
        yy = d->y+d->h-2-bh;
    }
    else if(d->h > 32+6)
    {
        if(yy < d->y+2+bh+pos)
            obj = top_bar;
        else if(yy >= d->y+2+bh+pos+len)
            obj = bottom_bar;
    }
    
    while(gui_mouse_b())
    {
        _calc_scroll_bar(d->h, height, listsize, *offset, &bh, &len, &pos);
        
        switch(obj)
        {
        case top_btn:
        case bottom_btn:
            down = mouse_in_rect(xx, yy, 16, bh);
            
            if(!down)
                mouse_delay = 0;
            else
            {
                if((mouse_delay&1)==0)
                {
                    if(obj==top_btn && *offset>0)
                    {
                        (*offset)--;
                        redraw = 1;
                    }
                    
                    if(obj==bottom_btn && *offset<listsize-height)
                    {
                        (*offset)++;
                        redraw = 1;
                    }
                }
                
                mouse_delay++;
            }
            
            if(down!=last_draw || redraw)
            {
                vsync();
                d->proc(MSG_DRAW, d, 0);
                draw_arrow_button(screen, xx, yy, 16, bh, obj==top_btn, down*3);
                last_draw = down;
            }
            
            break;
            
        case top_bar:
        case bottom_bar:
            if(mouse_in_rect(xx, d->y+2, 16, d->h-4))
            {
                if(obj==top_bar)
                {
                    if(gui_mouse_y() < d->y+2+bh+pos)
                        yy = *offset - height;
                }
                else
                {
                    if(gui_mouse_y() >= d->y+2+bh+pos+len)
                        yy = *offset + height;
                }
                
                if(yy < 0)
                    yy = 0;
                    
                if(yy > listsize-height)
                    yy = listsize-height;
                    
                if(yy != *offset)
                {
                    *offset = yy;
                    vsync();
                    d->proc(MSG_DRAW, d, 0);
                }
            }
            
            _calc_scroll_bar(d->h, height, listsize, *offset, &bh, &len, &pos);
            
            if(!mouse_in_rect(xx, d->y+2+bh+pos, 16, len))
                break;
                
            // fall through
            
        case bar:
        default:
            xx = gui_mouse_y() - pos;
            
            while(gui_mouse_b())
            {
                yy = (listsize * (gui_mouse_y() - xx) + hh/2) / hh;
                
                if(yy > listsize-height)
                    yy = listsize-height;
                    
                if(yy < 0)
                    yy = 0;
                
                bool should_redraw = false;
                if(yy != *offset)
                {
                    *offset = yy;
                    d->proc(MSG_DRAW, d, 0);
                    should_redraw = true;
                }
                
                /* let other objects continue to animate */
                int r = broadcast_dialog_message(MSG_IDLE, 0);
                if (r & D_REDRAWME) should_redraw = true;

                if (should_redraw)
                {
                    update_hw_screen();
                }
            }
            
            break;
            
        }                                                       // switch(obj)
        
        redraw = 0;
        
		update_hw_screen();
        // let other objects continue to animate
        broadcast_dialog_message(MSG_IDLE, 0);
    }
    
    if(last_draw==1)
    {
        draw_arrow_button(screen, xx, yy, 16, bh, obj==top_btn, 0);
    }
}

/* _handle_scrollable_scroll:
  *  Helper function to scroll through a scrollable object.
  */

static void _handle_jwin_scrollable_scroll(DIALOG *d, int32_t listsize, int32_t *index, int32_t *offset, FONT *fnt)
{
    int32_t height = (d->h-3) / text_height(fnt);
    
    if(listsize <= 0)
    {
        *index = *offset = 0;
        return;
    }
    
    // check selected item
    if(*index < 0)
        *index = 0;
    else if(*index >= listsize)
        *index = listsize - 1;
        
    // check scroll position
    while((*offset > 0) && (*offset + height > listsize))
        (*offset)--;
        
    if(*offset >= *index)
    {
        if(*index < 0)
            *offset = 0;
        else
            *offset = *index;
    }
    else
    {
        while((*offset + height - 1) < *index)
            (*offset)++;
    }
}

/* idle_cb:
  *  rest_callback() routine to keep dialogs animating nice and smoothly.
  */

static void idle_cb()
{
    broadcast_dialog_message(MSG_IDLE, 0);
}

/* _handle_listbox_click:
  *  Helper to process a click on a listbox, doing hit-testing and moving
  *  the selection.
  */

static bool _handle_jwin_listbox_click(DIALOG *d)
{
    ListData *data = (ListData *)d->dp;
    char *sel = (char *)d->dp2;
    int32_t listsize, height;
    int32_t i, j;

    data->listFunc(-1, &listsize);

    if(!listsize)
        return false;
        
    height = (d->h-3) / text_height(*data->font);
    
    i = MID(0, ((gui_mouse_y() - d->y - 4) / text_height(*data->font)),
            ((d->h-3) / text_height(*data->font) - 1));
    i += d->d2;
    
    if(i < d->d2)
        i = d->d2;
    else
    {
        if(i > d->d2 + height-1)
            i = d->d2 + height-1;
            
        if(i >= listsize)
            i = listsize-1;
    }
    
    if(gui_mouse_y() <= d->y)
        i = MAX(i-1, 0);
    else if(gui_mouse_y() >= d->y+d->h)
        i = MIN(i+1, listsize-1);
        
    if(i != d->d1)
    {
        if(sel)
        {
            if(key_shifts & (KB_SHIFT_FLAG | KB_CTRL_FLAG))
            {
                if((key_shifts & KB_SHIFT_FLAG) || (d->flags & D_INTERNAL))
                {
                    for(j=MIN(i, d->d1); j<=MAX(i, d->d1); j++)
                        sel[j] = TRUE;
                }
                else
                    sel[i] = TRUE;
            }
        }
        
        d->d1 = i;
        i = d->d2;
		
        _handle_jwin_scrollable_scroll(d, listsize, &d->d1, &d->d2, *data->font);
        
        object_message(d, MSG_DRAW, 0);
        
        if(i != d->d2)
            rest_callback(MID(10, text_height(font)*16-d->h, 100), idle_cb);
		return true;
    }
	return false;
}

/* _jwin_draw_scrollable_frame:
  *  Helper function to draw a frame for all objects with vertical scrollbars.
  */
void _jwin_draw_scrollable_frame(DIALOG *d, int32_t listsize, int32_t offset, int32_t height, int32_t type)
{
    int32_t pos, len;
    int32_t xx, yy, hh, bh;
    static BITMAP *pattern = NULL;                            // just create it once
    
    /* draw frame */
    if(type)
        // for droplists
        jwin_draw_frame(screen, d->x, d->y, d->w, d->h, FR_DARK);
    else
        jwin_draw_frame(screen, d->x, d->y, d->w, d->h, FR_DEEP);
        
    /* possibly draw scrollbar */
    if(listsize > height)
    {
        _calc_scroll_bar(d->h, height, listsize, offset, &bh, &len, &pos);
        
        xx = d->x + d->w - 18;
        
        draw_arrow_button(screen, xx, d->y+2, 16, bh, 1, 0);
        draw_arrow_button(screen, xx, d->y+d->h-2-bh, 16, bh, 0, 0);
        
        if(d->h > 32)
        {
            yy = d->y + 16;
            hh = (d->h-32);
            
            /* create and draw the scrollbar */
            if(!pattern)
                pattern = create_bitmap_ex(bitmap_color_depth(screen),2,2);
                
            putpixel(pattern, 0, 1, scheme[jcLIGHT]);
            putpixel(pattern, 1, 0, scheme[jcLIGHT]);
            putpixel(pattern, 0, 0, scheme[jcBOX]);
            putpixel(pattern, 1, 1, scheme[jcBOX]);
            
            drawing_mode(DRAW_MODE_COPY_PATTERN, pattern, 0, 0);
            rectfill(screen, xx, yy, xx+15, yy+hh-1, 0);
            solid_mode();
            
            if(d->h > 32+6)
            {
                jwin_draw_button(screen, xx, yy+pos, 16, len, 0, 1);
            }
        }
        
        if(d->flags & D_GOTFOCUS)
            _dotted_rect(d->x+2, d->y+2, d->x+d->w-19, d->y+d->h-3, scheme[jcTEXTFG], scheme[jcTEXTBG]);
    }
    else if(d->flags & D_GOTFOCUS)
        _dotted_rect(d->x+2, d->y+2, d->x+d->w-3, d->y+d->h-3, scheme[jcTEXTFG], scheme[jcTEXTBG]);
}

/*
	Effectively an overload of _jwin_draw_listbox that is used eclusively for file/abc listers.
*/
void _jwin_draw_abclistbox(DIALOG *d)
{
	int32_t height, listsize, i, len, bar, x, y, w;
	int32_t fg_color, bg_color, fg, bg;
	char *sel = (char*)d->dp2;
    char s[1024] = { 0 };
	ListData *data = (ListData *)d->dp;

	FONT* oldfont = font;
	font = *data->font;
	
	data->listFunc(-1, &listsize);
	height = (d->h-3) / text_height(font);
	bar = (listsize > height);
	w = (bar ? d->w-21 : d->w-5);
	rectfill(screen, d->x,  d->y, d->x+d->w-1, d->y+d->h+9, scheme[jcBOX]);
	fg_color = (d->flags & D_DISABLED) ? scheme[jcDISABLED_FG] : (d->fg ? d->fg : scheme[jcTEXTFG]);
	bg_color = (d->flags & D_DISABLED) ? scheme[jcDISABLED_BG] : (d->bg ? d->bg : scheme[jcTEXTBG]);
	
	rectfill(screen, d->x+2,  d->y+2, d->x+w+2, d->y+3, bg_color);
	_allegro_vline(screen, d->x+2, d->y+4, d->y+d->h-3, bg_color);
	_allegro_vline(screen, d->x+3, d->y+4, d->y+d->h-3, bg_color);
	_allegro_vline(screen, d->x+w+1, d->y+4, d->y+d->h-3, bg_color);
	_allegro_vline(screen, d->x+w+2, d->y+4, d->y+d->h-3, bg_color);
	//al_trace("Drawing %s\n", abc_keypresses);
	{
		rectfill(screen, d->x+1,  d->y+d->h+2, d->x+d->w-2, d->y+1+d->h+text_height(font), bg_color);
		strncpy(s, abc_keypresses, 1023);
		char* s2 = s;
		int32_t tw = (d->w-1);
		while(text_length(font, s2) >= tw)
		{
			++s2;
		}
		textout_ex(screen, font, s2, d->x+1, d->y+d->h+2,fg_color, bg_color);
	}
	//d->flags|=D_DIRTY;
	
	/* draw box contents */
	for(i=0; i<height; i++)
	{
	    if(d->d2+i < listsize)
	    {
	        if(d->d2+i == d->d1 && !(d->flags & D_DISABLED))
	        {
	            fg = scheme[jcSELFG];
	            bg = scheme[jcSELBG];
	        }
	        else if((sel) && (sel[d->d2+i]))
	        {
	            fg = scheme[jcDISABLED_FG];
	            bg = scheme[jcSELBG];
	        }
	        else
	        {
	            fg = fg_color;
	            bg = bg_color;
	        }

	        strncpy(s, data->listFunc(i+d->d2, NULL), 1023);
	        x = d->x + 4;
	        y = d->y + 4 + i*text_height(*data->font);
	        //         text_mode(bg);
	        rectfill(screen, x, y, x+7, y+text_height(*data->font)-1, bg);
	        x += 8;
	        len = (int32_t)strlen(s);
	        
	        while(text_length(*data->font, s) >= d->w - (bar ? 26 : 10))
	        {
	            len--;
	            s[len] = 0;
	        }
	        
	        textout_ex(screen, *data->font, s, x, y, fg,bg);
	        x += text_length(*data->font, s);
	        
	        if(x <= d->x+w)
	            rectfill(screen, x, y, d->x+w, y+text_height(*data->font)-1, bg);
	    }
	    else
	        rectfill(screen, d->x+2,  d->y+4+i*text_height(*data->font),
	                 d->x+w+2, d->y+3+(i+1)*text_height(*data->font), bg_color);
	}
	
	if(d->y+4+i*text_height(font) <= d->y+d->h-3)
	    rectfill(screen, d->x+2, d->y+4+i*text_height(*data->font),
	             d->x+w+2, d->y+d->h-3, bg_color);
	             
	/* draw frame, maybe with scrollbar */
	_jwin_draw_scrollable_frame(d, listsize, d->d2, height, (d->flags&D_USER)?1:0);
	
	font = oldfont;
}

/* _jwin_draw_listbox:
  *  Helper function to draw a listbox object.
  */
void _jwin_draw_listbox(DIALOG *d)
{
    int32_t height, listsize, i, len, bar, x, y, w;
    int32_t fg_color, bg_color, fg, bg;
    char *sel = (char*)d->dp2;
    char s[1024] = {0};
    ListData *data = (ListData *)d->dp;

    data->listFunc(-1, &listsize);
    height = (d->h-3) / text_height(*data->font);
    bar = (listsize > height);
    w = (bar ? d->w-21 : d->w-5);
    fg_color = (d->flags & D_DISABLED) ? scheme[jcDISABLED_FG] : (d->fg ? d->fg : scheme[jcTEXTFG]);
    bg_color = (d->flags & D_DISABLED) ? scheme[jcDISABLED_BG] : (d->bg ? d->bg : scheme[jcTEXTBG]);
    
    rectfill(screen, d->x+2,  d->y+2, d->x+w+2, d->y+3, bg_color);
    _allegro_vline(screen, d->x+2, d->y+4, d->y+d->h-3, bg_color);
    _allegro_vline(screen, d->x+3, d->y+4, d->y+d->h-3, bg_color);
    _allegro_vline(screen, d->x+w+1, d->y+4, d->y+d->h-3, bg_color);
    _allegro_vline(screen, d->x+w+2, d->y+4, d->y+d->h-3, bg_color);
	//al_trace("Drawing %s\n", abc_keypresses);
    //d->flags|=D_DIRTY;
    
    /* draw box contents */
    for(i=0; i<height; i++)
    {
        if(d->d2+i < listsize)
        {
            if(d->d2+i == d->d1 && !(d->flags & D_DISABLED))
            {
                fg = scheme[jcSELFG];
                bg = scheme[jcSELBG];
            }
            else if((sel) && (sel[d->d2+i]))
            {
                fg = scheme[jcMEDDARK];
                bg = scheme[jcSELBG];
            }
            else
            {
                fg = fg_color;
                bg = bg_color;
            }

            strncpy(s, data->listFunc(i+d->d2, NULL), 1023);
            x = d->x + 4;
            y = d->y + 4 + i*text_height(*data->font);
            //         text_mode(bg);
            rectfill(screen, x, y, x+7, y+text_height(*data->font)-1, bg);
            x += 8;
            len = (int32_t)strlen(s);
            
            while(len > 0 && text_length(*data->font, s) >= d->w - (bar ? 26 : 10))
            {
                len--;
                s[len] = 0;
            }
            
            textout_ex(screen, *data->font, s, x, y, fg,bg);
            x += text_length(*data->font, s);
            
            if(x <= d->x+w)
                rectfill(screen, x, y, d->x+w, y+text_height(*data->font)-1, bg);
        }
        else
            rectfill(screen, d->x+2,  d->y+4+i*text_height(*data->font),
                     d->x+w+2, d->y+3+(i+1)*text_height(*data->font), bg_color);
    }
    
    if(d->y+4+i*text_height(font) <= d->y+d->h-3)
        rectfill(screen, d->x+2, d->y+4+i*text_height(*data->font),
                 d->x+w+2, d->y+d->h-3, bg_color);
                 
    /* draw frame, maybe with scrollbar */
    _jwin_draw_scrollable_frame(d, listsize, d->d2, height, (d->flags&D_USER)?1:0);
}

/* jwin_list_proc:
  *  A list box object. The dp field points to a ListData struct containing
  *  a function which it will call
  *  to obtain information about the list. This should follow the form:
  *     char *<list_func_name> (int32_t index, int32_t *list_size);
  *  If index is zero or positive, the function should return a pointer to
  *  the string which is to be displayed at position index in the list. If
  *  index is  negative, it should return null and list_size should be set
  *  to the number of items in the list. The list box object will allow the
  *  user to scroll through the list and to select items list by clicking
  *  on them, and if it has the input focus also by using the arrow keys. If
  *  the D_EXIT flag is set, double clicking on a list item will cause it to
  *  close the dialog. The index of the selected item is held in the d1
  *  field, and d2 is used to store how far it has scrolled through the list.
  */
int32_t jwin_list_proc(int32_t msg, DIALOG *d, int32_t c)
{
    ListData *data = (ListData *)d->dp;
    int32_t listsize, i, bottom, height, bar, orig;
    char *sel = (char *)d->dp2;
    int32_t redraw = FALSE;
    
    switch(msg)
    {
    
    case MSG_START:
        data->listFunc(-1, &listsize);
        _handle_jwin_scrollable_scroll(d, listsize, &d->d1, &d->d2, *data->font);
        break;
        
    case MSG_DRAW:
        _jwin_draw_listbox(d);
        break;
        
    case MSG_CLICK:
        data->listFunc(-1, &listsize);
        height = (d->h-3) / text_height(*data->font);
        bar = (listsize > height);
        
        if((!bar) || (gui_mouse_x() < d->x+d->w-18))
        {
            if((sel) && (!(key_shifts & KB_CTRL_FLAG)))
            {
                for(i=0; i<listsize; i++)
                {
                    if(sel[i])
                    {
                        redraw = TRUE;
                        sel[i] = FALSE;
                    }
                }
                
                if(redraw)
                {
                    object_message(d, MSG_DRAW, 0);
                }
            }
            
            if(_handle_jwin_listbox_click(d)) GUI_EVENT(d, geCHANGE_SELECTION);
            
            bool rightClicked=(gui_mouse_b()&2)!=0;
            while(gui_mouse_b())
            {
                broadcast_dialog_message(MSG_IDLE, 0);
                d->flags |= D_INTERNAL;
				bool should_redraw = false;
				if(_handle_jwin_listbox_click(d))
				{
					d->flags &= ~D_INTERNAL;
					GUI_EVENT(d, geCHANGE_SELECTION);
					should_redraw = true;
				}
				d->flags &= ~D_INTERNAL;
                
				/* let other objects continue to animate */
				int r = broadcast_dialog_message(MSG_IDLE, 0);
				if (r & D_REDRAWME) should_redraw = true;

				if (should_redraw)
				{
					update_hw_screen();
				}
            }
            
            if(rightClicked)
			{
				GUI_EVENT(d, geRCLICK);
				if((d->flags&(D_USER<<1))!=0 && d->dp3)
				{
					typedef void (*funcType)(int32_t /* index */, int32_t /* x */, int32_t /* y */);
					funcType func=reinterpret_cast<funcType>(d->dp3);
					func(d->d1, gui_mouse_x(), gui_mouse_y());
				}
			}
            
            if(d->flags & D_USER)
            {
                if(listsize)
                {
                    clear_keybuf();
                    return D_CLOSE;
                }
            }

			return D_REDRAWME;
        }
        else
        {
            _handle_jwin_scrollable_scroll_click(d, listsize, &d->d2, *data->font);
        }
        
        break;
        
    case MSG_DCLICK:
        // Ignore double right-click
        if((gui_mouse_b()&2)!=0)
            break;

        data->listFunc(-1, &listsize);
        height = (d->h-3) / text_height(*data->font);
        bar = (listsize > height);
        
        if((!bar) || (gui_mouse_x() < d->x+d->w-18))
        {
			if(listsize)
			{
				i = d->d1;
				object_message(d, MSG_CLICK, 0);
				
				if(i == d->d1)
				{
					if(d->flags & D_EXIT)
						return D_CLOSE;
					else GUI_EVENT(d, geDCLICK);
				}
			}
        }
        
        break;
        
    case MSG_KEY:
        data->listFunc(-1, &listsize);

        if((listsize) && (d->flags & D_EXIT))
            return D_CLOSE;
            
        break;
        
    case MSG_WANTFOCUS:
        return D_WANTFOCUS;
        
    case MSG_WANTWHEEL:
        return 1;

    case MSG_WHEEL:
        data->listFunc(-1, &listsize);
        height = (d->h-4) / text_height(*data->font);
        
        if(height < listsize)
        {
            int32_t delta = (height > 3) ? 3 : 1;
            
            if(c > 0)
            {
                i = MAX(0, d->d2-delta);
            }
            else
            {
                i = MIN(listsize-height, d->d2+delta);
            }
            
            if(i != d->d2)
            {
                d->d2 = i;
                object_message(d, MSG_DRAW, 0);
				GUI_EVENT(d, geCHANGE_SELECTION);
				return D_REDRAWME;
            }
        }
        
        break;
        
    case MSG_CHAR:
        data->listFunc(-1,&listsize);

        if(listsize)
        {
            c >>= 8;
            
            bottom = d->d2 + (d->h-3)/text_height(*data->font) - 1;
            
            if(bottom >= listsize-1)
                bottom = listsize-1;
                
            orig = d->d1;
            
            if(c == KEY_UP)
                d->d1--;
            else if(c == KEY_DOWN)
                d->d1++;
            else if(c == KEY_HOME)
                d->d1 = 0;
            else if(c == KEY_END)
                d->d1 = listsize-1;
            else if(c == KEY_PGUP)
            {
                if(d->d1 > d->d2)
                    d->d1 = d->d2;
                else
                    d->d1 -= (bottom - d->d2);
            }
            else if(c == KEY_PGDN)
            {
                if(d->d1 < bottom)
                    d->d1 = bottom;
                else
                    d->d1 += (bottom - d->d2);
            }
            else
                return D_O_K;
                
            if(sel)
            {
                if(!(key_shifts & (KB_SHIFT_FLAG | KB_CTRL_FLAG)))
                {
                    for(i=0; i<listsize; i++)
                        sel[i] = FALSE;
                }
                else if(key_shifts & KB_SHIFT_FLAG)
                {
                    for(i=MIN(orig, d->d1); i<=MAX(orig, d->d1); i++)
                    {
                        if(key_shifts & KB_CTRL_FLAG)
                            sel[i] = (i != d->d1);
                        else
                            sel[i] = TRUE;
                    }
                }
            }
            
            /* if we changed something, better redraw... !Also bounds the index! */
            _handle_jwin_scrollable_scroll(d, listsize, &d->d1, &d->d2, *data->font);
			
			GUI_EVENT(d, geCHANGE_SELECTION);
            
			if (d->d1 != orig)
				d->flags |= D_DIRTY;
            return D_USED_CHAR;
        }
        
        break;
    }
    
    return D_O_K;
}


/* 
	Effectively an overload of jwin_list_proc that i used eclusively for abc lists. 
	This calls the appropriate form of drawing for those listers. 
*/
int32_t jwin_do_abclist_proc(int32_t msg, DIALOG *d, int32_t c)
{
    ListData *data = (ListData *)d->dp;
    int32_t listsize, i, bottom, height, bar, orig, h;
	int32_t ret = D_O_K;
	bool revert_size = false;
	if((d->flags & D_RESIZED) == 0)
	{
		h = d->h;
		d->h -= text_height(*data->font);
		d->flags |= D_RESIZED;
		revert_size = true;
    }
	char *sel = (char *)d->dp2;
    int32_t redraw = FALSE;
    
    switch(msg)
    {
    
    case MSG_START:
        data->listFunc(-1, &listsize);
        _handle_jwin_scrollable_scroll(d, listsize, &d->d1, &d->d2, *data->font);
        break;
        
    case MSG_DRAW:
        _jwin_draw_abclistbox(d);
        break;
        
    case MSG_CLICK:
		if(gui_mouse_y() > (d->y+d->h-1))
		{
			if(gui_mouse_y() > (d->y+d->h+2))
			{
				//Clicked on the box displaying the patternmatch
			}
			else {} //Clicked between the lister and patternmatch
		}
		else //Clicked the lister
		{
			data->listFunc(-1, &listsize);
			height = (d->h-3) / text_height(*data->font);
			bar = (listsize > height);
			
			if((!bar) || (gui_mouse_x() < d->x+d->w-18))
			{
				if((sel) && (!(key_shifts & KB_CTRL_FLAG)))
				{
					for(i=0; i<listsize; i++)
					{
						if(sel[i])
						{
							redraw = TRUE;
							sel[i] = FALSE;
						}
					}
					
					if(redraw)
					{
						object_message(d, MSG_DRAW, 0);
					}
				}
				
				if(_handle_jwin_listbox_click(d)) GUI_EVENT(d, geCHANGE_SELECTION);
				
				bool rightClicked=(gui_mouse_b()&2)!=0;
				while(gui_mouse_b())
				{
					broadcast_dialog_message(MSG_IDLE, 0);
					d->flags |= D_INTERNAL;
					if(_handle_jwin_listbox_click(d))
					{
						d->flags &= ~D_INTERNAL;
						GUI_EVENT(d, geCHANGE_SELECTION);
						update_hw_screen();
					}
					d->flags &= ~D_INTERNAL;
					rest(1);
				}
				
				if(rightClicked)
				{
					GUI_EVENT(d, geRCLICK);
					if((d->flags&(D_USER<<1))!=0 && d->dp3)
					{
						typedef void (*funcType)(int32_t /* index */, int32_t /* x */, int32_t /* y */);
						funcType func=reinterpret_cast<funcType>(d->dp3);
						func(d->d1, gui_mouse_x(), gui_mouse_y());
					}
				}
				
				if(d->flags & D_USER)
				{
					if(listsize)
					{
						clear_keybuf();
						ret = D_CLOSE;
					}
				}

				return D_REDRAWME;
			}
			else
			{
				_handle_jwin_scrollable_scroll_click(d, listsize, &d->d2, *data->font);
			}
		}
        break;
        
    case MSG_DCLICK:
        // Ignore double right-click
        if((gui_mouse_b()&2)!=0)
            break;
		
        if(gui_mouse_y() > (d->y+d->h-1))
		{
			if(gui_mouse_y() > (d->y+d->h+2))
			{
				//Clicked on the box displaying the patternmatch
			}
			else {} //Clicked between the lister and patternmatch
		}
		else //Clicked the lister
		{
			data->listFunc(-1, &listsize);
			height = (d->h-3) / text_height(*data->font);
			bar = (listsize > height);
			
			if((!bar) || (gui_mouse_x() < d->x+d->w-18))
			{
				if(listsize)
				{
					i = d->d1;
					object_message(d, MSG_CLICK, 0);
					
					if(i == d->d1)
					{
						if(d->flags & D_EXIT)
							ret = D_CLOSE;
						else GUI_EVENT(d, geDCLICK);
					}
				}
			}
		}
        break;
        
    case MSG_KEY:
        data->listFunc(-1, &listsize);

        if((listsize) && (d->flags & D_EXIT))
            ret = D_CLOSE;
            
        break;
        
    case MSG_WANTFOCUS:
        ret = D_WANTFOCUS;
		break;
        
    case MSG_WANTWHEEL:
        return 1;

    case MSG_WHEEL:
        data->listFunc(-1, &listsize);
        height = (d->h-4) / text_height(*data->font);
        
        if(height < listsize)
        {
            int32_t delta = (height > 3) ? 3 : 1;
            
            if(c > 0)
            {
                i = MAX(0, d->d2-delta);
            }
            else
            {
                i = MIN(listsize-height, d->d2+delta);
            }
            
            if(i != d->d2)
            {
                d->d2 = i;
                object_message(d, MSG_DRAW, 0);
				GUI_EVENT(d, geCHANGE_SELECTION);
				ret |= D_REDRAWME;
            }
        }
        
        break;
        
    case MSG_CHAR:
        data->listFunc(-1,&listsize);

        if(listsize)
        {
            c >>= 8;
            
            bottom = d->d2 + (d->h-3)/text_height(*data->font) - 1;
            
            if(bottom >= listsize-1)
                bottom = listsize-1;
                
            orig = d->d1;
            
            if(c == KEY_UP)
                d->d1--;
            else if(c == KEY_DOWN)
                d->d1++;
            else if(c == KEY_HOME)
                d->d1 = 0;
            else if(c == KEY_END)
                d->d1 = listsize-1;
            else if(c == KEY_PGUP)
            {
                if(d->d1 > d->d2)
                    d->d1 = d->d2;
                else
                    d->d1 -= (bottom - d->d2);
            }
            else if(c == KEY_PGDN)
            {
                if(d->d1 < bottom)
                    d->d1 = bottom;
                else
                    d->d1 += (bottom - d->d2);
            }
            else
                break; //return D_O_K;
                
            if(sel)
            {
                if(!(key_shifts & (KB_SHIFT_FLAG | KB_CTRL_FLAG)))
                {
                    for(i=0; i<listsize; i++)
                        sel[i] = FALSE;
                }
                else if(key_shifts & KB_SHIFT_FLAG)
                {
                    for(i=MIN(orig, d->d1); i<=MAX(orig, d->d1); i++)
                    {
                        if(key_shifts & KB_CTRL_FLAG)
                            sel[i] = (i != d->d1);
                        else
                            sel[i] = TRUE;
                    }
                }
            }
            
            /* if we changed something, better redraw... */
            _handle_jwin_scrollable_scroll(d, listsize, &d->d1, &d->d2, *data->font);
			
			GUI_EVENT(d, geCHANGE_SELECTION);
			
			if (d->d1 != orig)
				d->flags |= D_DIRTY;
            ret = D_USED_CHAR;
        }
        
        break;
    }
    if(revert_size)
	{
		d->h = h;
		d->flags &= ~D_RESIZED;
    }
	return ret;
}

/* _jwin_draw_textbox:
  *  Helper function to draw a textbox object.
  */
void _jwin_draw_textbox(char *thetext, int32_t *listsize, int32_t draw, int32_t offset,
                        int32_t wword, int32_t tabsize, int32_t x, int32_t y, int32_t w, int32_t h,
                        int32_t disabled)
{
    int32_t fg = scheme[jcTEXTFG];
    int32_t bg = scheme[jcTEXTBG];
    int32_t y1 = y+4;
    int32_t x1;
    int32_t len;
    int32_t ww = w-10;
    char s[16] = {0};
    char text[16] = {0};
    char space[16] = {0};
    char *printed = text;
    char *scanned = text;
    char *oldscan = text;
    char *ignore = NULL;
    char *tmp, *ptmp;
    int32_t width;
    int32_t line = 0;
    int32_t i = 0;
    int32_t noignore;
    //   int32_t rtm;
    
    usetc(s+usetc(s, '.'), 0);
    usetc(text+usetc(text, ' '), 0);
    usetc(space+usetc(space, ' '), 0);
    
    /* find the correct text */
    if(thetext != NULL)
    {
        printed = thetext;
        scanned = thetext;
    }
    
    /* choose the text color */
    if(disabled)
    {
        fg = scheme[jcDISABLED_FG];
        bg = scheme[jcDISABLED_BG];
    }
    
    /* do some drawing setup */
    if(draw)
    {
        /* initial start blanking at the top */
        rectfill(screen, x+2, y+2, x+w-2, y1-1, bg);
    }
    
    //   rtm = text_mode(bg);
    
    /* loop over the entire string */
    for(;;)
    {
        width = 0;
        
        /* find the next break */
        while(ugetc(scanned))
        {
            /* check for a forced break */
            if(ugetc(scanned) == '\n')
            {
                scanned += uwidth(scanned);
                
                /* we are done parsing the line end */
                break;
            }
            
            /* the next character length */
            usetc(s+usetc(s, ugetc(scanned)), 0);
            len = text_length(font, s);
            
            /* modify length if its a tab */
            if(ugetc(s) == '\t')
                len = tabsize * text_length(font, space);
                
            /* check for the end of a line by excess width of next char */
            if(width+len >= ww)
            {
                /* we have reached end of line do we go back to find start */
                if(wword)
                {
                    /* remember where we were */
                    oldscan = scanned;
                    noignore = FALSE;
                    
                    /* go backwards looking for start of word */
                    while(!uisspace(ugetc(scanned)))
                    {
                        /* don't wrap too far */
                        if(scanned == printed)
                        {
                            /* the whole line is filled, so stop here */
                            tmp = ptmp = scanned;
                            
                            while(ptmp != oldscan)
                            {
                                ptmp = tmp;
                                tmp += uwidth(tmp);
                            }
                            
                            scanned = ptmp;
                            noignore = TRUE;
                            break;
                        }
                        
                        /* look further backwards to wrap */
                        tmp = ptmp = printed;
                        
                        while(tmp < scanned)
                        {
                            ptmp = tmp;
                            tmp += uwidth(tmp);
                        }
                        
                        scanned = ptmp;
                    }
                    
                    /* put the space at the end of the line */
                    if(!noignore)
                    {
                        ignore = scanned;
                        scanned += uwidth(scanned);
                    }
                    else
                        ignore = NULL;
                        
                    /* check for endline at the convenient place */
                    if(ugetc(scanned) == '\n')
                        scanned += uwidth(scanned);
                }
                
                /* we are done parsing the line end */
                break;
            }
            
            /* the character can be added */
            scanned += uwidth(scanned);
            width += len;
        }
        
        /* check if we are to print it */
        if((draw) && (line >= offset) && (y1+text_height(font) < (y+h-3)))
        {
            x1 = x+4;
            
            /* the initial blank bit */
            rectfill(screen, x+2, y1, x1-1, y1+text_height(font), bg);
            
            /* print up to the marked character */
            while(printed != scanned)
            {
                /* do special stuff for each character */
                switch(ugetc(printed))
                {
                
                case '\r':
                case '\n':
                    /* don't print endlines in the text */
                    break;
                    
                    /* possibly expand the tabs */
                case '\t':
                    for(i=0; i<tabsize; i++)
                    {
                        usetc(s+usetc(s, ' '), 0);
                        textout_ex(screen, font, s, x1, y1, fg,bg);
                        x1 += text_length(font, s);
                    }
                    
                    break;
                    
                    /* print a normal character */
                default:
                    if(printed != ignore)
                    {
                        usetc(s+usetc(s, ugetc(printed)), 0);
                        textout_ex(screen, font, s, x1, y1, fg,bg);
                        x1 += text_length(font, s);
                    }
                }
                
                /* goto the next character */
                printed += uwidth(printed);
            }
            
            /* the last blank bit */
            if(x1 <= x+w-3)
                rectfill(screen, x1, y1, x+w-2, y1+text_height(font)-1, bg);
                
            /* print the line end */
            y1 += text_height(font);
        }
        
        printed = scanned;
        
        /* we have done a line */
        line++;
        
        /* check if we are at the end of the string */
        if(!ugetc(printed))
        {
            /* the under blank bit */
            if(draw)
                rectfill(screen, x+1, y1, x+w-2, y+h-1, bg);
                
            /* tell how many lines we found */
            *listsize = line;
            //	 text_mode(rtm);
            return;
        }
    }
    
    //   text_mode(rtm);
}

/* jwin_textbox_proc:
  *  A text box object. The dp field points to a char * which is the text
  *  to be displayed in the text box. If the text is long, there will be
  *  a vertical scrollbar on the right hand side of the object which can
  *  be used to scroll through the text. The default is to print the text
  *  with word wrapping, but if the D_SELECTED flag is set, the text will
  *  be printed with character wrapping. The d1 field is used internally
  *  to store the number of lines of text, and d2 is used to store how far
  *  it has scrolled through the text.
  */
int32_t jwin_textbox_proc(int32_t msg, DIALOG *d, int32_t c)
{
    int32_t height, bar, ret = D_O_K;
    int32_t start, top, bottom,l;
    int32_t used, delta;
    //   int32_t fg_color = (d->flags & D_DISABLED) ? scheme[jcDISABLED_FG] : d->fg;
    
    FONT *oldfont=NULL;
    
    if(d->dp2!=NULL)
    {
        oldfont=font;
        font=(FONT*)d->dp2;
    }
    
    /* calculate the actual height */
    height = (d->h-4) / text_height(font);
    
    switch(msg)
    {
    
    case MSG_START:
        /* measure how many lines of text we contain */
        _jwin_draw_textbox((char*)d->dp, &d->d1,
                           0,                                                  /* DONT DRAW anything */
                           d->d2, !(d->flags & D_SELECTED), 8,
                           d->x, d->y, d->w, d->h,
                           (d->flags & D_DISABLED));
        break;
        
    case MSG_DRAW:
        /* tell the object to sort of draw, but only calculate the listsize */
        _jwin_draw_textbox((char*)d->dp, &d->d1,
                           0,                                                  /* DONT DRAW anything */
                           d->d2, !(d->flags & D_SELECTED), 8,
                           d->x, d->y, d->w, d->h,
                           (d->flags & D_DISABLED));
                           
        if(d->d1 > height)
        {
            bar = 16;
        }
        else
        {
            bar = 0;
            d->d2 = 0;
        }
        
        /* now do the actual drawing */
        _jwin_draw_textbox((char*)d->dp, &d->d1, 1, d->d2,
                           !(d->flags & D_SELECTED), 8,
                           d->x, d->y, d->w-bar-1, d->h,
                           (d->flags & D_DISABLED));
                           
        /* draw the frame around */
        _jwin_draw_scrollable_frame(d, d->d1, d->d2, height, 0);
        break;
        
    case MSG_CLICK:
        /* figure out if it's on the text or the scrollbar */
        bar = (d->d1 > height);
        
        if((!bar) || (gui_mouse_x() < d->x+d->w-18))
        {
            /* clicked on the text area */
            ret = D_O_K;
        }
        else
        {
            /* clicked on the scroll area */
            _handle_jwin_scrollable_scroll_click(d, d->d1, &d->d2, font);
        }
        
        break;
        
    case MSG_WANTWHEEL:
        return 1;

    case MSG_WHEEL:
        l = (d->h-8)/text_height(font);
        delta = (l > 3) ? 3 : 1;
        
        // scroll, making sure that the list stays in bounds
        start = d->d2;
        d->d2 = (c > 0) ? MAX(0, d->d2-delta) : MIN(d->d1-l, d->d2+delta);
        
        // if we changed something, better redraw...
        if(d->d2 != start)
        {
            d->flags |= D_DIRTY;
        }
        
        ret = D_O_K;
        break;
        
    case MSG_CHAR:
        start = d->d2;
        used = D_USED_CHAR;
        
        if(d->d1 > 0)
        {
            if(d->d2 > 0)
                top = d->d2+1;
            else
                top = 0;
                
            l = (d->h-3)/text_height(font);
            
            bottom = d->d2 + l - 1;
            
            if(bottom >= d->d1-1)
                bottom = d->d1-1;
            else
                bottom--;
                
            if((c>>8) == KEY_UP)
                d->d2--;
            else if((c>>8) == KEY_DOWN)
                d->d2++;
            else if((c>>8) == KEY_HOME)
                d->d2 = 0;
            else if((c>>8) == KEY_END)
                d->d2 = d->d1-l;
            else if((c>>8) == KEY_PGUP)
                d->d2 = d->d2-(bottom-top);
            else if((c>>8) == KEY_PGDN)
                d->d2 = d->d2+(bottom-top);
            else
                used = D_O_K;
                
            /* make sure that the list stays in bounds */
            if(d->d2 > d->d1-l)
                d->d2 = d->d1-l;
                
            if(d->d2 < 0)
                d->d2 = 0;
        }
        else
            used = D_O_K;
            
        /* if we changed something, better redraw... */
        if(d->d2 != start)
        {
            d->proc(MSG_DRAW, d, 0);
        }
        
        ret = used;
        break;
        
    case MSG_WANTFOCUS:
    
        /* if we don't have a scrollbar we can't do anything with the focus */
        if(d->d1 > height)
            ret = D_WANTFOCUS;
            
        break;
        
    default:
        ret = D_O_K;
    }
    
    if(d->dp2!=NULL)
    {
        font=oldfont;
    }
    
    return ret;
}

/* jwin_slider_proc:
  *  A slider control object. This object returns a value in d2, in the
  *  range from 0 to d1. It will display as a vertical slider if h is
  *  greater than or equal to w; otherwise, it will display as a horizontal
  *  slider. dp can contain an optional bitmap to use for the slider handle;
  *  dp2 can contain an optional callback function, which is called each
  *  time d2 changes. The callback function should have the following
  *  prototype:
  *
  *  int32_t function(void *dp3, int32_t d2);
  *
  *  The d_slider_proc object will return the value of the callback function.
  */
int32_t jwin_slider_proc(int32_t msg, DIALOG *d, int32_t c)
{
    BITMAP *slhan = NULL;
    int32_t sfg;                /* slider foreground color */
    int32_t vert = TRUE;        /* flag: is slider vertical? */
    int32_t hh = 7;             /* handle height (width for horizontal sliders) */
    int32_t hmar;               /* handle margin */
    int32_t slp;                /* slider position */
    int32_t irange;
    int32_t slx, sly, slh, slw;
    fixed slratio, slmax, slpos;
    ASSERT(d);
    
    /* check for slider direction */
    if(d->h < d->w)
    {
        vert = FALSE;
    }
    
    /* set up the metrics for the control */
    if(d->dp != NULL)
    {
        slhan = (BITMAP *)d->dp;
        
        if(vert)
        {
            hh = slhan->h;
        }
        else
        {
            hh = slhan->w;
        }
    }
    
    hmar = hh/2;
    irange = (vert) ? d->h : d->w;
    slmax = itofix(irange-hh);
    slratio = slmax / (d->d1);
    slpos = slratio * d->d2;
    slp = fixtoi(slpos);
    
    switch(msg)
    {
    case MSG_DRAW:
        //      sfg = (d->flags & D_DISABLED) ? scheme[jcDISABLED_FG] : scheme[jcBOXFG];
        sfg = (d->flags & D_DISABLED) ? scheme[jcDISABLED_FG] : scheme[jcBOXFG];
        
        if(vert)
        {
            rectfill(screen, d->x, d->y, d->x+d->w/2-2, d->y+d->h, scheme[jcBOX]);
            rectfill(screen, d->x+d->w/2-1, d->y, d->x+d->w/2+1, d->y+d->h, sfg);
            rectfill(screen, d->x+d->w/2+2, d->y, d->x+d->w, d->y+d->h, scheme[jcBOX]);
        }
        else
        {
            rectfill(screen, d->x, d->y, d->x+d->w, d->y+d->h/2-2, scheme[jcBOX]);
            rectfill(screen, d->x, d->y+d->h/2-1, d->x+d->w, d->y+d->h/2+1, sfg);
            rectfill(screen, d->x, d->y+d->h/2+2, d->x+d->w, d->y+d->h, scheme[jcBOX]);
        }
        
        if(d->flags & D_GOTFOCUS)
        {
            _dotted_rect(d->x, d->y, d->x+d->w, d->y+d->h, sfg, scheme[jcBOX]);
        }
        
        /* okay, background and slot are drawn, now draw the handle */
        if(slhan)
        {
            if(vert)
            {
                slx = d->x+(d->w/2)-(slhan->w/2);
                sly = d->y+d->h-(hh+slp);
            }
            else
            {
                slx = d->x+slp;
                sly = d->y+(d->h/2)-(slhan->h/2);
            }
            
            draw_sprite(screen, slhan, slx, sly);
        }
        else
        {
            /* draw default handle */
            if(vert)
            {
                slx = d->x;
                sly = d->y+d->h-(hh+slp);
                slw = d->w;
                slh = hh;
            }
            else
            {
                slx = d->x+slp;
                sly = d->y;
                slw = hh;
                slh = d->h;
            }
            
            jwin_draw_button(screen, slx, sly, slw+1, slh+1, d->flags&D_DISABLED?3:0, 0);
        }
        
        break;
        
    default:
        return d_jslider_proc(msg, d, c);
    }
    
    return D_O_K;
}

const char* rowpref(int32_t row, bool alt)
{
	static const char *lcol = "Level Colors", *syscol = "System Colors", *bosscol = "Boss Colors", *thmcol = "Theme Colors", *nlcol="";
	switch(row)
	{
		case 2: case 3: case 4: case 9:
			return lcol;
		case 14:
			return alt ? syscol : bosscol;
		case 15:
			return thmcol;
		default:
			return nlcol;
	}
}

byte getHighlightColor(int32_t c)
{
	RGB col;
	get_color(c, &col);
	return getHighlightColor(col);
}

byte getHighlightColor(RGB const& col)
{
	double lum = (pow(col.r/64.0, 2.2) * 0.2126) +
	             (pow(col.g/64.0, 2.2) * 0.7152) +
	             (pow(col.b/64.0, 2.2) * 0.0722);
	return lum < 0.4 ? vc(15) : vc(0);
	//Old -Em
	// byte bright = (col.r >= 32) + (col.g >= 32) + (col.b >= 32);
	// byte sbright = (col.r >= 48) + (col.g >= 48) + (col.b >= 48);
	// byte highlightColor = vc(7); //sysgray
	// if(bright >= 2)
	// {
		// if(sbright >= 2)
			// highlightColor = vc(0); //sysblack
		// else highlightColor = vc(8); //sysdarkgray
	// }
	// else if(!bright)
		// highlightColor = vc(15); //syswhite
	// return highlightColor;
}

int32_t jwin_selcolor_proc(int32_t msg, DIALOG *d, int32_t c)
{
	int32_t ret = D_O_K;
	if(!d->d2) d->d2 = 12;
	bool alt = d->d2 > 16;
	int32_t numcsets = alt ? 16 : d->d2;
	int32_t numcol = numcsets*0x10;
	if(msg==MSG_START)
	{
		d->w = d->h = (16*8) * 1.5;
	}
	int32_t csz = 12;
	d->w = csz * 16;
	d->h = csz * numcsets;
	switch(msg)
	{
		case MSG_DRAW:
		{
			jwin_draw_frame(screen, d->x-2, d->y-2, d->w+4, d->h+4, FR_ETCHED);
			for(int32_t c = 0; c < numcol; ++c)
			{
				int32_t x = (c%16)*csz, y = (c/16)*csz;
				rectfill(screen, d->x+x, d->y+y, d->x+x+csz-1, d->y+y+csz-1, c);
				if(c == d->d1)
				{
					byte highlightColor = getHighlightColor(c);
					rect(screen, d->x+x+0, d->y+y+0, d->x+x+csz-1, d->y+y+csz-1, highlightColor);
					rect(screen, d->x+x+1, d->y+y+1, d->x+x+csz-2, d->y+y+csz-2, highlightColor);
				}
			}
			
            FONT *oldfont = font;
            
            if(d->dp2)
            {
                font = (FONT*)d->dp2;
            }
			
			char buf[32]={0};
			for(int32_t col = 0; col < 16; ++col)
			{
				sprintf(buf, "%X", col);
				gui_textout_ln(screen, (uint8_t*)buf, d->x + (csz*col) + (csz/2), d->y-3-text_height(font), scheme[jcBOXFG], scheme[jcBOX], 1);
			}
			for(int32_t row = 0; row < numcsets; ++row)
			{
				sprintf(buf, "%s 0x%02X", rowpref(row, alt), row*16);
				gui_textout_ln(screen, (uint8_t*)buf, d->x-3, d->y + (csz*row) + (csz-text_height(font))/2, scheme[jcBOXFG], scheme[jcBOX], 2);
			}
			
            font = oldfont;
			break;
		}
		
		case MSG_CLICK:
		{
			if(mouse_in_rect(d->x, d->y, d->x+d->w-1, d->y+d->h-1))
			{
				int32_t col = ((gui_mouse_x() - d->x) / csz) + 16*((gui_mouse_y() - d->y) / csz);

				// for(int32_t c = 0; c < 0xC0; ++c) //to cset 11
				// {
					// int32_t x = (c%16)*csz, y = (c/16)*csz;
					// if(mouse_in_rect(d->x+x, d->y+y, d->x+x+csz-1, d->y+y+csz-1))
					// {
						// col = c;
						// break;
					// }
				// }
				if(col>-1 && col != d->d1)
				{
					d->d1 = col;
					ret |= D_REDRAWME;
				}
				ret |= D_WANTFOCUS;
			}
			break;
		}
		
		case MSG_WANTFOCUS:
		case MSG_LOSTFOCUS:
		case MSG_KEY:
			ret = D_WANTFOCUS;
			break;
		
		case MSG_CHAR:
		{
			ret = D_USED_CHAR | D_REDRAWME;
			switch(c>>8)
			{
				case KEY_LEFT:
				{
					if(d->d1 % 0x10)
						--d->d1;
					break;
				}
				case KEY_RIGHT:
				{
					if(d->d1 % 0x10 != 0x0F)
						++d->d1;
					break;
				}
				case KEY_UP:
				{
					if(d->d1 / 0x10)
						d->d1 -= 0x10;
					break;
				}
				case KEY_DOWN:
				{
					if(d->d1 / 0x10 != numcsets)
						d->d1 += 0x10;
					break;
				}
				case KEY_ENTER:
				{
					ret = D_CLOSE;
					break;
				}
				default: ret = D_O_K;
			}
			break;
		}
	}
	return ret;
}

static DIALOG selcolor_dlg[] =
{
	{ jwin_win_proc,       0,    0,  306,  63+16*8,       vc(14),      vc(1),    0,    D_EXIT,         0,    0,    (void *)"Select Color",  NULL,  NULL },
	{ jwin_button_proc,   75,  40+16*8,   61,   21,       vc(14),      vc(1),    0,    D_EXIT,         0,    0,    (void *)"OK",  NULL,  NULL },
	{ jwin_button_proc,  164,  40+16*8,   61,   21,       vc(14),      vc(1),    0,    D_EXIT,         0,    0,    (void *)"Cancel",  NULL,  NULL },
	{ jwin_selcolor_proc, 156-64,   34,   16*8,   16*8,            0,          0,    0,         0,         0,    0,    NULL,  NULL,  NULL },
	
	{ NULL,              0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL,  NULL,  NULL }
};

int32_t jwin_color_swatch(int32_t msg, DIALOG *d, int32_t c)
{
	int32_t ret = D_O_K;
	
	switch(msg)
	{
		case MSG_START:
		{
			if(d->d2 < 1) d->d2 = 12;
			else if(d->d2 > 17) d->d2 = 17;
			break;
		}
		
		case MSG_DRAW:
		{
			if(!d->d1 || (d->flags&D_DISABLED))
			{
				rectfill(screen, d->x, d->y, d->x+d->w-1, d->y+d->h-1,
					(d->flags&D_DISABLED) ? scheme[jcDISABLED_BG] : vc(0));
				line(screen, d->x, d->y, d->x+d->w-1, d->y+d->h-1, vc(15));
				line(screen, d->x, d->y+d->h-1, d->x+d->w-1, d->y, vc(15));
				jwin_draw_frame(screen, d->x-2, d->y-2, d->w+4, d->h+4, FR_DEEP);
			}
			else
			{
				int32_t c;
				switch(d->d1) //special cases
				{
					case BLACK:
						c = vc(0);
						break;
					case WHITE:
						c = vc(15);
						break;
					default:
						c = d->d1;
						break;
				}
				rectfill(screen, d->x, d->y, d->x+d->w-1, d->y+d->h-1, c);
				jwin_draw_frame(screen, d->x-2, d->y-2, d->w+4, d->h+4, FR_ETCHED);
			}
			break;
		}
		
		case MSG_CLICK:
		{
			if(d->flags&(D_READONLY|D_DISABLED)) break;
			selcolor_dlg[0].dp2 = get_zc_font(font_lfont);
			selcolor_dlg[3].bg = scheme[jcBOXFG];
			selcolor_dlg[3].fg = scheme[jcBOX];
			selcolor_dlg[3].d1 = d->d1;
			selcolor_dlg[3].d2 = d->d2;
			large_dialog(selcolor_dlg);
			
			while(gui_mouse_b()) rest(1); //wait for mouseup
			
			//!TODO Move this out of jwin, and do better palette management.
			//!TODO Allow loading different level palettes, sprite palettes, etc via buttons
			PALETTE oldpal;
			get_palette(oldpal);
			bool alt = d->d2 > 16;
			if(!alt)
			{
				PALETTE foopal;
				get_palette(foopal);
				foopal[BLACK] = _RGB(0,0,0);
				foopal[WHITE] = _RGB(63,63,63);
				zc_set_palette(foopal);
			}
			
			jwin_center_dialog(selcolor_dlg);
			int32_t val = do_zqdialog(selcolor_dlg, 3);
			ret = D_REDRAW;
			
			zc_set_palette(oldpal);
			if(val == 1 || val == 3)
			{
				d->d1 = selcolor_dlg[3].d1;
				GUI_EVENT(d, geCHANGE_VALUE);
				ret |= D_REDRAWME;
			}
			if(d->flags & D_EXIT)
				return D_CLOSE;
			break;
		}
	}
	return ret;
}

static DIALOG alert_dialog[] =
{
    /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)   (dp2)  (dp3) */
    { jwin_win_proc,     0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL,  NULL,  NULL },
    { d_ctext2_proc,      0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL,  NULL,  NULL },
    { d_ctext2_proc,      0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL,  NULL,  NULL },
    { d_ctext2_proc,      0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL,  NULL,  NULL },
    { jwin_button_proc,  0,    0,    0,    0,    0,    0,    0,    D_EXIT,  0,    0,    NULL,  NULL,  NULL },
    { jwin_button_proc,  0,    0,    0,    0,    0,    0,    0,    D_EXIT,  0,    0,    NULL,  NULL,  NULL },
    { jwin_button_proc,  0,    0,    0,    0,    0,    0,    0,    D_EXIT,  0,    0,    NULL,  NULL,  NULL },
    { NULL,              0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL,  NULL,  NULL }
};

#define A_S1  1
#define A_S2  2
#define A_S3  3
#define A_B1  4
#define A_B2  5
#define A_B3  6

/* jwin_alert3:
  *  Displays a simple alert box, containing three lines of text (s1-s3),
  *  and with either one, two, or three buttons. The text for these buttons
  *  is passed in b1, b2, and b3 (NULL for buttons which are not used), and
  *  the keyboard shortcuts in c1 and c2. Returns 1, 2, or 3 depending on
  *  which button was selected.
  */
int32_t jwin_alert3(const char *title, const char *s1, const char *s2, const char *s3, const char *b1, const char *b2, const char *b3, int32_t c1, int32_t c2, int32_t c3, FONT *title_font)
{
    int32_t maxlen = 0;
    int32_t len1, len2, len3;
    int32_t avg_w = text_length(font, " ");
    int32_t avg_h = text_height(font)+1;
    int32_t buttons = 0;
    int32_t yofs = (title ? 22 : 0);
    int32_t b[3];
    int32_t c;
    
#define SORT_OUT_BUTTON(x) { \
          if (b##x)          \
          {                  \
            alert_dialog[A_B##x].flags &= ~D_HIDDEN; \
            alert_dialog[A_B##x].key = c##x; \
            alert_dialog[A_B##x].dp =  (void *)b##x; \
            len##x = gui_strlen(b##x); \
            b[buttons++] = A_B##x; \
          }                  \
          else               \
          {                  \
            alert_dialog[A_B##x].flags |= D_HIDDEN; \
            len##x = 0;      \
          }                  \
        }
    
    if(title_font)
    {
        alert_dialog[0].dp2=title_font;
    }
    
    alert_dialog[A_S1].dp = alert_dialog[A_S2].dp = alert_dialog[A_S3].dp =
                                alert_dialog[A_B1].dp = alert_dialog[A_B2].dp = (void*)"";
                                
    if(s1)
    {
        alert_dialog[A_S1].dp = (void *)s1;
        maxlen = text_length(font, s1);
    }
    
    if(s2)
    {
        alert_dialog[A_S2].dp = (void *)s2;
        len1 = text_length(font, s2);
        
        if(len1 > maxlen)
            maxlen = len1;
    }
    
    if(s3)
    {
        alert_dialog[A_S3].dp = (void *)s3;
        len1 = text_length(font, s3);
        
        if(len1 > maxlen)
            maxlen = len1;
    }
    
    SORT_OUT_BUTTON(1);
    SORT_OUT_BUTTON(2);
    SORT_OUT_BUTTON(3);
    
    len1 = MAX(len1, MAX(len2, len3)) + avg_w*3;
    
    if(len1*buttons > maxlen)
        maxlen = len1*buttons;
        
    maxlen += avg_w*4;
    maxlen=zc_max(text_length(title_font?title_font:font,title)+29,maxlen);
    alert_dialog[0].w = maxlen;
    alert_dialog[A_S1].x = alert_dialog[A_S2].x = alert_dialog[A_S3].x =
                               alert_dialog[0].x + maxlen/2;
                               
    alert_dialog[A_B1].w = alert_dialog[A_B2].w = alert_dialog[A_B3].w = len1;
    
    alert_dialog[A_B1].x = alert_dialog[A_B2].x = alert_dialog[A_B3].x =
                               alert_dialog[0].x + maxlen/2 - len1/2;
                               
    if(buttons == 3)
    {
        alert_dialog[b[0]].x = alert_dialog[0].x + maxlen/2 - len1*3/2 - avg_w;
        alert_dialog[b[2]].x = alert_dialog[0].x + maxlen/2 + len1/2 + avg_w;
    }
    else if(buttons == 2)
    {
        alert_dialog[b[0]].x = alert_dialog[0].x + maxlen/2 - len1 - avg_w;
        alert_dialog[b[1]].x = alert_dialog[0].x + maxlen/2 + avg_w;
    }
    
    alert_dialog[0].h = avg_h*7 + 13 + yofs;
    alert_dialog[A_S1].y = alert_dialog[0].y + avg_h + yofs;
    alert_dialog[A_S2].y = alert_dialog[0].y + avg_h*2 + yofs;
    alert_dialog[A_S3].y = alert_dialog[0].y + avg_h*3 + yofs;
    alert_dialog[A_S1].h = alert_dialog[A_S2].h = alert_dialog[A_S3].h = avg_h;
    
    alert_dialog[A_B1].y = alert_dialog[A_B2].y = alert_dialog[A_B3].y =
                               alert_dialog[0].y + avg_h*5 + yofs;
                               
    alert_dialog[A_B1].h = alert_dialog[A_B2].h = alert_dialog[A_B3].h = avg_h+13;
    
    alert_dialog[0].dp = (void *)title;
    alert_dialog[0].flags = (title) ? D_EXIT : 0;
    
    jwin_center_dialog(alert_dialog);
    set_dialog_color(alert_dialog, scheme[jcTEXTFG], scheme[jcBOX]);
    
    clear_keybuf();
    
    do
    {
        rest(1);
    }
    while(gui_mouse_b());
    
	large_dialog(alert_dialog);
	alert_dialog[0].d1 = 0;
    
    c = do_zqdialog(alert_dialog, A_B1);
    
    if(c == A_B1)
        return 1;
    else if(c == A_B2)
        return 2;
    else
        return 3;
}

/* jwin_alert:
  *  Displays a simple alert box, containing three lines of text (s1-s3),
  *  and with either one or two buttons. The text for these buttons is passed
  *  in b1 and b2 (b2 may be null), and the keyboard shortcuts in c1 and c2.
  *  Returns 1 or 2 depending on which button was selected.
  */
int32_t jwin_alert(const char *title, const char *s1, const char *s2, const char *s3, const char *b1, const char *b2, int32_t c1, int32_t c2, FONT *title_font)
{
    int32_t ret;
    
    ret = jwin_alert3(title, s1, s2, s3, b1, b2, NULL, c1, c2, 0, title_font);
    
    if(ret > 2)
        ret = 2;
        
    return ret;
}

int32_t d_autotext_proc(int32_t msg, DIALOG *d, int32_t c)
{
	ASSERT(d);
	ASSERT(d->dp);
	#define AUTOBUF_SIZE 8092
	static char auto_buf[AUTOBUF_SIZE] = {0};
	static int32_t auto_inds[50] = {0};
	
	
	FONT *oldfont = font;
	
	if (d->dp2)
		font = (FONT*)d->dp2;
	switch(msg)
	{
		case MSG_START:
		{
			memset(auto_buf, 0, AUTOBUF_SIZE);
			memset(auto_inds, 0, 50);
			char* str = (char*)d->dp;
			int32_t len = strlen(str);
			int32_t pos = 0, curstrpos = 0, linecount = 1, lastWS = -1;
			BITMAP* dummy = create_bitmap_ex(8,8,8);
			for(int32_t q = 0; q < len; ++q)
			{
				switch(str[q])
				{
					case ' ': case '\t':
						lastWS = pos;
						break;
					case '\n': //Forced newline
						auto_inds[linecount++] = ++pos;
						curstrpos = pos;
						lastWS = -1;
						continue; //skip rest of for loop, go to next char
				}
				auto_buf[pos++] = str[q];
				if(gui_textout_ex(dummy,auto_buf+curstrpos,0,0,0,0,0) >= d->w) //too int32_t, wrap to lower line
				{
					if(lastWS<0)
					{
						auto_buf[pos-1] = 0;
						auto_inds[linecount++] = pos;
						curstrpos = pos;
						auto_buf[pos++] = str[q];
					}
					else
					{
						auto_buf[lastWS] = 0;
						auto_inds[linecount++] = lastWS+1;
						curstrpos = lastWS+1;
						if(gui_textout_ex(dummy,auto_buf+curstrpos,0,0,0,0,0) >= d->w) //STILL too int32_t?
						{
							auto_buf[pos-1] = 0;
							auto_inds[linecount++] = pos;
							curstrpos = pos;
							auto_buf[pos++] = str[q];
						}
						lastWS = -1;
					}
				}
			}
			destroy_bitmap(dummy);
			d->d2 = linecount;
			d->h = ((text_height(font) + d->d1) * linecount) - d->d1;
		}
		break;
		
		case MSG_DRAW:
		{
			int32_t fg = (d->flags & D_DISABLED) ? scheme[jcDISABLED_FG] : d->fg;
			int32_t linecount = d->d2;
			
			int32_t yinc = text_height(font)+d->d1;
			int32_t y = d->y;
			for(int32_t q = 0; q < linecount; ++q)
			{
				gui_textout_ex(screen, auto_buf+auto_inds[q], d->x, y, fg, d->bg, true);
				y += yinc;
			}
		}
		break;
	}
	font = oldfont;
	return D_O_K;
}

static DIALOG alert2_dialog[] =
{
    /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)   (dp2)  (dp3) */
    { jwin_win_proc,     0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL,  NULL,  NULL },
    { d_autotext_proc,   0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL,  NULL,  NULL },
    { jwin_button_proc,  0,    0,    0,    0,    0,    0,    0,    D_EXIT,  0,    0,    NULL,  NULL,  NULL },
    { jwin_button_proc,  0,    0,    0,    0,    0,    0,    0,    D_EXIT,  0,    0,    NULL,  NULL,  NULL },
    { jwin_button_proc,  0,    0,    0,    0,    0,    0,    0,    D_EXIT,  0,    0,    NULL,  NULL,  NULL },
    { NULL,              0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL,  NULL,  NULL }
};

#define A2_S1  1
#define A2_B1  2
#define A2_B2  3
#define A2_B3  4

/* jwin_auto_alert3:
  *  Displays a simple alert box, containing one line of text, auto-split
  *  across lines using 'lenlim' and 'vspace,
  *  and with either one, two, or three buttons. The text for these buttons
  *  is passed in b1, b2, and b3 (NULL for buttons which are not used), and
  *  the keyboard shortcuts in c1 and c2. Returns 1, 2, or 3 depending on
  *  which button was selected.
  */
int32_t jwin_auto_alert3(const char *title, const char *s1, int32_t lenlim, int32_t vspace, const char *b1, const char *b2, const char *b3, int32_t c1, int32_t c2, int32_t c3, FONT *title_font)
{
    int32_t maxlen = 0;
    int32_t len1, len2, len3;
    int32_t avg_w = text_length(font, " ");
    int32_t avg_h = text_height(font)+1;
    int32_t buttons = 0;
    int32_t yofs = (title ? 22 : 0);
    int32_t b[3];
    int32_t c;
    
#define SORT_OUT_AUTOBUTTON(x) { \
          if (b##x)          \
          {                  \
            alert2_dialog[A2_B##x].flags &= ~D_HIDDEN; \
            alert2_dialog[A2_B##x].key = c##x; \
            alert2_dialog[A2_B##x].dp =  (void *)b##x; \
            len##x = gui_strlen(b##x); \
            b[buttons++] = A2_B##x; \
          }                  \
          else               \
          {                  \
            alert2_dialog[A2_B##x].flags |= D_HIDDEN; \
            len##x = 0;      \
          }                  \
        }
    
    if(title_font)
    {
        alert2_dialog[0].dp2=title_font;
    }
    
    alert2_dialog[A2_S1].dp = alert2_dialog[A2_B1].dp = alert2_dialog[A2_B2].dp = (void*)"";
                                
    if(s1)
    {
        alert2_dialog[A2_S1].dp = (void *)s1;
        maxlen = lenlim;
    }
    
    SORT_OUT_AUTOBUTTON(1);
    SORT_OUT_AUTOBUTTON(2);
    SORT_OUT_AUTOBUTTON(3);
    
    len1 = MAX(len1, MAX(len2, len3)) + avg_w*3;
    
    if(len1*buttons > maxlen)
        maxlen = len1*buttons;
        
    maxlen += avg_w*4;
    maxlen=zc_max(text_length(title_font?title_font:font,title)+29,maxlen);
    
    alert2_dialog[A2_S1].x = alert2_dialog[0].x + maxlen/2;
    alert2_dialog[A2_S1].y = alert2_dialog[0].y + avg_h + yofs;
    alert2_dialog[A2_S1].w = lenlim;
    alert2_dialog[A2_S1].d1 = vspace;
    
	large_dialog(alert2_dialog);
	alert2_dialog[0].d1 = 0;
	
	object_message(&alert2_dialog[A2_S1], MSG_START, 0); //calculate height
	
	alert2_dialog[A2_S1].x = alert2_dialog[0].x + maxlen/2;
	alert2_dialog[A2_S1].y = alert2_dialog[0].y + avg_h + yofs;
	alert2_dialog[A2_S1].w = lenlim;
	alert2_dialog[A2_S1].d1 = vspace;
	
    alert2_dialog[A2_B1].w = alert2_dialog[A2_B2].w = alert2_dialog[A2_B3].w = len1;
    
    alert2_dialog[A2_B1].x = alert2_dialog[A2_B2].x = alert2_dialog[A2_B3].x =
                               alert2_dialog[0].x + maxlen/2 - len1/2;
                               
    if(buttons == 3)
    {
        alert2_dialog[b[0]].x = alert2_dialog[0].x + maxlen/2 - len1*3/2 - avg_w;
        alert2_dialog[b[2]].x = alert2_dialog[0].x + maxlen/2 + len1/2 + avg_w;
    }
    else if(buttons == 2)
    {
        alert2_dialog[b[0]].x = alert2_dialog[0].x + maxlen/2 - len1 - avg_w;
        alert2_dialog[b[1]].x = alert2_dialog[0].x + maxlen/2 + avg_w;
    }
	
    alert2_dialog[0].w = maxlen;
    alert2_dialog[0].h = avg_h*4 + yofs + alert2_dialog[A2_S1].h + 13;
    alert2_dialog[A2_B1].y = alert2_dialog[A2_B2].y = alert2_dialog[A2_B3].y =
                               alert2_dialog[0].y + avg_h*2 + yofs + alert2_dialog[A2_S1].h;
                               
    alert2_dialog[A2_B1].h = alert2_dialog[A2_B2].h = alert2_dialog[A2_B3].h = avg_h+13;
    
    alert2_dialog[0].dp = (void *)title;
    alert2_dialog[0].flags = (title) ? D_EXIT : 0;
    
    jwin_center_dialog(alert2_dialog);
    set_dialog_color(alert2_dialog, scheme[jcTEXTFG], scheme[jcBOX]);
    
    clear_keybuf();
    
    do
    {
        rest(1);
    }
    while(gui_mouse_b());
    
	large_dialog(alert2_dialog);
	alert2_dialog[0].d1 = 0;
    
    c = do_zqdialog(alert2_dialog, A2_B1);
    
    if(c == A2_B1)
        return 1;
    else if(c == A2_B2)
        return 2;
    else
        return 3;
}

int32_t jwin_auto_alert(const char *title, const char *s1, int32_t lenlim, int32_t vspace, const char *b1, const char *b2, int32_t c1, int32_t c2, FONT *title_font)
{
    int32_t ret;
    
    ret = jwin_auto_alert3(title, s1, lenlim, vspace, b1, b2, NULL, c1, c2, 0, title_font);
    
    if(ret > 2)
        ret = 2;
        
    return ret;
}

/*****************************************/
/***********  drop list proc  ************/
/*****************************************/
int32_t last_droplist_sel = -1;
static int32_t d_dropcancel_proc(int32_t msg,DIALOG *d,int32_t c)
{
    //these are here to bypass compiler warnings about unused arguments
    d=d;
    c=c;
    
    if(msg==MSG_CLICK || msg==MSG_DCLICK)
        return D_CLOSE;
        
    return D_O_K;
}

static DIALOG droplist_dlg[] =
{
    /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)     (d2)      (dp)              (dp2)   (dp3)*/
    { d_dropcancel_proc, 0,    0,    0,    0,    0,       0,      0,       0,          0,       0,        NULL,             NULL,   NULL },
    { d_list_proc,       0,    0,    0,    0,    0,       0,      0,       D_EXIT,     0,       0,        NULL,             NULL,   NULL },
    { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          0,       KEY_ESC, (void*)close_dlg, NULL,   NULL },
    { NULL,              0,    0,    0,    0,    0,       0,      0,       0,          0,       0,        NULL,             NULL,   NULL }
};

static int32_t droplist(DIALOG *d)
{
	ListData *data = (ListData *)d->dp;
	int32_t d1 = d->d1;
	int32_t listsize, x, y, w, h, max_w;
	auto oz = gui_mouse_z();

	data->listFunc(-1, &listsize);
	y = d->y + d->h;
	h = zc_min(abc_patternmatch ? listsize+1 : listsize,8) * text_height(*data->font) + 8;
	
	if(y+h >= zq_screen_h)
	{
		y = d->y - h;
	}
	
	x = d->x;
	w = d->w;
	max_w = zc_max(d->x+d->w, zq_screen_w-d->x);
	
	for(int32_t i=0; i<listsize; ++i)
	{
		w=zc_min(max_w,zc_max(w,text_length(*data->font,data->listFunc(i, NULL))+39));
	}
	
	if(x+w >= zq_screen_w)
	{
		x=zq_screen_w-w;
	}
	
	droplist_dlg[1] = *d;
	droplist_dlg[1].proc  = &jwin_abclist_proc;
	droplist_dlg[1].flags = D_EXIT + D_USER;
	droplist_dlg[1].x  = x;
	droplist_dlg[1].y  = y;
	droplist_dlg[1].w  = w;
	droplist_dlg[1].h  = h;
	droplist_dlg[1].d2 = listsize<=8 ? 0 : zc_max(d1-3,0);
	
	// cancel
	droplist_dlg[0].x = 0;
	droplist_dlg[0].y = 0;
	droplist_dlg[0].w = zq_screen_w;
	droplist_dlg[0].h = zq_screen_h;
	
	if(do_zq_subdialog(droplist_dlg,1)==1)
	{
		position_mouse_z(oz);
		return droplist_dlg[1].d1;
	}
	
	position_mouse_z(oz);
	return d1;
}

/* jwin_droplist_proc:
  *   A drop list...
  */
int32_t jwin_droplist_proc(int32_t msg,DIALOG *d,int32_t c)
{
	int32_t ret;
	int32_t down=0, last_draw=0;
	int32_t d1;
	
	switch(msg)
	{
		case MSG_CLICK:
			if(mouse_in_rect(d->x+d->w-18,d->y+2,16,d->h))
				goto dropit;
				
			break;
			
		case MSG_KEY:
			goto dropit;
			break;
	}
	
	d1 = d->d1;
	ret = jwin_list_proc(msg,d,c);
	
	if(d->d1!=d->d2)
	{
		d->d1=d->d2;
		jwin_droplist_proc(MSG_DRAW, d, 0);
	}

	if(d1 != d->d1)
	{
		GUI_EVENT(d, geCHANGE_SELECTION);
		if(d->flags&D_EXIT)
			ret |= D_CLOSE;
	}

	if(msg == MSG_DRAW)
	{
		draw_arrow_button(screen, d->x+d->w-18, d->y+2,16, d->h-4, 0, 0);
	}
	
	return ret;
	
dropit:
	last_draw = 0;
	
	while(gui_mouse_b())
	{
		down = mouse_in_rect(d->x+d->w-18,d->y+2,16,d->h);
		
		if(down!=last_draw)
		{
			draw_arrow_button(screen, d->x+d->w-18, d->y+2,16, d->h-4, 0, down*3);
			last_draw = down;
			update_hw_screen();
		}
		
		clear_keybuf();
		rest(1);
	}
	
	if(!down)
	{
		return D_O_K;
	}
	
	draw_arrow_button(screen, d->x+d->w-18, d->y+2,16, d->h-4, 0, 0);
	
	d1 = d->d1;
	d->d2 = d->d1 = droplist(d);
	
	object_message(d, MSG_DRAW, 0);
	
	while(gui_mouse_b())
	{
		clear_keybuf();
		rest(1);
		update_hw_screen();
	}

	if(d1!=d->d1)
		GUI_EVENT(d, geCHANGE_SELECTION);

	return ((d1 != d->d1) && (d->flags&D_EXIT)) ? D_CLOSE : D_O_K;
}

/*****************************************/
/************  ABC list proc  ************/
/*****************************************/



int32_t jwin_abclist_proc(int32_t msg,DIALOG *d,int32_t c)
{
    ListData *data = (ListData *)d->dp;
    if(msg == MSG_START) wipe_abc_keypresses();
    
	if(msg == MSG_CHAR && (key_shifts&KB_CTRL_FLAG))
		return D_O_K;
	
	if(abc_patternmatch) // Search style pattern match. 
	{
		if(msg==MSG_CHAR && ((c&0xFF) > 31) && ((c&0xFF) < 127)) //(isalpha(c&0xFF) || isdigit(c&0xFF)))
		{
			int32_t max,dummy,h;
			
			h = ((d->h-3) / text_height(*data->font))-1;
			if ( isalpha(c&0xFF) ) c = toupper(c&0xFF);
			for ( int32_t q = 0; q < 1023; ++q ) 
			{ 
				if ( !(abc_keypresses[q]) )
				{
					abc_keypresses[q] = (char)c;
					break; 
				}
			}
			data->listFunc(-1, &max);

			int32_t cur = d->d1;
			int32_t charpos = 0; int32_t listpos = 0; int32_t lastmatch = -1;
			char tmp[1024] = { 0 };
			char lsttmp[1024] = { 0 };
			int32_t lastmatches[32768] = {0};
			for ( int32_t a = 0; a < 32768; ++a ) lastmatches[a] = -1; 
			int32_t lmindx = 0;
			
			bool foundmatch = false;
			bool numsearch = true;
			for ( int32_t q = 0; q < 1023; ++q ) 
			{
				if(!abc_keypresses[q]) break;
				if(!isdigit(abc_keypresses[q]))
				{
					if(q == 0 && abc_keypresses[q] == '-')
						continue;
					numsearch = false;
					break; 
				}
			}
			if(numsearch) //Indexed search, first
			{
				int32_t num = atoi(abc_keypresses);
				//Find a different indexing type in the strings?
				if(!foundmatch)
				{
					char buf[16];
					if(num < 0) sprintf(buf, "(%04d)", num);
					else sprintf(buf, "(%03d)", num);
					std::string cmp = buf;
					for(int32_t listpos = 0; listpos < max; ++listpos)
					{
						std::string str((data->listFunc(listpos,&dummy)));
						size_t trimpos = str.find_last_not_of("-(0123456789)");
						if(trimpos != std::string::npos) ++trimpos;
						str.erase(0, trimpos);
						if(cmp == str)
						{
							d->d1 = listpos;
							d->d2 = zc_max(zc_min(listpos-(h>>1), max-h), 0);
							foundmatch = true;
							break;
						}
					}
				}
				//Search for match with first number in string?
				if(!foundmatch)
				{
					auto buf = fmt::format("{}", num);
					for(int32_t listpos = 0; listpos < max; ++listpos)
					{
						std::string str((data->listFunc(listpos,&dummy)));
						size_t pos1 = -1;
						do
						{
							pos1 = str.find_first_of("-0123456789", pos1+1);
						} while(pos1 != string::npos && str[pos1] == '-' && pos1+1 < str.size() && !isdigit(str[pos1+1]));
						if(pos1 == string::npos)
							continue;
						size_t pos2 = str.find_first_not_of("-0123456789", pos1);
						if(pos2 == string::npos)
							continue;
						str = str.substr(pos1,pos2-pos1);
						if(buf == str)
						{
							d->d1 = listpos;
							d->d2 = zc_max(zc_min(listpos-(h>>1), max-h), 0);
							foundmatch = true;
							break;
						}
					}
				}
			}
			if(!foundmatch)
			{
				strcpy(tmp, abc_keypresses);
				for ( int32_t listpos = 0; listpos < max; ++listpos )
				{
					memset(lsttmp, 0, 1024);
					strcpy(lsttmp, ((data->listFunc(listpos,&dummy))));
					
					if ( !(strnicmp(lsttmp, tmp, strlen(tmp))))
					{
						d->d1 = listpos;
						d->d2 = zc_max(zc_min(listpos-(h>>1), max-h), 0);
						foundmatch = true;
						break;
					}
				}
			}
			if(foundmatch)
				GUI_EVENT(d, geCHANGE_SELECTION);
			d->flags |= D_DIRTY;
			if ( gui_mouse_b() ) wipe_abc_keypresses();
			return foundmatch ? D_USED_CHAR : D_O_K;
		}
		else if(msg==MSG_CHAR && ( (c&0xFF) == 8) )//backspace
		{
			for ( int32_t q = 1023; q >= 0; --q )
			{
				if ( abc_keypresses[q] ) 
				{
					d->flags |= D_DIRTY;
					abc_keypresses[q] = '\0'; break;
				}
			}
			return D_USED_CHAR;
		}
		if ( gui_mouse_b() ) { wipe_abc_keypresses(); } //al_trace("keypresses: %s\n", abc_keypresses); }
	}
	else // Windows Explorer style jumping
	{
		if(msg==MSG_CHAR && (isalpha(c&0xFF) || isdigit(c&0xFF)))
		{
			int32_t max,dummy,h,i;
			
			h = (d->h-3) / text_height(*data->font);
			c = toupper(c&0xFF);

			data->listFunc(-1, &max);

			int32_t cur = d->d1;
			bool foundmatch = false;
			for(i=cur+1; (cur ? (i != cur) : (cur < max)); ++i) //don't infinite loop this. 
			{
				//al_trace("loop running\n");
				if(i>=max) i=0;
				if(toupper((data->listFunc(i,&dummy))[0]) == c)
				{
					d->d1 = i;
					d->d2 = zc_max(zc_min(i-(h>>1), max-h), 0);
					foundmatch = true;
					break;
				}
			}
			
			d->flags |= D_DIRTY;
			return foundmatch ? D_USED_CHAR : D_O_K;
		}
	}
	if ( gui_mouse_b() ) { wipe_abc_keypresses(); } //al_trace("keypresses: %s\n", abc_keypresses); }
	return ((abc_patternmatch) ? jwin_do_abclist_proc(msg,d,c) : jwin_list_proc(msg,d,c));
}

int32_t jwin_checkfont_proc(int32_t msg, DIALOG *d, int32_t c)
{

    FONT *oldfont = font;
    
    if(d->dp2)
    {
        font = (FONT *)d->dp2;
    }
    
    int32_t rval = jwin_check_proc(msg, d, c);
    font = oldfont;
    return rval;
}

/* jwin_check_proc:
  *  Who needs C++ after all? This is derived from d_button_proc,
  *  but overrides the drawing routine to provide a check box.
  */
int32_t jwin_check_proc(int32_t msg, DIALOG *d, int32_t c)
{
    //these are here to bypass compiler warnings about unused arguments
    c=c;
    int32_t x;
    int32_t bx=0, tl=0;
    int32_t tx=d->x;
    ASSERT(d);
    
    switch(msg)
    {
    case MSG_DRAW:
        x = d->x;
        
        if(!(d->d1))
        {
            if(d->dp)
            {
                if(d->flags & D_DISABLED)
                {
                    gui_textout_ln(screen, (uint8_t *)d->dp, tx+1, d->y+1+(d->h-(text_height(font)-gui_font_baseline))/2, scheme[jcLIGHT], scheme[jcBOX], 0);
                    tl=gui_textout_ln(screen, (uint8_t *)d->dp, tx, d->y+(d->h-(text_height(font)-gui_font_baseline))/2, scheme[jcDISABLED_FG], -1, 0);
                    bx=tl+text_height(font)/2;
                }
                else
                {
                    tl=gui_textout_ln(screen, (uint8_t *)d->dp, tx, d->y+(d->h-(text_height(font)-gui_font_baseline))/2, scheme[jcBOXFG], scheme[jcBOX], 0);
                    bx=tl+text_height(font)/2;
                }
            }
        }
        
        jwin_draw_frame(screen, x+bx, d->y, d->h, d->h, FR_DEEP);
        
        if(!(d->flags & D_DISABLED))
        {
            rectfill(screen, x+bx+2, d->y+2, x+bx+d->h-3, d->y+d->h-3, scheme[jcTEXTBG]);
        }
        
        if(d->d1)
        {
            tx=x+bx+d->h-1+(text_height(font)/2);
            
            if(d->dp)
            {
                if(d->flags & D_DISABLED)
                {
                    gui_textout_ln(screen, (uint8_t *)d->dp, tx+1, d->y+1+(d->h-(text_height(font)-gui_font_baseline))/2, scheme[jcLIGHT], scheme[jcBOX], 0);
                    tl=gui_textout_ln(screen, (uint8_t *)d->dp, tx, d->y+(d->h-(text_height(font)-gui_font_baseline))/2, scheme[jcDISABLED_FG], -1, 0);
                }
                else
                {
                    tl=gui_textout_ln(screen, (uint8_t *)d->dp, tx, d->y+(d->h-(text_height(font)-gui_font_baseline))/2, scheme[jcBOXFG], scheme[jcBOX], 0);
                }
            }
        }
        
        if(d->flags & D_SELECTED)
        {
            line(screen, x+bx+2, d->y+2, x+bx+d->h-3, d->y+d->h-3, scheme[jcTEXTFG]);
            line(screen, x+bx+2, d->y+d->h-3, x+bx+d->h-3, d->y+2, scheme[jcTEXTFG]);
        }
        
        d->w=int32_t(text_height(font)*1.5);
        
        if(d->dp)
        {
//      dotted_rect(screen, tx-1, d->y-1, tx+tl, d->y+d->h-1, (d->flags & D_GOTFOCUS)?scheme[jcDARK]:scheme[jcBOX], scheme[jcBOX]);
            dotted_rect(screen, tx-1, d->y-1, tx+tl, d->y+(text_height(font)), (d->flags & D_GOTFOCUS)?scheme[jcDARK]:scheme[jcBOX], scheme[jcBOX]);
            d->w+=tl+1;
        }
        
        return D_O_K;
        break;
    }
    
    return d_jwinbutton_proc(msg, d, 0);
}

int32_t new_check_proc(int32_t msg, DIALOG *d, int32_t)
{
	int32_t bx=0, tl=0;
	ASSERT(d);
	
    FONT *oldfont = font;
    uint8_t* str = (uint8_t*)d->dp;
	bool has_text = str && str[0];
    if(d->dp2)
    {
        font = (FONT *)d->dp2;
    }
	switch(msg)
	{
		case MSG_DRAW:
		{
			const int box_spacing = 4;
			int32_t tx = 2, ty = 2, tx2 = 2;
			int fh = text_height(font);
			auto txt_y = ty+(d->h-fh)/2;
			BITMAP* tmp = create_bitmap_ex(8, d->w+4, d->h+4);
			clear_bitmap(tmp);
			set_clip_rect(tmp, tx, ty, tmp->w-tx, tmp->h-ty);
			if(!(d->d1))
			{
				if(has_text)
				{
					if(d->flags & D_DISABLED)
					{
						gui_textout_ln(tmp, str, tx+1, txt_y+1, scheme[jcLIGHT], scheme[jcBOX], 0);
						tl=gui_textout_ln(tmp, str, tx, txt_y, scheme[jcDISABLED_FG], -1, 0);
						bx=tl+box_spacing;
					}
					else
					{
						tl=gui_textout_ln(tmp, str, tx, txt_y, scheme[jcBOXFG], scheme[jcBOX], 0);
						bx=tl+box_spacing;
					}
				}
			}
			
			jwin_draw_frame(tmp, tx+bx, ty, d->h, d->h, FR_DEEP);
			
			if(!(d->flags & D_DISABLED))
			{
				rectfill(tmp, tx+bx+2, ty+2, tx+bx+d->h-3, ty+d->h-3, scheme[jcTEXTBG]);
			}
			
			if(d->d1)
			{
				tx2=tx+bx+d->h-1+box_spacing;
				
				if(has_text)
				{
					if(d->flags & D_DISABLED)
					{
						gui_textout_ln(tmp, str, tx2+1, txt_y+1, scheme[jcLIGHT], scheme[jcBOX], 0);
						tl=gui_textout_ln(tmp, str, tx2, txt_y, scheme[jcDISABLED_FG], -1, 0);
					}
					else
					{
						tl=gui_textout_ln(tmp, str, tx2, txt_y, scheme[jcBOXFG], scheme[jcBOX], 0);
					}
				}
			}

			if(d->flags & D_SELECTED)
			{
				line(tmp, tx+bx+2, ty+2, tx+bx+d->h-3, ty+d->h-3, scheme[jcTEXTFG]);
				line(tmp, tx+bx+2, ty+d->h-3, tx+bx+d->h-3, ty+2, scheme[jcTEXTFG]);
			}
			
			set_clip_rect(tmp, 0, 0, tmp->w, tmp->h);
			if(has_text)
			{
				dotted_rect(tmp, tx2-1, txt_y-1, tx2+tl, txt_y+fh, (d->flags & D_GOTFOCUS)?scheme[jcDARK]:scheme[jcBOX], scheme[jcBOX]);
			}
			
			masked_blit(tmp, screen, 0, 0, d->x-tx, d->y-ty, d->w+tx+tx, d->h+ty+ty);
			break;
		}
	}
	
	int32_t rval = D_O_K;
	if(msg != MSG_DRAW)
		rval = d_jwinbutton_proc(msg, d, 0);
    font = oldfont;
	return rval;
}

int32_t jwin_radiofont_proc(int32_t msg, DIALOG *d, int32_t c)
{
    FONT *oldfont = font;
    
    if(d->dp2)
    {
        font = (FONT *)d->dp2;
    }
    
    int32_t rval = jwin_radio_proc(msg, d, c);
    font = oldfont;
    return rval;
}

/* jwin_radio_proc:
  *  GUI procedure for radio buttons.
  *  Parameters: d1-button group number; d2-button style (0=circle,1=square);
  *  dp-text to appear as label to the right of the button.
  */
int32_t jwin_radio_proc(int32_t msg, DIALOG *d, int32_t c)
{
    int32_t x, center, r, ret, tl=0, tx;
    ASSERT(d);
    
    switch(msg)
    {
    case MSG_DRAW:
        //      tx=d->x+d->h-1+text_height(font);
        tx=d->x+int32_t(text_height(font)*1.5);
        
        if(d->dp)
        {
            if(d->flags & D_DISABLED)
            {
                gui_textout_ln(screen, (uint8_t *)d->dp, tx+1, d->y+1+(d->h-(text_height(font)-gui_font_baseline))/2, scheme[jcLIGHT], scheme[jcBOX], 0);
                tl=gui_textout_ln(screen, (uint8_t *)d->dp, tx, d->y+(d->h-(text_height(font)-gui_font_baseline))/2, scheme[jcDISABLED_FG], -1, 0);
            }
            else
            {
                tl=gui_textout_ln(screen, (uint8_t *)d->dp, tx, d->y+(d->h-(text_height(font)-gui_font_baseline))/2, scheme[jcBOXFG], scheme[jcBOX], 0);
            }
        }
        
        x = d->x;
        r = d->h/2;
        
        center = x+r;
        rectfill(screen, x, d->y, x+d->h-1, d->y+d->h-1, scheme[jcBOX]);
        
        switch(d->d2)
        {
        case 1:
            jwin_draw_frame(screen, x, d->y, d->h, d->h, FR_DEEP);
            
            if(!(d->flags & D_DISABLED))
            {
                rectfill(screen, x+2, d->y+2, x+d->h-3, d->y+d->h-3, scheme[jcLIGHT]);
            }
            
            if(d->flags & D_SELECTED)
            {
                rectfill(screen, x+r/2, d->y+r/2, x+d->h-1-r/2, d->y+d->h-1-r/2, scheme[jcDARK]);
                //line(screen, x+2, d->y+2, x+d->h-3, d->y+d->h-3, scheme[jcDARK]);
                //line(screen, x+2, d->y+d->h-3, x+d->h-3, d->y+2, scheme[jcDARK]);
            }
            
            break;
            
        default:
            circlefill(screen, center, d->y+r, r, scheme[jcTEXTBG]);
            arc(screen, center, d->y+r, itofix(32), itofix(160), r, vc(0));
            circlefill(screen, center, d->y+r, r-1, scheme[jcTEXTBG]);
            arc(screen, center, d->y+r, itofix(32), itofix(160), r-1, vc(0));
            circlefill(screen, center, d->y+r, r-2, (d->flags & D_DISABLED)?scheme[jcDISABLED_BG]:scheme[jcTEXTBG]);
            
            if(d->flags & D_SELECTED)
            {
                circlefill(screen, center, d->y+r, r-3, scheme[jcTEXTFG]);
            }
            
            break;
        }
        
        if(d->dp)
        {
//      dotted_rect(screen, tx-1, d->y-1, tx+tl, d->y+d->h-1, (d->flags & D_GOTFOCUS)?scheme[jcDARK]:scheme[jcBOX], scheme[jcBOX]);
            dotted_rect(screen, tx-1, d->y-1, tx+tl, d->y+(text_height(font)), (d->flags & D_GOTFOCUS)?scheme[jcDARK]:scheme[jcBOX], scheme[jcBOX]);
            d->w=tl+int32_t(text_height(font)*1.5)+1;
        }
        
        return D_O_K;
        
    case MSG_KEY:
    case MSG_CLICK:
        if(d->flags & D_SELECTED)
        {
            return D_O_K;
        }
        
        break;
        
    case MSG_RADIO:
        if((c == d->d1) && (d->flags & D_SELECTED))
        {
            d->flags &= ~D_SELECTED;
            object_message(d, MSG_DRAW, 0);
        }
        
        break;
    }
    
    ret = d_jwinbutton_proc(msg, d, 0);
    
    if(((msg==MSG_KEY) || (msg==MSG_CLICK)) && (d->flags & D_SELECTED) && (!(d->flags & D_EXIT)))
    {
        d->flags &= ~D_SELECTED;
        broadcast_dialog_message(MSG_RADIO, d->d1);
        d->flags |= D_SELECTED;
		GUI_EVENT(d, geRADIO);
    }
    
    return ret;
}


/* 1.5k lookup table for color matching */
uint32_t col_diff[3*128];

/* bestfit_init:
  *  Color matching is done with weighted squares, which are much faster
  *  if we pregenerate a little lookup table...
  */
void bestfit_init(void)
{
    int32_t i;
    
    col_diff[0] = col_diff[128] = col_diff[256] = 0;
    
    for(i=1; i<64; i++)
    {
        int32_t k = i * i;
        col_diff[0  +i] = col_diff[0  +128-i] = k * (59 * 59);
        col_diff[128+i] = col_diff[128+128-i] = k * (30 * 30);
        col_diff[256+i] = col_diff[256+128-i] = k * (11 * 11);
    }
}



/* bestfit_color:
  *  Searches a palette for the color closest to the requested R, G, B value.
  */
int32_t bestfit_color_range(AL_CONST PALETTE pal, int32_t r, int32_t g, int32_t b, uint8_t start, uint8_t end)
{
    int32_t i, coldiff, lowest, bestfit;
    
    if(col_diff[1] == 0)
        bestfit_init();
        
    bestfit = start;
    lowest = INT_MAX;
    
    i = start;
    
    while(i<PAL_SIZE&&i<=end)
    {
        AL_CONST RGB *rgb = &pal[i];
        coldiff = (col_diff + 0) [(rgb->g - g) & 0x7F ];
        
        if(coldiff < lowest)
        {
            coldiff += (col_diff + 128) [(rgb->r - r) & 0x7F ];
            
            if(coldiff < lowest)
            {
                coldiff += (col_diff + 256) [(rgb->b - b) & 0x7F ];
                
                if(coldiff < lowest)
                {
                    bestfit = rgb - pal;    /* faster than `bestfit = i;' */
                    
                    if(coldiff == 0)
                    {
                        return bestfit;
                    }
                    
                    lowest = coldiff;
                }
            }
        }
        
        i++;
    }
    
    return bestfit;
}


/* makecol8:
  *  Converts R, G, and B values (ranging 0-255) to an 8 bit paletted color.
  *  If the global rgb_map table is initialised, it uses that, otherwise
  *  it searches through the current palette to find the best match.
  */
int32_t makecol8_map(int32_t r, int32_t g, int32_t b, RGB_MAP *table)
{
    return table->data[r>>3][g>>3][b>>3];
}


/* create_rgb_table:
  *  Fills an RGB_MAP lookup table with conversion data for the specified
  *  palette. This is the faster version by Jan Hubicka.
  *
  *  Uses alg. similar to floodfill - it adds one seed per every color in
  *  palette to its best position. Then areas around seed are filled by
  *  same color because it is best approximation for them, and then areas
  *  about them etc...
  *
  *  It does just about 80000 tests for distances and this is about 100
  *  times better than normal 256*32000 tests so the calculation time
  *  is now less than one second at all computers I tested.
  */
void create_rgb_table_range(RGB_MAP *table, AL_CONST PALETTE pal, uint8_t start, uint8_t end, void (*callback)(int32_t pos))
{
#define UNUSED 65535
#define LAST 65532

    /* macro add adds to single linked list */
#define add(i)    (next[(i)] == UNUSED ? (next[(i)] = LAST, \
                                          (first != LAST ? (next[last] = (i)) : (first = (i))), \
                                          (last = (i))) : 0)
    
    /* same but w/o checking for first element */
#define add1(i)   (next[(i)] == UNUSED ? (next[(i)] = LAST, \
                                          next[last] = (i), \
                                          (last = (i))) : 0)
    
    /* calculates distance between two colors */
#define dist(a1, a2, a3, b1, b2, b3) \
          (col_diff[ ((a2) - (b2)) & 0x7F] + \
           (col_diff + 128)[((a1) - (b1)) & 0x7F] + \
           (col_diff + 256)[((a3) - (b3)) & 0x7F])
    
    /* converts r,g,b to position in array and back */
#define pos(r, g, b) \
          (((r) / 2) * 32 * 32 + ((g) / 2) * 32 + ((b) / 2))
    
#define depos(pal, r, g, b) \
          ((b) = ((pal) & 31) * 2, \
           (g) = (((pal) >> 5) & 31) * 2, \
           (r) = (((pal) >> 10) & 31) * 2)
    
    /* is current color better than pal1? */
#define better(r1, g1, b1, pal1) \
          (((int32_t)dist((r1), (g1), (b1), \
                      (pal1).r, (pal1).g, (pal1).b)) > (int32_t)dist2)
    
    /* checking of position */
#define dopos(rp, gp, bp, ts) \
          if ((rp > -1 || r > 0) && (rp < 1 || r < 61) && \
              (gp > -1 || g > 0) && (gp < 1 || g < 61) && \
              (bp > -1 || b > 0) && (bp < 1 || b < 61)) { \
          i = first + rp * 32 * 32 + gp * 32 + bp; \
          if (!data[i]) {     \
            data[i] = val;    \
            add1(i);          \
          }                   \
          else if ((ts) && (data[i] != val)) { \
              dist2 = (rp ? (col_diff+128)[(r+2*rp-pal[val].r) & 0x7F] : r2) + \
                (gp ? (col_diff    )[(g+2*gp-pal[val].g) & 0x7F] : g2) + \
                (bp ? (col_diff+256)[(b+2*bp-pal[val].b) & 0x7F] : b2); \
              if (better((r+2*rp), (g+2*gp), (b+2*bp), pal[data[i]])) { \
                data[i] = val; \
                add1(i);      \
              }               \
            }                 \
        }
    
    int32_t i, curr, r, g, b, val, dist2;
    uint32_t r2, g2, b2;
    uint16_t next[32*32*32];
    uint8_t *data;
    int32_t first = LAST;
    int32_t last = LAST;
    int32_t count = 0;
    int32_t cbcount = 0;
    
#define AVERAGE_COUNT   18000
    
    if(col_diff[1] == 0)
        bestfit_init();
        
    memset(next, 255, sizeof(next));
    memset(table->data, 0, sizeof(char)*32*32*32);
    
    data = (uint8_t *)table->data;
    
    /* add starting seeds for floodfill */
    for(i=start; i<PAL_SIZE&&i<=end; i++)
    {
        curr = pos(pal[i].r, pal[i].g, pal[i].b);
        
        if(next[curr] == UNUSED)
        {
            data[curr] = i;
            add(curr);
        }
    }
    
    /* main floodfill: two versions of loop for faster growing in blue axis */
    while(first < LAST)
    {
        depos(first, r, g, b);
        
        /* calculate distance of current color */
        val = data[first];
        r2 = (col_diff+128)[((pal[val].r)-(r)) & 0x7F];
        g2 = (col_diff)[((pal[val].g)-(g)) & 0x7F];
        b2 = (col_diff+256)[((pal[val].b)-(b)) & 0x7F];
        
        /* try to grow to all directions */
#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif
        dopos(0, 0, 1, 1);
        dopos(0, 0,-1, 1);
        dopos(1, 0, 0, 1);
        dopos(-1, 0, 0, 1);
        dopos(0, 1, 0, 1);
        dopos(0,-1, 0, 1);
#ifdef _MSC_VER
#pragma warning(default:4127)
#endif
        
        /* faster growing of blue direction */
        if((b > 0) && (data[first-1] == val))
        {
            b -= 2;
            first--;
            b2 = (col_diff+256)[((pal[val].b)-(b)) & 0x7F];
            
#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif
            dopos(-1, 0, 0, 0);
            dopos(1, 0, 0, 0);
            dopos(0,-1, 0, 0);
            dopos(0, 1, 0, 0);
#ifdef _MSC_VER
#pragma warning(default:4127)
#endif
            
            first++;
        }
        
        /* get next from list */
        i = first;
        first = next[first];
        next[i] = UNUSED;
        
        /* second version of loop */
        if(first != LAST)
        {
            depos(first, r, g, b);
            
            val = data[first];
            r2 = (col_diff+128)[((pal[val].r)-(r)) & 0x7F];
            g2 = (col_diff)[((pal[val].g)-(g)) & 0x7F];
            b2 = (col_diff+256)[((pal[val].b)-(b)) & 0x7F];
            
#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif
            dopos(0, 0, 1, 1);
            dopos(0, 0,-1, 1);
            dopos(1, 0, 0, 1);
            dopos(-1, 0, 0, 1);
            dopos(0, 1, 0, 1);
            dopos(0,-1, 0, 1);
#ifdef _MSC_VER
#pragma warning(default:4127)
#endif
            
            if((b < 61) && (data[first + 1] == val))
            {
                b += 2;
                first++;
                b2 = (col_diff+256)[((pal[val].b)-(b)) & 0x7f];
                
#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif
                dopos(-1, 0, 0, 0);
                dopos(1, 0, 0, 0);
                dopos(0,-1, 0, 0);
                dopos(0, 1, 0, 0);
#ifdef _MSC_VER
#pragma warning(default:4127)
#endif
                
                first--;
            }
            
            i = first;
            first = next[first];
            next[i] = UNUSED;
        }
        
        count++;
        
        if(count == (cbcount+1)*AVERAGE_COUNT/256)
        {
            if(cbcount < 256)
            {
                if(callback)
                    callback(cbcount);
                    
                cbcount++;
            }
        }
    }
    
    if(callback)
        while(cbcount < 256)
            callback(cbcount++);
}

int32_t short_bmp_avg(BITMAP *bmp, int32_t i)
{
    int32_t j=((int16_t *)bmp->line[0])[i];
    int32_t r=getr15(j);
    int32_t g=getg15(j);
    int32_t b=getb15(j);
    int32_t k=1;
    
    if(i>0)
    {
        j=((int16_t *)bmp->line[0])[i-1];
        r+=getr15(j);
        g+=getg15(j);
        b+=getb15(j);
        ++k;
    }
    
    if(i<(bmp->w-2))
    {
        j=((int16_t *)bmp->line[0])[i+1];
        r+=getr15(j);
        g+=getg15(j);
        b+=getb15(j);
        ++k;
    }
    
    r/=k;
    g/=k;
    b/=k;
    return makecol15(r, g, b);
}

// A consistent RENG (random enough number generator) for dither_rect()
static uint16_t lfsr;

void lfsrInit()
{
    lfsr=1;
}

uint16_t lfsrNext()
{
    auto bits=(lfsr^(lfsr>>2)^(lfsr>>3)^(lfsr>>5))&1;
    lfsr=(lfsr>>1)|(bits<<15);
    return lfsr;
}

void dither_rect(BITMAP *bmp, PALETTE *pal, int32_t x1, int32_t y1, int32_t x2, int32_t y2,
                 int32_t src_color1, int32_t src_color2, uint8_t dest_color1,
                 uint8_t dest_color2)
{
    BITMAP *src_bmp=create_bitmap_ex(15, abs(x2-x1)+1, 1);
    BITMAP *dest_bmp=create_bitmap_ex(8, abs(x2-x1)+1, abs(y2-y1)+1);
    int32_t r, g, b, direction=1;
    int32_t c;
    int32_t r1, r2, g1, g2, b1, b2;
    //  int32_t diff[2][x2-x1+3][3];
    int32_t (*diff[2])[3];
    diff[0] = new int32_t[x2-x1+3][3];
    diff[1] = new int32_t[x2-x1+3][3];
    int32_t cdiff[3];
    RGB_MAP table;
    int32_t temp;
    int32_t red_rand_strength=0, green_rand_strength=0, blue_rand_strength=0;

    lfsrInit();
    clear_bitmap(dest_bmp);
    
    if(x1>x2)
    {
        temp=x1;
        x1=x2;
        x2=temp;
    }
    
    if(y1>y2)
    {
        temp=y1;
        y1=y2;
        y2=temp;
    }
    
    if(src_color1>src_color2)
    {
        temp=src_color1;
        src_color1=src_color2;
        src_color2=temp;
    }
    
    if(dest_color1>dest_color2)
    {
        temp=dest_color1;
        dest_color1=dest_color2;
        dest_color2=temp;
    }
    
    create_rgb_table_range(&table, *pal, dest_color1, dest_color2, NULL);
    r1=getr15(src_color1);
    r2=getr15(src_color2);
    g1=getg15(src_color1);
    g2=getg15(src_color2);
    b1=getb15(src_color1);
    b2=getb15(src_color2);
    red_rand_strength=getr8(dest_color1+1)-getr8(dest_color1);
    green_rand_strength=getg8(dest_color1+1)-getg8(dest_color1);
    blue_rand_strength=getb8(dest_color1+1)-getb8(dest_color1);
    memset(cdiff,0,3*sizeof(float));
    memset(diff[0],0,(x2-x1+3)*3*sizeof(int32_t));
    memset(diff[1],0,(x2-x1+3)*3*sizeof(int32_t));
    int32_t mc, mr, mg, mb;
    
    for(int32_t i=0; i<src_bmp->w; i++)
    {
        r = mix_value(r1, r2, i, src_bmp->w-1);
        g = mix_value(g1, g2, i, src_bmp->w-1);
        b = mix_value(b1, b2, i, src_bmp->w-1);
        c = makecol15(r,g,b);
        ((int16_t *)src_bmp->line[0])[i] = c;
    }
    
    uint8_t tempcolor, origcolor;
    
    for(int32_t j=0; j<=y2-y1; ++j)
    {
        if(direction==1)
        {
            for(int32_t i=0; i<=x2-x1; ++i)
            {
                mc=((int16_t *)src_bmp->line[0])[i];
                mr=bound(getr15(mc)+lfsrNext()%(red_rand_strength*2+1)-(red_rand_strength*1),0,255);
                mg=bound(getg15(mc)+lfsrNext()%(green_rand_strength*2+1)-(green_rand_strength*1),0,255);
                mb=bound(getb15(mc)+lfsrNext()%(blue_rand_strength*2+1)-(blue_rand_strength*1),0,255);
                cdiff[0]=bound(mr+
                               diff[0][i][0]+
                               diff[0][i+1][0]+
                               diff[0][i+2][0]+
                               cdiff[0],0,255);
                cdiff[1]=bound(mg+
                               diff[0][i][1]+
                               diff[0][i+1][1]+
                               diff[0][i+2][1]+
                               cdiff[1],0,255);
                cdiff[2]=bound(mb+
                               diff[0][i][2]+
                               diff[0][i+1][2]+
                               diff[0][i+2][2]+
                               cdiff[2],0,255);
                //        bmp->line[j][i+x1]=bound(makecol8_map(int32_t(cdiff[0]),int32_t(cdiff[1]),int32_t(cdiff[2]),&table),dest_color1,dest_color2);
                origcolor=makecol8_map(mr,mg,mb,&table);
                //        tempcolor=bound(makecol8_map(cdiff[0],cdiff[1],cdiff[2],&table),origcolor-1,origcolor+1);
                tempcolor=makecol8_map(cdiff[0],cdiff[1],cdiff[2],&table);
                dest_bmp->line[j][i]=tempcolor;
                r=getr8(tempcolor);
                g=getg8(tempcolor);
                b=getb8(tempcolor);
                diff[1][i][0]=(cdiff[0]-r)*3/16;
                diff[1][i][1]=(cdiff[1]-g)*3/16;
                diff[1][i][2]=(cdiff[2]-b)*3/16;
                diff[1][i+1][0]=(cdiff[0]-r)*5/16;
                diff[1][i+1][1]=(cdiff[1]-g)*5/16;
                diff[1][i+1][2]=(cdiff[2]-b)*5/16;
                diff[1][i+2][0]=(cdiff[0]-r)*1/16;
                diff[1][i+2][1]=(cdiff[1]-g)*1/16;
                diff[1][i+2][2]=(cdiff[2]-b)*1/16;
                cdiff[0]=(cdiff[0]-r)*7/16;
                cdiff[1]=(cdiff[1]-g)*7/16;
                cdiff[2]=(cdiff[2]-b)*7/16;
            }
            
            memcpy(diff[0],diff[1],(x2-x1+3)*3*sizeof(int32_t));
            memset(diff[1],0,(x2-x1+3)*3*sizeof(int32_t));
            direction=-1;
        }
        else
        {
            for(int32_t i=x2-x1; i>=0; --i)
            {
                mc=((int16_t *)src_bmp->line[0])[i];
                mr=getr15(mc);
                mg=getg15(mc);
                mb=getb15(mc);
                cdiff[0]=bound(mr+
                               diff[0][i][0]+
                               diff[0][i+1][0]+
                               diff[0][i+2][0]+
                               cdiff[0],0,255);
                cdiff[1]=bound(mg+
                               diff[0][i][1]+
                               diff[0][i+1][1]+
                               diff[0][i+2][1]+
                               cdiff[1],0,255);
                cdiff[2]=bound(mb+
                               diff[0][i][2]+
                               diff[0][i+1][2]+
                               diff[0][i+2][2]+
                               cdiff[2],0,255);
                //        bmp->line[j][i+x1]=bound(makecol8_map(int32_t(cdiff[0]),int32_t(cdiff[1]),int32_t(cdiff[2]),&table),dest_color1,dest_color2);
                origcolor=makecol8_map(mr,mg,mb,&table);
                //        tempcolor=bound(makecol8_map(cdiff[0],cdiff[1],cdiff[2],&table),origcolor-1,origcolor+1);
                tempcolor=makecol8_map(cdiff[0],cdiff[1],cdiff[2],&table);
                dest_bmp->line[j][i]=tempcolor;
                r=getr8(tempcolor);
                g=getg8(tempcolor);
                b=getb8(tempcolor);
                diff[1][i][0]=(cdiff[0]-r)*3/16;
                diff[1][i][1]=(cdiff[1]-g)*3/16;
                diff[1][i][2]=(cdiff[2]-b)*3/16;
                diff[1][i+1][0]=(cdiff[0]-r)*5/16;
                diff[1][i+1][1]=(cdiff[1]-g)*5/16;
                diff[1][i+1][2]=(cdiff[2]-b)*5/16;
                diff[1][i+2][0]=(cdiff[0]-r)*1/16;
                diff[1][i+2][1]=(cdiff[1]-g)*1/16;
                diff[1][i+2][2]=(cdiff[2]-b)*1/16;
                cdiff[0]=(cdiff[0]-r)*7/16;
                cdiff[1]=(cdiff[1]-g)*7/16;
                cdiff[2]=(cdiff[2]-b)*7/16;
            }
            
            memcpy(diff[0],diff[1],(x2-x1+3)*3*sizeof(float));
            memset(diff[1],0,(x2-x1+3)*3*sizeof(float));
            direction=1;
        }
    }
    
    blit(dest_bmp, bmp, 0, 0, x1, y1, x2-x1+1, y2-y1+1);
    delete[] diff[1];
    delete[] diff[0];
    destroy_bitmap(src_bmp);
    destroy_bitmap(dest_bmp);
    return;
}

bool do_text_button(int32_t x,int32_t y,int32_t w,int32_t h,const char *text)
{
    bool over=false;
    
    while(gui_mouse_b())
    {
        if(mouse_in_rect(x,y,w,h))
        {
            if(!over)
            {
                vsync();
                jwin_draw_text_button(screen, x, y, w, h, text, D_SELECTED, true);
                over=true;
				update_hw_screen(true);
            }
        }
        else
        {
            if(over)
            {
                vsync();
                jwin_draw_text_button(screen, x, y, w, h, text, 0, true);
                over=false;
				update_hw_screen(true);
            }
        }
		rest(1);
    }
    
    return over;
}

bool do_text_button_reset(int32_t x,int32_t y,int32_t w,int32_t h,const char *text)
{
    bool over=false;
    
    while(gui_mouse_b())
    {
        //vsync();
        if(mouse_in_rect(x,y,w,h))
        {
            if(!over)
            {
                vsync();
                jwin_draw_text_button(screen, x, y, w, h, text, D_SELECTED, true);
                over=true;
                
				update_hw_screen();
            }
        }
        else
        {
            if(over)
            {
                vsync();
                jwin_draw_text_button(screen, x, y, w, h, text, 0, true);
                over=false;
                
				update_hw_screen();
            }
        }
        rest(1);
    }
    
    if(over)
    {
        vsync();
        jwin_draw_text_button(screen, x, y, w, h, text, 0, true);
        
		update_hw_screen();
    }
    
    return over;
}
bool do_icon_button_reset(int32_t x,int32_t y,int32_t w,int32_t h,int icon)
{
    bool over=false;
    
    while(gui_mouse_b())
    {
        //vsync();
        if(mouse_in_rect(x,y,w,h))
        {
            if(!over)
            {
                vsync();
                jwin_draw_icon_button(screen, x, y, w, h, icon, D_SELECTED, true);
                over=true;
                
				update_hw_screen();
            }
        }
        else
        {
            if(over)
            {
                vsync();
                jwin_draw_icon_button(screen, x, y, w, h, icon, 0, true);
                over=false;
                
				update_hw_screen();
            }
        }
        rest(1);
    }
    
    if(over)
    {
        vsync();
        jwin_draw_icon_button(screen, x, y, w, h, icon, 0, true);
        
		update_hw_screen();
    }
    
    return over;
}

int32_t jwin_tab_proc(int32_t msg, DIALOG *d, int32_t c)
{
	int32_t i;
	int32_t tx;
	int32_t sd=2; //selected delta
	TABPANEL *panel=(TABPANEL *)d->dp;
	DIALOG   *panel_dialog=NULL, *current_object=NULL;
	int32_t selected=0;
	int32_t counter=0;
	ASSERT(d);
	int32_t temp_d, temp_d2;
	
	if(d->dp==NULL) return D_O_K;
	
	panel_dialog=(DIALOG *)d->dp3;
	
    if (msg != MSG_START && msg != MSG_END)
    {
        bool redraw = false;
        for (i = 0; panel[i].text; ++i)
        {
            if ((panel[i].flags & D_SELECTED) && !(d->flags & D_HIDDEN))
            {
                for (counter = 0; counter < panel[i].objects; counter++)
                {
                    current_object = panel_dialog + (panel[i].dialog[counter]);
                    current_object->flags &= ~D_HIDDEN;
                    if (object_message(current_object, MSG_IDLE, 0) & D_REDRAW)
                        redraw = true;
                }
            }
            else
            {
                for (counter = 0; counter < panel[i].objects; counter++)
                {
                    current_object = panel_dialog + (panel[i].dialog[counter]);
                    current_object->flags |= D_HIDDEN;
                    if (object_message(current_object, MSG_IDLE, 0) & D_REDRAW)
                        redraw = true;
                }
            }

            /*if (d->flags & D_HIDDEN)
            {
                for(counter=0; counter<panel[i].objects; counter++)
                {
                  current_object=panel_dialog+(panel[i].dialog[counter]);
                  current_object->x=zq_screen_w*3;
                  current_object->y=zq_screen_h*3;
                }
            }*/
        }
        if (redraw)
            broadcast_dialog_message(MSG_DRAW, 0);
    }
	FONT *oldfont = font;
	switch(msg)
	{
	case MSG_DRAW:
	{
		if(d->x<zq_screen_w&&d->y<zq_screen_h)
		{
			if(d->dp2)
			{
				font = (FONT *)d->dp2;
			}
			
			panel_dialog=(DIALOG *)d->dp3;
			rectfill(screen, d->x, d->y, d->x+d->w-1, d->y+8+text_height(font), scheme[jcBOX]); //tab area
			rectfill(screen, d->x+1, d->y+sd+text_height(font)+7, d->x+d->w-2, d->y+sd+d->h-2, scheme[jcBOX]); //panel
			_allegro_vline(screen, d->x, d->y+sd+7+text_height(font), d->y+sd+d->h-2, scheme[jcLIGHT]);
			_allegro_vline(screen, d->x+1, d->y+sd+7+text_height(font), d->y+sd+d->h-3, scheme[jcMEDLT]);
			_allegro_vline(screen, d->x+d->w-2, d->y+sd+7+text_height(font), d->y+sd+d->h-2, scheme[jcMEDDARK]);
			_allegro_vline(screen, d->x+d->w-1, d->y+sd+7+text_height(font)-1, d->y+sd+d->h-1, scheme[jcDARK]);
			_allegro_hline(screen, d->x+1, d->y+sd+d->h-2, d->x+d->w-3, scheme[jcMEDDARK]);
			_allegro_hline(screen, d->x, d->y+sd+d->h-1, d->x+d->w-2, scheme[jcDARK]);
			tx=d->x;
			
			if(d->dp)
			{
				if(!(panel[((d->d1&0xFF00)>>8)].flags&D_SELECTED))
				{
					_allegro_hline(screen, tx+1, d->y+sd+6+text_height(font)+1, tx+2, scheme[jcMEDLT]); //initial bottom
					_allegro_hline(screen, tx, d->y+sd+6+text_height(font), tx+1, scheme[jcLIGHT]);     //initial bottom
				}
				
				tx+=2;
				
				for(i=0; panel[i].text; ++i)
				{
					if(panel[i].flags&D_SELECTED)
					{
						selected=i;
					}
				}
				
				for(i=((d->d1&0xFF00)>>8); panel[i].text&&i<=last_visible_tab(panel,((d->d1&0xFF00)>>8),d->w); ++i)
				{
					sd=(panel[i].flags&D_SELECTED)?0:2;
					
					if((i==((d->d1&0xFF00)>>8)) || (!(panel[i-1].flags&D_SELECTED)))
					{
						_allegro_vline(screen, tx-(2-sd), d->y+sd+2, d->y+8+text_height(font), scheme[jcLIGHT]); //left side
						_allegro_vline(screen, tx-(2-sd)+1, d->y+sd+2, d->y+8+text_height(font), scheme[jcMEDLT]); //left side
						putpixel(screen, tx+1-(2-sd), d->y+sd+1, scheme[jcLIGHT]);                               //left angle
					}
					
					_allegro_hline(screen, tx+2-(2-sd), d->y+sd, tx+12+(2-sd)+text_length(font, (char *)panel[i].text), scheme[jcLIGHT]); //top
					_allegro_hline(screen, tx+2-(2-sd), d->y+sd+1, tx+12+(2-sd)+text_length(font, (char *)panel[i].text), scheme[jcMEDLT]); //top
					
					if(!(panel[i].flags&D_SELECTED))
					{
						_allegro_hline(screen, tx+1, d->y+sd+6+text_height(font), tx+13+text_length(font, (char *)panel[i].text)+1, scheme[jcLIGHT]); //bottom
						_allegro_hline(screen, tx, d->y+sd+6+text_height(font)+1, tx+13+text_length(font, (char *)panel[i].text)+1, scheme[jcMEDLT]); //bottom
					}
					
					tx+=4;
					gui_textout_ln(screen, (uint8_t*)panel[i].text, tx+4, d->y+sd+4, scheme[jcBOXFG], scheme[jcBOX], 0);
					tx+=text_length(font, (char *)panel[i].text)+10;
					
					if(!(panel[i+1].text) || (!(panel[i+1].flags&D_SELECTED)))
					{
						putpixel(screen, tx-1+(2-sd), d->y+sd+1, scheme[jcDARK]); //right angle
						_allegro_vline(screen, tx+(2-sd), d->y+sd+2, d->y+8+text_height(font)-1, scheme[jcDARK]); //right side
						_allegro_vline(screen, tx+(2-sd)-1, d->y+sd+2, d->y+8+text_height(font)-(sd?1:0), scheme[jcMEDDARK]); //right side
					}
					
					tx++;
				}
				
				if(((d->d1&0xFF00)>>8)!=0||last_visible_tab(panel,((d->d1&0xFF00)>>8),d->w)+1<tab_count(panel))
				{
					jwin_draw_icon_button(screen,d->x+d->w-14,d->y+2, 14, 14, BTNICON_ARROW_RIGHT, 0, true);
					jwin_draw_icon_button(screen,d->x+d->w-28,d->y+2, 14, 14, BTNICON_ARROW_LEFT, 0, true);
				}
			}
			
			if((tx+(2-sd))<(d->x+d->w))
			{
				_allegro_hline(screen, tx+(2-sd)-1, d->y+8+text_height(font), d->x+d->w-1, scheme[jcLIGHT]); //ending bottom
				_allegro_hline(screen, tx+(2-sd)-2, d->y+8+text_height(font)+1, d->x+d->w-2, scheme[jcMEDLT]); //ending bottom
			}
			
			font = oldfont;
			
			//what dialog is this tab control in (programmer must set manually)
			panel_dialog=(DIALOG *)d->dp3;
			
			//for each object handled by the currently selected tab...
			for(counter=0; counter<panel[selected].objects; counter++)
			{
				//assign current_object to one of the controls handled by the tab
				current_object=panel_dialog+(panel[selected].dialog[counter]);
				//remember the x and y positions of the control
				current_object->x=panel[selected].xy[counter*2];
				current_object->y=panel[selected].xy[counter*2+1];
				object_message(current_object, MSG_DRAW, 0);
			}
			
			//if there was a previously selected tab...
			if((d->d1&0x00FF)!=0x00FF)
			{
				//for each object handled by the tab
				for(counter=0; counter<panel[d->d1&0xFF].objects; counter++)
				{
					//assign current_object to one of the controls handled by the tab
					current_object=panel_dialog+(panel[d->d1&0xFF].dialog[counter]);
					//          //remember the x and y positions of the control
					//          panel[d->d1].xy[counter*2]=current_object->x;
					//          panel[d->d1].xy[counter*2+1]=current_object->y;
					current_object->x=zq_screen_w*3;
					current_object->y=zq_screen_h*3;
				}
			}
		}
	}
	break;
	
	case MSG_CLICK:
	{
		d->d1&=0xFF00;
		d->d1|=0x00FF;
		if(d->dp2)
		{
			font = (FONT *)d->dp2;
		}
		
		// is the mouse on one of the tab arrows (if visible) or in the tab area?
		if(uses_tab_arrows(panel, d->w)&&(mouse_in_rect(d->x+d->w-28, d->y+2, 28, 14)))
		{
			if(mouse_in_rect(d->x+d->w-28, d->y+2, 14, 14))
			{
				if(do_icon_button_reset(d->x+d->w-28, d->y+2, 14, 14, BTNICON_ARROW_LEFT))
				{
					temp_d=((d->d1&0xFF00)>>8);
					temp_d2=(d->d1&0x00FF);
					
					if(temp_d>0)
					{
						--temp_d;
					}
					
					d->d1=(temp_d<<8)|temp_d2;
					d->flags|=D_DIRTY;
				}
			}
			else if(mouse_in_rect(d->x+d->w-14, d->y+2, 14, 14))
			{
				if(do_icon_button_reset(d->x+d->w-14, d->y+2, 14, 14, BTNICON_ARROW_RIGHT))
				{
					temp_d=((d->d1&0xFF00)>>8);
					temp_d2=(d->d1&0x00FF);
					
					if(last_visible_tab(panel, temp_d, d->w)<(tab_count(panel)-1))
					{
						++temp_d;
					}
					
					d->d1=(temp_d<<8)|temp_d2;
					d->flags|=D_DIRTY;
				}
			}
		}
		else
		{
			d_tab_proc(msg, d, c);
		}
		font = oldfont;
		jwin_tab_proc(MSG_IDLE,d,0);
	}
	break;
	
	default:
		return d_tab_proc(msg, d, c);
		break;
	}
	
	panel_dialog=(DIALOG *)d->dp3;
	
	if(d->flags & D_HIDDEN)
	{
		for(i=0; panel[i].text; ++i)
		{
			for(counter=0; counter<panel[i].objects; counter++)
			{
				current_object=panel_dialog+(panel[i].dialog[counter]);
				current_object->x=zq_screen_w*3;
				current_object->y=zq_screen_h*3;
			}
		}
		
		//d->x=zq_screen_w*3;
		//d->y=zq_screen_h*3;
	}
	else
	{
		for(i=0; panel[i].text; ++i)
		{
			for(counter=0; counter<panel[i].objects; counter++)
			{
				current_object=panel_dialog+(panel[i].dialog[counter]);
				current_object->x=panel[i].xy[counter*2];
				current_object->y=panel[i].xy[counter*2+1];
			}
		}
		
		// d->x=zq_screen_w*3;
		//d->y=zq_screen_h*3;
	}
	
	return broadcast_dialog_message(MSG_IDLE, 0);
	
	// return D_O_K;
}

int32_t discern_tab(GUI::TabPanel *panel, int32_t first_tab, int32_t x)
{
    int32_t w=0;
    
    for(size_t i=first_tab; i < panel->getSize(); i++)
    {
        w+=text_length(font, panel->getName(i))+15;
        
        if(w>x)
        {
            return i;
        }
    }
    
    return -1;
}
int32_t tabs_width(GUI::TabPanel *panel)
{
    int32_t w=0;
    
    for(size_t i=0; i < panel->getSize(); ++i)
    {
        w+=text_length(font, panel->getName(i))+15;
    }
    
    return w+1;
}
bool uses_tab_arrows(GUI::TabPanel *panel, int32_t maxwidth)
{
	return (tabs_width(panel)>maxwidth);
}
size_t last_visible_tab(GUI::TabPanel *panel, int32_t first_tab, int32_t maxwidth)
{
    size_t i;
    int32_t w=0;
    
    if(uses_tab_arrows(panel, maxwidth))
    {
        maxwidth-=28;
    }
    
    for(i=first_tab; i < panel->getSize(); ++i)
    {
        w+=text_length(font, panel->getName(i))+15;
        
        if(w>maxwidth)
        {
            return i-1;
        }
    }
    
    return i-1;
}
int32_t displayed_tabs_width(GUI::TabPanel *panel, int32_t first_tab, int32_t maxwidth)
{
    size_t i=0;
    int32_t w=0;
    
    for(i=first_tab; i<=last_visible_tab(panel, first_tab, maxwidth); ++i)
    {
        w+=text_length(font, panel->getName(i))+15;
    }
    
    return w+1;
}

INLINE int32_t is_in_rect(int32_t x,int32_t y,int32_t rx1,int32_t ry1,int32_t rx2,int32_t ry2)
{
    return x>=rx1 && x<=rx2 && y>=ry1 && y<=ry2;
}

int32_t new_tab_proc(int32_t msg, DIALOG *d, int32_t c)
{
	assert(d->flags&D_NEW_GUI);
	
    int32_t tx;
	int32_t ret = D_O_K;
    int32_t sd=2; //selected delta
	static bool skipredraw = false;
	GUI::TabPanel *panel=(GUI::TabPanel*)d->dp;
    ASSERT(d);
    
    if(d->dp==NULL) return D_O_K;
    
	FONT *oldfont = font;
	if(d->dp2)
	{
		font = (FONT *)d->dp2;
	}
	
	switch(msg)
	{
		case MSG_DRAW:
		{
			if(skipredraw)
			{
				skipredraw = false;
				ret = D_REDRAW;
				break;
			}
			if(d->x<zq_screen_w&&d->y<zq_screen_h)
			{
				rectfill(screen, d->x, d->y, d->x+d->w-1, d->y+8+text_height(font), scheme[jcBOX]); //tab area
				rectfill(screen, d->x+1, d->y+sd+text_height(font)+7, d->x+d->w-2, d->y+sd+d->h-2, scheme[jcBOX]); //panel
				_allegro_vline(screen, d->x, d->y+sd+7+text_height(font), d->y+sd+d->h-2, scheme[jcLIGHT]);
				_allegro_vline(screen, d->x+1, d->y+sd+7+text_height(font), d->y+sd+d->h-3, scheme[jcMEDLT]);
				_allegro_vline(screen, d->x+d->w-2, d->y+sd+7+text_height(font), d->y+sd+d->h-2, scheme[jcMEDDARK]);
				_allegro_vline(screen, d->x+d->w-1, d->y+sd+7+text_height(font)-1, d->y+sd+d->h-1, scheme[jcDARK]);
				_allegro_hline(screen, d->x+1, d->y+sd+d->h-2, d->x+d->w-3, scheme[jcMEDDARK]);
				_allegro_hline(screen, d->x, d->y+sd+d->h-1, d->x+d->w-2, scheme[jcDARK]);
				tx=d->x;
				
				if(d->dp)
				{
					if(panel->getCurrentIndex() != d->d1)
					{
						_allegro_hline(screen, tx+1, d->y+sd+6+text_height(font)+1, tx+2, scheme[jcMEDLT]); //initial bottom
						_allegro_hline(screen, tx, d->y+sd+6+text_height(font), tx+1, scheme[jcLIGHT]);     //initial bottom
					}
					
					tx+=2;
					
					for(size_t i=d->d1; i < panel->getSize()&&i<=last_visible_tab(panel,d->d1,d->w); ++i)
					{
						sd=(i==panel->getCurrentIndex())?0:2;
						
						if((i==d->d1) || (i-1 != panel->getCurrentIndex()))
						{
							_allegro_vline(screen, tx-(2-sd), d->y+sd+2, d->y+8+text_height(font), scheme[jcLIGHT]); //left side
							_allegro_vline(screen, tx-(2-sd)+1, d->y+sd+2, d->y+8+text_height(font), scheme[jcMEDLT]); //left side
							putpixel(screen, tx+1-(2-sd), d->y+sd+1, scheme[jcLIGHT]);                               //left angle
						}
						
						_allegro_hline(screen, tx+2-(2-sd), d->y+sd, tx+12+(2-sd)+text_length(font, panel->getName(i)), scheme[jcLIGHT]); //top
						_allegro_hline(screen, tx+2-(2-sd), d->y+sd+1, tx+12+(2-sd)+text_length(font, panel->getName(i)), scheme[jcMEDLT]); //top
						
						if(i!=panel->getCurrentIndex())
						{
							_allegro_hline(screen, tx+1, d->y+sd+6+text_height(font), tx+13+text_length(font, panel->getName(i))+1, scheme[jcLIGHT]); //bottom
							_allegro_hline(screen, tx, d->y+sd+6+text_height(font)+1, tx+13+text_length(font, panel->getName(i))+1, scheme[jcMEDLT]); //bottom
						}
						else if(d->flags & D_GOTFOCUS)
						{
							_allegro_hline(screen, tx+1, d->y+sd+6+text_height(font), tx+13+text_length(font, panel->getName(i))+1, scheme[jcBOXFG]); //bottom
							_allegro_hline(screen, tx, d->y+sd+6+text_height(font)+1, tx+13+text_length(font, panel->getName(i))+1, scheme[jcBOXFG]); //bottom
						}
						
						tx+=4;
						uint8_t* pname = (uint8_t*)(panel->getName(i));
						bool dis = panel->getDisabled(i);
						if(dis)
							gui_textout_ln(screen, pname, tx+5, d->y+sd+5, scheme[jcLIGHT], scheme[jcBOX], 0);
						gui_textout_ln(screen, pname, tx+4, d->y+sd+4, scheme[dis ? jcDISABLED_FG : jcBOXFG], dis ? -1 : scheme[jcBOX], 0);
						tx+=text_length(font, (const char*)pname)+10;
						
						if((i+1>=panel->getSize()) || (i+1!=panel->getCurrentIndex()))
						{
							putpixel(screen, tx-1+(2-sd), d->y+sd+1, scheme[jcDARK]); //right angle
							_allegro_vline(screen, tx+(2-sd), d->y+sd+2, d->y+8+text_height(font)-1, scheme[jcDARK]); //right side
							_allegro_vline(screen, tx+(2-sd)-1, d->y+sd+2, d->y+8+text_height(font)-(sd?1:0), scheme[jcMEDDARK]); //right side
						}
						
						tx++;
					}
					
					if(d->d1!=0||last_visible_tab(panel,d->d1,d->w)+1<panel->getSize())
					{
						jwin_draw_icon_button(screen,d->x+d->w-14,d->y+2, 14, 14, BTNICON_ARROW_RIGHT, 0, true);
						jwin_draw_icon_button(screen,d->x+d->w-28,d->y+2, 14, 14, BTNICON_ARROW_LEFT, 0, true);
					}
				}
				
				if((tx+(2-sd))<(d->x+d->w))
				{
					_allegro_hline(screen, tx+(2-sd)-1, d->y+8+text_height(font), d->x+d->w-1, scheme[jcLIGHT]); //ending bottom
					_allegro_hline(screen, tx+(2-sd)-2, d->y+8+text_height(font)+1, d->x+d->w-2, scheme[jcMEDLT]); //ending bottom
				}
				
			}
		}
		break;
		
		case MSG_WANTFOCUS:
			// if(gui_mouse_b())
				ret = D_WANTFOCUS|D_REDRAW;
			break;
		case MSG_GOTFOCUS:
		case MSG_LOSTFOCUS:
			skipredraw = true;
			break;
		case MSG_CHAR:
		{
			int32_t ind = panel->getCurrentIndex();
			auto oldind = ind;
			switch(c>>8)
			{
				case KEY_LEFT:
					do
					{
						if(ind > 0)
						{
							--ind;
						}
						else
						{
							ind = panel->getSize()-1;
						}
					}
					while(ind != oldind && panel->getDisabled(ind));
					break;
				case KEY_RIGHT:
					do
					{
						if(ind+1 < signed(panel->getSize()))
						{
							++ind;
						}
						else
						{
							ind = 0;
						}
					}
					while(ind != oldind && panel->getDisabled(ind));
					break;
				default: ind = -1;
			}
			if(ind > -1 && ind != oldind)
			{
				panel->switchTo(ind);
				GUI_EVENT(d, geCHANGE_SELECTION);
				ret |= D_USED_CHAR;
			}
		}
		break;
			
		case MSG_CLICK:
		{
			// is the mouse on one of the tab arrows (if visible) or in the tab area?
			if(uses_tab_arrows(panel, d->w)&&(mouse_in_rect(d->x+d->w-28, d->y+2, 28, 14)))
			{
				if(mouse_in_rect(d->x+d->w-28, d->y+2, 14, 14))
				{
					if(do_icon_button_reset(d->x+d->w-28, d->y+2, 14, 14, BTNICON_ARROW_LEFT))
					{
						if(d->d1>0)
						{
							--d->d1;
						}
						
						ret |= D_REDRAW;
					}
				}
				else if(mouse_in_rect(d->x+d->w-14, d->y+2, 14, 14))
				{
					if(do_icon_button_reset(d->x+d->w-14, d->y+2, 14, 14, BTNICON_ARROW_RIGHT))
					{
						size_t t = last_visible_tab(panel, d->d1, d->w);
						if(t<(panel->getSize()-1))
						{
							while(t==last_visible_tab(panel, d->d1, d->w))
								++d->d1;
						}
						
						ret |= D_REDRAW;
					}
				}
			}
			else if(is_in_rect(gui_mouse_x(),gui_mouse_y(), d->x+2, d->y+2, d->x+displayed_tabs_width(panel,((d->d1&0xFF00)>>8),d->w), d->y+text_height(font)+9))
			{
				// find out what the new tab (tb) will be (where the mouse is)
				int32_t newtab = discern_tab(panel, d->d1, gui_mouse_x()-d->x-2);
				if(newtab > -1 && newtab != panel->getCurrentIndex() && !panel->getDisabled(newtab))
				{
					panel->switchTo(newtab);
					GUI_EVENT(d, geCHANGE_SELECTION);
				}
			}
		}
		break;
	}
    font = oldfont;
    return ret;
}




int32_t jwin_hline_proc(int32_t msg, DIALOG *d, int32_t)
{
    ASSERT(d);
    
    if(msg==MSG_DRAW)
    {
		if(d->w < 1) return D_O_K;
		for(int q = 0; q <= d->d1; ++q)
		{
			if(d->d2&1)
			{
				_allegro_hline(screen, d->x, d->y+q, d->x+d->w-1, d->fg);
			}
			else
			{
				_allegro_hline(screen, d->x, d->y-q,   d->x+d->w-1, scheme[jcMEDDARK]);
				_allegro_hline(screen, d->x, d->y+1+q, d->x+d->w-1, scheme[jcLIGHT]);
			}
		}
    }
    
    return D_O_K;
}

int32_t jwin_vline_proc(int32_t msg, DIALOG *d, int32_t)
{
    ASSERT(d);
    
    if(msg==MSG_DRAW)
    {
		if(d->h < 1) return D_O_K;
		for(int q = 0; q <= d->d1; ++q)
		{
			if(d->d2&1)
			{
				_allegro_vline(screen, d->x+q,   d->y, d->y+d->h-1, d->fg);
			}
			else
			{
				_allegro_vline(screen, d->x+q,   d->y, d->y+d->h-1, scheme[jcMEDDARK]);
				_allegro_vline(screen, d->x+1-q, d->y, d->y+d->h-1, scheme[jcLIGHT]);
			}
		}
    }
    
    return D_O_K;
}

int32_t jwin_editbox_proc(int32_t msg, DIALOG *d, int32_t c)
{
    return d_editbox_proc(msg, d, c);
}

//centers dialog based on first object, which should be the containing window
void jwin_center_dialog(DIALOG *dialog)
{
    int32_t xc, yc;
    int32_t c;
    ASSERT(dialog);
    
    /* how much to move by? */
    xc = (zq_screen_w - dialog[0].w) / 2 - dialog[0].x;
    yc = (zq_screen_h - dialog[0].h) / 2 - dialog[0].y;
    
    /* move it */
    for(c=0; dialog[c].proc; c++)
    {
        dialog[c].x += xc;
        dialog[c].y += yc;
    }
}
//up-left aligns dialog based on first object, which should be the containing window
void jwin_ulalign_dialog(DIALOG *dialog)
{
    int32_t xc, yc;
    int32_t c;
    ASSERT(dialog);
    
    /* how much to move by? */
    xc = dialog[0].x;
    yc = dialog[0].y;
    
    /* move it */
    for(c=0; dialog[c].proc; c++)
    {
        dialog[c].x -= xc;
        dialog[c].y -= yc;
    }
}

//Custom slider proc
int32_t d_jslider_proc(int32_t msg, DIALOG *d, int32_t c)
{
    BITMAP *gui_bmp = screen;
    BITMAP *slhan = NULL;
    int32_t oldpos, newpos;
    int32_t sfg;                /* slider foreground color */
    int32_t vert = TRUE;        /* flag: is slider vertical? */
    int32_t hh = 7;             /* handle height (width for horizontal sliders) */
    int32_t hmar;               /* handle margin */
    int32_t slp;                /* slider position */
    int32_t mp;                 /* mouse position */
    int32_t irange;
    int32_t slx, sly, slh, slw;
    int32_t msx, msy;
    int32_t retval = D_O_K;
    int32_t upkey, downkey;
    int32_t pgupkey, pgdnkey;
    int32_t homekey, endkey;
    int32_t delta;
    fixed slratio, slmax, slpos;
    typedef int32_t (*SLIDER_TYPE)(void*, int32_t);
    SLIDER_TYPE proc = NULL;
    //int32_t (*proc)(void *cbpointer, int32_t d2value);
    int32_t oldval;
    ASSERT(d);
    
    /* check for slider direction */
    if(d->h < d->w)
        vert = FALSE;
        
    /* set up the metrics for the control */
    if(d->dp != NULL)
    {
        slhan = (BITMAP *)d->dp;
        
        if(vert)
            hh = slhan->h;
        else
            hh = slhan->w;
    }
    
    hmar = hh/2;
    irange = (vert) ? d->h : d->w;
    slmax = itofix(irange-hh);
    slratio = slmax / (d->d1);
    slpos = slratio * d->d2;
    slp = fixtoi(slpos);
    
    switch(msg)
    {
    
    case MSG_DRAW:
        sfg = (d->flags & D_DISABLED) ? scheme[jcDISABLED_FG] : d->fg;
        
        if(vert)
        {
            rectfill(gui_bmp, d->x, d->y, d->x+d->w/2-2, d->y+d->h-1, d->bg);
            rectfill(gui_bmp, d->x+d->w/2-1, d->y, d->x+d->w/2+1, d->y+d->h-1, sfg);
            rectfill(gui_bmp, d->x+d->w/2+2, d->y, d->x+d->w-1, d->y+d->h-1, d->bg);
        }
        else
        {
            rectfill(gui_bmp, d->x, d->y, d->x+d->w-1, d->y+d->h/2-2, d->bg);
            rectfill(gui_bmp, d->x, d->y+d->h/2-1, d->x+d->w-1, d->y+d->h/2+1, sfg);
            rectfill(gui_bmp, d->x, d->y+d->h/2+2, d->x+d->w-1, d->y+d->h-1, d->bg);
        }
        
        /* okay, background and slot are drawn, now draw the handle */
        if(slhan)
        {
            if(vert)
            {
                slx = d->x+(d->w/2)-(slhan->w/2);
                sly = d->y+(d->h-1)-(hh+slp);
            }
            else
            {
                slx = d->x+slp;
                sly = d->y+(d->h/2)-(slhan->h/2);
            }
            
            draw_sprite(gui_bmp, slhan, slx, sly);
        }
        else
        {
            /* draw default handle */
            if(vert)
            {
                slx = d->x;
                sly = d->y+(d->h)-(hh+slp);
                slw = d->w-1;
                slh = hh-1;
            }
            else
            {
                slx = d->x+slp;
                sly = d->y;
                slw = hh-1;
                slh = d->h-1;
            }
            
            /* draw body */
            rectfill(gui_bmp, slx+2, sly, slx+(slw-2), sly+slh, sfg);
            vline(gui_bmp, slx+1, sly+1, sly+slh-1, sfg);
            vline(gui_bmp, slx+slw-1, sly+1, sly+slh-1, sfg);
            vline(gui_bmp, slx, sly+2, sly+slh-2, sfg);
            vline(gui_bmp, slx+slw, sly+2, sly+slh-2, sfg);
            vline(gui_bmp, slx+1, sly+2, sly+slh-2, d->bg);
            hline(gui_bmp, slx+2, sly+1, slx+slw-2, d->bg);
            putpixel(gui_bmp, slx+2, sly+2, d->bg);
        }
        
        if(d->flags & D_GOTFOCUS)
            dotted_rect(gui_bmp, d->x, d->y, d->x+d->w-1, d->y+d->h-1, sfg, d->bg);
            
        break;
        
    case MSG_WANTFOCUS:
    case MSG_LOSTFOCUS:
        return D_WANTFOCUS;
        
    case MSG_KEY:
        if(!(d->flags & D_GOTFOCUS))
            return D_WANTFOCUS;
        else
            return D_O_K;
            
    case MSG_CHAR:
        /* handle movement keys to move slider */
        c >>= 8;
        
        if(vert)
        {
            upkey = KEY_UP;
            downkey = KEY_DOWN;
            pgupkey = KEY_PGUP;
            pgdnkey = KEY_PGDN;
            homekey = KEY_END;
            endkey = KEY_HOME;
        }
        else
        {
            upkey = KEY_RIGHT;
            downkey = KEY_LEFT;
            pgupkey = KEY_PGDN;
            pgdnkey = KEY_PGUP;
            homekey = KEY_HOME;
            endkey = KEY_END;
        }
        
        if(c == upkey)
            delta = 1;
        else if(c == downkey)
            delta = -1;
        else if(c == pgdnkey)
            delta = -d->d1 / 16;
        else if(c == pgupkey)
            delta = d->d1 / 16;
        else if(c == homekey)
            delta = -d->d2;
        else if(c == endkey)
            delta = d->d1 - d->d2;
        else
            delta = 0;
            
        if(delta)
        {
            oldpos = slp;
            oldval = d->d2;
            
            //while (true) {
            for(; ;)     //thank you, MSVC ~pkmnfrk
            {
                d->d2 = d->d2+delta;
                slpos = slratio*d->d2;
                slp = fixtoi(slpos);
                
                if((slp != oldpos) || (d->d2 <= 0) || (d->d2 >= d->d1))
                    break;
            }
            
            if(d->d2 < 0)
                d->d2 = 0;
                
            if(d->d2 > d->d1)
                d->d2 = d->d1;
                
            retval = D_USED_CHAR;
            
            if(d->d2 != oldval)
            {
                /* call callback function here */
                if(d->dp2)
                {
                    proc = (SLIDER_TYPE)(d->dp2);
                    retval |= (*proc)(d->dp3, d->d2);
                }

				GUI_EVENT(d, geCHANGE_VALUE);
                
                object_message(d, MSG_DRAW, 0);
            }
        }
        
        break;
        
    case MSG_WANTWHEEL:
        return 1;

    case MSG_WHEEL:
        oldval = d->d2;
        d->d2 = MID(0, d->d2+c, d->d1);
        
        if(d->d2 != oldval)
        {
            /* call callback function here */
            if(d->dp2)
            {
                proc = (SLIDER_TYPE)(d->dp2);
                retval |= (*proc)(d->dp3, d->d2);
            }
            
			GUI_EVENT(d, geCHANGE_VALUE);
            object_message(d, MSG_DRAW, 0);
			retval |= D_REDRAWME;
        }
        
        break;
        
    case MSG_CLICK:
        /* track the mouse until it is released */
        mp = slp;
        
        while(gui_mouse_b())
        {
            msx = gui_mouse_x();
            msy = gui_mouse_y();
            oldval = d->d2;
            
            if(vert)
                mp = (d->y+d->h-hmar)-msy;
            else
                mp = msx-(d->x+hmar);
                
            if(mp < 0)
                mp = 0;
                
            if(mp > irange-hh)
                mp = irange-hh;
                
            slpos = itofix(mp);
            slmax = fixdiv(slpos, slratio);
            newpos = fixtoi(slmax);
            
            if(newpos != oldval)
            {
                d->d2 = newpos;
                
                /* call callback function here */
                if(d->dp2 != NULL)
                {
                    proc = (SLIDER_TYPE)(d->dp2);
                    retval |= (*proc)(d->dp3, d->d2);
                }
                
				GUI_EVENT(d, geCHANGE_VALUE);
                object_message(d, MSG_DRAW, 0);
            }
            
            /* let other objects continue to animate */
            broadcast_dialog_message(MSG_IDLE, 0);
			update_hw_screen();
        }
        
        break;
    }
    
    return retval;
}

// This is only used by jwin_check_proc and jwin_radio_proc.
int32_t d_jwinbutton_proc(int32_t msg, DIALOG *d, int32_t)
{
    BITMAP *gui_bmp;
    int32_t state1, state2;
    int32_t black;
    int32_t swap;
    int32_t g;
    ASSERT(d);
    
    gui_bmp = screen;
    
    switch(msg)
    {
		case MSG_DRAW:
		{
			if(d->flags & D_SELECTED)
			{
				g = 1;
				state1 = d->bg;
				state2 = (d->flags & D_DISABLED) ? scheme[jcDISABLED_FG] : d->fg;
			}
			else
			{
				g = 0;
				state1 = (d->flags & D_DISABLED) ? scheme[jcDISABLED_FG] : d->fg;
				state2 = d->bg;
			}
			
			rectfill(gui_bmp, d->x+1+g, d->y+1+g, d->x+d->w-3+g, d->y+d->h-3+g, state2);
			rect(gui_bmp, d->x+g, d->y+g, d->x+d->w-2+g, d->y+d->h-2+g, state1);
			gui_textout_ex(gui_bmp, (char *)d->dp, d->x+d->w/2+g, d->y+d->h/2-text_height(font)/2+g, state1, -1, TRUE);
			
			if(d->flags & D_SELECTED)
			{
				vline(gui_bmp, d->x, d->y, d->y+d->h-2, d->bg);
				hline(gui_bmp, d->x, d->y, d->x+d->w-2, d->bg);
			}
			else
			{
				black = makecol(0,0,0);
				vline(gui_bmp, d->x+d->w-1, d->y+1, d->y+d->h-2, black);
				hline(gui_bmp, d->x+1, d->y+d->h-1, d->x+d->w-1, black);
			}
			
			if((d->flags & D_GOTFOCUS) &&
					(!(d->flags & D_SELECTED) || !(d->flags & D_EXIT)))
				dotted_rect(gui_bmp, d->x+1+g, d->y+1+g, d->x+d->w-3+g, d->y+d->h-3+g, state1, state2);
				
			break;
		}
		case MSG_WANTFOCUS:
			return D_WANTFOCUS;
			
		case MSG_KEY:
		{
			/* close dialog? */
			if(d->flags & D_EXIT)
			{
				return D_CLOSE;
			}
			
			/* or just toggle */
			d->flags ^= D_SELECTED;
			GUI_EVENT(d, geTOGGLE);
			object_message(d, MSG_DRAW, 0);
			break;
		}
		
		case MSG_CLICK:
		{
			/* what state was the button originally in? */
			state1 = d->flags & D_SELECTED;
			
			swap = state1;
				
			/* track the mouse until it is released */
			while(gui_mouse_b())
			{
				state2 = ((gui_mouse_x() >= d->x) && (gui_mouse_y() >= d->y) &&
						  (gui_mouse_x() < d->x + d->w) && (gui_mouse_y() < d->y + d->h));
						  
				if(swap)
					state2 = !state2;
					
				/* redraw? */
				bool should_redraw = false;
				if(((state1) && (!state2)) || ((state2) && (!state1)))
				{
					d->flags ^= D_SELECTED;
					GUI_EVENT(d, geTOGGLE);
					state1 = d->flags & D_SELECTED;
					object_message(d, MSG_DRAW, 0);
					should_redraw = true;
				}
				
				/* let other objects continue to animate */
				int r = broadcast_dialog_message(MSG_IDLE, 0);
				if (r & D_REDRAWME) should_redraw = true;

				if (should_redraw)
				{
					update_hw_screen();
				}
			}
			
			if(d->dp3 != NULL)
			{
				//object_message(d, MSG_DRAW, 0);
				typedef int32_t (*funcType)(void);
				funcType func=reinterpret_cast<funcType>(d->dp3);
				
				return func();
			}
			
			/* should we close the dialog? */
			if(d->flags & D_EXIT)
			{
				return D_CLOSE;
			}
			break;
		}
    }
    
    return D_O_K;
}

//Misc bitmap drawing
void draw_x(BITMAP* dest, int x1, int y1, int x2, int y2, int color)
{
	line(dest, x1, y1, x2, y2, color);
	line(dest, x1, y2, x2, y1, color);
}

void draw_check(BITMAP* dest, int x1, int y1, int x2, int y2, int c)
{
	if(x2 < x1)
		zc_swap(x2,x1);
	if(y2 < y1)
		zc_swap(y2,y1);
	int x3 = ((x2-x1)/2)+x1;
	int y3 = y2-(x3-x1);
	line(dest, x1, y3, x3, y2, c);
	line(dest, x3, y2, x2, y1, c);
}

void draw_checkerboard(BITMAP* dest, int x, int y, int sz, optional<int> cb_sz, int offx, int offy)
{
	if(!cb_sz)
		cb_sz = sz/2;
	int ox = -x+offx, oy = -y+offy;
	ditherrectfill(dest, x, y, x+sz-1, y+sz-1, vc(CheckerCol1), dithChecker, *cb_sz, ox, oy, vc(CheckerCol2));
}

int32_t d_vsync_proc(int32_t msg,DIALOG *d,int32_t c)
{
	static std::chrono::steady_clock::time_point tics;
	auto now = std::chrono::steady_clock::now();

    switch(msg)
    {
		case MSG_START:
			tics = std::chrono::steady_clock::now();
			break;
			
		case MSG_IDLE:
		{
			int num_vsyncs = 0;
			while (now - tics >= std::chrono::milliseconds(1000/60))
			{
				tics += std::chrono::milliseconds(1000/60);
				broadcast_dialog_message(MSG_VSYNC, c);
				if(d->dp)
				{
					int32_t ret = (*(std::function<int32_t()>*)d->dp)();
					switch(ret)
					{
						case ONTICK_EXIT:
							if(d->flags&D_NEW_GUI)
								close_new_gui_dlg(d);
							return D_EXIT;
						case ONTICK_CLOSE:
							if(d->flags&D_NEW_GUI)
							{
								//Simulate a GUI_EVENT for the window proc
								DIALOG* window = d-1;
								while(window->proc != jwin_win_proc) --window;
								int32_t ret = new_gui_event(window-1, geCLOSE);
								if(ret >= 0)
									return ret;
							}
							return D_EXIT;
						case ONTICK_REDRAW:
							return D_REDRAW;
					}
				}

				if (++num_vsyncs == 3) break;
			}
			break;
		}
    }
    
    return D_O_K;
}

//

void draw_checkbox(BITMAP *dest,int x,int y,int sz,bool value)
{
	draw_checkbox(dest,x,y,sz,sz,value);
}
void draw_checkbox(BITMAP *dest,int x,int y,int wid,int hei,bool value)
{
	jwin_draw_frame(dest, x, y, wid, hei, FR_DEEP);
	rectfill(dest, x+2, y+2, x+wid-3, y+hei-3, jwin_pal[jcTEXTBG]);
	
	if(value)
	{
		line(dest, x+2, y+2, x+wid-3, y+hei-3, jwin_pal[jcTEXTFG]);
		line(dest, x+2, y+hei-3, x+wid-3, y+2, jwin_pal[jcTEXTFG]);
	}
}
void draw_dis_checkbox(BITMAP *dest,int x,int y,int wid,int hei,bool value)
{
	jwin_draw_frame(dest, x, y, wid, hei, FR_DEEP);
	
	if(value)
	{
		line(dest, x+2, y+2, x+wid-3, y+hei-3, jwin_pal[jcTEXTFG]);
		line(dest, x+2, y+hei-3, x+wid-3, y+2, jwin_pal[jcTEXTFG]);
	}
}

bool do_scheckbox(BITMAP *dest,int x,int y,int sz,int &value, int xoffs, int yoffs)
{
	return do_checkbox(dest,x,y,sz,sz,value,xoffs,yoffs);
}
bool do_checkbox(BITMAP *dest,int x,int y,int wid,int hei,int &value, int xoffs, int yoffs)
{
	bool over=false;
	
	while(gui_mouse_b())
	{
		if(isinRect(gui_mouse_x()-xoffs,gui_mouse_y()-yoffs,x,y,x+wid-1,y+hei-1))
		{
			if(!over)
			{
				value=value?0:1;
				draw_checkbox(dest,x,y,wid,hei,value!=0);
				over=true;
				update_hw_screen(true);
			}
		}
		else
		{
			if(over)
			{
				value=value?0:1;
				draw_checkbox(dest,x,y,wid,hei,value!=0);
				over=false;
				update_hw_screen(true);
			}
		}
		rest(1);
	}
	
	return over;
}
bool do_checkbox_tx(BITMAP *dest,int x,int y,int wid,int hei,int &value, int txtoffs, int xoffs, int yoffs)
{
	bool over=false;
	
	while(gui_mouse_b())
	{
		if(isinRect(gui_mouse_x()-xoffs,gui_mouse_y()-yoffs,x,y,x+wid-1+txtoffs,y+hei-1))
		{
			if(!over)
			{
				value=value?0:1;
				draw_checkbox(dest,x,y,wid,hei,value!=0);
				over=true;
				update_hw_screen(true);
			}
		}
		else
		{
			if(over)
			{
				value=value?0:1;
				draw_checkbox(dest,x,y,wid,hei,value!=0);
				over=false;
				update_hw_screen(true);
			}
		}
		rest(1);
	}
	
	return over;
}

//box_out stuff
static int32_t box_x = 0;
static int32_t box_y = 0;
static bool box_active=false;
static int32_t box_store_x = 0;
static FONT *box_title_font=font;
static FONT *box_message_font=font;
static int32_t box_style=0;
static int32_t box_titlebar_height=0;
static int32_t box_message_height=0;
static uint8_t box_text_scale=1;
static int32_t box_w=304;
static int32_t box_h=176;
static int32_t box_l=8;
static int32_t box_r=312;
static int32_t box_t=32;
static int32_t box_b=208;
static bool box_log=true;
static char box_log_msg[480];
static int32_t box_msg_pos=0;
static int32_t box_store_pos=0;

int32_t onSnapshot2()
{
    char buf[20];
    int32_t num=0;
    
    do
    {
        sprintf(buf, "zelda%03d.bmp", ++num);
    }
    while(num<999 && exists(buf));
    
    PALETTE temppal;
    get_palette(temppal);
    BITMAP *tempbmp=create_bitmap_ex(8,screen->w, screen->h);
    blit(screen,tempbmp,0,0,0,0,screen->w,screen->h);
    save_bitmap(buf,screen,temppal);
    destroy_bitmap(tempbmp);
    return D_O_K;
}

void set_default_box_size()
{
	int32_t screen_w = screen->w;
	int32_t screen_h = screen->h;
	
	box_w=MIN(512, screen_w-16);
	box_h=MIN(256, (screen_h-64)&0xFFF0);
	
	box_l=(screen_w-box_w)/2;
	box_t=(screen_h-box_h)/2;
	box_r=box_l+box_w;
	box_b=box_t+box_h;
}
/* resizes the box */
void set_box_size(int32_t w, int32_t h)
{
	int32_t screen_w = zq_screen_w;
	int32_t screen_h = zq_screen_h;
	
	if(w <= 0) w = 512;
	if(h <= 0) h = 256;
	box_w=MIN(w, screen_w-16);
	box_h=MIN(h, (screen_h-64)&0xFFF0);
	
	box_l=(screen_w-box_w)/2;
	box_t=(screen_h-box_h)/2;
	box_r=box_l+box_w;
	box_b=box_t+box_h;
}

/* starts outputting a progress message */
void box_start(int32_t style, const char *title, FONT *title_font, FONT *message_font, bool log, int32_t w, int32_t h, uint8_t scale)
{
	if (is_headless())
		return;

	box_text_scale=scale;
	box_style=style;
	box_title_font=(title_font!=NULL)?title_font:font;
	box_message_font=(message_font!=NULL)?message_font:font;
	box_message_height=text_height(box_message_font)*scale;
	box_titlebar_height=title?text_height(box_title_font)+2:0;
	set_box_size(w,h);
	/*
	box_w=BOX_W;
	box_h=BOX_H;
	box_l=BOX_L;
	box_r=BOX_R;
	box_t=BOX_T;
	box_b=BOX_B;
	*/
	box_log=log;
	memset(box_log_msg, 0, 480);
	box_msg_pos=0;
	box_store_pos=0;
	
	if(!box_active)
		popup_zqdialog_start();
	jwin_draw_win(screen, box_l, box_t, box_r-box_l, box_b-box_t, FR_WIN);
	
	if(title!=NULL)
	{
		zc_swap(font,box_title_font);
		jwin_draw_titlebar(screen, box_l+3, box_t+3, box_r-box_l-6, 18, title, false);
		zc_swap(font,box_title_font);
		box_titlebar_height=18;
	}
	
	
	box_store_x = box_x = box_y = 0;
	box_active = true;
	box_t+=box_titlebar_height;
	box_h-=box_titlebar_height;
	box_log=log;
	memset(box_log_msg, 0, 480);
	box_msg_pos=0;
	box_store_pos=0;
}

/* outputs text to the progress message */
void box_out(const char *msg)
{
	string remainder = "";
	string temp(msg);
	
	if(box_active)
	{
		//do primitive text wrapping
		uint32_t i;
		for(i=0; i<temp.size(); i++)
		{
			int32_t length = text_length(box_message_font,temp.substr(0,i).c_str())*box_text_scale;
			
			if(length > box_r-box_l-16)
			{
				i = zc_max(i-1,0);
				break;
			}
		}
		
		set_clip_rect(screen, box_l+8, box_t+1, box_r-8, box_b-1);
		if(box_text_scale == 1)
			textout_ex(screen, box_message_font, temp.substr(0,i).c_str(), box_l+8+box_x, box_t+(box_y+1)*box_message_height, gui_fg_color, gui_bg_color);
		else
		{
			int32_t length = text_length(box_message_font,temp.substr(0,i).c_str());
			BITMAP* tempbit = create_bitmap_ex(8, length, box_message_height);
			clear_bitmap(tempbit);
			textout_ex(tempbit, box_message_font, temp.substr(0,i).c_str(), 0, 0, gui_fg_color, gui_bg_color);
			stretch_blit(tempbit, screen, 0, 0, length, box_message_height/box_text_scale, box_l+8+box_x, box_t+(box_y+1)*box_message_height, length*box_text_scale, box_message_height);
			destroy_bitmap(tempbit);
		}
		set_clip_rect(screen, 0, 0, screen->w-1, screen->h-1);
		remainder = temp.substr(i,temp.size()-i);
	}
	
	if(box_log)
	{
		sprintf(box_log_msg+box_msg_pos, "%s", msg);
	}
	
	box_x += text_length(box_message_font, msg);
	box_msg_pos+=(int32_t)strlen(msg);
	
	if(remainder != "")
	{
		bool oldlog = box_log;
		box_log = false;
		box_eol();
		box_out(remainder.c_str());
		box_log = oldlog;
	}
	
	if (box_active)
		update_hw_screen(true);
}

/* calls box_out, and box_eol for newlines */
void box_out_nl(const char *msg)
{
	string line;
	istringstream reader(msg);
	while (getline(reader, line))
	{
		box_out(line.c_str());
		box_eol();
	}
}
		
/* remembers the current x position */
void box_save_x()
{
	if(box_active)
	{
		box_store_x=box_x;
	}
	
	box_store_pos=box_msg_pos;
}

/* remembers the current x position */
void box_load_x()
{
	if(box_active)
	{
		box_x=box_store_x;
	}
	
	box_msg_pos=box_store_pos;
}

/* outputs text to the progress message */
void box_eol()
{
	if(box_active)
	{
		box_x = 0;
		box_y++;
		
		if((box_y+2)*box_message_height >= box_h)
		{
			blit(screen, screen, box_l+8, box_t+(box_message_height*2), box_l+8, box_t+(box_message_height), box_w-16, box_y*box_message_height);
			rectfill(screen, box_l+8, box_t+box_y*box_message_height, box_l+box_w-8, box_t+(box_y+1)*box_message_height, gui_bg_color);
			box_y--;
		}
	}
	
	box_msg_pos = 0;
	
	if(box_log)
	{
		al_trace("%s", box_log_msg);
		al_trace("\n");
		memset(box_log_msg, 0, 480);
	}
	
	if (box_active)
		update_hw_screen(true);
}

/* ends output of a progress message */
void box_end(bool pause)
{
	if(box_active)
	{
		if(pause)
		{
			box_eol();
			box_pause();
		}
		
		box_active = false;
		popup_zqdialog_end();
	}
}

/* pauses box output */
void box_pause()
{
	if(box_active)
	{
		box_save_x();
		box_out("-- press a key --");
		
		while(gui_mouse_b()) rest(1);
		while(!(keypressed() || gui_mouse_b())) rest(1);
		while(gui_mouse_b()) rest(1);
		
		clear_keybuf();
		box_load_x();
	}
}


/***  The End  ***/
