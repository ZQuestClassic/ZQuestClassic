#include "zcsfx.h"
#include <allegro5/internal/aintern_audio.h>
#include <zalleg/zalleg.h>

#ifdef IS_PLAYER
#include "base/qrs.h"
#include "zc/ffscript.h"
#endif

vector<ZCSFX> quest_sounds;

size_t get_al_buffer_size(ALLEGRO_CHANNEL_CONF chan_conf, ALLEGRO_AUDIO_DEPTH depth, size_t len)
{
	size_t sample_size = al_get_channel_count(chan_conf) * al_get_audio_depth_size(depth);
	return sample_size * len;
}

static double calc_gain(double vol_perc)
{
#ifdef IS_PLAYER
	double vol_a4 = sfx_volume * vol_perc / 100.0;
	if (GameLoaded && !get_qr(qr_OLD_SCRIPT_VOLUME) && FFCore.usr_sfx_volume != 10000*100)
		vol_a4 *= FFCore.usr_sfx_volume / 1000000.0;
#else
	double vol_a4 = 128 * vol_perc / 100.0;
#endif
	return vol_a4 / 255.0;
}

ZCSFX::ZCSFX(SAMPLE& s)
{
	priority = s.priority;
	loop_start = s.loop_start;
	loop_end = s.loop_end;
	param = s.param;

	sample = nullptr;
	inst = nullptr;

	ALLEGRO_CHANNEL_CONF chan_conf = s.stereo ? ALLEGRO_CHANNEL_CONF_2 : ALLEGRO_CHANNEL_CONF_1;
	ALLEGRO_AUDIO_DEPTH depth = s.bits == 8 ? ALLEGRO_AUDIO_DEPTH_INT8 : ALLEGRO_AUDIO_DEPTH_INT16;
	size_t buffer_len = get_al_buffer_size(chan_conf, depth, s.len);
	uint8_t* data = (uint8_t*)al_malloc(buffer_len);
	if (data)
	{
		uint channels = al_get_channel_count(chan_conf);
		memcpy(data, s.data, buffer_len);
		size_t frames = (s.len * channels);
		if (s.bits == 8)
		{
			for (uint q = 0; q < frames; ++q)
				((int8_t*)data)[q] = (int8_t)(data[q] - 128);
		}
		else
		{
			int16_t *data16 = (int16_t *)data;
			for (uint q = 0; q < frames; ++q)
			{
#if ENDIAN_BE // swap endianness?
				data16[q] = (data16[q] >> 8 | data16[q] << 8);
#endif
				data16[q] = (int16_t)(data16[q] - 32768);
			}
		}
		
		load_sample((void*)data, s.len, s.freq, depth, chan_conf);
	}
	sample_type = SMPL_WAV;
	if (is_invalid())
		sample_type = SMPL_INVALID;
}

ZCSFX::ZCSFX(ZCSFX const& other)
{
	sample = nullptr;
	inst = nullptr;
	*this = other;
}
ZCSFX& ZCSFX::operator=(ZCSFX const& other)
{
	clear_sample();
	sample_type = other.sample_type;
	priority = other.priority;
	loop_start = other.loop_start;
	loop_end = other.loop_end;
	param = other.param;
	sfx_name = other.sfx_name;

	if (other.sample || other.databuf)
	{
		size_t buffer_len = other.get_buffer_size();
		uint8_t* data = (uint8_t*)al_malloc(buffer_len);
		memcpy(data, other.get_sample_data(), buffer_len);
		load_sample(data, other.get_len(), other.get_frequency(),
			other.get_depth(), other.get_chan_conf());
		// don't copy the 'inst'
	}
	if (!(sample || databuf))
		sample_type = SMPL_INVALID;
	return *this;
}
ZCSFX::ZCSFX(ZCSFX&& other)
{
	sample = nullptr;
	inst = nullptr;
	*this = other;
}
ZCSFX& ZCSFX::operator=(ZCSFX&& other)
{
	clear_sample();
	sample_type = other.sample_type;
	priority = other.priority;
	loop_start = other.loop_start;
	loop_end = other.loop_end;
	param = other.param;
	sfx_name = other.sfx_name;

	// Take ownership
	sample = other.sample;
	inst = other.inst;
	databuf = other.databuf;
	other.sample_type = SMPL_INVALID;
	other.sample = nullptr;
	other.inst = nullptr;
	other.databuf.reset();
	return *this;
}

