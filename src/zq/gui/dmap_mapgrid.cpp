#include "zq/gui/dmap_mapgrid.h"
#include "gui/common.h"
#include "gui/dialog.h"
#include "gui/dialog_runner.h"
#include "gui/size.h"
#include "jwin.h"
#include <cassert>
#include <utility>

using namespace GUI;

void custom_vsync();

void drawdmapscreenmarker(BITMAP* dest, int32_t pos, bool large, int dx, int dy, byte c1, byte c2)
{
	int32_t cols = (large ? 8 : 16);
	int32_t col_width = large ? 22 : 11;
	int32_t l = 10;
	int32_t x = pos % 16;
	int32_t y = pos / 16;
	if (x < cols)
	{
		rect(dest, dx + (x * col_width) - 1, dy + (y * l) - 1, dx + (x * col_width + col_width) + 1, dy + ((y * l) + l) + 1, c1);
		rect(dest, dx + (x * col_width), dy + (y * l), dx + (x * col_width + col_width), dy + ((y * l) + l), c2);
	}
}

namespace GUI
{
	int32_t new_d_grid_proc(int32_t msg, DIALOG* d, int32_t)
	{
		int32_t ret = D_O_K;

		DMapMapGrid* widg = (DMapMapGrid*)d->dp2;
		d->d1 = widg->getSmallDMap() ? 1 : 0;
		byte* mapgrid = widg->getMapGridPtr();
		if (!widg)
			return ret;

		int32_t frame_thickness = int32_t(2 * 1.5);
		int32_t button_thickness = 2;
		int32_t header_width = int32_t(4 * 1.5);
		int32_t header_height = int32_t(6 * 1.5);
		int32_t cols = d->d1 ? 8 : 16;
		int32_t col_width = d->d1 ? 22 : 11;
		int32_t l = 10;

		switch (msg)
		{
		case MSG_DRAW:
		{
			BITMAP* tempbmp = create_bitmap_ex(8, screen->w, screen->h);
			clear_bitmap(tempbmp);
			int32_t x = d->x;
			int32_t y = d->y;
			int32_t j = 0, k = 0;
			rectfill(tempbmp, x, y, x + d->w - 1, y + header_height - 1, jwin_pal[jcBOX]);

			FONT* nf = get_zc_font(font_nfont);
			for (j = 0; j < 8; ++j)
			{
				textprintf_ex(tempbmp, nf, x, y + header_height + frame_thickness + 1 + (j * l), jwin_pal[jcBOXFG], jwin_pal[jcBOX], "%d", j);
			}

			for (j = 0; j < cols; ++j)
			{
				textprintf_ex(tempbmp, nf, x + header_width + frame_thickness + ((col_width + 1) / 2) - (header_width / 2) + (j * col_width), y, jwin_pal[jcBOXFG], jwin_pal[jcBOX], "%X", j);
			}

			jwin_draw_frame(tempbmp, x + header_width + 1, y + header_height + 1, 180, 84, FR_DEEP);

			for (j = 0; j < 8; ++j)
			{
				for (k = 0; k < cols; ++k)
				{
					jwin_draw_frame(tempbmp, x + header_width + (k * col_width) + frame_thickness, y + header_height + (j * l) + frame_thickness, col_width, l, get_bit(mapgrid, 8 * j + (7-k)) || !d->d1 ? FR_MEDDARK : FR_BOX);
					rectfill(tempbmp, x + header_width + (k * col_width) + frame_thickness + button_thickness, y + header_height + (j * l) + frame_thickness + button_thickness,
						x + header_width + (k * col_width) + frame_thickness + col_width - button_thickness - 1, y + header_height + (j * l) + frame_thickness + l - button_thickness - 1, get_bit(mapgrid, 8 * j + (7-k)) && d->d1 ? jwin_pal[jcBOXFG] : jwin_pal[jcBOX]);
				}
			}

			if (widg->getCompassScreen() > -1 && widg->getShowCompass())
			{
				drawdmapscreenmarker(tempbmp, widg->getCompassScreen(), d->d1, d->x + header_width + 2, d->y + header_height + 2, 0xE4, 0xEC);
			}

			if (widg->getContinueScreen() > -1 && widg->getShowContinue())
			{
				drawdmapscreenmarker(tempbmp, widg->getContinueScreen(), d->d1, d->x + header_width + 2, d->y + header_height + 2, 0xE2, 0xEA);
			}

			masked_blit(tempbmp, screen, 0, 0, 0, 0, screen->w, screen->h);
			destroy_bitmap(tempbmp);
		}
		break;

		case MSG_LPRESS:
		{
			int32_t xx = -1;
			int32_t yy = -1;
			int32_t set = -1; // Set or unset

			while (gui_mouse_b())  // Drag across to select multiple
			{
				int32_t x = (gui_mouse_x() - (d->x) - frame_thickness - header_width) / col_width;
				int32_t y = (gui_mouse_y() - (d->y) - frame_thickness - header_height) / l;
				if (xx != x || yy != y)
				{
					xx = x;
					yy = y;

					if (y >= 0 && y < 8 && x >= 0 && x < cols)
					{
						if (key[KEY_ALT] || key[KEY_ALTGR])
						{
							widg->setCompassScreen(x + y * 16);
							GUI_EVENT(d, geCHANGE_VALUE);
						}

						if (key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
						{
							widg->setContinueScreen(x + y * 16);
							GUI_EVENT(d, geCHANGE_VALUE);
						}

						if (cols == 8)
						{
							if (!(key[KEY_ALT] || key[KEY_ALTGR] || key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]))
							{
								if (set == -1)
									set = !get_bit(mapgrid, 8 * y + (7 - x));

								set_bit(mapgrid, 8 * y + (7 - x), set);
							}
						}
					}
				}

				object_message(d, MSG_DRAW, 0);
				custom_vsync();
			}
		}
		break;
		}

		return ret;
	}

	DMapMapGrid::DMapMapGrid() : 
		continueScreen(0), compassScreen(0),
		show_continue(false), show_compass(false)
		
	{
		setPreferredWidth(187_px);
		setPreferredHeight(94_px); 
	}

	void DMapMapGrid::applyVisibility(bool visible)
	{
		Widget::applyVisibility(visible);
		if (alDialog) alDialog.applyVisibility(visible);
	}

	void DMapMapGrid::applyDisabled(bool dis)
	{
		Widget::applyDisabled(dis);
		if (alDialog) alDialog.applyDisabled(dis);
	}

	void DMapMapGrid::realize(DialogRunner& runner)
	{
		Widget::realize(runner);
		alDialog = runner.push(shared_from_this(), DIALOG{
			newGUIProc<new_d_grid_proc>,
			x, y, getWidth(), getHeight(),
			fgColor, bgColor,
			0,
			getFlags(),
			0, 0, // d1, d2,
			NULL, this, NULL // dp, dp2, dp3
			});
	}

	int32_t DMapMapGrid::onEvent(int32_t event, MessageDispatcher& sendMessage)
	{
		assert(event == geCHANGE_VALUE);
		int ret = -1;
		if (onUpdateFunc)
		{
			onUpdateFunc(getMapGridPtr(), getCompassScreen(), getContinueScreen());
			pendDraw();
		}
		return ret;
	}
}
