#ifndef ZC_SCRIPTING_WEBSOCKET_H_
#define ZC_SCRIPTING_WEBSOCKET_H_

#include "base/ints.h"
#include <optional>

void websocket_init();
std::optional<int32_t> websocket_get_register(int32_t reg);
std::optional<int32_t> websocket_run_command(word command);

#endif
