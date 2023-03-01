#include "precompiled.h" //always first

#include <ctype.h>
#include <string.h>
#include "base/zc_alleg.h"
#include <allegro/internal/aintern.h>
#include "jwin_a5.h"
#include "editbox.h"
#include <iostream>
#include <sstream>
#include "base/zsys.h"
#include <stdio.h>
#include "base/util.h"
#include "pal.h"
#include "gui/tabpanel.h"
#include "gui/text_field.h"
#include "dialog/info.h"
using namespace util;

#define zc_max(a,b)  ((a)>(b)?(a):(b))
#define zc_min(a,b)  ((a)<(b)?(a):(b))

void update_hw_screen(bool force);
extern int32_t zq_screen_w, zq_screen_h;
extern int32_t joystick_index;

extern bool is_zquest();
extern byte quest_rules[QUESTRULES_NEW_SIZE];
//Externed from jwin.cpp
extern bool no_hline;
extern char abc_keypresses[1024];

bool is_in_rect(int32_t x,int32_t y,int32_t rx1,int32_t ry1,int32_t rx2,int32_t ry2);
int32_t count_newline(uint8_t *s);
void idle_cb();
void wipe_abc_keypresses();
int32_t droplist(DIALOG *d);

//JWin A5 Palette

ALLEGRO_COLOR jwin_a5_colors[9];
ALLEGRO_COLOR db_a5_colors[9];
static ALLEGRO_COLOR tmpcol[9];
const int db_pal[] =
{
	4, 2, 3, 4, 8, 1,
	3, 7, 1, 6, 1, 5, 1,
	1, 1, 3, 4,
	3, 1, 3, 4
	// jcBOX, jcLIGHT, jcMEDLT, jcMEDDARK, jcDARK, jcBOXFG,
	// jcTITLEL, jcTITLER, jcTITLEFG, jcTEXTBG, jcTEXTFG, jcSELBG, jcSELFG,
	// jcCURSORMISC, jcCURSOROUTLINE, jcCURSORLIGHT, jcCURSORDARK,
	// jcALT_TEXTBG, jcALT_TEXTFG, jcDISABLED_BG, jcDISABLED_FG,
};
ALLEGRO_COLOR jwin_a5_pal(int jc)
{
	if(devcfg_active) return jwin_a5_colors[db_pal[jc]];
	return jwin_a5_colors[r_dvc(jwin_pal[jc])];
}

ALLEGRO_COLOR AL5_INVIS = al_map_rgba(0,0,0,0);
ALLEGRO_COLOR AL5_BLACK = al_map_rgb(0,0,0);
ALLEGRO_COLOR AL5_WHITE = al_map_rgb(255,255,255);
ALLEGRO_COLOR AL5_YELLOW = al_map_rgb(255,255,0);
ALLEGRO_COLOR AL5_PINK = al_map_rgb(255,0,255);
ALLEGRO_COLOR AL5_LGRAY = al_map_rgb(85,85,85);
ALLEGRO_COLOR AL5_DGRAY = al_map_rgb(170,170,170);
void jwin_reset_a5_colors()
{
	jwin_set_a5_colors(tmpcol);
}
void jwin_set_a5_colors(ALLEGRO_COLOR* colors, bool setmain)
{
	for(int q = 1; q <= 8; ++q)
	{
		jwin_a5_colors[q] = (devcfg_active ? db_a5_colors : colors)[q];
		if(setmain) tmpcol[q] = colors[q];
	}
}
void jwin_get_a5_colors(ALLEGRO_COLOR* colors, bool getmain)
{
	for(int q = 1; q <= 8; ++q)
		colors[q] = (getmain ? tmpcol : jwin_a5_colors)[q];
}

//Generic A5 helpers

void al_draw_hline(float x1, float y1, float x2, ALLEGRO_COLOR c)
{
	al_draw_line(x1,y1,x2,y1,c,1);
}
void al_draw_vline(float x1, float y1, float y2, ALLEGRO_COLOR c)
{
	al_draw_line(x1,y1,x1,y2,c,1);
}
void al_draw_x(float x1, float y1, float x2, float y2, ALLEGRO_COLOR c, float thickness)
{
	al_draw_line(x1,y1,x2,y2,c,thickness);
	al_draw_line(x1,y2,x2,y1,c,thickness);
}

//Static handling
static ALLEGRO_COLOR grayscale[256];
void init_gscale()
{
	static bool initd = false;
	if(initd) return;
	for(int q = 0; q < 256; ++q)
	{
		grayscale[q] = al_map_rgb(q,q,q);
	}
	initd = true;
}
static ALLEGRO_BITMAP* static_tex;
#define STATIC_SZ 1024
void init_static_texture()
{
	static bool initd = false;
	if(initd) return;
	init_gscale();
	
	set_bitmap_create_flags(true);
	static_tex = al_create_bitmap(STATIC_SZ,STATIC_SZ);
	al_set_new_bitmap_flags(0);
	
	
	ALLEGRO_STATE oldstate;
	al_store_state(&oldstate, ALLEGRO_STATE_TARGET_BITMAP);
	
	al_set_target_bitmap(static_tex);
	al_lock_bitmap(static_tex,ALLEGRO_PIXEL_FORMAT_ANY,ALLEGRO_LOCK_WRITEONLY);
	for(int x = 0; x < STATIC_SZ; ++x)
		for(int y = 0; y < STATIC_SZ; ++y)
		{
			al_put_pixel(x,y,grayscale[zc_rand(255)]);
		}
	al_unlock_bitmap(static_tex);
	al_restore_state(&oldstate);
	initd = true;
}
void draw_static(int x, int y, int w, int h)
{
	assert(w<=STATIC_SZ && h<=STATIC_SZ);
	init_static_texture();
	int sx = zc_rand(STATIC_SZ-w);
	int sy = zc_rand(STATIC_SZ-h);
	al_draw_bitmap_region(static_tex,sx,sy,w,h,x,y,0);
}
//JWin A5 drawing functions

void jwin_draw_frame_a5(int32_t x,int32_t y,int32_t w,int32_t h,int32_t style)
{
	int32_t c1,c2,c3,c4;
	
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
	
	al_draw_hline(x, y+1, x+w-1, jwin_a5_pal(c1));
	al_draw_vline(x+1, y+1, y+h-1, jwin_a5_pal(c1));
	
	al_draw_hline(x+1, y+2, x+w-2, jwin_a5_pal(c2));
	al_draw_vline(x+2, y+2, y+h-2, jwin_a5_pal(c2));
	
	al_draw_hline(x+1, y+h-1, x+w-1, jwin_a5_pal(c3));
	al_draw_vline(x+w-1, y+1, y+h-2, jwin_a5_pal(c3));
	
	al_draw_hline(x, y+h, x+w, jwin_a5_pal(c4));
	al_draw_vline(x+w, y, y+h-1, jwin_a5_pal(c4));
}
void jwin_draw_win_a5(int32_t x,int32_t y,int32_t w,int32_t h,int32_t frame)
{
	al_draw_filled_rectangle(x,y,x+w,y+h,jwin_a5_pal(jcBOX));
	jwin_draw_frame_a5(x, y, w, h, frame);
}

void dotted_rect_a5(int32_t x1, int32_t y1, int32_t x2, int32_t y2, ALLEGRO_COLOR fg, ALLEGRO_COLOR bg)
{
	int32_t x = ((x1+y1) & 1) ? 1 : 0;
	int32_t c;
	
	ALLEGRO_BITMAP* dest = al_get_target_bitmap();
	if(!al_lock_bitmap_region(dest,x1,y1,x2-x1+1,y2-y1+1,ALLEGRO_PIXEL_FORMAT_ANY,ALLEGRO_LOCK_READWRITE))
		return;
	
	for(c=x1; c<=x2; c++)
	{
		al_put_pixel(c, y1, (((c+y1) & 1) == x) ? fg : bg);
		al_put_pixel(c, y2, (((c+y2) & 1) == x) ? fg : bg);
	}
	
	for(c=y1+1; c<y2; c++)
	{
		al_put_pixel(x1, c, (((c+x1) & 1) == x) ? fg : bg);
		al_put_pixel(x2, c, (((c+x2) & 1) == x) ? fg : bg);
	}
	
	al_unlock_bitmap(dest);
}

void jwin_textout_a5(ALLEGRO_FONT* f, ALLEGRO_COLOR tc, float x, float y, int flag, char const* str)
{
	al_draw_text(f,tc,x,y,flag,str);
}
void jwin_textout_a5(ALLEGRO_FONT* f, ALLEGRO_COLOR tc, float x, float y, int flag, char const* str, ALLEGRO_COLOR bgc)
{
	unsigned char r,g,b,a;
	al_unmap_rgba(bgc,&r,&g,&b,&a);
	if(a)
	{
		float w = al_get_text_width(f, str);
		float h = al_get_font_line_height(f);
		float bgx = x;
		float bgy = y;
		switch(flag)
		{
			case ALLEGRO_ALIGN_LEFT:
				break;
			case ALLEGRO_ALIGN_CENTRE:
				bgx -= w/2;
				break;
			case ALLEGRO_ALIGN_RIGHT:
				bgx -= w;
				break;
		}
		al_draw_filled_rectangle(bgx,bgy,bgx+w,bgy+h,bgc);
	}
	al_draw_text(f,tc,x,y,flag,str);
}
void jwin_textout_a5_dis(ALLEGRO_FONT* f, ALLEGRO_COLOR tc, float x, float y, int flag, char const* str, ALLEGRO_COLOR bgc, ALLEGRO_COLOR dis_c)
{
	unsigned char r,g,b,a;
	al_unmap_rgba(bgc,&r,&g,&b,&a);
	if(a)
	{
		float w = al_get_text_width(f, str)+1;
		float h = al_get_font_line_height(f)+1;
		float bgx = x;
		float bgy = y;
		switch(flag)
		{
			case ALLEGRO_ALIGN_LEFT:
				break;
			case ALLEGRO_ALIGN_CENTRE:
				bgx -= w/2;
				break;
			case ALLEGRO_ALIGN_RIGHT:
				bgx -= w;
				break;
		}
		al_draw_filled_rectangle(bgx,bgy,bgx+w,bgy+h,bgc);
	}
	al_draw_text(f,dis_c,x+1,y+1,flag,str);
	al_draw_text(f,tc,x,y,flag,str);
}

