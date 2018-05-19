#include "Types.h"

// Standard Type definitions.
ZVarTypeSimple const ZVarType::VOID(ZVARTYPEID_VOID, "void");
ZVarTypeSimple const ZVarType::FLOAT(ZVARTYPEID_FLOAT, "float");
ZVarTypeSimple const ZVarType::BOOL(ZVARTYPEID_BOOL, "bool");
ZVarTypeSimple const ZVarType::FFC(ZVARTYPEID_FFC, "ffc");
ZVarTypeSimple const ZVarType::ITEM(ZVARTYPEID_ITEM, "item");
ZVarTypeSimple const ZVarType::ITEMCLASS(ZVARTYPEID_ITEMCLASS, "itemclass");
ZVarTypeSimple const ZVarType::NPC(ZVARTYPEID_NPC, "npc");
ZVarTypeSimple const ZVarType::LWPN(ZVARTYPEID_LWPN, "lwpn");
ZVarTypeSimple const ZVarType::EWPN(ZVARTYPEID_EWPN, "ewpn");
ZVarTypeSimple const ZVarType::GAME(ZVARTYPEID_GAME, "game");
ZVarTypeSimple const ZVarType::LINK(ZVARTYPEID_LINK, "link");
ZVarTypeSimple const ZVarType::SCREEN(ZVARTYPEID_SCREEN, "screen");
ZVarTypeSimple const ZVarType::NPCDATA(ZVARTYPEID_NPCDATA, "npcdata");
ZVarTypeSimple const ZVarType::DEBUG(ZVARTYPEID_DEBUG, "debug");
ZVarTypeSimple const ZVarType::AUDIO(ZVARTYPEID_AUDIO, "audio");
ZVarTypeSimple const ZVarType::COMBOS(ZVARTYPEID_COMBOS, "combos");
ZVarTypeSimple const ZVarType::SPRITEDATA(ZVARTYPEID_SPRITEDATA, "spritedata");
ZVarTypeSimple const ZVarType::GRAPHICS(ZVARTYPEID_GRAPHICS, "graphics");
ZVarTypeSimple const ZVarType::TEXT(ZVARTYPEID_TEXT, "text");
ZVarTypeSimple const ZVarType::INPUT(ZVARTYPEID_INPUT, "input");
ZVarTypeSimple const ZVarType::MAPDATA(ZVARTYPEID_MAPDATA, "mapdata");
ZVarTypeSimple const ZVarType::DMAPDATA(ZVARTYPEID_DMAPDATA, "dmapdata");
ZVarTypeSimple const ZVarType::ZMESSAGE(ZVARTYPEID_ZMESSAGE, "zmessage");
ZVarTypeSimple const ZVarType::SHOPDATA(ZVARTYPEID_SHOPDATA, "shopdata");
ZVarTypeSimple const ZVarType::UNTYPED(ZVARTYPEID_UNTYPED, "untyped");
ZVarTypeSimple const ZVarType::DROPSET(ZVARTYPEID_DROPSET, "dropset");
ZVarTypeSimple const ZVarType::PONDS(ZVARTYPEID_PONDS, "ponds");
ZVarTypeSimple const ZVarType::WARPRING(ZVARTYPEID_WARPRING, "warpring");
ZVarTypeSimple const ZVarType::DOORSET(ZVARTYPEID_DOORSET, "doorset");
ZVarTypeSimple const ZVarType::ZUICOLOURS(ZVARTYPEID_ZUICOLOURS, "zuicolours");
ZVarTypeSimple const ZVarType::RGBDATA(ZVARTYPEID_RGBDATA, "rgbdata");
ZVarTypeSimple const ZVarType::PALETTE(ZVARTYPEID_PALETTE, "palette");
ZVarTypeSimple const ZVarType::TUNES(ZVARTYPEID_TUNES, "tunes");
ZVarTypeSimple const ZVarType::PALCYCLE(ZVARTYPEID_PALCYCLE, "palcycle");
ZVarTypeSimple const ZVarType::GAMEDATA(ZVARTYPEID_GAMEDATA, "gamedata");
ZVarTypeSimple const ZVarType::CHEATS(ZVARTYPEID_CHEATS, "cheats");

////////////////////////////////////////////////////////////////
// ZVarType

int ZVarType::compare(ZVarType const& other) const
{
	int c = classCompareId() - other.classCompareId();
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