ZCSFX::~ZCSFX()
{
	clear_sample();
}

void ZCSFX::load_sample(void* data, int32_t len, int32_t freq, ALLEGRO_AUDIO_DEPTH depth, ALLEGRO_CHANNEL_CONF chan_conf)
{
	clear_sample();
	if (sound_was_installed)
	{
		sample = al_create_sample((void*)data, len, freq, depth, chan_conf, true);
		if (!sample)
			al_free(data);
	}
	else databuf = SampleDataBuffer((void*)data, len, freq, depth, chan_conf);
}
void ZCSFX::load_sample(ALLEGRO_SAMPLE* new_sample)
{
	if (!sound_was_installed) return;
	clear_sample();
	sample = new_sample;
}
bool ZCSFX::save_sample(char const* filepath) const
{
	if (!sound_was_installed) return false;
	if (!sample) return false;
	return al_save_sample(filepath, sample);
}

bool ZCSFX::is_invalid() const
{
	return sample_type == SMPL_INVALID || !(sample || databuf);
}

size_t ZCSFX::get_buffer_size() const
{
	if (sample)
		return get_al_buffer_size(sample->chan_conf, sample->depth, sample->len);
	if (databuf)
		return get_al_buffer_size(databuf->chan_conf, databuf->depth, databuf->len);
	return 0;
}

uint8_t const* ZCSFX::get_sample_data() const
{
	if (sample)
		return (uint8_t const*)al_get_sample_data(sample);;
	if (databuf)
		return (uint8_t const*)databuf->buf;
	return nullptr;
}

void ZCSFX::clear()
{
	*this = ZCSFX();
}
void ZCSFX::clear_sample() // clears any 'owned' memory
{
	if (inst)
		al_destroy_sample_instance(inst);
	if (sample)
		al_destroy_sample(sample);
	if (databuf)
	{
		if (databuf->buf)
			al_free(databuf->buf);
		databuf->buf = nullptr;
	}
	inst = nullptr;
	sample = nullptr;
	databuf.reset();
}

bool ZCSFX::play(int pan, bool loop, bool restart, zfix vol_perc, int freq)
{
	if (!sound_was_installed) return false;
	if (is_invalid())
	{
		if (inst)
			stop();
		return false;
	}
	if (inst)
	{
		if (al_get_sample(inst) != sample)
		{
			al_destroy_sample_instance(inst);
			inst = nullptr;
		}
		else if (restart)
			al_set_sample_instance_position(inst, 0); // restart the sound
	}
	if (!inst)
	{
		inst = al_create_sample_instance(sample);
		if (!inst || !al_attach_sample_instance_to_mixer(inst, al_get_default_mixer()))
		{
			stop();
			return false;
		}
	}
	
	update_pan(pan);
	update_loop(loop);
	update_freq(freq);
	update_vol_perc(vol_perc);
	al_play_sample_instance(inst);
	return true;
}
void ZCSFX::pause()
{
	if (inst)
		al_stop_sample_instance(inst);
}
void ZCSFX::resume()
{
	if (inst)
		al_play_sample_instance(inst);
}
void ZCSFX::stop()
{
	if (inst)
	{
		al_destroy_sample_instance(inst);
		inst = nullptr;
	}
}
void ZCSFX::cleanup()
{
	if (is_allocated() && !is_playing() && get_pos() <= 0)
		stop();
}

