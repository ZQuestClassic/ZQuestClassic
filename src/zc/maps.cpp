#include "base/combo.h"
#include "base/handles.h"
#include "base/util.h"
#include "base/zdefs.h"
#include "base/general.h"
#include <cstring>
#include <assert.h>
#include <math.h>
#include <vector>
#include <deque>
#include <string>
#include <set>
#include <array>
#include <sstream>
using std::set;

#include "base/qrs.h"
#include "base/dmap.h"
#include "base/mapscr.h"
#include "base/misctypes.h"
#include "base/initdata.h"
#include "zc/maps.h"
#include "zc/zelda.h"
#include "zc/zc_ffc.h"
#include "tiles.h"
#include "sprite.h"
#include "gui/jwin.h"
#include "base/zsys.h"
#include "subscr.h"
#include "zc/zc_subscr.h"
#include "zc/hero.h"
#include "zc/guys.h"
#include "zc/ffscript.h"
#include "drawing.h"
#include "zc/combos.h"
#include "zc/replay.h"
#include "slopes.h"
#include "particles.h"
#include <fmt/format.h>
#include "zc/render.h"
#include "iter.h"
#include <ranges>

// All the temporary screens (and their layers) for the currently loaded map.
static mapscr* temporary_screens[136*7];
// Set by load_region.
static bool screen_in_current_region[136];
static rpos_handle_t current_region_rpos_handles[136*7];
static bool current_region_rpos_handles_dirty;
static int current_region_screen_count;
static std::pair<const rpos_handle_t*, int> current_region_rpos_handles_scr[136];

viewport_t viewport;
static int viewport_sprite_uid;
ViewportMode viewport_mode;
int world_w, world_h;
int region_scr_dx, region_scr_dy;
int region_scr_count;
rpos_t region_max_rpos;
int region_num_rpos;
region_t cur_region, scrolling_region;

maze_state_t maze_state;
int scrolling_maze_last_solved_screen;

void maps_init_game_vars()
{
	viewport = {};
	viewport_mode = ViewportMode::CenterAndBound;
	viewport_sprite_uid = 1;
	currscr_for_passive_subscr = -1;
}

static region_ids_t current_region_ids;

static bool is_a_region(int map, int scr)
{
	return get_region_id(map, scr) != 0;
}

static bool is_same_region_id(int region_origin_scr, int map, int scr)
{
	if (!is_a_region(map, scr)) return false;
	int region_id = get_region_id(map, region_origin_scr);
	return region_id && region_id == get_region_id(map, scr);
}

bool is_in_current_region(int map, int screen)
{
	return map == cur_map && screen >= 0 && screen < 128 && screen_in_current_region[screen];
}

bool is_in_current_region(int screen)
{
	return screen < 128 && screen_in_current_region[screen];
}

bool is_in_current_region(mapscr* scr)
{
	return scr->map == cur_map && scr->screen < 128 && screen_in_current_region[scr->screen];
}

bool is_in_scrolling_region(int screen)
{
	if (!screenscrolling) return false;

	int x = screen % 16;
	int y = screen / 16;
	return
		scrolling_region.origin_screen_x >= x && scrolling_region.origin_screen_x < x + scrolling_region.screen_width &&
		scrolling_region.origin_screen_y >= y && scrolling_region.origin_screen_y < y + scrolling_region.screen_height;
}

bool is_in_scrolling_region()
{
	return cur_region.screen_count > 1;
}

bool is_extended_height_mode()
{
	return cur_region.screen_height > 1 && (DMaps[cur_dmap].flags & dmfEXTENDEDVIEWPORT);
}

// Returns 0 if this is not a region.
int get_region_id(int map, int screen)
{
	if (screen >= 128) return 0;
	if (map == cur_region.map) return current_region_ids[screen];

	return Regions[map].get_region_id(screen);
}

int get_current_region_id()
{
	return get_region_id(cur_map, cur_screen);
}

void calculate_region(int map, int screen, region_t& region, int& region_scr_dx, int& region_scr_dy)
{
	region.map = map;

	if (!(is_a_region(map, screen)) || screen >= 0x80)
	{
		region.region_id = 0;
		region.origin_screen = screen;
		region.origin_screen_x = screen % 16;
		region.origin_screen_y = screen / 16;
		region.screen_width = 1;
		region.screen_height = 1;
		region.screen_count = 1;
		region.width = 256;
		region.height = 176;
		region_scr_dx = 0;
		region_scr_dy = 0;
		return;
	}

	int input_scr_x = screen % 16;
	int input_scr_y = screen / 16;

	// For the given screen, find the top-left corner of its region.
	int origin_scr_x = input_scr_x;
	int origin_scr_y = input_scr_y;
	int origin_scr = screen;
	while (origin_scr_x > 0)
	{
		if (!is_same_region_id(origin_scr, map, map_scr_xy_to_index(origin_scr_x - 1, origin_scr_y))) break;
		origin_scr_x--;
	}
	while (origin_scr_y > 0)
	{
		if (!is_same_region_id(origin_scr, map, map_scr_xy_to_index(origin_scr_x, origin_scr_y - 1))) break;
		origin_scr_y--;
	}
	origin_scr = map_scr_xy_to_index(origin_scr_x, origin_scr_y);
	
	// Now find the bottom-right corner.
	int region_scr_right = origin_scr_x;
	while (region_scr_right < 15)
	{
		if (!is_same_region_id(origin_scr, map, map_scr_xy_to_index(region_scr_right + 1, origin_scr_y))) break;
		region_scr_right++;
	}
	int region_scr_bottom = origin_scr_y;
	while (region_scr_bottom < 7)
	{
		if (!is_same_region_id(origin_scr, map, map_scr_xy_to_index(origin_scr_x, region_scr_bottom + 1))) break;
		region_scr_bottom++;
	}

	region.region_id = get_region_id(map, origin_scr);
	region.origin_screen = origin_scr;
	region.origin_screen_x = origin_scr_x;
	region.origin_screen_y = origin_scr_y;
	region.screen_width = region_scr_right - origin_scr_x + 1;
	region.screen_height = region_scr_bottom - origin_scr_y + 1;
	region.screen_count = region.screen_width * region.screen_height;
	region.width = 256 * region.screen_width;
	region.height = 176 * region.screen_height;
	region_scr_dx = input_scr_x - origin_scr_x;
	region_scr_dy = input_scr_y - origin_scr_y;

	DCHECK_RANGE_INCLUSIVE(region.screen_width, 0, 16);
	DCHECK_RANGE_INCLUSIVE(region.screen_height, 0, 8);
}

void load_region(int dmap, int screen)
{
	clear_temporary_screens();

	int map = DMaps[dmap].map;
	current_region_ids = Regions[map].get_all_region_ids();

	calculate_region(map, screen, cur_region, region_scr_dx, region_scr_dy);
	cur_screen = cur_region.origin_screen;
	world_w = cur_region.width;
	world_h = cur_region.height;
	region_scr_count = cur_region.screen_count;
	region_max_rpos = (rpos_t)(cur_region.screen_count*176 - 1);
	region_num_rpos = cur_region.screen_count*176;
	scrolling_maze_last_solved_screen = 0;

	memset(screen_in_current_region, false, sizeof(screen_in_current_region));
	for (int x = 0; x < cur_region.screen_width; x++)
	{
		for (int y = 0; y < cur_region.screen_height; y++)
		{
			int screen = cur_screen + x + y*16;
			if (screen < 136)
			{
				screen_in_current_region[screen] = true;
			}
		}
	}

	mark_current_region_handles_dirty();
}

static void prepare_current_region_handles()
{
	current_region_rpos_handles_dirty = false;
	current_region_screen_count = 0;
	for (int y = 0; y < cur_region.screen_height; y++)
	{
		for (int x = 0; x < cur_region.screen_width; x++)
		{
			int screen = cur_screen + x + y*16;
			int index_start = current_region_screen_count;
			for (int layer = 0; layer <= 6; layer++)
			{
				mapscr* scr = get_scr_layer(screen, layer);
				if (!scr->is_valid())
				{
					if (layer == 0) break;
					continue;
				}

				rpos_t base_rpos = POS_TO_RPOS(0, get_region_relative_dx(screen), get_region_relative_dy(screen));
				current_region_rpos_handles[current_region_screen_count] = {scr, screen, layer, base_rpos, 0};
				current_region_screen_count += 1;
			}

			int num_handles_for_scr = current_region_screen_count - index_start;
			current_region_rpos_handles_scr[screen] = {&current_region_rpos_handles[index_start], num_handles_for_scr};
		}
	}
}

std::tuple<const rpos_handle_t*, int> get_current_region_handles()
{
	DCHECK(!current_region_rpos_handles_dirty);
	return {current_region_rpos_handles, current_region_screen_count};
}

std::tuple<const rpos_handle_t*, int> get_current_region_handles(mapscr* scr)
{
	DCHECK(!current_region_rpos_handles_dirty);
	if (scr == special_warp_return_scr || current_region_rpos_handles_dirty)
		return {nullptr, 0};

	if (cur_screen >= 0x80)
	{
		DCHECK(scr == origin_scr);
		return {nullptr, 0};
	}

	DCHECK(is_in_current_region(scr));
	return current_region_rpos_handles_scr[scr->screen];
}

void mark_current_region_handles_dirty()
{
	current_region_rpos_handles_dirty = true;
}

void clear_temporary_screens()
{
	for (int i = 0; i < 136*7; i++)
	{
		if (temporary_screens[i])
		{
			free(temporary_screens[i]);
			temporary_screens[i] = NULL;
		}
	}

	origin_scr = nullptr;
	hero_scr = nullptr;
}

std::vector<mapscr*> take_temporary_scrs()
{
	std::vector<mapscr*> screens(temporary_screens, temporary_screens + 136*7);
	for (int i = 0; i < 136*7; i++)
		temporary_screens[i] = nullptr;

	return screens;
}

void calculate_viewport(viewport_t& viewport, int dmap, int screen, int world_w, int world_h, int x, int y)
{
	// TODO: In future, maybe add x/y centering offsets to zscript (Viewport->TargetXOffset/Viewport->TargetYOffset).

	bool extended_height_mode = (DMaps[dmap].flags & dmfEXTENDEDVIEWPORT) && world_h > 176;
	viewport.w = 256;
	viewport.h = 176 + (extended_height_mode ? 56 : 0);

	if (viewport_mode == ViewportMode::Script)
		return;

	if (!is_a_region(DMaps[dmap].map, screen))
	{
		viewport.x = 0;
		viewport.y = 0;
	}
	else if (viewport_mode == ViewportMode::CenterAndBound)
	{
		// Clamp the viewport to the edges of the region.
		viewport.x = CLAMP(0, world_w - viewport.w, x - viewport.w/2);
		viewport.y = CLAMP(0, world_h - viewport.h, y - viewport.h/2);
	}
	else if (viewport_mode == ViewportMode::Center)
	{
		viewport.x = x - viewport.w/2;
		viewport.y = y - viewport.h/2;
	}
}

sprite* get_viewport_sprite()
{
	sprite* spr = sprite::getByUID(viewport_sprite_uid);
	if (!spr)
	{
		viewport_sprite_uid = 1; // Hero uid.
		spr = &Hero;
	}

	return spr;
}

void set_viewport_sprite(sprite* spr)
{
	viewport_sprite_uid = spr->uid;
}

void update_viewport()
{
	sprite* spr = get_viewport_sprite();
	int x = spr->x + spr->txsz*16/2;
	int y = spr->y + spr->tysz*16/2;
	calculate_viewport(viewport, cur_dmap, cur_screen, world_w, world_h, x, y);
}

void update_heroscr()
{
	void playLevelMusic();

	int x = vbound(Hero.getX().getInt(), 0, world_w - 1);
	int y = vbound(Hero.getY().getInt(), 0, world_h - 1);
	int dx = x / 256;
	int dy = y / 176;
	int new_screen = cur_screen + dx + dy * 16;
	if (maze_state.active == 1)
		new_screen = maze_state.scr->screen;
	if (hero_screen != new_screen && dx >= 0 && dy >= 0 && dx < 16 && dy < 8 && is_in_current_region(new_screen))
	{
		region_scr_dx = dx;
		region_scr_dy = dy;
		hero_screen = new_screen;
		prev_hero_scr = hero_scr;
		hero_scr = get_scr(hero_screen);
		Hero.screen_spawned = hero_screen;
		playLevelMusic();
	}
	if (game->get_regionmapping() == REGION_MAPPING_PHYSICAL)
		mark_visited(new_screen); // Mark each screen the hero steps foot in as visited
}

mapscr* determine_hero_screen_from_coords()
{
	int x = vbound(Hero.getX().getInt(), 0, world_w - 1);
	int y = vbound(Hero.getY().getInt(), 0, world_h - 1);
	int dx = x / 256;
	int dy = y / 176;
	return get_scr(cur_screen + dx + dy * 16);
}

bool edge_of_region(direction dir)
{
	if (!is_in_scrolling_region()) return true;

	int screen_x = hero_screen % 16;
	int screen_y = hero_screen / 16;
	if (dir == up) screen_y -= 1;
	if (dir == down) screen_y += 1;
	if (dir == left) screen_x -= 1;
	if (dir == right) screen_x += 1;
	if (screen_x < 0 || screen_x > 16 || screen_y < 0 || screen_y > 8) return true;
	return !is_in_current_region(map_scr_xy_to_index(screen_x, screen_y));
}

// x, y are world coordinates (aka, in relation to origin screen at the top-left).
// Coordinates are clamped to the world bounds.
int get_screen_for_world_xy(int x, int y)
{
	if (!is_in_scrolling_region())
		return cur_screen;

	int dx = std::clamp(x, 0, world_w - 1) / 256;
	int dy = std::clamp(y, 0, world_h - 1) / 176;
	int origin_screen_x = cur_screen % 16;
	int origin_screen_y = cur_screen / 16;
	int scr_x = origin_screen_x + dx;
	int scr_y = origin_screen_y + dy;
	return map_scr_xy_to_index(scr_x, scr_y);
}

int get_screen_for_rpos(rpos_t rpos)
{
	int origin_screen_x = cur_screen % 16;
	int origin_screen_y = cur_screen / 16;
	int screen = static_cast<int32_t>(rpos) / 176;
	int scr_x = origin_screen_x + screen%cur_region.screen_width;
	int scr_y = origin_screen_y + screen/cur_region.screen_width;
	return map_scr_xy_to_index(scr_x, scr_y);
}

rpos_handle_t get_rpos_handle(rpos_t rpos, int layer)
{
	DCHECK_LAYER_ZERO_INDEX(layer);
	if (!is_in_scrolling_region())
		return {get_scr_layer(cur_screen, layer), cur_screen, layer, rpos, RPOS_TO_POS(rpos)};
	int screen = get_screen_for_rpos(rpos);
	mapscr* scr = get_scr_layer(screen, layer);
	return {scr, screen, layer, rpos, RPOS_TO_POS(rpos)};
}

// x, y are world coordinates (aka, in relation to origin screen at the top-left).
// Coordinates are clamped to the world bounds.
rpos_handle_t get_rpos_handle_for_world_xy(int x, int y, int layer)
{
	x = std::clamp(x, 0, world_w - 1);
    y = std::clamp(y, 0, world_h - 1);

	DCHECK_LAYER_ZERO_INDEX(layer);
	if (!is_in_scrolling_region())
	{
		int pos = COMBOPOS(x, y);
		return {get_scr_layer(cur_screen, layer), cur_screen, layer, (rpos_t)pos, pos};
	}
	return get_rpos_handle(COMBOPOS_REGION(x, y), layer);
}

// Return a rpos_handle_t for a screen-specific `pos` (0-175).
rpos_handle_t get_rpos_handle_for_screen(int screen, int layer, int pos)
{
	DCHECK_LAYER_ZERO_INDEX(layer);
	return {get_scr_layer(screen, layer), screen, layer, POS_TO_RPOS(pos, screen), pos};
}

// Return a rpos_handle_t for a screen-specific `pos` (0-175).
// Use this instead of the other `get_pos_handle_for_screen` if you already have a reference to the screen.
rpos_handle_t get_rpos_handle_for_scr(mapscr* scr, int layer, int pos)
{
	DCHECK_LAYER_ZERO_INDEX(layer);
	return {scr, scr->screen, layer, POS_TO_RPOS(pos, scr->screen), pos};
}

void change_rpos_handle_layer(rpos_handle_t& rpos_handle, int layer)
{
	DCHECK_LAYER_ZERO_INDEX(layer);
	rpos_handle.layer = layer;
	rpos_handle.scr = get_scr_layer(rpos_handle.screen, layer);
}

// x, y are world coordinates (aka, in relation to origin screen at the top-left).
// Coordinates are clamped to the world bounds.
combined_handle_t get_combined_handle_for_world_xy(int x, int y, int layer)
{
	DCHECK_LAYER_ZERO_INDEX(layer);

	x = std::clamp(x, 0, world_w - 1);
    y = std::clamp(y, 0, world_h - 1);

	auto maybe_ffc_handle = getFFCAt(x, y);
	if (maybe_ffc_handle)
		return maybe_ffc_handle.value();

	auto rpos = COMBOPOS_REGION_B(x, y);
	if (rpos == rpos_t::None)
		return rpos_handle_t();
	return get_rpos_handle(rpos, layer);
}

// These functions all return _temporary_ screens. Any modifications made to them (either by the engine
// directly or via zscript) only last until the next area is loaded (via loadscr).

// Returns the screen containing the (x, y) world position.
mapscr* get_scr_for_world_xy(int x, int y)
{
	// Quick path, but should work the same without.
	if (!is_in_scrolling_region()) return origin_scr;
	return get_scr(get_screen_for_world_xy(x, y));
}

mapscr* get_scr_for_rpos(rpos_t rpos)
{
	// Quick path, but should work the same without.
	if (!is_in_scrolling_region()) return origin_scr;
	return get_scr(get_screen_for_rpos(rpos));
}

mapscr* get_scr_for_rpos_layer(rpos_t rpos, int layer)
{
	return get_scr_layer(get_screen_for_rpos(rpos), layer);
}

// Note: layer=0 is the base screen, 1 is the first layer, etc.
mapscr* get_scr_for_world_xy_layer(int x, int y, int layer)
{
	DCHECK_LAYER_ZERO_INDEX(layer);
	if (!is_in_scrolling_region()) return get_scr_layer(cur_screen, layer);
	return layer == 0 ?
		get_scr_for_world_xy(x, y) :
		get_scr_layer(get_screen_for_world_xy(x, y), layer);
}

int get_region_screen_offset(int screen)
{
	return get_region_relative_dx(screen) + get_region_relative_dy(screen) * cur_region.screen_width;
}

int get_screen_for_region_index_offset(int offset)
{
	int scr_dx = offset % cur_region.screen_width;
	int scr_dy = offset / cur_region.screen_width;
	int screen = cur_screen + scr_dx + scr_dy*16;
	return screen;
}

mapscr* get_scr_for_region_index_offset(int offset)
{
	int screen = get_screen_for_region_index_offset(offset);
	return get_scr(screen);
}

// The screen at (map, screen) must exist.
mapscr* get_scr(int map, int screen)
{
	mapscr* scr = get_scr_maybe(map, screen);
	CHECK(scr);
	return scr;
}

mapscr* get_scr(int screen)
{
	return get_scr(cur_map, screen);
}

// Returns null if active screen does not exist.
mapscr* get_scr_maybe(int map, int screen)
{
	DCHECK_RANGE_INCLUSIVE(screen, 0, 135);

	if (map == cur_map)
	{
		if (screen == cur_screen)
			return origin_scr;

		int index = screen*7;
		if (temporary_screens[index])
			return temporary_screens[index];

		if (screen == home_screen)
			return special_warp_return_scr;
	}

	if (screenscrolling && map == scrolling_map && !FFCore.ScrollingScreensAll.empty())
	{
		int index = screen*7;
	 	if (FFCore.ScrollingScreensAll[index])
			return FFCore.ScrollingScreensAll[index];
	}

	return nullptr;
}

// Note: layer=0 returns the base screen, layer=1 returns the first layer.
mapscr* get_scr_layer(int map, int screen, int layer)
{
	DCHECK_LAYER_ZERO_INDEX(layer);
	if (layer == 0)
		return get_scr(map, screen);

	if (map == cur_map)
	{
		int index = screen*7 + layer;
		if (temporary_screens[index])
			return temporary_screens[index];

		if (screen == home_screen)
			return &special_warp_return_scrs[layer];
	}

	if (screenscrolling && map == scrolling_map && !FFCore.ScrollingScreensAll.empty())
	{
		int index = screen*7 + layer;
	 	if (FFCore.ScrollingScreensAll[index])
			return FFCore.ScrollingScreensAll[index];
	}

	NOTREACHED();
}

// Note: layer=0 returns the base screen, layer=1 returns the first layer.
mapscr* get_scr_layer(int screen, int layer)
{
	return get_scr_layer(cur_map, screen, layer);
}

// Note: layer=0 returns the base screen, layer=1 returns the first layer.
// Return nullptr if screen is not valid.
mapscr* get_scr_layer_valid(int screen, int layer)
{
	if (mapscr* scr = get_scr_layer(cur_map, screen, layer); scr->is_valid())
		return scr;
	return nullptr;
}

mapscr* get_scr_current_region_dir(int screen, direction dir)
{
	int x = get_region_relative_dx(screen);
	int y = get_region_relative_dy(screen);
	if (dir == left && x == 0)
		return nullptr;
	if (dir == right && x == 15)
		return nullptr;
	if (dir == down && y == 7)
		return nullptr;
	if (dir == up && y == 0)
		return nullptr;

	screen = screen_index_direction(screen, dir);
	if (is_in_current_region(screen))
		return get_scr(screen);

	return nullptr;
}

ffc_handle_t get_ffc_handle(ffc_id_t id)
{
	uint8_t screen = get_screen_for_region_index_offset(id / MAXFFCS);
	uint8_t i = id % MAXFFCS;
	mapscr* scr = get_scr(screen);
	ffcdata* ffc = &scr->getFFC(id % MAXFFCS);
	return {scr, screen, id, i, ffc};
}

std::pair<int32_t, int32_t> translate_screen_coordinates_to_world(int screen, int x, int y)
{
	x += get_region_relative_dx(screen) * 256;
	y += get_region_relative_dy(screen) * 176;
	return {x, y};
}

std::pair<int32_t, int32_t> translate_screen_coordinates_to_world(int screen)
{
	int x = get_region_relative_dx(screen) * 256;
	int y = get_region_relative_dy(screen) * 176;
	return {x, y};
}

int32_t COMBOPOS(int32_t x, int32_t y)
{
	DCHECK(x >= 0 && x < 256 && y >= 0 && y < 176);
	return (y & 0xF0) + (x >> 4);
}
int32_t COMBOPOS_B(int32_t x, int32_t y)
{
	if(unsigned(x) >= 256 || unsigned(y) >= 176)
		return -1;
	return (y & 0xF0) + (x >> 4);
}
int32_t COMBOX(int32_t pos)
{
    return pos % 16 * 16;
}
int32_t COMBOY(int32_t pos)
{
    return pos & 0xF0;
}

rpos_t COMBOPOS_REGION(int32_t x, int32_t y)
{
	if (!is_in_scrolling_region())
		return (rpos_t) COMBOPOS(x, y);

	DCHECK(is_in_world_bounds(x, y));
	int scr_dx = x / (16*16);
	int scr_dy = y / (11*16);
	int pos = COMBOPOS(x%256, y%176);
	return static_cast<rpos_t>((scr_dx + scr_dy * cur_region.screen_width)*176 + pos);
}
rpos_t COMBOPOS_REGION_B(int32_t x, int32_t y)
{
	if (!is_in_world_bounds(x, y))
		return rpos_t::None;

	int scr_dx = x / (16*16);
	int scr_dy = y / (11*16);
	int pos = COMBOPOS(x%256, y%176);
	return static_cast<rpos_t>((scr_dx + scr_dy * cur_region.screen_width)*176 + pos);
}
std::pair<int32_t, int32_t> COMBOXY_REGION(rpos_t rpos)
{
	int scr_index = static_cast<int32_t>(rpos) / 176;
	int scr_dx = scr_index % cur_region.screen_width;
	int scr_dy = scr_index / cur_region.screen_width;
	int pos = RPOS_TO_POS(rpos);
	int x = scr_dx*16*16 + COMBOX(pos);
	int y = scr_dy*11*16 + COMBOY(pos);
	return {x, y};
}
int32_t COMBOX_REGION(rpos_t rpos)
{
	auto [x, y] = COMBOXY_REGION(rpos);
	return x;
}
int32_t COMBOY_REGION(rpos_t rpos)
{
	auto [x, y] = COMBOXY_REGION(rpos);
	return y;
}

rpos_t COMBOPOS_REGION_INDEX(int32_t x, int32_t y)
{
	DCHECK(is_in_world_bounds(x, y));
	if (!is_in_scrolling_region())
		return (rpos_t)(x + y * 16);

	int scr_dx = x / 16;
	int scr_dy = y / 11;
	x %= 16;
	y %= 11;
	return static_cast<rpos_t>((scr_dx + scr_dy * cur_region.screen_width)*176 + x + y * 16);
}
std::pair<int32_t, int32_t> COMBOXY_REGION_INDEX(rpos_t rpos)
{
	int scr_index = static_cast<int32_t>(rpos) / 176;
	int scr_dx = scr_index % cur_region.screen_width;
	int scr_dy = scr_index / cur_region.screen_width;
	int pos = RPOS_TO_POS(rpos);
	int x = scr_dx*16 + pos%16;
	int y = scr_dy*11 + pos/16;
	return {x, y};
}

int32_t mapind(int32_t map, int32_t scr)
{
	return map * MAPSCRSNORMAL + scr;
}

FONT *get_zc_font(int index);

extern sprite_list  guys, items, Ewpns, Lwpns, chainlinks, decorations;
extern movingblock mblock2;                                 //mblock[4]?
extern portal mirror_portal;

void Z_message_d(const char *format,...)
{
#ifdef _DEBUG
    char buf[512];
    va_list ap;
    va_start(ap, format);
    vsprintf(buf, format, ap);
    va_end(ap);
    
    al_trace("%s",buf);
#else
    format=format;
#endif
}



bool checktrigger=false;

void debugging_box(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    //reference/optimization: the start of the unused drawing command index can now be queried. -Gleeok
    int32_t index = script_drawing_commands.GetNext();
    
    if(index < 0)
        return;
        
    int32_t *sdci = &script_drawing_commands[index][0];
    
    sdci[0] = RECTR;
    sdci[1] = 30000;
    sdci[2] = x1*10000;
    sdci[3] = y1*10000;
    sdci[4] = x2*10000;
    sdci[5] = y2*10000;
    sdci[6] = 10000;
    sdci[7] = 10000;
    sdci[8] = 0;
    sdci[9] = 0;
    sdci[10] = 0;
    sdci[11] = 10000;
    sdci[12] = 1280000;
}

void clear_dmap(word i)
{
	DMaps[i].clear();
}

void clear_dmaps()
{
    for(int32_t i=0; i<MAXDMAPS; i++)
    {
        clear_dmap(i);
    }
}

int32_t isdungeon(int32_t dmap, int32_t screen)
{
	if (dmap < 0) dmap = cur_dmap;

	// dungeons can have any dlevel above 0
	if((DMaps[dmap].type&dmfTYPE) == dmDNGN)
	{
		if (get_canonical_scr(cur_map, screen)->flags6&fCAVEROOM)
			return 0;
			
		return 1;
	}

	// dlevels that aren't dungeons are caves
	if (get_canonical_scr(cur_map, screen)->flags6&fDUNGEONROOM)
		return 1;
		
	return 0;
}

int32_t isdungeon(int32_t screen)
{
	return isdungeon(cur_dmap, screen);
}

int32_t isdungeon()
{
	return isdungeon(cur_dmap, hero_screen);
}

bool canPermSecret(int32_t dmap, int32_t screen)
{
	return (!isdungeon(dmap, screen) || get_qr(qr_DUNGEON_DMAPS_PERM_SECRETS));
}

int32_t MAPCOMBO(int32_t x, int32_t y)
{
	x = vbound(x, 0, world_w-1);
	y = vbound(y, 0, world_h-1);
	int pos = COMBOPOS(x%256, y%176);
	mapscr* scr = get_scr_for_world_xy(x, y);
	return scr->data[pos];
}

//specific layers 1 to 6
int32_t MAPCOMBOL(int32_t layer,int32_t x,int32_t y)
{
	DCHECK(layer >= 1 && layer <= 6);
	if (!is_in_world_bounds(x, y) || layer <= 0)
		return 0;

	mapscr* m = get_scr_for_world_xy_layer(x, y, layer);
	if (!m->is_valid())
		return 0;

	int pos = COMBOPOS(x%256, y%176);
	return m->data[pos];
}

int32_t MAPCSETL(int32_t layer,int32_t x,int32_t y)
{
	DCHECK(layer >= 1 && layer <= 6);
	if (!is_in_world_bounds(x, y) || layer <= 0)
		return 0;

	mapscr* m = get_scr_for_world_xy_layer(x, y, layer);
    if (!m->is_valid())
		return 0;

    int pos = COMBOPOS(x%256, y%176);
    return m->cset[pos];
}

int32_t MAPFLAGL(int32_t layer,int32_t x,int32_t y)
{
	DCHECK(layer >= 1 && layer <= 6);
	if (!is_in_world_bounds(x, y) || layer <= 0)
		return 0;

	mapscr* m = get_scr_for_world_xy_layer(x, y, layer);
    if (!m->is_valid())
		return 0;

    int pos = COMBOPOS(x%256, y%176);
    return m->sflag[pos];
}

int32_t COMBOTYPEL(int32_t layer,int32_t x,int32_t y)
{
	DCHECK(layer >= 1 && layer <= 6);
	if (!is_in_world_bounds(x, y) || layer <= 0)
		return 0;

	mapscr* m = get_scr_for_world_xy_layer(x, y, layer);
    if (!m->is_valid())
		return 0;

	int pos = COMBOPOS(x%256, y%176);
    return combobuf[m->data[pos]].type;
}

int32_t MAPCOMBOFLAGL(int32_t layer,int32_t x,int32_t y)
{
	DCHECK(layer >= 1 && layer <= 6);
	if (!is_in_world_bounds(x, y) || layer <= 0)
		return 0;
	
	mapscr* m = get_scr_for_world_xy_layer(x, y, layer);
    if (!m->is_valid())
		return 0;

    int pos = COMBOPOS(x%256, y%176);
    return combobuf[m->data[pos]].flag;
}


// True if the FFC covers x, y and is not ethereal or a changer.
bool ffcIsAt(const ffc_handle_t& ffc_handle, int32_t x, int32_t y)
{
	if (ffc_handle.data()<=0)
        return false;

    if((ffc_handle.ffc->flags&(ffc_changer|ffc_ethereal))!=0)
        return false;

    int32_t fx=ffc_handle.ffc->x.getInt();
    if(x<fx || x>fx+(ffc_handle.scr->ffEffectWidth(ffc_handle.i)-1)) // FFC sizes are weird.
        return false;
    
    int32_t fy=ffc_handle.ffc->y.getInt();
    if(y<fy || y>fy+(ffc_handle.scr->ffEffectHeight(ffc_handle.i)-1))
        return false;

    return true;
}

int32_t MAPFFCOMBO(int32_t x,int32_t y)
{
	if (auto ffc_handle = getFFCAt(x, y))
		return ffc_handle->data();
    return 0;
}

int32_t MAPCSET(int32_t x, int32_t y)
{
	if (!is_in_world_bounds(x, y))
		return 0;
	mapscr* scr = get_scr_for_world_xy(x, y);
	int pos = COMBOPOS(x%256, y%176);
	return scr->cset[pos];
}

int32_t MAPFLAG(int32_t x, int32_t y)
{
	if (!is_in_world_bounds(x, y))
		return 0;
	mapscr* scr = get_scr_for_world_xy(x, y);
	int pos = COMBOPOS(x%256, y%176);
	return scr->sflag[pos];
}

int32_t COMBOTYPE(int32_t x,int32_t y)
{
	int32_t b=1;
	if(x&8) b<<=2;
	if(y&8) b<<=1;

	for (int32_t i = 0; i <= 1; ++i)
	{
		if (get_qr(qr_OLD_BRIDGE_COMBOS))
		{
			if (combobuf[MAPCOMBO2(i,x,y)].type == cBRIDGE && !_walkflag_layer(x, y, i)) return cNONE;
		}
		else
		{
			if (combobuf[MAPCOMBO2(i,x,y)].type == cBRIDGE && _effectflag_layer(x, y, i)) return cNONE;
		}
	}
	
	newcombo const& cmb = combobuf[MAPCOMBO(x,y)];
	if (cmb.type == cWATER && (cmb.walk&b) && ((cmb.walk>>4)&b))
	{
		if(cmb.usrflags&cflag4) return cSHALLOWWATER;
		if(cmb.usrflags&cflag3) return cNONE;
	}
	return cmb.type;
}

int32_t FFCOMBOTYPE(int32_t x,int32_t y)
{
    return combobuf[MAPFFCOMBO(x,y)].type;
}

int32_t FFORCOMBO(int32_t x, int32_t y)
{
	if (auto ffc_handle = getFFCAt(x, y))
		return ffc_handle->data();
	
	return MAPCOMBO(x,y);
}

int32_t FFORCOMBOTYPE(int32_t x, int32_t y)
{
	for (int32_t i = 0; i <= 1; ++i)
	{
		if (get_qr(qr_OLD_BRIDGE_COMBOS))
		{
			if (combobuf[MAPCOMBO2(i,x,y)].type == cBRIDGE && !_walkflag_layer(x,y,i)) return cNONE;
		}
		else
		{
			if (combobuf[MAPCOMBO2(i,x,y)].type == cBRIDGE && _effectflag_layer(x,y,i)) return cNONE;
		}
	}
	int32_t b=1;
    
	if(x&8) b<<=2;
    
	if(y&8) b<<=1;
	newcombo const& cmb = combobuf[FFORCOMBO(x,y)];
	if (cmb.type == cWATER && (cmb.usrflags&cflag4) && (cmb.walk&b)) return cSHALLOWWATER;
	if (cmb.type == cWATER && (cmb.usrflags&cflag3) && (cmb.walk&b)) return cNONE;
	return cmb.type;
}

int32_t FFORCOMBO_L(int32_t layer, int32_t x, int32_t y)
{
	if (auto ffc_handle = getFFCAt(x, y))
		return ffc_handle->data();
	
	return layer ? MAPCOMBOL(layer, x, y) : MAPCOMBO(x,y);
}

int32_t FFORCOMBOTYPE_L(int32_t layer, int32_t x, int32_t y)
{
	return combobuf[FFORCOMBO_L(layer,x,y)].type;
}

int32_t MAPCOMBOFLAG(int32_t x,int32_t y)
{
	if (!is_in_world_bounds(x, y))
		return 0;

	mapscr* scr = get_scr_for_world_xy(x, y);
	int pos = COMBOPOS(x%256, y%176);
	return combobuf[scr->data[pos]].flag;
}

int32_t MAPFFCOMBOFLAG(int32_t x,int32_t y)
{
	if (auto ffc_handle = getFFCAt(x, y))
		return ffc_handle->cflag();

    return 0;
}

std::optional<ffc_handle_t> getFFCAt(int32_t x, int32_t y)
{
	return find_ffc([&](const ffc_handle_t& ffc_handle) {
		return ffcIsAt(ffc_handle, x, y);
	});
}

