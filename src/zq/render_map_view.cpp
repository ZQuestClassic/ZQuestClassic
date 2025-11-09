#include "zq/render_map_view.h"
#include "drawing.h"
#include "tiles.h"
#include "zq/render.h"
#include "zq/zq_class.h"
#include "zq/zquest.h"
#include <algorithm>

static MapViewRTI rti_map_view;

void MapViewRTI::render(bool bitmap_resized)
{
	// This mode is used by "View Map", via "mapview_open".
	if (view_map_mode)
	{
		MapCursor previous_cursor = Map.getCursor();
		Map.setViewSize(1);

		BITMAP* bmap4_single = create_bitmap_ex(8,256,176);
		set_bitmap_create_flags(true);
		ALLEGRO_BITMAP* bmap5_single = al_create_bitmap(256,176);
		for(int32_t y=0; y<8; y++)
		{
			for(int32_t x=0; x<16; x++)
			{
				clear_bitmap(bmap4_single);
				Map.setCurrScr(y*16+x);
				Map.draw(bmap4_single, 0, 0, flags, -1, y*16+x, -1);
				stretch_blit(bmap4_single, bmap4_single, 0, 0, 0, 0, 256, 176, 256, 176);
				all_render_a5_bitmap(bmap4_single, bmap5_single);
				al_draw_scaled_bitmap(bmap5_single, 0, 0, 256, 176, sw * x, sh * y, sw, sh, 0);
			}
		}

		Map.setCursor(previous_cursor);
		destroy_bitmap(bmap4_single);
		al_destroy_bitmap(bmap5_single);
		return;
	}

	// Otherwise, this is rendering for the screens editor if "High Quality Screen Rendering" is enabled.

	int view_scr_x = Map.getViewScr() % 16;
	int view_scr_y = Map.getViewScr() / 16;
	int size = Map.getViewSize();
	auto& transform = get_transform();

	extern int ActiveLayerHighlight;
	int highlight_layer = ActiveLayerHighlight ? CurrentLayer : -1;

	extern int LayerDitherBG, LayerDitherSz;
	int layer_dither_bg = LayerDitherBG;
	int layer_dither_sz = LayerDitherSz;

	MapCursor previous_cursor = Map.getCursor();
	Map.setViewSize(1);

	static BITMAP* bmap4_single = create_bitmap_ex(8,256,176);
	set_bitmap_create_flags(true);
	static ALLEGRO_BITMAP* bmap5_single = al_create_bitmap(256,176);

	struct ScreenColorInfo {
		int screen, x, y, color;
	};
	std::vector<ScreenColorInfo> screens_to_draw;

	for (int y = 0; y < size; y++)
	{
		for (int x = 0; x < size; x++)
		{
			int mx = view_scr_x + x;
			int my = view_scr_y + y;
			if (mx < 0 || mx >= 16 || my < 0 || my >= 9)
				continue;

			int screen = my*16 + mx;
			if (screen >= MAPSCRS)
				continue;

			screens_to_draw.push_back(ScreenColorInfo{screen, x, y, Map.getcolor(screen)});
		}
	}

	// Sort by palette, because switching palettes has non-trivial overhead.
	std::sort(screens_to_draw.begin(), screens_to_draw.end(), [](const auto& a, const auto& b) {
		return a.color < b.color;
	});

	for (auto& [screen, x, y, _] : screens_to_draw)
	{
		if (layer_dither_bg > -1)
		{
			if (layer_dither_sz > 0)
			{
				clear_to_color(mapscreenbmp, 0);
				ditherblit(mapscreenbmp, nullptr, vc(layer_dither_bg), dithChecker, layer_dither_sz);
			}
			else
			{
				clear_to_color(mapscreenbmp, vc(layer_dither_bg));
			}
		}

		Map.setCurrScr(screen);

		int xoff = x * 256 * transform.xscale;
		int yoff = y * 176 * transform.yscale;
		combotile_add_x = transform.x + xoff;
		combotile_add_y = transform.y + yoff;
		combotile_mul_x = transform.xscale;
		combotile_mul_y = transform.yscale;
		Map.draw(bmap4_single, 0, 0, flags, -1, screen, highlight_layer);
		combotile_add_x = 0;
		combotile_add_y = 0;
		combotile_mul_x = 1;
		combotile_mul_y = 1;

		all_render_a5_bitmap(bmap4_single, bmap5_single);
		al_draw_bitmap(bmap5_single, 256 * x, 176 * y, 0);
	}

	Map.setCursor(previous_cursor);
}

MapViewRTI* mapview_get_rti()
{
	return &rti_map_view;
}

void mapview_open(int flags, int sw, int sh, int bw, int bh)
{
	MapViewRTI* rti_map_view = mapview_get_rti();
	rti_map_view->view_map_mode = true;
	rti_map_view->flags = flags;
	rti_map_view->sw = sw;
	rti_map_view->sh = sh;
	rti_map_view->set_size(bw, bh);
	rti_map_view->dirty = true;
	get_root_rti()->add_child(rti_map_view);
	render_zq();
}

void mapview_close()
{
	rti_map_view.view_map_mode = false;
	rti_map_view.remove();

	void reload_zq_gui();
	reload_zq_gui();
}
