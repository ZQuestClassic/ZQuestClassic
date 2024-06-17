/* ZQuest Classic Music Library         */
/* ------------------------------------*/
/* Wrapper for transparently extending */
/* supported music file formats.       */

#include "base/zc_alleg.h" // TODO: why do we get "_malloca macro redefinition" in Windows debug builds without this include?



#include <cstring>

#ifdef _DEBUG
#ifdef _malloca
#undef _malloca
#endif
#endif

#include <aldumb.h>
#ifdef SOUND_LIBS_BUILT_FROM_SOURCE
#include <gme.h>
#endif

#include "base/zsys.h"
#include "base/util.h"
#include <sound/zcmusic.h>
#include <filesystem>
#include <stdlib.h>
#include <allegro5/allegro_audio.h>
#include <vector>
#include <unordered_set>
#include <source_location>

#ifdef __EMSCRIPTEN__
#include "base/emscripten_utils.h"
#endif

using namespace util;

#ifndef SOUND_LIBS_BUILT_FROM_SOURCE
// short of fixing gme, these warnings will always be there...
#pragma warning(disable : 4512) // assignment operator could not be generated
#pragma warning(disable : 4100) // unreferenced formal parameter
#include "Nsf_Emu.h"
#include "Gbs_Emu.h"
#include "Spc_Emu.h"
#include "Vgm_Emu.h"
#include "Gym_Emu.h"
#pragma warning(default : 4100)
#pragma warning(default : 4512)
#endif

#ifdef _MSC_VER
#define stricmp _stricmp
#endif

// MSVC fix
#if _MSC_VER >= 1900
namespace
{
	FILE _iob[] = {*stdin, *stdout, *stderr};
	FILE *__cdecl __iob_func(void) { return _iob; }
} // namespace
#endif

namespace {
    constexpr int32_t DUH_SAMPLES_IN_HZ = 44100;
    constexpr int32_t DUH_CHANNELS = 2; // stereo
    constexpr int32_t DUH_RESAMPLE = 1;

    ALLEGRO_MUTEX *playlistmutex = nullptr;
} // namespace

namespace zcmusic {
    namespace {
        void log(const ZCMUSIC &zcm, const std::source_location location = std::source_location::current()) {

#ifdef DEBUGMODE
            std::cout << "file: "
                    << location.file_name() << '('
                    << location.line() << ':'
                    << location.column() << ") `"
                    << location.function_name() << "`:\nzcm: "
                    << zcm << '\n';
#endif
        }

        std::string_view get_extension(std::string_view filename) {
            const auto start_index = filename.rfind('.');
            if (start_index == std::string_view::npos)
                // ReSharper disable once CppDFALocalValueEscapesFunction
                return filename;
            return filename.substr(start_index + 1L);
        }

        std::string_view extract_name(std::string_view path) {
            auto start_index = path.rfind('/');
            if (start_index == std::string_view::npos) {
                start_index = path.rfind('\\');
            }
            if (start_index == std::string_view::npos)
                // ReSharper disable once CppDFALocalValueEscapesFunction
                return path;

            return path.substr(start_index + 1L);
        }

        class DUHFILE : public ZCMUSIC {
        private:
            DUH *stream = nullptr;
            AL_DUH_PLAYER *player = nullptr;

        public:
            DUHFILE(std::string_view fileName, DUH *stream) : ZCMUSIC{fileName, ZCMF_TYPE::DUH}, stream{stream} {
            }

            ~DUHFILE() override { DUHFILE::unload_file(); }

            [[maybe_unused]] bool poll() const override {
                if (!player)
                    return false;

                al_poll_duh(player);
                return true;
            }

            bool unload_file() override {
                stop();
                if (stream != nullptr) {
                    unload_duh(stream);
                    stream = nullptr;
                }
                return true;
            }

            bool stop() override {
                if (playing_status == ZCM_PLAY_STATUS::STOPPED) return true;
                if (player == nullptr)
                    return false;
                al_stop_duh(player);
                player = nullptr;
                return ZCMUSIC::stop();
            }