void jwin_draw_button_a5(int32_t x,int32_t y,int32_t w,int32_t h,int32_t state,int32_t type)
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
	
	jwin_draw_win_a5(x, y, w, h, frame);
}
void jwin_draw_text_button_a5(int32_t x, int32_t y, int32_t w, int32_t h, const char *str, int32_t flags, bool show_dotted_rect)
{
	int32_t g = (flags & D_SELECTED) ? 1 : 0;
	
	if(flags & D_SELECTED)
		jwin_draw_button_a5(x, y, w, h, 2, 0);
	else if(!(flags & D_GOTFOCUS))
		jwin_draw_button_a5(x, y, w, h, 0, 0);
	else
	{
		al_draw_rectangle(x, y, x+w-1, y+h-1, jwin_a5_pal(jcDARK), 1);
		jwin_draw_button_a5(x+1, y+1, w-2, h-2, 0, 0);
	}
	
	int th = al_get_font_line_height(a5font);
	int tlen;
	if(flags & D_DISABLED)
		tlen = gui_textout_ln_a5_dis(a5font,str,x+w/2+g, y+(h-th)/2+g,jwin_a5_pal(jcDISABLED_FG),AL5_INVIS,ALLEGRO_ALIGN_CENTRE,jwin_a5_pal(jcLIGHT));
	else
		tlen = gui_textout_ln_a5(a5font,str,x+w/2+g, y+(h-th)/2+g,jwin_a5_pal(jcBOXFG),AL5_INVIS,ALLEGRO_ALIGN_CENTRE);
	
	if(show_dotted_rect&&(flags & D_GOTFOCUS))
	{
		int doff = 6;
		while(doff && h < th+(doff*2))
			--doff;
		while(doff && w < tlen+10+(doff*2))
			--doff;
		if(doff)
			dotted_rect_a5(x+doff, y+doff, x+w-doff-1, y+h-doff-1, jwin_a5_pal(jcDARK), jwin_a5_pal(jcBOX));
	}
}
bool do_text_button_a5(int32_t x,int32_t y,int32_t w,int32_t h,const char *text)
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
                jwin_draw_text_button_a5(x, y, w, h, text, D_SELECTED, true);
                over=true;
                
				update_hw_screen();
            }
        }
        else
        {
            if(over)
            {
                vsync();
                jwin_draw_text_button_a5(x, y, w, h, text, 0, true);
                over=false;
                
				update_hw_screen();
            }
        }
    }
    
    return over;
}
bool do_text_button_reset_a5(int32_t x,int32_t y,int32_t w,int32_t h,const char *text)
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
				jwin_draw_text_button_a5(x, y, w, h, text, D_SELECTED, true);
				over=true;
				
				update_hw_screen();
			}
		}
		else
		{
			if(over)
			{
				vsync();
				jwin_draw_text_button_a5(x, y, w, h, text, 0, true);
				over=false;
				
				update_hw_screen();
			}
		}
		
	}
	
	if(over)
	{
		vsync();
		jwin_draw_text_button_a5(x, y, w, h, text, 0, true);
		
		update_hw_screen();
	}
	
	return over;
}

void draw_question_button_a5(int32_t x, int32_t y, int32_t state)
{
	ALLEGRO_COLOR c = jwin_a5_pal(jcBOXFG);
	
	jwin_draw_button_a5(x,y,16,14,state,0);
	x += 4 + (state?1:0);
	y += 4 + (state?1:0);
	
	al_draw_hline(x+2, y+0, x+5, c);
	al_draw_hline(x+1, y+1, x+2, c);
	al_draw_hline(x+5, y+1, x+6, c);
	al_draw_hline(x+4, y+2, x+5, c);
	al_draw_hline(x+3, y+3, x+4, c);
	al_draw_hline(x+3, y+4, x+4, c);
	al_draw_hline(x+3, y+6, x+4, c);
	al_draw_hline(x+3, y+7, x+4, c);
}

void draw_x_button_a5(int32_t x, int32_t y, int32_t state)
{
	ALLEGRO_COLOR c = jwin_a5_pal(jcBOXFG);
	
	jwin_draw_button_a5(x,y,16,14,state,0);
	x += 4 + (state?1:0);
	y += 3 + (state?1:0);
	
	al_draw_line(x,y,x+7,y+7,c,1);
	al_draw_line(x+1,y,x+8,y+7,c,1);
	al_draw_line(x,y+7,x+7,y,c,1);
	al_draw_line(x+1,y+7,x+8,y,c,1);
}
void draw_checkbox_a5(int32_t x,int32_t y,int32_t sz,bool value)
{
	draw_checkbox_a5(x,y,sz,sz,value);
}
void draw_checkbox_a5(int32_t x,int32_t y,int32_t wid,int32_t hei,bool value)
{
	jwin_draw_frame_a5(x, y, wid, hei, FR_DEEP);
	al_draw_filled_rectangle(x+2, y+2, x+wid-2, y+hei-2, jwin_a5_pal(jcTEXTBG));
	
	if(value)
		al_draw_x(x+2,y+2,x+wid-2,y+hei-2,jwin_a5_pal(jcTEXTFG),1);
}

void draw_arrow_a5(ALLEGRO_COLOR c, int32_t x, int32_t y, int32_t h, bool up, bool center)
{
	if(!center)
		x += h-1;
	for(int i = 0; i<h; i++)
	{
		al_draw_hline(x-(up?i:h-i-1), y+i, x+(up?i:h-i-1)+1, c);
	}
}
void draw_arrow_button_a5(int32_t x, int32_t y, int32_t w, int32_t h, int32_t up, int32_t state)
{
	ALLEGRO_COLOR c = jwin_a5_pal(jcDARK);
	int32_t ah = zc_min(h/3, 5);
	
	jwin_draw_button_a5(x,y,w,h,state,1);
	x += w/2 - (state?0:1);
	y += (h-ah)/2 + (state?1:0);
	draw_arrow_a5(c,x,y,ah,up,true);
}

int32_t jwin_do_x_button_dlg_a5(int32_t x, int32_t y)
{
	int32_t down=0, last_draw = 0;
	
	while(gui_mouse_b())
	{
		down = mouse_in_rect(x,y,16,14);
		
		if(down!=last_draw)
		{
			draw_x_button_a5(x,y,down);
			last_draw = down;
		}
		
		/* let other objects continue to animate */
		broadcast_dialog_message(MSG_IDLE, 0);
		rest(1);
	}
	
	if(down)
	{
		draw_x_button_a5(x,y,0);
	}
	
	return down;
}
bool jwin_do_x_button_a5(int32_t x, int32_t y)
{
    bool over=false;
    
    while(gui_mouse_b())
    {
        update_hw_screen();
        
        if(isinRect(gui_mouse_x(),gui_mouse_y(),x,y,x+15,y+13))
        {
            if(!over)
            {
                draw_x_button_a5(x, y, D_SELECTED);
                over=true;
            }
        }
        else
        {
            if(over)
            {
                draw_x_button_a5(x, y, 0);
                over=false;
            }
        }
    }
    return over;
}
bool jwin_do_question_button_a5(int32_t x, int32_t y)
{
    bool over=false;
    
    while(gui_mouse_b())
    {
        update_hw_screen();
        
        if(isinRect(gui_mouse_x(),gui_mouse_y(),x,y,x+15,y+13))
        {
            if(!over)
            {
                draw_question_button_a5(x, y, D_SELECTED);
                over=true;
            }
        }
        else
        {
            if(over)
            {
                draw_question_button_a5(x, y, 0);
                over=false;
            }
        }
    }
    return over;
}
bool do_checkbox_a5(int32_t x,int32_t y,int32_t sz,int32_t &value)
{
	return do_checkbox_a5(x,y,sz,sz,value);
}
bool do_checkbox_a5(int32_t x,int32_t y,int32_t wid,int32_t hei,int32_t &value)
{
	bool over=false;
	
	while(gui_mouse_b())
	{
		update_hw_screen();
		
		if(isinRect(gui_mouse_x(),gui_mouse_y(),x,y,x+wid-1,y+hei-1))               //if on checkbox
		{
			if(!over)                                             //if wasn't here before
			{
				value=!value;
				draw_checkbox_a5(x,y,wid,hei,value!=0);
				over=true;
			}
		}
		else                                                    //if not on checkbox
		{
			if(over)                                              //if was here before
			{
				value=!value;
				draw_checkbox_a5(x,y,wid,hei,value!=0);
				over=false;
			}
		}
	}
	
	return over;
}

void dither_rect_a5(int32_t x1, int32_t y1, int32_t x2, int32_t y2,
	ALLEGRO_COLOR c1, ALLEGRO_COLOR c2)
{
	if(x1>x2)
	{
		zc_swap(x1,x2);
	}
	if(y1>y2)
	{
		zc_swap(y1,y2);
	}
	
	int32_t r, g, b, direction=1;
	int32_t c;
	byte r1, r2, g1, g2, b1, b2;
	int w = x2-x1+1;
	int32_t temp;
	int32_t red_rand_strength=0, green_rand_strength=0, blue_rand_strength=0;
	
	lfsrInit();
	
	al_unmap_rgb(c1,&r1,&g1,&b1);
	al_unmap_rgb(c2,&r2,&g2,&b2);
	
	red_rand_strength=abs(r2-r1)/4;
	green_rand_strength=abs(g2-g1)/4;
	blue_rand_strength=abs(b2-b1)/4;
	byte mr, mg, mb;
	
	std::vector<ALLEGRO_COLOR> colors;
	for(int32_t i=0; i<w; i++)
	{
		r = mix_value(r1, r2, i, w-1);
		g = mix_value(g1, g2, i, w-1);
		b = mix_value(b1, b2, i, w-1);
		colors.push_back(al_map_rgb(r,g,b));
	}
	
	for(int32_t j=0; j<=y2-y1; ++j)
	{
		if(direction==1)
		{
			for(int32_t i=0; i<=x2-x1; ++i)
			{
				al_unmap_rgb(colors[i],&mr,&mg,&mb);
				mr=vbound(mr+lfsrNext()%(red_rand_strength*2+1)-(red_rand_strength*1),0,255);
				mg=vbound(mg+lfsrNext()%(green_rand_strength*2+1)-(green_rand_strength*1),0,255);
				mb=vbound(mb+lfsrNext()%(blue_rand_strength*2+1)-(blue_rand_strength*1),0,255);
				al_put_pixel(i+x1,j+y1,al_map_rgb(mr,mg,mb));
			}
			direction=-1;
		}
		else
		{
			for(int32_t i=x2-x1; i>=0; --i)
			{
				al_put_pixel(i+x1,j+y1,colors[i]);
			}
			direction=1;
		}
	}
	
	
}

void jwin_draw_titlebar_a5(int32_t x, int32_t y, int32_t w, int32_t h, const char *str, bool draw_button, bool helpbtn)
{
	char buf[512];
	int32_t len = (int32_t)strlen(str);
	int32_t length = al_get_text_width(a5font,str);
	int32_t height = al_get_font_line_height(a5font);
	
	int32_t tx = x + 2;
	int32_t ty = y + (h-height)/2;
	
	dither_rect_a5(x, y, x+w-1, y+h-1, jwin_a5_pal(jcTITLER), jwin_a5_pal(jcTITLEL));
	
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
			length = al_get_text_width(a5font,buf);
		}
	}
	
	jwin_textout_a5(a5font,jwin_a5_pal(jcTITLEFG),tx,ty,0,buf);
	
	if(draw_button)
	{
		draw_x_button_a5(x + w - 18, y+2, 0);
	}
	
	if(helpbtn)
	{
		draw_question_button_a5(x + w - (draw_button ? 36 : 18), y+2, 0);
	}
	
}

//Misc stolen from jwin.cpp

////Tabs

int32_t discern_tab_a5(GUI::TabPanel *panel, int32_t first_tab, int32_t x)
{
	int32_t w=0;
	
	for(size_t i=first_tab; i < panel->getSize(); i++)
	{
		w+=al_get_text_width(a5font, panel->getName(i))+15;
		
		if(w>x)
		{
			return i;
		}
	}
	
	return -1;
}
int32_t tabs_width_a5(GUI::TabPanel *panel)
{
	int32_t w=0;
	
	for(size_t i=0; i < panel->getSize(); ++i)
	{
		w+=al_get_text_width(a5font, panel->getName(i))+15;
	}
	
	return w+1;
}
bool uses_tab_arrows_a5(GUI::TabPanel *panel, int32_t maxwidth)
{
	return (tabs_width_a5(panel)>maxwidth);
}
size_t last_visible_tab_a5(GUI::TabPanel *panel, int32_t first_tab, int32_t maxwidth)
{
	size_t i;
	int32_t w=0;
	
	if(uses_tab_arrows_a5(panel, maxwidth))
	{
		maxwidth-=28;
	}
	
	for(i=first_tab; i < panel->getSize(); ++i)
	{
		w+=al_get_text_width(a5font, panel->getName(i))+15;
		
		if(w>maxwidth)
		{
			return i-1;
		}
	}
	
	return i-1;
}
int32_t displayed_tabs_width_a5_a5(GUI::TabPanel *panel, int32_t first_tab, int32_t maxwidth)
{
	size_t i=0;
	int32_t w=0;
	
	for(i=first_tab; i<=last_visible_tab_a5(panel, first_tab, maxwidth); ++i)
	{
		w+=al_get_text_width(a5font, panel->getName(i))+15;
	}
	
	return w+1;
}

