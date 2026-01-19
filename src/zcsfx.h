#pragma once

#include <allegro.h>
#include <allegro5/allegro_audio.h>
#include "base/headers.h"
#include "base/zapp.h"

class zcsfx_exception : public std::exception
{
public:
	const char * what() const noexcept override
	{
		return msg.c_str();
	}
	zcsfx_exception(string const& msg);
private:
	string msg;
};
class zcsfx_io_exception : public std::exception
{
public:
	const char * what() const noexcept override
	{
		return msg.c_str();
	}
	zcsfx_io_exception(char const* fpath, bool saving);
	zcsfx_io_exception& add_msg(string const& str);
private:
	string msg;
};

size_t get_al_buffer_size(ALLEGRO_CHANNEL_CONF chan_conf, ALLEGRO_AUDIO_DEPTH depth, size_t len);

enum SampleType : uint8_t
{
	SMPL_INVALID,
	SMPL_WAV,
	SMPL_OGG,
};

struct SampleBase
{
	virtual ~SampleBase();
	virtual SampleBase* duplicate() const = 0;
	virtual void cleanup_memory() = 0;
	virtual bool validate() const = 0;
	virtual SampleType get_sample_type() const = 0;
	virtual string get_sound_info() const = 0;
	
	virtual int read(PACKFILE* f, word s_version) = 0;
	virtual int write(PACKFILE* f) const = 0;
	virtual void save_sound(char const* fpath) const;
	
	virtual void update_pan(int pan) = 0;
	virtual void update_loop(bool loop) = 0;
	virtual void update_freq(int freq) = 0;
	virtual void update_gain(double gain) = 0;
	
	size_t get_len_frames() const;
	virtual size_t get_pos() const = 0;
	virtual size_t get_len() const = 0;
	virtual size_t get_frequency() const = 0;
	
	virtual bool is_allocated() const = 0;
	virtual bool is_playing() const = 0;
	
	virtual bool play(int pan, bool loop, bool restart, double gain, int freq) = 0;
	virtual void pause() = 0;
	virtual void resume() = 0;
	virtual void stop() = 0;
};

struct ZCSFX
{
	string sfx_name;
private:
	SampleBase* internal = nullptr;
	zfix base_vol = 100_zf; // not saved, only used for internal logic
	
	void set_internal(SampleBase* new_internal);
public:
	
	ZCSFX() = default;
	ZCSFX(SAMPLE const& s);
	ZCSFX(ZCSFX const& other);
	ZCSFX& operator=(ZCSFX const& other);
	ZCSFX(ZCSFX&& other);
	ZCSFX& operator=(ZCSFX&& other);
	~ZCSFX();
	
	SampleType get_sample_type() const;
	
	int read(PACKFILE* f, word s_version);
	int write(PACKFILE* f) const;
	
	void load_file(char const* filepath);
	void save_sound(char const* filepath) const;
	
	bool is_invalid() const;
	
	void clear();
	void cleanup_memory();
	
	bool play(int pan, bool loop, bool restart = true, zfix vol_perc = 100_zf, int freq = -1);
	void pause();
	void resume();
	void stop();
	void cleanup();
	
	void update_pan(int pan);
	void update_loop(bool loop);
	void update_freq(int freq);
	void update_gain(double gain);
	
	void update_vol_perc(optional<zfix> vol_perc = nullopt);
	
	bool is_allocated() const;
	bool is_playing() const;
	
	size_t get_len_frames() const;
	size_t get_pos() const;
	size_t get_len() const;
	
	string get_sound_info() const;
};

extern vector<ZCSFX> quest_sounds;

SampleType sfx_get_type(int32_t index);
int32_t sfx_count();
void sfx_cleanup();
void sfx_ex(int32_t index, int32_t pan, bool loop, bool restart = true, zfix vol_perc = 100_zf, int32_t freq = -1);
bool sfx_is_allocated(int32_t index);
INLINE void sfx(int32_t index, int32_t pan = 128, bool loop = false)
{
	sfx_ex(index, vbound(pan, 0, 255), false);
}
INLINE void sfx_no_repeat(int32_t index, int32_t pan = 128)
{
	if (!sfx_is_allocated(index))
		sfx_ex(index, vbound(pan, 0, 255), false, false);
}
INLINE void cont_sfx(int32_t index, int32_t pan = 128)
{
	sfx_ex(index, pan, true, false);
}
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

void delete_quest_sounds(std::function<bool(ZCSFX const&)> proc);
void delete_quest_sounds(size_t idx);
void swap_quest_sounds(size_t idx1, size_t idx2);
