#pragma once

#include "base/ints.h"
#include <cstdint>

void bad_subwidg_type(bool func, byte type);

int32_t subscreenwidget_get_register(int32_t reg);
void subscreenwidget_set_register(int32_t reg, int32_t value);
