#pragma once

#include "base/ints.h"
#include "core/weapon_data.h"

#include <optional>

weapon_data* checkWeaponData(int32_t ref, bool skipError = false);

int32_t weapondata_get_register(int32_t reg);
void weapondata_set_register(int32_t reg, int32_t value);
std::optional<int32_t> weapondata_run_command(word command);
