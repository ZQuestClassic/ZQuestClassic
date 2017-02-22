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
