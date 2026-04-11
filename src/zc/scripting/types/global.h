#ifndef ZC_SCRIPTING_GLOBAL_H_
#define ZC_SCRIPTING_GLOBAL_H_

#include <cstdint>

int32_t global_get_register(int32_t reg);
void global_set_register(int32_t reg, int32_t value);

#endif
