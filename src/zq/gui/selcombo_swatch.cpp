#include "selcombo_swatch.h"
#include "gui/common.h"
#include "gui/dialog.h"
#include "gui/dialog_runner.h"
#include "gui/size.h"
#include "gui/use_size.h"
#include "gui/jwin.h"
#include <cassert>
#include <utility>
#include "tiles.h"

#include "zq/zquest.h"
#include "zq/zq_tiles.h"

int32_t newg_selcombo_proc(int32_t msg,DIALOG *d,int32_t)
{
	bool dis = d->flags & D_DISABLED;
	switch(msg)
	{
		case MSG_CLICK:
		{
			int32_t cmb = d->d1;
			int32_t cs = d->d2;
			bool ctrl = CHECK_CTRL_CMD;
			if(key[KEY_ALT])
			{
				if(!(key[KEY_LSHIFT] || key[KEY_RSHIFT]))
					d->d1 = Combo;
				d->d2 = CSet;
				GUI_EVENT(d, geCHANGE_SELECTION);
				return D_REDRAW;
			}
			else if(gui_mouse_b()&2) //rclick
			{
				d->d1 = 0;
				d->d2 = 0;
				GUI_EVENT(d, geCHANGE_SELECTION);
				return D_REDRAW;
			}
			else if(gui_mouse_b()&1)
			{
				if((ctrl ? select_combo_3 : select_combo_2)(cmb,cs))
				{
					d->d1 = cmb;
					d->d2 = cs;
					GUI_EVENT(d, geCHANGE_SELECTION);
					return D_REDRAW;
				}
			}
		}
		break;
		
		case MSG_DRAW:
			BITMAP *buf = create_bitmap_ex(8,20,20);
			BITMAP *bigbmp = create_bitmap_ex(8,d->h,d->h);
			
			if(buf && bigbmp)
			{
				if(dis)
					rectfill(bigbmp,0,0,d->h-1,d->h-1,jwin_pal[jcBOX]);
				else
				{
					clear_bitmap(buf);
					
					if(d->d1)
						overtile16(buf,combobuf[d->d1].tile,2,2,d->d2,combobuf[d->d1].flip);
						
					stretch_blit(buf, bigbmp, 2,2, 17, 17, 2, 2, d->h-4, d->h-4);
				}
				destroy_bitmap(buf);
				jwin_draw_frame(bigbmp,0,0,d->h,d->h,FR_DEEP);
				blit(bigbmp,screen,0,0,d->x,d->y,d->h,d->h);
				destroy_bitmap(bigbmp);
			}
			
			//    text_mode(d->bg);
			if(d->bg)
			{
				FONT *fonty = font;
				if(d->dp2) fonty = (FONT*)d->dp2;
				int32_t xo = 5;
				if(dis)
				{
					textprintf_ex(screen,fonty,d->x+d->h+xo+1,d->y+3,jwin_pal[jcLIGHT],jwin_pal[jcDISABLED_BG],"Combo: %d",d->d1);
					textprintf_ex(screen,fonty,d->x+d->h+xo,d->y+2,jwin_pal[jcDISABLED_FG],-1,"Combo: %d",d->d1);
					
					textprintf_ex(screen,fonty,d->x+d->h+xo+1,d->y+text_height(fonty)+4,jwin_pal[jcLIGHT],jwin_pal[jcDISABLED_BG],"CSet: %d",d->d2);
					textprintf_ex(screen,fonty,d->x+d->h+xo,d->y+text_height(fonty)+3,jwin_pal[jcDISABLED_FG],-1,"CSet: %d",d->d2);
				}
				else
				{
					textprintf_ex(screen,fonty,d->x+d->h+xo,d->y+2,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Combo: %d",d->d1);
					textprintf_ex(screen,fonty,d->x+d->h+xo,d->y+text_height(fonty)+3,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"CSet: %d",d->d2);
				}
			}
			break;
	}
	return D_O_K;
}

namespace GUI
{

SelComboSwatch::SelComboSwatch(): message(-1), combo(0),
	cset(0), showsVals(true), alDialog()
{
	Size s = 32_px+4_px;
	setPreferredWidth(s);
	setPreferredHeight(s);
}

void SelComboSwatch::setCombo(int32_t value)
{
	combo = value;
	if(alDialog)
	{
		alDialog->d1 = value;
		pendDraw();
	}
}

void SelComboSwatch::setCSet(int32_t value)
{
	cset = value;
	if(alDialog)
	{
		alDialog->d2 = value;
		pendDraw();
	}
}

void SelComboSwatch::setShowVals(bool val)
{
	showsVals = val;
	if(alDialog)
	{
		alDialog->bg = val?1:0;
		pendDraw();
	}
}

int32_t SelComboSwatch::getCombo()
{
	return alDialog ? alDialog->d1 : combo;
}

int32_t SelComboSwatch::getCSet()
{
	return alDialog ? alDialog->d2 : cset;
}

void SelComboSwatch::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	if(alDialog) alDialog.applyVisibility(visible);
}

void SelComboSwatch::applyDisabled(bool dis)
{
	Widget::applyDisabled(dis);
	if(alDialog) alDialog.applyDisabled(dis);
}

void SelComboSwatch::applyFont(FONT* newFont)
{
	if(alDialog)
	{
		alDialog->dp2 = newFont;
		pendDraw();
	}
	Widget::applyFont(newFont);
}

void SelComboSwatch::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	alDialog = runner.push(shared_from_this(), DIALOG {
		newGUIProc<newg_selcombo_proc>,
		x, y, getHeight(), getHeight(),
		fgColor, showsVals?1:0,
		0,
		getFlags(),
		combo, cset, // d1, d2,
		nullptr, widgFont, nullptr // dp, dp2, dp3
	});
}

void SelComboSwatch::calculateSize()
{
	Size s = 32_px+4_px;
	setPreferredWidth(s + (showsVals ? 5_px+text_length(widgFont, "Combo: 99999") : 0_px));
	setPreferredHeight(s);
	Widget::calculateSize();
}

int32_t SelComboSwatch::onEvent(int32_t event, MessageDispatcher& sendMessage)
{
	assert(event == geCHANGE_SELECTION);
	if(onSelectFunc)
		onSelectFunc(alDialog->d1, alDialog->d2);
	if(message >= 0)
		sendMessage(message, alDialog->d1);
	return -1;
}

}
