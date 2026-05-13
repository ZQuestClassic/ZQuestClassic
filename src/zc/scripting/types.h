#ifndef ZC_SCRIPTING_SCRIPTING_ENGINE_H_
#define ZC_SCRIPTING_SCRIPTING_ENGINE_H_

#include "base/check.h"
#include "base/compiler.h"
#include "base/ints.h"
#include "components/zasm/defines.h"
#include "zc/scripting/types/audio.h"
#include "zc/scripting/types/bitmap.h"
#include "zc/scripting/types/bottledata.h"
#include "zc/scripting/types/combodata.h"
#include "zc/scripting/types/combotrigger.h"
#include "zc/scripting/types/debug.h"
#include "zc/scripting/types/directory.h"
#include "zc/scripting/types/dmapdata.h"
#include "zc/scripting/types/dropsetdata.h"
#include "zc/scripting/types/eweapon.h"
#include "zc/scripting/types/ffc.h"
#include "zc/scripting/types/file.h"
#include "zc/scripting/types/game.h"
#include "zc/scripting/types/genericdata.h"
#include "zc/scripting/types/global.h"
#include "zc/scripting/types/graphics.h"
#include "zc/scripting/types/hero.h"
#include "zc/scripting/types/input.h"
#include "zc/scripting/types/itemdata.h"
#include "zc/scripting/types/itemsprite.h"
#include "zc/scripting/types/lweapon.h"
#include "zc/scripting/types/mapdata.h"
#include "zc/scripting/types/messagedata.h"
#include "zc/scripting/types/misc.h"
#include "zc/scripting/types/musicdata.h"
#include "zc/scripting/types/npc.h"
#include "zc/scripting/types/npcdata.h"
#include "zc/scripting/types/portal.h"
#include "zc/scripting/types/region.h"
#include "zc/scripting/types/savedportal.h"
#include "zc/scripting/types/savemenu.h"
#include "zc/scripting/types/screendata.h"
#include "zc/scripting/types/shopdata.h"
#include "zc/scripting/types/sprite.h"
#include "zc/scripting/types/spritedata.h"
#include "zc/scripting/types/stack.h"
#include "zc/scripting/types/subscreendata.h"
#include "zc/scripting/types/subscreenpage.h"
#include "zc/scripting/types/subscreenwidget.h"
#include "zc/scripting/types/user_object.h"
#include "zc/scripting/types/viewport.h"
#include "zc/scripting/types/websocket.h"

#include <array>
#include <optional>

enum class EngineSubsystem : uint8_t
{
	none,
	audio,
	bitmap,
	bottledata,
	combodata,
	combotrigger,
	debug,
	directory,
	dmapdata,
	dropsetdata,
	eweapon,
	ffc,
	file,
	game,
	genericdata,
	global,
	graphics,
	hero,
	input,
	itemdata,
	itemsprite,
	lweapon,
	mapdata,
	messagedata,
	misc,
	musicdata,
	npc,
	npcdata,
	portal,
	region,
	savedportal,
	savemenu,
	screendata,
	shopdata,
	sprite,
	spritedata,
	stack,
	subscreendata,
	subscreenpage,
	subscreenwidget,
	user_object,
	viewport,
	weapon,
	websocket,
};

constexpr int MAX_REGISTER_ID = NUMVARIABLES;
extern const std::array<EngineSubsystem, MAX_REGISTER_ID + 1> register_routing_table;

ZC_FORCE_INLINE int32_t scripting_engine_get_register(int32_t reg)
{
	if (reg < 0 || reg > MAX_REGISTER_ID)
		return 0;

	switch (register_routing_table[reg])
	{
		case EngineSubsystem::audio: return audio_get_register(reg);
		case EngineSubsystem::bitmap: return bitmap_get_register(reg);
		case EngineSubsystem::bottledata: return bottledata_get_register(reg);
		case EngineSubsystem::combodata: return combodata_get_register(reg);
		case EngineSubsystem::combotrigger: return combotrigger_get_register(reg);
		case EngineSubsystem::debug: return debug_get_register(reg);
		case EngineSubsystem::directory: return directory_get_register(reg);
		case EngineSubsystem::dmapdata: return dmapdata_get_register(reg);
		case EngineSubsystem::dropsetdata: return dropsetdata_get_register(reg);
		case EngineSubsystem::eweapon: return eweapon_get_register(reg);
		case EngineSubsystem::ffc: return ffc_get_register(reg);
		case EngineSubsystem::file: return file_get_register(reg);
		case EngineSubsystem::game: return game_get_register(reg);
		case EngineSubsystem::genericdata: return genericdata_get_register(reg);
		case EngineSubsystem::global: return global_get_register(reg);
		case EngineSubsystem::graphics: return graphics_get_register(reg);
		case EngineSubsystem::hero: return hero_get_register(reg);
		case EngineSubsystem::input: return input_get_register(reg);
		case EngineSubsystem::itemdata: return itemdata_get_register(reg);
		case EngineSubsystem::itemsprite: return itemsprite_get_register(reg);
		case EngineSubsystem::lweapon: return lweapon_get_register(reg);
		case EngineSubsystem::mapdata: return mapdata_get_register(reg);
		case EngineSubsystem::messagedata: return messagedata_get_register(reg);
		case EngineSubsystem::misc: return misc_get_register(reg);
		case EngineSubsystem::musicdata: return musicdata_get_register(reg);
		case EngineSubsystem::npc: return npc_get_register(reg);
		case EngineSubsystem::npcdata: return npcdata_get_register(reg);
		case EngineSubsystem::portal: return portal_get_register(reg);
		case EngineSubsystem::region: return region_get_register(reg);
		case EngineSubsystem::savedportal: return savedportal_get_register(reg);
		case EngineSubsystem::savemenu: return savemenu_get_register(reg);
		case EngineSubsystem::screendata: return screendata_get_register(reg);
		case EngineSubsystem::shopdata: return shopdata_get_register(reg);
		case EngineSubsystem::sprite: return sprite_get_register(reg);
		case EngineSubsystem::spritedata: return spritedata_get_register(reg);
		case EngineSubsystem::stack: return stack_get_register(reg);
		case EngineSubsystem::subscreendata: return subscreendata_get_register(reg);
		case EngineSubsystem::subscreenpage: return subscreenpage_get_register(reg);
		case EngineSubsystem::subscreenwidget: return subscreenwidget_get_register(reg);
		case EngineSubsystem::viewport: return viewport_get_register(reg);
		case EngineSubsystem::websocket: return websocket_get_register(reg);
    }

#ifdef DEBUG_REGISTER_DEPS
	return 0; // TODO: remove this branch.
#else
	NOTREACHED();
#endif
}

