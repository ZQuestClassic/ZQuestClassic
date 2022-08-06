//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  maps.cc
//
//  Map for zelda.cc
//
//--------------------------------------------------------

#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include "precompiled.h" //always first

#include <string.h>
#include <assert.h>
#include <math.h>
#include <vector>
#include <deque>
#include <string>
#include <set>
using std::set;

#include "maps.h"
#include "zelda.h"
#include "tiles.h"
#include "sprite.h"
#include "jwin.h"
#include "base/zsys.h"
#include "subscr.h"
#include "zc_subscr.h"
#include "hero.h"
#include "guys.h"
#include "ffscript.h"
#include "drawing.h"
#include "combos.h"
extern word combo_doscript[176];
extern refInfo screenScriptData;
extern FFScript FFCore;
#include "particles.h"
#include "mem_debug.h"


#define EPSILON 0.01 // Define your own tolerance
#define FLOAT_EQ(x,v) (((v - EPSILON) < x) && (x <( v + EPSILON)))
#define DegtoFix(d)     ((d)*0.71111111)

extern HeroClass Hero;

// TODO z3 checklist
// screen secrets:
//    - trigger all secrets in region
//    - multiple triggers across many screens in a region (multi-block puzzle)
//    - perm secrets
// sword beams / ewpns should only despawn when leaving viewport

// dark rooms, and transitions
// screen wipe in when spawning in middle of region
// ffcs
// define regions dynamically

static std::map<int, std::vector<mapscr*>> temporary_screens;
static mapscr* temporary_screens_currmap[136*7] = {nullptr};
int viewport_x, viewport_y;
int viewport_y_offset;
int world_w, world_h;
static int z3_origin_scr;
int region_scr_dx, region_scr_dy;
int region_scr_width, region_scr_height;
rpos_t region_max_rpos;
int scrolling_maze_scr, scrolling_maze_state;
int scrolling_maze_mode = 0;

static bool global_z3_scrolling = true;

// majora's ALTTP test
#define hardcode_regions_mode 0
// z1
// #define hardcode_regions_mode 1
// entire map is region
// #define hardcode_regions_mode 2

static const int hardcode_z3_regions[] = {
#if hardcode_regions_mode == 0
	1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3,
#elif hardcode_regions_mode == 1
	// 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	// 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	// 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	// 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	// 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	// 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	// 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	// 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	// 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	// 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	// 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	// 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	// 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	// 4, 4, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	// 4, 4, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	// 4, 4, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

	3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	4, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
#elif hardcode_regions_mode == 2
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
#endif
};

static int scr_xy_to_index(int x, int y) {
	// TODO can't do this check, because some code expected to be able to go slightly out of bounds
	// DCHECK(x >= 0 && x < 16 && y >= 0 && y < 8);
	x = CLAMP(0, x, 15);
	y = CLAMP(0, y, 7);
	return x + y*16;
}

static bool is_a_region(int map, int scr)
{
	if (!global_z3_scrolling) return false;
#if hardcode_regions_mode == 0
	if (map != 1) return false;
#endif
#if hardcode_regions_mode == 1
	if (map != 0) return false;
#endif
	if (scr >= 128) return false;
	return hardcode_z3_regions[scr];
}

static bool is_in_region(int region_origin_scr, int scr)
{
	if (!is_a_region(currmap, scr)) return false;
	int region_id = hardcode_z3_regions[region_origin_scr];
	return region_id && region_id == hardcode_z3_regions[scr];
}

bool is_z3_scrolling_mode()
{
	// Note: `screenscrolling` bit is only needed for some funky logic in do_scrolling_layer().
	return is_a_region(currmap, currscr) || (screenscrolling && is_a_region(scrolling_map, scrolling_scr));
}

// TODO z3 map
int z3_get_region_id(int scr)
{
	if (!global_z3_scrolling) return 0;
	return hardcode_z3_regions[scr];
}

void z3_calculate_region(int scr, int& origin_scr, int& region_scr_width, int& region_scr_height, int& region_scr_dx, int& region_scr_dy, int& world_w, int& world_h)
{
	if (!is_z3_scrolling_mode())
	{
		origin_scr = scr;
		region_scr_dx = 0;
		region_scr_dy = 0;
		region_scr_width = 1;
		region_scr_height = 1;
		world_w = 256;
		world_h = 176;
		return;
	}

	int input_scr_x = scr % 16;
	int input_scr_y = scr / 16;

	// For the given screen, find the top-left corner of its region.
	int origin_scr_x = input_scr_x;
	int origin_scr_y = input_scr_y;
	origin_scr = scr;
	while (origin_scr_x > 0)
	{
		if (!is_in_region(origin_scr, scr_xy_to_index(origin_scr_x - 1, origin_scr_y))) break;
		origin_scr_x--;
	}
	while (origin_scr_y > 0)
	{
		if (!is_in_region(origin_scr, scr_xy_to_index(origin_scr_x, origin_scr_y - 1))) break;
		origin_scr_y--;
	}
	origin_scr = scr_xy_to_index(origin_scr_x, origin_scr_y);
	
	// Now find the bottom-right corner.
	int region_scr_right = origin_scr_x;
	while (region_scr_right < 15)
	{
		if (!is_in_region(origin_scr, scr_xy_to_index(region_scr_right + 1, origin_scr_y))) break;
		region_scr_right++;
	}
	int region_scr_bottom = origin_scr_y;
	while (region_scr_bottom < 7)
	{
		if (!is_in_region(origin_scr, scr_xy_to_index(origin_scr_x, region_scr_bottom + 1))) break;
		region_scr_bottom++;
	}

	region_scr_width = region_scr_right - origin_scr_x + 1;
	region_scr_height = region_scr_bottom - origin_scr_y + 1;
	world_w = 256*region_scr_width;
	world_h = 176*region_scr_height;
	region_scr_dx = input_scr_x - origin_scr_x;
	region_scr_dy = input_scr_y - origin_scr_y;
}

static int initial_region_scr = 0;
void z3_load_region()
{
	z3_calculate_region(currscr, z3_origin_scr, region_scr_width, region_scr_height, region_scr_dx, region_scr_dy, world_w, world_h);
	region_max_rpos = static_cast<rpos_t>(region_scr_width*region_scr_height*176 - 1);
	initial_region_scr = currscr;
	scrolling_maze_state = 0;
	scrolling_maze_scr = 0;
	z3_clear_temporary_screens();
}

