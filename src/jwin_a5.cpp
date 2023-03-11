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

ALLEGRO_COLOR jwin_a5_colors[9];
ALLEGRO_COLOR jwin_a5_pal(int jc)
{
	return jwin_a5_colors[r_dvc(jwin_pal[jc])];
}

void jwin_set_a5_colors(ALLEGRO_COLOR* colors)
{
	for(int q = 1; q <= 8; ++q)
		jwin_a5_colors[q] = colors[q];
}

ALLEGRO_COLOR a5color(RGB c)
{
	return al_map_rgb(c.r*4,c.g*4,c.b*4);
}
ALLEGRO_COLOR a5color(int index)
{
	RGB tmp;
	get_color(index,&tmp);
	return a5color(tmp);
}

void al_draw_hline(float x1, float y1, float x2, ALLEGRO_COLOR c)
{
	al_draw_line(x1,y1,x2,y1,c,1);
}
void al_draw_vline(float x1, float y1, float y2, ALLEGRO_COLOR c)
{
	al_draw_line(x1,y1,x1,y2,c,1);
}
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
	al_draw_filled_rectangle(x+2,y+2,x+w-2,y+h-2,jwin_a5_pal(jcBOX));
	jwin_draw_frame_a5(x, y, w, h, frame);
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

void draw_question_button_a5(int32_t x, int32_t y, int32_t state)
{
	ALLEGRO_COLOR c = jwin_a5_pal(jcBOXFG);
	
	jwin_draw_button_a5(x,y,16,14,state,0);
	x += 4 + (state?1:0);
	y += 3 + (state?1:0);
	
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

static int32_t jwin_do_x_button_a5(int32_t x, int32_t y)
{
	int32_t down=0, last_draw = 0;
	
	while(gui_mouse_b())
	{
		down = mouse_in_rect(x,y,16,14);
		
		if(down!=last_draw)
		{
			scare_mouse();
			draw_x_button_a5(x,y,down);
			unscare_mouse();
			last_draw = down;
		}
		
		/* let other objects continue to animate */
		broadcast_dialog_message(MSG_IDLE, 0);
		rest(1);
	}
	
	if(down)
	{
		scare_mouse();
		draw_x_button_a5(x,y,0);
		unscare_mouse();
	}
	
	return down;
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
	
	al_draw_text(a5font,jwin_a5_pal(jcTITLEFG),tx,ty,0,buf);
	
	if(draw_button)
	{
		draw_x_button_a5(x + w - 18, y+2, 0);
	}
	
	if(helpbtn)
	{
		draw_question_button_a5(x + w - (draw_button ? 36 : 18), y+2, 0);
	}
	
}

int32_t jwin_win_proc_a5(int32_t msg, DIALOG *d, int32_t)
{
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
			
			rectfill(screen, d->x, d->y, d->x+d->w-1, d->y+d->h-1, 0xFF);
			
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
				if(jwin_do_x_button_a5(d->x+d->w-21, d->y+5))
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



