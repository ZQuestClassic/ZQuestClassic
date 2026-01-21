#include "zcsfx.h"
#include <allegro5/internal/aintern_audio.h>
#include <zalleg/zalleg.h>
#include "base/qst.h"
#include "base/packfile.h"

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


SampleBase::~SampleBase()
{
	cleanup_memory();
}

void SampleBase::cleanup_memory()
{
	
}

bool SampleBase::save_sample(char const* fpath) const
{
	return false;
}

size_t SampleBase::get_len_frames() const
{
	size_t freq = get_frequency();
	if (!freq) return 0;
	return 60 * get_len() / freq;
}

namespace // sample implementations
{
	///
	/// OGG Files
	///
	
	// struct SampleOGG : public SampleBase
	// {
		// ALLEGRO_AUDIO_STREAM* stream = nullptr;
		// void* raw_ogg_buf = nullptr;
		
		// int read(PACKFILE* f, word s_version) override;
		// int write(PACKFILE* f) const override;
		
		// size_t get_pos() const override;
		// size_t get_len() const override;
		// size_t get_frequency() const override;
		// ALLEGRO_CHANNEL_CONF get_chan_conf() const override;
		// ALLEGRO_AUDIO_DEPTH get_depth() const override;
	// };
	
	///
	/// Raw Audio Data, alternate storage
	/// Used only if nosound is enabled, as ALLEGRO_SAMPLE* can't be created.
	/// Stores all the data that would otherwise be in the ALLEGRO_SAMPLE*.
	///
	
	struct SampleWAV_NoSound : public SampleBase
	{
		void* buf = nullptr;
		int32_t len, frequency;
		ALLEGRO_AUDIO_DEPTH depth;
		ALLEGRO_CHANNEL_CONF chan_conf;
		
		SampleWAV_NoSound() = default;
		SampleWAV_NoSound(SAMPLE& s);
		SampleWAV_NoSound(SampleWAV_NoSound const& other);
		SampleWAV_NoSound& operator=(SampleWAV_NoSound const& other);
		
		SampleBase* duplicate() const override;
		void cleanup_memory() override;
		bool validate() const override;
		SampleType get_sample_type() const override;
		string get_sound_info() const override;
		
		int read(PACKFILE* f, word s_version) override;
		int write(PACKFILE* f) const override;
		
		void update_pan(int pan) override;
		void update_loop(bool loop) override;
		void update_freq(int freq) override;
		void update_gain(double gain) override;
		
		size_t get_pos() const override;
		size_t get_len() const override;
		size_t get_frequency() const override;
		ALLEGRO_CHANNEL_CONF get_chan_conf() const override;
		ALLEGRO_AUDIO_DEPTH get_depth() const override;
		
		bool is_allocated() const override;
		bool is_playing() const override;
		
		bool play(int pan, bool loop, bool restart, double gain, int freq) override;
		void pause() override;
		void resume() override;
		void stop() override;
	};
	
	SampleWAV_NoSound::SampleWAV_NoSound(SAMPLE& s)
	{
		chan_conf = s.stereo ? ALLEGRO_CHANNEL_CONF_2 : ALLEGRO_CHANNEL_CONF_1;
		depth = s.bits == 8 ? ALLEGRO_AUDIO_DEPTH_INT8 : ALLEGRO_AUDIO_DEPTH_INT16;
		len = s.len;
		frequency = s.freq;
		
		size_t buffer_len = get_al_buffer_size(chan_conf, depth, len);
		uint8_t* data = (uint8_t*)al_malloc(buffer_len);
		if (data)
		{
			uint channels = al_get_channel_count(chan_conf);
			memcpy(data, s.data, buffer_len);
			size_t frames = (len * channels);
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
			buf = data;
		}
	}
	SampleWAV_NoSound::SampleWAV_NoSound(SampleWAV_NoSound const& other) : SampleWAV_NoSound()
	{
		*this = other;
	}
	SampleWAV_NoSound& SampleWAV_NoSound::operator=(SampleWAV_NoSound const& other)
	{
		cleanup_memory();
		len = other.len;
		frequency = other.frequency;
		depth = other.depth;
		chan_conf = other.chan_conf;
		
		if (other.buf)
		{
			size_t buffer_len = get_al_buffer_size(chan_conf, depth, len);
			uint8_t* data = (uint8_t*)al_malloc(buffer_len);
			memcpy(data, other.buf, buffer_len);
			buf = (void*)data;
		}
		return *this;
	}
	
