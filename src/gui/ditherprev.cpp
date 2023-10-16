#include "gui/ditherprev.h"
#include "gui/dialog_runner.h"
#include "jwin.h"

namespace GUI
{

int32_t dith_prev_proc(int32_t msg, DIALOG* d, int32_t)
{
	if(!d || !d->dp) return D_O_K;
	DitherPreview& prev = *(DitherPreview*)d->dp;
	switch(msg)
	{
		case MSG_DRAW:
		{
			rectfill(screen, d->x, d->y, d->x+d->w-1, d->y+d->h-1, jwin_pal[jcBOX]);
			jwin_draw_frame(screen, d->x, d->y, d->w, d->h, FR_ETCHED);
			int tx = d->x+2, ty = d->y+2;
			int tw = d->w-4, th = d->h-4;
			byte col = prev.dcol;
			switch(col) //special hardcoded colors
			{
				case BLACK: col = 0xE0; break;
				case WHITE: col = 0xEF; break;
			}
			BITMAP* tmpbmp = create_bitmap_ex(8,256,168);
			clear_bitmap(tmpbmp);
			ditherrectfill(tmpbmp, 0, 0, 255, 167,
				col, prev.dtype, prev.darg, 0, 0);
			masked_stretch_blit(tmpbmp, screen, 0, 0, 256, 168, tx, ty, tw, th);
			destroy_bitmap(tmpbmp);
			break;
		}
	}
	return D_O_K;
}

DitherPreview::DitherPreview()
	: dtype(0), darg(4), dcol(0xE0), scale(1)
{
	setPreferredWidth(256_px+4_px);
	setPreferredHeight(168_px+4_px);
	setPadding(0_px);
}

void DitherPreview::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	if(alDialog)
		alDialog.applyVisibility(visible);
}

void DitherPreview::applyDisabled(bool dis)
{
	Widget::applyDisabled(dis);
	if(alDialog)
		alDialog.applyDisabled(dis);
}

void DitherPreview::calculateSize()
{
	setPreferredWidth((256_px*scale)+4_px);
	setPreferredHeight((168_px*scale)+4_px);
}

void DitherPreview::realize(DialogRunner& runner)
{
	alDialog = runner.push(shared_from_this(), DIALOG {
		newGUIProc<dith_prev_proc>,
		x, y, getWidth(), getHeight(),
		0, 0,
		0, // key
		getFlags(), // flags
		0, 0, // d1, d2
		(void*)this, nullptr, nullptr // dp, dp2, dp3
	});
}

}