////Scrollers

void _handle_jwin_scrollable_scroll_click_a5(DIALOG *d, int32_t listsize, int32_t *offset, ALLEGRO_FONT *fnt)
{
	enum { top_btn, bottom_btn, bar, top_bar, bottom_bar };
	
	int32_t xx, yy;
	int32_t height = (d->h-3) / (fnt ? al_get_font_line_height(fnt) : 1);
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
				draw_arrow_button_a5(xx, yy, 16, bh, obj==top_btn, down*3);
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
					
				if(yy != *offset)
				{
					*offset = yy;
					d->proc(MSG_DRAW, d, 0);
				}
				
				// let other objects continue to animate
				broadcast_dialog_message(MSG_IDLE, 0);
				
				update_hw_screen();
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
		draw_arrow_button_a5(xx, yy, 16, bh, obj==top_btn, 0);
	}
}
void _handle_jwin_scrollable_scroll_a5(DIALOG *d, int32_t listsize, int32_t *index, int32_t *offset, ALLEGRO_FONT *fnt)
{
    int32_t height = (d->h-3) / (fnt ? al_get_font_line_height(fnt) : 1);
    
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
bool _handle_jwin_listbox_click_a5(DIALOG *d)
{
    ListData *data = (ListData *)d->dp;
    char *sel = (char *)d->dp2;
    int32_t listsize, height;
    int32_t i, j;

    data->listFunc(-1, &listsize);

    if(!listsize)
        return false;
        
    height = (d->h-3) / al_get_font_line_height(*data->a5font);
    
    i = MID(0, ((gui_mouse_y() - d->y - 4) / al_get_font_line_height(*data->a5font)),
            ((d->h-3) / al_get_font_line_height(*data->a5font) - 1));
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
		
        _handle_jwin_scrollable_scroll_a5(d, listsize, &d->d1, &d->d2, *data->a5font);
        
        object_message(d, MSG_DRAW, 0);
		update_hw_screen(true);
        return true;
    }
	return false;
}
void _jwin_draw_scrollable_frame_a5(DIALOG *d, int32_t listsize, int32_t offset, int32_t height, int32_t type)
{
	int32_t pos, len;
	int32_t xx, yy, hh, bh;
	
	/* draw frame */
	if(type) // for droplists
		jwin_draw_frame_a5(d->x, d->y, d->w, d->h, FR_DARK);
	else
		jwin_draw_frame_a5(d->x, d->y, d->w, d->h, FR_DEEP);
		
	/* possibly draw scrollbar */
	if(listsize > height)
	{
		_calc_scroll_bar(d->h, height, listsize, offset, &bh, &len, &pos);
		
		xx = d->x + d->w - 18;
		
		draw_arrow_button_a5(xx, d->y+2, 16, bh, 1, 0);
		draw_arrow_button_a5(xx, d->y+d->h-2-bh, 16, bh, 0, 0);
		
		if(d->h > 32)
		{
			yy = d->y + 16;
			hh = (d->h-32);
			
			al_draw_filled_rectangle(xx, yy, xx+16, yy+hh, jwin_a5_pal(jcLIGHT));
			
			if(d->h > 32+6)
			{
				jwin_draw_button_a5(xx, yy+pos, 16, len, 0, 1);
			}
		}
		
		if(d->flags & D_GOTFOCUS)
			dotted_rect_a5(d->x+2, d->y+2, d->x+d->w-19, d->y+d->h-3, jwin_a5_pal(jcTEXTFG), jwin_a5_pal(jcTEXTBG));
	}
	else if(d->flags & D_GOTFOCUS)
		dotted_rect_a5(d->x+2, d->y+2, d->x+d->w-3, d->y+d->h-3, jwin_a5_pal(jcTEXTFG), jwin_a5_pal(jcTEXTBG));
}

void _jwin_draw_listbox_a5(DIALOG *d,bool abc)
{
	int32_t height, listsize, i, len, bar, x, y, w;
	ALLEGRO_COLOR fg, bg;
	char *sel = (char*)d->dp2;
	char s[1024] = {0};
	ListData *data = (ListData *)d->dp;
	
	ALLEGRO_FONT* oldfont = a5font;
	a5font = *data->a5font;
	int fh = al_get_font_line_height(a5font);

	data->listFunc(-1, &listsize);
	height = (d->h-3) / fh;
	bar = (listsize > height);
	w = (bar ? d->w-21 : d->w-5);
	ALLEGRO_COLOR fg_color = jwin_a5_pal((d->flags & D_DISABLED) ? jcDISABLED_FG : jcTEXTFG);
	ALLEGRO_COLOR bg_color = jwin_a5_pal((d->flags & D_DISABLED) ? jcDISABLED_BG : jcTEXTBG);
	
	al_draw_filled_rectangle(d->x,  d->y, d->x+d->w, d->y+d->h+(abc?fh:0), bg_color);
	
	/* draw frame, maybe with scrollbar */
	_jwin_draw_scrollable_frame_a5(d, listsize, d->d2, height, (d->flags&D_USER)?1:0);
	
	al_draw_filled_rectangle(d->x+2,  d->y+2, d->x+w+3, d->y+4, bg_color);
	al_draw_vline(d->x+2, d->y+4, d->y+d->h-2, bg_color);
	al_draw_vline(d->x+3, d->y+4, d->y+d->h-2, bg_color);
	al_draw_vline(d->x+w+1, d->y+4, d->y+d->h-2, bg_color);
	al_draw_vline(d->x+w+2, d->y+4, d->y+d->h-2, bg_color);
	
	/* draw box contents */
	for(i=0; i<height; i++)
	{
		if(d->d2+i < listsize)
		{
			if(d->d2+i == d->d1 && !(d->flags & D_DISABLED))
			{
				fg = jwin_a5_pal(jcSELFG);
				bg = jwin_a5_pal(jcSELBG);
			}
			else if((sel) && (sel[d->d2+i]))
			{
				fg = jwin_a5_pal(jcMEDDARK);
				bg = jwin_a5_pal(jcSELBG);
			}
			else
			{
				fg = fg_color;
				bg = bg_color;
			}

			strncpy(s, data->listFunc(i+d->d2, NULL), 1023);
			x = d->x + 4;
			y = d->y + 4 + i*fh;
			
			al_draw_filled_rectangle(x, y, x+8, y+fh, bg);
			x += 8;
			len = (int32_t)strlen(s);
			
			while(len > 0 && al_get_text_width(a5font, s) >= d->w - (bar ? 26 : 10))
			{
				len--;
				s[len] = 0;
			}
			
			jwin_textout_a5(a5font, fg, x, y, 0, s, bg);
			x += al_get_text_width(a5font, s);
			
			if(x <= d->x+w)
				al_draw_filled_rectangle(x, y, d->x+w+1, y+fh, bg);
		}
		else
			al_draw_filled_rectangle(d->x+2, d->y+4+i*fh,
					 d->x+w+3, d->y+4+(i+1)*fh, bg_color);
	}
	
	if(d->y+4+i*fh <= d->y+d->h-3)
		al_draw_filled_rectangle(d->x+2, d->y+4+i*fh,
				 d->x+w+3, d->y+d->h-2, bg_color);
	
	if(abc)
	{
		fg = jwin_a5_pal(jcSELFG);
		bg = jwin_a5_pal(jcSELBG);
		al_draw_filled_rectangle(d->x+1,  d->y+d->h+2, d->x+d->w-1, d->y+d->h+2+fh, bg);
		strncpy(s, abc_keypresses, 1023);
		char* s2 = s;
		int32_t tw = (d->w-1);
		while(al_get_text_width(a5font, s2) >= tw)
			++s2;
		jwin_textout_a5(a5font, fg, d->x+1, d->y+d->h+2, 0, s2, bg);
	}
	
	a5font = oldfont;
}

////Texts

int32_t gui_textout_ln_a5(ALLEGRO_FONT *f, const char *s, int32_t x, int32_t y, ALLEGRO_COLOR color, ALLEGRO_COLOR bg, int32_t pos)
{
	char tmp[1024];
	int32_t c = 0;
	int32_t len;
	int32_t pix_len = 0;
	int32_t max_len = 0;
	int32_t hline_pos;
	int32_t xx = x;
	
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
		pix_len = al_get_text_width(f, tmp);
		if (pix_len > max_len) max_len = pix_len;
		x = xx;
		
		jwin_textout_a5(f, color, x, y, pos, tmp, bg);
		
		if(hline_pos >= 0)
		{
			if(pos==1) //center
				x -= pix_len / 2;
			else if(pos==2) //right
				x -= pix_len;
			int32_t i = tmp[hline_pos];
			tmp[hline_pos] = 0;
			hline_pos = al_get_text_width(f, tmp);
			tmp[0] = i;
			tmp[1] = 0;
			i = al_get_text_width(f, tmp);
			al_draw_hline(x+hline_pos, y+al_get_font_line_height(f)-gui_font_baseline, x+hline_pos+i, color);
		}
		
		y += al_get_font_line_height(f);
	}
	
	return max_len;
}

int32_t gui_textout_ln_a5_dis(ALLEGRO_FONT *f, const char *s, int32_t x, int32_t y, ALLEGRO_COLOR color, ALLEGRO_COLOR bg, int32_t pos, ALLEGRO_COLOR dis_c)
{
	char tmp[1024];
	int32_t c = 0;
	int32_t len;
	int32_t pix_len = 0;
	int32_t max_len = 0;
	int32_t hline_pos;
	int32_t xx = x;
	
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
		pix_len = al_get_text_width(f, tmp);
		if (pix_len > max_len) max_len = pix_len;
		x = xx;
		
		jwin_textout_a5_dis(f, color, x, y, pos, tmp, bg, dis_c);
		
		if(hline_pos >= 0)
		{
			int32_t i;
			i = tmp[hline_pos];
			tmp[hline_pos] = 0;
			hline_pos = al_get_text_width(f, tmp);
			tmp[0] = i;
			tmp[1] = 0;
			i = al_get_text_width(f, tmp);
			al_draw_hline(x+hline_pos+1, y+al_get_font_line_height(f)-gui_font_baseline+1, x+hline_pos+i+1, dis_c);
			al_draw_hline(x+hline_pos, y+al_get_font_line_height(f)-gui_font_baseline, x+hline_pos+i, color);
		}
		
		y += al_get_font_line_height(f);
	}
	
	return max_len;
}

int32_t gui_text_width_a5(ALLEGRO_FONT *f, const char *s)
{
	char tmp[1024];
	int32_t c = 0;
	int32_t len;
	int32_t pix_len = 0;
	int32_t max_len = 0;
	int32_t hline_pos;
	
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
		pix_len = al_get_text_width(f, tmp);
		if (pix_len > max_len) max_len = pix_len;
	}
	
	return max_len;
}

