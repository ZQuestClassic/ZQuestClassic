#include "zq/gui/regiongrid.h"
#include "gui/common.h"
#include "gui/dialog_runner.h"
#include "gui/size.h"
#include "gui/jwin.h"
#include "zq/zq_class.h"
#include <cassert>

using namespace GUI;

void custom_vsync();

static int rg_current_region_id = 0;
static int rg_frame_thickness = 5;
static int rg_button_thickness = 2;
static int rg_header_width = 6;
static int rg_header_height = 9;
static int rg_cols = 16;
static int rg_col_width = 27;
static int rg_l = 25;

static void draw_region_square(BITMAP* bmp, int frame, int region_id, int x, int y, FONT* f, int text_height)
{
	int color = vc(region_id);
	jwin_draw_frame(bmp, x, y, rg_col_width, rg_l, frame);

	int x0 = x + rg_button_thickness;
	int y0 = y + rg_button_thickness;
	rectfill(bmp, x0, y0,
		x + rg_col_width - rg_button_thickness - 1, y + rg_l - rg_button_thickness - 1, color);

	// Ideally would just use `getHighlightColor(color)` but the method isn't good enough yet.
	int text_color;
	switch (region_id) {
	case 2:
	case 3:
	case 5:
	case 7:
	case 9:
		// getHighlightColor currently looks awfule for these colors, so just use black.
		text_color = vc(0);
		break;
	default:
		text_color = getHighlightColor(color);
	}
	textprintf_centre_ex(bmp, f, x0 + rg_col_width / 2 - rg_button_thickness, y0 + rg_l / 2 - text_height / 2, text_color, -1, "%d", region_id);
}

int32_t d_region_grid_proc(int32_t msg, DIALOG* d, int32_t c)
{
	RegionGrid* widg = (RegionGrid*)d->dp2;

	FONT* nf = get_zc_font(font_nfont);

	int map = Map.getCurrMap();
	regions_data* local_regions_data = widg->getLocalRegionsData();

	switch (msg)
	{
	case MSG_START:
	{
		for (int j = 0; j < 8; ++j)
		{
			for (int k = 0; k < rg_cols; ++k)
			{
				int screen = map_scr_xy_to_index(k, j);
				int region_id = local_regions_data->get_region_id(screen);
				if (region_id && !Map.isValid(map, screen))
					local_regions_data->set_region_id(screen, 0);
			}
		}
		return D_WANTFOCUS;
	}

	case MSG_WANTFOCUS:
		return D_WANTFOCUS;

	case MSG_DRAW:
	{
		BITMAP* tempbmp = create_bitmap_ex(8, SCREEN_W, SCREEN_H);
		clear_bitmap(tempbmp);
		int32_t x = d->x;
		int32_t y = d->y;
		int32_t j = 0, k = 0;
		int txtheight = text_height(nf);
		rectfill(tempbmp, x, y, x + d->w - 18, y + rg_header_height - 1, jwin_pal[jcBOX]);

		for (j = 0; j < 8; ++j)
		{
			textprintf_ex(tempbmp, nf, x, y + rg_header_height + rg_frame_thickness + 1 + (j * rg_l) + (rg_l-txtheight)/2, jwin_pal[jcBOXFG], jwin_pal[jcBOX], "%d", j);
		}

		for (j = 0; j < rg_cols; ++j)
		{
			textprintf_ex(tempbmp, nf, x + rg_header_width + rg_frame_thickness + ((rg_col_width + 1) / 2) - (rg_header_width / 2) + (j * rg_col_width), y, jwin_pal[jcBOXFG], jwin_pal[jcBOX], "%X", j);
		}

		// why this not look good
		// jwin_draw_frame(tempbmp, x+header_width+is_large, y+rg_header_height+is_large, (is_large?180:116)*2, (is_large?84:60)*2, FR_DEEP);

		for (j = 0; j < 8; ++j)
		{
			for (k = 0; k < rg_cols; ++k)
			{
				int screen = map_scr_xy_to_index(k, j);
				if (!Map.isValid(map, screen))
					continue;

				byte region_id = local_regions_data->get_region_id(k, j);
				int frame = Map.getCurrScr() == screen ? FR_GREEN : FR_MEDDARK;
				int x2 = x + rg_header_width + (k * rg_col_width) + rg_frame_thickness;
				int y2 = y + rg_header_height + (j * rg_l) + rg_frame_thickness;
				draw_region_square(tempbmp, frame, region_id, x2, y2, nf, txtheight);
			}
		}

		masked_blit(tempbmp, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
		destroy_bitmap(tempbmp);
	}
	break;

	case MSG_CHAR:
	{
		int32_t k = c >> 8;

		int num = -1;
		if ((k >= KEY_0_PAD) && (k <= KEY_9_PAD)) {
			num = k - KEY_0_PAD;
		}
		else if ((k >= KEY_0) && (k <= KEY_9)) {
			num = k - KEY_0;
		}

		if (num != -1)
		{
			rg_current_region_id = num;
			d->flags |= D_DIRTY;
			GUI_EVENT(d, geCHANGE_VALUE);
			return D_USED_CHAR;
		}
	}
	break;

	case MSG_LPRESS:
	{
		int32_t xx = -1;
		int32_t yy = -1;

		while (gui_mouse_b())  // Drag across to select multiple
		{
			int32_t x = (gui_mouse_x() - (d->x) - rg_frame_thickness - rg_header_width) / rg_col_width;
			int32_t y = (gui_mouse_y() - (d->y) - rg_frame_thickness - rg_header_height) / rg_l;

			if (xx != x || yy != y)
			{
				xx = x;
				yy = y;

				int screen = y * 16 + x;
				if (y >= 0 && y < 8 && x >= 0 && x < rg_cols && Map.isValid(map, screen))
					local_regions_data->set_region_id(screen, rg_current_region_id);
			}

			scare_mouse();
			object_message(d, MSG_DRAW, 0);
			unscare_mouse();
			rest(16);
			custom_vsync();
		}
	}
	break;
	}

	return D_O_K;
}

namespace GUI
{
	RegionGrid::RegionGrid() : 
		message(-1), localRegionsData(nullptr)
		
	{
		setPreferredWidth(465_px);
		setPreferredHeight(218_px);
	}

	void RegionGrid::setCurrentRegionIndex(int newindex)
	{
		rg_current_region_id = newindex;
	}
	int RegionGrid::getCurrentRegionIndex()
	{
		return rg_current_region_id;
	}

	void RegionGrid::applyVisibility(bool visible)
	{
		Widget::applyVisibility(visible);
		if (alDialog) alDialog.applyVisibility(visible);
	}

	void RegionGrid::applyDisabled(bool dis)
	{
		Widget::applyDisabled(dis);
		if (alDialog) alDialog.applyDisabled(dis);
	}

	void RegionGrid::realize(DialogRunner& runner)
	{
		Widget::realize(runner);
		alDialog = runner.push(shared_from_this(), DIALOG{
			newGUIProc<d_region_grid_proc>,
			x, y, getWidth(), getHeight(),
			fgColor, bgColor,
			0,
			getFlags(),
			0, 0, // d1, d2,
			NULL, this, NULL // dp, dp2, dp3
			});
	}

	int32_t RegionGrid::onEvent(int32_t event, MessageDispatcher& sendMessage)
	{
		assert(event == geCHANGE_VALUE);
		int ret = -1;
		if (onUpdateFunc)
		{
			onUpdateFunc();
			pendDraw();
		}
		return ret;
	}
}
