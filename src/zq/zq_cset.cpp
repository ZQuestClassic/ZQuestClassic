//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zquest.cc
//
//  Main code for the quest editor.
//
//--------------------------------------------------------

#include "precompiled.h" //always first

//#include <mem.h>
#include <string.h>
#include <stdio.h>

#include "base/gui.h"
#include "base/zc_alleg.h"
#include "base/zdefs.h"
#include "base/colors.h"
#include "pal.h"
#include "zquest.h"
#include "jwin.h"
#include "base/zsys.h"
#include "zq_tiles.h"
#include "zq_misc.h"
#include "zq_cset.h"
#include "zq_class.h"
#include "dialog/paledit.h"
#include "zq/render.h"
#include "gfxpal.h"

int32_t color_index=0, color_copy=-1;

void get_cset(int32_t dataset,int32_t row,RGB *pal)
{
    saved=false;
    
    for(int32_t i=0; i<16; i++)
    {
        colordata[(CSET(dataset)+i)*3]   = pal[(row<<4)+i].r;
        colordata[(CSET(dataset)+i)*3+1] = pal[(row<<4)+i].g;
        colordata[(CSET(dataset)+i)*3+2] = pal[(row<<4)+i].b;
    }
}

int32_t color = 0;
int32_t gray  = 0;
int32_t ratio = 32;

int32_t jwin_hsl_proc(int32_t msg, DIALOG *d, int32_t c);
int jwin_cset_proc(int msg, DIALOG* d, int c);
int32_t edit_cset_kb_handler(int32_t msg, DIALOG* d, int32_t c);
void onInsertColor();
void onInsertColor_Text();
void onInsertColor_Hex();
void jumpText(int32_t r, int32_t g, int32_t b);
void jumpHex(int32_t r, int32_t g, int32_t b);
void onJumpText();
void onJumpHex();
void onJumpHSL();