int32_t MAPCOMBO(const rpos_handle_t& rpos_handle)
{
	if (!rpos_handle.scr->is_valid()) return 0;
	return rpos_handle.data();
}

int32_t MAPCOMBO2(int32_t layer, int32_t x, int32_t y)
{
	DCHECK_LAYER_NEG1_INDEX(layer);
	if (!is_in_world_bounds(x, y)) return 0;
    if (layer == -1) return MAPCOMBO(x, y);
    
	auto rpos_handle = get_rpos_handle_for_world_xy(x, y, layer + 1);
	if (!rpos_handle.scr->is_valid()) return 0;

	return rpos_handle.data();
}

static void apply_state_changes_to_screen(mapscr& scr, int32_t map, int32_t screen, int32_t flags, bool secrets_do_replay_comment)
{
	auto screen_handles = create_screen_handles_one(&scr);

	if ((flags & mSECRET) && canPermSecret(cur_dmap, screen))
	{
		reveal_hidden_stairs(&scr, screen, false);
		bool do_layers = false;
		bool from_active_screen = false;
		trigger_secrets_for_screen_internal(screen_handles, do_layers, from_active_screen, -3, secrets_do_replay_comment);
	}
	if (flags & mLIGHTBEAM)
	{
		for_every_rpos_in_screen(screen_handles, [&](const rpos_handle_t& rpos_handle) {
			if (rpos_handle.ctype() == cLIGHTTARGET)
			{
				if (!(rpos_handle.combo().usrflags&cflag1)) //Unlit version
					rpos_handle.increment_data();
			}
		});
	}

	int lvl = DMaps[cur_dmap].level;
	toggle_switches(game->lvlswitches[lvl], true, screen_handles);
	toggle_gswitches_load(screen_handles);

	if(flags&mLOCKBLOCK)              // if special stuff done before
	{
	    remove_screenstatecombos2(screen_handles, false, cLOCKBLOCK, cLOCKBLOCK2);
	}

	if(flags&mBOSSLOCKBLOCK)          // if special stuff done before
	{
	    remove_screenstatecombos2(screen_handles, false, cBOSSLOCKBLOCK, cBOSSLOCKBLOCK2);
	}

	if(flags&mCHEST)              // if special stuff done before
	{
	    remove_screenstatecombos2(screen_handles, false, cCHEST, cCHEST2);
	}

	if(flags&mCHEST)              // if special stuff done before
	{
	    remove_screenstatecombos2(screen_handles, false, cLOCKEDCHEST, cLOCKEDCHEST2);
	}

	if(flags&mBOSSCHEST)              // if special stuff done before
	{
	    remove_screenstatecombos2(screen_handles, false, cBOSSCHEST, cBOSSCHEST2);
	}
	

	int mi = mapind(map, screen);
	clear_xdoors_mi(screen_handles, mi);
	clear_xstatecombos_mi(screen_handles, mi);
	
	for_every_combo_in_screen(screen_handles, [&](const auto& handle) {
		// This is duplicated 3 places... can this be reduced?
		auto cid = handle.data();
		auto* cmb = &handle.combo();
		bool done = false;
		std::set<int32_t> visited;
		while(!done)
		{
			if(visited.contains(cid))
			{
				Z_error("Combo '%d' was part of an infinite trigger loop, breaking out of loop", cid);
				break; // prevent infinite loop
			}
			visited.insert(cid);
			
			done = true; // don't loop again unless something changes
			for(size_t idx = 0; idx < cmb->triggers.size(); ++idx)
			{
				auto& trig = cmb->triggers[idx];
				if (trig.triggerflags[4] & combotriggerSCREENLOAD)
					do_trigger_combo(handle, idx);
				else continue; // can skip checking handle.data()
				
				if(handle.data() != cid)
				{
					cid = handle.data();
					cmb = &handle.combo();
					done = false; // loop again for the new combo
					break;
				}
			}
		}
	});
}

std::optional<mapscr> load_temp_mapscr_and_apply_secrets(int32_t map, int32_t screen, int32_t layer, bool secrets, bool secrets_do_replay_comment)
{
	if (map < 0 || screen < 0)
		return std::nullopt;

	const mapscr* source = get_canonical_scr(map, screen);
	if (!source->is_valid())
		return std::nullopt;

	if (layer >= 0)
	{
		if (source->layermap[layer] <= 0)
			return std::nullopt;

		source = get_canonical_scr(source->layermap[layer] - 1, source->layerscreen[layer]);
		if (!source->is_valid())
			return std::nullopt;
	}

	int flags = secrets ? game->maps[mapind(map, screen)] : 0;
	mapscr scr = *source;
	apply_state_changes_to_screen(scr, map, screen, flags, secrets_do_replay_comment);

	return scr;
}

static int32_t MAPCOMBO3_impl(int32_t map, int32_t screen, int32_t layer, int32_t pos, bool secrets)
{
	if (map < 0 || screen < 0) return 0;

	if(pos>175 || pos < 0)
		return 0;

	// TODO: consider caching this (invalidate on any modification via scripting, or anything
	// `apply_state_changes_to_screen` checks).
	if (auto s = load_temp_mapscr_and_apply_secrets(map, screen, layer, secrets))
		return s->data[pos];

	return 0;
}

// Read from the current temporary screens or, if (map, screen) is not loaded,
// load that screen and apply the relevant secrets before evaluating the combo at that position.
int32_t MAPCOMBO3(int32_t map, int32_t screen, int32_t layer, int32_t x, int32_t y, bool secrets)
{
	DCHECK_LAYER_NEG1_INDEX(layer);
	DCHECK(map >= 0 && screen >= 0);

	if (is_in_current_region(map, screen)) return MAPCOMBO2(layer, x, y);

	// Screen is not in the current region, so we have to load and trigger some secrets.
	int pos = COMBOPOS(x, y);
	return MAPCOMBO3_impl(map, screen, layer, pos, secrets);
}

int32_t MAPCOMBO3(int32_t map, int32_t screen, int32_t layer, rpos_t rpos, bool secrets)
{ 
	DCHECK_LAYER_NEG1_INDEX(layer);
	DCHECK(map >= 0 && screen >= 0);
	DCHECK(is_valid_rpos(rpos));
	
	if (is_in_current_region(map, screen)) return MAPCOMBO(get_rpos_handle(rpos, layer + 1));
	
	// Screen is not currently loaded, so we have to load and trigger some secrets.
	return MAPCOMBO3_impl(map, screen, layer, RPOS_TO_POS(rpos), secrets);
}

int32_t MAPCSET2(int32_t layer,int32_t x,int32_t y)
{
	DCHECK_LAYER_NEG1_INDEX(layer);
	if (!is_in_world_bounds(x, y))
		return 0;
    if (layer == -1) return MAPCSET(x, y);

	auto rpos_handle = get_rpos_handle_for_world_xy(x, y, layer + 1);
	if (!rpos_handle.scr->is_valid()) return 0;
	
	return rpos_handle.cset();
}

int32_t MAPFLAG2(int32_t layer,int32_t x,int32_t y)
{
	DCHECK_LAYER_NEG1_INDEX(layer);
	if (!get_qr(qr_BUGGED_LAYERED_FLAGS) && (!is_in_world_bounds(x, y)))
		return 0;
    if (layer == -1) return MAPFLAG(x, y);

	auto rpos_handle = get_rpos_handle_for_world_xy(x, y, layer + 1);
	if (!rpos_handle.scr->is_valid()) return 0;

	return rpos_handle.sflag();
}

int32_t COMBOTYPE2(int32_t layer,int32_t x,int32_t y)
{
	if(layer < 1)
	{
		for (int32_t i = layer+1; i <= 1; ++i)
		{
			if (get_qr(qr_OLD_BRIDGE_COMBOS))
			{
				if (combobuf[MAPCOMBO2(i,x,y)].type == cBRIDGE && !_walkflag_layer(x,y,i)) return cNONE;
			}
			else
			{
				if (combobuf[MAPCOMBO2(i,x,y)].type == cBRIDGE && _effectflag_layer(x,y,i)) return cNONE;
			}
		}
	}
	if(layer==-1) return COMBOTYPE(x,y);

	auto rpos_handle = get_rpos_handle_for_world_xy(x, y, layer + 1);
	if (!rpos_handle.scr->is_valid()) return 0;

	return rpos_handle.ctype();
}

// Returns the flag for the combo at the given position.
// This is also known as an "inherent flag".
int32_t MAPCOMBOFLAG2(int32_t layer,int32_t x,int32_t y)
{
	DCHECK_LAYER_NEG1_INDEX(layer);
	if (!is_in_world_bounds(x, y))
		return 0;
    if (layer == -1) return MAPCOMBOFLAG(x, y);

	auto rpos_handle = get_rpos_handle_for_world_xy(x, y, layer + 1);
	if (!rpos_handle.scr->is_valid()) return 0;

	return rpos_handle.cflag();
}

bool HASFLAG(int32_t flag, int32_t layer, rpos_t rpos)
{
	DCHECK_LAYER_ZERO_INDEX(layer);	
	auto rpos_handle = get_rpos_handle(rpos, layer);
	if (!rpos_handle.scr->is_valid()) return false;
	if (rpos_handle.sflag() == flag) return true;
	if (rpos_handle.cflag() == flag) return true;
	return false;
}

bool HASFLAG_ANY(int32_t flag, rpos_t rpos)
{
	DCHECK(is_valid_rpos(rpos));
	if (rpos > region_max_rpos) return false;

	for(auto q = 0; q < 7; ++q)
	{
		if(HASFLAG(flag, q, rpos))
			return true;
	}
	return false;
}

const char *screenstate_string[16] =
{
    "Door Up", "Door Down", "Door Left", "Door Right", "Item", "Special Item", "No Return",
    "Temporary No Return", "Lock Blocks", "Boss Lock Blocks", "Chests", "Locked Chests",
    "Boss Locked Chests", "Secrets", "Visited", "Light Beams"
};

void eventlog_mapflags()
{
	std::ostringstream oss;
	
	int mi = mapind(cur_map, home_screen);
    word g = game->maps[mi] &0x3FFF;
    
	oss << fmt::format("Screen ({}, {:02X})", cur_map+1, home_screen);
	if(g) // Main States
	{
		static const int order[] =
		{
			mSECRET, mITEM, mSPECIALITEM, mLOCKBLOCK, mBOSSLOCKBLOCK,
			mCHEST, mLOCKEDCHEST, mBOSSCHEST,
			mDOOR_UP, mDOOR_DOWN, mDOOR_LEFT, mDOOR_RIGHT,
			mNEVERRET, mTMPNORET
		};
		
		oss << " [";
		bool comma = false;
		for(int fl : order)
		{
			if(!(g&fl))
				continue;
			byte ind = byte(log2(double(fl)));
			if(comma)
				oss << ", ";
			oss << screenstate_string[ind];
			comma = true;
		}
		oss << "]";
	}
	if(game->xstates[mi]) // ExStates
	{
		oss << " Ex[";
		bool comma = false;
		for(byte fl = 0; fl < 32; ++fl)
		{
			if(game->xstates[mi] & (1<<fl))
			{
				if(comma)
					oss << ", ";
				oss << int(fl);
				comma = true;
			}
		}
		oss << "]";
	}
	{ // ExDoors
		for(int q = 0; q < 4; ++q)
		{
			bool comma = false;
			if(auto v = game->xdoors[mi][q])
			{
				if(comma)
					oss << ",";
				else oss << " ExDoor";
				oss << "[" << dirstr[q];
				for(int fl = 0; fl < 8; ++fl)
					if(v & (1<<fl))
						oss << " " << int(fl);
				oss << "]";
				comma = true;
			}
		}
	}
	Z_eventlog("%s\n", oss.str().c_str());
}

// set specific flag
void setmapflag(mapscr* scr, int32_t flag)
{
	if (scr->screen >= 0x80) scr = special_warp_return_scr;
	int mi = mapind(cur_map, scr->screen);
	setmapflag_mi(scr, mi, flag);
}
void setmapflag_homescr(int32_t flag)
{
	int mi = mapind(cur_map, home_screen);
    setmapflag_mi(origin_scr, mi, flag);
}
void setmapflag_mi(int32_t mi, int32_t flag)
{
	byte cscr = mi&((1<<7)-1);
	byte cmap = (mi>>7);
	mapscr* scr = origin_scr;
	if (is_in_current_region(cmap, cscr))
		scr = get_scr(cmap, cscr);

	setmapflag_mi(scr, mi, flag);
}

static void log_state_change(int map, int screen, std::string action)
{
	if (is_in_current_region(map, screen) || (map == cur_map && screen == home_screen))
		Z_eventlog("[Map %d, Screen %02X (current)] %s\n", map + 1, screen, action.c_str());
	else
		Z_eventlog("[Map %d, Screen %02X] %s\n", map + 1, screen, action.c_str());
}

void setmapflag_mi(mapscr* scr, int32_t mi, int32_t flag)
{
    byte cscr = mi&((1<<7)-1);
    byte cmap = (mi>>7);

    float temp=log2((float)flag);
    const char* state_string = flag>0 ? screenstate_string[(int32_t)temp] : "<Unknown>";

    if (replay_is_active() && !(game->maps[mi] & flag))
        replay_step_comment(fmt::format("map {} scr {} flag {}", cmap, cscr, state_string));
    game->maps[mi] |= flag;
    log_state_change(cmap, cscr, fmt::format("State set: {}", state_string));
               
    if(flag==mSECRET||flag==mITEM||flag==mSPECIALITEM||flag==mLOCKBLOCK||
            flag==mBOSSLOCKBLOCK||flag==mCHEST||flag==mBOSSCHEST||flag==mLOCKEDCHEST)
    {
        byte nmap=TheMaps[((cmap)*MAPSCRS)+cscr].nextmap;
        byte nscr=TheMaps[((cmap)*MAPSCRS)+cscr].nextscr;
        
        std::vector<int32_t> done;
        bool looped = (nmap==cmap+1 && nscr==cscr);
        
        while((nmap!=0) && !looped && !(nscr>=128))
        {
            if((scr->nocarry&flag)!=flag && !(game->maps[((nmap-1)<<7)+nscr] & flag))
            {
                log_state_change(nmap, nscr, "State change carried over");
                if (replay_is_active())
                    replay_step_comment(fmt::format("map {} scr {} flag {} carry", nmap, nscr, state_string));
                game->maps[((nmap-1)<<7)+nscr] |= flag;
            }
            
            cmap=nmap;
            cscr=nscr;
            nmap=TheMaps[((cmap-1)*MAPSCRS)+cscr].nextmap;
            nscr=TheMaps[((cmap-1)*MAPSCRS)+cscr].nextscr;
            
            for(auto it = done.begin(); it != done.end(); it++)
            {
                if(*it == ((nmap-1)<<7)+nscr)
                    looped = true;
            }
            
            done.push_back(((nmap-1)<<7)+nscr);
        }
    }
}

void unsetmapflag_home(int32_t flag, bool anyflag)
{
	int mi = mapind(cur_map, home_screen);
    unsetmapflag_mi(origin_scr, mi, flag, anyflag);
}

void unsetmapflag(mapscr* scr, int32_t flag, bool anyflag)
{
	if (scr->screen >= 0x80) scr = special_warp_return_scr;
	int mi = mapind(cur_map, scr->screen);
	unsetmapflag_mi(scr, mi, flag, anyflag);
}

void unsetmapflag_mi(int32_t mi, int32_t flag, bool anyflag)
{
	byte cscr = mi&((1<<7)-1);
	byte cmap = (mi>>7);
	mapscr* scr = origin_scr;
	if (is_in_current_region(cmap, cscr))
		scr = get_scr(cmap, cscr);

	unsetmapflag_mi(scr, mi, flag, anyflag);
}

void unsetmapflag_mi(mapscr* scr, int32_t mi, int32_t flag, bool anyflag)
{
    byte cscr = mi&((1<<7)-1);
    byte cmap = (mi>>7);

    if(anyflag)
        game->maps[mi] &= ~flag;
    else if(flag==mITEM || flag==mSPECIALITEM)
    {
        if(!(scr->flags4&fNOITEMRESET))
            game->maps[mi] &= ~flag;
    }
    else game->maps[mi] &= ~flag;
    
    float temp=log2((float)flag);
    const char* state_string = flag>0 ? screenstate_string[(int32_t)temp] : "<Unknown>";
    log_state_change(cmap, cscr, fmt::format("State unset: {}", state_string));
               
    if(flag==mSECRET||flag==mITEM||flag==mSPECIALITEM||flag==mLOCKBLOCK||
            flag==mBOSSLOCKBLOCK||flag==mCHEST||flag==mBOSSCHEST||flag==mLOCKEDCHEST)
    {
        byte nmap=TheMaps[((cmap)*MAPSCRS)+cscr].nextmap;
        byte nscr=TheMaps[((cmap)*MAPSCRS)+cscr].nextscr;
        
        std::vector<int32_t> done;
        bool looped = (nmap==cmap+1 && nscr==cscr);
        
        while((nmap!=0) && !looped && !(nscr>=128))
        {
            if((scr->nocarry&flag)!=flag && (game->maps[((nmap-1)<<7)+nscr] & flag))
            {
                log_state_change(nmap, nscr, "State change carried over");
                game->maps[((nmap-1)<<7)+nscr] &= ~flag;
            }
            
            cmap=nmap;
            cscr=nscr;
            nmap=TheMaps[((cmap-1)*MAPSCRS)+cscr].nextmap;
            nscr=TheMaps[((cmap-1)*MAPSCRS)+cscr].nextscr;
            
            for(std::vector<int32_t>::iterator it = done.begin(); it != done.end(); it++)
            {
                if(*it == ((nmap-1)<<7)+nscr)
                    looped = true;
            }
            
            done.push_back(((nmap-1)<<7)+nscr);
        }
    }
}

bool getmapflag(int32_t screen, int32_t flag)
{
	int mi = mapind(cur_map, screen >= 0x80 ? home_screen : screen);
    return (game->maps[mi] & flag) != 0;
}
bool getmapflag(mapscr* scr, int32_t flag)
{
	return getmapflag(scr->screen, flag);
}

void setxmapflag(int32_t screen, uint32_t flag)
{
	int mi = mapind(cur_map, screen >= 0x80 ? home_screen : screen);
	setxmapflag_mi(mi, flag);
}
void setxmapflag_mi(int32_t mi, uint32_t flag)
{
	if(game->xstates[mi] & flag) return;
	byte cscr = mi&((1<<7)-1);
	byte cmap = (mi>>7);

	byte temp=(byte)log2((double)flag);
	log_state_change(cmap, cscr, fmt::format("ExtraState set: {}", temp));

	game->xstates[mi] |= flag;
}
void unsetxmapflag(int32_t screen, uint32_t flag)
{
	int mi = mapind(cur_map, screen >= 0x80 ? home_screen : screen);
	unsetxmapflag_mi(mi, flag);
}
void unsetxmapflag_mi(int32_t mi, uint32_t flag)
{
	if(!(game->xstates[mi] & flag)) return;
	byte cscr = mi&((1<<7)-1);
	byte cmap = (mi>>7);
	byte temp=(byte)log2((double)flag);
	log_state_change(cmap, cscr, fmt::format("ExtraState unset: {}", temp));
	game->xstates[mi] &= ~flag;
}
bool getxmapflag(int32_t screen, uint32_t flag)
{
	int mi = mapind(cur_map, screen >= 0x80 ? home_screen : screen);
	return getxmapflag_mi(mi, flag);
}
bool getxmapflag_mi(int32_t mi, uint32_t flag)
{
	return (game->xstates[mi] & flag) != 0;
}

void setxdoor_mi(uint mi, uint dir, uint ind, bool state)
{
	if(mi > game->xdoors.size() || dir > 3 || ind > 8)
		return;
	if(!(game->xdoors[mi][dir] & (1<<ind)) == !state)
		return;

	SETFLAG(game->xdoors[mi][dir], 1<<ind, state);

	int cscr = mi % MAPSCRSNORMAL;
	int cmap = mi / MAPSCRSNORMAL;
	if (state)
		log_state_change(cmap, cscr, fmt::format("ExDoor[{}][{}] set", dirstr[dir], ind));
	else
		log_state_change(cmap, cscr, fmt::format("ExDoor[{}][{}] unset", dirstr[dir], ind));
}
bool getxdoor_mi(uint mi, uint dir, uint ind)
{
	if(mi >= game->xdoors.size() || dir >= 4 || ind >= 8)
		return false;
	return (game->xdoors[mi][dir] & (1<<ind));
}
bool getxdoor(int32_t screen, uint dir, uint ind)
{
	int mi = mapind(cur_map, screen);
	return getxdoor_mi(mi,dir,ind);
}

void set_doorstate_mi(uint mi, uint dir)
{
	if(dir >= 4)
		return;
	setmapflag_mi(mi, mDOOR_UP << dir);
	if(auto di = nextscr_mi(mi, dir))
		setmapflag_mi(*di, mDOOR_UP << oppositeDir[dir]);
}
void set_doorstate(uint screen, uint dir)
{
	int mi = mapind(cur_map, screen);
	set_doorstate_mi(mi, dir);
}

void set_xdoorstate_mi(uint mi, uint dir, uint ind, bool state)
{
	if(mi >= game->xdoors.size() || dir >= 4 || ind >= 8)
		return;
	setxdoor_mi(mi, dir, ind, state);
	if(auto di = nextscr_mi(mi, dir))
		setxdoor_mi(*di, oppositeDir[dir], ind, state);
}

void set_xdoorstate(int32_t screen,uint dir, uint ind, bool state)
{
	int mi = mapind(cur_map, screen);
	set_xdoorstate_mi(mi, dir, ind, state);
}

int32_t WARPCODE(int32_t dmap,int32_t screen,int32_t dw)
// returns: -1 = not a warp screen
//          0+ = warp screen code ( high byte=dmap, low byte=scr )
{
    const mapscr *scr = get_canonical_scr(DMaps[dmap].map, screen);
    
    if(scr->room!=rWARP)
        return -1;
        
    int32_t ring=scr->catchall;
    int32_t size=QMisc.warp[ring].size;
    
    if(size==0)
        return -2;
        
    int32_t index=-1;
    
    for(int32_t i=0; i<size; i++)
        if(dmap==QMisc.warp[ring].dmap[i] && screen==
                (QMisc.warp[ring].scr[i] + DMaps[dmap].xoff))
            index=i;
            
    if(index==-1)
        return -3;
        
    index = (index+dw)%size;
    return (QMisc.warp[ring].dmap[index] << 8) + QMisc.warp[ring].scr[index];
}

void update_combo_cycling()
{
	auto& combo_cache = combo_caches::can_cycle;

	static int32_t newdata[176];
	static int32_t newcset[176];
	static bool initialized=false;

	// Just a simple bit of optimization
	if(!initialized)
	{
		for(int32_t i=0; i<176; i++)
		{
			newdata[i]=-1;
			newcset[i]=-1;
		}
		
		initialized=true;
	}

	std::set<uint16_t> restartanim;

	for_every_base_screen_in_region([&](mapscr* scr, unsigned int region_scr_x, unsigned int region_scr_y) {
		int screen = scr->screen;
		int32_t x;

		for(int32_t i=0; i<176; i++)
		{
			x=scr->data[i];
			auto& mini_cmb = combo_cache.minis[x];
			if (!mini_cmb.can_cycle)
				continue;

			newcombo const& cmb = combobuf[x];
			
			//time to restart
			if ((cmb.aclk>=cmb.speed) && combocheck(cmb))
			{
				bool cycle_under = (cmb.animflags & AF_CYCLEUNDERCOMBO);
				auto c = cycle_under ? scr->undercombo : cmb.nextcombo;
				newdata[i] = c;
				if(!(cmb.animflags & AF_CYCLENOCSET))
					newcset[i] = cycle_under ? scr->undercset : cmb.nextcset;
				
				if(combobuf[c].animflags & AF_CYCLE)
				{
					restartanim.insert(c);
				}
			}
		}
		
		int rpos_base = (int)POS_TO_RPOS(0, region_scr_x, region_scr_y);
		for(int32_t i=0; i<176; i++)
		{
			if(newdata[i]==-1)
				continue;

			rpos_t rpos = (rpos_t)(rpos_base + i);
			rpos_handle_t rpos_handle = {scr, screen, 0, rpos, i};
			screen_combo_modify_preroutine(rpos_handle);
			scr->data[i]=newdata[i];
			if(newcset[i]>-1)
				scr->cset[i]=newcset[i];
			screen_combo_modify_postroutine(rpos_handle);
			
			newdata[i]=-1;
			newcset[i]=-1;
		}
		
		word c = scr->numFFC();
		for(word i=0; i<c; i++)
		{
			ffcdata& ffc = scr->ffcs[i];
			auto& mini_cmb = combo_cache.minis[ffc.data];
			if (!mini_cmb.can_cycle)
				continue;

			newcombo const& cmb = combobuf[ffc.data];
			
			//time to restart
			if ((cmb.aclk>=cmb.speed) && combocheck(cmb))
			{
				bool cycle_under = (cmb.animflags & AF_CYCLEUNDERCOMBO);
				auto c = cycle_under ? scr->undercombo : cmb.nextcombo;
				zc_ffc_set(ffc, c);
				if(!(cmb.animflags & AF_CYCLENOCSET))
					ffc.cset = cycle_under ? scr->undercset : cmb.nextcset;

				if(combobuf[ffc.data].animflags & AF_CYCLE)
				{
					restartanim.insert(ffc.data);
				}
			}
		}
		
		if(get_qr(qr_CMBCYCLELAYERS))
		{
			for(int32_t j=1; j<=6; j++)
			{
				mapscr* layer_scr = get_scr_layer_valid(screen, j);
				if (!layer_scr)
					continue;

				for(int32_t i=0; i<176; i++)
				{
					x=layer_scr->data[i];
					auto& mini_cmb = combo_cache.minis[x];
					if (!mini_cmb.can_cycle)
						continue;

					newcombo const& cmb = combobuf[x];
					
					//time to restart
					if ((cmb.aclk>=cmb.speed) && combocheck(cmb))
					{
						bool cycle_under = (cmb.animflags & AF_CYCLEUNDERCOMBO);
						auto c = cycle_under ? layer_scr->undercombo : cmb.nextcombo;
						newdata[i] = c;
						if(!(cmb.animflags & AF_CYCLENOCSET))
							newcset[i] = cycle_under ? layer_scr->undercset : cmb.nextcset;
						else newcset[i] = layer_scr->cset[i];
						
						if(combobuf[c].animflags & AF_CYCLE)
						{
							restartanim.insert(c);
						}
					}
				}
				
				for (int32_t i=0; i<176; i++)
				{
					if(newdata[i]!=-1)
					{
						layer_scr->data[i]=newdata[i];
						if(newcset[i]>-1)
							layer_scr->cset[i]=newcset[i];
						newdata[i]=-1;
						newcset[i]=-1;
					}
				}
			}
		}
	});

	for (auto i : restartanim)
	{
		combobuf[i].tile = combobuf[i].o_tile;
		combobuf[i].cur_frame=0;
		combobuf[i].aclk = 0;
		combo_caches::drawing.refresh(i);
	}
}

bool iswater_type(int32_t type)
{
//  return type==cOLD_WATER || type==cSWIMWARP || type==cDIVEWARP || type==cDIVEWARPB || type==cDIVEWARPC || type==cDIVEWARPD || type==cSWIMWARPB || type==cSWIMWARPC || type==cSWIMWARPD;
    return (combo_class_buf[type].water!=0);
}

bool iswater(int32_t combo)
{
    return iswater_type(combobuf[combo].type) && !DRIEDLAKE;
}
int32_t iswaterexzq(int32_t combo, int32_t map, int32_t screen, int32_t layer, int32_t x, int32_t y, bool secrets, bool fullcheck, bool LayerCheck)
{
	return iswaterex(combo, map, screen, layer, x, y, secrets, fullcheck, LayerCheck);
}

// (x, y) are world coordinates
int32_t iswaterex_z3(int32_t combo, int32_t layer, int32_t x, int32_t y, bool secrets, bool fullcheck, bool LayerCheck, bool ShallowCheck, bool hero)
{
	if (x<0 || x>=world_w || y<0 || y>=world_h)
		return false;

	return iswaterex(combo, cur_map, cur_screen, layer, x, y, secrets, fullcheck, LayerCheck, ShallowCheck, hero);
}

int32_t iswaterex(int32_t combo, int32_t map, int32_t screen, int32_t layer, int32_t x, int32_t y, bool secrets, bool fullcheck, bool LayerCheck, bool ShallowCheck, bool hero)
{
	DCHECK_LAYER_NEG1_INDEX(layer);
	//Honestly, fullcheck is kinda useless... I made this function back when I thought it was checking the entire combo and not just a glorified x/y value.
	//Fullcheck makes no sense to ever be on, but hey I guess it's here in case you ever need it... 
	
	//Oh hey, Zoras might actually need it. Nevermind, this had a use!
	if (get_qr(qr_SMARTER_WATER))
	{
		if (DRIEDLAKE) return 0;
		if (LayerCheck && (get_qr(qr_WATER_ON_LAYER_1) || get_qr(qr_WATER_ON_LAYER_2))) //LayerCheck is a bit dumber, but it lets me add this QR without having to replace all calls, again.
		{
			for (int32_t m = layer; m <= 1; m++)
			{
				if (m < 0 || m == 0 && get_qr(qr_WATER_ON_LAYER_1)
				|| m == 1 && get_qr(qr_WATER_ON_LAYER_2))
				{
					int32_t checkwater = iswaterex(combo, map, screen, m, x, y, secrets, fullcheck, false, ShallowCheck);
					if (checkwater > 0) 
					{
						return checkwater;
					}
				}
			}
			return 0;
		}
		else
		{
			for(int32_t i=(fullcheck?3:0); i>=0; i--)
			{
				int32_t tx2=((i&2)<<2)+x;
				int32_t ty2=((i&1)<<3)+y;
				int32_t b = i; //Originally b was not needed and I read off i, but then I added the boolean for fullcheck.
				//In which case it's just easier to change b if fullcheck is false instead of changing i and potentially screwing up the for loop.
				if (!fullcheck)
				{
					tx2 = x;
					ty2 = y;
					if(tx2&8) b+=2;
					if(ty2&8) b+=1;
				}
				for (int32_t m = layer; m <= 1; m++)
				{
					newcombo const& cmb = combobuf[MAPCOMBO3(map, screen, m,tx2,ty2, true)];
					if (get_qr(qr_OLD_BRIDGE_COMBOS))
					{
						if (cmb.type == cBRIDGE && !(cmb.walk&(1<<b))) 
						{
							return 0;
						}
					}
					else
					{
						if (cmb.type == cBRIDGE && (cmb.walk&(0x10<<b))) 
						{
							return 0;
						}
					}
					if (get_qr(qr_NO_SOLID_SWIM))
					{
						if ((cmb.type != cBRIDGE || (!get_qr(qr_OLD_BRIDGE_COMBOS) && !(cmb.walk&(0x10<<b)))) && (cmb.walk&(1<<b)) && !((cmb.usrflags&cflag4) && cmb.type == cWATER && (cmb.walk&(0x10<<b)) && ShallowCheck))
						{
							return 0;
						}						
					}
					if (iswater_type(cmb.type) && (cmb.walk&(1<<b)) && ((cmb.usrflags&cflag3) || (cmb.usrflags&cflag4)
						|| (hero && current_item(itype_flippers) < cmb.attribytes[0])
						|| (hero && ((cmb.usrflags&cflag1) && !(itemsbuf[current_item_id(itype_flippers)].flags & item_flag3)))))
					{
						if (!(ShallowCheck && (cmb.walk&(1<<b)) && (cmb.usrflags&cflag4))) return 0;
					}
				}

				auto found_ffc_not_water = find_ffc([&](const ffc_handle_t& ffc_handle) {
					if (ffcIsAt(ffc_handle, tx2, ty2))
					{
						auto ty = ffc_handle.ctype();
						if(!combo_class_buf[ty].water && !(ShallowCheck && ty == cSHALLOWWATER))
							return true;
					}

					return false;
				});
				if (found_ffc_not_water) return 0;

				if(!i)
				{
					auto found_ffc_water = find_ffc([&](const ffc_handle_t& ffc_handle) {
						if (ffcIsAt(ffc_handle, tx2, ty2))
						{
							auto ty = ffc_handle.ctype();
							if(combo_class_buf[ty].water || (ShallowCheck && ty == cSHALLOWWATER))
								return true;
						}

						return false;
					});
					if (found_ffc_water) return found_ffc_water->data();
				}

				int32_t checkcombo = MAPCOMBO3(map, screen, layer, tx2, ty2, secrets);
				if (!(combobuf[checkcombo].walk&(1<<(b+4)))) return 0;
				if (iswater_type(combobuf[checkcombo].type)||(ShallowCheck && (combobuf[checkcombo].type == cSHALLOWWATER || (iswater_type(combobuf[checkcombo].type) && (combobuf[checkcombo].walk&(1<<b)) && (combobuf[checkcombo].usrflags&cflag4))))) 
				{
					if (i == 0) return checkcombo;
				}
			}
			return 0;
		}
	}
	else
	{
		int32_t b = 0;
		if(x&8) b+=2;
		if(y&8) b+=1;
		if (get_qr(qr_NO_SOLID_SWIM))
		{
			if (combobuf[combo].walk&(1<<b))
			{
				return 0;
			}						
		}
		if (!(combobuf[combo].walk&(1<<(b+4)))) return 0;
		return (((iswater_type(combobuf[combo].type) || (ShallowCheck && combobuf[combo].type == cSHALLOWWATER)) && !DRIEDLAKE)?combo:0);//These used to return booleans; returning the combo id of the water combo it caught is essential for Emily's proposed water changes.
	}
}

bool isdamage_type(int32_t type)
{
	switch(type)
	{
		case cDAMAGE1: case cDAMAGE2: case cDAMAGE3: case cDAMAGE4:
		case cDAMAGE5: case cDAMAGE6: case cDAMAGE7:
			return true;
	}
	return false;
}

bool ispitfall_type(int32_t type)
{
	return combo_class_buf[type].pit != 0;
}

bool ispitfall(int32_t combo)
{
    return ispitfall_type(combobuf[combo].type);
}

bool ispitfall(int32_t x, int32_t y)
{
	if(int32_t c = MAPFFCOMBO(x,y))
	{
		return ispitfall(c) ? true : false;
	}
	int32_t c = MAPCOMBOL(2,x,y);
	if(ispitfall(c)) return true;
	if (get_qr(qr_OLD_BRIDGE_COMBOS))
	{
		if (combobuf[MAPCOMBO2(1,x,y)].type == cBRIDGE && !_walkflag_layer(x,y,1)) return false;
	}
	else
	{
		if (combobuf[MAPCOMBO2(1,x,y)].type == cBRIDGE && _effectflag_layer(x,y,1)) return false;
	}
	c = MAPCOMBOL(1,x,y);
	if(ispitfall(c)) return true;

	if (get_qr(qr_OLD_BRIDGE_COMBOS))
	{
		if (combobuf[MAPCOMBO2(0,x,y)].type == cBRIDGE && !_walkflag_layer(x,y,0)) return false;
	}
	else
	{
		if (combobuf[MAPCOMBO2(0,x,y)].type == cBRIDGE && _effectflag_layer(x,y,0)) return false;
	}
	c = MAPCOMBO(x,y);
	if(ispitfall(c)) return true;
	return false;
}