int32_t gui_text_height_a5(ALLEGRO_FONT* f, const char *s)
{
	return al_get_font_line_height(f) * (count_newline((uint8_t*)s) + 1);
}

int32_t al_char_length(ALLEGRO_FONT* f, char c)
{
	char buf[2] = {c,0};
	return al_get_text_width(f,buf);
}

//Special full dialogs

////SelColor
const char* rowpref(int32_t row)
{
	static const char *lcol = "Level Colors", *bosscol = "Boss Colors", *thmcol = "Theme Colors", *nlcol="";
	switch(row)
	{
		case 1:
			return get_bit(quest_rules,qr_CSET1_LEVEL) ? lcol : nlcol;
		case 5:
			return get_bit(quest_rules,qr_CSET5_LEVEL) ? lcol : nlcol;
		case 7:
			return get_bit(quest_rules,qr_CSET7_LEVEL) ? lcol : nlcol;
		case 8:
			return get_bit(quest_rules,qr_CSET8_LEVEL) ? lcol : nlcol;
		case 2: case 3: case 4: case 9:
			return lcol;
		case 14:
			return bosscol;
		case 15:
			return thmcol;
		default:
			return nlcol;
	}
}
ALLEGRO_COLOR zc_pal(int c)
{
	switch(c)
	{
		case BLACK:
			return AL5_BLACK;
		case WHITE:
			return AL5_WHITE;
	}
	return a5color(c);
}
int32_t jwin_selcolor_proc_a5(int32_t msg, DIALOG *d, int32_t c)
{
	int32_t ret = D_O_K;
	if(!d->d2) d->d2 = 12;
	int32_t numcsets = d->d2;
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
			jwin_draw_frame_a5(d->x-2, d->y-2, d->w+4, d->h+4, FR_ETCHED);
			for(int32_t c = 0; c < numcol; ++c)
			{
				int32_t x = (c%16)*csz, y = (c/16)*csz;
				ALLEGRO_COLOR col = zc_pal(c);
				al_draw_filled_rectangle(d->x+x, d->y+y, d->x+x+csz, d->y+y+csz, col);
				if(c == d->d1)
				{
					ALLEGRO_COLOR highlightColor = getHighlightColor(col);
					al_draw_rectangle(d->x+x, d->y+y, d->x+x+csz-1, d->y+y+csz-1, highlightColor, 2);
				}
			}
			
            ALLEGRO_FONT *oldfont = a5font;
            
            if(d->dp2)
            {
                a5font = (ALLEGRO_FONT*)d->dp2;
            }
			
			char buf[32]={0};
			for(int32_t col = 0; col < 16; ++col)
			{
				sprintf(buf, "%X", col);
				gui_textout_ln_a5(a5font, buf, d->x + (csz*col) + (csz/2), d->y-3-al_get_font_line_height(a5font), jwin_a5_pal(jcBOXFG), jwin_a5_pal(jcBOX), 1);
			}
			for(int32_t row = 0; row < numcsets; ++row)
			{
				sprintf(buf, "%s 0x%02X", rowpref(row), row*16);
				gui_textout_ln_a5(a5font, buf, d->x-3, d->y + (csz*row) + (csz-al_get_font_line_height(a5font))/2, jwin_a5_pal(jcBOXFG), jwin_a5_pal(jcBOX), 2);
			}
			
            a5font = oldfont;
			break;
		}
		
		case MSG_CLICK:
		{
			if(mouse_in_rect(d->x, d->y, d->x+d->w-1, d->y+d->h-1))
			{
				int32_t col = ((gui_mouse_x() - d->x) / csz) + 16*((gui_mouse_y() - d->y) / csz);
				
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
	{ jwin_win_proc_a5,       0,    0,  306,  63+16*8,       vc(14),      vc(1),    0,    D_EXIT,         0,    0,    (void *)"Select Color",  NULL,  NULL },
	{ jwin_button_proc_a5,   75,  40+16*8,   61,   21,       vc(14),      vc(1),    0,    D_EXIT,         0,    0,    (void *)"OK",  NULL,  NULL },
	{ jwin_button_proc_a5,  164,  40+16*8,   61,   21,       vc(14),      vc(1),    0,    D_EXIT,         0,    0,    (void *)"Cancel",  NULL,  NULL },
	{ jwin_selcolor_proc_a5, 156-64,   34,   16*8,   16*8,            0,          0,    0,         0,         0,    0,    NULL,  NULL,  NULL },
	
	{ NULL,              0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL,  NULL,  NULL }
};
int32_t jwin_color_swatch_a5(int32_t msg, DIALOG *d, int32_t c)
{
	int32_t ret = D_O_K;
	
	switch(msg)
	{
		case MSG_START:
		{
			if(d->d2 < 1) d->d2 = 12;
			else if(d->d2 > 16) d->d2 = 16;
			break;
		}
		
		case MSG_DRAW:
		{
			if(!d->d1 || (d->flags&D_DISABLED))
			{
				al_draw_filled_rectangle(d->x, d->y, d->x+d->w, d->y+d->h,
					(d->flags&D_DISABLED) ? jwin_a5_pal(jcDISABLED_BG) : AL5_BLACK);
				al_draw_x(d->x, d->y, d->x+d->w, d->y+d->h, AL5_WHITE,1);
				jwin_draw_frame_a5(d->x-2, d->y-2, d->w+4, d->h+4, FR_DEEP);
			}
			else
			{
				al_draw_filled_rectangle(d->x, d->y, d->x+d->w, d->y+d->h, zc_pal(d->d1));
				jwin_draw_frame_a5(d->x-2, d->y-2, d->w+4, d->h+4, FR_ETCHED);
			}
			break;
		}
		
		case MSG_CLICK:
		{
			if(d->flags&(D_READONLY|D_DISABLED)) break;
			selcolor_dlg[3].bg = scheme[jcBOXFG];
			selcolor_dlg[3].fg = scheme[jcBOX];
			selcolor_dlg[3].d1 = d->d1;
			selcolor_dlg[3].d2 = d->d2;
			selcolor_dlg[3].dp2 = get_zc_font_a5(font_lfont_l);
			large_dialog(selcolor_dlg);
			jwin_ulalign_dialog(selcolor_dlg);
			
			while(gui_mouse_b()); //wait for mouseup
			
			//!TODO Allow loading different level palettes, sprite palettes, etc via buttons
			
			int dx, dy, dw, dh;
			jwin_get_dlg_center(selcolor_dlg, dx, dy, dw, dh);
			popup_zqdialog_start_a5(dx,dy,dw,dh);
			int32_t val = new_popup_dlg(selcolor_dlg, 3);
			popup_zqdialog_end_a5();
			ret = D_REDRAW;
			
			if(val == 1 || val == 3)
			{
				d->d1 = selcolor_dlg[3].d1;
				GUI_EVENT(d, geCHANGE_VALUE);
				ret |= D_REDRAWME;
			}
			break;
		}
	}
	return ret;
}

//JWin A5 procs

int32_t jwin_win_proc_a5(int32_t msg, DIALOG *d, int32_t)
{
	rest(1);
	static bool skipredraw = false;
	if(!d->dp2)
		d->dp2 = get_custom_font_a5(CFONT_TITLE);
	
	switch(msg)
	{
		case MSG_DRAW:
			if(skipredraw)
			{
				skipredraw = false;
				break;
			}
			
			if(a4_bmp_active())
				rectfill(screen, d->x, d->y, d->x+d->w-1, d->y+d->h-1, get_zqdialog_a4_clear_color()); //!TODO Remove when a5 dialog done - Clear a4 screen layer
			
			jwin_draw_win_a5(d->x, d->y, d->w, d->h, FR_WIN);
			
			if(d->dp)
			{
				ALLEGRO_FONT *oldfont = a5font;
				
				if(d->dp2)
				{
					a5font = (ALLEGRO_FONT*)d->dp2;
				}
				
				jwin_draw_titlebar_a5(d->x+3, d->y+3, d->w-6, 18, (char*)d->dp, d->flags & D_EXIT, d->dp3!=NULL);
				a5font = oldfont;
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
				if(jwin_do_x_button_dlg_a5(d->x+d->w-21, d->y+5))
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

int32_t jwin_tab_proc_a5(int32_t msg, DIALOG *d, int32_t c)
{
	ASSERT(d);
	assert(d->flags&D_NEW_GUI);
	if(d->dp==NULL) return D_O_K;
	
	int32_t tx;
	int32_t ret = D_O_K;
	int32_t sd=2; //selected delta
	static bool skipredraw = false;
	GUI::TabPanel *panel=(GUI::TabPanel*)d->dp;
	
	ALLEGRO_FONT *oldfont = a5font;
	if(d->dp2)
	{
		a5font = (ALLEGRO_FONT*)d->dp2;
	}
	int th = al_get_font_line_height(a5font);
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
				if(a4_bmp_active())
					rectfill(screen, d->x, d->y, d->x+d->w-1, d->y+d->h-1, get_zqdialog_a4_clear_color()); //!TODO Remove when a5 dialog done - Clear a4 screen layer
				al_draw_filled_rectangle(d->x-2, d->y-2, d->x+d->w, d->y+d->h, jwin_a5_pal(jcBOX));
				al_draw_vline(d->x, d->y+sd+7+th, d->y+sd+d->h-1, jwin_a5_pal(jcLIGHT));
				al_draw_vline(d->x+1, d->y+sd+7+th, d->y+sd+d->h-2, jwin_a5_pal(jcMEDLT));
				al_draw_vline(d->x+d->w-2, d->y+sd+7+th, d->y+sd+d->h-1, jwin_a5_pal(jcMEDDARK));
				al_draw_vline(d->x+d->w-1, d->y+sd+7+th-1, d->y+sd+d->h, jwin_a5_pal(jcDARK));
				al_draw_hline(d->x+1, d->y+sd+d->h-2, d->x+d->w-2, jwin_a5_pal(jcMEDDARK));
				al_draw_hline(d->x, d->y+sd+d->h-1, d->x+d->w-1, jwin_a5_pal(jcDARK));
				tx=d->x;
				
				if(d->dp)
				{
					if(panel->getCurrentIndex() != d->d1)
					{
						al_draw_hline(tx+1, d->y+sd+6+th+1, tx+3, jwin_a5_pal(jcMEDLT));
						al_draw_hline(tx, d->y+sd+6+th, tx+2, jwin_a5_pal(jcLIGHT));
					}
					
					tx+=2;
					
					for(size_t i=d->d1; i < panel->getSize()&&i<=last_visible_tab_a5(panel,d->d1,d->w); ++i)
					{
						sd=(i==panel->getCurrentIndex())?0:2;
						
						if((i==d->d1) || (i-1 != panel->getCurrentIndex()))
						{
							al_draw_vline(tx-(2-sd), d->y+sd+2, d->y+8+th+1, jwin_a5_pal(jcLIGHT));
							al_draw_vline(tx-(2-sd)+1, d->y+sd+2, d->y+8+th+1, jwin_a5_pal(jcMEDLT));
							al_put_pixel(tx+1-(2-sd), d->y+sd+1, jwin_a5_pal(jcLIGHT));
						}
						
						int tw = al_get_text_width(a5font,panel->getName(i));
						al_draw_hline(tx+2-(2-sd), d->y+sd, tx+12+(2-sd)+tw+1, jwin_a5_pal(jcLIGHT));
						al_draw_hline(tx+2-(2-sd), d->y+sd+1, tx+12+(2-sd)+tw+1, jwin_a5_pal(jcMEDLT));
						
						if(i!=panel->getCurrentIndex())
						{
							al_draw_hline(tx+1, d->y+sd+6+th, tx+13+tw+2, jwin_a5_pal(jcLIGHT));
							al_draw_hline(tx, d->y+sd+6+th+1, tx+13+tw+2, jwin_a5_pal(jcMEDLT));
						}
						else if(d->flags & D_GOTFOCUS)
						{
							al_draw_hline(tx+1, d->y+sd+6+th, tx+13+tw+2, jwin_a5_pal(jcBOXFG));
							al_draw_hline(tx, d->y+sd+6+th+1, tx+13+tw+2, jwin_a5_pal(jcBOXFG));
						}
						
						tx+=4;
						const char* pname = (const char*)(panel->getName(i));
						bool dis = panel->getDisabled(i);
						if(dis)
							jwin_textout_a5_dis(a5font, jwin_a5_pal(jcDISABLED_FG), tx+4, d->y+sd+4, 0, pname, jwin_a5_pal(jcDISABLED_BG), jwin_a5_pal(jcLIGHT));
						else jwin_textout_a5(a5font, jwin_a5_pal(jcBOXFG), tx+4, d->y+sd+4, 0, pname, jwin_a5_pal(jcBOX));
						tx+=al_get_text_width(a5font, pname)+10;
						
						if((i+1>=panel->getSize()) || (i+1!=panel->getCurrentIndex()))
						{
							al_put_pixel(tx-1+(2-sd), d->y+sd+1, jwin_a5_pal(jcDARK));
							al_draw_vline(tx+(2-sd), d->y+sd+2, d->y+sd+5+th, jwin_a5_pal(jcDARK));
							al_draw_vline(tx+(2-sd)-1, d->y+sd+2, d->y+sd+5+th, jwin_a5_pal(jcMEDDARK));
						}
						
						tx++;
					}
					
					if(d->d1!=0||last_visible_tab_a5(panel,d->d1,d->w)+1<panel->getSize())
					{
						jwin_draw_text_button_a5(d->x+d->w-14,d->y+2, 14, 14, "\x8B", 0, true);
						jwin_draw_text_button_a5(d->x+d->w-28,d->y+2, 14, 14, "\x8A", 0, true);
					}
				}
				
				if((tx+(2-sd))<(d->x+d->w))
				{
					al_draw_hline(tx+(2-sd)-1, d->y+8+th, d->x+d->w, jwin_a5_pal(jcLIGHT));
					al_draw_hline(tx+(2-sd)-2, d->y+8+th+1, d->x+d->w-1, jwin_a5_pal(jcMEDLT));
				}
			}
		}
		break;
		
		case MSG_WANTFOCUS:
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
			if(uses_tab_arrows_a5(panel, d->w)&&(mouse_in_rect(d->x+d->w-28, d->y+2, 28, 14)))
			{
				if(mouse_in_rect(d->x+d->w-28, d->y+2, 14, 14))
				{
					if(do_text_button_reset_a5(d->x+d->w-28, d->y+2, 14, 14, "\x8A"))
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
					if(do_text_button_reset_a5(d->x+d->w-14, d->y+2, 14, 14, "\x8B"))
					{
						size_t t = last_visible_tab_a5(panel, d->d1, d->w);
						if(t<(panel->getSize()-1))
						{
							while(t==last_visible_tab_a5(panel, d->d1, d->w))
								++d->d1;
						}
						
						ret |= D_REDRAW;
					}
				}
			}
			else if(is_in_rect(gui_mouse_x(),gui_mouse_y(), d->x+2, d->y+2, d->x+displayed_tabs_width_a5_a5(panel,((d->d1&0xFF00)>>8),d->w), d->y+th+9))
			{
				// find out what the new tab (tb) will be (where the mouse is)
				int32_t newtab = discern_tab_a5(panel, d->d1, gui_mouse_x()-d->x-2);
				if(newtab > -1 && newtab != panel->getCurrentIndex() && !panel->getDisabled(newtab))
				{
					panel->switchTo(newtab);
					GUI_EVENT(d, geCHANGE_SELECTION);
				}
			}
		}
		break;
	}
	a5font = oldfont;
	return ret;
}

