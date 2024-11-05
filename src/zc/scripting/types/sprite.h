#ifndef ZC_SCRIPTING_SPRITE_H_
#define ZC_SCRIPTING_SPRITE_H_

#include <cstdint>
#include <optional>

std::optional<int32_t> sprite_get_register(int32_t reg);
bool sprite_set_register(int32_t reg, int32_t value);

#endif