int32_t getpitfall(int32_t x, int32_t y) //Return the highest-layer active pit combo at the given position
{
	if(int32_t c = MAPFFCOMBO(x,y))
	{
		return ispitfall(c) ? c : 0;
	}
	int32_t c = MAPCOMBOL(2,x,y);
	if(ispitfall(c)) return c;

	if (get_qr(qr_OLD_BRIDGE_COMBOS))
	{
		if (combobuf[MAPCOMBO2(1,x,y)].type == cBRIDGE && !_walkflag_layer(x,y,1)) return 0;
	}
	else
	{
		if (combobuf[MAPCOMBO2(1,x,y)].type == cBRIDGE && _effectflag_layer(x,y,1)) return 0;
	}
	c = MAPCOMBOL(1,x,y);
	if(ispitfall(c)) return c;

	if (get_qr(qr_OLD_BRIDGE_COMBOS))
	{
		if (combobuf[MAPCOMBO2(0,x,y)].type == cBRIDGE && !_walkflag_layer(x,y,0)) return 0;
	}
	else
	{
		if (combobuf[MAPCOMBO2(0,x,y)].type == cBRIDGE && _effectflag_layer(x,y,0)) return 0;
	}
	c = MAPCOMBO(x,y);
	if(ispitfall(c)) return c;
	return 0;
}
bool check_icy(newcombo const& cmb, int type)
{
	if(cmb.type != cICY)
		return false;
	switch(type)
	{
		case ICY_BLOCK:
			return cmb.usrflags&cflag1;
		case ICY_PLAYER:
			return cmb.usrflags&cflag2;
	}
	return false;
}
int get_icy(int x, int y, int type)
{
	int32_t c = MAPCOMBOL(2,x,y);
	if(check_icy(combobuf[c], type)) return c;

	int screen = get_screen_for_world_xy(x, y);

	mapscr* scr = get_scr_layer_valid(screen, 2);
	if (scr)
	{
		if (get_qr(qr_OLD_BRIDGE_COMBOS))
		{
			if (combobuf[MAPCOMBO2(1,x,y)].type == cBRIDGE && !_walkflag_layer(x,y,1, scr)) return 0;
		}
		else
		{
			if (combobuf[MAPCOMBO2(1,x,y)].type == cBRIDGE && _effectflag_layer(x,y,1, scr)) return 0;
		}
	}
	c = MAPCOMBOL(1,x,y);
	if(check_icy(combobuf[c], type)) return c;

	scr = get_scr_layer_valid(screen, 1);
	if (scr)
	{
		if (get_qr(qr_OLD_BRIDGE_COMBOS))
		{
			if (combobuf[MAPCOMBO2(0,x,y)].type == cBRIDGE && !_walkflag_layer(x,y,1, scr)) return 0;
		}
		else
		{
			if (combobuf[MAPCOMBO2(0,x,y)].type == cBRIDGE && _effectflag_layer(x,y,1, scr)) return 0;
		}
	}
	c = MAPCOMBO(x,y);
	if(check_icy(combobuf[c], type)) return c;
	return 0;
}

static bool checkSV(int32_t x, int32_t y, int32_t flag)
{
	if(x<0 || x>=world_w || y<0 || y>=world_h)
        return false;
	
	auto rpos_handle = get_rpos_handle_for_world_xy(x, y, 0);
	if (rpos_handle.sflag() == flag || rpos_handle.cflag() == flag)
		return true;
	
	change_rpos_handle_layer(rpos_handle, 1);
	if (rpos_handle.scr->is_valid())
		if (rpos_handle.sflag() == flag || rpos_handle.cflag() == flag)
			return true;
	
	change_rpos_handle_layer(rpos_handle, 2);
	if (rpos_handle.scr->is_valid())
		if (rpos_handle.sflag() == flag || rpos_handle.cflag() == flag)
			return true;
	
	return false;
}

bool isSVLadder(int32_t x, int32_t y)
{
	return checkSV(x, y, mfSIDEVIEWLADDER);
}

bool isSVPlatform(int32_t x, int32_t y)
{
	return checkSV(x, y, mfSIDEVIEWPLATFORM);
}

bool checkSVLadderPlatform(int32_t x, int32_t y)
{
	return isSVPlatform(x,y) || (isSVLadder(x,y) && !isSVLadder(x,y-16));
}

bool isstepable(int32_t combo)                                  //can use ladder on it
{
    if(combo_class_buf[combobuf[combo].type].ladder_pass) return true;
	if(combo_class_buf[combobuf[combo].type].pit)
	{
		if(combobuf[combo].usrflags&cflag4)
		{
			int32_t ldrid = current_item_id(itype_ladder);
			return (ldrid > -1 && itemsbuf[ldrid].flags & item_flag1);
		}
	}
	return false;
}

bool isHSGrabbable(newcombo const& cmb)
{
	if(cmb.type == cHSGRAB) return true;
	return cmb.genflags & cflag1;
}

bool isSwitchHookable(newcombo const& cmb)
{
	if(cmb.type == cSWITCHHOOK) return true;
	return cmb.genflags & cflag2;
}

bool check_hshot(int32_t layer, int32_t x, int32_t y, bool switchhook, rpos_t *out_rpos, ffcdata **out_ffc)
{
	rpos_t cpos = rpos_t::None;
	if(out_rpos)
	{
		int32_t id = MAPCOMBO2(layer-1,x,y);
		if(id > 0)
		{
			newcombo const& cmb = combobuf[id];
			cpos = (switchhook ? isSwitchHookable(cmb) : isHSGrabbable(cmb)) ? COMBOPOS_REGION(x,y) : rpos_t::None;
		}
	}

	ffcdata* ffc = nullptr;
	if (out_ffc && !get_qr(qr_OLD_FFC_FUNCTIONALITY))
	{
		for_some_ffcs([&](const ffc_handle_t& ffc_handle) {
			if (ffcIsAt(ffc_handle, x, y))
			{
				auto& cmb = ffc_handle.combo();
				if (switchhook ? isSwitchHookable(cmb) : isHSGrabbable(cmb))
				{
					ffc = ffc_handle.ffc;
					return false;
				}
			}
			return true;
		});
	}

	if (out_rpos && cpos != rpos_t::None) *out_rpos = cpos;
	if (out_ffc && ffc) *out_ffc = ffc;
	return (cpos != rpos_t::None || ffc);
}

bool ishookshottable(int32_t bx, int32_t by)
{
	if(!_walkflag(bx,by,1))
		return true;
	
	if (collide_object(bx, by, 1, 1))
		return false;
	
	bool ret = true;
	for(int32_t i=2; i>=0; i--)
	{
		int32_t c = MAPCOMBO2(i-1,bx,by);
		int32_t t = combobuf[c].type;
		
		if(i == 0 && (t == cHOOKSHOTONLY || t == cLADDERHOOKSHOT)) return true;
		
		bool dried = (iswater_type(t) && DRIEDLAKE);
		
		int32_t b=1;
		
		if(bx&8) b<<=2;
		
		if(by&8) b<<=1;
		
		if(combobuf[c].walk&b && !dried && !(combo_class_buf[t].ladder_pass && t!=cLADDERONLY) && t!=cHOOKSHOTONLY)
			ret = false;
	}
	
	return ret;
}

bool reveal_hidden_stairs(mapscr *s, int32_t screen, bool redraw)
{
    if((s->stairx || s->stairy) && s->secretcombo[sSTAIRS])
    {
        int pos = COMBOPOS(s->stairx,s->stairy);
        s->data[pos] = s->secretcombo[sSTAIRS];
        s->cset[pos] = s->secretcset[sSTAIRS];
        s->sflag[pos] = s->secretflag[sSTAIRS];
        
        if (redraw)
		{
			auto [x, y] = translate_screen_coordinates_to_world(screen, s->stairx, s->stairy);
            putcombo(scrollbuf,x,y,s->data[pos],s->cset[pos]);
		}
            
        return true;
    }
    
    return false;
}

screen_handles_t create_screen_handles_one(mapscr* base_scr)
{
	DCHECK(base_scr->is_valid());

	screen_handles_t screen_handles{};
	screen_handles[0] = {base_scr, base_scr, base_scr->screen, 0};
	return screen_handles;
}

screen_handles_t create_screen_handles(mapscr* base_scr)
{
	DCHECK(get_scr(base_scr->screen) == base_scr);
	DCHECK(base_scr->is_valid());

	int screen = base_scr->screen;
	screen_handles_t screen_handles;
	screen_handles[0] = {base_scr, base_scr, screen, 0};
	for (int i = 1; i <= 6; i++)
		screen_handles[i] = {base_scr, get_scr_layer_valid(screen, i), screen, i};
	return screen_handles;
}

bool remove_screenstatecombos2(const screen_handles_t& screen_handles, bool do_layers, int32_t what1, int32_t what2)
{
	mapscr* scr = screen_handles[0].scr;
	bool didit=false;
	
	for(int32_t i=0; i<176; i++)
	{
		newcombo const& cmb = combobuf[scr->data[i]];
		if(cmb.usrflags&cflag16) continue; //custom state instead of normal state
		if((cmb.type == what1) || (cmb.type== what2))
		{
			scr->data[i]++;
			didit=true;
		}
	}
	
	if (do_layers)
	{
		for(int32_t j=1; j<=6; j++)
		{
			mapscr* layer_scr = screen_handles[j].scr;
			if (!layer_scr) continue;

			for(int32_t i=0; i<176; i++)
			{
				newcombo const& cmb = combobuf[layer_scr->data[i]];
				if(cmb.usrflags&cflag16) continue; //custom state instead of normal state
				if((cmb.type== what1) || (cmb.type== what2))
				{
					layer_scr->data[i]++;
					didit=true;
				}
			}
		}
	}

	// 'do_layers' also means that this is called on an active temp screen, so update its ffcs.
	if (!get_qr(qr_OLD_FFC_FUNCTIONALITY) && do_layers)
	{
		word c = scr->numFFC();
		for(word i=0; i<c; i++)
		{
			ffcdata* ffc = &scr->ffcs[i];
			newcombo const& cmb = combobuf[ffc->data];
			if(cmb.usrflags&cflag16) continue; //custom state instead of normal state
			if((cmb.type== what1) || (cmb.type== what2))
			{
				zc_ffc_modify(*ffc, 1);
				didit=true;
			}
		}
	}
	
	return didit;
}

bool remove_xstatecombos(const screen_handles_t& screen_handles, byte xflag, bool triggers)
{
	int screen = screen_handles[0].scr->screen;
	int mi = mapind(cur_map, screen >= 0x80 ? home_screen : screen);
	return remove_xstatecombos_mi(screen_handles, mi, xflag, triggers);
}
bool remove_xstatecombos_mi(const screen_handles_t& screen_handles, int32_t mi, byte xflag, bool triggers)
{
	bool didit=false;
	if(!getxmapflag_mi(mi, 1<<xflag)) return false;

	mapscr* s = screen_handles[0].scr;
	int screen = s->screen;
	bool is_active_screen = is_in_current_region(s);

	for_every_rpos_in_screen(screen_handles, [&](const rpos_handle_t& rpos_handle) {
		if(triggers && force_ex_trigger_any(rpos_handle, xflag))
			didit = true;
		else switch (rpos_handle.ctype())
		{
			case cLOCKBLOCK: case cLOCKBLOCK2:
			case cBOSSLOCKBLOCK: case cBOSSLOCKBLOCK2:
			case cCHEST: case cCHEST2:
			case cLOCKEDCHEST: case cLOCKEDCHEST2:
			case cBOSSCHEST: case cBOSSCHEST2:
			{
				auto& cmb = rpos_handle.combo();
				if(!(cmb.usrflags&cflag16)) return; //custom state instead of normal state
				if(cmb.attribytes[5] == xflag)
				{
					rpos_handle.increment_data();
					didit=true;
				}
				break;
			}
		}
	});

	if (is_active_screen && !get_qr(qr_OLD_FFC_FUNCTIONALITY))
	{
		word c = s->numFFC();
		int screen_index_offset = get_region_screen_offset(screen);
		for (uint8_t i = 0; i < c; i++)
		{
			auto ffc_handle = *s->getFFCHandle(i, screen_index_offset);
			auto& cmb = ffc_handle.combo();
			if(triggers && force_ex_trigger_ffc_any(ffc_handle, xflag))
				didit = true;
			else switch(cmb.type)
			{
				case cLOCKBLOCK: case cLOCKBLOCK2:
				case cBOSSLOCKBLOCK: case cBOSSLOCKBLOCK2:
				case cCHEST: case cCHEST2:
				case cLOCKEDCHEST: case cLOCKEDCHEST2:
				case cBOSSCHEST: case cBOSSCHEST2:
				{
					if(!(cmb.usrflags&cflag16)) continue; //custom state instead of normal state
					if(cmb.attribytes[5] == xflag)
					{
						zc_ffc_modify(*ffc_handle.ffc, 1);
						didit=true;
					}
					break;
				}
			}
		}
	}
	
	return didit;
}

void clear_xstatecombos(const screen_handles_t& screen_handles, bool triggers)
{
	int screen = screen_handles[0].screen;
	int mi = mapind(cur_map, screen >= 0x80 ? home_screen : screen);
	clear_xstatecombos_mi(screen_handles, mi, triggers);
}

void clear_xstatecombos_mi(const screen_handles_t& screen_handles, int32_t mi, bool triggers)
{
	for (int q = 0; q < 32; ++q)
	{
		remove_xstatecombos_mi(screen_handles, mi, q, triggers);
	}
}

bool remove_xdoors(const screen_handles_t& screen_handles, uint dir, uint ind, bool triggers)
{
	int screen = screen_handles[0].screen;
	int mi = mapind(cur_map, screen >= 0x80 ? home_screen : screen);
	return remove_xdoors_mi(screen_handles, mi, dir, ind, triggers);
}
bool remove_xdoors_mi(const screen_handles_t& screen_handles, int32_t mi, uint dir, uint ind, bool triggers)
{
	bool didit=false;
	if (!getxdoor_mi(mi, dir, ind)) return false;

	mapscr* scr = screen_handles[0].scr;
	int screen = scr->screen;
	bool is_active_screen = is_in_current_region(scr);

	for_every_rpos_in_screen(screen_handles, [&](const rpos_handle_t& rpos_handle) {
		if (triggers && force_ex_door_trigger_any(rpos_handle, dir, ind))
			didit = true;
		else; //future door combo types?
	});

	if (!get_qr(qr_OLD_FFC_FUNCTIONALITY) && is_active_screen)
	{
		word c = scr->numFFC();
		int screen_index_offset = get_region_screen_offset(screen);
		for (uint8_t i = 0; i < c; i++)
		{
			auto ffc_handle = *scr->getFFCHandle(i, screen_index_offset);
			if (triggers && force_ex_door_trigger_ffc_any(ffc_handle, dir, ind))
				didit = true;
			else; //future door combo types?
		}
	}
	
	return didit;
}

void clear_xdoors(const screen_handles_t& screen_handles, bool triggers)
{
	mapscr* scr = screen_handles[0].scr;
	int mi = mapind(cur_map, scr->screen >= 0x80 ? home_screen : scr->screen);
	clear_xdoors_mi(screen_handles, mi, triggers);
}

void clear_xdoors_mi(const screen_handles_t& screen_handles, int32_t mi, bool triggers)
{
	for (int q = 0; q < 32; ++q)
	{
		remove_xdoors(screen_handles, mi, q, triggers);
	}
}

bool remove_lockblocks(const screen_handles_t& screen_handles)
{
    return remove_screenstatecombos2(screen_handles, true, cLOCKBLOCK, cLOCKBLOCK2);
}

bool remove_bosslockblocks(const screen_handles_t& screen_handles)
{
    return remove_screenstatecombos2(screen_handles, true, cBOSSLOCKBLOCK, cBOSSLOCKBLOCK2);
}

bool remove_chests(const screen_handles_t& screen_handles)
{
    return remove_screenstatecombos2(screen_handles, true, cCHEST, cCHEST2);
}

bool remove_lockedchests(const screen_handles_t& screen_handles)
{
    return remove_screenstatecombos2(screen_handles, true, cLOCKEDCHEST, cLOCKEDCHEST2);
}

bool remove_bosschests(const screen_handles_t& screen_handles)
{
    return remove_screenstatecombos2(screen_handles, true, cBOSSCHEST, cBOSSCHEST2);
}

void delete_fireball_shooter(const rpos_handle_t& rpos_handle)
{
    int32_t ct=rpos_handle.ctype();
    
    if(ct!=cL_STATUE && ct!=cR_STATUE && ct!=cC_STATUE)
        return;
    
    auto [cx, cy] = rpos_handle.xy();
    switch(ct)
    {
    case cL_STATUE:
        cx += 4;
        cy += 7;
        break;
        
    case cR_STATUE:
        cx -= 8;
        cy -= 1;
        break;
        
    case cC_STATUE:
        break;
    }
    
    for(int32_t j=0; j<guys.Count(); j++)
    {
        // Finds the smallest enemy ID
        if((int32_t(guys.spr(j)->x)==cx)&&(int32_t(guys.spr(j)->y)==cy)&&(guysbuf[(guys.spr(j)->id)&0xFFF].flags & guy_fire))
        {
            guys.del(j);
        }
    }
}

static int32_t findtrigger(int32_t screen)
{
    int32_t checkflag=0;
    int32_t ret = 0;

	mapscr* screens[7];
	for (int32_t j = 0; j <= 6; j++)
	{
		screens[j] = get_scr_layer_valid(screen, j);
	}
    
	bool sflag = false;
    for(word j=0; j<176; j++)
    {
        for(int32_t layer = -1; layer < 6; ++layer)
		{
			mapscr* scr = screens[layer+1];
			if (!scr) continue;

			if(sflag)
				checkflag = scr->sflag[j];
			else
				checkflag = combobuf[scr->data[j]].flag;
			sflag = !sflag;
			if (sflag) --layer;

			switch(checkflag)
			{
				case mfANYFIRE:
				case mfSTRONGFIRE:
				case mfMAGICFIRE:
				case mfDIVINEFIRE:
				case mfARROW:
				case mfSARROW:
				case mfGARROW:
				case mfSBOMB:
				case mfBOMB:
				case mfBRANG:
				case mfMBRANG:
				case mfFBRANG:
				case mfWANDMAGIC:
				case mfREFMAGIC:
				case mfREFFIREBALL:
				case mfSWORD:
				case mfWSWORD:
				case mfMSWORD:
				case mfXSWORD:
				case mfSWORDBEAM:
				case mfWSWORDBEAM:
				case mfMSWORDBEAM:
				case mfXSWORDBEAM:
				case mfHOOKSHOT:
				case mfWAND:
				case mfHAMMER:
				case mfSTRIKE:
					ret += 1;
					break;
			}
		}
    }
    
    return ret;
}

static void log_trigger_secret_reason(TriggerSource source)
{
	if (source == TriggerSource::Singular)
	{
		Z_eventlog("Restricted Screen Secrets triggered\n");
	}
	else
	{
		const char* source_str = "";
		switch (source)
		{
			case TriggerSource::Singular: break;
			case TriggerSource::Unspecified: source_str = "unspecified means"; break;
			case TriggerSource::EnemiesScreenFlag: source_str = "the 'Enemies->Secret' screen flag"; break;
			case TriggerSource::SecretsScreenState: source_str = "the 'Secrets' screen state"; break;
			case TriggerSource::Script: source_str = "a script"; break;
			case TriggerSource::ItemsSecret: source_str = "Items->Secrets"; break;
			case TriggerSource::GenericCombo: source_str = "Generic Combo"; break;
			case TriggerSource::LightTrigger: source_str = "Light Triggers"; break;
			case TriggerSource::SCC: source_str = "SCC"; break;
			case TriggerSource::CheatTemp: source_str = "Cheat (Temp)"; break;
			case TriggerSource::CheatPerm: source_str = "Cheat (Perm)"; break;
		}
		Z_eventlog("Screen Secrets triggered by %s\n", source_str);
	}
}

// single:
// >-1 : the singular triggering combo
// -1: triggered by some other cause
void trigger_secrets_for_screen(TriggerSource source, mapscr* scr, bool high16only, int32_t single)
{
	log_trigger_secret_reason(source);
	if (single < 0)
		get_screen_state(scr->screen).triggered_secrets = true;

	bool do_replay_comment = true;
	bool from_active_screen = true;
	trigger_secrets_for_screen_internal(create_screen_handles(scr), from_active_screen, high16only, single, do_replay_comment);

	// Respect secret state carryovers for active screens.
	if (single >= 0) return;
	int flag = mSECRET;
	int cmap = scr->map;
	int cscr = scr->screen;
	int nmap=TheMaps[((cmap)*MAPSCRS)+cscr].nextmap;
	int nscr=TheMaps[((cmap)*MAPSCRS)+cscr].nextscr;

	std::vector<int32_t> done;
	bool looped = (nmap==cmap+1 && nscr==cscr);

	while((nmap!=0) && !looped && !(nscr>=128))
	{
		if (nmap - 1 == cur_map && is_in_current_region(nscr) && (scr->nocarry&flag)!=flag && !get_screen_state(nscr).triggered_secrets)
		{
			log_trigger_secret_reason(TriggerSource::SecretsScreenState);
			trigger_secrets_for_screen_internal(create_screen_handles(get_scr(nscr)), from_active_screen, high16only, single, do_replay_comment);
			get_screen_state(nscr).triggered_secrets = true;
		}
		
		cmap=nmap;
		cscr=nscr;
		nmap=TheMaps[((cmap-1)*MAPSCRS)+cscr].nextmap;
		nscr=TheMaps[((cmap-1)*MAPSCRS)+cscr].nextscr;
		
		for(auto it = done.begin(); it != done.end(); it++)
		{
			if(*it == ((nmap-1)<<7)+nscr)
				looped = true;
		}
		
		done.push_back(((nmap-1)<<7)+nscr);
	}
}

void trigger_secrets_for_screen(TriggerSource source, int32_t screen, bool high16only, int32_t single)
{
	trigger_secrets_for_screen(source, get_scr(screen), high16only, single);
}

void trigger_secrets_for_screen_internal(const screen_handles_t& screen_handles, bool from_active_screen, bool high16only, int32_t single, bool do_replay_comment)
{
	mapscr* scr = screen_handles[0].scr;
	int screen = scr->screen;

	// TODO(replays): No real reason for "do_replay_comment" to exist - I just did not want to update many replays when fixing
	// slopes in sideview mode (which required loading nearby screens in loadscr).
	// TODO(replays): This should just use `screen`.
	if (replay_is_active() && do_replay_comment)
		replay_step_comment(fmt::format("trigger secrets scr={}", from_active_screen && scr != special_warp_return_scr ? screen : cur_screen));

	if (from_active_screen)
	{
		for_every_combo_in_screen(screen_handles, [&](const auto& handle) {
			auto cid = handle.data();
			auto& cmb = handle.combo();	
			for(size_t idx = 0; idx < cmb.triggers.size(); ++idx)
			{
				auto& trig = cmb.triggers[idx];
				if (trig.triggerflags[2] & combotriggerSECRETSTR)
				{
					do_trigger_combo(handle, idx, ctrigSECRETS);
					if(handle.data() != cid) break;
				}
			}
		});
	}

	int32_t ft=0; //Flag trigger?
	int32_t msflag=0; // Misc. secret flag
	
	for(int32_t i=0; i<176; i++) //Do the 'trigger flags' (non 16-31)
	{
		if(single>=0 && i!=single) continue; //If it's got a singular flag and i isn't where the flag is
		
		// Remember the misc. secret flag; if triggered, use this instead
		if(scr->sflag[i]>=mfSECRETS01 && scr->sflag[i]<=mfSECRETS16)
			msflag=sSECRET01+(scr->sflag[i]-mfSECRETS01);
		else if(combobuf[scr->data[i]].flag>=mfSECRETS01 && combobuf[scr->data[i]].flag<=mfSECRETS16)
			msflag=sSECRET01+(combobuf[scr->data[i]].flag-mfSECRETS01);
		else
			msflag=0;
			
		if(!high16only || single>=0)
		{
			int32_t newflag = -1;
			
			for(int32_t iter=0; iter<2; ++iter)
			{
				int32_t checkflag=combobuf[scr->data[i]].flag; //Inherent
				
				if(iter==1) checkflag=scr->sflag[i]; //Placed
				
				ft = combo_trigger_flag_to_secret_combo_index(checkflag);
				if (ft != -1)  //Change the combos for the secret
				{
					// Use misc. secret flag instead if one is present
					if(msflag!=0)
						ft=msflag;

					rpos_handle_t rpos_handle;
					if (from_active_screen)
					{
						rpos_handle = get_rpos_handle_for_scr(scr, 0, i);
						screen_combo_modify_preroutine(rpos_handle);
					}

					if(ft==sSECNEXT)
					{
						scr->data[i]++;
					}
					else
					{
						scr->data[i] = scr->secretcombo[ft];
						scr->cset[i] = scr->secretcset[ft];
					}
					newflag = scr->secretflag[ft];

					if (from_active_screen)
						screen_combo_modify_postroutine(rpos_handle);
				}
			}
			
			if(newflag >-1) scr->sflag[i] = newflag; //Tiered secret
			
			for(int32_t j=1; j<=6; j++)  //Layers
			{
				mapscr* layer_scr = screen_handles[j].scr;
				if (!layer_scr) continue;
				
				if(single>=0 && i!=single) continue; //If it's got a singular flag and i isn't where the flag is
				
				int32_t newflag2 = -1;
				
				// Remember the misc. secret flag; if triggered, use this instead
				if(layer_scr->sflag[i]>=mfSECRETS01 && layer_scr->sflag[i]<=mfSECRETS16)
					msflag=sSECRET01+(layer_scr->sflag[i]-mfSECRETS01);
				else if(combobuf[layer_scr->data[i]].flag>=mfSECRETS01 && combobuf[layer_scr->data[i]].flag<=mfSECRETS16)
					msflag=sSECRET01+(combobuf[layer_scr->data[i]].flag-mfSECRETS01);
				else
					msflag=0;
					
				for(int32_t iter=0; iter<2; ++iter)
				{
					int32_t checkflag=combobuf[layer_scr->data[i]].flag; //Inherent
					if(iter==1) checkflag=layer_scr->sflag[i];  //Placed

					ft = combo_trigger_flag_to_secret_combo_index(checkflag);
					if (ft != -1)  //Change the combos for the secret
					{
						// Use misc. secret flag instead if one is present
						if(msflag!=0)
							ft=msflag;
						
						if(ft==sSECNEXT)
						{
							layer_scr->data[i]++;
						}
						else
						{
							layer_scr->data[i] = layer_scr->secretcombo[ft];
							layer_scr->cset[i] = layer_scr->secretcset[ft];
						}
						newflag2 = layer_scr->secretflag[ft];
						int32_t c=layer_scr->data[i];
						int32_t cs=layer_scr->cset[i];
						
						if (from_active_screen && combobuf[c].type==cSPINTILE1)  //Surely this means we can have spin tiles on layers 3+? Isn't that bad? ~Joe123
						{
							auto [offx, offy] = translate_screen_coordinates_to_world(screen, COMBOX(i), COMBOY(i));
							addenemy(screen,offx,offy,(cs<<12)+eSPINTILE1,combobuf[c].o_tile+zc_max(1,combobuf[c].frames));
						}
					}
				}
				
				if(newflag2 >-1) layer_scr->sflag[i] = newflag2;  //Tiered secret
			}
		}
	}
	
	word c = scr->numFFC();
	for(word i=0; i<c; i++) //FFC 'trigger flags'
	{
		if(single>=0) if(i+176!=single) continue;
		
		if((!high16only)||(single>=0))
		{
			//for (int32_t iter=0; iter<1; ++iter) // Only one kind of FFC flag now.
			{
				int32_t checkflag=combobuf[scr->ffcs[i].data].flag; //Inherent
				//No placed flags yet

				ft = combo_trigger_flag_to_secret_combo_index(checkflag);
				if (ft != -1)  //Change the ffc's combo
				{
					if(ft==sSECNEXT)
					{
						zc_ffc_modify(scr->ffcs[i], 1);
					}
					else
					{
						zc_ffc_set(scr->ffcs[i], scr->secretcombo[ft]);
						scr->ffcs[i].cset = scr->secretcset[ft];
					}
				}
			}
		}
	}
	
	if(checktrigger) //Hit all triggers->16-31
	{
		checktrigger=false;
		
		if(scr->flags6&fTRIGGERF1631)
		{
			int32_t tr = findtrigger(screen);  //Normal flags
			
			if(tr)
			{
				Z_eventlog("Hit All Triggers->16-31 not fulfilled (%d trigger flag%s remain).\n", tr, tr>1?"s":"");
				goto endhe;
			}
		}
	}
	
	for(int32_t i=0; i<176; i++) // Do the 16-31 secrets
	{
		//If it's an enemies->secret screen, only do the high 16 if told to
		//That way you can have secret and burn/bomb entrance separately
		bool old_enem_secret = get_qr(qr_ENEMIES_SECRET_ONLY_16_31);
		if(((!(old_enem_secret && (scr->flags2&fCLEARSECRET)) /*Enemies->Secret*/ && single < 0) || high16only || scr->flags4&fENEMYSCRTPERM))
		{
			int32_t newflag = -1;
			
			for(int32_t iter=0; iter<2; ++iter)
			{
				int32_t checkflag=combobuf[scr->data[i]].flag; //Inherent
				
				if(iter==1) checkflag=scr->sflag[i];  //Placed
				
				if((checkflag > 15)&&(checkflag < 32)) //If we've got a 16->32 flag change the combo
				{
					rpos_handle_t rpos_handle;
					if (from_active_screen)
					{
						rpos_handle = get_rpos_handle_for_scr(scr, 0, i);
						screen_combo_modify_preroutine(rpos_handle);
					}

					scr->data[i] = scr->secretcombo[checkflag-16+4];
					scr->cset[i] = scr->secretcset[checkflag-16+4];
					newflag = scr->secretflag[checkflag-16+4];

					if (from_active_screen)
						screen_combo_modify_postroutine(rpos_handle);
				}
			}
			
			if(newflag >-1) scr->sflag[i] = newflag;  //Tiered flag

			for(int32_t j=1; j<=6; j++)  //Layers
			{
				mapscr* layer_scr = screen_handles[j].scr;
				if (!layer_scr) continue;
				
				int32_t newflag2 = -1;
				
				for(int32_t iter=0; iter<2; ++iter)
				{
					int32_t checkflag=combobuf[layer_scr->data[i]].flag; //Inherent
					
					if(iter==1) checkflag=layer_scr->sflag[i];  //Placed
					
					if((checkflag > 15)&&(checkflag < 32)) //If we've got a 16->32 flag change the combo
					{
						layer_scr->data[i] = layer_scr->secretcombo[checkflag-16+4];
						layer_scr->cset[i] = layer_scr->secretcset[checkflag-16+4];
						newflag2 = layer_scr->secretflag[checkflag-16+4];
					}
				}
				
				if(newflag2 >-1) layer_scr->sflag[i] = newflag2;  //Tiered flag
			}
		}
	}
	
	for(word i=0; i<c; i++) // FFCs
	{
		if((!(scr->flags2&fCLEARSECRET) /*Enemies->Secret*/ && single < 0) || high16only || scr->flags4&fENEMYSCRTPERM)
		{
			int32_t checkflag=combobuf[scr->ffcs[i].data].flag; //Inherent
			
			//No placed flags yet
			if((checkflag > 15)&&(checkflag < 32)) //If we find a flag, change the combo
			{
				if (from_active_screen)
					zc_ffc_set(scr->ffcs[i], scr->secretcombo[checkflag - 16 + 4]);
				else
					scr->ffcs[i].data = scr->secretcombo[checkflag - 16 + 4];
				scr->ffcs[i].cset = scr->secretcset[checkflag-16+4];
			}
		}
	}
	
endhe:

	if (scr->flags4&fDISABLETIME) //Finish timed warp if 'Secrets Disable Timed Warp'
	{
		if (from_active_screen)
			activated_timed_warp = true;
		scr->timedwarptics = 0;
	}
}

// x,y are world coordinates.
// Returns true if there is a flag (either combo, screen, or ffc) at (x, y).
// Out parameters will be set if the flag is Trigger->Self, which modifies how secrets will be triggered.
static bool has_flag_trigger(int32_t x, int32_t y, int32_t flag, rpos_t& out_rpos, bool& out_single16)
{
	if (!is_in_world_bounds(x, y)) return false;

    bool found_cflag = false;
    bool found_nflag = false;
	bool single16 = false;
	rpos_t rpos = rpos_t::None;

	for (int32_t layer = -1; layer < 6; layer++)
	{
		if (MAPFLAG2(layer, x, y) == flag)
		{
			found_nflag = true;
			break;
		}
	}

	for (int32_t layer = -1; layer < 6; layer++)
	{
		if (MAPCOMBOFLAG2(layer, x, y) == flag)
		{
			found_cflag = true;
			break;
		}
	}

	for (int32_t i=-1; i<6; i++)  // Look for Trigger->Self on all layers
    {
        if (found_nflag) // Trigger->Self (a.k.a Singular) is inherent
        {
            if ((MAPCOMBOFLAG2(i, x, y) == mfSINGLE) && (MAPFLAG2(i, x, y) == flag))
            {
                rpos = COMBOPOS_REGION(x, y);
            }
            else if ((MAPCOMBOFLAG2(i, x, y) == mfSINGLE16) && (MAPFLAG2(i, x, y) == flag))
            {
                rpos = COMBOPOS_REGION(x, y);
                single16 = true;
            }
        }
        
        if (found_cflag) // Trigger->Self (a.k.a Singular) is non-inherent
        {
            if ((MAPFLAG2(i, x, y) == mfSINGLE) && (MAPCOMBOFLAG2(i, x, y) == flag))
            {
                rpos = COMBOPOS_REGION(x, y);
            }
            else if ((MAPFLAG2(i, x, y) == mfSINGLE16) && (MAPCOMBOFLAG2(i, x, y) == flag))
            {
                rpos = COMBOPOS_REGION(x, y);
                single16 = true;
            }
        }
    }

	out_rpos = rpos;
	out_single16 = single16;
	return found_nflag || found_cflag || MAPFFCOMBOFLAG(x,y) == flag;
}

bool trigger_secrets_if_flag(int32_t x, int32_t y, int32_t flag, bool setflag)
{
	if (x < -16 || y < -16 || x >= world_w || y >= world_h) return false;

	mapscr* scr = NULL;
	int32_t screen = -1;
	rpos_t trigger_rpos = rpos_t::None;
	bool single16 = false;

	std::vector<std::pair<int, int>> coords;
	coords.push_back({x, y});
	coords.push_back({x + 15, y});
	coords.push_back({x, y + 15});
	coords.push_back({x + 15, y + 15});
	std::vector<rpos_t> rposes_seen;
	for (auto [x, y] : coords)
	{
		rpos_t rpos = COMBOPOS_REGION_B(x, y);
		if (rpos == rpos_t::None)
			continue;

		if (MAPFFCOMBOFLAG(x, y) == flag)
		{
			screen = get_screen_for_world_xy(x, y);
			break;
		}

		bool seen = false;
		for (rpos_t r : rposes_seen)
		{
			if (r == rpos)
			{
				seen = true;
				break;
			}
		}
		if (seen)
			continue;

		rposes_seen.push_back(rpos);
		if (has_flag_trigger(x, y, flag, trigger_rpos, single16))
		{
			screen = get_screen_for_world_xy(x, y);
			break;
		}
	}

	if (screen != -1) scr = get_scr(screen);
	if (!scr) return false;

	if (trigger_rpos == rpos_t::None)
	{
		checktrigger = true;
		trigger_secrets_for_screen(TriggerSource::Unspecified, screen);
	}
	else
	{
		checktrigger = true;
		trigger_secrets_for_screen(TriggerSource::Singular, scr, single16, RPOS_TO_POS(trigger_rpos));
	}
	
	sfx(scr->secretsfx);
	
	if(scr->flags6&fTRIGGERFPERM)
	{
		int32_t flags_remaining = findtrigger(screen);  //Normal flags
		
		if (flags_remaining)
		{
			Z_eventlog("Hit All Triggers->Perm Secret not fulfilled (%d trigger flag%s remain).\n", flags_remaining, flags_remaining>1?"s":"");
			setflag=false;
		}
		
		// Only actually trigger secrets now if 1) all triggers are gone and 2) QR qr_ALLTRIG_PERMSEC_NO_TEMP is off, in
		// which case only the screen state for mSECRET may be set below.
		if (!flags_remaining && !get_qr(qr_ALLTRIG_PERMSEC_NO_TEMP))
		{
			trigger_secrets_for_screen(TriggerSource::Unspecified, scr, scr->flags6&fTRIGGERF1631, -1);
		}
	}
	
	if (setflag && canPermSecret(cur_dmap, screen))
		if(!(scr->flags5&fTEMPSECRETS))
			setmapflag(scr, mSECRET);

	return true;
}

