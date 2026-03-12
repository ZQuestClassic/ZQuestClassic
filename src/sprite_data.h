#pragma once

#include "base/containers.h"

struct itemdata;

struct sprite_data
{
	std::string name;
	int32_t tile;
	byte misc;         // 000bvhff (vh:flipping, f:flash (1:NES, 2:BSZ))
	byte csets;        // ffffcccc (f:flash cset, c:cset)
	byte frames;       // animation frame count
	byte speed;        // animation speed
	byte type;         // used by certain weapons
	word script;
	
	byte flip() const
	{
		return (misc >> 2) & 0b11;
	}
	byte cs() const
	{
		return csets & 0xF;
	}
	word total_duration(bool min1 = true) const
	{
		if (min1)
			return zc_max(1, frames) * zc_max(1, speed);
		return frames * speed;
	}
	void clear()
	{
		*this = sprite_data();
	}
	void load_item(itemdata const& itm);
	bool operator==(sprite_data const& other) const = default;
};

extern bounded_vec<word, sprite_data> sprite_data_buf;

void delete_quest_sprites(std::function<bool(itemdata const&)> proc);
void delete_quest_sprites(size_t idx);
void swap_quest_sprites(size_t idx1, size_t idx2);

