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

#if !defined(_DEBUG) && defined(__GNUC__)
  #define ZC_FORCE_INLINE inline __attribute__((__always_inline__))
#elif !defined(_DEBUG) && defined(_MSC_VER)
  #define ZC_FORCE_INLINE __forceinline
#else
  #define ZC_FORCE_INLINE inline
#endif

// Iterates over every base screen (layer 0) in the current region.
// Callback function: void fn(mapscr* scr, unsigned int region_scr_x, unsigned int region_scr_x)
// region_scr_x and region_scr_y are the screen coordinates relative to the origin screen. For example, the relative
// coordinates for the screen just to the right of the origin screen is (1, 0). This is always (0, 0) when not in a
// region.
template<typename T>
requires std::is_invocable_v<T, mapscr*, unsigned int, unsigned int>
ZC_FORCE_INLINE void for_every_screen_in_region(T&& fn)
{
	if (!is_z3_scrolling_mode())
	{
		fn(tmpscr, 0, 0);
		return;
	}

	auto [handles, count] = z3_get_current_region_handles();

	for (int i = 0; i < count; i++)
	{
		if (handles[i].layer == 0)
		{
			mapscr* scr = handles[i].scr;
			unsigned int screen = handles[i].screen;
			unsigned int region_scr_x = z3_get_region_relative_dx(screen);
			unsigned int region_scr_y = z3_get_region_relative_dy(screen);
			fn(scr, region_scr_x, region_scr_y);
		}
	}
}

// Iterates over every rpos in the current region, but only for screens that are valid.
// Callback function: void fn(const pos_handle_t& rpos_handle)
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
// until execution is requested to stop.
// Callback function: bool fn(const pos_handle_t& rpos_handle)
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
		int screen_index_offset = get_region_screen_index_offset(screen);
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
		int screen_index_offset = get_region_screen_index_offset(screen);
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
		int screen_index_offset = get_region_screen_index_offset(screen);
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
// Callback function: void fn(const pos_handle_t& pos_handle_t)
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

	// Not in this region.

	rpos_handle_t rpos_handle;
	int map = scr->map;
	int screen = scr->screen;
	rpos_handle.screen = screen;
	rpos_t base_rpos = POS_TO_RPOS(0, z3_get_region_relative_dx(screen), z3_get_region_relative_dy(screen));
	for (int lyr = 0; lyr <= 6; ++lyr)
	{
		rpos_handle.scr = lyr == 0 ? scr : get_layer_scr(map, screen, lyr - 1);
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
	int screen_index_offset = get_region_screen_index_offset(screen);
	int num_ffc = scr->numFFC();
	for (uint8_t i = 0; i < num_ffc; i++)
	{
		uint16_t id = screen_index_offset * MAXFFCS + i;
		ffc_handle_t ffc_handle = {scr, screen, id, i, &scr->ffcs[i]};
		fn(ffc_handle);
	}
}
