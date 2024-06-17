#pragma once

#include <cstdint>
#include <string_view>
#include <sound/zcmusic.h>

namespace playback {
    bool try_zcmusic(std::string_view filename, std::string_view path, zcmusic::track_number_t track, int32_t midi, zcmusic::volume_t vol,
                     int32_t fadeoutframes = 0);

    bool play_enh_music_crossfade(std::string_view name, std::string_view path, zcmusic::track_number_t track, zcmusic::volume_t vol,
                                  int32_t fadeinframes, int32_t fadeoutframes, int32_t fademiddleframes = 0,
                                  int32_t startpos = 0, bool revertonfail = false);
} // namespace playback