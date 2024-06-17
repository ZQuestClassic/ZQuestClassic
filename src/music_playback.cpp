#include "music_playback.h"
#include <sound/zcmusic.h>
#include <sound/zcmixer.h>
#include <sound/play_midi.h>
// #include "base/qrs.h"
#include "zc/zc_sys.h"

#include <iostream>
// #include <base/zc_math.h>

namespace playback
{
    // Run an NSF, or a MIDI if the NSF is missing somehow.
    bool try_zcmusic(const std::string_view filename, const std::string_view path, const zcmusic::track_number_t track,
                     const int32_t midi, const zcmusic::volume_t vol, const int32_t fadeoutframes)
    {
        std::cout << "try_zcmusic\nfilename: " << filename << "\npath: " << path << "\nvol: " << static_cast<int32_t>(vol)
                  << "\n";

        auto newzcmusic{zcmusic::load_for_quest(filename, path)};

        // Found it
        if (newzcmusic)
        {
            newzcmusic->fadevolume = 10000;
            newzcmusic->fadeoutframes = fadeoutframes;

            g_zcmixer->newtrack = std::move(newzcmusic);

            auto &current_track{g_zcmixer->current_track};
            current_track.reset();
            midi::play_midi::stop();

            current_track = std::move(g_zcmixer->newtrack);
            std::cout << "current_track: " << *g_zcmixer << "\n";
            current_track->play(vol);

            if (track > 0)
            {
                current_track->change_track(track);
            }

            return true;
        }
#ifndef IS_EDITOR
        // Not found, play MIDI - unless this was called by a script (yay, magic numbers)
        else if (midi > -1000)
            jukebox(midi);
#endif

        return false;
    }

    bool play_enh_music_crossfade(const std::string_view name, const std::string_view path,
                                  const zcmusic::track_number_t track, const zcmusic::volume_t vol,
                                  const int32_t fadeinframes, const int32_t fadeoutframes,
                                  const int32_t fademiddleframes,
                                  const zcmusic::cursor_pos_t startpos, const bool revertonfail)
    {
        std::cout << "play_enh_music_crossfade\nfilename: " << name << "\npath: " << path << "\n";

        // TODO: remove comments
        // auto &oldtrack = zcmixer->current_track.get();

        if (name.empty())
        {
            // Pass currently playing music off to the mixer
            g_zcmixer->oldtrack = std::move(g_zcmixer->current_track);
            // Do not play new music
            g_zcmixer->newtrack.reset();

            g_zcmixer->setup_transition(fadeinframes, fadeoutframes, fademiddleframes);

            if (g_zcmixer->oldtrack)
                g_zcmixer->oldtrack->fadevolume = 10000;
            if (g_zcmixer->newtrack)
                g_zcmixer->newtrack->fadevolume = 0;

            return false;
        }
        else // Pointer to a string..

            // Pass currently playing music to the mixer
        g_zcmixer->oldtrack = std::move(g_zcmixer->current_track);
        g_zcmixer->newtrack.reset();

        // If new music was found try_zcmusic sets zcmixer->current_track
        if (try_zcmusic(name, path, track, -1000, vol, fadeoutframes))
        {
            g_zcmixer->newtrack = std::move(g_zcmixer->current_track);

            g_zcmixer->setup_transition(fadeinframes, fadeoutframes, fademiddleframes);

            if (startpos > 0)
                g_zcmixer->newtrack->set_curpos(startpos);
            if (g_zcmixer->oldtrack)
                g_zcmixer->oldtrack->fadevolume = fadeoutframes > 0 ? 10000 : 0;
            if (g_zcmixer->newtrack)
                g_zcmixer->newtrack->fadevolume = fadeinframes > 0 ? 0 : 10000;
            return true;
        }

        if (revertonfail)
        {
            // Switch back to the old music
            g_zcmixer->current_track = std::move(g_zcmixer->oldtrack);
            g_zcmixer->newtrack.reset();
        }

        // TODO: Remove comment block
        // // If there was already an old track playing, stop it
        // if (oldold) {
        //     // Don't allow it to null both tracks if running twice in a row
        //     if (zcmixer->newtrack == nullptr && zcmixer->oldtrack == nullptr) {
        //         zcmixer->oldtrack = oldold;
        //
        //         if (oldold->fadeoutframes > 0) {
        //             zcmixer->fadeoutframes = zc_max(oldold->fadeoutframes * fadeoutpct, 1);
        //             zcmixer->fadeoutmaxframes = oldold->fadeoutframes;
        //             if (zcmixer->oldtrack != nullptr)
        //                 zcmixer->oldtrack->fadevolume = 10000;
        //             oldold->fadeoutframes = 0;
        //         }
        //     } else {
        //         zcmusic::unload_file(oldold);
        //         oldold = nullptr;
        //     }
        // }

        return false;
    }
} // namespace playback