void update_slopes()
{
	for (auto& p : slopes)
	{
		slope_object& s = p.second;
		s.updateslope(); //sets old x/y poses
	}
}

void update_freeform_combos()
{
	ffscript_engine(false);
	if ( !FFCore.system_suspend[susptUPDATEFFC] )
	{
		int wrap_right = world_w + 32;
		int wrap_bottom = world_h + 32;

		for_every_ffc([&](const ffc_handle_t& ffc_handle) {
			mapscr* scr = ffc_handle.scr;
			ffcdata& thisffc = *ffc_handle.ffc;

			// Combo 0?
			if(thisffc.data==0)
				return;
				
			// Changer?
			if(thisffc.flags&ffc_changer)
				return;
				
			// Stationary?
			if(thisffc.flags&ffc_stationary)
				return;
				
			// Frozen because Hero's holding up an item?
			if(Hero.getHoldClk()>0 && (thisffc.flags&ffc_ignoreholdup)==0)
				return;
				
			// Check for changers
			if (thisffc.link==0)
			{
				for_some_ffcs([&](const ffc_handle_t& other_ffc_handle) {
					if (ffc_handle.id == other_ffc_handle.id)
						return true;

					ffcdata& otherffc = *other_ffc_handle.ffc;
					// Combo 0?
					if(otherffc.data==0)
						return true;
						
					// Not a changer?
					if(!(otherffc.flags&ffc_changer))
						return true;
						
					// Ignore this changer?
					if((otherffc.x.getInt()==thisffc.changer_x&&otherffc.y.getInt()==thisffc.changer_y) || thisffc.flags&ffc_ignorechanger)
						return true;
						
					if((isonline(thisffc.x.getZLong(), thisffc.y.getZLong(), thisffc.prev_changer_x, thisffc.prev_changer_y, otherffc.x.getZLong(), otherffc.y.getZLong()) || // Along the line, or...
						( // At exactly the same position, 
							(thisffc.x==otherffc.x && thisffc.y==otherffc.y)) 
							||
							//or imprecision and close enough
							( (thisffc.flags&ffc_imprecisionchanger) && ((abs(thisffc.x.getZLong() - otherffc.x.getZLong()) < 10000) && abs(thisffc.y.getZLong() - otherffc.y.getZLong()) < 10000) )
						)
					&& //and...
						(thisffc.prev_changer_x>-10000000 && thisffc.prev_changer_y>-10000000)) // This isn't the first frame on this screen
					{
						zc_ffc_changer(thisffc, otherffc, ffc_handle.id, other_ffc_handle.id);
						return false;
					}

					return true;
				});
			}

			ffcdata* linked_ffc = thisffc.link ? get_ffc_handle(thisffc.link - 1).ffc : nullptr;
			if (linked_ffc ? !linked_ffc->delay : !thisffc.delay)
			{
				if(thisffc.link && (thisffc.link-1) != ffc_handle.id)
				{
					thisffc.prev_changer_x = thisffc.x.getZLong();
					thisffc.prev_changer_y = thisffc.y.getZLong();
					thisffc.x += linked_ffc->vx;
					thisffc.y += linked_ffc->vy;
				}
				else
				{
					thisffc.prev_changer_x = thisffc.x.getZLong();
					thisffc.prev_changer_y = thisffc.y.getZLong();
					thisffc.x += thisffc.vx;
					thisffc.y += thisffc.vy;
					thisffc.vx += thisffc.ax;
					thisffc.vy += thisffc.ay;
					
					
					if(get_qr(qr_OLD_FFC_SPEED_CAP))
					{
						if(thisffc.vx>128) thisffc.vx=128;
						
						if(thisffc.vx<-128) thisffc.vx=-128;
						
						if(thisffc.vy>128) thisffc.vy=128;
						
						if(thisffc.vy<-128) thisffc.vy=-128;
					}
				}
			}
			else
			{
				if(!thisffc.link || (thisffc.link-1) == ffc_handle.id)
					thisffc.delay--;
			}
			
			// Check if the FFC's off the side of the screen
			
			// Left
			if(thisffc.x<-32)
			{
				if(scr->flags6&fWRAPAROUNDFF)
				{
					thisffc.x = wrap_right+(thisffc.x+32);
					thisffc.solid_update(false);
					thisffc.prev_changer_y = thisffc.y.getZLong();
					// Re-enable previous changer
					thisffc.changer_x = -1000;
					thisffc.changer_y = -1000;
				}
				else if(thisffc.x<-64)
				{
					zc_ffc_set(thisffc, 0);
					thisffc.flags&=~ffc_carryover;
				}
			}
			// Right
			else if(thisffc.x>=wrap_right)
			{
				if(scr->flags6&fWRAPAROUNDFF)
				{
					thisffc.x = thisffc.x-wrap_right-32;
					thisffc.solid_update(false);
					thisffc.prev_changer_y = thisffc.y.getZLong();
					thisffc.changer_x = -1000;
					thisffc.changer_y = -1000;
				}
				else
				{
					zc_ffc_set(thisffc, 0);
					thisffc.flags&=~ffc_carryover;
				}
			}
			
			// Top
			if(thisffc.y<-32)
			{
				if(scr->flags6&fWRAPAROUNDFF)
				{
					thisffc.y = wrap_bottom+(thisffc.y+32);
					thisffc.solid_update(false);
					thisffc.prev_changer_x = thisffc.x.getZLong();
					thisffc.changer_x = -1000;
					thisffc.changer_y = -1000;
				}
				else if(thisffc.y<-64)
				{
					zc_ffc_set(thisffc, 0);
					thisffc.flags&=~ffc_carryover;
				}
			}
			// Bottom
			else if(thisffc.y>=wrap_bottom)
			{
				if(scr->flags6&fWRAPAROUNDFF)
				{
					thisffc.y = thisffc.y-wrap_bottom-32;
					thisffc.solid_update(false);
					thisffc.prev_changer_y = thisffc.x.getZLong();
					thisffc.changer_x = -1000;
					thisffc.changer_y = -1000;
				}
				else
				{
					zc_ffc_set(thisffc, 0);
					thisffc.flags&=~ffc_carryover;
				}
			}
			thisffc.solid_update();
		});
	}
}

bool hitcombo(int32_t x, int32_t y, int32_t combotype, byte layers)
{
	for(int q = 0; q < 7; ++q)
	{
		if(layers&(1<<q)) //if layer is to be checked
			if(COMBOTYPE2(q-1,x,y)==combotype) //matching type
				return true;
	}
	return false;
}

int gethitcombo(int32_t x, int32_t y, int32_t combotype, byte layers)
{
	for(int q = 0; q < 7; ++q)
	{
		if(layers&(1<<q)) //if layer is to be checked
			if(COMBOTYPE2(q-1,x,y)==combotype) //matching type
				return MAPCOMBO2(q-1,x,y);
	}
	return -1;
}

bool hitflag(int32_t x, int32_t y, int32_t flagtype, byte layers)
{
	for(int q = 0; q < 7; ++q)
	{
		if(layers&(1<<q)) //if layer is to be checked
			if(MAPFLAG2(q-1,x,y)==flagtype||MAPCOMBOFLAG2(q-1,x,y)==flagtype) //matching flag
				return true;
	}
	return false;
}

optional<int> nextscr(int screen, int dir)
{
	auto [m, s] = nextscr2(cur_map, screen, dir);
	if (m == -1) return nullopt;
    return (m<<7) + s;
}

std::pair<int32_t, int32_t> nextscr2(int32_t dir)
{
	int32_t map = cur_map;
    int32_t screen = screenscrolling ? scrolling_hero_screen : hero_screen;
	return nextscr2(map, screen, dir);
}

std::pair<int32_t, int32_t> nextscr2(int map, int screen, int32_t dir)
{
	screen = screen_index_direction(screen, (direction)dir);

    // need to check for screens on other maps, 's' not valid, etc.
    int32_t index = (hero_scr->sidewarpindex >> (dir*2))&3;
    
    // Fun fact: when a scrolling warp is triggered, this function
    // is never even called! - Saf
    if(hero_scr->sidewarptype[index] == 3)                                // scrolling warp
    {
        switch(dir)
        {
        case up:
            if(!(hero_scr->flags2&wfUP))    goto nowarp;
            
            break;
            
        case down:
            if(!(hero_scr->flags2&wfDOWN))  goto nowarp;
            
            break;
            
        case left:
            if(!(hero_scr->flags2&wfLEFT))  goto nowarp;
            
            break;
            
        case right:
            if(!(hero_scr->flags2&wfRIGHT)) goto nowarp;
            
            break;
        }
        
        map = DMaps[hero_scr->sidewarpdmap[index]].map;
        screen = hero_scr->sidewarpscr[index] + DMaps[hero_scr->sidewarpdmap[index]].xoff;
    }
    
nowarp:
    if(screen<0||screen>=128)
        return {-1, -1};

    return {map, screen};
}

optional<int> nextscr_mi(int mi, int dir)
{
	int map = mi/MAPSCRSNORMAL;
	int screen = mi%MAPSCRSNORMAL;
	auto [m, s] = nextscr2(map, screen, dir);
	if (m == -1) return nullopt;
    return (m<<7) + s;
}

void bombdoor(int32_t x,int32_t y)
{
	if (!is_in_world_bounds(x, y))
		return;

	auto rpos_handle = get_rpos_handle_for_world_xy(x, y, 0);
	mapscr* scr = rpos_handle.scr;
	int screen = scr->screen;
	auto [x0, y0] = translate_screen_coordinates_to_world(rpos_handle.screen);
	#define CHECK_RECT(x,y,rx1,ry1,rx2,ry2) (isinRect(x,y,x0+rx1,y0+ry1,x0+rx2,y0+ry2))

    if(scr->door[0]==dBOMB && CHECK_RECT(x,y,100,0,139,48))
    {
        scr->door[0]=dBOMBED;
        putdoor(scr, scrollbuf, 0, dBOMBED);
        setmapflag(scr, mDOOR_UP);
        markBmap(-1, screen);
        
        if(auto v = nextscr(screen, up))
        {
            setmapflag_mi(*v, mDOOR_DOWN);
            markBmap(-1,*v-(get_currdmap()<<7));
        }
    }
    
    if(scr->door[1]==dBOMB && CHECK_RECT(x,y,100,112,139,176))
    {
        scr->door[1]=dBOMBED;
        putdoor(scr, scrollbuf, 1, dBOMBED);
        setmapflag(scr, mDOOR_DOWN);
        markBmap(-1, rpos_handle.screen);
        
        if(auto v = nextscr(rpos_handle.screen, down))
        {
            setmapflag_mi(*v, mDOOR_UP);
            markBmap(-1,*v-(get_currdmap()<<7));
        }
    }
    
    if(scr->door[2]==dBOMB && CHECK_RECT(x,y,0,60,48,98))
    {
        scr->door[2]=dBOMBED;
        putdoor(scr, scrollbuf, 2, dBOMBED);
        setmapflag(scr, mDOOR_LEFT);
        markBmap(-1, rpos_handle.screen);
        
        if(auto v = nextscr(rpos_handle.screen, left))
        {
            setmapflag_mi(*v, mDOOR_RIGHT);
            markBmap(-1,*v-(get_currdmap()<<7));
        }
    }
    
    if(scr->door[3]==dBOMB && CHECK_RECT(x,y,192,60,240,98))
    {
        scr->door[3]=dBOMBED;
        putdoor(scr, scrollbuf, 3, dBOMBED);
        setmapflag(scr, mDOOR_RIGHT);
        markBmap(-1, rpos_handle.screen);
        
        if(auto v = nextscr(rpos_handle.screen, right))
        {
            setmapflag_mi(*v, mDOOR_LEFT);
            markBmap(-1,*v-(get_currdmap()<<7));
        }
    }
}

void draw_cmb(BITMAP* dest, int32_t x, int32_t y, int32_t cid, int32_t cset,
	bool over, bool transp)
{
	if(over)
	{
		if(combobuf[cid].animflags & AF_TRANSPARENT)
			transp = !transp;
		if(transp)
			overcombotranslucent(dest, x, y, cid, cset, 128);
		else overcombo(dest, x, y, cid, cset);
	}
	else putcombo(dest, x, y, cid, cset);
}
void draw_cmb_pos(BITMAP* dest, int32_t x, int32_t y, rpos_t rpos, int32_t cid,
	int32_t cset, byte layer, bool over, bool transp)
{
	if (rpos != rpos_t::None)
	{
		rpos_t plrpos = COMBOPOS_REGION_B(Hero.x+8, Hero.y+8);
		if (plrpos != rpos_t::None)
		{
			bool dosw = false;
			if (rpos == hooked_comborpos && (hooked_layerbits & (1<<layer)))
			{
				if(hooked_undercombos[layer] > -1)
				{
					draw_cmb(dest, x, y,
						hooked_undercombos[layer], hooked_undercombos[layer+7], over, transp);
				}
				dosw = true;
			}
			else if (rpos == plrpos && (hooked_layerbits & (1<<(layer+8))))
			{
				dosw = true;
			}
			if (dosw)
			{
				switch (Hero.switchhookstyle)
				{
					default: case swPOOF:
						break; //Nothing special here
					case swFLICKER:
					{
						if(abs(Hero.switchhookclk-33)&0b1000)
							break; //Drawn this frame
						return; //Not drawn this frame
					}
					case swRISE:
					{
						//Draw rising up
						y -= 8-(abs(Hero.switchhookclk-32)/4);
						break;
					}
				}
			}
		}
	}

	draw_cmb(dest, x, y, cid, cset, over, transp);
}

// `draw_cmb_pos` only does meaningful work if the combo being drawn is within the bounds of
// the `bmp` bitmap. However, even getting to the point where `puttile16` (for example) knows
// to cull is somewhat expensive. Since it can only draw 16x16 pixels, we can do the equivalent
// culling here by determining the rows/columns that are within the bitmap bounds. This nets
// on the order of ~30 FPS in uncapped mode on my machine, depending on the viewport/region size.
//
// These two inequalities must be true for `draw_cmb_pos` to do anything useful:
//
//     -16 < comboPositionX*16 + x < bitmapWidth
//     -16 < comboPositionY*16 + y < bitmapHeight
//
// The following start/end values are derived directly from the above.
//
// `x` and `y` are the offsets the combos will be drawn into the bitmap.
static void get_bounds_for_draw_cmb_calls(BITMAP* bmp, int x, int y, int& start_x, int& end_x, int& start_y, int& end_y)
{
	// if (bmp->clip)
	// {
	// 	start_x = MAX(0,  ceil((bmp->cl - 15 - x)    / 16.0));
	// 	end_x   = MIN(16, ceil((bmp->cr - x) / 16.0));
	// 	start_y = MAX(0,  ceil((bmp->ct - 15 - y)    / 16.0));
	// 	end_y   = MIN(11, ceil((bmp->cb - y) / 16.0));
	// 	return;
	// }

	start_x = MAX(0,  ceil((-15 - x)    / 16.0));
	end_x   = MIN(16, ceil((bmp->w - x) / 16.0));
	start_y = MAX(0,  ceil((-15 - y)    / 16.0));
	end_y   = MIN(11, ceil((bmp->h - y) / 16.0));
}

void do_ffc_layer(BITMAP* bmp, int32_t layer, const screen_handle_t& screen_handle, int32_t x, int32_t y)
{
	if(!show_ffcs) return;
	mapscr* scr = screen_handle.scr;
	mapscr* base_scr = screen_handle.base_scr;
	
	y += playing_field_offset;

	bool is_bg_layer = layer < -1;
	int real_layer = is_bg_layer ? abs(layer) : layer;
	for(int32_t i = (base_scr->numFFC()-1); i >= 0; --i)
	{
		if (base_scr->ffcs[i].data == 0)
			continue;
		if(real_layer == 2 && (is_bg_layer != XOR(base_scr->flags7&fLAYER2BG, DMaps[cur_dmap].flags&dmfLAYER2BG)))
			continue;
		else if (real_layer == 3 && (is_bg_layer != XOR(base_scr->flags7&fLAYER3BG, DMaps[cur_dmap].flags&dmfLAYER3BG)))
			continue;
		if (real_layer > -1 && base_scr->ffcs[i].layer != real_layer)
			continue;

		if (screenscrolling && (base_scr->ffcs[i].flags & ffc_carryover) != 0 && screen_handle.screen != scrolling_hero_screen)
			continue; //If scrolling, only draw carryover ffcs from newscr and not oldscr.

		base_scr->ffcs[i].draw_ffc(bmp, x, y, (layer==-1));
	}
}
void _do_current_ffc_layer(BITMAP* bmp, int32_t layer)
{
	if(!show_ffcs) return;
	for_every_base_screen_in_region([&](mapscr* base_scr, unsigned int region_scr_x, unsigned int region_scr_y) {
		screen_handle_t handle = {base_scr, base_scr, base_scr->screen, 0};
		do_ffc_layer(bmp, layer, handle, 0, 0);
	});
}
void do_scrolling_layer(BITMAP *bmp, int32_t type, const screen_handle_t& screen_handle, int32_t x, int32_t y)
{
	mapscr* scr = screen_handle.scr;
	mapscr* base_scr = screen_handle.base_scr;

	if (type == -3 || type == -4)
	{
		y += playing_field_offset;

		for(int32_t i = (base_scr->numFFC()-1); i >= 0; --i)
		{
			if (base_scr->ffcs[i].data == 0)
				continue;

			if (screenscrolling && (base_scr->ffcs[i].flags & ffc_carryover) != 0 && screen_handle.screen != scrolling_hero_screen)
				continue; //If scrolling, only draw carryover ffcs from newscr and not oldscr.

			base_scr->ffcs[i].draw_ffc(bmp, x, y, (type==-4));
		}
		return;
	}

	x -= viewport.x;
	y -= viewport.y - playing_field_offset;

	bool over = true, transp = false;
	int layer = screen_handle.layer;
	
	switch(type ? type : layer)
	{
		case -2:                                                //push blocks
			if (scr)
			{
				for(int32_t i=0; i<176; i++)
				{
					int32_t mf=scr->sflag[i], mf2 = combobuf[scr->data[i]].flag;
					
					if(mf==mfPUSHUD || mf==mfPUSH4 || mf==mfPUSHED || ((mf>=mfPUSHLR)&&(mf<=mfPUSHRINS))
						|| mf2==mfPUSHUD || mf2==mfPUSH4 || mf2==mfPUSHED || ((mf2>=mfPUSHLR)&&(mf2<=mfPUSHRINS)))
					{
						auto rpos = screenscrolling || ViewingMap ? rpos_t::None : POS_TO_RPOS(i, screen_handle.screen);
						draw_cmb_pos(bmp, x + COMBOX(i), y + COMBOY(i), rpos, scr->data[i], scr->cset[i], layer, true, false);
					}
				}
			}
			return;
			
		case -1:                                                //over combo
			if (scr)
			{
				for(int32_t i=0; i<176; i++)
				{
					if(combo_class_buf[combobuf[scr->data[i]].type].overhead)
					{
						auto rpos = screenscrolling || ViewingMap ? rpos_t::None : POS_TO_RPOS(i, screen_handle.screen);
						draw_cmb_pos(bmp, x + COMBOX(i), y + COMBOY(i), rpos, scr->data[i], scr->cset[i], layer, true, false);
					}
				}
			}
			return;
			
		case 1:
		case 4:
		case 5:
		case 6:
			if(TransLayers || base_scr->layeropacity[layer-1]==255)
			{
				if (scr)
				{
					if(base_scr->layeropacity[layer-1]!=255)
						transp = true;
					break;
				}
			}
			return;
			
		case 2:
			if(TransLayers || base_scr->layeropacity[layer-1]==255)
			{
				if (scr)
				{
					if(base_scr->layeropacity[layer-1]!=255)
						transp = true;
					
					if(XOR(base_scr->flags7&fLAYER2BG, DMaps[cur_dmap].flags&dmfLAYER2BG))
						over = false;
					
					break;
				}
			}
			return;
			
		case 3:
			if(TransLayers || base_scr->layeropacity[layer-1]==255)
			{
				if (scr)
				{
					if(base_scr->layeropacity[layer-1]!=255)
						transp = true;
					
					if(XOR(base_scr->flags7&fLAYER3BG, DMaps[cur_dmap].flags&dmfLAYER3BG)
						&& !XOR(base_scr->flags7&fLAYER2BG, DMaps[cur_dmap].flags&dmfLAYER2BG))
						over = false;
					
					break;
				}
			}
			return;
	}

	int start_x, end_x, start_y, end_y;
	get_bounds_for_draw_cmb_calls(bmp, x, y, start_x, end_x, start_y, end_y);
	for (int cy = start_y; cy < end_y; cy++)
	{
		for (int cx = start_x; cx < end_x; cx++)
		{
			int i = cx + cy*16;
			auto rpos = screenscrolling || ViewingMap ? rpos_t::None : POS_TO_RPOS(i, screen_handle.screen);
			draw_cmb_pos(bmp, x + COMBOX(i), y + COMBOY(i), rpos, scr->data[i], scr->cset[i], layer, over, transp);
		}
	}
}

bool lenscheck(mapscr* scr, int layer)
{
	if(layer < 0 || layer > 6) return true;
	if(get_qr(qr_OLD_LENS_LAYEREFFECT))
	{
		if(!layer) return true;
		if((layer==(int32_t)(scr->lens_layer&7)+1) && ((scr->lens_layer&llLENSSHOWS && !lensclk) || (scr->lens_layer&llLENSHIDES && lensclk)))
			return false;
	}
	else
	{
		if((lensclk ? scr->lens_hide : scr->lens_show) & (1<<layer))
			return false;
	}
	return true;
}

void do_layer(BITMAP *bmp, int32_t type, const screen_handle_t& screen_handle, int32_t x, int32_t y)
{
	bool showlayer = true;
	mapscr* base_scr = screen_handle.base_scr;
	int layer = screen_handle.layer;
	int target = type ? type : layer;
	switch(target)
	{
		case -2:
			if(!show_layer_push)
				showlayer = false;
			break;
			
		case -1:
			if(!show_layer_over)
				showlayer = false;
			break;
			
		case 1: case 2: case 3:
		case 4: case 5: case 6:
			showlayer = show_layers[target];
			break;
	}

	if(!type)
	{
		if(!lenscheck(base_scr,layer))
			showlayer = false;
	}

	if(showlayer)
	{
		if (screen_handle.scr && (type || !(base_scr->hidelayers & (1 << (layer)))))
		{
			do_scrolling_layer(bmp, type, screen_handle, x, y);
			if(!type && !get_qr(qr_PUSHBLOCK_SPRITE_LAYER))
				if(mblock2.draw(bmp,layer))
					do_primitives(bmp, SPLAYER_MOVINGBLOCK);
		}
	}
}

void do_layer_primitives(BITMAP *bmp, int32_t layer)
{
	bool showlayer = true;
	
	if(layer >= 0 && layer <= 6)
	{
		showlayer = show_layers[layer];
		
		if(!lenscheck(origin_scr,layer))
			showlayer = false;
	}
	
	if (showlayer)
		do_primitives(bmp, layer);
}

// Called by do_walkflags
void put_walkflags(BITMAP *dest,int32_t x,int32_t y,int32_t xofs,int32_t yofs, word cmbdat,int32_t lyr)
{
	newcombo const &c = combobuf[cmbdat];
	
	if (c.type == cBRIDGE && get_qr(qr_OLD_BRIDGE_COMBOS)) return;
	
	int32_t xx = x-xofs;
	int32_t yy = y+playing_field_offset-yofs;
	
	int32_t bridgedetected = 0;
	
	for(int32_t i=0; i<4; i++)
	{
		int32_t tx=((i&2)<<2)+xx - viewport.x;
		int32_t ty=((i&1)<<3)+yy - viewport.y;
		int32_t tx2=((i&2)<<2)+x - viewport.x;
		int32_t ty2=((i&1)<<3)+y - viewport.y;
		for (int32_t j = lyr-1; j <= 1; j++)
		{
			if (get_qr(qr_OLD_BRIDGE_COMBOS))
			{
				if (combobuf[MAPCOMBO2(j,tx2,ty2)].type == cBRIDGE && !_walkflag_layer(tx2,ty2,j)) 
				{
					bridgedetected |= (1<<i);
				}
			}
			else
			{
				if (combobuf[MAPCOMBO2(j,tx2,ty2)].type == cBRIDGE && _effectflag_layer(tx2,ty2,j)) 
				{
					bridgedetected |= (1<<i);
				}
			}
		}
		if ((bridgedetected & (1<<i))) 
		{
			if (i >= 3) break;
			else continue;
		}
		bool doladdercheck = true;
		
		if ((c.walk&(1<<(i+4))) && ((c.walk&(1<<i) && ((c.usrflags&cflag4)) && c.type == cWATER) || c.type == cSHALLOWWATER)) 
		{
			for(int32_t k=0; k<8; k+=2)
				for(int32_t j=0; j<8; j+=2)
					if(((k+j)/2)%2)
						rectfill(dest,tx+k,ty+j,tx+k+1,ty+j+1,makecol(85,85,255));
		}
		if (iswater_type(c.type) && !DRIEDLAKE && !((c.walk&(1<<(i+4))) && ((c.walk&(1<<i) && ((c.usrflags&cflag3) || (c.usrflags&cflag4)) && c.type == cWATER)))) //Yes, I realize this is horribly inaccurate; the alternative is the game chugging every time you turn on walk cheats.
		{
			if (get_qr(qr_NO_SOLID_SWIM)) doladdercheck = false;
			if((lyr==0 || (get_qr(qr_WATER_ON_LAYER_1) && lyr == 1) || (get_qr(qr_WATER_ON_LAYER_2) && lyr == 2)) && get_qr(qr_DROWN))
				rectfill(dest,tx,ty,tx+7,ty+7,makecol(85,85,255));
			else rectfill(dest,tx,ty,tx+7,ty+7,makecol(170,170,170));
		}
		
		if(c.walk&(1<<i) && !(iswater_type(c.type) && DRIEDLAKE) && !((c.walk&(1<<(i+4))) && ((c.walk&(1<<i) && ((c.usrflags&cflag3) || (c.usrflags&cflag4)) && c.type == cWATER))))  // Check for dried lake (watertype && not water)
		{
			if(c.type==cLADDERHOOKSHOT && isstepable(cmbdat) && ishookshottable(xx,yy))
			{
				for(int32_t k=0; k<8; k+=2)
					for(int32_t j=0; j<8; j+=2)
						rectfill(dest,tx+k,ty+j,tx+k+1,ty+j+1,((k+j)/2)%2 ? makecol(165,105,8) : makecol(170,170,170));
			}
			else
			{
				int32_t color = makecol(178,36,36);
				
				if(isstepable(cmbdat)&& (!doladdercheck))
					color=makecol(165,105,8);
				else if((c.type==cHOOKSHOTONLY || c.type==cLADDERHOOKSHOT) && ishookshottable(xx,yy))
					color=makecol(170,170,170);
					
				rectfill(dest,tx,ty,tx+7,ty+7,color);
			}
		}
	}
	
	// Draw damage combos
	bool dmg = combo_class_buf[combobuf[MAPCOMBO2(-1,xx,yy)].type].modify_hp_amount
	   || combo_class_buf[combobuf[MAPCOMBO2(0,xx,yy)].type].modify_hp_amount
	   || combo_class_buf[combobuf[MAPCOMBO2(1,xx,yy)].type].modify_hp_amount;
			   
	if(dmg)
	{
		int32_t color = makecol(255,255,0);
		if (bridgedetected <= 0)
		{
			for(int32_t k=0; k<16; k+=2)
				for(int32_t j=0; j<16; j+=2)
				if(((k+j)/2)%2)
				{
					int32_t x0 = x - viewport.x;
					int32_t y0 = y - viewport.y;
					rectfill(dest,x0+k,y0+j,x0+k+1,y0+j+1,color);
				}
		}
		else
		{
			for(int32_t i=0; i<4; i++)
			{
				if (!(bridgedetected & (1<<i)))
				{
					int32_t tx=((i&2)<<2)+x - viewport.x;
					int32_t ty=((i&1)<<3)+y - viewport.y;
					for(int32_t k=0; k<8; k+=2)
						for(int32_t j=0; j<8; j+=2)
							if((k+j)%4 < 2) rectfill(dest,tx+k,ty+j,tx+k+1,ty+j+1,color);
				}
			}
		}
	}
}
static void put_walkflags_a5(int32_t x, int32_t y, word cmbdat, int32_t lyr)
{
	ALLEGRO_COLOR col_solid = al_map_rgba(178,36,36,info_opacity);
	ALLEGRO_COLOR col_water1 = al_map_rgba(85,85,255,info_opacity);
	ALLEGRO_COLOR col_lhook = al_map_rgba(170,170,170,info_opacity);
	ALLEGRO_COLOR col_stepable = al_map_rgba(165,105,8,info_opacity);
	ALLEGRO_COLOR col_dmg = al_map_rgba(255,255,0,info_opacity);
	newcombo const &c = combobuf[cmbdat];
	
	if (c.type == cBRIDGE && get_qr(qr_OLD_BRIDGE_COMBOS)) return;
	
	int32_t xx = x-viewport.x;
	int32_t yy = y+playing_field_offset-viewport.y;
	
	int32_t bridgedetected = 0;
	
	// Draw damage combos
	bool dmg = combo_class_buf[c.type].modify_hp_amount;
	
	for(int32_t i=0; i<4; i++)
	{
		int32_t tx=((i&2)<<2)+xx;
		int32_t ty=((i&1)<<3)+yy;
		int32_t tx2=((i&2)<<2)+x;
		int32_t ty2=((i&1)<<3)+y;
		for (int32_t m = lyr-1; m <= 1; m++)
		{
			if (get_qr(qr_OLD_BRIDGE_COMBOS))
			{
				if (m >= 0 && combobuf[MAPCOMBO2(m, tx2, ty2)].type == cBRIDGE && !_walkflag_layer(tx2, ty2, m)) 
				{
					bridgedetected |= (1<<i);
				}
			}
			else
			{
				if (m >= 0 && combobuf[MAPCOMBO2(m, tx2, ty2)].type == cBRIDGE && _effectflag_layer(tx2, ty2, m)) 
				{
					bridgedetected |= (1<<i);
				}
			}
		}
		if ((bridgedetected & (1<<i)))
			continue;
		bool doladdercheck = true;
		
		if ((c.walk&(1<<(i+4))) && ((c.walk&(1<<i) && ((c.usrflags&cflag4)) && c.type == cWATER) || c.type == cSHALLOWWATER)) 
		{
			for(int32_t k=0; k<8; k+=2)
				for(int32_t j=0; j<8; j+=2)
					if(((k+j)/2)%2)
						al_draw_filled_rectangle(tx+k,ty+j,tx+k+2,ty+j+2,col_water1);
		}
		if (iswater_type(c.type) && !DRIEDLAKE && !((c.walk&(1<<(i+4))) && ((c.walk&(1<<i) && ((c.usrflags&cflag3) || (c.usrflags&cflag4)) && c.type == cWATER)))) //Yes, I realize this is horribly inaccurate; the alternative is the game chugging every time you turn on walk cheats.
		{
			if (get_qr(qr_NO_SOLID_SWIM)) doladdercheck = false;
			if((lyr==0 || (get_qr(qr_WATER_ON_LAYER_1) && lyr == 1) || (get_qr(qr_WATER_ON_LAYER_2) && lyr == 2)) && get_qr(qr_DROWN))
				al_draw_filled_rectangle(tx,ty,tx+8,ty+8,col_water1);
			else al_draw_filled_rectangle(tx,ty,tx+8,ty+8,col_lhook);
		}
		
		if(c.walk&(1<<i) && !(iswater_type(c.type) && DRIEDLAKE) && !((c.walk&(1<<(i+4))) && ((c.walk&(1<<i) && ((c.usrflags&cflag3) || (c.usrflags&cflag4)) && c.type == cWATER))))  // Check for dried lake (watertype && not water)
		{
			if(c.type==cLADDERHOOKSHOT && isstepable(cmbdat) && ishookshottable(xx,yy))
			{
				for(int32_t k=0; k<8; k+=2)
					for(int32_t j=0; j<8; j+=2)
						al_draw_filled_rectangle(tx+k,ty+j,tx+k+2,ty+j+2,((k+j)/2)%2 ? col_stepable : col_lhook);
			}
			else
			{
				ALLEGRO_COLOR* color = &col_solid;
				
				if(isstepable(cmbdat)&& (!doladdercheck))
					color=&col_stepable;
				else if((c.type==cHOOKSHOTONLY || c.type==cLADDERHOOKSHOT) && ishookshottable(xx,yy))
					color=&col_lhook;
					
				al_draw_filled_rectangle(tx,ty,tx+8,ty+8,*color);
			}
		}
		
		if(dmg)
		{
			for(int32_t k=0; k<8; k+=2)
				for(int32_t j=0; j<8; j+=2)
					if((k+j)%4 < 2) al_draw_filled_rectangle(tx+k,ty+j,tx+k+2,ty+j+2,col_dmg);
		}
	}
}

void put_effectflags(BITMAP *dest,int32_t x,int32_t y,int32_t xofs,int32_t yofs, word cmbdat,int32_t lyr)
{
	newcombo const &c = combobuf[cmbdat];
	
	int32_t xx = x-xofs-viewport.x;
	int32_t yy = y+playing_field_offset-yofs-viewport.y;
	
	for(int32_t i=0; i<4; i++)
	{
		int32_t tx=((i&2)<<2)+xx;
		int32_t ty=((i&1)<<3)+yy;
	
		if(((c.walk>>4)&(1<<i)) && c.type != cNONE)
		{
				int32_t color = vc(10);
					
				rectfill(dest,tx,ty,tx+7,ty+7,color);
		}
	}
}
static void put_effectflags_a5(int32_t x, int32_t y, word cmbdat, int32_t lyr)
{
	ALLEGRO_COLOR col_eff = al_map_rgba(85,255,85,info_opacity);
	newcombo const &c = combobuf[cmbdat];
	
	int32_t xx = x-viewport.x;
	int32_t yy = y+playing_field_offset-viewport.y;
	
	for(int32_t i=0; i<4; i++)
	{
		int32_t tx=((i&2)<<2)+xx;
		int32_t ty=((i&1)<<3)+yy;
	
		if(((c.walk>>4)&(1<<i)) && c.type != cNONE)
		{
			al_draw_filled_rectangle(tx,ty,tx+8,ty+8,col_eff);
		}
	}
}

