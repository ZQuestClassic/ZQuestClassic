#include "Types.h"
#include "DataStructs.h"

// Standard Type definitions.
ZVarTypeSimple const ZVarType::VOID(ZVARTYPEID_VOID, "void");
ZVarTypeSimple const ZVarType::FLOAT(ZVARTYPEID_FLOAT, "float");
ZVarTypeSimple const ZVarType::BOOL(ZVARTYPEID_BOOL, "bool");
ZVarTypeSimple const ZVarType::FFC(ZVARTYPEID_FFC, "ffc");
ZVarTypeSimple const ZVarType::ITEM(ZVARTYPEID_ITEM, "item");
ZVarTypeSimple const ZVarType::ITEMCLASS(ZVARTYPEID_ITEMCLASS, "itemdata");
ZVarTypeSimple const ZVarType::NPC(ZVARTYPEID_NPC, "npc");
ZVarTypeSimple const ZVarType::LWPN(ZVARTYPEID_LWPN, "lweapon");
ZVarTypeSimple const ZVarType::EWPN(ZVARTYPEID_EWPN, "eweapon");
ZVarTypeSimple const ZVarType::GAME(ZVARTYPEID_GAME, "game");
ZVarTypeSimple const ZVarType::LINK(ZVARTYPEID_LINK, "link");
ZVarTypeSimple const ZVarType::SCREEN(ZVARTYPEID_SCREEN, "screen");

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
