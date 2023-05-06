#include <functional>
#include <type_traits>
#include "base/zdefs.h"
#include "maps.h"

// TODO z3_scr_dx -> offset_x
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

	int z3_scr_x = z3_get_origin_scr() % 16;
	int z3_scr_y = z3_get_origin_scr() / 16;

	for (int scr = 0; scr < 128; scr++)
	{
		if (is_in_current_region(scr))
		{
			unsigned int scr_x = scr % 16;
			unsigned int scr_y = scr / 16;
			unsigned int z3_scr_dx = scr_x - z3_scr_x;
			unsigned int z3_scr_dy = scr_y - z3_scr_y;
			mapscr* z3_scr = get_scr(currmap, scr);
			fn(z3_scr, scr, z3_scr_dx, z3_scr_dy);
		}
	}
}

template<typename T, typename = std::enable_if_t<
    std::is_invocable_v<T, const pos_handle_t&>
>>
void for_every_rpos_in_region(T fn)
{
	pos_handle_t pos_handle;
	for (int screen_index = 0; screen_index < 128; screen_index++)
	{
		// TODO z3 should cache this
		if (!is_in_current_region(screen_index)) continue;

		rpos_t base_rpos = POS_TO_RPOS(0, z3_get_region_relative_dx(screen_index), z3_get_region_relative_dy(screen_index));
		pos_handle.screen_index = screen_index;
		for (int lyr = 0; lyr <= 6; ++lyr)
		{
			mapscr* scr = get_layer_scr(currmap, screen_index, lyr - 1);
			pos_handle.screen = scr;
			pos_handle.layer = lyr;

			for (int pos = 0; pos < 176; ++pos)
			{
				pos_handle.rpos = (rpos_t)((int)base_rpos + pos);
				fn(pos_handle);
			}
		}
	}
}

// Stops execution when lambda returns false.
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

			int c = screen->numFFC();
			for (int i = 0; i < c; i++)
			{
				ffc_handle_t ffc_handle = {screen, screen_index, i, &screen->ffcs[i]};
				if (!fn(ffc_handle)) return;
			}
		}
	}
}

template<typename T, typename = std::enable_if_t<
    std::is_invocable_v<T, const pos_handle_t&>
>>
void for_every_rpos_in_screen(mapscr* screen, int screen_index, T fn)
{
	pos_handle_t pos_handle;
	pos_handle.screen_index = screen_index;
	rpos_t base_rpos = POS_TO_RPOS(0, z3_get_region_relative_dx(screen_index), z3_get_region_relative_dy(screen_index));
	for (int lyr = 0; lyr <= 6; ++lyr)
	{
		pos_handle.screen = lyr == 0 ? screen : get_layer_scr(currmap, screen_index, lyr - 1);
		pos_handle.layer = lyr;
		for (int pos = 0; pos < 176; ++pos)
		{
			pos_handle.rpos = (rpos_t)((int)base_rpos + pos);
			fn(pos_handle);
		}
	}
}