	SampleBase* SampleWAV_NoSound::duplicate() const
	{
		return new SampleWAV_NoSound(*this);
	}
	void SampleWAV_NoSound::cleanup_memory()
	{
		if (buf)
			al_free(buf);
		buf = nullptr;
	}
	bool SampleWAV_NoSound::validate() const
	{
		return bool(buf);
	}
	SampleType SampleWAV_NoSound::get_sample_type() const
	{
		return SMPL_WAV;
	}
	string SampleWAV_NoSound::get_sound_info() const
	{
		std::ostringstream oss;
		oss << "Type: .wav";
		oss << "\nLen: " << get_len();
		oss << "\nFreq: " << get_frequency();
		oss << "\nDuration: " << (zfix(int(get_len())) / int(get_frequency())).str();
		auto channels = get_chan_conf();
		oss << "\nChannels: " << int(channels >> 4);
		if (channels & 0xF)
			oss << "." << int(channels & 0xF);
		auto depth = get_depth();
		oss << "\nDepth: ";
		if (depth & ALLEGRO_AUDIO_DEPTH_UNSIGNED)
			oss << "U";
		switch (depth&0x7)
		{
			case ALLEGRO_AUDIO_DEPTH_INT8:
				oss << "INT8";
				break;
			case ALLEGRO_AUDIO_DEPTH_INT16:
				oss << "INT16";
				break;
			case ALLEGRO_AUDIO_DEPTH_INT24:
				oss << "INT24";
				break;
			case ALLEGRO_AUDIO_DEPTH_FLOAT32:
				oss << "FLOAT32";
				break;
		}
		return oss.str();
	}
	
	int SampleWAV_NoSound::read(PACKFILE* f, word s_version)
	{
		byte dummy;
		if (!p_getc(&dummy, f))
			return qe_invalid;
		depth = (ALLEGRO_AUDIO_DEPTH)dummy;
		if (!p_getc(&dummy, f))
			return qe_invalid;
		chan_conf = (ALLEGRO_CHANNEL_CONF)dummy;
		if (!p_igetl(&frequency, f))
			return qe_invalid;
		if (!p_igetl(&len, f))
			return qe_invalid;
		size_t buffer_len = get_al_buffer_size(chan_conf, depth, len);
		byte* data = (byte*)al_malloc(buffer_len);
		if (!data)
			return qe_nomem;
		if (!pfread(data, buffer_len, f))
		{
			al_free(data);
			return qe_invalid;
		}
		buf = (void*)data;
		return 0;
	}
	int SampleWAV_NoSound::write(PACKFILE* f) const
	{
		if (!p_putc(depth, f))
			new_return(12);
		if (!p_putc(chan_conf, f))
			new_return(13);
		if (!p_iputl(frequency, f))
			new_return(14);
		if (!p_iputl(len, f))
			new_return(15);
		byte const* data = (byte const*)buf;
		size_t sz = get_al_buffer_size(chan_conf, depth, len);
		if (!pfwrite(data, sz, f))
			new_return(16);
		return 0;
	}
	
	void SampleWAV_NoSound::update_pan(int pan) {}
	void SampleWAV_NoSound::update_loop(bool loop) {}
	void SampleWAV_NoSound::update_freq(int freq) {}
	void SampleWAV_NoSound::update_gain(double gain) {}
	
	size_t SampleWAV_NoSound::get_pos() const
	{
		return 0;
	}
	size_t SampleWAV_NoSound::get_len() const
	{
		return len;
	}
	size_t SampleWAV_NoSound::get_frequency() const
	{
		return frequency;
	}
	ALLEGRO_CHANNEL_CONF SampleWAV_NoSound::get_chan_conf() const
	{
		return chan_conf;
	}
	ALLEGRO_AUDIO_DEPTH SampleWAV_NoSound::get_depth() const
	{
		return depth;
	}
	
