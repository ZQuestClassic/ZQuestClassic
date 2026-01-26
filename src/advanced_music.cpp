#include "advanced_music.h"
#include "music_playback.h"
#include "play_midi.h"
#include "sound/zcmixer.h"
#include "base/qst.h"
#include "base/dmap.h"

void mark_save_dirty();
extern ZCMUSIC* zcmusic;
extern ZCMIXER* zcmixer;

#ifdef IS_PLAYER

#include "zc/zelda.h"
#include "zc/zc_sys.h"

#define QST_PATH qstpath

#else

#include "zq/zquest.h"

#define QST_PATH filepath

extern int32_t midi_volume;
int get_emusic_volume()
{
	return midi_volume;
}

#endif

vector<AdvancedMusic> quest_music;
bool engine_music_active = false;
size_t active_music_index = 0;

void music_stop()
{
	zcmusic_stop(zcmusic);
	zcmusic_unload_file(zcmusic);
	if (zcmixer->oldtrack)
	{
		zcmusic_stop(zcmixer->oldtrack);
		zcmusic_unload_file(zcmixer->oldtrack);
	}
	zcmixer->newtrack = NULL;
	zc_stop_midi();
#ifdef IS_PLAYER
	currmidi = -1;
#endif
	clear_current_music_data();
}

bool EnhancedMusic::is_empty() const
{
	return path.empty();
}
void EnhancedMusic::clear()
{
	*this = EnhancedMusic();
}
bool EnhancedMusic::is_playing() const
{
	if (zcmusic==NULL)
		return false;
	if (strcmp(zcmusic->filename, path.c_str()) != 0)
		return false;
	switch (zcmusic->type)
	{
		case ZCMF_DUH:
		case ZCMF_GME:
			if (zcmusic->track != track)
				return false;
			break;
	}
	return true;
}
bool EnhancedMusic::play() const
{
	if (is_empty())
		return false;
	if (is_playing())
	{
		// these are not checked, so ensure they are updated
		zcmusic->fadeoutframes = xfade_out;
		zcmusic_set_loop(zcmusic, loop_start / 10000.0, loop_end / 10000.0);
		return true;
	}
	int fadeoutframes = 0;
	if (zcmusic)
		fadeoutframes = zcmusic->fadeoutframes;
	if (xfade_in > 0 || fadeoutframes > 0)
	{
		if (!play_enh_music_crossfade(path.c_str(), QST_PATH, track, get_emusic_volume(), xfade_in, fadeoutframes))
			return false;
		
		if (zcmusic)
		{
			zcmusic->fadeoutframes = xfade_out;
			zcmusic_set_loop(zcmusic, loop_start / 10000.0, loop_end / 10000.0);
		}
	}
	else
	{
		if (zcmusic != NULL)
		{
			zcmusic_stop(zcmusic);
			zcmusic_unload_file(zcmusic);
			zcmusic = NULL;
			zcmixer->newtrack = NULL;
		}
		
		zcmusic = zcmusic_load_for_quest(path.c_str(), QST_PATH).first;
		zcmixer->newtrack = zcmusic;

		if (!zcmusic)
			return false;
		zc_stop_midi();
		zcmusic_play(zcmusic, get_emusic_volume());
		int32_t temptracks = zcmusic_get_tracks(zcmusic);
		if (temptracks < 1) temptracks = 1;
		zcmusic_change_track(zcmusic, vbound(track, 0, temptracks - 1));
		zcmusic_set_loop(zcmusic, loop_start / 10000.0, loop_end / 10000.0);
	}
	return true;
}