            bool play(const volume_t vol) override {
                if (playing_status == ZCM_PLAY_STATUS::PLAYING) return true;
                if (stream == nullptr)
                    return false;

                // Set the buffer to 128KB, which should be ~1.5s of audio.
                static constexpr long bufsize = 128 * 1024;
                player = al_start_duh(stream, DUH_CHANNELS, 0 /*pos*/, (vol.as_float()) / 255.0f, bufsize,
                                      DUH_SAMPLES_IN_HZ);
                return player && ZCMUSIC::play(vol);
            }

            bool pause() override {
                if (playing_status == ZCM_PLAY_STATUS::PAUSED) return true;
                if (playing_status == ZCM_PLAY_STATUS::STOPPED) return false;
                if (player == nullptr)
                    return false;
                al_pause_duh(player);
                return ZCMUSIC::pause();;
            };

            bool resume() override {
                if (playing_status == ZCM_PLAY_STATUS::PLAYING) return true;
                if (playing_status == ZCM_PLAY_STATUS::STOPPED) return false;
                if (player == nullptr)
                    return false;
                al_resume_duh(player);
                return ZCMUSIC::resume();
            };

            bool set_volume(volume_t vol) override {
                if (player == nullptr)
                    return false;

                al_duh_set_volume(player, vol.as_float() / 255.0f);
                return ZCMUSIC::set_volume(vol);
            }
        };

        // used for allegro streamed music (ogg, mp3)
        struct ALSTREAMFILE : public ZCMUSIC {
            const std::string fname;

        private:
            ALLEGRO_AUDIO_STREAM *stream = nullptr;

        public:
            ALSTREAMFILE(std::string_view fileName, ZCMF_TYPE type) : ZCMUSIC{fileName, type}, fname{fileName} {
                ALLEGRO_AUDIO_STREAM *const stream = al_load_audio_stream(fname.c_str(), 4, 2048);
                if (!stream)
                    return;

                if (!al_attach_audio_stream_to_mixer(stream, al_get_default_mixer())) {
                    al_destroy_audio_stream(stream);
                    return;
                }

                this->stream = stream;

                al_reserve_samples(5);

                al_set_audio_stream_playing(stream, false);
                al_set_audio_stream_playmode(stream, ALLEGRO_PLAYMODE_LOOP);
            }

            ~ALSTREAMFILE() override { ALSTREAMFILE::unload_file(); }

            bool unload_file() override {
                log(*this);
                if (!stop())
                    return false;

                al_destroy_audio_stream(stream);
                stream = nullptr;
                return true;
            }

            [[maybe_unused]] bool poll() const override {
                //log(*this);
                return true;
            }

            bool stop() override {
                log(*this);
                if (playing_status == ZCM_PLAY_STATUS::STOPPED) return true;
                if (!pause())
                    return false;

                al_seek_audio_stream_secs(stream, 0.0);
                return ZCMUSIC::stop();
            }

            bool play(const volume_t vol) override {
                log(*this);
                if (playing_status == ZCM_PLAY_STATUS::PLAYING) return true;
                if (stream == nullptr)
                    return false;
                if (!set_volume(vol))
                    return false;
                set_curpos(0);
                return al_set_audio_stream_playing(stream, true) && ZCMUSIC::play(vol);
            }

            bool pause() override {
                log(*this);
                if (playing_status == ZCM_PLAY_STATUS::PAUSED) return true;
                if (playing_status == ZCM_PLAY_STATUS::STOPPED) return false;
                if (stream == nullptr)
                    return false;
                return al_set_audio_stream_playing(stream, true) && ZCMUSIC::pause();;
                //return al_set_audio_stream_playing(stream, false) && ZCMUSIC::pause();;
            };

            bool resume() override {
                log(*this);
                if (playing_status == ZCM_PLAY_STATUS::PLAYING) return true;
                if (stream == nullptr)
                    return false;
                return al_set_audio_stream_gain(stream, vol.as_float() / 255.0f)
                    && al_set_audio_stream_playing(stream, true) && ZCMUSIC::resume();
            };

