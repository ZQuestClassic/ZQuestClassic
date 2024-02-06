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
using std::vector;

////////////////////////////////////////////////////////////////
// TypeStore

TypeStore::TypeStore()
{
	// Assign builtin types.
	for (DataTypeId id = ZTID_START; id < ZTID_END; ++id)
		assignTypeId(*DataType::get(id));

	// Assign builtin classes.
	for (int32_t id = ZTID_CLASS_START; id < ZTID_CLASS_END; ++id)
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

// Classes

ZClass* TypeStore::getClass(int32_t classId) const
{
	if (classId < 0 || classId > int32_t(ownedClasses.size())) return NULL;
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
//Classes: Global Pointer
DataTypeClassConst DataType::GAME(ZCLID_GAME, "Game");
DataTypeClassConst DataType::PLAYER(ZCLID_PLAYER, "Player");
DataTypeClassConst DataType::SCREEN(ZCLID_SCREEN, "Screen");
DataTypeClassConst DataType::REGION(ZCLID_REGION, "Region");
DataTypeClassConst DataType::VIEWPORT(ZCLID_VIEWPORT, "Viewport");
DataTypeClassConst DataType::AUDIO(ZCLID_AUDIO, "Audio");
DataTypeClassConst DataType::DEBUG(ZCLID_DEBUG, "Debug");
DataTypeClassConst DataType::GRAPHICS(ZCLID_GRAPHICS, "Graphics");
DataTypeClassConst DataType::INPUT(ZCLID_INPUT, "Input");
DataTypeClassConst DataType::TEXT(ZCLID_TEXT, "Text");
DataTypeClassConst DataType::FILESYSTEM(ZCLID_FILESYSTEM, "FileSystem");
DataTypeClassConst DataType::ZINFO(ZCLID_ZINFO, "ZInfo");
//Class: Types
DataTypeClassConst DataType::CBITMAP(ZCLID_BITMAP, "const Bitmap");
DataTypeClassConst DataType::CCHEATS(ZCLID_CHEATS, "const Cheats");
DataTypeClassConst DataType::CCOMBOS(ZCLID_COMBOS, "const Combos");
DataTypeClassConst DataType::CDOORSET(ZCLID_DOORSET, "const DoorSet");
DataTypeClassConst DataType::CDROPSET(ZCLID_DROPSET, "const DropSet");
DataTypeClassConst DataType::CDMAPDATA(ZCLID_DMAPDATA, "const DMapData");
DataTypeClassConst DataType::CEWPN(ZCLID_EWPN, "const EWeapon");
DataTypeClassConst DataType::CFFC(ZCLID_FFC, "const FFC");
DataTypeClassConst DataType::CGAMEDATA(ZCLID_GAMEDATA, "const GameData");
DataTypeClassConst DataType::CITEM(ZCLID_ITEM, "const Item");
DataTypeClassConst DataType::CITEMCLASS(ZCLID_ITEMCLASS, "const ItemData");
DataTypeClassConst DataType::CLWPN(ZCLID_LWPN, "const LWeapon");
DataTypeClassConst DataType::CMAPDATA(ZCLID_MAPDATA, "const MapData");
DataTypeClassConst DataType::CZMESSAGE(ZCLID_ZMESSAGE, "const ZMessage");
DataTypeClassConst DataType::CZUICOLOURS(ZCLID_ZUICOLOURS, "const ZuiColours");
DataTypeClassConst DataType::CNPC(ZCLID_NPC, "const NPC");
DataTypeClassConst DataType::CNPCDATA(ZCLID_NPCDATA, "const NPCData");
DataTypeClassConst DataType::CPALCYCLE(ZCLID_PALCYCLE, "const PalCycle");
DataTypeClassConst DataType::CPALETTEOLD(ZTID_PALETTEOLD, "const paletteold");
DataTypeClassConst DataType::CPONDS(ZCLID_PONDS, "const Ponds");
DataTypeClassConst DataType::CRGBDATAOLD(ZTID_RGBDATAOLD, "const rgbdataold");
DataTypeClassConst DataType::CSHOPDATA(ZCLID_SHOPDATA, "const ShopData");
DataTypeClassConst DataType::CSPRITEDATA(ZCLID_SPRITEDATA, "const SpriteData");
DataTypeClassConst DataType::CTUNES(ZCLID_TUNES, "const Tunes");
DataTypeClassConst DataType::CWARPRING(ZCLID_WARPRING, "const WarpRing");
DataTypeClassConst DataType::CSUBSCREENDATA(ZCLID_SUBSCREENDATA, "const SubscreenData");
DataTypeClassConst DataType::CFILE(ZCLID_FILE, "const File");
DataTypeClassConst DataType::CDIRECTORY(ZCLID_DIRECTORY, "const Directory");
DataTypeClassConst DataType::CSTACK(ZCLID_STACK, "const Stack");
DataTypeClassConst DataType::CRNG(ZCLID_RNG, "const RNG");
DataTypeClassConst DataType::CPALDATA(ZCLID_PALDATA, "const paldata");
DataTypeClassConst DataType::CBOTTLETYPE(ZCLID_BOTTLETYPE, "const bottledata");
DataTypeClassConst DataType::CBOTTLESHOP(ZCLID_BOTTLESHOP, "const bottleshopdata");
DataTypeClassConst DataType::CGENERICDATA(ZCLID_GENERICDATA, "const genericdata");
DataTypeClassConst DataType::CPORTAL(ZCLID_PORTAL, "const portal");
DataTypeClassConst DataType::CSAVEDPORTAL(ZCLID_SAVPORTAL, "const savedportal");
DataTypeClassConst DataType::CSUBSCREENPAGE(ZCLID_SUBSCREENPAGE, "const SubscreenPage");
DataTypeClassConst DataType::CSUBSCREENWIDGET(ZCLID_SUBSCREENWIDGET, "const SubscreenWidget");
DataTypeClassConst DataType::CWEBSOCKET(ZCLID_WEBSOCKET, "const WebSocket");
//Class: Var Types
DataTypeClass DataType::BITMAP(ZCLID_BITMAP, "Bitmap", &CBITMAP);
DataTypeClass DataType::CHEATS(ZCLID_CHEATS, "Cheats", &CCHEATS);
DataTypeClass DataType::COMBOS(ZCLID_COMBOS, "Combos", &CCOMBOS);
DataTypeClass DataType::DOORSET(ZCLID_DOORSET, "DoorSet", &CDOORSET);
DataTypeClass DataType::DROPSET(ZCLID_DROPSET, "DropSet", &CDROPSET);
DataTypeClass DataType::DMAPDATA(ZCLID_DMAPDATA, "DMapData", &CDMAPDATA);
DataTypeClass DataType::EWPN(ZCLID_EWPN, "EWeapon", &CEWPN);
DataTypeClass DataType::FFC(ZCLID_FFC, "FFC", &CFFC);
DataTypeClass DataType::GAMEDATA(ZCLID_GAMEDATA, "GameData", &CGAMEDATA);
DataTypeClass DataType::ITEM(ZCLID_ITEM, "Item", &CITEM);
DataTypeClass DataType::ITEMCLASS(ZCLID_ITEMCLASS, "ItemData", &CITEMCLASS);
DataTypeClass DataType::LWPN(ZCLID_LWPN, "LWeapon", &CLWPN);
DataTypeClass DataType::MAPDATA(ZCLID_MAPDATA, "MapData", &CMAPDATA);
DataTypeClass DataType::ZMESSAGE(ZCLID_ZMESSAGE, "ZMessage", &CZMESSAGE);
DataTypeClass DataType::ZUICOLOURS(ZCLID_ZUICOLOURS, "ZuiColours", &CZUICOLOURS);
DataTypeClass DataType::NPC(ZCLID_NPC, "NPC", &CNPC);
DataTypeClass DataType::NPCDATA(ZCLID_NPCDATA, "NPCData", &CNPCDATA);
DataTypeClass DataType::PALCYCLE(ZCLID_PALCYCLE, "PalCycle", &CPALCYCLE);
DataTypeClass DataType::PALETTEOLD(ZCLID_PALETTE, "paletteold", &CPALETTEOLD);
DataTypeClass DataType::PONDS(ZCLID_PONDS, "Ponds", &CPONDS);
DataTypeClass DataType::RGBDATAOLD(ZCLID_RGBDATA, "rgbdataold", &CRGBDATAOLD);
DataTypeClass DataType::SHOPDATA(ZCLID_SHOPDATA, "ShopData", &CSHOPDATA);
DataTypeClass DataType::SPRITEDATA(ZCLID_SPRITEDATA, "SpriteData", &CSPRITEDATA);
DataTypeClass DataType::TUNES(ZCLID_TUNES, "Tunes", &CTUNES);
DataTypeClass DataType::WARPRING(ZCLID_WARPRING, "WarpRing", &CWARPRING);
DataTypeClass DataType::SUBSCREENDATA(ZCLID_SUBSCREENDATA, "SubscreenData", &CSUBSCREENDATA);
DataTypeClass DataType::FILE(ZCLID_FILE, "File", &CFILE);
DataTypeClass DataType::DIRECTORY(ZCLID_DIRECTORY, "Directory", &CDIRECTORY);
DataTypeClass DataType::STACK(ZCLID_STACK, "Stack", &CSTACK);
DataTypeClass DataType::RNG(ZCLID_RNG, "RNG", &CRNG);
DataTypeClass DataType::PALDATA(ZCLID_PALDATA, "PALDATA", &CPALDATA);
DataTypeClass DataType::BOTTLETYPE(ZCLID_BOTTLETYPE, "bottledata", &CBOTTLETYPE);
DataTypeClass DataType::BOTTLESHOP(ZCLID_BOTTLESHOP, "bottleshopdata", &CBOTTLESHOP);
DataTypeClass DataType::GENERICDATA(ZCLID_GENERICDATA, "genericdata", &CGENERICDATA);
DataTypeClass DataType::PORTAL(ZCLID_PORTAL, "portal", &CPORTAL);
DataTypeClass DataType::SAVEDPORTAL(ZCLID_SAVPORTAL, "savedportal", &CSAVEDPORTAL);
DataTypeClass DataType::SUBSCREENPAGE(ZCLID_SUBSCREENPAGE, "SubscreenPage", &CSUBSCREENPAGE);
DataTypeClass DataType::SUBSCREENWIDGET(ZCLID_SUBSCREENWIDGET, "SubscreenWidget", &CSUBSCREENWIDGET);
DataTypeClass DataType::WEBSOCKET(ZCLID_WEBSOCKET, "WebSocket", &CWEBSOCKET);

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
		case ZTID_UNTYPED: return &UNTYPED;
		case ZTID_AUTO: return &ZAUTO;
		case ZTID_VOID: return &ZVOID;
		case ZTID_FLOAT: return &FLOAT;
		case ZTID_CHAR: return &CHAR;
		case ZTID_LONG: return &LONG;
		case ZTID_BOOL: return &BOOL;
		case ZTID_RGBDATA: return &RGBDATA;
		case ZTID_GAME: return &GAME;
		case ZTID_PLAYER: return &PLAYER;
		case ZTID_SCREEN: return &SCREEN;
		case ZTID_REGION: return &REGION;
		case ZTID_VIEWPORT: return &VIEWPORT;
		case ZTID_FFC: return &FFC;
		case ZTID_ITEM: return &ITEM;
		case ZTID_ITEMCLASS: return &ITEMCLASS;
		case ZTID_NPC: return &NPC;
		case ZTID_LWPN: return &LWPN;
		case ZTID_EWPN: return &EWPN;
		case ZTID_NPCDATA: return &NPCDATA;
		case ZTID_DEBUG: return &DEBUG;
		case ZTID_AUDIO: return &AUDIO;
		case ZTID_COMBOS: return &COMBOS;
		case ZTID_SPRITEDATA: return &SPRITEDATA;
		case ZTID_SUBSCREENDATA: return &SUBSCREENDATA;
		case ZTID_SUBSCREENPAGE: return &SUBSCREENPAGE;
		case ZTID_SUBSCREENWIDGET: return &SUBSCREENWIDGET;
		case ZTID_FILE: return &FILE;
		case ZTID_DIRECTORY: return &DIRECTORY;
		case ZTID_STACK: return &STACK;
		case ZTID_RNG: return &RNG;
		case ZTID_PALDATA: return &PALDATA;
		case ZTID_BOTTLETYPE: return &BOTTLETYPE;
		case ZTID_BOTTLESHOP: return &BOTTLESHOP;
		case ZTID_GENERICDATA: return &GENERICDATA;
		case ZTID_PORTAL: return &PORTAL;
		case ZTID_SAVPORTAL: return &SAVEDPORTAL;
		case ZTID_GRAPHICS: return &GRAPHICS;
		case ZTID_BITMAP: return &BITMAP;
		case ZTID_TEXT: return &TEXT;
		case ZTID_INPUT: return &INPUT;
		case ZTID_MAPDATA: return &MAPDATA;
		case ZTID_DMAPDATA: return &DMAPDATA;
		case ZTID_ZMESSAGE: return &ZMESSAGE;
		case ZTID_SHOPDATA: return &SHOPDATA;
		case ZTID_DROPSET: return &DROPSET;
		case ZTID_PONDS: return &PONDS;
		case ZTID_WARPRING: return &WARPRING;
		case ZTID_DOORSET: return &DOORSET;
		case ZTID_ZUICOLOURS: return &ZUICOLOURS;
		case ZTID_RGBDATAOLD: return &RGBDATAOLD;
		case ZTID_PALETTEOLD: return &PALETTEOLD;
		case ZTID_TUNES: return &TUNES;
		case ZTID_PALCYCLE: return &PALCYCLE;
		case ZTID_GAMEDATA: return &GAMEDATA;
		case ZTID_CHEATS: return &CHEATS;
		case ZTID_FILESYSTEM: return &FILESYSTEM;
		case ZTID_ZINFO: return &ZINFO;
		case ZTID_WEBSOCKET: return &WEBSOCKET;
		default: return NULL;
	}
}

