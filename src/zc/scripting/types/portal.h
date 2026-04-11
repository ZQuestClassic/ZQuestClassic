#ifndef ZC_SCRIPTING_PORTAL_H_
#define ZC_SCRIPTING_PORTAL_H_

#include <cstdint>

class portal;

portal* checkPortal(int32_t ref, bool skiperr = false);

int32_t portal_get_register(int32_t reg);
void portal_set_register(int32_t reg, int32_t value);

#endif
