#include <typeinfo>
#include "Types.h"
#include "DataStructs.h"
#include "Scope.h"

// Standard Type definitions.
ZVarTypeSimple const ZVarType::VOID(ZVARTYPEID_VOID, "void", "Void");
ZVarTypeSimple const ZVarType::FLOAT(ZVARTYPEID_FLOAT, "float", "Float");
ZVarTypeSimple const ZVarType::BOOL(ZVARTYPEID_BOOL, "bool", "Bool");
ZVarTypeSimple const ZVarType::FFC(ZVARTYPEID_FFC, "ffc", "FFC");
ZVarTypeSimple const ZVarType::ITEM(ZVARTYPEID_ITEM, "item", "Item");
ZVarTypeSimple const ZVarType::ITEMCLASS(ZVARTYPEID_ITEMCLASS, "itemdata", "ItemData");
ZVarTypeSimple const ZVarType::NPC(ZVARTYPEID_NPC, "npc", "NPC");
ZVarTypeSimple const ZVarType::LWPN(ZVARTYPEID_LWPN, "lweapon", "LWeapon");
ZVarTypeSimple const ZVarType::EWPN(ZVARTYPEID_EWPN, "eweapon", "EWeapon");
ZVarTypeSimple const ZVarType::GAME(ZVARTYPEID_GAME, "game", "Game");
ZVarTypeSimple const ZVarType::LINK(ZVARTYPEID_LINK, "link", "Link");
ZVarTypeSimple const ZVarType::SCREEN(ZVARTYPEID_SCREEN, "screen", "Screen");
ZVarTypeSimple const ZVarType::NPCDATA(ZVARTYPEID_NPCDATA, "npcdata", "NpcData");
ZVarTypeSimple const ZVarType::DEBUG(ZVARTYPEID_DEBUG, "debug", "Debug");
ZVarTypeSimple const ZVarType::AUDIO(ZVARTYPEID_AUDIO, "audio", "Audio");
ZVarTypeSimple const ZVarType::COMBOS(ZVARTYPEID_COMBOS, "combos", "Combos");
ZVarTypeSimple const ZVarType::SPRITEDATA(ZVARTYPEID_SPRITEDATA, "spritedata", "SpriteData");
ZVarTypeSimple const ZVarType::GRAPHICS(ZVARTYPEID_GRAPHICS, "graphics", "Graphics");
ZVarTypeSimple const ZVarType::TEXT(ZVARTYPEID_TEXT, "text", "Text");
ZVarTypeSimple const ZVarType::INPUT(ZVARTYPEID_INPUT, "input", "Input");
ZVarTypeSimple const ZVarType::MAPDATA(ZVARTYPEID_MAPDATA, "mapdata", "MapData");
ZVarTypeSimple const ZVarType::DMAPDATA(ZVARTYPEID_DMAPDATA, "dmapdata", "DMapData");
ZVarTypeSimple const ZVarType::ZMESSAGE(ZVARTYPEID_ZMESSAGE, "zmessage", "ZMessage");
ZVarTypeSimple const ZVarType::SHOPDATA(ZVARTYPEID_SHOPDATA, "shopdata", "ShopData");
ZVarTypeSimple const ZVarType::UNTYPED(ZVARTYPEID_UNTYPED, "untyped", "Untyped");
ZVarTypeSimple const ZVarType::DROPSET(ZVARTYPEID_DROPSET, "dropset", "DropSet");
ZVarTypeSimple const ZVarType::PONDS(ZVARTYPEID_PONDS, "ponds", "Ponds");
ZVarTypeSimple const ZVarType::WARPRING(ZVARTYPEID_WARPRING, "warpring", "WarpRing");
ZVarTypeSimple const ZVarType::DOORSET(ZVARTYPEID_DOORSET, "doorset", "DoorSet");
ZVarTypeSimple const ZVarType::ZUICOLOURS(ZVARTYPEID_ZUICOLOURS, "zuicolours", "ZuiColours");
ZVarTypeSimple const ZVarType::RGBDATA(ZVARTYPEID_RGBDATA, "rgbdata", "RgbData");
ZVarTypeSimple const ZVarType::PALETTE(ZVARTYPEID_PALETTE, "palette", "Palette");
ZVarTypeSimple const ZVarType::TUNES(ZVARTYPEID_TUNES, "tunes", "Tunes");
ZVarTypeSimple const ZVarType::PALCYCLE(ZVARTYPEID_PALCYCLE, "palcycle", "PalCycle");
ZVarTypeSimple const ZVarType::GAMEDATA(ZVARTYPEID_GAMEDATA, "gamedata", "GameData");
ZVarTypeSimple const ZVarType::CHEATS(ZVARTYPEID_CHEATS, "cheats", "Cheats");
ZVarTypeConstFloat const ZVarType::CONST_FLOAT;

////////////////////////////////////////////////////////////////
// ZVarType

