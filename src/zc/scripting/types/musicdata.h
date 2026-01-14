#pragma once

#include "base/ints.h"
#include "zc/ffscript.h"
#include "advanced_music.h"
#include <optional>

AdvancedMusic* checkMusic(int32_t ref, bool skipError = false);

std::optional<int32_t> musicdata_get_register(int32_t reg);
bool musicdata_set_register(int32_t reg, int32_t value);
std::optional<int32_t> musicdata_run_command(word command);
