#include <cstdio>
#include <string>
#include <typeinfo>
#include "CompileError.h"
#include "DataStructs.h"
#include "Scope.h"
#include "Types.h"

using namespace std;
using namespace ZScript;

////////////////////////////////////////////////////////////////
// TypeStore

TypeStore::TypeStore()
{
	// Assign builtin types.
	for (DataTypeId id = ZVARTYPEID_START; id < ZVARTYPEID_END; ++id)
		assignTypeId(*DataType::get(id));

	// Assign builtin classes.
	for (int id = ZVARTYPEID_CLASS_START; id < ZVARTYPEID_CLASS_END; ++id)
	{
		DataTypeClass& type = *(DataTypeClass*)DataType::get(id);
		assert(type.getClassId() == ownedClasses.size());
		ownedClasses.push_back(
				new ZClass(*this, type.getClassName(), type.getClassId()));
	}
}

TypeStore::~TypeStore()
{
	deleteElements(ownedTypes);
	deleteElements(ownedClasses);
}

// Types

DataType const* TypeStore::getType(DataTypeId typeId) const
{
	if (typeId < 0 || typeId > (int)ownedTypes.size()) return NULL;
	return ownedTypes[typeId];
}

optional<DataTypeId> TypeStore::getTypeId(DataType const& type) const
{
	return find<DataTypeId>(typeIdMap, &type);
}

optional<DataTypeId> TypeStore::assignTypeId(DataType const& type)
{
	if (!type.isResolved())
	{
		box_out_err(CompileError::UnresolvedType(NULL, type.getName()));
		return nullopt;
	}

	if (find<DataTypeId>(typeIdMap, &type)) return nullopt;

	DataTypeId id = ownedTypes.size();
	DataType const* storedType = type.clone();
	ownedTypes.push_back(storedType);
	typeIdMap[storedType] = id;
	return id;
}

optional<DataTypeId> TypeStore::getOrAssignTypeId(DataType const& type)
{
	if (!type.isResolved())
	{
		box_out_err(CompileError::UnresolvedType(NULL, type.getName()));
		return nullopt;
	}

	if (optional<DataTypeId> typeId = find<DataTypeId>(typeIdMap, &type))
		return typeId;
	
	DataTypeId id = ownedTypes.size();
	DataType* storedType = type.clone();
	ownedTypes.push_back(storedType);
	typeIdMap[storedType] = id;
	return id;
}

// Classes

ZClass* TypeStore::getClass(int classId) const
{
	if (classId < 0 || classId > int(ownedClasses.size())) return NULL;
	return ownedClasses[classId];
}

ZClass* TypeStore::createClass(string const& name)
{
	ZClass* klass = new ZClass(*this, name, ownedClasses.size());
	ownedClasses.push_back(klass);
	return klass;
}

vector<Function*> ZScript::getClassFunctions(TypeStore const& store)
{
	vector<Function*> functions;
	vector<ZClass*> classes = store.getClasses();
	for (vector<ZClass*>::const_iterator it = classes.begin();
	     it != classes.end(); ++it)
	{
		appendElements(functions, (*it)->getLocalFunctions());
		appendElements(functions, (*it)->getLocalGetters());
		appendElements(functions, (*it)->getLocalSetters());
	}
	return functions;
}

// Internal

bool TypeStore::TypeIdMapComparator::operator()(
		DataType const* const& lhs, DataType const* const& rhs) const
{
	if (rhs == NULL) return false;
	if (lhs == NULL) return true;
	return *lhs < *rhs;
}

////////////////////////////////////////////////////////////////

