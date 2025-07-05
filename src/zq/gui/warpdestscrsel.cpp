#include "zq/gui/warpdestscrsel.h"
#include "gui/common.h"
#include "gui/dialog.h"
#include "gui/dialog_runner.h"
#include "gui/jwin.h"
#include <base/dmap.h>

void custom_vsync();

int32_t new_warpdestscrsel_proc(int32_t msg, DIALOG* d, int32_t c)
{
	using GUI::WarpDestScrSel;

	if (msg == MSG_START)
	{
		d->d1 = -1; //cached val
		d->d2 = -1; //cached dmap
		d->fg = 0; //cached 'force_16'
	}
	char* buf = (char*)d->dp;
	vector<DIALOG*>* dlgs = (vector<DIALOG*>*)d->dp2;
	int* dmap_ptr = (int*)d->dp3;
	if (!(buf && dmap_ptr))
		return D_O_K;
	bool is_overworld = ((DMaps[*dmap_ptr].type & dmfTYPE) == dmOVERW);
	int scrw = is_overworld ? 16 : 8, scrh = 9;
	const int max = 0x87;
	int bufval = util::zc_xtoi(buf);
	int val = vbound(bufval, 0, max);
	auto& dm = DMaps[*dmap_ptr];
	auto val_offset = dm.xoff < 0 ? -dm.xoff : 0;
	bool force_16 = d->fg;
	if (!is_overworld)
	{
		if ((val & 0xF) >= 0x8)
			force_16 = true;
		else if ((val & 0xF) < val_offset && (val & 0xF0) < 0x80)
			force_16 = true;
	}
	if (force_16) //can't bound, some quests need to warp out of bounds... -Em
	{
		scrw = 16; //just force show the larger grid instead
		val_offset = 0;
	}

	int xscl = d->w / scrw;
	int yscl = d->h / scrh;

	int ret = D_O_K;
	bool redraw = false;
	if (d->d1 != val)
	{
		redraw = true;
		d->d1 = val;
	}
	if (bufval != val)
	{
		redraw = true;
		sprintf(buf, "%X", val);
	}
	if (d->d2 != *dmap_ptr)
	{
		redraw = true;
		d->d2 = *dmap_ptr;
	}
	switch (msg)
	{
	case MSG_WANTFOCUS:
		ret = D_WANTFOCUS;
		break;
	case MSG_CLICK:
	{
		d->fg = force_16 ? 1 : 0;
		bool redraw2 = false;
		while (gui_mouse_b())
		{
			if (redraw2)
			{
				broadcast_dialog_message(MSG_DRAW, 0);
				redraw2 = false;
			}
			if (!d->fg && (gui_mouse_b() & 2))
			{
				scrw = 16;
				xscl = d->w / scrw;
				yscl = d->h / scrh;
				val_offset = 0;
				d->fg = 1;
				redraw2 = true;
			}
			custom_vsync();
			if (!mouse_in_rect(d->x, d->y, d->w, d->h))
				continue;
			int mx = gui_mouse_x() - d->x, my = gui_mouse_y() - d->y;
			int y = vbound(my / yscl, 0, scrh - 1);
			auto offs = y == 8 ? 0 : val_offset;
			int x = vbound(mx / xscl, offs, scrw - 1);
			auto val2 = (y * 16) + x;
			if (val2 > max) //out of bounds in the bottom-right
				continue;
			val = val2;
			if (d->d1 != val)
			{
				d->d1 = val;
				sprintf(buf, "%02X", val);
				redraw2 = true;
			}
		}
		redraw = true;
		d->fg = 0;
		break;
	}
	case MSG_DRAW:
	{
		rectfill(screen, d->x, d->y, d->x + d->w - 1, d->y + d->h - 1, jwin_pal[jcBOX]);
		jwin_draw_frame(screen, d->x - 2, d->y - 2, d->w + 4, d->h + 4, FR_MENU);
		for (int yind = 0; yind < scrh; ++yind)
		{
			auto gr = (yind < 8 ? dm.grid[yind] : 0);
			for (int xind = (yind == 8 ? 0 : val_offset); xind < scrw; ++xind)
			{
				int screen_index = xind + (yind * 16);
				if (screen_index > max)
					continue;
				int fr = FR_MENU;
				if (screen_index == d->d1)
					fr = FR_GREEN;
				else if (!is_overworld && xind < 8 && (gr & (1 << (8 - xind - 1))))
					fr = FR_MENU_INV;
				jwin_draw_frame(screen, d->x + (xind * xscl), d->y + (yind * yscl), xscl, yscl, fr);
			}
		}
		break;
	}
	/*case MSG_XCHAR:
	{
		bool on_80 = (val & 0xF0) == 0x80;
		switch (c >> 8)
		{
		case KEY_UP:
			if ((val & 0xF0) && !(val_offset && on_80 && (val & 0xF) < val_offset))
			{
				val -= 0x10;
				redraw = true;
			}
			ret |= D_USED_CHAR;
			break;
		case KEY_DOWN:
			if ((val & 0xF0) < ((val & 0xF) < 0x8 ? 0x80 : 0x70))
			{
				val += 0x10;
				redraw = true;
			}
			ret |= D_USED_CHAR;
			break;
		case KEY_LEFT:
			if ((val & 0xF) > (on_80 ? 0 : val_offset))
			{
				--val;
				redraw = true;
			}
			ret |= D_USED_CHAR;
			break;
		case KEY_RIGHT:
			if ((val & 0xF) < scrw - 1 && val < 0x87)
			{
				++val;
				redraw = true;
			}
			ret |= D_USED_CHAR;
			break;
		}
		if (redraw)
			sprintf(buf, "%02X", val);
		break;
	}
	*/
	}
	if (redraw)
	{
		if (msg == MSG_IDLE)
			broadcast_dialog_message(MSG_DRAW, 0);
		else
		{
			d->d1 = d->d2 = -1;
			object_message(d, MSG_IDLE, 0);
		}
	}

	return ret;
}

namespace GUI
{
	WarpDestScrSel::WarpDestScrSel() : alDialog()
	{
		setPreferredWidth(132_px); //8*16 + 4
		setPreferredHeight(76_px); //8*9 + 4
	}

	void WarpDestScrSel::setScreen(int32_t value)
	{
		if (value < 0 || value >= 0x8f)
			return;
		scr = value;
		if (alDialog) alDialog->d1 = value;
		pendDraw();
	}

	void WarpDestScrSel::setDMap(int32_t value)
	{
		if (value < 0 || value >= MAXDMAPS)
			return;
		Dmap = value;
		if (alDialog) alDialog->d2 = value;
		pendDraw();
	}

	int16_t WarpDestScrSel::getScreen()
	{
		return scr;
	}

	int16_t WarpDestScrSel::getDMap()
	{
		return Dmap;
	}

	void WarpDestScrSel::applyVisibility(bool visible)
	{
		Widget::applyVisibility(visible);
		if (alDialog) alDialog.applyVisibility(visible);
	}

	void WarpDestScrSel::applyDisabled(bool dis)
	{
		Widget::applyDisabled(dis);
		if (alDialog) alDialog.applyDisabled(dis);
	}

	void WarpDestScrSel::realize(DialogRunner& runner)
	{
		Widget::realize(runner);
		alDialog = runner.push(shared_from_this(), DIALOG{
			newGUIProc<new_warpdestscrsel_proc>,
			x, y, getHeight() - 4, getHeight() - 4,
			force_16, bgColor,
			0,
			getFlags(),
			scr, Dmap, // d1, d2,
			nullptr, nullptr, nullptr // dp, dp2, dp3
			});
	}
}