#ifndef ZC_SCRIPTING_VIEWPORT_H_
#define ZC_SCRIPTING_VIEWPORT_H_

#include <cstdint>
#include <optional>

#include "base/ints.h"
#include "base/zfix.h"

int32_t viewport_get_register(int32_t reg);
void viewport_set_register(int32_t reg, int32_t value);
std::optional<int32_t> viewport_run_command(word command);
// Validates a viewport follow speed a script is setting (Viewport-> or dmapdata->), logging an
// error and returning nothing if it is out of range.
std::optional<zfix> validate_viewport_follow_speed(int32_t value);

#endif
