#include <cstdio>
#include <string>
#include <typeinfo>
#include "CompileError.h"
#include "DataStructs.h"
#include "Scope.h"
#include "Types.h"
#include "AST.h"

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
DataTypeSimpleConst DataType::CUNTYPED(ZVARTYPEID_UNTYPED, "const untyped");
DataTypeSimpleConst DataType::CFLOAT(ZVARTYPEID_FLOAT, "const float");
DataTypeSimpleConst DataType::CCHAR(ZVARTYPEID_CHAR, "const char32");
DataTypeSimpleConst DataType::CBOOL(ZVARTYPEID_BOOL, "const bool");
DataTypeSimple DataType::UNTYPED(ZVARTYPEID_UNTYPED, "untyped", &CUNTYPED);
DataTypeSimple DataType::ZVOID(ZVARTYPEID_VOID, "void", NULL);
DataTypeSimple DataType::FLOAT(ZVARTYPEID_FLOAT, "float", &CFLOAT);
DataTypeSimple DataType::CHAR(ZVARTYPEID_CHAR, "char32", &CCHAR);
DataTypeSimple DataType::BOOL(ZVARTYPEID_BOOL, "bool", &CBOOL);
DataTypeArray DataType::STRING(CHAR);
//Classes: Global Pointer
DataTypeClassConst DataType::GAME(ZCLASSID_GAME, "Game");
DataTypeClassConst DataType::LINK(ZCLASSID_LINK, "Link");
DataTypeClassConst DataType::SCREEN(ZCLASSID_SCREEN, "Screen");
DataTypeClassConst DataType::AUDIO(ZCLASSID_AUDIO, "Audio");
DataTypeClassConst DataType::DEBUG(ZCLASSID_DEBUG, "Debug");
DataTypeClassConst DataType::GRAPHICS(ZCLASSID_GRAPHICS, "Graphics");
DataTypeClassConst DataType::INPUT(ZCLASSID_INPUT, "Input");
DataTypeClassConst DataType::TEXT(ZCLASSID_TEXT, "Text");
DataTypeClassConst DataType::FILESYSTEM(ZCLASSID_FILESYSTEM, "FileSystem");
//Class: Types
DataTypeClassConst DataType::CBITMAP(ZCLASSID_BITMAP, "const Bitmap");
DataTypeClassConst DataType::CCHEATS(ZCLASSID_CHEATS, "const Cheats");
DataTypeClassConst DataType::CCOMBOS(ZCLASSID_COMBOS, "const Combos");
DataTypeClassConst DataType::CDOORSET(ZCLASSID_DOORSET, "const DoorSet");
DataTypeClassConst DataType::CDROPSET(ZCLASSID_DROPSET, "const DropSet");
DataTypeClassConst DataType::CDMAPDATA(ZCLASSID_DMAPDATA, "const DMapData");
DataTypeClassConst DataType::CEWPN(ZCLASSID_EWPN, "const EWeapon");
DataTypeClassConst DataType::CFFC(ZCLASSID_FFC, "const FFC");
DataTypeClassConst DataType::CGAMEDATA(ZCLASSID_GAMEDATA, "const GameData");
DataTypeClassConst DataType::CITEM(ZCLASSID_ITEM, "const Item");
DataTypeClassConst DataType::CITEMCLASS(ZCLASSID_ITEMCLASS, "const ItemData");
DataTypeClassConst DataType::CLWPN(ZCLASSID_LWPN, "const LWeapon");
DataTypeClassConst DataType::CMAPDATA(ZCLASSID_MAPDATA, "const MapData");
DataTypeClassConst DataType::CZMESSAGE(ZCLASSID_ZMESSAGE, "const ZMessage");
DataTypeClassConst DataType::CZUICOLOURS(ZCLASSID_ZUICOLOURS, "const ZuiColours");
DataTypeClassConst DataType::CNPC(ZCLASSID_NPC, "const NPC");
DataTypeClassConst DataType::CNPCDATA(ZCLASSID_NPCDATA, "const NPCData");
DataTypeClassConst DataType::CPALCYCLE(ZCLASSID_PALCYCLE, "const PalCycle");
DataTypeClassConst DataType::CPALETTE(ZCLASSID_PALETTE, "const Palette");
DataTypeClassConst DataType::CPONDS(ZCLASSID_PONDS, "const Ponds");
DataTypeClassConst DataType::CRGBDATA(ZCLASSID_RGBDATA, "const RgbData");
DataTypeClassConst DataType::CSHOPDATA(ZCLASSID_SHOPDATA, "const ShopData");
DataTypeClassConst DataType::CSPRITEDATA(ZCLASSID_SPRITEDATA, "const SpriteData");
DataTypeClassConst DataType::CTUNES(ZCLASSID_TUNES, "const Tunes");
DataTypeClassConst DataType::CWARPRING(ZCLASSID_WARPRING, "const WarpRing");
DataTypeClassConst DataType::CSUBSCREENDATA(ZCLASSID_SUBSCREENDATA, "const SubscreenData");
DataTypeClassConst DataType::CFILE(ZCLASSID_FILE, "const File");
//Class: Var Types
DataTypeClass DataType::BITMAP(ZCLASSID_BITMAP, "Bitmap", &CBITMAP);
DataTypeClass DataType::CHEATS(ZCLASSID_CHEATS, "Cheats", &CCHEATS);
DataTypeClass DataType::COMBOS(ZCLASSID_COMBOS, "Combos", &CCOMBOS);
DataTypeClass DataType::DOORSET(ZCLASSID_DOORSET, "DoorSet", &CDOORSET);
DataTypeClass DataType::DROPSET(ZCLASSID_DROPSET, "DropSet", &CDROPSET);
DataTypeClass DataType::DMAPDATA(ZCLASSID_DMAPDATA, "DMapData", &CDMAPDATA);
DataTypeClass DataType::EWPN(ZCLASSID_EWPN, "EWeapon", &CEWPN);
DataTypeClass DataType::FFC(ZCLASSID_FFC, "FFC", &CFFC);
DataTypeClass DataType::GAMEDATA(ZCLASSID_GAMEDATA, "GameData", &CGAMEDATA);
DataTypeClass DataType::ITEM(ZCLASSID_ITEM, "Item", &CITEM);
DataTypeClass DataType::ITEMCLASS(ZCLASSID_ITEMCLASS, "ItemData", &CITEMCLASS);
DataTypeClass DataType::LWPN(ZCLASSID_LWPN, "LWeapon", &CLWPN);
DataTypeClass DataType::MAPDATA(ZCLASSID_MAPDATA, "MapData", &CMAPDATA);
DataTypeClass DataType::ZMESSAGE(ZCLASSID_ZMESSAGE, "ZMessage", &CZMESSAGE);
DataTypeClass DataType::ZUICOLOURS(ZCLASSID_ZUICOLOURS, "ZuiColours", &CZUICOLOURS);
DataTypeClass DataType::NPC(ZCLASSID_NPC, "NPC", &CNPC);
DataTypeClass DataType::NPCDATA(ZCLASSID_NPCDATA, "NPCData", &CNPCDATA);
DataTypeClass DataType::PALCYCLE(ZCLASSID_PALCYCLE, "PalCycle", &CPALCYCLE);
DataTypeClass DataType::PALETTE(ZCLASSID_PALETTE, "Palette", &CPALETTE);
DataTypeClass DataType::PONDS(ZCLASSID_PONDS, "Ponds", &CPONDS);
DataTypeClass DataType::RGBDATA(ZCLASSID_RGBDATA, "RgbData", &CRGBDATA);
DataTypeClass DataType::SHOPDATA(ZCLASSID_SHOPDATA, "ShopData", &CSHOPDATA);
DataTypeClass DataType::SPRITEDATA(ZCLASSID_SPRITEDATA, "SpriteData", &CSPRITEDATA);
DataTypeClass DataType::TUNES(ZCLASSID_TUNES, "Tunes", &CTUNES);
DataTypeClass DataType::WARPRING(ZCLASSID_WARPRING, "WarpRing", &CWARPRING);
DataTypeClass DataType::SUBSCREENDATA(ZCLASSID_SUBSCREENDATA, "SubscreenData", &CSUBSCREENDATA);
DataTypeClass DataType::FILE(ZCLASSID_FILE, "File", &CFILE);

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
		case ZVARTYPEID_CHAR: return &CHAR;
		case ZVARTYPEID_BOOL: return &BOOL;
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
		case ZVARTYPEID_SUBSCREENDATA: return &SUBSCREENDATA;
		case ZVARTYPEID_FILE: return &FILE;
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
		case ZVARTYPEID_FILESYSTEM: return &FILESYSTEM;
		default: return NULL;
	}
}

