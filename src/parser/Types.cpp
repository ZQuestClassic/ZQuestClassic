#include "../precompiled.h"
#include "Types.h"

#include <cassert>
#include <string>
#include <typeinfo>
#include "DataStructs.h"
#include "Library.h"
#include "Scope.h"

// using namespace std;

using namespace ZScript;
using namespace ZScript::detail;

////////////////////////////////////////////////////////////////
// TypeStore
TypeStore::TypeStore()
	: dataTypeGame(buildClass("Game", ZClass::Id_Game)),
	  dataTypeDebug(buildClass("Debug", ZClass::Id_Debug)),
	  dataTypeScreen(buildClass("Screen", ZClass::Id_Screen)),
	  dataTypeAudio(buildClass("Audio", ZClass::Id_Audio)),
	  dataTypeLink(buildClass("Link", ZClass::Id_Link)),
	  dataTypeItemClass(buildClass("ItemClass", ZClass::Id_ItemClass)),
	  dataTypeItem(buildClass("Item", ZClass::Id_Item)),
	  dataTypeNpcClass(buildClass("NpcClass", ZClass::Id_NpcClass)),
	  dataTypeNpc(buildClass("Npc", ZClass::Id_Npc)),
	  dataTypeFfc(buildClass("Ffc", ZClass::Id_Ffc)),
	  dataTypeLWpn(buildClass("LWpn", ZClass::Id_LWpn)),
	  dataTypeEWpn(buildClass("EWpn", ZClass::Id_EWpn))
{
	Libraries::Game::singleton().addTo(dataTypeGame->klass);
	Libraries::Debug::singleton().addTo(dataTypeDebug->klass);
	Libraries::Screen::singleton().addTo(dataTypeScreen->klass);
	Libraries::Audio::singleton().addTo(dataTypeAudio->klass);
	Libraries::Link::singleton().addTo(dataTypeLink->klass);
	Libraries::ItemClass::singleton().addTo(dataTypeItemClass->klass);
	Libraries::Item::singleton().addTo(dataTypeItem->klass);
	Libraries::NpcClass::singleton().addTo(dataTypeNpcClass->klass);
	Libraries::Npc::singleton().addTo(dataTypeNpc->klass);
	Libraries::Ffc::singleton().addTo(dataTypeFfc->klass);
	Libraries::LWeapon::singleton().addTo(dataTypeLWpn->klass);
	Libraries::EWeapon::singleton().addTo(dataTypeEWpn->klass);
}

TypeStore::~TypeStore()
{
	deleteElements(ownedDataTypes);
	deleteElements(ownedClasses);
}

// Builtin type accessors

DataType TypeStore::getVoid()
{
	return DataType(this, &DataTypeImplSimple::getVoid());
}

DataType TypeStore::getBool()
{
	return DataType(this, &DataTypeImplSimple::getBool());
}

DataType TypeStore::getFloat()
{
	return DataType(this, &DataTypeImplSimple::getFloat());
}

DataType TypeStore::getConstFloat()
{
	return DataType(this, &DataTypeImplConstFloat::singleton());
}

DataType TypeStore::getGame()
{
	return DataType(this, dataTypeGame);
}

DataType TypeStore::getDebug()
{
	return DataType(this, dataTypeDebug);
}

DataType TypeStore::getScreen()
{
	return DataType(this, dataTypeScreen);
}

DataType TypeStore::getAudio()
{
	return DataType(this, dataTypeAudio);
}

DataType TypeStore::getLink()
{
	return DataType(this, dataTypeLink);
}

DataType TypeStore::getItemClass()
{
	return DataType(this, dataTypeItemClass);
}

DataType TypeStore::getItem()
{
	return DataType(this, dataTypeItem);
}

DataType TypeStore::getNpcClass()
{
	return DataType(this, dataTypeNpcClass);
}

DataType TypeStore::getNpc()
{
	return DataType(this, dataTypeNpc);
}

DataType TypeStore::getFfc()
{
	return DataType(this, dataTypeFfc);
}

DataType TypeStore::getLWpn()
{
	return DataType(this, dataTypeLWpn);
}

DataType TypeStore::getEWpn()
{
	return DataType(this, dataTypeEWpn);
}

// Types

DataType TypeStore::getUnresolved(string const& name)
{
	DataTypeImplUnresolved const* newType =
		new DataTypeImplUnresolved(name);

	if (DataTypeImpl const* existingType =
			find<DataTypeImpl const*>(ownedDataTypes, newType)
			.value_or(NULL))
	{
		delete newType;
		return DataType(this, existingType);
	}

	ownedDataTypes.insert(newType);
	return DataType(this, newType);
}

