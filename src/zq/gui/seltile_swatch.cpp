#include "seltile_swatch.h"
#include "gui/common.h"
#include "gui/dialog.h"
#include "gui/dialog_runner.h"
#include "gui/size.h"
#include "../jwin_a5.h"
#include "zquest.h"
#include <cassert>
#include <utility>
#include "tiles.h"
#include "zq_tiles.h"
#include <fmt/format.h>

int32_t newg_seltile_proc(int32_t msg,DIALOG *d,int32_t)
{
	bool dis = d->flags & D_DISABLED;
	GUI::SelTileSwatch* ptr = (GUI::SelTileSwatch*)d->dp3;
	switch(msg)
	{
		case MSG_START:
		{
			ptr->calc_selcolor();
			break;
		}
		case MSG_CLICK:
		{
			if(gui_mouse_b()&1) //leftmouse
			{
				if(ptr->getIsMini() && ptr->getMiniOnly()) break;
				int32_t f = d->fg;
				int32_t t = d->d1;
				int32_t cs = d->d2;
				bool showflip = d->bg & 0b10;
				
				if(select_tile(t,f,1,cs,true,0,showflip))
				{
					d->d1 = t;
					d->d2 = cs;
					d->fg = f;
					ptr->calc_selcolor();
					GUI_EVENT(d, geCHANGE_SELECTION);
					return D_REDRAW;
				}
			}
			else if(gui_mouse_b()&2) //right mouse
			{
				if(!ptr->getIsMini()) break;
				int32_t tw = (ptr ? ptr->getTileWid() : 1);
				int32_t th = (ptr ? ptr->getTileHei() : 1);
				int32_t old = ptr->getMiniCrn();
				int32_t mx=vbound(gui_mouse_x()-d->x,0,d->w-1);
				int32_t my=vbound(gui_mouse_y()-d->y,0,d->h-1);
				mx = (mx*2*tw)/d->w; //0 to tw-1
				my = (my*2*th)/d->h; //0 to th-1
				int32_t crn = ((my&1)<<1) | (mx&1);
				if(tw > 1 || th > 1)
					crn += ((mx/2)+((my/2)*tw))<<2;
				
				ptr->setMiniCrn(crn);
				if(old != crn)
				{
					ptr->calc_selcolor();
					GUI_EVENT(d, geCHANGE_SELECTION);
					return D_REDRAW;
				}
			}
		}
		break;

		case MSG_DRAW:
		{
			int32_t tw = (ptr ? ptr->getTileWid() : 1);
			int32_t th = (ptr ? ptr->getTileHei() : 1);
			int32_t tile = (d->d1 ? d->d1 : ptr->getDefTile());
			int32_t cset = (d->d1 ? d->d2 : ptr->getDefCS());
			if(dis || !(tile || ptr->getShowT0()))
			{
				ALLEGRO_COLOR fill = dis ? jwin_a5_pal(jcBOX) : a5color(CSET(cset));
				al_draw_filled_rectangle(d->x,d->y,d->x+d->w,d->y+d->h,fill);
			}
			else
			{
				// Setup a5 buffer
				set_bitmap_create_flags(false);
				ALLEGRO_BITMAP *buf = al_create_bitmap(tw*16,th*16);
				set_bitmap_create_flags(0);
				ALLEGRO_STATE old_state;
				al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP);
				al_set_target_bitmap(buf);
				auto* lock = al_lock_bitmap(buf,ALLEGRO_PIXEL_FORMAT_ANY,ALLEGRO_LOCK_READWRITE);
				// Draw the tile(s)
				for(auto tx = 0; tx < tw; ++tx)
				{
					for(auto ty = 0; ty < th; ++ty)
					{
						int32_t tmptile = tile+tx+(TILES_PER_ROW*ty);
						
						a5_draw_tile(tx*16,ty*16,tmptile,cset,0,0,false);
					}
				}
				//Unlock and handle minitile
				al_unlock_bitmap(buf);
				if(ptr->getIsMini()) //Minitile corner
				{
					int32_t crn = ptr->getMiniCrn()% 4;
					int32_t cx = (2+((crn&1)?8:0));
					int32_t cy = (2+((crn&2)?8:0));
					int32_t toffs = (ptr->getMiniCrn() / 4);
					cx += (toffs%tw) * 16;
					cy += (toffs/tw) * 16;
					al_draw_rectangle(cx,cy,cx+7,cy+7,ptr->sel_color,1);
				}
				// Un-target the buffer
				al_restore_state(&old_state);
				al_draw_scaled_bitmap(buf, 0, 0, tw*16, th*16, d->x+2, d->y+2, d->w-4, d->h-4, 0);
				al_destroy_bitmap(buf);
			}
			jwin_draw_frame_a5(d->x,d->y,d->w,d->h,dis ? FR_ETCHED : FR_DEEP);
			
			if(d->bg & 0b1)
			{
				ALLEGRO_FONT *fonty = a5font;
				if(d->dp2) fonty = (ALLEGRO_FONT*)d->dp2;
				int fh = al_get_font_line_height(fonty);
				if(dis)
				{
					jwin_textout_a5_dis(fonty,jwin_a5_pal(jcDISABLED_FG),d->x+d->w,d->y+2,0,fmt::format("Tile: {}",d->d1).c_str(),jwin_a5_pal(jcDISABLED_BG),jwin_a5_pal(jcLIGHT));
					jwin_textout_a5_dis(fonty,jwin_a5_pal(jcDISABLED_FG),d->x+d->w,d->y+fh+3,0,fmt::format("CSet: {}",d->d2).c_str(),jwin_a5_pal(jcDISABLED_BG),jwin_a5_pal(jcLIGHT));
				}
				else
				{
					jwin_textout_a5(fonty,jwin_a5_pal(jcBOXFG),d->x+d->w,d->y+2,0,fmt::format("Tile: {}",d->d1).c_str(),jwin_a5_pal(jcBOX));
					jwin_textout_a5(fonty,jwin_a5_pal(jcBOXFG),d->x+d->w,d->y+fh+3,0,fmt::format("CSet: {}",d->d2).c_str(),jwin_a5_pal(jcBOX));
				}
			}
		}
		break;
	}

	return D_O_K;
}

