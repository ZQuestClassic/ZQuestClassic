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

// Creating rgb_table and trans_table is pretty expensive, so try not to redo the same work
// within a short period of time by using a cache.
struct pal_table_cache_entry {
	RGB_MAP rgb_table;
	COLOR_MAP trans_table;
	COLOR_MAP trans_table2;
};
static std::unordered_map<pal_table_cache_key, std::unique_ptr<pal_table_cache_entry>, PalKeyHasher> pal_table_cache;
static constexpr int pal_table_cache_max_memory_mb = 10;
static constexpr int pal_table_cache_max_size = pal_table_cache_max_memory_mb / ((double)sizeof(pal_table_cache_entry) / 1024 / 1024);

// Direct cache for the most-recently-used entry — avoids the hash map lookup
// on consecutive calls with the same palette (common in steady-state gameplay
// and for the repeated loadlvlpal calls inside the fade loop).
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

	pal_table_cache_entry* entry;

	if (last_entry && key == last_key)
	{
		entry = last_entry;
	}
	else
	{
		if (pal_table_cache.size() > (size_t)pal_table_cache_max_size)
			pal_table_cache.erase(pal_table_cache.begin());

		auto cache_it = pal_table_cache.find(key);
		if (cache_it == pal_table_cache.end())
		{
			auto new_entry = std::make_unique<pal_table_cache_entry>();

			create_rgb_table(&new_entry->rgb_table, RAMpal, NULL);
			rgb_map = &new_entry->rgb_table;

			create_zc_trans_table(&new_entry->trans_table, RAMpal, 128, 128, 128);
			memcpy(&new_entry->trans_table2, &new_entry->trans_table, sizeof(COLOR_MAP));

			cache_it = pal_table_cache.emplace(key, std::move(new_entry)).first;
		}

		entry = cache_it->second.get();
		last_key = key;
		last_entry = entry;
	}

	rgb_table = &entry->rgb_table;
	rgb_map = rgb_table;
	trans_table = &entry->trans_table;
	trans_table2 = &entry->trans_table2;

	for (int i = 0; i < PAL_SIZE; i++)
	{
		trans_table2->data[0][i] = i;
		trans_table2->data[i][i] = i;
	}
}
