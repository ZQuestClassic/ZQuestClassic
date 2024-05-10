#include "base/handles.h"
#include "base/zdefs.h"
#include <cstring>
#include <assert.h>
#include <math.h>
#include <vector>
#include <deque>
#include <string>
#include <set>
#include <array>
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
extern FFScript FFCore;
#include "particles.h"
#include <fmt/format.h>
#include "zc/render.h"
#include "iter.h"

extern HeroClass Hero;

// TODO z3 ! checklist do all before starting beta
// screen secrets:
//    - trigger all secrets in region
//    - multiple triggers across many screens in a region (multi-block puzzle)
//    - perm secrets
// sword beams / ewpns should only despawn when leaving viewport
// ffc carryovers
// zscript docs

static std::map<int, std::vector<mapscr*>> temporary_screens;
static mapscr* temporary_screens_currmap[136*7];
static bool screen_in_current_region[176];
static rpos_handle_t current_region_rpos_handles[136*7];
static int current_region_screen_count;

viewport_t viewport;
ViewportMode viewport_mode;
int world_w, world_h;
int region_scr_dx, region_scr_dy;
int region_scr_count;
rpos_t region_max_rpos;
int region_num_rpos;
int scrolling_maze_scr, scrolling_maze_state;
int scrolling_maze_mode = 0;
region current_region, scrolling_region;

static int current_region_indices[128];

static int scr_xy_to_index(int x, int y)
{
	DCHECK(x >= 0 && x < 16 && y >= 0 && y < 8);
	return x + y*16;
}

static byte getNibble(byte byte, bool high)
{
    if (high) return byte >> 4 & 0xF;
    else      return byte & 0xF;
}

static bool is_a_region(int dmap, int scr)
{
	return get_region_id(dmap, scr) != 0;
}

static bool is_same_region_id(int region_origin_scr, int dmap, int scr)
{
	if (!is_a_region(dmap, scr)) return false;
	int region_id = get_region_id(dmap, region_origin_scr);
	return region_id && region_id == get_region_id(dmap, scr);
}

bool is_in_current_region(int scr)
{
	// Set by z3_load_region.
	return screen_in_current_region[scr];
}

bool is_valid_rpos(rpos_t rpos)
{
	return (int)rpos >= 0 && rpos <= region_max_rpos;
}

bool is_z3_scrolling_mode()
{
	return current_region.region_id && is_in_current_region(currscr);
}

bool is_extended_height_mode()
{
	return (DMaps[currdmap].flags & dmfEXTENDEDVIEWPORT) && current_region.screen_height > 1;
}

// Returns 0 if this is not a region.
int get_region_id(int dmap, int scr)
{
	if (scr >= 128) return 0;
	if (dmap == current_region.dmap) return current_region_indices[scr];

	int sx = scr % 16;
	int sy = scr / 16;
	return getNibble(DMaps[dmap].region_indices[sy][sx/2], sx % 2 == 0);
}

int get_current_region_id()
{
	return get_region_id(currdmap, cur_origin_screen_index);
}

void z3_calculate_region(int dmap, int screen, region& region, int& region_scr_dx, int& region_scr_dy)
{
	region.dmap = dmap;

	if (!(is_a_region(dmap, screen)) || screen >= 0x80)
	{
		region.region_id = 0;
		region.origin_screen_index = screen;
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
		if (!is_same_region_id(origin_scr, dmap, scr_xy_to_index(origin_scr_x - 1, origin_scr_y))) break;
		origin_scr_x--;
	}
	while (origin_scr_y > 0)
	{
		if (!is_same_region_id(origin_scr, dmap, scr_xy_to_index(origin_scr_x, origin_scr_y - 1))) break;
		origin_scr_y--;
	}
	origin_scr = scr_xy_to_index(origin_scr_x, origin_scr_y);
	
	// Now find the bottom-right corner.
	int region_scr_right = origin_scr_x;
	while (region_scr_right < 15)
	{
		if (!is_same_region_id(origin_scr, dmap, scr_xy_to_index(region_scr_right + 1, origin_scr_y))) break;
		region_scr_right++;
	}
	int region_scr_bottom = origin_scr_y;
	while (region_scr_bottom < 7)
	{
		if (!is_same_region_id(origin_scr, dmap, scr_xy_to_index(origin_scr_x, region_scr_bottom + 1))) break;
		region_scr_bottom++;
	}

	region.region_id = get_region_id(dmap, origin_scr);
	region.origin_screen_index = origin_scr;
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

void z3_load_region(int screen, int dmap)
{
	z3_clear_temporary_screens();

	if (dmap == -1) dmap = currdmap;

	for (int sy = 0; sy < 8; sy++)
	{
		for (int sx = 0; sx < 16; sx++)
		{
			int id = getNibble(DMaps[dmap].region_indices[sy][sx/2], sx % 2 == 0);
			int screen = scr_xy_to_index(sx, sy);
			if (id && (get_canonical_scr(DMaps[dmap].map, screen)->valid & mVALID))
				current_region_indices[screen] = id;
			else
				current_region_indices[screen] = 0;
		}
	}

	z3_calculate_region(dmap, screen, current_region, region_scr_dx, region_scr_dy);
	currscr = cur_origin_screen_index = current_region.origin_screen_index;
	world_w = current_region.width;
	world_h = current_region.height;
	region_scr_count = current_region.screen_count;
	region_max_rpos = (rpos_t)(current_region.screen_width*current_region.screen_height*176 - 1);
	region_num_rpos = current_region.screen_width*current_region.screen_height*176;
	scrolling_maze_state = 0;
	scrolling_maze_scr = 0;

	memset(screen_in_current_region, false, sizeof(screen_in_current_region));
	for (int x = 0; x < current_region.screen_width; x++)
	{
		for (int y = 0; y < current_region.screen_height; y++)
		{
			int scr = cur_origin_screen_index + x + y*16;
			screen_in_current_region[scr] = true;
		}
	}
}

std::tuple<const rpos_handle_t*, int> z3_get_current_region_handles()
{
	return {current_region_rpos_handles, current_region_screen_count};
}

void z3_clear_temporary_screens()
{
	for (auto screens : temporary_screens)
	{
		for (auto screen : screens.second)
		{
			free(screen);
		}
	}
	temporary_screens.clear();

	for (int i = 0; i < 136*7; i++)
	{
		if (temporary_screens_currmap[i])
		{
			free(temporary_screens_currmap[i]);
			temporary_screens_currmap[i] = NULL;
		}
	}
}

std::vector<mapscr*> z3_take_temporary_screens()
{
	std::vector<mapscr*> screens(temporary_screens_currmap, temporary_screens_currmap + 136*7);
	for (int i = 0; i < 136*7; i++)
	{
		temporary_screens_currmap[i] = nullptr;
	}

	// To make calling code simpler, let's copy the few screens that don't live
	// in the temporary screens array.
	for (int i = 0; i < 7; i++)
	{
		mapscr* s = get_layer_scr(currmap, currscr, i - 1);
		DCHECK(s);
		DCHECK(!screens[currscr*7 + i]);
		screens[currscr*7 + i] = new mapscr(*s);
	}

	return screens;
}

void z3_calculate_viewport(int dmap, int screen, int world_w, int world_h, int hero_x, int hero_y, viewport_t& viewport)
{
	bool extended_height_mode = (DMaps[dmap].flags & dmfEXTENDEDVIEWPORT) && world_h > 176;
	viewport.w = 256;
	// Note: the viewport height does not take into account that the bottom 8 pixels are not visible, for historical reasons.
	// For that to be the case a few things must change in hero.cpp scrollscr and red_shift.
	viewport.h = 176 + (extended_height_mode ? 56 : 0);

	if (viewport_mode == ViewportMode::Script)
	{
		return;
	}

	if (!is_a_region(dmap, screen))
	{
		viewport.x = 0;
		viewport.y = 0;
	}
	else if (viewport_mode == ViewportMode::CenterAndBound)
	{
		// Clamp the viewport to the edges of the region.
		viewport.x = CLAMP(0, world_w - viewport.w, hero_x - viewport.w/2);
		viewport.y = CLAMP(0, world_h - viewport.h, hero_y - viewport.h/2 + viewport.centering_y_offset + 16);
	}
	else if (viewport_mode == ViewportMode::Center)
	{
		viewport.x = hero_x - viewport.w/2;
		viewport.y = hero_y - viewport.h/2 + viewport.centering_y_offset + 16;
	}
}

void z3_update_viewport()
{
	z3_calculate_viewport(currdmap, cur_origin_screen_index, world_w, world_h, Hero.getX(), Hero.getY(), viewport);
}

void playLevelMusic();

void z3_update_heroscr()
{
	int x = vbound(Hero.getX().getInt(), 0, world_w - 1);
	int y = vbound(Hero.getY().getInt(), 0, world_h - 1);
	int dx = x / 256;
	int dy = y / 176;
	int newscr = cur_origin_screen_index + dx + dy * 16;
	if (heroscr != newscr && dx >= 0 && dy >= 0 && dx < 16 && dy < 8 && is_in_current_region(newscr))
	{
		region_scr_dx = dx;
		region_scr_dy = dy;
		heroscr = newscr;
		hero_screen = get_scr(currmap, heroscr);
		playLevelMusic();
	}
}

bool edge_of_region(direction dir)
{
	if (!is_z3_scrolling_mode()) return true;

	int scr_x = heroscr % 16;
	int scr_y = heroscr / 16;
	if (dir == up) scr_y -= 1;
	if (dir == down) scr_y += 1;
	if (dir == left) scr_x -= 1;
	if (dir == right) scr_x += 1;
	if (scr_x < 0 || scr_x > 16 || scr_y < 0 || scr_y > 8) return true;
	return !is_in_current_region(scr_xy_to_index(scr_x, scr_y));
}

// x, y are world coordinates (aka, in relation to origin screen at the top-left)
int get_screen_index_for_world_xy(int x, int y)
{
	if (!is_z3_scrolling_mode())
		return currscr;

	int dx = vbound(x, 0, world_w - 1) / 256;
	int dy = vbound(y, 0, world_h - 1) / 176;
	int origin_scr_x = cur_origin_screen_index % 16;
	int origin_scr_y = cur_origin_screen_index / 16;
	int scr_x = origin_scr_x + dx;
	int scr_y = origin_scr_y + dy;
	return scr_xy_to_index(scr_x, scr_y);
}

int get_screen_index_for_rpos(rpos_t rpos)
{
	int origin_scr_x = cur_origin_screen_index % 16;
	int origin_scr_y = cur_origin_screen_index / 16;
	int scr_index = static_cast<int32_t>(rpos) / 176;
	int scr_x = origin_scr_x + scr_index%current_region.screen_width;
	int scr_y = origin_scr_y + scr_index/current_region.screen_width;
	return scr_xy_to_index(scr_x, scr_y);
}

rpos_handle_t get_rpos_handle(rpos_t rpos, int layer)
{
	DCHECK_LAYER_ZERO_INDEX(layer);
	if (!is_z3_scrolling_mode())
		return {get_layer_scr(currmap, currscr, layer - 1), currscr, layer, rpos, RPOS_TO_POS(rpos)};
	int screen = get_screen_index_for_rpos(rpos);
	mapscr* scr = get_layer_scr(currmap, screen, layer - 1);
	return {scr, screen, layer, rpos, RPOS_TO_POS(rpos)};
}

rpos_handle_t get_rpos_handle_for_world_xy(int x, int y, int layer)
{
	DCHECK_LAYER_ZERO_INDEX(layer);
	if (!is_z3_scrolling_mode())
	{
		int pos = COMBOPOS(x, y);
		return {get_layer_scr(currmap, currscr, layer - 1), currscr, layer, (rpos_t)pos, pos};
	}
	return get_rpos_handle(COMBOPOS_REGION(x, y), layer);
}

// Return a pos_handle_t for a screen-specific `pos` (0-175).
rpos_handle_t get_rpos_handle_for_screen(int screen, int layer, int pos)
{
	DCHECK_LAYER_ZERO_INDEX(layer);
	return {get_layer_scr(currmap, screen, layer - 1), screen, layer, POS_TO_RPOS(pos, screen), pos};
}

// Return a pos_handle_t for a screen-specific `pos` (0-175).
// Use this instead of the other `get_pos_handle_for_screen` if you already have a reference to the screen.
rpos_handle_t get_rpos_handle_for_screen(mapscr* scr, int screen, int layer, int pos)
{
	DCHECK_LAYER_ZERO_INDEX(layer);
	return {scr, screen, layer, POS_TO_RPOS(pos, screen), pos};
}

void change_rpos_handle_layer(rpos_handle_t& rpos_handle, int layer)
{
	DCHECK_LAYER_ZERO_INDEX(layer);
	rpos_handle.layer = layer;
	rpos_handle.scr = get_layer_scr(currmap, rpos_handle.screen, layer - 1);
}

combined_handle_t get_combined_handle_for_world_xy(int x, int y, int layer)
{
	DCHECK_LAYER_ZERO_INDEX(layer);

	auto maybe_ffc_handle = getFFCAt(x, y);
	if (maybe_ffc_handle)
		return maybe_ffc_handle.value();

	auto rpos = COMBOPOS_REGION_CHECK_BOUNDS(x, y);
	if (rpos == rpos_t::None)
		return rpos_handle_t();
	return get_rpos_handle(rpos, layer);
}

// These functions all return _temporary_ screens. Any modifications made to them (either by the engine
// directly or via zscript) only last until the next area is loaded (via loadscr).

// Returns the screen containing the (x, y) world position.
mapscr* get_screen_for_world_xy(int x, int y)
{
	// Quick path, but should work the same without.
	if (!is_z3_scrolling_mode()) return tmpscr;
	return get_scr(currmap, get_screen_index_for_world_xy(x, y));
}

mapscr* get_screen_for_rpos(rpos_t rpos)
{
	// Quick path, but should work the same without.
	if (!is_z3_scrolling_mode()) return tmpscr;
	return get_scr(currmap, get_screen_index_for_rpos(rpos));
}

mapscr* get_screen_layer_for_rpos(rpos_t rpos, int layer)
{
	return get_layer_scr(currmap, get_screen_index_for_rpos(rpos), layer);
}

// Note: layer=0 is the base screen, 1 is the first layer, etc.
// TODO rename get_screen_layer_for_world_xy
// TODO z3 de-dupe this and get_layer_scr_for_xy
mapscr* get_screen_layer_for_xy_offset(int x, int y, int layer)
{
	DCHECK_LAYER_ZERO_INDEX(layer);
	if (!is_z3_scrolling_mode()) return FFCore.tempScreens[layer];
	return layer == 0 ?
		get_screen_for_world_xy(x, y) :
		get_layer_scr(currmap, get_screen_index_for_world_xy(x, y), layer - 1);
}

int z3_get_region_relative_dx(int screen)
{
	return z3_get_region_relative_dx(screen, cur_origin_screen_index);
}
int z3_get_region_relative_dx(int screen, int origin_screen_index)
{
	return screen % 16 - origin_screen_index % 16;
}

int z3_get_region_relative_dy(int screen)
{
	return z3_get_region_relative_dy(screen, cur_origin_screen_index);
}
int z3_get_region_relative_dy(int screen, int origin_screen_index)
{
	return screen / 16 - origin_screen_index / 16;
}

int get_region_screen_index_offset(int screen)
{
	return z3_get_region_relative_dx(screen) + z3_get_region_relative_dy(screen) * current_region.screen_width;
}

int get_screen_index_for_region_index_offset(int offset)
{
	int scr_dx = offset % current_region.screen_width;
	int scr_dy = offset / current_region.screen_width;
	int screen = cur_origin_screen_index + scr_dx + scr_dy*16;
	return screen;
}

mapscr* get_screen_for_region_index_offset(int offset)
{
	int screen = get_screen_index_for_region_index_offset(offset);
	return get_scr(currmap, screen);
}

const mapscr* get_canonical_scr(int map, int screen)
{
	return &TheMaps[map*MAPSCRS + screen];
}

mapscr* get_scr(int map, int screen)
{
	DCHECK_RANGE_INCLUSIVE(screen, 0, 135);
	if (screen == currscr && map == currmap) return tmpscr;
	if (screen == homescr && map == currmap) return &special_warp_return_screen;

	if (map == currmap)
	{
		int index = screen*7;
		if (!temporary_screens_currmap[index])
		{
			// Only needed during screen scrolling / dowarp
			load_a_screen_and_layers(currdmap, map, screen, -1);
		}
		return temporary_screens_currmap[index];
	}

	int index = map*MAPSCRS + screen;
	auto it = temporary_screens.find(index);
	if (it != temporary_screens.end()) return it->second[0];
	load_a_screen_and_layers(currdmap, map, screen, -1);
	return temporary_screens[index][0];
}

mapscr* get_scr_no_load(int map, int screen)
{
	DCHECK_RANGE_INCLUSIVE(screen, 0, 135);
	if (screen == currscr && map == currmap) return tmpscr;
	if (screen == homescr && map == currmap) return &special_warp_return_screen;

	if (map == currmap)
	{
		int index = screen*7;
		return temporary_screens_currmap[index];
	}

	int index = map*MAPSCRS + screen;
	auto it = temporary_screens.find(index);
	if (it != temporary_screens.end()) return it->second[0];

	return nullptr;
}

// Note: layer=-1 returns the base screen, layer=0 returns the first layer.
mapscr* get_layer_scr(int map, int screen, int layer)
{
	DCHECK_LAYER_NEG1_INDEX(layer);
	if (layer == -1) return get_scr(map, screen);
	if (screen == currscr && map == currmap) return &tmpscr2[layer];
	if (screen == homescr && map == currmap) return &tmpscr3[layer];

	if (map == currmap)
	{
		int index = screen*7;
		if (!temporary_screens_currmap[index])
		{
			// Only needed during screen scrolling / dowarp
			load_a_screen_and_layers(currdmap, map, screen, -1);
		}
		return temporary_screens_currmap[index+layer+1];
	}

	int index = map*MAPSCRS + screen;
	auto it = temporary_screens.find(index);
	if (it != temporary_screens.end()) return it->second[layer + 1];
	load_a_screen_and_layers(currdmap, map, screen, -1);
	return temporary_screens[index][layer + 1];
}

// Same as get_layer_scr, but if scrolling will pull from the scrolling screens as needed.
mapscr* get_layer_scr_allow_scrolling(int map, int screen, int layer)
{
	DCHECK_LAYER_NEG1_INDEX(layer);
	if (!screenscrolling || screen != scrolling_scr || FFCore.ScrollingScreensAll.empty())
		return get_layer_scr(map, screen, layer);

	return FFCore.ScrollingScreensAll[screen * 7 + layer + 1];
}

// Note: layer=-1 returns the base screen, layer=0 returns the first layer.
mapscr* get_layer_scr_for_xy(int x, int y, int layer)
{
	if (!is_z3_scrolling_mode())
		return get_layer_scr(currmap, currscr, layer);
	return get_layer_scr(currmap, get_screen_index_for_world_xy(x, y), layer);
}

ffc_handle_t get_ffc(int id)
{
	uint8_t screen = get_screen_index_for_region_index_offset(id / MAXFFCS);
	uint8_t i = id % MAXFFCS;
	mapscr* scr = get_scr(currmap, screen);
	ffcdata* ffc = &scr->ffcs[id % MAXFFCS];
	return {scr, screen, (uint16_t)id, i, ffc};
}

std::pair<int32_t, int32_t> translate_screen_coordinates_to_world(int screen, int x, int y)
{
	x += z3_get_region_relative_dx(screen) * 256;
	y += z3_get_region_relative_dy(screen) * 176;
	return {x, y};
}

std::pair<int32_t, int32_t> translate_screen_coordinates_to_world(int screen)
{
	int x = z3_get_region_relative_dx(screen) * 256;
	int y = z3_get_region_relative_dy(screen) * 176;
	return {x, y};
}

// You probably don't want to use these - use COMBOPOS_REGION instead.
// Only use these EXTENDED functions if you want the index value to go like this:
//     0 1 2 ... 14 15 (end screen 0x0) (start screen 1x0) 16 17 18 ...
int32_t COMBOPOS_REGION_EXTENDED(int32_t pos, int32_t scr_dx, int32_t scr_dy)
{
	int x = (pos%16) + scr_dx*16;
	int y = (pos/16) + scr_dy*11;
	int combos_wide = current_region.screen_width * 16;
	return x + y * combos_wide;
}
int32_t COMBOPOS_REGION_EXTENDED(int32_t x, int32_t y)
{
	int combos_wide = current_region.screen_width * 16;
	return x / 16 + y / 16 * combos_wide;
}
int32_t COMBOPOS_REGION_EXTENDED_B(int32_t x, int32_t y)
{
	if(unsigned(x) >= world_w || unsigned(y) >= world_h)
		return -1;
	int combos_wide = current_region.screen_width * 16;
	return x / 16 + y / 16 * combos_wide;
}
int32_t COMBOX_REGION_EXTENDED(int32_t pos)
{
	int combos_wide = current_region.screen_width * 16;
	return pos % combos_wide * 16;
}
int32_t COMBOY_REGION_EXTENDED(int32_t pos)
{
	int combos_wide = current_region.screen_width * 16;
	return pos / combos_wide * 16;
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
	return COMBOPOS(x,y);
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
	DCHECK(x >= 0 && x < world_w && y >= 0 && y < world_h);
	if (!is_z3_scrolling_mode())
		return (rpos_t) COMBOPOS(x, y);

	int scr_dx = x / (16*16);
	int scr_dy = y / (11*16);
	int pos = COMBOPOS(x%256, y%176);
	return static_cast<rpos_t>((scr_dx + scr_dy * current_region.screen_width)*176 + pos);
}
rpos_t COMBOPOS_REGION_CHECK_BOUNDS(int32_t x, int32_t y)
{
	if (x < 0 || y < 0 || x >= world_w || y >= world_h)
		return rpos_t::None;

	int scr_dx = x / (16*16);
	int scr_dy = y / (11*16);
	int pos = COMBOPOS(x%256, y%176);
	return static_cast<rpos_t>((scr_dx + scr_dy * current_region.screen_width)*176 + pos);
}
int32_t RPOS_TO_POS(rpos_t rpos)
{
	DCHECK(is_valid_rpos(rpos));
	return static_cast<int32_t>(rpos)%176;
}
rpos_t POS_TO_RPOS(int32_t pos, int32_t scr_dx, int32_t scr_dy)
{
	DCHECK(scr_dx >= 0 && scr_dy >= 0);
	DCHECK_RANGE_EXCLUSIVE(pos, 0, 176);
	return static_cast<rpos_t>((scr_dx + scr_dy * current_region.screen_width)*176 + pos);
}
rpos_t POS_TO_RPOS(int32_t pos, int32_t scr)
{
	DCHECK_RANGE_EXCLUSIVE(pos, 0, 176);
	return POS_TO_RPOS(pos, z3_get_region_relative_dx(scr), z3_get_region_relative_dy(scr));
}
std::pair<int32_t, int32_t> COMBOXY_REGION(rpos_t rpos)
{
	int scr_index = static_cast<int32_t>(rpos) / 176;
	int scr_dx = scr_index % current_region.screen_width;
	int scr_dy = scr_index / current_region.screen_width;
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

int32_t mapind(int32_t map, int32_t scr)
{
	return (map<<7)+scr;
}

FONT *get_zc_font(int index);

extern sprite_list  guys, items, Ewpns, Lwpns, chainlinks, decorations;
extern particle_list particles;
extern movingblock mblock2;                                 //mblock[4]?
extern portal mirror_portal;
bool triggered_screen_secrets=false;

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

int32_t isdungeon(int32_t dmap, int32_t scr) // The arg is only used by loadscr2 and loadscr
{
    if (scr < 0) scr = currscr;
    if (dmap < 0) dmap = currdmap;
    
    // dungeons can have any dlevel above 0
    if((DMaps[dmap].type&dmfTYPE) == dmDNGN)
    {
        if(TheMaps[(currmap*MAPSCRS)+scr].flags6&fCAVEROOM)
            return 0;
            
        return 1;
    }
    
    // dlevels that aren't dungeons are caves
    if(TheMaps[(currmap*MAPSCRS)+scr].flags6&fDUNGEONROOM)
        return 1;
        
    return 0;
}

bool canPermSecret(int32_t dmap, int32_t screen)
{
	return (!isdungeon(dmap, screen) || get_qr(qr_DUNGEON_DMAPS_PERM_SECRETS));
}

int32_t MAPCOMBO(int32_t x, int32_t y)
{
	x = vbound(x, 0, world_w-1);
	y = vbound(y, 0, world_h-1);
	int32_t combo = COMBOPOS(x%256, y%176);
	auto screen = get_screen_for_world_xy(x, y);
	return screen->data[combo];
}

int32_t MAPCOMBOzq(int32_t x,int32_t y)
{
	return MAPCOMBO(x,y);
}

//specific layers 1 to 6
int32_t MAPCOMBOL(int32_t layer,int32_t x,int32_t y)
{
	DCHECK(layer >= 1 && layer <= 6);
	if (x < 0 || x >= world_w || y < 0 || y >= world_h || layer <= 0)
	{
		return 0;
	}

	mapscr* m = get_layer_scr_for_xy(x, y, layer - 1);
	if (!m->valid)
	{
		return 0;
	}

	int32_t combo = COMBOPOS(x%256, y%176);
	return m->data[combo];
}

int32_t MAPCSETL(int32_t layer,int32_t x,int32_t y)
{
	DCHECK(layer >= 1 && layer <= 6);
	if (x < 0 || x >= world_w || y < 0 || y >= world_h)
		return 0;
    
	mapscr* m = get_layer_scr_for_xy(x, y, layer - 1);
    if(!m->valid) return 0;
    
    int32_t combo = COMBOPOS(x%256, y%176);
    return m->cset[combo];
}

int32_t MAPFLAGL(int32_t layer,int32_t x,int32_t y)
{
	DCHECK(layer >= 1 && layer <= 6);
	if (x < 0 || x >= world_w || y < 0 || y >= world_h)
		return 0;
    
	mapscr* m = get_layer_scr_for_xy(x, y, layer - 1);
    if(!m->valid) return 0;
    
    int32_t combo = COMBOPOS(x%256, y%176);
    return m->sflag[combo];
}

int32_t COMBOTYPEL(int32_t layer,int32_t x,int32_t y)
{
	DCHECK(layer >= 1 && layer <= 6);
	if (x < 0 || x >= world_w || y < 0 || y >= world_h)
		return 0;
	
	mapscr* m = get_layer_scr_for_xy(x, y, layer - 1);
    if (!layer || m->valid == 0) return 0;

	int32_t combo = COMBOPOS(x%256, y%176);
    return combobuf[m->data[combo]].type;
}

int32_t MAPCOMBOFLAGL(int32_t layer,int32_t x,int32_t y)
{
	DCHECK(layer >= 1 && layer <= 6);
	if (x < 0 || x >= world_w || y < 0 || y >= world_h)
		return 0;
	
	mapscr* m = get_layer_scr_for_xy(x, y, layer - 1);
    if (m->valid == 0) return 0;
	
    int32_t combo = COMBOPOS(x%256, y%176);
    return combobuf[m->data[combo]].flag;
}


// True if the FFC covers x, y and is not ethereal or a changer.
bool ffcIsAt(const ffc_handle_t& ffc_handle, int32_t x, int32_t y)
{
	if (ffc_handle.data()<=0)
        return false;

    if((ffc_handle.ffc->flags&(ffCHANGER|ffETHEREAL))!=0)
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
	auto ffc_handle = getFFCAt(x,y);
	if (ffc_handle)
		return ffc_handle->data();
    return 0;
}

int32_t MAPCSET(int32_t x, int32_t y)
{
	if (x < 0 || x >= world_w || y < 0 || y >= world_h)
		return 0;
	mapscr* scr = get_screen_for_world_xy(x, y);
	int32_t combo = COMBOPOS(x%256, y%176);
	return scr->cset[combo];
}

int32_t MAPFLAG(int32_t x, int32_t y)
{
	if (x < 0 || x >= world_w || y < 0 || y >= world_h)
		return 0;
	mapscr* scr = get_screen_for_world_xy(x, y);
	int32_t combo = COMBOPOS(x%256, y%176);
	return scr->sflag[combo];
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
	auto ffc_handle = getFFCAt(x, y);
	if (ffc_handle)
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
	auto ffc_handle = getFFCAt(x,y);
	if (ffc_handle)
		return ffc_handle->data();
	
	return layer ? MAPCOMBOL(layer, x, y) : MAPCOMBO(x,y);
}

int32_t FFORCOMBOTYPE_L(int32_t layer, int32_t x, int32_t y)
{
	return combobuf[FFORCOMBO_L(layer,x,y)].type;
}

int32_t MAPCOMBOFLAG(int32_t x,int32_t y)
{
	if (x < 0 || x >= world_w || y < 0 || y >= world_h)
		return 0;
	mapscr* scr = get_screen_for_world_xy(x, y);
	int32_t combo = COMBOPOS(x%256, y%176);
	return combobuf[scr->data[combo]].flag;
}

int32_t MAPFFCOMBOFLAG(int32_t x,int32_t y)
{
	auto ffc_handle = getFFCAt(x, y);
	if (ffc_handle)
		return combobuf[ffc_handle->data()].flag;
    return 0;
}

std::optional<ffc_handle_t> getFFCAt(int32_t x, int32_t y)
{
	return find_ffc([&](const ffc_handle_t& ffc_handle) {
		return ffcIsAt(ffc_handle, x, y);
	});
}

// ffc_handle_t get_ffc_handle(uint16_t ffc_id)
// {
// 	uint8_t i = ffc_id % MAXFFCS;
// 	uint8_t screen_index_offset = ffc_id / MAXFFCS;
// 	uint8_t scr_dx = screen_index_offset % current_region.screen_width;
// 	uint8_t scr_dy = screen_index_offset / current_region.screen_width;
// 	uint8_t screen = cur_origin_screen_index + scr_dx + scr_dy*16;
// 	mapscr* scr = get_scr(currmap, screen);
// 	return {scr, screen, ffc_id, i, &scr->ffcs[i]};
// }

int32_t MAPCOMBO(const rpos_handle_t& rpos_handle)
{
	if (!rpos_handle.scr->valid) return 0;
	return rpos_handle.data();
}

int32_t MAPCOMBO2(int32_t layer, int32_t x, int32_t y)
{
	DCHECK_LAYER_NEG1_INDEX(layer);
	if (x < 0 || y < 0 || x >= world_w || y >= world_h) return 0;
    if (layer == -1) return MAPCOMBO(x, y);
    
	auto rpos_handle = get_rpos_handle_for_world_xy(x, y, layer + 1);
	if (!rpos_handle.scr->valid) return 0;

	return rpos_handle.data();
}

static void apply_state_changes_to_screen(mapscr& scr, int32_t map, int32_t screen, int32_t flags)
{
	if ((flags & mSECRET) && canPermSecret(currdmap, screen))
	{
		reveal_hidden_stairs(&scr, screen, false);
		bool do_layers = false;
		trigger_secrets_for_screen_internal(-1, &scr, do_layers, false, -3);
	}
	if(flags & mLIGHTBEAM)
	{
		for(size_t pos = 0; pos < 176; ++pos)
		{
			newcombo const* cmb = &combobuf[scr.data[pos]];
			if(cmb->type == cLIGHTTARGET)
			{
				if(!(cmb->usrflags&cflag1)) //Unlit version
				{
					scr.data[pos] += 1;
				}
			}
		}
	}
	if(flags&mLOCKBLOCK)              // if special stuff done before
	{
	    remove_screenstatecombos2(&scr, screen, false, cLOCKBLOCK, cLOCKBLOCK2);
	}

	if(flags&mBOSSLOCKBLOCK)          // if special stuff done before
	{
	    remove_screenstatecombos2(&scr, screen, false, cBOSSLOCKBLOCK, cBOSSLOCKBLOCK2);
	}

	if(flags&mCHEST)              // if special stuff done before
	{
	    remove_screenstatecombos2(&scr, screen, false, cCHEST, cCHEST2);
	}

	if(flags&mCHEST)              // if special stuff done before
	{
	    remove_screenstatecombos2(&scr, screen, false, cLOCKEDCHEST, cLOCKEDCHEST2);
	}

	if(flags&mBOSSCHEST)              // if special stuff done before
	{
	    remove_screenstatecombos2(&scr, screen, false, cBOSSCHEST, cBOSSCHEST2);
	}

	int32_t mi = (map*MAPSCRSNORMAL)+screen;
	clear_xdoors_mi(&scr, screen, mi);
	clear_xstatecombos_mi(&scr, screen, mi);
}

static int32_t MAPCOMBO3_impl(int32_t map, int32_t screen, int32_t layer, int32_t pos, bool secrets)
{
	const mapscr *m = &TheMaps[(map*MAPSCRS)+screen];
	if(!m->valid) return 0;

	int32_t mi = (map*MAPSCRSNORMAL)+screen;
	int32_t flags = 0;

	if(secrets)
	{
		flags = game->maps[mi];
	}

	int32_t mapid = (layer < 0 ? -1 : ((m->layermap[layer] - 1) * MAPSCRS + m->layerscreen[layer]));

	if (layer >= 0 && (mapid < 0 || mapid > MAXMAPS*MAPSCRS)) return 0;

	// TODO: copying a mapscr and applying secrets is a lot of work, and this may be called a lot.
	// consider caching (and invalidate per-frame/or at least on loadscr).
	mapscr scr = ((mapid < 0 || mapid > MAXMAPS*MAPSCRS) ? *m : TheMaps[mapid]);
	if (scr.valid==0) return 0;

	apply_state_changes_to_screen(scr, map, screen, flags);

	return scr.data[pos];
}

// Read from the current temporary screens or, if (map, screen) is not loaded,
// load that screen and apply the relevant secrets before evaluating the combo at that position.
int32_t MAPCOMBO3(int32_t map, int32_t screen, int32_t layer, int32_t x, int32_t y, bool secrets)
{
	DCHECK_LAYER_NEG1_INDEX(layer);
	DCHECK(map >= 0 && screen >= 0);

	if (map == currmap && is_in_current_region(screen)) return MAPCOMBO2(layer, x, y);

	// Screen is not in the current region, so we have to load and trigger some secrets.
	int pos = COMBOPOS(x, y);
	return MAPCOMBO3_impl(map, screen, layer, pos, secrets);
}

int32_t MAPCOMBO3(int32_t map, int32_t screen, int32_t layer, rpos_t rpos, bool secrets)
{ 
	DCHECK_LAYER_NEG1_INDEX(layer);
	DCHECK(map >= 0 && screen >= 0);
	DCHECK(is_valid_rpos(rpos));
	
	if (map == currmap && is_in_current_region(screen)) return MAPCOMBO(get_rpos_handle(rpos, layer + 1));
	
	// Screen is not currently loaded, so we have to load and trigger some secrets.
	return MAPCOMBO3_impl(map, screen, layer, RPOS_TO_POS(rpos), secrets);
}

int32_t MAPCSET2(int32_t layer,int32_t x,int32_t y)
{
	DCHECK_LAYER_NEG1_INDEX(layer);
	if (x < 0 || x >= world_w || y < 0 || y >= world_h)
		return 0;
    if (layer == -1) return MAPCSET(x, y);

	auto rpos_handle = get_rpos_handle_for_world_xy(x, y, layer + 1);
	if (!rpos_handle.scr->valid) return 0;
	
	return rpos_handle.cset();
}

int32_t MAPFLAG2(int32_t layer,int32_t x,int32_t y)
{
	DCHECK_LAYER_NEG1_INDEX(layer);
	if (!get_qr(qr_BUGGED_LAYERED_FLAGS) && (x < 0 || x >= world_w || y < 0 || y >= world_h))
		return 0;
    if (layer == -1) return MAPFLAG(x, y);

	auto rpos_handle = get_rpos_handle_for_world_xy(x, y, layer + 1);
	if (!rpos_handle.scr->valid) return 0;

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
	if (!rpos_handle.scr->valid) return 0;

	return rpos_handle.combo().type;
}

// Returns the flag for the combo at the given position.
// This is also known as an "inherent flag".
int32_t MAPCOMBOFLAG2(int32_t layer,int32_t x,int32_t y)
{
	DCHECK_LAYER_NEG1_INDEX(layer);
	if (x < 0 || x >= world_w || y < 0 || y >= world_h)
		return 0;
    if (layer == -1) return MAPCOMBOFLAG(x, y);

	auto rpos_handle = get_rpos_handle_for_world_xy(x, y, layer + 1);
	if (!rpos_handle.scr->valid) return 0;

	return rpos_handle.cflag();
}

bool HASFLAG(int32_t flag, int32_t layer, rpos_t rpos)
{
	DCHECK_LAYER_ZERO_INDEX(layer);	
	auto rpos_handle = get_rpos_handle(rpos, layer);
	if (!rpos_handle.scr->valid) return false;
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
	
	int32_t mi = (currmap*MAPSCRSNORMAL)+homescr;
    word g = game->maps[mi] &0x3FFF;
    
	oss << fmt::format("Screen ({}, {:02X})", currmap+1, homescr);
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
				oss << fl;
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
						oss << " " << fl;
				oss << "]";
				comma = true;
			}
		}
	}
	Z_eventlog("%s\n", oss.str().c_str());
}

