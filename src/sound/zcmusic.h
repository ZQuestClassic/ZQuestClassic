/* ZQuest Classic Music Library        	*/
/* ------------------------------------ */
/* Wrapper for transparently extending	*/
/* supported music file formats.      	*/

#pragma once

#include <cstdint>
#include <string>
#include <iostream>
#include <memory>
#include <atomic>
#include <algorithm>

#ifdef __EMSCRIPTEN__
#include <SDL2/SDL_mixer.h>
#endif

namespace zcmusic {
    enum class ZCMF_TYPE : int32_t {
        NONE,
        DUH,
        OGG,
        MP3,
        GME
    };

    enum class ZCM_PLAY_STATUS {
        PLAYING,
        STOPPED,
        PAUSED
    };

    constexpr std::string_view zcmusic_types = "it;mod;mp3;ogg;s3m;spc;gym;nsf;gbs;vgm;xm";

    class volume_t {
        int32_t vol;

    public:
        constexpr explicit volume_t(int32_t vol) : vol{std::clamp(vol, 0, 255)} {
        }

        constexpr explicit operator int32_t() const { return vol; }
        [[nodiscard]] constexpr float as_float() const { return static_cast<float>(vol); }

        constexpr volume_t operator*(float scale) const {
            return volume_t{static_cast<int32_t>(scale * static_cast<float>(vol))};
        }

        constexpr volume_t operator-(volume_t rhs) const {
            return volume_t{vol - rhs.vol};
        }
    };

    using track_number_t = int32_t;
    using cursor_pos_t = int32_t;

    // class ZCM_DLL_MODE ZCMUSIC
    class ZCMUSIC {
        static inline std::atomic<int32_t> cnt = 0;

    protected:
        cursor_pos_t position{0}; // Only needed to sync Triforce jingle

    public:
        constexpr static int32_t INT_ERROR_RESULT = -10000;
        ZCM_PLAY_STATUS playing_status = ZCM_PLAY_STATUS::STOPPED; // -1 = paused, 0 = stopped, 1 = playing
        bool has_played = false;

        const ZCMF_TYPE type; // uses ZCMF defines
        const std::string filename;
        const int32_t id = cnt++;


        int32_t track = 0;
        int32_t fadevolume{10000};
        int32_t fadeoutframes = 0;
#ifdef __EMSCRIPTEN__
        Mix_Music *mus;
#endif

    protected:
        volume_t vol{0};
        virtual bool unload_file() = 0;

    public:
        virtual ~ZCMUSIC();

        [[nodiscard]] constexpr ZCM_PLAY_STATUS get_play_status() const { return playing_status; }
        [[nodiscard]] constexpr volume_t get_volume() const { return vol; }

        [[maybe_unused]] virtual bool poll() const = 0;

        /**
         * The libraries require polling of individual streams, so here we must keep record of each file which is
         * playing, so we can iterate over all of them when zcmusic_poll() is called.
         *
         * In addtion, any music library which actually has a 'play' function or similar will be called from here.
         *
         * @param vol the volume level that music file will be playing at.
         * @returns If this music object is playing.
         */
        virtual bool play(volume_t vol);

        virtual bool stop();

        virtual bool pause();

        virtual bool resume();

        virtual bool toggle();

        virtual bool set_volume(const volume_t vol);

        // Track Functions.
        [[nodiscard]] virtual track_number_t get_tracks() const;

        [[nodiscard]] virtual std::string_view get_track_name(track_number_t track_number) const;

        virtual track_number_t change_track(track_number_t track_number);

        // Cursor Functions.
        [[nodiscard]] virtual cursor_pos_t get_curpos() const;

        [[nodiscard]] virtual cursor_pos_t get_length() const;

        virtual bool set_curpos(cursor_pos_t value);

        virtual bool set_speed(int32_t value);

        virtual bool set_loop(double start, double end);

        friend std::ostream &operator<<(std::ostream &os, const ZCMUSIC &zcm);

    protected:
        ZCMUSIC(std::string_view fileName, ZCMF_TYPE type);
    };

    using ZCMUSIC_owner_ptr_t = std::unique_ptr<ZCMUSIC>;

    ZCMUSIC_owner_ptr_t load_for_quest(std::string_view filename, std::string_view quest_path = "");

    bool init();

    bool exit();
} // namespace zcmusic


std::ostream &print(const zcmusic::ZCMUSIC *zcm, std::ostream &os = std::cout);
