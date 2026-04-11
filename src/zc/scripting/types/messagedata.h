#ifndef ZC_SCRIPTING_MESSAGEDATA_H_
#define ZC_SCRIPTING_MESSAGEDATA_H_

#include <cstdint>

int32_t do_msgheight(int32_t msg);
int32_t do_msgwidth(int32_t msg);

int32_t messagedata_get_register(int32_t reg);
void messagedata_set_register(int32_t reg, int32_t value);

#endif