int32_t _handle_text_proc_a5(int32_t msg, DIALOG* d, int32_t align)
{
	ASSERT(d);
	switch(msg)
	{
		case MSG_START:
		{
			ALLEGRO_FONT *oldfont = a5font;
			
			if(d->dp2)
			{
				a5font = (ALLEGRO_FONT*)d->dp2;
			}
			
			const char* str = (const char*)d->dp;
			d->w = gui_text_width_a5(a5font,str);
			d->h = gui_text_height_a5(a5font,str);
			
			a5font = oldfont;
			break;
		}
		case MSG_DRAW:
		{
			ALLEGRO_FONT *oldfont = a5font;
			
			if(d->dp2)
			{
				a5font = (ALLEGRO_FONT*)d->dp2;
			}
			
			const char* str = (const char*)d->dp;
			if(d->flags & D_DISABLED)
				d->w = gui_textout_ln_a5_dis(a5font, str, d->x, d->y, jwin_a5_pal(jcDISABLED_FG), jwin_a5_pal(jcDISABLED_BG), align, jwin_a5_pal(jcLIGHT));
			else
				d->w = gui_textout_ln_a5(a5font, str, d->x, d->y, jwin_a5_pal(jcBOXFG), jwin_a5_pal(jcBOX), align);
			
			a5font = oldfont;
			break;
		}
	}
	
	return D_O_K;
}
int32_t jwin_text_proc_a5(int32_t msg, DIALOG *d, int32_t)
{
	return _handle_text_proc_a5(msg,d,0);
}
int32_t jwin_ctext_proc_a5(int32_t msg, DIALOG *d, int32_t)
{
	return _handle_text_proc_a5(msg,d,1);
}
int32_t jwin_rtext_proc_a5(int32_t msg, DIALOG *d, int32_t)
{
	return _handle_text_proc_a5(msg,d,2);
}

void apply_current_transform(int& x, int& y)
{
	const ALLEGRO_TRANSFORM* tr = al_get_current_transform();
	if(tr)
	{
		float fx=x,fy=y;
		al_transform_coordinates(tr,&fx,&fy);
		x=(int)fx;
		y=(int)fy;
	}
}

bool jwin_cliprect(int x, int y, int w, int h)
{
	apply_current_transform(x,y);
	collide_clip_rect(x,y,w,h);
	if(!w) return false; //clipped out
	al_set_clipping_rectangle(x,y,w,h);
	return true;
}

int32_t new_text_proc_a5(int32_t msg, DIALOG *d, int32_t)
{
	int ocx,ocy,ocw,och;
	if(msg==MSG_DRAW)
	{
		if(d->flags & D_HIDDEN) return D_O_K;
		al_get_clipping_rectangle(&ocx,&ocy,&ocw,&och);
		if(!jwin_cliprect(d->x,d->y,d->w,d->h))
			return D_O_K; //clipped out
	}
	int32_t ret = D_O_K;
	int32_t w = d->w, h = d->h, x = d->x, y = d->y;
	if(d->d2) no_hline = true;
	switch(d->d1)
	{
		case 1:
			d->x += d->w/2;
			break;
		case 2:
			d->x += d->w - 1;
			break;
	}
	ret = _handle_text_proc_a5(msg,d,d->d1);
	no_hline = false;
	d->w = w;
	d->h = h;
	d->x = x;
	d->y = y;
	if(msg==MSG_DRAW)
	{
		al_set_clipping_rectangle(ocx,ocy,ocw,och);
	}
	if(msg==MSG_WANTFOCUS && gui_mouse_b())
		ret |= D_WANTFOCUS|D_REDRAW;
	return ret;
}

int32_t jwin_button_proc_a5(int32_t msg, DIALOG *d, int32_t)
{
    int32_t down=0;
    int32_t selected=(d->flags&D_SELECTED)?1:0;
    int32_t last_draw;
    
	int ret = D_O_K;
    switch(msg)
    {
		case MSG_DRAW:
		{
			ALLEGRO_FONT *oldfont = a5font;
			
			if(d->dp2)
			{
				a5font = (ALLEGRO_FONT*)d->dp2;
			}
			
			jwin_draw_text_button_a5(d->x, d->y, d->w, d->h, (const char*)d->dp, d->flags, true);
			a5font = oldfont;
		}
		break;
		
		case MSG_WANTFOCUS:
			ret = D_WANTFOCUS;
			break;
			
		case MSG_KEY:
			/* close dialog? */
			if(d->flags & D_EXIT)
			{
				ret = D_CLOSE;
				break;
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
					{
						ret = D_CLOSE;
						break;
					}
				}
			}
			else
			{
				last_draw = -1;
				
				/* track the mouse until it is released */
				while(gui_mouse_b())
				{
					down = mouse_in_rect(d->x, d->y, d->w, d->h)?1:0;
					
					/* redraw? */
					if(last_draw != down)
					{
						if(down != selected)
							d->flags |= D_SELECTED;
						else
							d->flags &= ~D_SELECTED;
							
						object_message(d, MSG_DRAW, 0);
						update_hw_screen(true);
						last_draw = down;
					}
					
					/* let other objects continue to animate */
					broadcast_dialog_message(MSG_IDLE, 0);
					
					update_hw_screen();
				}
				
				/* redraw in normal state */
				if(down)
				{
					if(d->flags&D_EXIT)
					{
						d->flags &= ~D_SELECTED;
						object_message(d, MSG_DRAW, 0);
					}
					GUI_EVENT(d, geCLICK);
				}
				
				/* should we close the dialog? */
				if(down && (d->flags & D_EXIT))
					ret = D_CLOSE;
				ret |= D_REDRAW;
			}
		}
		break;
	}
	return ret;
}