void ZCSFX::update_pan(int pan)
{
	if (inst)
		al_set_sample_instance_pan(inst, pan / 255.0);
}
void ZCSFX::update_loop(bool loop)
{
	if (inst)
		al_set_sample_instance_playmode(inst, loop ? ALLEGRO_PLAYMODE_LOOP : ALLEGRO_PLAYMODE_ONCE);
}
void ZCSFX::update_freq(int freq)
{
	if (inst)
	{
		if (freq < 0)
			freq = al_get_sample_frequency(sample);
		inst->spl_data.frequency = freq;
	}
}
void ZCSFX::update_vol_perc(optional<zfix> vol_perc)
{
	if (vol_perc)
		base_vol = *vol_perc;
	update_gain(calc_gain(base_vol));
}
void ZCSFX::update_gain(double gain)
{
	if (inst)
		al_set_sample_instance_gain(inst, gain);
}

bool ZCSFX::is_allocated() const
{
	return inst != nullptr;
}
bool ZCSFX::is_playing() const
{
	return inst && al_get_sample_instance_playing(inst);
}

size_t ZCSFX::get_len_frames() const
{
	size_t freq = get_frequency();
	if (!freq) return 0;
	return 60 * get_len() / freq;
}
size_t ZCSFX::get_pos() const
{
	if (!sample || !is_playing()) return 0;
	return al_get_sample_instance_position(inst);
}
size_t ZCSFX::get_len() const
{
	if (sample)
		return al_get_sample_length(sample);
	if (databuf)
		return databuf->len;
	return 0;
}
size_t ZCSFX::get_frequency() const
{
	if (sample)
		return al_get_sample_frequency(sample);
	if (databuf)
		return databuf->frequency;
	return 0;
}
ALLEGRO_CHANNEL_CONF ZCSFX::get_chan_conf() const
{
	if (!sample) return (ALLEGRO_CHANNEL_CONF)0;
	return al_get_sample_channels(sample);
}
ALLEGRO_AUDIO_DEPTH ZCSFX::get_depth() const
{
	if (!sample) return (ALLEGRO_AUDIO_DEPTH)0;
	return al_get_sample_depth(sample);
}



// returns number of voices currently allocated
int32_t sfx_count()
{
	if (!sound_was_installed)
		return 0;
	int32_t c = 0;
	
	for (auto& sound : quest_sounds)
		if (sound.is_allocated())
			++c;
			
	return c;
}

// clean up finished samples
void sfx_cleanup()
{
	if (!sound_was_installed)
		return;
	for (auto& sound : quest_sounds)
		sound.cleanup();
}

// plays an sfx sample
void sfx(int32_t index, int32_t pan, bool loop, bool restart, zfix vol_perc, int32_t freq)
{
	if (unsigned(index-1) < quest_sounds.size())
	{
		if (sound_was_installed)
		{
			ZCSFX& s = quest_sounds[index-1];
			
			s.play(pan, loop, vol_perc, restart, freq);
		}
	}
	
#ifdef IS_PLAYER
	if (restart && replay_is_debug())
	{
		string sfx_name;
		if (unsigned(index-1) < quest_sounds.size())
			sfx_name = quest_sounds[index-1].sfx_name;
		else if (index > 0 && index < 256 && replay_version_check(0, 49))
			sfx_name = fmt::format("s{:03}", index);
		else return; // no comment for out-of-bounds sfx
		
		// TODO(replays): get rid of this bandaid next time replays are mass-updated.
		if (sfx_name == "Hero is hit")
			sfx_name = "Player is hit";
		else if (sfx_name == "Hero dies")
			sfx_name = "Player dies";
		replay_step_comment(fmt::format("sfx {}", sfx_name));
	}
#endif
}

bool sfx_is_allocated(int32_t index)
{
	if (!sound_was_installed)
		return false;
	if (unsigned(index-1) >= quest_sounds.size())
		return false;
	return quest_sounds[index-1].is_allocated();
}

// play in loop mode
void cont_sfx(int32_t index)
{
	if (!sound_was_installed)
		return;
	sfx(index, 128, true, false);
}

void adjust_sfx(int32_t index,int32_t pan,bool loop)
{
	if (!sound_was_installed)
		return;
	if (unsigned(index-1) >= quest_sounds.size())
		return;
	if (is_headless())
		return;
	ZCSFX& sound = quest_sounds[index-1];
	sound.update_loop(loop);
	sound.update_pan(pan);
}