void draw_ladder_platform(BITMAP* dest, int32_t x, int32_t y, int32_t c)
{
	for(auto cx = 0; cx < 256; cx += 16)
	{
		for(auto cy = 0; cy < 176; cy += 16)
		{
			if(isSVLadder(cx,cy))
			{
				auto nx = cx+x, ny = cy+y;
				if(cy && !isSVLadder(cx,cy-16))
					line(dest,nx,ny,nx+15,ny,c);
				rectfill(dest,nx,ny,nx+3,ny+15,c);
				rectfill(dest,nx+12,ny,nx+15,ny+15,c);
				rectfill(dest,nx+4,ny+2,nx+11,ny+5,c);
				rectfill(dest,nx+4,ny+10,nx+11,ny+13,c);
			}
			else if(isSVPlatform(cx,cy))
			{
				line(dest,cx+x,cy+y,cx+x+15,cy+y,c);
			}
		}
	}
}
void draw_ladder_platform_a5(int32_t x, int32_t y, ALLEGRO_COLOR c)
{
	for(auto cx = 0; cx < 256; cx += 16)
	{
		for(auto cy = 0; cy < 176; cy += 16)
		{
			if(isSVLadder(cx,cy))
			{
				auto nx = cx+x, ny = cy+y;
				if(cy && !isSVLadder(cx,cy-16))
					al_draw_line(nx,ny,nx+15,ny,c,1);
				al_draw_filled_rectangle(nx,ny,nx+4,ny+16,c);
				al_draw_filled_rectangle(nx+12,ny,nx+16,ny+16,c);
				al_draw_filled_rectangle(nx+4,ny+2,nx+12,ny+6,c);
				al_draw_filled_rectangle(nx+4,ny+10,nx+12,ny+14,c);
			}
			else if(isSVPlatform(cx,cy))
			{
				al_draw_line(cx+x,cy+y,cx+x+15,cy+y,c,1);
			}
		}
	}
}

// Walkflags L4 cheat
void do_walkflags(const screen_handles_t& screen_handles, int32_t x, int32_t y)
{
	if (!show_walkflags)
		return;

	start_info_bmp();

	mapscr* scr = screen_handles[0].scr;
	for(int32_t i=0; i<176; i++)
	{
		put_walkflags_a5(((i&15)<<4) + x, (i&0xF0) + y, scr->data[i], 0);
	}

	for(int32_t k=0; k<2; k++)
	{
		scr = screen_handles[k + 1].scr;

		if (scr)
		{
			for(int32_t i=0; i<176; i++)
			{
				put_walkflags_a5(((i&15)<<4) + x, (i&0xF0) + y, scr->data[i], k%2+1);
			}
		}
	}

	end_info_bmp();
}

void do_walkflags(int32_t x, int32_t y)
{
	if (!show_walkflags)
		return;

	x += -viewport.x;
	y += playing_field_offset - viewport.y;

	start_info_bmp();

	draw_ladder_platform_a5(x,y,al_map_rgba(165,105,8,info_opacity));
	draw_solid_objects_a5(x,y,al_map_rgba(178,36,36,info_opacity));
	draw_slopes_a5(x,y,al_map_rgba(255,85,255,info_opacity));

	end_info_bmp();
}

// Effectflags L4 cheat
void do_effectflags(mapscr* scr, int32_t x, int32_t y)
{
	if(show_effectflags)
	{
		start_info_bmp();
		
		for(int32_t i=0; i<176; i++)
		{
			put_effectflags_a5(((i&15)<<4) + x, (i&0xF0) + y, scr->data[i], 0);
		}
		
		end_info_bmp();
	}
}

void calc_darkroom_combos(mapscr* scr, int offx, int offy)
{
	int map = scr->map;
	int screen = scr->screen;

	for(int32_t lyr = 0; lyr < 7; ++lyr)
	{
		mapscr* scr = get_scr_layer(map, screen, lyr);
		if (!scr->is_valid()) continue;

		for(int32_t q = 0; q < 176; ++q)
		{
			newcombo const& cmb = combobuf[scr->data[q]];
			if(cmb.type == cTORCH)
			{
				do_torch_combo(cmb, COMBOX(q)+8+offx, COMBOY(q)+8+offy, darkscr_bmp);
			}
		}
	}
}

void calc_darkroom_ffcs(mapscr* scr, int offx, int offy)
{
	word c = scr->numFFC();
	for(int q = 0; q < c; ++q)
	{
		newcombo const& cmb = combobuf[scr->ffcs[q].data];
		if(cmb.type == cTORCH)
		{
			int cx = scr->ffcs[q].x.getInt()+(scr->ffEffectWidth(q)/2)+offx;
			int cy = (scr->ffcs[q].y.getInt())+(scr->ffEffectHeight(q)/2)+offy;
			do_torch_combo(cmb, cx, cy, darkscr_bmp);
		}
	}
}

struct nearby_screen_t
{
	int screen;
	int offx;
	int offy;
	screen_handles_t screen_handles;
};
typedef std::vector<nearby_screen_t> nearby_screens_t;

static nearby_screens_t get_nearby_screens_non_scrolling_region()
{
	nearby_screens_t nearby_screens;

	mapscr* base_scr = origin_scr;
	auto& nearby_screen = nearby_screens.emplace_back();
	nearby_screen.screen = cur_screen;
	nearby_screen.screen_handles = create_screen_handles(base_scr);

	return nearby_screens;
}

static nearby_screens_t get_nearby_screens_scrolling_region()
{
	nearby_screens_t nearby_screens;

	int screens_x0 = viewport.left() / 256;
	int screens_x1 = (viewport.right() - 1) / 256;
	int screens_y0 = viewport.top() / 176;
	int screens_y1 = (viewport.bottom() - 1) / 176;

	screens_x0 = std::clamp(screens_x0, 0, 15);
	screens_x1 = std::clamp(screens_x1, 0, 15);
	screens_y0 = std::clamp(screens_y0, 0, 8);
	screens_y1 = std::clamp(screens_y1, 0, 8);

	for (int x = screens_x0; x <= screens_x1; x++)
	{
		for (int y = screens_y0; y <= screens_y1; y++)
		{
			int screen = cur_screen + x + y*16;
			if (!is_in_current_region(screen)) continue;

			mapscr* base_scr = get_scr(screen);
			if (!base_scr->is_valid()) continue;

			auto [offx, offy] = translate_screen_coordinates_to_world(screen);

			auto& nearby_screen = nearby_screens.emplace_back();
			nearby_screen.screen = screen;
			nearby_screen.offx = offx;
			nearby_screen.offy = offy;
			nearby_screen.screen_handles = create_screen_handles(base_scr);
		}
	}

	return nearby_screens;
}

static nearby_screens_t get_nearby_screens_smooth_maze()
{
	nearby_screens_t nearby_screens;

	int screens_x0 = viewport.left() / 256;
	int screens_x1 = (viewport.right() - 1) / 256;
	int screens_y0 = viewport.top() / 176;
	int screens_y1 = (viewport.bottom() - 1) / 176;

	if (viewport.left() < 0) screens_x0--;
	if (viewport.top() < 0) screens_y0--;

	for (int x = screens_x0; x <= screens_x1; x++)
	{
		for (int y = screens_y0; y <= screens_y1; y++)
		{
			int screen = -1;
			mapscr* base_scr;
			int offx, offy;

			mapscr* maze_scr = maze_state.scr;
			int maze_screen = maze_scr->screen;
			int maze_screen_x = get_region_relative_dx(maze_screen);
			int maze_screen_y = get_region_relative_dy(maze_screen);
			int maze_screen_dx = x - maze_screen_x;
			int maze_screen_dy = y - maze_screen_y;
			int exitdir = maze_scr->exitdir;

			bool should_draw_maze_screen;
			if (maze_state.lost)
			{
				should_draw_maze_screen = true;
			}
			else
			{
				should_draw_maze_screen = true;
				should_draw_maze_screen &= XY_DELTA_TO_DIR(maze_screen_dx, 0) != exitdir && XY_DELTA_TO_DIR(0, maze_screen_dy) != exitdir;
				if (maze_state.enter_dir != dir_invalid)
					should_draw_maze_screen &= XY_DELTA_TO_DIR(maze_screen_dx, 0) != maze_state.enter_dir && XY_DELTA_TO_DIR(0, maze_screen_dy) != maze_state.enter_dir;
			}

			if (should_draw_maze_screen)
			{
				screen = maze_state.scr->screen;
				base_scr = maze_state.scr;
				std::tie(offx, offy) = translate_screen_coordinates_to_world(cur_screen + x + y*16);
			}

			if (screen == -1)
			{
				screen = cur_screen + x + y*16;
				if (!is_in_current_region(screen)) continue;

				base_scr = get_scr(screen);
				if (!base_scr->is_valid()) continue;

				std::tie(offx, offy) = translate_screen_coordinates_to_world(screen);
			}

			auto& nearby_screen = nearby_screens.emplace_back();
			nearby_screen.screen = screen;
			nearby_screen.offx = offx;
			nearby_screen.offy = offy;
			nearby_screen.screen_handles = create_screen_handles(base_scr);
		}
	}

	return nearby_screens;
}

static nearby_screens_t get_nearby_screens()
{
	if (maze_state.active && maze_state.loopy)
		return get_nearby_screens_smooth_maze();

	if (is_in_scrolling_region())
		return get_nearby_screens_scrolling_region();

	return get_nearby_screens_non_scrolling_region();
}

static void for_every_nearby_screen(const nearby_screens_t& nearby_screens, const std::function <void (screen_handles_t, int, int, int)>& fn)
{
	for (auto& nearby_screen : nearby_screens)
		fn(nearby_screen.screen_handles, nearby_screen.screen, nearby_screen.offx, nearby_screen.offy);
}

static void draw_msgstr(byte layer, BITMAP* dest = nullptr)
{
	if(layer != msgstr_layer) return;
	if(!dest) dest = framebuf;

	if(!(msg_bg_display_buf->clip))
	{
		blit_msgstr_bg(dest,0,0,0,playing_field_offset,256,176);
	}
	
	if(!(msg_portrait_display_buf->clip))
	{
		blit_msgstr_prt(dest,0,0,0,playing_field_offset,256,176);
	}
	
	if(!(msg_txt_display_buf->clip))
	{
		blit_msgstr_fg(dest,0,0,0,playing_field_offset,256,176);
	}
}

static void putscrdoors(const nearby_screens_t& nearby_screens, BITMAP *dest, int32_t x, int32_t y);

static void set_draw_screen_clip(BITMAP* bmp)
{
	set_clip_rect(bmp, draw_screen_clip_rect_x1, draw_screen_clip_rect_y1, draw_screen_clip_rect_x2, draw_screen_clip_rect_y2);
}

void draw_screen(bool showhero, bool runGeneric)
{
	clear_info_bmp();
	if((GameFlags & (GAMEFLAG_SCRIPTMENU_ACTIVE|GAMEFLAG_F6SCRIPT_ACTIVE))!=0)
	{
		FFCore.doScriptMenuDraws();
		return;
	}
	
	if(runGeneric) FFCore.runGenericPassiveEngine(SCR_TIMING_PRE_DRAW);

	clear_bitmap(framebuf);
	clear_clip_rect(framebuf);
	
	int32_t cmby2=0;
	
	// Draw some background layers
	clear_bitmap(scrollbuf);

	auto nearby_screens = get_nearby_screens();

	// Handle layer 2/3 possibly being background layers.
	for_every_nearby_screen(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy) {
		mapscr* base_scr = screen_handles[0].base_scr;
		if (XOR(base_scr->flags7&fLAYER2BG, DMaps[cur_dmap].flags&dmfLAYER2BG))
			do_layer(scrollbuf, 0, screen_handles[2], offx, offy);
	});

	if (XOR(origin_scr->flags7&fLAYER2BG, DMaps[cur_dmap].flags&dmfLAYER2BG))
	{
		do_layer_primitives(scrollbuf, 2);
		particles.draw(scrollbuf, true, 2);
	}
	_do_current_ffc_layer(scrollbuf, -2);
	if (XOR(origin_scr->flags7&fLAYER2BG, DMaps[cur_dmap].flags&dmfLAYER2BG))
		draw_msgstr(2, scrollbuf);

	for_every_nearby_screen(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy) {
		mapscr* base_scr = screen_handles[0].base_scr;
		if (XOR(base_scr->flags7&fLAYER3BG, DMaps[cur_dmap].flags&dmfLAYER3BG))
			do_layer(scrollbuf, 0, screen_handles[3], offx, offy);
	});

	if (XOR(origin_scr->flags7&fLAYER3BG, DMaps[cur_dmap].flags&dmfLAYER3BG))
	{
		do_layer_primitives(scrollbuf, 3);
		particles.draw(scrollbuf, true, 3);
	}
	_do_current_ffc_layer(scrollbuf, -3);
	if (XOR(origin_scr->flags7&fLAYER3BG, DMaps[cur_dmap].flags&dmfLAYER3BG))
		draw_msgstr(3, scrollbuf);

	// Draw the main combo screens ("layer 0").
	for_every_nearby_screen(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy) {
		mapscr* base_scr = screen_handles[0].base_scr;
		if (lenscheck(base_scr, 0))
		{
			putscr(base_scr, scrollbuf, offx, offy + playing_field_offset);
		}
	});

	if (lenscheck(hero_scr, 0))
	{
		if(!get_qr(qr_PUSHBLOCK_SPRITE_LAYER))
			if(mblock2.draw(scrollbuf,0))
				do_primitives(scrollbuf, SPLAYER_MOVINGBLOCK);
	}

	// Lens hints, then primitives, then particles.
	if((lensclk || (get_debug() && zc_getkey(KEY_L))) && !get_qr(qr_OLDLENSORDER))
	{
		draw_lens_under(scrollbuf, false);
		do_primitives(scrollbuf, SPLAYER_LENS_UNDER_1);
	}
	
	if(show_layers[0] && lenscheck(hero_scr,0))
		do_primitives(scrollbuf, 0);
	particles.draw(scrollbuf, true, 0);
	_do_current_ffc_layer(scrollbuf, 0);
	draw_msgstr(0, scrollbuf);

	set_draw_screen_clip(scrollbuf);

	if(!(get_qr(qr_LAYER12UNDERCAVE)))
	{
		if(showhero &&
				((Hero.getAction()==climbcovertop)||(Hero.getAction()==climbcoverbottom)))
		{
			if(Hero.getAction()==climbcovertop)
			{
				cmby2=16;
			}
			else if(Hero.getAction()==climbcoverbottom)
			{
				cmby2=-16;
			}
			
			decorations.draw2(scrollbuf,true);
			Hero.draw(scrollbuf);
			decorations.draw(scrollbuf,true);
			int32_t ccx = (int32_t)Hero.getClimbCoverX();
			int32_t ccy = (int32_t)Hero.getClimbCoverY();
			
			overcombo(scrollbuf,ccx-viewport.x,ccy+cmby2+playing_field_offset-viewport.y,MAPCOMBO(ccx,ccy+cmby2),MAPCSET(ccx,ccy+cmby2));
			putcombo(scrollbuf,ccx-viewport.x,ccy+playing_field_offset-viewport.y,MAPCOMBO(ccx,ccy),MAPCSET(ccx,ccy));
			
			if(int32_t(Hero.getX())&15)
			{
				overcombo(scrollbuf,ccx+16-viewport.x,ccy+cmby2+playing_field_offset-viewport.y,MAPCOMBO(ccx+16,ccy+cmby2),MAPCSET(ccx+16,ccy+cmby2));
				putcombo(scrollbuf,ccx+16-viewport.x,ccy+playing_field_offset-viewport.y,MAPCOMBO(ccx+16,ccy),MAPCSET(ccx+16,ccy));
			}
		}
	}

	for_every_nearby_screen(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy) {
		do_layer(scrollbuf, 0, screen_handles[1], offx, offy); // LAYER 1
	});

	do_layer_primitives(scrollbuf, 1);
	particles.draw(scrollbuf, true, 1);
	_do_current_ffc_layer(scrollbuf, 1);
	draw_msgstr(1, scrollbuf);

	// Handle layer 2 NOT being used as background layers.
	for_every_nearby_screen(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy) {
		mapscr* base_scr = screen_handles[0].base_scr;
		if (!XOR(base_scr->flags7&fLAYER2BG, DMaps[cur_dmap].flags&dmfLAYER2BG))
			do_layer(scrollbuf, 0, screen_handles[2], offx, offy);
	});

	if(!XOR(origin_scr->flags7&fLAYER2BG, DMaps[cur_dmap].flags&dmfLAYER2BG))
	{
		do_layer_primitives(scrollbuf, 2);
		particles.draw(scrollbuf, true, 2);
	}
	_do_current_ffc_layer(scrollbuf, 2);
	if(!XOR(origin_scr->flags7&fLAYER2BG, DMaps[cur_dmap].flags&dmfLAYER2BG))
		draw_msgstr(2, scrollbuf);

	do_primitives(scrollbuf, SPLAYER_FFC_DRAW);

	if(get_qr(qr_LAYER12UNDERCAVE))
	{
		if(showhero &&
				((Hero.getAction()==climbcovertop)||(Hero.getAction()==climbcoverbottom)))
		{
			if(Hero.getAction()==climbcovertop)
			{
				cmby2=16;
			}
			else if(Hero.getAction()==climbcoverbottom)
			{
				cmby2=-16;
			}
			
			decorations.draw2(scrollbuf,true);
			Hero.draw(scrollbuf);
			decorations.draw(scrollbuf,true);
			int32_t ccx = (int32_t)(Hero.getClimbCoverX());
			int32_t ccy = (int32_t)(Hero.getClimbCoverY());
			
			overcombo(scrollbuf,ccx-viewport.x,ccy+cmby2+playing_field_offset-viewport.y,MAPCOMBO(ccx,ccy+cmby2),MAPCSET(ccx,ccy+cmby2));
			putcombo(scrollbuf,ccx-viewport.x,ccy+playing_field_offset-viewport.y,MAPCOMBO(ccx,ccy),MAPCSET(ccx,ccy));
			
			if(int32_t(Hero.getX())&15)
			{
				overcombo(scrollbuf,ccx+16-viewport.x,ccy+cmby2+playing_field_offset-viewport.y,MAPCOMBO(ccx+16,ccy+cmby2),MAPCSET(ccx+16,ccy+cmby2));
				putcombo(scrollbuf,ccx+16-viewport.x,ccy+playing_field_offset-viewport.y,MAPCOMBO(ccx+16,ccy),MAPCSET(ccx+16,ccy));
			}
		}
	}

	if (get_qr(qr_PUSHBLOCK_SPRITE_LAYER))
	{
		for_every_nearby_screen(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy) {
			do_layer(scrollbuf, -2, screen_handles[0], offx, offy); // push blocks!
			if(get_qr(qr_PUSHBLOCK_LAYER_1_2))
			{
				do_layer(scrollbuf, -2, screen_handles[1], offx, offy); // push blocks!
				do_layer(scrollbuf, -2, screen_handles[2], offx, offy); // push blocks!
			}
		});

		do_primitives(scrollbuf, SPLAYER_PUSHBLOCK);
	}

	// Show walkflags cheat
	if (show_walkflags || show_effectflags)
	{
		for_every_nearby_screen(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy) {
			do_walkflags(screen_handles, offx, offy);
			do_effectflags(screen_handles[0].base_scr, offx, offy);
		});

		do_walkflags(0, 0);
	}

	putscrdoors(nearby_screens, scrollbuf, 0, playing_field_offset);
	
	// Lens hints, doors etc.
	if(lensclk || (get_debug() && zc_getkey(KEY_L)))
	{
		if(get_qr(qr_OLDLENSORDER))
		{
			draw_lens_under(scrollbuf, false);
			do_primitives(scrollbuf, SPLAYER_LENS_UNDER_1);
		}
		
		draw_lens_under(scrollbuf, true);
		do_primitives(scrollbuf, SPLAYER_LENS_UNDER_2);
	}
	
	// Blit those layers onto framebuf

	set_draw_screen_clip(framebuf);

	blit(scrollbuf, framebuf, 0, 0, 0, 0, 256, 232);

	// After this point, scrollbuf is no longer drawn to - so things like dosubscr have access to a "partially rendered" frame.
	// I think only used for COOLSCROLL==0? Seems like a silly feature...

	// Draw the subscreen, without clipping
	if(!get_qr(qr_SUBSCREENOVERSPRITES))
	{
		bool dotime = false;
		if (replay_version_check(22)) dotime = game->should_show_time();
		put_passive_subscr(framebuf, 0, 0, dotime, sspUP);
	}
	
	// Draw some sprites onto framebuf
	set_clip_rect(framebuf,0,0,256,232);
	
	if(!(pricesdisplaybuf->clip))
	{
		masked_blit(pricesdisplaybuf,framebuf,0,0,0,playing_field_offset,256,176);
	}

	if (!is_extended_height_mode() && is_in_scrolling_region() && !get_qr(qr_SUBSCREENOVERSPRITES))
		add_clip_rect(framebuf, 0, playing_field_offset, framebuf->w, framebuf->h);

	if(showhero && ((Hero.getAction()!=climbcovertop)&&(Hero.getAction()!=climbcoverbottom)))
	{
		Hero.draw_under(framebuf);
		
		if(Hero.isSwimming())
		{
			decorations.draw2(framebuf,true);
			Hero.draw(framebuf);
			decorations.draw(framebuf,true);
		}
	}
	
	if(drawguys)
	{
		if(get_qr(qr_NOFLICKER) || (frame&1))
		{
			// Just clips sprites if in a repeating, smooth maze.
			bool do_clip = maze_state.active && maze_state.loopy;

			for(int32_t i=0; i<Ewpns.Count(); i++)
			{
				if(((weapon *)Ewpns.spr(i))->behind)
					Ewpns.spr(i)->draw(framebuf);
			}
			do_primitives(framebuf, SPLAYER_EWEAP_BEHIND_DRAW);
			
			for(int32_t i=0; i<Lwpns.Count(); i++)
			{
				if(((weapon *)Lwpns.spr(i))->behind)
					Lwpns.spr(i)->draw(framebuf);
			}
			do_primitives(framebuf, SPLAYER_LWEAP_BEHIND_DRAW);

			if(get_qr(qr_SHADOWS)&&(!get_qr(qr_SHADOWSFLICKER)||frame&1))
			{
				if (do_clip)
					guys.drawshadow_smooth_maze(framebuf,get_qr(qr_TRANSSHADOWS)!=0);
				else
					guys.drawshadow(framebuf,get_qr(qr_TRANSSHADOWS)!=0,true);
			}
			if (do_clip)
				guys.draw_smooth_maze(framebuf);
			else
				guys.draw(framebuf,true);
			if (do_clip)
			{
				int maze_screen = maze_state.scr->screen;
				auto [sx, sy] = translate_screen_coordinates_to_world(maze_screen);
				set_clip_rect(framebuf, sx - viewport.x, sy - viewport.y, sx + 256 - viewport.x, sy + 176 - viewport.y);
			}
			do_primitives(framebuf, SPLAYER_NPC_DRAW);
			if (do_clip)
				clear_clip_rect(framebuf);

			chainlinks.draw(framebuf,true);
			do_primitives(framebuf, SPLAYER_CHAINLINK_DRAW);
			//Lwpns.draw(framebuf,true);
			
			for(int32_t i=0; i<Ewpns.Count(); i++)
			{
				if(!((weapon *)Ewpns.spr(i))->behind)
					Ewpns.spr(i)->draw(framebuf);
			}
			do_primitives(framebuf, SPLAYER_EWEAP_FRONT_DRAW);
			
			for(int32_t i=0; i<Lwpns.Count(); i++)
			{
				if(!((weapon *)Lwpns.spr(i))->behind)
					Lwpns.spr(i)->draw(framebuf);
			}
			do_primitives(framebuf, SPLAYER_LWEAP_FRONT_DRAW);

			if (do_clip)
				items.draw_smooth_maze(framebuf);
			else
				items.draw(framebuf,true);
			if (do_clip)
			{
				int maze_screen = maze_state.scr->screen;
				auto [sx, sy] = translate_screen_coordinates_to_world(maze_screen);
				set_clip_rect(framebuf, sx - viewport.x, sy - viewport.y, sx + 256 - viewport.x, sy + 176 - viewport.y);
			}
			do_primitives(framebuf, SPLAYER_ITEMSPRITE_DRAW);
			if (do_clip)
				clear_clip_rect(framebuf);
		}
		else
		{
			for(int32_t i=0; i<Ewpns.Count(); i++)
			{
				if(((weapon *)Ewpns.spr(i))->behind)
					Ewpns.spr(i)->draw(framebuf);
			}
			do_primitives(framebuf, SPLAYER_EWEAP_BEHIND_DRAW);
		
			for(int32_t i=0; i<Lwpns.Count(); i++)
			{
				if(((weapon *)Lwpns.spr(i))->behind)
					Lwpns.spr(i)->draw(framebuf);
			}
			do_primitives(framebuf, SPLAYER_LWEAP_BEHIND_DRAW);
			
			if(get_qr(qr_SHADOWS)&&(!get_qr(qr_SHADOWSFLICKER)||frame&1))
			{
				guys.drawshadow(framebuf,get_qr(qr_TRANSSHADOWS)!=0,true);
			}
			
			items.draw(framebuf,false);
			do_primitives(framebuf, SPLAYER_ITEMSPRITE_DRAW);
			chainlinks.draw(framebuf,false);
			do_primitives(framebuf, SPLAYER_CHAINLINK_DRAW);
			//Lwpns.draw(framebuf,false);
			guys.draw(framebuf,false);
			do_primitives(framebuf, SPLAYER_NPC_DRAW);
			
			for(int32_t i=0; i<Ewpns.Count(); i++)
			{
				if(!((weapon *)Ewpns.spr(i))->behind)
				{
					Ewpns.spr(i)->draw(framebuf);
				}
			}
			do_primitives(framebuf, SPLAYER_EWEAP_FRONT_DRAW);
		
			for(int32_t i=0; i<Lwpns.Count(); i++)
			{
				if(!((weapon *)Lwpns.spr(i))->behind)
				{
					Lwpns.spr(i)->draw(framebuf);
				}
			}
			do_primitives(framebuf, SPLAYER_LWEAP_FRONT_DRAW);
		}
		
		guys.draw2(framebuf,true);
	}
	
	if(mirror_portal.destdmap > -1)
		mirror_portal.draw(framebuf);
	portals.draw(framebuf,true);
	
	if(showhero && ((Hero.getAction()!=climbcovertop)&& (Hero.getAction()!=climbcoverbottom)))
	{
		if(get_qr(qr_PUSHBLOCK_SPRITE_LAYER))
		{
			mblock2.draw(framebuf,-1);
			do_primitives(framebuf, SPLAYER_MOVINGBLOCK);
		}
		if(!Hero.isSwimming())
		{
			if((Hero.getZ()>0 || Hero.getFakeZ()>0) &&(!get_qr(qr_SHADOWSFLICKER)||frame&1))
			{
				Hero.drawshadow(framebuf,get_qr(qr_TRANSSHADOWS)!=0);
			}
			
			if(Hero.getZ() <= (zfix)zinit.jump_hero_layer_threshold)
			{
				decorations.draw2(framebuf,true);
				Hero.draw(framebuf);
				decorations.draw(framebuf,true);
			}
		}
	}
	
	for(int32_t i=0; i<guys.Count(); i++)
	{
		if(((enemy*)guys.spr(i))->family == eeWALK)
		{
			if(((eStalfos*)guys.spr(i))->hashero)
			{
				guys.spr(i)->draw(framebuf);
			}
		}
		
		if(((enemy*)guys.spr(i))->family == eeWALLM)
		{
			if(((eWallM*)guys.spr(i))->hashero)
			{
				guys.spr(i)->draw(framebuf);
			}
		}
		
		if(guys.spr(i)->z+guys.spr(i)->fakez > Hero.getZ()+Hero.getFakeZ())
		{
			//Jumping enemies in front of Hero.
			guys.spr(i)->draw(framebuf);
		}
		do_primitives(framebuf, SPLAYER_NPC_ABOVEPLAYER_DRAW);
	}
	
	// Draw some layers onto framebuf
	set_draw_screen_clip(framebuf);
	if (!is_extended_height_mode() && is_in_scrolling_region() && !get_qr(qr_SUBSCREENOVERSPRITES))
		add_clip_rect(framebuf, 0, playing_field_offset, framebuf->w, framebuf->h);

	// Handle layer 3 NOT being used as background layers.
	for_every_nearby_screen(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy) {
		mapscr* base_scr = screen_handles[0].base_scr;
		if (!XOR(base_scr->flags7&fLAYER3BG, DMaps[cur_dmap].flags&dmfLAYER3BG))
			do_layer(framebuf, 0, screen_handles[3], offx, offy);
	});

	if(!XOR(origin_scr->flags7&fLAYER3BG, DMaps[cur_dmap].flags&dmfLAYER3BG))
	{
		do_layer_primitives(framebuf, 3);
		particles.draw(framebuf, true, 3);
	}
	_do_current_ffc_layer(framebuf, 3);
	if(!XOR(origin_scr->flags7&fLAYER3BG, DMaps[cur_dmap].flags&dmfLAYER3BG))
		draw_msgstr(3);

	for_every_nearby_screen(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy) {
		do_layer(framebuf, 0, screen_handles[4], offx, offy);
	});

	do_layer_primitives(framebuf, 4);
	particles.draw(framebuf, true, 4);
	_do_current_ffc_layer(framebuf, 4);
	draw_msgstr(4);

	for_every_nearby_screen(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy) {
		do_layer(framebuf, -1, screen_handles[0], offx, offy);
		if (get_qr(qr_OVERHEAD_COMBOS_L1_L2))
		{
			do_layer(framebuf, -1, screen_handles[1], offx, offy);
			do_layer(framebuf, -1, screen_handles[2], offx, offy);
		}
	});

	do_primitives(framebuf, SPLAYER_OVERHEAD_CMB);
	
	// Draw some flying sprites onto framebuf
	clear_clip_rect(framebuf);
	if (!is_extended_height_mode() && is_in_scrolling_region() && !get_qr(qr_SUBSCREENOVERSPRITES))
		add_clip_rect(framebuf, 0, playing_field_offset, framebuf->w, framebuf->h);

	//Jumping Hero and jumping enemies are drawn on this layer.
	if(Hero.getZ() > (zfix)zinit.jump_hero_layer_threshold)
	{
		decorations.draw2(framebuf,false);
		Hero.draw(framebuf);
		chainlinks.draw(framebuf,true);
		do_primitives(framebuf, SPLAYER_CHAINLINK_DRAW);
		
		for(int32_t i=0; i<Lwpns.Count(); i++)
		{
			if(Lwpns.spr(i)->z+Lwpns.spr(i)->fakez > (zfix)zinit.jump_hero_layer_threshold)
			{
				Lwpns.spr(i)->draw(framebuf);
			}
		}
		do_primitives(framebuf, SPLAYER_LWEAP_ABOVE_DRAW);
		
		decorations.draw(framebuf,false);
	}
	
	if(!get_qr(qr_ENEMIESZAXIS)) for(int32_t i=0; i<guys.Count(); i++)
		{
			if((isflier(guys.spr(i)->id)) || (guys.spr(i)->z+guys.spr(i)->fakez) > (zfix)zinit.jump_hero_layer_threshold)
			{
				guys.spr(i)->draw(framebuf);
			}
		}
	else
	{
		for(int32_t i=0; i<guys.Count(); i++)
		{
			if((isflier(guys.spr(i)->id)) || guys.spr(i)->z > 0 || guys.spr(i)->fakez > 0)
			{
				guys.spr(i)->draw(framebuf);
			}
		}
	}
	do_primitives(framebuf, SPLAYER_NPC_AIRBORNE_DRAW);
	
	// Draw the Moving Fairy above layer 3
	for(int32_t i=0; i<items.Count(); i++)
		if(itemsbuf[items.spr(i)->id].family == itype_fairy && itemsbuf[items.spr(i)->id].misc3)
			items.spr(i)->draw(framebuf);
	do_primitives(framebuf, SPLAYER_FAIRYITEM_DRAW);
	
	// Draw some layers onto framebuf

	set_draw_screen_clip(framebuf);

	if (lightbeam_present)
	{
		color_map = &trans_table2;
		if(get_qr(qr_LIGHTBEAM_TRANSPARENT))
			draw_trans_sprite(framebuf, lightbeam_bmp, 0, playing_field_offset);
		else 
			masked_blit(lightbeam_bmp, framebuf, 0, 0, 0, playing_field_offset, 256, 176);
		color_map = &trans_table;
	}

	for_every_nearby_screen(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy) {
		do_layer(framebuf, 0, screen_handles[5], offx, offy);
	});

	do_layer_primitives(framebuf, 5);
	particles.draw(framebuf, true, 5);
	_do_current_ffc_layer(framebuf, 5);
	draw_msgstr(5);
	
	_do_current_ffc_layer(framebuf, -1); // 'overhead' freeform combos

	do_primitives(framebuf, SPLAYER_OVERHEAD_FFC);

	for_every_nearby_screen(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy) {
		do_layer(framebuf, 0, screen_handles[6], offx, offy);
	});

	do_layer_primitives(framebuf, 6);
	particles.draw(framebuf, true, 6);
	_do_current_ffc_layer(framebuf, 6);

	bool any_dark = false;
	for_every_nearby_screen(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy) {
		mapscr* base_scr = screen_handles[0].scr;
		any_dark |= is_dark(base_scr);
	});

	// Handle low drawn darkness
	if(get_qr(qr_NEW_DARKROOM) && any_dark)
	{
		for_every_nearby_screen(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy) {
			mapscr* base_scr = screen_handles[0].scr;
			calc_darkroom_combos(base_scr, offx, offy + playing_field_offset);
			calc_darkroom_ffcs(base_scr, 0, playing_field_offset);
		});
		if(showhero)
			Hero.calc_darkroom_hero(0, -playing_field_offset);
		for_every_nearby_screen(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy) {
			mapscr* base_scr = screen_handles[0].scr;
			if (!is_dark(base_scr))
			{
				offy += playing_field_offset;
				rectfill(darkscr_bmp, offx - viewport.x, offy - viewport.y, offx - viewport.x + 256 - 1, offy - viewport.y + 176 - 1, 0);
				rectfill(darkscr_bmp_trans, offx - viewport.x, offy - viewport.y, offx - viewport.x + 256 - 1, offy - viewport.y + 176 - 1, 0);
			}
		});
	}
	
	//Darkroom if under the subscreen
	if(get_qr(qr_NEW_DARKROOM) && get_qr(qr_NEWDARK_L6) && any_dark)
	{
		do_primitives(framebuf, SPLAYER_DARKROOM_UNDER);
		set_clip_rect(framebuf, 0, playing_field_offset, framebuf->w, framebuf->h);
		if(hero_scr->flags9 & fDARK_DITHER) //dither the entire bitmap
		{
			ditherblit(darkscr_bmp,darkscr_bmp,0,game->get_dither_type(),game->get_dither_arg());
			ditherblit(darkscr_bmp_trans,darkscr_bmp_trans,0,game->get_dither_type(),game->get_dither_arg());
		}
		
		color_map = &trans_table2;
		if(hero_scr->flags9 & fDARK_TRANS) //draw the dark as transparent
		{
			draw_trans_sprite(framebuf, darkscr_bmp, 0, 0);
			if(get_qr(qr_NEWDARK_TRANS_STACKING))
				draw_trans_sprite(framebuf, darkscr_bmp_trans, 0, 0);
		}
		else
		{
			masked_blit(darkscr_bmp, framebuf, 0, 0, 0, 0, framebuf->w, framebuf->h);
			draw_trans_sprite(framebuf, darkscr_bmp_trans, 0, 0);
		}
		color_map = &trans_table;
		
		set_clip_rect(framebuf, 0, 0, framebuf->w, framebuf->h);
		do_primitives(framebuf, SPLAYER_DARKROOM_OVER);
	}

	if (is_extended_height_mode() && lensclk && !FFCore.system_suspend[susptLENS])
	{
		draw_lens_over();
		--lensclk;
	}

	// Draw some text on framebuf
	
	set_clip_rect(framebuf,0,0,256,232);
	if(!get_qr(qr_LAYER6_STRINGS_OVER_SUBSCREEN))
		draw_msgstr(6);
	
	// Draw the subscreen, without clipping
	if(get_qr(qr_SUBSCREENOVERSPRITES))
	{
		put_passive_subscr(framebuf, 0, 0, game->should_show_time(), sspUP);
		
		// Draw primitives over subscren
		do_primitives(framebuf, 7); //Layer '7' appears above subscreen if quest rule is set
	}
	
	if(get_qr(qr_LAYER6_STRINGS_OVER_SUBSCREEN))
		draw_msgstr(6);
	
	// Handle high-drawn darkness
	if(get_qr(qr_NEW_DARKROOM) && !get_qr(qr_NEWDARK_L6) && any_dark)
	{
		do_primitives(framebuf, SPLAYER_DARKROOM_UNDER);
		set_clip_rect(framebuf, 0, playing_field_offset, framebuf->w, framebuf->h);
		if(hero_scr->flags9 & fDARK_DITHER) //dither the entire bitmap
		{
			ditherblit(darkscr_bmp,darkscr_bmp,0,game->get_dither_type(),game->get_dither_arg());
			ditherblit(darkscr_bmp_trans,darkscr_bmp_trans,0,game->get_dither_type(),game->get_dither_arg());
		}
		
		color_map = &trans_table2;
		if(hero_scr->flags9 & fDARK_TRANS) //draw the dark as transparent
		{
			draw_trans_sprite(framebuf, darkscr_bmp, 0, 0);
			if(get_qr(qr_NEWDARK_TRANS_STACKING))
				draw_trans_sprite(framebuf, darkscr_bmp_trans, 0, 0);
		}
		else
		{
			masked_blit(darkscr_bmp, framebuf, 0, 0, 0, 0, framebuf->w, framebuf->h);
			draw_trans_sprite(framebuf, darkscr_bmp_trans, 0, 0);
		}
		color_map = &trans_table;
		
		set_clip_rect(framebuf, 0, 0, framebuf->w, framebuf->h);
		do_primitives(framebuf, SPLAYER_DARKROOM_OVER);
	}
	
	_do_current_ffc_layer(framebuf, 7);
	draw_msgstr(7);
	
	set_clip_rect(scrollbuf, 0, 0, scrollbuf->w, scrollbuf->h);
	if(runGeneric) FFCore.runGenericPassiveEngine(SCR_TIMING_POST_DRAW);
}

