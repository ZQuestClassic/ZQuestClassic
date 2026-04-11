#ifndef ZC_SCRIPTING_DEBUG_H_
#define ZC_SCRIPTING_DEBUG_H_

#include <cstdint>

int32_t debug_get_register(int32_t reg);
void debug_set_register(int32_t reg, int32_t value);

#endif