DataTypeClass const* DataType::getClass(int classId)
{
	switch (classId)
	{
		case ZCLASSID_GAME: return &GAME;
		case ZCLASSID_LINK: return &LINK;
		case ZCLASSID_SCREEN: return &SCREEN;
		case ZCLASSID_FFC: return &FFC;
		case ZCLASSID_ITEM: return &ITEM;
		case ZCLASSID_ITEMCLASS: return &ITEMCLASS;
		case ZCLASSID_NPC: return &NPC;
		case ZCLASSID_LWPN: return &LWPN;
		case ZCLASSID_EWPN: return &EWPN;
		case ZCLASSID_NPCDATA: return &NPCDATA;
		case ZCLASSID_DEBUG: return &DEBUG;
		case ZCLASSID_AUDIO: return &AUDIO;
		case ZCLASSID_COMBOS: return &COMBOS;
		case ZCLASSID_SPRITEDATA: return &SPRITEDATA;
		case ZCLASSID_SUBSCREENDATA: return &SUBSCREENDATA;
		case ZCLASSID_FILE: return &FILE;
		case ZCLASSID_GRAPHICS: return &GRAPHICS;
		case ZCLASSID_BITMAP: return &BITMAP;
		case ZCLASSID_TEXT: return &TEXT;
		case ZCLASSID_INPUT: return &INPUT;
		case ZCLASSID_MAPDATA: return &MAPDATA;
		case ZCLASSID_DMAPDATA: return &DMAPDATA;
		case ZCLASSID_ZMESSAGE: return &ZMESSAGE;
		case ZCLASSID_SHOPDATA: return &SHOPDATA;
		case ZCLASSID_DROPSET: return &DROPSET;
		case ZCLASSID_PONDS: return &PONDS;
		case ZCLASSID_WARPRING: return &WARPRING;
		case ZCLASSID_DOORSET: return &DOORSET;
		case ZCLASSID_ZUICOLOURS: return &ZUICOLOURS;
		case ZCLASSID_RGBDATA: return &RGBDATA;
		case ZCLASSID_PALETTE: return &PALETTE;
		case ZCLASSID_TUNES: return &TUNES;
		case ZCLASSID_PALCYCLE: return &PALCYCLE;
		case ZCLASSID_GAMEDATA: return &GAMEDATA;
		case ZCLASSID_CHEATS: return &CHEATS;
		case ZCLASSID_FILESYSTEM: return &FILESYSTEM;
		default: return NULL;
	}
}