DataTypeClass const* DataType::getClass(int32_t classId)
{
	switch (classId)
	{
		case ZCLID_GAME: return &GAME;
		case ZCLID_PLAYER: return &PLAYER;
		case ZCLID_SCREEN: return &SCREEN;
		case ZCLID_REGION: return &REGION;
		case ZCLID_VIEWPORT: return &VIEWPORT;
		case ZCLID_FFC: return &FFC;
		case ZCLID_ITEM: return &ITEM;
		case ZCLID_ITEMCLASS: return &ITEMCLASS;
		case ZCLID_NPC: return &NPC;
		case ZCLID_LWPN: return &LWPN;
		case ZCLID_EWPN: return &EWPN;
		case ZCLID_NPCDATA: return &NPCDATA;
		case ZCLID_DEBUG: return &DEBUG;
		case ZCLID_AUDIO: return &AUDIO;
		case ZCLID_COMBOS: return &COMBOS;
		case ZCLID_SPRITEDATA: return &SPRITEDATA;
		case ZCLID_SUBSCREENDATA: return &SUBSCREENDATA;
		case ZCLID_SUBSCREENPAGE: return &SUBSCREENPAGE;
		case ZCLID_SUBSCREENWIDGET: return &SUBSCREENWIDGET;
		case ZCLID_FILE: return &FILE;
		case ZCLID_DIRECTORY: return &DIRECTORY;
		case ZCLID_STACK: return &STACK;
		case ZCLID_RNG: return &RNG;
		case ZCLID_PALDATA: return &PALDATA;
		case ZCLID_BOTTLETYPE: return &BOTTLETYPE;
		case ZCLID_BOTTLESHOP: return &BOTTLESHOP;
		case ZCLID_GENERICDATA: return &GENERICDATA;
		case ZCLID_PORTAL: return &PORTAL;
		case ZCLID_SAVPORTAL: return &SAVEDPORTAL;
		case ZCLID_GRAPHICS: return &GRAPHICS;
		case ZCLID_BITMAP: return &BITMAP;
		case ZCLID_TEXT: return &TEXT;
		case ZCLID_INPUT: return &INPUT;
		case ZCLID_MAPDATA: return &MAPDATA;
		case ZCLID_DMAPDATA: return &DMAPDATA;
		case ZCLID_ZMESSAGE: return &ZMESSAGE;
		case ZCLID_SHOPDATA: return &SHOPDATA;
		case ZCLID_DROPSET: return &DROPSET;
		case ZCLID_PONDS: return &PONDS;
		case ZCLID_WARPRING: return &WARPRING;
		case ZCLID_DOORSET: return &DOORSET;
		case ZCLID_ZUICOLOURS: return &ZUICOLOURS;
		// case ZCLID_RGBDATA: return &RGBDATA;
		// case ZCLID_PALETTE: return &PALETTE;
		case ZCLID_TUNES: return &TUNES;
		case ZCLID_PALCYCLE: return &PALCYCLE;
		case ZCLID_GAMEDATA: return &GAMEDATA;
		case ZCLID_CHEATS: return &CHEATS;
		case ZCLID_FILESYSTEM: return &FILESYSTEM;
		case ZCLID_ZINFO: return &ZINFO;
		case ZCLID_WEBSOCKET: return &WEBSOCKET;
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
// DataTypeClass

DataTypeClass::DataTypeClass(int32_t classId, DataType* constType)
	: DataType(constType), classId(classId), className("")
{}

DataTypeClass::DataTypeClass(int32_t classId, string const& className, DataType* constType)
	: DataType(constType), classId(classId), className(className)
{}

DataType* DataTypeClass::resolve(Scope& scope, CompileErrorHandler* errorHandler)
{
	// Grab the proper name for the class the first time it's resolved.
	if (className == "")
		className = scope.getTypeStore().getClass(classId)->name;

	return this;
}
DataType const* DataTypeClass::baseType(Scope& scope, CompileErrorHandler* errorHandler) const
{
	return DataType::getClass(classId);
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

int32_t DataTypeClass::selfCompare(DataType const& rhs) const
{
	DataTypeClass const& o = static_cast<DataTypeClass const&>(rhs);
	return classId - o.classId;
}

////////////////////////////////////////////////////////////////
// DataTypeClassConst

DataTypeClassConst::DataTypeClassConst(int32_t classId, string const& name)
	: DataTypeClass(classId, name, NULL)
{}
DataType const* DataTypeClassConst::baseType(Scope& scope, CompileErrorHandler* errorHandler) const
{
	auto* ty = DataType::getClass(classId);
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

bool DataTypeCustom::canCastTo(DataType const& target) const
{
	if (target.isVoid()) return false;
	if (target.isUntyped()) return true;

	if (DataTypeArray const* t =
			dynamic_cast<DataTypeArray const*>(&target))
		return canCastTo(getBaseType(*t));
	
	if(!isClass())
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
	//the 'this' 'this->' stuff. -Z
	ScriptTypeData scriptTypes[ParserScriptType::idEnd] = {
		{"invalid", ZTID_VOID},
		{"global", ZTID_VOID},
		{"ffc", ZTID_FFC},
		{"item", ZTID_ITEMCLASS},
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
