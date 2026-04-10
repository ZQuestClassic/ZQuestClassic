#ifndef ZC_SCRIPTING_SCRIPTING_ENGINE_H_
#define ZC_SCRIPTING_SCRIPTING_ENGINE_H_

#include "base/check.h"
#include "base/compiler.h"
#include "base/ints.h"
#include "zc/scripting/types/directory.h"
#include "zc/scripting/types/file.h"
#include "zc/scripting/types/game.h"
#include "zc/scripting/types/itemsprite.h"
#include "zc/scripting/types/musicdata.h"
#include "zc/scripting/types/npc.h"
#include "zc/scripting/types/savemenu.h"
#include "zc/scripting/types/sprite.h"
#include "zc/scripting/types/user_object.h"
#include "zc/scripting/types/websocket.h"

#include <optional>

enum class EngineSubsystem : uint8_t
{
	None,
	Directory,
	File,
	Game,
	Item,
	Music,
	Npc,
	SaveMenu,
	Sprite,
	Websocket,
};

constexpr int MAX_REGISTER_ID = NUMVARIABLES;
extern EngineSubsystem register_routing_table[MAX_REGISTER_ID + 1];

void initializeRegisterRoutingTable();

ZC_FORCE_INLINE int32_t scripting_engine_get_register(int32_t reg)
{
	if (reg < 0 || reg > MAX_REGISTER_ID)
		return 0;

	switch (register_routing_table[reg])
	{
		case EngineSubsystem::Directory: return directory_get_register(reg);
		case EngineSubsystem::File: return file_get_register(reg);
		case EngineSubsystem::Game: return game_get_register(reg);
		case EngineSubsystem::Item: return itemsprite_get_register(reg);
		case EngineSubsystem::Music: return musicdata_get_register(reg);
		case EngineSubsystem::Npc: return npc_get_register(reg);
		case EngineSubsystem::SaveMenu: return savemenu_get_register(reg);
		case EngineSubsystem::Sprite: return sprite_get_register(reg);
		case EngineSubsystem::Websocket: return websocket_get_register(reg);
    }

	NOTREACHED();
}

ZC_FORCE_INLINE void scripting_engine_set_register(int32_t reg, int32_t value)
{
	if (reg < 0 || reg > MAX_REGISTER_ID)
		return;

	switch (register_routing_table[reg])
	{
		case EngineSubsystem::Directory: directory_set_register(reg, value); return;
		case EngineSubsystem::File: file_set_register(reg, value); return;
		case EngineSubsystem::Game: game_set_register(reg, value); return;
		case EngineSubsystem::Item: itemsprite_set_register(reg, value); return;
		case EngineSubsystem::Music: musicdata_set_register(reg, value); return;
		case EngineSubsystem::Npc: npc_set_register(reg, value); return;
		case EngineSubsystem::SaveMenu: savemenu_set_register(reg, value); return;
		case EngineSubsystem::Sprite: sprite_set_register(reg, value); return;
		case EngineSubsystem::Websocket: websocket_set_register(reg, value); return;
    }

	NOTREACHED();
}

ZC_FORCE_INLINE std::optional<int32_t> scripting_engine_run_command(word command)
{
	if (auto r = user_object_run_command(command))
		return *r;
	if (auto r = itemsprite_run_command(command))
		return *r;
	if (auto r = game_run_command(command))
		return *r;
	if (auto r = npc_run_command(command))
		return *r;
	if (auto r = file_run_command(command))
		return *r;
	if (auto r = savemenu_run_command(command))
		return *r;
	if (auto r = directory_run_command(command))
		return *r;
	if (auto r = websocket_run_command(command))
		return *r;
	if (auto r = musicdata_run_command(command))
		return *r;

	return std::nullopt;
}

#endif