            bool set_volume(const volume_t vol) override {
                log(*this);
                if (stream == nullptr)
                    return false;
                return al_set_audio_stream_gain(stream, vol.as_float() / 255.0f)
                       && ZCMUSIC::set_volume(vol);
            }

            [[nodiscard]] cursor_pos_t get_curpos() const override {
                if (stream == nullptr)
                    return INT_ERROR_RESULT;

                return static_cast<cursor_pos_t>(al_get_audio_stream_position_secs(stream) * 10000.0);
            }

            bool set_curpos(const cursor_pos_t msecs) override {
                // log(*this);
                if (stream == nullptr)
                    return false;
                return al_seek_audio_stream_secs(stream, double(msecs) / 10000.0)
                       && ZCMUSIC::set_curpos(msecs);
            }

            bool set_speed(const int32_t speed) override {
                log(*this);
                if (stream == nullptr)
                    return false;
                return al_set_audio_stream_speed(stream, float(speed / 10000.0));
            }

            [[nodiscard]] cursor_pos_t get_length() const override {
                log(*this);
                if (stream == nullptr)
                    return ZCMUSIC::INT_ERROR_RESULT;
                return cursor_pos_t(al_get_audio_stream_length_secs(stream) * 10000.0);
            }

            bool set_loop(double start, double end) override {
                log(*this);
                if (stream == nullptr)
                    return false;

                // No loop set
                if (start == 0.0 && end == 0.0) {
                    return false;
                }
                // Don't allow end point before start point
                if (end < start) {
                    end = double(get_length() / 10000.0);
                }
                return al_set_audio_stream_loop_secs(stream, start, end);
            }
        };

        struct GMEFILE : public ZCMUSIC {
        private:
            AUDIOSTREAM *stream = nullptr;
            Music_Emu *emu = nullptr;
            int32_t samples{};

        public:
            GMEFILE(std::string_view fileName, Music_Emu *emu) : ZCMUSIC{fileName, ZCMF_TYPE::GME}, emu{emu} {
            }

            ~GMEFILE() override { GMEFILE::unload_file(); }

            [[maybe_unused]] bool poll() const override {
                constexpr static auto mask = static_cast<int16_t>(0x8000);

                if (!emu)
                    return false;

                auto const audio_buffer = static_cast<int16_t *>(get_audio_stream_buffer(stream));
                if (!audio_buffer)
                    return false;

                const int32_t samples2 = this->samples * 2;
                memset(audio_buffer, 0, samples2);
                ::gme_play(emu, samples2, audio_buffer);

                // Allegro only uses UNSIGNED samples ...
                auto audio_buffer_it = audio_buffer;
                for (int32_t j = 0; j < samples2; ++j) {
                    *audio_buffer_it ^= mask;
                    audio_buffer_it++;
                }

                free_audio_stream_buffer(stream);
                return true;
            }

            bool unload_file() override {
                if (emu == nullptr)
                    return false;

                stop();
                gme_delete(emu);

                return true;
            }

            bool stop() override {
                if (playing_status == ZCM_PLAY_STATUS::STOPPED) return true;

                if (emu == nullptr)
                    return false;

                stop_audio_stream(stream);

                return ZCMUSIC::stop();
            }

            bool play(const volume_t vol) override {
                if (playing_status == ZCM_PLAY_STATUS::PLAYING) return true;
                if (emu == nullptr)
                    return false;

                gme_start_track(emu, 0);
                int32_t local_samples = 512;
                int32_t buf_size = 2 * DUH_SAMPLES_IN_HZ / 50;

                while (local_samples < buf_size)
                    local_samples *= 2;

                samples = local_samples;

                if (playing_status != ZCM_PLAY_STATUS::STOPPED)
                    stop_audio_stream(stream);

                stream = play_audio_stream(samples, 16, TRUE, DUH_SAMPLES_IN_HZ, static_cast<int>(vol), 128);
                return stream && ZCMUSIC::play(vol);
            }

