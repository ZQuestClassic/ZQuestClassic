#ifndef ZC_SCRIPTING_SCREENDATA_H_
#define ZC_SCRIPTING_SCREENDATA_H_

#include <cstdint>

int32_t screendata_get_register(int32_t reg);
void screendata_set_register(int32_t reg, int32_t value);

#endif