ZC_FORCE_INLINE void scripting_engine_set_register(int32_t reg, int32_t value)
{
	if (reg < 0 || reg > MAX_REGISTER_ID)
		return;

	switch (register_routing_table[reg])
	{
		case EngineSubsystem::audio: audio_set_register(reg, value); return;
		case EngineSubsystem::bitmap: bitmap_set_register(reg, value); return;
		case EngineSubsystem::bottledata: bottledata_set_register(reg, value); return;
		case EngineSubsystem::combodata: combodata_set_register(reg, value); return;
		case EngineSubsystem::combotrigger: combotrigger_set_register(reg, value); return;
		case EngineSubsystem::debug: debug_set_register(reg, value); return;
		case EngineSubsystem::directory: directory_set_register(reg, value); return;
		case EngineSubsystem::dmapdata: dmapdata_set_register(reg, value); return;
		case EngineSubsystem::dropsetdata: dropsetdata_set_register(reg, value); return;
		case EngineSubsystem::eweapon: eweapon_set_register(reg, value); return;
		case EngineSubsystem::ffc: ffc_set_register(reg, value); return;
		case EngineSubsystem::file: file_set_register(reg, value); return;
		case EngineSubsystem::game: game_set_register(reg, value); return;
		case EngineSubsystem::genericdata: genericdata_set_register(reg, value); return;
		case EngineSubsystem::global: global_set_register(reg, value); return;
		case EngineSubsystem::graphics: graphics_set_register(reg, value); return;
		case EngineSubsystem::hero: hero_set_register(reg, value); return;
		case EngineSubsystem::input: input_set_register(reg, value); return;
		case EngineSubsystem::itemdata: itemdata_set_register(reg, value); return;
		case EngineSubsystem::itemsprite: itemsprite_set_register(reg, value); return;
		case EngineSubsystem::lweapon: lweapon_set_register(reg, value); return;
		case EngineSubsystem::mapdata: mapdata_set_register(reg, value); return;
		case EngineSubsystem::messagedata: messagedata_set_register(reg, value); return;
		case EngineSubsystem::misc: misc_set_register(reg, value); return;
		case EngineSubsystem::musicdata: musicdata_set_register(reg, value); return;
		case EngineSubsystem::npc: npc_set_register(reg, value); return;
		case EngineSubsystem::npcdata: npcdata_set_register(reg, value); return;
		case EngineSubsystem::portal: portal_set_register(reg, value); return;
		case EngineSubsystem::region: region_set_register(reg, value); return;
		case EngineSubsystem::savedportal: savedportal_set_register(reg, value); return;
		case EngineSubsystem::savemenu: savemenu_set_register(reg, value); return;
		case EngineSubsystem::screendata: screendata_set_register(reg, value); return;
		case EngineSubsystem::shopdata: shopdata_set_register(reg, value); return;
		case EngineSubsystem::sprite: sprite_set_register(reg, value); return;
		case EngineSubsystem::spritedata: spritedata_set_register(reg, value); return;
		case EngineSubsystem::stack: stack_set_register(reg, value); return;
		case EngineSubsystem::subscreendata: subscreendata_set_register(reg, value); return;
		case EngineSubsystem::subscreenpage: subscreenpage_set_register(reg, value); return;
		case EngineSubsystem::subscreenwidget: subscreenwidget_set_register(reg, value); return;
		case EngineSubsystem::viewport: viewport_set_register(reg, value); return;
		case EngineSubsystem::websocket: websocket_set_register(reg, value); return;
    }

#ifdef DEBUG_REGISTER_DEPS
	// TODO: remove this branch.
#else
	NOTREACHED();
#endif
}

// TODO: replace cascading code with a routing table (like above), and move all commands to files in types/
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
