#ifndef _ZC_SAVES_H_
#define _ZC_SAVES_H_

#include "gamedata.h"
#include <filesystem>

namespace fs = std::filesystem;

// Non-copyable, moveable.
struct save_t
{
	fs::path path;
	bool write_to_disk = true;
	// If loaded is false, this is null.
	// If loaded is true and `game` is null, this memory is owned by save_t.
	// If loaded is true and `game` is not null, this points to the header field in `game`.
	gamedata_header* header;
	// May be null - we do not keep every save's gamedata in memory at all times.
	gamedata* game;

	save_t() = default;
	save_t(save_t&& other): path(other.path), header(other.header), game(other.game)
	{
		other.path = "";
		other.header = nullptr;
		other.game = nullptr;
	}
	save_t& operator=(save_t&& other)
	{
		std::swap(this->path, other.path);
		std::swap(this->header, other.header);
		std::swap(this->game, other.game);
		return *this;
	}
	save_t(const save_t&) = delete;
	save_t& operator=(const save_t&) = delete;
	~save_t();
	void unload();
};

int32_t saves_init();
int32_t saves_load();
int32_t saves_write();
bool saves_select(int32_t index);
void saves_unselect();
void saves_unload(int32_t index);
int32_t saves_count();
int32_t saves_current_selection();
bool saves_create_slot(gamedata* game, bool write_to_disk = true);
bool saves_create_slot(fs::path path, bool write_to_disk = true);
bool saves_is_slot_loaded(int32_t index, bool full_data = false);
const save_t* saves_get_slot(int32_t index, bool full_data = false);
const save_t* saves_get_current_slot();
void saves_delete(int32_t index);
void saves_copy(int32_t from_index);
int saves_do_first_time_stuff(int index);
void saves_enable_save_current_replay();
bool saves_test();

#endif