int32_t d_jwinbutton_proc_a5(int32_t msg, DIALOG *d, int32_t)
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
			int discol = -1;
			if(d->flags & D_DISABLED)
			{
				state1 = jcDISABLED_BG;
				state2 = jcDISABLED_FG;
				discol = jcLIGHT;
			}
			else
			{
				state1 = jcTEXTBG;
				state2 = jcTEXTFG;
			}
			g = 0;
			if(d->flags & D_SELECTED)
			{
				g = 1;
				zc_swap(state1,state2);
			}
			ALLEGRO_COLOR s1 = jwin_a5_pal(state1), s2 = jwin_a5_pal(state2);
			
			int fh = al_get_font_line_height(a5font);
			
			al_draw_filled_rectangle(d->x+1+g, d->y+1+g, d->x+d->w-2+g, d->y+d->h-2+g, s2);
			al_draw_rectangle(d->x+g, d->y+g, d->x+d->w-1+g, d->y+d->h-1+g, s1, 0);
			if(discol > -1)
				gui_textout_ln_a5_dis(a5font, (char *)d->dp, d->x+d->w/2+g, d->y+d->h/2-fh/2+g, s1, AL5_INVIS, 1, jwin_a5_pal(discol));
			else gui_textout_ln_a5(a5font, (char *)d->dp, d->x+d->w/2+g, d->y+d->h/2-fh/2+g, s1, AL5_INVIS, 1);
			
			if(d->flags & D_SELECTED)
			{
				al_draw_vline(d->x, d->y, d->y+d->h-1, jwin_a5_pal(jcTEXTBG));
				al_draw_hline(d->x, d->y, d->x+d->w-1, jwin_a5_pal(jcTEXTBG));
			}
			else
			{
				al_draw_vline(d->x+d->w-1, d->y+1, d->y+d->h-1, AL5_BLACK);
				al_draw_hline(d->x+1, d->y+d->h-1, d->x+d->w, AL5_BLACK);
			}
			
			if((d->flags & D_GOTFOCUS) &&
					(!(d->flags & D_SELECTED) || !(d->flags & D_EXIT)))
				dotted_rect_a5(d->x+1+g, d->y+1+g, d->x+d->w-2+g, d->y+d->h-2+g, s1, s2);
				
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
				if(((state1) && (!state2)) || ((state2) && (!state1)))
				{
					d->flags ^= D_SELECTED;
					GUI_EVENT(d, geTOGGLE);
					state1 = d->flags & D_SELECTED;
					object_message(d, MSG_DRAW, 0);
				}
				
				/* let other objects continue to animate */
				broadcast_dialog_message(MSG_IDLE, 0);
				
				update_hw_screen();
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
int32_t new_check_proc_a5(int32_t msg, DIALOG *d, int32_t)
{
	int ocx,ocy,ocw,och;
	int32_t bx=0, tl=0;
	ASSERT(d);
	const char* str = (const char*)d->dp;
	if(str && !str[0])
		str = nullptr;
	
	int32_t tx = d->x-2, ty = d->y-2, tw = d->w+4, th = d->h+4, tx2 = 0;
	if(msg==MSG_DRAW)
	{
		if(d->flags & D_HIDDEN) return D_O_K;
		al_get_clipping_rectangle(&ocx,&ocy,&ocw,&och);
		if(!jwin_cliprect(tx,ty,tw,th))
			return D_O_K; //clipped out
	}
	
    ALLEGRO_FONT *oldfont = a5font;
    
    if(d->dp2)
    {
        a5font = (ALLEGRO_FONT *)d->dp2;
    }
	int fh = al_get_font_line_height(a5font);
	switch(msg)
	{
		case MSG_DRAW:
		{
			tx += 2;
			ty += 2;
			tw -= 4;
			th -= 4;
			if(!(d->d1))
			{
				if(str)
				{
					if(d->flags & D_DISABLED)
					{
						tl=gui_textout_ln_a5_dis(a5font, str, tx, ty+(d->h-(fh-gui_font_baseline))/2, jwin_a5_pal(jcDISABLED_FG),jwin_a5_pal(jcDISABLED_BG),0,jwin_a5_pal(jcLIGHT));
						bx=tl+fh/2;
					}
					else
					{
						tl=gui_textout_ln_a5(a5font, str, tx, ty+(d->h-(fh-gui_font_baseline))/2, jwin_a5_pal(jcBOXFG),jwin_a5_pal(jcBOX),0);
						bx=tl+fh/2;
					}
				}
			}
			
			jwin_draw_frame_a5(tx+bx, ty, d->h, d->h, FR_DEEP);
			
			if(!(d->flags & D_DISABLED))
			{
				al_draw_filled_rectangle(tx+bx+2, ty+2, tx+bx+d->h-2, ty+d->h-2, jwin_a5_pal(jcTEXTBG));
			}
			
			if(d->d1)
			{
				tx2=tx+bx+d->h-1+(fh/2);
				
				if(str)
				{
					if(d->flags & D_DISABLED)
						tl=gui_textout_ln_a5_dis(a5font, str, tx2, ty+(d->h-(fh-gui_font_baseline))/2, jwin_a5_pal(jcDISABLED_FG),jwin_a5_pal(jcDISABLED_BG),0,jwin_a5_pal(jcLIGHT));
					else
						tl=gui_textout_ln_a5(a5font, str, tx2, ty+(d->h-(fh-gui_font_baseline))/2, jwin_a5_pal(jcBOXFG),jwin_a5_pal(jcBOX),0);
				}
			}

			if(d->flags & D_SELECTED)
				al_draw_x(tx+bx+2, ty+2, tx+bx+d->h-2, ty+d->h-2, jwin_a5_pal(jcTEXTFG),1);
			
			if(str)
				dotted_rect_a5(tx2-1, ty-1, tx2+tl, ty+fh, (d->flags & D_GOTFOCUS)?jwin_a5_pal(jcDARK):jwin_a5_pal(jcBOX), jwin_a5_pal(jcBOX));
			break;
		}
	}
	
	int32_t rval = D_O_K;
	if(msg==MSG_DRAW)
		al_set_clipping_rectangle(ocx,ocy,ocw,och);
	else
		rval = jwin_button_proc_a5(msg, d, 0);
	
    a5font = oldfont;
	return rval;
}

