#include "base/compiler.h"
#include "base/handles.h"
#include "base/mapscr.h"
#include "base/qrs.h"
#include "base/zdefs.h"
#include "zc/maps.h"
#include "zc/zelda.h"
#include <functional>
#include <optional>
#include <type_traits>
#include <stdint.h>

// Iterates over every base screen (layer 0) in the current region.
// Callback function: void fn(mapscr* scr, unsigned int region_scr_x, unsigned int region_scr_x)
// region_scr_x and region_scr_y are the screen coordinates relative to the origin screen. For example, the relative
// coordinates for the screen just to the right of the origin screen is (1, 0). This is always (0, 0) when not in a
// region.
template<typename T>
requires std::is_invocable_v<T, mapscr*, unsigned int, unsigned int>
ZC_FORCE_INLINE void for_every_screen_in_region(T&& fn)
{
	if (!is_in_scrolling_region())
	{
		fn(origin_scr, 0, 0);
		return;
	}

	for (int y = 0; y < current_region.screen_height; y++)
	{
		for (int x = 0; x < current_region.screen_width; x++)
		{
			int screen = current_region.origin_screen + map_scr_xy_to_index(x, y);
			mapscr* scr = get_scr(screen);
			fn(scr, x, y);
		}
	}
}

// Iterates over every rpos in the current region, but only for screens that are valid.
// Hits every layer too.
// Callback function: void fn(const rpos_handle_t& rpos_handle)
template<typename T>
requires std::is_invocable_v<T, const rpos_handle_t&>
ZC_FORCE_INLINE void for_every_rpos(T&& fn)
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

// Iterates over every rpos in the current region, but only for screens that are valid,
// and only for layer 0.
// Callback function: void fn(const rpos_handle_t& rpos_handle)
template<typename T>
requires std::is_invocable_v<T, const rpos_handle_t&>
ZC_FORCE_INLINE void for_every_rpos_layer0(T&& fn)
{
	auto [handles, count] = z3_get_current_region_handles();

	for (int i = 0; i < count; i++)
	{
		rpos_handle_t rpos_handle = handles[i];
		if (rpos_handle.layer != 0) continue;

		for (int j = 0; j < 176; j++)
		{
			fn(rpos_handle);
			rpos_handle.rpos = (rpos_t)((int)rpos_handle.rpos + 1);
			rpos_handle.pos += 1;
		}
	}
}

// Iterates over every visbile rpos in the current region, but only for screens that are valid,
// but only for layer 0.
// Callback function: void fn(const rpos_handle_t& rpos_handle)
template<typename T>
requires std::is_invocable_v<T, const rpos_handle_t&>
ZC_FORCE_INLINE void for_every_visible_rpos_layer0(T&& fn)
{
	if (!is_in_scrolling_region())
	{
		for_every_rpos_layer0(fn);
		return;
	}

	int x0 = viewport.x / 16;
	int y0 = viewport.y / 16;
	int x1 = std::ceil(viewport.right() / 16.0);
	int y1 = std::ceil(viewport.bottom() / 16.0);

	for (int x = x0; x <= x1; x++)
	{
		for (int y = y0; y <= y1; y++)
		{
			rpos_handle_t rpos_handle = get_rpos_handle_for_world_xy(x*16, y*16, 0);
			fn(rpos_handle);
		}
	}
}

// Iterates over every rpos in the current region, but only for screens that are valid,
// until execution is requested to stop.
// Callback function: bool fn(const rpos_handle_t& rpos_handle)
// If the callback returns false, the exeuction stops early.
template<typename T>
requires std::is_invocable_v<T, const rpos_handle_t&>
ZC_FORCE_INLINE void for_some_rpos(T&& fn)
{
	auto [handles, count] = z3_get_current_region_handles();

	for (int i = 0; i < count; i++)
	{
		rpos_handle_t rpos_handle = handles[i];
		for (int j = 0; j < 176; j++)
		{
			if (!fn(rpos_handle))
				return;

			rpos_handle.rpos = (rpos_t)((int)rpos_handle.rpos + 1);
			rpos_handle.pos += 1;
		}
	}
}

// Iterates over every ffc in the current region.
// Callback function: void fn(const ffc_handle_t& ffc_handle)
template<typename T>
requires std::is_invocable_v<T, const ffc_handle_t&>
ZC_FORCE_INLINE void for_every_ffc(T&& fn)
{
	auto [handles, count] = z3_get_current_region_handles();
	
	for (int i = 0; i < count; i++)
	{
		if (handles[i].layer != 0)
			continue;

		mapscr* scr = handles[i].scr;
		uint8_t screen = handles[i].screen;
		int screen_index_offset = get_region_screen_offset(screen);
		int c = scr->numFFC();
		for (uint8_t j = 0; j < c; j++)
		{
			uint16_t id = screen_index_offset * MAXFFCS + j;
			ffc_handle_t ffc_handle = {scr, screen, id, j, &scr->ffcs[j]};
			fn(ffc_handle);
		}
	}
}

struct every_combo_opts
{
	bool include_ffcs = !get_qr(qr_OLD_FFC_FUNCTIONALITY);
	bool include_rposes = true;
	bool include_rposes_base_screen_only = false;
};

