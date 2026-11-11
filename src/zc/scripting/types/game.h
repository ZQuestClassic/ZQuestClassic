#pragma once

#include "base/ints.h"

#include <optional>

int32_t game_get_register(int32_t reg);
void game_set_register(int32_t reg, int32_t value);
std::optional<int32_t> game_run_command(word command);
