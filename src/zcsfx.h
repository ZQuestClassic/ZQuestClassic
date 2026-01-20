#pragma once

#include <allegro.h>
#include <allegro5/allegro_audio.h>
#include "base/headers.h"
#include "base/zapp.h"

size_t get_al_buffer_size(ALLEGRO_CHANNEL_CONF chan_conf, ALLEGRO_AUDIO_DEPTH depth, size_t len);

enum SampleType : uint8_t
{
	SMPL_INVALID,
	SMPL_WAV,
	SMPL_OGG,
};

struct SampleDataBuffer
{
	void* buf = nullptr;
	int32_t len, frequency;
	ALLEGRO_AUDIO_DEPTH depth;
	ALLEGRO_CHANNEL_CONF chan_conf;
};

struct ZCSFX
{
	SampleType sample_type = SMPL_INVALID;
	string sfx_name;
	int priority, loop_start, loop_end, param; // unused values leftover from A4 samples. Unsure if they were ever used?
private:
	ALLEGRO_SAMPLE* sample = nullptr;
	ALLEGRO_SAMPLE_INSTANCE* inst = nullptr;
	
	zfix base_vol = 100_zf; // not saved, only used for internal logic
	
	// used only if no_sound is enabled, as ALLEGRO_SAMPLE* can't be created
	// stores all the data that would otherwise be in `sample`
	optional<SampleDataBuffer> databuf = nullopt;
public:
	
	ZCSFX() = default;
	ZCSFX(SAMPLE& s);
	ZCSFX(ZCSFX const& other);
	ZCSFX& operator=(ZCSFX const& other);
	ZCSFX(ZCSFX&& other);
	ZCSFX& operator=(ZCSFX&& other);
	~ZCSFX();
	
	void load_sample(void* data, int32_t len, int32_t freq, ALLEGRO_AUDIO_DEPTH depth, ALLEGRO_CHANNEL_CONF chan_conf);
	void load_sample(ALLEGRO_SAMPLE* new_sample);
	bool save_sample(char const* filepath) const;
	
	bool is_invalid() const;
	size_t get_buffer_size() const;
	uint8_t const* get_sample_data() const;
	void clear();
	void clear_sample();
	bool play(int pan, bool loop, bool restart = true, zfix vol_perc = 100_zf, int freq = -1);
	void pause();
	void resume();
	void stop();
	void cleanup();
	
	void update_pan(int pan);
	void update_loop(bool loop);
	void update_freq(int freq);
	void update_vol_perc(optional<zfix> vol_perc = nullopt);
	void update_gain(double gain);
	
	bool is_allocated() const;
	bool is_playing() const;
	size_t get_len_frames() const;
	size_t get_pos() const;
	size_t get_len() const;
	size_t get_frequency() const;
	ALLEGRO_CHANNEL_CONF get_chan_conf() const;
	ALLEGRO_AUDIO_DEPTH get_depth() const;
};

extern vector<ZCSFX> quest_sounds;

int32_t sfx_count();
void sfx_cleanup();
void sfx(int32_t index,int32_t pan,bool loop, bool restart = true, zfix vol_perc = 100_zf, int32_t freq = -1);
bool sfx_is_allocated(int32_t index);
INLINE void sfx(int32_t index,int32_t pan = 128)
{
	sfx(index,vbound(pan, 0, 255) ,false);
}
INLINE void sfx_no_repeat(int32_t index, int32_t pan = 128)
{
	if (!sfx_is_allocated(index))
		sfx(index, vbound(pan, 0, 255), false, false);
}
void cont_sfx(int32_t index);
void stop_sfx(int32_t index);
void adjust_sfx(int32_t index,int32_t pan,bool loop);
void adjust_sfx_vol(int32_t index, optional<zfix> vol_perc = nullopt);
void adjust_all_sfx_vol(optional<zfix> vol_perc = nullopt);
void pause_sfx(int32_t index);
void resume_sfx(int32_t index);
void pause_all_sfx();
void resume_all_sfx();
void stop_sfx(int32_t index);
void kill_sfx();
void kill_sfx_except(std::set<size_t> const& skip_idxs);