#include "base/handles.h"
#include "base/qrs.h"
#include "base/zdefs.h"
#include "zc/maps.h"
#include "zc/zelda.h"
#include <functional>
#include <optional>
#include <type_traits>
#include <stdint.h>

// Iterates over every base screen in the current region.
// Callback function: void fn(mapscr* screen, int screen_index, unsigned int region_scr_x, unsigned int region_scr_x)
// region_scr_x and region_scr_y are the screen coordinates relative to the origin screen. For example, the relative
// coordinates for the screen just to the right of the origin screen is (1, 0). This is always (0, 0) when not in a
// region.
template<typename T>
requires std::is_invocable_v<T, mapscr*, int, unsigned int, unsigned int>
void for_every_screen_in_region(T&& fn)
{
	if (!is_z3_scrolling_mode())
	{
		fn(tmpscr, currscr, 0, 0);
		return;
	}

	auto [handles, count] = z3_get_current_region_handles();

	for (int i = 0; i < count; i++)
	{
		if (handles[i].layer == 0)
		{
			mapscr* screen = handles[i].screen;
			unsigned int screen_index = handles[i].screen_index;
			unsigned int region_scr_x = z3_get_region_relative_dx(screen_index);
			unsigned int region_scr_y = z3_get_region_relative_dy(screen_index);
			fn(screen, screen_index, region_scr_x, region_scr_y);
		}
	}
}

// Iterates over every rpos in the current region, but only for screens that are valid.
// Callback function: void fn(const pos_handle_t& rpos_handle)
template<typename T>
requires std::is_invocable_v<T, const rpos_handle_t&>
void for_every_rpos_in_region(T&& fn)
{
	auto [handles, count] = z3_get_current_region_handles();

	for (int i = 0; i < count; i++)
	{
		rpos_handle_t rpos_handle = handles[i];
		for (int j = 0; j < 176; j++)
		{
			fn(rpos_handle);
			rpos_handle.rpos = (rpos_t)((int)rpos_handle.rpos + 1);
			rpos_handle.pos += 1;
		}
	}
}

// Iterates over every ffc in the current region.
// Callback function: void fn(const ffc_handle_t& ffc_handle)
template<typename T>
requires std::is_invocable_v<T, const ffc_handle_t&>
void for_every_ffc_in_region(T&& fn)
{
	auto [handles, count] = z3_get_current_region_handles();
	
	for (int i = 0; i < count; i++)
	{
		if (handles[i].layer != 0)
			continue;

		mapscr* screen = handles[i].screen;
		uint8_t screen_index = handles[i].screen_index;
		int screen_index_offset = get_region_screen_index_offset(screen_index);
		int c = screen->numFFC();
		for (uint8_t j = 0; j < c; j++)
		{
			uint16_t id = screen_index_offset * MAXFFCS + j;
			ffc_handle_t ffc_handle = {screen, screen_index, id, j, &screen->ffcs[j]};
			fn(ffc_handle);
		}
	}
}

template<typename T>
requires std::is_invocable_v<T, const rpos_handle_t&> && std::is_invocable_v<T, const ffc_handle_t&>
void for_every_combo_in_region(T&& fn, bool include_ffcs = !get_qr(qr_OLD_FFC_FUNCTIONALITY))
{
	for_every_rpos_in_region(fn);
	if (include_ffcs)
		for_every_ffc_in_region(fn);
}

// Iterates over every ffc in the current region, until execution is requested to stop.
// Callback function: bool fn(const ffc_handle_t& ffc_handle)
// If the callback returns false, the exeuction stops early.
template<typename T>
requires std::is_invocable_v<T, const ffc_handle_t&>
void for_some_ffcs_in_region(T&& fn)
{
	auto [handles, count] = z3_get_current_region_handles();
	
	for (int i = 0; i < count; i++)
	{
		if (handles[i].layer != 0)
			continue;

		mapscr* screen = handles[i].screen;
		uint8_t screen_index = handles[i].screen_index;
		int screen_index_offset = get_region_screen_index_offset(screen_index);
		int c = screen->numFFC();
		for (uint8_t j = 0; j < c; j++)
		{
			uint16_t id = screen_index_offset * MAXFFCS + j;
			ffc_handle_t ffc_handle = {screen, screen_index, id, j, &screen->ffcs[j]};
			if (!fn(ffc_handle)) return;
		}
	}
}

// Iterates over every ffc in the current region, and returns a handle to the
// first one for which the callback functions evaluates true for.
// Callback function: bool fn(const ffc_handle_t& ffc_handle)
template<typename T>
requires std::is_invocable_v<T, const ffc_handle_t&>
std::optional<ffc_handle_t> find_ffc_in_region(T&& fn)
{
	auto [handles, count] = z3_get_current_region_handles();
	
	for (int i = 0; i < count; i++)
	{
		if (handles[i].layer != 0)
			continue;

		mapscr* screen = handles[i].screen;
		uint8_t screen_index = handles[i].screen_index;
		int screen_index_offset = get_region_screen_index_offset(screen_index);
		int c = screen->numFFC();
		for (uint8_t j = 0; j < c; j++)
		{
			if (screen->ffcs[j].data)
			{
				uint16_t id = screen_index_offset * MAXFFCS + j;
				ffc_handle_t ffc_handle = {screen, screen_index, id, j, &screen->ffcs[j]};
				if (fn(ffc_handle)) return ffc_handle;
			}
		}
	}

	return std::nullopt;
}

// __attribute__((pure)) static mapscr* get_layer_pure(int map, int screen_index, int layer)
// {
// 	return get_layer_scr(map, screen_index, layer);
// }

// Iterates over every rpos for a specified screen.
// Callback function: void fn(const pos_handle_t& pos_handle_t)
template<typename T>
requires std::is_invocable_v<T, const rpos_handle_t&>
void for_every_rpos_in_screen(mapscr* screen, int screen_index, T&& fn)
{
	rpos_handle_t rpos_handle;
	rpos_handle.screen_index = screen_index;
	rpos_t base_rpos = POS_TO_RPOS(0, z3_get_region_relative_dx(screen_index), z3_get_region_relative_dy(screen_index));
	for (int lyr = 0; lyr <= 6; ++lyr)
	{
		// TODO z3 would `__attribute__((pure))` on get_layer_scr allow compiler to optimize more here?
		rpos_handle.screen = lyr == 0 ? screen : get_layer_scr(currmap, screen_index, lyr - 1);
		rpos_handle.layer = lyr;
		for (int pos = 0; pos < 176; ++pos)
		{
			rpos_handle.rpos = (rpos_t)((int)base_rpos + pos);
			rpos_handle.pos = pos;
			fn(rpos_handle);
		}
	}
}

template<typename T>
requires std::is_invocable_v<T, const ffc_handle_t&>
void for_every_ffc_in_screen(mapscr* screen, int screen_index, T&& fn)
{
	int screen_index_offset = get_region_screen_index_offset(screen_index);
	int c = screen->numFFC();
	for (uint8_t i = 0; i < c; i++)
	{
		uint16_t id = screen_index_offset * MAXFFCS + i;
		ffc_handle_t ffc_handle = {screen, (uint8_t)screen_index, id, i, &screen->ffcs[i]};
		fn(ffc_handle);
	}
}
