#include "seltile_swatch.h"
#include "gui/common.h"
#include "gui/dialog.h"
#include "gui/dialog_runner.h"
#include "gui/size.h"
#include "../jwin.h"
#include "zquest.h"
#include <cassert>
#include <utility>
#include "tiles.h"
#include "zq_tiles.h"
#include <allegro/internal/aintern.h>

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
			BITMAP *buf = create_bitmap_ex(8,16*tw+4,16*th+4);
			BITMAP *bigbmp = create_bitmap_ex(8,d->w,d->h);
			
			if(buf && bigbmp)
			{
				clear_bitmap(buf);
				if(dis)
					rectfill(bigbmp,0,0,d->h-1,d->h-1,jwin_pal[jcBOX]);
				else
				{
					int32_t tile = (d->d1 ? d->d1 : ptr->getDefTile());
					int32_t cset = (d->d1 ? d->d2 : ptr->getDefCS());
					if(tile || ptr->getShowT0())
					{
						if(tw > 1 || th > 1)
							overtileblock16(buf,tile,2,2,tw,th,cset,d->fg);
						else overtile16(buf,tile,2,2,cset,d->fg);
					}
					
					if(ptr->getIsMini()) //Minitile corner
					{
						int32_t crn = ptr->getMiniCrn()% 4;
						int32_t cx = (2+((crn&1)?8:0));
						int32_t cy = (2+((crn&2)?8:0));
						int32_t toffs = (ptr->getMiniCrn() / 4);
						cx += (toffs%tw) * 16;
						cy += (toffs/tw) * 16;
						rect(buf,cx,cy,cx+7,cy+7,ptr->sel_color);
					}
				}
				
				stretch_blit(buf, bigbmp, 2,2, (16*tw), (16*th), 2, 2, d->w-4, d->h-4);
				destroy_bitmap(buf);
				jwin_draw_frame(bigbmp,0,0,d->w,d->h,dis ? FR_ETCHED : FR_DEEP);
				blit(bigbmp,screen,0,0,d->x,d->y,d->w,d->h);
				destroy_bitmap(bigbmp);
			}
			
			//    text_mode(d->bg);
			if(d->bg & 0b1)
			{
				FONT *fonty = font;
				if(d->dp2) fonty = (FONT*)d->dp2;
				if(dis)
				{
					textprintf_ex(screen,fonty,d->x+d->w+1,d->y+3,jwin_pal[jcLIGHT],jwin_pal[jcDISABLED_BG],"Tile: %d",d->d1);
					textprintf_ex(screen,fonty,d->x+d->w,d->y+2,jwin_pal[jcDISABLED_FG],-1,"Tile: %d",d->d1);
					
					textprintf_ex(screen,fonty,d->x+d->w+1,d->y+text_height(fonty)+4,jwin_pal[jcLIGHT],jwin_pal[jcDISABLED_BG],"CSet: %d",d->d2);
					textprintf_ex(screen,fonty,d->x+d->w,d->y+text_height(fonty)+3,jwin_pal[jcDISABLED_FG],-1,"CSet: %d",d->d2);
				}
				else
				{
					textprintf_ex(screen,fonty,d->x+d->w,d->y+2,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Tile: %d",d->d1);
					textprintf_ex(screen,fonty,d->x+d->w,d->y+text_height(fonty)+3,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"CSet: %d",d->d2);
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
	sel_color = jwin_pal[jcTITLER];
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

void SelTileSwatch::applyFont(FONT* newFont)
{
	if(alDialog)
	{
		alDialog->dp2 = newFont;
		pendDraw();
	}
	Widget::applyFont(newFont);
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
		nullptr, widgFont, this // dp, dp2, dp3
	});
}

void SelTileSwatch::calculateSize()
{
	if(isMini)
		tw = th = 1;
	Size w = (tw*32_px)+4_px;
	Size h = (th*32_px)+4_px;
	setPreferredWidth(w + (showsVals ? text_length(widgFont, "Tile: 999999") : 0));
	setPreferredHeight(h);
	Widget::calculateSize();
}

void SelTileSwatch::calc_selcolor()
{
	if(!alDialog) return;
	if(!isMini) return;
	BITMAP* buf = create_bitmap_ex(8,8,8);
	
	RGB col;
	int32_t tile = (alDialog->d1 ? alDialog->d1 : deftile);
	int32_t cset = (alDialog->d1 ? alDialog->d2 : defcs);
	if(tile || showT0)
	{
		
		int32_t crn = mini_crn % 4;
		int32_t cx = (2+((crn&1)?8:0));
		int32_t cy = (2+((crn&2)?8:0));
		int32_t toffs = (mini_crn / 4);
		cx += (toffs%tw) * 16;
		cy += (toffs/tw) * 16;
		if(tw > 1 || th > 1)
			overtileblock16(buf,tile,-cx,-cy,tw,th,cset,alDialog->fg);
		else overtile16(buf,tile,-cx,-cy,cset,alDialog->fg);
		int32_t r = 0, g = 0, b = 0, count = 0;
		for(int32_t ty = 1; ty < 7; ++ty)
		{
			uintptr_t read_addr = bmp_read_line(buf, ty);
			for(int32_t tx = 1; tx < 7; ++tx)
			{
				RGB foo;
				get_color(bmp_read8(read_addr+tx), &foo);
				r += foo.r;
				b += foo.b;
				g += foo.g;
				++count;
			}
		}
		bmp_unwrite_line(buf);
		col.r = r/count;
		col.g = g/count;
		col.b = b/count;
	}
	else get_color(0, &col);
	
	sel_color = getHighlightColor(col);
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
