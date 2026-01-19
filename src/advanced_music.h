#pragma once

#include "base/headers.h"
#include "base/packfile.h"
#include <set>

#define MAX_QUEST_MUSIC 65535

enum
{
	BUILTIN_MIDI_MIN = -7,
	BUILTIN_MIDI_TRIFORCE = -7,
	BUILTIN_MIDI_TITLE = -6,
	BUILTIN_MIDI_OVERWORLD = -5,
	BUILTIN_MIDI_LEVEL9 = -4,
	BUILTIN_MIDI_GAMEOVER = -3,
	BUILTIN_MIDI_ENDING = -2,
	BUILTIN_MIDI_DUNGEON = -1,
	BUILTIN_MIDI_NONE = 0,
};

struct EnhancedMusic
{
	string path;
	int loop_start, loop_end;
	int xfade_in, xfade_out;
	byte track;
	
	bool is_empty() const;
	void clear();
	bool is_playing() const;
	bool play() const;
	
	int32_t read(PACKFILE *f, word s_version);
	int32_t write(PACKFILE *f) const;
};

struct AdvancedMusic
{
	size_t id;
	string name;
	EnhancedMusic enhanced;
	int16_t midi;
	
	bool is_empty() const;
	void clear();
	void play() const;
	
	int get_real_midi() const;
	
	bool is_playing() const;
	
	int32_t read(PACKFILE *f, word s_version);
	int32_t write(PACKFILE *f) const;
};

extern vector<AdvancedMusic> quest_music;
extern bool engine_music_active;
extern size_t active_music_index;

void clear_current_music_data();

int convert_from_old_midi_id(int midi, bool for_screen = false);
int convert_to_old_midi_id(int midi, bool for_screen = false);
int32_t find_or_make_midi_music(int16_t midi);
int32_t find_or_make_dmap_music(int16_t dmap);
void delete_quest_music(std::function<bool(AdvancedMusic const&)> proc);
void delete_quest_music(size_t idx);
void swap_quest_music(size_t idx1, size_t idx2);

