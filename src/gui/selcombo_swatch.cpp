#include "selcombo_swatch.h"
#include "common.h"
#include "dialog.h"
#include "dialog_runner.h"
#include "size.h"
#include "../jwin.h"
#include "../zquest.h"
#include <cassert>
#include <utility>
#include "tiles.h"

#ifdef IS_ZQUEST
#include "zq_tiles.h"
#endif

int32_t newg_selcombo_proc(int32_t msg,DIALOG *d,int32_t)
{
#ifdef IS_ZQUEST
	switch(msg)
	{
		case MSG_CLICK:
		{
			int32_t cmb = d->d1;
			int32_t cs = d->d2;
			
			if(select_combo_2(cmb,cs))
			{
				d->d1 = cmb;
				d->d2 = cs;
				GUI_EVENT(d, geCHANGE_SELECTION);
				return D_REDRAW;
			}
		}
		break;

		case MSG_DRAW:
			BITMAP *buf = create_bitmap_ex(8,20,20);
			BITMAP *bigbmp = create_bitmap_ex(8,d->h,d->h);
			
			if(buf && bigbmp)
			{
				clear_bitmap(buf);
				
				if(d->d1)
					overtile16(buf,combobuf[d->d1].o_tile,2,2,d->d2,0);
					
				stretch_blit(buf, bigbmp, 2,2, 17, 17, 2, 2, d->h-4, d->h-4);
				destroy_bitmap(buf);
				jwin_draw_frame(bigbmp,0,0,d->h,d->h,FR_DEEP);
				blit(bigbmp,screen,0,0,d->x,d->y,d->h,d->h);
				destroy_bitmap(bigbmp);
			}
			
			//    text_mode(d->bg);
			if(d->bg)
			{
				FONT *fonty = (is_large ? font : pfont);
				if(d->dp2) fonty = (FONT*)d->dp2;
				textprintf_ex(screen,fonty,d->x+d->h,d->y+2,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Combo: %d",d->d1);
				textprintf_ex(screen,fonty,d->x+d->h,d->y+text_height(fonty)+3,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"CSet: %d",d->d2);
			}
			break;
	}
#endif
	return D_O_K;
}

namespace GUI
{

SelComboSwatch::SelComboSwatch(): combo(0), cset(0),
	alDialog(), message(-1), showsVals(true)
{
	Size s = sized(16_px,32_px)+4_px;
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
	Size s = sized(16_px,32_px)+4_px;
	setPreferredWidth(s + (showsVals ? text_length(widgFont, "Combo: 99999") : 0));
	setPreferredHeight(s);
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