namespace GUI
{

SelTileSwatch::SelTileSwatch(): tile(0), cset(0), flip(0),
	tw(1), th(1), isMini(false), mini_crn(0),
	showFlip(false), showsVals(true),
	alDialog(), message(-1), showT0(false),
	minionly(false), deftile(0), defcs(0)
{
	sel_color = jwin_a5_pal(jcTITLER);
	Size s = 32_px+4_px;
	setPreferredWidth(s);
	setPreferredHeight(s);
}

void SelTileSwatch::click()
{
	if(alDialog && allowDraw())
	{
		if(alDialog.message(MSG_CLICK, 0) & D_REDRAW)
		{
			pendDraw();
		}
	}
}

void SelTileSwatch::setTile(int32_t value)
{
	tile = value;
	if(alDialog)
	{
		alDialog->d1 = value;
		pendDraw();
	}
}

void SelTileSwatch::setCSet(int32_t value)
{
	cset = value;
	if(alDialog)
	{
		alDialog->d2 = value;
		calc_selcolor();
		pendDraw();
	}
}

void SelTileSwatch::setFlip(int32_t value)
{
	flip = value;
	if(alDialog)
	{
		alDialog->fg = value;
		pendDraw();
	}
}

void SelTileSwatch::setIsMini(bool val)
{
	isMini = val;
	if(val) calc_selcolor();
}
void SelTileSwatch::setShowT0(bool val)
{
	showT0 = val;
	if(alDialog && !alDialog->d1)
		calc_selcolor();
}

void SelTileSwatch::setMiniCrn(int32_t val)
{
	mini_crn = vbound(val,0,(4*tw*th)-1);
}
void SelTileSwatch::setTileWid(int32_t val)
{
	tw = std::max(1,val);
	if(alDialog)
	{
		int32_t wid = (tw*32)+4;
		alDialog->w = wid;
		mini_crn %= (4*tw*th);
	}
}
void SelTileSwatch::setTileHei(int32_t val)
{
	th = std::max(1,val);
	if(alDialog)
	{
		int32_t hei = (th*32)+4;
		alDialog->h = hei;
		mini_crn %= (4*tw*th);
	}
}

void SelTileSwatch::setShowFlip(bool val)
{
	showFlip = val;
	if(alDialog)
	{
		SETFLAG(alDialog->bg, 0b10, val);
		pendDraw();
	}
}

void SelTileSwatch::setShowVals(bool val)
{
	showsVals = val;
	if(alDialog)
	{
		SETFLAG(alDialog->bg, 0b1, val);
		pendDraw();
	}
}

int32_t SelTileSwatch::getTile()
{
	return alDialog ? alDialog->d1 : tile;
}

int32_t SelTileSwatch::getCSet()
{
	return alDialog ? alDialog->d2 : cset;
}

int32_t SelTileSwatch::getFlip()
{
	return alDialog ? alDialog->fg : flip;
}

void SelTileSwatch::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	if(alDialog) alDialog.applyVisibility(visible);
}

