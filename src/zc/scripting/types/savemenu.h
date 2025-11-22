#pragma once

#include "base/ints.h"
#include "zc/ffscript.h"
#include <optional>

SaveMenu* checkSaveMenu(int32_t ref, bool skipError = false);

std::optional<int32_t> savemenu_get_register(int32_t reg);
bool savemenu_set_register(int32_t reg, int32_t value);
std::optional<int32_t> savemenu_run_command(word command);
