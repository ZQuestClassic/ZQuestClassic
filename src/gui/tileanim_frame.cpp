#include "tileanim_frame.h"
#include "common.h"
#include "dialog.h"
#include "dialog_runner.h"
#include "size.h"
#include "../jwin.h"
#include "../zquest.h"
#include <cassert>
#include <utility>
#include "tiles.h"

int tile_anim_proc(int msg,DIALOG *d,int c)
{
	using GUI::TileFrame;
	assert(d->dp);
	int *data = ((int*)d->dp);
	int &clk = data[TileFrame::tfr_aclk];
	int &frm = data[TileFrame::tfr_aframe];
	switch(msg)
	{
		case MSG_START:
		{
			clk = 0;
			break;
		}
		case MSG_VSYNC:
		{
			if(d->flags & D_DISABLED)
				break; //Disable animation
			if(++clk > data[TileFrame::tfr_speed])
				d->flags |= D_DIRTY; //mark for redraw
			else if(clk < -data[TileFrame::tfr_delay])
				clk = -data[TileFrame::tfr_delay]; //Handle delay modification
			break;
		}
		case MSG_DRAW:
		{
			if(clk >= data[TileFrame::tfr_speed])
			{
				clk %= data[TileFrame::tfr_speed];
				if(++frm >= data[TileFrame::tfr_frames])
				{
					frm %= data[TileFrame::tfr_frames];
					clk = -data[TileFrame::tfr_delay];
				}
			}
			BITMAP *buf = create_bitmap_ex(8,16,16);
			BITMAP *bigbmp = create_bitmap_ex(8,d->w+4,d->h+4);
			clear_to_color(buf, d->bg);
			clear_to_color(bigbmp, d->bg);
			//
			overtile16(buf, data[TileFrame::tfr_tile]+frm, 0, 0, data[TileFrame::tfr_cset], 0);
			stretch_blit(buf, bigbmp, 0, 0, 16, 16, 2, 2, d->w, d->h);
			jwin_draw_frame(bigbmp, 0, 0, d->w+4, d->h+4, FR_DEEP);
			masked_blit(bigbmp, screen, 0, 0, d->x, d->y, d->w+4, d->h+4);
			//
			destroy_bitmap(buf);
			destroy_bitmap(bigbmp);
			break;
		}
	}
	return D_O_K;
}

namespace GUI
{

TileFrame::TileFrame(): alDialog()
{
	Size s = sized(16_px,32_px)+4_px;
	setPreferredWidth(s);
	setPreferredHeight(s);
	bgColor = palette_color[scheme[jcBOX]];
	for(int q = 0; q < tfr_MAX; ++q)
	{
		data[q] = (q == tfr_frames || q == tfr_speed) ? 1 : 0;
	}
}

void TileFrame::setTile(int value)
{
	data[tfr_tile] = vbound(value, 0, NEWMAXTILES);
	pendDraw();
}

void TileFrame::setCSet(int value)
{
	data[tfr_cset] = vbound(value, 0, 15);
	pendDraw();
}

void TileFrame::setFrames(int value)
{
	data[tfr_frames] = std::max(1,value);
	pendDraw();
}

void TileFrame::setSpeed(int value)
{
	data[tfr_speed] = std::max(1,value);
	pendDraw();
}

void TileFrame::setDelay(int value)
{
	data[tfr_delay] = vbound(value,0,255);
	pendDraw();
}

void TileFrame::resetAnim()
{
	data[tfr_aclk] = 0;
	data[tfr_aframe] = 0;
	pendDraw();
}

void TileFrame::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	if(alDialog) alDialog.applyVisibility(visible);
}

void TileFrame::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	alDialog = runner.push(shared_from_this(), DIALOG {
		newGUIProc<tile_anim_proc>,
		x, y, getHeight()-4, getHeight()-4,
		fgColor, bgColor,
		0,
		getFlags(),
		0, 0, // d1, d2,
		data, nullptr, nullptr // dp, dp2, dp3
	});
}

}