            bool pause() override {
                if (playing_status == ZCM_PLAY_STATUS::PAUSED) return true;
                if (playing_status == ZCM_PLAY_STATUS::STOPPED) return false;
                if (emu == nullptr)
                    return false;
                voice_stop(stream->voice);
                return ZCMUSIC::pause();;
            };

            bool resume() override {
                if (playing_status == ZCM_PLAY_STATUS::PLAYING) return true;
                if (playing_status == ZCM_PLAY_STATUS::STOPPED) return false;
                if (emu == nullptr)
                    return false;
                voice_start(stream->voice);
                return ZCMUSIC::resume();
            };

            bool set_volume(const volume_t vol) override {
                if (stream == nullptr)
                    return false;
                voice_set_volume(stream->voice, static_cast<int32_t>(vol));
                return ZCMUSIC::set_volume(vol);
            }

            track_number_t change_track(track_number_t track_number) override {
                if (emu == nullptr)
                    return -1;

                const track_number_t num_of_tracks = gme_track_count(emu);

                if (track_number < 0 || track_number >= num_of_tracks) {
                    track_number = 0;
                }

                gme_start_track(emu, track_number);

                track = track_number;
                return track_number;
            }

            [[nodiscard]] track_number_t get_tracks() const override {
                if (emu == nullptr)
                    return 0;

                const track_number_t track_count = gme_track_count(emu);
                return (track_count > 0) ? track_count : 0;
            }
        };

        //============================================================================================================

        std::unordered_set<ZCMF_TYPE> supported_media_types;
        bool is_media_type_supported(ZCMF_TYPE type) noexcept { return supported_media_types.contains(type); }

        Music_Emu *gme_load_file(const std::string_view filename, std::string_view ext) {
            // TODO
            Music_Emu *emu = nullptr;
            if (gme_open_file(filename.data(), &emu, DUH_SAMPLES_IN_HZ)) {
                gme_delete(emu);
                return nullptr;
            }

            gme_set_autoload_playback_limit(emu, false);

            return emu;
        }

        ZCMUSIC *load_file(std::string_view filename) {
            std::cout << "load_file\nfilename: " << filename << "\n";
            al_trace("Trying to load %s\n", filename.data());
#ifdef __EMSCRIPTEN__
			if (em_is_lazy_file(filename))
			{
				em_fetch_file(filename);
			}
#endif

            const auto ext = get_extension(filename);
            if (ext.empty())
                return nullptr;

            if (ext == "ogg") {
                if (!is_media_type_supported(ZCMF_TYPE::OGG))
                    return nullptr;
                return new ALSTREAMFILE(filename, ZCMF_TYPE::OGG);
            }

            if ((ext == "mp3")) {
                if (!zcmusic::is_media_type_supported(ZCMF_TYPE::MP3))
                    return nullptr;
                return new ALSTREAMFILE(filename, ZCMF_TYPE::MP3);
            }

            if (ext == "spc" || ext == "gbs" || ext == "vgm" || ext == "gym" || ext == "nsf") {
                if (!is_media_type_supported(ZCMF_TYPE::GME))
                    return nullptr;

                Music_Emu *const emu = gme_load_file(std::string(filename).c_str(), ext);

                if (emu)
                    return new GMEFILE(filename, emu);
                else
                    al_trace("%s file '%s' not found.\n", ext.data(), filename.data());
                return nullptr;
            }

            if (is_media_type_supported(ZCMF_TYPE::DUH)) {
                DUH *stream = nullptr;

                const char *c_filename = filename.data();
                if (ext == "it") {
                    stream = dumb_load_it(c_filename);

                    if (!stream)
                        al_trace("IT file '%s' not found.\n", c_filename);
                } else if (ext == "xm") {
                    stream = dumb_load_xm(c_filename);

                    if (!stream)
                        al_trace("XM file '%s' not found.\n", c_filename);
                } else if (ext == "s3m") {
                    stream = dumb_load_s3m(c_filename);

                    if (!stream)
                        al_trace("S3M file '%s' not found.\n", c_filename);
                } else if (ext == "mod") {
#ifdef SOUND_LIBS_BUILT_FROM_SOURCE
                    // No idea what this second arg does...
                    stream = dumb_load_mod(c_filename, 0);
#else
					stream = dumb_load_mod(c_filename);
#endif

                    if (!stream)
                        al_trace("MOD file '%s' not found.\n", c_filename);
                }

                if (stream)
                    return new DUHFILE(filename, stream);
                return nullptr;
            }

            return nullptr;
        }

