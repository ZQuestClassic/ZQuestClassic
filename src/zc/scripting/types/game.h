#ifndef ZC_SCRIPTING_GAME_H_
#define ZC_SCRIPTING_GAME_H_

#include "base/ints.h"

#include <optional>

std::optional<int32_t> game_get_register(int32_t reg);
bool game_set_register(int32_t reg, int32_t value);
std::optional<int32_t> game_run_command(word command);

#endif