void DataType::addCustom(DataTypeCustom* custom)
{
	customTypes[custom->getCustomId()] = custom;
}

int DataType::nextCustomId_;
std::map<int, DataTypeCustom*> DataType::customTypes;

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
		
		if(DataTypeClassConst const* tc = dynamic_cast<DataTypeClassConst const*>(t))
		{
			t = DataType::getClass(tc->getClassId());
		}
		
		if(DataTypeCustomConst const* tcu = dynamic_cast<DataTypeCustomConst const*>(t))
		{
			t = DataType::getCustom(tcu->getCustomId());
		}
	}

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
	return NULL;
}
 
std::string DataTypeUnresolved::getName() const
{
	return name;
}

int DataTypeUnresolved::selfCompare(DataType const& rhs) const
{
	DataTypeUnresolved const& o = static_cast<DataTypeUnresolved const&>(rhs);
	return name.compare(name);
}

////////////////////////////////////////////////////////////////
// DataTypeSimple

DataTypeSimple::DataTypeSimple(int simpleId, string const& name, DataType* constType)
	: DataType(constType), simpleId(simpleId), name(name)
{}

int DataTypeSimple::selfCompare(DataType const& rhs) const
{
	DataTypeSimple const& o = static_cast<DataTypeSimple const&>(rhs);
	return simpleId - o.simpleId;
}

