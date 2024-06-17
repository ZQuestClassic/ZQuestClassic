#pragma once

#include <base/zdefs.h>
#include <string_view>

namespace midi
{
    /*  Allegro MIDI struct

      typedef struct MIDI                    // a midi file
      {
      int32_t divisions;                      // number of ticks per quarter note
      struct {
      uint8_t *data;             // MIDI message stream
      int32_t len;                         // length of the track data
      } track[MIDI_TRACKS];
      } MIDI;

      */

    /* ---  All this code just to calculate the length of a MIDI song.  --- */

    constexpr std::size_t MAX_TEMPO_CHANGES = 512;

    struct midi_info
    {
        // midi info
        int32_t format = 0;
        std::size_t num_tracks = 0;
        int32_t divisions = 0;
        int32_t len_beats = 0;
        double len_sec = 0;
        int32_t tempo_changes = 0;
        std::array<double, MAX_TEMPO_CHANGES> tempo = {}; // tempo can change during song
        std::array<dword, MAX_TEMPO_CHANGES> tempo_c = {}; // store the total delta time before each change in tempo

        // MTrk event info (used by parse_mtrk())
        dword dt{}, nbytes{};
        byte event{}, running_status{}, type{}, byte2{};
        byte* buf{};

        constexpr midi_info() = default;
        explicit midi_info(const MIDI* midi) noexcept;
    };

    int32_t save_midi(std::string_view filename, const MIDI* midi);
    //dword getval(const byte* buf, int32_t nbytes);
    char* timestr(double sec);
} // namespace midi
