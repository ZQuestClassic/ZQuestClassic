#include <functional>
#include <optional>
#include <type_traits>
#include "base/zdefs.h"
#include "zc/maps.h"

// Iterates over every screen in the current region.
// Callback function: void fn(mapscr* screen, int screen_index, unsigned int region_scr_x, unsigned int region_scr_x)
// region_scr_x and region_scr_y are the screen coordinates relative to the origin screen. For example, the relative
// coordinates for the screen just to the right of the origin screen is (1, 0). This is always (0, 0) when not in a
// region.
template<typename T, typename = std::enable_if_t<
    std::is_invocable_v<T, mapscr*, int, unsigned int, unsigned int>
>>
void for_every_screen_in_region(T fn)
{
	if (!is_z3_scrolling_mode())
	{
		fn(&tmpscr, currscr, 0, 0);
		return;
	}

	for (int screen_index = 0; screen_index < 128; screen_index++)
	{
		if (is_in_current_region(screen_index))
		{
			mapscr* screen = get_scr(currmap, screen_index);
			unsigned int region_scr_x = z3_get_region_relative_dx(screen_index);
			unsigned int region_scr_y = z3_get_region_relative_dy(screen_index);
			fn(screen, screen_index, region_scr_x, region_scr_y);
		}
	}
}

// Iterates over every rpos in the current region.
// Callback function: void fn(const pos_handle_t& rpos_handle)
template<typename T, typename = std::enable_if_t<
    std::is_invocable_v<T, const rpos_handle_t&>
>>
void for_every_rpos_in_region(T fn)
{
	rpos_handle_t rpos_handle;
	for (int screen_index = 0; screen_index < 128; screen_index++)
	{
		// TODO z3 should cache this
		if (!is_in_current_region(screen_index)) continue;

		rpos_t base_rpos = POS_TO_RPOS(0, z3_get_region_relative_dx(screen_index), z3_get_region_relative_dy(screen_index));
		rpos_handle.screen_index = screen_index;
		for (int lyr = 0; lyr <= 6; ++lyr)
		{
			mapscr* scr = get_layer_scr(currmap, screen_index, lyr - 1);
			rpos_handle.screen = scr;
			rpos_handle.layer = lyr;

			for (int pos = 0; pos < 176; ++pos)
			{
				rpos_handle.rpos = (rpos_t)((int)base_rpos + pos);
				fn(rpos_handle);
			}
		}
	}
}

// Iterates over every ffc in the current region.
// Callback function: bool fn(const ffc_handle_t& ffc_handle)
// If the callback returns false, the exeuction stops early.
template<typename T, typename = std::enable_if_t<
    std::is_invocable_r_v<bool, T, const ffc_handle_t&>
>>
void for_every_ffc_in_region(T fn)
{
	for (int screen_index = 0; screen_index < 128; screen_index++)
	{
		if (is_in_current_region(screen_index))
		{
			mapscr* screen = get_scr(currmap, screen_index);
			int screen_index_offset = get_region_screen_index_offset(screen_index);

			int c = screen->numFFC();
			for (int i = 0; i < c; i++)
			{
				int region_id = screen_index_offset * 128 + i;
				ffc_handle_t ffc_handle = {screen, screen_index, region_id, i, &screen->ffcs[i]};
				if (!fn(ffc_handle)) return;
			}
		}
	}
}

// Iterates over every ffc in the current region, and returns a handle to the
// first one for which the callback functions evaluates true for.
// Callback function: bool fn(const ffc_handle_t& ffc_handle)
template<typename T, typename = std::enable_if_t<
    std::is_invocable_r_v<bool, T, const ffc_handle_t&>
>>
std::optional<ffc_handle_t> find_ffc_in_region(T fn)
{
	for (int screen_index = 0; screen_index < 128; screen_index++)
	{
		if (is_in_current_region(screen_index))
		{
			mapscr* screen = get_scr(currmap, screen_index);
			int screen_index_offset = get_region_screen_index_offset(screen_index);

			int c = screen->numFFC();
			for (int i = 0; i < c; i++)
			{
				int region_id = screen_index_offset * 128 + i;
				ffc_handle_t ffc_handle = {screen, screen_index, region_id, i, &screen->ffcs[i]};
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
template<typename T, typename = std::enable_if_t<
    std::is_invocable_v<T, const rpos_handle_t&>
>>
void for_every_rpos_in_screen(mapscr* screen, int screen_index, T fn)
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
			fn(rpos_handle);
		}
	}
}