int32_t EnhancedMusic::read(PACKFILE *f, word s_version)
{
	clear();
	if (!p_getcstr(&path, f))
		return qe_invalid;
	
	if (path.empty()) return 0;
	
	if (!p_igetl(&loop_start, f))
		return qe_invalid;
	if (!p_igetl(&loop_end, f))
		return qe_invalid;
	if (!p_igetl(&xfade_in, f))
		return qe_invalid;
	if (!p_igetl(&xfade_out, f))
		return qe_invalid;
	if (!p_getc(&track, f))
		return qe_invalid;
	return 0;
}
int32_t EnhancedMusic::write(PACKFILE *f) const
{
	if (!p_putcstr(path, f))
		new_return(1);
	
	if (path.empty()) return 0;
	
	if (!p_iputl(loop_start, f))
		new_return(2);
	if (!p_iputl(loop_end, f))
		new_return(3);
	if (!p_iputl(xfade_in, f))
		new_return(4);
	if (!p_iputl(xfade_out, f))
		new_return(5);
	if (!p_putc(track, f))
		new_return(6);
	return 0;
}


bool AdvancedMusic::is_empty() const
{
	return enhanced.is_empty() && !midi;
}
void AdvancedMusic::clear()
{
	*this = AdvancedMusic();
}
void AdvancedMusic::play() const
{
	if (enhanced.play())
	{
		engine_music_active = false;
		active_music_index = id;
		return;
	}
	
	int fadeoutframes = 0;
	if (zcmusic)
		fadeoutframes = zcmusic->fadeoutframes;
	
	int m = get_real_midi();
	if (m < 0)
	{
		if (fadeoutframes > 0 && zcmusic)
			play_enh_music_crossfade(NULL, QST_PATH, enhanced.track, get_emusic_volume(), enhanced.xfade_in, fadeoutframes);
		else music_stop();
	}
	else
	{
#ifdef IS_PLAYER
		jukebox(m);
#else
		if (m >= ZC_MIDI_COUNT)
		{
			m -= ZC_MIDI_COUNT;
			if (unsigned(m) < MAXCUSTOMMIDIS)
				zc_play_midi((MIDI*)customtunes[m].data, true);
		}
#endif
	}
	engine_music_active = false;
	active_music_index = id;
}
int AdvancedMusic::get_real_midi() const
{
	if (midi >= BUILTIN_MIDI_MIN && midi < BUILTIN_MIDI_NONE)
		return -(midi + 1);
	else if (midi <= 0)
		return -1;
	return vbound(midi + MIDIOFFSET_ZSCRIPT, MAXMIDIS-1, 0);
}

bool AdvancedMusic::is_playing() const
{
	return active_music_index == id;
}

int32_t AdvancedMusic::read(PACKFILE *f, word s_version)
{
	clear();
	if (!p_getwstr(&name, f))
		return qe_invalid;
	if (auto ret = enhanced.read(f, s_version))
		return ret;
	if (!p_igetw(&midi, f))
		return qe_invalid;
	return 0;
}
int32_t AdvancedMusic::write(PACKFILE *f) const
{
	if (!p_putwstr(name, f))
		new_return(1);
	if (auto ret = enhanced.write(f))
		return ret;
	if (!p_iputw(midi, f))
		new_return(2);
	return 0;
}

void clear_current_music_data()
{
	engine_music_active = false;
	active_music_index = 0;
}

