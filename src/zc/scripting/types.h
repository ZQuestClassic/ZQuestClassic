#ifndef ZC_SCRIPTING_SCRIPTING_ENGINE_H_
#define ZC_SCRIPTING_SCRIPTING_ENGINE_H_

#include "base/compiler.h"
#include "base/ints.h"
#include "zc/scripting/types/npc.h"
#include "zc/scripting/types/user_object.h"
#include "zc/scripting/types/websocket.h"

#include <optional>

ZC_FORCE_INLINE std::optional<int32_t> scripting_engine_get_register(int32_t reg)
{
	if (auto r = npc_get_register(reg))
		return *r;
	if (auto r = websocket_get_register(reg))
		return *r;

	return std::nullopt;
}

ZC_FORCE_INLINE bool scripting_engine_set_register(int32_t reg, int32_t value)
{
	if (npc_set_register(reg, value))
		return true;

	return false;
}

ZC_FORCE_INLINE std::optional<int32_t> scripting_engine_run_command(word command)
{
	if (auto r = user_object_run_command(command))
		return *r;
	if (auto r = npc_run_command(command))
		return *r;
	if (auto r = websocket_run_command(command))
		return *r;

	return std::nullopt;
}

#endif
