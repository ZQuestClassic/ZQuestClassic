#include <cstdio>
#include <string>
#include <typeinfo>
#include "CompileError.h"
#include "DataStructs.h"
#include "Scope.h"
#include "Types.h"
#include "AST.h"

using namespace ZScript;
using std::string;

////////////////////////////////////////////////////////////////
// TypeStore

TypeStore::TypeStore()
{
	// Assign builtin types.
	for (DataTypeId id = ZTID_PRIMITIVE_START; id < ZTID_PRIMITIVE_END; ++id)
		assignTypeId(*DataType::get(id));
}

TypeStore::~TypeStore()
{
	deleteElements(ownedTypes);
}

// Types

DataType const* TypeStore::getType(DataTypeId typeId) const
{
	if (typeId < 0 || typeId > (int32_t)ownedTypes.size()) return NULL;
	return ownedTypes[typeId];
}

std::optional<DataTypeId> TypeStore::getTypeId(DataType const& type) const
{
	return find<DataTypeId>(typeIdMap, &type);
}

std::optional<DataTypeId> TypeStore::assignTypeId(DataType const& type)
{
	if (!type.isResolved())
	{
		log_error(CompileError::UnresolvedType(NULL, type.getName()));
		return std::nullopt;
	}

	if (find<DataTypeId>(typeIdMap, &type)) return std::nullopt;

	DataTypeId id = ownedTypes.size();
	DataType const* storedType = type.clone();
	ownedTypes.push_back(storedType);
	typeIdMap[storedType] = id;
	return id;
}

