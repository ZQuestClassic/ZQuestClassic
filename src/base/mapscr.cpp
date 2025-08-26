#include "base/mapscr.h"

#include "base/general.h"
#include "base/handles.h"
#include "base/qrs.h"
#include "base/util.h"
#include "base/zsys.h"
#include <memory>

std::array<regions_data, MAXMAPS> Regions;
std::vector<mapscr> TheMaps;
std::vector<map_info> map_infos;
word map_count = 0;

byte regions_data::get_region_id(int screen_x, int screen_y) const
{
	return util::nibble(region_ids[screen_y][screen_x/2], screen_x % 2 == 0);
}

byte regions_data::get_region_id(int screen) const
{
	return get_region_id(screen % 16, screen / 16);
}

void regions_data::set_region_id(int screen, byte value)
{
	int screen_x = screen % 16;
	int screen_y = screen / 16;
	byte& datum = region_ids[screen_y][screen_x/2];
	if (screen_x % 2 == 0)
		datum = util::nibble_set_upper_byte(datum, value);
	else
		datum = util::nibble_set_lower_byte(datum, value);
}

bool regions_data::is_same_region(int screen_1, int screen_2) const
{
	auto rid = get_region_id(screen_1);
	return rid && rid == get_region_id(screen_2);
}

void regions_data::for_each_screen(int screen, const std::function<void(int)>& fn) const
{
	int ox, oy, ex, ey;
	determine_region_size(get_all_region_ids(), screen, ox, oy, ex, ey);
	for(int y = oy; y <= ey; ++y)
		for(int x = ox; x <= ex; ++x)
			fn(x + y*0x10);
}

region_ids_t regions_data::get_all_region_ids() const
{
	region_ids_t region_ids{};

	for (int sy = 0; sy < 8; sy++)
	{
		for (int sx = 0; sx < 16; sx++)
		{
			int screen = map_scr_xy_to_index(sx, sy);
			region_ids[screen] = get_region_id(sx, sy);
		}
	}

	return region_ids;
}

void determine_region_size(const region_ids_t& region_ids, int input_screen, int& origin_scr_x, int& origin_scr_y, int& end_scr_x, int& end_scr_y)
{
	// yoink'd the following from zc/maps.cpp calculate_region

	int id = region_ids[input_screen];
	origin_scr_x = input_screen % 16;
	origin_scr_y = input_screen / 16;
	if (id == 0)
	{
		end_scr_x = origin_scr_x;
		end_scr_y = origin_scr_y;
		return;
	}

	// For the given screen, find the top-left corner of its region.
	while (origin_scr_x > 0)
	{
		if (id != region_ids[map_scr_xy_to_index(origin_scr_x - 1, origin_scr_y)]) break;
		origin_scr_x--;
	}
	while (origin_scr_y > 0)
	{
		if (id != region_ids[map_scr_xy_to_index(origin_scr_x, origin_scr_y - 1)]) break;
		origin_scr_y--;
	}

	// Now find the bottom-right corner.
	end_scr_x = origin_scr_x;
	while (end_scr_x < 15)
	{
		if (id != region_ids[map_scr_xy_to_index(end_scr_x + 1, origin_scr_y)]) break;
		end_scr_x++;
	}
	end_scr_y = origin_scr_y;
	while (end_scr_y < 7)
	{
		if (id != region_ids[map_scr_xy_to_index(origin_scr_x, end_scr_y + 1)]) break;
		end_scr_y++;
	}
}

bool get_all_region_descriptions(std::vector<region_description>& result, const region_ids_t& region_ids)
{
	std::array<bool, MAPSCRSNORMAL> seen{};
	for (int i = 0; i < MAPSCRSNORMAL; i++)
	{
		if (seen[i])
			continue;

		int origin_scr_x, origin_scr_y, end_scr_x, end_scr_y;
		determine_region_size(region_ids, i, origin_scr_x, origin_scr_y, end_scr_x, end_scr_y);

		int region_id = region_ids[map_scr_xy_to_index(origin_scr_x, origin_scr_y)];
		if (region_id == 0)
		{
			seen[i] = true;
			continue;
		}

		int w = end_scr_x - origin_scr_x + 1;
		int h = end_scr_y - origin_scr_y + 1;
		result.emplace_back(region_description{region_id, i, w, h});

		// Confirm everything within rectangle is the same region id, and has not been seen yet.
		for (int x = origin_scr_x; x <= end_scr_x; x++)
		{
			for (int y = origin_scr_y; y <= end_scr_y; y++)
			{
				int screen = map_scr_xy_to_index(x, y);
				if (seen[screen])
					return false;
				if (region_ids[screen] != region_id)
					return false;

				seen[screen] = true;
			}
		}

		// Confirm not bordering (in cardinal directions) anything of same region id.
		for (int x = origin_scr_x - 1; x <= end_scr_x + 1; x++)
		{
			for (int y = origin_scr_y - 1; y <= end_scr_y + 1; y++)
			{
				if (x < 0 || y < 0 || x >= 16 || y >= 8)
					continue;

				int count = 0;
				if (x == origin_scr_x - 1 || x == end_scr_x + 1) count++;
				if (y == origin_scr_y - 1 || y == end_scr_y + 1) count++;
				if (count != 1)
					continue;

				int screen = map_scr_xy_to_index(x, y);
				if (region_ids[screen] == region_id)
					return false;
			}
		}
	}

	return true;
}

