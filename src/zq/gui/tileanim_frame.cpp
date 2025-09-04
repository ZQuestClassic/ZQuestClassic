#include "tileanim_frame.h"
#include "zq/zquest.h"
#include "gui/common.h"
#include "gui/dialog.h"
#include "gui/dialog_runner.h"
#include "gui/size.h"
#include "gui/jwin.h"
#include "base/zsys.h"
#include <cassert>
#include <utility>
#include "tiles.h"
#include "base/qrs.h"

int32_t tile_anim_proc(int32_t msg,DIALOG *d,int32_t c)
{
	using GUI::TileFrame;
	assert(d->dp);
	int32_t *data = ((int32_t*)d->dp);
	int32_t &clk = data[TileFrame::tfr_aclk];
	int32_t &flashclk = data[TileFrame::tfr_flash_clk];
	int32_t &frm = data[TileFrame::tfr_aframe];
	int32_t delay = -data[TileFrame::tfr_delay] * data[TileFrame::tfr_speed];
	switch(msg)
	{
		case MSG_START:
		{
			clk = delay;
			flashclk = 0;
			break;
		}
		case MSG_VSYNC:
		{
			if(d->flags & D_DISABLED)
				break; //Disable animation
			flashclk = (flashclk+1)&0xF; //checking (flashclk&8) is on half the time
			if(data[TileFrame::tfr_flash_cs] > -1 && !(flashclk%8))
				d->flags |= D_DIRTY; //mark for redraw
			if(data[TileFrame::tfr_frames] < 2)
				break; //nothing to animate
			if(++clk > data[TileFrame::tfr_speed])
				d->flags |= D_DIRTY; //mark for redraw
			else if(clk < delay)
				clk = delay; //Handle delay modification
			if(clk >= data[TileFrame::tfr_speed])
			{
				clk %= data[TileFrame::tfr_speed];
				if(++frm >= data[TileFrame::tfr_frames])
				{
					frm %= data[TileFrame::tfr_frames];
					clk = delay;
				}
				d->flags |= D_DIRTY;
			}
			else if(frm >= data[TileFrame::tfr_frames])
			{ //Incase frames was changed
				frm %= data[TileFrame::tfr_frames];
				clk = delay;
				d->flags |= D_DIRTY;
			}
			break;
		}
		case MSG_DRAW:
		{
			int32_t tileToDraw = data[TileFrame::tfr_tile];
			tileToDraw += frm; //the frame count
			tileToDraw += (frm*data[TileFrame::tfr_skipx]); //xskip
			if(int32_t rowdiff = (tileToDraw/TILES_PER_ROW)-(data[TileFrame::tfr_tile]/TILES_PER_ROW))
			{
				tileToDraw += rowdiff*(data[TileFrame::tfr_skipy]) * TILES_PER_ROW;
			}
			int32_t tw = data[TileFrame::tfr_dosized] ? data[TileFrame::tfr_skipx]+1 : 1;
			int32_t th = data[TileFrame::tfr_dosized] ? data[TileFrame::tfr_skipy]+1 : 1;
			if(tw < 1) tw = 1;
			if(th < 1) th = 1;
			BITMAP *bigbmp = create_bitmap_ex(8,d->w+4,d->h+4);
			clear_to_color(bigbmp, d->bg);
			if(!(d->flags&D_DISABLED))
			{
				BITMAP *buf = create_bitmap_ex(8,tw*16,th*16);
				clear_to_color(buf, d->bg);
				//
				int32_t cset = data[TileFrame::tfr_cset];
				int32_t flashcs = data[TileFrame::tfr_flash_cs];
				if(flashcs > -1 && (flashclk&8))
					cset = flashcs;
				int32_t cs2 = data[TileFrame::tfr_cset2];
				for(auto tx = 0; tx < tw; ++tx)
				{
					for(auto ty = 0; ty < th; ++ty)
					{
						int32_t tmptile = tileToDraw+tx+(TILES_PER_ROW*ty);
						if(!(cs2&0xF0) || !(cs2&0xF) || newtilebuf[tileToDraw].format > tf4Bit)
							overtile16(buf, tmptile, tx*16, ty*16, cset, data[TileFrame::tfr_flip]);
						else
						{
							int32_t csets[4];
							int32_t cofs = cs2&0xF;
							if(cofs&8)
								cofs |= ~int32_t(0xF);
							
							for(int32_t i=0; i<4; ++i)
								csets[i] = cs2&(16<<i) ? WRAP_CS2(cset, cofs) : cset;
							
							drawtile16_cs2(buf,tmptile,tx*16,ty*16,csets,data[TileFrame::tfr_flip],true);
						}
					}
				}
				//
				stretch_blit(buf, bigbmp, 0, 0, tw*16, th*16, 2, 2, d->w, d->h);
				destroy_bitmap(buf);
			}
			jwin_draw_frame(bigbmp, 0, 0, d->w+4, d->h+4, FR_DEEP);
			masked_blit(bigbmp, screen, 0, 0, d->x, d->y, d->w+4, d->h+4);
			//
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
	Size s = 32_px+4_px;
	setPreferredWidth(s);
	setPreferredHeight(s);
	bgColor = scheme[jcBOX];
	for(int32_t q = 0; q < tfr_MAX; ++q)
	{
		switch(q)
		{
			case tfr_flash_cs:
				data[q] = -1;
				break;
			case tfr_frames: case tfr_speed:
				data[q] = 1;
				break;
			default:
				data[q] = 0;
		}
	}
}

void TileFrame::setTile(int32_t value)
{
	data[tfr_tile] = vbound(value, 0, NEWMAXTILES);
	pendDraw();
}

void TileFrame::setCSet(int32_t value)
{
	data[tfr_cset] = vbound(value, 0, 15);
	pendDraw();
}

void TileFrame::setCSet2(int32_t value)
{
	data[tfr_cset2] = value&0xFF;
	pendDraw();
}

void TileFrame::setFrames(int32_t value)
{
	data[tfr_frames] = std::max(1,value);
	pendDraw();
}

void TileFrame::setSpeed(int32_t value)
{
	data[tfr_speed] = std::max(1,value);
	pendDraw();
}

void TileFrame::setDelay(int32_t value)
{
	data[tfr_delay] = vbound(value,0,255);
	pendDraw();
}

void TileFrame::setSkipX(int32_t value)
{
	data[tfr_skipx] = vbound(value,0,255);
	pendDraw();
}

void TileFrame::setSkipY(int32_t value)
{
	data[tfr_skipy] = vbound(value,0,255);
	pendDraw();
}

void TileFrame::setFlip(int32_t value)
{
	data[tfr_flip] = value&0x7;
	pendDraw();
}

void TileFrame::setFlashCS(int32_t value)
{
	data[tfr_flash_cs] = value;
	pendDraw();
}

void TileFrame::setDoSized(bool value)
{
	data[tfr_dosized] = value;
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

void TileFrame::applyDisabled(bool dis)
{
	Widget::applyDisabled(dis);
	if(alDialog) alDialog.applyDisabled(dis);
}

void TileFrame::calculateSize()
{
	setPreferredWidth(32_px*(data[tfr_dosized]?data[tfr_skipx]+1:1)+4_px);
	setPreferredHeight(32_px*(data[tfr_dosized]?data[tfr_skipy]+1:1)+4_px);
	Widget::calculateSize();
}

void TileFrame::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	alDialog = runner.push(shared_from_this(), DIALOG {
		newGUIProc<tile_anim_proc>,
		x, y, getWidth()-4, getHeight()-4,
		fgColor, bgColor,
		0,
		getFlags(),
		0, 0, // d1, d2,
		data, nullptr, nullptr // dp, dp2, dp3
	});
}

}