void z3_clear_temporary_screens()
{
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

void z3_calculate_viewport(mapscr* scr, int world_w, int world_h, int hero_x, int hero_y, int& viewport_x, int& viewport_y)
{
	if (!is_z3_scrolling_mode())
	{
		viewport_x = 0;
		viewport_y = 0;
		return;
	}

	int viewport_w = 256;
	int viewport_h = 176;
	viewport_x = hero_x - viewport_w/2;
	// TODO z3 this is quite a hack
	if (global_z3_scrolling_extended_height_mode)
		viewport_y = viewport_y_offset + hero_y - (viewport_h-64)/2;
	else
		viewport_y = viewport_y_offset + hero_y - viewport_h/2;
	
	// if (scr->flags&fMAZE) return;

	// Clamp the viewport to the edges of the region.
	viewport_x = CLAMP(0, world_w - viewport_w, viewport_x);
	viewport_y = CLAMP(0, world_h - viewport_h, viewport_y);
}

void z3_update_viewport()
{
	z3_calculate_viewport(&tmpscr, world_w, world_h, Hero.getX(), Hero.getY(), global_viewport_x, global_viewport_y);
}

void z3_update_currscr()
{
	int dx = Hero.getX().getFloor() / 256;
	int dy = Hero.getY().getFloor() / 176;
	int newscr = z3_origin_scr + dx + dy * 16;
	if (dx >= 0 && dy >= 0 && dx < 16 && dy < 8 && is_in_region(z3_origin_scr, newscr))
	{
		region_scr_dx = dx;
		region_scr_dy = dy;
		currscr = newscr;
	}
}

bool edge_of_region(direction dir)
{
	if (!is_z3_scrolling_mode()) return true;

	int scr_x = currscr % 16;
	int scr_y = currscr / 16;
	if (dir == up) scr_y -= 1;
	if (dir == down) scr_y += 1;
	if (dir == left) scr_x -= 1;
	if (dir == right) scr_x += 1;
	if (scr_x < 0 || scr_x > 16 || scr_y < 0 || scr_y > 8) return true;
	return !is_in_region(z3_origin_scr, scr_xy_to_index(scr_x, scr_y));
}

// x, y are world coordinates (aka, where hero is in relation to origin screen)
int z3_get_scr_for_xy_offset(int x, int y)
{
	int dx = x / 256;
	int dy = y / 176;
	int origin_scr_x = z3_origin_scr % 16;
	int origin_scr_y = z3_origin_scr / 16;
	int scr_x = origin_scr_x + dx;
	int scr_y = origin_scr_y + dy;
	return scr_xy_to_index(scr_x, scr_y);
}

int z3_get_scr_for_rpos(rpos_t rpos)
{
	int origin_scr_x = z3_origin_scr % 16;
	int origin_scr_y = z3_origin_scr / 16;
	int scr_index = static_cast<int32_t>(rpos) / 176;
	int scr_x = origin_scr_x + scr_index%region_scr_width;
	int scr_y = origin_scr_y + scr_index/region_scr_width;
	return scr_xy_to_index(scr_x, scr_y);
}

mapscr* z3_get_mapscr_for_rpos(rpos_t rpos)
{
	return get_scr(currmap, z3_get_scr_for_rpos(rpos));
}

pos_handle z3_get_pos_handle(rpos_t rpos, int layer)
{
	DCHECK_LAYER_ZERO_INDEX(layer);
	int screen_index = z3_get_scr_for_rpos(rpos);
	mapscr* screen = get_layer_scr(currmap, screen_index, layer - 1);
	return {screen, screen_index, layer, rpos};
}

pos_handle z3_get_pos_handle_for_world_xy(int x, int y, int layer)
{
	DCHECK_LAYER_ZERO_INDEX(layer);
	return z3_get_pos_handle(COMBOPOS_REGION(x, y), layer);
}

// These functions all return _temporary_ screens. Any modifcations made to them (either by the engine
// directly or via zscript) only last until the next screen (or region) is loaded.

// Returns the screen containing the (x, y) world position.
// TODO z3 rename 'get_mapscr_of_world_position'
mapscr* z3_get_mapscr_for_xy_offset(int x, int y)
{
	// Quick path, but should work the same without.
	if (!is_z3_scrolling_mode()) return &tmpscr;
	return get_scr(currmap, z3_get_scr_for_xy_offset(x, y));
}

// Note: layer=0 is the base screen, 1 is the first layer, etc.
mapscr* z3_get_mapscr_layer_for_xy_offset(int x, int y, int layer)
{
	DCHECK_LAYER_ZERO_INDEX(layer);
	return layer == 0 ?
		z3_get_mapscr_for_xy_offset(x, y) :
		get_layer_scr(currmap, z3_get_scr_for_xy_offset(x, y), layer - 1);
}

int z3_get_origin_scr()
{
	return z3_origin_scr;
}

// z3_origin_scr TODO z3_origin_screen
int z3_get_region_relative_dx(int scr)
{
	return scr % 16 - z3_origin_scr % 16;
}

int z3_get_region_relative_dy(int scr)
{
	return scr / 16 - z3_origin_scr / 16;
}

// TODO: replace this with current region data.
// current_region_src_x, current_region_src_y
int z3_get_z3scr_dx()
{
	return currscr % 16 - z3_origin_scr % 16;
}

int z3_get_z3scr_dy()
{
	return currscr / 16 - z3_origin_scr / 16;
}

// TODO z3 use this.
const mapscr* get_canonical_scr(int map, int screen)
{
	return &TheMaps[map*MAPSCRS + screen];
}

mapscr* get_scr(int map, int screen)
{
	DCHECK(screen >= 0 && screen < 136);
	if (screen == initial_region_scr && map == currmap) return &tmpscr;

	if (map == currmap)
	{
		int index = screen*7;
		if (!temporary_screens_currmap[index])
		{
			load_a_screen_and_layers(currdmap, map, screen);
		}
		return temporary_screens_currmap[index];
	}

	int index = map*MAPSCRS + screen;
	auto it = temporary_screens.find(index);
	if (it != temporary_screens.end()) return it->second[0];
	load_a_screen_and_layers(currdmap, map, screen);
	return temporary_screens[index][0];
}

// Note: layer=-1 returns the base screen, layer=0 returns the first layer.
mapscr* get_layer_scr(int map, int screen, int layer)
{
	DCHECK_LAYER_NEG1_INDEX(layer);
	if (layer == -1) return get_scr(map, screen);
	if (screen == initial_region_scr && map == currmap) return &tmpscr2[layer];

	if (map == currmap)
	{
		int index = screen*7;
		if (!temporary_screens_currmap[index])
		{
			load_a_screen_and_layers(currdmap, map, screen);
		}
		return temporary_screens_currmap[index+layer+1];
	}

	int index = map*MAPSCRS + screen;
	auto it = temporary_screens.find(index);
	if (it != temporary_screens.end()) return it->second[layer + 1];
	load_a_screen_and_layers(currdmap, map, screen);
	return temporary_screens[index][layer + 1];
}

// Note: layer=-1 returns the base screen, layer=0 returns the first layer.
mapscr* get_layer_scr_for_xy(int x, int y, int layer)
{
	return get_layer_scr(currmap, z3_get_scr_for_xy_offset(x, y), layer);
}

mapscr* get_home_scr()
{
	return get_scr(currmap, homescr);
}

int32_t COMBOPOS_REGION_EXTENDED(int32_t pos, int32_t scr_dx, int32_t scr_dy)
{
	int x = (pos%16) + scr_dx*16;
	int y = (pos/16) + scr_dy*11;
	int combos_wide = region_scr_width * 16;
	return x + y * combos_wide;
}
int32_t COMBOPOS_REGION_EXTENDED(int32_t x, int32_t y)
{
	int combos_wide = region_scr_width  * 16;
	return x / 16 + y / 16 * combos_wide;
}
int32_t COMBOX_REGION_EXTENDED(int32_t pos)
{
	int combos_wide = region_scr_width * 16;
	return pos % combos_wide * 16;
}
int32_t COMBOY_REGION_EXTENDED(int32_t pos)
{
	int combos_wide = region_scr_width * 16;
	return pos / combos_wide * 16;
}

int32_t COMBOPOS(int32_t x, int32_t y)
{
	// TODO z3
	// DCHECK(x >= 0 && x < 16 && y >= 0 && y < 11);
	return (((y) & 0xF0) + ((x) >> 4));
}
int32_t COMBOX(int32_t pos)
{
    return ((pos) % 16 * 16);
}
int32_t COMBOY(int32_t pos)
{
    return ((pos) & 0xF0);
}

rpos_t COMBOPOS_REGION(int32_t x, int32_t y)
{
	int scr_dx = x / (16*16);
	int scr_dy = y / (11*16);
	int pos = COMBOPOS(x%256, y%176);
	return static_cast<rpos_t>((scr_dx + scr_dy * region_scr_width)*176 + pos);
}
int32_t RPOS_TO_POS(rpos_t rpos)
{
	return static_cast<int32_t>(rpos)%176;
}
rpos_t POS_TO_RPOS(int32_t pos, int32_t scr_dx, int32_t scr_dy)
{
	// TODO z3 abs is needed because of layers during scrolling between regions ... do_scrolling_layer
	return static_cast<rpos_t>(abs(scr_dx + scr_dy * region_scr_width)*176 + pos);
}
rpos_t POS_TO_RPOS(int32_t pos, int32_t scr)
{
	return POS_TO_RPOS(pos, z3_get_region_relative_dx(scr), z3_get_region_relative_dy(scr));
}
void COMBOXY_REGION(rpos_t rpos, int32_t& out_x, int32_t& out_y)
{
	int scr_index = static_cast<int32_t>(rpos) / 176;
	int scr_dx = scr_index % region_scr_width;
	int scr_dy = scr_index / region_scr_width;
    int pos = RPOS_TO_POS(rpos);
	out_x = scr_dx*16*16 + COMBOX(pos);
	out_y = scr_dy*11*16 + COMBOY(pos);
}
// TODO z3 "COMBOX" and overload?
int32_t COMBOX_REGION(rpos_t rpos)
{
	int x, y;
	COMBOXY_REGION(rpos, x, y);
	return x;
}
int32_t COMBOY_REGION(rpos_t rpos)
{
	int x, y;
	COMBOXY_REGION(rpos, x, y);
	return y;
}

int32_t mapind(int32_t map, int32_t scr)
{
	return (map<<7)+scr;
}

FONT *get_zc_font(int32_t index);

extern sprite_list  guys, items, Ewpns, Lwpns, Sitems, chainlinks, decorations;
extern particle_list particles;
extern movingblock mblock2;                                 //mblock[4]?
extern portal* mirror_portal;
extern zinitdata zinit;
int32_t current_ffcombo=-1;
bool triggered_screen_secrets=false;

int16_t ffposx[32]= {-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
                   -1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000
                  };
int16_t ffposy[32]= {-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
                   -1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000
                  };
int32_t ffprvx[32]= {-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
                  -10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000
                 };
int32_t ffprvy[32]= {-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
                  -10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000
                 };




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
    memset(&DMaps[i],0,sizeof(dmap));
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
	// TODO z3
    if(scr < 0) scr=global_z3_cur_scr_drawing == -1 ? currscr : global_z3_cur_scr_drawing;
    
    if(dmap < 0) dmap = currdmap;
    
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

bool canPermSecret(int32_t dmap, int32_t scr)
{
	return (!isdungeon(dmap, scr) || get_bit(quest_rules,qr_DUNGEON_DMAPS_PERM_SECRETS));
}

int32_t MAPCOMBO(int32_t x,int32_t y)
{
	if (!is_z3_scrolling_mode())
	{
		//extend combos outwards if out of bounds -DD
		x = vbound(x, 0, (16*16)-1);
		y = vbound(y, 0, (11*16)-1);
		int32_t combo = COMBOPOS(x,y);
		
		if(combo>175 || combo < 0)
			return 0;
			
		return tmpscr.data[combo];                               // entire combo code
	}
	else
	{
		// TODO z3
		//extend combos outwards if out of bounds -DD
		x = vbound(x, 0, world_w-1);
		y = vbound(y, 0, world_h-1);
		int32_t combo = COMBOPOS(x % 256, y % 176);
		if(combo>175 || combo < 0)
			return 0;

		auto z3_scr = z3_get_mapscr_for_xy_offset(x, y);
		return z3_scr->data[combo];
	}
}

int32_t MAPCOMBOzq(int32_t x,int32_t y)
{
	return MAPCOMBO(x,y);
}

//specific layers 1 to 6
int32_t MAPCOMBOL(int32_t layer,int32_t x,int32_t y)
{
	DCHECK_LAYER_ZERO_INDEX(layer);
    // TODO z3 blocks
    if(tmpscr2[layer-1].data.empty()) return 0;
    
    if(tmpscr2[layer-1].valid==0) return 0;
    
    int32_t combo = COMBOPOS(x,y);
    
    if(combo>175 || combo < 0)
        return 0;
        
    return tmpscr2[layer-1].data[combo];                        // entire combo code
}

int32_t MAPCSETL(int32_t layer,int32_t x,int32_t y)
{
    
    if(tmpscr2[layer-1].cset.empty()) return 0;
    
    if(tmpscr2[layer-1].valid==0) return 0;
    
    int32_t combo = COMBOPOS(x,y);
    
    if(combo>175 || combo < 0)
        return 0;
        
    return tmpscr2[layer-1].cset[combo];                        // entire combo code
}

int32_t MAPFLAGL(int32_t layer,int32_t x,int32_t y)
{
    
    if(tmpscr2[layer-1].sflag.empty()) return 0;
    
    if(tmpscr2[layer-1].valid==0) return 0;
    
    int32_t combo = COMBOPOS(x,y);
    
    if(combo>175 || combo < 0)
        return 0;
        
    return tmpscr2[layer-1].sflag[combo];                       // flag
}

int32_t COMBOTYPEL(int32_t layer,int32_t x,int32_t y)
{
    if(!layer || tmpscr2[layer-1].valid==0)
    {
        return 0;
    }
    
    return combobuf[MAPCOMBO2(layer-1,x,y)].type;
}

int32_t MAPCOMBOFLAGL(int32_t layer,int32_t x,int32_t y)
{
    if(layer==-1) return MAPCOMBOFLAG(x,y);
    
    if(tmpscr2[layer-1].data.empty()) return 0;
    
    if(tmpscr2[layer-1].valid==0) return 0;
    
    int32_t combo = COMBOPOS(x,y);
    
    if(combo>175 || combo < 0)
        return 0;
        
    return combobuf[tmpscr2[layer-1].data[combo]].flag;                        // entire combo code
}


// True if the FFC covers x, y and is not ethereal or a changer.
// Used by MAPFFCOMBO(), MAPFFCOMBOFLAG, and getFFCAt().
inline bool ffcIsAt(int32_t index, int32_t x, int32_t y)
{
    int32_t fx=tmpscr.ffx[index]/10000;
    if(x<fx || x>fx+(tmpscr.ffwidth[index]&63)) // FFC sizes are weird.
        return false;
    
    int32_t fy=tmpscr.ffy[index]/10000;
    if(y<fy || y>fy+(tmpscr.ffheight[index]&63))
        return false;
    
    if((tmpscr.ffflags[index]&(ffCHANGER|ffETHEREAL))!=0)
        return false;
	
    if(tmpscr.ffdata[index]<=0)
        return false;
    
    return true;
}

int32_t MAPFFCOMBO(int32_t x,int32_t y)
{
    for(int32_t i=0; i<32; i++)
    {
        if(ffcIsAt(i, x, y))
            return tmpscr.ffdata[i];
    }
    
    return 0;
}

int32_t MAPCSET(int32_t x, int32_t y)
{
	if (x < 0 || x >= world_w || y < 0 || y >= world_h)
		return 0;
	mapscr* scr = z3_get_mapscr_for_xy_offset(x, y);
	int32_t combo = COMBOPOS(x%256, y%176);
	return scr->cset[combo];
}

int32_t MAPFLAG(int32_t x, int32_t y)
{
	if (x < 0 || x >= world_w || y < 0 || y >= world_h)
		return 0;
	mapscr* scr = z3_get_mapscr_for_xy_offset(x, y);
	int32_t combo = COMBOPOS(x%256, y%176);
	return scr->sflag[combo];
}

int32_t COMBOTYPE(int32_t x,int32_t y)
{
	if (x < 0 || y < 0 || x >= world_w || y >= world_h) return 0;

	int32_t b=1;
	if(x&8) b<<=2;
	if(y&8) b<<=1;

	for (int32_t i = 0; i <= 1; ++i)
	{
		if (get_bit(quest_rules, qr_OLD_BRIDGE_COMBOS))
		{
			if (combobuf[MAPCOMBO2(i,x,y)].type == cBRIDGE && !_walkflag_layer(x, y, i)) return cNONE;
		}
		else
		{
			if (combobuf[MAPCOMBO2(i,x,y)].type == cBRIDGE && _effectflag_layer(x, y, i)) return cNONE;
		}
	}
	
	newcombo const& cmb = combobuf[MAPCOMBO(x,y)];
	if (cmb.type == cWATER && (cmb.usrflags&cflag4) && (cmb.walk&b) && ((cmb.walk>>4)&b)) return cSHALLOWWATER;
	if (cmb.type == cWATER && (cmb.usrflags&cflag3) && (cmb.walk&b) && ((cmb.walk>>4)&b)) return cNONE;
	return cmb.type;
}

int32_t FFCOMBOTYPE(int32_t x,int32_t y)
{
    return combobuf[MAPFFCOMBO(x,y)].type;
}

int32_t FFORCOMBO(int32_t x, int32_t y)
{
	for(int32_t i=0; i<32; i++)
    {
        if(ffcIsAt(i, x, y))
            return tmpscr.ffdata[i];
    }
	
	return MAPCOMBO(x,y);
}

int32_t FFORCOMBOTYPE(int32_t x, int32_t y)
{
	for (int32_t i = 0; i <= 1; ++i)
	{
		if (get_bit(quest_rules, qr_OLD_BRIDGE_COMBOS))
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
	for(int32_t i=0; i<32; i++)
    {
        if(ffcIsAt(i, x, y))
            return tmpscr.ffdata[i];
    }
	
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
	mapscr* scr = z3_get_mapscr_for_xy_offset(x, y);
	int32_t combo = COMBOPOS(x%256, y%176);
	return combobuf[scr->data[combo]].flag;
}

int32_t MAPFFCOMBOFLAG(int32_t x,int32_t y)
{
    for(int32_t i=0; i<32; i++)
    {
        if(ffcIsAt(i, x, y))
        {
            current_ffcombo = i;
            return combobuf[tmpscr.ffdata[i]].flag;
        }
    }
    
    current_ffcombo=-1;
    return 0;
}

int32_t getFFCAt(int32_t x, int32_t y)
{
    for(int32_t i=0; i<32; i++)
    {
        if(ffcIsAt(i, x, y))
            return i;
    }
    
    return -1;
}

int32_t MAPCOMBO(const pos_handle& pos_handle)
{
	if (pos_handle.screen->data.empty()) return 0;
	if (pos_handle.screen->valid == 0) return 0;
	return pos_handle.screen->data[RPOS_TO_POS(pos_handle.rpos)];
}

int32_t MAPCOMBO2(int32_t layer, int32_t x, int32_t y)
{
	DCHECK_LAYER_NEG1_INDEX(layer);
	if (x < 0 || y < 0 || x >= world_w || y >= world_h) return 0;
    if (layer <= -1) return MAPCOMBO(x, y);
    
	auto pos_handle = z3_get_pos_handle_for_world_xy(x, y, layer + 1);
	if (pos_handle.screen->data.empty()) return 0;
	if (pos_handle.screen->valid == 0) return 0;

	return pos_handle.screen->data[RPOS_TO_POS(pos_handle.rpos)];
}

int32_t MAPCOMBO3(int32_t map, int32_t screen, int32_t layer, int32_t x,int32_t y, bool secrets)
{
	return MAPCOMBO3(map, screen, layer, COMBOPOS(x,y), secrets);
}

int32_t MAPCOMBO3(int32_t map, int32_t screen, int32_t layer, int32_t pos, bool secrets)
{ 
	DCHECK_LAYER_NEG1_INDEX(layer);
	if (map < 0 || screen < 0) return 0;
	
	// TODO z3 this should run even in scrolling mode, if screen is in region
	if (!is_z3_scrolling_mode() && map == currmap && screen == currscr) return MAPCOMBO2(layer,COMBOX(pos),COMBOY(pos));
	
	// Screen is not in temporary memory, so we have to load and trigger some secrets in MAPCOMBO3.

	if(pos>175 || pos < 0)
		return 0;
		
	mapscr *m = &TheMaps[(map*MAPSCRS)+screen];
	return MAPCOMBO3(m, map, screen, layer, pos, secrets);
}

int32_t MAPCOMBO3(mapscr *m, int32_t map, int32_t screen, int32_t layer, int32_t pos, bool secrets)
{
	if(m->data.empty()) return 0;
    
	if(m->valid==0) return 0;
	
	int32_t flags = 0;
	
	if(secrets && game->maps[(map*MAPSCRSNORMAL)+screen])
	{
		flags = game->maps[(map*MAPSCRSNORMAL)+screen];
		//secrets = false;
	}
	
	int32_t mapid = (layer < 0 ? -1 : ((m->layermap[layer] - 1) * MAPSCRS + m->layerscreen[layer]));
	
	if (layer >= 0 && (mapid < 0 || mapid > MAXMAPS2*MAPSCRS)) return 0;
	
	// TODO z3 super expensive...
	mapscr scr = ((mapid < 0 || mapid > MAXMAPS2*MAPSCRS) ? *m : TheMaps[mapid]);
	
	if(scr.data.empty()) return 0;
    
	if(scr.valid==0) return 0;
	
	// TODO z3 can this not be called all the time?
	if ((flags & mSECRET) && canPermSecret(currdmap, screen))
	{
		hiddenstair2(&scr, false);
		bool do_layers = false;
		hidden_entrance2(-1, &scr, do_layers, false, -3);
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
	    remove_screenstatecombos2(&scr, (mapscr*)NULL, cLOCKBLOCK, cLOCKBLOCK2);
	}

	if(flags&mBOSSLOCKBLOCK)          // if special stuff done before
	{
	    remove_screenstatecombos2(&scr, (mapscr*)NULL, cBOSSLOCKBLOCK, cBOSSLOCKBLOCK2);
	}

	if(flags&mCHEST)              // if special stuff done before
	{
	    remove_screenstatecombos2(&scr, (mapscr*)NULL, cCHEST, cCHEST2);
	}

	if(flags&mCHEST)              // if special stuff done before
	{
	    remove_screenstatecombos2(&scr, (mapscr*)NULL, cLOCKEDCHEST, cLOCKEDCHEST2);
	}

	if(flags&mBOSSCHEST)              // if special stuff done before
	{
	    remove_screenstatecombos2(&scr, (mapscr*)NULL, cBOSSCHEST, cBOSSCHEST2);
	}
	
	return scr.data[pos];						// entire combo code
}

int32_t MAPCSET2(int32_t layer,int32_t x,int32_t y)
{
	DCHECK_LAYER_NEG1_INDEX(layer);
	if (x < 0 || x >= world_w || y < 0 || y >= world_h)
		return 0;
    if (layer == -1) return MAPCSET(x, y);

	auto pos_handle = z3_get_pos_handle_for_world_xy(x, y, layer + 1);
	if (pos_handle.screen->valid == 0) return 0;
	if (pos_handle.screen->cset.empty()) return 0;
	
	return pos_handle.screen->cset[RPOS_TO_POS(pos_handle.rpos)];
}

int32_t MAPFLAG2(int32_t layer,int32_t x,int32_t y)
{
	DCHECK_LAYER_NEG1_INDEX(layer);
	if (x < 0 || x >= world_w || y < 0 || y >= world_h)
		return 0;
    if (layer == -1) return MAPFLAG(x, y);

	auto pos_handle = z3_get_pos_handle_for_world_xy(x, y, layer + 1);
	if (pos_handle.screen->valid == 0) return 0;
	if (pos_handle.screen->sflag.empty()) return 0;

	return pos_handle.screen->sflag[RPOS_TO_POS(pos_handle.rpos)];
}

int32_t COMBOTYPE2(int32_t layer,int32_t x,int32_t y)
{
    if(layer < 1)
    {
	for (int32_t i = layer+1; i <= 1; ++i)
	{
		if (get_bit(quest_rules, qr_OLD_BRIDGE_COMBOS))
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
    if(tmpscr2[layer].valid==0)
    {
        return 0;
    }
    
    return combobuf[MAPCOMBO2(layer,x,y)].type;
}

int32_t MAPCOMBOFLAG2(int32_t layer,int32_t x,int32_t y)
{
	DCHECK_LAYER_NEG1_INDEX(layer);
	if (x < 0 || x >= world_w || y < 0 || y >= world_h)
		return 0;
    if (layer == -1) return MAPCOMBOFLAG(x, y);

	auto pos_handle = z3_get_pos_handle_for_world_xy(x, y, layer + 1);
	if (pos_handle.screen->valid == 0) return 0;
	if (pos_handle.screen->data.empty()) return 0;

	int cid = pos_handle.screen->data[RPOS_TO_POS(pos_handle.rpos)];
	return combobuf[cid].flag;
}

bool HASFLAG(int32_t flag, int32_t layer, int32_t pos)
{
	if(unsigned(pos) > 175) return false;
	if(unsigned(layer) > 6) return false;
	mapscr* m = (layer ? &tmpscr2[layer-1] : &tmpscr);
	if(!m->valid) return false;
	if(m->data.empty()) return false;
	
	if(m->sflag[pos] == flag) return true;
	if(combobuf[m->data[pos]].flag == flag) return true;
	
	return false;
}

bool HASFLAG_ANY(int32_t flag, int32_t pos)
{
	if(unsigned(pos) > 175) return false;
	for(auto q = 0; q < 7; ++q)
	{
		if(HASFLAG(flag, q, pos))
			return true;
	}
	return false;
}

void setmapflag(int32_t flag)
{
    setmapflag((currmap*MAPSCRSNORMAL)+homescr,flag);
}

const char *screenstate_string[16] =
{
    "Door Up", "Door Down", "Door Left", "Door Right", "Item", "Special Item", "No Return",
    "Temporary No Return", "Lock Blocks", "Boss Lock Blocks", "Chests", "Locked Chests",
    "Boss Locked Chests", "Secrets", "Visited", "Light Beams"
};

void eventlog_mapflag_line(word* g, word flag, int32_t ss_s_index)
{
    if((*g)&flag)
    {
        (*g)&=~flag;
        Z_eventlog("%s%s", screenstate_string[ss_s_index], (*g)!=0 ? ", " : "");
    }
}

void eventlog_mapflags()
{
	int32_t mi = (currmap*MAPSCRSNORMAL)+homescr;
    word g = game->maps[mi] &0x3FFF;
    word g2 = g;
    Z_eventlog("Screen (%d, %02x) %s", currmap+1, homescr, (g2) != 0 ? "[":"");
    // Print them in order of importance.
    eventlog_mapflag_line(&g, mSECRET,13);
    eventlog_mapflag_line(&g, mITEM,4);
    eventlog_mapflag_line(&g, mSPECIALITEM,5);
    eventlog_mapflag_line(&g, mLOCKBLOCK,8);
    eventlog_mapflag_line(&g, mBOSSLOCKBLOCK,9);
    eventlog_mapflag_line(&g, mCHEST,10);
    eventlog_mapflag_line(&g, mLOCKEDCHEST,11);
    eventlog_mapflag_line(&g, mBOSSCHEST,12);
    eventlog_mapflag_line(&g, mDOOR_UP,0);
    eventlog_mapflag_line(&g, mDOOR_DOWN,1);
    eventlog_mapflag_line(&g, mDOOR_LEFT,2);
    eventlog_mapflag_line(&g, mDOOR_RIGHT,3);
    eventlog_mapflag_line(&g, mNEVERRET,6);
    eventlog_mapflag_line(&g, mTMPNORET,7);
    if(g2) Z_eventlog("%s","]");
	if(game->xstates[mi])
	{
		Z_eventlog(" Ex[");
		bool comma = false;
		for(byte fl = 0; fl < 32; ++fl)
		{
			if(game->xstates[mi] & (1<<fl))
			{
				Z_eventlog("%s%d", comma ? ", " : "", fl);
				comma = true;
			}
		}
		Z_eventlog("]");
	}
	Z_eventlog("\n");
}

// set specific flag
void setmapflag2(mapscr* scr, int32_t screen, int32_t flag)
{
	int mi = (currmap * MAPSCRSNORMAL) + (screen >= 0x80 ? homescr : screen);
	setmapflag(scr, mi, flag);
}
void setmapflag(int32_t mi2, int32_t flag)
{
	setmapflag(&tmpscr, mi2, flag);
}
void setmapflag(mapscr* scr, int32_t mi2, int32_t flag)
{
    byte cscr = mi2&((1<<7)-1);
    byte cmap = (mi2>>7);
    char buf[20];
    sprintf(buf,"Screen (%d, %02X)",cmap+1,cscr);
    
    game->maps[mi2] |= flag;
    float temp=log2((float)flag);
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
            if((scr->nocarry&flag)!=flag)
            {
                Z_eventlog("State change carried over to (%d, %02X)\n",nmap,nscr);
                game->maps[((nmap-1)<<7)+nscr] |= flag;
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
    if(anyflag)
        game->maps[mi2] &= ~flag;
    else if(flag==mITEM || flag==mSPECIALITEM)
    {
        if(!(tmpscr.flags4&fNOITEMRESET))
            game->maps[mi2] &= ~flag;
    }
    else game->maps[mi2] &= ~flag;
    
    byte cscr = mi2&((1<<7)-1);
    byte cmap = (mi2>>7);
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
            if((tmpscr.nocarry&flag)!=flag)
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

void setxmapflag2(int32_t screen, int32_t flag)
{
	int mi = (currmap * MAPSCRSNORMAL) + (screen >= 0x80 ? homescr : screen);
	setxmapflag(mi, flag);
}
void setxmapflag(int32_t mi2, uint32_t flag)
{
    byte cscr = mi2&((1<<7)-1);
    byte cmap = (mi2>>7);
    char buf[20];
    sprintf(buf,"Screen (%d, %02X)",cmap+1,cscr);
    
    byte temp=(byte)log2((double)flag);
	Z_eventlog("%s's ExtraState was set: %d\n",
		mi2 != (currmap*MAPSCRSNORMAL)+homescr ? buf : "Current screen", temp);
	
	game->xstates[mi2] |= flag;
}
void setxmapflag(uint32_t flag)
{
	setxmapflag((currmap*MAPSCRSNORMAL)+homescr, flag);
}
void unsetxmapflag(int32_t mi2, uint32_t flag)
{
    byte cscr = mi2&((1<<7)-1);
    byte cmap = (mi2>>7);
    char buf[20];
    sprintf(buf,"Screen (%d, %02X)",cmap+1,cscr);
    
    byte temp=(byte)log2((double)flag);
	Z_eventlog("%s's ExtraState was unset: %d\n",
		mi2 != (currmap*MAPSCRSNORMAL)+homescr ? buf : "Current screen", temp);
	
	game->xstates[mi2] &= ~flag;
}
void unsetxmapflag(uint32_t flag)
{
	unsetxmapflag((currmap*MAPSCRSNORMAL)+homescr, flag);
}
// TODO z3
bool getxmapflag2(int32_t screen_index, uint32_t flag)
{
	int mi = (currmap * MAPSCRSNORMAL) + (screen_index >= 0x80 ? homescr : screen_index);
	return getxmapflag(mi, flag);
}
bool getxmapflag(int32_t mi2, uint32_t flag)
{
	return (game->xstates[mi2] & flag) != 0;
}
bool getxmapflag(uint32_t flag)
{
	return getxmapflag((currmap*MAPSCRSNORMAL)+homescr, flag);
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
    int32_t x,y;
    y = 0;
    static int32_t newdata[176];
    static int32_t newcset[176];
    static int32_t newdata2[176];
    static int32_t newcset2[176];
    static bool restartanim[MAXCOMBOS];
    static bool restartanim2[MAXCOMBOS];
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
        
        memset(restartanim, 0, MAXCOMBOS);
        memset(restartanim2, 0, MAXCOMBOS);
        initialized=true;
    }
    
    for(int32_t i=0; i<176; i++)
    {
        x=tmpscr.data[i];
        //y=animated_combo_table[x][0];
        
        if(combobuf[x].animflags & AF_FRESH) continue;
        
        //time to restart
        if((combobuf[x].aclk>=combobuf[x].speed) &&
                (combobuf[x].tile-combobuf[x].frames>=combobuf[x].o_tile-1) &&
                (combobuf[x].nextcombo!=0))
        {
            newdata[i]=combobuf[x].nextcombo;
			if(!(combobuf[x].animflags & AF_CYCLENOCSET))
				newcset[i]=combobuf[x].nextcset;
            int32_t c=newdata[i];
            
            if(combobuf[c].animflags & AF_CYCLE)
            {
                restartanim[c]=true;
            }
        }
    }
    
    for(int32_t i=0; i<176; i++)
    {
        x=tmpscr.data[i];
        //y=animated_combo_table2[x][0];
        
        if(!(combobuf[x].animflags & AF_FRESH)) continue;
        
        //time to restart
        if((combobuf[x].aclk>=combobuf[x].speed) &&
                (combobuf[x].tile-combobuf[x].frames>=combobuf[x].o_tile-1) &&
                (combobuf[x].nextcombo!=0))
        {
            newdata[i]=combobuf[x].nextcombo;
            if(!(combobuf[x].animflags & AF_CYCLENOCSET))
				newcset[i]=combobuf[x].nextcset;
            int32_t c=newdata[i];
            
            if(combobuf[c].animflags & AF_CYCLE)
            {
                restartanim2[c]=true;
            }
        }
    }
    
    for(int32_t i=0; i<176; i++)
    {
        if(newdata[i]==-1)
            continue;
            
        screen_combo_modify_preroutine(&tmpscr,i);
        tmpscr.data[i]=newdata[i];
		if(newcset[i]>-1)
			tmpscr.cset[i]=newcset[i];
        screen_combo_modify_postroutine(&tmpscr,i);
        
        newdata[i]=-1;
        newcset[i]=-1;
    }
    
    for(int32_t i=0; i<32; i++)
    {
        x=tmpscr.ffdata[i];
        //y=animated_combo_table[x][0];
        
        if(combobuf[x].animflags & AF_FRESH) continue;
        
        //time to restart
        if((combobuf[x].aclk>=combobuf[x].speed) &&
                (combobuf[x].tile-combobuf[x].frames>=combobuf[x].o_tile-1) &&
                (combobuf[x].nextcombo!=0))
        {
            newdata[i]=combobuf[x].nextcombo;
            if(!(combobuf[x].animflags & AF_CYCLENOCSET))
				newcset[i]=combobuf[x].nextcset;
            int32_t c=newdata[i];
            
            if(combobuf[c].animflags & AF_CYCLE)
            {
                restartanim[c]=true;
            }
        }
    }
    
    for(int32_t i=0; i<32; i++)
    {
        x=tmpscr.ffdata[i];
        //y=animated_combo_table2[x][0];
        
        if(!(combobuf[x].animflags & AF_FRESH)) continue;
        
        //time to restart
        if((combobuf[x].aclk>=combobuf[x].speed) &&
                (combobuf[x].tile-combobuf[x].frames>=combobuf[x].o_tile-1) &&
                (combobuf[x].nextcombo!=0))
        {
            newdata[i]=combobuf[x].nextcombo;
            if(!(combobuf[x].animflags & AF_CYCLENOCSET))
				newcset[i]=combobuf[x].nextcset;
            int32_t c=newdata[i];
            
            if(combobuf[c].animflags & AF_CYCLE)
            {
                restartanim2[c]=true;
            }
        }
    }
    
    for(int32_t i=0; i<32; i++)
    {
        if(newdata[i]==-1)
            continue;
            
        tmpscr.ffdata[i]=newdata[i];
        if(newcset[i]>-1)
			tmpscr.ffcset[i]=newcset[i];
        
        newdata[i]=-1;
        newcset[i]=-1;
    }
    
    if(get_bit(quest_rules,qr_CMBCYCLELAYERS))
    {
        for(int32_t j=0; j<6; j++)
        {
            if(tmpscr2[j].data.empty()) continue;
            
            for(int32_t i=0; i<176; i++)
            {
                x=(tmpscr2+j)->data[i];
               // y=animated_combo_table[x][0];
                
                if(combobuf[x].animflags & AF_FRESH) continue;
                
                //time to restart
                if((combobuf[x].aclk>=combobuf[x].speed) &&
                        (combobuf[x].tile-combobuf[x].frames>=combobuf[x].o_tile-1) &&
                        (combobuf[x].nextcombo!=0))
                {
                    newdata[i]=combobuf[x].nextcombo;
                    if(!(combobuf[x].animflags & AF_CYCLENOCSET))
						newcset[i]=combobuf[x].nextcset;
                    int32_t c=newdata[i];
                    
                    if(combobuf[c].animflags & AF_CYCLE)
                    {
                        restartanim[c]=true;
                    }
                }
            }
            
            for(int32_t i=0; i<176; i++)
            {
                x=(tmpscr2+j)->data[i];
                //y=animated_combo_table2[x][0];
                
                if(!(combobuf[x].animflags & AF_FRESH)) continue;
                
                //time to restart
                if((combobuf[x].aclk>=combobuf[x].speed) &&
                        (combobuf[x].tile-combobuf[x].frames>=combobuf[x].o_tile-1) &&
                        (combobuf[x].nextcombo!=0))
                {
                    newdata2[i]=combobuf[x].nextcombo;
					if(!(combobuf[x].animflags & AF_CYCLENOCSET))
						newcset2[i]=combobuf[x].nextcset;
					else newcset2[i]=(tmpscr2+j)->cset[i];
                    int32_t c=newdata2[i];
                    int32_t cs=newcset2[i];
                    
                    if(combobuf[c].animflags & AF_CYCLE)
                    {
                        restartanim2[c]=true;
                    }
                    
                    if(combobuf[c].type==cSPINTILE1)
                    {
                        // Uses animated_combo_table2
                        addenemy((i&15)<<4,i&0xF0,(cs<<12)+eSPINTILE1,combobuf[c].o_tile+zc_max(1,combobuf[c].frames));
                    }
                }
            }
            
            for(int32_t i=0; i<176; i++)
            {
                if(newdata[i]!=-1)
                {
                    screen_combo_modify_preroutine(tmpscr2+j,i);
                    (tmpscr2+j)->data[i]=newdata[i];
                    if(newcset[i]>-1)
						(tmpscr2+j)->cset[i]=newcset[i];
                    screen_combo_modify_postroutine(tmpscr2+j,i);
                    
                    newdata[i]=-1;
                    newcset[i]=-1;
                }
                
                if(newdata2[i]!=-1)
                {
                    (tmpscr2+j)->data[i]=newdata2[i];
                    if(newcset2[i]>-1)
						(tmpscr2+j)->cset[i]=newcset2[i];
                    newdata2[i]=-1;
                    newcset2[i]=-1;
                }
            }
        }
    }
    
    for(int32_t i=0; i<MAXCOMBOS; i++)
    {
        if(restartanim[i])
        {
            combobuf[i].tile = combobuf[i].o_tile;
			combobuf[i].cur_frame=0;
			combobuf[i].aclk = 0;
            restartanim[i]=false;
        }
        
        if(restartanim2[i])
        {
            combobuf[i].tile = combobuf[i].o_tile;
			combobuf[i].cur_frame=0;
			combobuf[i].aclk = 0;
            restartanim2[i]=false;
        }
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

// uses currmap and currscr and takes world coordinates
// TODO z3 just make iswaterrex take world coords, then delete this one.
int32_t iswaterex_z3(int32_t combo, int32_t layer, int32_t x, int32_t y, bool secrets, bool fullcheck, bool LayerCheck, bool ShallowCheck, bool hero)
{
	int screen = z3_get_scr_for_xy_offset(x, y);
	x %= 256;
	y %= 176;
	return iswaterex(combo, currmap, screen, layer, x, y, secrets, fullcheck, LayerCheck, ShallowCheck, hero);
}

int32_t iswaterex(int32_t combo, int32_t map, int32_t screen, int32_t layer, int32_t x, int32_t y, bool secrets, bool fullcheck, bool LayerCheck, bool ShallowCheck, bool hero)
{
	//Honestly, fullcheck is kinda useless... I made this function back when I thought it was checking the entire combo and not just a glorified x/y value.
	//Fullcheck makes no sense to ever be on, but hey I guess it's here in case you ever need it... 
	
	//Oh hey, Zoras might actually need it. Nevermind, this had a use!
	if (get_bit(quest_rules, qr_SMARTER_WATER))
	{
		if (DRIEDLAKE) return 0;
		if (LayerCheck && (get_bit(quest_rules,  qr_WATER_ON_LAYER_1) || get_bit(quest_rules,  qr_WATER_ON_LAYER_2))) //LayerCheck is a bit dumber, but it lets me add this QR without having to replace all calls, again.
		{
			for (int32_t m = layer; m <= 1; m++)
			{
				if (m < 0 || m == 0 && get_bit(quest_rules,  qr_WATER_ON_LAYER_1)
				|| m == 1 && get_bit(quest_rules,  qr_WATER_ON_LAYER_2))
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
					if (get_bit(quest_rules, qr_OLD_BRIDGE_COMBOS))
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
					if (get_bit(quest_rules, qr_NO_SOLID_SWIM))
					{
						if ((cmb.type != cBRIDGE || (!get_bit(quest_rules, qr_OLD_BRIDGE_COMBOS) && !(cmb.walk&(0x10<<b)))) && (cmb.walk&(1<<b)) && !((cmb.usrflags&cflag4) && cmb.type == cWATER && (cmb.walk&(0x10<<b)) && ShallowCheck))
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
				for(int32_t k=0; k<32; k++)
				{
					if(ffcIsAt(k, tx2, ty2) && !combo_class_buf[FFCOMBOTYPE(tx2,ty2)].water && !(ShallowCheck && FFCOMBOTYPE(tx2,ty2) == cSHALLOWWATER))
						return 0;
				}
				for(int32_t k=0; k<32; k++)
				{
					if(combo_class_buf[FFCOMBOTYPE(tx2,ty2)].water || (ShallowCheck && FFCOMBOTYPE(tx2,ty2) == cSHALLOWWATER))
					{
						if (i == 0) return MAPFFCOMBO(tx2,ty2);
						else continue;
					}
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
		if (get_bit(quest_rules, qr_NO_SOLID_SWIM))
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
	/*
	if(int32_t c = MAPFFCOMBO(x,y))
		return ispitfall(c);
	return ispitfall(MAPCOMBO(x,y)) || ispitfall(MAPCOMBOL(1,x,y)) || ispitfall(MAPCOMBOL(2,x,y));
	*/
	// TODO z3 keese?
	if(int32_t c = MAPFFCOMBO(x,y))
	{
		return ispitfall(c) ? true : false;
	}
	int32_t c = MAPCOMBOL(2,x,y);
	if(ispitfall(c)) return true;
	if (get_bit(quest_rules, qr_OLD_BRIDGE_COMBOS))
	{
		if (combobuf[MAPCOMBO2(1,x,y)].type == cBRIDGE && !_walkflag_layer(x,y,1)) return false;
	}
	else
	{
		if (combobuf[MAPCOMBO2(1,x,y)].type == cBRIDGE && _effectflag_layer(x,y,1)) return false;
	}
	c = MAPCOMBOL(1,x,y);
	if(ispitfall(c)) return true;

	if (get_bit(quest_rules, qr_OLD_BRIDGE_COMBOS))
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

	if (get_bit(quest_rules, qr_OLD_BRIDGE_COMBOS))
	{
		if (combobuf[MAPCOMBO2(1,x,y)].type == cBRIDGE && !_walkflag_layer(x,y,1)) return 0;
	}
	else
	{
		if (combobuf[MAPCOMBO2(1,x,y)].type == cBRIDGE && _effectflag_layer(x,y,1)) return 0;
	}
	c = MAPCOMBOL(1,x,y);
	if(ispitfall(c)) return c;

	if (get_bit(quest_rules, qr_OLD_BRIDGE_COMBOS))
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

bool isSVLadder(int32_t x, int32_t y)
{
	if(x<0 || x>255 || y<0 || y>175)
        return false;
	
    mapscr *s1, *s2;
    s1=(tmpscr2->valid)?tmpscr2:&tmpscr;
    s2=(tmpscr2[1].valid)?tmpscr2+1:&tmpscr;
	
    int32_t combo = COMBOPOS(x,y);
    return (tmpscr.sflag[combo] == mfSIDEVIEWLADDER) || (combobuf[tmpscr.data[combo]].flag == mfSIDEVIEWLADDER) ||
		(s1->sflag[combo] == mfSIDEVIEWLADDER) || (combobuf[s1->data[combo]].flag == mfSIDEVIEWLADDER) ||
		(s2->sflag[combo] == mfSIDEVIEWLADDER) || (combobuf[s2->data[combo]].flag == mfSIDEVIEWLADDER);
}

bool isSVPlatform(int32_t x, int32_t y)
{
	if(x<0 || x>255 || y<0 || y>175)
        return false;
	
    mapscr *s1, *s2;
    s1=(tmpscr2->valid)?tmpscr2:&tmpscr;
    s2=(tmpscr2[1].valid)?tmpscr2+1:&tmpscr;
	
    int32_t combo = COMBOPOS(x,y);
    return (tmpscr.sflag[combo] == mfSIDEVIEWPLATFORM) || (combobuf[tmpscr.data[combo]].flag == mfSIDEVIEWPLATFORM) ||
		(s1->sflag[combo] == mfSIDEVIEWPLATFORM) || (combobuf[s1->data[combo]].flag == mfSIDEVIEWPLATFORM) ||
		(s2->sflag[combo] == mfSIDEVIEWPLATFORM) || (combobuf[s2->data[combo]].flag == mfSIDEVIEWPLATFORM);
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

rpos_t check_hshot(int32_t layer, int32_t x, int32_t y, bool switchhook)
{
	int32_t id = MAPCOMBO2(layer-1,x,y);
	if (id <= 0) return rpos_t::NONE; // Never hook combo 0
	newcombo const& cmb = combobuf[id];
	return (switchhook ? isSwitchHookable(cmb) : isHSGrabbable(cmb)) ? COMBOPOS_REGION(x, y) : rpos_t::NONE;
}

bool ishookshottable(int32_t bx, int32_t by)
{
    if(!_walkflag(bx,by,1))
        return true;
        
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

bool ishookshottable(int32_t map, int32_t screen, int32_t bx, int32_t by)
{
	if (map < 0 || screen < 0) return false;
		
	mapscr *m = &TheMaps[(map*MAPSCRS)+screen];
	
	if(m->data.empty()) return false;
	
	if(m->valid==0) return false;
	
	if(!_walkflag(bx,by,1, m))
		return true;
		
	bool ret = true;
	
	for(int32_t i=2; i>=0; i--)
	{
		int32_t c = MAPCOMBO3(map, screen, i-1,bx,by);
		int32_t t = combobuf[c].type;
		
		if(i == 0 && (t == cHOOKSHOTONLY || t == cLADDERHOOKSHOT)) return true;
		
		//bool dried = (iswater_type(t) && DRIEDLAKE);
		
		int32_t b=1;
		
		if(bx&8) b<<=2;
		
		if(by&8) b<<=1;
		
		if(combobuf[c].walk&b && !(combo_class_buf[t].ladder_pass && t!=cLADDERONLY) && t!=cHOOKSHOTONLY)
			ret = false;
	}
	
	return ret;
}

bool hiddenstair(int32_t tmp,bool redraw)                       // tmp = index of tmpscr[]
{
    return hiddenstair2(tmp == 0 ? &tmpscr : &special_warp_return_screen, redraw);
}

bool hiddenstair2(mapscr *s,bool redraw)                       // tmp = index of tmpscr[]
{
    
    if((s->stairx || s->stairy) && s->secretcombo[sSTAIRS])
    {
        int32_t di = COMBOPOS(s->stairx,s->stairy);
        s->data[di] = s->secretcombo[sSTAIRS];
        s->cset[di] = s->secretcset[sSTAIRS];
        s->sflag[di] = s->secretflag[sSTAIRS];
        
        if(redraw)
            putcombo(scrollbuf,s->stairx,s->stairy,s->data[di],s->cset[di]);
            
        return true;
    }
    
    return false;
}

bool remove_screenstatecombos(int32_t tmp, int32_t what1, int32_t what2)
{
	mapscr *s = tmp == 0 ? &tmpscr : &special_warp_return_screen;
	mapscr *t = tmpscr2;
	return remove_screenstatecombos2(s, t, what1, what2);
}

bool remove_screenstatecombos2(mapscr *s, mapscr *t, int32_t what1, int32_t what2)
{
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
	
	if(t)
	{
		for(int32_t j=0; j<6; j++)
		{
			if(t[j].data.empty()) continue;
			
			for(int32_t i=0; i<176; i++)
			{
				newcombo const& cmb = combobuf[t[j].data[i]];
				if(cmb.usrflags&cflag16) continue; //custom state instead of normal state
				if((cmb.type== what1) || (cmb.type== what2))
				{
					t[j].data[i]++;
					didit=true;
				}
			}
		}
	}
	
	return didit;
}

// TODO z3
bool remove_xstatecombos_old(int32_t tmp, byte xflag)
{
	return remove_xstatecombos_old(tmp, (currmap*MAPSCRSNORMAL)+homescr, xflag);
}
bool remove_xstatecombos_old(int32_t tmp, int32_t mi, byte xflag)
{
	mapscr *s = tmp == 0 ? &tmpscr : &special_warp_return_screen;
	return remove_xstatecombos2(s, currscr, mi, xflag);
}
bool remove_xstatecombos2(mapscr *s, int32_t scr, byte xflag)
{
	int mi = (currmap * MAPSCRSNORMAL) + (scr >= 0x80 ? homescr : scr);
	return remove_xstatecombos2(s, scr, mi, xflag);
}
bool remove_xstatecombos2(mapscr *s, int32_t scr, int32_t mi, byte xflag)
{
	bool didit=false;
	
	for(int32_t j=-1; j<6; j++)
	{
		if (j != -1) s = get_layer_scr(currmap, scr, j);
		if (s->data.empty()) continue;
		
		for (int32_t i=0; i<176; i++)
		{
			// TODO z3 very slow! prob best to figure out how to not call this function so much.
			newcombo const& cmb = combobuf[s->data[i]];
			if(!(cmb.usrflags&cflag16)) continue; //custom state instead of normal state
			switch(cmb.type)
			{
				case cLOCKBLOCK: case cLOCKBLOCK2:
				case cBOSSLOCKBLOCK: case cBOSSLOCKBLOCK2:
				case cCHEST: case cCHEST2:
				case cLOCKEDCHEST: case cLOCKEDCHEST2:
				case cBOSSCHEST: case cBOSSCHEST2:
				{
					if(cmb.attribytes[5] == xflag && getxmapflag(mi, 1<<xflag))
					{
						s->data[i]++;
						didit=true;
					}
					break;
				}
			}
		}
	}
	
	return didit;
}

// TODO z3
void clear_xstatecombos_old(int32_t tmp)
{
	clear_xstatecombos_old(tmp, (currmap*MAPSCRSNORMAL)+homescr);
}
void clear_xstatecombos_old(int32_t tmp, int32_t mi)
{
	mapscr *s = tmp == 0 ? &tmpscr : &special_warp_return_screen;
	clear_xstatecombos2(s, currscr, mi);
}

void clear_xstatecombos2(mapscr *s, int32_t scr, int32_t mi)
{
	for(byte q = 0; q < 32; ++q)
	{
		remove_xstatecombos2(s,scr,mi,q);
	}
}

// TODO z3 refactor all this :)
bool remove_lockblocks(int32_t tmp)
{
    return remove_screenstatecombos(tmp, cLOCKBLOCK, cLOCKBLOCK2);
}

bool remove_bosslockblocks(int32_t tmp)
{
    return remove_screenstatecombos(tmp, cBOSSLOCKBLOCK, cBOSSLOCKBLOCK2);
}

bool remove_chests(int32_t tmp)                 // tmp = index of tmpscr[]
{
    return remove_screenstatecombos(tmp, cCHEST, cCHEST2);
}

bool remove_lockedchests(int32_t tmp)                 // tmp = index of tmpscr[]
{
    return remove_screenstatecombos(tmp, cLOCKEDCHEST, cLOCKEDCHEST2);
}

bool remove_bosschests(int32_t tmp)                 // tmp = index of tmpscr[]
{
    return remove_screenstatecombos(tmp, cBOSSCHEST, cBOSSCHEST2);
}


bool overheadcombos(mapscr *s)
{
    for(int32_t i=0; i<176; i++)
    {
//    if (combobuf[s->data[i]].type==cOLD_OVERHEAD)
        if(combo_class_buf[combobuf[s->data[i]].type].overhead)
        {
            return true;
        }
    }
    
    return false;
}

void delete_fireball_shooter(mapscr *s, int32_t i)
{
    int32_t cx=0, cy=0;
    int32_t ct=combobuf[s->data[i]].type;
    
    if(ct!=cL_STATUE && ct!=cR_STATUE && ct!=cC_STATUE)
        return;
        
    switch(ct)
    {
    case cL_STATUE:
        cx=((i&15)<<4)+4;
        cy=(i&0xF0)+7;
        break;
        
    case cR_STATUE:
        cx=((i&15)<<4)-8;
        cy=(i&0xF0)-1;
        break;
        
    case cC_STATUE:
        cx=((i&15)<<4);
        cy=(i&0xF0);
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

int32_t findtrigger(int32_t scombo, bool ff)
{
    int32_t checkflag=0;
    int32_t iter;
    int32_t ret = 0;
    
    for(int32_t j=0; j<(ff?32:176); j++)
    {
        if(ff)
        {
            checkflag=combobuf[tmpscr.ffdata[j]].flag;
            iter=1;
        }
        else iter=2;
        
        for(int32_t layer=-1; !ff && layer<6; layer++)
        {
            if(layer>-1 && tmpscr2[layer].valid==0) continue;
            
            for(int32_t i=0; i<iter; i++)
            {
                if(i==0&&!ff)
                    checkflag = (layer>-1 ? combobuf[tmpscr2[layer].data[j]].flag : combobuf[tmpscr.data[j]].flag);
                else if(i==1&&!ff)
                    checkflag = (layer>-1 ? tmpscr2[layer].sflag[j] : tmpscr.sflag[j]);
                    
                switch(checkflag)
                {
                case mfBCANDLE:
                case mfRCANDLE:
                case mfWANDFIRE:
                case mfDINSFIRE:
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
                    if(scombo!=j)
                        ret += 1;
					[[fallthrough]];
                default:
                    break;
                }
            }
        }
    }
    
    return ret;
}

// single:
// >-1 : the singular triggering combo
// -1: triggered by some other cause
// -2: triggered by Enemies->Secret
// -3: triggered by Secrets screen state
// -4: Screen->TriggerSecrets()
// -5: triggered by Items->Secret
// -5: triggered by Generic Combo
// TODO z3 take pos_handle or a new screen_handle ???
void trigger_secrets_for_screen(int32_t screen, bool high16only, int32_t single)
{
	//There are no calls to 'hidden_entrance' in the code where tmp != 0
	Z_eventlog("%sScreen Secrets triggered%s.\n",
			   single>-1? "Restricted ":"",
			   single==-2? " by the 'Enemies->Secret' screen flag":
			   single==-3? " by the 'Secrets' screen state" :
			   single==-4? " by a script":
			   single==-5? " by Items->Secret":
			   single==-6? " by Generic Combo":
			   single==-7? " by Light Triggers":
			   single==-8? " by SCC":
			   "");
	if(single < 0)
		triggered_screen_secrets = true;
	bool do_layers = true;
	hidden_entrance2(screen, NULL, do_layers, high16only, single);
}

// TODO z3 remove
void hidden_entrance(int32_t tmp, bool refresh, bool high16only, int32_t single) //Perhaps better known as 'Trigger Secrets'
{
	//There are no calls to 'hidden_entrance' in the code where tmp != 0
	Z_eventlog("%sScreen Secrets triggered%s.\n",
			   single>-1? "Restricted ":"",
			   single==-2? " by the 'Enemies->Secret' screen flag":
			   single==-3? " by the 'Secrets' screen state" :
			   single==-4? " by a script":
			   single==-5? " by Items->Secret":
			   single==-6? " by Generic Combo":
			   single==-7? " by Light Triggers":
			   single==-8? " by SCC":
			   "");
	if(single < 0)
		triggered_screen_secrets = true;
	bool do_layers = true;
	hidden_entrance2(-1, tmp == 0 ? &tmpscr : &special_warp_return_screen, do_layers, high16only, single);
}

void hidden_entrance2(int32_t screen, mapscr *s, bool do_layers, bool high16only, int32_t single) //Perhaps better known as 'Trigger Secrets'
{
	DCHECK(screen != -1 || s);
	if (!s) s = get_scr(currmap, screen);
	if (screen == -1) screen = currscr;

	/*
	mapscr *s = tmp == 0 ? &tmpscr : &special_warp_return_screen;
	mapscr *t = tmpscr2;
	*/
	int32_t ft=0; //Flag trigger?
	int32_t msflag=0; // Misc. secret flag
	
	for(int32_t i=0; i<176; i++) //Do the 'trigger flags' (non 16-31)
	{
		if(single>=0 && i!=single) continue; //If it's got a singular flag and i isn't where the flag is
		
		bool putit;
		
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
				putit=true;
				int32_t checkflag=combobuf[s->data[i]].flag; //Inherent
				
				if(iter==1) checkflag=s->sflag[i]; //Placed
				
				switch(checkflag)
				{
				case mfBCANDLE:
					ft=sBCANDLE;
					break;
					
				case mfRCANDLE:
					ft=sRCANDLE;
					break;
					
				case mfWANDFIRE:
					ft=sWANDFIRE;
					break;
					
				case mfDINSFIRE:
					ft=sDINSFIRE;
					break;
					
				case mfARROW:
					ft=sARROW;
					break;
					
				case mfSARROW:
					ft=sSARROW;
					break;
					
				case mfGARROW:
					ft=sGARROW;
					break;
					
				case mfSBOMB:
					ft=sSBOMB;
					break;
					
				case mfBOMB:
					ft=sBOMB;
					break;
					
				case mfBRANG:
					ft=sBRANG;
					break;
					
				case mfMBRANG:
					ft=sMBRANG;
					break;
					
				case mfFBRANG:
					ft=sFBRANG;
					break;
					
				case mfWANDMAGIC:
					ft=sWANDMAGIC;
					break;
					
				case mfREFMAGIC:
					ft=sREFMAGIC;
					break;
					
				case mfREFFIREBALL:
					ft=sREFFIREBALL;
					break;
					
				case mfSWORD:
					ft=sSWORD;
					break;
					
				case mfWSWORD:
					ft=sWSWORD;
					break;
					
				case mfMSWORD:
					ft=sMSWORD;
					break;
					
				case mfXSWORD:
					ft=sXSWORD;
					break;
					
				case mfSWORDBEAM:
					ft=sSWORDBEAM;
					break;
					
				case mfWSWORDBEAM:
					ft=sWSWORDBEAM;
					break;
					
				case mfMSWORDBEAM:
					ft=sMSWORDBEAM;
					break;
					
				case mfXSWORDBEAM:
					ft=sXSWORDBEAM;
					break;
					
				case mfHOOKSHOT:
					ft=sHOOKSHOT;
					break;
					
				case mfWAND:
					ft=sWAND;
					break;
					
				case mfHAMMER:
					ft=sHAMMER;
					break;
					
				case mfSTRIKE:
					ft=sSTRIKE;
					break;
				
				case mfSECRETSNEXT:
					ft=sSECNEXT;
					break;
					
				default:
					putit = false;
					break;
				}
				
				if(putit)  //Change the combos for the secret
				{
					// Use misc. secret flag instead if one is present
					if(msflag!=0)
						ft=msflag;
						
					screen_combo_modify_preroutine(s,i);
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
					screen_combo_modify_postroutine(s,i);
				}
			}
			
			if(newflag >-1) s->sflag[i] = newflag; //Tiered secret
			
			if (do_layers)
			{
				for(int32_t j=0; j<6; j++)  //Layers
				{
					mapscr* layer_scr = get_layer_scr(currmap, screen, j);
					if (layer_scr->data.empty() || layer_scr->cset.empty()) continue; //If layer isn't used
					
					if (single>=0 && i!=single) continue; //If it's got a singular flag and i isn't where the flag is
					
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
						putit=true;
						int32_t checkflag=combobuf[layer_scr->data[i]].flag; //Inherent
						
						if(iter==1) checkflag=layer_scr->sflag[i];  //Placed
						
						switch(checkflag)
						{
							case mfBCANDLE:
								ft=sBCANDLE;
								break;
							
							case mfRCANDLE:
								ft=sRCANDLE;
								break;
							
							case mfWANDFIRE:
								ft=sWANDFIRE;
								break;
							
							case mfDINSFIRE:
								ft=sDINSFIRE;
								break;
							
							case mfARROW:
								ft=sARROW;
								break;
							
							case mfSARROW:
								ft=sSARROW;
								break;
							
							case mfGARROW:
								ft=sGARROW;
								break;
							
							case mfSBOMB:
								ft=sSBOMB;
								break;
							
							case mfBOMB:
								ft=sBOMB;
								break;
							
							case mfBRANG:
								ft=sBRANG;
								break;
							
							case mfMBRANG:
								ft=sMBRANG;
								break;
							
							case mfFBRANG:
								ft=sFBRANG;
								break;
							
							case mfWANDMAGIC:
								ft=sWANDMAGIC;
								break;
							
							case mfREFMAGIC:
								ft=sREFMAGIC;
								break;
							
							case mfREFFIREBALL:
								ft=sREFFIREBALL;
								break;
							
							case mfSWORD:
								ft=sSWORD;
								break;
							
							case mfWSWORD:
								ft=sWSWORD;
								break;
							
							case mfMSWORD:
								ft=sMSWORD;
								break;
							
							case mfXSWORD:
								ft=sXSWORD;
								break;
							
							case mfSWORDBEAM:
								ft=sSWORDBEAM;
								break;
							
							case mfWSWORDBEAM:
								ft=sWSWORDBEAM;
								break;
							
							case mfMSWORDBEAM:
								ft=sMSWORDBEAM;
								break;
							
							case mfXSWORDBEAM:
								ft=sXSWORDBEAM;
								break;
							
							case mfHOOKSHOT:
								ft=sHOOKSHOT;
								break;
							
							case mfWAND:
								ft=sWAND;
								break;
							
							case mfHAMMER:
								ft=sHAMMER;
								break;
							
							case mfSTRIKE:
								ft=sSTRIKE;
								break;
							
							case mfSECRETSNEXT:
								ft=sSECNEXT;
								break;
							
							default:
								putit = false;
								break;
						}
						
						if(putit)  //Change the combos for the secret
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
								addenemy((i&15)<<4,i&0xF0,(cs<<12)+eSPINTILE1,combobuf[c].o_tile+zc_max(1,combobuf[c].frames));
						}
					}
					
					if(newflag2 >-1) layer_scr->sflag[i] = newflag2;  //Tiered secret
				}
			}
		}
	}
	
	for(int32_t i=0; i<32; i++) //FFC 'trigger flags'
	{
		if(single>=0) if(i+176!=single) continue;
		
		bool putit;
		
		if((!high16only)||(single>=0))
		{
			//for (int32_t iter=0; iter<1; ++iter) // Only one kind of FFC flag now.
			{
				putit=true;
				int32_t checkflag=combobuf[s->ffdata[i]].flag; //Inherent
				
				//No placed flags yet
				switch(checkflag)
				{
					case mfBCANDLE:
						ft=sBCANDLE;
						break;
						
					case mfRCANDLE:
						ft=sRCANDLE;
						break;
						
					case mfWANDFIRE:
						ft=sWANDFIRE;
						break;
						
					case mfDINSFIRE:
						ft=sDINSFIRE;
						break;
						
					case mfARROW:
						ft=sARROW;
						break;
						
					case mfSARROW:
						ft=sSARROW;
						break;
						
					case mfGARROW:
						ft=sGARROW;
						break;
						
					case mfSBOMB:
						ft=sSBOMB;
						break;
						
					case mfBOMB:
						ft=sBOMB;
						break;
						
					case mfBRANG:
						ft=sBRANG;
						break;
						
					case mfMBRANG:
						ft=sMBRANG;
						break;
						
					case mfFBRANG:
						ft=sFBRANG;
						break;
						
					case mfWANDMAGIC:
						ft=sWANDMAGIC;
						break;
						
					case mfREFMAGIC:
						ft=sREFMAGIC;
						break;
						
					case mfREFFIREBALL:
						ft=sREFFIREBALL;
						break;
						
					case mfSWORD:
						ft=sSWORD;
						break;
						
					case mfWSWORD:
						ft=sWSWORD;
						break;
						
					case mfMSWORD:
						ft=sMSWORD;
						break;
						
					case mfXSWORD:
						ft=sXSWORD;
						break;
						
					case mfSWORDBEAM:
						ft=sSWORDBEAM;
						break;
						
					case mfWSWORDBEAM:
						ft=sWSWORDBEAM;
						break;
						
					case mfMSWORDBEAM:
						ft=sMSWORDBEAM;
						break;
						
					case mfXSWORDBEAM:
						ft=sXSWORDBEAM;
						break;
						
					case mfHOOKSHOT:
						ft=sHOOKSHOT;
						break;
						
					case mfWAND:
						ft=sWAND;
						break;
						
					case mfHAMMER:
						ft=sHAMMER;
						break;
						
					case mfSTRIKE:
						ft=sSTRIKE;
						break;
					
					case mfSECRETSNEXT:
						ft=sSECNEXT;
						break;
						
					default:
						putit = false;
						break;
				}
				
				if(putit)  //Change the ffc's combo
				{
					if(ft==sSECNEXT)
					{
						s->ffdata[i]++;
					}
					else
					{
						s->ffdata[i] = s->secretcombo[ft];
						s->ffcset[i] = s->secretcset[ft];
					}
				}
			}
		}
	}
	
	if(checktrigger) //Hit all triggers->16-31
	{
		checktrigger=false;
		
		if(tmpscr.flags6&fTRIGGERF1631)
		{
			int32_t tr = findtrigger(-1,false);  //Normal flags
			
			if(tr)
			{
				Z_eventlog("Hit All Triggers->16-31 not fulfilled (%d trigger flag%s remain).\n", tr, tr>1?"s":"");
				goto endhe;
			}
			
			int32_t ftr = findtrigger(-1,true); //FFCs
			
			if(ftr)
			{
				Z_eventlog("Hit All Triggers->16-31 not fulfilled (%d trigger FFC%s remain).\n", ftr, ftr>1?"s":"");
				goto endhe;
			}
		}
	}
	
	for(int32_t i=0; i<176; i++) // Do the 16-31 secrets
	{
		//If it's an enemies->secret screen, only do the high 16 if told to
		//That way you can have secret and burn/bomb entrance separately
		bool old_enem_secret = get_bit(quest_rules,qr_ENEMIES_SECRET_ONLY_16_31);
		if(((!(old_enem_secret && (s->flags2&fCLEARSECRET)) /*Enemies->Secret*/ && single < 0) || high16only || s->flags4&fENEMYSCRTPERM))
		{
			int32_t newflag = -1;
			
			for(int32_t iter=0; iter<2; ++iter)
			{
				int32_t checkflag=combobuf[s->data[i]].flag; //Inherent
				
				if(iter==1) checkflag=s->sflag[i];  //Placed
				
				if((checkflag > 15)&&(checkflag < 32)) //If we've got a 16->32 flag change the combo
				{
					screen_combo_modify_preroutine(s,i);
					s->data[i] = s->secretcombo[checkflag-16+4];
					s->cset[i] = s->secretcset[checkflag-16+4];
					newflag = s->secretflag[checkflag-16+4];
					screen_combo_modify_postroutine(s,i);
				}
			}
			
			if(newflag >-1) s->sflag[i] = newflag;  //Tiered flag
			
			if (do_layers)
			{
				for(int32_t j=0; j<6; j++)  //Layers
				{
					mapscr* layer_scr = get_layer_scr(currmap, screen, j);
					if (layer_scr->data.empty() || layer_scr->cset.empty()) continue; //If layer is not valid (surely checking for 'valid' would be better?)
					
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
		
		/*
		  if(putit && refresh)
		  putcombo(scrollbuf,(i&15)<<4,i&0xF0,s->data[i],s->cset[i]);
		  */
	}
	
	for(int32_t i=0; i<32; i++) // FFCs
	{
		if((!(s->flags2&fCLEARSECRET) /*Enemies->Secret*/ && single < 0) || high16only || s->flags4&fENEMYSCRTPERM)
		{
			for(int32_t iter=0; iter<1; ++iter)  // Only one kind of FFC flag now.
			{
				int32_t checkflag=combobuf[s->ffdata[i]].flag; //Inherent
				
				//No placed flags yet
				if((checkflag > 15)&&(checkflag < 32)) //If we find a flag, change the combo
				{
					s->ffdata[i] = s->secretcombo[checkflag-16+4];
					s->ffcset[i] = s->secretcset[checkflag-16+4];
				}
			}
		}
	}
	
endhe:

	if(tmpscr.flags4&fDISABLETIME) //Finish timed warp if 'Secrets Disable Timed Warp'
	{
		activated_timed_warp=true;
		tmpscr.timedwarptics = 0;
	}
}

// x,y are world coordinates.
static bool has_flag_trigger(int32_t x, int32_t y, int32_t flag, int32_t& out_scombo, bool& out_single16)
{
	if (x < 0 || y < 0 || x >= world_w || y >= world_h) return false;

    bool found_cflag = false;
    bool found_nflag = false;
	bool single16 = false;
	int32_t scombo = -1;

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
                scombo = COMBOPOS(x, y);
            }
            else if ((MAPCOMBOFLAG2(i, x, y) == mfSINGLE16) && (MAPFLAG2(i, x, y) == flag))
            {
                scombo = COMBOPOS(x, y);
                single16 = true;
            }
        }
        
        if (found_cflag) // Trigger->Self (a.k.a Singular) is non-inherent
        {
            if ((MAPFLAG2(i, x, y) == mfSINGLE) && (MAPCOMBOFLAG2(i, x, y) == flag))
            {
                scombo = COMBOPOS(x, y);
            }
            else if ((MAPFLAG2(i, x, y) == mfSINGLE16) && (MAPCOMBOFLAG2(i, x, y) == flag))
            {
                scombo = COMBOPOS(x, y);
                single16 = true;
            }
        }
    }

	out_scombo = scombo;
	out_single16 = single16;
	return found_nflag || found_cflag || MAPFFCOMBOFLAG(x,y) == flag;
}

// TODO: rename to 'trigger_secrets_if_flag'
bool findentrance(int32_t x, int32_t y, int32_t flag, bool setflag)
{
	if (is_z3_scrolling_mode())
	{
		if (x < -16 || y < -16 || x >= world_w || y >= world_h) return false;

		mapscr* scr = NULL;
		int32_t screen = -1;
		int32_t scombo = -1;
		bool single16 = false;
		if (has_flag_trigger(x, y, flag, scombo, single16))
		{
			screen = z3_get_scr_for_xy_offset(x, y);
		}
		else if (has_flag_trigger(x + 15, y, flag, scombo, single16))
		{
			screen = z3_get_scr_for_xy_offset(x + 15, y);
		}
		else if (has_flag_trigger(x, y + 15, flag, scombo, single16))
		{
			screen = z3_get_scr_for_xy_offset(x, y + 15);
		}
		else if (has_flag_trigger(x + 15, y + 15, flag, scombo, single16))
		{
			screen = z3_get_scr_for_xy_offset(x + 15, y + 15);
		}
		if (screen != -1) scr = get_scr(currmap, screen);
		if (!scr) return false;

		if (scombo < 0)
		{
			checktrigger = true;
			trigger_secrets_for_screen(screen);
		}
		else
		{
			checktrigger = true;
			trigger_secrets_for_screen(screen, single16, scombo);
		}
		
		sfx(scr->secretsfx);
		
		if(scr->flags6&fTRIGGERFPERM)
		{
			// TODO z3 findtrigger
			int32_t tr = findtrigger(-1, false);  //Normal flags
			
			if(tr)
			{
				Z_eventlog("Hit All Triggers->Perm Secret not fulfilled (%d trigger flag%s remain).\n", tr, tr>1?"s":"");
				setflag=false;
			}
			
			int32_t ftr = findtrigger(-1, true); //FFCs
			
			if(ftr)
			{
				Z_eventlog("Hit All Triggers->Perm Secret not fulfilled (%d trigger FFC%s remain).\n", ftr, ftr>1?"s":"");
				setflag=false;
			}
			
			if(!(tr||ftr) && !get_bit(quest_rules, qr_ALLTRIG_PERMSEC_NO_TEMP))
			{
				trigger_secrets_for_screen(screen, true, scr->flags6&fTRIGGERF1631);
			}
		}
		
		if(setflag && canPermSecret())
			if(!(scr->flags5&fTEMPSECRETS))
				setmapflag2(scr, screen, mSECRET);

		return true;
	}
	// TODO z3 reduce code

    bool foundflag=false;
    bool foundcflag=false;
    bool foundnflag=false;
    bool foundfflag=false;
    bool single16=false;
    int32_t scombo=-1;
    
    for(int32_t i=-1; i<6; i++)  // Layers. -1 = screen.
    {
        if(MAPFLAG2(i,x,y)==flag || MAPFLAG2(i,x+15,y)==flag ||
                MAPFLAG2(i,x,y+15)==flag || MAPFLAG2(i,x+15,y+15)==flag)
        {
            foundflag=true;
            foundnflag=true;
			break;
        }
    }
    
    for(int32_t i=-1; i<6; i++)  // Layers. -1 = screen.
    {
        if(MAPCOMBOFLAG2(i,x,y)==flag || MAPCOMBOFLAG2(i,x+15,y)==flag ||
                MAPCOMBOFLAG2(i,x,y+15)==flag || MAPCOMBOFLAG2(i,x+15,y+15)==flag)
        {
            foundflag=true;
            foundcflag=true;
			break;
        }
    }
    
    if(MAPFFCOMBOFLAG(x,y)==flag)
    {
        foundflag=true;
        foundfflag=true;
    }
        
    if(MAPFFCOMBOFLAG(x+15,y)==flag)
    {
        foundflag=true;
        foundfflag=true;
    }
        
    if(MAPFFCOMBOFLAG(x,y+15)==flag)
    {
        foundflag=true;
        foundfflag=true;
    }
        
    if(MAPFFCOMBOFLAG(x+15,y+15)==flag)
    {
        foundflag=true;
        foundfflag=true;
    }
        
    if(!foundflag)
    {
        return false;
    }
    
    for(int32_t i=-1; i<6; i++)  // Look for Trigger->Self on all layers
    {
        if(foundnflag) // Trigger->Self (a.k.a Singular) is inherent
        {
            if((MAPCOMBOFLAG2(i,x,y)==mfSINGLE)&&(MAPFLAG2(i,x,y)==flag))
            {
                scombo=COMBOPOS(x,y);
            }
            else if((MAPCOMBOFLAG2(i,x,y)==mfSINGLE16)&&(MAPFLAG2(i,x,y)==flag))
            {
                scombo=COMBOPOS(x,y);
                single16=true;
            }
            else if((MAPCOMBOFLAG2(i,x+15,y)==mfSINGLE)&&(MAPFLAG2(i,x+15,y)==flag))
            {
                scombo=COMBOPOS(x+15,y);
            }
            else if((MAPCOMBOFLAG2(i,x+15,y)==mfSINGLE16)&&(MAPFLAG2(i,x+15,y)==flag))
            {
                scombo=COMBOPOS(x+15,y);
                single16=true;
            }
            else if((MAPCOMBOFLAG2(i,x,y+15)==mfSINGLE)&&(MAPFLAG2(i,x,y+15)==flag))
            {
                scombo=COMBOPOS(x,y+15);
            }
            else if((MAPCOMBOFLAG2(i,x,y+15)==mfSINGLE16)&&(MAPFLAG2(i,x,y+15)==flag))
            {
                scombo=COMBOPOS(x,y+15);
                single16=true;
            }
            else if((MAPCOMBOFLAG2(i,x+15,y+15)==mfSINGLE)&&(MAPFLAG2(i,x+15,y+15)==flag))
            {
                scombo=COMBOPOS(x+15,y+15);
            }
            else if((MAPCOMBOFLAG2(i,x+15,y+15)==mfSINGLE16)&&(MAPFLAG2(i,x+15,y+15)==flag))
            {
                scombo=COMBOPOS(x+15,y+15);
                single16=true;
            }
        }
        
        if(foundcflag) // Trigger->Self (a.k.a Singular) is non-inherent
        {
            if((MAPFLAG2(i,x,y)==mfSINGLE)&&(MAPCOMBOFLAG2(i,x,y)==flag))
            {
                scombo=COMBOPOS(x,y);
            }
            else if((MAPFLAG2(i,x,y)==mfSINGLE16)&&(MAPCOMBOFLAG2(i,x,y)==flag))
            {
                scombo=COMBOPOS(x,y);
                single16=true;
            }
            else if((MAPFLAG2(i,x+15,y)==mfSINGLE)&&(MAPCOMBOFLAG2(i,x+15,y)==flag))
            {
                scombo=COMBOPOS(x+15,y);
            }
            else if((MAPFLAG2(i,x+15,y)==mfSINGLE16)&&(MAPCOMBOFLAG2(i,x+15,y)==flag))
            {
                scombo=COMBOPOS(x+15,y);
                single16=true;
            }
            else if((MAPFLAG2(i,x,y+15)==mfSINGLE)&&(MAPCOMBOFLAG2(i,x,y+15)==flag))
            {
                scombo=COMBOPOS(x,y+15);
            }
            else if((MAPFLAG2(i,x,y+15)==mfSINGLE16)&&(MAPCOMBOFLAG2(i,x,y+15)==flag))
            {
                scombo=COMBOPOS(x,y+15);
                single16=true;
            }
            else if((MAPFLAG2(i,x+15,y+15)==mfSINGLE)&&(MAPCOMBOFLAG2(i,x+15,y+15)==flag))
            {
                scombo=COMBOPOS(x+15,y+15);
            }
            else if((MAPFLAG2(i,x+15,y+15)==mfSINGLE16)&&(MAPCOMBOFLAG2(i,x+15,y+15)==flag))
            {
                scombo=COMBOPOS(x+15,y+15);
                single16=true;
            }
        }
    }
    
	if(scombo<0)
	{
		checktrigger=true;
		hidden_entrance(0,true);
	}
	else
	{
		checktrigger=true;
		hidden_entrance(0,true,single16,scombo);
	}
    
    sfx(tmpscr.secretsfx);
    
    if(tmpscr.flags6&fTRIGGERFPERM)
    {
        int32_t tr = findtrigger(-1,false);  //Normal flags
        
        if(tr)
        {
            Z_eventlog("Hit All Triggers->Perm Secret not fulfilled (%d trigger flag%s remain).\n", tr, tr>1?"s":"");
            setflag=false;
        }
        
        int32_t ftr = findtrigger(-1,true); //FFCs
        
        if(ftr)
        {
            Z_eventlog("Hit All Triggers->Perm Secret not fulfilled (%d trigger FFC%s remain).\n", ftr, ftr>1?"s":"");
            setflag=false;
        }
		
		if(!(tr||ftr) && !get_bit(quest_rules, qr_ALLTRIG_PERMSEC_NO_TEMP))
		{
			hidden_entrance(0,true,(tmpscr.flags6&fTRIGGERF1631));
		}
    }
    
    if(setflag && canPermSecret())
        if(!(tmpscr.flags5&fTEMPSECRETS))
            setmapflag(mSECRET);
            
    return true;
}

void update_freeform_combos()
{
    ffscript_engine(false);
    if ( !FFCore.system_suspend[susptUPDATEFFC] )
    {
	    for(int32_t i=0; i<32; i++)
	    {
		// Combo 0?
		if(tmpscr.ffdata[i]==0)
		    continue;
		    
		// Changer?
		if(tmpscr.ffflags[i]&ffCHANGER)
		    continue;
		    
		// Stationary?
		if(tmpscr.ffflags[i]&ffSTATIONARY)
		    continue;
		    
		// Frozen because Hero's holding up an item?
		if(Hero.getHoldClk()>0 && (tmpscr.ffflags[i]&ffIGNOREHOLDUP)==0)
		    continue;
		    
		// Check for changers
		if(tmpscr.fflink[i]==0)
		{
		    for(int32_t j=0; j<32; j++)
		    {
			// Combo 0?
			if(tmpscr.ffdata[j]==0)
			    continue;
			    
			// Not a changer?
			if(!(tmpscr.ffflags[j]&ffCHANGER))
			    continue;
			    
			// Ignore this changer? (ffposx and ffposy are last changer position)
			if((tmpscr.ffx[j]/10000==ffposx[i]&&tmpscr.ffy[j]/10000==ffposy[i]) || tmpscr.ffflags[i]&ffIGNORECHANGER)
			    continue;
			    
			if((isonline(tmpscr.ffx[i], tmpscr.ffy[i], ffprvx[i],ffprvy[i], tmpscr.ffx[j], tmpscr.ffy[j]) || // Along the line, or...
				//(tmpscr.ffx[i]==tmpscr.ffx[j] && tmpscr.ffy[i]==tmpscr.ffy[j])) && // At exactly the same position, and...
				( // At exactly the same position, 
					(tmpscr.ffx[i]==tmpscr.ffx[j] && tmpscr.ffy[i]==tmpscr.ffy[j])) 
					||
					//or imprecision and close enough
					( (tmpscr.ffflags[i]&ffIMPRECISIONCHANGER) && ((abs(tmpscr.ffx[i] - tmpscr.ffx[j]) < 10000) && abs(tmpscr.ffy[i] - tmpscr.ffy[j]) < 10000) )
				)
			&& //and...
				(ffprvx[i]>-10000000 && ffprvy[i]>-10000000)) // This isn't the first frame on this screen
			{
			    if(tmpscr.ffflags[j]&ffCHANGETHIS)
			    {
				tmpscr.ffdata[i] = tmpscr.ffdata[j];
				tmpscr.ffcset[i] = tmpscr.ffcset[j];
			    }
			    
			    if(tmpscr.ffflags[j]&ffCHANGENEXT)
				tmpscr.ffdata[i]++;
				
			    if(tmpscr.ffflags[j]&ffCHANGEPREV)
				tmpscr.ffdata[i]--;
				
			    tmpscr.ffdelay[i]=tmpscr.ffdelay[j];
			    tmpscr.ffx[i]=tmpscr.ffx[j];
			    tmpscr.ffy[i]=tmpscr.ffy[j];
			    tmpscr.ffxdelta[i]=tmpscr.ffxdelta[j];
			    tmpscr.ffydelta[i]=tmpscr.ffydelta[j];
			    tmpscr.ffxdelta2[i]=tmpscr.ffxdelta2[j];
			    tmpscr.ffydelta2[i]=tmpscr.ffydelta2[j];
			    tmpscr.fflink[i]=tmpscr.fflink[j];
			    tmpscr.ffwidth[i]=tmpscr.ffwidth[j];
			    tmpscr.ffheight[i]=tmpscr.ffheight[j];
			    
			    if(tmpscr.ffflags[i]&ffCARRYOVER)
				tmpscr.ffflags[i]=tmpscr.ffflags[j]|ffCARRYOVER;
			    else
				tmpscr.ffflags[i]=tmpscr.ffflags[j];
				
			    tmpscr.ffflags[i]&=~ffCHANGER;
			    ffposx[i]=(int16_t)(tmpscr.ffx[j]/10000);
			    ffposy[i]=(int16_t)(tmpscr.ffy[j]/10000);
			    
			    if(combobuf[tmpscr.ffdata[j]].flag>15 && combobuf[tmpscr.ffdata[j]].flag<32)
				tmpscr.ffdata[j]=tmpscr.secretcombo[combobuf[tmpscr.ffdata[j]].flag-16+4];
				
			    if((tmpscr.ffflags[j]&ffSWAPNEXT)||(tmpscr.ffflags[j]&ffSWAPPREV))
			    {
				int32_t k=0;
				
				if(tmpscr.ffflags[j]&ffSWAPNEXT)
				    k=j<31?j+1:0;
				    
				if(tmpscr.ffflags[j]&ffSWAPPREV)
				    k=j>0?j-1:31;
				    
				zc_swap(tmpscr.ffdata[j],tmpscr.ffdata[k]);
				zc_swap(tmpscr.ffcset[j],tmpscr.ffcset[k]);
				zc_swap(tmpscr.ffdelay[j],tmpscr.ffdelay[k]);
				zc_swap(tmpscr.ffxdelta[j],tmpscr.ffxdelta[k]);
				zc_swap(tmpscr.ffydelta[j],tmpscr.ffydelta[k]);
				zc_swap(tmpscr.ffxdelta2[j],tmpscr.ffxdelta2[k]);
				zc_swap(tmpscr.ffydelta2[j],tmpscr.ffydelta2[k]);
				zc_swap(tmpscr.fflink[j],tmpscr.fflink[k]);
				zc_swap(tmpscr.ffwidth[j],tmpscr.ffwidth[k]);
				zc_swap(tmpscr.ffheight[j],tmpscr.ffheight[k]);
				zc_swap(tmpscr.ffflags[j],tmpscr.ffflags[k]);
			    }
			    
			    break;
			}
		    }
		}
		
		if(tmpscr.fflink[i] ? !tmpscr.ffdelay[tmpscr.fflink[i]] : !tmpscr.ffdelay[i])
		{
		    if(tmpscr.fflink[i]&&(tmpscr.fflink[i]-1)!=i)
		    {
			ffprvx[i] = tmpscr.ffx[i];
			ffprvy[i] = tmpscr.ffy[i];
			tmpscr.ffx[i]+=tmpscr.ffxdelta[tmpscr.fflink[i]-1];
			tmpscr.ffy[i]+=tmpscr.ffydelta[tmpscr.fflink[i]-1];
		    }
		    else
		    {
			ffprvx[i] = tmpscr.ffx[i];
			ffprvy[i] = tmpscr.ffy[i];
			tmpscr.ffx[i]+=tmpscr.ffxdelta[i];
			tmpscr.ffy[i]+=tmpscr.ffydelta[i];
			tmpscr.ffxdelta[i]+=tmpscr.ffxdelta2[i];
			tmpscr.ffydelta[i]+=tmpscr.ffydelta2[i];
			
			if(tmpscr.ffxdelta[i]>1280000) tmpscr.ffxdelta[i]=1280000;
			
			if(tmpscr.ffxdelta[i]<-1280000) tmpscr.ffxdelta[i]=-1280000;
			
			if(tmpscr.ffydelta[i]>1280000) tmpscr.ffydelta[i]=1280000;
			
			if(tmpscr.ffydelta[i]<-1280000) tmpscr.ffydelta[i]=-1280000;
		    }
		}
		else
		{
		    if(!tmpscr.fflink[i] || (tmpscr.fflink[i]-1)==i)
			tmpscr.ffdelay[i]--;
		}
		
		// Check if the FFC's off the side of the screen
		
		// Left
		if(tmpscr.ffx[i]<-320000)
		{
		    if(tmpscr.flags6&fWRAPAROUNDFF)
		    {
			tmpscr.ffx[i] = 2880000+(tmpscr.ffx[i]+320000);
			ffprvy[i] = tmpscr.ffy[i];
			ffposx[i]=-1000; // Re-enable previous changer
			ffposy[i]=-1000;
		    }
		    else if(tmpscr.ffx[i]<-640000)
		    {
			tmpscr.ffdata[i]=0;
			tmpscr.ffflags[i]&=~ffCARRYOVER;
		    }
		}
		
		// Right
		else if(tmpscr.ffx[i]>=2880000)
		{
		    if(tmpscr.flags6&fWRAPAROUNDFF)
		    {
			tmpscr.ffx[i] = tmpscr.ffx[i]-2880000-320000;
			ffprvy[i] = tmpscr.ffy[i];
			ffposx[i]=-1000;
			ffposy[i]=-1000;
		    }
		    else
		    {
			tmpscr.ffdata[i]=0;
			tmpscr.ffflags[i]&=~ffCARRYOVER;
		    }
		}
		
		// Top
		if(tmpscr.ffy[i]<-320000)
		{
		    if(tmpscr.flags6&fWRAPAROUNDFF)
		    {
			tmpscr.ffy[i] = 2080000+(tmpscr.ffy[i]+320000);
			ffprvx[i] = tmpscr.ffx[i];
			ffposx[i]=-1000;
			ffposy[i]=-1000;
		    }
		    else if(tmpscr.ffy[i]<-640000)
		    {
			tmpscr.ffdata[i]=0;
			tmpscr.ffflags[i]&=~ffCARRYOVER;
		    }
		}
		
		// Bottom
		else if(tmpscr.ffy[i]>=2080000)
		{
		    if(tmpscr.flags6&fWRAPAROUNDFF)
		    {
			tmpscr.ffy[i] = tmpscr.ffy[i]-2080000-320000;
			ffprvy[i] = tmpscr.ffy[i];
			ffposx[i]=-1000;
			ffposy[i]=-1000;
		    }
		    else
		    {
			tmpscr.ffdata[i]=0;
			tmpscr.ffflags[i]&=~ffCARRYOVER;
		    }
		}
	    }
    }
}

bool hitcombo(int32_t x, int32_t y, int32_t combotype)
{
    return (COMBOTYPE(x,y)==combotype);
}

bool hitflag(int32_t x, int32_t y, int32_t flagtype)
{
    return (MAPFLAG(x,y)==flagtype||MAPCOMBOFLAG(x,y)==flagtype);
}

int32_t nextscr(int32_t dir)
{
    int32_t m = currmap;
    int32_t s = currscr;
    
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
    
    int32_t index = (tmpscr.sidewarpindex >> (dir*2))&3;
    
    // Fun fact: when a scrolling warp is triggered, this function
    // is never even called! - Saf
    if(tmpscr.sidewarptype[index] == 3)                                // scrolling warp
    {
        switch(dir)
        {
        case up:
            if(!(tmpscr.flags2&wfUP))    goto nowarp;
            
            break;
            
        case down:
            if(!(tmpscr.flags2&wfDOWN))  goto nowarp;
            
            break;
            
        case left:
            if(!(tmpscr.flags2&wfLEFT))  goto nowarp;
            
            break;
            
        case right:
            if(!(tmpscr.flags2&wfRIGHT)) goto nowarp;
            
            break;
        }
        
        m = DMaps[tmpscr.sidewarpdmap[index]].map;
        s = tmpscr.sidewarpscr[index] + DMaps[tmpscr.sidewarpdmap[index]].xoff;
    }
    
nowarp:
    if(s<0||s>=128)
        return 0xFFFF;

    return (m<<7) + s;
}

void bombdoor(int32_t x,int32_t y)
{
    if(tmpscr.door[0]==dBOMB && isinRect(x,y,100,0,139,48))
    {
        tmpscr.door[0]=dBOMBED;
        putdoor(scrollbuf,0,0,dBOMBED);
        setmapflag(mDOOR_UP);
        markBmap(-1);
        
        if(nextscr(up)!=0xFFFF)
        {
            setmapflag(nextscr(up), mDOOR_DOWN);
            markBmap(-1,nextscr(up)-(get_currdmap()<<7));
        }
    }
    
    if(tmpscr.door[1]==dBOMB && isinRect(x,y,100,112,139,176))
    {
        tmpscr.door[1]=dBOMBED;
        putdoor(scrollbuf,0,1,dBOMBED);
        setmapflag(mDOOR_DOWN);
        markBmap(-1);
        
        if(nextscr(down)!=0xFFFF)
        {
            setmapflag(nextscr(down), mDOOR_UP);
            markBmap(-1,nextscr(down)-(get_currdmap()<<7));
        }
    }
    
    if(tmpscr.door[2]==dBOMB && isinRect(x,y,0,60,48,98))
    {
        tmpscr.door[2]=dBOMBED;
        putdoor(scrollbuf,0,2,dBOMBED);
        setmapflag(mDOOR_LEFT);
        markBmap(-1);
        
        if(nextscr(left)!=0xFFFF)
        {
            setmapflag(nextscr(left), mDOOR_RIGHT);
            markBmap(-1,nextscr(left)-(get_currdmap()<<7));
        }
    }
    
    if(tmpscr.door[3]==dBOMB && isinRect(x,y,192,60,240,98))
    {
        tmpscr.door[3]=dBOMBED;
        putdoor(scrollbuf,0,3,dBOMBED);
        setmapflag(mDOOR_RIGHT);
        markBmap(-1);
        
        if(nextscr(right)!=0xFFFF)
        {
            setmapflag(nextscr(right), mDOOR_LEFT);
            markBmap(-1,nextscr(right)-(get_currdmap()<<7));
        }
    }
}

void draw_cmb(BITMAP* dest, int32_t x, int32_t y, int32_t cid, int32_t cset,
	bool over, bool transp)
{
	if(over)
	{
		if(transp)
			overcombotranslucent(dest, x, y, cid, cset, 128);
		else overcombo(dest, x, y, cid, cset);
	}
	else putcombo(dest, x, y, cid, cset);
}
void draw_cmb_pos(BITMAP* dest, int32_t x, int32_t y, rpos_t rpos, int32_t cid,
	int32_t cset, byte layer, bool over, bool transp)
{
	byte pos = RPOS_TO_POS(rpos);
	rpos_t plrpos = COMBOPOS_REGION(Hero.x+8, Hero.y+8);
	bool dosw = false;
	if(rpos == hooked_comborpos && (hooked_layerbits & (1<<layer)))
	{
		if(hooked_undercombos[layer] > -1)
		{
			draw_cmb(dest, COMBOX(pos)+x, COMBOY(pos)+y,
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
	draw_cmb(dest, COMBOX(pos)+x, COMBOY(pos)+y, cid, cset, over, transp);
}

// `draw_cmb_pos` only does meaningful work if the combo being drawn is within the bounds of
// the `bmp` bitmap. However, even getting to that point where `puttile16` (for example) knows
// to cull is somewhat expensive. Since it can only draw 16x16 pixels, we can do the equivalent
// culling here by only drawing the rows/columns that are within the bitmap bounds. This nets
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

void do_scrolling_layer(BITMAP *bmp, int32_t type, int32_t map, int32_t scr, int32_t layer, mapscr* basescr, int32_t x, int32_t y, bool scrolling, int32_t tempscreen)
{
	DCHECK(layer >= 0 && layer <= 6);
	x += global_viewport_x;
	y += global_viewport_y;

	mapscr const* tmp = NULL;
	if (!is_z3_scrolling_mode())
	{
		tmp = layer > 0 ?
			(&(tempscreen==2?tmpscr2[layer-1]:tmpscr3[layer-1])) :
			(layer ? NULL : (tempscreen==2 ? &tmpscr : &special_warp_return_screen));
	}
	else if (layer > 0)
	{
		tmp = get_layer_scr(map, scr, layer - 1);
	}

	bool over = true, transp = false;
	
	switch(type ? type : layer)
	{
		case -4: //overhead FFCs
		case -3:                                                //freeform combos
			for(int32_t i = 31; i >= 0; i--)
			{
				if(basescr->ffdata[i])
				{
					if(!(basescr->ffflags[i]&ffCHANGER) //If FFC is a changer, don't draw
						&& !((basescr->ffflags[i]&ffLENSINVIS) && lensclk) //If lens is active and ffc is invis to lens, don't draw
						&& (!(basescr->ffflags[i]&ffLENSVIS) || lensclk)) //If FFC does not require lens, or lens is active, draw
					{
						if(scrolling && (basescr->ffflags[i] & ffCARRYOVER) != 0 && tempscreen == 3)
							continue; //If scrolling, only draw carryover ffcs from newscr and not oldscr,
							
						//otherwise we'll draw the same one twice
						
						if(!!(basescr->ffflags[i]&ffOVERLAY) == (type==-4)) //what exactly is this supposed to mean?
						{
							int32_t tx=((basescr->ffx[i]/10000));
							int32_t ty=((basescr->ffy[i]/10000))+playing_field_offset;
							
							if(basescr->ffflags[i]&ffTRANS)
							{
								overcomboblocktranslucent(bmp, tx-x, ty-y, basescr->ffdata[i], basescr->ffcset[i], 1+(basescr->ffwidth[i]>>6), 1+(basescr->ffheight[i]>>6),128);
							}
							else
							{
								overcomboblock(bmp, tx-x, ty-y, basescr->ffdata[i], basescr->ffcset[i], 1+(basescr->ffwidth[i]>>6), 1+(basescr->ffheight[i]>>6));
							}
						}
					}
				}
			}
			
			return;
			
		case -2:                                                //push blocks
			if(tmp && tmp->valid)
			{
				for(int32_t i=0; i<176; i++)
				{
					int32_t mf=tmp->sflag[i], mf2 = combobuf[tmp->data[i]].flag;
					
					if(mf==mfPUSHUD || mf==mfPUSH4 || mf==mfPUSHED || ((mf>=mfPUSHLR)&&(mf<=mfPUSHRINS))
						|| mf2==mfPUSHUD || mf2==mfPUSH4 || mf2==mfPUSHED || ((mf2>=mfPUSHLR)&&(mf2<=mfPUSHRINS)))
					{
						auto rpos = POS_TO_RPOS(i, scr);
						draw_cmb_pos(bmp, -x, playing_field_offset-y, rpos, tmp->data[i], tmp->cset[i], layer, true, false);
					}
				}
			}
			return;
			
		case -1:                                                //over combo
			if(tmp && tmp->valid)
			{
				for(int32_t i=0; i<176; i++)
				{
					if(combo_class_buf[combobuf[tmp->data[i]].type].overhead)
					{
						auto rpos = POS_TO_RPOS(i, scr);
						draw_cmb_pos(bmp, -x, playing_field_offset-y, rpos, tmp->data[i], tmp->cset[i], layer, true, false);
					}
				}
			}
			return;
			
		case 1:
		case 4:
		case 5:
		case 6:
			if(TransLayers || basescr->layeropacity[layer-1]==255)
			{
				if(tmp && tmp->valid)
				{
					if(basescr->layeropacity[layer-1]!=255)
						transp = true;
					break;
				}
			}
			return;
			
		case 2:
			if(TransLayers || basescr->layeropacity[layer-1]==255)
			{
				if(tmp && tmp->valid)
				{
					if(basescr->layeropacity[layer-1]!=255)
						transp = true;
					
					if(XOR(basescr->flags7&fLAYER2BG, DMaps[currdmap].flags&dmfLAYER2BG))
						over = false;
					
					break;
				}
			}
			return;
			
		case 3:
			if(TransLayers || basescr->layeropacity[layer-1]==255)
			{
				if(tmp && tmp->valid)
				{
					if(basescr->layeropacity[layer-1]!=255)
						transp = true;
					
					if(XOR(basescr->flags7&fLAYER3BG, DMaps[currdmap].flags&dmfLAYER3BG)
						&& !XOR(basescr->flags7&fLAYER2BG, DMaps[currdmap].flags&dmfLAYER2BG))
						over = false;
					
					break;
				}
			}
			return;
	}

	// ?
	x = -x;
	y = -y+playing_field_offset;

	int start_x, end_x, start_y, end_y;
	get_bounds_for_draw_cmb_calls(bmp, x, y, start_x, end_x, start_y, end_y);
	for (int cy = start_y; cy < end_y; cy++)
	{
		for (int cx = start_x; cx < end_x; cx++)
		{
			int i = cx + cy*16;
			auto rpos = POS_TO_RPOS(i, scr);
			draw_cmb_pos(bmp, x, y, rpos, tmp->data[i], tmp->cset[i], layer, over, transp);
		}
	}
}

// TODO z3 remove
void do_layer(BITMAP *bmp, int32_t type, int32_t layer, mapscr* basescr, int32_t x, int32_t y, int32_t tempscreen, bool scrolling, bool drawprimitives)
{
	do_layer(bmp, type, currmap, currscr, layer, basescr, x, y, tempscreen, scrolling, drawprimitives);
}

void do_layer(BITMAP *bmp, int32_t type, int32_t map, int32_t scr, int32_t layer, mapscr* basescr, int32_t x, int32_t y, int32_t tempscreen, bool scrolling, bool drawprimitives)
{
    bool showlayer = true;
    
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
	
    if(!type && (layer==(int32_t)(basescr->lens_layer&7)+1) && ((basescr->lens_layer&llLENSSHOWS && !lensclk) || (basescr->lens_layer&llLENSHIDES && lensclk)))
    {
        showlayer = false;
    }
    
	
    if(showlayer)
    {
		if(type || !(basescr->hidelayers & (1 << (layer))))
			do_scrolling_layer(bmp, type, map, scr, layer, basescr, x, y, scrolling, tempscreen);
        
        if(!type && drawprimitives && layer > 0 && layer <= 6)
        {
			// TODO z3
            do_primitives(bmp, layer, basescr, 0,  playing_field_offset);
        }
    }
}


// Called by do_walkflags
void put_walkflags(BITMAP *dest,int32_t x,int32_t y,int32_t xofs,int32_t yofs, word cmbdat,int32_t lyr)
{
	newcombo const &c = combobuf[cmbdat];
	
	if (c.type == cBRIDGE && get_bit(quest_rules, qr_OLD_BRIDGE_COMBOS)) return;
	
	int32_t xx = x-xofs;
	int32_t yy = y+playing_field_offset-yofs;
	
	int32_t bridgedetected = 0;
	
	for(int32_t i=0; i<4; i++)
	{
		int32_t tx=((i&2)<<2)+xx - global_viewport_x;
		int32_t ty=((i&1)<<3)+yy - global_viewport_y;
		int32_t tx2=((i&2)<<2)+x - global_viewport_x;
		int32_t ty2=((i&1)<<3)+y - global_viewport_y;
		for (int32_t j = lyr-1; j <= 1; j++)
		{
			if (get_bit(quest_rules, qr_OLD_BRIDGE_COMBOS))
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
		//if ( iswaterex(cmbdat, currmap, currscr, lyr, tx2, ty2, true, false, false)!=0 )
		if (iswater_type(c.type) && !DRIEDLAKE) //Yes, I realize this is horribly inaccurate; the alternative is the game chugging every time you turn on walk cheats.
		{
			if (get_bit(quest_rules,  qr_NO_SOLID_SWIM)) doladdercheck = false;
			if((lyr==0 || (get_bit(quest_rules,  qr_WATER_ON_LAYER_1) && lyr == 1) || (get_bit(quest_rules,  qr_WATER_ON_LAYER_2) && lyr == 2)) && get_bit(quest_rules, qr_DROWN))
				rectfill(dest,tx,ty,tx+7,ty+7,makecol(85,85,255));
			else rectfill(dest,tx,ty,tx+7,ty+7,makecol(0,0,255));
		}
		
		if(c.walk&(1<<i) && !(iswater_type(c.type) && DRIEDLAKE))  // Check for dried lake (watertype && not water)
		{
			if(c.type==cLADDERHOOKSHOT && isstepable(cmbdat) && ishookshottable(xx,yy))
			{
				for(int32_t k=0; k<8; k+=2)
					for(int32_t j=0; j<8; j+=2)
						rectfill(dest,tx+k,ty+j,tx+k+1,ty+j+1,((k+j)/2)%2 ? makecol(165,105,8) : makecol(170,170,170));
			}
			else
			{
				int32_t color = makecol(255,85,85);
				
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
					int32_t x0 = x - global_viewport_x;
					int32_t y0 = y - global_viewport_y;
					rectfill(dest,x0+k,y0+j,x0+k+1,y0+j+1,color);
				}
		}
		else
		{
			for(int32_t i=0; i<4; i++)
			{
				if (!(bridgedetected & (1<<i)))
				{
					int32_t tx=((i&2)<<2)+x - global_viewport_x;
					int32_t ty=((i&1)<<3)+y - global_viewport_y;
					for(int32_t k=0; k<8; k+=2)
						for(int32_t j=0; j<8; j+=2)
							if((k+j)%4 < 2) rectfill(dest,tx+k,ty+j,tx+k+1,ty+j+1,color);
				}
			}
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
		int32_t tx=((i&2)<<2)+xx - global_viewport_x;
		int32_t ty=((i&1)<<3)+yy - global_viewport_y;
		int32_t tx2=((i&2)<<2)+x - global_viewport_x;
		int32_t ty2=((i&1)<<3)+y - global_viewport_y;
	
		if(((c.walk>>4)&(1<<i)) && c.type != cNONE)
		{
				int32_t color = vc(10);
					
				rectfill(dest,tx,ty,tx+7,ty+7,color);
		}
	}
}

// Effectflags L4 cheat
void do_effectflags(BITMAP *dest,mapscr* layer,int32_t x, int32_t y, int32_t tempscreen)
{
	if(show_effectflags)
	{
		for(int32_t i=0; i<176; i++)
		{
			put_effectflags(dest,((i&15)<<4),(i&0xF0),x,y,layer->data[i], 0);
		}
	}
}

// Walkflags L4 cheat
void do_walkflags(BITMAP *dest,mapscr* layer,int32_t x, int32_t y, int32_t tempscreen)
{
	if(show_walkflags)
	{
		for(int32_t i=0; i<176; i++)
		{
			put_walkflags(dest,((i&15)<<4),(i&0xF0),x,y,layer->data[i], 0);
		}
		
		for(int32_t k=0; k<2; k++)
		{
			mapscr* lyr = (tempscreen==2 ? tmpscr2+k : tmpscr3+k);
			
			if(lyr->valid)
			{
				if(tempscreen==2)
				{
					for(int32_t i=0; i<176; i++)
					{
						put_walkflags(temp_buf,((i&15)<<4),(i&0xF0),x,y,tmpscr2[k].data[i], k%2+1);
						put_walkflags(scrollbuf,((i&15)<<4),(i&0xF0),x,y,tmpscr2[k].data[i], k%2+1);
					}
				}
				else
				{
					for(int32_t i=0; i<176; i++)
					{
						put_walkflags(temp_buf,((i&15)<<4),(i&0xF0),x,y,tmpscr3[k].data[i], k%2+1);
						put_walkflags(scrollbuf,((i&15)<<4),(i&0xF0),x,y,tmpscr3[k].data[i], k%2+1);
					}
				}
			}
		}
	}
}

void doTorchCircle(BITMAP* bmp, int32_t pos, newcombo const& cmb, int32_t xoffs = 0, int32_t yoffs = 0)
{
	if(cmb.type != cTORCH) return;
	doDarkroomCircle(COMBOX(pos)+8+xoffs, COMBOY(pos)+8+yoffs, cmb.attribytes[0], bmp);
}

void calc_darkroom_combos(int screen, int offx, int offy, bool scrolling)
{
	mapscr* scr = get_scr(currmap, screen);

	int32_t scrolldir = get_bit(quest_rules, qr_NEWDARK_SCROLLEDGE) ? FFCore.ScrollingData[SCROLLDATA_DIR] : -1;
	int32_t scrollxoffs = 0, scrollyoffs = 0;
	switch(scrolldir)
	{
		case up:
			scrollyoffs = -176;
			break;
		case down:
			scrollyoffs = 176;
			break;
		case left:
			scrollxoffs = -256;
			break;
		case right:
			scrollxoffs = 256;
			break;
	}
	for(int32_t q = 0; q < 176; ++q)
	{
		newcombo const& cmb = combobuf[scr->data[q]];
		if(cmb.type == cTORCH)
		{
			doTorchCircle(darkscr_bmp_curscr, q, cmb, offx, offy);
			if(scrolldir > -1)
				doTorchCircle(darkscr_bmp_scrollscr, q, cmb, offx + scrollxoffs, offy + scrollyoffs);
		}
	}
	for(int32_t lyr = 0; lyr < 6; ++lyr)
	{
		mapscr* layer_scr = get_layer_scr(currmap, screen, lyr);
		if(!layer_scr->valid) continue; //invalid layer
		for(int32_t q = 0; q < 176; ++q)
		{
			newcombo const& cmb = combobuf[layer_scr->data[q]];
			if(cmb.type == cTORCH)
			{
				doTorchCircle(darkscr_bmp_curscr, q, cmb, offx, offy);
				if(scrolldir > -1)
					doTorchCircle(darkscr_bmp_scrollscr, q, cmb, offx + scrollxoffs, offy + scrollyoffs);
			}
		}
	}
	for(int q = 0; q < 32; ++q)
	{
		newcombo const& cmb = combobuf[scr->ffdata[q]];
		if(cmb.type == cTORCH)
		{
			doDarkroomCircle(offx+(scr->ffx[q]/10000)+(scr->ffEffectWidth(q)/2), offy+(scr->ffy[q]/10000)+(scr->ffEffectHeight(q)/2), cmb.attribytes[0], darkscr_bmp_curscr);
			if(scrolldir > -1)
				doDarkroomCircle(offx+(scr->ffx[q]/10000)+(scr->ffEffectWidth(q)/2)+scrollxoffs, offy+(scr->ffy[q]/10000)+(scr->ffEffectHeight(q)/2)+scrollyoffs, cmb.attribytes[0], darkscr_bmp_scrollscr);
		}
	}
	
	if(!scrolling) return; //not a scrolling call, don't run code for scrolling screen

	if (is_z3_scrolling_mode()) return; // todo z3 LOL
	
	for(int32_t q = 0; q < 176; ++q)
	{
		newcombo const& cmb = combobuf[special_warp_return_screen.data[q]];
		if(cmb.type == cTORCH)
		{
			doTorchCircle(darkscr_bmp_scrollscr, q, cmb);
			if(scrolldir > -1)
				doTorchCircle(darkscr_bmp_curscr, q, cmb, -scrollxoffs, -scrollyoffs);
		}
	}
	for(int32_t lyr = 0; lyr < 6; ++lyr)
	{
		if(!tmpscr3[lyr].valid) continue; //invalid layer
		for(int32_t q = 0; q < 176; ++q)
		{
			newcombo const& cmb = combobuf[tmpscr3[lyr].data[q]];
			if(cmb.type == cTORCH)
			{
				doTorchCircle(darkscr_bmp_scrollscr, q, cmb);
				if(scrolldir > -1)
					doTorchCircle(darkscr_bmp_curscr, q, cmb, -scrollxoffs, -scrollyoffs);
			}
		}
	}
	for(int q = 0; q < 32; ++q)
	{
		newcombo const& cmb = combobuf[special_warp_return_screen.ffdata[q]];
		if(cmb.type == cTORCH)
		{
			doDarkroomCircle((special_warp_return_screen.ffx[q]/10000)+(special_warp_return_screen.ffEffectWidth(q)/2), (special_warp_return_screen.ffy[q]/10000)+(special_warp_return_screen.ffEffectHeight(q)/2), cmb.attribytes[0], darkscr_bmp_scrollscr);
			if(scrolldir > -1)
				doDarkroomCircle((special_warp_return_screen.ffx[q]/10000)+(special_warp_return_screen.ffEffectWidth(q)/2)-scrollxoffs, (special_warp_return_screen.ffy[q]/10000)+(special_warp_return_screen.ffEffectHeight(q)/2)-scrollyoffs, cmb.attribytes[0], darkscr_bmp_curscr);
		}
	}
}

// TODO z3_scr_dx -> offset_x
void for_every_screen_in_region(const std::function <void (mapscr*, int, unsigned int, unsigned int)>& fn)
{
	if (!is_z3_scrolling_mode())
	{
		fn(&tmpscr, currscr, 0, 0);
		return;
	}

	int z3_scr_x = z3_origin_scr % 16;
	int z3_scr_y = z3_origin_scr / 16;

	for (int scr = 0; scr < 128; scr++)
	{
		if (is_in_region(z3_origin_scr, scr))
		{
			int scr_x = scr % 16;
			int scr_y = scr / 16;
			if (scr_x < z3_scr_x || scr_y < z3_scr_y) continue;

			unsigned int z3_scr_dx = scr_x - z3_scr_x;
			unsigned int z3_scr_dy = scr_y - z3_scr_y;
			mapscr* z3_scr = get_scr(currmap, scr);
			global_z3_cur_scr_drawing = scr;
			fn(z3_scr, scr, z3_scr_dx, z3_scr_dy);
		}
	}

	global_z3_cur_scr_drawing = -1;
}

static void for_every_nearby_screen(const std::function <void (mapscr*, int, int, int)>& fn)
{
	if (!is_z3_scrolling_mode())
	{
		fn(&tmpscr, currscr, 0, 0);
		return;
	}

	int currscr_x = currscr % 16;
	int currscr_y = currscr / 16;

	for (int currscr_dx = 1; currscr_dx >= -1; currscr_dx--)
	{
		for (int currscr_dy = -1; currscr_dy <= 1; currscr_dy++)
		{
			int scr_x = currscr_x + currscr_dx;
			int scr_y = currscr_y + currscr_dy;
			if (tmpscr.flags&fMAZE && !(XY_DELTA_TO_DIR(currscr_dx, 0) == tmpscr.exitdir || XY_DELTA_TO_DIR(0, currscr_dy) == tmpscr.exitdir))
			{
				scr_x = currscr_x;
				scr_y = currscr_y;
			}
			if (currscr_dx || currscr_dy)
			{
				if (Hero.edge_of_dmap(XY_DELTA_TO_DIR(currscr_dx, 0))) continue;
				if (Hero.edge_of_dmap(XY_DELTA_TO_DIR(0, currscr_dy))) continue;
				if (scr_x < 0 || scr_x >= 16) continue;
				if (scr_y < 0 || scr_y >= 8) continue;
			}

			int scr = global_z3_cur_scr_drawing = scr_x + scr_y * 16;
			if (!is_in_region(z3_origin_scr, scr)) continue;

			mapscr* myscr = get_scr(currmap, scr);
			if (!(myscr->valid & mVALID)) continue;

			int offx = z3_get_region_relative_dx(scr) * 256;
			int offy = z3_get_region_relative_dy(scr) * 176;

			if (offx - global_viewport_x <= -256) continue;
			if (offy - global_viewport_y <= -176) continue;
			if (offx - global_viewport_x >= 256) continue;
			if (offy - global_viewport_y >= 176) continue;

			fn(myscr, scr, offx, offy);
		}
	}

	global_z3_cur_scr_drawing = -1;
}

// TODO z3 remove this_screen
void draw_screen(mapscr* this_screen, bool showhero, bool runGeneric)
{
	// DCHECK(this_screen == &tmpscr);
	if((GameFlags & (GAMEFLAG_SCRIPTMENU_ACTIVE|GAMEFLAG_F6SCRIPT_ACTIVE))!=0)
	{
		FFCore.doScriptMenuDraws();
		return;
	}
	
	if(runGeneric) FFCore.runGenericPassiveEngine(SCR_TIMING_PRE_DRAW);
	
	//The Plan:
	//0: Set sideview gravity from dmaps. -Z
	//1. Draw some layers onto scrollbuf with clipping
	//2. Blit scrollbuf onto framebuf
	//3. Draw some sprites onto framebuf
	//4. Blit framebuf onto temp_buf
	//5. Draw some layers onto temp_buf and scrollbuf
	//6. Blit temp_buf onto framebuf with clipping
	//6b. Draw the subscreen onto temp_buf, without clipping
	//7. Draw some flying sprites onto framebuf
	//8. Blit frame_buf onto temp_buf
	//9. Draw some layers onto temp_buf
	//10. Blit temp_buf onto framebuf with clipping
	//11. Draw some text on framebuf and scrollbuf
	//12. Draw the subscreen onto framebuf, without clipping
	clear_bitmap(framebuf);
	set_clip_rect(framebuf,0,0,256,224);
	
	clear_bitmap(temp_buf);
	set_clip_state(temp_buf,1);
	set_clip_rect(temp_buf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2);
	
	int32_t cmby2=0;
	
	//0: Sideview Grvity from DMaps.
	
	/* DON'T MODIFY THE SCREEN DIRECTLY!
	if ( DMaps[currdmap].sideview != 0 ) 
	{
		this_screen->flags7 |= fSIDEVIEW;
	}*/
	//1. Draw some layers onto temp_buf
	clear_bitmap(scrollbuf);

	// TODO z3 move to game loop?
	z3_update_viewport();
	
	for_every_nearby_screen([&](mapscr* myscr, int scr, int offx, int offy) {
		if(XOR(myscr->flags7&fLAYER2BG, DMaps[currdmap].flags&dmfLAYER2BG))
		{
			do_layer(scrollbuf, 0, currmap, scr, 2, myscr, -offx, -offy, 2, false, true);
			if (scr == currscr) particles.draw(temp_buf, true, 1);
		}
		
		if(XOR(myscr->flags7&fLAYER3BG, DMaps[currdmap].flags&dmfLAYER3BG))
		{
			do_layer(scrollbuf, 0, currmap, scr, 3, myscr, -offx, -offy, 2, false, true);
			if (scr == currscr) particles.draw(temp_buf, true, 2);
		}
	});
	
	for_every_nearby_screen([&](mapscr* myscr, int scr, int offx, int offy) {
		putscr(scrollbuf, offx, offy + playing_field_offset, myscr);
	});

	// Lens hints, then primitives, then particles.
	if((lensclk || (get_debug() && zc_getkey(KEY_L))) && !get_bit(quest_rules, qr_OLDLENSORDER))
	{
		draw_lens_under(scrollbuf, false);
	}
	
	// TODO z3
	if(show_layer_0)
		do_primitives(scrollbuf, 0, this_screen, 0, playing_field_offset);
		
	particles.draw(temp_buf, true, -3);
	
	set_clip_rect(scrollbuf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2);
	
	if(!(get_bit(quest_rules,qr_LAYER12UNDERCAVE)))
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
			
			overcombo(scrollbuf,ccx-global_viewport_x,ccy+cmby2+playing_field_offset-global_viewport_y,MAPCOMBO(ccx,ccy+cmby2),MAPCSET(ccx,ccy+cmby2));
			putcombo(scrollbuf,ccx-global_viewport_x,ccy+playing_field_offset-global_viewport_y,MAPCOMBO(ccx,ccy),MAPCSET(ccx,ccy));
			
			if(int32_t(Hero.getX())&15)
			{
				overcombo(scrollbuf,ccx+16-global_viewport_x,ccy+cmby2+playing_field_offset-global_viewport_y,MAPCOMBO(ccx+16,ccy+cmby2),MAPCSET(ccx+16,ccy+cmby2));
				putcombo(scrollbuf,ccx+16-global_viewport_x,ccy+playing_field_offset-global_viewport_y,MAPCOMBO(ccx+16,ccy),MAPCSET(ccx+16,ccy));
			}
		}
	}
	
	for_every_nearby_screen([&](mapscr* myscr, int scr, int offx, int offy) {
		do_layer(scrollbuf, 0, currmap, scr, 1, myscr, -offx, -offy, 2, false, true); // LAYER 1
		if (scr == currscr) particles.draw(temp_buf, true, 0);
		
		do_layer(scrollbuf, -3, currmap, scr, 0, myscr, -offx, -offy, 2); // freeform combos!

		if(!XOR(myscr->flags7&fLAYER2BG, DMaps[currdmap].flags&dmfLAYER2BG))
		{
			do_layer(scrollbuf, 0, currmap, scr, 2, myscr, -offx, -offy, 2, false, true); // LAYER 2
			if (scr == currscr) particles.draw(temp_buf, true, 1);
		}
	});
	
	if(get_bit(quest_rules,qr_LAYER12UNDERCAVE))
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
			
			overcombo(scrollbuf,ccx-global_viewport_x,ccy+cmby2+playing_field_offset-global_viewport_y,MAPCOMBO(ccx,ccy+cmby2),MAPCSET(ccx,ccy+cmby2));
			putcombo(scrollbuf,ccx-global_viewport_x,ccy+playing_field_offset-global_viewport_y,MAPCOMBO(ccx,ccy),MAPCSET(ccx,ccy));
			
			if(int32_t(Hero.getX())&15)
			{
				overcombo(scrollbuf,ccx+16-global_viewport_x,ccy+cmby2+playing_field_offset-global_viewport_y,MAPCOMBO(ccx+16,ccy+cmby2),MAPCSET(ccx+16,ccy+cmby2));
				putcombo(scrollbuf,ccx+16-global_viewport_x,ccy+playing_field_offset-global_viewport_y,MAPCOMBO(ccx+16,ccy),MAPCSET(ccx+16,ccy));
			}
		}
	}
	
	for_every_nearby_screen([&](mapscr* myscr, int scr, int offx, int offy) {
		do_layer(scrollbuf, -2, currmap, scr, 0, myscr, -offx, -offy, 2); // push blocks!
		if(get_bit(quest_rules, qr_PUSHBLOCK_LAYER_1_2))
		{
			do_layer(scrollbuf, -2, currmap, scr, 1, myscr, -offx, -offy, 2); // push blocks!
			do_layer(scrollbuf, -2, currmap, scr, 2, myscr, -offx, -offy, 2); // push blocks!
		}

		// Show walkflags cheat
		do_walkflags(scrollbuf, myscr, -offx, -offy, 2);
		do_effectflags(scrollbuf, myscr, -offx, -offy, 2);
	});
	
	putscrdoors(scrollbuf,0,playing_field_offset,this_screen);
	
	// Lens hints, doors etc.
	if(lensclk || (get_debug() && zc_getkey(KEY_L)))
	{
		if(get_bit(quest_rules, qr_OLDLENSORDER))
		{
			draw_lens_under(scrollbuf, false);
		}
		
		draw_lens_under(scrollbuf, true);
	}
	
	//2. Blit those layers onto framebuf
	
	set_clip_rect(framebuf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2);
	masked_blit(scrollbuf, framebuf, 0, 0, 0, 0, 256, 224);

	// We no longer draw to the scrollbuf - so things like dosubscr have access to a "partially rendered" frame. 
	// I think only used for COOLSCROLL=0? Seems like a silly feature...
	
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
		if(get_bit(quest_rules,qr_NOFLICKER) || (frame&1))
		{
			for(int32_t i=0; i<Ewpns.Count(); i++)
			{
				if(((weapon *)Ewpns.spr(i))->behind)
					Ewpns.spr(i)->draw(framebuf);
			}
		
			for(int32_t i=0; i<Lwpns.Count(); i++)
			{
				if(((weapon *)Lwpns.spr(i))->behind)
					Lwpns.spr(i)->draw(framebuf);
			}
			
			if(get_bit(quest_rules,qr_SHADOWS)&&(!get_bit(quest_rules,qr_SHADOWSFLICKER)||frame&1))
			{
				guys.drawshadow(framebuf,get_bit(quest_rules,qr_TRANSSHADOWS)!=0,true);
			}
			
			guys.draw(framebuf,true);
			chainlinks.draw(framebuf,true);
			//Lwpns.draw(framebuf,true);
			
			for(int32_t i=0; i<Ewpns.Count(); i++)
			{
				if(!((weapon *)Ewpns.spr(i))->behind)
					Ewpns.spr(i)->draw(framebuf);
			}
		
			for(int32_t i=0; i<Lwpns.Count(); i++)
			{
				if(!((weapon *)Lwpns.spr(i))->behind)
					Lwpns.spr(i)->draw(framebuf);
			}
		
			
			items.draw(framebuf,true);
		}
		else
		{
			for(int32_t i=0; i<Ewpns.Count(); i++)
			{
				if(((weapon *)Ewpns.spr(i))->behind)
					Ewpns.spr(i)->draw(framebuf);
			}
		
			for(int32_t i=0; i<Lwpns.Count(); i++)
			{
				if(((weapon *)Lwpns.spr(i))->behind)
					Lwpns.spr(i)->draw(framebuf);
			}
			
			if(get_bit(quest_rules,qr_SHADOWS)&&(!get_bit(quest_rules,qr_SHADOWSFLICKER)||frame&1))
			{
				guys.drawshadow(framebuf,get_bit(quest_rules,qr_TRANSSHADOWS)!=0,true);
			}
			
			items.draw(framebuf,false);
			chainlinks.draw(framebuf,false);
			//Lwpns.draw(framebuf,false);
			guys.draw(framebuf,false);
			
			for(int32_t i=0; i<Ewpns.Count(); i++)
			{
				if(!((weapon *)Ewpns.spr(i))->behind)
				{
					Ewpns.spr(i)->draw(framebuf);
				}
			}
		
			for(int32_t i=0; i<Lwpns.Count(); i++)
			{
				if(!((weapon *)Lwpns.spr(i))->behind)
				{
					Lwpns.spr(i)->draw(framebuf);
				}
			}
		}
		
		guys.draw2(framebuf,true);
	}
	
	if(mirror_portal)
		mirror_portal->draw(framebuf);
	
	if(showhero && ((Hero.getAction()!=climbcovertop)&& (Hero.getAction()!=climbcoverbottom)))
	{
		mblock2.draw(framebuf);
		
		if(!Hero.isSwimming())
		{
			if((Hero.getZ()>0 || Hero.getFakeZ()>0) &&(!get_bit(quest_rules,qr_SHADOWSFLICKER)||frame&1))
			{
				Hero.drawshadow(framebuf,get_bit(quest_rules,qr_TRANSSHADOWS)!=0);
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
	}
	
	//4. Blit framebuf onto temp_buf
	
	//you have to do this, because do_layer calls overcombo, which doesn't respect the clipping rectangle, which messes up the triforce curtain. -DD
	blit(framebuf, temp_buf, 0, 0, 0, 0, 256, 224);
	
	//5. Draw some layers onto temp_buf and scrollbuf
	
	for_every_nearby_screen([&](mapscr* myscr, int scr, int offx, int offy) {
		if(!XOR(myscr->flags7&fLAYER3BG, DMaps[currdmap].flags&dmfLAYER3BG))
		{
			do_layer(temp_buf, 0, currmap, scr, 3, myscr, -offx, -offy, 2, false, true);
			if (scr == currscr) particles.draw(temp_buf, true, 2);
		}
		
		do_layer(temp_buf, 0, currmap, scr, 4, myscr, -offx, -offy, 2, false, true);
		//do_primitives(temp_buf, 3, myscr, 0,playing_field_offset);//don't uncomment me
		
		if (scr == currscr) particles.draw(temp_buf, true, 3);

		do_layer(temp_buf, -1, currmap, scr, 0, myscr, -offx, -offy, 2);
		if (get_bit(quest_rules,qr_OVERHEAD_COMBOS_L1_L2))
		{
			do_layer(temp_buf, -1, currmap, scr, 1, myscr, -offx, -offy, 2);
			do_layer(temp_buf, -1, currmap, scr, 2, myscr, -offx, -offy, 2);
		}
	});
	
	if (!global_z3_scrolling_extended_height_mode)
	{
		rectfill(temp_buf, 0, 0, 256, playing_field_offset, 0);
	}
	
	particles.draw(temp_buf, true, -1);
	
	//6. Blit temp_buf onto framebuf with clipping
	
	set_clip_rect(framebuf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2);
	blit(temp_buf, framebuf, 0, 0, 0, 0, 256, 224);
	
	//6b. Draw the subscreen, without clipping
	if(!global_z3_scrolling_extended_height_mode && !get_bit(quest_rules,qr_SUBSCREENOVERSPRITES))
	{
		set_clip_rect(framebuf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2);
		put_passive_subscr(framebuf, &QMisc, 0, passive_subscreen_offset, false, sspUP);
	}
	
	
	//7. Draw some flying sprites onto framebuf
	set_clip_rect(framebuf,0,0,256,224);
	
	//Jumping Hero and jumping enemies are drawn on this layer.
	if(Hero.getZ() > (zfix)zinit.jump_hero_layer_threshold)
	{
		decorations.draw2(framebuf,false);
		Hero.draw(framebuf);
		chainlinks.draw(framebuf,true);
		
		for(int32_t i=0; i<Lwpns.Count(); i++)
		{
			if(Lwpns.spr(i)->z+Lwpns.spr(i)->fakez > (zfix)zinit.jump_hero_layer_threshold)
			{
				Lwpns.spr(i)->draw(framebuf);
			}
		}
		
		decorations.draw(framebuf,false);
	}
	
	if(!get_bit(quest_rules,qr_ENEMIESZAXIS)) for(int32_t i=0; i<guys.Count(); i++)
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
	
	// Draw the Moving Fairy above layer 3
	for(int32_t i=0; i<items.Count(); i++)
		if(itemsbuf[items.spr(i)->id].family == itype_fairy && itemsbuf[items.spr(i)->id].misc3)
			items.spr(i)->draw(framebuf);
			
	//8. Blit framebuf onto temp_buf
	
	masked_blit(framebuf, temp_buf, 0, 0, 0, 0, 256, 224);
	
	//9. Draw some layers onto temp_buf and scrollbuf
	
	set_clip_rect(framebuf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2);
	
	color_map = &trans_table2;
	if(get_bit(quest_rules,qr_LIGHTBEAM_TRANSPARENT))
		draw_trans_sprite(temp_buf, lightbeam_bmp, 0, playing_field_offset);
	else 
		masked_blit(lightbeam_bmp, temp_buf, 0, 0, 0, playing_field_offset, 256, 176);
	color_map = &trans_table;

	for_every_nearby_screen([&](mapscr* myscr, int scr, int offx, int offy) {
		do_layer(temp_buf, 0, currmap, scr, 5, myscr, -offx, -offy, 2, false, true);
		if (scr == currscr) particles.draw(temp_buf, true, 4);
		// overhead freeform combos!
		do_layer(temp_buf, -4, currmap, scr, 0, myscr, -offx, -offy, 2);
		// ---
		do_layer(temp_buf, 0, currmap, scr, 6, myscr, -offx, -offy, 2, false, true);
		if (scr == currscr) particles.draw(temp_buf, true, 5);
	});
	
	//10. Blit temp_buf onto framebuf with clipping
	
	set_clip_rect(framebuf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2);
	blit(temp_buf, framebuf, 0, 0, 0, 0, 256, 224);
	
	//11. Handle low drawn darkness
	if(get_bit(quest_rules, qr_NEW_DARKROOM) && (this_screen->flags&fDARK))
	{
		for_every_nearby_screen([&](mapscr* myscr, int scr, int offx, int offy) {
			calc_darkroom_combos(scr, offx, offy);
		});
		Hero.calc_darkroom_hero();
	}
	
	//Darkroom if under the subscreen
	if(get_bit(quest_rules, qr_NEW_DARKROOM) && get_bit(quest_rules, qr_NEWDARK_L6) && (this_screen->flags&fDARK))
	{
		set_clip_rect(framebuf, 0, playing_field_offset, 256, 168+playing_field_offset);
		if(this_screen->flags9 & fDARK_DITHER) //dither the entire bitmap
		{
			ditherblit(darkscr_bmp_curscr,darkscr_bmp_curscr,0,game->get_dither_type(),game->get_dither_arg());
			ditherblit(darkscr_bmp_curscr_trans,darkscr_bmp_curscr_trans,0,game->get_dither_type(),game->get_dither_arg());
		}
		
		color_map = &trans_table2;
		if(this_screen->flags9 & fDARK_TRANS) //draw the dark as transparent
			draw_trans_sprite(framebuf, darkscr_bmp_curscr, 0, playing_field_offset);
		else 
			masked_blit(darkscr_bmp_curscr, framebuf, 0, 0, 0, playing_field_offset, 256, 168);
		draw_trans_sprite(framebuf, darkscr_bmp_curscr_trans, 0, playing_field_offset);
		color_map = &trans_table;
		
		set_clip_rect(framebuf, 0, 0, framebuf->w, framebuf->h);
	}	
	
	
	//12. Draw some text on framebuf
	
	set_clip_rect(framebuf,0,0,256,224);
	
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
	
	//13. Draw the subscreen, without clipping
	// TODO z3
	if(!global_z3_scrolling_extended_height_mode && get_bit(quest_rules,qr_SUBSCREENOVERSPRITES))
	{
		put_passive_subscr(framebuf, &QMisc, 0, passive_subscreen_offset, false, sspUP);
		
		// Draw primitives over subscren
		do_primitives(framebuf, 7, this_screen, 0, playing_field_offset); //Layer '7' appears above subscreen if quest rule is set
	}
	
	//14. Handle high-drawn darkness
	if(get_bit(quest_rules, qr_NEW_DARKROOM) && !get_bit(quest_rules, qr_NEWDARK_L6) && (this_screen->flags&fDARK))
	{
		set_clip_rect(framebuf, 0, playing_field_offset, 256, 168+playing_field_offset);
		if(this_screen->flags9 & fDARK_DITHER) //dither the entire bitmap
		{
			ditherblit(darkscr_bmp_curscr,darkscr_bmp_curscr,0,game->get_dither_type(),game->get_dither_arg());
			ditherblit(darkscr_bmp_curscr_trans,darkscr_bmp_curscr_trans,0,game->get_dither_type(),game->get_dither_arg());
		}
		
		color_map = &trans_table2;
		if(this_screen->flags9 & fDARK_TRANS) //draw the dark as transparent
			draw_trans_sprite(framebuf, darkscr_bmp_curscr, 0, playing_field_offset);
		else 
			masked_blit(darkscr_bmp_curscr, framebuf, 0, 0, 0, playing_field_offset, 256, 168);
		draw_trans_sprite(framebuf, darkscr_bmp_curscr_trans, 0, playing_field_offset);
		color_map = &trans_table;
		
		set_clip_rect(framebuf, 0, 0, framebuf->w, framebuf->h);
	}
	
	set_clip_rect(scrollbuf, 0, 0, scrollbuf->w, scrollbuf->h);
	if(runGeneric) FFCore.runGenericPassiveEngine(SCR_TIMING_POST_DRAW);
}

void put_door(BITMAP *dest,int32_t t,int32_t pos,int32_t side,int32_t type,bool redraw,bool even_walls)
{
	if (type > 8) return;

	mapscr& screen = t == 0 ? tmpscr : special_warp_return_screen;
	int32_t d=screen.door_combo_set;
	
	switch(type)
	{
	case dt_wall:
	case dt_walk:
		if(!even_walls)
			break;
		[[fallthrough]];
	case dt_pass:
		if(!get_bit(quest_rules, qr_REPLACEOPENDOORS) && !even_walls)
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
			screen.data[pos]   = DoorComboSets[d].doorcombo_u[type][0];
			screen.cset[pos]   = DoorComboSets[d].doorcset_u[type][0];
			screen.sflag[pos]  = 0;
			screen.data[pos+1]   = DoorComboSets[d].doorcombo_u[type][1];
			screen.cset[pos+1]   = DoorComboSets[d].doorcset_u[type][1];
			screen.sflag[pos+1]  = 0;
			screen.data[pos+16]   = DoorComboSets[d].doorcombo_u[type][2];
			screen.cset[pos+16]   = DoorComboSets[d].doorcset_u[type][2];
			screen.sflag[pos+16]  = 0;
			screen.data[pos+16+1]   = DoorComboSets[d].doorcombo_u[type][3];
			screen.cset[pos+16+1]   = DoorComboSets[d].doorcset_u[type][3];
			screen.sflag[pos+16+1]  = 0;
			
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
			screen.data[pos]   = DoorComboSets[d].doorcombo_d[type][0];
			screen.cset[pos]   = DoorComboSets[d].doorcset_d[type][0];
			screen.sflag[pos]  = 0;
			screen.data[pos+1]   = DoorComboSets[d].doorcombo_d[type][1];
			screen.cset[pos+1]   = DoorComboSets[d].doorcset_d[type][1];
			screen.sflag[pos+1]  = 0;
			screen.data[pos+16]   = DoorComboSets[d].doorcombo_d[type][2];
			screen.cset[pos+16]   = DoorComboSets[d].doorcset_d[type][2];
			screen.sflag[pos+16]  = 0;
			screen.data[pos+16+1]   = DoorComboSets[d].doorcombo_d[type][3];
			screen.cset[pos+16+1]   = DoorComboSets[d].doorcset_d[type][3];
			screen.sflag[pos+16+1]  = 0;
			
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
			screen.data[pos]   = DoorComboSets[d].doorcombo_l[type][0];
			screen.cset[pos]   = DoorComboSets[d].doorcset_l[type][0];
			screen.sflag[pos]  = 0;
			screen.data[pos+1]   = DoorComboSets[d].doorcombo_l[type][1];
			screen.cset[pos+1]   = DoorComboSets[d].doorcset_l[type][1];
			screen.sflag[pos+1]  = 0;
			screen.data[pos+16]   = DoorComboSets[d].doorcombo_l[type][2];
			screen.cset[pos+16]   = DoorComboSets[d].doorcset_l[type][2];
			screen.sflag[pos+16]  = 0;
			screen.data[pos+16+1]   = DoorComboSets[d].doorcombo_l[type][3];
			screen.cset[pos+16+1]   = DoorComboSets[d].doorcset_l[type][3];
			screen.sflag[pos+16+1]  = 0;
			screen.data[pos+32]   = DoorComboSets[d].doorcombo_l[type][4];
			screen.cset[pos+32]   = DoorComboSets[d].doorcset_l[type][4];
			screen.sflag[pos+32]  = 0;
			screen.data[pos+32+1]   = DoorComboSets[d].doorcombo_l[type][5];
			screen.cset[pos+32+1]   = DoorComboSets[d].doorcset_l[type][5];
			screen.sflag[pos+32+1]  = 0;
			
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
			screen.data[pos]   = DoorComboSets[d].doorcombo_r[type][0];
			screen.cset[pos]   = DoorComboSets[d].doorcset_r[type][0];
			screen.sflag[pos]  = 0;
			screen.data[pos+1]   = DoorComboSets[d].doorcombo_r[type][1];
			screen.cset[pos+1]   = DoorComboSets[d].doorcset_r[type][1];
			screen.sflag[pos+1]  = 0;
			screen.data[pos+16]   = DoorComboSets[d].doorcombo_r[type][2];
			screen.cset[pos+16]   = DoorComboSets[d].doorcset_r[type][2];
			screen.sflag[pos+16]  = 0;
			screen.data[pos+16+1]   = DoorComboSets[d].doorcombo_r[type][3];
			screen.cset[pos+16+1]   = DoorComboSets[d].doorcset_r[type][3];
			screen.sflag[pos+16+1]  = 0;
			screen.data[pos+32]   = DoorComboSets[d].doorcombo_r[type][4];
			screen.cset[pos+32]   = DoorComboSets[d].doorcset_r[type][4];
			screen.sflag[pos+32]  = 0;
			screen.data[pos+32+1]   = DoorComboSets[d].doorcombo_r[type][5];
			screen.cset[pos+32+1]   = DoorComboSets[d].doorcset_r[type][5];
			screen.sflag[pos+32+1]  = 0;
			
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

void over_door(BITMAP *dest,int32_t t, int32_t pos,int32_t side, int32_t xoff, int32_t yoff)
{
	mapscr& screen = t == 0 ? tmpscr : special_warp_return_screen;
	int32_t d=screen.door_combo_set;
	int32_t x=(pos&15)<<4;
	int32_t y=(pos&0xF0);
	
	switch(side)
	{
	case up:
		overcombo2(dest,x+xoff,y+yoff,
				   DoorComboSets[d].bombdoorcombo_u[0],
				   DoorComboSets[d].bombdoorcset_u[0]);
		overcombo2(dest,x+16+xoff,y+yoff,
				   DoorComboSets[d].bombdoorcombo_u[1],
				   DoorComboSets[d].bombdoorcset_u[1]);
		break;
		
	case down:
		overcombo2(dest,x+xoff,y+yoff,
				   DoorComboSets[d].bombdoorcombo_d[0],
				   DoorComboSets[d].bombdoorcset_d[0]);
		overcombo2(dest,x+16+xoff,y+yoff,
				   DoorComboSets[d].bombdoorcombo_d[1],
				   DoorComboSets[d].bombdoorcset_d[1]);
		break;
		
	case left:
		overcombo2(dest,x+xoff,y+yoff,
				   DoorComboSets[d].bombdoorcombo_l[0],
				   DoorComboSets[d].bombdoorcset_l[0]);
		overcombo2(dest,x+xoff,y+yoff+16,
				   DoorComboSets[d].bombdoorcombo_l[1],
				   DoorComboSets[d].bombdoorcset_l[1]);
		overcombo2(dest,x+xoff,y+yoff+16,
				   DoorComboSets[d].bombdoorcombo_l[2],
				   DoorComboSets[d].bombdoorcset_l[2]);
		break;
		
	case right:
		overcombo2(dest,x+xoff,y+yoff,
				   DoorComboSets[d].bombdoorcombo_r[0],
				   DoorComboSets[d].bombdoorcset_r[0]);
		overcombo2(dest,x+xoff,y+yoff+16,
				   DoorComboSets[d].bombdoorcombo_r[1],
				   DoorComboSets[d].bombdoorcset_r[1]);
		overcombo2(dest,x+xoff,y+yoff+16,
				   DoorComboSets[d].bombdoorcombo_r[2],
				   DoorComboSets[d].bombdoorcset_r[2]);
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
	
	if(!even_walls&&(door==dWALL||door==dWALK))
	{
		return;
	}
	
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
    int32_t d=tmpscr.door_combo_set;
    
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

void openshutters()
{
	for(int32_t i=0; i<4; i++)
		if(tmpscr.door[i]==dSHUTTER)
		{
			putdoor(scrollbuf,0,i,dOPENSHUTTER);
			tmpscr.door[i]=dOPENSHUTTER;
		}
	
	for_every_screen_in_region([&](mapscr* z3_scr, int screen_index, unsigned int z3_scr_dx, unsigned int z3_scr_dy) {
		pos_handle pos_handle;
		for (auto lyr = 0; lyr < 7; ++lyr)
		{
			mapscr* scr = get_layer_scr(currmap, screen_index, lyr - 1);
			pos_handle.screen = scr;
			pos_handle.screen_index = screen_index;
			pos_handle.layer = lyr; // TODO z3 work out if this is -1 or 0 indexed
			for (auto pos = 0; pos < 176; ++pos)
			{
				newcombo const& cmb = combobuf[scr->data[pos]];
				if(cmb.triggerflags[0] & combotriggerSHUTTER)
				{
					pos_handle.rpos = POS_TO_RPOS(pos, z3_scr_dx, z3_scr_dy);
					do_trigger_combo(pos_handle);
				}
			}
		}
	});
	
	sfx(WAV_DOOR, 128);
}

void load_a_screen_and_layers(int dmap, int map, int screen)
{
	std::vector<mapscr*> screens;

	const mapscr* source = get_canonical_scr(map, screen);
	mapscr* base_screen = new mapscr(*source);
	if (map == currmap) temporary_screens_currmap[screen*7] = base_screen;
	screens.push_back(base_screen);

	base_screen->valid |= mVALID; // layer 0 is always valid

	if (source->script > 0)
	{
		base_screen->screendatascriptInitialised = 0;
		base_screen->doscript = 1;
	}
	else
	{
		base_screen->script = 0;
		base_screen->screendatascriptInitialised = 0;
		base_screen->doscript = 0;
	}

	for (int i = 0; i < 6; i++)
	{
		if(source->layermap[i]>0 && (ZCMaps[source->layermap[i]-1].tileWidth==ZCMaps[currmap].tileWidth)
					&& (ZCMaps[source->layermap[i]-1].tileHeight==ZCMaps[currmap].tileHeight))
		{
			screens.push_back(new mapscr(*get_canonical_scr(source->layermap[i]-1, source->layerscreen[i])));
		}
		else
		{
			screens.push_back(new mapscr());
		}
		if (map == currmap) temporary_screens_currmap[screen*7+i+1] = screens[i+1];
	}

	// Apply perm secrets, if applicable.
	// TODO z3
	if (canPermSecret(dmap, screen))
	{
		if(game->maps[map*MAPSCRSNORMAL + screen] & mSECRET)    // if special stuff done before
		{
			// hiddenstair(screen, false);
			bool do_layers = true;
			hidden_entrance2(screen, NULL, do_layers, false, -3);
		}
		if(game->maps[map*MAPSCRSNORMAL + screen] & mLIGHTBEAM) // if special stuff done before
		{
			for(size_t layer = 0; layer < 7; ++layer)
			{
				// TODO z3 ?
				// mapscr* layer_scr = (tmp==0) ? FFCore.tempScreens[layer] : FFCore.ScrollingScreens[layer];
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

	if (map != currmap) temporary_screens[map*MAPSCRS + screen] = screens;
}

// Sets `currscr` to `scr` and loads new screens into temporary memory.
// Called anytime a player moves to a new screen (either via warping, scrolling, continue,
// starting the game, etc...)
// Note: for regions, on the initial screen load calls this function. Simply walking between screens
// in the same region does not use this.
// If scr >= 0x80, `currscr` will be saved to `homescr` and also be loaded into `special_warp_return_screen`.
void loadscr(int32_t destdmap, int32_t scr, int32_t ldir, bool overlay, bool no_x80_dir)
{
	int previous_currscr = currscr;
	currscr = scr;
	z3_load_region();

	if (scr >= 0x80)
	{
		homescr = previous_currscr;
		loadscr_old(1, destdmap, homescr, no_x80_dir ? -1 : ldir, overlay);
	}
	else
	{
		homescr = scr;
	}

	loadscr_old(0, destdmap, scr, ldir, overlay);
	if (!is_z3_scrolling_mode()) return;

	for (int screen_index = 0; screen_index < 128; screen_index++)
	{
		if (screen_index != scr && is_in_region(z3_origin_scr, screen_index))
		{
			load_a_screen_and_layers(destdmap, currmap, screen_index);
		}
	}

	int32_t destlvl = DMaps[destdmap < 0 ? currdmap : destdmap].level;
	// TODO z3 special warp return scr?
	// toggle_switches(game->lvlswitches[destlvl], true, tmp == 0 ? &tmpscr : &special_warp_return_screen, tmp == 0 ? currscr : homescr);
	toggle_switches(game->lvlswitches[destlvl], true);
}

// Don't use this directly!
void loadscr_old(int32_t tmp,int32_t destdmap, int32_t scr,int32_t ldir,bool overlay=false)
{
	bool is_setting_special_warp_return_screen = tmp == 1;

	if(!is_setting_special_warp_return_screen)
	{
		triggered_screen_secrets = false; //Reset var
		init_combo_timers();
		timeExitAllGenscript(GENSCR_ST_CHANGE_SCREEN);
	}
	clear_to_color(darkscr_bmp_curscr, game->get_darkscr_color());
	clear_to_color(darkscr_bmp_curscr_trans, game->get_darkscr_color());
	clear_to_color(darkscr_bmp_scrollscr, game->get_darkscr_color());
	clear_to_color(darkscr_bmp_scrollscr_trans, game->get_darkscr_color());
	int32_t destlvl = DMaps[destdmap < 0 ? currdmap : destdmap].level;
	
	//  introclk=intropos=msgclk=msgpos=dmapmsgclk=0;
	for(word x=0; x<animated_combos; x++)
	{
		if(combobuf[animated_combo_table4[x][0]].nextcombo!=0)
		{
			combobuf[animated_combo_table4[x][0]].aclk = 0;
		}
	}
	
	for(word x=0; x<animated_combos2; x++)
	{
		if(combobuf[animated_combo_table24[x][0]].nextcombo!=0)
		{
			combobuf[animated_combo_table24[x][0]].aclk = 0;
		}
	}
	
	reset_combo_animations2();
	
	
	mapscr previous_scr = tmp == 0 ? tmpscr : special_warp_return_screen;
	mapscr* screen = tmp == 0 ? &tmpscr : &special_warp_return_screen;
	*screen = TheMaps[currmap*MAPSCRS+scr];
	
	const int32_t _mapsSize = ZCMaps[currmap].tileHeight*ZCMaps[currmap].tileWidth;
	screen->valid |= mVALID; //layer 0 is always valid
	screen->data = TheMaps[currmap*MAPSCRS+scr].data;
	screen->sflag = TheMaps[currmap*MAPSCRS+scr].sflag;
	screen->cset = TheMaps[currmap*MAPSCRS+scr].cset;
	
	//screen / screendata script
	FFCore.clear_screen_stack();
	screenScriptData.Clear();
	FFCore.deallocateAllArrays(SCRIPT_SCREEN, 0);
	FFCore.deallocateAllArrays(SCRIPT_COMBO, 0);
	//reset combo script doscripts
	//Init combo scripts
	FFCore.init_combo_doscript();
	if ( TheMaps[currmap*MAPSCRS+scr].script > 0 )
	{
		screen->script = TheMaps[currmap*MAPSCRS+scr].script;
		al_trace("The screen script id is: %d \n", TheMaps[currmap*MAPSCRS+scr].script);
		//if ( !screen->screendatascriptInitialised )
		//{
			for ( int32_t q = 0; q < 8; q++ )
			{
			screen->screeninitd[q] = TheMaps[currmap*MAPSCRS+scr].screeninitd[q];
			}
		//}
		screen->screendatascriptInitialised = 0;
		screen->doscript = 1;
	}
	else
	{
		screen->script = 0;
		screen->screendatascriptInitialised = 0;
		screen->doscript = 0;
	}
	
	
	screen->data.resize(_mapsSize, 0);
	screen->sflag.resize(_mapsSize, 0);
	screen->cset.resize(_mapsSize, 0);
	
	if(overlay)
	{
		for(int32_t c=0; c< ZCMaps[currmap].tileHeight*ZCMaps[currmap].tileWidth; ++c)
		{
			if(screen->data[c]==0)
			{
				screen->data[c]=previous_scr.data[c];
				screen->sflag[c]=previous_scr.sflag[c];
				screen->cset[c]=previous_scr.cset[c];
			}
		}
		
		for(int32_t i=0; i<6; i++)
		{
			if(previous_scr.layermap[i]>0 && screen->layermap[i]>0)
			{
				int32_t lm = (screen->layermap[i]-1)*MAPSCRS+screen->layerscreen[i];
				int32_t fm = (previous_scr.layermap[i]-1)*MAPSCRS+previous_scr.layerscreen[i];
				
				if(!TheMaps[lm].data.empty() && !TheMaps[fm].data.empty())
				{
					for(int32_t c=0; c< ZCMaps[currmap].tileHeight*ZCMaps[currmap].tileWidth; ++c)
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
	}
	
	// Carry over FFCs from previous screen to the next. Only for the ones that have the carry over flag set, and if the previous
	// screen has FF carryover enabled.
	if (!is_setting_special_warp_return_screen)
	{
		// Before loading new FFCs, deallocate the arrays used by those that aren't carrying over
		
		for(int32_t ffid = 0; ffid < 32; ++ffid)
		{
			if(!(previous_scr.flags5&fNOFFCARRYOVER) && (previous_scr.ffflags[ffid]&ffCARRYOVER)) continue;
			FFCore.deallocateAllArrays(SCRIPT_FFC, ffid, false); //false means this does not require 'qr_ALWAYS_DEALLOCATE_ARRAYS' to be checked. -V
		}
		FFCore.deallocateAllArrays(SCRIPT_SCREEN, 0);
		
		for(int32_t i = 0; i < 32; i++)
		{
			// If these aren't reset, changers may not work right
			ffposx[i]=-1000;
			ffposy[i]=-1000;
			ffprvx[i]=-10000000;
			ffprvy[i]=-10000000;
			
			if((previous_scr.ffflags[i]&ffCARRYOVER) && !(previous_scr.flags5&fNOFFCARRYOVER))
			{
				screen->ffdata[i] = previous_scr.ffdata[i];
				screen->ffx[i] = previous_scr.ffx[i];
				screen->ffy[i] = previous_scr.ffy[i];
				screen->ffxdelta[i] = previous_scr.ffxdelta[i];
				screen->ffydelta[i] = previous_scr.ffydelta[i];
				screen->ffxdelta2[i] = previous_scr.ffxdelta2[i];
				screen->ffydelta2[i] = previous_scr.ffydelta2[i];
				screen->fflink[i] = previous_scr.fflink[i];
				screen->ffdelay[i] = previous_scr.ffdelay[i];
				screen->ffcset[i] = previous_scr.ffcset[i];
				screen->ffwidth[i] = previous_scr.ffwidth[i];
				screen->ffheight[i] = previous_scr.ffheight[i];
				screen->ffflags[i] = previous_scr.ffflags[i];
				screen->ffscript[i] = previous_scr.ffscript[i];
				
				for(int32_t j=0; j<2; ++j)
				{
					screen->inita[i][j] = previous_scr.inita[i][j];
				}
				
				for(int32_t j=0; j<8; ++j)
				{
					screen->initd[i][j] = previous_scr.initd[i][j];
				}
				
				if(!(previous_scr.ffflags[i]&ffSCRIPTRESET))
				{
					screen->ffscript[i] = previous_scr.ffscript[i]; // Restart script if it has halted.
					screen->initialized[i] = previous_scr.initialized[i];
				}
				else
				{
					screen->initialized[i] = false;
					
					ffcScriptData[i].pc = 0;
					ffcScriptData[i].sp = 0;
					ffcScriptData[i].ffcref = 0;
				}
			}
			else
			{
				memset(ffmisc[i], 0, 16 * sizeof(int32_t));
				ffcScriptData[i].Clear();
				clear_ffc_stack(i);
			}
		}

		for(int32_t i=0; i<6; i++)
		{
			mapscr layerscr = tmpscr2[i];
			
			// Don't delete the old tmpscr2's data yet!
			if(screen->layermap[i]>0 && (ZCMaps[screen->layermap[i]-1].tileWidth==ZCMaps[currmap].tileWidth)
					&& (ZCMaps[screen->layermap[i]-1].tileHeight==ZCMaps[currmap].tileHeight))
			{
				// const int32_t _mapsSize = (ZCMaps[currmap].tileWidth)*(ZCMaps[currmap].tileHeight);
				
				tmpscr2[i]=TheMaps[(screen->layermap[i]-1)*MAPSCRS+screen->layerscreen[i]];
				
				tmpscr2[i].data.resize(_mapsSize, 0);
				tmpscr2[i].sflag.resize(_mapsSize, 0);
				tmpscr2[i].cset.resize(_mapsSize, 0);
				
				if(overlay)
				{
					for(int32_t y=0; y<ZCMaps[currmap].tileHeight; ++y)
					{
						for(int32_t x=0; x<ZCMaps[currmap].tileWidth; ++x)
						{
							int32_t c=y*ZCMaps[currmap].tileWidth+x;
							
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
	
	// Apply perm secrets, if applicable.
	if(canPermSecret(destdmap,scr)/*||TheMaps[(currmap*MAPSCRS)+currscr].flags6&fTRIGGERFPERM*/)
	{
		if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mSECRET)			   // if special stuff done before
		{
			hiddenstair(tmp,false);
			hidden_entrance(tmp,false,false,-3);
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
	
	toggle_switches(game->lvlswitches[destlvl], true, tmp == 0 ? &tmpscr : &special_warp_return_screen, tmp == 0 ? currscr : homescr);
	
	if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mLOCKBLOCK)			  // if special stuff done before
	{
		remove_lockblocks(tmp);
	}
	
	if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mBOSSLOCKBLOCK)		  // if special stuff done before
	{
		remove_bosslockblocks(tmp);
	}
	
	if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mCHEST)			  // if special stuff done before
	{
		remove_chests(tmp);
	}
	
	if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mLOCKEDCHEST)			  // if special stuff done before
	{
		remove_lockedchests(tmp);
	}
	
	if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mBOSSCHEST)			  // if special stuff done before
	{
		remove_bosschests(tmp);
	}
	
	clear_xstatecombos_old(tmp, (currmap*MAPSCRSNORMAL)+scr);
	
	// check doors
	if(isdungeon(destdmap,scr))
	{
		// CHECK(!is_z3_scrolling_mode());
		for(int32_t i=0; i<4; i++)
		{
			int32_t door=screen->door[i];
			bool putit=true;
			
			switch(door)
			{
			case d1WAYSHUTTER:
			case dSHUTTER:
				if((ldir^1)==i)
				{
					screen->door[i]=dOPENSHUTTER;
					//		  putit=false;
				}
				
				opendoors = -4;
				break;
				
			case dLOCKED:
				if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&(1<<i))
				{
					screen->door[i]=dUNLOCKED;
					//		  putit=false;
				}
				
				break;
				
			case dBOSS:
				if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&(1<<i))
				{
					screen->door[i]=dOPENBOSS;
					//		  putit=false;
				}
				
				break;
				
			case dBOMB:
				if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&(1<<i))
				{
					screen->door[i]=dBOMBED;
				}
				
				break;
			}
			
			if(putit)
			{
				putdoor(scrollbuf,tmp,i,screen->door[i],false);
			}
			
			if(door==dSHUTTER||door==d1WAYSHUTTER)
			{
				screen->door[i]=door;
			}
		}
	}
	
	
	for(int32_t j=-1; j<6; ++j)  // j == -1 denotes the current screen
	{
		if(j<0 || ((screen->layermap[j]>0)&&(ZCMaps[screen->layermap[j]-1].tileWidth==ZCMaps[currmap].tileWidth) && (ZCMaps[screen->layermap[j]-1].tileHeight==ZCMaps[currmap].tileHeight)))
		{
			mapscr *layerscreen= (j<0 ? screen : !tmpscr2[j].data.empty() ? &tmpscr2[j] :
								  &TheMaps[(screen->layermap[j]-1)*MAPSCRS]+screen->layerscreen[j]);
								  
			for(int32_t i=0; i<(ZCMaps[currmap].tileWidth)*(ZCMaps[currmap].tileHeight); ++i)
			{
				int32_t c=layerscreen->data[i];
				int32_t cs=layerscreen->cset[i];
				
				// New screen flag: Cycle Combos At Screen Init
				if(combobuf[c].nextcombo != 0 && (screen->flags3 & fCYCLEONINIT) && (j<0 || get_bit(quest_rules,qr_CMBCYCLELAYERS)))
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
	
	game->load_portal();
}

// Screen is being viewed by the Overworld Map viewer.
void loadscr2(int32_t tmp,int32_t scr,int32_t)
{
	for(word x=0; x<animated_combos; x++)
	{
		if(combobuf[animated_combo_table4[x][0]].nextcombo!=0)
		{
			combobuf[animated_combo_table4[x][0]].aclk=0;
		}
	}
	
	const int32_t _mapsSize = (ZCMaps[currmap].tileWidth)*(ZCMaps[currmap].tileHeight);
	
	mapscr& screen = tmp == 0 ? tmpscr : special_warp_return_screen;
	screen = TheMaps[currmap*MAPSCRS+scr];
	
	screen.data.resize(_mapsSize, 0);
	screen.sflag.resize(_mapsSize, 0);
	screen.cset.resize(_mapsSize, 0);
	
	if(tmp==0)
	{
		for(int32_t i=0; i<6; i++)
		{
			if(screen.layermap[i]>0)
			{
			
				if((ZCMaps[screen.layermap[i]-1].tileWidth==ZCMaps[currmap].tileWidth) && (ZCMaps[screen.layermap[i]-1].tileHeight==ZCMaps[currmap].tileHeight))
				{
					tmpscr2[i]=TheMaps[(screen.layermap[i]-1)*MAPSCRS+screen.layerscreen[i]];
					
					tmpscr2[i].data.resize(_mapsSize, 0);
					tmpscr2[i].sflag.resize(_mapsSize, 0);
					tmpscr2[i].cset.resize(_mapsSize, 0);
				}
				else
				{
					(tmpscr2+i)->zero_memory();
				}
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
			hiddenstair(tmp,false);
			hidden_entrance(tmp,false,false,-3);
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
		remove_lockblocks(tmp);
	}
	
	if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mBOSSLOCKBLOCK)		  // if special stuff done before
	{
		remove_bosslockblocks(tmp);
	}
	
	if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mCHEST)			  // if special stuff done before
	{
		remove_chests(tmp);
	}
	
	if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mLOCKEDCHEST)			  // if special stuff done before
	{
		remove_lockedchests(tmp);
	}
	
	if(game->maps[(currmap*MAPSCRSNORMAL)+scr]&mBOSSCHEST)			  // if special stuff done before
	{
		remove_bosschests(tmp);
	}
	
	clear_xstatecombos_old(tmp, (currmap*MAPSCRSNORMAL)+scr);
	
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
		if(j<0 || ((screen.layermap[j]>0)&&(ZCMaps[screen.layermap[j]-1].tileWidth==ZCMaps[currmap].tileWidth) && (ZCMaps[screen.layermap[j]-1].tileHeight==ZCMaps[currmap].tileHeight)))
		{
			mapscr *layerscreen= (j<0 ? &screen
								  : &(TheMaps[(screen.layermap[j]-1)*MAPSCRS+screen.layerscreen[j]]));
								  
			for(int32_t i=0; i<(ZCMaps[currmap].tileWidth)*(ZCMaps[currmap].tileHeight); ++i)
			{
				int32_t c=layerscreen->data[i];
				int32_t cs=layerscreen->cset[i];
				
				// New screen flag: Cycle Combos At Screen Init
				if((screen.flags3 & fCYCLEONINIT) && (j<0 || get_bit(quest_rules,qr_CMBCYCLELAYERS)))
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

void putscr(BITMAP* dest,int32_t x,int32_t y, mapscr* screen)
{
	int scr = z3_get_scr_for_xy_offset(x, y);
	x -= global_viewport_x;
	y -= global_viewport_y;

	if(screen->valid==0||!show_layer_0||screen->hidelayers & 1)
	{
		rectfill(dest,x,y,x+255,y+175,0);
		return;
	}
	
	bool over = XOR(screen->flags7&fLAYER2BG,DMaps[currdmap].flags&dmfLAYER2BG)
		|| XOR(screen->flags7&fLAYER3BG, DMaps[currdmap].flags&dmfLAYER3BG);

	// if (global_z3_scrolling_extended_height_mode)

	int start_x, end_x, start_y, end_y;
	get_bounds_for_draw_cmb_calls(dest, x, y, start_x, end_x, start_y, end_y);
	for (int cy = start_y; cy < end_y; cy++)
	{
		for (int cx = start_x; cx < end_x; cx++)
		{
			int i = cx + cy*16;
			auto rpos = POS_TO_RPOS(i, scr);
			draw_cmb_pos(dest, x, y, rpos, screen->data[i], screen->cset[i], 0, over, false);
		}
	}
}

void putscrdoors(BITMAP *dest,int32_t x,int32_t y, mapscr* scrn)
{
	if(scrn->valid==0||!show_layer_0)
	{
		return;
	}
	
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
bool _walkflag(int32_t x,int32_t y,int32_t cnt)
{
	return _walkflag(x,y,cnt,zfix(0));
}

// Returns true if the combo at viewport position x,y is solid. Looks at a combo's quadrant walkablity flags.
static bool _walkflag_new(int32_t x, int32_t y, zfix const& switchblockstate)
{
	mapscr* s0 = z3_get_mapscr_layer_for_xy_offset(x, y, 0);
	mapscr* s1 = z3_get_mapscr_layer_for_xy_offset(x, y, 1);
	mapscr* s2 = z3_get_mapscr_layer_for_xy_offset(x, y, 2);
	if (!s1->valid) s1 = s0;
	if (!s2->valid) s2 = s0;

	int32_t bx = COMBOPOS(x % 256, y % 176);
	newcombo c = combobuf[s0->data[bx]];
	newcombo c1 = combobuf[s1->data[bx]];
	newcombo c2 = combobuf[s2->data[bx]];
	bool dried = (((iswater_type(c.type)) || (iswater_type(c1.type)) ||
				   (iswater_type(c2.type))) && DRIEDLAKE);
	int32_t b=1;
	if(x&8) b<<=2;
	if(y&8) b<<=1;
	
	int32_t cwalkflag = c.walk;
	if(onSwitch(c,switchblockstate) && c.type == cCSWITCHBLOCK && c.usrflags&cflag9) cwalkflag &= (c.walk>>4)^0xF;
	else if ((c.type == cBRIDGE && get_bit(quest_rules, qr_OLD_BRIDGE_COMBOS)) || (iswater_type(c.type) && ((c.walk>>4)&b) && ((c.usrflags&cflag3) || (c.usrflags&cflag4)))) cwalkflag = 0;
	if (s1 != s0)
	{
		if(onSwitch(c1,switchblockstate) && c1.type == cCSWITCHBLOCK && c1.usrflags&cflag9) cwalkflag &= (c1.walk>>4)^0xF;
		else if ((iswater_type(c1.type) && ((c1.walk>>4)&b) && get_bit(quest_rules,  qr_WATER_ON_LAYER_1) && !((c1.usrflags&cflag3) || (c1.usrflags&cflag4)))) cwalkflag &= c1.walk;
		else if (c1.type == cBRIDGE)
		{
			if (!get_bit(quest_rules, qr_OLD_BRIDGE_COMBOS))
			{
				int efflag = (c1.walk & 0xF0)>>4;
				int newsolid = (c1.walk & 0xF);
				cwalkflag = ((newsolid | cwalkflag) & (~efflag)) | (newsolid & efflag);
			}
			else cwalkflag &= c1.walk;
		}
		else if ((iswater_type(c1.type) && get_bit(quest_rules,  qr_WATER_ON_LAYER_1) && ((c1.usrflags&cflag3) || (c1.usrflags&cflag4)) && ((c1.walk>>4)&b))) cwalkflag = 0;
		else cwalkflag |= c1.walk;
	}
	if (s2 != s0)
	{
		if(onSwitch(c2,switchblockstate) && c2.type == cCSWITCHBLOCK && c2.usrflags&cflag9) cwalkflag &= (c2.walk>>4)^0xF;
		else if ((iswater_type(c2.type) && ((c2.walk>>4)&b) && get_bit(quest_rules,  qr_WATER_ON_LAYER_2) && !((c2.usrflags&cflag3) || (c2.usrflags&cflag4)))) cwalkflag &= c2.walk;
		else if (c2.type == cBRIDGE)
		{
			if (!get_bit(quest_rules, qr_OLD_BRIDGE_COMBOS))
			{
				int efflag = (c2.walk & 0xF0)>>4;
				int newsolid = (c2.walk & 0xF);
				cwalkflag = ((newsolid | cwalkflag) & (~efflag)) | (newsolid & efflag);
			}
			else cwalkflag &= c2.walk;
		}
		else if ((iswater_type(c2.type) && get_bit(quest_rules,  qr_WATER_ON_LAYER_2) && ((c2.usrflags&cflag3) || (c2.usrflags&cflag4))) && ((c2.walk>>4)&b)) cwalkflag = 0;
		else cwalkflag |= c2.walk;
	}
	
	return (cwalkflag&b) ? !dried : false;
}

bool _walkflag(int32_t x,int32_t y,int32_t cnt,zfix const& switchblockstate)
{
	if (is_z3_scrolling_mode())
	{
		int max_x = world_w;
		int max_y = world_h;
		if (!get_bit(quest_rules, qr_LTTPWALK))
		{
			max_x -= 7;
			max_y -= 7;
		}
		if (x < 0 || y < 0) return false;
		if (x >= max_x) return false;
		if (x >= max_x - 8 && cnt == 2) return false;
		if (y >= max_y) return false;
		return _walkflag_new(x, y, switchblockstate) || (cnt == 2 && _walkflag_new(x + 8, y, switchblockstate));
	}
	else if(get_bit(quest_rules,qr_LTTPWALK))
	{
		if(x<0||y<0) return false;
		
		if(x>255) return false;
		
		if(x>247&&cnt==2) return false;
		
		if(y>175) return false;
	}
	else
	{
		if(x<0||y<0) return false;
		
		if(x>248) return false;
		
		if(x>240&&cnt==2) return false;
		
		if(y>168) return false;
	}
	
	mapscr *s0, *s1, *s2;
	s0=&tmpscr;
	s1=(tmpscr2->valid)?tmpscr2:&tmpscr;
	s2=(tmpscr2[1].valid)?tmpscr2+1:&tmpscr;
	//  s2=TheMaps+((*tmpscr).layermap[1]-1)MAPSCRS+((*tmpscr).layerscreen[1]);
	
	int32_t bx=(x>>4)+(y&0xF0);
	newcombo c = combobuf[s0->data[bx]];
	newcombo c1 = combobuf[s1->data[bx]];
	newcombo c2 = combobuf[s2->data[bx]];
	bool dried = (((iswater_type(c.type)) || (iswater_type(c1.type)) ||
				   (iswater_type(c2.type))) && DRIEDLAKE);
	int32_t b=1;
	
	if(x&8) b<<=2;
	
	if(y&8) b<<=1;
	
	int32_t cwalkflag = c.walk;
	if(onSwitch(c,switchblockstate) && c.type == cCSWITCHBLOCK && c.usrflags&cflag9) cwalkflag &= (c.walk>>4)^0xF;
	else if ((c.type == cBRIDGE && get_bit(quest_rules, qr_OLD_BRIDGE_COMBOS)) || (iswater_type(c.type) && ((c.walk>>4)&b) && ((c.usrflags&cflag3) || (c.usrflags&cflag4)))) cwalkflag = 0;
	if (s1 != s0)
	{
		if(onSwitch(c1,switchblockstate) && c1.type == cCSWITCHBLOCK && c1.usrflags&cflag9) cwalkflag &= (c1.walk>>4)^0xF;
		else if ((iswater_type(c1.type) && ((c1.walk>>4)&b) && get_bit(quest_rules,  qr_WATER_ON_LAYER_1) && !((c1.usrflags&cflag3) || (c1.usrflags&cflag4)))) cwalkflag &= c1.walk;
		else if (c1.type == cBRIDGE)
		{
			if (!get_bit(quest_rules, qr_OLD_BRIDGE_COMBOS))
			{
				int efflag = (c1.walk & 0xF0)>>4;
				int newsolid = (c1.walk & 0xF);
				cwalkflag = ((newsolid | cwalkflag) & (~efflag)) | (newsolid & efflag);
			}
			else cwalkflag &= c1.walk;
		}
		else if ((iswater_type(c1.type) && get_bit(quest_rules,  qr_WATER_ON_LAYER_1) && ((c1.usrflags&cflag3) || (c1.usrflags&cflag4)) && ((c1.walk>>4)&b))) cwalkflag = 0;
		else cwalkflag |= c1.walk;
	}
	if (s2 != s0)
	{
		if(onSwitch(c2,switchblockstate) && c2.type == cCSWITCHBLOCK && c2.usrflags&cflag9) cwalkflag &= (c2.walk>>4)^0xF;
		else if ((iswater_type(c2.type) && ((c2.walk>>4)&b) && get_bit(quest_rules,  qr_WATER_ON_LAYER_2) && !((c2.usrflags&cflag3) || (c2.usrflags&cflag4)))) cwalkflag &= c2.walk;
		else if (c2.type == cBRIDGE)
		{
			if (!get_bit(quest_rules, qr_OLD_BRIDGE_COMBOS))
			{
				int efflag = (c2.walk & 0xF0)>>4;
				int newsolid = (c2.walk & 0xF);
				cwalkflag = ((newsolid | cwalkflag) & (~efflag)) | (newsolid & efflag);
			}
			else cwalkflag &= c2.walk;
		}
		else if ((iswater_type(c2.type) && get_bit(quest_rules,  qr_WATER_ON_LAYER_2) && ((c2.usrflags&cflag3) || (c2.usrflags&cflag4))) && ((c2.walk>>4)&b)) cwalkflag = 0;
		else cwalkflag |= c2.walk;
	}
	
	if((cwalkflag&b) && !dried)
		return true;
		
	if(cnt==1) return false;
	
	++bx;
	
	if(!(x&8))
		b<<=2;
	else
	{
		c  = combobuf[s0->data[bx]];
		c1 = combobuf[s1->data[bx]];
		c2 = combobuf[s2->data[bx]];
		dried = (((iswater_type(c.type)) || (iswater_type(c1.type)) ||
				  (iswater_type(c2.type))) && DRIEDLAKE);
		b=1;
		
		if(y&8) b<<=1;
	}
	cwalkflag = c.walk;
	if(onSwitch(c,switchblockstate) && c.type == cCSWITCHBLOCK && c.usrflags&cflag9) cwalkflag &= (c.walk>>4)^0xF;
	else if ((c.type == cBRIDGE && get_bit(quest_rules, qr_OLD_BRIDGE_COMBOS)) || (iswater_type(c.type) && ((c.walk>>4)&b) && ((c.usrflags&cflag3) || (c.usrflags&cflag4)))) cwalkflag = 0;
	if (s1 != s0)
	{
		if(onSwitch(c1,switchblockstate) && c1.type == cCSWITCHBLOCK && c1.usrflags&cflag9) cwalkflag &= (c1.walk>>4)^0xF;
		else if ((iswater_type(c1.type) && ((c1.walk>>4)&b) && get_bit(quest_rules,  qr_WATER_ON_LAYER_1) && !((c1.usrflags&cflag3) || (c1.usrflags&cflag4)))) cwalkflag &= c1.walk;
		else if (c1.type == cBRIDGE)
		{
			if (!get_bit(quest_rules, qr_OLD_BRIDGE_COMBOS))
			{
				int efflag = (c1.walk & 0xF0)>>4;
				int newsolid = (c1.walk & 0xF);
				cwalkflag = ((newsolid | cwalkflag) & (~efflag)) | (newsolid & efflag);
			}
			else cwalkflag &= c1.walk;
		}
		else if ((iswater_type(c1.type) && get_bit(quest_rules,  qr_WATER_ON_LAYER_1) && ((c1.usrflags&cflag3) || (c1.usrflags&cflag4))) && ((c1.walk>>4)&b)) cwalkflag = 0;
		else cwalkflag |= c1.walk;
	}
	if (s2 != s0)
	{
		if(onSwitch(c2,switchblockstate) && c2.type == cCSWITCHBLOCK && c2.usrflags&cflag9) cwalkflag &= (c2.walk>>4)^0xF;
		else if ((iswater_type(c2.type) && ((c2.walk>>4)&b) && get_bit(quest_rules,  qr_WATER_ON_LAYER_2) && !((c2.usrflags&cflag3) || (c2.usrflags&cflag4)))) cwalkflag &= c2.walk;
		else if (c2.type == cBRIDGE)
		{
			if (!get_bit(quest_rules, qr_OLD_BRIDGE_COMBOS))
			{
				int efflag = (c2.walk & 0xF0)>>4;
				int newsolid = (c2.walk & 0xF);
				cwalkflag = ((newsolid | cwalkflag) & (~efflag)) | (newsolid & efflag);
			}
			else cwalkflag &= c2.walk;
		}
		else if ((iswater_type(c2.type) && get_bit(quest_rules,  qr_WATER_ON_LAYER_2) && ((c2.usrflags&cflag3) || (c2.usrflags&cflag4))) && ((c2.walk>>4)&b)) cwalkflag = 0;
		else cwalkflag |= c2.walk;
	}
	return (cwalkflag&b) ? !dried : false;
}

bool _effectflag_new(int32_t x, int32_t y, int32_t layer)
{
	mapscr* s0 = z3_get_mapscr_for_xy_offset(x, y);
	mapscr* s1 = z3_get_mapscr_layer_for_xy_offset(x, y, 1);
	mapscr* s2 = z3_get_mapscr_layer_for_xy_offset(x, y, 2);
	if (!s1->valid) s1 = s0;
	if (!s2->valid) s2 = s0;

	int32_t bx = COMBOPOS(x % 256, y % 176);
	newcombo c = combobuf[s0->data[bx]];
	newcombo c1 = combobuf[s1->data[bx]];
	newcombo c2 = combobuf[s2->data[bx]];
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

bool _effectflag(int32_t x,int32_t y,int32_t cnt, int32_t layer)
{
	if (is_z3_scrolling_mode())
	{
		int max_x = world_w;
		int max_y = world_h;
		if (!get_bit(quest_rules, qr_LTTPWALK))
		{
			max_x -= 7;
			max_y -= 7;
		}
		if (x < 0 || y < 0) return false;
		if (x >= max_x) return false;
		if (x >= max_x - 8 && cnt == 2) return false;
		if (y >= max_y) return false;
		return _effectflag_new(x, y, layer) || (cnt == 2 && _effectflag_new(x + 8, y, layer));
	}

	int max_x = world_w;
	int max_y = world_h;
	if (!get_bit(quest_rules, qr_LTTPWALK))
	{
		max_x -= 7;
		max_y -= 7;
	}
	if (x < 0 || y < 0) return false;
	if (x >= max_x) return false;
	if (x >= max_x - 8 && cnt == 2) return false;
	if (y >= max_y) return false;
	
	mapscr *s0, *s1, *s2;
	s0=&tmpscr;
	s1=(tmpscr2->valid)?tmpscr2:&tmpscr;
	s2=(tmpscr2[1].valid)?tmpscr2+1:&tmpscr;
	//  s2=TheMaps+((*tmpscr).layermap[1]-1)MAPSCRS+((*tmpscr).layerscreen[1]);
	
	if (layer == 0 && (s1 == s0)) return false;
	if (layer == 1 && (s2 == s0)) return false;
	
	int32_t bx=COMBOPOS(x, y);
	newcombo c = combobuf[s0->data[bx]];
	newcombo c1 = combobuf[s1->data[bx]];
	newcombo c2 = combobuf[s2->data[bx]];
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
	
	if((cwalkflag&b) && !dried)
		return true;
		
	if(cnt==1) return false;
	
	++bx;
	
	if(!(x&8))
		b<<=2;
	else
	{
		c  = combobuf[s0->data[bx]];
		c1 = combobuf[s1->data[bx]];
		c2 = combobuf[s2->data[bx]];
		dried = (((iswater_type(c.type)) || (iswater_type(c1.type)) ||
				  (iswater_type(c2.type))) && DRIEDLAKE);
		b=1;
		
		if(y&8) b<<=1;
	}
	cwalkflag = (c.walk>>4);
	if (layer == 0) cwalkflag = (c1.walk>>4);
	if (layer == 1) cwalkflag = (c2.walk>>4);
	//if (c.type == cBRIDGE || (iswater_type(c.type) && ((c.usrflags&cflag3) || (c.usrflags&cflag4)))) cwalkflag = 0;
	if (s1 != s0 && layer < 0)
	{
		if (c1.type == cBRIDGE) 
		{
			cwalkflag &= (~(c1.walk>>4));
		}
	}
	if (s2 != s0 && layer < 1)
	{
		if (c2.type == cBRIDGE) 
		{
			cwalkflag &= (~(c2.walk>>4));
		}
	}
	return (cwalkflag&b) ? !dried : false;
}

//used by mapdata->isSolid(x,y) in ZScript:
bool _walkflag(int32_t x,int32_t y,int32_t cnt, mapscr* m)
{
	{
		int max_x = world_w;
		int max_y = world_h;
		if (!get_bit(quest_rules, qr_LTTPWALK))
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
	
	// TODO z3 script
	// rpos_t rpos = COMBOPOS_REGION(bx, by);
	int32_t bx=COMBOPOS(x, y);
	newcombo c =  !is_z3_scrolling_mode() ? combobuf[m->data[bx]]  : combobuf[MAPCOMBO3(currmap, currscr, 0, bx, false)];
	newcombo c1 = !is_z3_scrolling_mode() ? combobuf[s1->data[bx]] : combobuf[MAPCOMBO3(currmap, currscr, 1, bx, false)];
	newcombo c2 = !is_z3_scrolling_mode() ? combobuf[s2->data[bx]] : combobuf[MAPCOMBO3(currmap, currscr, 2, bx, false)];
	bool dried = (((iswater_type(c.type)) || (iswater_type(c1.type)) ||
				   (iswater_type(c2.type))) && DRIEDLAKE);
	int32_t b=1;
	
	if(x&8) b<<=2;
	
	if(y&8) b<<=1;
	
	int32_t cwalkflag = c.walk;
	// Bridge combos supress the solidity of the layer below them.
	if (c1.type == cBRIDGE)
	{
		if (!get_bit(quest_rules, qr_OLD_BRIDGE_COMBOS))
		{
			int efflag = (c1.walk & 0xF0)>>4;
			int newsolid = (c1.walk & 0xF);
			cwalkflag = ((newsolid | cwalkflag) & (~efflag)) | (newsolid & efflag);
		}
		else cwalkflag &= c1.walk;
	}
	else if (s1 != m) cwalkflag |= c1.walk;
	if (c2.type == cBRIDGE)
	{
		if (!get_bit(quest_rules, qr_OLD_BRIDGE_COMBOS))
		{
			int efflag = (c2.walk & 0xF0)>>4;
			int newsolid = (c2.walk & 0xF);
			cwalkflag = ((newsolid | cwalkflag) & (~efflag)) | (newsolid & efflag);
		}
		else cwalkflag &= c2.walk;
	}
	else if (s2 != m) cwalkflag |= c2.walk;
	
	if((cwalkflag&b) && !dried)
		return true;
		
	if(cnt==1) return false;
	
	++bx;
	
	if(!(x&8))
		b<<=2;
	else
	{
		c  = combobuf[m->data[bx]];
		c1 = combobuf[s1->data[bx]];
		c2 = combobuf[s2->data[bx]];
		dried = (((iswater_type(c.type)) || (iswater_type(c1.type)) ||
				  (iswater_type(c2.type))) && DRIEDLAKE);
		b=1;
		
		if(y&8) b<<=1;
	}
	
	cwalkflag = c.walk;
	if (c1.type == cBRIDGE)
	{
		if (!get_bit(quest_rules, qr_OLD_BRIDGE_COMBOS))
		{
			int efflag = (c1.walk & 0xF0)>>4;
			int newsolid = (c1.walk & 0xF);
			cwalkflag = ((newsolid | cwalkflag) & (~efflag)) | (newsolid & efflag);
		}
		else cwalkflag &= c1.walk;
	}
	else if (s1 != m) cwalkflag |= c1.walk;
	if (c2.type == cBRIDGE)
	{
		if (!get_bit(quest_rules, qr_OLD_BRIDGE_COMBOS))
		{
			int efflag = (c2.walk & 0xF0)>>4;
			int newsolid = (c2.walk & 0xF);
			cwalkflag = ((newsolid | cwalkflag) & (~efflag)) | (newsolid & efflag);
		}
		else cwalkflag &= c2.walk;
	}
	else if (s2 != m) cwalkflag |= c2.walk;
	return (cwalkflag&b) ? !dried : false;
}

// TODO z3 script
bool _walkflag(int32_t x,int32_t y,int32_t cnt, mapscr* m, mapscr* s1, mapscr* s2)
{
	{
		int max_x = world_w;
		int max_y = world_h;
		if (!get_bit(quest_rules, qr_LTTPWALK))
		{
			max_x -= 7;
			max_y -= 7;
		}
		if (x < 0 || y < 0) return false;
		if (x >= max_x) return false;
		if (x >= max_x - 8 && cnt == 2) return false;
		if (y >= max_y) return false;
	}

	if(!s1) s1 = m;
	if(!s2) s2 = m;
	
	int32_t bx=(x>>4)+(y&0xF0);
	newcombo c = combobuf[m->data[bx]];
	newcombo c1 = combobuf[s1->data[bx]];
	newcombo c2 = combobuf[s2->data[bx]];
	bool dried = (((iswater_type(c.type)) || (iswater_type(c1.type)) ||
				   (iswater_type(c2.type))) && DRIEDLAKE);
	int32_t b=1;
	
	if(x&8) b<<=2;
	
	if(y&8) b<<=1;
	
	int32_t cwalkflag = c.walk;
	if (c1.type == cBRIDGE)
	{
		if (!get_bit(quest_rules, qr_OLD_BRIDGE_COMBOS))
		{
			int efflag = (c1.walk & 0xF0)>>4;
			int newsolid = (c1.walk & 0xF);
			cwalkflag = ((newsolid | cwalkflag) & (~efflag)) | (newsolid & efflag);
		}
		else cwalkflag &= c1.walk;
	}
	else if (s1 != m) cwalkflag |= c1.walk;
	if (c2.type == cBRIDGE)
	{
		if (!get_bit(quest_rules, qr_OLD_BRIDGE_COMBOS))
		{
			int efflag = (c2.walk & 0xF0)>>4;
			int newsolid = (c2.walk & 0xF);
			cwalkflag = ((newsolid | cwalkflag) & (~efflag)) | (newsolid & efflag);
		}
		else cwalkflag &= c2.walk;
	}
	else if (s2 != m) cwalkflag |= c2.walk;
	
	if((cwalkflag&b) && !dried)
		return true;
		
	if(cnt==1) return false;
	
	++bx;
	
	if(!(x&8))
		b<<=2;
	else
	{
		c  = combobuf[m->data[bx]];
		c1 = combobuf[s1->data[bx]];
		c2 = combobuf[s2->data[bx]];
		dried = (((iswater_type(c.type)) || (iswater_type(c1.type)) ||
				  (iswater_type(c2.type))) && DRIEDLAKE);
		b=1;
		
		if(y&8) b<<=1;
	}
	
	cwalkflag = c.walk;
	if (c1.type == cBRIDGE)
	{
		if (!get_bit(quest_rules, qr_OLD_BRIDGE_COMBOS))
		{
			int efflag = (c1.walk & 0xF0)>>4;
			int newsolid = (c1.walk & 0xF);
			cwalkflag = ((newsolid | cwalkflag) & (~efflag)) | (newsolid & efflag);
		}
		else cwalkflag &= c1.walk;
	}
	else if (s1 != m) cwalkflag |= c1.walk;
	if (c2.type == cBRIDGE) 
	{
		if (!get_bit(quest_rules, qr_OLD_BRIDGE_COMBOS))
		{
			int efflag = (c2.walk & 0xF0)>>4;
			int newsolid = (c2.walk & 0xF);
			cwalkflag = ((newsolid | cwalkflag) & (~efflag)) | (newsolid & efflag);
		}
		else cwalkflag &= c2.walk;
	}
	else if (s2 != m) cwalkflag |= c2.walk;
	return (cwalkflag&b) ? !dried : false;
}

bool _walkflag_layer(int32_t x, int32_t y, int32_t layer, int32_t cnt)
{
	mapscr* m = get_layer_scr_for_xy(x, y, layer);
	if (m->valid == 0) return false;
	return _walkflag_layer(x, y, cnt, m);
}

//Only check the given mapscr*, not it's layer 1&2
bool _walkflag_layer(int32_t x,int32_t y,int32_t cnt, mapscr* m)
{
	// TODO z3
	if (is_z3_scrolling_mode())
	{
		int max_x = world_w;
		int max_y = world_h;
		if (!get_bit(quest_rules, qr_LTTPWALK))
		{
			max_x -= 7;
			max_y -= 7;
		}
		if (x < 0 || y < 0) return false;
		if (x >= max_x) return false;
		if (x >= max_x - 8 && cnt == 2) return false;
		if (y >= max_y) return false;
	}
	else if(get_bit(quest_rules,qr_LTTPWALK))
	{
		if(x<0||y<0) return false;
		
		if(x>255) return false;
		
		if(x>247&&cnt==2) return false;
		
		if(y>175) return false;
	}
	else
	{
		if(x<0||y<0) return false;
		
		if(x>248) return false;
		
		if(x>240&&cnt==2) return false;
		
		if(y>168) return false;
	}
	if(!m) return true;
	
	int32_t bx=(x>>4)+(y&0xF0);
	newcombo c = combobuf[m->data[bx]];
	bool dried = ((iswater_type(c.type)) && DRIEDLAKE);
	int32_t b=1;
	
	if(x&8) b<<=2;
	
	if(y&8) b<<=1;
	
	if((c.walk&b) && !dried)
		return true;
		
	if(cnt==1) return false;
	
	++bx;
	
	if(!(x&8))
		b<<=2;
	else
	{
		c  = combobuf[m->data[bx]];
		dried = ((iswater_type(c.type)) && DRIEDLAKE);
		b=1;
		
		if(y&8) b<<=1;
	}
	
	return (c.walk&b) ? !dried : false;
}

bool _effectflag_layer(int32_t x, int32_t y, int32_t layer, int32_t cnt)
{
	mapscr* m = get_layer_scr_for_xy(x, y, layer);
	if (m->valid == 0) return false;
	return _effectflag_layer(x, y, cnt, m);
}

bool _effectflag_layer(int32_t x,int32_t y,int32_t cnt, mapscr* m)
{
	int max_x = world_w;
	int max_y = world_h;
	if (!get_bit(quest_rules, qr_LTTPWALK))
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
	newcombo c = combobuf[m->data[bx]];
	bool dried = ((iswater_type(c.type)) && DRIEDLAKE);
	int32_t b=1;
	
	if(x&8) b<<=2;
	
	if(y&8) b<<=1;
	
	if(((c.walk>>4)&b) && !dried)
		return true;
		
	if(cnt==1) return false;
	
	++bx;
	
	if(!(x&8))
		b<<=2;
	else
	{
		c  = combobuf[m->data[bx]];
		dried = ((iswater_type(c.type)) && DRIEDLAKE);
		b=1;
		
		if(y&8) b<<=1;
	}
	
	return ((c.walk>>4)&b) ? !dried : false;
}

bool water_walkflag(int32_t x,int32_t y,int32_t cnt)
{
	if(get_bit(quest_rules,qr_LTTPWALK))
	{
		if(x<0||y<0) return false;
		
		if(x>255) return false;
		
		if(x>247&&cnt==2) return false;
		
		if(y>175) return false;
	}
	else
	{
		if(x<0||y<0) return false;
		
		if(x>248) return false;
		
		if(x>240&&cnt==2) return false;
		
		if(y>168) return false;
	}
	
	mapscr *s1, *s2;
    s1=(tmpscr2->valid)?tmpscr2:&tmpscr;
    s2=(tmpscr2[1].valid)?tmpscr2+1:&tmpscr;
	
	int32_t bx=(x>>4)+(y&0xF0);
	newcombo c = combobuf[tmpscr.data[bx]];
	newcombo c1 = combobuf[s1->data[bx]];
	newcombo c2 = combobuf[s2->data[bx]];
	int32_t b=1;
	
	if(x&8) b<<=2;
	
	if(y&8) b<<=1;
	
	if(get_bit(quest_rules, qr_NO_SOLID_SWIM))
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
	if(cnt==1) return false;
	
	if(x&8)
		b<<=2;
	else
	{
		c = combobuf[tmpscr.data[++bx]];
		c1 = combobuf[s1->data[bx]];
		c2 = combobuf[s2->data[bx]];
		b=1;
		
		if(y&8) b<<=1;
	}
	
	if(get_bit(quest_rules, qr_NO_SOLID_SWIM))
	{
		return (c.walk&b) || (c1.walk&b) || (c2.walk&b);
	}
	else return (c.walk&b) ? !iswater_type(c.type) :
		   (c1.walk&b) ? !iswater_type(c1.type) :
		   (c2.walk&b) ? !iswater_type(c2.type) :false;
}

bool hit_walkflag(int32_t x,int32_t y,int32_t cnt)
{
	if(dlevel)
		if(x<32 || y<40 || (x+(cnt-1)*8)>=224 || y>=144)
			return true;
			
	if(blockpath && y<((get_bit(quest_rules,qr_LTTPCOLLISION))?80:88))
		return true;
		
	if(x<16 || y<16 || (x+(cnt-1)*8)>=240 || y>=160)
		return true;
		
	//  for(int32_t i=0; i<4; i++)
	if(mblock2.clk && mblock2.hit(x,y,0,cnt*8,1,16))
		return true;
		
	return _walkflag(x,y,cnt);
}

void map_bkgsfx(bool on)
{
	if(on)
	{
		cont_sfx(tmpscr.oceansfx);
		
		if(tmpscr.bosssfx && !(game->lvlitems[dlevel]&liBOSS))
			cont_sfx(tmpscr.bosssfx);
	}
	else
	{
		adjust_sfx(tmpscr.oceansfx,128,false);
		adjust_sfx(tmpscr.bosssfx,128,false);
		
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

	for_every_screen_in_region([&](mapscr* z3_scr, int screen_index, unsigned int z3_scr_dx, unsigned int z3_scr_dy) {
		mapscr* screen = get_layer_scr(currmap, screen_index, -1);
		toggle_switches(flags, entry, screen, screen_index);
	});
}
void toggle_switches(dword flags, bool entry, mapscr* m, int screen_index)
{
	if(!flags) return; //No flags to toggle
	byte togglegrid[176] = {0};
	for(int32_t lyr = 0; lyr < 7; ++lyr)
	{
		mapscr* scr = lyr ? get_layer_scr(currmap, screen_index, lyr - 1) : m;
		for(int32_t pos = 0; pos < 176; ++pos)
		{
			newcombo const& cmb = combobuf[scr->data[pos]];
			if((cmb.type == cCSWITCH || cmb.type == cCSWITCHBLOCK) && cmb.attribytes[0] < 32)
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
					if(cmb.type == cCSWITCH) continue; //Switches don't toggle other layers
					for(int32_t lyr2 = 0; lyr2 < 7; ++lyr2) //Toggle same pos on other layers, if flag set
					{
						if(lyr==lyr2) continue;
						if(!(cmb.usrflags&(1<<lyr2))) continue;
						if(togglegrid[pos]&(1<<lyr2)) continue;
						mapscr* scr_2 = (lyr2 ? get_layer_scr(currmap, screen_index, lyr2 - 1) : m);
						if(!scr_2->data[pos]) //Don't increment empty space
							continue;
						newcombo const& cmb_2 = combobuf[scr_2->data[pos]];
						if(lyr2 > lyr && (cmb_2.type == cCSWITCH || cmb_2.type == cCSWITCHBLOCK) && cmb_2.attribytes[0] < 32 && (flags&(1<<cmb_2.attribytes[0])))
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
}

/****  View Map  ****/

//BITMAP *mappic = NULL;
int32_t mapres = 0;

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
	mapscr tmpscr_a[2];
	mapscr tmpscr_b[2];
	mapscr tmpscr_c[6];

	tmpscr_a[0] = tmpscr;
	tmpscr.zero_memory();
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
		system_pal();
		jwin_alert("View Map","Not enough memory.",NULL,NULL,"OK",NULL,13,27,lfont);
		game_pal();
		return;
	}
	
	// draw the map
	set_clip_rect(scrollbuf_old, 0, 0, scrollbuf_old->w, scrollbuf_old->h);

	global_viewport_x = 0;
	global_viewport_y = 0;
	
	for(int32_t y=0; y<8; y++)
	{
		for(int32_t x=0; x<16; x++)
		{
			rectfill(scrollbuf_old, 256, 0, 511, 223, WHITE);
			if(displayOnMap(x, y))
			{
				int32_t s = (y<<4) + x;
				tmpscr.zero_memory();
				special_warp_return_screen.zero_memory();
				loadscr2(1,s,-1);
				tmpscr = special_warp_return_screen;
				if(tmpscr.valid&mVALID)
				{
					for(int32_t i=0; i<6; i++)
					{
						tmpscr2[i].zero_memory();
						if(tmpscr.layermap[i]<=0)
							continue;
						
						if((ZCMaps[tmpscr.layermap[i]-1].tileWidth==ZCMaps[currmap].tileWidth) &&
						   (ZCMaps[tmpscr.layermap[i]-1].tileHeight==ZCMaps[currmap].tileHeight))
						{
							const int32_t _mapsSize = (ZCMaps[currmap].tileWidth)*(ZCMaps[currmap].tileHeight);
							
							tmpscr2[i]=TheMaps[(tmpscr.layermap[i]-1)*MAPSCRS+tmpscr.layerscreen[i]];
							
							tmpscr2[i].data.resize(_mapsSize, 0);
							tmpscr2[i].sflag.resize(_mapsSize, 0);
							tmpscr2[i].cset.resize(_mapsSize, 0);
						}
					}
					
					if(XOR(tmpscr.flags7&fLAYER2BG, DMaps[currdmap].flags&dmfLAYER2BG)) do_layer(scrollbuf_old, 0, 2, &tmpscr, -256, playing_field_offset, 2);
					
					if(XOR(tmpscr.flags7&fLAYER3BG, DMaps[currdmap].flags&dmfLAYER3BG)) do_layer(scrollbuf_old, 0, 3, &tmpscr, -256, playing_field_offset, 2);
					
					putscr(scrollbuf_old,256,0,&tmpscr);
					do_layer(scrollbuf_old, 0, 1, &tmpscr, -256, playing_field_offset, 2);
					
					if(!XOR((tmpscr.flags7&fLAYER2BG), DMaps[currdmap].flags&dmfLAYER2BG)) do_layer(scrollbuf_old, 0, 2, &tmpscr, -256, playing_field_offset, 2);
					
					putscrdoors(scrollbuf_old,256,0,&tmpscr);
					do_layer(scrollbuf_old,-2, 0, &tmpscr, -256, playing_field_offset, 2);
					if(get_bit(quest_rules, qr_PUSHBLOCK_LAYER_1_2))
					{
						do_layer(scrollbuf_old,-2, 1, &tmpscr, -256, playing_field_offset, 2);
						do_layer(scrollbuf_old,-2, 2, &tmpscr, -256, playing_field_offset, 2);
					}
					do_layer(scrollbuf_old,-3, 0, &tmpscr, -256, playing_field_offset, 2); // Freeform combos!
					
					if(!XOR((tmpscr.flags7&fLAYER3BG), DMaps[currdmap].flags&dmfLAYER3BG)) do_layer(scrollbuf_old, 0, 3, &tmpscr, -256, playing_field_offset, 2);
					
					do_layer(scrollbuf_old, 0, 4, &tmpscr, -256, playing_field_offset, 2);
					do_layer(scrollbuf_old,-1, 0, &tmpscr, -256, playing_field_offset, 2);
					if(get_bit(quest_rules, qr_OVERHEAD_COMBOS_L1_L2))
					{
						do_layer(scrollbuf_old,-1, 1, &tmpscr, -256, playing_field_offset, 2);
						do_layer(scrollbuf_old,-1, 2, &tmpscr, -256, playing_field_offset, 2);
					}
					do_layer(scrollbuf_old, 0, 5, &tmpscr, -256, playing_field_offset, 2);
					do_layer(scrollbuf_old, 0, 6, &tmpscr, -256, playing_field_offset, 2);
				}
			}
			
			stretch_blit(scrollbuf_old, mappic, 256, 0, 256, 176, x<<(8-mapres), (y*176)>>mapres, 256>>mapres, 176>>mapres);
		}
	}
	
	tmpscr = tmpscr_a[0];
	special_warp_return_screen = tmpscr_a[1];
	for(int32_t i=0; i<6; ++i)
	{
		tmpscr2[i]=tmpscr_c[i];
	}
	
	
	clear_keybuf();
	pause_all_sfx();
	
	// view it
	int32_t delay = 0;
	static int32_t show  = 3;
	
	do
	{
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
			--show;
			
		px = vbound(px,-4096,4096);
		py = vbound(py,-1408,1408);
		
		double scale = scales[sc];
		
		if(!redraw)
		{
			blit(scrollbuf,framebuf,256,0,0,0,256,224);
		}
		else
		{
			clear_to_color(framebuf,BLACK);
			stretch_blit(mappic,framebuf,0,0,mappic->w,mappic->h,
						 int32_t(256+(int64_t(px)-mappic->w)*scale)/2,int32_t(224+(int64_t(py)-mappic->h)*scale)/2,
						 int32_t(mappic->w*scale),int32_t(mappic->h*scale));
						 
			blit(framebuf,scrollbuf,0,0,256,0,256,224);
			redraw=false;
		}
		
		int32_t x = int32_t(256+(px-((2048-int64_t(lx))*2))*scale)/2;
		int32_t y = int32_t(224+(py-((704-int64_t(ly))*2))*scale)/2;
		
		if(show&1)
		{
			line(framebuf,x-7,y-7,x+7,y+7,(frame&3)+252);
			line(framebuf,x+7,y-7,x-7,y+7,(frame&3)+252);
		}
		
		//    text_mode(BLACK);
		
		if(show&2 || r)
			textprintf_ex(framebuf,font,224,216,WHITE,BLACK,"%1.2f",scale);
			
		if(r)
		{
			textprintf_ex(framebuf,font,0,208,WHITE,BLACK,"m: %d %d",px,py);
			textprintf_ex(framebuf,font,0,216,WHITE,BLACK,"x: %d %d",x,y);
		}
		
		//since stuff in here accesses tmpscr and tmpscr2... -DD
		advanceframe(false, false);
		
		
		if(getInput(btnS, true, false, true)) //rSbtn
			done = true;
			
	}
	while(!done && !Quit);
	
	destroy_bitmap(mappic);
	
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


/*** end of maps.cc ***/

