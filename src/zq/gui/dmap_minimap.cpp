#include "zq/gui/dmap_minimap.h"
#include "gui/common.h"
#include "gui/dialog.h"
#include "gui/dialog_runner.h"
#include "gui/size.h"
#include "jwin.h"
#include <cassert>
#include <utility>

using namespace GUI;

void drawxmap(ALLEGRO_BITMAP* dest, int32_t themap, int32_t xoff, bool large, int dx, int dy);

namespace GUI
{
	int32_t new_xmaplist_proc(int32_t msg, DIALOG* d, int32_t c)
	{
		static bool xmap_drawn = false;
		int32_t ret = D_O_K;

		DMapMinimap* widg = (DMapMinimap*)d->dp3;
		if (!widg)
			return ret;
		
		if (msg == MSG_START && !widg->rti)
		{
			widg->rti = add_dlg_layer();
		}
		else if (msg == MSG_END && widg->rti)
		{
			remove_dlg_layer(widg->rti);
			widg->rti = nullptr;
		}
		if (!widg->rti)
			return ret;

		if (d->flags & D_HIDDEN)
			return ret;

		if (msg == MSG_DRAW)
		{
			int32_t frame_thickness = int32_t(2 * 1.5);
			int32_t header_width = int32_t(4 * 1.5);
			int32_t header_height = int32_t(6 * 1.5);
			int32_t cols = widg->small_dmap ? 8 : 16;
			int32_t col_width = widg->small_dmap ? 22 : 11;
			int32_t x = d->x + 6;
			int32_t y = d->y + 9;
			int32_t j = 0;
			rectfill(screen, x, y - header_height - frame_thickness - 1, int32_t(x + 116 * 1.5 - 1), y - 1, jwin_pal[jcBOX]);

			FONT* nf = get_zc_font(font_nfont);
			for (j = 0; j < 8; ++j)
			{
				textprintf_ex(screen, nf, x - header_width - frame_thickness, y + 1 + (j * 10), jwin_pal[jcBOXFG], jwin_pal[jcBOX], "%d", j);
			}

			for (j = 0; j < cols; ++j)
			{
				textprintf_ex(screen, nf, x + ((col_width + 1) / 2) - (header_width / 2) + (j * col_width), y - header_height - frame_thickness, jwin_pal[jcBOXFG], jwin_pal[jcBOX], "%X", j);
			}

			jwin_draw_frame(screen, (x - frame_thickness) + 1, (y - frame_thickness) + 1, 180, 84, FR_DEEP);
			drawxmap(widg->rti->bitmap, widg->cur_map-1, widg->offset, widg->small_dmap, x, y);
			xmap_drawn = true;
		}

		return ret;
	}

	DMapMinimap::DMapMinimap() :
		cur_map(1), small_dmap(false), offset(0), rti(nullptr)
	{
		setPreferredWidth(186_px); // 180 + 6
		setPreferredHeight(93_px); // 84 + 9
	}

	void DMapMinimap::applyVisibility(bool visible)
	{
		Widget::applyVisibility(visible);
		if (rti)
			rti->visible = visible;
		if (alDialog) alDialog.applyVisibility(visible);
	}

	void DMapMinimap::applyDisabled(bool dis)
	{
		Widget::applyDisabled(dis);
		if (alDialog) alDialog.applyDisabled(dis);
	}

	void DMapMinimap::realize(DialogRunner& runner)
	{
		Widget::realize(runner);
		alDialog = runner.push(shared_from_this(), DIALOG{
			newGUIProc<new_xmaplist_proc>,
			x, y, getWidth(), getHeight(),
			fgColor, bgColor,
			0,
			0,
			0, 0, // d1, d2,
			NULL, NULL, this // dp, dp2, dp3
			});
	}

	void DMapMinimap::calculateSize()
	{
		Widget::calculateSize();
	}

}