	bool SampleWAV_NoSound::is_allocated() const
	{
		return false;
	}
	bool SampleWAV_NoSound::is_playing() const
	{
		return false;
	}
	
	bool SampleWAV_NoSound::play(int pan, bool loop, bool restart, double gain, int freq)
	{
		return false;
	}
	void SampleWAV_NoSound::pause() {}
	void SampleWAV_NoSound::resume() {}
	void SampleWAV_NoSound::stop() {}
	
	///
	/// Raw Audio Data
	///
	
	struct SampleWAV : public SampleBase
	{
		ALLEGRO_SAMPLE* sample = nullptr;
		ALLEGRO_SAMPLE_INSTANCE* inst = nullptr;
		
		SampleWAV() = default;
		SampleWAV(SAMPLE& s);
		SampleWAV(SampleWAV const& other);
		SampleWAV& operator=(SampleWAV const& other);
		
		SampleBase* duplicate() const override;
		void cleanup_memory() override;
		bool validate() const override;
		SampleType get_sample_type() const override;
		string get_sound_info() const override;
		
		int read(PACKFILE* f, word s_version) override;
		int write(PACKFILE* f) const override;
		bool save_sample(char const* fpath) const override;
		
		void update_pan(int pan) override;
		void update_loop(bool loop) override;
		void update_freq(int freq) override;
		void update_gain(double gain) override;
		
		size_t get_pos() const override;
		size_t get_len() const override;
		size_t get_frequency() const override;
		ALLEGRO_CHANNEL_CONF get_chan_conf() const override;
		ALLEGRO_AUDIO_DEPTH get_depth() const override;
		
		bool is_allocated() const override;
		bool is_playing() const override;
		
		bool play(int pan, bool loop, bool restart, double gain, int freq) override;
		void pause() override;
		void resume() override;
		void stop() override;
	};
	
	SampleWAV::SampleWAV(SAMPLE& s)
	{
		SampleWAV_NoSound wavns(s); // use the other class here, to reduce code duplication
		if (wavns.validate())
		{
			sample = al_create_sample((void*)(wavns.buf), wavns.len, wavns.frequency,
				wavns.depth, wavns.chan_conf, true);
			if (sample)
				wavns.buf = nullptr; // stop the wavns destructor from freeing the data
		}
	}
	SampleWAV::SampleWAV(SampleWAV const& other) : SampleWAV()
	{
		*this = other;
	}
	SampleWAV& SampleWAV::operator=(SampleWAV const& other)
	{
		cleanup_memory();
		if (other.sample)
		{
			auto chan_conf = other.get_chan_conf();
			auto depth = other.get_depth();
			auto len = other.get_len();
			size_t buffer_len = get_al_buffer_size(chan_conf, depth, len);
			uint8_t* data = (uint8_t*)al_malloc(buffer_len);
			memcpy(data, al_get_sample_data(other.sample), buffer_len);
			sample = al_create_sample(data, len, other.get_frequency(), depth, chan_conf, true);
			// don't copy the 'inst'
		}
		return *this;
	}
	
	SampleBase* SampleWAV::duplicate() const
	{
		return new SampleWAV(*this);
	}
	void SampleWAV::cleanup_memory()
	{
		if (inst)
			al_destroy_sample_instance(inst);
		if (sample)
			al_destroy_sample(sample);
		inst = nullptr;
		sample = nullptr;
	}
	bool SampleWAV::validate() const
	{
		return bool(sample);
	}
	SampleType SampleWAV::get_sample_type() const
	{
		return SMPL_WAV;
	}
	string SampleWAV::get_sound_info() const
	{
		SampleWAV_NoSound tmp;
		tmp.chan_conf = get_chan_conf();
		tmp.depth = get_depth();
		tmp.len = get_len();
		tmp.frequency = get_frequency();
		return tmp.get_sound_info();
	}
	
