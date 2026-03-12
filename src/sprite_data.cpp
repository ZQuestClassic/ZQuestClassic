#include "sprite_data.h"
#include "items.h"

void mark_save_dirty();
bounded_vec<word, sprite_data> sprite_data_buf = {MAXSPRITES};

void sprite_data::load_item(itemdata const& itm)
{
	tile = itm.tile;
	misc = itm.misc_flags;
	csets = itm.csets;
	frames = itm.frames;
	speed = itm.speed;
	type = itm.delay;
}


// For deleting / moving quest sprites, updating references to affected sprites
static void update_quest_sprites(std::map<size_t, size_t> changes)
{
	// TODO: This will take a lot of work, and won't help any hardcoded sprite IDs.
	// For now just going to warn that Sprites won't be updated;
	// can come back to this later when more Sprite hardcodes have been removed.
	// -Em
	
	for (auto it = changes.begin(); it != changes.end();) // trim non-changes
	{
		if (it->first == it->second)
			it = changes.erase(it);
		else ++it;
	}
	if (changes.empty())
		return;
	
	mark_save_dirty();
}
void delete_quest_sprites(std::function<bool(sprite_data const&)> proc)
{
	size_t del_count = 0;
	std::map<size_t, size_t> changes;
	auto& cont = sprite_data_buf.mut_inner();
	size_t sz = cont.size();
	auto it = cont.begin();
	for (size_t q = 0; q < sz; ++q)
	{
		if (proc(*it))
		{
			it = cont.erase(it);
			changes[q] = -1;
			++del_count;
		}
		else
		{
			++it;
			if (del_count)
				changes[q] = q - del_count;
		}
	}
	update_quest_sprites(changes);
}
void delete_quest_sprites(size_t idx)
{
	if (unsigned(idx) >= sprite_data_buf.capacity()) return;
	std::map<size_t, size_t> changes;
	auto& cont = sprite_data_buf.mut_inner();
	size_t sz = cont.size();
	auto it = cont.begin();
	for (size_t q = 0; q < sz; ++q)
	{
		if (q == idx)
		{
			it = cont.erase(it);
			changes[q] = -1;
		}
		else
		{
			++it;
			if (q > idx)
				changes[q] = q - 1;
		}
	}
	update_quest_sprites(changes);
}
void swap_quest_sprites(size_t idx1, size_t idx2)
{
	if (unsigned(idx1) >= MAXSPRITES ||
		unsigned(idx2) >= MAXSPRITES ||
		idx1 == idx2) return;
	zc_swap_mv(sprite_data_buf[idx1], sprite_data_buf[idx2]);
	update_quest_sprites({{idx1, idx2},{idx2, idx1}});
}

