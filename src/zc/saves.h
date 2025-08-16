#ifndef ZC_SAVES_H_
#define ZC_SAVES_H_

#include "gamedata.h"
#include "base/expected.h"
#include <filesystem>

namespace fs = std::filesystem;

// Non-copyable, moveable.
struct save_t
{
	uint16_t index;
	// If >0, there was an error when reading this save file. This is the mtime of the file at that time.
	time_t error_time = 0;
	bool did_error = false;
	fs::path path;
	bool write_to_disk = true;
	// If loaded is false, this is null.
	// If loaded is true and `game` is null, this memory is owned by save_t.
	// If loaded is true and `game` is not null, this points to the header field in `game`.
	gamedata_header* header;
	// May be null - we do not keep every save's gamedata in memory at all times.
	gamedata* game;

	save_t() = default;
	save_t(save_t&& other) noexcept
	  : index(other.index),
		error_time(other.error_time),
		did_error(other.did_error),
		path(std::move(other.path)),
		write_to_disk(other.write_to_disk),
		header(other.header),
		game(other.game)
	{
		other.header = nullptr;
		other.game = nullptr;
	}
	save_t& operator=(save_t&& other)
	{
		if (this != &other)
		{
			std::swap(this->index, other.index);
			std::swap(this->error_time, other.error_time);
			std::swap(this->did_error, other.did_error);
			std::swap(this->path, other.path);
			std::swap(this->write_to_disk, other.write_to_disk);
			std::swap(this->header, other.header);
			std::swap(this->game, other.game);
		}
		return *this;
	}
	save_t(const save_t&) = delete;
	save_t& operator=(const save_t&) = delete;
	~save_t();
	void unload();
};

void saves_init();
bool saves_load(std::string& err);
bool saves_write();
expected<bool, std::string> saves_select(save_t* save);
expected<save_t*, std::string> saves_select(int32_t index);
void saves_unselect();
void saves_unload(int32_t index);
int32_t saves_count();
int32_t saves_current_selection();
std::string saves_current_path();
int saves_find_index(std::string path);
expected<save_t*, std::string> saves_create_slot(gamedata* game, fs::path path = "", bool write_to_disk = true);
expected<save_t*, std::string> saves_create_slot(fs::path path, bool write_to_disk = true);
expected<bool, std::string> saves_update_slot(save_t* save, std::string qstpath);
save_t* saves_create_test_slot(gamedata* game, fs::path path);
bool saves_is_slot_loaded(int32_t index, bool full_data = false);
bool saves_has_error(int32_t index);
expected<save_t*, std::string> saves_get_slot(int32_t index, bool full_data = false);
bool saves_is_valid_slot(int index);
bool saves_delete(int32_t index, std::string& err);
bool saves_copy(int32_t from_index, std::string& err);
void saves_enable_save_current_replay();
bool saves_test();

#endif