// TODO: separate setting door data and drawing door
void put_door(mapscr* scr, BITMAP *dest, int32_t pos, int32_t side, int32_t type, bool redraw, bool even_walls)
{
	if (type > 8) return;

	int32_t d=scr->door_combo_set;

	switch(type)
	{
	case dt_wall:
	case dt_walk:
		if(!even_walls)
			break;
		[[fallthrough]];
	case dt_pass:
		if(!get_qr(qr_REPLACEOPENDOORS) && !even_walls)
			break;
		[[fallthrough]];
	case dt_lock:
	case dt_shut:
	case dt_boss:
	case dt_olck:
	case dt_osht:
	case dt_obos:
	case dt_bomb:
		switch(side)
		{
		case up:
			scr->data[pos]   = DoorComboSets[d].doorcombo_u[type][0];
			scr->cset[pos]   = DoorComboSets[d].doorcset_u[type][0];
			scr->sflag[pos]  = 0;
			scr->data[pos+1]   = DoorComboSets[d].doorcombo_u[type][1];
			scr->cset[pos+1]   = DoorComboSets[d].doorcset_u[type][1];
			scr->sflag[pos+1]  = 0;
			scr->data[pos+16]   = DoorComboSets[d].doorcombo_u[type][2];
			scr->cset[pos+16]   = DoorComboSets[d].doorcset_u[type][2];
			scr->sflag[pos+16]  = 0;
			scr->data[pos+16+1]   = DoorComboSets[d].doorcombo_u[type][3];
			scr->cset[pos+16+1]   = DoorComboSets[d].doorcset_u[type][3];
			scr->sflag[pos+16+1]  = 0;
			
			if(redraw)
			{
				putcombo(dest,(pos&15)<<4,pos&0xF0,
						 DoorComboSets[d].doorcombo_u[type][0],
						 DoorComboSets[d].doorcset_u[type][0]);
				putcombo(dest,((pos&15)<<4)+16,pos&0xF0,
						 DoorComboSets[d].doorcombo_u[type][1],
						 DoorComboSets[d].doorcset_u[type][1]);
			}
			
			break;
			
		case down:
			scr->data[pos]   = DoorComboSets[d].doorcombo_d[type][0];
			scr->cset[pos]   = DoorComboSets[d].doorcset_d[type][0];
			scr->sflag[pos]  = 0;
			scr->data[pos+1]   = DoorComboSets[d].doorcombo_d[type][1];
			scr->cset[pos+1]   = DoorComboSets[d].doorcset_d[type][1];
			scr->sflag[pos+1]  = 0;
			scr->data[pos+16]   = DoorComboSets[d].doorcombo_d[type][2];
			scr->cset[pos+16]   = DoorComboSets[d].doorcset_d[type][2];
			scr->sflag[pos+16]  = 0;
			scr->data[pos+16+1]   = DoorComboSets[d].doorcombo_d[type][3];
			scr->cset[pos+16+1]   = DoorComboSets[d].doorcset_d[type][3];
			scr->sflag[pos+16+1]  = 0;
			
			if(redraw)
			{
				putcombo(dest,(pos&15)<<4,(pos&0xF0)+16,
						 DoorComboSets[d].doorcombo_d[type][2],
						 DoorComboSets[d].doorcset_d[type][2]);
				putcombo(dest,((pos&15)<<4)+16,(pos&0xF0)+16,
						 DoorComboSets[d].doorcombo_d[type][3],
						 DoorComboSets[d].doorcset_d[type][3]);
			}
			
			break;
			
		case left:
			scr->data[pos]   = DoorComboSets[d].doorcombo_l[type][0];
			scr->cset[pos]   = DoorComboSets[d].doorcset_l[type][0];
			scr->sflag[pos]  = 0;
			scr->data[pos+1]   = DoorComboSets[d].doorcombo_l[type][1];
			scr->cset[pos+1]   = DoorComboSets[d].doorcset_l[type][1];
			scr->sflag[pos+1]  = 0;
			scr->data[pos+16]   = DoorComboSets[d].doorcombo_l[type][2];
			scr->cset[pos+16]   = DoorComboSets[d].doorcset_l[type][2];
			scr->sflag[pos+16]  = 0;
			scr->data[pos+16+1]   = DoorComboSets[d].doorcombo_l[type][3];
			scr->cset[pos+16+1]   = DoorComboSets[d].doorcset_l[type][3];
			scr->sflag[pos+16+1]  = 0;
			scr->data[pos+32]   = DoorComboSets[d].doorcombo_l[type][4];
			scr->cset[pos+32]   = DoorComboSets[d].doorcset_l[type][4];
			scr->sflag[pos+32]  = 0;
			scr->data[pos+32+1]   = DoorComboSets[d].doorcombo_l[type][5];
			scr->cset[pos+32+1]   = DoorComboSets[d].doorcset_l[type][5];
			scr->sflag[pos+32+1]  = 0;
			
			if(redraw)
			{
				putcombo(dest,(pos&15)<<4,pos&0xF0,
						 DoorComboSets[d].doorcombo_l[type][0],
						 DoorComboSets[d].doorcset_l[type][0]);
				putcombo(dest,(pos&15)<<4,(pos&0xF0)+16,
						 DoorComboSets[d].doorcombo_l[type][2],
						 DoorComboSets[d].doorcset_l[type][2]);
				putcombo(dest,(pos&15)<<4,(pos&0xF0)+32,
						 DoorComboSets[d].doorcombo_l[type][4],
						 DoorComboSets[d].doorcset_l[type][4]);
			}
			
			break;
			
		case right:
			scr->data[pos]   = DoorComboSets[d].doorcombo_r[type][0];
			scr->cset[pos]   = DoorComboSets[d].doorcset_r[type][0];
			scr->sflag[pos]  = 0;
			scr->data[pos+1]   = DoorComboSets[d].doorcombo_r[type][1];
			scr->cset[pos+1]   = DoorComboSets[d].doorcset_r[type][1];
			scr->sflag[pos+1]  = 0;
			scr->data[pos+16]   = DoorComboSets[d].doorcombo_r[type][2];
			scr->cset[pos+16]   = DoorComboSets[d].doorcset_r[type][2];
			scr->sflag[pos+16]  = 0;
			scr->data[pos+16+1]   = DoorComboSets[d].doorcombo_r[type][3];
			scr->cset[pos+16+1]   = DoorComboSets[d].doorcset_r[type][3];
			scr->sflag[pos+16+1]  = 0;
			scr->data[pos+32]   = DoorComboSets[d].doorcombo_r[type][4];
			scr->cset[pos+32]   = DoorComboSets[d].doorcset_r[type][4];
			scr->sflag[pos+32]  = 0;
			scr->data[pos+32+1]   = DoorComboSets[d].doorcombo_r[type][5];
			scr->cset[pos+32+1]   = DoorComboSets[d].doorcset_r[type][5];
			scr->sflag[pos+32+1]  = 0;
			
			if(redraw)
			{
				putcombo(dest,(pos&15)<<4,pos&0xF0,
						 DoorComboSets[d].doorcombo_r[type][0],
						 DoorComboSets[d].doorcset_r[type][0]);
				putcombo(dest,(pos&15)<<4,(pos&0xF0)+16,
						 DoorComboSets[d].doorcombo_r[type][2],
						 DoorComboSets[d].doorcset_r[type][2]);
				putcombo(dest,(pos&15)<<4,(pos&0xF0)+32,
						 DoorComboSets[d].doorcombo_r[type][4],
						 DoorComboSets[d].doorcset_r[type][4]);
			}
			
			break;
		}
		
		break;
		
	default:
		break;
	}
}

static void over_door(mapscr* scr, BITMAP *dest, int32_t pos, int32_t side, int32_t offx, int32_t offy)
{
	int32_t door_combo_set = scr->door_combo_set;
	int32_t x = (pos&15)<<4;
	int32_t y = (pos&0xF0);
	int32_t d = door_combo_set;
	x += offx;
	y += offy;
	
	switch(side)
	{
	case up:
		overcombo2(dest,x,y,
				   DoorComboSets[d].bombdoorcombo_u[0],
				   DoorComboSets[d].bombdoorcset_u[0]);
		overcombo2(dest,x+16,y,
				   DoorComboSets[d].bombdoorcombo_u[1],
				   DoorComboSets[d].bombdoorcset_u[1]);
		break;
		
	case down:
		overcombo2(dest,x,y,
				   DoorComboSets[d].bombdoorcombo_d[0],
				   DoorComboSets[d].bombdoorcset_d[0]);
		overcombo2(dest,x+16,y,
				   DoorComboSets[d].bombdoorcombo_d[1],
				   DoorComboSets[d].bombdoorcset_d[1]);
		break;
		
	case left:
		overcombo2(dest,x,y,
				   DoorComboSets[d].bombdoorcombo_l[0],
				   DoorComboSets[d].bombdoorcset_l[0]);
		overcombo2(dest,x,y+16,
				   DoorComboSets[d].bombdoorcombo_l[1],
				   DoorComboSets[d].bombdoorcset_l[1]);
		overcombo2(dest,x,y+16,
				   DoorComboSets[d].bombdoorcombo_l[2],
				   DoorComboSets[d].bombdoorcset_l[2]);
		break;
		
	case right:
		overcombo2(dest,x,y,
				   DoorComboSets[d].bombdoorcombo_r[0],
				   DoorComboSets[d].bombdoorcset_r[0]);
		overcombo2(dest,x,y+16,
				   DoorComboSets[d].bombdoorcombo_r[1],
				   DoorComboSets[d].bombdoorcset_r[1]);
		overcombo2(dest,x,y+16,
				   DoorComboSets[d].bombdoorcombo_r[2],
				   DoorComboSets[d].bombdoorcset_r[2]);
		break;
	}
}

void update_door(mapscr* scr, int32_t side, int32_t door, bool even_walls)
{
	if(door == dNONE || (!even_walls&&(door==dWALL||door==dWALK)))
		return;
	
	int32_t doortype;
	
	switch(door)
	{
	case dWALL:
		doortype=dt_wall;
		break;
		
	case dWALK:
		doortype=dt_walk;
		break;
		
	case dOPEN:
		doortype=dt_pass;
		break;
		
	case dLOCKED:
		doortype=dt_lock;
		break;
		
	case dUNLOCKED:
		doortype=dt_olck;
		break;
		
	case dSHUTTER:
		if(screenscrolling && ((HeroDir()^1)==side))
		{
			doortype=dt_osht;
			get_screen_state(scr->screen).open_doors = -4;
			break;
		}

		[[fallthrough]];
	case d1WAYSHUTTER:
		doortype=dt_shut;
		break;
		
	case dOPENSHUTTER:
		doortype=dt_osht;
		break;
		
	case dBOSS:
		doortype=dt_boss;
		break;
		
	case dOPENBOSS:
		doortype=dt_obos;
		break;
		
	case dBOMBED:
		doortype=dt_bomb;
		break;
		
	default:
		return;
	}
	
	switch(side)
	{
	case up:
		put_door(scr,nullptr,7,side,doortype,false,even_walls);
		break;
		
	case down:
		put_door(scr,nullptr,151,side,doortype,false,even_walls);
		break;
		
	case left:
		put_door(scr,nullptr,64,side,doortype,false,even_walls);
		break;
		
	case right:
		put_door(scr,nullptr,78,side,doortype,false,even_walls);
		break;
	}
}

void putdoor(mapscr* scr, BITMAP *dest, int32_t side, int32_t door, bool redraw, bool even_walls)
{
	/*
	  #define dWALL		   0  //  000	0
	  #define dBOMB		   6  //  011	0
	  #define			  8  //  100	0
	  enum {dt_pass=0, dt_lock, dt_shut, dt_boss, dt_olck, dt_osht, dt_obos, dt_wall, dt_bomb, dt_walk, dt_max};
	  */
	
	if(door == dNONE || (!even_walls&&(door==dWALL||door==dWALK)))
		return;
	
	int32_t doortype;
	
	switch(door)
	{
	case dWALL:
		doortype=dt_wall;
		break;
		
	case dWALK:
		doortype=dt_walk;
		break;
		
	case dOPEN:
		doortype=dt_pass;
		break;
		
	case dLOCKED:
		doortype=dt_lock;
		break;
		
	case dUNLOCKED:
		doortype=dt_olck;
		break;
		
	case dSHUTTER:
		if(screenscrolling && ((HeroDir()^1)==side))
		{
			doortype=dt_osht;
			get_screen_state(cur_screen).open_doors = -4;
			break;
		}

		[[fallthrough]];
	case d1WAYSHUTTER:
		doortype=dt_shut;
		break;
		
	case dOPENSHUTTER:
		doortype=dt_osht;
		break;
		
	case dBOSS:
		doortype=dt_boss;
		break;
		
	case dOPENBOSS:
		doortype=dt_obos;
		break;
		
	case dBOMBED:
		doortype=dt_bomb;
		break;
		
	default:
		return;
	}
	
	switch(side)
	{
	case up:
		switch(door)
		{
		case dBOMBED:
			if(redraw)
			{
				over_door(scr,dest,39,side,0,0);
			}
			[[fallthrough]];
		default:
			put_door(scr,dest,7,side,doortype,redraw, even_walls);
			break;
		}
		
		break;
		
	case down:
		switch(door)
		{
		case dBOMBED:
			if(redraw)
			{
				over_door(scr,dest,135,side,0,0);
			}
			[[fallthrough]];
		default:
			put_door(scr,dest,151,side,doortype,redraw, even_walls);
			break;
		}
		
		break;
		
	case left:
		switch(door)
		{
		case dBOMBED:
			if(redraw)
			{
				over_door(scr,dest,66,side,0,0);
			}
			[[fallthrough]];
		default:
			put_door(scr,dest,64,side,doortype,redraw, even_walls);
			break;
		}
		
		break;
		
	case right:
		switch(door)
		{
		case dBOMBED:
			if(redraw)
			{
				over_door(scr,dest,77,side,0,0);
			}
			[[fallthrough]];
		default:
			put_door(scr,dest,78,side,doortype,redraw, even_walls);
			break;
		}
		
		break;
	}
}

void putcombo_not_zero(BITMAP *dest, int32_t x, int32_t y, int32_t combo, int32_t cset)
{
	if(combo!=0)
	{
		putcombo(dest,x, y, combo, cset);
	}
}

void overcombo_not_zero(BITMAP *dest, int32_t x, int32_t y, int32_t combo, int32_t cset)
{
    if(combo!=0)
    {
        overcombo(dest,x, y, combo, cset);
    }
}

void showbombeddoor(mapscr* scr, BITMAP *dest, int32_t side, int32_t offx, int32_t offy)
{
    int32_t d = scr->door_combo_set;
    
    switch(side)
    {
    case up:
        putcombo_not_zero(dest,((7&15)<<4) + offx,(7&0xF0) + offy,
                          DoorComboSets[d].doorcombo_u[dt_bomb][0],
                          DoorComboSets[d].doorcset_u[dt_bomb][0]);
        putcombo_not_zero(dest,((8&15)<<4) + offx,(8&0xF0) + offy,
                          DoorComboSets[d].doorcombo_u[dt_bomb][1],
                          DoorComboSets[d].doorcset_u[dt_bomb][1]);
        putcombo_not_zero(dest,((23&15)<<4) + offx,(23&0xF0) + offy,
                          DoorComboSets[d].doorcombo_u[dt_bomb][2],
                          DoorComboSets[d].doorcset_u[dt_bomb][2]);
        putcombo_not_zero(dest,((24&15)<<4) + offx,(24&0xF0) + offy,
                          DoorComboSets[d].doorcombo_u[dt_bomb][3],
                          DoorComboSets[d].doorcset_u[dt_bomb][3]);
        overcombo_not_zero(dest,((39&15)<<4) + offx,(39&0xF0) + offy,
                           DoorComboSets[d].bombdoorcombo_u[0],
                           DoorComboSets[d].bombdoorcset_u[0]);
        overcombo_not_zero(dest,((40&15)<<4) + offx,(40&0xF0) + offy,
                           DoorComboSets[d].bombdoorcombo_u[1],
                           DoorComboSets[d].bombdoorcset_u[1]);
        break;
        
    case down:
        putcombo_not_zero(dest,((151&15)<<4) + offx,(151&0xF0) + offy,
                          DoorComboSets[d].doorcombo_d[dt_bomb][0],
                          DoorComboSets[d].doorcset_d[dt_bomb][0]);
        putcombo_not_zero(dest,((152&15)<<4) + offx,(152&0xF0) + offy,
                          DoorComboSets[d].doorcombo_d[dt_bomb][1],
                          DoorComboSets[d].doorcset_d[dt_bomb][1]);
        putcombo_not_zero(dest,((167&15)<<4) + offx,(167&0xF0) + offy,
                          DoorComboSets[d].doorcombo_d[dt_bomb][2],
                          DoorComboSets[d].doorcset_d[dt_bomb][2]);
        putcombo_not_zero(dest,((168&15)<<4) + offx,(168&0xF0) + offy,
                          DoorComboSets[d].doorcombo_d[dt_bomb][3],
                          DoorComboSets[d].doorcset_d[dt_bomb][3]);
        overcombo_not_zero(dest,((135&15)<<4) + offx,(135&0xF0) + offy,
                           DoorComboSets[d].bombdoorcombo_d[0],
                           DoorComboSets[d].bombdoorcset_d[0]);
        overcombo_not_zero(dest,((136&15)<<4) + offx,(136&0xF0) + offy,
                           DoorComboSets[d].bombdoorcombo_d[1],
                           DoorComboSets[d].bombdoorcset_d[1]);
        break;
        
    case left:
        putcombo_not_zero(dest,((64&15)<<4) + offx,(64&0xF0) + offy,
                          DoorComboSets[d].doorcombo_l[dt_bomb][0],
                          DoorComboSets[d].doorcset_l[dt_bomb][0]);
        putcombo_not_zero(dest,((65&15)<<4) + offx,(65&0xF0) + offy,
                          DoorComboSets[d].doorcombo_l[dt_bomb][1],
                          DoorComboSets[d].doorcset_l[dt_bomb][1]);
        putcombo_not_zero(dest,((80&15)<<4) + offx,(80&0xF0) + offy,
                          DoorComboSets[d].doorcombo_l[dt_bomb][2],
                          DoorComboSets[d].doorcset_l[dt_bomb][2]);
        putcombo_not_zero(dest,((81&15)<<4) + offx,(81&0xF0) + offy,
                          DoorComboSets[d].doorcombo_l[dt_bomb][3],
                          DoorComboSets[d].doorcset_l[dt_bomb][3]);
        putcombo_not_zero(dest,((96&15)<<4) + offx,(96&0xF0) + offy,
                          DoorComboSets[d].doorcombo_l[dt_bomb][4],
                          DoorComboSets[d].doorcset_l[dt_bomb][4]);
        putcombo_not_zero(dest,((97&15)<<4) + offx,(97&0xF0) + offy,
                          DoorComboSets[d].doorcombo_l[dt_bomb][5],
                          DoorComboSets[d].doorcset_l[dt_bomb][5]);
        overcombo_not_zero(dest,((66&15)<<4) + offx,(66&0xF0) + offy,
                           DoorComboSets[d].bombdoorcombo_l[0],
                           DoorComboSets[d].bombdoorcset_l[0]);
        overcombo_not_zero(dest,((82&15)<<4) + offx,(82&0xF0) + offy,
                           DoorComboSets[d].bombdoorcombo_l[1],
                           DoorComboSets[d].bombdoorcset_l[1]);
        overcombo_not_zero(dest,((98&15)<<4) + offx,(98&0xF0) + offy,
                           DoorComboSets[d].bombdoorcombo_l[2],
                           DoorComboSets[d].bombdoorcset_l[2]);
        break;
        
    case right:
        putcombo_not_zero(dest,((78&15)<<4) + offx,(78&0xF0) + offy,
                          DoorComboSets[d].doorcombo_r[dt_bomb][0],
                          DoorComboSets[d].doorcset_r[dt_bomb][0]);
        putcombo_not_zero(dest,((79&15)<<4) + offx,(79&0xF0) + offy,
                          DoorComboSets[d].doorcombo_r[dt_bomb][1],
                          DoorComboSets[d].doorcset_r[dt_bomb][1]);
        putcombo_not_zero(dest,((94&15)<<4) + offx,(94&0xF0) + offy,
                          DoorComboSets[d].doorcombo_r[dt_bomb][2],
                          DoorComboSets[d].doorcset_r[dt_bomb][2]);
        putcombo_not_zero(dest,((95&15)<<4) + offx,(95&0xF0) + offy,
                          DoorComboSets[d].doorcombo_r[dt_bomb][3],
                          DoorComboSets[d].doorcset_r[dt_bomb][3]);
        putcombo_not_zero(dest,((110&15)<<4) + offx,(110&0xF0) + offy,
                          DoorComboSets[d].doorcombo_r[dt_bomb][4],
                          DoorComboSets[d].doorcset_r[dt_bomb][4]);
        putcombo_not_zero(dest,((111&15)<<4) + offx,(111&0xF0) + offy,
                          DoorComboSets[d].doorcombo_r[dt_bomb][5],
                          DoorComboSets[d].doorcset_r[dt_bomb][5]);
        overcombo_not_zero(dest,((77&15)<<4) + offx,(77&0xF0) + offy,
                           DoorComboSets[d].bombdoorcombo_r[0],
                           DoorComboSets[d].bombdoorcset_r[0]);
        overcombo_not_zero(dest,((93&15)<<4) + offx,(93&0xF0) + offy,
                           DoorComboSets[d].bombdoorcombo_r[1],
                           DoorComboSets[d].bombdoorcset_r[1]);
        overcombo_not_zero(dest,((109&15)<<4) + offx,(109&0xF0) + offy,
                           DoorComboSets[d].bombdoorcombo_r[2],
                           DoorComboSets[d].bombdoorcset_r[2]);
        break;
    }
}

void openshutters(mapscr* scr)
{
	bool opened_door = false;
	for(int32_t i=0; i<4; i++)
		if(scr->door[i]==dSHUTTER)
		{
			putdoor(scr, scrollbuf, i, dOPENSHUTTER);
			scr->door[i]=dOPENSHUTTER;
			opened_door = true;
		}
	
	auto& combo_cache = combo_caches::shutter;
	for_every_combo_in_screen(create_screen_handles(scr), [&](const auto& handle) {
		if (!combo_cache.minis[handle.data()].shutter)
			return;
		auto cid = handle.data();
		auto& cmb = handle.combo();
		for(size_t idx = 0; idx < cmb.triggers.size(); ++idx)
		{
			auto& trig = cmb.triggers[idx];
			if(trig.triggerflags[0] & combotriggerSHUTTER)
			{
				do_trigger_combo(handle, idx);
				if(handle.data() != cid) break;
			}
		}
	});

	if(opened_door)
		sfx(WAV_DOOR,128);
}

void clear_darkroom_bitmaps()
{
	clear_to_color(darkscr_bmp, game->get_darkscr_color());
	clear_to_color(darkscr_bmp_trans, game->get_darkscr_color());
}

bool is_dark(const mapscr* scr)
{
	bool dark = scr->flags&fDARK;
	if (region_is_lit) return !dark;
	return dark;
}

bool scrolling_is_dark(const mapscr* scr)
{
	bool dark = scr->flags&fDARK;
	if (scrolling_region_is_lit) return !dark;
	return dark;
}

bool is_any_dark()
{
	bool dark = false;
	for_every_base_screen_in_region([&](mapscr* scr, unsigned int region_scr_x, unsigned int region_scr_y) {
		dark |= (bool)(is_dark(scr));
	});
	return dark;
}

static mapscr prev_origin_scrs[7];
static std::set<int> loadscr_ffc_script_ids_to_remove;

static void handle_screen_overlay(const std::vector<mapscr*>& screens)
{
	mapscr* base_scr = screens[0];
	mapscr* previous_scr = &prev_origin_scrs[0];

	for (int i = 0; i < 176; i++)
	{
		if (base_scr->data[i] == 0)
		{
			base_scr->data[i] = previous_scr->data[i];
			base_scr->sflag[i] = previous_scr->sflag[i];
			base_scr->cset[i] = previous_scr->cset[i];
		}
	}
	
	for (int i = 0; i < 6; i++)
	{
		if (previous_scr->layermap[i] > 0 && base_scr->layermap[i] > 0)
		{
			int lm = (base_scr->layermap[i]-1)*MAPSCRS+base_scr->layerscreen[i];
			int fm = (previous_scr->layermap[i]-1)*MAPSCRS+previous_scr->layerscreen[i];

			for (int j = 0; j < 176; j++)
			{
				if (TheMaps[lm].data[j] == 0)
				{
					TheMaps[lm].data[j] = TheMaps[fm].data[j];
					TheMaps[lm].sflag[j] = TheMaps[fm].sflag[j];
					TheMaps[lm].cset[j] = TheMaps[fm].cset[j];
				}
			}
		}
	}

	for (int i = 1; i <= 6; i++)
	{
		mapscr* scr = screens[i];
		previous_scr = &prev_origin_scrs[i];

		if (scr->layermap[i] > 0)
		{
			for (int y = 0; y < 11; y++)
			{
				for (int x = 0; x < 16; x++)
				{
					int c = y*16+x;

					if (scr->data[c]==0)
					{
						scr->data[c] = previous_scr->data[c];
						scr->sflag[c] = previous_scr->sflag[c];
						scr->cset[c] = previous_scr->cset[c];
					}
				}
			}
		}
	}
}

static void load_a_screen_and_layers_init(int dmap, int screen, int ldir, bool screen_overlay, bool ffc_overlay)
{
	std::vector<mapscr*> screens;

	const mapscr* source = get_canonical_scr(cur_map, screen);
	mapscr* base_scr = new mapscr(*source);
	temporary_screens[screen*7] = base_scr;
	screens.push_back(base_scr);

	base_scr->valid |= mVALID; // layer 0 is always valid

	if (screen == cur_screen)
		origin_scr = base_scr;
	if (screen == hero_screen)
		hero_scr = prev_hero_scr = base_scr;

	if (source->script > 0)
		FFCore.reset_script_engine_data(ScriptType::Screen, screen);

	for (int i = 0; i < 6; i++)
	{
		if(source->layermap[i]>0)
		{
			mapscr* layer_scr = new mapscr(*get_canonical_scr(source->layermap[i]-1, source->layerscreen[i]));
			layer_scr->map = cur_map;
			layer_scr->screen = screen;
			screens.push_back(layer_scr);
		}
		else
		{
			mapscr* layer_scr = new mapscr();
			layer_scr->map = cur_map;
			layer_scr->screen = screen;
			screens.push_back(layer_scr);
		}
		temporary_screens[screen*7+i+1] = screens[i+1];
	}

	if (screen_overlay)
		handle_screen_overlay(screens);

	if (ffc_overlay)
	{
		mapscr* previous_scr = &prev_origin_scrs[0];
		int num_ffcs = previous_scr->numFFC();
		for (int i = 0; i < num_ffcs; i++)
		{
			if ((previous_scr->ffcs[i].flags&ffc_carryover))
			{
				auto& ffc = base_scr->getFFC(i) = previous_scr->ffcs[i];
				ffc.screen_spawned = screen;

				ffc_id_t ffc_id = get_region_screen_offset(screen)*MAXFFCS + i;
				loadscr_ffc_script_ids_to_remove.erase(ffc_id);
				if (previous_scr->ffcs[i].flags&ffc_scriptreset)
				{
					FFCore.reset_script_engine_data(ScriptType::FFC, ffc_id);
				}
			}
		}
	}

	auto [offx, offy] = translate_screen_coordinates_to_world(screen);
	int num_ffcs = base_scr->numFFC();
	for (word i = 0; i < num_ffcs; i++)
	{
		base_scr->ffcs[i].screen_spawned = screen;
		base_scr->ffcs[i].x += offx;
		base_scr->ffcs[i].y += offy;
	}
}

static void load_a_screen_and_layers_post(int dmap, int screen, int ldir)
{
	mapscr* base_scr = get_scr(screen);
	int mi = mapind(cur_map, screen);

	// Apply perm secrets, if applicable.
	if (canPermSecret(dmap, screen))
	{
		if(game->maps[mi] & mSECRET)    // if special stuff done before
		{
			reveal_hidden_stairs(base_scr, screen, false);
			trigger_secrets_for_screen(TriggerSource::SecretsScreenState, base_scr, false);
		}
		if(game->maps[mi] & mLIGHTBEAM) // if special stuff done before
		{
			for (int layer = 0; layer <= 6; layer++)
			{
				mapscr* layer_scr = get_scr_layer(screen, layer);
				for (int pos = 0; pos < 176; pos++)
				{
					newcombo const* cmb = &combobuf[layer_scr->data[pos]];
					if(cmb->type == cLIGHTTARGET)
					{
						if (!(cmb->usrflags&cflag1)) //Unlit version
						{
							layer_scr->data[pos] += 1;
						}
					}
				}
			}
		}
	}

	auto screen_handles = create_screen_handles(base_scr);

	int destlvl = DMaps[dmap].level;
	toggle_switches(game->lvlswitches[destlvl], true, screen_handles);
	toggle_gswitches_load(screen_handles);

	bool should_check_for_state_things = (screen < 0x80);
	if (should_check_for_state_things)
	{
		if (game->maps[mi]&mLOCKBLOCK)
			remove_lockblocks(screen_handles);
		if (game->maps[mi]&mBOSSLOCKBLOCK)
			remove_bosslockblocks(screen_handles);
		if (game->maps[mi]&mCHEST)
			remove_chests(screen_handles);
		if (game->maps[mi]&mLOCKEDCHEST)
			remove_lockedchests(screen_handles);
		if (game->maps[mi]&mBOSSCHEST)
			remove_bosschests(screen_handles);
		
		clear_xdoors_mi(screen_handles, mi, true);
		clear_xstatecombos_mi(screen_handles, mi, true);
		for_every_combo_in_screen(screen_handles, [&](const auto& handle) {
			// This is duplicated 3 places... can this be reduced?
			auto cid = handle.data();
			auto* cmb = &handle.combo();
			bool done = false;
			std::set<int32_t> visited;
			while(!done)
			{
				if(visited.contains(cid))
				{
					Z_error("Combo '%d' was part of an infinite trigger loop, breaking out of loop", cid);
					break; // prevent infinite loop
				}
				visited.insert(cid);
				
				done = true; // don't loop again unless something changes
				for(size_t idx = 0; idx < cmb->triggers.size(); ++idx)
				{
					auto& trig = cmb->triggers[idx];
					if (trig.triggerflags[4] & combotriggerSCREENLOAD)
						do_trigger_combo(handle, idx);
					else continue; // can skip checking handle.data()
					
					if(handle.data() != cid)
					{
						cid = handle.data();
						cmb = &handle.combo();
						done = false; // loop again for the new combo
						break;
					}
				}
			}
		});
	}

	// check doors
	if (isdungeon(dmap, screen))
	{
		for(int32_t i=0; i<4; i++)
		{
			int32_t door=base_scr->door[i];
			
			switch(door)
			{
			case d1WAYSHUTTER:
			case dSHUTTER:
				if ((ldir^1)==i && screen == hero_screen)
				{
					base_scr->door[i]=dOPENSHUTTER;
				}
				
				get_screen_state(screen).open_doors = -4;
				break;
				
			case dLOCKED:
				if(should_check_for_state_things && game->maps[mi]&(1<<i))
				{
					base_scr->door[i]=dUNLOCKED;
				}
				
				break;
				
			case dBOSS:
				if(should_check_for_state_things && game->maps[mi]&(1<<i))
				{
					base_scr->door[i]=dOPENBOSS;
				}
				
				break;
				
			case dBOMB:
				if(should_check_for_state_things && game->maps[mi]&(1<<i))
				{
					base_scr->door[i]=dBOMBED;
				}
				
				break;
			}

			update_door(base_scr, i, base_scr->door[i]);

			if(door==dSHUTTER||door==d1WAYSHUTTER)
			{
				base_scr->door[i]=door;
			}
		}
	}

	if (!(base_scr->flags3 & fCYCLEONINIT))
		return;

	for (int32_t j=-1; j<6; ++j)  // j == -1 denotes the current screen
	{
		if (j<0 || base_scr->layermap[j] > 0)
		{
			mapscr* layer_scr = get_scr_layer(screen, j + 1);
			mapscr* layerscreen= (j<0 ? base_scr : layer_scr->valid ? layer_scr :
								  &TheMaps[(base_scr->layermap[j]-1)*MAPSCRS]+base_scr->layerscreen[j]);

			for(int32_t i=0; i<176; ++i)
			{
				int32_t c=layerscreen->data[i];
				
				// New screen flag: Cycle Combos At Screen Init
				if(combobuf[c].nextcombo != 0 && (j<0 || get_qr(qr_CMBCYCLELAYERS)))
				{
					int32_t r = 0;
					
					while(combobuf[c].can_cycle() && r++ < 10)
					{
						newcombo const& cmb = combobuf[c];
						bool cycle_under = (cmb.animflags & AF_CYCLEUNDERCOMBO);
						auto cid = cycle_under ? layerscreen->undercombo : cmb.nextcombo;
						layerscreen->data[i] = cid;
						if(!(combobuf[c].animflags & AF_CYCLENOCSET))
							layerscreen->cset[i] = cycle_under ? layerscreen->undercset : cmb.nextcset;
						c = layerscreen->data[i];
					}
				}
			}
		}
	}
}

