#include "zcsfx.h"
#include <allegro5/internal/aintern_audio.h>
#include <allegro5/allegro_memfile.h>
#include <zalleg/zalleg.h>
#include "base/qst.h"
#include "base/packfile.h"

#ifdef IS_PLAYER
#include "base/qrs.h"
#include "zc/ffscript.h"
#endif

vector<ZCSFX> quest_sounds;

zcsfx_exception::zcsfx_exception(string const& msg) :
	msg(msg)
{}
zcsfx_io_exception::zcsfx_io_exception(char const* fpath, bool saving) :
	msg(fmt::format("Error {} file:\n'{}'", saving ? "saving" : "loading", fpath))
{}
zcsfx_io_exception& zcsfx_io_exception::add_msg(string const& str)
{
	msg += fmt::format("\n{}", str);
	return *this;
}

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
{}

void SampleBase::save_sound(char const* fpath) const
{
	throw zcsfx_io_exception(fpath, true).add_msg("Save code unimplemented for this sound type! Report to devs!");
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
	
	struct SampleOGG : public SampleBase
	{
		size_t buffer_size = 0;
		uint frequency, len; // sample size data
		void* raw_ogg_buf = nullptr;
		ALLEGRO_FILE* memfile = nullptr;
		ALLEGRO_AUDIO_STREAM* stream = nullptr;
		
		SampleOGG() = default;
		SampleOGG(char const* filepath);
		SampleOGG(SampleOGG const& other);
		SampleOGG& operator=(SampleOGG const& other);
		~SampleOGG();
		
		SampleBase* duplicate() const override;
		void cleanup_memory() override;
		bool validate() const override;
		SampleType get_sample_type() const override;
		string get_sound_info() const override;
		
		int read(PACKFILE* f, word s_version) override;
		int write(PACKFILE* f) const override;
		void save_sound(char const* fpath) const override;
		
		void update_pan(int pan) override;
		void update_loop(bool loop) override;
		void update_freq(int freq) override;
		void update_gain(double gain) override;
		
		size_t get_pos() const override;
		size_t get_len() const override;
		size_t get_frequency() const override;
		
		bool is_allocated() const override;
		bool is_playing() const override;
		
		bool play(int pan, bool loop, bool restart, double gain, int freq) override;
		void pause() override;
		void resume() override;
		void stop() override;
		
	private:
		void cleanup_stream();
		bool ensure_stream();
	};
	SampleOGG::SampleOGG(char const* filepath) : SampleOGG()
	{
		zcsfx_io_exception error(filepath, false);
		ALLEGRO_FILE *f = al_fopen(filepath, "rb");
		if (!f)
			throw error.add_msg("Failed to open file for reading!");
		try
		{
			int64_t sz = al_fsize(f);
			if (sz < 0)
				throw error.add_msg("Failed determine file size!");
			if (sz)
			{
				raw_ogg_buf = al_malloc(sz);
				buffer_size = size_t(sz);
				if (!raw_ogg_buf)
					throw error.add_msg("Failed to allocate sfx data buffer!");
				if (sz != al_fread(f, raw_ogg_buf, buffer_size))
					throw error.add_msg("Failed reading data from file!");
			}
			if (!ensure_stream())
				throw error.add_msg("Failed to parse data as '.ogg' audio stream!");
			frequency = al_get_audio_stream_frequency(stream);
			len = uint(al_get_audio_stream_length_secs(stream) * frequency);
		}
		catch (zcsfx_io_exception& e)
		{
			al_fclose(f);
			throw e;
		}
		al_fclose(f);
	}
	SampleOGG::SampleOGG(SampleOGG const& other) : SampleOGG()
	{
		*this = other;
	}
	SampleOGG& SampleOGG::operator=(SampleOGG const& other)
	{
		cleanup_memory();
		frequency = other.frequency;
		len = other.len;
		if (other.raw_ogg_buf)
		{
			raw_ogg_buf = al_malloc(other.buffer_size);
			if (raw_ogg_buf)
			{
				buffer_size = other.buffer_size;
				memcpy(raw_ogg_buf, other.raw_ogg_buf, buffer_size);
			}
		}
		return *this;
	}
	SampleOGG::~SampleOGG()
	{
		cleanup_memory();
	}
	
	SampleBase* SampleOGG::duplicate() const
	{
		return new SampleOGG(*this);
	}
	void SampleOGG::cleanup_stream()
	{
		if (stream)
		{
			al_set_audio_stream_playing(stream, false);
			al_destroy_audio_stream(stream); // closes the memfile!
			memfile = nullptr;
		}
		stream = nullptr;
		if (memfile)
			al_fclose(memfile);
		memfile = nullptr;
	}
	void SampleOGG::cleanup_memory()
	{
		cleanup_stream();
		if (raw_ogg_buf)
			al_free(raw_ogg_buf);
		raw_ogg_buf = nullptr;
		buffer_size = 0;
		frequency = 0;
		len = 0;
	}
	bool SampleOGG::validate() const
	{
		return raw_ogg_buf && buffer_size;
	}
	SampleType SampleOGG::get_sample_type() const
	{
		return SMPL_OGG;
	}
	string SampleOGG::get_sound_info() const
	{
		std::ostringstream oss;
		oss << "Type: .ogg";
		oss << "\nLen: " << get_len();
		oss << "\nFreq: " << get_frequency();
		oss << "\nDuration: " << zfix(double(get_len()) / double(get_frequency())).str();
		return oss.str();
	}
	
	int SampleOGG::read(PACKFILE* f, word s_version)
	{
		cleanup_memory();
		if (!p_igetl(&buffer_size, f))
			return qe_invalid;
		if (!p_igetl(&frequency, f))
			return qe_invalid;
		if (!p_igetl(&len, f))
			return qe_invalid;
		raw_ogg_buf = al_malloc(buffer_size);
		if (!raw_ogg_buf)
			return qe_nomem;
		if (!pfread(raw_ogg_buf, buffer_size, f))
			return qe_invalid;
		
		return 0;
	}
	int SampleOGG::write(PACKFILE* f) const
	{
		if (!p_iputl(buffer_size, f))
			new_return(1);
		if (!p_iputl(frequency, f))
			new_return(2);
		if (!p_iputl(len, f))
			new_return(3);
		if (!pfwrite(raw_ogg_buf, buffer_size, f))
			new_return(4);
		
		return 0;
	}
	void SampleOGG::save_sound(char const* fpath) const
	{
		zcsfx_io_exception error(fpath, true);
		if (!validate())
			throw error.add_msg("SFX is invalid!");
		string path = fpath;
		if (path.ends_with(".ogg"))
		{
			ALLEGRO_FILE* f = al_fopen(fpath, "wb");
			if (!f)
				throw error.add_msg("Failed to open/create file for writing!");
			auto sz = al_fwrite(f, raw_ogg_buf, buffer_size);
			al_fclose(f);
			if (buffer_size != sz)
				throw error.add_msg("Failed writing data to file!");
		}
		else
		{
			ALLEGRO_FILE* f = al_open_memfile(raw_ogg_buf, buffer_size, "rb");
			if (!f)
				throw error.add_msg("Failed to load memfile to data buffer");
			ALLEGRO_SAMPLE* samp = al_load_sample_f(f, ".ogg");
			try
			{
				if (!samp)
					throw error.add_msg("Failed to parse data as ogg sample");
				if (!al_save_sample(fpath, samp))
					throw error.add_msg("Failed to write data as specified output type");
			}
			catch (zcsfx_io_exception& e)
			{
				al_fclose(f);
				if (samp)
					al_destroy_sample(samp);
				throw e;
			}
			al_fclose(f);
		}
	}
	
	void SampleOGG::update_pan(int pan)
	{
		if (stream)
			al_set_audio_stream_pan(stream, pan / 255.0);
	}
	void SampleOGG::update_loop(bool loop)
	{
		if (stream)
			al_set_audio_stream_playmode(stream, loop ? ALLEGRO_PLAYMODE_LOOP : ALLEGRO_PLAYMODE_ONCE);
	}
	void SampleOGG::update_freq(int freq)
	{
		// can't modify frequency of a stream, except via mixer
		// but, can't connect mixer to mixer of different frequency
		// so, no way to update frequency for streamed sfx like this
		// Anywhere that calls `sfx_ex` with freq >= 0 should be prepared to catch this exception.
		// Currently, that is ONLY one spot in `ffscript.cpp`.
		if (freq >= 0)
			throw zcsfx_exception("Updated frequency of invalid SFX type 'SMPL_OGG'");
	}
	void SampleOGG::update_gain(double gain)
	{
		if (stream)
			al_set_audio_stream_gain(stream, gain);
	}
	
	size_t SampleOGG::get_pos() const
	{
		if (!is_playing())
			return 0;
		return size_t(al_get_audio_stream_position_secs(stream) * get_frequency());
	}
	size_t SampleOGG::get_len() const
	{
		if (stream) return uint(al_get_audio_stream_length_secs(stream) * get_frequency());
		return len;
	}
	size_t SampleOGG::get_frequency() const
	{
		if (stream) return al_get_audio_stream_frequency(stream);
		return frequency;
	}
	
	bool SampleOGG::is_allocated() const
	{
		return bool(stream);
	}
	bool SampleOGG::is_playing() const
	{
		if (!stream) return false;
		return al_get_audio_stream_playing(stream);
	}
	
	bool SampleOGG::play(int pan, bool loop, bool restart, double gain, int freq)
	{
		if (stream && restart)
			cleanup_stream();
		if (!ensure_stream())
			return false;
		
		update_pan(pan);
		update_loop(loop);
		update_freq(freq);
		update_gain(gain);
		resume();
		return true;
	}
	void SampleOGG::pause()
	{
		if (stream)
		{
			double pos = al_get_audio_stream_position_secs(stream);
			al_set_audio_stream_playing(stream, false);
			al_seek_audio_stream_secs(stream, pos);
		}
	}
	void SampleOGG::resume()
	{
		if (stream)
			al_set_audio_stream_playing(stream, true);
	}
	void SampleOGG::stop()
	{
		if (stream)
			al_set_audio_stream_playing(stream, false);
	}
	bool SampleOGG::ensure_stream()
	{
		if (!validate())
			return false;
		if (stream)
			return true;
		cleanup_stream();
		bool ret = false;
		if ((memfile = al_open_memfile(raw_ogg_buf, buffer_size, "rb")))
		{
			stream = al_load_audio_stream_f(memfile, ".ogg", 4, 2048);
			if (stream && al_attach_audio_stream_to_mixer(stream, al_get_default_mixer()))
			{
				ret = true;
				al_set_audio_stream_playing(stream, false); // don't necessarily want to play right now
			}
		}
		if (!ret)
			cleanup_stream();
		return ret;
	}
	
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
		SampleWAV_NoSound(SAMPLE const& s);
		SampleWAV_NoSound(SampleWAV_NoSound const& other);
		SampleWAV_NoSound& operator=(SampleWAV_NoSound const& other);
		~SampleWAV_NoSound();
		
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
		ALLEGRO_CHANNEL_CONF get_chan_conf() const;
		ALLEGRO_AUDIO_DEPTH get_depth() const;
		
		bool is_allocated() const override;
		bool is_playing() const override;
		
		bool play(int pan, bool loop, bool restart, double gain, int freq) override;
		void pause() override;
		void resume() override;
		void stop() override;
	};
	
	SampleWAV_NoSound::SampleWAV_NoSound(SAMPLE const& s) : SampleWAV_NoSound()
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
	SampleWAV_NoSound::~SampleWAV_NoSound()
	{
		cleanup_memory();
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
		buf = al_malloc(buffer_len);
		if (!buf)
			return qe_nomem;
		if (!pfread(buf, buffer_len, f))
			return qe_invalid;
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
		SampleWAV(SAMPLE const& s);
		SampleWAV(char const* filepath);
		SampleWAV(SampleWAV const& other);
		SampleWAV& operator=(SampleWAV const& other);
		~SampleWAV();
		
		SampleBase* duplicate() const override;
		void cleanup_memory() override;
		bool validate() const override;
		SampleType get_sample_type() const override;
		string get_sound_info() const override;
		
		int read(PACKFILE* f, word s_version) override;
		int write(PACKFILE* f) const override;
		void save_sound(char const* fpath) const override;
		
		void update_pan(int pan) override;
		void update_loop(bool loop) override;
		void update_freq(int freq) override;
		void update_gain(double gain) override;
		
		size_t get_pos() const override;
		size_t get_len() const override;
		size_t get_frequency() const override;
		ALLEGRO_CHANNEL_CONF get_chan_conf() const;
		ALLEGRO_AUDIO_DEPTH get_depth() const;
		
		bool is_allocated() const override;
		bool is_playing() const override;
		
		bool play(int pan, bool loop, bool restart, double gain, int freq) override;
		void pause() override;
		void resume() override;
		void stop() override;
	};
	
	SampleWAV::SampleWAV(SAMPLE const& s) : SampleWAV()
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
	SampleWAV::SampleWAV(char const* filepath) : SampleWAV()
	{
		if (ALLEGRO_SAMPLE* s = al_load_sample(filepath))
			sample = s;
		else
			throw zcsfx_io_exception(filepath, false);
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
	SampleWAV::~SampleWAV()
	{
		cleanup_memory();
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
	void SampleWAV::save_sound(char const* fpath) const
	{
		zcsfx_io_exception error(fpath, true);
		if (!sample)
			throw error.add_msg("Empty sound sample!");
		string path = fpath;
		if (path.ends_with(".wav"))
		{
			if (!al_save_sample(fpath, sample))
				throw error.add_msg("Failed to save file!");
		}
		else
		{
			if (!al_save_sample(fpath, sample))
				throw error.add_msg("Failed to save file as specified output type!");
		}
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

ZCSFX::ZCSFX(SAMPLE const& s) : ZCSFX()
{
	if (sound_was_installed)
		internal = new SampleWAV(s);
	else
		internal = new SampleWAV_NoSound(s);
	if (!internal->validate())
		cleanup_memory();
}

ZCSFX::ZCSFX(ZCSFX const& other) : ZCSFX()
{
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
ZCSFX::ZCSFX(ZCSFX&& other) : ZCSFX()
{
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

void ZCSFX::set_internal(SampleBase* new_internal)
{
	cleanup_memory();
	internal = new_internal;
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
		case SMPL_OGG:
			internal = new SampleOGG();
			break;
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

// either succeeds or throws 'zcsfx_io_exception'
void ZCSFX::load_file(char const* filepath)
{
	if (!sound_was_installed)
		throw zcsfx_io_exception(filepath, false).add_msg("Sound is disabled, cannot load sound files!");
	string t = get_filename(filepath);
	
	if (t.ends_with(".wav"))
		set_internal(new SampleWAV(filepath));
	else if (t.ends_with(".ogg"))
		set_internal(new SampleOGG(filepath));
	else
		throw zcsfx_io_exception(filepath, false).add_msg("Unrecognized extension!");
	sfx_name = t.substr(0, t.find_first_of("."));
}
void ZCSFX::save_sound(char const* filepath) const
{
	if (!sound_was_installed)
		throw zcsfx_io_exception(filepath, true).add_msg("Sound is disabled, cannot save sound files!");
	if (!internal)
		throw zcsfx_io_exception(filepath, true).add_msg("Can't save an empty sound!");
	internal->save_sound(filepath);
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

SampleType sfx_get_type(int32_t index)
{
	if (unsigned(index-1) >= quest_sounds.size())
		return SMPL_INVALID;
	ZCSFX const& s = quest_sounds[index-1];
	return s.get_sample_type();
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

static void sfx_replay_comment(string sfx_name)
{
#ifdef IS_PLAYER
	if (replay_is_debug())
	{
		// TODO(replays): get rid of this bandaid next time replays are mass-updated.
		if (sfx_name == "Hero is hit")
			sfx_name = "Player is hit";
		else if (sfx_name == "Hero dies")
			sfx_name = "Player dies";
		replay_step_comment(fmt::format("sfx {}", sfx_name));
	}
#endif
}

// plays an sfx sample, with specified settings
void sfx_ex(int32_t index, int32_t pan, bool loop, bool restart, zfix vol_perc, int32_t freq)
{
	if (unsigned(index-1) >= quest_sounds.size())
		return;
	ZCSFX& sound = quest_sounds[index-1];
	if (sound_was_installed)
		sound.play(pan, loop, restart, vol_perc, freq);
	if (restart)
		sfx_replay_comment(sound.sfx_name);
}

bool sfx_is_allocated(int32_t index)
{
	if (!sound_was_installed)
		return false;
	if (unsigned(index-1) >= quest_sounds.size())
		return false;
	return quest_sounds[index-1].is_allocated();
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