byte mapscr::ffEffectWidth(size_t ind) const
{
	return (byte)ffcs[ind].hit_width;
}

void mapscr::ffEffectWidth(size_t ind, byte val)
{
	ffcs[ind].hit_width = val;
}

byte mapscr::ffEffectHeight(size_t ind) const
{
	return ffcs[ind].hit_height;
}

void mapscr::ffEffectHeight(size_t ind, byte val)
{
	ffcs[ind].hit_height = val;
}

byte mapscr::ffTileWidth(size_t ind) const
{
	return ffcs[ind].txsz;
}

void mapscr::ffTileWidth(size_t ind, byte val)
{
	ffcs[ind].txsz = val;
}

byte mapscr::ffTileHeight(size_t ind) const
{
	return ffcs[ind].tysz;
}

void mapscr::ffTileHeight(size_t ind, byte val)
{
	ffcs[ind].tysz = val;
}

void mapscr::zero_memory()
{
	*this = mapscr();
}

void mapscr::shrinkToFitFFCs()
{
	int lastffc = -1;
	for (int w = ffcs.size() - 1; w >= 0; --w)
	{
		if (ffcs[w].data)
		{
			lastffc = w;
			break;
		}
	}

	num_ffcs = lastffc + 1;
	if (num_ffcs == ffcs.size())
		return;

	ffcs.resize(num_ffcs);
	ffcs.shrink_to_fit();
	for (int i = 0; i < ffcs.size(); i++)
	{
		ffcs[i].screen_spawned = screen;
		ffcs[i].index = i;
		ffcs[i].setLoaded(true);
	}
}

void mapscr::resizeFFC(size_t size)
{
	if (size > 0)
		getFFC(size - 1);
	else
		ffcs.clear();
}

void mapscr::ensureFFC(size_t ind)
{
	getFFC(ind);
}

ffcdata& mapscr::getFFC(size_t ind)
{
	assert(ind < MAXFFCS);

	if (ind < ffcs.size())
		return ffcs[ind];

	size_t prev = ffcs.size();
	int32_t uids[MAXFFCS];
	for (size_t i = 0; i < prev; i++)
		uids[i] = ffcs[i].uid;

	// ffc_count_dirty does not need to be set because `data` is still zero, so the count
	// won't have changed. Only need to mark it dirty when `data` changes (handled in
	// `screen_ffc_modify_postroutine`).
	ffcs.resize(ind + 1);
	for (size_t i = 0; i < prev; i++)
		if (uids[i]) ffcs[i].reassignUid(uids[i]);
	for (size_t i = prev; i < ffcs.size(); i++)
	{
		ffcs[i].screen_spawned = screen;
		ffcs[i].index = i;
		ffcs[i].setLoaded(true);
	}

	return ffcs[ind];
}

std::unique_ptr<ffc_handle_t> mapscr::getFFCHandle(int index, int screen_index_offset)
{
	ffc_id_t ffc_id = screen_index_offset * MAXFFCS + index;
	ffc_handle_t handle = {this, (uint8_t)screen, ffc_id, (uint8_t)index, &getFFC(index)};
	return std::make_unique<ffc_handle_t>(handle);
}

word mapscr::numFFC()
{
	if (!ffc_count_dirty)
		return num_ffcs;

	int lastffc = -1;
	for (int w = ffcs.size() - 1; w >= 0; --w)
	{
		if (ffcs[w].data)
		{
			lastffc = w;
			break;
		}
	}
	if (lastffc < 31 && get_qr(qr_OLD_FFC_FUNCTIONALITY))
	{
		lastffc = 31;
		ensureFFC(31);
	}
	num_ffcs = lastffc + 1;
	ffc_count_dirty = false;
	return num_ffcs;
}

void mapscr::ffcCountMarkDirty()
{
	ffc_count_dirty = true;
}

int map_screen_index(int map, int screen)
{
	return map*MAPSCRS + screen;
}

int screen_index_direction(int screen, direction dir)
{
	if (dir == left)  screen--;
	if (dir == right) screen++;
	if (dir == up)    screen -= 16;
	if (dir == down)  screen += 16;
	return screen;
}

const mapscr* get_canonical_scr(int map, int screen)
{
	return &TheMaps[map_screen_index(map, screen)];
}

int map_scr_xy_to_index(int x, int y)
{
	DCHECK(x >= 0 && x < 16 && y >= 0 && y < 8);
	return x + y*16;
}
