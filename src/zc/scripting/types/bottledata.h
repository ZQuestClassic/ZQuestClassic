#ifndef ZC_SCRIPTING_BOTTLEDATA_H_
#define ZC_SCRIPTING_BOTTLEDATA_H_

#include "core/misctypes.h"
#include <cstdint>

bottletype* checkBottleData(int32_t ref, bool skipError = false);

int32_t bottledata_get_register(int32_t reg);
void bottledata_set_register(int32_t reg, int32_t value);

#endif
