#include "zalleg/pal_tables.h"

#include "zalleg/colors.h"
#include "zalleg/zsys.h"

#include <functional>
#include <memory>
#include <unordered_map>

PALETTE RAMpal;
RGB_MAP* rgb_table;
COLOR_MAP* trans_table;
COLOR_MAP* trans_table2;

using pal_table_cache_key = std::array<uint32_t, PAL_SIZE>;

struct PalKeyHasher {
    std::size_t operator()(const pal_table_cache_key& k) const {
        std::size_t h = 0;
        // Use a simple combining hash (boost::hash_combine).
        for (auto e : k) {
            h ^= std::hash<uint32_t>{}(e) + 0x9e3779b9 + (h << 6) + (h >> 2);
        }
        return h;
    }
};

// Creating rgb_table is pretty expensive (~1ms), so try not to redo the same
// work by using a cache. The trans tables are derived from rgb_table with a
// cheap pass (~70us), so they are not cached per-palette: they are rebuilt
// into the global buffers below whenever the active palette changes. This
// keeps cache entries small (32KB), which matters because palette cycling can
// produce a few hundred distinct palettes in a single level (the three cycles
// advance at different speeds, so the combined states multiply). If the cache
// is smaller than that working set, the cyclic access pattern evicts entries
// right before they are needed again and every call pays the full rebuild.
struct pal_table_cache_entry {
	RGB_MAP rgb_table;
};
static std::unordered_map<pal_table_cache_key, std::unique_ptr<pal_table_cache_entry>, PalKeyHasher> pal_table_cache;
static constexpr int pal_table_cache_max_memory_mb = 20;
static constexpr int pal_table_cache_max_size = pal_table_cache_max_memory_mb / ((double)sizeof(pal_table_cache_entry) / 1024 / 1024);

static COLOR_MAP trans_table_buf;
static COLOR_MAP trans_table2_buf;

// Direct cache for the most-recently-used entry — avoids the hash map lookup
// and the trans table rebuild on consecutive calls with the same palette
// (common in steady-state gameplay and for the repeated loadlvlpal calls
// inside the fade loop).
static pal_table_cache_key last_key;
static pal_table_cache_entry* last_entry;

void refresh_rgb_tables()
{
	// Key on 6-bit values (>> 2) because both create_rgb_table and
	// create_zc_trans_table divide components by 4 before any computation.
	// Palettes that differ only within a 4-unit 8-bit band produce identical
	// tables, so keying on 6-bit values gives correct cache hits for small
	// palette changes such as fine-grained fades.
	pal_table_cache_key key;
	for (int i = 0; i < PAL_SIZE; i++)
		key[i] = (RAMpal[i].r >> 2) | (uint32_t(RAMpal[i].g >> 2) << 8) | (uint32_t(RAMpal[i].b >> 2) << 16);

	// Palette unchanged: every table is already current. Still re-assert the
	// global pointers, since callers (e.g. the editor's tile code) repoint
	// rgb_map temporarily and rely on this call to restore it.
	if (last_entry && key == last_key)
	{
		rgb_table = &last_entry->rgb_table;
		rgb_map = rgb_table;
		trans_table = &trans_table_buf;
		trans_table2 = &trans_table2_buf;
		return;
	}

	if (pal_table_cache.size() > (size_t)pal_table_cache_max_size)
		pal_table_cache.erase(pal_table_cache.begin());

	auto cache_it = pal_table_cache.find(key);
	if (cache_it == pal_table_cache.end())
	{
		auto new_entry = std::make_unique<pal_table_cache_entry>();
		create_rgb_table(&new_entry->rgb_table, RAMpal, NULL);
		cache_it = pal_table_cache.emplace(key, std::move(new_entry)).first;
	}

	pal_table_cache_entry* entry = cache_it->second.get();
	last_key = key;
	last_entry = entry;

	rgb_table = &entry->rgb_table;
	rgb_map = rgb_table;

	// Derive the trans tables for the new palette (uses rgb_map, so this must
	// come after it is set above).
	create_zc_trans_table(&trans_table_buf, RAMpal, 128, 128, 128);
	memcpy(&trans_table2_buf, &trans_table_buf, sizeof(COLOR_MAP));
	for (int i = 0; i < PAL_SIZE; i++)
	{
		trans_table2_buf.data[0][i] = i;
		trans_table2_buf.data[i][i] = i;
	}
	trans_table = &trans_table_buf;
	trans_table2 = &trans_table2_buf;
}
