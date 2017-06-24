#include <string>
#include "Types.h"
#include "DataStructs.h"
#include "Scope.h"

// Standard Type definitions.
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
ZVarTypeClass const ZVarType::AUDIO(ZCLASSID_AUDIO, "Audio");
ZVarTypeClass const ZVarType::DEBUG(ZCLASSID_DEBUG, "Debug");
ZVarTypeClass const ZVarType::NPCDATA(ZCLASSID_NPCDATA, "NPCData");
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
		case ZVARTYPEID_AUDIO: return &AUDIO;
		case ZVARTYPEID_DEBUG: return &DEBUG;
		case ZVARTYPEID_NPCDATA: return &NPCDATA;
	default: return NULL;
	}
}
	
int ZVarType::getArrayDepth() const
{
	ZVarType const* type = this;
	int depth = 0;
	while (type->isArray())
	{
		++depth;
		type = &((ZVarTypeArray const*)type)->getElementType();
	}
	return depth;
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
	if (target.typeClassId() == ZVARTYPE_CLASSID_CONST_FLOAT)
		return canCastTo(ZVarType::FLOAT);
	if (target.typeClassId() == ZVARTYPE_CLASSID_ARRAY)
		return canCastTo(((ZVarTypeArray const&)target).getBaseType());

	if (target.typeClassId() != ZVARTYPE_CLASSID_SIMPLE) return false;
	ZVarTypeSimple const& t = (ZVarTypeSimple const&)target;

	if (simpleId == ZVARTYPEID_VOID || t.simpleId == ZVARTYPEID_VOID)
		return false;
	if (simpleId == t.simpleId) return true;
	if (simpleId == ZVARTYPEID_FLOAT && t.simpleId == ZVARTYPEID_BOOL)
		return true;
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

string ZVarTypeClass::getName() const
{
	string name = className == "" ? "anonymous" : className;
	char tmp[32];
	sprintf(tmp, "%d", classId);
	return name + "[class " + tmp + "]";
}

bool ZVarTypeClass::canCastTo(ZVarType const& target) const
{
	if (target.typeClassId() == ZVARTYPE_CLASSID_ARRAY)
		return canCastTo(((ZVarTypeArray const&)target).getBaseType());
	return *this == target;
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

////////////////////////////////////////////////////////////////
// ZVarTypeArray

bool ZVarTypeArray::canCastTo(ZVarType const& target) const
{
	if (target.typeClassId() == ZVARTYPE_CLASSID_ARRAY)
		return canCastTo(((ZVarTypeArray const&)target).getBaseType());
	return getBaseType().canCastTo(target);
}

ZVarType const& ZVarTypeArray::getBaseType() const
{
	ZVarType const* type = &elementType;
	while (type->typeClassId() == ZVARTYPE_CLASSID_ARRAY)
		type = &((ZVarTypeArray const*)type)->elementType;
	return *type;
}

int ZVarTypeArray::selfCompare(ZVarType const& other) const
{
	ZVarTypeArray const& o = (ZVarTypeArray const&)other;
	return elementType.compare(o.elementType);
}