void SelTileSwatch::applyDisabled(bool dis)
{
	Widget::applyDisabled(dis);
	if(alDialog) alDialog.applyDisabled(dis);
}

void SelTileSwatch::applyFont_a5(ALLEGRO_FONT* newFont)
{
	if(alDialog)
	{
		alDialog->dp2 = newFont;
		pendDraw();
	}
	Widget::applyFont_a5(newFont);
}

void SelTileSwatch::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	int32_t wid = (tw*32)+4;
	int32_t hei = (th*32)+4;
	alDialog = runner.push(shared_from_this(), DIALOG {
		newGUIProc<newg_seltile_proc>,
		x, y, wid, hei,
		flip, (showsVals?0b1:0) | (showFlip?0b10:0),
		0,
		getFlags(),
		tile, cset, // d1, d2,
		nullptr, widgFont_a5, this // dp, dp2, dp3
	});
}

void SelTileSwatch::calculateSize()
{
	if(isMini)
		tw = th = 1;
	Size w = (tw*32_px)+4_px;
	Size h = (th*32_px)+4_px;
	setPreferredWidth(w + (showsVals ? al_get_text_width(widgFont_a5, "Tile: 999999") : 0));
	setPreferredHeight(h);
	Widget::calculateSize();
}

void SelTileSwatch::calc_selcolor()
{
	if(!alDialog) return;
	if(!isMini) return;
	
	int32_t tile = (alDialog->d1 ? alDialog->d1 : deftile);
	int32_t cset = (alDialog->d1 ? alDialog->d2 : defcs);
	if(tile || showT0)
	{
		PALETTE tpal;
		get_palette(tpal);
		
		byte buf[8*8];
		load_minitile(buf, tile, mini_crn);
		
		int32_t r = 0, g = 0, b = 0, count = 0;
		for(int32_t ty = 1; ty < 7; ++ty)
		{
			for(int32_t tx = 1; tx < 7; ++tx)
			{
				RGB& c = tpal[tx+(ty*8)];
				r += c.r;
				b += c.b;
				g += c.g;
				++count;
			}
		}
		unsigned char nr = r*4/count, ng = g*4/count, nb = b*4/count;
		
		sel_color = getHighlightColor(al_map_rgb(nr,ng,nb));
	}
	else
		sel_color = getHighlightColor(a5color(0));
	
}

int32_t SelTileSwatch::onEvent(int32_t event, MessageDispatcher& sendMessage)
{
	assert(event == geCHANGE_SELECTION);
	if(onSelectFunc)
		onSelectFunc(alDialog->d1, alDialog->d2, alDialog->fg, mini_crn);
	if(message >= 0)
		sendMessage(message, alDialog->d1);
	return -1;
}

}