// for old screen_midi values
const int MIDI_SCREEN_INHERIT = -200000;
int convert_from_old_midi_id(int midi, bool for_screen)
{
	switch (midi)
	{
		case -3:
			return 0;
		case -4:
			if (for_screen)
				return MIDI_SCREEN_INHERIT; // "use dmap's music"
			return BUILTIN_MIDI_GAMEOVER; // for save menus
		case -2:
			return BUILTIN_MIDI_OVERWORLD;
		case -1:
			return BUILTIN_MIDI_DUNGEON;
		case 0:
			return BUILTIN_MIDI_LEVEL9;
		default:
			if (midi < 0)
				return 0;
			return midi;
	}
}
int convert_to_old_midi_id(int midi, bool for_screen)
{
	if (for_screen && midi == MIDI_SCREEN_INHERIT)
		return -1;
	switch (midi)
	{
		case 0:
			return -3;
		case BUILTIN_MIDI_GAMEOVER:
			return -4;
		case BUILTIN_MIDI_OVERWORLD:
			return -2;
		case BUILTIN_MIDI_DUNGEON:
			return -1;
		case BUILTIN_MIDI_LEVEL9:
			return 0;
		default:
			if (midi < 0)
				return 0;
			return midi;
	}
}
int32_t find_or_make_midi_music(int16_t midi)
{
	if (midi == MIDI_SCREEN_INHERIT)
		return -1;
	if (midi == 0)
		return 0;
	string name = fmt::format("MIDI {}", midi);
	for (word q = 0; q < quest_music.size(); ++q)
	{
		auto const& amus = quest_music[q];
		if (amus.enhanced.is_empty() && amus.midi == midi
			&& amus.name == name)
			return q+1;
	}
	if (quest_music.size() >= MAX_QUEST_MUSIC)
		return 0; // failed, extremely highly unlikely to ever happen
	auto& m = quest_music.emplace_back();
	m.midi = midi;
	m.name = name;
	return (m.id = quest_music.size());
}
// for old dmap values
int32_t find_or_make_dmap_music(int16_t dmap)
{
	if (unsigned(dmap) < MAXDMAPS && DMaps[dmap].music)
		return DMaps[dmap].music;
	string name = fmt::format("DMap {}", dmap);
	for (word q = 0; q < quest_music.size(); ++q)
	{
		auto const& amus = quest_music[q];
		if (amus.name == name)
			return q+1;
	}
	if (quest_music.size() >= MAX_QUEST_MUSIC)
		return 0; // failed, extremely highly unlikely to ever happen
	auto& m = quest_music.emplace_back();
	m.name = name;
	return (DMaps[dmap].music = (m.id = quest_music.size()));
}

// For deleting / moving quest music, updating references to affected musics
static void update_quest_music(std::map<size_t, size_t> changes)
{
	for (auto it = changes.begin(); it != changes.end();) // trim non-changes
	{
		if (it->first == it->second)
			it = changes.erase(it);
		else ++it;
	}
	if (changes.empty())
		return;
	
	for (int q = 0; q < MAXDMAPS; ++q)
	{
		if (auto m = DMaps[q].music; m > 0)
		{
			if (changes.contains(m))
				DMaps[q].music = changes[m];
		}
	}
	for (int mi = 0; mi < map_count*MAPSCRS;  ++mi)
	{
		if (auto m = TheMaps[mi].music; m > 0)
		{
			if (changes.contains(m))
				TheMaps[mi].music = changes[m];
		}
	}
	mark_save_dirty();
}
void delete_quest_music(std::function<bool(AdvancedMusic const&)> proc)
{
	size_t del_count = 0;
	std::map<size_t, size_t> changes;
	size_t sz = quest_music.size();
	auto it = quest_music.begin();
	for (size_t q = 1; q <= sz; ++q)
	{
		if (proc(*it))
		{
			it = quest_music.erase(it);
			changes[q] = 0;
			++del_count;
		}
		else
		{
			++it;
			if (del_count)
				changes[q] = (it->id = q - del_count);
		}
	}
	update_quest_music(changes);
}
void delete_quest_music(size_t idx)
{
	if (idx <= 0) return;
	std::map<size_t, size_t> changes;
	size_t sz = quest_music.size();
	auto it = quest_music.begin();
	for (size_t q = 1; q <= sz; ++q)
	{
		if (q == idx)
		{
			it = quest_music.erase(it);
			changes[q] = 0;
		}
		else
		{
			++it;
			if (q > idx)
				changes[q] = (it->id = q - 1);
		}
	}
	update_quest_music(changes);
}
void swap_quest_music(size_t idx1, size_t idx2)
{
	if (unsigned(idx1-1) > quest_music.size() ||
		unsigned(idx2-1) > quest_music.size() ||
		idx1 == idx2) return;
	zc_swap(quest_music[idx1-1], quest_music[idx2-1]);
	quest_music[idx1-1].id = idx1;
	quest_music[idx2-1].id = idx2;
	update_quest_music({{idx1, idx2},{idx2, idx1}});
}

