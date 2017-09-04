#include <string>
#include "Types.h"
#include "DataStructs.h"
#include "Scope.h"

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
		CompileError::UnresolvedType.print(NULL, type.getName());
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
		CompileError::UnresolvedType.print(NULL, type.getName());
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

////////////////////////////////////////////////////////////////

// Standard Type definitions.
DataTypeSimple const DataType::ZVOID(ZVARTYPEID_VOID, "void", "Void");
DataTypeSimple const DataType::FLOAT(ZVARTYPEID_FLOAT, "float", "Float");
DataTypeSimple const DataType::BOOL(ZVARTYPEID_BOOL, "bool", "Bool");
DataTypeClass const DataType::GAME(ZCLASSID_GAME, "Game");
DataTypeClass const DataType::_LINK(ZCLASSID_LINK, "Link");
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
DataTypeConstFloat const DataType::CONST_FLOAT;

////////////////////////////////////////////////////////////////
// DataType

int DataType::compare(DataType const& other) const
{
	int c = typeClassId() - other.typeClassId();
	if (c) return c;
	return selfCompare(other);
}

DataType const* DataType::get(DataTypeId id)
{
	switch (id)
	{
	case ZVARTYPEID_VOID: return &ZVOID;
	case ZVARTYPEID_FLOAT: return &FLOAT;
	case ZVARTYPEID_BOOL: return &BOOL;
	case ZVARTYPEID_CONST_FLOAT: return &CONST_FLOAT;
	case ZVARTYPEID_GAME: return &GAME;
	case ZVARTYPEID_LINK: return &_LINK;
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
	
int DataType::getArrayDepth() const
{
	DataType const* type = this;
	int depth = 0;
	while (type->isArray())
	{
		++depth;
		type = &((DataTypeArray const*)type)->getElementType();
	}
	return depth;
}

////////////////////////////////////////////////////////////////
// DataTypeSimple

int DataTypeSimple::selfCompare(DataType const& other) const
{
	DataTypeSimple const& o = (DataTypeSimple const&)other;
	return simpleId - o.simpleId;
}

bool DataTypeSimple::canBeGlobal() const
{
	return simpleId == ZVARTYPEID_FLOAT || simpleId == ZVARTYPEID_BOOL;
}

bool DataTypeSimple::canCastTo(DataType const& target) const
{
	if (target.typeClassId() == ZVARTYPE_CLASSID_CONST_FLOAT)
		return canCastTo(DataType::FLOAT);
	if (target.typeClassId() == ZVARTYPE_CLASSID_ARRAY)
		return canCastTo(((DataTypeArray const&)target).getBaseType());

	if (target.typeClassId() != ZVARTYPE_CLASSID_SIMPLE) return false;
	DataTypeSimple const& t = (DataTypeSimple const&)target;

	if (simpleId == ZVARTYPEID_VOID || t.simpleId == ZVARTYPEID_VOID)
		return false;
	if (simpleId == t.simpleId) return true;
	if (simpleId == ZVARTYPEID_FLOAT && t.simpleId == ZVARTYPEID_BOOL)
		return true;
	return false;
}

////////////////////////////////////////////////////////////////
// DataTypeUnresolved

DataType* DataTypeUnresolved::resolve(Scope& scope)
{
	if (DataType const* type = lookupType(scope, name))
		return type->clone();
	return NULL;
}

int DataTypeUnresolved::selfCompare(DataType const& other) const
{
	DataTypeUnresolved const& o = (DataTypeUnresolved const&)other;
	return name.compare(o.name);
}

////////////////////////////////////////////////////////////////
// DataTypeConstFloat

bool DataTypeConstFloat::canCastTo(DataType const& target) const
{
	if (*this == target) return true;
	return DataType::FLOAT.canCastTo(target);
}

////////////////////////////////////////////////////////////////
// DataTypeClass

string DataTypeClass::getName() const
{
	string name = className == "" ? "anonymous" : className;
	char tmp[32];
	sprintf(tmp, "%d", classId);
	return name + "[class " + tmp + "]";
}

bool DataTypeClass::canCastTo(DataType const& target) const
{
	if (target.typeClassId() == ZVARTYPE_CLASSID_ARRAY)
		return canCastTo(((DataTypeArray const&)target).getBaseType());
	return *this == target;
}

DataType* DataTypeClass::resolve(Scope& scope)
{
	// Grab the proper name for the class the first time it's resolved.
	if (className == "")
		className = scope.getTypeStore().getClass(classId)->name;

	return this;
}

int DataTypeClass::selfCompare(DataType const& other) const
{
	DataTypeClass const& o = (DataTypeClass const&)other;
	return classId - o.classId;
}

////////////////////////////////////////////////////////////////
// DataTypeArray

bool DataTypeArray::canCastTo(DataType const& target) const
{
	if (target.typeClassId() == ZVARTYPE_CLASSID_ARRAY)
		return canCastTo(((DataTypeArray const&)target).getBaseType());
	return getBaseType().canCastTo(target);
}

DataType const& DataTypeArray::getBaseType() const
{
	DataType const* type = &elementType;
	while (type->typeClassId() == ZVARTYPE_CLASSID_ARRAY)
		type = &((DataTypeArray const*)type)->elementType;
	return *type;
}

int DataTypeArray::selfCompare(DataType const& other) const
{
	DataTypeArray const& o = (DataTypeArray const&)other;
	return elementType.compare(o.elementType);
}

////////////////////////////////////////////////////////////////
// Script Types

ScriptType const ScriptType::GLOBAL(
		ScriptType::ID_GLOBAL, "global", ZVARTYPEID_VOID);
ScriptType const ScriptType::FFC(
		ScriptType::ID_FFC, "ffc", ZVARTYPEID_FFC);
ScriptType const ScriptType::ITEM(
		ScriptType::ID_ITEM, "item", ZVARTYPEID_ITEMCLASS);