        ZCMUSIC *load_for_quest_for_file(const std::filesystem::path &fullFilename) {
            std::cout << "zcmusic::load_for_quest_for_file: starting with fullFilename: " << fullFilename << "\n";
            if (!std::filesystem::exists(fullFilename))
                return nullptr;

            return load_file(fullFilename.string());
        }

        //============================================================================================================
    } // namespace

    //============================================================================================================

    ZCMUSIC::~ZCMUSIC()
    = default;

    ZCMUSIC::ZCMUSIC(std::string_view fileName,
                     const ZCMF_TYPE type) : type{type}, filename{zcmusic::extract_name(fileName)} {
    }

    track_number_t ZCMUSIC::change_track(track_number_t track_number) { return 0; }

    track_number_t ZCMUSIC::get_tracks() const { return 0; }

    cursor_pos_t ZCMUSIC::get_curpos() const { return position; }

    bool ZCMUSIC::play(volume_t vol) {
        log(*this);
        position = 0;
        playing_status = ZCM_PLAY_STATUS::PLAYING;
        has_played = true;
        return true;
    }

    bool ZCMUSIC::stop() {
        log(*this);
        position = 0;
        playing_status = ZCM_PLAY_STATUS::STOPPED;
        return true;
    }

    bool ZCMUSIC::pause() {
        log(*this);
        playing_status = ZCM_PLAY_STATUS::PAUSED;
        return true;
    }

    bool ZCMUSIC::resume() {
        log(*this);
        playing_status = ZCM_PLAY_STATUS::PLAYING;
        return true;
    }

    bool ZCMUSIC::toggle() {
        log(*this);
        if (playing_status == ZCM_PLAY_STATUS::STOPPED) return false;
        const ZCM_PLAY_STATUS new_play_status = (playing_status == ZCM_PLAY_STATUS::PAUSED)
                                                    ? ZCM_PLAY_STATUS::PLAYING
                                                    : ZCM_PLAY_STATUS::PAUSED;

        // if the state has actually changed
        if (new_play_status == playing_status) return false;

        if (new_play_status != ZCM_PLAY_STATUS::PAUSED) {
            return pause();
        }
        return resume();
    }

    bool ZCMUSIC::set_volume(const volume_t vol) {
        this->vol = vol;
        return true;
    };


    bool ZCMUSIC::set_curpos(const cursor_pos_t value) {
        position = value;
        return true;
    }

    cursor_pos_t ZCMUSIC::get_length() const { return ZCMUSIC::INT_ERROR_RESULT; }

    bool ZCMUSIC::set_speed(int32_t value) { return false; }

    bool ZCMUSIC::set_loop(double start, double end) { return false; }

    [[nodiscard]] std::string_view ZCMUSIC::get_track_name(const track_number_t track_number) const {
        return "";
    }

    std::ostream &operator<<(std::ostream &os, const ZCMF_TYPE type) {
        switch (type) {
            case ZCMF_TYPE::DUH:
                return os << "DUH";
            case ZCMF_TYPE::GME:
                return os << "GME";
            case ZCMF_TYPE::MP3:
                return os << "MP3";
            case ZCMF_TYPE::OGG:
                return os << "OGG";
            case ZCMF_TYPE::NONE:
                return os << "NONE";
            default:
                break;
        }
        return os;
    }

    std::ostream &operator<<(std::ostream &os, const ZCM_PLAY_STATUS status) {
        switch (status) {
            case ZCM_PLAY_STATUS::PAUSED:
                return os << "Paused";
            case ZCM_PLAY_STATUS::PLAYING:
                return os << "Playing";
            case ZCM_PLAY_STATUS::STOPPED:
                return os << "Stopped";
            default:
                break;
        }
        return os;
    }