// Standard Type definitions.
DataTypeSimple const DataType::UNTYPED(ZVARTYPEID_UNTYPED, "untyped");
DataTypeSimple const DataType::ZVOID(ZVARTYPEID_VOID, "void");
DataTypeSimple const DataType::FLOAT(ZVARTYPEID_FLOAT, "float");
DataTypeSimple const DataType::BOOL(ZVARTYPEID_BOOL, "bool");
DataTypeArray const DataType::STRING(FLOAT);
DataTypeClass const DataType::GAME(ZCLASSID_GAME, "Game");
DataTypeClass const DataType::LINK(ZCLASSID_LINK, "Link");
//DataTypeClass const DataType::PLAYER(ZCLASSID_LINK, "Player");
DataTypeClass const DataType::SCREEN(ZCLASSID_SCREEN, "Screen");
DataTypeClass const DataType::FFC(ZCLASSID_FFC, "FFC");
DataTypeClass const DataType::ITEM(ZCLASSID_ITEM, "Item");
DataTypeClass const DataType::ITEMCLASS(ZCLASSID_ITEMCLASS, "ItemData");
DataTypeClass const DataType::NPC(ZCLASSID_NPC, "NPC");
DataTypeClass const DataType::LWPN(ZCLASSID_LWPN, "LWeapon");
DataTypeClass const DataType::EWPN(ZCLASSID_EWPN, "EWeapon");
DataTypeClass const DataType::AUDIO(ZCLASSID_AUDIO, "Audio");
DataTypeClass const DataType::DEBUG(ZCLASSID_DEBUG, "Debug");
DataTypeClass const DataType::NPCDATA(ZCLASSID_NPCDATA, "NPCData");
DataTypeClass const DataType::COMBOS(ZCLASSID_COMBOS, "Combos");
DataTypeClass const DataType::SPRITEDATA(ZCLASSID_SPRITEDATA, "SpriteData");
DataTypeClass const DataType::GRAPHICS(ZCLASSID_GRAPHICS, "Graphics");
DataTypeClass const DataType::BITMAP(ZCLASSID_BITMAP, "Bitmap");
DataTypeClass const DataType::TEXT(ZCLASSID_TEXT, "Text");
DataTypeClass const DataType::INPUT(ZCLASSID_INPUT, "Input");
DataTypeClass const DataType::MAPDATA(ZCLASSID_MAPDATA, "MapData");
DataTypeClass const DataType::DMAPDATA(ZCLASSID_DMAPDATA, "DMapData");
DataTypeClass const DataType::ZMESSAGE(ZCLASSID_ZMESSAGE, "ZMessage");
DataTypeClass const DataType::SHOPDATA(ZCLASSID_SHOPDATA, "ShopData");
DataTypeClass const DataType::DROPSET(ZCLASSID_DROPSET, "DropSet");
DataTypeClass const DataType::PONDS(ZCLASSID_PONDS, "Ponds");
DataTypeClass const DataType::WARPRING(ZCLASSID_WARPRING, "WarpRing");
DataTypeClass const DataType::DOORSET(ZCLASSID_DOORSET, "DoorSet");
DataTypeClass const DataType::ZUICOLOURS(ZCLASSID_ZUICOLOURS, "ZuiColours");
DataTypeClass const DataType::RGBDATA(ZCLASSID_RGBDATA, "RgbData");
DataTypeClass const DataType::PALETTE(ZCLASSID_PALETTE, "Palette");
DataTypeClass const DataType::TUNES(ZCLASSID_TUNES, "Tunes");
DataTypeClass const DataType::PALCYCLE(ZCLASSID_PALCYCLE, "PalCycle");
DataTypeClass const DataType::GAMEDATA(ZCLASSID_GAMEDATA, "GameData");
DataTypeClass const DataType::CHEATS(ZCLASSID_CHEATS, "Cheats");
DataTypeConstFloat const DataType::CONST_FLOAT;

////////////////////////////////////////////////////////////////
// DataType

int DataType::compare(DataType const& rhs) const
{
	type_info const& lhsType = typeid(*this);
	type_info const& rhsType = typeid(rhs);
	if (lhsType.before(rhsType)) return -1;
	if (rhsType.before(lhsType)) return 1;
	return selfCompare(rhs);
}