	int SampleWAV::read(PACKFILE* f, word s_version)
	{
		SampleWAV_NoSound tmp;
		if (auto ret = tmp.read(f, s_version))
			return ret;
		
		sample = al_create_sample((void*)(tmp.buf), tmp.len, tmp.frequency, tmp.depth, tmp.chan_conf, true);
		if (!sample)
			return qe_invalid;
		tmp.buf = nullptr; // take ownership of the buf before tmp is destroyed
		return 0;
	}
	int SampleWAV::write(PACKFILE* f) const
	{
		SampleWAV_NoSound tmp;
		tmp.chan_conf = get_chan_conf();
		tmp.depth = get_depth();
		tmp.len = get_len();
		tmp.frequency = get_frequency();
		tmp.buf = al_get_sample_data(sample);
		auto ret = tmp.write(f);
		tmp.buf = nullptr; // don't let tmp think it owns the buf
		return ret;
	}
	bool SampleWAV::save_sample(char const* fpath) const
	{
		if (!sample) return false;
		return al_save_sample(fpath, sample);
	}
	
	void SampleWAV::update_pan(int pan)
	{
		if (inst)
			al_set_sample_instance_pan(inst, pan / 255.0);
	}
	void SampleWAV::update_loop(bool loop)
	{
		if (inst)
			al_set_sample_instance_playmode(inst, loop ? ALLEGRO_PLAYMODE_LOOP : ALLEGRO_PLAYMODE_ONCE);
	}
	void SampleWAV::update_freq(int freq)
	{
		if (inst)
		{
			if (freq < 0)
				freq = al_get_sample_frequency(sample);
			inst->spl_data.frequency = freq;
		}
	}
	void SampleWAV::update_gain(double gain)
	{
		if (inst)
			al_set_sample_instance_gain(inst, gain);
	}
	
	size_t SampleWAV::get_pos() const
	{
		if (!sample || !is_playing()) return 0;
		return al_get_sample_instance_position(inst);
	}
	size_t SampleWAV::get_len() const
	{
		return al_get_sample_length(sample);
	}
	size_t SampleWAV::get_frequency() const
	{
		return al_get_sample_frequency(sample);
	}
	ALLEGRO_CHANNEL_CONF SampleWAV::get_chan_conf() const
	{
		return al_get_sample_channels(sample);
	}
	ALLEGRO_AUDIO_DEPTH SampleWAV::get_depth() const
	{
		return al_get_sample_depth(sample);
	}
	
	bool SampleWAV::is_allocated() const
	{
		return inst != nullptr;
	}
	bool SampleWAV::is_playing() const
	{
		return inst && al_get_sample_instance_playing(inst);
	}
	
