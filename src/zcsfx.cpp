#include "zcsfx.h"
#include <allegro5/internal/aintern_audio.h>

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
	if (GameLoaded && !get_qr(qr_OLD_SCRIPT_VOLUME))
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
				data[q] = (int8_t)(data[q] - 128);
		}
		else
		{
			uint16_t *data16 = (uint16_t *)data;
			for (uint q = 0; q < frames; ++q)
			{
#if ENDIAN_BE // swap endianness?
				data16[q] = (data16[q] >> 8 | data16[q] << 8);
#endif
				data16[q] = (int16_t)(data16[q] - 32768);
			}
		}
		//memcpy(data, s.data, buffer_len);
		sample = al_create_sample((void*)data, s.len, s.freq, depth, chan_conf, true);
		if (!sample)
			al_free(data);
	}
	type = sample ? SMPL_WAV : SMPL_INVALID;
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
	type = other.type;
	priority = other.priority;
	loop_start = other.loop_start;
	loop_end = other.loop_end;
	param = other.param;
	sfx_name = other.sfx_name;

	if (other.sample)
	{
		size_t buffer_len = other.get_buffer_size();
		uint8_t* data = (uint8_t*)al_malloc(buffer_len);
		memcpy(data, other.get_sample_data(), buffer_len);
		sample = al_create_sample((void*)data, other.sample->len,
			other.sample->frequency, other.sample->depth, other.sample->chan_conf, true);
		if (!sample)
			al_free(data);
	}
	// don't copy the 'inst'
	if (!sample)
		type = SMPL_INVALID;
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
	type = other.type;
	priority = other.priority;
	loop_start = other.loop_start;
	loop_end = other.loop_end;
	param = other.param;
	sfx_name = other.sfx_name;

	// Take ownership
	sample = other.sample;
	inst = other.inst;
	other.type = SMPL_INVALID;
	other.sample = nullptr;
	other.inst = nullptr;
	return *this;
}

ZCSFX::~ZCSFX()
{
	clear_sample();
}

bool ZCSFX::is_invalid() const
{
	return type == SMPL_INVALID || !sample;
}

size_t ZCSFX::get_buffer_size() const
{
	if (is_invalid())
		return 0;
	return get_al_buffer_size(sample->chan_conf, sample->depth, sample->len);
}

uint8_t const* ZCSFX::get_sample_data() const
{
	if (is_invalid())
		return nullptr;
	return (uint8_t const*)al_get_sample_data(sample);
}

void ZCSFX::clear()
{
	*this = ZCSFX();
}
void ZCSFX::clear_sample()
{
	if (inst)
		al_destroy_sample_instance(inst);
	if (sample)
		al_destroy_sample(sample);
	inst = nullptr;
	sample = nullptr;
}

bool ZCSFX::play(int pan, bool loop, bool restart, zfix vol_perc, int freq)
{
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
	if (!sample) return 0;
	size_t freq = get_frequency();
	if (!freq) return 0;
	
	return 60 * al_get_sample_length(sample) / freq;
}
size_t ZCSFX::get_pos() const
{
	if (!sample || !is_playing()) return 0;
	return al_get_sample_instance_position(inst);
}
size_t ZCSFX::get_len() const
{
	if (!sample) return 0;
	return al_get_sample_length(sample);
}
size_t ZCSFX::get_frequency() const
{
	if (!sample) return 0;
	return al_get_sample_frequency(sample);
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
	int32_t c = 0;
	
	for (auto& sound : quest_sounds)
		if (sound.is_allocated())
			++c;
			
	return c;
}

// clean up finished samples
void sfx_cleanup()
{
	for (auto& sound : quest_sounds)
		sound.cleanup();
}

// plays an sfx sample
void sfx(int32_t index, int32_t pan, bool loop, bool restart, zfix vol_perc, int32_t freq)
{
	if (unsigned(index-1) >= quest_sounds.size())
		return;
	if (!is_headless())
	{
		ZCSFX& s = quest_sounds[index-1];
		
		s.play(pan, loop, vol_perc, restart, freq);
	}

#ifdef IS_PLAYER
	if (restart && replay_is_debug())
	{
		// TODO(replays): get rid of this bandaid next time replays are mass-updated.
		string sfx_name = quest_sounds[index-1].sfx_name;
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
	if (unsigned(index-1) >= quest_sounds.size())
		return false;
	return quest_sounds[index-1].is_allocated();
}

// play in loop mode
void cont_sfx(int32_t index)
{
	sfx(index, 128, true, false);
}

void adjust_sfx(int32_t index,int32_t pan,bool loop)
{
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
	if (unsigned(index-1) >= quest_sounds.size())
		return;
	if (is_headless())
		return;
	ZCSFX& sound = quest_sounds[index-1];
	sound.update_vol_perc(vol_perc);
}
void adjust_all_sfx_vol(optional<zfix> vol_perc)
{
	for (uint q = 0; q < quest_sounds.size(); ++q)
		adjust_sfx_vol(q+1, vol_perc);
}

void pause_sfx(int32_t index)
{
	if (is_headless())
		return;
	if (unsigned(index-1) >= quest_sounds.size())
		return;
	quest_sounds[index-1].pause();
}

void resume_sfx(int32_t index)
{
	if (is_headless())
		return;
	if (unsigned(index-1) >= quest_sounds.size())
		return;
	quest_sounds[index-1].resume();
}

void stop_sfx(int32_t index)
{
	if (is_headless())
		return;
	if (unsigned(index-1) >= quest_sounds.size())
		return;
	quest_sounds[index-1].stop();
}

void pause_all_sfx()
{
	for (auto& sound : quest_sounds)
		sound.pause();
}

void resume_all_sfx()
{
	for (auto& sound : quest_sounds)
		sound.resume();
}

void kill_sfx()
{
	for (auto& sound : quest_sounds)
		sound.stop();
}

void kill_sfx_except(std::set<size_t> const& skip_idxs)
{
	for (size_t idx = 0; idx < quest_sounds.size(); ++idx)
	{
		if (skip_idxs.contains(idx))
			continue;
		quest_sounds[idx].stop();
	}
}

