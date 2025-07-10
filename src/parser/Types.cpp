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
using std::shared_ptr;

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
const DataTypeArray* DataType::STRING; // set in ZScript::compile due to lifetime issues

////////////////////////////////////////////////////////////////
// DataType

int32_t DataType::compare(DataType const& rhs) const
{
	int lhsType = this->unique_type_id();
	int rhsType = rhs.unique_type_id();
	int delta = lhsType - rhsType;
	if (delta) return delta;
	return selfCompare(rhs);
}

DataType const* DataType::get(DataTypeId id)
{
	switch (id)
	{
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

DataType const* DataTypeUnresolved::resolve(Scope& scope, CompileErrorHandler* errorHandler)
{
	if (DataType const* type = lookupDataType(scope, *iden, errorHandler))
		return type;
	return this;
}
DataType const* DataTypeUnresolved::baseType(Scope& scope, CompileErrorHandler* errorHandler) const
{
	if (DataType const* type = lookupDataType(scope, *iden, errorHandler))
		return type;
	return nullptr;
}

DataType const& DataTypeUnresolved::getShared(DataType const& target, Scope const* scope) const
{
	return UNTYPED;
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

bool DataTypeSimple::canCastTo(DataType const& target, bool allowDeprecatedArrayCast) const
{
	if (isVoid() || target.isVoid()) return false;
	if (isUntyped() || target.isUntyped()) return true;
	if (target.isArray())
	{
		// Allows simple types to cast to an array.
		// Types that retain object references are not allowed to be cast like this, as that would break reference counting.
		// This should avoid breaking the vast majority of legacy arrays (which was mostly int).
		if (!isObject() && allowDeprecatedArrayCast)
			return canCastTo(static_cast<DataTypeArray const*>(&target)->getBaseType(), allowDeprecatedArrayCast);
		return false;
	}
	if (simpleId == ZTID_CHAR || simpleId == ZTID_LONG)
		return FLOAT.canCastTo(target, allowDeprecatedArrayCast); //Char/Long cast the same as float.

	if (DataTypeSimple const* t =
			dynamic_cast<DataTypeSimple const*>(&target))
	{
		if (t->simpleId == ZTID_CHAR || t->simpleId == ZTID_LONG)
			return canCastTo(FLOAT, allowDeprecatedArrayCast); //Char/Long cast the same as float.
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

DataType const& DataTypeSimple::getShared(DataType const& target, Scope const* scope) const
{
	if(isVoid() || target.isVoid()) return ZVOID;
	if(isUntyped()) return target;
	if(target.isUntyped()) return *this;
	if(target.isArray()) return UNTYPED;
	
	if (DataTypeSimple const* t =
			dynamic_cast<DataTypeSimple const*>(&target))
	{
		if (simpleId == t->simpleId)
			return target;
		int float_like = 0, is_bool = 0;
		for(auto v : {simpleId, t->simpleId})
			switch(simpleId)
			{
				case ZTID_FLOAT: case ZTID_CHAR: case ZTID_LONG:
					++float_like;
					break;
				case ZTID_BOOL:
					++is_bool;
					break;
			}
		if(float_like >= 2)
			return FLOAT;
		if(is_bool && float_like)
			return BOOL;
	}
	
	return UNTYPED;
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

bool DataTypeArray::canCastTo(DataType const& target, bool allowDeprecatedArrayCast) const
{
	if (target.isVoid()) return false;
	if (target.isUntyped()) return true;
	if (!target.isArray())
	{
		// Allows simple types to cast to an array.
		// Types that retain object references are not allowed to be cast like this, as that would break reference counting.
		// This should avoid breaking the vast majority of legacy arrays (which was mostly int).
		if (!canHoldObject() && allowDeprecatedArrayCast)
			return getBaseType().canCastTo(target, allowDeprecatedArrayCast);
		return false;
	}
	DataTypeArray const* targ_arr = static_cast<DataTypeArray const*>(&target);
	
	return getElementType().canCastTo(targ_arr->getElementType(), allowDeprecatedArrayCast);
}

DataType const& DataTypeArray::getShared(DataType const& target, Scope const* scope) const
{
	if(target.isVoid()) return target;
	if(target.isUntyped()) return *this;
	if(!target.isArray()) return UNTYPED;

	DataTypeArray const* targ_arr = static_cast<DataTypeArray const*>(&target);
	return *DataTypeArray::create(getElementType().getShared(targ_arr->getElementType(), scope));
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
	auto& ty = ZScript::getBaseType(elementType);
	return &ty;
}
DataType const& DataTypeArray::getBaseType() const
{
	return ZScript::getBaseType(elementType);
}
DataType const* DataTypeArray::getConstType() const
{
	if(isConstant()) return this;
	return create_depth(*getBaseType().getConstType(), getArrayDepth());
}
DataType const* DataTypeArray::getMutType() const
{
	if(!isConstant()) return this;
	return create_depth(*getBaseType().getMutType(), getArrayDepth());
}

std::vector<std::unique_ptr<DataTypeArray>> DataTypeArray::created_arr_types;
DataTypeArray const* DataTypeArray::create(DataType const& elementType)
{
	for(auto& ty : created_arr_types)
		if(ty->elementType == elementType)
			return ty.get();
	return created_arr_types.emplace_back(new DataTypeArray(elementType)).get();
}
DataTypeArray const* DataTypeArray::create_depth(DataType const& elementType, uint depth)
{
	DataType const* ret = &elementType;
	if(!depth) depth = 1;
	while(depth--)
		ret = create(*ret);
	return static_cast<DataTypeArray const*>(ret);
}
DataTypeArray const* DataTypeArray::create_owning(DataType* elementType)
{
	for(auto& ty : created_arr_types)
		if(ty->elementType == *elementType)
			return ty.get();
	auto& ty = created_arr_types.emplace_back(new DataTypeArray(*elementType));
	ty->owned_type.reset(elementType);
	return ty.get();
}

////////////////////////////////////////////////////////////////
// DataTypeCustom

bool DataTypeCustom::isEnum() const
{
	if (dynamic_cast<ASTCustomDataTypeDef*>(source))
		return true;

	return false;
}

bool DataTypeCustom::isBitflagsEnum() const
{
	if (auto enum_node = dynamic_cast<ASTCustomDataTypeDef*>(source))
		return enum_node->definition->getBitMode() != ASTDataEnum::BIT_NONE;

	return false;
}

bool DataTypeCustom::isLong() const
{
	if (auto enum_node = dynamic_cast<ASTCustomDataTypeDef*>(source))
		return enum_node->definition->getBitMode() == ASTDataEnum::BIT_LONG;

	return false;
}

bool DataTypeCustom::canHoldObject() const {
	if (!user_class)
		return false;

	if (user_class->getScope().getConstructors().empty())
		return false;

	return true;
}

bool DataTypeCustom::canCastTo(DataType const& target, bool allowDeprecatedArrayCast) const
{
	if (target.isVoid()) return false;
	if (target.isUntyped()) return true;
	if (target.isArray())
	{
		// Allows simple types to cast to an array.
		// Types that retain object references are not allowed to be cast like this, as that would break reference counting.
		// This should avoid breaking the vast majority of legacy arrays (which was mostly int).
		if (!canHoldObject() && allowDeprecatedArrayCast)
			return canCastTo(static_cast<DataTypeArray const*>(&target)->getBaseType(), allowDeprecatedArrayCast);
		return false;
	}
	
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
		UserClass* parent = user_class ? user_class->getParentClass() : nullptr;
		while (parent)
		{
			if (DataTypeCustom const* parent_type = dynamic_cast<DataTypeCustom const*>(parent->getType()))
			{
				if (parent_type->id == t->id)
					return true;
			}
			parent = parent->getParentClass();
		}

		//Enum-declared types and class types cannot cast to each other,
		//only within themselves
		return id == t->id;
	}
	
	return false;
}

DataType const& DataTypeCustom::getShared(DataType const& target, Scope const* scope) const
{
	if(target.isVoid()) return target;
	if(target.isUntyped()) return *this;
	if(target.isArray()) return UNTYPED;
	
	if(!isClass() && !isUsrClass())
	{
		if (DataTypeSimple const* t =
				dynamic_cast<DataTypeSimple const*>(&target))
		{
			if(t->getId() == ZTID_BOOL) //Can cast to bool
				return BOOL;
			if(t->getId() == ZTID_FLOAT //Shares FLOAT with FLOAT/LONG/CHAR
				|| t->getId() == ZTID_LONG
				|| t->getId() == ZTID_CHAR)
				return FLOAT;
		}
	}
	
	if (DataTypeCustom const* t =
			dynamic_cast<DataTypeCustom const*>(&target))
	{
		if(id == t->id) //Self-case
			return *this;
	}
	return UNTYPED;
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

/////////////////////////////////////////
// Templates

DataTypeTemplate* DataTypeTemplate::create(std::string const& name)
{
	static uint32_t uid = 0;
	auto id = uid++;
	DataTypeTemplateConst* constty = new DataTypeTemplateConst(name, id);
	DataTypeTemplate* mutty = new DataTypeTemplate(name, id, constty);
	//The constructor here actually clones 'constty', so we can delete the original
	delete constty;
	return mutty;
}
DataTypeTemplate::DataTypeTemplate(string const& name, uint32_t id, DataTypeTemplateConst* constty)
	: DataType(constty), name(name), id(id)
{
	if(constty)
		constty->mut_type = this;
}

bool DataTypeTemplate::canCastTo(DataType const& target, bool allowDeprecatedArrayCast) const
{
	if(target.isUntyped()) return true;
	if (DataTypeTemplate const* t = dynamic_cast<DataTypeTemplate const*>(&target))
		return id == t->id;
	return false;
}

DataType const& DataTypeTemplate::getShared(DataType const& target, Scope const* scope) const
{
	if(target.isUntyped()) return *this;
	if (DataTypeTemplate const* t = dynamic_cast<DataTypeTemplate const*>(&target))
		if(id == t->id)
			return *this;
	return UNTYPED;
}

int32_t DataTypeTemplate::selfCompare(DataType const& other) const
{
	DataTypeTemplate const& o = static_cast<DataTypeTemplate const&>(other);
	return id - o.id;
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