// set specific flag
void setmapflag(mapscr* scr, int32_t screen, int32_t flag)
{
	int mi = (currmap * MAPSCRSNORMAL) + (screen >= 0x80 ? homescr : screen);
	setmapflag_mi(scr, mi, flag);
}
void setmapflag(int32_t screen, int32_t flag)
{
	int mi = (currmap * MAPSCRSNORMAL) + (screen >= 0x80 ? homescr : screen);
	mapscr* scr = get_scr(currmap, screen >= 0x80 ? homescr : screen);
	setmapflag_mi(scr, mi, flag);
}
void setmapflag(int32_t flag)
{
    setmapflag_mi(tmpscr, (currmap*MAPSCRSNORMAL)+homescr, flag);
}
void setmapflag_mi(int32_t mi2, int32_t flag)
{
	setmapflag_mi(tmpscr, mi2, flag);
}
void setmapflag_mi(mapscr* scr, int32_t mi2, int32_t flag)
{
    byte cscr = mi2&((1<<7)-1);
    byte cmap = (mi2>>7);
    char buf[20];
    sprintf(buf,"Screen (%d, %02X)",cmap+1,cscr);
    
    float temp=log2((float)flag);
    if (replay_is_active() && !(game->maps[mi2] & flag))
        replay_step_comment(fmt::format("map {} scr {} flag {}", cmap, cscr, flag > 0 ? screenstate_string[(int32_t)temp] : "<Unknown>"));
    game->maps[mi2] |= flag;
    Z_eventlog("%s's State was set: %s\n",
               mi2 != (currmap*MAPSCRSNORMAL)+homescr ? buf : "Current screen",
               flag>0 ? screenstate_string[(int32_t)temp] : "<Unknown>");
               
    if(flag==mSECRET||flag==mITEM||flag==mSPECIALITEM||flag==mLOCKBLOCK||
            flag==mBOSSLOCKBLOCK||flag==mCHEST||flag==mBOSSCHEST||flag==mLOCKEDCHEST)
    {
        byte nmap=TheMaps[((cmap)*MAPSCRS)+cscr].nextmap;
        byte nscr=TheMaps[((cmap)*MAPSCRS)+cscr].nextscr;
        
        std::vector<int32_t> done;
        bool looped = (nmap==cmap+1 && nscr==cscr);
        
        while((nmap!=0) && !looped && !(nscr>=128))
        {
            // TODO z3 !!! correct? upstream.
            if((scr->nocarry&flag)!=flag && !(game->maps[((nmap-1)<<7)+nscr] & flag))
            {
                Z_eventlog("State change carried over to (%d, %02X)\n",nmap+1,nscr);
                if (replay_is_active())
                    replay_step_comment(fmt::format("map {} scr {} flag {} carry", nmap, nscr, flag > 0 ? screenstate_string[(int32_t)temp] : "<Unknown>"));
                game->maps[((nmap-1)<<7)+nscr] |= flag;
				if (flag == mSECRET && nmap-1 == currmap && is_in_current_region(nscr))
				{
					trigger_secrets_for_screen(TriggerSource::SecretsScreenState, nscr);
				}
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

void unsetmapflag(int32_t flag, bool anyflag)
{
    unsetmapflag((currmap*MAPSCRSNORMAL)+homescr,flag,anyflag);
}

void unsetmapflag(int32_t mi2, int32_t flag, bool anyflag)
{
    byte cscr = mi2&((1<<7)-1);
    byte cmap = (mi2>>7);

    if(anyflag)
        game->maps[mi2] &= ~flag;
    else if(flag==mITEM || flag==mSPECIALITEM)
    {
        if(!(get_scr(cmap, cscr)->flags4&fNOITEMRESET))
            game->maps[mi2] &= ~flag;
    }
    else game->maps[mi2] &= ~flag;
    
    char buf[20];
    sprintf(buf,"Screen (%d, %02X)",cmap+1,cscr);
    
    float temp=log2((float)flag);
    Z_eventlog("%s's State was unset: %s\n",
               mi2 != (currmap*MAPSCRSNORMAL)+homescr ? buf : "Current screen",
               flag>0 ? screenstate_string[(int32_t)temp] : "<Unknown>");
               
    if(flag==mSECRET||flag==mITEM||flag==mSPECIALITEM||flag==mLOCKBLOCK||
            flag==mBOSSLOCKBLOCK||flag==mCHEST||flag==mBOSSCHEST||flag==mLOCKEDCHEST)
    {
        byte nmap=TheMaps[((cmap)*MAPSCRS)+cscr].nextmap;
        byte nscr=TheMaps[((cmap)*MAPSCRS)+cscr].nextscr;
        
        std::vector<int32_t> done;
        bool looped = (nmap==cmap+1 && nscr==cscr);
        
        while((nmap!=0) && !looped && !(nscr>=128))
        {
            if((tmpscr->nocarry&flag)!=flag && (game->maps[((nmap-1)<<7)+nscr] & flag))
            {
                Z_eventlog("State change carried over to (%d, %02X)\n",nmap,nscr);
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

bool getmapflag(int32_t flag)
{
    return (game->maps[(currmap*MAPSCRSNORMAL)+homescr] & flag) != 0;
}
bool getmapflag(int32_t screen, int32_t flag)
{
	int mi = (currmap * MAPSCRSNORMAL) + (screen >= 0x80 ? homescr : screen);
    return (game->maps[mi] & flag) != 0;
}

void setxmapflag(int32_t screen, uint32_t flag)
{
	int mi = (currmap * MAPSCRSNORMAL) + (screen >= 0x80 ? homescr : screen);
	setxmapflag_mi(mi, flag);
}
void setxmapflag_mi(int32_t mi2, uint32_t flag)
{
	if(game->xstates[mi2] & flag) return;
    byte cscr = mi2&((1<<7)-1);
    byte cmap = (mi2>>7);
    char buf[20];
    sprintf(buf,"Screen (%d, %02X)",cmap+1,cscr);
    
    byte temp=(byte)log2((double)flag);
	Z_eventlog("%s's ExtraState was set: %d\n",
		mi2 != (currmap*MAPSCRSNORMAL)+homescr ? buf : "Current screen", temp);
	
	game->xstates[mi2] |= flag;
}
void unsetxmapflag(int32_t screen, uint32_t flag)
{
	int mi = (currmap * MAPSCRSNORMAL) + (screen >= 0x80 ? homescr : screen);
	unsetxmapflag_mi(mi, flag);
}
void unsetxmapflag_mi(int32_t mi2, uint32_t flag)
{
	if(!(game->xstates[mi2] & flag)) return;
    byte cscr = mi2&((1<<7)-1);
    byte cmap = (mi2>>7);
    char buf[20];
    sprintf(buf,"Screen (%d, %02X)",cmap+1,cscr);
    
    byte temp=(byte)log2((double)flag);
	Z_eventlog("%s's ExtraState was unset: %d\n",
		mi2 != (currmap*MAPSCRSNORMAL)+homescr ? buf : "Current screen", temp);
	
	game->xstates[mi2] &= ~flag;
}
bool getxmapflag(int32_t screen, uint32_t flag)
{
	int mi = (currmap * MAPSCRSNORMAL) + (screen >= 0x80 ? homescr : screen);
	return getxmapflag_mi(mi, flag);
}
bool getxmapflag_mi(int32_t mi2, uint32_t flag)
{
	return (game->xstates[mi2] & flag) != 0;
}

void setxdoor(uint mi, uint dir, uint ind, bool state)
{
	if(mi > game->xdoors.size() || dir > 3 || ind > 8)
		return;
	if(!(game->xdoors[mi][dir] & (1<<ind)) == !state)
		return;
	SETFLAG(game->xdoors[mi][dir], 1<<ind, state);
    int cscr = mi % MAPSCRSNORMAL;
    int cmap = mi / MAPSCRSNORMAL;
	bool iscurrscr = mi == ((currmap*MAPSCRSNORMAL)+homescr);
	Z_eventlog("%s's ExDoor[%s][%d] was %sset\n",
		iscurrscr ? "Current screen" : fmt::format("Screen ({}, {:02X})",cmap+1,cscr).c_str(),
		dirstr[dir], ind, state ? "" : "un");
}
void setxdoor(uint dir, uint ind, bool state)
{
	setxdoor((currmap*MAPSCRSNORMAL)+homescr,dir,ind,state);
}
bool getxdoor(uint mi, uint dir, uint ind)
{
	if(mi >= game->xdoors.size() || dir >= 4 || ind >= 8)
		return false;
	return (game->xdoors[mi][dir] & (1<<ind));
}
bool getxdoor(uint dir, uint ind)
{
	return getxdoor((currmap*MAPSCRSNORMAL)+homescr,dir,ind);
}

void set_doorstate_mi(uint mi, uint dir)
{
	if(dir >= 4)
		return;
	setmapflag_mi(mi, mDOOR_UP << dir);
	if(auto di = nextscr_mi(mi, dir, true))
		setmapflag_mi(*di, mDOOR_UP << oppositeDir[dir]);
}
void set_doorstate(uint screen, uint dir)
{
	int mi = (currmap*MAPSCRSNORMAL) + screen;
	set_doorstate_mi(mi, dir);
}
void set_doorstate(uint dir)
{
	set_doorstate_mi((currmap*MAPSCRSNORMAL) + currscr, dir);
}

void set_xdoorstate(uint mi, uint dir, uint ind)
{
	if(mi >= game->xdoors.size() || dir >= 4 || ind >= 8)
		return;
	setxdoor(mi, dir, ind, true);
	if(auto di = nextscr_mi(mi, dir, true))
		setxdoor(*di, oppositeDir[dir], ind);
}
void set_xdoorstate(uint dir, uint ind)
{
	set_xdoorstate((currmap*MAPSCRSNORMAL) + currscr, dir, ind);
}

int32_t WARPCODE(int32_t dmap,int32_t scr,int32_t dw)
// returns: -1 = not a warp screen
//          0+ = warp screen code ( high byte=dmap, low byte=scr )
{
    mapscr *s = &TheMaps[DMaps[dmap].map*MAPSCRS+scr];
    
    if(s->room!=rWARP)
        return -1;
        
    int32_t ring=s->catchall;
    int32_t size=QMisc.warp[ring].size;
    
    if(size==0)
        return -2;
        
    int32_t index=-1;
    
    for(int32_t i=0; i<size; i++)
        if(dmap==QMisc.warp[ring].dmap[i] && scr==
                (QMisc.warp[ring].scr[i] + DMaps[dmap].xoff))
            index=i;
            
    if(index==-1)
        return -3;
        
    index = (index+dw)%size;
    return (QMisc.warp[ring].dmap[index] << 8) + QMisc.warp[ring].scr[index];
}

void update_combo_cycling()
{
	static int32_t newdata[176];
	static int32_t newcset[176];
	static int32_t newdata2[176];
	static int32_t newcset2[176];
	static bool initialized=false;

	// Just a simple bit of optimization
	if(!initialized)
	{
		for(int32_t i=0; i<176; i++)
		{
			newdata[i]=-1;
			newcset[i]=-1;
			newdata2[i]=-1;
			newcset2[i]=-1;
		}
		
		initialized=true;
	}

	for_every_screen_in_region([&](mapscr* scr, int screen, unsigned int region_scr_x, unsigned int region_scr_y) {
		int32_t x;
		std::set<uint16_t> restartanim;
		std::set<uint16_t> restartanim2;
		
		for(int32_t i=0; i<176; i++)
		{
			x=scr->data[i];
			
			if(combobuf[x].animflags & AF_FRESH) continue;
			
			//time to restart
			if ((combobuf[x].aclk>=combobuf[x].speed) && combobuf[x].nextcombo!=0 && combocheck(combobuf[x]))
			{
				newdata[i]=combobuf[x].nextcombo;
				if(!(combobuf[x].animflags & AF_CYCLENOCSET))
					newcset[i]=combobuf[x].nextcset;
				int32_t c=newdata[i];
				
				if(combobuf[c].animflags & AF_CYCLE)
				{
					restartanim.insert(c);
				}
			}
		}
		
		for(int32_t i=0; i<176; i++)
		{
			x=scr->data[i];
			
			if(!(combobuf[x].animflags & AF_FRESH)) continue;
			
			//time to restart
			if ((combobuf[x].aclk>=combobuf[x].speed) && combobuf[x].nextcombo!=0 && combocheck(combobuf[x]))
			{
				newdata[i]=combobuf[x].nextcombo;
				if(!(combobuf[x].animflags & AF_CYCLENOCSET))
					newcset[i]=combobuf[x].nextcset;
				int32_t c=newdata[i];
				
				if(combobuf[c].animflags & AF_CYCLE)
				{
					restartanim2.insert(c);
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
			newcombo const& cmb = combobuf[ffc.data];
			
			bool fresh = cmb.animflags & AF_FRESH;
			
			//time to restart
			if ((cmb.aclk>=cmb.speed) && cmb.nextcombo!=0 && combocheck(cmb))
			{
				zc_ffc_set(ffc, cmb.nextcombo);
				if(!(cmb.animflags & AF_CYCLENOCSET))
					ffc.cset=cmb.nextcset;

				if(combobuf[ffc.data].animflags & AF_CYCLE)
				{
					auto& animset = fresh ? restartanim2 : restartanim;
					animset.insert(ffc.data);
				}
			}
		}
		
		if(get_qr(qr_CMBCYCLELAYERS))
		{
			for(int32_t j=0; j<6; j++)
			{
				mapscr* layer_scr = get_layer_scr(currmap, screen, j);

				for(int32_t i=0; i<176; i++)
				{
					x=layer_scr->data[i];
					
					if(combobuf[x].animflags & AF_FRESH) continue;
					
					//time to restart
					if ((combobuf[x].aclk>=combobuf[x].speed) && combobuf[x].nextcombo!=0 && combocheck(combobuf[x]))
					{
						newdata[i]=combobuf[x].nextcombo;
						if(!(combobuf[x].animflags & AF_CYCLENOCSET))
							newcset[i]=combobuf[x].nextcset;
						int32_t c=newdata[i];
						
						if(combobuf[c].animflags & AF_CYCLE)
						{
							restartanim.insert(c);
						}
					}
				}
				
				for(int32_t i=0; i<176; i++)
				{
					x=layer_scr->data[i];
					
					if(!(combobuf[x].animflags & AF_FRESH)) continue;
					
					//time to restart
					if ((combobuf[x].aclk>=combobuf[x].speed) && combobuf[x].nextcombo!=0 && combocheck(combobuf[x]))
					{
						newdata2[i]=combobuf[x].nextcombo;
						if(!(combobuf[x].animflags & AF_CYCLENOCSET))
							newcset2[i]=combobuf[x].nextcset;
						else newcset2[i]=layer_scr->cset[i];
						int32_t c=newdata2[i];
						int32_t cs=newcset2[i];
						
						if(combobuf[c].animflags & AF_CYCLE)
						{
							restartanim2.insert(c);
						}
						
						if(combobuf[c].type==cSPINTILE1)
						{
							// Uses animated_combo_table2
							rpos_t rpos = (rpos_t)(rpos_base+i);
							addenemy(screen, COMBOX_REGION(rpos),COMBOY_REGION(rpos),(cs<<12)+eSPINTILE1,combobuf[c].o_tile+zc_max(1,combobuf[c].frames));
						}
					}
				}
				
				for(int32_t i=0; i<176; i++)
				{
					if(newdata[i]!=-1)
					{
						rpos_t rpos = (rpos_t)(rpos_base + i);
						rpos_handle_t rpos_handle = {layer_scr, screen, j, rpos, i};
						screen_combo_modify_preroutine(rpos_handle);
						layer_scr->data[i]=newdata[i];
						if(newcset[i]>-1)
							layer_scr->cset[i]=newcset[i];
						screen_combo_modify_postroutine(rpos_handle);
						
						newdata[i]=-1;
						newcset[i]=-1;
					}
					
					if(newdata2[i]!=-1)
					{
						layer_scr->data[i]=newdata2[i];
						if(newcset2[i]>-1)
							layer_scr->cset[i]=newcset2[i];
						newdata2[i]=-1;
						newcset2[i]=-1;
					}
				}
			}
		}

		for (auto i : restartanim)
		{
			combobuf[i].tile = combobuf[i].o_tile;
			combobuf[i].cur_frame=0;
			combobuf[i].aclk = 0;
		}

		for (auto i : restartanim2)
		{
			combobuf[i].tile = combobuf[i].o_tile;
			combobuf[i].cur_frame=0;
			combobuf[i].aclk = 0;
		}
	});
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
	int screen = get_screen_index_for_world_xy(x, y);
	return iswaterex(combo, currmap, screen, layer, x, y, secrets, fullcheck, LayerCheck, ShallowCheck, hero);
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
						|| (hero && ((cmb.usrflags&cflag1) && !(itemsbuf[current_item_id(itype_flippers)].flags & ITEM_FLAG3)))))
					{
						if (!(ShallowCheck && (cmb.walk&(1<<b)) && (cmb.usrflags&cflag4))) return 0;
					}
				}

				auto found_ffc_not_water = find_ffc([&](const ffc_handle_t& ffc_handle) {
					if (ffcIsAt(ffc_handle, tx2, ty2))
					{
						auto ty = ffc_handle.combo().type;
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
							auto ty = ffc_handle.combo().type;
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
	if(tmpscr2[1].valid!=0)
	{
		if (get_qr(qr_OLD_BRIDGE_COMBOS))
		{
			if (combobuf[MAPCOMBO2(1,x,y)].type == cBRIDGE && !_walkflag_layer(x,y,1, &(tmpscr2[1]))) return 0;
		}
		else
		{
			if (combobuf[MAPCOMBO2(1,x,y)].type == cBRIDGE && _effectflag_layer(x,y,1, &(tmpscr2[1]))) return 0;
		}
	}
	c = MAPCOMBOL(1,x,y);
	if(check_icy(combobuf[c], type)) return c;
	if(tmpscr2[0].valid!=0)
	{
		if (get_qr(qr_OLD_BRIDGE_COMBOS))
		{
			if (combobuf[MAPCOMBO2(0,x,y)].type == cBRIDGE && !_walkflag_layer(x,y,1, &(tmpscr2[0]))) return 0;
		}
		else
		{
			if (combobuf[MAPCOMBO2(0,x,y)].type == cBRIDGE && _effectflag_layer(x,y,1, &(tmpscr2[0]))) return 0;
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
	if (rpos_handle.scr->valid)
		if (rpos_handle.sflag() == flag || rpos_handle.cflag() == flag)
			return true;
	
	change_rpos_handle_layer(rpos_handle, 2);
	if (rpos_handle.scr->valid)
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
			return (ldrid > -1 && itemsbuf[ldrid].flags & ITEM_FLAG1);
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
        int32_t di = COMBOPOS(s->stairx,s->stairy);
        s->data[di] = s->secretcombo[sSTAIRS];
        s->cset[di] = s->secretcset[sSTAIRS];
        s->sflag[di] = s->secretflag[sSTAIRS];
        
        if (redraw)
		{
			auto [x, y] = translate_screen_coordinates_to_world(screen, s->stairx, s->stairy);
            putcombo(scrollbuf,x,y,s->data[di],s->cset[di]);
		}
            
        return true;
    }
    
    return false;
}

bool remove_screenstatecombos2(mapscr *s, int32_t screen, bool do_layers, int32_t what1, int32_t what2)
{
	if (screen >= 128) s = &special_warp_return_screen;
	DCHECK(s);
	
	bool didit=false;
	
	for(int32_t i=0; i<176; i++)
	{
		newcombo const& cmb = combobuf[s->data[i]];
		if(cmb.usrflags&cflag16) continue; //custom state instead of normal state
		if((cmb.type == what1) || (cmb.type== what2))
		{
			s->data[i]++;
			didit=true;
		}
	}
	
	if (do_layers)
	{
		for(int32_t j=0; j<6; j++)
		{
			mapscr* layer_scr = get_layer_scr(currmap, screen, j);
			if(!layer_scr->valid) continue;
			
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
	
	if (!get_qr(qr_OLD_FFC_FUNCTIONALITY))
	{
		word c = tmpscr->numFFC();
		for(word i=0; i<c; i++)
		{
			ffcdata* ffc2 = &tmpscr->ffcs[i];
			newcombo const& cmb = combobuf[ffc2->data];
			if(cmb.usrflags&cflag16) continue; //custom state instead of normal state
			if((cmb.type== what1) || (cmb.type== what2))
			{
				zc_ffc_modify(*ffc2, 1);
				didit=true;
			}
		}
	}
	
	return didit;
}

bool remove_xstatecombos(mapscr *s, int32_t scr, byte xflag, bool triggers)
{
	int mi = (currmap * MAPSCRSNORMAL) + (scr >= 0x80 ? homescr : scr);
	return remove_xstatecombos_mi(s, scr, mi, xflag, triggers);
}
bool remove_xstatecombos_mi(mapscr *s, int32_t scr, int32_t mi, byte xflag, bool triggers)
{
	bool didit=false;
	if(!getxmapflag_mi(mi, 1<<xflag)) return false;

	if (scr >= 0x80) s = &special_warp_return_screen;
	scr = scr >= 0x80 ? homescr : scr;

	rpos_handle_t rpos_handle;
	rpos_handle.screen = scr;
	rpos_handle.layer = 0;
	for (int j = -1; j < 6; j++)
	{
		if (j != -1) s = get_layer_scr(currmap, scr, j);
		if (!s->valid) continue;

		rpos_handle.scr = s;
		rpos_handle.layer = j + 1;
		
		for (int32_t i=0; i<176; i++)
		{
			rpos_handle.rpos = POS_TO_RPOS(i, scr);
			rpos_handle.pos = i;
			newcombo const& cmb = rpos_handle.combo();
			if(triggers && force_ex_trigger(rpos_handle, xflag))
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
						rpos_handle.increment_data();
						didit=true;
					}
					break;
				}
			}
		}
	}

	if (!get_qr(qr_OLD_FFC_FUNCTIONALITY))
	{
		word c = s->numFFC();
		int screen_index_offset = get_region_screen_index_offset(scr);
		for (uint8_t i = 0; i < c; i++)
		{
			ffcdata* ffc2 = &s->ffcs[i];
			uint16_t ffc_id = screen_index_offset * MAXFFCS + i;
			newcombo const& cmb = combobuf[ffc2->data];
			if(triggers && force_ex_trigger_ffc({s, (uint8_t)scr, ffc_id, i, ffc2}, xflag))
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
						zc_ffc_modify(*ffc2, 1);
						didit=true;
					}
					break;
				}
			}
		}
	}
	
	return didit;
}

void clear_xstatecombos(mapscr *s, int32_t scr, bool triggers)
{
	int mi = (currmap*MAPSCRSNORMAL) + (scr >= 0x80 ? homescr : scr);
	clear_xstatecombos_mi(s, scr, mi, triggers);
}

void clear_xstatecombos_mi(mapscr *s, int32_t scr, int32_t mi, bool triggers)
{
	for (int q = 0; q < 32; ++q)
	{
		remove_xstatecombos_mi(s,scr,mi,q,triggers);
	}
}

bool remove_xdoors(mapscr *s, int32_t scr, uint dir, uint ind, bool triggers)
{
	int mi = (currmap * MAPSCRSNORMAL) + (scr >= 0x80 ? homescr : scr);
	return remove_xdoors_mi(s, scr, mi, dir, ind, triggers);
}
bool remove_xdoors_mi(mapscr *s, int32_t scr, int32_t mi, uint dir, uint ind, bool triggers)
{
	bool didit=false;
	if(!getxdoor(mi, dir, ind)) return false;

	if (scr >= 0x80) s = &special_warp_return_screen;
	scr = scr >= 0x80 ? homescr : scr;

	rpos_handle_t rpos_handle;
	rpos_handle.screen = scr;
	rpos_handle.layer = 0;
	for (int j = -1; j < 6; j++)
	{
		if (j != -1) s = get_layer_scr(currmap, scr, j);
		if (!s->valid) continue;

		rpos_handle.scr = s;
		rpos_handle.screen = scr;
		rpos_handle.layer = j + 1;
		
		for (int32_t i=0; i<176; i++)
		{
			rpos_handle.rpos = POS_TO_RPOS(i, scr);
			rpos_handle.pos = i;
			if(triggers && force_ex_door_trigger(rpos_handle, dir, ind))
				didit = true;
			else; //future door combo types?
		}
	}

	if (!get_qr(qr_OLD_FFC_FUNCTIONALITY))
	{
		word c = s->numFFC();
		int screen_index_offset = get_region_screen_index_offset(scr);
		for (uint8_t i = 0; i < c; i++)
		{
			ffcdata* ffc2 = &s->ffcs[i];
			uint16_t ffc_id = screen_index_offset * MAXFFCS + i;
			if(triggers && force_ex_door_trigger_ffc({s, (uint8_t)scr, ffc_id, i, ffc2}, dir, ind))
				didit = true;
			else; //future door combo types?
		}
	}
	
	return didit;
}

void clear_xdoors(mapscr *s, int32_t scr, bool triggers)
{
	int mi = (currmap*MAPSCRSNORMAL) + (scr >= 0x80 ? homescr : scr);
	clear_xdoors_mi(s, scr, mi, triggers);
}

void clear_xdoors_mi(mapscr *s, int32_t scr, int32_t mi, bool triggers)
{
	for (int q = 0; q < 32; ++q)
	{
		remove_xdoors(s,scr,mi,q,triggers);
	}
}

bool remove_lockblocks(mapscr* s, int32_t screen)
{
    return remove_screenstatecombos2(s, screen, true, cLOCKBLOCK, cLOCKBLOCK2);
}

bool remove_bosslockblocks(mapscr* s, int32_t screen)
{
    return remove_screenstatecombos2(s, screen, true, cBOSSLOCKBLOCK, cBOSSLOCKBLOCK2);
}

bool remove_chests(mapscr* s, int32_t screen)
{
    return remove_screenstatecombos2(s, screen, true, cCHEST, cCHEST2);
}

bool remove_lockedchests(mapscr* s, int32_t screen)
{
    return remove_screenstatecombos2(s, screen, true, cLOCKEDCHEST, cLOCKEDCHEST2);
}

bool remove_bosschests(mapscr* s, int32_t screen)
{
    return remove_screenstatecombos2(s, screen, true, cBOSSCHEST, cBOSSCHEST2);
}


bool overheadcombos(mapscr *s)
{
    for(int32_t i=0; i<176; i++)
    {
        if(combo_class_buf[combobuf[s->data[i]].type].overhead)
        {
            return true;
        }
    }
    
    return false;
}

void delete_fireball_shooter(const rpos_handle_t& rpos_handle)
{
    int32_t ct=rpos_handle.combo().type;
    
    if(ct!=cL_STATUE && ct!=cR_STATUE && ct!=cC_STATUE)
        return;
    
    auto [cx, cy] = COMBOXY_REGION(rpos_handle.rpos);
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
        if((int32_t(guys.spr(j)->x)==cx)&&(int32_t(guys.spr(j)->y)==cy)&&(guysbuf[(guys.spr(j)->id)&0xFFF].flags2 & eneflag_fire))
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
	for (int32_t j = 0; j < 7; j++)
	{
		screens[j] = get_layer_scr(currmap, screen, j - 1);
	}
    
	bool sflag = false;
    for(word j=0; j<176; j++)
    {
        for(int32_t layer = -1; layer < 6; ++layer)
		{
			mapscr* scr = screens[layer+1];
			if (layer>-1 && scr->valid==0) continue;

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
	if (source == Singular)
	{
		Z_eventlog("Restricted Screen Secrets triggered\n");
	}
	else
	{
		const char* source_str = "";
		switch (source)
		{
			case Singular: break;
			case Unspecified: source_str = "by unspecified means"; break;
			case EnemiesScreenFlag: source_str = "the 'Enemies->Secret' screen flag"; break;
			case SecretsScreenState: source_str = "the 'Secrets' screen state"; break;
			case Script: source_str = "a script"; break;
			case ItemsSecret: source_str = "Items->Secret"; break;
			case GenericCombo: source_str = "Generic Combo"; break;
			case LightTrigger: source_str = "Light Triggers"; break;
			case SCC: source_str = "by SCC"; break;
			case CheatTemp: source_str = "by Cheat (Temp)"; break;
			case CheatPerm: source_str = "by Cheat (Perm)"; break;
		}
		Z_eventlog("Screen Secrets triggered by %s\n", source_str);
	}
}

// single:
// >-1 : the singular triggering combo
// -1: triggered by some other cause
void trigger_secrets_for_screen(TriggerSource source, int32_t screen, bool high16only, int32_t single)
{
	log_trigger_secret_reason(source);
	if (single < 0)
		triggered_screen_secrets = true;
	bool do_combo_triggers = true;
	trigger_secrets_for_screen_internal(screen, NULL, do_combo_triggers, high16only, single);
}

void trigger_secrets_for_screen(TriggerSource source, int32_t screen, mapscr *s, bool high16only, int32_t single)
{
	log_trigger_secret_reason(source);
	if (single < 0)
		triggered_screen_secrets = true;
	bool do_combo_triggers = true;
	trigger_secrets_for_screen_internal(screen, s, do_combo_triggers, high16only, single);
}

void trigger_secrets_for_screen_internal(int32_t screen, mapscr *s, bool do_combo_triggers, bool high16only, int32_t single)
{
	DCHECK(screen != -1 || s);
	if (!s) s = get_scr(currmap, screen);
	if (screen == -1) screen = currscr;

	if (replay_is_active())
		replay_step_comment(fmt::format("trigger secrets scr={}", screen));

	if (do_combo_triggers)
	{
		for_every_rpos_in_screen(s, screen, [&](const rpos_handle_t& rpos_handle) {
			if (rpos_handle.combo().triggerflags[2] & combotriggerSECRETSTR)
				do_trigger_combo(rpos_handle, ctrigSECRETS);
		});
		if (!get_qr(qr_OLD_FFC_FUNCTIONALITY))
		{
			for_every_ffc_in_screen(s, screen, [&](const ffc_handle_t& ffc_handle) {
				if (ffc_handle.combo().triggerflags[2] & combotriggerSECRETSTR)
					do_trigger_combo_ffc(ffc_handle);
			});
		}
	}

	int32_t ft=0; //Flag trigger?
	int32_t msflag=0; // Misc. secret flag
	
	for(int32_t i=0; i<176; i++) //Do the 'trigger flags' (non 16-31)
	{
		if(single>=0 && i!=single) continue; //If it's got a singular flag and i isn't where the flag is
		
		// Remember the misc. secret flag; if triggered, use this instead
		if(s->sflag[i]>=mfSECRETS01 && s->sflag[i]<=mfSECRETS16)
			msflag=sSECRET01+(s->sflag[i]-mfSECRETS01);
		else if(combobuf[s->data[i]].flag>=mfSECRETS01 && combobuf[s->data[i]].flag<=mfSECRETS16)
			msflag=sSECRET01+(combobuf[s->data[i]].flag-mfSECRETS01);
		else
			msflag=0;
			
		if(!high16only || single>=0)
		{
			int32_t newflag = -1;
			
			for(int32_t iter=0; iter<2; ++iter)
			{
				int32_t checkflag=combobuf[s->data[i]].flag; //Inherent
				
				if(iter==1) checkflag=s->sflag[i]; //Placed
				
				ft = combo_trigger_flag_to_secret_combo_index(checkflag);
				if (ft != -1)  //Change the combos for the secret
				{
					// Use misc. secret flag instead if one is present
					if(msflag!=0)
						ft=msflag;
					
					auto rpos_handle = get_rpos_handle_for_screen(s, screen, 0, i);
					screen_combo_modify_preroutine(rpos_handle);
					if(ft==sSECNEXT)
					{
						s->data[i]++;
					}
					else
					{
						s->data[i] = s->secretcombo[ft];
						s->cset[i] = s->secretcset[ft];
					}
					newflag = s->secretflag[ft];
					screen_combo_modify_postroutine(rpos_handle);
				}
			}
			
			if(newflag >-1) s->sflag[i] = newflag; //Tiered secret
			
			if (do_combo_triggers)
			{
				for(int32_t j=0; j<6; j++)  //Layers
				{
					mapscr* layer_scr = get_layer_scr(currmap, screen, j);
					// TODO z3 maybe instead `get_layer_scr` return null if not valid?
					if (!layer_scr->valid) continue; //If layer isn't used
					
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
							
							if(combobuf[c].type==cSPINTILE1)  //Surely this means we can have spin tiles on layers 3+? Isn't that bad? ~Joe123
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
	}
	
	word c = s->numFFC();
	for(word i=0; i<c; i++) //FFC 'trigger flags'
	{
		if(single>=0) if(i+176!=single) continue;
		
		if((!high16only)||(single>=0))
		{
			//for (int32_t iter=0; iter<1; ++iter) // Only one kind of FFC flag now.
			{
				int32_t checkflag=combobuf[s->ffcs[i].data].flag; //Inherent
				//No placed flags yet

				ft = combo_trigger_flag_to_secret_combo_index(checkflag);
				if (ft != -1)  //Change the ffc's combo
				{
					if(ft==sSECNEXT)
					{
						zc_ffc_modify(s->ffcs[i], 1);
					}
					else
					{
						zc_ffc_set(s->ffcs[i], s->secretcombo[ft]);
						s->ffcs[i].cset = s->secretcset[ft];
					}
				}
			}
		}
	}
	
	if(checktrigger) //Hit all triggers->16-31
	{
		checktrigger=false;
		
		if(s->flags6&fTRIGGERF1631)
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
		if(((!(old_enem_secret && (s->flags2&fCLEARSECRET)) /*Enemies->Secret*/ && single < 0) || high16only || s->flags4&fENEMYSCRTPERM))
		{
			int32_t newflag = -1;
			
			for(int32_t iter=0; iter<2; ++iter)
			{
				int32_t checkflag=combobuf[s->data[i]].flag; //Inherent
				
				if(iter==1) checkflag=s->sflag[i];  //Placed
				
				if((checkflag > 15)&&(checkflag < 32)) //If we've got a 16->32 flag change the combo
				{
					auto rpos_handle = get_rpos_handle_for_screen(s, screen, 0, i);
					screen_combo_modify_preroutine(rpos_handle);
					s->data[i] = s->secretcombo[checkflag-16+4];
					s->cset[i] = s->secretcset[checkflag-16+4];
					newflag = s->secretflag[checkflag-16+4];
					screen_combo_modify_postroutine(rpos_handle);
				}
			}
			
			if(newflag >-1) s->sflag[i] = newflag;  //Tiered flag
			
			if (do_combo_triggers)
			{
				for(int32_t j=0; j<6; j++)  //Layers
				{
					mapscr* layer_scr = get_layer_scr(currmap, screen, j);
					if (!layer_scr->valid) continue;
					
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
	}
	
	for(word i=0; i<c; i++) // FFCs
	{
		if((!(s->flags2&fCLEARSECRET) /*Enemies->Secret*/ && single < 0) || high16only || s->flags4&fENEMYSCRTPERM)
		{
			int32_t checkflag=combobuf[s->ffcs[i].data].flag; //Inherent
			
			//No placed flags yet
			if((checkflag > 15)&&(checkflag < 32)) //If we find a flag, change the combo
			{
				zc_ffc_set(s->ffcs[i], s->secretcombo[checkflag - 16 + 4]);
				s->ffcs[i].cset = s->secretcset[checkflag-16+4];
			}
		}
	}
	
endhe:

	if (s->flags4&fDISABLETIME) //Finish timed warp if 'Secrets Disable Timed Warp'
	{
		activated_timed_warp = true;
		s->timedwarptics = 0;
	}
}

// x,y are world coordinates.
// Returns true if there is a flag (either combo, screen, or ffc) at (x, y).
// Out parameters will be set if the flag is Trigger->Self, which modifies how secrets will be triggered.
static bool has_flag_trigger(int32_t x, int32_t y, int32_t flag, rpos_t& out_rpos, bool& out_single16)
{
	if (x < 0 || y < 0 || x >= world_w || y >= world_h) return false;

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
		rpos_t rpos = COMBOPOS_REGION_CHECK_BOUNDS(x, y);
		if (rpos == rpos_t::None)
			continue;

		if (MAPFFCOMBOFLAG(x, y) == flag)
		{
			screen = get_screen_index_for_world_xy(x, y);
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
			screen = get_screen_index_for_world_xy(x, y);
			break;
		}
	}

	if (screen != -1) scr = get_scr(currmap, screen);
	if (!scr) return false;

	if (trigger_rpos == rpos_t::None)
	{
		checktrigger = true;
		trigger_secrets_for_screen(TriggerSource::Unspecified, screen);
	}
	else
	{
		checktrigger = true;
		trigger_secrets_for_screen(TriggerSource::Singular, screen, single16, RPOS_TO_POS(trigger_rpos));
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
			trigger_secrets_for_screen(TriggerSource::Unspecified, screen, scr->flags6&fTRIGGERF1631, -1);
		}
	}
	
	if (setflag && canPermSecret(currdmap, screen))
		if(!(scr->flags5&fTEMPSECRETS))
			setmapflag(scr, screen, mSECRET);

	return true;
}

bool triggerfire(int x, int y, bool setflag, bool any, bool strong, bool magic, bool divine)
{
	int trigflags = (any?combotriggerANYFIRE:0)
		| (strong?combotriggerSTRONGFIRE:0)
		| (magic?combotriggerMAGICFIRE:0)
		| (divine?combotriggerDIVINEFIRE:0);
	if(!trigflags) return false;
	bool ret = false;
	if(any)
		ret = ret||trigger_secrets_if_flag(x,y,mfANYFIRE,setflag);
	if(strong)
		ret = ret||trigger_secrets_if_flag(x,y,mfSTRONGFIRE,setflag);
	if(magic)
		ret = ret||trigger_secrets_if_flag(x,y,mfMAGICFIRE,setflag);
	if(divine)
		ret = ret||trigger_secrets_if_flag(x,y,mfDIVINEFIRE,setflag);
	
	std::set<rpos_t> rposes({COMBOPOS_REGION_CHECK_BOUNDS(x,y),COMBOPOS_REGION_CHECK_BOUNDS(x,y+15),COMBOPOS_REGION_CHECK_BOUNDS(x+15,y),COMBOPOS_REGION_CHECK_BOUNDS(x+15,y+15)});
	for(int q = 0; q < 7; ++q)
	{
		mapscr* m = FFCore.tempScreens[q];
		for (rpos_t rpos : rposes)
		{
			if (rpos == rpos_t::None)
				continue;

			auto rpos_handle = get_rpos_handle(rpos, q);
			if (rpos_handle.combo().triggerflags[2] & trigflags)
			{
				do_trigger_combo(rpos_handle);
				ret = true;
			}
		}
	}

	for_every_ffc([&](const ffc_handle_t& ffc_handle) {
		if ((ffc_handle.combo().triggerflags[2] & trigflags) && ffc_handle.ffc->collide(x,y,16,16))
		{
			do_trigger_combo_ffc(ffc_handle);
			ret = true;
		}
	});

	return ret;
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
			if(thisffc.flags&ffCHANGER)
				return;
				
			// Stationary?
			if(thisffc.flags&ffSTATIONARY)
				return;
				
			// Frozen because Hero's holding up an item?
			if(Hero.getHoldClk()>0 && (thisffc.flags&ffIGNOREHOLDUP)==0)
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
					if(!(otherffc.flags&ffCHANGER))
						return true;
						
					// Ignore this changer?
					if((otherffc.x.getInt()==thisffc.changer_x&&otherffc.y.getInt()==thisffc.changer_y) || thisffc.flags&ffIGNORECHANGER)
						return true;
						
					if((isonline(thisffc.x.getZLong(), thisffc.y.getZLong(), thisffc.prev_changer_x, thisffc.prev_changer_y, otherffc.x.getZLong(), otherffc.y.getZLong()) || // Along the line, or...
						( // At exactly the same position, 
							(thisffc.x==otherffc.x && thisffc.y==otherffc.y)) 
							||
							//or imprecision and close enough
							( (thisffc.flags&ffIMPRECISIONCHANGER) && ((abs(thisffc.x.getZLong() - otherffc.x.getZLong()) < 10000) && abs(thisffc.y.getZLong() - otherffc.y.getZLong()) < 10000) )
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

			ffcdata* linked_ffc = thisffc.link ? get_ffc(thisffc.link - 1).ffc : nullptr;
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
					thisffc.x+=thisffc.vx;
					thisffc.y+=thisffc.vy;
					thisffc.vx+=thisffc.ax;
					thisffc.vy+=thisffc.ay;
					
					
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
					thisffc.flags&=~ffCARRYOVER;
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
					thisffc.flags&=~ffCARRYOVER;
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
					thisffc.flags&=~ffCARRYOVER;
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
					thisffc.flags&=~ffCARRYOVER;
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

optional<int> nextscr(int map, int screen, int dir, bool normal)
{
	auto [m, s] = nextscr2(dir);
	if (m == -1) return nullopt;
    return (m<<7) + s;
}

std::pair<int32_t, int32_t> nextscr2(int32_t dir)
{
	int32_t m = currmap;
    int32_t s = screenscrolling ? scrolling_scr : heroscr;
    
    switch(dir)
    {
    case up:
        s-=16;
        break;
        
    case down:
        s+=16;
        break;
        
    case left:
        s-=1;
        break;
        
    case right:
        s+=1;
        break;
    }
    
    // need to check for screens on other maps, 's' not valid, etc.
    int32_t index = (hero_screen->sidewarpindex >> (dir*2))&3;
    
    // Fun fact: when a scrolling warp is triggered, this function
    // is never even called! - Saf
    if(hero_screen->sidewarptype[index] == 3)                                // scrolling warp
    {
        switch(dir)
        {
        case up:
            if(!(hero_screen->flags2&wfUP))    goto nowarp;
            
            break;
            
        case down:
            if(!(hero_screen->flags2&wfDOWN))  goto nowarp;
            
            break;
            
        case left:
            if(!(hero_screen->flags2&wfLEFT))  goto nowarp;
            
            break;
            
        case right:
            if(!(hero_screen->flags2&wfRIGHT)) goto nowarp;
            
            break;
        }
        
        m = DMaps[hero_screen->sidewarpdmap[index]].map;
        s = hero_screen->sidewarpscr[index] + DMaps[hero_screen->sidewarpdmap[index]].xoff;
    }
    
nowarp:
    if(s<0||s>=128)
        return {-1, -1};

    return {m, s};
}

optional<int> nextscr_mi(int mi, int dir, bool normal)
{
	return nextscr(mi/(normal?MAPSCRSNORMAL:MAPSCRS),
		mi%(normal?MAPSCRSNORMAL:MAPSCRS), dir, normal);
}
optional<int> nextscr(int dir, bool normal)
{
	return nextscr(currmap, currscr, dir, normal);
}

void bombdoor(int32_t x,int32_t y)
{
	if (x < 0 || y < 0 || x >= world_w || y >= world_h)
		return;

	auto rpos_handle = get_rpos_handle_for_world_xy(x, y, 0);
	mapscr* scr = rpos_handle.scr;
	auto [x0, y0] = translate_screen_coordinates_to_world(rpos_handle.screen);
	#define CHECK_RECT(x,y,rx1,ry1,rx2,ry2) (isinRect(x,y,x0+rx1,y0+ry1,x0+rx2,y0+ry2))

    if(scr->door[0]==dBOMB && CHECK_RECT(x,y,100,0,139,48))
    {
        scr->door[0]=dBOMBED;
        putdoor(scrollbuf,0,0,dBOMBED);
        setmapflag(rpos_handle.screen, mDOOR_UP);
        markBmap(-1, rpos_handle.screen);
        
        if(auto v = nextscr(currmap, rpos_handle.screen, up, true))
        {
            setmapflag_mi(*v, mDOOR_DOWN);
            markBmap(-1,*v-(get_currdmap()<<7));
        }
    }
    
    if(scr->door[1]==dBOMB && CHECK_RECT(x,y,100,112,139,176))
    {
        scr->door[1]=dBOMBED;
        putdoor(scrollbuf,0,1,dBOMBED);
        setmapflag(rpos_handle.screen, mDOOR_DOWN);
        markBmap(-1, rpos_handle.screen);
        
        if(auto v = nextscr(currmap, rpos_handle.screen, down, true))
        {
            setmapflag_mi(*v, mDOOR_UP);
            markBmap(-1,*v-(get_currdmap()<<7));
        }
    }
    
    if(scr->door[2]==dBOMB && CHECK_RECT(x,y,0,60,48,98))
    {
        scr->door[2]=dBOMBED;
        putdoor(scrollbuf,0,2,dBOMBED);
        setmapflag(rpos_handle.screen, mDOOR_LEFT);
        markBmap(-1, rpos_handle.screen);
        
        if(auto v = nextscr(currmap, rpos_handle.screen, left, true))
        {
            setmapflag_mi(*v, mDOOR_RIGHT);
            markBmap(-1,*v-(get_currdmap()<<7));
        }
    }
    
    if(scr->door[3]==dBOMB && CHECK_RECT(x,y,192,60,240,98))
    {
        scr->door[3]=dBOMBED;
        putdoor(scrollbuf,0,3,dBOMBED);
        setmapflag(rpos_handle.screen, mDOOR_RIGHT);
        markBmap(-1, rpos_handle.screen);
        
        if(auto v = nextscr(currmap, rpos_handle.screen, right, true))
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
	if (!screenscrolling)
	{
		rpos_t plrpos = COMBOPOS_REGION_CHECK_BOUNDS(Hero.x+8, Hero.y+8);
		if (plrpos != rpos_t::None)
		{
			bool dosw = false;
			if(rpos == hooked_comborpos && (hooked_layerbits & (1<<layer)))
			{
				if(hooked_undercombos[layer] > -1)
				{
					draw_cmb(dest, x, y,
						hooked_undercombos[layer], hooked_undercombos[layer+7], over, transp);
				}
				dosw = true;
			}
			else if(rpos == plrpos && (hooked_layerbits & (1<<(layer+8))))
			{
				dosw = true;
			}
			if(dosw)
			{
				switch(Hero.switchhookstyle)
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
	start_x = MAX(0,  ceil((-15 - x)    / 16.0));
	end_x   = MIN(16, ceil((bmp->w - x) / 16.0));
	start_y = MAX(0,  ceil((-15 - y)    / 16.0));
	end_y   = MIN(11, ceil((bmp->h - y) / 16.0));
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

			if (screenscrolling && (base_scr->ffcs[i].flags & ffCARRYOVER) != 0 && screen_handle.screen != scrolling_scr)
				continue; //If scrolling, only draw carryover ffcs from newscr and not oldscr.

			base_scr->ffcs[i].draw(bmp, x, y, (type==-4));
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
			if(scr && scr->valid)
			{
				for(int32_t i=0; i<176; i++)
				{
					int32_t mf=scr->sflag[i], mf2 = combobuf[scr->data[i]].flag;
					
					if(mf==mfPUSHUD || mf==mfPUSH4 || mf==mfPUSHED || ((mf>=mfPUSHLR)&&(mf<=mfPUSHRINS))
						|| mf2==mfPUSHUD || mf2==mfPUSH4 || mf2==mfPUSHED || ((mf2>=mfPUSHLR)&&(mf2<=mfPUSHRINS)))
					{
						auto rpos = screenscrolling ? rpos_t::None : POS_TO_RPOS(i, screen_handle.screen);
						draw_cmb_pos(bmp, x + COMBOX(i), y + COMBOY(i), rpos, scr->data[i], scr->cset[i], layer, true, false);
					}
				}
			}
			return;
			
		case -1:                                                //over combo
			if(scr && scr->valid)
			{
				for(int32_t i=0; i<176; i++)
				{
					if(combo_class_buf[combobuf[scr->data[i]].type].overhead)
					{
						auto rpos = screenscrolling ? rpos_t::None : POS_TO_RPOS(i, screen_handle.screen);
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
				if(scr && scr->valid)
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
				if(scr && scr->valid)
				{
					if(base_scr->layeropacity[layer-1]!=255)
						transp = true;
					
					if(XOR(base_scr->flags7&fLAYER2BG, DMaps[currdmap].flags&dmfLAYER2BG))
						over = false;
					
					break;
				}
			}
			return;
			
		case 3:
			if(TransLayers || base_scr->layeropacity[layer-1]==255)
			{
				if(scr && scr->valid)
				{
					if(base_scr->layeropacity[layer-1]!=255)
						transp = true;
					
					if(XOR(base_scr->flags7&fLAYER3BG, DMaps[currdmap].flags&dmfLAYER3BG)
						&& !XOR(base_scr->flags7&fLAYER2BG, DMaps[currdmap].flags&dmfLAYER2BG))
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
			auto rpos = screenscrolling ? rpos_t::None : POS_TO_RPOS(i, screen_handle.screen);
			draw_cmb_pos(bmp, x + COMBOX(i), y + COMBOY(i), rpos, scr->data[i], scr->cset[i], layer, over, transp);
		}
	}
}

bool lenscheck(mapscr* basescr, int layer)
{
	if(layer < 0 || layer > 6) return true;
	if(get_qr(qr_OLD_LENS_LAYEREFFECT))
	{
		if(!layer) return true;
		if((layer==(int32_t)(basescr->lens_layer&7)+1) && ((basescr->lens_layer&llLENSSHOWS && !lensclk) || (basescr->lens_layer&llLENSHIDES && lensclk)))
			return false;
	}
	else
	{
		if((lensclk ? basescr->lens_hide : basescr->lens_show) & (1<<layer))
			return false;
	}
	return true;
}

void do_layer_old(BITMAP *bmp, int32_t type, int32_t layer, mapscr* basescr, int32_t x, int32_t y, int32_t tempscreen, bool scrolling, bool drawprimitives)
{
	DCHECK_LAYER_ZERO_INDEX(layer);
	mapscr* layerscr = get_layer_scr(currmap, currscr, layer - 1);
	do_layer(bmp, type, {basescr, layerscr, currmap, currscr, layer}, x, y, drawprimitives);
}

void do_layer(BITMAP *bmp, int32_t type, const screen_handle_t& screen_handle, int32_t x, int32_t y, bool drawprimitives)
{
    bool showlayer = true;
	mapscr* base_scr = screen_handle.base_scr;
	int layer = screen_handle.layer;
    
    switch(type ? type : layer)
    {
    case -4:
    case -3:
        if(!show_ffcs)
        {
            showlayer = false;
        }
        
        break;
        
    case -2:
        if(!show_layer_push)
        {
            showlayer = false;
        }
        
        break;
        
    case -1:
        if(!show_layer_over)
        {
            showlayer = false;
        }
        
        break;
        
    case 1:
        if(!show_layer_1)
        {
            showlayer = false;
        }
        
        break;
        
    case 2:
        if(!show_layer_2)
        {
            showlayer = false;
        }
        
        break;
        
    case 3:
        if(!show_layer_3)
        {
            showlayer = false;
        }
        
        break;
        
    case 4:
        if(!show_layer_4)
        {
            showlayer = false;
        }
        
        break;
        
    case 5:
        if(!show_layer_5)
        {
            showlayer = false;
        }
        
        break;
        
    case 6:
        if(!show_layer_6)
        {
            showlayer = false;
        }
        
        break;
    }
	
    if(!type && (layer==(int32_t)(base_scr->lens_layer&7)+1) && ((base_scr->lens_layer&llLENSSHOWS && !lensclk) || (base_scr->lens_layer&llLENSHIDES && lensclk)))
    {
		if(!lenscheck(base_scr,layer))
        	showlayer = false;
    }
    
    if(showlayer)
    {
		if(type || !(base_scr->hidelayers & (1 << (layer))))
		{
			do_scrolling_layer(bmp, type, screen_handle, x, y);
			if(!type && !get_qr(qr_PUSHBLOCK_SPRITE_LAYER))
				if(mblock2.draw(bmp,layer))
					do_primitives(bmp, SPLAYER_MOVINGBLOCK, 0, playing_field_offset);
		}
        
        if(!type && drawprimitives && layer > 0 && layer <= 6)
        {
            do_primitives(bmp, layer, 0, playing_field_offset);
        }
    }
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
void put_walkflags_a5(int32_t x,int32_t y,int32_t xofs,int32_t yofs, word cmbdat,int32_t lyr)
{
	ALLEGRO_COLOR col_solid = al_map_rgba(178,36,36,info_opacity);
	ALLEGRO_COLOR col_water1 = al_map_rgba(85,85,255,info_opacity);
	ALLEGRO_COLOR col_lhook = al_map_rgba(170,170,170,info_opacity);
	ALLEGRO_COLOR col_stepable = al_map_rgba(165,105,8,info_opacity);
	ALLEGRO_COLOR col_dmg = al_map_rgba(255,255,0,info_opacity);
	newcombo const &c = combobuf[cmbdat];
	
	if (c.type == cBRIDGE && get_qr(qr_OLD_BRIDGE_COMBOS)) return;
	
	int32_t xx = x-xofs;
	int32_t yy = y+playing_field_offset-yofs;
	
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
				if (combobuf[MAPCOMBO2(m,tx2,ty2)].type == cBRIDGE && !_walkflag_layer(tx2,ty2,1, &(tmpscr2[m]))) 
				{
					bridgedetected |= (1<<i);
				}
			}
			else
			{
				if (combobuf[MAPCOMBO2(m,tx2,ty2)].type == cBRIDGE && _effectflag_layer(tx2,ty2,1, &(tmpscr2[m]))) 
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
	
	int32_t xx = x-xofs;
	int32_t yy = y+playing_field_offset-yofs;
	
	for(int32_t i=0; i<4; i++)
	{
		int32_t tx=((i&2)<<2)+xx - viewport.x;
		int32_t ty=((i&1)<<3)+yy - viewport.y;
	
		if(((c.walk>>4)&(1<<i)) && c.type != cNONE)
		{
				int32_t color = vc(10);
					
				rectfill(dest,tx,ty,tx+7,ty+7,color);
		}
	}
}
void put_effectflags_a5(int32_t x,int32_t y,int32_t xofs,int32_t yofs, word cmbdat,int32_t lyr)
{
	ALLEGRO_COLOR col_eff = al_map_rgba(85,255,85,info_opacity);
	newcombo const &c = combobuf[cmbdat];
	
	int32_t xx = x-xofs;
	int32_t yy = y+playing_field_offset-yofs;
	
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
void do_walkflags(mapscr* layer,int32_t x, int32_t y, int32_t tempscreen)
{
	if(show_walkflags)
	{
		start_info_bmp();
		
		for(int32_t i=0; i<176; i++)
		{
			put_walkflags_a5(((i&15)<<4),(i&0xF0),x,y,layer->data[i], 0);
		}
		
		for(int32_t k=0; k<2; k++)
		{
			mapscr* lyr = (tempscreen==2 ? tmpscr2+k : tmpscr3+k);
			
			if(lyr->valid)
			{
				for(int32_t i=0; i<176; i++)
				{
					put_walkflags_a5(((i&15)<<4),(i&0xF0),x,y,lyr->data[i], k%2+1);
				}
			}
		}
		
		if (tempscreen == 2)
		{
			draw_ladder_platform_a5(-x,-y+playing_field_offset,al_map_rgba(165,105,8,info_opacity));
			draw_solid_objects_a5(-x,-y+playing_field_offset,al_map_rgba(178,36,36,info_opacity));
			draw_slopes_a5(-x,-y+playing_field_offset,al_map_rgba(255,85,255,info_opacity));
		}
		
		end_info_bmp();
	}
}

// Effectflags L4 cheat
void do_effectflags(mapscr* layer,int32_t x, int32_t y, int32_t tempscreen)
{
	if(show_effectflags)
	{
		start_info_bmp();
		
		for(int32_t i=0; i<176; i++)
		{
			put_effectflags_a5(((i&15)<<4),(i&0xF0),x,y,layer->data[i], 0);
		}
		
		end_info_bmp();
	}
}

void calc_darkroom_combos(int screen, int offx, int offy)
{
	for(int32_t lyr = 0; lyr < 7; ++lyr)
	{
		mapscr* scr = get_layer_scr_allow_scrolling(currmap, screen, lyr-1);
		if (!scr->valid) continue;

		for(int32_t q = 0; q < 176; ++q)
		{
			newcombo const& cmb = combobuf[scr->data[q]];
			if(cmb.type == cTORCH)
			{
				do_torch_combo(cmb, COMBOX(q)+8+offx, COMBOY(q)+8+offy, darkscr_bmp_z3);
			}
		}
	}

	mapscr* scr = get_layer_scr_allow_scrolling(currmap, screen, -1);
	word c = scr->numFFC();
	for(int q = 0; q < c; ++q)
	{
		newcombo const& cmb = combobuf[scr->ffcs[q].data];
		if(cmb.type == cTORCH)
		{
			int cx = scr->ffcs[q].x.getInt()+(scr->ffEffectWidth(q)/2)+offx;
			int cy = (scr->ffcs[q].y.getInt())+(scr->ffEffectHeight(q)/2)+offy;
			do_torch_combo(cmb, cx, cy, darkscr_bmp_z3);
		}
	}
}

static void for_every_nearby_screen(const std::function <void (std::array<screen_handle_t, 7>, int, int, int)>& fn)
{
	if (!is_z3_scrolling_mode())
	{
		int screen = currscr;
		mapscr* base_scr = get_scr(currmap, screen);
		std::array<screen_handle_t, 7> screen_handles;
		screen_handles[0] = {base_scr, base_scr, currmap, screen, 0};
		for (int i = 1; i < 7; i++)
		{
			mapscr* scr = get_layer_scr(currmap, screen, i - 1);
			screen_handles[i] = {base_scr, scr, currmap, screen, i};
		}

		fn(screen_handles, screen, 0, 0);
		return;
	}

	int heroscr_x = heroscr % 16;
	int heroscr_y = heroscr / 16;

	for (int heroscr_dx = -1; heroscr_dx <= 1; heroscr_dx++)
	{
		for (int heroscr_dy = -1; heroscr_dy <= 1; heroscr_dy++)
		{
			int scr_x = heroscr_x + heroscr_dx;
			int scr_y = heroscr_y + heroscr_dy;
			if (tmpscr->flags&fMAZE && !(XY_DELTA_TO_DIR(heroscr_dx, 0) == tmpscr->exitdir || XY_DELTA_TO_DIR(0, heroscr_dy) == tmpscr->exitdir))
			{
				scr_x = heroscr_x;
				scr_y = heroscr_y;
			}
			if (heroscr_dx || heroscr_dy)
			{
				if (Hero.edge_of_dmap(XY_DELTA_TO_DIR(heroscr_dx, 0))) continue;
				if (Hero.edge_of_dmap(XY_DELTA_TO_DIR(0, heroscr_dy))) continue;
				if (scr_x < 0 || scr_x >= 16) continue;
				if (scr_y < 0 || scr_y >= 8) continue;
			}

			int screen = scr_x + scr_y * 16;
			if (!is_in_current_region(screen)) continue;

			mapscr* base_scr = get_scr(currmap, screen);
			if (!(base_scr->valid & mVALID)) continue;

			auto [offx, offy] = translate_screen_coordinates_to_world(screen);

			// Skip processsing screen if out of viewport.
			if (offx - viewport.x <= -256) continue;
			if (offy - viewport.y <= -176) continue;
			if (offx - viewport.x >= 256) continue;
			if (offy - viewport.y >= (is_extended_height_mode() ? 240 : 176)) continue;

			std::array<screen_handle_t, 7> screen_handles;
			screen_handles[0] = {base_scr, base_scr, currmap, screen, 0};
			for (int i = 1; i < 7; i++)
			{
				mapscr* scr = get_layer_scr(currmap, screen, i - 1);
				screen_handles[i] = {base_scr, scr, currmap, screen, i};
			}

			fn(screen_handles, screen, offx, offy);
		}
	}
}

static void for_every_screen_in_region_check_viewport(const std::function <void (std::array<screen_handle_t, 7>, int, int, int, bool)>& fn)
{
	for_every_screen_in_region([&](mapscr* base_scr, int screen, unsigned int region_scr_x, unsigned int region_scr_y) {
		std::array<screen_handle_t, 7> screen_handles;
		screen_handles[0] = {base_scr, base_scr, currmap, screen, 0};
		for (int i = 1; i < 7; i++)
		{
			mapscr* scr = get_layer_scr(currmap, screen, i - 1);
			screen_handles[i] = {base_scr, scr, currmap, screen, i};
		}

		int offx = region_scr_x * 256;
		int offy = region_scr_y * 176;
		bool in_viewport = viewport.intersects_with(offx, offy, offx + 256, offy + 256);

		fn(screen_handles, screen, offx, offy, in_viewport);
	});
}

void draw_msgstr(byte layer)
{
	if(layer != msgstr_layer) return;

	if(!(msg_bg_display_buf->clip))
	{
		blit_msgstr_bg(framebuf,0,0,0,playing_field_offset,256,168);
	}
	
	if(!(msg_portrait_display_buf->clip))
	{
		blit_msgstr_prt(framebuf,0,0,0,playing_field_offset,256,168);
	}
	
	if(!(msg_txt_display_buf->clip))
	{
		blit_msgstr_fg(framebuf,0,0,0,playing_field_offset,256,168);
	}
}

void draw_screen(bool showhero, bool runGeneric)
{
	if (!screenscrolling)
		z3_update_viewport();

	mapscr* this_screen = tmpscr;
	clear_info_bmp();
	if((GameFlags & (GAMEFLAG_SCRIPTMENU_ACTIVE|GAMEFLAG_F6SCRIPT_ACTIVE))!=0)
	{
		FFCore.doScriptMenuDraws();
		return;
	}
	
	if(runGeneric) FFCore.runGenericPassiveEngine(SCR_TIMING_PRE_DRAW);
	
	//The Plan:
	//1. Draw some background layers
	//2. Blit scrollbuf onto framebuf
	//3. Draw some sprites
	//4. -----
	//5. Draw some layers
	//6. -----
	//6b. Draw the subscreen, without clipping
	//7. Draw some flying sprites
	//8. -----
	//9. Draw some layers
	//10. ----
	//11. Draw some text
	//12. Draw the subscreen, without clipping
	clear_bitmap(framebuf);
	clear_clip_rect(framebuf);
	
	int32_t cmby2=0;
	
	//1. Draw some background layers
	clear_bitmap(scrollbuf);
	
	for_every_nearby_screen([&](std::array<screen_handle_t, 7> screen_handles, int screen, int offx, int offy) {
		mapscr* base_scr = screen_handles[0].base_scr;
		if(XOR(base_scr->flags7&fLAYER2BG, DMaps[currdmap].flags&dmfLAYER2BG))
		{
			do_layer(scrollbuf, 0, screen_handles[2], offx, offy, true);
			if (screen == currscr) particles.draw(framebuf, true, 1);
			if (screen == currscr) draw_msgstr(2);
		}
		
		if(XOR(base_scr->flags7&fLAYER3BG, DMaps[currdmap].flags&dmfLAYER3BG))
		{
			do_layer(scrollbuf, 0, screen_handles[3], offx, offy, true);
			if (screen == currscr) particles.draw(framebuf, true, 2);
			if (screen == currscr) draw_msgstr(3);
		}
	});

	for_every_nearby_screen([&](std::array<screen_handle_t, 7> screen_handles, int screen, int offx, int offy) {
		mapscr* base_scr = screen_handles[0].base_scr;
		if (lenscheck(base_scr, 0))
		{
			putscr(scrollbuf, offx, offy + playing_field_offset, base_scr);
		}
	});

	if (lenscheck(hero_screen, 0))
	{
		if(!get_qr(qr_PUSHBLOCK_SPRITE_LAYER))
			if(mblock2.draw(scrollbuf,0))
				do_primitives(scrollbuf, SPLAYER_MOVINGBLOCK, 0, playing_field_offset);
	}

	// Lens hints, then primitives, then particles.
	if((lensclk || (get_debug() && zc_getkey(KEY_L))) && !get_qr(qr_OLDLENSORDER))
	{
		draw_lens_under(scrollbuf, false);
		do_primitives(scrollbuf, SPLAYER_LENS_UNDER_1, 0, playing_field_offset);
	}
	
	if(show_layer_0 && lenscheck(this_screen,0))
		do_primitives(scrollbuf, 0, 0, playing_field_offset);
		
	particles.draw(framebuf, true, -3);
	draw_msgstr(0);
	
	set_clip_rect(scrollbuf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2);
	
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
	
	for_every_screen_in_region_check_viewport([&](std::array<screen_handle_t, 7> screen_handles, int screen, int offx, int offy, bool in_viewport) {
		mapscr* base_scr = screen_handles[0].base_scr;

		if (in_viewport)
		{
			do_layer(scrollbuf, 0, screen_handles[1], offx, offy, true); // LAYER 1
			if (screen == currscr) particles.draw(framebuf, true, 0);
			if (screen == currscr) draw_msgstr(1);
		}
		
		do_layer(scrollbuf, -3, screen_handles[0], 0, 0); // freeform combos!

		if (in_viewport)
		{
			if(!XOR(base_scr->flags7&fLAYER2BG, DMaps[currdmap].flags&dmfLAYER2BG))
			{
				do_layer(scrollbuf, 0, screen_handles[2], offx, offy, true); // LAYER 2
				if (screen == currscr) particles.draw(framebuf, true, 1);
				if (screen == currscr) draw_msgstr(2);
			}
		}
	});

	do_primitives(framebuf, SPLAYER_FFC_DRAW, 0, playing_field_offset);

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
	
	for_every_nearby_screen([&](std::array<screen_handle_t, 7> screen_handles, int screen, int offx, int offy) {
		mapscr* base_scr = screen_handles[0].base_scr;

		if (get_qr(qr_PUSHBLOCK_SPRITE_LAYER))
		{
			do_layer(scrollbuf, -2, screen_handles[0], offx, offy); // push blocks!
			if(get_qr(qr_PUSHBLOCK_LAYER_1_2))
			{
				do_layer(scrollbuf, -2, screen_handles[1], offx, offy); // push blocks!
				do_layer(scrollbuf, -2, screen_handles[2], offx, offy); // push blocks!
			}
			// TODO z3 ?
			do_primitives(scrollbuf, SPLAYER_PUSHBLOCK, offx, offy + playing_field_offset);
		}

		// Show walkflags cheat
		do_walkflags(base_scr, offx, offy, 2);
		do_effectflags(base_scr, offx, offy, 2);
	});
	
	putscrdoors(scrollbuf,0,playing_field_offset);
	
	// Lens hints, doors etc.
	if(lensclk || (get_debug() && zc_getkey(KEY_L)))
	{
		if(get_qr(qr_OLDLENSORDER))
		{
			draw_lens_under(scrollbuf, false);
			do_primitives(scrollbuf, SPLAYER_LENS_UNDER_1, 0, playing_field_offset);
		}
		
		draw_lens_under(scrollbuf, true);
		do_primitives(scrollbuf, SPLAYER_LENS_UNDER_2, 0, playing_field_offset);
	}
	
	//2. Blit those layers onto framebuf
	
	set_clip_rect(framebuf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2);

	blit(scrollbuf, framebuf, 0, 0, 0, 0, 256, 224);

	// After this point, we no longer draw to the scrollbuf - so things like dosubscr have access to a "partially rendered" frame.
	// I think only used for COOLSCROLL==0? Seems like a silly feature...

	//6b. Draw the subscreen, without clipping
	if(!get_qr(qr_SUBSCREENOVERSPRITES))
	{
		bool dotime = false;
		if (replay_version_check(22) || !replay_is_active()) dotime = game->should_show_time();
		put_passive_subscr(framebuf, 0, passive_subscreen_offset, dotime, sspUP);
	}
	
	//3. Draw some sprites onto framebuf
	set_clip_rect(framebuf,0,0,256,224);
	
	if(!(pricesdisplaybuf->clip))
	{
		masked_blit(pricesdisplaybuf,framebuf,0,0,0,playing_field_offset,256,168);
	}
	
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
			for(int32_t i=0; i<Ewpns.Count(); i++)
			{
				if(((weapon *)Ewpns.spr(i))->behind)
					Ewpns.spr(i)->draw(framebuf);
			}
			do_primitives(framebuf, SPLAYER_EWEAP_BEHIND_DRAW, 0, playing_field_offset);
			
			for(int32_t i=0; i<Lwpns.Count(); i++)
			{
				if(((weapon *)Lwpns.spr(i))->behind)
					Lwpns.spr(i)->draw(framebuf);
			}
			do_primitives(framebuf, SPLAYER_LWEAP_BEHIND_DRAW, 0, playing_field_offset);
			
			if(get_qr(qr_SHADOWS)&&(!get_qr(qr_SHADOWSFLICKER)||frame&1))
			{
				guys.drawshadow(framebuf,get_qr(qr_TRANSSHADOWS)!=0,true);
			}
			
			guys.draw(framebuf,true);
			do_primitives(framebuf, SPLAYER_NPC_DRAW, 0, playing_field_offset);
			chainlinks.draw(framebuf,true);
			do_primitives(framebuf, SPLAYER_CHAINLINK_DRAW, 0, playing_field_offset);
			//Lwpns.draw(framebuf,true);
			
			for(int32_t i=0; i<Ewpns.Count(); i++)
			{
				if(!((weapon *)Ewpns.spr(i))->behind)
					Ewpns.spr(i)->draw(framebuf);
			}
			do_primitives(framebuf, SPLAYER_EWEAP_FRONT_DRAW, 0, playing_field_offset);
			
			for(int32_t i=0; i<Lwpns.Count(); i++)
			{
				if(!((weapon *)Lwpns.spr(i))->behind)
					Lwpns.spr(i)->draw(framebuf);
			}
			do_primitives(framebuf, SPLAYER_LWEAP_FRONT_DRAW, 0, playing_field_offset);
			
			
			items.draw(framebuf,true);
			do_primitives(framebuf, SPLAYER_ITEMSPRITE_DRAW, 0, playing_field_offset);
		}
		else
		{
			for(int32_t i=0; i<Ewpns.Count(); i++)
			{
				if(((weapon *)Ewpns.spr(i))->behind)
					Ewpns.spr(i)->draw(framebuf);
			}
			do_primitives(framebuf, SPLAYER_EWEAP_BEHIND_DRAW, 0, playing_field_offset);
		
			for(int32_t i=0; i<Lwpns.Count(); i++)
			{
				if(((weapon *)Lwpns.spr(i))->behind)
					Lwpns.spr(i)->draw(framebuf);
			}
			do_primitives(framebuf, SPLAYER_LWEAP_BEHIND_DRAW, 0, playing_field_offset);
			
			if(get_qr(qr_SHADOWS)&&(!get_qr(qr_SHADOWSFLICKER)||frame&1))
			{
				guys.drawshadow(framebuf,get_qr(qr_TRANSSHADOWS)!=0,true);
			}
			
			items.draw(framebuf,false);
			do_primitives(framebuf, SPLAYER_ITEMSPRITE_DRAW, 0, playing_field_offset);
			chainlinks.draw(framebuf,false);
			do_primitives(framebuf, SPLAYER_CHAINLINK_DRAW, 0, playing_field_offset);
			//Lwpns.draw(framebuf,false);
			guys.draw(framebuf,false);
			do_primitives(framebuf, SPLAYER_NPC_DRAW, 0, playing_field_offset);
			
			for(int32_t i=0; i<Ewpns.Count(); i++)
			{
				if(!((weapon *)Ewpns.spr(i))->behind)
				{
					Ewpns.spr(i)->draw(framebuf);
				}
			}
			do_primitives(framebuf, SPLAYER_EWEAP_FRONT_DRAW, 0, playing_field_offset);
		
			for(int32_t i=0; i<Lwpns.Count(); i++)
			{
				if(!((weapon *)Lwpns.spr(i))->behind)
				{
					Lwpns.spr(i)->draw(framebuf);
				}
			}
			do_primitives(framebuf, SPLAYER_LWEAP_FRONT_DRAW, 0, playing_field_offset);
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
			do_primitives(framebuf, SPLAYER_MOVINGBLOCK, 0, playing_field_offset);
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
		do_primitives(framebuf, SPLAYER_NPC_ABOVEPLAYER_DRAW, 0, playing_field_offset);
	}
	
	//5. Draw some layers onto framebuf
	set_clip_rect(framebuf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2);
	
	for_every_nearby_screen([&](std::array<screen_handle_t, 7> screen_handles, int screen, int offx, int offy) {
		mapscr* base_scr = screen_handles[0].base_scr;

		if(!XOR(base_scr->flags7&fLAYER3BG, DMaps[currdmap].flags&dmfLAYER3BG))
		{
			do_layer(framebuf, 0, screen_handles[3], offx, offy, true);
			if (screen == currscr) particles.draw(framebuf, true, 2);
			if (screen == currscr) draw_msgstr(3);
		}
		
		do_layer(framebuf, 0, screen_handles[4], offx, offy, true);
		//do_primitives(framebuf, 3, 0,playing_field_offset);//don't uncomment me
		
		if (screen == currscr) particles.draw(framebuf, true, 3);
		if (screen == currscr) draw_msgstr(4);

		do_layer(framebuf, -1, screen_handles[0], offx, offy);
		if (get_qr(qr_OVERHEAD_COMBOS_L1_L2))
		{
			do_layer(framebuf, -1, screen_handles[1], offx, offy);
			do_layer(framebuf, -1, screen_handles[2], offx, offy);
		}
	});

	if (!is_extended_height_mode() && is_z3_scrolling_mode() && !get_qr(qr_SUBSCREENOVERSPRITES))
	{
		rectfill(framebuf, 0, 0, 256, playing_field_offset - 1, 0);
	}
	do_primitives(framebuf, SPLAYER_OVERHEAD_CMB, 0, playing_field_offset);
	
	particles.draw(framebuf, true, -1);
	
	// //6b. Draw the subscreen, without clipping
	// if(!get_qr(qr_SUBSCREENOVERSPRITES))
	// {
	// 	bool dotime = false;
	// 	if (replay_version_check(22) || !replay_is_active()) dotime = game->should_show_time();
	// 	put_passive_subscr(framebuf, 0, passive_subscreen_offset, dotime, sspUP);
	// }
	
	
	//7. Draw some flying sprites onto framebuf
	clear_clip_rect(framebuf);
	// set_clip_rect(framebuf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2);
	
	//Jumping Hero and jumping enemies are drawn on this layer.
	if(Hero.getZ() > (zfix)zinit.jump_hero_layer_threshold)
	{
		decorations.draw2(framebuf,false);
		Hero.draw(framebuf);
		chainlinks.draw(framebuf,true);
		do_primitives(framebuf, SPLAYER_CHAINLINK_DRAW, 0, playing_field_offset);
		
		for(int32_t i=0; i<Lwpns.Count(); i++)
		{
			if(Lwpns.spr(i)->z+Lwpns.spr(i)->fakez > (zfix)zinit.jump_hero_layer_threshold)
			{
				Lwpns.spr(i)->draw(framebuf);
			}
		}
		do_primitives(framebuf, SPLAYER_LWEAP_ABOVE_DRAW, 0, playing_field_offset);
		
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
	do_primitives(framebuf, SPLAYER_NPC_AIRBORNE_DRAW, 0, playing_field_offset);
	
	// Draw the Moving Fairy above layer 3
	for(int32_t i=0; i<items.Count(); i++)
		if(itemsbuf[items.spr(i)->id].family == itype_fairy && itemsbuf[items.spr(i)->id].misc3)
			items.spr(i)->draw(framebuf);
	do_primitives(framebuf, SPLAYER_FAIRYITEM_DRAW, 0, playing_field_offset);
	
	//9. Draw some layers onto framebuf

	set_clip_rect(framebuf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2);
	
	if (lightbeam_present)
	{
		color_map = &trans_table2;
		if(get_qr(qr_LIGHTBEAM_TRANSPARENT))
			draw_trans_sprite(framebuf, lightbeam_bmp, 0, playing_field_offset);
		else 
			masked_blit(lightbeam_bmp, framebuf, 0, 0, 0, playing_field_offset, 256, 176);
		color_map = &trans_table;
	}

	for_every_screen_in_region_check_viewport([&](std::array<screen_handle_t, 7> screen_handles, int screen, int offx, int offy, bool in_viewport) {
		mapscr* base_scr = screen_handles[0].base_scr;

		if (in_viewport)
		{
			do_layer(framebuf, 0, screen_handles[5], offx, offy, true);
			if (screen == currscr) particles.draw(framebuf, true, 4);
			if (screen == currscr) draw_msgstr(5);
		}

		// overhead freeform combos!
		do_layer(framebuf, -4, screen_handles[0], 0, 0);

		if (in_viewport)
		{
			if (screen == currscr)
			{
				do_primitives(framebuf, SPLAYER_OVERHEAD_FFC, offx, offy + playing_field_offset);
			}
			// ---
			do_layer(framebuf, 0, screen_handles[6], offx, offy, true);
			if (screen == currscr) particles.draw(framebuf, true, 5);
		}
	});
	
	//11. Handle low drawn darkness
	bool draw_dark = false;
	if(get_qr(qr_NEW_DARKROOM) && room_is_dark)
	{
		for_every_nearby_screen([&](std::array<screen_handle_t, 7> screen_handles, int screen, int offx, int offy) {
			mapscr* base_scr = screen_handles[0].scr;
			if (base_scr->flags&fDARK)
			{
				calc_darkroom_combos(screen, offx, offy + playing_field_offset);
				draw_dark = true;
			}
		});
		if(showhero)
			Hero.calc_darkroom_hero(0, -playing_field_offset);
		if (draw_dark)
		{
			for_every_nearby_screen([&](std::array<screen_handle_t, 7> screen_handles, int screen, int offx, int offy) {
				mapscr* base_scr = screen_handles[0].scr;
				bool should_be_dark = (base_scr->flags & fDARK);
				if (!should_be_dark)
				{
					offy += playing_field_offset;
					rectfill(darkscr_bmp_z3, offx - viewport.x, offy - viewport.y, offx - viewport.x + 256 - 1, offy - viewport.y + 176 - 1, 0);
					rectfill(darkscr_bmp_z3_trans, offx - viewport.x, offy - viewport.y, offx - viewport.x + 256 - 1, offy - viewport.y + 176 - 1, 0);
				}
			});
		}
	}
	
	//Darkroom if under the subscreen
	if(get_qr(qr_NEW_DARKROOM) && get_qr(qr_NEWDARK_L6) && draw_dark && room_is_dark)
	{
		do_primitives(framebuf, SPLAYER_DARKROOM_UNDER, 0, playing_field_offset);
		set_clip_rect(framebuf, 0, playing_field_offset, framebuf->w, framebuf->h);
		if(this_screen->flags9 & fDARK_DITHER) //dither the entire bitmap
		{
			ditherblit(darkscr_bmp_z3,darkscr_bmp_z3,0,game->get_dither_type(),game->get_dither_arg());
			ditherblit(darkscr_bmp_z3_trans,darkscr_bmp_z3_trans,0,game->get_dither_type(),game->get_dither_arg());
		}
		
		color_map = &trans_table2;
		if(this_screen->flags9 & fDARK_TRANS) //draw the dark as transparent
		{
			draw_trans_sprite(framebuf, darkscr_bmp_z3, 0, 0);
			if(get_qr(qr_NEWDARK_TRANS_STACKING))
				draw_trans_sprite(framebuf, darkscr_bmp_z3_trans, 0, 0);
		}
		else
		{
			masked_blit(darkscr_bmp_z3, framebuf, 0, 0, 0, 0, framebuf->w, framebuf->h);
			draw_trans_sprite(framebuf, darkscr_bmp_z3_trans, 0, 0);
		}
		color_map = &trans_table;
		
		set_clip_rect(framebuf, 0, 0, framebuf->w, framebuf->h);
		do_primitives(framebuf, SPLAYER_DARKROOM_OVER, 0, playing_field_offset);
	}
	
	
	//12. Draw some text on framebuf
	
	set_clip_rect(framebuf,0,0,256,224);
	
	//13. Draw the subscreen, without clipping
	// TODO z3
	if(get_qr(qr_SUBSCREENOVERSPRITES))
	{
		put_passive_subscr(framebuf, 0, passive_subscreen_offset, game->should_show_time(), sspUP);
		
		// Draw primitives over subscren
		do_primitives(framebuf, 7, 0, playing_field_offset); //Layer '7' appears above subscreen if quest rule is set
	}

	draw_msgstr(6);
	
	//14. Handle high-drawn darkness
	if(get_qr(qr_NEW_DARKROOM) && !get_qr(qr_NEWDARK_L6) && draw_dark && room_is_dark)
	{
		do_primitives(framebuf, SPLAYER_DARKROOM_UNDER, 0, playing_field_offset);
		set_clip_rect(framebuf, 0, playing_field_offset, framebuf->w, framebuf->h);
		if(this_screen->flags9 & fDARK_DITHER) //dither the entire bitmap
		{
			ditherblit(darkscr_bmp_z3,darkscr_bmp_z3,0,game->get_dither_type(),game->get_dither_arg());
			ditherblit(darkscr_bmp_z3_trans,darkscr_bmp_z3_trans,0,game->get_dither_type(),game->get_dither_arg());
		}
		
		color_map = &trans_table2;
		if(this_screen->flags9 & fDARK_TRANS) //draw the dark as transparent
		{
			draw_trans_sprite(framebuf, darkscr_bmp_z3, 0, 0);
			if(get_qr(qr_NEWDARK_TRANS_STACKING))
				draw_trans_sprite(framebuf, darkscr_bmp_z3_trans, 0, 0);
		}
		else
		{
			masked_blit(darkscr_bmp_z3, framebuf, 0, 0, 0, 0, framebuf->w, framebuf->h);
			draw_trans_sprite(framebuf, darkscr_bmp_z3_trans, 0, 0);
		}
		color_map = &trans_table;
		
		set_clip_rect(framebuf, 0, 0, framebuf->w, framebuf->h);
		do_primitives(framebuf, SPLAYER_DARKROOM_OVER, 0, playing_field_offset);
	}
	
	draw_msgstr(7);
	
	set_clip_rect(scrollbuf, 0, 0, scrollbuf->w, scrollbuf->h);
	if(runGeneric) FFCore.runGenericPassiveEngine(SCR_TIMING_POST_DRAW);
}

void put_door(BITMAP *dest,int32_t t,int32_t pos,int32_t side,int32_t type,bool redraw,bool even_walls)
{
	mapscr* m = t == 0 ? tmpscr : &special_warp_return_screen;
	put_door(dest, m, pos, side, type, redraw, even_walls);
}

// TODO: separate setting door data and drawing door
void put_door(BITMAP *dest,mapscr* scr,int32_t pos,int32_t side,int32_t type,bool redraw,bool even_walls)
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

void over_door_new(BITMAP *dest, int32_t pos, int32_t side, int32_t door_combo_set, int32_t offx, int32_t offy)
{
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

void over_door(BITMAP *dest,int32_t t, int32_t pos,int32_t side, int32_t xoff, int32_t yoff)
{
	mapscr& screen = t == 0 ? *tmpscr : special_warp_return_screen;
	over_door_new(dest, pos, side, screen.door_combo_set, xoff, yoff);
}

void update_door(mapscr* m,int32_t side,int32_t door,bool even_walls)
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
			opendoors=-4;
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
		put_door(nullptr,m,7,side,doortype,false,even_walls);
		break;
		
	case down:
		put_door(nullptr,m,151,side,doortype,false,even_walls);
		break;
		
	case left:
		put_door(nullptr,m,64,side,doortype,false,even_walls);
		break;
		
	case right:
		put_door(nullptr,m,78,side,doortype,false,even_walls);
		break;
	}
}

void putdoor(BITMAP *dest,int32_t t,int32_t side,int32_t door,bool redraw,bool even_walls)
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
			opendoors=-4;
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
				over_door(dest,t,39,side,0,0);
			}
			[[fallthrough]];
		default:
			put_door(dest,t,7,side,doortype,redraw, even_walls);
			break;
		}
		
		break;
		
	case down:
		switch(door)
		{
		case dBOMBED:
			if(redraw)
			{
				over_door(dest,t,135,side,0,0);
			}
			[[fallthrough]];
		default:
			put_door(dest,t,151,side,doortype,redraw, even_walls);
			break;
		}
		
		break;
		
	case left:
		switch(door)
		{
		case dBOMBED:
			if(redraw)
			{
				over_door(dest,t,66,side,0,0);
			}
			[[fallthrough]];
		default:
			put_door(dest,t,64,side,doortype,redraw, even_walls);
			break;
		}
		
		break;
		
	case right:
		switch(door)
		{
		case dBOMBED:
			if(redraw)
			{
				over_door(dest,t,77,side,0,0);
			}
			[[fallthrough]];
		default:
			put_door(dest,t,78,side,doortype,redraw, even_walls);
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

void showbombeddoor(BITMAP *dest, int32_t side)
{
    int32_t d=tmpscr->door_combo_set;
    
    switch(side)
    {
    case up:
        putcombo_not_zero(dest,(7&15)<<4,(7&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_u[dt_bomb][0],
                          DoorComboSets[d].doorcset_u[dt_bomb][0]);
        putcombo_not_zero(dest,(8&15)<<4,(8&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_u[dt_bomb][1],
                          DoorComboSets[d].doorcset_u[dt_bomb][1]);
        putcombo_not_zero(dest,(23&15)<<4,(23&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_u[dt_bomb][2],
                          DoorComboSets[d].doorcset_u[dt_bomb][2]);
        putcombo_not_zero(dest,(24&15)<<4,(24&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_u[dt_bomb][3],
                          DoorComboSets[d].doorcset_u[dt_bomb][3]);
        overcombo_not_zero(dest,(39&15)<<4,(39&0xF0)+playing_field_offset,
                           DoorComboSets[d].bombdoorcombo_u[0],
                           DoorComboSets[d].bombdoorcset_u[0]);
        overcombo_not_zero(dest,(40&15)<<4,(40&0xF0)+playing_field_offset,
                           DoorComboSets[d].bombdoorcombo_u[1],
                           DoorComboSets[d].bombdoorcset_u[1]);
        break;
        
    case down:
        putcombo_not_zero(dest,(151&15)<<4,(151&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_d[dt_bomb][0],
                          DoorComboSets[d].doorcset_d[dt_bomb][0]);
        putcombo_not_zero(dest,(152&15)<<4,(152&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_d[dt_bomb][1],
                          DoorComboSets[d].doorcset_d[dt_bomb][1]);
        putcombo_not_zero(dest,(167&15)<<4,(167&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_d[dt_bomb][2],
                          DoorComboSets[d].doorcset_d[dt_bomb][2]);
        putcombo_not_zero(dest,(168&15)<<4,(168&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_d[dt_bomb][3],
                          DoorComboSets[d].doorcset_d[dt_bomb][3]);
        overcombo_not_zero(dest,(135&15)<<4,(135&0xF0)+playing_field_offset,
                           DoorComboSets[d].bombdoorcombo_d[0],
                           DoorComboSets[d].bombdoorcset_d[0]);
        overcombo_not_zero(dest,(136&15)<<4,(136&0xF0)+playing_field_offset,
                           DoorComboSets[d].bombdoorcombo_d[1],
                           DoorComboSets[d].bombdoorcset_d[1]);
        break;
        
    case left:
        putcombo_not_zero(dest,(64&15)<<4,(64&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_l[dt_bomb][0],
                          DoorComboSets[d].doorcset_l[dt_bomb][0]);
        putcombo_not_zero(dest,(65&15)<<4,(65&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_l[dt_bomb][1],
                          DoorComboSets[d].doorcset_l[dt_bomb][1]);
        putcombo_not_zero(dest,(80&15)<<4,(80&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_l[dt_bomb][2],
                          DoorComboSets[d].doorcset_l[dt_bomb][2]);
        putcombo_not_zero(dest,(81&15)<<4,(81&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_l[dt_bomb][3],
                          DoorComboSets[d].doorcset_l[dt_bomb][3]);
        putcombo_not_zero(dest,(96&15)<<4,(96&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_l[dt_bomb][4],
                          DoorComboSets[d].doorcset_l[dt_bomb][4]);
        putcombo_not_zero(dest,(97&15)<<4,(97&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_l[dt_bomb][5],
                          DoorComboSets[d].doorcset_l[dt_bomb][5]);
        overcombo_not_zero(dest,(66&15)<<4,(66&0xF0)+playing_field_offset,
                           DoorComboSets[d].bombdoorcombo_l[0],
                           DoorComboSets[d].bombdoorcset_l[0]);
        overcombo_not_zero(dest,(82&15)<<4,(82&0xF0)+playing_field_offset,
                           DoorComboSets[d].bombdoorcombo_l[1],
                           DoorComboSets[d].bombdoorcset_l[1]);
        overcombo_not_zero(dest,(98&15)<<4,(98&0xF0)+playing_field_offset,
                           DoorComboSets[d].bombdoorcombo_l[2],
                           DoorComboSets[d].bombdoorcset_l[2]);
        break;
        
    case right:
        putcombo_not_zero(dest,(78&15)<<4,(78&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_r[dt_bomb][0],
                          DoorComboSets[d].doorcset_r[dt_bomb][0]);
        putcombo_not_zero(dest,(79&15)<<4,(79&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_r[dt_bomb][1],
                          DoorComboSets[d].doorcset_r[dt_bomb][1]);
        putcombo_not_zero(dest,(94&15)<<4,(94&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_r[dt_bomb][2],
                          DoorComboSets[d].doorcset_r[dt_bomb][2]);
        putcombo_not_zero(dest,(95&15)<<4,(95&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_r[dt_bomb][3],
                          DoorComboSets[d].doorcset_r[dt_bomb][3]);
        putcombo_not_zero(dest,(110&15)<<4,(110&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_r[dt_bomb][4],
                          DoorComboSets[d].doorcset_r[dt_bomb][4]);
        putcombo_not_zero(dest,(111&15)<<4,(111&0xF0)+playing_field_offset,
                          DoorComboSets[d].doorcombo_r[dt_bomb][5],
                          DoorComboSets[d].doorcset_r[dt_bomb][5]);
        overcombo_not_zero(dest,(77&15)<<4,(77&0xF0)+playing_field_offset,
                           DoorComboSets[d].bombdoorcombo_r[0],
                           DoorComboSets[d].bombdoorcset_r[0]);
        overcombo_not_zero(dest,(93&15)<<4,(93&0xF0)+playing_field_offset,
                           DoorComboSets[d].bombdoorcombo_r[1],
                           DoorComboSets[d].bombdoorcset_r[1]);
        overcombo_not_zero(dest,(109&15)<<4,(109&0xF0)+playing_field_offset,
                           DoorComboSets[d].bombdoorcombo_r[2],
                           DoorComboSets[d].bombdoorcset_r[2]);
        break;
    }
}

void openshutters(mapscr* scr, int screen)
{
	bool opened_door = false;
	for(int32_t i=0; i<4; i++)
		if(scr->door[i]==dSHUTTER)
		{
			putdoor(scrollbuf,0,i,dOPENSHUTTER);
			scr->door[i]=dOPENSHUTTER;
			opened_door = true;
		}

	for_every_rpos_in_screen(scr, screen, [&](const rpos_handle_t& rpos_handle) {
		auto& cmb = rpos_handle.combo();	
		if (cmb.triggerflags[0] & combotriggerSHUTTER)
		{
			do_trigger_combo(rpos_handle);
		}
	});
	if (!get_qr(qr_OLD_FFC_FUNCTIONALITY))
	{
		for_every_ffc_in_screen(scr, screen, [&](const ffc_handle_t& ffc_handle) {
			auto& cmb = ffc_handle.combo();
			if(cmb.triggerflags[0] & combotriggerSHUTTER)
				do_trigger_combo_ffc(ffc_handle);
		});
	}

	if(opened_door)
		sfx(WAV_DOOR,128);
}

void clear_darkroom_bitmaps()
{
	clear_to_color(darkscr_bmp_z3, game->get_darkscr_color());
	clear_to_color(darkscr_bmp_z3_trans, game->get_darkscr_color());
}

void load_a_screen_and_layers(int dmap, int map, int screen, int ldir)
{
	std::vector<mapscr*> screens;

	const mapscr* source = get_canonical_scr(map, screen);
	mapscr* base_scr = new mapscr(*source);
	if (map == currmap) temporary_screens_currmap[screen*7] = base_scr;
	screens.push_back(base_scr);

	base_scr->valid |= mVALID; // layer 0 is always valid

	if (source->script > 0)
	{
		FFCore.reset_script_engine_data(ScriptType::Screen, screen);
	}

	for (int i = 0; i < 6; i++)
	{
		if(source->layermap[i]>0)
		{
			screens.push_back(new mapscr(*get_canonical_scr(source->layermap[i]-1, source->layerscreen[i])));
		}
		else
		{
			screens.push_back(new mapscr());
		}
		if (map == currmap) temporary_screens_currmap[screen*7+i+1] = screens[i+1];
	}

	if (map != currmap) temporary_screens[map*MAPSCRS + screen] = screens;

	// Apply perm secrets, if applicable.
	if (canPermSecret(dmap, screen))
	{
		if(game->maps[map*MAPSCRSNORMAL + screen] & mSECRET)    // if special stuff done before
		{
			reveal_hidden_stairs(base_scr, screen, false);
			bool do_layers = true;
			trigger_secrets_for_screen(TriggerSource::SecretsScreenState, screen, false);
		}
		if(game->maps[map*MAPSCRSNORMAL + screen] & mLIGHTBEAM) // if special stuff done before
		{
			for(size_t layer = 0; layer < 7; ++layer)
			{
				mapscr* layer_scr = screens[layer + 1];
				for(size_t pos = 0; pos < 176; ++pos)
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

	int destlvl = DMaps[dmap].level;
	toggle_switches(game->lvlswitches[destlvl], true, base_scr, screen);
	toggle_gswitches_load(base_scr, screen);

	int mi = currmap*MAPSCRSNORMAL + screen;
	bool should_check_for_state_things = (screen < 0x80) && mi < MAXMAPS*MAPSCRSNORMAL;
	if (should_check_for_state_things)
	{
		if (game->maps[mi]&mLOCKBLOCK)
		{
			remove_lockblocks(base_scr, screen);
		}
		
		if (game->maps[mi]&mBOSSLOCKBLOCK)
		{
			remove_bosslockblocks(base_scr, screen);
		}
		
		if (game->maps[mi]&mCHEST)
		{
			remove_chests(base_scr, screen);
		}
		
		if (game->maps[mi]&mLOCKEDCHEST)
		{
			remove_lockedchests(base_scr, screen);
		}
		
		if (game->maps[mi]&mBOSSCHEST)
		{
			remove_bosschests(base_scr, screen);
		}
		
		clear_xdoors_mi(base_scr, screen, mi, true);
		clear_xstatecombos_mi(base_scr, screen, mi, true);
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
				if((ldir^1)==i)
				{
					base_scr->door[i]=dOPENSHUTTER;
				}
				
				opendoors = -4;
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

	auto [offx, offy] = is_a_region(dmap, screen) ?
		translate_screen_coordinates_to_world(screen) :
		std::make_pair(0, 0);
	for (word i = 0; i < MAXFFCS; i++)
	{
		base_scr->ffcs[i].screen = screen;
		base_scr->ffcs[i].x += offx;
		base_scr->ffcs[i].y += offy;
	}
}

// Sets `currscr` to `scr` and loads new screens into temporary memory.
// Called anytime a player moves to a new screen (either via warping, scrolling, continue,
// starting the game, etc...)
// Note: for regions, only the initial screen load calls this function. Simply walking between screens
// in the same region does not use this, because every screen in a region is loaded into temporary memory up front.
// If scr >= 0x80, `heroscr` will be saved to `homescr` and also be loaded into `special_warp_return_screen`.
// If overlay is true, the old tmpscr combos will be copied to the new tmpscr combos on all layers (but only where
// the new screen has a 0 combo).
// TODO: loadscr should set curdmap, but currently callers do that.
void loadscr(int32_t destdmap, int32_t scr, int32_t ldir, bool overlay, bool no_x80_dir)
{
	zapp_reporting_set_tag("screen", scr);
	if (destdmap != -1)
		zapp_reporting_set_tag("dmap", destdmap);

	int32_t orig_destdmap = destdmap;
	if (destdmap < 0) destdmap = currdmap;

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
		replay_step_comment_loadscr(scr);

		// Reset the rngs and frame count so that recording steps can be modified without impacting
		// behavior of later screens.
		replay_sync_rng();
	}

	slopes.clear();
	triggered_screen_secrets = false;
	Hero.clear_platform_ffc();
	timeExitAllGenscript(GENSCR_ST_CHANGE_SCREEN);
	clear_darkroom_bitmaps();

	for (word x=0; x<animated_combos; x++)
	{
		if(combobuf[animated_combo_table4[x][0]].nextcombo!=0)
		{
			combobuf[animated_combo_table4[x][0]].aclk = 0;
		}
	}
	for (word x=0; x<animated_combos2; x++)
	{
		if(combobuf[animated_combo_table24[x][0]].nextcombo!=0)
		{
			combobuf[animated_combo_table24[x][0]].aclk = 0;
		}
	}
	reset_combo_animations2();

	currscr_for_passive_subscr = -1;
	z3_load_region(scr, destdmap);
	homescr = scr >= 0x80 ? heroscr : cur_origin_screen_index;

	cpos_clear_all();
	FFCore.clear_script_engine_data_of_type(ScriptType::Screen);
	FFCore.clear_combo_scripts();
	FFCore.deallocateAllScriptOwnedOfType(ScriptType::Screen);
	FFCore.deallocateAllScriptOwnedOfType(ScriptType::Combo);

	// Load the origin screen (top-left in region) into tmpscr
	loadscr_old(0, orig_destdmap, cur_origin_screen_index, ldir, overlay);
	// Store the current tmpscr into special_warp_return_screen, if on a special screen.
	if (scr >= 0x80)
		loadscr_old(1, orig_destdmap, homescr, no_x80_dir ? -1 : ldir, overlay);

	if (is_z3_scrolling_mode())
	{
		for (int screen = 0; screen < 128; screen++)
		{
			if (screen != cur_origin_screen_index && is_in_current_region(screen))
			{
				load_a_screen_and_layers(destdmap, currmap, screen, ldir);
			}
		}
	}

	// Temp set currdmap so that get_layer_scr -> load_a_screen_and_layers will know if this is a region.
	int o_currdmap = currdmap;
	currdmap = destdmap;

	current_region_screen_count = 0;
	for (int y = 0; y < current_region.screen_height; y++)
	{
		for (int x = 0; x < current_region.screen_width; x++)
		{
			for (int layer = 0; layer <= 6; layer++)
			{
				int screen = cur_origin_screen_index + x + y*16;
				mapscr* scr = get_layer_scr(currmap, screen, layer - 1);
				if (!scr->valid)
				{
					if (layer == 0) break;
					continue;
				}

				rpos_t base_rpos = POS_TO_RPOS(0, z3_get_region_relative_dx(screen), z3_get_region_relative_dy(screen));
				current_region_rpos_handles[current_region_screen_count] = {scr, screen, layer, base_rpos, 0};
				current_region_screen_count += 1;
			}
		}
	}

	update_slope_comboposes();
	currdmap = o_currdmap;
	heroscr = scr;
	hero_screen = get_scr_no_load(currmap, scr);
	CHECK(hero_screen);

	cpos_force_update();
	trig_trigger_groups();

	for_every_ffc([&](const ffc_handle_t& ffc_handle) {
		// Handled in loadscr_old.
		if (ffc_handle.screen == cur_origin_screen_index)
			return;

		FFCore.reset_script_engine_data(ScriptType::FFC, ffc_handle.id);
		memset(ffc_handle.ffc->script_misc, 0, 16 * sizeof(int32_t));
	});

	// "extended height mode" includes the top 56 pixels as part of the visible mapscr viewport,
	// allowing for regions to display 4 more rows of combos (as many as ALTTP does). This part of
	// screen is normally reserved for the passive subscreen, but in this mode mapscr combos are drawn below it.
	// It is up to the quest designer to make their subscreen be actually transparent.
	//
	// When not in "extended height mode" (otherwise 56 is 0):
	//  - playing_field_offset: 56-ish, but changes during earthquakes
	//  - original_playing_field_offset: always 56
	//
	// These values are used to adjust where things are drawn on screen to account for the passive subscreen. Examples:
	// - yofs of sprites
	// - bitmap y offsets in draw_screen
	// - drawing offsets for putscr, do_layer
	// - drawing offsets for various calls to overtile16 (see bomb weapon explosion)
	// - lots
	//
	// TODO z3 !!! maybe instead- make yofs start as 0 by default, and add playing_field_offset at draw time?
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

	// TODO z3 !!!! per-screen?
	room_is_dark = (tmpscr->flags & fDARK);

	game->load_portal();
	throwGenScriptEvent(GENSCR_EVENT_CHANGE_SCREEN);
	if (Hero.lift_wpn && get_qr(qr_CARRYABLE_NO_ACROSS_SCREEN))
	{
		delete Hero.lift_wpn;
		Hero.lift_wpn = nullptr;
	}
}

// Don't use this directly!
// Some stuff needs to be refactored before this function can be removed:
//    - remove tmpscr, tmpscr2, etc. Just store these things in the larger temporary screen vectors.
//      (this is hard)
//    - do the "overlay" logic (but just for tmpscr, not every single screen in a region) in
//      load_a_screen_and_layers
void loadscr_old(int32_t tmp,int32_t destdmap, int32_t screen,int32_t ldir,bool overlay)
{
	bool is_setting_special_warp_return_screen = tmp == 1;
	int32_t destlvl = DMaps[destdmap < 0 ? currdmap : destdmap].level;

	mapscr previous_scr = tmp == 0 ? *tmpscr : special_warp_return_screen;
	mapscr* scr = tmp == 0 ? tmpscr : &special_warp_return_screen;
	*scr = TheMaps[currmap*MAPSCRS+screen];
	if (tmp == 0)
		hero_screen = scr;
	if (!tmp)
		for (uint8_t i = 0; i < MAXFFCS; ++i)
		{
			scr->ffcs[i].setLoaded(true);
			scr->ffcs[i].solid_update(false);
			screen_ffc_modify_postroutine({scr, (uint8_t)screen, i, i, &scr->ffcs[i]});
		}
	
	scr->valid |= mVALID; //layer 0 is always valid
	memcpy(scr->data, TheMaps[currmap*MAPSCRS+screen].data, sizeof(scr->data));
	memcpy(scr->sflag, TheMaps[currmap*MAPSCRS+screen].sflag, sizeof(scr->sflag));
	memcpy(scr->cset, TheMaps[currmap*MAPSCRS+screen].cset, sizeof(scr->cset));

	if ( TheMaps[currmap*MAPSCRS+screen].script > 0 )
	{
		scr->script = TheMaps[currmap*MAPSCRS+screen].script;
		al_trace("The screen script id is: %d \n", TheMaps[currmap*MAPSCRS+screen].script);
		for ( int32_t q = 0; q < 8; q++ )
		{
			scr->screeninitd[q] = TheMaps[currmap*MAPSCRS+screen].screeninitd[q];
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
	
	// Carry over FFCs from previous screen to the next. Only for the ones that have the carry over flag set, and if the previous
	// screen has FF carryover enabled.
	if (!is_setting_special_warp_return_screen)
	{
		for(word i = 0; i < MAXFFCS; i++)
		{
			if((previous_scr.ffcs[i].flags&ffCARRYOVER) && !(previous_scr.flags5&fNOFFCARRYOVER))
			{
				scr->ffcs[i] = previous_scr.ffcs[i];
				
				if (previous_scr.ffcs[i].flags&ffSCRIPTRESET)
				{
					int ffc_id = get_region_screen_index_offset(screen)*MAXFFCS + i;
					FFCore.reset_script_engine_data(ScriptType::FFC, ffc_id);
				}
			}
			else
			{
				int ffc_id = get_region_screen_index_offset(screen)*MAXFFCS + i;
				FFCore.deallocateAllScriptOwned(ScriptType::FFC, ffc_id, false);
				memset(scr->ffcs[i].script_misc, 0, 16 * sizeof(int32_t));
				FFCore.reset_script_engine_data(ScriptType::FFC, ffc_id);
			}
		}

		for(int32_t i=0; i<6; i++)
		{
			mapscr layerscr = tmpscr2[i];
			
			// Don't delete the old tmpscr2's data yet!
			if(scr->layermap[i]>0)
			{
				tmpscr2[i]=TheMaps[(scr->layermap[i]-1)*MAPSCRS+scr->layerscreen[i]];
				
				if(overlay)
				{
					for(int32_t y=0; y<11; ++y)
					{
						for(int32_t x=0; x<16; ++x)
						{
							int32_t c=y*16+x;
							
							if(tmpscr2[i].data[c]==0)
							{
								tmpscr2[i].data[c]=layerscr.data[c];
								tmpscr2[i].sflag[c]=layerscr.sflag[c];
								tmpscr2[i].cset[c]=layerscr.cset[c];
							}
						}
					}
				}
			}
			else
			{
				(tmpscr2+i)->zero_memory();
			}
		}
	}

	auto [offx, offy] = is_a_region(destdmap < 0 ? currdmap : destdmap, screen) ?
		translate_screen_coordinates_to_world(screen) :
		std::make_pair(0, 0);
	for (word i = 0; i < MAXFFCS; i++)
	{
		scr->ffcs[i].screen = screen;
		scr->ffcs[i].x += offx;
		scr->ffcs[i].y += offy;
	}

	if (!tmp)
	{
		// cpos_force_update();
		// trig_trigger_groups();
	}

	// Apply perm secrets, if applicable.
	if(canPermSecret(destdmap,screen)/*||TheMaps[(currmap*MAPSCRS)+currscr].flags6&fTRIGGERFPERM*/)
	{
		if(game->maps[(currmap*MAPSCRSNORMAL)+screen]&mSECRET)			   // if special stuff done before
		{
			reveal_hidden_stairs(scr, screen, false);
			trigger_secrets_for_screen(SecretsScreenState, currscr, tmp == 0 ? tmpscr : &special_warp_return_screen, false, -1);
		}
		if(game->maps[(currmap*MAPSCRSNORMAL)+screen]&mLIGHTBEAM) // if special stuff done before
		{
			for(size_t layer = 0; layer < 7; ++layer)
			{
				mapscr* tscr = (tmp==0) ? FFCore.tempScreens[layer] : FFCore.ScrollingScreens[layer];
				for(size_t pos = 0; pos < 176; ++pos)
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
	
	toggle_switches(game->lvlswitches[destlvl], true, tmp == 0 ? tmpscr : &special_warp_return_screen, tmp == 0 ? cur_origin_screen_index : homescr);
	toggle_gswitches_load(tmp == 0 ? tmpscr : &special_warp_return_screen, tmp == 0 ? cur_origin_screen_index : homescr);
	
	if(game->maps[(currmap*MAPSCRSNORMAL)+screen]&mLOCKBLOCK)			  // if special stuff done before
	{
		remove_lockblocks(scr, screen);
	}
	
	if(game->maps[(currmap*MAPSCRSNORMAL)+screen]&mBOSSLOCKBLOCK)		  // if special stuff done before
	{
		remove_bosslockblocks(scr, screen);
	}
	
	if(game->maps[(currmap*MAPSCRSNORMAL)+screen]&mCHEST)			  // if special stuff done before
	{
		remove_chests(scr, screen);
	}
	
	if(game->maps[(currmap*MAPSCRSNORMAL)+screen]&mLOCKEDCHEST)			  // if special stuff done before
	{
		remove_lockedchests(scr, screen);
	}
	
	if(game->maps[(currmap*MAPSCRSNORMAL)+screen]&mBOSSCHEST)			  // if special stuff done before
	{
		remove_bosschests(scr, screen);
	}
	
	clear_xdoors(scr, screen, true);
	clear_xstatecombos(scr, screen, true);

	if (!tmp)
	{
		// cpos_force_update();
		// trig_trigger_groups();
	}

	// check doors
	if(isdungeon(destdmap,screen))
	{
		for(int32_t i=0; i<4; i++)
		{
			int32_t door=scr->door[i];
			
			switch(door)
			{
			case d1WAYSHUTTER:
			case dSHUTTER:
				if((ldir^1)==i)
				{
					scr->door[i]=dOPENSHUTTER;
				}
				
				opendoors = -4;
				break;
				
			case dLOCKED:
				if(game->maps[(currmap*MAPSCRSNORMAL)+screen]&(1<<i))
				{
					scr->door[i]=dUNLOCKED;
				}
				
				break;
				
			case dBOSS:
				if(game->maps[(currmap*MAPSCRSNORMAL)+screen]&(1<<i))
				{
					scr->door[i]=dOPENBOSS;
				}
				
				break;
				
			case dBOMB:
				if(game->maps[(currmap*MAPSCRSNORMAL)+screen]&(1<<i))
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

	// if (tmp == 0)
	// 	update_slope_comboposes();
	
	for(int32_t j=-1; j<6; ++j)  // j == -1 denotes the current screen
	{
		if (j<0 || scr->layermap[j] > 0)
		{
			mapscr *layerscreen= (j<0 ? scr : tmpscr2[j].valid ? &tmpscr2[j] :
								  &TheMaps[(scr->layermap[j]-1)*MAPSCRS]+scr->layerscreen[j]);
								  
			for(int32_t i=0; i<176; ++i)
			{
				int32_t c=layerscreen->data[i];
				int32_t cs=layerscreen->cset[i];
				
				// New screen flag: Cycle Combos At Screen Init
				if(combobuf[c].nextcombo != 0 && (scr->flags3 & fCYCLEONINIT) && (j<0 || get_qr(qr_CMBCYCLELAYERS)))
				{
					int32_t r = 0;
					
					while(combobuf[c].nextcombo != 0 && r++ < 10)
					{
						layerscreen->data[i] = combobuf[c].nextcombo;
						if(!(combobuf[c].animflags & AF_CYCLENOCSET))
							layerscreen->cset[i] = combobuf[c].nextcset;
						c=layerscreen->data[i];
						cs=layerscreen->cset[i];
					}
				}
			}
		}
	}
}

// Screen is being viewed by the Overworld Map viewer.
void loadscr2(int32_t tmp,int32_t scr,int32_t)
{
	auto oscr = homescr;
	homescr = scr;

	for(word x=0; x<animated_combos; x++)
	{
		if(combobuf[animated_combo_table4[x][0]].nextcombo!=0)
		{
			combobuf[animated_combo_table4[x][0]].aclk=0;
		}
	}
	
	mapscr& screen = tmp == 0 ? *tmpscr : special_warp_return_screen;
	screen = TheMaps[currmap*MAPSCRS+scr];
	
	if(tmp==0)
	{
		for(int32_t i=0; i<6; i++)
		{
			if(screen.layermap[i]>0)
			{
				tmpscr2[i]=TheMaps[(screen.layermap[i]-1)*MAPSCRS+screen.layerscreen[i]];
			}
			else
			{
				(tmpscr2+i)->zero_memory();
			}
		}
	}
	
	if(canPermSecret(-1,scr))
	{
		if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mSECRET)			   // if special stuff done before
		{
			reveal_hidden_stairs(&screen, scr, false);
			trigger_secrets_for_screen_internal(-1, tmp == 0 ? tmpscr2 : &special_warp_return_screen, false, false, -1);
		}
		if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mLIGHTBEAM) // if special stuff done before
		{
			for(size_t layer = 0; layer < 7; ++layer)
			{
				mapscr* tscr = (tmp==0) ? FFCore.tempScreens[layer] : FFCore.ScrollingScreens[layer];
				for(size_t pos = 0; pos < 176; ++pos)
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
	
	if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mLOCKBLOCK)			  // if special stuff done before
	{
		remove_lockblocks(&screen, scr);
	}
	
	if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mBOSSLOCKBLOCK)		  // if special stuff done before
	{
		remove_bosslockblocks(&screen, scr);
	}
	
	if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mCHEST)			  // if special stuff done before
	{
		remove_chests(&screen, scr);
	}
	
	if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mLOCKEDCHEST)			  // if special stuff done before
	{
		remove_lockedchests(&screen, scr);
	}
	
	if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mBOSSCHEST)			  // if special stuff done before
	{
		remove_bosschests(&screen, scr);
	}
	
	clear_xdoors(&screen, scr);
	clear_xstatecombos(&screen, scr);
	
	// check doors
	if(isdungeon(scr))
	{
		for(int32_t i=0; i<4; i++)
		{
			int32_t door=screen.door[i];
			bool putit=true;
			
			switch(door)
			{
			case d1WAYSHUTTER:
			case dSHUTTER:
				/*
						if((ldir^1)==i)
						{
						  screen.door[i]=dOPENSHUTTER;
						  //		  putit=false;
						}
				*/
				break;
				
			case dLOCKED:
				if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&(1<<i))
				{
					screen.door[i]=dUNLOCKED;
					//		  putit=false;
				}
				
				break;
				
			case dBOSS:
				if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&(1<<i))
				{
					screen.door[i]=dOPENBOSS;
					//		  putit=false;
				}
				
				break;
				
			case dBOMB:
				if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&(1<<i))
				{
					screen.door[i]=dBOMBED;
				}
				
				break;
			}
			
			if(putit)
			{
				putdoor(scrollbuf,tmp,i,screen.door[i],false);
			}
			
			if(door==dSHUTTER||door==d1WAYSHUTTER)
			{
				screen.door[i]=door;
			}
		}
	}
	
	for(int32_t j=-1; j<6; ++j)  // j == -1 denotes the current screen
	{
		if (j < 0 || screen.layermap[j] > 0)
		{
			mapscr *layerscreen= (j<0 ? &screen
								  : &(TheMaps[(screen.layermap[j]-1)*MAPSCRS+screen.layerscreen[j]]));
								  
			for(int32_t i=0; i<176; ++i)
			{
				int32_t c=layerscreen->data[i];
				int32_t cs=layerscreen->cset[i];
				
				// New screen flag: Cycle Combos At Screen Init
				if((screen.flags3 & fCYCLEONINIT) && (j<0 || get_qr(qr_CMBCYCLELAYERS)))
				{
					int32_t r = 0;
					
					while(combobuf[c].nextcombo != 0 && r++ < 10)
					{
						layerscreen->data[i] = combobuf[c].nextcombo;
						if(!(combobuf[c].animflags & AF_CYCLENOCSET))
							layerscreen->cset[i] = combobuf[c].nextcset;
						c=layerscreen->data[i];
						cs=layerscreen->cset[i];
					}
				}
			}
		}
	}

	homescr = oscr;
}

void putscr(BITMAP* dest,int32_t x,int32_t y, mapscr* scr)
{
	// This is a bogus value while screenscrolling == true, but that's ok
	// because it is only used to calculate the rpos, and during screenscrolling
	// only the modulus to get pos (draw_cmb_pos does RPOS_TO_POS) is needed, which
	// is always the same no matter the value of scr.
	int screen = get_screen_index_for_world_xy(x, y);

	x -= viewport.x;
	y -= viewport.y;

	if(scr->valid==0||!show_layer_0||scr->hidelayers & 1)
	{
		rectfill(dest,x,y,x+255,y+175,0);
		return;
	}
	
	bool over = XOR(scr->flags7&fLAYER2BG,DMaps[currdmap].flags&dmfLAYER2BG)
		|| XOR(scr->flags7&fLAYER3BG, DMaps[currdmap].flags&dmfLAYER3BG);

	int start_x, end_x, start_y, end_y;
	get_bounds_for_draw_cmb_calls(dest, x, y, start_x, end_x, start_y, end_y);
	for (int cy = start_y; cy < end_y; cy++)
	{
		for (int cx = start_x; cx < end_x; cx++)
		{
			int i = cx + cy*16;
			auto rpos = screenscrolling ? rpos_t::None : POS_TO_RPOS(i, screen);
			draw_cmb_pos(dest, x + COMBOX(i), y + COMBOY(i), rpos, scr->data[i], scr->cset[i], 0, over, false);
		}
	}
}

void putscrdoors(BITMAP *dest,int32_t x,int32_t y)
{
	if (!show_layer_0)
	{
		return;
	}
	
	x -= viewport.x;
	y -= viewport.y;

	for_every_nearby_screen([&](std::array<screen_handle_t, 7> screen_handles, int screen, int offx, int offy) {
		mapscr* scr = screen_handles[0].base_scr;
		if (scr->valid==0)
			return;

		if(scr->door[0]==dBOMBED)
		{
			over_door_new(dest, 39, up, scr->door_combo_set, offx+x, offy+y);
		}
		
		if(scr->door[1]==dBOMBED)
		{
			over_door_new(dest, 135, down, scr->door_combo_set, offx+x, offy+y);
		}
		
		if(scr->door[2]==dBOMBED)
		{
			over_door_new(dest, 66, left, scr->door_combo_set, offx+x, offy+y);
		}
		
		if(scr->door[3]==dBOMBED)
		{
			over_door_new(dest, 77, right, scr->door_combo_set, offx+x, offy+y);
		}
	});
}

void putscrdoors(BITMAP *dest,int32_t x,int32_t y, mapscr* scrn)
{
	if(scrn->valid==0||!show_layer_0)
	{
		return;
	}
	
	x -= viewport.x;
	y -= viewport.y;
	
	if(scrn->door[0]==dBOMBED)
	{
		over_door(dest,0,39,up,x,y);
	}
	
	if(scrn->door[1]==dBOMBED)
	{
		over_door(dest,0,135,down,x,y);
	}
	
	if(scrn->door[2]==dBOMBED)
	{
		over_door(dest,0,66,left,x,y);
	}
	
	if(scrn->door[3]==dBOMBED)
	{
		over_door(dest,0,77,right,x,y);
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
	int32_t bx = COMBOPOS(x % 256, y % 176);
	const newcombo& c = combobuf[s0->data[bx]];
	const newcombo& c1 = combobuf[s1->data[bx]];
	const newcombo& c2 = combobuf[s2->data[bx]];
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
	mapscr* s0 = get_screen_layer_for_xy_offset(x, y, 0);
	mapscr* s1 = get_screen_layer_for_xy_offset(x, y, 1);
	mapscr* s2 = get_screen_layer_for_xy_offset(x, y, 2);
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
	mapscr* s0 = get_screen_for_world_xy(x, y);
	mapscr* s1 = get_screen_layer_for_xy_offset(x, y, 1);
	mapscr* s2 = get_screen_layer_for_xy_offset(x, y, 2);
	if (!s1->valid) s1 = s0;
	if (!s2->valid) s2 = s0;

	int32_t bx = COMBOPOS(x % 256, y % 176);
	const newcombo& c = combobuf[s0->data[bx]];
	const newcombo& c1 = combobuf[s1->data[bx]];
	const newcombo& c2 = combobuf[s2->data[bx]];
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
	mapscr* m = get_layer_scr_for_xy(x, y, layer);
	if (m->valid == 0) return false;
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
	
	int32_t bx = COMBOPOS(x%256, y%176);
	const newcombo* c = &combobuf[m->data[bx]];
	bool dried = ((iswater_type(c->type)) && DRIEDLAKE);
	int32_t b=1;
	
	if(x&8) b<<=2;
	
	if(y&8) b<<=1;
	
	if((c->walk&b) && !dried)
		return true;
		
	if(cnt==1) return false;
	
	++bx;
	
	if(!(x&8))
		b<<=2;
	else
	{
		c  = &combobuf[m->data[bx]];
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
	mapscr* m = get_layer_scr_for_xy(x, y, layer);
	if (m->valid == 0) return false;
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
	
	int32_t bx = COMBOPOS(x%256, y%176);
	const newcombo* c = &combobuf[m->data[bx]];
	bool dried = ((iswater_type(c->type)) && DRIEDLAKE);
	int32_t b=1;
	
	if(x&8) b<<=2;
	
	if(y&8) b<<=1;
	
	if(((c->walk>>4)&b) && !dried)
		return true;
		
	if(cnt==1) return false;
	
	++bx;
	
	if(!(x&8))
		b<<=2;
	else
	{
		c  = &combobuf[m->data[bx]];
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
	if(x<0 || y<0 || x>=world_w || y>=world_h)
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
		cont_sfx(tmpscr->oceansfx);
		
		if(tmpscr->bosssfx && !(game->lvlitems[dlevel]&liBOSS))
			cont_sfx(tmpscr->bosssfx);
	}
	else
	{
		adjust_sfx(tmpscr->oceansfx,128,false);
		adjust_sfx(tmpscr->bosssfx,128,false);
		
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

	for_every_screen_in_region([&](mapscr* scr, int screen, unsigned int region_scr_x, unsigned int region_scr_y) {
		toggle_switches(flags, entry, scr, screen);
	});
}
void toggle_switches(dword flags, bool entry, mapscr* m, int screen)
{
	if(!flags) return; //No flags to toggle
	bool iscurscr = m==tmpscr;

	for_every_rpos_in_screen(m, screen, [&](const rpos_handle_t& rpos_handle) {
		byte togglegrid[176] = {0};
		mapscr* scr = rpos_handle.scr;
		int lyr = rpos_handle.layer;
		int pos = rpos_handle.pos;
		newcombo const& cmb = combobuf[scr->data[pos]];
		if(iscurscr)
			if((cmb.triggerflags[3] & combotriggerTRIGLEVELSTATE) && cmb.trig_lstate < 32)
				if(flags&(1<<cmb.trig_lstate))
					do_trigger_combo(rpos_handle, ctrigSWITCHSTATE);
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
					while(tmp->nextcombo && (oldData.find(tmp->nextcombo) == oldData.end()))
					{
						scr->data[pos] = tmp->nextcombo;
						if(!(tmp->animflags & AF_CYCLENOCSET))
							scr->cset[pos] = tmp->nextcset;
						oldData.insert(tmp->nextcombo);
						tmp = &combobuf[tmp->nextcombo];
					}
				}
				int32_t cmbid = scr->data[pos];
				if(combobuf[cmbid].animflags & AF_CYCLE)
				{
					combobuf[cmbid].tile = combobuf[cmbid].o_tile;
					combobuf[cmbid].cur_frame=0;
					combobuf[cmbid].aclk = 0;
				}
				togglegrid[pos] |= (1<<lyr); //Mark this pos toggled for this layer
				if(cmb.type == cCSWITCH) return; //Switches don't toggle other layers
				for(int32_t lyr2 = 0; lyr2 < 7; ++lyr2) //Toggle same pos on other layers, if flag set
				{
					if(lyr==lyr2) return;
					if(!(cmb.usrflags&(1<<lyr2))) return;
					if(togglegrid[pos]&(1<<lyr2)) return;
					mapscr* scr_2 = (lyr2 ? get_layer_scr(currmap, screen, lyr2 - 1) : m);
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
						while(tmp->nextcombo && (oldData2.find(tmp->nextcombo) == oldData2.end()))
						{
							scr_2->data[pos] = tmp->nextcombo;
							if(!(tmp->animflags & AF_CYCLENOCSET))
								scr_2->cset[pos] = tmp->nextcset;
							oldData2.insert(tmp->nextcombo);
							tmp = &combobuf[tmp->nextcombo];
						}
					}
					int32_t cmbid2 = scr_2->data[pos];
					if(combobuf[cmbid2].animflags & AF_CYCLE)
					{
						combobuf[cmbid2].tile = combobuf[cmbid2].o_tile;
						combobuf[cmbid2].cur_frame=0;
						combobuf[cmbid2].aclk = 0;
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
				}
			}
		}
	}
	
	if(iscurscr)
	{
		int screen_index_offset = get_region_screen_index_offset(screen);
		word c = m->numFFC();
		for (uint8_t q = 0; q < c; ++q)
		{
			uint16_t ffc_id = screen_index_offset * MAXFFCS + q;
			newcombo const& cmb = combobuf[m->ffcs[q].data];
			if((cmb.triggerflags[3] & combotriggerTRIGLEVELSTATE) && cmb.trig_lstate < 32)
				if(flags&(1<<cmb.trig_lstate))
					do_trigger_combo_ffc({m, (uint8_t)screen, ffc_id, q, &m->ffcs[q]}, ctrigSWITCHSTATE);
		}
	}
}

void toggle_gswitches(int32_t state, bool entry)
{
	for_every_screen_in_region([&](mapscr* scr, int screen, unsigned int region_scr_x, unsigned int region_scr_y) {
		toggle_gswitches(state, entry, scr, screen);
	});
}
void toggle_gswitches(int32_t state, bool entry, mapscr* base_scr, int screen)
{
	bool states[256] = {false};
	states[state] = true;
	toggle_gswitches(states, entry, base_scr, screen);
}
void toggle_gswitches(bool* states, bool entry, mapscr* base_scr, int screen)
{
	if(!states) return;
	bool iscurscr = base_scr==tmpscr;
	byte togglegrid[176] = {0};
	for(int32_t lyr = 0; lyr < 7; ++lyr)
	{
		mapscr* scr = lyr == 0 ? base_scr : get_layer_scr(currmap, screen, lyr - 1);
		for(int32_t pos = 0; pos < 176; ++pos)
		{
			newcombo const& cmb = combobuf[scr->data[pos]];
			if(iscurscr)
				if(cmb.triggerflags[3] & combotriggerTRIGGLOBALSTATE)
					if(states[cmb.trig_gstate])
						do_trigger_combo(lyr,pos,ctrigSWITCHSTATE);
			if(!(cmb.usrflags & cflag11)) //not global state
				continue;
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
						while(tmp->nextcombo && (oldData.find(tmp->nextcombo) == oldData.end()))
						{
							scr->data[pos] = tmp->nextcombo;
							if(!(tmp->animflags & AF_CYCLENOCSET))
								scr->cset[pos] = tmp->nextcset;
							oldData.insert(tmp->nextcombo);
							tmp = &combobuf[tmp->nextcombo];
						}
					}
					int32_t cmbid = scr->data[pos];
					if(combobuf[cmbid].animflags & AF_CYCLE)
					{
						combobuf[cmbid].tile = combobuf[cmbid].o_tile;
						combobuf[cmbid].cur_frame=0;
						combobuf[cmbid].aclk = 0;
					}
					togglegrid[pos] |= (1<<lyr); //Mark this pos toggled for this layer
					if(cmb.type == cCSWITCH) continue; //Switches don't toggle other layers
					for(int32_t lyr2 = 0; lyr2 < 7; ++lyr2) //Toggle same pos on other layers, if flag set
					{
						if(lyr==lyr2) continue;
						if(!(cmb.usrflags&(1<<lyr2))) continue;
						if(togglegrid[pos]&(1<<lyr2)) continue;
						mapscr* scr_2 = lyr2 == 0 ? base_scr : get_layer_scr(currmap, screen, lyr2 - 1);
						if(!scr_2->data[pos]) //Don't increment empty space
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
							while(tmp->nextcombo && (oldData2.find(tmp->nextcombo) == oldData2.end()))
							{
								scr_2->data[pos] = tmp->nextcombo;
								if(!(tmp->animflags & AF_CYCLENOCSET))
									scr_2->cset[pos] = tmp->nextcset;
								oldData2.insert(tmp->nextcombo);
								tmp = &combobuf[tmp->nextcombo];
							}
						}
						int32_t cmbid2 = scr_2->data[pos];
						if(combobuf[cmbid2].animflags & AF_CYCLE)
						{
							combobuf[cmbid2].tile = combobuf[cmbid2].o_tile;
							combobuf[cmbid2].cur_frame=0;
							combobuf[cmbid2].aclk = 0;
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
				}
			}
		}
	}
	
	if(iscurscr)
	{
		word c = base_scr->numFFC();
		int screen_index_offset = get_region_screen_index_offset(screen);
		for (uint8_t q = 0; q < c; ++q)
		{
			newcombo const& cmb = combobuf[base_scr->ffcs[q].data];
			uint16_t ffc_id = screen_index_offset * MAXFFCS + q;
			if(cmb.triggerflags[3] & combotriggerTRIGGLOBALSTATE)
				if(states[cmb.trig_gstate])
					do_trigger_combo_ffc({base_scr, (uint8_t)screen, ffc_id, q, &base_scr->ffcs[q]}, ctrigSWITCHSTATE);
		}
	}
}
void toggle_gswitches_load(mapscr* base_scr, int screen)
{
	bool states[256];
	for(auto q = 0; q < 256; ++q)
	{
		states[q] = game->gswitch_timers[q] != 0;
	}
	toggle_gswitches(states, true, base_scr, screen);
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
	for_every_screen_in_region([&](mapscr* scr, int screen, unsigned int region_scr_x, unsigned int region_scr_y) {
		toggle_gswitches(states, false, scr, screen);
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

//BITMAP *mappic = NULL;
int32_t mapres = 0;
int32_t view_map_show_mode = 3;

bool displayOnMap(int32_t x, int32_t y)
{
    int32_t s = (y<<4) + x;
    if(!(game->maps[(currmap*MAPSCRSNORMAL)+s]&mVISITED))
        return false;

    // Don't display if not part of DMap
    if(((DMaps[currdmap].flags&dmfDMAPMAP) &&
       (DMaps[currdmap].type != dmOVERW) &&
       !(x >= DMaps[currdmap].xoff &&
         x < DMaps[currdmap].xoff+8 &&
         DMaps[currdmap].grid[y]&(128>>(x-DMaps[currdmap].xoff)))))
        return false;
    else
        return true;
}

void ViewMap()
{
	// The only reason this is static is that the stack is otherwise too large.
	// It will still compile, but segfaults in this function.
	// Ex: python tests/run_replay_tests.py --filter link_to_the_zelda_2_of_3.zplay --frame 3000
	static mapscr tmpscr_a[2];
	static mapscr tmpscr_b[2];
	static mapscr tmpscr_c[6];

	tmpscr_a[0] = *tmpscr;
	tmpscr->zero_memory();
	tmpscr_a[1] = special_warp_return_screen;
	special_warp_return_screen.zero_memory();
	
	for(int32_t i=0; i<6; ++i)
	{
		tmpscr_c[i] = tmpscr2[i];
		tmpscr2[i].zero_memory();
	}
	
	BITMAP* mappic = NULL;
	static double scales[17] =
	{
		0.03125, 0.04419, 0.0625, 0.08839, 0.125, 0.177, 0.25, 0.3535,
		0.50, 0.707, 1.0, 1.414, 2.0, 2.828, 4.0, 5.657, 8.0
	};
	
	int32_t px = ((8-(currscr&15)) << 9)  - 256;
	int32_t py = ((4-(currscr>>4)) * 352) - 176;
	int32_t lx = ((currscr&15)<<8)  + HeroX()+8;
	int32_t ly = ((currscr>>4)*176) + HeroY()+8;
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

	// draw the map
	BITMAP* screen_bmp = create_bitmap_ex(8, 256, 176);
	viewport.x = 0;
	viewport.y = 0;
	combotile_add_x = 256;
	combotile_add_y = 0;
	for(int32_t y=0; y<8; y++)
	{
		for(int32_t x=0; x<16; x++)
		{
			clear_to_color(screen_bmp, WHITE);
			if(displayOnMap(x, y))
			{
				int32_t s = (y<<4) + x;
				tmpscr->zero_memory();
				special_warp_return_screen.zero_memory();
				loadscr2(1,s,-1);
				*tmpscr = special_warp_return_screen;
				if(tmpscr->valid&mVALID)
				{
					for(int32_t i=0; i<6; i++)
					{
						tmpscr2[i].zero_memory();
						if(tmpscr->layermap[i]<=0)
							continue;
						
						tmpscr2[i]=TheMaps[(tmpscr->layermap[i]-1)*MAPSCRS+tmpscr->layerscreen[i]];
					}

					int xx = 0;
					int yy = -playing_field_offset;
					
					if(XOR(tmpscr->flags7&fLAYER2BG, DMaps[currdmap].flags&dmfLAYER2BG)) do_layer_old(screen_bmp, 0, 2, tmpscr, xx, yy, 2);
					
					if(XOR(tmpscr->flags7&fLAYER3BG, DMaps[currdmap].flags&dmfLAYER3BG)) do_layer_old(screen_bmp, 0, 3, tmpscr, xx, yy, 2);
					
					if(lenscheck(tmpscr,0)) putscr(screen_bmp,0,0,tmpscr);
					do_layer_old(screen_bmp, 0, 1, tmpscr, xx, yy, 2);
					
					if(!XOR((tmpscr->flags7&fLAYER2BG), DMaps[currdmap].flags&dmfLAYER2BG)) do_layer_old(screen_bmp, 0, 2, tmpscr, xx, yy, 2);
					
					putscrdoors(screen_bmp,0,0,tmpscr);
					if (get_qr(qr_PUSHBLOCK_SPRITE_LAYER))
					{
						do_layer_old(screen_bmp,-2, 0, tmpscr, xx, yy, 2);
						if(get_qr(qr_PUSHBLOCK_LAYER_1_2))
						{
							do_layer_old(screen_bmp,-2, 1, tmpscr, xx, yy, 2);
							do_layer_old(screen_bmp,-2, 2, tmpscr, xx, yy, 2);
						}
					}
					do_layer_old(screen_bmp,-3, 0, tmpscr, xx, yy, 2); // Freeform combos!
					
					if(!XOR((tmpscr->flags7&fLAYER3BG), DMaps[currdmap].flags&dmfLAYER3BG)) do_layer_old(screen_bmp, 0, 3, tmpscr, xx, yy, 2);
					
					do_layer_old(screen_bmp, 0, 4, tmpscr, xx, yy, 2);
					do_layer_old(screen_bmp,-1, 0, tmpscr, xx, yy, 2);
					if(get_qr(qr_OVERHEAD_COMBOS_L1_L2))
					{
						do_layer_old(screen_bmp,-1, 1, tmpscr, xx, yy, 2);
						do_layer_old(screen_bmp,-1, 2, tmpscr, xx, yy, 2);
					}
					do_layer_old(screen_bmp, 0, 5, tmpscr, xx, yy, 2);
					do_layer_old(screen_bmp, 0, 6, tmpscr, xx, yy, 2);
				}
			}
			
			stretch_blit(screen_bmp, mappic, 0, 0, 256, 176, x<<(8-mapres), (y*176)>>mapres, 256>>mapres, 176>>mapres);
		}
	}
	
	*tmpscr = tmpscr_a[0];
	special_warp_return_screen = tmpscr_a[1];
	for(int32_t i=0; i<6; ++i)
	{
		tmpscr2[i]=tmpscr_c[i];
	}
	
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
			blit(scrollbuf_old,framebuf,256,0,0,0,256,224);
		}
		else
		{
			clear_to_color(framebuf,BLACK);
			stretch_blit(mappic,framebuf,0,0,mappic->w,mappic->h,
						 int32_t(256+(int64_t(px)-mappic->w)*scale)/2,int32_t(224+(int64_t(py)-mappic->h)*scale)/2,
						 int32_t(mappic->w*scale),int32_t(mappic->h*scale));
						 
			blit(framebuf,scrollbuf_old,0,0,256,0,256,224);
			redraw=false;
		}
		
		int32_t x = int32_t(256+(px-((2048-int64_t(lx))*2))*scale)/2;
		int32_t y = int32_t(224+(py-((704-int64_t(ly))*2))*scale)/2;
		
		if(view_map_show_mode&1)
		{
			line(framebuf,x-7,y-7,x+7,y+7,(frame&3)+252);
			line(framebuf,x+7,y-7,x-7,y+7,(frame&3)+252);
		}
		
		//    text_mode(BLACK);
		
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
	
	destroy_bitmap(mappic);
	combotile_add_x = 0;
	combotile_add_y = 0;
	
	resume_all_sfx();
}

int32_t onViewMap()
{
    if(Playing && currscr<128 && DMaps[currdmap].flags&dmfVIEWMAP)
    {
        clear_to_color(framebuf,BLACK);
        //      text_mode(BLACK);
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

static std::map<int, ScreenItemState> screen_item_state;

ScreenItemState screen_item_get_state(int screen)
{
	auto it = screen_item_state.find(screen);
	return it == screen_item_state.end() ? ScreenItemState::None : it->second;
}

void screen_item_set_state(int screen, ScreenItemState state)
{
	screen_item_state[screen] = state;
}

void screen_item_clear_state(int screen)
{
	screen_item_state[screen] = ScreenItemState::None;
}

void screen_item_clear_state()
{
	screen_item_state.clear();
}

optional<int32_t> get_combo(int x, int y, int maxlayer, bool ff, std::function<bool(newcombo const&)> proc)
{
	if(unsigned(x) >= 256 || unsigned(y) >= 176)
		return nullopt;
	if(ff)
	{
		int ffcid = MAPFFCOMBO(x,y);
		if(ffcid && proc(combobuf[ffcid]))
			return ffcid;
	}
	if(maxlayer > 6)
		maxlayer = 6;
	int pos = COMBOPOS(x,y);
	for(int lyr = maxlayer; lyr >= 0; --lyr)
	{
		mapscr* m = FFCore.tempScreens[lyr];
		int cid = m->data[pos];
		newcombo const& cmb = combobuf[cid];
		if(_effectflag_layer(x,y,1,m,true) && proc(cmb))
		{
			for(int i = lyr; i <=1; ++i)
			{
				if(!tmpscr2[i].valid) continue;
				
				auto tcid = MAPCOMBO2(i,x,y);
				if(combobuf[tcid].type == cBRIDGE)
				{
					if (get_qr(qr_OLD_BRIDGE_COMBOS))
					{
						if (!_walkflag_layer(x,y,1,&(tmpscr2[i]))) return nullopt;
					}
					else
					{
						if (_effectflag_layer(x,y,1,&(tmpscr2[i]), true)) return nullopt;
					}
				}
			}
			return cid;
		}
	}
	return nullopt;
}
