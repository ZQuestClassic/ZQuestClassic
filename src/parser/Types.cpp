#include <cstdio>
#include <string>
#include <cassert>
#include "Types.h"
#include "DataStructs.h"
#include "Scope.h"

// Standard Type definitions.
ZVarTypeSimple const ZVarType::UNTYPED(ZVARTYPEID_UNTYPED, "untyped", "Untyped");
ZVarTypeSimple const ZVarType::ZVOID(ZVARTYPEID_VOID, "void", "Void");
ZVarTypeSimple const ZVarType::FLOAT(ZVARTYPEID_FLOAT, "float", "Float");
ZVarTypeSimple const ZVarType::BOOL(ZVARTYPEID_BOOL, "bool", "Bool");
ZVarTypeClass const ZVarType::GAME(ZCLASSID_GAME, "Game");
ZVarTypeClass const ZVarType::LINK(ZCLASSID_LINK, "Link");
ZVarTypeClass const ZVarType::SCREEN(ZCLASSID_SCREEN, "Screen");
ZVarTypeClass const ZVarType::FFC(ZCLASSID_FFC, "FFC");
ZVarTypeClass const ZVarType::ITEM(ZCLASSID_ITEM, "Item");
ZVarTypeClass const ZVarType::ITEMCLASS(ZCLASSID_ITEMCLASS, "ItemData");
ZVarTypeClass const ZVarType::NPC(ZCLASSID_NPC, "NPC");
ZVarTypeClass const ZVarType::LWPN(ZCLASSID_LWPN, "LWeapon");
ZVarTypeClass const ZVarType::EWPN(ZCLASSID_EWPN, "EWeapon");
ZVarTypeClass const ZVarType::NPCDATA(ZCLASSID_NPCDATA, "NpcData");
ZVarTypeClass const ZVarType::DEBUG(ZCLASSID_DEBUG, "Debug");
ZVarTypeClass const ZVarType::AUDIO(ZCLASSID_AUDIO, "Audio");
ZVarTypeClass const ZVarType::COMBOS(ZCLASSID_COMBOS, "Combos");
ZVarTypeClass const ZVarType::SPRITEDATA(ZCLASSID_SPRITEDATA, "SpriteData");
ZVarTypeClass const ZVarType::GRAPHICS(ZCLASSID_GRAPHICS, "Graphics");
ZVarTypeClass const ZVarType::TEXT(ZCLASSID_TEXT, "Text");
ZVarTypeClass const ZVarType::INPUT(ZCLASSID_INPUT, "Input");
ZVarTypeClass const ZVarType::MAPDATA(ZCLASSID_MAPDATA, "MapData");
ZVarTypeClass const ZVarType::DMAPDATA(ZCLASSID_DMAPDATA, "DMapData");
ZVarTypeClass const ZVarType::ZMESSAGE(ZCLASSID_ZMESSAGE, "ZMessage");
ZVarTypeClass const ZVarType::SHOPDATA(ZCLASSID_SHOPDATA, "ShopData");
ZVarTypeClass const ZVarType::DROPSET(ZCLASSID_DROPSET, "DropSet");
ZVarTypeClass const ZVarType::PONDS(ZCLASSID_PONDS, "Ponds");
ZVarTypeClass const ZVarType::WARPRING(ZCLASSID_WARPRING, "WarpRing");
ZVarTypeClass const ZVarType::DOORSET(ZCLASSID_DOORSET, "DoorSet");
ZVarTypeClass const ZVarType::ZUICOLOURS(ZCLASSID_ZUICOLOURS, "ZuiColours");
ZVarTypeClass const ZVarType::RGBDATA(ZCLASSID_RGBDATA, "RgbData");
ZVarTypeClass const ZVarType::PALETTE(ZCLASSID_PALETTE, "Palette");
ZVarTypeClass const ZVarType::TUNES(ZCLASSID_TUNES, "Tunes");
ZVarTypeClass const ZVarType::PALCYCLE(ZCLASSID_PALCYCLE, "PalCycle");
ZVarTypeClass const ZVarType::GAMEDATA(ZCLASSID_GAMEDATA, "GameData");
ZVarTypeClass const ZVarType::CHEATS(ZCLASSID_CHEATS, "Cheats");
ZVarTypeConstFloat const ZVarType::CONST_FLOAT;

////////////////////////////////////////////////////////////////
// ZVarType

int ZVarType::compare(ZVarType const& other) const
{
	int c = typeClassId() - other.typeClassId();
	if (c) return c;
	return selfCompare(other);
}

ZVarType const* ZVarType::get(ZVarTypeId id)
{
	switch (id)
	{
	case ZVARTYPEID_VOID: return &ZVOID;
	case ZVARTYPEID_FLOAT: return &FLOAT;
	case ZVARTYPEID_BOOL: return &BOOL;
	case ZVARTYPEID_CONST_FLOAT: return &CONST_FLOAT;
	case ZVARTYPEID_GAME: return &GAME;
	case ZVARTYPEID_LINK: return &LINK;
	case ZVARTYPEID_SCREEN: return &SCREEN;
	case ZVARTYPEID_FFC: return &FFC;
	case ZVARTYPEID_ITEM: return &ITEM;
	case ZVARTYPEID_ITEMCLASS: return &ITEMCLASS;
	case ZVARTYPEID_NPC: return &NPC;
	case ZVARTYPEID_LWPN: return &LWPN;
	case ZVARTYPEID_EWPN: return &EWPN;
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
	if (simpleId == ZVARTYPEID_FLOAT && target.typeClassId() == ZVARTYPE_CLASSID_CONST_FLOAT) return true;
	if (target.typeClassId() != ZVARTYPE_CLASSID_SIMPLE) return false;
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

////////////////////////////////////////////////////////////////
// ZVarTypeClass

std::string to_string( int x ) {
  int length = snprintf( NULL, 0, "%d", x );
  assert( length >= 0 );
  char* buf = new char[length + 1];
  snprintf( buf, length + 1, "%d", x );
  std::string str( buf );
  delete[] buf;
  return str;
}

string ZVarTypeClass::getName() const
{
	string name = className == "" ? "anonymous" : className;
	char tmp[32];
	sprintf(tmp, "%d", classId);
	return name + "[class " + tmp + "]";
}

ZVarType* ZVarTypeClass::resolve(Scope& scope)
{
	// Grab the proper name for the class the first time it's resolved.
	if (className == "")
		className = scope.getTable().getClass(classId)->name;

	return this;
}

int ZVarTypeClass::selfCompare(ZVarType const& other) const
{
	ZVarTypeClass const& o = (ZVarTypeClass const&)other;
	return classId - o.classId;
}

