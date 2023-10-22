#include "cornerselect.h"
#include "gui/common.h"
#include "gui/dialog.h"
#include "gui/dialog_runner.h"
#include "gui/size.h"
#include "jwin.h"
#include <cassert>
#include <utility>
#include "tiles.h"

int32_t newg_cornersel_proc(int32_t msg,DIALOG *d,int32_t)
{
	switch(msg)
	{
		case MSG_CLICK:
		{
			int32_t bw = (d->w-4)/2, bh = (d->h-4)/2;
			bool shift = (key_shifts & KB_SHIFT_FLAG);
			bool ctrl = (key_shifts & KB_CTRL_FLAG);
			while(gui_mouse_b())
			{
				/*Do nothing*/
				rest(1);
			}
			int32_t bit = 0;
			if(mouse_in_rect(d->x+2,d->y+2,bw,bh))
				bit = 0b0001;
			else if(mouse_in_rect(d->x+2+bw,d->y+2,bw,bh))
				bit = 0b0010;
			else if(mouse_in_rect(d->x+2,d->y+2+bh,bw,bh))
				bit = 0b0100;
			else if(mouse_in_rect(d->x+2+bw,d->y+2+bh,bw,bh))
				bit = 0b1000;
			else return D_O_K;
			
			if(ctrl) //toggle all bits
				d->d1 ^= 0b1111;
			else if(shift) //set all bits to opposite of clicked bit
			{
				d->d1 = (d->d1&bit) ? 0b0000 : 0b1111;
			}
			else d->d1 ^= bit; //toggle one bit
			GUI_EVENT(d, geCHANGE_SELECTION);
			return D_REDRAW;
		}
		break;

		case MSG_DRAW:
		{
			BITMAP *buf = create_bitmap_ex(8,20,20);
			BITMAP *bigbmp = create_bitmap_ex(8,d->h,d->h);
			
			if(buf && bigbmp)
			{
				clear_bitmap(buf);
				rectfill(buf, 2, 2, 17, 17, d->bg);
				if(d->d1 & 0b0001)
					rectfill(buf,  2,  2,  9,  9, d->fg);
				if(d->d1 & 0b0010)
					rectfill(buf, 10,  2, 17,  9, d->fg);
				if(d->d1 & 0b0100)
					rectfill(buf,  2, 10,  9, 17, d->fg);
				if(d->d1 & 0b1000)
					rectfill(buf, 10, 10, 17, 17, d->fg);
					
				stretch_blit(buf, bigbmp, 2,2, 17, 17, 2, 2, d->h-4, d->h-4);
				destroy_bitmap(buf);
				jwin_draw_frame(bigbmp,0,0,d->h,d->h,FR_DEEP);
				blit(bigbmp,screen,0,0,d->x,d->y,d->h,d->h);
				destroy_bitmap(bigbmp);
			}
		}
		break;
	}

	return D_O_K;
}

namespace GUI
{

CornerSwatch::CornerSwatch(): message(-1),
	val(0), alDialog()
{
	Size s = 32_px+4_px;
	setPreferredWidth(s);
	setPreferredHeight(s);
	bgColor = vc(7);
}

void CornerSwatch::setVal(int32_t value)
{
	val = value&0xF;
	if(alDialog)
	{
		alDialog->d1 = val;
		pendDraw();
	}
}

void CornerSwatch::setColor(int32_t value)
{
	fgColor = value;
	if(alDialog)
	{
		alDialog->fg = value;
		pendDraw();
	}
}

int32_t CornerSwatch::getVal()
{
	return alDialog ? alDialog->d1 : val;
}

int32_t CornerSwatch::getColor()
{
	return alDialog ? alDialog->fg : fgColor;
}

void CornerSwatch::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	if(alDialog) alDialog.applyVisibility(visible);
}

void CornerSwatch::applyDisabled(bool dis)
{
	Widget::applyDisabled(dis);
	if(alDialog) alDialog.applyDisabled(dis);
}

void CornerSwatch::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	alDialog = runner.push(shared_from_this(), DIALOG {
		newGUIProc<newg_cornersel_proc>,
		x, y, getHeight(), getHeight(),
		fgColor, bgColor,
		0,
		getFlags(),
		val, 0, // d1, d2,
		nullptr, widgFont, nullptr // dp, dp2, dp3
	});
}

void CornerSwatch::calculateSize()
{
	Size s = 32_px+4_px;
	setPreferredWidth(s);
	setPreferredHeight(s);
	Widget::calculateSize();
}

int32_t CornerSwatch::onEvent(int32_t event, MessageDispatcher& sendMessage)
{
	assert(event == geCHANGE_SELECTION);
	if(onSelectFunc)
		onSelectFunc(alDialog->d1);
	if(message >= 0)
		sendMessage(message, alDialog->d1);
	return -1;
}

}