DataType const* DataType::get(DataTypeId id)
{
	switch (id)
	{
	case ZVARTYPEID_UNTYPED: return &UNTYPED;
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
	case ZVARTYPEID_BITMAP: return &BITMAP;
	case ZVARTYPEID_TEXT: return &TEXT;
	case ZVARTYPEID_INPUT: return &INPUT;
	case ZVARTYPEID_MAPDATA: return &MAPDATA;
	case ZVARTYPEID_DMAPDATA: return &DMAPDATA;
	case ZVARTYPEID_ZMESSAGE: return &ZMESSAGE;
	case ZVARTYPEID_SHOPDATA: return &SHOPDATA;
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

bool ZScript::operator==(DataType const& lhs, DataType const& rhs)
{
	return lhs.compare(rhs) == 0;
}

bool ZScript::operator!=(DataType const& lhs, DataType const& rhs)
{
	return lhs.compare(rhs) != 0;
}

bool ZScript::operator<(DataType const& lhs, DataType const& rhs)
{
	return lhs.compare(rhs) < 0;
}

bool ZScript::operator<=(DataType const& lhs, DataType const& rhs)
{
	return lhs.compare(rhs) <= 0;
}

bool ZScript::operator>(DataType const& lhs, DataType const& rhs)
{
	return lhs.compare(rhs) > 0;
}

bool ZScript::operator>=(DataType const& lhs, DataType const& rhs)
{
	return lhs.compare(rhs) >= 0;
}

DataType const& ZScript::getNaiveType(DataType const& type)
{
	if (type == DataType::CONST_FLOAT) return DataType::FLOAT;

	DataType const* t = &type;
	while (DataTypeArray const* ta = dynamic_cast<DataTypeArray const*>(t))
		t = &ta->getElementType();

	return *t;
}

int ZScript::getArrayDepth(DataType const& type)
{
	DataType const* ptype = &type;
	int depth = 0;
	while (DataTypeArray const* t = dynamic_cast<DataTypeArray const*>(ptype))
	{
		++depth;
		ptype = &t->getElementType();
	}
	return depth;
}

////////////////////////////////////////////////////////////////
// DataTypeUnresolved

DataType* DataTypeUnresolved::resolve(Scope& scope)
{
	if (DataType const* type = lookupDataType(scope, name))
		return type->clone();
	return NULL;
}

int DataTypeUnresolved::selfCompare(DataType const& rhs) const
{
	DataTypeUnresolved const& o = static_cast<DataTypeUnresolved const&>(rhs);
	return name.compare(o.name);
}

////////////////////////////////////////////////////////////////
// DataTypeSimple

DataTypeSimple::DataTypeSimple(int simpleId, string const& name)
	: simpleId(simpleId), name(name)
{}

int DataTypeSimple::selfCompare(DataType const& rhs) const
{
	DataTypeSimple const& o = static_cast<DataTypeSimple const&>(rhs);
	return simpleId - o.simpleId;
}

bool DataTypeSimple::canCastTo(DataType const& target) const
{
	if (simpleId == ZVARTYPEID_UNTYPED) return true;
	if (target == UNTYPED) return true;
	
	if (DataTypeConstFloat const* t =
			dynamic_cast<DataTypeConstFloat const*>(&target))
		return canCastTo(DataType::FLOAT);

	if (DataTypeArray const* t =
			dynamic_cast<DataTypeArray const*>(&target))
		return canCastTo(getBaseType(*t));

	if (DataTypeSimple const* t =
			dynamic_cast<DataTypeSimple const*>(&target))
	{
		if (simpleId == ZVARTYPEID_UNTYPED || t->simpleId == ZVARTYPEID_UNTYPED)
			return true;
		if (simpleId == ZVARTYPEID_VOID || t->simpleId == ZVARTYPEID_VOID)
			return false;
		if (simpleId == t->simpleId)
			return true;
		if (simpleId == ZVARTYPEID_FLOAT && t->simpleId == ZVARTYPEID_BOOL)
			return true;
	}
	
	return false;
}

bool DataTypeSimple::canBeGlobal() const
{
	return simpleId == ZVARTYPEID_FLOAT || simpleId == ZVARTYPEID_BOOL;
}

////////////////////////////////////////////////////////////////
// DataTypeConstFloat

bool DataTypeConstFloat::canCastTo(DataType const& target) const
{
	if (target == UNTYPED) return true;
	
	if (*this == target) return true;
	return DataType::FLOAT.canCastTo(target);
}

////////////////////////////////////////////////////////////////
// DataTypeClass

DataTypeClass::DataTypeClass(int classId)
	: classId(classId), className("")
{}

DataTypeClass::DataTypeClass(int classId, string const& className)
	: classId(classId), className(className)
{}

DataTypeClass* DataTypeClass::resolve(Scope& scope)
{
	// Grab the proper name for the class the first time it's resolved.
	if (className == "")
		className = scope.getTypeStore().getClass(classId)->name;

	return this;
}

string DataTypeClass::getName() const
{
	string name = className == "" ? "anonymous" : className;
	char tmp[32];
	sprintf(tmp, "%d", classId);
	return name + "[class " + tmp + "]";
}

bool DataTypeClass::canCastTo(DataType const& target) const
{
	if (target == UNTYPED) return true;
	
	if (DataTypeArray const* t =
			dynamic_cast<DataTypeArray const*>(&target))
		return canCastTo(getBaseType(*t));

	return *this == target;
}

int DataTypeClass::selfCompare(DataType const& rhs) const
{
	DataTypeClass const& o = static_cast<DataTypeClass const&>(rhs);
	return classId - o.classId;
}

////////////////////////////////////////////////////////////////
// DataTypeArray

bool DataTypeArray::canCastTo(DataType const& target) const
{
	if (target == UNTYPED) return true;
	
	if (DataTypeArray const* t =
			dynamic_cast<DataTypeArray const*>(&target))
		return canCastTo(getBaseType(*t));
	
	return getBaseType(*this).canCastTo(target);
}

int DataTypeArray::selfCompare(DataType const& rhs) const
{
	DataTypeArray const& o = static_cast<DataTypeArray const&>(rhs);
	return elementType.compare(o.elementType);
}

DataType const& ZScript::getBaseType(DataType const& type)
{
	DataType const* current = &type;
	while (DataTypeArray const* t = dynamic_cast<DataTypeArray const*>(current))
		current = &t->getElementType();
	return *current;
}

////////////////////////////////////////////////////////////////
// Script Types

namespace // file local
{
	struct ScriptTypeData
	{
		string name;
		DataTypeId thisTypeId;
	};

	ScriptTypeData scriptTypes[ScriptType::idEnd] = {
		{"invalid", ZVARTYPEID_VOID},
		{"global", ZVARTYPEID_VOID},
		{"ffc", ZVARTYPEID_FFC},
		{"item", ZVARTYPEID_ITEMCLASS},
		{"npc", ZVARTYPEID_NPC},
		{"eweapon", ZVARTYPEID_EWPN},
		{"lweapon", ZVARTYPEID_LWPN},
		{"link", ZVARTYPEID_LINK},
		{"player", ZVARTYPEID_LINK},
		{"screendata", ZVARTYPEID_SCREEN},
		{"dmapdata", ZVARTYPEID_DMAPDATA},
	};
}

ScriptType const ScriptType::invalid(idInvalid);
ScriptType const ScriptType::global(idGlobal);
ScriptType const ScriptType::ffc(idFfc);
ScriptType const ScriptType::item(idItem);
ScriptType const ScriptType::npc(idNPC);
ScriptType const ScriptType::lweapon(idLWeapon);
ScriptType const ScriptType::eweapon(idEWeapon);
ScriptType const ScriptType::link(idLink);
ScriptType const ScriptType::player(idPlayer);
ScriptType const ScriptType::screendata(idScreen);
ScriptType const ScriptType::dmapdata(idDMap);

string const& ScriptType::getName() const
{
	if (isValid()) return scriptTypes[id_].name;
	return scriptTypes[idInvalid].name;
}

DataTypeId ScriptType::getThisTypeId() const
{
	if (isValid()) return scriptTypes[id_].thisTypeId;
	return scriptTypes[idInvalid].thisTypeId;
}

bool ZScript::operator==(ScriptType const& lhs, ScriptType const& rhs)
{
	if (!lhs.isValid()) return !rhs.isValid();
	return lhs.id_ == rhs.id_;
}

bool ZScript::operator!=(ScriptType const& lhs, ScriptType const& rhs)
{
	if (lhs.isValid()) return lhs.id_ != rhs.id_;
	return rhs.isValid();
}