static DIALOG edit_cset_dlg[] =
{
	/* (dialog proc)       (x)    (y)   (w)   (h)     (fg)      (bg)     (key)      (flags)     (d1)           (d2)     (dp) */
	{ jwin_win_proc,         0,   0,    320,  240,    vc(14),   vc(1),      0,      D_EXIT,     0,             0, (void *) "Edit CSet", NULL, NULL },
	{ d_timer_proc,          0,    0,     0,    0,    0,        0,          0,      0,          0,             0,       NULL, NULL, NULL },
	{ d_dummy_proc,          0,    0,     0,    0,    0,        0,          0,      0,          0,             0, NULL, NULL, NULL },
	{ jwin_button_proc,    240,  184,    61,   21,    vc(14),   vc(1),     27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
	{ jwin_button_proc,    240,  152,    61,   21,    vc(14),   vc(1),     13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
	{ d_keyboard_proc,       0,    0,     0,    0,         0,       0,      0,      0,          KEY_F1,        0, (void *) onHelp, NULL, NULL },
	{ edit_cset_kb_handler,  0,    0,     0,    0,         0,       0,      0,      0,          0,             0, NULL, NULL, NULL },
	// 7
	{ d_dummy_proc,         160, 140,    61,   21,    vc(14),   vc(1),      0,      0,          0,             0, (void *) "Jump", NULL, (void*) onJumpHSL },
	{ jwin_hsl_proc,        72,   28,   174,   88,    vc(14),   vc(1),      0,      0,          0,             0, NULL, NULL, NULL },
	{ jwin_cset_proc,       96,  170,   128,   24,    vc(14),   vc(1),      0,      0,          0,             0, NULL, NULL, NULL },
    // 10
	{ jwin_edit_proc,       24,  120,   24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,       24,  140,   24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,       24,  160,   24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          3,             0,       NULL, NULL, NULL },
	{ jwin_func_button_proc,16,  180,   40,   21,    vc(14),   vc(1),      0,      0,          0,             0, (void *) "Insert", NULL, (void*) onInsertColor_Text },
	{ jwin_func_button_proc,16,  205,   40,   21,    vc(14),   vc(1),      0,      0,          0,             0, (void *) "Jump", NULL, (void*) onJumpText },
	// 15
	{ jwin_func_button_proc,130, 140,    61,   21,    vc(14),   vc(1),      0,      0,          0,             0, (void *) "&Insert", NULL, (void*) onInsertColor },
    { jwin_text_proc,        16,  124,   8,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "R", NULL, NULL },
    { jwin_text_proc,        16,  144,   8,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "G", NULL, NULL },
    { jwin_text_proc,        16,  164,   8,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "B", NULL, NULL },
    { jwin_check_proc,      230,   30,   8,   9,    vc(0),              vc(11),           0,       0,          0,             0, (void *) "View 8b values", NULL, NULL },
	// 20
    { jwin_text_proc,       16,   54,   8,     8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "0x", NULL, NULL },
    { jwin_edit_proc,       24,   50,   48,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          6,             0,       NULL, NULL, NULL },
	{ jwin_func_button_proc,16,   70,   40,   21,    vc(14),   vc(1),      0,      0,          0,             0, (void *) "Insert", NULL, (void*) onInsertColor_Hex },
	{ jwin_func_button_proc,16,   95,   40,   21,    vc(14),   vc(1),      0,      0,          0,             0, (void *) "Jump", NULL, (void*) onJumpHex },
	
	{ NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

int32_t jwin_hsl_proc(int32_t msg, DIALOG *d, int32_t c)
{
#define HSL_FLAG_HASMOUSE	1
#define HSL_FLAG_COLOR		2
#define HSL_FLAG_SATURATION	4
#define HSL_FLAGS_CS		(HSL_FLAG_COLOR|HSL_FLAG_SATURATION)
	ASSERT(d);
	//d->w and d->h are ignored
	d->w = 1.5*174;
	d->h = 1.5*88;
	int32_t ret = D_O_K;
	static int32_t hue_x_offs = 24, hue_y_offs = 2, light_x_offs = 2, light_y_offs = 24,
		sat_x_offs = 158, sat_y_offs = 24, c_x_offs = 24, c_y_offs = 24, c_wid = 128, c_hei = 64, misc_wh = 16;
	if(d->flags & D_DIRTY)
	{
		ret |= D_REDRAWME;
		d->flags &= ~D_DIRTY;
	}
	if(c_hei == 64)
	{
		hue_x_offs *= 1.5;
		hue_y_offs *= 1.5;
		light_x_offs *= 1.5;
		light_y_offs *= 1.5;
		sat_x_offs *= 1.5;
		sat_y_offs *= 1.5;
		c_x_offs *= 1.5;
		c_y_offs *= 1.5;
		c_wid *= 1.5;
		c_hei *= 1.5;
		misc_wh *= 1.5;
	}
	int32_t clr = color, gr = gray, rat = ratio;
	int32_t x=gui_mouse_x();
	int32_t y=gui_mouse_y();
	switch(msg)
	{
		case MSG_START:
			d->d1 = 0;
			break;
		case MSG_GOTMOUSE:
			d->d1 |= HSL_FLAG_HASMOUSE;
			break;
		case MSG_LOSTMOUSE:
			d->d1 &= ~HSL_FLAG_HASMOUSE;
			break;
		case MSG_CLICK:
		case MSG_DCLICK:
		case MSG_LPRESS:
			if((d->d1 & HSL_FLAG_HASMOUSE)&&!(d->d1 & HSL_FLAGS_CS))
			{
				if(isinRect(x,y,(d->x+c_x_offs),(d->y+c_y_offs),d->x+c_x_offs+c_wid-1,(d->y+c_y_offs)+c_hei-1))
				{
					d->d1 |= HSL_FLAG_COLOR;
				}
				else if(isinRect(x,y,(d->x+sat_x_offs),(d->y+sat_y_offs),(d->x+sat_x_offs)+misc_wh-1,(d->y+sat_y_offs)+c_hei-1))
				{
					d->d1 |= HSL_FLAG_SATURATION;
				}
			}
			break;
		case MSG_LRELEASE:
			d->d1 &= ~HSL_FLAGS_CS;
			break;
		case MSG_IDLE:
			if(gui_mouse_b()==1)
			{
				if(d->d1 & HSL_FLAG_COLOR)
				{
					color = vbound(x-(d->x+c_x_offs),0,c_wid-1);
					gray = vbound(y-(d->y+c_y_offs),0,c_hei-1);
				}
				if(d->d1 & HSL_FLAG_SATURATION)
				{
					ratio = vbound(y-(d->y+sat_y_offs),0,c_hei-1);
				}
			}
			else
			{
				d->d1 &= ~(HSL_FLAG_COLOR|HSL_FLAG_SATURATION);
			}
			//Fallthrough
		case MSG_DRAW:
			clr /= 1.5;
			gr /= 1.5;
			rat /= 1.5;
			custom_vsync();
			//Hue
			jwin_draw_frame(screen, d->x+hue_x_offs-2, d->y+hue_y_offs-2, int32_t(128*1.5+4), misc_wh+4, FR_DEEP);
			for(int32_t i=0; i<128; i++)
			{
				rectfill(screen,int32_t(floor(i*(1.5))+d->x+hue_x_offs),d->y+hue_y_offs,int32_t(ceil(i*(1.5))+d->x+hue_x_offs),d->y+hue_y_offs+misc_wh-1,i);
			}
			//Light
			jwin_draw_frame(screen, d->x+light_x_offs-2, d->y+light_y_offs-2, misc_wh+4, int32_t(64*(1.5)+4), FR_DEEP);
			for(int32_t i=0; i<32; i++)
			{
				rectfill(screen,d->x+light_x_offs,((int32_t)floor(i*(1.5))<<1)+d->y+light_y_offs,
						 d->x+light_x_offs+misc_wh-1,((int32_t)ceil(i*(1.5))<<1)+d->y+light_y_offs+1,i+128);
			}
			//Saturation
			jwin_draw_frame(screen, d->x + (sat_x_offs-2), d->y + (sat_y_offs-2), misc_wh+4, int32_t(64*(1.5)+4), FR_DEEP);
			for(int32_t i=0; i<32; i++)
			{
				RAMpal[i+160] = mixRGB(gfx_pal[clr*3],gfx_pal[clr*3+1],
									   gfx_pal[clr*3+2],gr,gr,gr,i<<1); //saturatn
				rectfill(screen,d->x + sat_x_offs,((int32_t)floor(i*(1.5))<<1)+d->y+sat_y_offs,
						 d->x+sat_x_offs+misc_wh-1,((int32_t)ceil(i*1.5)<<1)+d->y+sat_y_offs+1,i+160);
			}
			RAMpal[edc] = mixRGB(gfx_pal[clr*3],gfx_pal[clr*3+1],gfx_pal[clr*3+2],gr,gr,gr,rat);
			RAMpal[edi] = invRGB(RAMpal[edc]);
			zc_set_palette_range(RAMpal,0,255,false);
			//Color
			jwin_draw_frame(screen, d->x+c_x_offs-2, d->y+c_y_offs-2, c_wid+4, c_hei+4, FR_DEEP);
			rectfill(screen,d->x+c_x_offs,d->y+c_y_offs,d->x+c_x_offs+c_wid-1,d->y+c_y_offs+c_hei-1,edc);
			//
			_allegro_hline(screen,d->x+c_x_offs,gray+d->y+c_y_offs,d->x+c_x_offs+c_wid-1,edi);
			_allegro_vline(screen,color+d->x+c_x_offs,d->y+c_y_offs,d->y+c_y_offs+c_hei-1,edi);
			_allegro_hline(screen,d->x+sat_x_offs,ratio+d->y+sat_y_offs,d->x+sat_x_offs+misc_wh-1,edi);
			if((edit_cset_dlg[19].flags & D_SELECTED))
				textprintf_centre_ex(screen,font,d->x+(d->w/2),int32_t(d->y+c_y_offs+c_hei+10*(1.5)),jwin_pal[jcBOXFG],jwin_pal[jcBOX],"  RGB - %3d %3d %3d  ",RAMpal[edc].r*4,RAMpal[edc].g*4,RAMpal[edc].b*4);
			else textprintf_centre_ex(screen,font,d->x+(d->w/2),int32_t(d->y+c_y_offs+c_hei+10*(1.5)),jwin_pal[jcBOXFG],jwin_pal[jcBOX],"  RGB - %2d %2d %2d  ",RAMpal[edc].r,RAMpal[edc].g,RAMpal[edc].b);
			SCRFIX();
			break;
	}
	return ret;
}

int jwin_cset_proc(int msg, DIALOG* d, int c)
{
	static int cs_hei = 12, last_copy = 0, lastshow16 = 0;
	ASSERT(d);
	int ret = D_O_K;
	if(lastshow16 != (edit_cset_dlg[19].flags & D_SELECTED))
	{
		lastshow16 = (edit_cset_dlg[19].flags & D_SELECTED);
		char* c1 = (char*)edit_cset_dlg[10].dp;
		char* c2 = (char*)edit_cset_dlg[11].dp;
		char* c3 = (char*)edit_cset_dlg[12].dp;
		int32_t r = atoi(c1), g = atoi(c2), b = atoi(c3);
		
		if(!lastshow16) //Just turned off
		{
			r /= 4;
			g /= 4;
			b /= 4;
		}
		jumpText(r,g,b);
		ret |= D_REDRAW | D_REDRAWME;
	}
	d->h = cs_hei * 3;
	int32_t x = gui_mouse_x();
	int32_t y = gui_mouse_y();
	switch(msg)
	{
		case MSG_START:
			color_index = 0;
			color_copy = last_copy = -1;
			break;
		case MSG_CLICK:
		{
			if(gui_mouse_b() != 1) break;
			color_index = vbound((gui_mouse_x()-d->x)/cs_hei,0,15);
			d->flags |= D_DIRTY;
			while(gui_mouse_b() == 1)
			{
				int new_index = vbound((gui_mouse_x()-d->x)/cs_hei,0,15);
				if(new_index != color_index)
				{
					if(color_index<new_index)
					{
						for(int32_t i=color_index; i<new_index; ++i)
							zc_swap(RAMpal[14*16+i], RAMpal[14*16+i+1]);
					}
					else
					{
						for(int32_t i=color_index; i>new_index; --i)
							zc_swap(RAMpal[14*16+i], RAMpal[14*16+i-1]);
					}
					color_index = new_index;
					d->flags |= D_DIRTY;
				}
				broadcast_dialog_message(MSG_IDLE,0);
				custom_vsync();
			}
			break;
		}
		case MSG_IDLE:
			if(color_copy != last_copy)
			{
				last_copy = color_copy;
				ret |= D_REDRAWME;
			}
			break;
		case MSG_DRAW:
			rectfill(screen,d->x-2,d->y-2,d->x+d->w+4,d->y+d->h+20,jwin_pal[jcBOX]);
			// Old colors
			jwin_draw_frame(screen, d->x-2, d->y-2, d->w+4, cs_hei+4, FR_DEEP);
			for(int32_t i=0; i<16; ++i)
			{
				rectfill(screen,i*cs_hei+d->x,d->y,i*cs_hei+d->x+cs_hei-1,d->y+cs_hei-1,12*16+i);
			}
			// New colors
			jwin_draw_frame(screen, d->x-2, d->y-2+(cs_hei*2), d->w+4, cs_hei+4, FR_DEEP);
			for(int32_t i=0; i<16; ++i)
			{
				rectfill(screen,i*cs_hei+d->x,d->y+(cs_hei*2),i*cs_hei+d->x+cs_hei-1,d->y+(cs_hei*3)-1,14*16+i);
			}
			//Text
			rectfill(screen,d->x,d->y + d->h + 3,d->x + d->w - 1,d->y + d->h + 48,jwin_pal[jcBOX]);
			
			if(color_copy>=0)
				textout_ex(screen,(get_zc_font(font_lfont_l)),"\x81",color_copy*cs_hei+d->x,d->y + d->h + 3,jwin_pal[jcBOXFG],jwin_pal[jcBOX]);
			
			textout_ex(screen,(get_zc_font(font_lfont_l)),"\x88",color_index*cs_hei+d->x,d->y + d->h + 3,jwin_pal[jcBOXFG],jwin_pal[jcBOX]);
			
			if((edit_cset_dlg[19].flags & D_SELECTED))
			{
				textprintf_centre_ex(screen,(get_zc_font(font_lfont_l)),d->x + d->w/2,d->y + d->h + 18,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Old: %2d - %3d %3d %3d",color_index, RAMpal[12*16+color_index].r*4,RAMpal[12*16+color_index].g*4,RAMpal[12*16+color_index].b*4);
				textprintf_centre_ex(screen,(get_zc_font(font_lfont_l)),d->x + d->w/2,d->y + d->h + 33,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"New: %2d - %3d %3d %3d",color_index, RAMpal[14*16+color_index].r*4,RAMpal[14*16+color_index].g*4,RAMpal[14*16+color_index].b*4);
			}
			else
			{
				textprintf_centre_ex(screen,(get_zc_font(font_lfont_l)),d->x + d->w/2,d->y + d->h + 18,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Old: %2d - %2d %2d %2d",color_index, RAMpal[12*16+color_index].r,RAMpal[12*16+color_index].g,RAMpal[12*16+color_index].b);
				textprintf_centre_ex(screen,(get_zc_font(font_lfont_l)),d->x + d->w/2,d->y + d->h + 33,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"New: %2d - %2d %2d %2d",color_index, RAMpal[14*16+color_index].r,RAMpal[14*16+color_index].g,RAMpal[14*16+color_index].b);
			}
			break;
	}
	return ret;
}

int32_t edit_cset_kb_handler(int32_t msg, DIALOG* d, int32_t c)
{
	ASSERT(d);
	int32_t ret = D_O_K;
	switch(msg)
	{
		case MSG_START:
			d->h = d->w = 0;
			break;
		case MSG_XCHAR:
		case MSG_CHAR:
			switch(c>>8)
			{
				case KEY_LEFT:
				case KEY_4:
				case KEY_4_PAD:
					if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
					{
						zc_swap(RAMpal[14*16+color_index], RAMpal[14*16+((color_index-1)&15)]);
					}
					if(color_index==0)
						color_index = 15;
					else --color_index;
					edit_cset_dlg[9].flags |= D_DIRTY;
					break;
				case KEY_RIGHT:
				case KEY_6:
				case KEY_6_PAD:
					if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
					{
						zc_swap(RAMpal[14*16+color_index], RAMpal[14*16+((color_index+1)&15)]);
					}
					if(color_index==15)
						color_index = 0;
					else ++color_index;
					edit_cset_dlg[9].flags |= D_DIRTY;
					break;
				case KEY_HOME:
				case KEY_7:
				case KEY_7_PAD:
					++RAMpal[14*16+color_index].r;
					RAMpal[14*16+color_index].r&=0x3F;
					edit_cset_dlg[9].flags |= D_DIRTY;
					break;
					
				case KEY_END:
				case KEY_1:
				case KEY_1_PAD:
					--RAMpal[14*16+color_index].r;
					RAMpal[14*16+color_index].r&=0x3F;
					edit_cset_dlg[9].flags |= D_DIRTY;
					break;
					
				case KEY_UP:
				case KEY_8:
				case KEY_8_PAD:
					++RAMpal[14*16+color_index].g;
					RAMpal[14*16+color_index].g&=0x3F;
					edit_cset_dlg[9].flags |= D_DIRTY;
					break;
					
				case KEY_DOWN:
				case KEY_2:
				case KEY_2_PAD:
					--RAMpal[14*16+color_index].g;
					RAMpal[14*16+color_index].g&=0x3F;
					edit_cset_dlg[9].flags |= D_DIRTY;
					break;
					
				case KEY_PGUP:
				case KEY_9:
				case KEY_9_PAD:
					++RAMpal[14*16+color_index].b;
					RAMpal[14*16+color_index].b&=0x3F;
					edit_cset_dlg[9].flags |= D_DIRTY;
					break;
					
				case KEY_PGDN:
				case KEY_3:
				case KEY_3_PAD:
					--RAMpal[14*16+color_index].b;
					RAMpal[14*16+color_index].b&=0x3F;
					edit_cset_dlg[9].flags |= D_DIRTY;
					break;
					
				case KEY_C:
					color_copy=color_index;
					edit_cset_dlg[9].flags |= D_DIRTY;
					break;
					
				case KEY_V:
					if(color_copy>=0)
					{
						RAMpal[14*16+color_index]=RAMpal[14*16+color_copy];
						color_copy=-1;
						edit_cset_dlg[9].flags |= D_DIRTY;
					}
					break;
					
				case KEY_M:
					if(color_copy>=0)
					{
						RGB temp=RAMpal[14*16+color_index];
						RAMpal[14*16+color_index]=RAMpal[14*16+color_copy];
						RAMpal[14*16+color_copy]=temp;
						color_copy=-1;
						edit_cset_dlg[9].flags |= D_DIRTY;
					}
					break;
					
				case KEY_I:
				case KEY_INSERT:
					RAMpal[14*16+color_index]=RAMpal[edc];
					edit_cset_dlg[9].flags |= D_DIRTY;
					break;
					
				case KEY_S:
					onSnapshot();
					break;
				default:
				{
					switch(c&255)
					{
						case '4':
						{
							if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
							{
								zc_swap(RAMpal[14*16+color_index], RAMpal[14*16+((color_index-1)&15)]);
							}
							
							color_index=(color_index-1)&15;
							break;
						}
						
						case '6':
						{
							if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
							{
								zc_swap(RAMpal[14*16+color_index], RAMpal[14*16+((color_index+1)&15)]);
							}
							
							color_index=(color_index+1)&15;
							break;
						}
						
						case '7':
							++RAMpal[14*16+color_index].r;
							RAMpal[14*16+color_index].r&=0x3F;
							break;
							
						case '1':
							--RAMpal[14*16+color_index].r;
							RAMpal[14*16+color_index].r&=0x3F;
							break;
							
						case '8':
							++RAMpal[14*16+color_index].g;
							RAMpal[14*16+color_index].g&=0x3F;
							break;
							
						case '2':
							--RAMpal[14*16+color_index].g;
							RAMpal[14*16+color_index].g&=0x3F;
							break;
							
						case '9':
							++RAMpal[14*16+color_index].b;
							RAMpal[14*16+color_index].b&=0x3F;
							break;
							
						case '3':
							--RAMpal[14*16+color_index].b;
							RAMpal[14*16+color_index].b&=0x3F;
							break;
						
						default:
							return ret;
					}
					edit_cset_dlg[9].flags |= D_DIRTY;
				}
			}
			ret |= D_USED_CHAR;
			broadcast_dialog_message(MSG_IDLE,0);
			
	}
	return ret;
}

void onInsertColor()
{
	RAMpal[14*16+color_index] = RAMpal[edc];
	zc_set_palette_range(RAMpal,0,255,false);
	edit_cset_dlg[9].flags |= D_DIRTY;
}

void onInsertColor_Text()
{
	int32_t r = atoi((char*)edit_cset_dlg[10].dp),
		g = atoi((char*)edit_cset_dlg[11].dp),
		b = atoi((char*)edit_cset_dlg[12].dp);
	if(edit_cset_dlg[19].flags & D_SELECTED)
	{
		r = vbound(r, 0, 252)/4;
		g = vbound(g, 0, 252)/4;
		b = vbound(b, 0, 252)/4;
	}
	else
	{
		r = vbound(r, 0, 63);
		g = vbound(g, 0, 63);
		b = vbound(b, 0, 63);
	}
	RAMpal[14*16+color_index].r = r;
	RAMpal[14*16+color_index].g = g;
	RAMpal[14*16+color_index].b = b;
	zc_set_palette_range(RAMpal,0,255,false);
	edit_cset_dlg[9].flags |= D_DIRTY;
}

void jumpText(int32_t r, int32_t g, int32_t b)
{
	if(edit_cset_dlg[19].flags & D_SELECTED)
	{
		r*=4;
		g*=4;
		b*=4;
	}
	char* c1 = (char*)edit_cset_dlg[10].dp;
	char* c2 = (char*)edit_cset_dlg[11].dp;
	char* c3 = (char*)edit_cset_dlg[12].dp;
	for(int32_t q = 0; q < 2; ++q)
	{
		c1[q] = 0;
		c2[q] = 0;
		c3[q] = 0;
	}
	sprintf(c1, "%d", r);
	sprintf(c2, "%d", g);
	sprintf(c3, "%d", b);
	object_message(&edit_cset_dlg[10], MSG_DRAW, 0);
	object_message(&edit_cset_dlg[11], MSG_DRAW, 0);
	object_message(&edit_cset_dlg[12], MSG_DRAW, 0);
}

void onJumpText()
{
	jumpText(RAMpal[14*16+color_index].r, RAMpal[14*16+color_index].g, RAMpal[14*16+color_index].b);
}

void onInsertColor_Hex()
{
	int32_t col = zc_xtoi((char*)edit_cset_dlg[21].dp);
	auto r = (col&0xFF0000)>>16;
	auto g = (col&0x00FF00)>>8;
	auto b = (col&0x0000FF)>>0;
	
	r = vbound(r, 0, 252)/4;
	g = vbound(g, 0, 252)/4;
	b = vbound(b, 0, 252)/4;
	RAMpal[14*16+color_index].r = r;
	RAMpal[14*16+color_index].g = g;
	RAMpal[14*16+color_index].b = b;
	zc_set_palette_range(RAMpal,0,255,false);
	edit_cset_dlg[9].flags |= D_DIRTY;
}

void jumpHex(int32_t r, int32_t g, int32_t b)
{
	r*=4;
	g*=4;
	b*=4;
	char* hexbuf = (char*)edit_cset_dlg[21].dp;
	sprintf(hexbuf, "%06X", (r<<16)|(g<<8)|(b<<0));
	object_message(&edit_cset_dlg[21], MSG_DRAW, 0);
}

void onJumpHex()
{
	jumpHex(RAMpal[14*16+color_index].r, RAMpal[14*16+color_index].g, RAMpal[14*16+color_index].b);
}

void onJumpHSL()
{
	
}

void init_gfxpal()
{
	for(int32_t i=0; i<128; i++)
	{
		RAMpal[i] = _RGB(gfx_pal+i*3); //hue
	}
	for(int32_t i=0; i<32; i++)
	{
		RAMpal[i+128] = _RGB(i<<1,i<<1,i<<1); //lightness
	}
}

bool edit_dataset(int32_t dataset)
{
	PALETTE holdpal;
	memcpy(holdpal,RAMpal,sizeof(RAMpal));
	
	rectfill(screen, 0, 0, screen->w, screen->h, 128);
	large_dialog(edit_cset_dlg);
	
	load_cset(RAMpal,12,dataset);
	load_cset(RAMpal,14,dataset);
	zc_set_palette_range(RAMpal,0,255,false);
	FONT* old = font;
	font = get_zc_font(font_lfont_l);
	
	init_gfxpal();
	char bufr[4] = "0", bufg[4] = "0", bufb[4] = "0";
	edit_cset_dlg[10].dp = (void*)bufr;
	edit_cset_dlg[11].dp = (void*)bufg;
	edit_cset_dlg[12].dp = (void*)bufb;
	char bufhex[7] = "000000";
	edit_cset_dlg[21].dp = (void*)bufhex;
	
	while(gui_mouse_b()) {
		rest(1);
	} //Do nothing
	edit_cset_dlg[0].dp2 = get_zc_font(font_lfont);
	int32_t ret = zc_popup_dialog(edit_cset_dlg,3);
	//al_trace("DLG RETURN VAL -------------------------- %d", ret);
	switch(ret)
	{
		case 3: //Cancel
			break;
		case 4: //OK
			get_cset(dataset,14,RAMpal);
			saved=false;
			break;
	}
	
	memcpy(RAMpal, holdpal, sizeof(holdpal));
	
	zc_set_palette(RAMpal);
	rectfill(screen, 0, 0, screen->w, screen->h, BLACK);
	while(gui_mouse_b()) {
		rest(1);
	} //Do nothing
	font = old;
	return ret==4;
}

int32_t pal_index(RGB *pal,RGB c)
{
    for(int32_t i=0; i<256; i++)
        if(pal[i].r==c.r && pal[i].g==c.g && pal[i].b==c.b)
            return i;
            
    return -1;
}

bool grab_dataset(int32_t dataset)
{
	int row=0;
	size_and_pos imagepos(4,4,654,548);
	size_and_pos palpos(665,200,16,16,15,15);
	int buttonx=570;
	int buttony=521;
	int filenamex=4;
	int filenamey=583;

	uint32_t picbackpal[256];
	ALLEGRO_COLOR a5picpal[256];
	uint32_t tmpbackpal[256];
	ALLEGRO_COLOR a5tmppal[256];
	PALETTE tmp;

	if(!pic && load_the_pic_new(&pic,picpal))
		return false;
	
	load_palette(picbackpal,a5picpal,picpal);
	
	char fname[65];
	extract_name(imagepath,fname,-1);
	
	ALLEGRO_BITMAP* oldtarg = al_get_target_bitmap();
	
	popup_zqdialog_start();
	auto* rti_image = add_dlg_layer(imagepos.x, imagepos.y, imagepos.tw(), imagepos.th());
	auto* rti_pal = add_dlg_layer(palpos.x-1, palpos.y-1, 2+palpos.tw(), 2+palpos.th());
	
	bool redraw=true;
	bool reload=false;
	int done=0;
	int f=0;
	FONT *fnt = font;
	
	font = get_zc_font(font_lfont_l);
	
	do
	{
		rest(1);
		int x=gui_mouse_x();
		int y=gui_mouse_y();
		
		custom_vsync();
		
		if(reload)
		{
			reload=false;
			
			if(load_the_pic_new(&pic,picpal)==2)
				done=1;
			else
			{
				load_palette(picbackpal,a5picpal,picpal);
				redraw=true;
				memset(fname,0,sizeof(fname));
				extract_name(imagepath,fname,-1);
			}
		}
		
		if(redraw)
		{
			redraw=false;
			clear_to_color(screen,jwin_pal[jcBOX]);
			
			jwin_draw_frame(screen,imagepos.x-2,imagepos.y-2,imagepos.tw()+4,imagepos.th()+4,FR_DEEP);
			jwin_draw_frame(screen,palpos.x-3,palpos.y-3,palpos.tw()+6,palpos.th()+6,FR_DEEP);
			
			textout_ex(screen,get_zc_font(font_lfont_l),fname,filenamex,filenamey,jwin_pal[jcBOXFG],jwin_pal[jcBOX]);
			draw_text_button(screen,buttonx,buttony+(36),(90),(31),"File",vc(0),vc(15),0,true);
			draw_text_button(screen,buttonx+(114),buttony,
							 (90),(31),"OK",vc(0),vc(15),0,true);
			draw_text_button(screen,buttonx+(114),buttony+(36),
							 (90),(31),"Cancel",vc(0),vc(15),0,true);
			
			al_set_target_bitmap(rti_image->bitmap);
			clear_a5_bmp(AL5_PINK);
			render_a4_a5(pic,0,0,0,0,std::min(pic->w,imagepos.tw()),std::min(pic->h,imagepos.th()),0,picbackpal);
		}
		
		if(gui_mouse_b()&1)
		{
			if(palpos.rect(x,y))
			{
				int oldrow = row;
				while(gui_mouse_b()&1)
				{
					row = vbound((gui_mouse_y()-palpos.y)/palpos.yscale,0,palpos.h-1);
					
					++f;
					al_set_target_bitmap(rti_pal->bitmap);
					if(oldrow != row)
					{
						oldrow = row;
						clear_a5_bmp();
						for(int i=0; i<256; i++)
						{
							auto& sqr = palpos.rel_subsquare(1,1,i);
							al_draw_filled_rectangle(sqr.x,sqr.y,sqr.x+sqr.w,sqr.y+sqr.h,a5picpal[i]);
						}
					}
					int gr = vbound(abs(((f*3)%510)-255),0x10,0xF0);
					al_draw_rectangle(0.5,(row*palpos.yscale)+0.5,palpos.tw()+0.5,((row+1)*palpos.yscale)+0.5,al_map_rgb(gr,gr,gr),0);
					rest(1);
					custom_vsync();
				}
			}
			
			if(isinRect(x,y,buttonx,buttony+(36),buttonx+(90),buttony+(36+31)))
				if(do_text_button(buttonx,buttony+(36),(90),(31),"File",vc(0),vc(15),true))
					reload=true;
			
			if(isinRect(x,y,buttonx+(114),buttony,buttonx+(114+90),buttony+(31)))
				if(do_text_button(buttonx+(114),buttony,(90),(31),"OK",vc(0),vc(15),true))
					done=2;
			
			if(isinRect(x,y,buttonx+(114),buttony+(36),buttonx+(114+90),buttony+(36+31)))
				if(do_text_button(buttonx+(114),buttony+(36),(90),(31),"Cancel",vc(0),vc(15),true))
					done=1;
		}
		
		if(keypressed())
		{
			switch(readkey()>>8)
			{
			case KEY_ESC:
				done=1;
				break;
				
			case KEY_ENTER:
				done=2;
				break;
				
			case KEY_SPACE:
				reload=true;
				break;
				
			case KEY_UP:
				row=(row-1)&15;
				break;
				
			case KEY_DOWN:
				row=(row+1)&15;
				break;
				
			case KEY_LEFT:
			case KEY_RIGHT:
				break;
				
			case KEY_TAB:
				memcpy(tmp,picpal,sizeof(picpal));
				
				for(int i=0; i<16; i++)
					tmp[(row*16)+i] = invRGB(tmp[(row*16)+i]);
				load_palette(tmpbackpal,a5tmppal,tmp);
				
				for(int i=0; i<12; i++)
				{
					bool t = i&2;
					custom_vsync();
					
					al_set_target_bitmap(rti_image->bitmap);
					render_a4_a5(pic,0,0,0,0,std::min(pic->w,imagepos.tw()),std::min(pic->h,imagepos.th()),0,t?picbackpal:tmpbackpal);
					al_set_target_bitmap(rti_pal->bitmap);
					for(int i=0; i<palpos.w; i++)
					{
						int c = i+row*16;
						auto& sqr = palpos.rel_subsquare(1,1,c);
						al_draw_filled_rectangle(sqr.x,sqr.y,sqr.x+sqr.w,sqr.y+sqr.h,(t?a5picpal:a5tmppal)[c]);
					}
				}
				
				break;
			}
		}
		
		++f;
		
		al_set_target_bitmap(rti_pal->bitmap);
		clear_a5_bmp();
		for(int i=0; i<256; i++)
		{
			auto& sqr = palpos.rel_subsquare(1,1,i);
			al_draw_filled_rectangle(sqr.x,sqr.y,sqr.x+sqr.w,sqr.y+sqr.h,a5picpal[i]);
		}
		int gr = vbound(abs(((f*3)%510)-255),0x10,0xF0);
		al_draw_rectangle(0.5,(row*palpos.yscale)+0.5,palpos.tw()+0.5,((row+1)*palpos.yscale)+0.5,al_map_rgb(gr,gr,gr),0);
	}
	while(!done);
	
	if(done==2)
		get_cset(dataset,row,picpal);
		
	font = fnt;
	
	while(gui_mouse_b())
		rest(1);
	
	popup_zqdialog_end();
	al_set_target_bitmap(oldtarg);
	return (done == 2);
}

byte cset_hold[15][16*3];
byte cset_hold_cnt;
bool cset_ready = false;
int32_t cset_count,cset_first;
PALETTE pal,undopal;
RGB** gUndoPal = NULL;

//#define FLASH 243
//byte rc[16] = {253,248,0,0,0,0,0,246,247,249,250,251,252,240,255,254};

void undo_pal()
{
	if(gUndoPal)
	{
		for(int32_t i=0; i<cset_count; i++)
			get_cset(cset_first+i,i,*gUndoPal);
		memcpy(*gUndoPal, pal, sizeof(RGB)*16*cset_count);
		return;
	}
	for(int32_t i=0; i<cset_count; i++)
		get_cset(cset_first+i,i,undopal);
	
	memcpy(undopal,pal,sizeof(pal));
}

void calc_dark(int32_t first)
{
    //  undopal=pal;
    memcpy(undopal,pal,sizeof(pal));
    
    PALETTE tmp;
    //  tmp = black_palette;
    memcpy(tmp,black_palette,sizeof(black_palette));
    
    fade_interpolate(pal,black_palette,tmp,16,0,47);
    
    for(int32_t i=0; i<48; i++)
        tmp[i+64] = tmp[i];
        
    fade_interpolate(pal,black_palette,tmp,32,0,47);
    
    for(int32_t i=0; i<48; i++)
        tmp[i+112] = tmp[i];
        
    tmp[160+3] = tmp[3];
    
    for(int32_t i=4; i<11; i++)
        get_cset(first+i,i,tmp);
}

static DIALOG cycle_dlg[] =
{
    // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
    { jwin_win_proc, 64,   48,   192+1,  152+1,  0,  0,  0,       D_EXIT,          0,             0, (void *) "Palette Cycles", NULL, NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { jwin_button_proc,     90,   176,  61,   21,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { jwin_button_proc,     170,  176,  61,   21,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { jwin_text_proc,       152,  72,   96,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "1   2   3", NULL, NULL },
    { jwin_text_proc,       88,   88,   56,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "CSet:", NULL, NULL },
    { jwin_text_proc,       88,   104,  56,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "First:", NULL, NULL },
    { jwin_text_proc,       88,   120,  56,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "Last:", NULL, NULL },
    { jwin_text_proc,       88,   136,  56,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "Count:", NULL, NULL },
    { jwin_text_proc,       88,   152,  56,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "Speed:", NULL, NULL },
    // 10
    { jwin_edit_proc,       152,  88,   24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          1,             0,       NULL, NULL, NULL },
    { d_hexedit_proc,    152,  104,  24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          1,             0,       NULL, NULL, NULL },
    { d_hexedit_proc,    152,  120,  24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          1,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,       152,  136,  24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,       152,  152,  24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          3,             0,       NULL, NULL, NULL },
    
    { jwin_edit_proc,       184,  88,   24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          1,             0,       NULL, NULL, NULL },
    { d_hexedit_proc,    184,  104,  24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          1,             0,       NULL, NULL, NULL },
    { d_hexedit_proc,    184,  120,  24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          1,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,       184,  136,  24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,       184,  152,  24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          3,             0,       NULL, NULL, NULL },
    
    { jwin_edit_proc,       216,  88,   24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          1,             0,       NULL, NULL, NULL },
    { d_hexedit_proc,    216,  104,  24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          1,             0,       NULL, NULL, NULL },
    { d_hexedit_proc,    216,  120,  24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          1,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,       216,  136,  24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,       216,  152,  24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          3,             0,       NULL, NULL, NULL },
    
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

void edit_cycles(int32_t level)
{
    char buf[15][8];
    palcycle cycle_none[1][3];  //create a null palette cycle here. -Z
	memset(cycle_none, 0, sizeof(cycle_none)); 
    for(int32_t i=0; i<3; i++)
    {
        palcycle c = ( level < 256 ) ? misc.cycles[level][i] : cycle_none[0][i]; //Only level palettes 0 through 255 have valid data in 2.50.x. -Z
        sprintf(buf[i*5  ],"%d",c.first>>4);
        sprintf(buf[i*5+1],"%X",c.first&15);
        sprintf(buf[i*5+2],"%X",c.count&15);
        sprintf(buf[i*5+3],"%d",c.count>>4);
        sprintf(buf[i*5+4],"%d",c.speed);
    }
    
    for(int32_t i=0; i<15; i++)
        cycle_dlg[i+10].dp = buf[i];
        
    large_dialog(cycle_dlg);
        
    if(zc_popup_dialog(cycle_dlg,3)==2)
    {
        saved=false;
        reset_pal_cycling();
        
        for(int32_t i=0; i<3; i++)
        {
            palcycle c;
            c.first = (atoi(buf[i*5])&7)<<4;
            c.first += zc_xtoi(buf[i*5+1])&15;
            c.count =  zc_xtoi(buf[i*5+2])&15;
            c.count += (atoi(buf[i*5+3])&15)<<4;
            c.speed =  atoi(buf[i*5+4]);
            misc.cycles[level][i] = c;
        }
    }
}

void draw_cset_proc(DIALOG *d)
{
    int32_t d1 = zc_min(d->d1, d->d2);
    int32_t d2 = zc_max(d->d1, d->d2);
    
    
    d_bitmap_proc(MSG_DRAW,d,0);
    //  text_mode(d->bg);
    
    //  rect(screen,d->x,(d1<<3)+d->y,d->x+d->w-1,(d2<<3)+d->y+7,FLASH);
    rect(screen,d->x,int32_t((d1<<3)*(1.5)+d->y),d->x+d->w-1,int32_t((d2<<3)*(1.5)+d->y+(11)),dvc(0)); //highlights currently selected cset
    int32_t drc;
    
    if((d->flags & D_GOTFOCUS))
        drc = (d->flags & D_DISABLED) ? jwin_pal[jcDISABLED_FG] : d->fg;
    else
        drc = d->bg;
        
    dotted_rect(d->x-1, d->y-1, d->x+d->w, d->y+d->h, drc, d->bg);
}

int32_t d_cset_proc(int32_t msg,DIALOG *d,int32_t c)
{
	switch(msg)
	{
		case MSG_START:
			d->d2 = d->d1;
			break;
			
		case MSG_WANTFOCUS:
			return D_WANTFOCUS;
			
		case MSG_DRAW:
			draw_cset_proc(d);
			break;
			
		case MSG_CLICK:
		{
			bool dragging=false;
			int32_t src=0;
			int32_t x;
			int32_t y;
			
			// Start dragging?
			if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
			{
				x=gui_mouse_x();
				y=gui_mouse_y();
				
				if(isinRect(x,y,d->x,d->y,d->x+d->w-1,d->y+d->h-1))
				{
					dragging=true;
					src=vbound((int32_t)((y-d->y) / (1.5))>>3,0,15) * 16 +
						vbound((int32_t)((x-d->x) / (1.5))>>3,0,15);
				}
			}
			
			do
			{
				x=gui_mouse_x();
				y=gui_mouse_y();
				
				if(!dragging && isinRect(x,y,d->x,d->y,d->x+d->w-1,d->y+d->h-1))
				{
					d->d2 = vbound((int32_t)((y-d->y)/(1.5))>>3,0,15);
					
					if(!(key_shifts&KB_SHIFT_FLAG))
						d->d1 = d->d2;
				}
				
				custom_vsync();
				draw_cset_proc(d);
				//sniggles
				//        ((RGB*)d->dp3)[243]=((RGB*)d->dp3)[rc[(fc++)&15]];
				//        zc_set_palette_range(((RGB*)d->dp3),FLASH,FLASH,false);
				((RGB*)d->dp3)[dvc(0)]=((RGB*)d->dp3)[zc_oldrand()%14+dvc(1)];
				zc_set_palette_range(((RGB*)d->dp3),dvc(0),dvc(0),false);
			}
			while(gui_mouse_b());
			
			if(dragging && isinRect(x,y,d->x,d->y,d->x+d->w-1,d->y+d->h-1))
			{
				int32_t dest=vbound((int32_t)((y-d->y) / (1.5))>>3,0,15) * 16 +
						 vbound((int32_t)((x-d->x) / (1.5))>>3,0,15);
						 
				if(src!=dest)
				{
					if(gUndoPal)
						memcpy(*gUndoPal, pal, sizeof(RGB)*16*cset_count);
					else memcpy(undopal,pal,sizeof(pal));
					
					if(key[KEY_LSHIFT] || key[KEY_RSHIFT])
					{
						for(int32_t i=0; i<3; i++)
							zc_swap(*(((byte*)d->dp2)+dest*3+i), *(((byte*)d->dp2)+src*3+i));
					}
					else
					{
						for(int32_t i=0; i<3; i++)
							*(((byte*)d->dp2)+dest*3+i) = *(((byte*)d->dp2)+src*3+i);
					}
					
					for(int32_t i=0; i<cset_count; i++)
						load_cset(pal,i,cset_first+i);
						
					zc_set_palette(pal);
					saved=false;
				}
			}
			GUI_EVENT(d, geCHANGE_SELECTION);
		}
		break;
		
		case MSG_CHAR:
		{
			int32_t shift = (key_shifts&KB_SHIFT_FLAG);
			int32_t k=c>>8;
			
			switch(k)
			{
				case KEY_UP:
					if(d->d2>0) --d->d2;
					
					if(!shift) d->d1 = d->d2;
					
					break;
					
				case KEY_DOWN:
					if(d->d2<((int32_t)ceil((d->h)/(1.5))>>3)-1)
						++d->d2;
						
					if(!shift) d->d1 = d->d2;
					
					break;
					
				case KEY_PGUP:
					d->d2=0;
					
					if(!shift) d->d1 = d->d2;
					
					break;
					
				case KEY_PGDN:
					d->d2=((int32_t)ceil((d->h)/(1.5))>>3)-1;
					
					if(!shift) d->d1 = d->d2;
					
					break;
					
				case KEY_C:
					cset_hold_cnt=0;
					
					for(int32_t row=0; row <= abs(d->d1 - d->d2); row++)
					{
						int32_t d1 = zc_min(d->d1,d->d2);
						++cset_hold_cnt;
						
						for(int32_t i=0; i<16*3; i++)
							cset_hold[row][i] = *(((byte*)d->dp2)+CSET(d1+row)*3+i);
					}
					
					cset_ready=true;
					break;
					
					
				case KEY_V:
					if(cset_ready)
					{
						if(gUndoPal)
							memcpy(*gUndoPal, pal, sizeof(RGB)*16*cset_count);
						else memcpy(undopal,pal,sizeof(pal));
						int32_t d1 = zc_min(d->d1, d->d2);
						for(int32_t row=0; row<cset_hold_cnt && d1+row<cset_count; row++)
						{
							for(int32_t i=0; i<16*3; i++)
								*(((byte*)d->dp2)+CSET(d1+row)*3+i) = cset_hold[row][i];
						}
						
						for(int32_t i=0; i<cset_count; i++)
							load_cset(pal,i,cset_first+i);
							
						zc_set_palette(pal);
						saved=false;
					}
					
					break;
					
				case KEY_U:
					undo_pal();
					
					for(int32_t i=0; i<cset_count; i++)
						load_cset(pal,i,cset_first+i);
						
					zc_set_palette(pal);
					break;
					
				default:
					return D_O_K;
			}
			
			custom_vsync();
			draw_cset_proc(d);
			GUI_EVENT(d, geCHANGE_SELECTION);
			return D_USED_CHAR;
		}
	}
	
	return D_O_K;
}

byte mainpal_csets[30]    = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14, 11,11,12,12,12,11, 10,10,10,12,10,10,10,10,9 };
byte levelpal_csets[26]   = { 2,3,4,9,2,3,4,2,3,4, 2, 3, 4,       15,15,15,15, 7,7,7, 8,8,8, 0,0,0 };
byte levelpal2_csets[26]  = { 2,3,4,9,2,0,1,2,3,4, 5, 6, 7,       15,15,15,15, 8,  9,9,9,9,9,9,9,9 };
byte spritepal_csets[30]  = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14, 14,14,14,14,14,14, 14,14,14,14,14,14,14,14,14 };
byte spritepal2_csets[30] = { 15,16,17,18,19,20,21,22,23,24,25,26,27,28,29, 14,14,14,14,14,14, 14,14,14,14,14,14,14,14,14 };

static DIALOG colors_dlg[] =
{
    // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
    { jwin_win_proc,     40,   16,   240+1,  216+1,  jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       D_EXIT,          0,             0,       NULL, NULL, NULL },
    
    { jwin_frame_proc,   103,  48,   96,  256,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          FR_DEEP,             0,       NULL, NULL, NULL },
    { d_cset_proc,       106,  51,   256,  96,   jwin_pal[jcBOXFG],  jwin_pal[jcLIGHT],  0,       0,          0,             0,       NULL, NULL, NULL },
    
    { jwin_rtext_proc,      102,   0,    16,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_rtext_proc,      102,   0,    16,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_rtext_proc,      102,   0,    16,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_rtext_proc,      102,   0,    16,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_rtext_proc,      102,   0,    16,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_rtext_proc,      102,   0,    16,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_rtext_proc,      102,   0,    16,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_rtext_proc,      102,   0,    16,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_rtext_proc,      102,   0,    16,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_rtext_proc,      102,   0,    16,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_rtext_proc,      102,   0,    16,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_rtext_proc,      102,   0,    16,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_rtext_proc,      102,   0,    16,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_rtext_proc,      102,   0,    16,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_rtext_proc,      102,   0,    16,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL, NULL, NULL },
    
    { d_dummy_proc,       104,  39,   128,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "0123456789ABCDEF", NULL, NULL },
    // 19
    
    { jwin_button_proc,     60,   178,  61,   21,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  'e',     D_EXIT,     0,             0, (void *) "&Edit", NULL, NULL },
    { jwin_button_proc,     130,  178,  61,   21,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  'g',     D_EXIT,     0,             0, (void *) "&Grab", NULL, NULL },
    { jwin_button_proc,     200,  178,  61,   21,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       D_EXIT,     0,             0, (void *) "Cycle", NULL, NULL },
    { jwin_button_proc,     244,  152,  25,   21,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  'u',     D_EXIT,     0,             0, (void *) "&U", NULL, NULL },
    // 23
    
    { jwin_button_proc,     60,   204,  131,  21,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  'l',     D_EXIT,     0,             0, (void *) "&Load to CS 9", NULL, NULL },
    
    { jwin_button_proc,     200,  204,  61,   21,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  27,      D_EXIT,     0,             0, (void *) "Done", NULL, NULL },
    { jwin_rtext_proc,      60,   164,  48,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "Name: ", NULL, NULL },
    { jwin_edit_proc,       88,  160, 136,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          16,            0,       NULL, NULL, NULL },
    { jwin_ctext_proc,       110,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "0", NULL, NULL },
    { jwin_ctext_proc,       118,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "1", NULL, NULL },
    { jwin_ctext_proc,       126,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "2", NULL, NULL },
    { jwin_ctext_proc,       134,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "3", NULL, NULL },
    { jwin_ctext_proc,       142,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "4", NULL, NULL },
    { jwin_ctext_proc,       150,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "5", NULL, NULL },
    { jwin_ctext_proc,       158,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "6", NULL, NULL },
    { jwin_ctext_proc,       166,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "7", NULL, NULL },
    { jwin_ctext_proc,       174,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "8", NULL, NULL },
    { jwin_ctext_proc,       182,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "9", NULL, NULL },
    { jwin_ctext_proc,       190,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "A", NULL, NULL },
    { jwin_ctext_proc,       198,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "B", NULL, NULL },
    { jwin_ctext_proc,       206,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "C", NULL, NULL },
    { jwin_ctext_proc,       214,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "D", NULL, NULL },
    { jwin_ctext_proc,       222,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "E", NULL, NULL },
    { jwin_ctext_proc,       230,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "F", NULL, NULL },
    //43
    { jwin_button_proc,     244,  125,  25,   21,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  's',     D_EXIT,     0,             0, (void *) "Save", NULL, NULL },
    
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

int32_t EditColors(const char *caption,int32_t first,int32_t count,byte *label)
{
	popup_zqdialog_start();
	zq_hide_screen(true);
    char tempstuff[17];
    cset_first=first;
    cset_count=count;
    
    for(int32_t i=240; i<256; i++)
    {
        pal[i] = RAMpal[i];
    }
    
    go();
    
    int32_t bw = 128;
    int32_t bh = count*8;
    
    if(colors_dlg[0].d1)
    {
        bw = (192);
        bh = count*(12);
    }
    
    BITMAP *bmp = create_bitmap_ex(8,(192),count*(12));
    
    if(!bmp)
        return 0;
        
    for(int32_t i=0; i<16*count; i++)
    {
        int32_t x=int32_t(((i&15)<<3)*(1.5));
        int32_t y=int32_t(((i>>4)<<3)*(1.5));
        rectfill(bmp,x,y,x+(15),y+(15),i);
    }
    
    colors_dlg[2].dp = bmp;
    colors_dlg[2].w  = bw;
    colors_dlg[2].h  = bh;
    colors_dlg[2].d1 = colors_dlg[2].d1 = 0;
    colors_dlg[2].dp2 = colordata+CSET(first)*3;
    colors_dlg[2].dp3 = pal;
    colors_dlg[1].w  = bw + 6;
    colors_dlg[1].h  = bh + 6;
    colors_dlg[21].proc = (count==pdLEVEL) ? jwin_button_proc : d_dummy_proc;
    //if the palette is > 255, disable button [21]
    colors_dlg[21].dp   = get_bit(quest_rules,qr_FADE) ? (void *) "Cycle" : (void *) "Dark";
    colors_dlg[26].dp   =  tempstuff;
    colors_dlg[25].x    =(count==pdLEVEL)?colors_dlg[0].x+60:colors_dlg[0].x+12;
    colors_dlg[25].w    =(count==pdLEVEL)?48:0;
    colors_dlg[25].dp   =(count==pdLEVEL)?(void *) "Name: ":(void *) " ";
    colors_dlg[26].proc =(count==pdLEVEL) ? jwin_edit_proc : d_dummy_proc;
    colors_dlg[26].x    =(count==pdLEVEL)?colors_dlg[0].x+63:colors_dlg[0].x+12;
    colors_dlg[26].w    =(count==pdLEVEL)?134:0;
    colors_dlg[26].d1   =(count==pdLEVEL)?16:0;
    colors_dlg[26].dp   =(count==pdLEVEL)?palnames[(first-poLEVEL)/pdLEVEL]:NULL;
    
    //char (*buf)[4]= new char[count][4];
    char buf[50][4];
    
    for(int32_t i=0; i<15; i++)
    {
        if(i<count)
        {
            sprintf(buf[i],"%2d",label[i]);
        }
        else
        {
            buf[i][0]=0;
        }
        
        colors_dlg[3+i].dp=buf[i];
        colors_dlg[3+i].y=colors_dlg[0].y+int32_t(((i<<3)+36-(3))*((colors_dlg[0].d1)?1.5:1));
        //sniggles
        //    colors_dlg[3+i].fg=rc[label[i+count]];
    }
    
    colors_dlg[0].dp  = (void *)caption;
    colors_dlg[0].dp2 = get_zc_font(font_lfont);
    
    for(int32_t i=0; i<count; i++)
    {
        load_cset(pal,i,i+first);
    }
    
    memcpy(undopal,pal,sizeof(pal));
	gUndoPal = NULL;
    int32_t ret=0;
    
    do
    {
        rest(4);
        
        for(int32_t i=0; i<count; i++)
        {
            load_cset(pal,i,i+first);
        }
        
        clear_to_color(screen,0);
        zc_set_palette(pal);
        colors_dlg[19].flags =
            colors_dlg[20].flags =
                colors_dlg[23].flags = D_EXIT;
                
		if(!colors_dlg[0].d1)
		{
			colors_dlg[2].x  += 0;
			colors_dlg[2].y  -= 1;
		}
		
		large_dialog(colors_dlg);
        
        DIALOG_PLAYER *p = init_dialog(colors_dlg,2);
        bool enable = true;
        
        while(update_dialog(p))
        {
            custom_vsync();
            //sniggles
            //      pal[FLASH]=pal[rc[(fc++)&15]];
            pal[dvc(0)]=pal[zc_oldrand()%14+dvc(1)];
            zc_set_palette_range(pal,dvc(0),dvc(0),false);
            
            bool en = (colors_dlg[2].d1 == colors_dlg[2].d2);
            
            if(en!=enable)
            {
                colors_dlg[19].flags =
                    colors_dlg[20].flags =
                        colors_dlg[23].flags = en  ? D_EXIT : D_DISABLED;
                broadcast_dialog_message(MSG_DRAW,0);
                enable = en;
            }
        }
        
        ret = shutdown_dialog(p);
        
        if(ret==19)
        {
            memcpy(undopal,pal,sizeof(pal));
            edit_dataset(first+colors_dlg[2].d2);
        }
        
        if(ret==20)
        {
            memcpy(undopal,pal,sizeof(pal));
            grab_dataset(first+colors_dlg[2].d2);
        }
        
        if(ret==21)
        {
		int32_t curpal = (first/pdLEVEL+poLEVEL)-10; 
		
		
		    if(!get_bit(quest_rules,qr_FADE))
		    {
			calc_dark(first);
		    }
		    else
		    {
			if ( curpal < 256 ) //don't display cycle data for palettes 256 through 511. They don't have valid cycle data. 
				edit_cycles((first-poLEVEL)/pdLEVEL);
			else jwin_alert("Notice","Palettes above 0xFF do not have Palette Cycles",NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
		    }
		
        }
        
        if(ret==22)
        {
            undo_pal();
        }
	if ( ret == 43 )
	{
		if(getname("Save Palette (.png)","png",NULL,datapath,false))
		{
			//bool cancel;
			//char buf[80],buf2[80],
			char name[13];
			extract_name(temppath,name,FILENAME8_3);
			save_bitmap(temppath, bmp, pal);
			//save_bitmap("current_lvl_pal.png", bmp, pal);
		}
	}
    }
    while(ret<23&&ret!=0);
    
    while(gui_mouse_b())
    {
        /* do nothing */
				rest(1);
    }
    
    clear_to_color(screen,vc(0));
    zc_set_palette(RAMpal);
    
    loadlvlpal(Color);
    
    if(ret==23)
    {
        load_cset(RAMpal,9,first+colors_dlg[2].d2);
        set_pal();
    }
    
    saved=false; //It's just easier this way :)
    //  gui_fg_color = vc(14);
    //  gui_bg_color = vc(1);
    
    comeback();
    destroy_bitmap(bmp);
    //delete[] buf;
	zq_hide_screen(false);
	popup_zqdialog_end();
    return int32_t(ret==23);
}

int32_t onColors_Main()
{
    int32_t l9 = EditColors("Main Palette",0,pdFULL-1,mainpal_csets);
    
    // copy main to level 0
    int32_t di = CSET(poLEVEL)*3;
    int32_t si = CSET(2)*3;
    
    for(int32_t i=0; i<CSET(3)*3; i++)
        colordata[di++] = colordata[si++];
        
    si = CSET(9)*3;
    
    for(int32_t i=0; i<16*3; i++)
        colordata[di++] = colordata[si++];
        
    loadlvlpal(Color);
    
    if(l9)
    {
        load_cset(RAMpal,9,colors_dlg[2].d2);
        set_pal();
    }
    
    return D_O_K;
}

void copyPal(int32_t src, int32_t dest)
{
    int32_t srcStart=CSET(src*pdLEVEL+poLEVEL)*3;
    int32_t destStart=CSET(dest*pdLEVEL+poLEVEL)*3;
    
    for(int32_t i=0; i<pdLEVEL*16*3; i++)
        colordata[destStart+i]=colordata[srcStart+i];
}

int32_t onColors_Levels()
{
	int32_t cycle = get_bit(quest_rules,qr_FADE);
	int32_t index=Map.getcolor();
	
	while((index=select_data("Select Level",index,levelnumlist,"Edit","Done",get_zc_font(font_lfont), copyPal))!=-1)
	{
		char buf[40];
		sprintf(buf,"Level %X Palettes",index);
		bool l9 = call_paledit_dlg(palnames[index], colordata+CSET(index*pdLEVEL+poLEVEL)*3, &pal, index*pdLEVEL+poLEVEL, index);
		//int32_t l9 = EditColors(buf,index*pdLEVEL+poLEVEL,pdLEVEL,cycle?levelpal2_csets:levelpal_csets);
		zc_set_palette(RAMpal);
		
		if(index==0)
		{
			// copy level 0 to main
			int32_t si = CSET(poLEVEL)*3;
			int32_t di = CSET(2)*3;
			
			for(int32_t i=0; i<CSET(3)*3; i++)
				colordata[di++] = colordata[si++];
				
			di = CSET(9)*3;
			
			for(int32_t i=0; i<16*3; i++)
				colordata[di++] = colordata[si++];
		}
		
		loadlvlpal(Color);
		
		if(l9)
		{
			load_cset(RAMpal,9,index*pdLEVEL+poLEVEL+colors_dlg[2].d2);
			set_pal();
		}
	}
	
	return D_O_K;
}

int32_t onColors_Sprites()
{
    int32_t index;
    
    do
    {
        index = jwin_alert3("Edit Palette", "Select an extra sprite","palette set to edit",NULL,"&1","&2","&Done",'1','2','d', get_zc_font(font_lfont));
        
        if(index==1)
            EditColors("Extra Sprite Palettes 1",poSPRITE255,15,spritepal_csets);
            
        if(index==2)
            EditColors("Extra Sprite Palettes 2",poSPRITE255+15,15,spritepal2_csets);
    }
    while(index==1 || index==2);
    
    return D_O_K;
}


void center_zq_cset_dialogs()
{
    jwin_center_dialog(cycle_dlg);
    jwin_center_dialog(colors_dlg);
	jwin_center_dialog(edit_cset_dlg);
}