bool DataTypeSimple::canCastTo(DataType const& target) const
{
	if (isVoid() || target.isVoid()) return false;
	if (isUntyped() || target.isUntyped()) return true;
	if (simpleId == ZVARTYPEID_CHAR) return FLOAT.canCastTo(target); //Char casts the same as float.

	if (DataTypeArray const* t =
			dynamic_cast<DataTypeArray const*>(&target))
		return canCastTo(getBaseType(*t));

	if (DataTypeSimple const* t =
			dynamic_cast<DataTypeSimple const*>(&target))
	{
		if (t->simpleId == ZVARTYPEID_CHAR) return canCastTo(FLOAT); //Char casts the same as float.
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
	return true; //All types can be global, now. 
	//return simpleId == ZVARTYPEID_FLOAT || simpleId == ZVARTYPEID_BOOL;
}

////////////////////////////////////////////////////////////////
// DataTypeSimpleConst

DataTypeSimpleConst::DataTypeSimpleConst(int simpleId, string const& name)
	: DataTypeSimple(simpleId, name, NULL)
{}

////////////////////////////////////////////////////////////////
// DataTypeClass

DataTypeClass::DataTypeClass(int classId, DataType* constType)
	: DataType(constType), classId(classId), className("")
{}

DataTypeClass::DataTypeClass(int classId, string const& className, DataType* constType)
	: DataType(constType), classId(classId), className(className)
{}

DataTypeClass* DataTypeClass::resolve(Scope& scope, CompileErrorHandler* errorHandler)
{
	// Grab the proper name for the class the first time it's resolved.
	if (className == "")
		className = scope.getTypeStore().getClass(classId)->name;

	return this;
}

string DataTypeClass::getName() const
{
	/* This doesn't look good in errors/warns...
	string name = className == "" ? "anonymous" : className;
	char tmp[32];
	sprintf(tmp, "%d", classId);
	return name + "[class " + tmp + "]";*/
	return className;
}

bool DataTypeClass::canCastTo(DataType const& target) const
{
	if (target.isVoid()) return false;
	if (target.isUntyped()) return true;
	
	if (DataTypeArray const* t =
			dynamic_cast<DataTypeArray const*>(&target))
		return canCastTo(getBaseType(*t));

	if (DataTypeClass const* t =
			dynamic_cast<DataTypeClass const*>(&target))
		return classId == t->classId;
		
	return false;
}

int DataTypeClass::selfCompare(DataType const& rhs) const
{
	DataTypeClass const& o = static_cast<DataTypeClass const&>(rhs);
	return classId - o.classId;
}

////////////////////////////////////////////////////////////////
// DataTypeClassConst

DataTypeClassConst::DataTypeClassConst(int classId, string const& name)
	: DataTypeClass(classId, name, NULL)
{}

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
// DataTypeCustom

bool DataTypeCustom::canCastTo(DataType const& target) const
{
	if (target.isVoid()) return false;
	if (target.isUntyped()) return true;

	if (DataTypeArray const* t =
			dynamic_cast<DataTypeArray const*>(&target))
		return canCastTo(getBaseType(*t));

	if (DataTypeSimple const* t =
			dynamic_cast<DataTypeSimple const*>(&target))
	{
		//Enum-declared types can be cast to any non-void simple
		return(t->getId() == ZVARTYPEID_UNTYPED
			|| t->getId() == ZVARTYPEID_BOOL
			|| t->getId() == ZVARTYPEID_FLOAT
			|| t->getId() == ZVARTYPEID_CHAR);
	}
	
	if (DataTypeCustom const* t =
			dynamic_cast<DataTypeCustom const*>(&target))
	{
		//Enum-declared types cannot cast to each other, only within themselves, or to simple
		return id == t->id;
	}
	
	return false;
}

int DataTypeCustom::selfCompare(DataType const& other) const
{
	DataTypeCustom const& o = static_cast<DataTypeCustom const&>(other);
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
	//the 'this' 'this->' stuff. -Z
	ScriptTypeData scriptTypes[ScriptType::idEnd] = {
		{"invalid", ZVARTYPEID_VOID},
		{"global", ZVARTYPEID_VOID},
		{"ffc", ZVARTYPEID_FFC},
		{"item", ZVARTYPEID_ITEMCLASS},
		{"npc", ZVARTYPEID_NPC},
		{"eweapon", ZVARTYPEID_EWPN},
		{"lweapon", ZVARTYPEID_LWPN},
		{"link", ZVARTYPEID_LINK},
		{"screendata", ZVARTYPEID_SCREEN},
		{"dmapdata", ZVARTYPEID_DMAPDATA},
		{"itemsprite", ZVARTYPEID_ITEM},
		{"untyped", ZVARTYPEID_VOID},
		{"combodata", ZVARTYPEID_COMBOS},
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
ScriptType const ScriptType::screendata(idScreen);
ScriptType const ScriptType::dmapdata(idDMap);
ScriptType const ScriptType::itemsprite(idItemSprite);
ScriptType const ScriptType::untyped(idUntyped);
ScriptType const ScriptType::subscreendata(idSubscreenData);
ScriptType const ScriptType::combodata(idComboData);

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