int32_t jwin_list_proc_a5(int32_t msg, DIALOG *d, int32_t c)
{
    ListData *data = (ListData *)d->dp;
    int32_t listsize, i, bottom, height, bar, orig;
    char *sel = (char *)d->dp2;
    int32_t redraw = FALSE;
    int ret = D_O_K;
    switch(msg)
    {
    
    case MSG_START:
        data->listFunc(-1, &listsize);
        _handle_jwin_scrollable_scroll_a5(d, listsize, &d->d1, &d->d2, *data->a5font);
        break;
        
    case MSG_DRAW:
        _jwin_draw_listbox_a5(d,false);
        break;
        
    case MSG_CLICK:
        data->listFunc(-1, &listsize);
        height = (d->h-3) / al_get_font_line_height(*data->a5font);
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
            
            if(_handle_jwin_listbox_click_a5(d)) GUI_EVENT(d, geCHANGE_SELECTION);
            
            bool rightClicked=(gui_mouse_b()&2)!=0;
            while(gui_mouse_b())
            {
                broadcast_dialog_message(MSG_IDLE, 0);
                d->flags |= D_INTERNAL;
				if(_handle_jwin_listbox_click_a5(d))
				{
					d->flags &= ~D_INTERNAL;
					GUI_EVENT(d, geCHANGE_SELECTION);
				}
				d->flags &= ~D_INTERNAL;
                
				update_hw_screen();
            }
            
            if(rightClicked && (d->flags&(D_USER<<1))!=0 && d->dp3)
            {
                typedef void (*funcType)(int32_t /* index */, int32_t /* x */, int32_t /* y */);
                funcType func=reinterpret_cast<funcType>(d->dp3);
                func(d->d1, gui_mouse_x(), gui_mouse_y());
            }
            
            if(d->flags & D_USER)
            {
                if(listsize)
                {
                    clear_keybuf();
                    return D_CLOSE;
                }
            }
        }
        else
        {
            _handle_jwin_scrollable_scroll_click_a5(d, listsize, &d->d2, *data->a5font);
        }
        ret |= D_REDRAW;
        break;
        
    case MSG_DCLICK:
        // Ignore double right-click
        if((gui_mouse_b()&2)!=0)
            break;

        data->listFunc(-1, &listsize);
        height = (d->h-3) / al_get_font_line_height(*data->a5font);
        bar = (listsize > height);
        
        if((!bar) || (gui_mouse_x() < d->x+d->w-18))
        {
            if(d->flags & D_EXIT)
            {
                if(listsize)
                {
                    i = d->d1;
                    object_message(d, MSG_CLICK, 0);
                    
                    if(i == d->d1)
                        return D_CLOSE;
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
        height = (d->h-4) / al_get_font_line_height(*data->a5font);
        
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
            }
        }
        
        break;
        
    case MSG_CHAR:
        data->listFunc(-1,&listsize);

        if(listsize)
        {
            c >>= 8;
            
            bottom = d->d2 + (d->h-3)/al_get_font_line_height(*data->a5font) - 1;
            
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
            _handle_jwin_scrollable_scroll_a5(d, listsize, &d->d1, &d->d2, *data->a5font);
			
			GUI_EVENT(d, geCHANGE_SELECTION);
            
            object_message(d, MSG_DRAW, 0);
            return D_USED_CHAR;
        }
        
        break;
    }
    
    return ret;
}
int32_t jwin_do_abclist_proc_a5(int32_t msg, DIALOG *d, int32_t c)
{
    ListData *data = (ListData *)d->dp;
    int32_t listsize, i, bottom, height, bar, orig, h;
	int32_t ret = D_O_K;
	bool revert_size = false;
	if((d->flags & D_RESIZED) == 0)
	{
		h = d->h;
		d->h -= al_get_font_line_height(*data->a5font);
		d->flags |= D_RESIZED;
		revert_size = true;
    }
	char *sel = (char *)d->dp2;
    int32_t redraw = FALSE;
    
    switch(msg)
    {
    
    case MSG_START:
        data->listFunc(-1, &listsize);
        _handle_jwin_scrollable_scroll_a5(d, listsize, &d->d1, &d->d2, *data->a5font);
        break;
        
    case MSG_DRAW:
        _jwin_draw_listbox_a5(d,true);
        break;
        
    case MSG_CLICK:
		if(gui_mouse_y() > (d->y+d->h-1))
		{
			// if(gui_mouse_y() > (d->y+d->h+2))
				// ;//Clicked on the box displaying the patternmatch
			// else ;//Clicked between the lister and patternmatch
		}
		else //Clicked the lister
		{
			data->listFunc(-1, &listsize);
			height = (d->h-3) / al_get_font_line_height(*data->a5font);
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
				
				if(_handle_jwin_listbox_click_a5(d)) GUI_EVENT(d, geCHANGE_SELECTION);
				
				bool rightClicked=(gui_mouse_b()&2)!=0;
				while(gui_mouse_b())
				{
					broadcast_dialog_message(MSG_IDLE, 0);
					d->flags |= D_INTERNAL;
					if(_handle_jwin_listbox_click_a5(d))
					{
						d->flags &= ~D_INTERNAL;
						GUI_EVENT(d, geCHANGE_SELECTION);
					}
					d->flags &= ~D_INTERNAL;
					
					update_hw_screen();
				}
				
				if(rightClicked && (d->flags&(D_USER<<1))!=0 && d->dp3)
				{
					typedef void (*funcType)(int32_t /* index */, int32_t /* x */, int32_t /* y */);
					funcType func=reinterpret_cast<funcType>(d->dp3);
					func(d->d1, gui_mouse_x(), gui_mouse_y());
				}
				
				if(d->flags & D_USER)
				{
					if(listsize)
					{
						clear_keybuf();
						ret = D_CLOSE;
					}
				}
			}
			else
			{
				_handle_jwin_scrollable_scroll_click_a5(d, listsize, &d->d2, *data->a5font);
			}
			ret |= D_REDRAW;
		}
        break;
        
    case MSG_DCLICK:
        // Ignore double right-click
        if((gui_mouse_b()&2)!=0)
            break;
		
        if(gui_mouse_y() > (d->y+d->h-1))
		{
			// if(gui_mouse_y() > (d->y+d->h+2))
				// ;//Clicked on the box displaying the patternmatch
			// else ;//Clicked between the lister and patternmatch
		}
		else //Clicked the lister
		{
			data->listFunc(-1, &listsize);
			height = (d->h-3) / al_get_font_line_height(*data->a5font);
			bar = (listsize > height);
			
			if((!bar) || (gui_mouse_x() < d->x+d->w-18))
			{
				if(d->flags & D_EXIT)
				{
					if(listsize)
					{
						i = d->d1;
						object_message(d, MSG_CLICK, 0);
						
						if(i == d->d1)
							ret = D_CLOSE;
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
        height = (d->h-4) / al_get_font_line_height(*data->a5font);
        
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
            }
        }
        
        break;
        
    case MSG_CHAR:
        data->listFunc(-1,&listsize);

        if(listsize)
        {
            c >>= 8;
            
            bottom = d->d2 + (d->h-3)/al_get_font_line_height(*data->a5font) - 1;
            
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
            _handle_jwin_scrollable_scroll_a5(d, listsize, &d->d1, &d->d2, *data->a5font);
			
			GUI_EVENT(d, geCHANGE_SELECTION);
			
            object_message(d, MSG_DRAW, 0);
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
int32_t jwin_abclist_proc_a5(int32_t msg,DIALOG *d,int32_t c)
{
    ListData *data = (ListData *)d->dp;
    if(msg == MSG_START) wipe_abc_keypresses();
    
	if(msg==MSG_CHAR)
	{
		if(abc_patternmatch) // Search style pattern match. 
		{
			if(((c&0xFF) > 31) && ((c&0xFF) < 127))
			{
				int32_t max,dummy,h;
				
				h = ((d->h-3) / al_get_font_line_height(*data->a5font))-1;
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
				jwin_do_abclist_proc_a5(MSG_DRAW,d,0);
				if ( gui_mouse_b() ) wipe_abc_keypresses();
				return foundmatch ? D_USED_CHAR : D_O_K;
			}
			else if((c&0xFF) == 8) //backspace
			{
				for ( int32_t q = 1023; q >= 0; --q )
				{
					if ( abc_keypresses[q] ) 
					{
						abc_keypresses[q] = '\0'; break;
					}
				}
				jwin_do_abclist_proc_a5(MSG_DRAW,d,0);
				return D_USED_CHAR;
			}
		}
		else // Windows Explorer style jumping
		{
			if(isalpha(c&0xFF) || isdigit(c&0xFF))
			{
				int32_t max,dummy,h,i;
				
				h = (d->h-3) / al_get_font_line_height(*data->a5font);
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
				
				jwin_list_proc_a5(MSG_DRAW,d,0);
				return foundmatch ? D_USED_CHAR : D_O_K;
			}
		}
	}
	
	if(gui_mouse_b())
		wipe_abc_keypresses();
	return ((abc_patternmatch) ? jwin_do_abclist_proc_a5(msg,d,c) : jwin_list_proc_a5(msg,d,c));
}

int32_t jwin_droplist_proc_a5(int32_t msg,DIALOG *d,int32_t c)
{
	int32_t ret;
	int32_t down=0, last_draw=0;
	int32_t d1;
	
	switch(msg)
	{
		case MSG_CLICK:
			if(mouse_in_rect(d->x+d->w-18,d->y+2,16,d->h))
				goto dropit_a5;
				
			break;
			
		case MSG_KEY:
			goto dropit_a5;
			break;
	}
	
	d1 = d->d1;
	ret = jwin_list_proc_a5(msg,d,c);
	
	if(d->d1!=d->d2)
	{
		d->d1=d->d2;
		jwin_droplist_proc_a5(MSG_DRAW, d, 0);
	}

	if(d1 != d->d1)
	{
		GUI_EVENT(d, geCHANGE_SELECTION);
		if(d->flags&D_EXIT)
			ret |= D_CLOSE;
	}

	if(msg == MSG_DRAW)
		draw_arrow_button_a5(d->x+d->w-18, d->y+2, 16, d->h-4, 0, 0);
	
	return ret;
	
dropit_a5:
	last_draw = 0;
	
	while(gui_mouse_b())
	{
		down = mouse_in_rect(d->x+d->w-18,d->y+2,16,d->h);
		
		if(down!=last_draw)
		{
			draw_arrow_button_a5(d->x+d->w-18, d->y+2, 16, d->h-4, 0, down*3);
			last_draw = down;
		}
		
		clear_keybuf();
		
		update_hw_screen();
	}
	
	if(!down)
		return D_O_K;
	
	draw_arrow_button_a5(d->x+d->w-18, d->y+2, 16, d->h-4, 0, 0);
	
	d1 = d->d1;
	d->d2 = d->d1 = droplist(d);
	
	object_message(d, MSG_DRAW, 0);
	
	while(gui_mouse_b()) {
		clear_keybuf();
		rest(1);
	}

	if(d1!=d->d1)
		GUI_EVENT(d, geCHANGE_SELECTION);

	return ((d1 != d->d1) && (d->flags&D_EXIT)) ? D_CLOSE : D_O_K;
}

int32_t jwin_frame_proc_a5(int32_t msg, DIALOG *d, int32_t)
{
    if(msg == MSG_DRAW)
        jwin_draw_frame_a5(d->x, d->y, d->w, d->h, d->d1);
    return D_O_K;
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
int32_t jwin_swapbtn_proc_a5(int32_t msg, DIALOG* d, int32_t c)
{
	static char* swp[nswapMAX] = {"D", "H", "LD", "LH", "B"};
	d->dp = swp[d->d1&0xF];
	//d1 is (0xF0 = old val, 0x0F = new val)
	//d2 is max val
	if(d->d2 < 2 || d->d2 > nswapMAX) return D_O_K; //Not setup yet, or bad value
	DIALOG* relproc = (DIALOG*)d->dp3;
	GUI::TextField *tf_obj = nullptr;
	if(d->d2 > nswapBOOL) tf_obj = (GUI::TextField*)relproc->dp3;
	int32_t ret = jwin_button_proc_a5(msg, d, c);
	if(d->flags & D_SELECTED) //On selection
	{
		d->d1 = ((d->d1&0x0F)<<4) | (((d->d1&0x0F)+1)%d->d2);
		d->dp = swp[d->d1&0xF];
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
int32_t jwin_numedit_swap_byte_proc_a5(int32_t msg, DIALOG *d, int32_t c)
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
			ret |= jwin_numedit_proc_a5(msg, d, c);
			break;
		case nswapHEX:
			d->d1 = 2; //2 digits max
			if(msg == MSG_CHAR && isalpha(c&255)) //always capitalize
				c = (c&~255) | (toupper(c&255));
			ret |= jwin_numedit_proc_a5(msg, d, c);
			break;
	}
	
	swapbtn->d1 = (ntype<<4)|ntype; //Mark the type change processed
	
	return ret;
}
int32_t jwin_numedit_swap_sshort_proc_a5(int32_t msg, DIALOG *d, int32_t c)
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
			ret |= jwin_numedit_proc_a5(msg, d, c);
			break;
		case nswapHEX:
			d->d1 = 5; //5 digits max (incl '-')
			if(msg == MSG_CHAR && !editproc_special_key(c) && isalpha(c&255)) //always capitalize
				c = (c&~255) | (toupper(c&255));
			ret |= jwin_numedit_proc_a5(msg, d, c);
			break;
	}
	if(rev_d2 && ref_d2 == d->d2)
	{
		d->d2 = old_d2;
	}
	
	swapbtn->d1 = (ntype<<4)|ntype; //Mark the type change processed
	
	return ret;
}
int32_t jwin_numedit_swap_zsint_proc_a5(int32_t msg, DIALOG *d, int32_t c)
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
			ret |= jwin_numedit_proc_a5(msg, d, c);
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
			ret |= jwin_numedit_proc_a5(msg, d, c);
			break;
		case nswapLDEC:
			d->d1 = 11; //11 digits max (incl '-')
			ret |= jwin_numedit_proc_a5(msg, d, c);
			break;
		case nswapLHEX:
			d->d1 = 9; //9 digits max (incl '-')
			if(msg == MSG_CHAR && !editproc_special_key(c) && isalpha(c&255)) //always capitalize
				c = (c&~255) | (toupper(c&255));
			ret |= jwin_numedit_proc_a5(msg, d, c);
			break;
	}
	if(rev_d2 && ref_d2 == d->d2)
	{
		d->d2 = old_d2;
	}
	
	swapbtn->d1 = (ntype<<4)|ntype; //Mark the type change processed
	
	return ret;
}
int32_t jwin_numedit_swap_zsint_nodec_proc_a5(int32_t msg, DIALOG *d, int32_t c)
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
			ret |= jwin_numedit_proc_a5(msg, d, c);
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
			ret |= jwin_numedit_proc_a5(msg, d, c);
			break;
	}
	if(rev_d2 && ref_d2 == d->d2)
	{
		d->d2 = old_d2;
	}
	
	swapbtn->d1 = (ntype<<4)|ntype; //Mark the type change processed
	
	return ret;
}
int32_t jwin_numedit_swap_zsint2_proc_a5(int32_t msg, DIALOG *d, int32_t c)
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
			ret |= jwin_numedit_proc_a5(msg, d, c);
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
			ret |= jwin_numedit_proc_a5(msg, d, c);
			break;
		case nswapLDEC:
			d->d1 = 11; //11 digits max (incl '-')
			ret |= jwin_numedit_proc_a5(msg, d, c);
			break;
		case nswapLHEX:
			d->d1 = 9; //9 digits max (incl '-')
			if(msg == MSG_CHAR && !editproc_special_key(c) && isalpha(c&255)) //always capitalize
				c = (c&~255) | (toupper(c&255));
			ret |= jwin_numedit_proc_a5(msg, d, c);
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