// Set `cur_screen` to `screen` and load new screens into temporary memory.
//
// Called anytime a player moves to a new screen (either via warping, scrolling, continue, starting
// the game, etc...)
//
// Note: for regions, only the initial screen load calls this function. Simply walking between
// screens in the same region does not use this, because every screen in a region is loaded into
// temporary memory up front.
//
// If scr >= 0x80, `hero_screen` will be saved to `home_screen` and also be loaded into
// `special_warp_return_scr`.
//
// If origin_screen_overlay is true, the old origin_scr combos will be copied to the new origin_scr combos
// on all layers (but only where the new screen has a 0 combo).
//
// TODO: loadscr should set curdmap, but currently callers do that.
void loadscr(int32_t destdmap, int32_t screen, int32_t ldir, bool origin_screen_overlay, bool no_x80_dir)
{
	zapp_reporting_set_tag("screen", screen);
	if (destdmap != -1)
		zapp_reporting_set_tag("dmap", destdmap);

	int32_t orig_destdmap = destdmap;
	if (destdmap < 0) destdmap = cur_dmap;

	if (replay_is_active())
	{
		if (replay_get_mode() == ReplayMode::ManualTakeover)
			replay_stop_manual_takeover();

		if (orig_destdmap != -1)
		{
			if (strlen(DMaps[orig_destdmap].name) > 0)
			{
				replay_step_comment(fmt::format("dmap={} {}", orig_destdmap, DMaps[orig_destdmap].name));
			}
			else
			{
				replay_step_comment(fmt::format("dmap={}", orig_destdmap));
			}
		}
		replay_step_comment_loadscr(screen);

		// Reset the rngs and frame count so that recording steps can be modified without impacting
		// behavior of later screens.
		replay_sync_rng();
	}

	for (auto& state : get_screen_states())
		state.second.triggered_secrets = false;
	slopes.clear();
	Hero.clear_platform_ffc();
	timeExitAllGenscript(GENSCR_ST_CHANGE_SCREEN);
	clear_darkroom_bitmaps();
	msgscr = nullptr;

	for (word x=0; x<animated_combos; x++)
	{
		if(combobuf[animated_combo_table4[x][0]].nextcombo!=0)
		{
			combobuf[animated_combo_table4[x][0]].aclk = 0;
		}
	}
	reset_combo_animations2();
	region_is_lit = false;

	// Legacy features (combo and ffc overlays) may need the previous origin screens during loading
	// of the new ones.
	bool origin_ffc_overlay = false;
	if (origin_scr)
	{
		if (!(origin_scr->flags5&fNOFFCARRYOVER))
		{
			int c = origin_scr->numFFC();
			for (int i = 0; i < c; i++)
			{
				if (origin_scr->ffcs[i].flags&ffc_carryover)
				{
					origin_ffc_overlay = true;
					break;
				}
			}
		}

		if (origin_screen_overlay || origin_ffc_overlay)
		{
			for (int i = 0; i <= 6; i++)
			{
				mapscr* prev_scr = get_scr_layer(cur_screen, i);
				if (prev_scr)
					prev_origin_scrs[i] = *prev_scr;
				else
					prev_origin_scrs[i] = {};
			}
		}
	}

	// Based on origin_ffc_overlay, some ffc scripts don't get reset. This set starts with all of
	// them, but scripts that need their data to persist will be removed.
	loadscr_ffc_script_ids_to_remove.clear();
	for (auto& key : scriptEngineDatas | std::views::keys)
	{
		if (key.first == ScriptType::FFC)
			loadscr_ffc_script_ids_to_remove.insert(key.second);
	}

	load_region(destdmap, screen);
	home_screen = screen >= 0x80 ? hero_screen : cur_screen;
	hero_screen = screen;

	cpos_clear_all();
	FFCore.deallocateAllScriptOwnedOfType(ScriptType::Screen);
	FFCore.deallocateAllScriptOwnedOfType(ScriptType::Combo);
	FFCore.clear_script_engine_data_of_type(ScriptType::Screen);
	FFCore.clear_combo_scripts();

	if (is_in_scrolling_region())
	{
		for (int screen = 0; screen < 128; screen++)
		{
			if (is_in_current_region(screen))
			{
				bool screen_overlay = origin_screen_overlay && screen == cur_screen;
				bool ffc_overlay = origin_ffc_overlay && screen == cur_screen;
				load_a_screen_and_layers_init(destdmap, screen, ldir, screen_overlay, ffc_overlay);
			}
		}
	}
	else
	{
		load_a_screen_and_layers_init(destdmap, screen, ldir, origin_screen_overlay, origin_ffc_overlay);
	}

	prepare_current_region_handles();

	if (is_in_scrolling_region())
	{
		for (int screen = 0; screen < 128; screen++)
		{
			if (is_in_current_region(screen))
			{
				load_a_screen_and_layers_post(destdmap, screen, ldir);
			}
		}
	}
	else
	{
		load_a_screen_and_layers_post(destdmap, screen, ldir);
	}

	// If on a special screen, load the screen the player is currently on (home_screen) into special_warp_return_scr.
	if (screen >= 0x80)
		loadscr_old(orig_destdmap, home_screen, no_x80_dir ? -1 : ldir, origin_screen_overlay);

	update_slope_comboposes();
	cpos_force_update();
	trig_trigger_groups();

	for (int index : loadscr_ffc_script_ids_to_remove)
	{
		FFCore.deallocateAllScriptOwned(ScriptType::FFC, index);
		FFCore.reset_script_engine_data(ScriptType::FFC, index);
	}

	// "extended height mode" includes the top 56 pixels as part of the visible mapscr viewport,
	// allowing for regions to display 4 more rows of combos (as many as ALTTP does). This part of
	// screen is normally reserved for the passive subscreen, but in this mode mapscr combos are drawn below it.
	// It is up to the quest designer to make their subscreen be actually transparent.
	//
	// When not in "extended height mode" (otherwise 56 is 0):
	//  - playing_field_offset: 56, but changes during earthquakes
	//  - original_playing_field_offset: always 56
	//
	// These values are used to adjust where things are drawn on screen to account for the passive subscreen. Examples:
	// - yofs of sprites
	// - bitmap y offsets in draw_screen
	// - drawing offsets for putscr, do_layer
	// - drawing offsets for various calls to overtile16 (see bomb weapon explosion)
	// - lots more
	//
	// TODO: consider refactor of yofs, make yofs start as 0 by default and add playing_field_offset at draw time?
	if (is_extended_height_mode())
	{
		playing_field_offset = 0;
		original_playing_field_offset = 0;
		// A few sprites exist as globals, so we must manually reset them.
		Hero.yofs = 0;
		mblock2.yofs = 0;
	}
	else
	{
		mblock2.yofs = Hero.yofs = playing_field_offset = 56;
		original_playing_field_offset = 56;
	}
	is_any_room_dark = is_any_dark();

	game->load_portal();
	throwGenScriptEvent(GENSCR_EVENT_CHANGE_SCREEN);
	if (Hero.lift_wpn && get_qr(qr_CARRYABLE_NO_ACROSS_SCREEN))
	{
		delete Hero.lift_wpn;
		Hero.lift_wpn = nullptr;
	}

	enemy_spawning_has_checked_been_here = false;
	markBmap(-1, hero_screen);
	Hero.maybe_begin_advanced_maze();
}

// Don't use this directly! Use `loadscr` instead.
void loadscr_old(int32_t destdmap, int32_t screen,int32_t ldir,bool overlay)
{
	int32_t destlvl = DMaps[destdmap < 0 ? cur_dmap : destdmap].level;

	mapscr previous_scr = *special_warp_return_scr;
	mapscr* scr = special_warp_return_scr;
	const mapscr* source = get_canonical_scr(cur_map, screen);
	*scr = *source;

	for (int i = 1; i <= 6; i++)
	{
		if (scr->layermap[i-1] > 0)
			special_warp_return_scrs[i] = *get_canonical_scr(scr->layermap[i-1] - 1, scr->layerscreen[i-1]);
		else
			special_warp_return_scrs[i] = {};
	}

	scr->valid |= mVALID; //layer 0 is always valid

	if ( source->script > 0 )
	{
		scr->script = source->script;
		for ( int32_t q = 0; q < 8; q++ )
		{
			scr->screeninitd[q] = source->screeninitd[q];
		}
		FFCore.reset_script_engine_data(ScriptType::Screen, screen);
	}
	else
	{
		scr->script = 0;
	}
	
	if(overlay)
	{
		for(int32_t c=0; c< 176; ++c)
		{
			if(scr->data[c]==0)
			{
				scr->data[c]=previous_scr.data[c];
				scr->sflag[c]=previous_scr.sflag[c];
				scr->cset[c]=previous_scr.cset[c];
			}
		}
		
		for(int32_t i=0; i<6; i++)
		{
			if(previous_scr.layermap[i]>0 && scr->layermap[i]>0)
			{
				int32_t lm = (scr->layermap[i]-1)*MAPSCRS+scr->layerscreen[i];
				int32_t fm = (previous_scr.layermap[i]-1)*MAPSCRS+previous_scr.layerscreen[i];
				
				for(int32_t c=0; c< 176; ++c)
				{
					if(TheMaps[lm].data[c]==0)
					{
						TheMaps[lm].data[c] = TheMaps[fm].data[c];
						TheMaps[lm].sflag[c] = TheMaps[fm].sflag[c];
						TheMaps[lm].cset[c] = TheMaps[fm].cset[c];
					}
				}
			}
		}
	}

	auto [offx, offy] = translate_screen_coordinates_to_world(screen);
	int c = scr->numFFC();
	for (word i = 0; i < c; i++)
	{
		scr->ffcs[i].screen_spawned = screen;
		scr->ffcs[i].x += offx;
		scr->ffcs[i].y += offy;
	}

	int mi = mapind(cur_map, screen);

	// Apply perm secrets, if applicable.
	if(canPermSecret(destdmap,screen))
	{
		if(game->maps[mi]&mSECRET)			   // if special stuff done before
		{
			reveal_hidden_stairs(scr, screen, false);

			log_trigger_secret_reason(TriggerSource::SecretsScreenState);
			get_screen_state(special_warp_return_scr->screen).triggered_secrets = true;
			bool do_replay_comment = true;
			bool from_active_screen = false;
			trigger_secrets_for_screen_internal(create_screen_handles(special_warp_return_scr), from_active_screen, false, -1, do_replay_comment);
		}
		if(game->maps[mi]&mLIGHTBEAM) // if special stuff done before
		{
			for (int layer = 0; layer <= 6; layer++)
			{
				mapscr* tscr = &special_warp_return_scrs[layer];
				for (int pos = 0; pos < 176; pos++)
				{
					newcombo const* cmb = &combobuf[tscr->data[pos]];
					if(cmb->type == cLIGHTTARGET)
					{
						if(!(cmb->usrflags&cflag1)) //Unlit version
						{
							tscr->data[pos] += 1;
						}
					}
				}
			}
		}
	}
	
	screen_handles_t screen_handles;
	for (int i = 0; i <= 6; i++)
		screen_handles[i] = {scr, special_warp_return_scrs[i].is_valid() ? &special_warp_return_scrs[i] : nullptr, screen, i};

	toggle_switches(game->lvlswitches[destlvl], true, screen_handles);
	toggle_gswitches_load(screen_handles);

	if(game->maps[mi]&mLOCKBLOCK)			  // if special stuff done before
	{
		remove_lockblocks(screen_handles);
	}
	
	if(game->maps[mi]&mBOSSLOCKBLOCK)		  // if special stuff done before
	{
		remove_bosslockblocks(screen_handles);
	}
	
	if(game->maps[mi]&mCHEST)			  // if special stuff done before
	{
		remove_chests(screen_handles);
	}
	
	if(game->maps[mi]&mLOCKEDCHEST)			  // if special stuff done before
	{
		remove_lockedchests(screen_handles);
	}
	
	if(game->maps[mi]&mBOSSCHEST)			  // if special stuff done before
	{
		remove_bosschests(screen_handles);
	}
	
	clear_xdoors(screen_handles, true);
	clear_xstatecombos(screen_handles, true);
	
	for_every_combo_in_screen(screen_handles, [&](const auto& handle) {
		// This is duplicated 3 places... can this be reduced?
		auto cid = handle.data();
		auto* cmb = &handle.combo();
		bool done = false;
		std::set<int32_t> visited;
		while(!done)
		{
			if(visited.contains(cid))
			{
				Z_error("Combo '%d' was part of an infinite trigger loop, breaking out of loop", cid);
				break; // prevent infinite loop
			}
			visited.insert(cid);
			
			done = true; // don't loop again unless something changes
			for(size_t idx = 0; idx < cmb->triggers.size(); ++idx)
			{
				auto& trig = cmb->triggers[idx];
				if (trig.triggerflags[4] & combotriggerSCREENLOAD)
					do_trigger_combo(handle, idx);
				else continue; // can skip checking handle.data()
				
				if(handle.data() != cid)
				{
					cid = handle.data();
					cmb = &handle.combo();
					done = false; // loop again for the new combo
					break;
				}
			}
		}
	});

	// check doors
	if (isdungeon(destdmap, screen))
	{
		for(int32_t i=0; i<4; i++)
		{
			int32_t door=scr->door[i];
			
			switch(door)
			{
			case d1WAYSHUTTER:
			case dSHUTTER:
				if ((ldir^1)==i && screen == home_screen)
				{
					scr->door[i]=dOPENSHUTTER;
				}
				
				get_screen_state(screen).open_doors = -4;
				break;
				
			case dLOCKED:
				if(game->maps[mi]&(1<<i))
				{
					scr->door[i]=dUNLOCKED;
				}
				
				break;
				
			case dBOSS:
				if(game->maps[mi]&(1<<i))
				{
					scr->door[i]=dOPENBOSS;
				}
				
				break;
				
			case dBOMB:
				if(game->maps[mi]&(1<<i))
				{
					scr->door[i]=dBOMBED;
				}
				
				break;
			}

			update_door(scr, i, scr->door[i]);

			if(door==dSHUTTER||door==d1WAYSHUTTER)
			{
				scr->door[i]=door;
			}
		}
	}
	
	for(int32_t j=-1; j<6; ++j)  // j == -1 denotes the current screen
	{
		if (j<0 || scr->layermap[j] > 0)
		{
			mapscr *layerscreen= (j<0 ? scr : special_warp_return_scrs[j+1].valid ? &special_warp_return_scrs[j+1] :
								  &TheMaps[(scr->layermap[j]-1)*MAPSCRS]+scr->layerscreen[j]);
								  
			for(int32_t i=0; i<176; ++i)
			{
				int32_t c=layerscreen->data[i];
				
				// New screen flag: Cycle Combos At Screen Init
				if(combobuf[c].nextcombo != 0 && (scr->flags3 & fCYCLEONINIT) && (j<0 || get_qr(qr_CMBCYCLELAYERS)))
				{
					int32_t r = 0;
					
					while(combobuf[c].can_cycle() && r++ < 10)
					{
						newcombo const& cmb = combobuf[c];
						bool cycle_under = (cmb.animflags & AF_CYCLEUNDERCOMBO);
						auto cid = cycle_under ? layerscreen->undercombo : cmb.nextcombo;
						layerscreen->data[i] = cid;
						if(!(combobuf[c].animflags & AF_CYCLENOCSET))
							layerscreen->cset[i] = cycle_under ? layerscreen->undercset : cmb.nextcset;
						c = layerscreen->data[i];
					}
				}
			}
		}
	}
}

// Load screen (and layers). Unlike loadscr, this doesn't load to the global temporary_screens, but
// instead returns an array of mapscr.
// Used to draw/save the map.
std::array<mapscr, 7> loadscr2(int32_t screen)
{
	std::array<mapscr, 7> scrs;
	mapscr* scr = &scrs[0];

	for(word x=0; x<animated_combos; x++)
	{
		if(combobuf[animated_combo_table4[x][0]].nextcombo!=0)
		{
			combobuf[animated_combo_table4[x][0]].aclk=0;
		}
	}

	const mapscr* source = get_canonical_scr(cur_map, screen);
	if (!source->is_valid())
	{
		scrs[0].valid = 0;
		return scrs;
	}

	*scr = *get_canonical_scr(cur_map, screen);
	for (int i = 1; i <= 6; i++)
	{
		if (scr->layermap[i-1] > 0)
			scrs[i] = *get_canonical_scr(scr->layermap[i-1] - 1, scr->layerscreen[i-1]);
		else
			scrs[i] = {};
	}

	screen_handles_t screen_handles;
	for (int i = 0; i < 7; i++)
		screen_handles[i] = {scr, scrs[i].is_valid() ? &scrs[i] : nullptr, screen, i};

	int mi = mapind(cur_map, screen);
	
	if(canPermSecret(-1,screen))
	{
		if(game->maps[mi]&mSECRET)			   // if special stuff done before
		{
			reveal_hidden_stairs(scr, screen, false);
			bool from_active_screen = false;
			bool do_replay_comment = true;
			trigger_secrets_for_screen_internal(screen_handles, from_active_screen, false, -1, do_replay_comment);
		}
		if(game->maps[mi]&mLIGHTBEAM) // if special stuff done before
		{
			for (int layer = 0; layer <= 6; layer++)
			{
				mapscr* tscr = &scrs[layer];
				for (int pos = 0; pos < 176; pos++)
				{
					newcombo const* cmb = &combobuf[tscr->data[pos]];
					if(cmb->type == cLIGHTTARGET)
					{
						if(!(cmb->usrflags&cflag1)) //Unlit version
						{
							tscr->data[pos] += 1;
						}
					}
				}
			}
		}
	}

	if(game->maps[mi]&mLOCKBLOCK)			  // if special stuff done before
	{
		remove_lockblocks(screen_handles);
	}
	
	if(game->maps[mi]&mBOSSLOCKBLOCK)		  // if special stuff done before
	{
		remove_bosslockblocks(screen_handles);
	}
	
	if(game->maps[mi]&mCHEST)			  // if special stuff done before
	{
		remove_chests(screen_handles);
	}
	
	if(game->maps[mi]&mLOCKEDCHEST)			  // if special stuff done before
	{
		remove_lockedchests(screen_handles);
	}
	
	if(game->maps[mi]&mBOSSCHEST)			  // if special stuff done before
	{
		remove_bosschests(screen_handles);
	}

	clear_xdoors(screen_handles);
	clear_xstatecombos(screen_handles);
	
	// check doors
	if (isdungeon(screen))
	{
		for(int32_t i=0; i<4; i++)
		{
			int32_t door=scr->door[i];
			bool putit=true;
			
			switch(door)
			{
			case d1WAYSHUTTER:
			case dSHUTTER:
				break;
				
			case dLOCKED:
				if(game->maps[mi]&(1<<i))
				{
					scr->door[i]=dUNLOCKED;
				}
				
				break;
				
			case dBOSS:
				if(game->maps[mi]&(1<<i))
				{
					scr->door[i]=dOPENBOSS;
				}
				
				break;
				
			case dBOMB:
				if(game->maps[mi]&(1<<i))
				{
					scr->door[i]=dBOMBED;
				}
				
				break;
			}
			
			if(putit)
			{
				putdoor(scr, scrollbuf, i, scr->door[i], false);
			}
			
			if(door==dSHUTTER||door==d1WAYSHUTTER)
			{
				scr->door[i]=door;
			}
		}
	}
	
	for(int32_t j=-1; j<6; ++j)  // j == -1 denotes the current screen
	{
		if (j < 0 || scr->layermap[j] > 0)
		{
			mapscr *layerscreen= (j<0 ? scr
								  : &(TheMaps[(scr->layermap[j]-1)*MAPSCRS+scr->layerscreen[j]]));
								  
			for(int32_t i=0; i<176; ++i)
			{
				int32_t c=layerscreen->data[i];
				
				// New screen flag: Cycle Combos At Screen Init
				if((scr->flags3 & fCYCLEONINIT) && (j<0 || get_qr(qr_CMBCYCLELAYERS)))
				{
					int32_t r = 0;
					
					while(combobuf[c].can_cycle() && r++ < 10)
					{
						newcombo const& cmb = combobuf[c];
						bool cycle_under = (cmb.animflags & AF_CYCLEUNDERCOMBO);
						auto cid = cycle_under ? layerscreen->undercombo : cmb.nextcombo;
						layerscreen->data[i] = cid;
						if(!(combobuf[c].animflags & AF_CYCLENOCSET))
							layerscreen->cset[i] = cycle_under ? layerscreen->undercset : cmb.nextcset;
						c = layerscreen->data[i];
					}
				}
			}
		}
	}

	return scrs;
}

void putscr(mapscr* scr, BITMAP* dest, int32_t x, int32_t y)
{
	// This is a bogus value while screenscrolling == true, but that's ok
	// because it is only used to calculate the rpos, and during screenscrolling
	// only the modulus to get pos (draw_cmb_pos does RPOS_TO_POS) is needed, which
	// is always the same no matter the value of scr.
	int screen = get_screen_for_world_xy(x, y);

	x -= viewport.x;
	y -= viewport.y;

	if (!scr->is_valid()||!show_layers[0]||scr->hidelayers & 1)
	{
		rectfill(dest,x,y,x+255,y+175,0);
		return;
	}
	
	bool over = XOR(scr->flags7&fLAYER2BG,DMaps[cur_dmap].flags&dmfLAYER2BG)
		|| XOR(scr->flags7&fLAYER3BG, DMaps[cur_dmap].flags&dmfLAYER3BG);

	int start_x, end_x, start_y, end_y;
	get_bounds_for_draw_cmb_calls(dest, x, y, start_x, end_x, start_y, end_y);
	for (int cy = start_y; cy < end_y; cy++)
	{
		for (int cx = start_x; cx < end_x; cx++)
		{
			int i = cx + cy*16;
			auto rpos = screenscrolling ? rpos_t::None : POS_TO_RPOS(i, screen);
			draw_cmb_pos(dest, x + cx*16, y + cy*16, rpos, scr->data[i], scr->cset[i], 0, over, false);
		}
	}
}

static void putscrdoors(const nearby_screens_t& nearby_screens, BITMAP *dest, int32_t x, int32_t y)
{
	if (!show_layers[0])
	{
		return;
	}
	
	x -= viewport.x;
	y -= viewport.y;

	for_every_nearby_screen(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy) {
		mapscr* scr = screen_handles[0].base_scr;
		if (!scr->is_valid())
			return;

		if(scr->door[0]==dBOMBED)
		{
			over_door(scr, dest, 39, up, offx+x, offy+y);
		}
		
		if(scr->door[1]==dBOMBED)
		{
			over_door(scr, dest, 135, down, offx+x, offy+y);
		}
		
		if(scr->door[2]==dBOMBED)
		{
			over_door(scr, dest, 66, left, offx+x, offy+y);
		}
		
		if(scr->door[3]==dBOMBED)
		{
			over_door(scr, dest, 77, right, offx+x, offy+y);
		}
	});
}

void putscrdoors(mapscr* scr, BITMAP *dest, int32_t x, int32_t y)
{
	if (!scr->is_valid() || !show_layers[0])
		return;

	x -= viewport.x;
	y -= viewport.y;

	if(scr->door[0]==dBOMBED)
	{
		over_door(scr,dest,39,up,x,y);
	}

	if(scr->door[1]==dBOMBED)
	{
		over_door(scr,dest,135,down,x,y);
	}

	if(scr->door[2]==dBOMBED)
	{
		over_door(scr,dest,66,left,x,y);
	}

	if(scr->door[3]==dBOMBED)
	{
		over_door(scr,dest,77,right,x,y);
	}
}
static inline bool onSwitch(newcombo const& cmb, zfix const& switchblockstate)
{
	return (switchblockstate < 0 || (cmb.attributes[2]>0 && (zslongToFix(cmb.attributes[2]) - zslongToFix(zc_max(cmb.attributes[3], 0))) <=switchblockstate));
}
bool _walkflag(zfix_round zx,zfix_round zy,int32_t cnt)
{
	return _walkflag(zx,zy,cnt,0_zf);
}

bool _walkflag_new(const mapscr* s0, const mapscr* s1, const mapscr* s2, zfix_round zx, zfix_round zy, zfix const& switchblockstate, bool is_temp_screens)
{
	int x = zx.getRound(), y = zy.getRound();
	int pos = COMBOPOS(x % 256, y % 176);
	const newcombo& c = combobuf[s0->data[pos]];
	const newcombo& c1 = combobuf[s1->data[pos]];
	const newcombo& c2 = combobuf[s2->data[pos]];
	bool dried = (((iswater_type(c.type)) || (iswater_type(c1.type)) ||
				   (iswater_type(c2.type))) && DRIEDLAKE);
	int32_t b=1;
	if(x&8) b<<=2;
	if(y&8) b<<=1;
	
	int32_t cwalkflag = c.walk;
	if(is_temp_screens && onSwitch(c,switchblockstate) && c.type == cCSWITCHBLOCK && c.usrflags&cflag9) cwalkflag &= (c.walk>>4)^0xF;
	else if ((c.type == cBRIDGE && get_qr(qr_OLD_BRIDGE_COMBOS)) || (iswater_type(c.type) && ((c.walk>>4)&b) && ((c.usrflags&cflag3) || (c.usrflags&cflag4)))) cwalkflag = 0;
	if (s1 != s0)
	{
		if(is_temp_screens && onSwitch(c1,switchblockstate) && c1.type == cCSWITCHBLOCK && c1.usrflags&cflag9) cwalkflag &= (c1.walk>>4)^0xF;
		else if ((iswater_type(c1.type) && ((c1.walk>>4)&b) && get_qr(qr_WATER_ON_LAYER_1) && !((c1.usrflags&cflag3) || (c1.usrflags&cflag4)))) cwalkflag &= c1.walk;
		else if (c1.type == cBRIDGE)
		{
			if (!get_qr(qr_OLD_BRIDGE_COMBOS))
			{
				int efflag = (c1.walk & 0xF0)>>4;
				int newsolid = (c1.walk & 0xF);
				cwalkflag = ((newsolid | cwalkflag) & (~efflag)) | (newsolid & efflag);
			}
			else cwalkflag &= c1.walk;
		}
		else if ((iswater_type(c1.type) && get_qr(qr_WATER_ON_LAYER_1) && ((c1.usrflags&cflag3) || (c1.usrflags&cflag4)) && ((c1.walk>>4)&b))) cwalkflag = 0;
		else cwalkflag |= c1.walk;
	}
	if (s2 != s0)
	{
		if(is_temp_screens && onSwitch(c2,switchblockstate) && c2.type == cCSWITCHBLOCK && c2.usrflags&cflag9) cwalkflag &= (c2.walk>>4)^0xF;
		else if ((iswater_type(c2.type) && ((c2.walk>>4)&b) && get_qr(qr_WATER_ON_LAYER_2) && !((c2.usrflags&cflag3) || (c2.usrflags&cflag4)))) cwalkflag &= c2.walk;
		else if (c2.type == cBRIDGE)
		{
			if (!get_qr(qr_OLD_BRIDGE_COMBOS))
			{
				int efflag = (c2.walk & 0xF0)>>4;
				int newsolid = (c2.walk & 0xF);
				cwalkflag = ((newsolid | cwalkflag) & (~efflag)) | (newsolid & efflag);
			}
			else cwalkflag &= c2.walk;
		}
		else if ((iswater_type(c2.type) && get_qr(qr_WATER_ON_LAYER_2) && ((c2.usrflags&cflag3) || (c2.usrflags&cflag4))) && ((c2.walk>>4)&b)) cwalkflag = 0;
		else cwalkflag |= c2.walk;
	}
	
	if((cwalkflag&b) && !dried)
		return true;
	
	if (is_temp_screens && collide_object(zx, zy, 0.0001_zf, 0.0001_zf)) return true;

	return false;
}

// Returns true if the combo at viewport position x,y is solid. Looks at a combo's quadrant walkablity flags.
static bool _walkflag_new(zfix_round zx, zfix_round zy, zfix const& switchblockstate)
{
	int x = zx.getRound(), y = zy.getRound();
	mapscr* s0 = get_scr_for_world_xy_layer(x, y, 0);
	mapscr* s1 = get_scr_for_world_xy_layer(x, y, 1);
	mapscr* s2 = get_scr_for_world_xy_layer(x, y, 2);
	if (!s1->valid) s1 = s0;
	if (!s2->valid) s2 = s0;
	return _walkflag_new(s0, s1, s2, zx, zy, switchblockstate, true);
}

bool _walkflag(zfix_round x,zfix_round y,int32_t cnt,zfix const& switchblockstate)
{
	int max_x = world_w;
	int max_y = world_h;
	if (!get_qr(qr_LTTPWALK))
	{
		max_x -= 7;
		max_y -= 7;
	}
	if (x < 0 || y < 0) return false;
	if (x >= max_x) return false;
	if (x >= max_x - 8 && cnt == 2) return false;
	if (y >= max_y) return false;
	
	return _walkflag_new(x, y, switchblockstate) || (cnt != 1 && _walkflag_new(x + 8, y, switchblockstate));
}

static bool effectflag(int32_t x, int32_t y, int32_t layer)
{
	mapscr* s0 = get_scr_for_world_xy(x, y);
	mapscr* s1 = get_scr_for_world_xy_layer(x, y, 1);
	mapscr* s2 = get_scr_for_world_xy_layer(x, y, 2);
	if (!s1->valid) s1 = s0;
	if (!s2->valid) s2 = s0;

	int pos = COMBOPOS(x % 256, y % 176);
	const newcombo& c = combobuf[s0->data[pos]];
	const newcombo& c1 = combobuf[s1->data[pos]];
	const newcombo& c2 = combobuf[s2->data[pos]];
	bool dried = (((iswater_type(c.type)) || (iswater_type(c1.type)) ||
				   (iswater_type(c2.type))) && DRIEDLAKE);
	int32_t b=1;
	if(x&8) b<<=2;
	if(y&8) b<<=1;

	int32_t cwalkflag = (c.walk>>4);
	if (layer == 0) cwalkflag = (c1.walk>>4);
	if (layer == 1) cwalkflag = (c2.walk>>4);
	//if (c.type == cBRIDGE || (iswater_type(c.type) && ((c.usrflags&cflag3) || (c.usrflags&cflag4)))) cwalkflag = 0;
	if (s1 != s0 && layer < 0)
	{
		if (c1.type == cBRIDGE) cwalkflag &= (~(c1.walk>>4));
	}
	if (s2 != s0 && layer < 1)
	{
		if (c2.type == cBRIDGE) cwalkflag &= (~(c2.walk>>4));
	}
	
	return (cwalkflag&b) ? !dried : false;
}

bool _effectflag(int32_t x,int32_t y,int32_t cnt, int32_t layer, bool notLink)
{
	DCHECK(cnt == 0 || cnt == 1);
	int max_x = world_w;
	int max_y = world_h;
	if (!get_qr(qr_LTTPWALK) && !notLink)
	{
		max_x -= 7;
		max_y -= 7;
	}
	if (x < 0 || y < 0) return false;
	if (x >= max_x) return false;
	if (x >= max_x - 8 && cnt == 2) return false;
	if (y >= max_y) return false;

	return effectflag(x, y, layer) || (cnt == 2 && effectflag(x + 8, y, layer));
}

// used by mapdata->isSolid(x,y) in ZScript
bool _walkflag(zfix_round zx,zfix_round zy,int32_t cnt, mapscr* m)
{
	int x = zx.getRound(), y = zy.getRound();
	{
		int max_x = 256;
		int max_y = 176;
		if (!get_qr(qr_LTTPWALK))
		{
			max_x -= 7;
			max_y -= 7;
		}
		if (x < 0 || y < 0) return false;
		if (x >= max_x) return false;
		if (x >= max_x - 8 && cnt == 2) return false;
		if (y >= max_y) return false;
	}
	
	const mapscr *s1, *s2;
	
	if ( m->layermap[0] > 0 )
	{
		s1 = get_canonical_scr(m->layermap[0], m->layerscreen[0]);
	}
	else s1 = m;
	
	if ( m->layermap[1] > 0 )
	{
		s2 = get_canonical_scr(m->layermap[1], m->layerscreen[1]);
	}
	else s2 = m;

	zfix unused;
	return _walkflag_new(m, s1, s2, x, y, unused, false) || (cnt != 1 && _walkflag_new(m, s1, s2, x + 8, y, unused, false));
}

bool _walkflag_layer(zfix_round x, zfix_round y, int32_t layer, int32_t cnt)
{
	DCHECK_LAYER_NEG1_INDEX(layer);
	mapscr* m = get_scr_for_world_xy_layer(x, y, layer + 1);
	if (!m->is_valid()) return false;
	return _walkflag_layer(x, y, cnt, m);
}

static bool _walkflag_layer_new(zfix_round zx,zfix_round zy,int32_t cnt, mapscr* m, int max_x, int max_y)
{
	int x = zx.getRound(), y = zy.getRound();

	if (!get_qr(qr_LTTPWALK))
	{
		max_x -= 7;
		max_y -= 7;
	}
	if (x < 0 || y < 0) return false;
	if (x >= max_x) return false;
	if (x >= max_x - 8 && cnt == 2) return false;
	if (y >= max_y) return false;

	if(!m) return true;
	
	int pos = COMBOPOS(x%256, y%176);
	const newcombo* c = &combobuf[m->data[pos]];
	bool dried = ((iswater_type(c->type)) && DRIEDLAKE);
	int32_t b=1;
	
	if(x&8) b<<=2;
	
	if(y&8) b<<=1;
	
	if((c->walk&b) && !dried)
		return true;
		
	if(cnt==1) return false;
	
	++pos;
	
	if(!(x&8))
		b<<=2;
	else
	{
		c  = &combobuf[m->data[pos]];
		dried = ((iswater_type(c->type)) && DRIEDLAKE);
		b=1;
		
		if(y&8) b<<=1;
	}
	
	return (c->walk&b) ? !dried : false;
}

//Only check the given mapscr*, not its layer 1&2
bool _walkflag_layer(zfix_round zx,zfix_round zy,int32_t cnt, mapscr* m)
{
	return _walkflag_layer_new(zx, zy, cnt, m, world_w, world_h);
}

bool _walkflag_layer_scrolling(zfix_round zx,zfix_round zy,int32_t cnt, mapscr* m)
{
	return _walkflag_layer_new(zx, zy, cnt, m, scrolling_region.width, scrolling_region.height);
}

