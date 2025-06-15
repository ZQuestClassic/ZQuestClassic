#include "zq/gui/dmap_frame.h"
#include "zq/zquest.h"
#include "gui/common.h"
#include "gui/dialog.h"
#include "gui/dialog_runner.h"
#include "gui/jwin.h"

int dmap_frame_proc(int32_t msg, DIALOG* d, int32_t c)
{
	using GUI::DMapFrame;
	if (msg == MSG_DRAW)
	{
		int32_t dmap = d->d1;
		drawdmap(dmap);		
		int32_t x = d->x;
		int32_t y = d->y;
		int32_t w = 84;
		int32_t h = 52;
		jwin_draw_frame(screen, x, y, w, h, FR_DEEP);
		drawdmap_screen(x + 2, y + 2, w - 4, h - 4, dmap);		
	}
	return D_O_K;
}

namespace GUI
{

	DMapFrame::DMapFrame() : alDialog()
	{
		setPreferredWidth(84_px);
		setPreferredHeight(52_px);
	}

	void DMapFrame::setDMap(int32_t value)
	{
		if(value < 0 || value >= MAXDMAPS)
			return;
		Dmap = value;
		if (alDialog) alDialog->d1 = value;
		pendDraw();
	}

	void DMapFrame::applyVisibility(bool visible)
	{
		Widget::applyVisibility(visible);
		if (alDialog) alDialog.applyVisibility(visible);
	}

	void DMapFrame::applyDisabled(bool dis)
	{
		Widget::applyDisabled(dis);
		if (alDialog) alDialog.applyDisabled(dis);
	}

	void DMapFrame::realize(DialogRunner& runner)
	{
		Widget::realize(runner);
		alDialog = runner.push(shared_from_this(), DIALOG{
			newGUIProc<dmap_frame_proc>,
			x, y, getHeight() - 4, getHeight() - 4,
			fgColor, bgColor,
			0,
			getFlags(),
			Dmap, 0, // d1, d2,
			nullptr, nullptr, nullptr // dp, dp2, dp3
			});
	}
}