int ZVarType::compare(ZVarType const& other) const
{
	int c = classId() - other.classId();
	if (c) return c;
	return selfCompare(other);
}

ZVarType const* ZVarType::get(ZVarTypeId id)
{
	switch (id)
	{
	case ZVARTYPEID_VOID: return &VOID;
	case ZVARTYPEID_FLOAT: return &FLOAT;
	case ZVARTYPEID_BOOL: return &BOOL;
	case ZVARTYPEID_FFC: return &FFC;
	case ZVARTYPEID_ITEM: return &ITEM;
	case ZVARTYPEID_ITEMCLASS: return &ITEMCLASS;
	case ZVARTYPEID_NPC: return &NPC;
	case ZVARTYPEID_LWPN: return &LWPN;
	case ZVARTYPEID_EWPN: return &EWPN;
	case ZVARTYPEID_GAME: return &GAME;
	case ZVARTYPEID_LINK: return &LINK;
	case ZVARTYPEID_SCREEN: return &SCREEN;
	case ZVARTYPEID_NPCDATA: return &NPCDATA;
	case ZVARTYPEID_DEBUG: return &DEBUG;
	case ZVARTYPEID_AUDIO: return &AUDIO;
	case ZVARTYPEID_COMBOS: return &COMBOS;
	case ZVARTYPEID_SPRITEDATA: return &SPRITEDATA;
	case ZVARTYPEID_GRAPHICS: return &GRAPHICS;
	case ZVARTYPEID_TEXT: return &TEXT;
	case ZVARTYPEID_INPUT: return &INPUT;
	case ZVARTYPEID_MAPDATA: return &MAPDATA;
	case ZVARTYPEID_DMAPDATA: return &DMAPDATA;
	case ZVARTYPEID_ZMESSAGE: return &ZMESSAGE;
	case ZVARTYPEID_SHOPDATA: return &SHOPDATA;
	case ZVARTYPEID_UNTYPED: return &UNTYPED;
	case ZVARTYPEID_DROPSET: return &DROPSET;
	case ZVARTYPEID_PONDS: return &PONDS;
	case ZVARTYPEID_WARPRING: return &WARPRING;
	case ZVARTYPEID_DOORSET: return &DOORSET;
	case ZVARTYPEID_ZUICOLOURS: return &ZUICOLOURS;
	case ZVARTYPEID_RGBDATA: return &RGBDATA;
	case ZVARTYPEID_PALETTE: return &PALETTE;
	case ZVARTYPEID_TUNES: return &TUNES;
	case ZVARTYPEID_PALCYCLE: return &PALCYCLE;
	case ZVARTYPEID_GAMEDATA: return &GAMEDATA;
	case ZVARTYPEID_CHEATS: return &CHEATS;
	default: return NULL;
	}
}

////////////////////////////////////////////////////////////////
// ZVarTypeSimple

int ZVarTypeSimple::selfCompare(ZVarType const& other) const
{
	ZVarTypeSimple const& o = (ZVarTypeSimple const&)other;
	return simpleId - o.simpleId;
}

bool ZVarTypeSimple::canBeGlobal() const
{
	return simpleId == ZVARTYPEID_FLOAT || simpleId == ZVARTYPEID_BOOL;
}

bool ZVarTypeSimple::canCastTo(ZVarType const& target) const
{
	if (simpleId == ZVARTYPEID_FLOAT && target.classId() == ZVARTYPE_CLASSID_CONST_FLOAT) return true;
	if (target.classId() != ZVARTYPE_CLASSID_SIMPLE) return false;
	ZVarTypeSimple const& t = (ZVarTypeSimple const&)target;
	if (simpleId == ZVARTYPEID_UNTYPED || t.simpleId == ZVARTYPEID_UNTYPED) return true;
	if (simpleId == ZVARTYPEID_VOID || t.simpleId == ZVARTYPEID_VOID) return false;
	if (simpleId == t.simpleId) return true;
	if (simpleId == ZVARTYPEID_FLOAT && t.simpleId == ZVARTYPEID_BOOL) return true;
	return false;
}

////////////////////////////////////////////////////////////////
// ZVarTypeUnresolved

ZVarType* ZVarTypeUnresolved::resolve(Scope& scope)
{
	ZVarTypeId id = scope.getTypeId(name);
	if (id == -1) return this;
	return scope.getTable().getType(id)->clone();
}

int ZVarTypeUnresolved::selfCompare(ZVarType const& other) const
{
	ZVarTypeUnresolved const& o = (ZVarTypeUnresolved const&)other;
	return name.compare(o.name);
}

////////////////////////////////////////////////////////////////
// ZVarTypeConstFloat

bool ZVarTypeConstFloat::canCastTo(ZVarType const& target) const
{
	if (*this == target) return true;
	return ZVarType::FLOAT.canCastTo(target);
}