void adjust_sfx_vol(int32_t index, optional<zfix> vol_perc)
{
	if (!sound_was_installed)
		return;
	if (unsigned(index-1) >= quest_sounds.size())
		return;
	if (is_headless())
		return;
	ZCSFX& sound = quest_sounds[index-1];
	sound.update_vol_perc(vol_perc);
}
void adjust_all_sfx_vol(optional<zfix> vol_perc)
{
	if (!sound_was_installed)
		return;
	for (uint q = 0; q < quest_sounds.size(); ++q)
		adjust_sfx_vol(q+1, vol_perc);
}

void pause_sfx(int32_t index)
{
	if (!sound_was_installed)
		return;
	if (is_headless())
		return;
	if (unsigned(index-1) >= quest_sounds.size())
		return;
	quest_sounds[index-1].pause();
}

void resume_sfx(int32_t index)
{
	if (!sound_was_installed)
		return;
	if (is_headless())
		return;
	if (unsigned(index-1) >= quest_sounds.size())
		return;
	quest_sounds[index-1].resume();
}

void stop_sfx(int32_t index)
{
	if (!sound_was_installed)
		return;
	if (is_headless())
		return;
	if (unsigned(index-1) >= quest_sounds.size())
		return;
	quest_sounds[index-1].stop();
}

void pause_all_sfx()
{
	if (!sound_was_installed)
		return;
	for (auto& sound : quest_sounds)
		sound.pause();
}

void resume_all_sfx()
{
	if (!sound_was_installed)
		return;
	for (auto& sound : quest_sounds)
		sound.resume();
}

void kill_sfx()
{
	if (!sound_was_installed)
		return;
	for (auto& sound : quest_sounds)
		sound.stop();
}

void kill_sfx_except(std::set<size_t> const& skip_idxs)
{
	if (!sound_was_installed)
		return;
	for (size_t idx = 0; idx < quest_sounds.size(); ++idx)
	{
		if (skip_idxs.contains(idx))
			continue;
		quest_sounds[idx].stop();
	}
}


// For deleting / moving quest sounds, updating references to affected sounds
static void update_quest_sounds(std::map<size_t, size_t> changes)
{
	// TODO: This will take a lot of work, and won't help any hardcoded sfx.
	// For now just going to warn that SFX won't be updated;
	// can come back to this later when more SFX hardcodes have been removed.
	// -Em
	
	// for (auto it = changes.begin(); it != changes.end();) // trim non-changes
	// {
		// if (it->first == it->second)
			// it = changes.erase(it);
		// else ++it;
	// }
	// if (changes.empty())
		// return;
	
	// mark_save_dirty();
}
void delete_quest_sounds(std::function<bool(ZCSFX const&)> proc)
{
	size_t del_count = 0;
	std::map<size_t, size_t> changes;
	size_t sz = quest_sounds.size();
	auto it = quest_sounds.begin();
	for (size_t q = 1; q <= sz; ++q)
	{
		if (proc(*it))
		{
			it = quest_sounds.erase(it);
			changes[q] = 0;
			++del_count;
		}
		else
		{
			++it;
			if (del_count)
				changes[q] = q - del_count;
		}
	}
	update_quest_sounds(changes);
}
void delete_quest_sounds(size_t idx)
{
	if (idx <= 0) return;
	std::map<size_t, size_t> changes;
	size_t sz = quest_sounds.size();
	auto it = quest_sounds.begin();
	for (size_t q = 1; q <= sz; ++q)
	{
		if (q == idx)
		{
			it = quest_sounds.erase(it);
			changes[q] = 0;
		}
		else
		{
			++it;
			if (q > idx)
				changes[q] = q - 1;
		}
	}
	update_quest_sounds(changes);
}
void swap_quest_sounds(size_t idx1, size_t idx2)
{
	if (unsigned(idx1-1) > quest_sounds.size() ||
		unsigned(idx2-1) > quest_sounds.size() ||
		idx1 == idx2) return;
	zc_swap_mv(quest_sounds[idx1-1], quest_sounds[idx2-1]);
	update_quest_sounds({{idx1, idx2},{idx2, idx1}});
}