    std::ostream &operator<<(std::ostream &os, const ZCMUSIC &zcm) {
        return os << "ZCMUSIC: ("
               << "\n\tid: " << zcm.id << ", "
               << "\n\ttype: " << zcm.type << ", "
               << "\n\tplaying: " << zcm.playing_status << ", "
               << "\n\thas_played: " << zcm.has_played << ", "
               << "\n\tvolume: " << static_cast<int>(zcm.vol) << ","
               << "\n\tposition: " << zcm.position << ", "
               << "\n\tcurrent_position: " << zcm.get_curpos() << ", "
               << "\n\tfilename: \"" << zcm.filename << "\", "
               << "\n\ttrack: " << zcm.track << ", "
               << "\n\tfadevolume: " << zcm.fadevolume << ", "
               << "\n\tfadeoutframes: " << zcm.fadeoutframes
               << "\n)";
    }

    ZCMUSIC_owner_ptr_t load_for_quest(std::string_view filename, std::string_view quest_path) {
        std::cout << "zcmusic_load_for_quest filename: " << filename << "\nquest_path: " << quest_path << "\n";
        if (!al_is_audio_installed()) {
            std::cout << "zcmusic_load_for_quest !al_is_audio_installed()\n";
            return {};
        }
        char exe_path[PATH_MAX];
        get_executable_name(exe_path, PATH_MAX);
        const auto exe_dir = std::filesystem::path(exe_path).parent_path();
        const auto quest_dir = std::filesystem::path(quest_path).parent_path(); {
            const auto newzcmusic = load_for_quest_for_file(exe_dir / filename);
            if (newzcmusic)
                return ZCMUSIC_owner_ptr_t{newzcmusic};
        } {
            const auto newzcmusic = load_for_quest_for_file(quest_dir / filename);
            if (newzcmusic)
                return ZCMUSIC_owner_ptr_t{newzcmusic};
        } {
            std::filesystem::path dir = exe_dir / std::filesystem::path(quest_path).filename();
            dir += "_music";

            const auto newzcmusic = load_for_quest_for_file(dir / filename);
            if (newzcmusic)
                return ZCMUSIC_owner_ptr_t{newzcmusic};
        } {
            const auto newzcmusic = load_for_quest_for_file(exe_dir / "music" / filename);
            if (newzcmusic)
                return ZCMUSIC_owner_ptr_t{newzcmusic};
        } {
            std::filesystem::path dir = quest_dir / std::filesystem::path(quest_path).filename();
            dir += "_music";

            const auto newzcmusic = load_for_quest_for_file(dir / filename);
            if (newzcmusic)
                return ZCMUSIC_owner_ptr_t{newzcmusic};
        } {
            const auto newzcmusic = load_for_quest_for_file(quest_dir / "music" / filename);
            if (newzcmusic)
                return ZCMUSIC_owner_ptr_t{newzcmusic};
        }

        return ZCMUSIC_owner_ptr_t{};
    }

    //============================================================================================================

    bool init() {
        dumb_register_stdfiles();
        dumb_resampling_quality = DUH_RESAMPLE;
        auto &supported_media_types = zcmusic::supported_media_types;
        supported_media_types.insert(ZCMF_TYPE::DUH);
        supported_media_types.insert(ZCMF_TYPE::OGG);
        supported_media_types.insert(ZCMF_TYPE::MP3);
        supported_media_types.insert(ZCMF_TYPE::GME);

        if (!playlistmutex)
            playlistmutex = al_create_mutex();
        return true;
    }

    bool exit() {
        if (is_media_type_supported(ZCMF_TYPE::DUH)) {
            dumb_exit();
        }
        supported_media_types.clear();
        return true;
    }
} // namespace zcmusic

std::ostream &print(const zcmusic::ZCMUSIC *zcm, std::ostream &os) {
    if (zcm) return os << *zcm;
    return os << "nullptr";
}