bool _effectflag_layer(int32_t x, int32_t y, int32_t layer, int32_t cnt, bool notLink)
{
	DCHECK_LAYER_NEG1_INDEX(layer);
	mapscr* m = get_scr_for_world_xy_layer(x, y, layer + 1);
	if (!m->is_valid()) return false;
	return _effectflag_layer(x, y, cnt, m, notLink);
}

bool _effectflag_layer(int32_t x,int32_t y,int32_t cnt, mapscr* m, bool notLink)
{
	int max_x = world_w;
	int max_y = world_h;
	if (!get_qr(qr_LTTPWALK) && !notLink)
	{
		max_x -= 7;
		max_y -= 7;
	}
	if (x < 0 || y < 0) return false;
	if (x >= max_x) return false;
	if (x >= max_x - 8 && cnt == 2) return false;
	if (y >= max_y) return false;

	if (!m) return true;
	
	int pos = COMBOPOS(x%256, y%176);
	const newcombo* c = &combobuf[m->data[pos]];
	bool dried = ((iswater_type(c->type)) && DRIEDLAKE);
	int32_t b=1;
	
	if(x&8) b<<=2;
	
	if(y&8) b<<=1;
	
	if(((c->walk>>4)&b) && !dried)
		return true;
		
	if(cnt==1) return false;
	
	++pos;
	
	if(!(x&8))
		b<<=2;
	else
	{
		c  = &combobuf[m->data[pos]];
		dried = ((iswater_type(c->type)) && DRIEDLAKE);
		b=1;
		
		if(y&8) b<<=1;
	}
	
	return ((c->walk>>4)&b) ? !dried : false;
}

bool water_walkflag(int32_t x, int32_t y, int32_t cnt)
{
	int max_x = world_w;
	int max_y = world_h;
	if (!get_qr(qr_LTTPWALK))
	{
		max_x -= 7;
		max_y -= 7;
	}
	if (x < 0 || y < 0) return false;
	if (x >= max_x) return false;
	if (x >= max_x - 8 && cnt == 2) return false;
	if (y >= max_y) return false;

	return water_walkflag(x, y) || (cnt != 1 && water_walkflag(x + 8, y));
}

bool water_walkflag(int32_t x, int32_t y)
{
	const newcombo& c = combobuf[MAPCOMBO2(-1, x, y)];
	const newcombo& c1 = combobuf[MAPCOMBO2(0, x, y)];
	const newcombo& c2 = combobuf[MAPCOMBO2(1, x, y)];

	int32_t b=1;
	if(x&8) b<<=2;
	if(y&8) b<<=1;
	
	if(get_qr(qr_NO_SOLID_SWIM))
	{
		if(c.walk&b)
			return true;
			
		if(c1.walk&b)
			return true;
			
		if(c2.walk&b)
			return true;
	}
	else
	{
		if((c.walk&b) && !iswater_type(c.type))
			return true;
			
		if((c1.walk&b) && !iswater_type(c1.type))
			return true;
			
		if((c2.walk&b) && !iswater_type(c2.type))
			return true;
	}

	return false;
}

bool hit_walkflag(int32_t x,int32_t y,int32_t cnt)
{
	if(dlevel)
		if(x<32 || y<40 || (x+(cnt-1)*8)>=world_w-32 || y>=world_h-32)
			return true;
	
	if(blockpath && y<((get_qr(qr_LTTPCOLLISION))?80:88))
		return true;
		
	if(x<16 || y<16 || (x+(cnt-1)*8)>=world_w-16 || y>=world_h-16)
		return true;
		
	//  for(int32_t i=0; i<4; i++)
	if(mblock2.clk && mblock2.hit(x,y,0,cnt*8,1,16))
		return true;
	
	if (collide_object(x, y,cnt*8, 1))
		return true;
		
	return _walkflag(x,y,cnt);
}

bool solpush_walkflag(int32_t x, int32_t y, int32_t cnt, solid_object const* ign)
{
	// 16 pixel buffer to account for slopes that are on bordering screens.
	if(x<0 || y<0 || x>=world_w+16 || y>=world_h+16)
		return true;
		
	//  for(int32_t i=0; i<4; i++)
	if(mblock2.clk && mblock2.hit(x,y,0,cnt*8,1,16))
		return true;
	
	if (collide_object(x, y,cnt*8, 1, ign))
		return true;
		
	return _walkflag(x,y,cnt);
}

void map_bkgsfx(bool on)
{
	if(on)
	{
		cont_sfx(hero_scr->oceansfx);
		
		if(hero_scr->bosssfx && !(game->lvlitems[dlevel]&liBOSS))
			cont_sfx(hero_scr->bosssfx);
	}
	else
	{
		adjust_sfx(hero_scr->oceansfx,128,false);
		adjust_sfx(hero_scr->bosssfx,128,false);
		
		for(int32_t i=0; i<guys.Count(); i++)
		{
			if(((enemy*)guys.spr(i))->bgsfx)
				stop_sfx(((enemy*)guys.spr(i))->bgsfx);
		}
	}
}

void toggle_switches(dword flags, bool entry)
{
	if(!flags) return; //No flags to toggle

	for_every_base_screen_in_region([&](mapscr* scr, unsigned int region_scr_x, unsigned int region_scr_y) {
		toggle_switches(flags, entry, create_screen_handles(scr));
	});
}
void toggle_switches(dword flags, bool entry, const screen_handles_t& screen_handles)
{
	if(!flags) return; //No flags to toggle

	mapscr* m = screen_handles[0].base_scr;
	int screen = m->screen;
	bool is_active_screen = is_in_current_region(m);

	for_every_rpos_in_screen(screen_handles, [&](const rpos_handle_t& rpos_handle) {
		byte togglegrid[176] = {0};
		mapscr* scr = rpos_handle.scr;
		int lyr = rpos_handle.layer;
		int pos = rpos_handle.pos;
		newcombo const& cmb = combobuf[scr->data[pos]];
		if(is_active_screen)
			for(size_t idx = 0; idx < cmb.triggers.size(); ++idx)
			{
				auto& trig = cmb.triggers[idx];
				if((trig.triggerflags[3] & combotriggerTRIGLEVELSTATE) && trig.trig_lstate < 32)
					if(flags&(1<<trig.trig_lstate))
					{
						auto oldcombo = rpos_handle.data();
						do_trigger_combo(rpos_handle, idx, ctrigSWITCHSTATE);
						if(rpos_handle.data() != oldcombo) break;
					}
			}
		if((cmb.type == cCSWITCH || cmb.type == cCSWITCHBLOCK) && cmb.attribytes[0] < 32
			&& !(cmb.usrflags & cflag11)) //global state
		{
			if(flags&(1<<cmb.attribytes[0]))
			{
				set<int32_t> oldData;
				//Increment the combo/cset by the attributes
				int32_t cmbofs = (cmb.attributes[0]/10000L);
				int32_t csofs = (cmb.attributes[1]/10000L);
				oldData.insert(scr->data[pos]);
				scr->data[pos] = BOUND_COMBO(scr->data[pos] + cmbofs);
				scr->cset[pos] = (scr->cset[pos] + csofs) & 15;
				if(entry && (cmb.usrflags&cflag8))
				{
					newcombo const* tmp = &combobuf[scr->data[pos]];
					while(tmp->can_cycle())
					{
						bool cycle_under = (tmp->animflags & AF_CYCLEUNDERCOMBO);
						auto cid = cycle_under ? scr->undercombo : tmp->nextcombo;
						if(oldData.find(cid) != oldData.end())
							break;

						scr->data[pos] = cid;
						if(!(tmp->animflags & AF_CYCLENOCSET))
							scr->cset[pos] = cycle_under ? scr->undercset : tmp->nextcset;
						oldData.insert(cid);
						tmp = &combobuf[cid];
					}
				}
				int32_t cmbid = scr->data[pos];
				if(combobuf[cmbid].animflags & AF_CYCLE)
				{
					combobuf[cmbid].tile = combobuf[cmbid].o_tile;
					combobuf[cmbid].cur_frame=0;
					combobuf[cmbid].aclk = 0;
					combo_caches::drawing.refresh(cmbid);
				}
				togglegrid[pos] |= (1<<lyr); //Mark this pos toggled for this layer
				if(cmb.type == cCSWITCH) return; //Switches don't toggle other layers
				for(int32_t lyr2 = 0; lyr2 < 7; ++lyr2) //Toggle same pos on other layers, if flag set
				{
					if(lyr==lyr2) return;
					if(!(cmb.usrflags&(1<<lyr2))) return;
					if(togglegrid[pos]&(1<<lyr2)) return;
					mapscr* scr_2 = (lyr2 ? get_scr_layer(screen, lyr2) : m);
					if(!scr_2->data[pos]) //Don't increment empty space
						return;
					newcombo const& cmb_2 = combobuf[scr_2->data[pos]];
					if(lyr2 > lyr && (cmb_2.type == cCSWITCH || cmb_2.type == cCSWITCHBLOCK) && !(cmb.usrflags & cflag11)
							&& cmb_2.attribytes[0] < 32 && (flags&(1<<cmb_2.attribytes[0])))
						return; //This is a switch/block that will be hit later in the loop!
					set<int32_t> oldData2;
					//Increment the combo/cset by the original cmb's attributes
					oldData2.insert(scr_2->data[pos]);
					scr_2->data[pos] = BOUND_COMBO(scr_2->data[pos] + cmbofs);
					scr_2->cset[pos] = (scr_2->cset[pos] + csofs) & 15;
					if(entry && (cmb.usrflags&cflag8)) //Skip cycling on screen entry
					{
						newcombo const* tmp = &combobuf[scr_2->data[pos]];
						while(tmp->can_cycle())
						{
							bool cycle_under = (tmp->animflags & AF_CYCLEUNDERCOMBO);
							auto cid = cycle_under ? scr_2->undercombo : tmp->nextcombo;
							if(oldData2.find(cid) != oldData2.end())
								break;

							scr_2->data[pos] = cid;
							if(!(tmp->animflags & AF_CYCLENOCSET))
								scr_2->cset[pos] = cycle_under ? scr_2->undercset : tmp->nextcset;
							oldData2.insert(cid);
							tmp = &combobuf[cid];
						}
					}
					int32_t cmbid2 = scr_2->data[pos];
					if(combobuf[cmbid2].animflags & AF_CYCLE)
					{
						combobuf[cmbid2].tile = combobuf[cmbid2].o_tile;
						combobuf[cmbid2].cur_frame=0;
						combobuf[cmbid2].aclk = 0;
						combo_caches::drawing.refresh(cmbid2);
					}
					togglegrid[pos] |= (1<<lyr2); //Mark this pos toggled for this layer
				}
			}
		}
	});

	if(get_qr(qr_SWITCHES_AFFECT_MOVINGBLOCKS) && mblock2.clk)
	{
		newcombo const& cmb = combobuf[mblock2.bcombo];
		if(!(cmb.usrflags & cflag11) && (cmb.type == cCSWITCH || cmb.type == cCSWITCHBLOCK) && cmb.attribytes[0] < 32)
		{
			if(flags&(1<<cmb.attribytes[0]))
			{
				//Increment the combo/cset by the attributes
				int32_t cmbofs = (cmb.attributes[0]/10000L);
				int32_t csofs = (cmb.attributes[1]/10000L);
				mblock2.bcombo = BOUND_COMBO(mblock2.bcombo + cmbofs);
				mblock2.cs = (mblock2.cs + csofs) & 15;
				int32_t cmbid = mblock2.bcombo;
				if(combobuf[cmbid].animflags & AF_CYCLE)
				{
					combobuf[cmbid].tile = combobuf[cmbid].o_tile;
					combobuf[cmbid].cur_frame=0;
					combobuf[cmbid].aclk = 0;
					combo_caches::drawing.refresh(cmbid);
				}
			}
		}
	}
	
	if (is_active_screen)
	{
		int screen_index_offset = get_region_screen_offset(m->screen);
		word c = m->numFFC();
		for (int q = 0; q < c; ++q)
		{
			auto ffc_handle = *m->getFFCHandle(q, screen_index_offset);
			auto& cmb = ffc_handle.combo();
			for(size_t idx = 0; idx < cmb.triggers.size(); ++idx)
			{
				auto& trig = cmb.triggers[idx];
				if((trig.triggerflags[3] & combotriggerTRIGLEVELSTATE) && trig.trig_lstate < 32)
					if(flags&(1<<trig.trig_lstate))
					{
						auto oldcombo = ffc_handle.data();
						do_trigger_combo(ffc_handle, idx, ctrigSWITCHSTATE);
						if(ffc_handle.data() != oldcombo) break;
					}
			}
		}
	}
}

void toggle_gswitches(int32_t state, bool entry)
{
	for_every_base_screen_in_region([&](mapscr* scr, unsigned int region_scr_x, unsigned int region_scr_y) {
		toggle_gswitches(state, entry, create_screen_handles(scr));
	});
}
void toggle_gswitches(int32_t state, bool entry, const screen_handles_t& screen_handles)
{
	bool states[256] = {false};
	states[state] = true;
	toggle_gswitches(states, entry, screen_handles);
}
void toggle_gswitches(bool* states, bool entry, const screen_handles_t& screen_handles)
{
	if(!states) return;

	auto& combo_cache = combo_caches::gswitch;
	mapscr* base_scr = screen_handles[0].base_scr;
	int screen = base_scr->screen;
	bool is_active_screen = is_in_current_region(base_scr);
	byte togglegrid[176] = {0};
	for(int32_t lyr = 0; lyr <= 6; ++lyr)
	{
		mapscr* scr = screen_handles[lyr].scr;
		if (!scr)
			continue;

		for(int32_t pos = 0; pos < 176; ++pos)
		{
			int cid = scr->data[pos];
			auto& mini_cmb = combo_cache.minis[cid];

			if (is_active_screen)
			{
				if (mini_cmb.trigger_global_state)
				{
					auto& cmb = combobuf[cid];
					for(size_t idx = 0; idx < cmb.triggers.size(); ++idx)
					{
						auto& trig = cmb.triggers[idx];
						if ((trig.triggerflags[3] & combotriggerTRIGGLOBALSTATE) && states[trig.trig_gstate])
						{
							auto rpos_handle = get_rpos_handle_for_screen(screen, lyr, pos);
							do_trigger_combo(rpos_handle, idx, ctrigSWITCHSTATE);
							if(rpos_handle.data() != cid) break;
						}
					}
				}
			}

			if (!mini_cmb.has_global_state)
				continue;

			newcombo const& cmb = combobuf[cid];
			if(cmb.type == cCSWITCH || cmb.type == cCSWITCHBLOCK)
			{
				if(states[cmb.attribytes[0]])
				{
					set<int32_t> oldData;
					//Increment the combo/cset by the attributes
					int32_t cmbofs = (cmb.attributes[0]/10000L);
					int32_t csofs = (cmb.attributes[1]/10000L);
					oldData.insert(scr->data[pos]);
					scr->data[pos] = BOUND_COMBO(scr->data[pos] + cmbofs);
					scr->cset[pos] = (scr->cset[pos] + csofs) & 15;
					if(entry && (cmb.usrflags&cflag8))
					{
						newcombo const* tmp = &combobuf[scr->data[pos]];
						while(tmp->can_cycle())
						{
							bool cycle_under = (tmp->animflags & AF_CYCLEUNDERCOMBO);
							auto cid = cycle_under ? scr->undercombo : tmp->nextcombo;
							if(oldData.find(cid) != oldData.end())
								break;
							scr->data[pos] = cid;
							if(!(tmp->animflags & AF_CYCLENOCSET))
								scr->cset[pos] = cycle_under ? scr->undercset : tmp->nextcset;
							oldData.insert(cid);
							tmp = &combobuf[cid];
						}
					}
					int32_t cmbid = scr->data[pos];
					if(combobuf[cmbid].animflags & AF_CYCLE)
					{
						combobuf[cmbid].tile = combobuf[cmbid].o_tile;
						combobuf[cmbid].cur_frame=0;
						combobuf[cmbid].aclk = 0;
						combo_caches::drawing.refresh(cmbid);
					}
					togglegrid[pos] |= (1<<lyr); //Mark this pos toggled for this layer
					if(cmb.type == cCSWITCH) continue; //Switches don't toggle other layers
					for(int32_t lyr2 = 0; lyr2 <= 6; ++lyr2) //Toggle same pos on other layers, if flag set
					{
						if(lyr==lyr2) continue;
						if(!(cmb.usrflags&(1<<lyr2))) continue;
						if(togglegrid[pos]&(1<<lyr2)) continue;
						mapscr* scr_2 = lyr2 == 0 ? base_scr : get_scr_layer_valid(screen, lyr2);
						if(!scr_2 || !scr_2->data[pos]) //Don't increment empty space
							continue;
						newcombo const& cmb_2 = combobuf[scr_2->data[pos]];
						if(lyr2 > lyr && (cmb_2.type == cCSWITCH || cmb_2.type == cCSWITCHBLOCK)
							&& (cmb_2.usrflags & cflag11) && (states[cmb_2.attribytes[0]]))
							continue; //This is a switch/block that will be hit later in the loop!
						set<int32_t> oldData2;
						//Increment the combo/cset by the original cmb's attributes
						oldData2.insert(scr_2->data[pos]);
						scr_2->data[pos] = BOUND_COMBO(scr_2->data[pos] + cmbofs);
						scr_2->cset[pos] = (scr_2->cset[pos] + csofs) & 15;
						if(entry && (cmb.usrflags&cflag8)) //Skip cycling on screen entry
						{
							newcombo const* tmp = &combobuf[scr_2->data[pos]];
							while(tmp->can_cycle())
							{
								bool cycle_under = (tmp->animflags & AF_CYCLEUNDERCOMBO);
								auto cid = cycle_under ? scr_2->undercombo : tmp->nextcombo;
								if(oldData2.find(cid) != oldData2.end())
									break;
								scr_2->data[pos] = cid;
								if(!(tmp->animflags & AF_CYCLENOCSET))
									scr_2->cset[pos] = cycle_under ? scr_2->undercset : tmp->nextcset;
								oldData2.insert(cid);
								tmp = &combobuf[cid];
							}
						}
						int32_t cmbid2 = scr_2->data[pos];
						if(combobuf[cmbid2].animflags & AF_CYCLE)
						{
							combobuf[cmbid2].tile = combobuf[cmbid2].o_tile;
							combobuf[cmbid2].cur_frame=0;
							combobuf[cmbid2].aclk = 0;
							combo_caches::drawing.refresh(cmbid2);
						}
						togglegrid[pos] |= (1<<lyr2); //Mark this pos toggled for this layer
					}
				}
			}
		}
	}
	
	if(get_qr(qr_SWITCHES_AFFECT_MOVINGBLOCKS) && mblock2.clk)
	{
		newcombo const& cmb = combobuf[mblock2.bcombo];
		if((cmb.type == cCSWITCH || cmb.type == cCSWITCHBLOCK) && (cmb.usrflags & cflag11))
		{
			if(states[cmb.attribytes[0]])
			{
				//Increment the combo/cset by the attributes
				int32_t cmbofs = (cmb.attributes[0]/10000L);
				int32_t csofs = (cmb.attributes[1]/10000L);
				mblock2.bcombo = BOUND_COMBO(mblock2.bcombo + cmbofs);
				mblock2.cs = (mblock2.cs + csofs) & 15;
				int32_t cmbid = mblock2.bcombo;
				if(combobuf[cmbid].animflags & AF_CYCLE)
				{
					combobuf[cmbid].tile = combobuf[cmbid].o_tile;
					combobuf[cmbid].cur_frame=0;
					combobuf[cmbid].aclk = 0;
					combo_caches::drawing.refresh(cmbid);
				}
			}
		}
	}
	
	if(is_active_screen)
	{
		int screen_index_offset = get_region_screen_offset(screen);
		word c = base_scr->numFFC();
		for (int q = 0; q < c; ++q)
		{
			auto ffc_handle = *base_scr->getFFCHandle(q, screen_index_offset);
			int cid = ffc_handle.data();
			// auto& mini_cmb = combo_cache.minis[cid];
			// if (mini_cmb.trigger_global_state)
			auto& cmb = combobuf[cid];
			for(size_t idx = 0; idx < cmb.triggers.size(); ++idx)
			{
				auto& trig = cmb.triggers[idx];
				if (states[trig.trig_gstate])
					do_trigger_combo(ffc_handle, idx, ctrigSWITCHSTATE);
				if(ffc_handle.data() != cid) break;
			}
		}
	}
}
void toggle_gswitches_load(const screen_handles_t& screen_handles)
{
	bool states[256];
	for(auto q = 0; q < 256; ++q)
	{
		states[q] = game->gswitch_timers[q] != 0;
	}
	toggle_gswitches(states, true, screen_handles);
}
void run_gswitch_timers()
{
	bool states[256] = {false};
	auto& m = game->gswitch_timers.mut_inner();
	for(auto it = m.begin(); it != m.end();)
	{
		if(it->second > 0)
			if(!--it->second)
			{
				states[it->first] = true;
				it = m.erase(it);
				continue;
			}
		++it;
	}
	for_every_base_screen_in_region([&](mapscr* scr, unsigned int region_scr_x, unsigned int region_scr_y) {
		toggle_gswitches(states, false, create_screen_handles(scr));
	});
}
void onload_gswitch_timers() //Reset all timers that were counting down, no trigger necessary
{
	for(auto q = 0; q < 256; ++q)
	{
		if(game->gswitch_timers[q] > 0)
			game->gswitch_timers[q] = 0;
	}
}

/****  View Map  ****/

int32_t mapres = 0;
int32_t view_map_show_mode = 3;

bool displayOnMap(int32_t x, int32_t y)
{
    int32_t s = (y<<4) + x;
	int mi = mapind(cur_map, s);
    if (!(game->maps[mi]&mVISITED))
        return false;

    // Don't display if not part of DMap
    if(((DMaps[cur_dmap].flags&dmfDMAPMAP) &&
       (DMaps[cur_dmap].type != dmOVERW) &&
       !(x >= DMaps[cur_dmap].xoff &&
         x < DMaps[cur_dmap].xoff+8 &&
         DMaps[cur_dmap].grid[y]&(128>>(x-DMaps[cur_dmap].xoff)))))
        return false;
    else
        return true;
}

void ViewMap()
{
	ViewingMap = true;

	BITMAP* mappic = NULL;
	static double scales[17] =
	{
		0.03125, 0.04419, 0.0625, 0.08839, 0.125, 0.177, 0.25, 0.3535,
		0.50, 0.707, 1.0, 1.414, 2.0, 2.828, 4.0, 5.657, 8.0
	};
	
	int32_t px = ((8-(cur_screen&15)) << 9)  - 256;
	int32_t py = ((4-(cur_screen>>4)) * 352) - 176;
	int32_t lx = ((cur_screen&15)<<8)  + HeroX()+8;
	int32_t ly = ((cur_screen>>4)*176) + HeroY()+8;
	int32_t sc = 6;
	
	bool done=false, redraw=true;
	
	mappic = create_bitmap_ex(8,(256*16)>>mapres,(176*8)>>mapres);
	
	if(!mappic)
	{
		enter_sys_pal();
		jwin_alert("View Map","Not enough memory.",NULL,NULL,"OK",NULL,13,27,get_zc_font(font_lfont));
		exit_sys_pal();
		return;
	}

	clear_to_color(mappic, WHITE);

	auto prev_viewport = viewport;
	viewport.x = 0;
	viewport.y = 0;

	// draw the map
	BITMAP* screen_bmp = create_bitmap_ex(8, 256, 176);
	combotile_add_x = 256;
	combotile_add_y = 0;
	for(int32_t y=0; y<8; y++)
	{
		for(int32_t x=0; x<16; x++)
		{
			if (!displayOnMap(x, y))
				continue;

			int screen = map_scr_xy_to_index(x, y);
			auto scrs = loadscr2(screen);
			mapscr* scr = &scrs[0];
			if (!scr->is_valid())
				continue;

			screen_handles_t screen_handles;
			for (int i = 0; i <= 6; i++)
				screen_handles[i] = {scr, scrs[i].is_valid() ? &scrs[i] : nullptr, screen, i};

			int xx = 0;
			int yy = -playing_field_offset;
			
			if(XOR(scr->flags7&fLAYER2BG, DMaps[cur_dmap].flags&dmfLAYER2BG))
			{
				do_layer(screen_bmp, 0, screen_handles[2], xx, yy);
				do_ffc_layer(screen_bmp, -2, screen_handles[0], xx, yy);
			}
			
			if(XOR(scr->flags7&fLAYER3BG, DMaps[cur_dmap].flags&dmfLAYER3BG))
			{
				do_layer(screen_bmp, 0, screen_handles[3], xx, yy);
				do_ffc_layer(screen_bmp, -3, screen_handles[0], xx, yy);
			}
			
			if(lenscheck(scr,0)) putscr(scr, screen_bmp, 0, 0);
			do_ffc_layer(screen_bmp, 0, screen_handles[0], xx, yy);
			do_layer(screen_bmp, 0, screen_handles[1], xx, yy);
			do_ffc_layer(screen_bmp, 1, screen_handles[0], xx, yy);
			
			if(!XOR((scr->flags7&fLAYER2BG), DMaps[cur_dmap].flags&dmfLAYER2BG))
			{
				do_layer(screen_bmp, 0, screen_handles[2], xx, yy);
				do_ffc_layer(screen_bmp, 2, screen_handles[0], xx, yy);
			}
			
			putscrdoors(scr, screen_bmp, xx, yy);
			if (get_qr(qr_PUSHBLOCK_SPRITE_LAYER))
			{
				do_layer(screen_bmp, -2, screen_handles[0], xx, yy);
				if(get_qr(qr_PUSHBLOCK_LAYER_1_2))
				{
					do_layer(screen_bmp,-2, screen_handles[1], xx, yy);
					do_layer(screen_bmp,-2, screen_handles[2], xx, yy);
				}
			}
			
			if(!XOR((scr->flags7&fLAYER3BG), DMaps[cur_dmap].flags&dmfLAYER3BG))
			{
				do_layer(screen_bmp, 0, screen_handles[3], xx, yy);
				do_ffc_layer(screen_bmp, 3, screen_handles[0], xx, yy);
			}

			do_layer(screen_bmp, 0, screen_handles[4], xx, yy);
			do_ffc_layer(screen_bmp, 4, screen_handles[0], xx, yy);
			do_layer(screen_bmp, -1, screen_handles[0], xx, yy);
			if(get_qr(qr_OVERHEAD_COMBOS_L1_L2))
			{
				do_layer(screen_bmp,-1, screen_handles[1], xx, yy);
				do_layer(screen_bmp,-1, screen_handles[2], xx, yy);
			}
			do_layer(screen_bmp, 0, screen_handles[5], xx, yy);
			do_ffc_layer(screen_bmp, 5, screen_handles[0], xx, yy);
			if(replay_version_check(40))
				do_ffc_layer(screen_bmp, -1, screen_handles[0], xx, yy);
			do_layer(screen_bmp, 0, screen_handles[6], xx, yy);
			do_ffc_layer(screen_bmp, 6, screen_handles[0], xx, yy);
			do_ffc_layer(screen_bmp, 7, screen_handles[0], xx, yy);
			
			stretch_blit(screen_bmp, mappic, 0, 0, 256, 176, x<<(8-mapres), (y*176)>>mapres, 256>>mapres, 176>>mapres);
		}
	}

	viewport = prev_viewport;
	combotile_add_x = 0;
	combotile_add_y = 0;

	destroy_bitmap(screen_bmp);
	clear_keybuf();
	pause_all_sfx();
	
	// view it
	int32_t delay = 0;
	
	do
	{
		if (replay_version_check(0, 11))
			load_control_state();

		int32_t step = int32_t(16.0/scales[sc]);
		step = (step>>1) + (step&1);
		bool r = cRbtn();
		
		if(cLbtn())
		{
			step <<= 2;
			delay = 0;
		}
		
		if(r)
		{
			if(rUp())
			{
				py+=step;
				redraw=true;
			}
			
			if(rDown())
			{
				py-=step;
				redraw=true;
			}
			
			if(rLeft())
			{
				px+=step;
				redraw=true;
			}
			
			if(rRight())
			{
				px-=step;
				redraw=true;
			}
		}
		else
		{
			if(Up())
			{
				py+=step;
				redraw=true;
			}
			
			if(Down())
			{
				py-=step;
				redraw=true;
			}
			
			if(Left())
			{
				px+=step;
				redraw=true;
			}
			
			if(Right())
			{
				px-=step;
				redraw=true;
			}
		}
		
		if(delay)
			--delay;
		else
		{
			bool a = cAbtn();
			bool b = cBbtn();
			
			if(a && !b)
			{
				sc=zc_min(sc+1,16);
				delay=8;
				redraw=true;
			}
			
			if(b && !a)
			{
				sc=zc_max(sc-1,0);
				delay=8;
				redraw=true;
			}
		}
		
		if(rPbtn())
			--view_map_show_mode;
			
		px = vbound(px,-4096,4096);
		py = vbound(py,-1408,1408);
		
		double scale = scales[sc];
		
		if(!redraw)
		{
			blit(scrollbuf_old,framebuf,256,0,0,0,256,232);
		}
		else
		{
			clear_to_color(framebuf,BLACK);
			stretch_blit(mappic,framebuf,0,0,mappic->w,mappic->h,
						 int32_t(256+(int64_t(px)-mappic->w)*scale)/2,int32_t(224+(int64_t(py)-mappic->h)*scale)/2,
						 int32_t(mappic->w*scale),int32_t(mappic->h*scale));
						 
			blit(framebuf,scrollbuf_old,0,0,256,0,256,232);
			redraw=false;
		}
		
		int32_t x = int32_t(256+(px-((2048-int64_t(lx))*2))*scale)/2;
		int32_t y = int32_t(224+(py-((704-int64_t(ly))*2))*scale)/2;
		
		if(view_map_show_mode&1)
		{
			line(framebuf,x-7,y-7,x+7,y+7,(frame&3)+252);
			line(framebuf,x+7,y-7,x-7,y+7,(frame&3)+252);
		}
		
		if(view_map_show_mode&2 || r)
			textprintf_ex(framebuf,font,224,216,WHITE,BLACK,"%1.2f",scale);
			
		if(r)
		{
			textprintf_ex(framebuf,font,0,208,WHITE,BLACK,"m: %d %d",px,py);
			textprintf_ex(framebuf,font,0,216,WHITE,BLACK,"x: %d %d",x,y);
		}
		
		advanceframe(false, false);
		if (replay_version_check(11))
			load_control_state();
		
		if(getInput(btnS, true, false, true)) //rSbtn
			done = true;
			
	}
	while(!done && !Quit);

	ViewingMap = false;
	destroy_bitmap(mappic);
	resume_all_sfx();
}

int32_t onViewMap()
{
    if(Playing && cur_screen<128 && DMaps[cur_dmap].flags&dmfVIEWMAP)
    {
        clear_to_color(framebuf,BLACK);
        textout_centre_ex(framebuf,font,"Drawing map...",128,108,WHITE,BLACK);
        advanceframe(true);
        ViewMap();
    }
    
    return D_O_K;
}

bool isGrassType(int32_t type)
{
    switch(type)
    {
    case cTALLGRASS:
    case cTALLGRASSNEXT:
    case cTALLGRASSTOUCHY:
        return true;
    }
    
    return false;
}

bool isFlowersType(int32_t type)
{
    switch(type)
    {
    case cFLOWERS:
    case cFLOWERSTOUCHY:
        return true;
    }
    
    return false;
}

bool isGenericType(int32_t type)
{
    switch(type)
    {
    case cTRIGGERGENERIC:
        return true;
    }
    
    return false;
}

bool isBushType(int32_t type)
{
    switch(type)
    {
    case cBUSH:
    case cBUSHNEXT:
    case cBUSHTOUCHY:
    case cBUSHNEXTTOUCHY:

        return true;
    }
    
    return false;
}

bool isSlashType(int32_t type)
{
    switch(type)
    {
    case cSLASH:
    case cSLASHITEM:
    case cSLASHTOUCHY:
    case cSLASHITEMTOUCHY:
    case cSLASHNEXT:
    case cSLASHNEXTITEM:
    case cSLASHNEXTTOUCHY:
    case cSLASHNEXTITEMTOUCHY:
        return true;
    }
    
    return false;
}

bool isCuttableNextType(int32_t type)
{
    switch(type)
    {
    case cSLASHNEXT:
    case cSLASHNEXTITEM:
    case cTALLGRASSNEXT:
    case cBUSHNEXT:
    case cSLASHNEXTTOUCHY:
    case cSLASHNEXTITEMTOUCHY:
    case cBUSHNEXTTOUCHY:
        return true;
    }
    
    return false;
}

bool isTouchyType(int32_t type)
{
    switch(type)
    {
    case cSLASHTOUCHY:
    case cSLASHITEMTOUCHY:
    case cBUSHTOUCHY:
    case cFLOWERSTOUCHY:
    case cTALLGRASSTOUCHY:
    case cSLASHNEXTTOUCHY:
    case cSLASHNEXTITEMTOUCHY:
    case cBUSHNEXTTOUCHY:
        return true;
    }
    
    return false;
}

bool isCuttableType(int32_t type)
{
	switch(type)
	{
		case cSLASH:
		case cSLASHITEM:
		case cBUSH:
		case cFLOWERS:
		case cTALLGRASS:
		case cTALLGRASSNEXT:
		case cSLASHNEXT:
		case cSLASHNEXTITEM:
		case cBUSHNEXT:
		
		case cSLASHTOUCHY:
		case cSLASHITEMTOUCHY:
		case cBUSHTOUCHY:
		case cFLOWERSTOUCHY:
		case cTALLGRASSTOUCHY:
		case cSLASHNEXTTOUCHY:
		case cSLASHNEXTITEMTOUCHY:
		case cBUSHNEXTTOUCHY:
			return true;
	}
	
	return false;
}

bool isCuttableItemType(int32_t type)
{
    switch(type)
    {
    case cSLASHITEM:
    case cBUSH:
    case cFLOWERS:
    case cTALLGRASS:
    case cTALLGRASSNEXT:
    case cSLASHNEXTITEM:
    case cBUSHNEXT:
    
    case cSLASHITEMTOUCHY:
    case cBUSHTOUCHY:
    case cFLOWERSTOUCHY:
    case cTALLGRASSTOUCHY:
    case cSLASHNEXTITEMTOUCHY:
    case cBUSHNEXTTOUCHY:
        return true;
    }
    
    return false;
}

bool is_push(mapscr* m, int32_t pos)
{
	if(is_push_flag(m->sflag[pos]))
		return true;
	newcombo const& cmb = combobuf[m->data[pos]];
	if(is_push_flag(cmb.flag))
		return true;
	if(cmb.type == cPUSHBLOCK)
		return true;
	if(cmb.type == cSWITCHHOOK && (cmb.usrflags&cflag7))
		return true; //Counts as 'pushblock' flag
	return false;
}

static std::map<int, screen_state_t> screen_states;

std::map<int, screen_state_t>& get_screen_states()
{
	return screen_states;
}

screen_state_t& get_screen_state(int screen)
{
	return screen_states[screen];
}

void clear_screen_states()
{
	screen_states.clear();
}

void screen_item_set_state(int screen, ScreenItemState state)
{
	get_screen_state(screen).item_state = state;
}

void mark_visited(int screen)
{
	if (screen < 0x80)
	{
		if(DMaps[cur_dmap].flags&dmfVIEWMAP)
			game->maps[mapind(cur_map, screen)] |= mVISITED;

		markBmap(-1, screen);
	}
}