	bool SampleWAV::play(int pan, bool loop, bool restart, double gain, int freq)
	{
		if (inst && restart)
		{
			al_destroy_sample_instance(inst);
			inst = nullptr;
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
		update_gain(gain);
		al_play_sample_instance(inst);
		return true;
	}
	void SampleWAV::pause()
	{
		if (inst)
		{
			auto pos = get_pos();
			al_stop_sample_instance(inst);
			al_set_sample_instance_position(inst, pos);
		}
	}
	void SampleWAV::resume()
	{
		if (inst)
			al_play_sample_instance(inst);
	}
	void SampleWAV::stop()
	{
		if (inst)
		{
			al_destroy_sample_instance(inst);
			inst = nullptr;
		}
	}
}

ZCSFX::ZCSFX(SAMPLE& s)
{
	if (sound_was_installed)
		internal = new SampleWAV(s);
	else
		internal = new SampleWAV_NoSound(s);
	if (!internal->validate())
		cleanup_memory();
}

ZCSFX::ZCSFX(ZCSFX const& other)
{
	internal = nullptr;
	*this = other;
}
ZCSFX& ZCSFX::operator=(ZCSFX const& other)
{
	cleanup_memory();
	sfx_name = other.sfx_name;
	
	if (other.internal)
		internal = other.internal->duplicate();

	if (internal && !internal->validate())
		cleanup_memory();
	return *this;
}
ZCSFX::ZCSFX(ZCSFX&& other)
{
	internal = nullptr;
	*this = other;
}
ZCSFX& ZCSFX::operator=(ZCSFX&& other)
{
	cleanup_memory();
	sfx_name = other.sfx_name;

	// Take ownership
	internal = other.internal;
	other.internal = nullptr;
	return *this;
}

ZCSFX::~ZCSFX()
{	
	cleanup_memory();
}

SampleType ZCSFX::get_sample_type() const
{
	if (internal)
		return internal->get_sample_type();
	return SMPL_INVALID;
}

int ZCSFX::read(PACKFILE* f, word s_version)
{
	cleanup_memory();
	byte sample_type;
	if (!p_getc(&sample_type, f))
		return qe_invalid;
	if (!p_getcstr(&sfx_name, f))
		return qe_invalid;
	switch (sample_type)
	{
		case SMPL_WAV:
			if (sound_was_installed)
				internal = new SampleWAV();
			else
				internal = new SampleWAV_NoSound();
			break;
		// case SMPL_OGG:
			// internal = new SampleOGG();
			// break;
		default:
			return 0;
	}
	if (auto ret = internal->read(f, s_version))
	{
		clear();
		return ret;
	}
	return 0;
}
int ZCSFX::write(PACKFILE* f) const
{
	auto stype = get_sample_type();
	if (!p_putc(stype, f))
		new_return(1);
	if (!p_putcstr(sfx_name, f))
		new_return(2);
	if (internal)
		if (auto ret = internal->write(f))
			return ret;
	return 0;
}

void ZCSFX::load_sample(ALLEGRO_SAMPLE* new_sample)
{
	if (!sound_was_installed) return;
	cleanup_memory();
	SampleWAV* ptr = new SampleWAV();
	ptr->sample = new_sample;
	internal = ptr;
}
bool ZCSFX::save_sample(char const* filepath) const
{
	if (!sound_was_installed) return false;
	if (!internal) return false;
	return internal->save_sample(filepath);
}

bool ZCSFX::is_invalid() const
{
	return !internal;
}

void ZCSFX::clear()
{
	cleanup_memory();
	*this = ZCSFX();
}
void ZCSFX::cleanup_memory()
{
	if (internal)
		delete internal;
	internal = nullptr;
}

bool ZCSFX::play(int pan, bool loop, bool restart, zfix vol_perc, int freq)
{
	if (!sound_was_installed) return false;
	if (!internal) return false;
	
	base_vol = vol_perc;
	double gain = calc_gain(vol_perc);
	internal->play(pan, loop, restart, gain, freq);
	return true;
}
void ZCSFX::pause()
{
	if (internal)
		internal->pause();
}
void ZCSFX::resume()
{
	if (internal)
		internal->resume();
}
void ZCSFX::stop()
{
	if (internal)
		internal->stop();
}
void ZCSFX::cleanup()
{
	if (is_allocated() && !is_playing() && get_pos() <= 0)
		stop();
}

void ZCSFX::update_pan(int pan)
{
	if (internal)
		internal->update_pan(pan);
}
void ZCSFX::update_loop(bool loop)
{
	if (internal)
		internal->update_loop(loop);
}
void ZCSFX::update_freq(int freq)
{
	if (internal)
		internal->update_freq(freq);
}
void ZCSFX::update_gain(double gain)
{
	if (internal)
		internal->update_gain(gain);
}

void ZCSFX::update_vol_perc(optional<zfix> vol_perc)
{
	if (vol_perc)
		base_vol = *vol_perc;
	update_gain(calc_gain(base_vol));
}

bool ZCSFX::is_allocated() const
{
	return internal && internal->is_allocated();
}
bool ZCSFX::is_playing() const
{
	return internal && internal->is_playing();
}

size_t ZCSFX::get_len_frames() const
{
	return internal ? internal->get_len_frames() : 0;
}
size_t ZCSFX::get_pos() const
{
	return internal ? internal->get_pos() : 0;
}
size_t ZCSFX::get_len() const
{
	return internal ? internal->get_len() : 0;
}

string ZCSFX::get_sound_info() const
{
	return internal ? internal->get_sound_info() : "[Empty]";
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
	if (unsigned(index-1) >= quest_sounds.size())
		return;
	if (sound_was_installed)
	{
		ZCSFX& s = quest_sounds[index-1];
		
		s.play(pan, loop, restart, vol_perc, freq);
	}
	
#ifdef IS_PLAYER
	if (restart && replay_is_debug())
	{
		string sfx_name = quest_sounds[index-1].sfx_name;
		
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