template<typename T>
requires std::is_invocable_v<T, const rpos_handle_t&> && std::is_invocable_v<T, const ffc_handle_t&>
ZC_FORCE_INLINE void for_every_combo(T&& fn, every_combo_opts opts)
{
	if (opts.include_rposes_base_screen_only)
	{
		for_every_rpos([&](const rpos_handle_t& handle) {
			if (handle.layer == 0)
				fn(handle);
		});
	} else if (opts.include_rposes)
		for_every_rpos(fn);

	if (opts.include_ffcs)
		for_every_ffc(fn);
}

template<typename T>
requires std::is_invocable_v<T, const rpos_handle_t&> && std::is_invocable_v<T, const ffc_handle_t&>
ZC_FORCE_INLINE void for_every_combo(T&& fn, bool include_ffcs = !get_qr(qr_OLD_FFC_FUNCTIONALITY))
{
	for_every_rpos(fn);
	if (include_ffcs)
		for_every_ffc(fn);
}

template<typename T>
requires std::is_invocable_v<T, const rpos_handle_t&> && std::is_invocable_v<T, const ffc_handle_t&>
ZC_FORCE_INLINE void for_every_combo_in_screen(mapscr* scr, T&& fn, bool include_ffcs = !get_qr(qr_OLD_FFC_FUNCTIONALITY))
{
	for_every_rpos_in_screen(scr, fn);
	if (include_ffcs)
		for_every_ffc_in_screen(scr, fn);
}

// Iterates over every ffc in the current region, until execution is requested to stop.
// Callback function: bool fn(const ffc_handle_t& ffc_handle)
// If the callback returns false, the exeuction stops early.
template<typename T>
requires std::is_invocable_v<T, const ffc_handle_t&>
ZC_FORCE_INLINE void for_some_ffcs(T&& fn)
{
	auto [handles, count] = z3_get_current_region_handles();
	
	for (int i = 0; i < count; i++)
	{
		if (handles[i].layer != 0)
			continue;

		mapscr* scr = handles[i].scr;
		uint8_t screen = handles[i].screen;
		int screen_index_offset = get_region_screen_offset(screen);
		int c = scr->numFFC();
		for (uint8_t j = 0; j < c; j++)
		{
			uint16_t id = screen_index_offset * MAXFFCS + j;
			ffc_handle_t ffc_handle = {scr, screen, id, j, &scr->ffcs[j]};
			if (!fn(ffc_handle)) return;
		}
	}
}

// Iterates over every ffc in the current region, and returns a handle to the
// first one for which the callback functions evaluates true for.
// Callback function: bool fn(const ffc_handle_t& ffc_handle)
template<typename T>
requires std::is_invocable_v<T, const ffc_handle_t&>
ZC_FORCE_INLINE std::optional<ffc_handle_t> find_ffc(T&& fn)
{
	auto [handles, count] = z3_get_current_region_handles();
	
	for (int i = 0; i < count; i++)
	{
		if (handles[i].layer != 0)
			continue;

		mapscr* scr = handles[i].scr;
		uint8_t screen = handles[i].screen;
		int screen_index_offset = get_region_screen_offset(screen);
		int c = scr->numFFC();
		for (uint8_t j = 0; j < c; j++)
		{
			if (scr->ffcs[j].data)
			{
				uint16_t id = screen_index_offset * MAXFFCS + j;
				ffc_handle_t ffc_handle = {scr, screen, id, j, &scr->ffcs[j]};
				if (fn(ffc_handle)) return ffc_handle;
			}
		}
	}

	return std::nullopt;
}

// Iterates over every rpos for a specified screen.
// Callback function: void fn(const rpos_handle_t& rpos_handle_t)
template<typename T>
requires std::is_invocable_v<T, const rpos_handle_t&>
ZC_FORCE_INLINE void for_every_rpos_in_screen(mapscr* scr, T&& fn)
{
	auto [handles, count] = z3_get_current_region_handles(scr);

	if (handles != nullptr)
	{
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

		return;
	}

	// Not in current region.

	rpos_handle_t rpos_handle;
	int map = scr->map;
	int screen = scr->screen;
	rpos_handle.screen = screen;
	rpos_t base_rpos = POS_TO_RPOS(0, z3_get_region_relative_dx(screen), z3_get_region_relative_dy(screen));
	for (int lyr = 0; lyr <= 6; ++lyr)
	{
		rpos_handle.scr = lyr == 0 ? scr : get_scr_layer(map, screen, lyr - 1);
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
ZC_FORCE_INLINE void for_every_ffc_in_screen(mapscr* scr, T&& fn)
{
	byte screen = scr->screen;
	int screen_index_offset = get_region_screen_offset(screen);
	int num_ffc = scr->numFFC();
	for (uint8_t i = 0; i < num_ffc; i++)
	{
		uint16_t id = screen_index_offset * MAXFFCS + i;
		ffc_handle_t ffc_handle = {scr, screen, id, i, &scr->ffcs[i]};
		fn(ffc_handle);
	}
}
