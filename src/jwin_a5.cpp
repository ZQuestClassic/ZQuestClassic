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
//Externed from jwin.cpp
extern bool no_hline;

bool is_in_rect(int32_t x,int32_t y,int32_t rx1,int32_t ry1,int32_t rx2,int32_t ry2);
int32_t count_newline(uint8_t *s);

//JWin A5 Palette

ALLEGRO_COLOR jwin_a5_colors[9];
ALLEGRO_COLOR jwin_a5_pal(int jc)
{
	return jwin_a5_colors[r_dvc(jwin_pal[jc])];
}

static ALLEGRO_COLOR AL5_INVIS = al_map_rgba(0,0,0,0);
void jwin_set_a5_colors(ALLEGRO_COLOR* colors)
{
	for(int q = 1; q <= 8; ++q)
		jwin_a5_colors[q] = colors[q];
}
void jwin_get_a5_colors(ALLEGRO_COLOR* colors)
{
	for(int q = 1; q <= 8; ++q)
		colors[q] = jwin_a5_colors[q];
}

ALLEGRO_COLOR db_a5_colors[9];
static ALLEGRO_COLOR tmpcol[9];
static int in_dbproc = 0;
#define ALL_DB_PROC (zc_get_config("zquest","devmode",0)==42)
void start_db_proc()
{
	if(!in_dbproc)
	{
		jwin_get_a5_colors(tmpcol);
		jwin_set_a5_colors(db_a5_colors);
	}
	++in_dbproc;
}
void end_db_proc()
{
	if(ALL_DB_PROC) return;
	if(in_dbproc)
	{
		--in_dbproc;
		if(!in_dbproc)
		{
			jwin_set_a5_colors(tmpcol);
		}
	}
}
void end_all_db_proc()
{
	if(in_dbproc)
	{
		in_dbproc = 1;
		end_db_proc();
	}
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
	al_draw_filled_rectangle(x+2,y+2,x+w-2,y+h-2,jwin_a5_pal(jcBOX));
	jwin_draw_frame_a5(x, y, w, h, frame);
}

void dotted_rect_a5(int32_t x1, int32_t y1, int32_t x2, int32_t y2, ALLEGRO_COLOR fg, ALLEGRO_COLOR bg)
{
	int32_t x = ((x1+y1) & 1) ? 1 : 0;
	int32_t c;
	
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
	al_unmap_rgba(bgc,nullptr,nullptr,nullptr,&a);
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
	if(flags & D_DISABLED)
		gui_textout_ln_a5_dis(a5font,str,x+w/2+g, y+(h-th)/2+g,jwin_a5_pal(jcBOXFG),AL5_INVIS,ALLEGRO_ALIGN_CENTRE,jwin_a5_pal(jcLIGHT));
	else
		gui_textout_ln_a5(a5font,str,x+w/2+g, y+(h-th)/2+g,jwin_a5_pal(jcBOXFG),AL5_INVIS,ALLEGRO_ALIGN_CENTRE);
	
	if(show_dotted_rect&&(flags & D_GOTFOCUS))
		dotted_rect_a5(x+4, y+4, x+w-5, y+h-5, jwin_a5_pal(jcDARK), jwin_a5_pal(jcBOX));
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
				scare_mouse();
				jwin_draw_text_button_a5(x, y, w, h, text, D_SELECTED, true);
				unscare_mouse();
				over=true;
				
				update_hw_screen();
			}
		}
		else
		{
			if(over)
			{
				vsync();
				scare_mouse();
				jwin_draw_text_button_a5(x, y, w, h, text, 0, true);
				unscare_mouse();
				over=false;
				
				update_hw_screen();
			}
		}
		
	}
	
	if(over)
	{
		vsync();
		scare_mouse();
		jwin_draw_text_button_a5(x, y, w, h, text, 0, true);
		unscare_mouse();
		
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

void draw_arrow_button_a5(int32_t x, int32_t y, int32_t w, int32_t h, int32_t up, int32_t state)
{
	ALLEGRO_COLOR c = jwin_a5_pal(jcDARK);
	int32_t ah = zc_min(h/3, 5);
	int32_t i = 0;
	
	jwin_draw_button_a5(x,y,w,h,state,1);
	x += w/2 - (state?0:1);
	y += (h-ah)/2 + (state?1:0);
	
	for(; i<ah; i++)
	{
		al_draw_hline(x-(up?i:ah-i-1), y+i, x+(up?i:ah-i-1)+1, c);
	}
}

int32_t jwin_do_x_button_a5(int32_t x, int32_t y)
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

//Tabs

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

//Scrollers

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
				scare_mouse();
				d->proc(MSG_DRAW, d, 0);
				draw_arrow_button_a5(xx, yy, 16, bh, obj==top_btn, down*3);
				unscare_mouse();
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
					scare_mouse();
					d->proc(MSG_DRAW, d, 0);
					unscare_mouse();
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
					scare_mouse();
					d->proc(MSG_DRAW, d, 0);
					unscare_mouse();
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
		scare_mouse();
		draw_arrow_button_a5(xx, yy, 16, bh, obj==top_btn, 0);
		unscare_mouse();
	}
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

//Texts

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

//JWin A5 procs

int32_t jwin_win_proc_a5(int32_t msg, DIALOG *d, int32_t)
{
	end_all_db_proc();
	if(ALL_DB_PROC) start_db_proc();
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
			
			rectfill(screen, d->x, d->y, d->x+d->w-1, d->y+d->h-1, 0); //!TODO Remove when a5 dialog done - Clear a4 screen layer
			
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
							al_draw_vline(tx+(2-sd), d->y+sd+2, d->y+8+th, jwin_a5_pal(jcDARK));
							al_draw_vline(tx+(2-sd)-1, d->y+sd+2, d->y+8+th+(sd?0:1), jwin_a5_pal(jcMEDDARK));
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

int32_t new_text_proc_a5(int32_t msg, DIALOG *d, int32_t)
{
	if(msg==MSG_DRAW)
	{
		if(d->flags & D_HIDDEN) return D_O_K;
		al_set_clipping_rectangle(d->x, d->y, d->x+d->w-1, d->y+d->h-1);
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
		clear_a5_clip_rect();
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
			scare_mouse();
			object_message(d, MSG_DRAW, 0);
			unscare_mouse();
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
					if(last_draw != down)
					{
						if(down != selected)
							d->flags |= D_SELECTED;
						else
							d->flags &= ~D_SELECTED;
							
						scare_mouse();
						object_message(d, MSG_DRAW, 0);
						unscare_mouse();
						last_draw = down;
					}
					
					/* let other objects continue to animate */
					broadcast_dialog_message(MSG_IDLE, 0);
					
					update_hw_screen();
				}
				
				/* redraw in normal state */
				if(down)
				{
					GUI_EVENT(d, geCLICK);
					if(d->flags&D_EXIT)
					{
						d->flags &= ~D_SELECTED;
						scare_mouse();
						object_message(d, MSG_DRAW, 0);
						unscare_mouse();
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


