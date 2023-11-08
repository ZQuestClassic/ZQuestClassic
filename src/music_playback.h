#ifndef _MUSIC_PLAYBACK_H_
#define _MUSIC_PLAYBACK_H_

#include <cstdint>

bool try_zcmusic(const char* filename, const char* path, int32_t track, int32_t midi, int32_t vol, int32_t fadeoutframes=0);
bool play_enh_music_crossfade(const char* name, const char* path, int32_t track, int32_t vol, int32_t fadeinframes, int32_t fadeoutframes, int32_t fademiddleframes = 0, int32_t startpos = 0, bool revertonfail = false);

#endif