DataType TypeStore::getArrayOf(DataType const& elementType)
{
	DataTypeImplArray const* newType =
		new DataTypeImplArray(*elementType.impl);

	if (DataTypeImpl const* existingType =
			find<DataTypeImpl const*>(ownedDataTypes, newType)
			.value_or(NULL))
	{
		delete newType;
		return DataType(this, existingType);
	}

	ownedDataTypes.insert(newType);
	return DataType(this, newType);
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

bool TypeStore::DataTypeImplPtrLess::operator()(
		DataTypeImpl const* const& lhs, DataTypeImpl const* const& rhs)
		const
{
	if (rhs == NULL) return false;
	if (lhs == NULL) return true;
	return *lhs < *rhs;
}

DataTypeImplClass const* TypeStore::buildClass(string const& name, int id)
{
	// Create builtin class.
	ZClass* klass = new ZClass(*this, name, id);
	ownedClasses.push_back(klass);
	// Create type for class.
	DataTypeImplClass const* impl = new DataTypeImplClass(*klass);
	assert(ownedDataTypes.insert(impl).second);
	// Return new type.
	return impl;
}

////////////////////////////////////////////////////////////////
// DataType

DataType::DataType()
	: impl(&DataTypeImplNull::singleton())
{}

bool DataType::isResolved() const {return impl->isResolved();}

void DataType::resolve(Scope& scope)
{
	if (!impl->isResolved())
		impl = impl->resolve(scope);
}

std::string DataType::getName() const {return impl->getName();}

bool DataType::canCastTo(DataType const& target) const
{
	return impl->canCastTo(*target.impl);
}

bool DataType::canBeGlobal() const {return impl->canBeGlobal();}

bool DataType::isArray() const {return impl->getElementType();}

optional<DataType> DataType::getElementType() const
{
	if (DataTypeImpl const* elementType = impl->getElementType())
		return DataType(container, elementType);
	return nullopt;
}

bool DataType::isConst() const {return impl->isConst();}

ZClass* DataType::getClass() const {return impl->getClass();}

int DataType::compare(DataType const& rhs) const
{
	return impl->compare(*rhs.impl);
}

DataType ZScript::arrayType(DataType const& elementType)
{
	return elementType.getTypeStore().getArrayOf(elementType);
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

int ZScript::getArrayDepth(DataType const& type)
{
	DataType t = type;
	int depth = 0;
	while (optional<DataType> elementType = t.getElementType())
	{
		t = *elementType;
		++depth;
	}
	return depth;
}

////////////////////////////////////////////////////////////////
// detail::DataTypeImpl

int DataTypeImpl::compare(DataTypeImpl const& rhs) const
{
	type_info const& lhsType = typeid(*this);
	type_info const& rhsType = typeid(rhs);
	if (lhsType.before(rhsType)) return -1;
	if (rhsType.before(lhsType)) return 1;
	return selfCompare(rhs);
}

bool ZScript::detail::operator==(
		DataTypeImpl const& lhs, DataTypeImpl const& rhs)
{
	return lhs.compare(rhs) == 0;
}

bool ZScript::detail::operator!=(
		DataTypeImpl const& lhs, DataTypeImpl const& rhs)
{
	return lhs.compare(rhs) != 0;
}

bool ZScript::detail::operator<(
		DataTypeImpl const& lhs, DataTypeImpl const& rhs)
{
	return lhs.compare(rhs) < 0;
}

bool ZScript::detail::operator<=(
		DataTypeImpl const& lhs, DataTypeImpl const& rhs)
{
	return lhs.compare(rhs) <= 0;
}

bool ZScript::detail::operator>(
		DataTypeImpl const& lhs, DataTypeImpl const& rhs)
{
	return lhs.compare(rhs) > 0;
}

bool ZScript::detail::operator>=(
		DataTypeImpl const& lhs, DataTypeImpl const& rhs)
{
	return lhs.compare(rhs) >= 0;
}

////////////////////////////////////////////////////////////////
// detail::DataTypeImplNull

DataTypeImplNull const& DataTypeImplNull::singleton()
{
	static DataTypeImplNull const instance;
	return instance;
}

////////////////////////////////////////////////////////////////
// detail::DataTypeImplUnresolved

DataTypeImpl const* DataTypeImplUnresolved::resolve(Scope& scope) const
{
	if (optional<DataType> type = lookupType(scope, name))
		return type->impl;
	return NULL;
}

int DataTypeImplUnresolved::selfCompare(DataTypeImpl const& rhs) const
{
	DataTypeImplUnresolved const& o =
		static_cast<DataTypeImplUnresolved const&>(rhs);
	return name.compare(o.name);
}

////////////////////////////////////////////////////////////////
// detail::DataTypeImplSimple

DataTypeImplSimple const& DataTypeImplSimple::getVoid()
{
	static DataTypeImplSimple const instance(Id_Void, "void");
	return instance;
}

DataTypeImplSimple const& DataTypeImplSimple::getBool()
{
	static DataTypeImplSimple const instance(Id_Bool, "bool");
	return instance;
}

DataTypeImplSimple const& DataTypeImplSimple::getFloat()
{
	static DataTypeImplSimple const instance(Id_Float, "float");
	return instance;
}

bool DataTypeImplSimple::canCastTo(DataTypeImpl const& target) const
{
	if (target.isConst())
		return canCastTo(*target.unconst());
	
	if (DataTypeImpl const* elementType = target.getElementType())
		return canCastTo(*elementType);

	if (DataTypeImplSimple const* t =
			dynamic_cast<DataTypeImplSimple const*>(&target))
	{
		if (id == Id_Void || t->id == Id_Void) return false;
		if (id == t->id) return true;
		if (id == Id_Float && t->id == Id_Bool) return true;
	}
	
	return false;
}

DataTypeImplSimple::DataTypeImplSimple(
		Id id, string const& name)
	: id(id), name(name)
{}

int DataTypeImplSimple::selfCompare(DataTypeImpl const& rhs) const
{
	DataTypeImplSimple const& o =
		static_cast<DataTypeImplSimple const&>(rhs);
	return id - o.id;
}

////////////////////////////////////////////////////////////////
// detail::DataTypeImplConstFloat

DataTypeImplConstFloat const& DataTypeImplConstFloat::singleton()
{
	static DataTypeImplConstFloat const instance;
	return instance;
}

bool DataTypeImplConstFloat::canCastTo(DataTypeImpl const& target) const
{
	if (*this == target) return true;

	return DataTypeImplSimple::getFloat().canCastTo(target);
}

////////////////////////////////////////////////////////////////
// detail::DataTypeImplClass

string DataTypeImplClass::getName() const
{
	return klass.getName().value_or(string("anonymous_class"));
}

bool DataTypeImplClass::canCastTo(DataTypeImpl const& target) const
{
	if (ZClass* targetKlass = target.getClass())
		return &klass == targetKlass; // XXX add ZClass comparison?
	
	if (DataTypeImpl const* elementType = target.getElementType())
		return canCastTo(*elementType);

	return false;
}

int DataTypeImplClass::selfCompare(DataTypeImpl const& rhs) const
{
	DataTypeImplClass const& o =
		static_cast<DataTypeImplClass const&>(rhs);
	return &klass - &o.klass; // XXX add ZClass comparison?
}

////////////////////////////////////////////////////////////////
// detail::DataTypeImplArray

bool DataTypeImplArray::canCastTo(DataTypeImpl const& target) const
{
	if (DataTypeImpl const* targetElementType = target.getElementType())
		return canCastTo(*targetElementType);
	
	return elementType.canCastTo(target);
}

int DataTypeImplArray::selfCompare(DataTypeImpl const& rhs) const
{
	DataTypeImplArray const& o =
		static_cast<DataTypeImplArray const&>(rhs);
	return elementType.compare(o.elementType);
}

////////////////////////////////////////////////////////////////
// Script Types

ScriptType const& ScriptType::getGlobal()
{
	static ScriptType const instance(Id_Global, "global");
	return instance;
}

ScriptType const& ScriptType::getFfc()
{
	static ScriptType const instance(Id_Ffc, "ffc");
	return instance;
}

ScriptType const& ScriptType::getItem()
{
	static ScriptType const instance(Id_Item, "item");
	return instance;
}

ScriptType::ScriptType() : id(Id_Null), name("null") {}

optional<DataType> ScriptType::getThisType(TypeStore& typeStore) const
{
	if (id == Id_Ffc) return typeStore.getFfc();
	if (id == Id_Item) return typeStore.getItemClass();
	return nullopt;
}

ScriptType::ScriptType(Id id, string const& name)
	: id(id), name(name)
{}