std::optional<DataTypeId> TypeStore::getOrAssignTypeId(DataType const& type)
{
	if (!type.isResolved())
	{
		log_error(CompileError::UnresolvedType(NULL, type.getName()));
		return std::nullopt;
	}

	if (std::optional<DataTypeId> typeId = find<DataTypeId>(typeIdMap, &type))
		return typeId;
	
	DataTypeId id = ownedTypes.size();
	DataType* storedType = type.clone();
	ownedTypes.push_back(storedType);
	typeIdMap[storedType] = id;
	return id;
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

// Template types.
DataTypeSimpleConst DataType::CTEMPLATE_T(ZTID_TEMPLATE_T, "const T");
DataTypeSimpleConst DataType::CTEMPLATE_T_ARR(ZTID_TEMPLATE_T_ARR, "const T[]");
DataTypeSimple DataType::TEMPLATE_T(ZTID_TEMPLATE_T, "T", &CTEMPLATE_T);
DataTypeSimple DataType::TEMPLATE_T_ARR(ZTID_TEMPLATE_T_ARR, "T[]", &CTEMPLATE_T_ARR);
// Standard Type definitions.
DataTypeSimpleConst DataType::CAUTO(ZTID_AUTO, "const auto");
DataTypeSimpleConst DataType::CUNTYPED(ZTID_UNTYPED, "const untyped");
DataTypeSimpleConst DataType::CFLOAT(ZTID_FLOAT, "const int");
DataTypeSimpleConst DataType::CCHAR(ZTID_CHAR, "const char32");
DataTypeSimpleConst DataType::CLONG(ZTID_LONG, "const long");
DataTypeSimpleConst DataType::CBOOL(ZTID_BOOL, "const bool");
DataTypeSimpleConst DataType::CRGBDATA(ZTID_RGBDATA, "const rgb");
DataTypeSimple DataType::UNTYPED(ZTID_UNTYPED, "untyped", &CUNTYPED);
DataTypeSimple DataType::ZAUTO(ZTID_AUTO, "auto", &CAUTO);
DataTypeSimple DataType::ZVOID(ZTID_VOID, "void", NULL);
DataTypeSimple DataType::FLOAT(ZTID_FLOAT, "int", &CFLOAT);
DataTypeSimple DataType::CHAR(ZTID_CHAR, "char32", &CCHAR);
DataTypeSimple DataType::LONG(ZTID_LONG, "long", &CLONG);
DataTypeSimple DataType::BOOL(ZTID_BOOL, "bool", &CBOOL);
DataTypeSimple DataType::RGBDATA(ZTID_RGBDATA, "rgb", &CRGBDATA);
DataTypeArray DataType::STRING(CHAR);

////////////////////////////////////////////////////////////////
// DataType

int32_t DataType::compare(DataType const& rhs) const
{
	std::type_info const& lhsType = typeid(*this);
	std::type_info const& rhsType = typeid(rhs);
	if (lhsType.before(rhsType)) return -1;
	if (rhsType.before(lhsType)) return 1;
	return selfCompare(rhs);
}

DataType const* DataType::get(DataTypeId id)
{
	switch (id)
	{
		case ZTID_TEMPLATE_T: return &TEMPLATE_T;
		case ZTID_TEMPLATE_T_ARR: return &TEMPLATE_T_ARR;
		case ZTID_UNTYPED: return &UNTYPED;
		case ZTID_AUTO: return &ZAUTO;
		case ZTID_VOID: return &ZVOID;
		case ZTID_FLOAT: return &FLOAT;
		case ZTID_CHAR: return &CHAR;
		case ZTID_LONG: return &LONG;
		case ZTID_BOOL: return &BOOL;
		case ZTID_RGBDATA: return &RGBDATA;
		default: return NULL;
	}
}

void DataType::addCustom(DataTypeCustom* custom)
{
	customTypes[custom->getCustomId()] = custom;
}

int32_t DataType::nextCustomId_;
std::map<int32_t, DataTypeCustom*> DataType::customTypes;

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

DataType const& ZScript::getNaiveType(DataType const& type, Scope* scope)
{

	DataType const* t = &type;
	while (t->isArray()) //Avoid dynamic_cast<>
	{
		DataTypeArray const* ta = static_cast<DataTypeArray const*>(t);
		t = &ta->getElementType();
	}
	
	//Convert constant types to their variable counterpart
	if(t->isConstant())
	{
		if(DataTypeSimpleConst const* ts = dynamic_cast<DataTypeSimpleConst const*>(t))
		{
			t = DataType::get(ts->getId());
		}
		
		if(DataTypeCustomConst const* tcu = dynamic_cast<DataTypeCustomConst const*>(t))
		{
			t = DataType::getCustom(tcu->getCustomId());
		}
	}

	return *t;
}

int32_t ZScript::getArrayDepth(DataType const& type)
{
	DataType const* ptype = &type;
	int32_t depth = 0;
	while (DataTypeArray const* t = dynamic_cast<DataTypeArray const*>(ptype))
	{
		++depth;
		ptype = &t->getElementType();
	}
	return depth;
}

////////////////////////////////////////////////////////////////
// DataTypeUnresolved

DataTypeUnresolved::DataTypeUnresolved(ASTExprIdentifier* iden)
	: DataType(NULL), iden(iden)
{
	name = iden->components.back();
}

DataTypeUnresolved::~DataTypeUnresolved()
{
	delete iden;
}

DataTypeUnresolved* DataTypeUnresolved::clone() const
{
	DataTypeUnresolved* copy = new DataTypeUnresolved(*this);
	copy->iden = iden->clone();
	return copy;
}

DataType* DataTypeUnresolved::resolve(Scope& scope, CompileErrorHandler* errorHandler)
{
	if (DataType const* type = lookupDataType(scope, *iden, errorHandler))
		return type->clone();
	return this;
}
DataType const* DataTypeUnresolved::baseType(Scope& scope, CompileErrorHandler* errorHandler) const
{
	if (DataType const* type = lookupDataType(scope, *iden, errorHandler))
		return type;
	return nullptr;
}
 
std::string DataTypeUnresolved::getName() const
{
	return name;
}

int32_t DataTypeUnresolved::selfCompare(DataType const& rhs) const
{
	DataTypeUnresolved const& o = static_cast<DataTypeUnresolved const&>(rhs);
	return name.compare(name);
}

////////////////////////////////////////////////////////////////
// DataTypeSimple

DataTypeSimple::DataTypeSimple(int32_t simpleId, string const& name, DataType* constType)
	: DataType(constType), simpleId(simpleId), name(name)
{}

int32_t DataTypeSimple::selfCompare(DataType const& rhs) const
{
	DataTypeSimple const& o = static_cast<DataTypeSimple const&>(rhs);
	return simpleId - o.simpleId;
}

bool DataTypeSimple::canCastTo(DataType const& target) const
{
	if (isVoid() || target.isVoid()) return false;
	if (isUntyped() || target.isUntyped()) return true;
	if (simpleId == ZTID_CHAR || simpleId == ZTID_LONG)
		return FLOAT.canCastTo(target); //Char/Long cast the same as float.

	if (DataTypeArray const* t =
			dynamic_cast<DataTypeArray const*>(&target))
		return canCastTo(getBaseType(*t));

	if (DataTypeSimple const* t =
			dynamic_cast<DataTypeSimple const*>(&target))
	{
		if (t->simpleId == ZTID_CHAR || t->simpleId == ZTID_LONG)
			return canCastTo(FLOAT); //Char/Long cast the same as float.
		if (simpleId == ZTID_UNTYPED || t->simpleId == ZTID_UNTYPED)
			return true;
		if (simpleId == ZTID_VOID || t->simpleId == ZTID_VOID)
			return false;
		if (simpleId == t->simpleId)
			return true;
		if (simpleId == ZTID_FLOAT && t->simpleId == ZTID_BOOL)
			return true;
	}
	
	return false;
}

bool DataTypeSimple::canBeGlobal() const
{
	return true; //All types can be global, now. 
	//return simpleId == ZTID_FLOAT || simpleId == ZTID_BOOL;
}
DataType const* DataTypeSimple::baseType(Scope& scope, CompileErrorHandler* errorHandler) const
{
	return DataType::get(simpleId);
}

////////////////////////////////////////////////////////////////
// DataTypeSimpleConst

DataTypeSimpleConst::DataTypeSimpleConst(int32_t simpleId, string const& name)
	: DataTypeSimple(simpleId, name, NULL)
{}
DataType const* DataTypeSimpleConst::baseType(Scope& scope, CompileErrorHandler* errorHandler) const
{
	auto* ty = DataType::get(simpleId);
	return ty ? ty->getConstType() : nullptr;
}

////////////////////////////////////////////////////////////////
// DataTypeArray

bool DataTypeArray::canCastTo(DataType const& target) const
{
	if (target.isVoid()) return false;
	if (target.isUntyped()) return true;
	
	if (DataTypeArray const* t =
			dynamic_cast<DataTypeArray const*>(&target))
		return canCastTo(getBaseType(*t));
	
	return getBaseType(*this).canCastTo(target);
}

int32_t DataTypeArray::selfCompare(DataType const& rhs) const
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
DataType const* DataTypeArray::baseType(Scope& scope, CompileErrorHandler* errorHandler) const
{
	auto& ty = getBaseType(elementType);
	return &ty;
}

////////////////////////////////////////////////////////////////
// DataTypeCustom

bool DataTypeCustom::canHoldObject() const {
	if (!user_class)
		return false;

	if (user_class->getScope().getConstructors().empty())
		return false;

	return true;
}

bool DataTypeCustom::canCastTo(DataType const& target) const
{
	if (target.isVoid()) return false;
	if (target.isUntyped()) return true;

	if (DataTypeArray const* t =
			dynamic_cast<DataTypeArray const*>(&target))
		return canCastTo(getBaseType(*t));
	
	if(!isClass() && !isUsrClass())
	{
		if (DataTypeSimple const* t =
				dynamic_cast<DataTypeSimple const*>(&target))
		{
			//Enum-declared types can be cast to any non-void simple
			return(t->getId() == ZTID_UNTYPED
				|| t->getId() == ZTID_BOOL
				|| t->getId() == ZTID_FLOAT
				|| t->getId() == ZTID_LONG
				|| t->getId() == ZTID_CHAR);
		}
	}
	
	if (DataTypeCustom const* t =
			dynamic_cast<DataTypeCustom const*>(&target))
	{
		//Enum-declared types and class types cannot cast to each other,
		//only within themselves
		return id == t->id;
	}
	
	return false;
}

int32_t DataTypeCustom::selfCompare(DataType const& other) const
{
	DataTypeCustom const& o = static_cast<DataTypeCustom const&>(other);
	return id - o.id;
}

DataType const* DataTypeCustom::baseType(Scope& scope, CompileErrorHandler* errorHandler) const
{
	return DataType::getCustom(id);
}

DataType const* DataTypeCustomConst::baseType(Scope& scope, CompileErrorHandler* errorHandler) const
{
	auto* ty = DataType::getCustom(id);
	return ty ? ty->getConstType() : nullptr;
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
	ScriptTypeData scriptTypes[ParserScriptType::idEnd] = {
		{"invalid", ZTID_VOID},
		{"global", ZTID_VOID},
		{"ffc", ZTID_FFC},
		{"itemdata", ZTID_ITEMCLASS},
		{"npc", ZTID_NPC},
		{"eweapon", ZTID_EWPN},
		{"lweapon", ZTID_LWPN},
		{"player", ZTID_PLAYER},
		{"screendata", ZTID_SCREEN},
		{"dmapdata", ZTID_DMAPDATA},
		{"itemsprite", ZTID_ITEM},
		{"untyped", ZTID_VOID},
		{"combodata", ZTID_COMBOS},
		{"subscreendata", ZTID_SUBSCREENDATA},
		{"generic",ZTID_GENERICDATA},
	};
}

ParserScriptType const ParserScriptType::invalid(idInvalid);
ParserScriptType const ParserScriptType::global(idGlobal);
ParserScriptType const ParserScriptType::ffc(idFfc);
ParserScriptType const ParserScriptType::item(idItem);
ParserScriptType const ParserScriptType::npc(idNPC);
ParserScriptType const ParserScriptType::lweapon(idLWeapon);
ParserScriptType const ParserScriptType::eweapon(idEWeapon);
ParserScriptType const ParserScriptType::player(idPlayer);
ParserScriptType const ParserScriptType::screendata(idScreen);
ParserScriptType const ParserScriptType::dmapdata(idDMap);
ParserScriptType const ParserScriptType::itemsprite(idItemSprite);
ParserScriptType const ParserScriptType::untyped(idUntyped);
ParserScriptType const ParserScriptType::subscreendata(idSubscreenData);
ParserScriptType const ParserScriptType::combodata(idComboData);
ParserScriptType const ParserScriptType::genericscr(idGenericScript);

string const& ParserScriptType::getName() const
{
	if (isValid()) return scriptTypes[id_].name;
	return scriptTypes[idInvalid].name;
}

DataTypeId ParserScriptType::getThisTypeId() const
{
	if (isValid()) return scriptTypes[id_].thisTypeId;
	return scriptTypes[idInvalid].thisTypeId;
}

bool ZScript::operator==(ParserScriptType const& lhs, ParserScriptType const& rhs)
{
	if (!lhs.isValid()) return !rhs.isValid();
	return lhs.id_ == rhs.id_;
}

bool ZScript::operator!=(ParserScriptType const& lhs, ParserScriptType const& rhs)
{
	if (lhs.isValid()) return lhs.id_ != rhs.id_;
	return rhs.isValid();
}
