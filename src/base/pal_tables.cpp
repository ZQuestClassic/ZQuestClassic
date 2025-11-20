#include "base/pal_tables.h"

#include "base/colors.h"
#include "base/zsys.h"

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

void refresh_rgb_tables()
{
	if (pal_table_cache.size() > pal_table_cache_max_size)
		pal_table_cache.erase(pal_table_cache.begin());

	pal_table_cache_key key;
	for (int i = 0; i < PAL_SIZE; i++)
		key[i] = RAMpal[i].r + (RAMpal[i].g << 8) + (RAMpal[i].b << 16);

	auto cache_it = pal_table_cache.find(key);
	if (cache_it == pal_table_cache.end())
	{
		auto new_entry = std::make_unique<pal_table_cache_entry>();

		create_rgb_table(&new_entry->rgb_table, RAMpal, NULL);
		rgb_table = &new_entry->rgb_table;
		rgb_map = rgb_table;

		create_zc_trans_table(&new_entry->trans_table, RAMpal, 128, 128, 128);
		memcpy(&new_entry->trans_table2, &new_entry->trans_table, sizeof(COLOR_MAP));

		trans_table = &new_entry->trans_table;
		trans_table2 = &new_entry->trans_table2;

		pal_table_cache[key] = std::move(new_entry);
	}
	else
	{
		rgb_table = &cache_it->second->rgb_table;
	 rgb_map = rgb_table;
		trans_table = &cache_it->second->trans_table;
		trans_table2 = &cache_it->second->trans_table2;
	}

	for (int i = 0; i < PAL_SIZE; i++)
	{
		trans_table2->data[0][i] = i;
		trans_table2->data[i][i] = i;
	}
}
