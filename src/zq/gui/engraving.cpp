#include "engraving.h"
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
#include <zq/zquestdat.h>

extern DATAFILE* zcdata;

int32_t engraving_proc(int32_t msg,DIALOG *d,int32_t)
{
	switch(msg)
	{
		case MSG_DRAW:
			int32_t h = d->h - 4;
			BITMAP *buf = create_bitmap_ex(8,16,16);
			BITMAP *bigbmp = create_bitmap_ex(8,h,h);
			
			if(buf && bigbmp)
			{
				
				clear_bitmap(buf);
				
				blit((BITMAP*)zcdata[BMP_ENGRAVINGS].dat, buf, d->d1, d->d2, 0, 0, 16, 16);
				
				stretch_blit(buf, bigbmp, 0, 0, 16, 16, 0, 0, h, h);
				
				destroy_bitmap(buf);
				//jwin_draw_frame(bigbmp,0,0,d->h,d->h,FR_DEEP);
				masked_blit(bigbmp,screen,0,0,d->x+2,d->y+2,d->h,d->h);
				destroy_bitmap(bigbmp);
			}
			break;
	}
	return D_O_K;
}

namespace GUI
{

Engraving::Engraving() : slot(0)
{
	setSlot(slot);

	Size s = 32_px + 4_px;
	setPreferredWidth(s);
	setPreferredHeight(s);
}
Engraving::Engraving(int32_t useslot)
{
	setSlot(useslot);

	Size s = 32_px+4_px;
	setPreferredWidth(s);
	setPreferredHeight(s);
}

void Engraving::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	if(alDialog) alDialog.applyVisibility(visible);
}

void Engraving::applyDisabled(bool dis)
{
	Widget::applyDisabled(dis);
	if(alDialog) alDialog.applyDisabled(dis);
}

void Engraving::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	alDialog = runner.push(shared_from_this(), DIALOG {
		newGUIProc<engraving_proc>,
		x, y, getHeight(), getHeight(),
		0, 0,
		0,
		getFlags(),
		drawx, drawy, // d1, d2,
		nullptr, nullptr, nullptr // dp, dp2, dp3
	});
}

}
