#ifndef ZC_SCRIPTING_AUDIO_H_
#define ZC_SCRIPTING_AUDIO_H_

#include <cstdint>

int32_t audio_get_register(int32_t reg);
void audio_set_register(int32_t reg, int32_t value);

#endif
