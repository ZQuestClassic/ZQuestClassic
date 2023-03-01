#include "selcombo_swatch.h"
#include "gui/common.h"
#include "gui/dialog.h"
#include "gui/dialog_runner.h"
#include "gui/size.h"
#include "gui/use_size.h"
#include "../jwin_a5.h"
#include <cassert>
#include <utility>
#include <fmt/format.h>
#include "tiles.h"

#include "zquest.h"
#include "zq_tiles.h"

int32_t newg_selcombo_proc(int32_t msg,DIALOG *d,int32_t)
{
	bool dis = d->flags & D_DISABLED;
	switch(msg)
	{
		case MSG_CLICK:
		{
			int32_t cmb = d->d1;
			int32_t cs = d->d2;
			bool ctrl = key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL];
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
				if(ctrl ? select_combo_3(cmb,cs) : select_combo_2(cmb,cs))
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
			int32_t tile = combobuf[d->d1].tile;
			int32_t cset = d->d2;
			int fl = combobuf[d->d1].flip;
			if(dis || !d->d1)
			{
				ALLEGRO_COLOR fill = jwin_a5_pal(jcBOX);
				al_draw_filled_rectangle(d->x,d->y,d->x+d->w,d->y+d->h,fill);
			}
			else
			{
				// Setup a5 buffer
				set_bitmap_create_flags(false);
				ALLEGRO_BITMAP *buf = al_create_bitmap(16,16);
				set_bitmap_create_flags(0);
				ALLEGRO_STATE old_state;
				al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP);
				al_set_target_bitmap(buf);
				auto* lock = al_lock_bitmap(buf,ALLEGRO_PIXEL_FORMAT_ANY,ALLEGRO_LOCK_READWRITE);
				// Draw the tile
				a5_draw_tile(0,0,tile,cset,0,fl);
				// Un-target the buffer
				al_unlock_bitmap(buf);
				al_restore_state(&old_state);
				al_draw_scaled_bitmap(buf, 0, 0, 16, 16, d->x+2, d->y+2, d->w-4, d->h-4, 0);
				al_destroy_bitmap(buf);
			}
			jwin_draw_frame_a5(d->x,d->y,d->w,d->h,dis ? FR_ETCHED : FR_DEEP);
			
			if(d->bg)
			{
				ALLEGRO_FONT *fonty = a5font;
				if(d->dp2) fonty = (ALLEGRO_FONT*)d->dp2;
				int fh = al_get_font_line_height(fonty);
				int32_t xo = 5;
				if(dis)
				{
					jwin_textout_a5_dis(fonty,jwin_a5_pal(jcDISABLED_FG),d->x+d->h+xo,d->y+2,0,fmt::format("Combo: {}",d->d1).c_str(),jwin_a5_pal(jcDISABLED_BG),jwin_a5_pal(jcLIGHT));
					jwin_textout_a5_dis(fonty,jwin_a5_pal(jcDISABLED_FG),d->x+d->h+xo,d->y+fh+3,0,fmt::format("CSet: {}",d->d2).c_str(),jwin_a5_pal(jcDISABLED_BG),jwin_a5_pal(jcLIGHT));
				}
				else
				{
					jwin_textout_a5(fonty,jwin_a5_pal(jcBOXFG),d->x+d->h+xo,d->y+2,0,fmt::format("Combo: {}",d->d1).c_str(),jwin_a5_pal(jcBOX));
					jwin_textout_a5(fonty,jwin_a5_pal(jcBOXFG),d->x+d->h+xo,d->y+fh+3,0,fmt::format("CSet: {}",d->d2).c_str(),jwin_a5_pal(jcBOX));
				}
			}
			break;
	}
	return D_O_K;
}

namespace GUI
{

SelComboSwatch::SelComboSwatch(): combo(0), cset(0),
	alDialog(), message(-1), showsVals(true)
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

void SelComboSwatch::applyFont_a5(ALLEGRO_FONT* newFont)
{
	if(alDialog)
	{
		alDialog->dp2 = newFont;
		pendDraw();
	}
	Widget::applyFont_a5(newFont);
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
		nullptr, widgFont_a5, nullptr // dp, dp2, dp3
	});
}

void SelComboSwatch::calculateSize()
{
	Size s = 32_px+4_px;
	setPreferredWidth(s + (showsVals ? 5_px+al_get_text_width(widgFont_a5, "Combo: 99999") : 0_px));
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