int32_t jwin_vedit_proc_a5(int32_t msg, DIALOG *d, int32_t c)
{
	if(d->flags & D_HIDDEN)
	{
		switch(msg)
		{
			case MSG_CHAR: case MSG_UCHAR: case MSG_XCHAR: case MSG_DRAW: case MSG_CLICK: case MSG_DCLICK: case MSG_KEY: case MSG_WANTFOCUS:
				return D_O_K;
		}
	}
	if(d->h < 2+((al_get_font_line_height(d->dp2 ? (ALLEGRO_FONT*)d->dp2 : a5font))+2)*2)
		return jwin_edit_proc_a5(msg, d, c);
	static char nullbuf[2];
	sprintf(nullbuf, " ");
	int32_t f, l, p, w, x, y;
	int32_t lastSpace = -1;
	char *s;
	char buf[2] = {0,0};
	
	if(d->dp==NULL)
	{
		d->dp=(void *)nullbuf;
	}
	
	s = (char*)d->dp;
	l = (int32_t)strlen(s);
	
	int16_t cursor_start = d->d2 & 0x0000FFFF;
	int16_t cursor_end = int16_t((d->d2 & 0xFFFF0000) >> 16);
	
	if(cursor_start > l)
		cursor_start = l;
	if(cursor_end > l)
		cursor_end = l;
	auto low_cursor = cursor_start<0 ? cursor_end : (cursor_end<0 ? cursor_start : (zc_min(cursor_start, cursor_end)));
	auto high_cursor = zc_max(cursor_start,cursor_end);
	bool multiselect = cursor_end > -1;
	
	ALLEGRO_FONT *oldfont = a5font;
	if(d->dp2)
		a5font = (ALLEGRO_FONT*)d->dp2;
	int fh = al_get_font_line_height(a5font);
	
	std::vector<size_t> lines;
	x = 0;
	
	y = d->y + 2;
	size_t ind = 0;
	int32_t yinc = fh+2;
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
				x += al_char_length(a5font, c);
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
	int ret = D_O_K;
	switch(msg)
	{
		case MSG_START:
			cursor_start = (int32_t)strlen((char*)d->dp);
			cursor_end = -1;
			d->d2 = cursor_start | ((cursor_end & 0xFFFF) << 16);
			break;
			
		case MSG_DRAW:
		{
			ALLEGRO_COLOR fg = jwin_a5_pal(jcTEXTFG), bg = jwin_a5_pal(jcTEXTBG);
			int discol = -1;
			if(d->flags & D_DISABLED)
			{
				discol = jcLIGHT;
				fg = jwin_a5_pal(jcDISABLED_FG);
				bg = jwin_a5_pal(jcDISABLED_BG);
			}
			else if(d->flags & D_READONLY)
			{
				fg = jwin_a5_pal(jcALT_TEXTFG);
				bg = jwin_a5_pal(jcALT_TEXTBG);
			}
			
			//Fill the BG
			al_draw_filled_rectangle(d->x+2, d->y+2, d->x+d->w-2, d->y+d->h-2, bg);
			
			//Now the text
			size_t m = zc_min(line_scroll + maxlines, lines.size());
			jwin_draw_frame_a5(d->x, d->y, d->w, d->h, FR_DEEP);
			for(size_t line = line_scroll; line < m; ++line, y+=yinc)
			{
				x = 3;
				for(ind = line ? lines[line-1] : 0; ind < lines[line]; ++ind)
				{
					char c = s[ind] ? s[ind] : ' ';
					w = al_char_length(a5font, c);
					bool focused = multiselect
						? (ind >= low_cursor && ind <= high_cursor)
						: (ind == cursor_start);
					f = (focused && (d->flags & D_GOTFOCUS));
					buf[0] = c;
					if(discol > -1)
						jwin_textout_a5_dis(a5font, f ? bg : fg, d->x+x, y, 0, buf, f ? fg : bg, jwin_a5_pal(discol));
					else jwin_textout_a5(a5font, f ? bg : fg, d->x+x, y, 0, buf, f ? fg : bg);
					x += w;
				}
			}
			break;
		}
			
		case MSG_CLICK:
		{
			if(d->flags & (D_DISABLED|D_READONLY))
				break;
			
			bool found = false;
			for(size_t line = line_scroll; line < lines.size() && line < (line_scroll + maxlines); ++line, y+=yinc)
			{
				if(gui_mouse_y() >= y+yinc)
					continue;
				x = d->x+3;
				for(ind = line ? lines[line-1] : 0; ind < lines[line]; ++ind)
				{
					if(x >= gui_mouse_x())
					{
						if(key_shifts&KB_SHIFT_FLAG)
							cursor_end = ind;
						else
						{
							cursor_start = ind;
							cursor_end = -1;
						}
						found = true;
						break;
					}
					x += al_char_length(a5font, s[ind]);
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
				}
			}
			if(cursor_end == cursor_start) cursor_end = -1;
			d->d2 = cursor_start | ((cursor_end&0xFFFF) << 16);
			
			object_message(d, MSG_DRAW, 0);
			break;
		}
		
		case MSG_WANTFOCUS:
		case MSG_LOSTFOCUS:
		case MSG_KEY:
			if(d->flags & (D_DISABLED|D_READONLY))
				break;
			ret = D_WANTFOCUS;
			break;
			
		case MSG_CHAR:
		{
			if(d->flags & (D_DISABLED|D_READONLY))
				break;
			bool shifted = key_shifts & KB_SHIFT_FLAG;
			bool ctrl = key_shifts & KB_CTRL_FLAG;
			bool change_cursor = true;
			int16_t scursor = cursor_start, ecursor = cursor_end;
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
						x = d->x + 3;
						for(ind = focused_line2 ? lines[focused_line2-1] : 0; ind < lines[focused_line2]; ++ind)
						{
							w = al_char_length(a5font, s[ind]);
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
							tx += al_char_length(a5font, s[ind] ? s[ind] : ' ');
							if(tx < x)
								continue;
							ecursor = ind;
							done = true;
							break;
						}
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
						x = d->x + 3;
						for(ind = focused_line ? lines[focused_line-1] : 0; ind < lines[focused_line]; ++ind)
						{
							w = al_char_length(a5font, s[ind]);
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
							tx += al_char_length(a5font, s[ind] ? s[ind] : ' ');
							if(tx < x)
								continue;
							scursor = ind;
							done = true;
							break;
						}
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
					ret = D_CLOSE;
				}
				break;
			}
			else if(upper_c == KEY_TAB)
			{
				change_cursor = false;
				break;
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
						--paste_len;
						--paste_end;
						--ind;
						--ind2;
					}
					size_t new_l = ind2+1;
					while(ind >= paste_start)
					{
						if(s[ind] || (ind&&s[ind-1]))
						{
							s[ind2] = s[ind];
						}
						--ind2; --ind;
					}
					for(auto q = 0; q < paste_len; ++q)
					{
						s[paste_start+q] = cb.at(q);
					}
					s[paste_start+paste_len] = 0;
					scursor = paste_start + paste_len;
					ecursor = -1;
					GUI_EVENT(d, geCHANGE_VALUE);
				}
			}
			else if(ctrl && (lower_c=='a' || lower_c=='A'))
			{
				scursor = 0;
				ecursor = d->d1;
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
			else break;
			
			if(change_cursor)
			{
				cursor_end = ecursor; cursor_start = scursor;
				if (cursor_end == cursor_start) cursor_end = -1;
				d->d2 = cursor_start | ((cursor_end&0xFFFF) << 16);
			}
			
			/* if we changed something, better redraw... */
			object_message(d, MSG_DRAW, 0);
			ret = D_USED_CHAR;
			break;
		}
	}
	a5font = oldfont;
	return ret;
}
int32_t jwin_edit_proc_a5(int32_t msg, DIALOG *d, int32_t c)
{
	if(d->flags & D_HIDDEN)
	{
		switch(msg)
		{
			case MSG_CHAR: case MSG_UCHAR: case MSG_XCHAR: case MSG_DRAW: case MSG_CLICK: case MSG_DCLICK: case MSG_KEY: case MSG_WANTFOCUS:
				return D_O_K;
		}
	}
	if(d->h >= 2+((al_get_font_line_height(d->dp2 ? (ALLEGRO_FONT*)d->dp2 : a5font)+2)*2))
		return jwin_vedit_proc_a5(msg, d, c);
	int32_t f, l, p, w, x, y;
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
	
	int16_t cursor_start = d->d2 & 0x0000FFFF;
	int16_t cursor_end = int16_t((d->d2 & 0xFFFF0000) >> 16);
	
	if(cursor_start > l)
		cursor_start = l;
	if(cursor_end > l)
		cursor_end = l;
	auto low_cursor = cursor_start<0 ? cursor_end : (cursor_end<0 ? cursor_start : (zc_min(cursor_start, cursor_end)));
	auto high_cursor = zc_max(cursor_start,cursor_end);
		
	/* calculate maximal number of displayable characters */
	b = x = 0;
	
	ALLEGRO_FONT* oldfont = a5font;
	if(d->dp2)
		a5font = (ALLEGRO_FONT*)d->dp2;
	int fh = al_get_font_line_height(a5font);
	
	if(cursor_start == l)
	{
		buf[0] = ' ';
		buf[1] = 0;
		
		x = al_get_text_width(a5font, buf);
	}
	
	buf[1] = 0;
	
	for(p=cursor_start; p>=0; p--)
	{
		buf[0] = s[p];
		b++;
		
		x += al_get_text_width(a5font, buf);
			
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
	
	int ret = D_O_K;
	switch(msg)
	{
		case MSG_START:
			cursor_start = (int32_t)strlen((char*)d->dp);
			cursor_end = -1;
			d->d2 = cursor_start | ((cursor_end & 0xFFFF) << 16);
			break;
			
		case MSG_DRAW:
		{
			ALLEGRO_COLOR fg = jwin_a5_pal(jcTEXTFG), bg = jwin_a5_pal(jcTEXTBG);
			int discol = -1;
			if(d->flags & D_DISABLED)
			{
				discol = jcLIGHT;
				fg = jwin_a5_pal(jcDISABLED_FG);
				bg = jwin_a5_pal(jcDISABLED_BG);
			}
			else if(d->flags & D_READONLY)
			{
				fg = jwin_a5_pal(jcALT_TEXTFG);
				bg = jwin_a5_pal(jcALT_TEXTBG);
			}
			
			x = 3;
			y = (d->h - fh) / 2 + d->y;
			
			//Fill the bg
			al_draw_filled_rectangle(d->x, d->y, d->x+d->w, d->y+d->h, bg);
			
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
				w = al_get_text_width(a5font, buf);
				
				if(x+w > d->w)
					break;
				bool focused = (cursor_end>-1)
					? (p >= low_cursor && p <= high_cursor)
					: (p == cursor_start);
				f = discol < 0 && (focused && (d->flags & D_GOTFOCUS));
				if(discol > -1)
					jwin_textout_a5_dis(a5font, f ? bg : fg, d->x+x, y, 0, buf, f ? fg : bg, jwin_a5_pal(discol));
				else jwin_textout_a5(a5font, f ? bg : fg, d->x+x, y, 0, buf, f ? fg : bg);
				x += w;
			}
			
			if(x < d->w-2)
				al_draw_filled_rectangle(d->x+x, y, d->x+d->w-2, y+fh, bg);
				
			jwin_draw_frame_a5(d->x, d->y, d->w, d->h, FR_DEEP);
			break;
		}
			
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
				x += al_get_text_width(a5font, buf);
				
				if(x > gui_mouse_x())
					break;
			}
			
			if(key_shifts&KB_SHIFT_FLAG)
				cursor_end = MID(0, p, l);
			else
			{
				cursor_end = -1;
				cursor_start = MID(0, p, l);
			}
			if(cursor_end == cursor_start) cursor_end = -1;
			d->d2 = cursor_start | ((cursor_end&0xFFFF) << 16);
			object_message(d, MSG_DRAW, 0);
			break;
		}
			
		case MSG_WANTFOCUS:
		case MSG_LOSTFOCUS:
		case MSG_KEY:
			if(d->flags & (D_DISABLED|D_READONLY))
				break;
			ret = D_WANTFOCUS;
			
		case MSG_CHAR:
		{
			if(d->flags & (D_DISABLED|D_READONLY))
				break;
			bool shifted = key_shifts & KB_SHIFT_FLAG;
			bool ctrl = key_shifts & KB_CTRL_FLAG;
			bool change_cursor = true;
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
					ret = D_CLOSE;
				}
				break;
			}
			else if(upper_c == KEY_TAB)
			{
				change_cursor = false;
				break;
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
						--paste_len;
						--paste_end;
						--ind;
						--ind2;
					}
					size_t new_l = ind2+1;
					while(ind >= paste_start)
					{
						if(s[ind] || (ind&&s[ind-1]))
						{
							s[ind2] = s[ind];
						}
						--ind2; --ind;
					}
					for(auto q = 0; q < paste_len; ++q)
					{
						s[paste_start+q] = cb.at(q);
					}
					s[paste_start+paste_len] = 0;
					scursor = paste_start + paste_len;
					ecursor = -1;
					GUI_EVENT(d, geCHANGE_VALUE);
				}
			}
			else if(ctrl && (lower_c=='a' || lower_c=='A'))
			{
				scursor = 0;
				ecursor = d->d1;
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
			else break;
			if(change_cursor)
			{
				cursor_end = ecursor; cursor_start = scursor;
				if (cursor_end == cursor_start) cursor_end = -1;
				d->d2 = cursor_start | ((cursor_end&0xFFFF) << 16);
			}
			/* if we changed something, better redraw... */
			object_message(d, MSG_DRAW, 0);
			ret = D_USED_CHAR;
			break;
		}
	}
	a5font = oldfont;
	return ret;
}
int32_t jwin_hexedit_proc_a5(int32_t msg,DIALOG *d,int32_t c)
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
	
	auto ret = jwin_edit_proc_a5(msg,d,c);
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
			jwin_edit_proc_a5(MSG_DRAW,d,0);
		}
	}
	return ret;
}
int32_t jwin_numedit_proc_a5(int32_t msg,DIALOG *d,int32_t c)
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
			
	return jwin_edit_proc_a5(msg,d,c);
}


