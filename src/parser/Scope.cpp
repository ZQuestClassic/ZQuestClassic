#include "GlobalSymbols.h"
#include "Scope.h"
#include "Types.h"

////////////////////////////////////////////////////////////////
// Scope

Scope* Scope::makeGlobalScope(SymbolTable& table)
{
	Scope* global = new BasicScope(table);

	// Add library functions to the top level scope.
    GlobalSymbols::getInst().addSymbolsToScope(*global);
    FFCSymbols::getInst().addSymbolsToScope(*global);
    ItemSymbols::getInst().addSymbolsToScope(*global);
    ItemclassSymbols::getInst().addSymbolsToScope(*global);
    LinkSymbols::getInst().addSymbolsToScope(*global);
    ScreenSymbols::getInst().addSymbolsToScope(*global);
    GameSymbols::getInst().addSymbolsToScope(*global);
    NPCSymbols::getInst().addSymbolsToScope(*global);
    LinkWeaponSymbols::getInst().addSymbolsToScope(*global);
    EnemyWeaponSymbols::getInst().addSymbolsToScope(*global);
    TextPtrSymbols::getInst().addSymbolsToScope(*global);
	GfxPtrSymbols::getInst().addSymbolsToScope(*global);
	SpriteDataSymbols::getInst().addSymbolsToScope(*global);
	CombosPtrSymbols::getInst().addSymbolsToScope(*global);
	AudioSymbols::getInst().addSymbolsToScope(*global);
	DebugSymbols::getInst().addSymbolsToScope(*global);
	NPCDataSymbols::getInst().addSymbolsToScope(*global);
	InputSymbols::getInst().addSymbolsToScope(*global);
	MapDataSymbols::getInst().addSymbolsToScope(*global);
	DMapDataSymbols::getInst().addSymbolsToScope(*global);
	MessageDataSymbols::getInst().addSymbolsToScope(*global);
	ShopDataSymbols::getInst().addSymbolsToScope(*global);
	UntypedSymbols::getInst().addSymbolsToScope(*global);
	DropsetSymbols::getInst().addSymbolsToScope(*global);
	PondSymbols::getInst().addSymbolsToScope(*global);
	WarpringSymbols::getInst().addSymbolsToScope(*global);
	DoorsetSymbols::getInst().addSymbolsToScope(*global);
	MiscColourSymbols::getInst().addSymbolsToScope(*global);
	RGBSymbols::getInst().addSymbolsToScope(*global);
	PaletteSymbols::getInst().addSymbolsToScope(*global);
	TunesSymbols::getInst().addSymbolsToScope(*global);
	PalCycleSymbols::getInst().addSymbolsToScope(*global);
	GamedataSymbols::getInst().addSymbolsToScope(*global);
	CheatsSymbols::getInst().addSymbolsToScope(*global);

	// Add global pointers.
    table.addGlobalPointer(global->addVar("Link", ZVARTYPEID_LINK));
    table.addGlobalPointer(global->addVar("Screen", ZVARTYPEID_SCREEN));
    table.addGlobalPointer(global->addVar("Game", ZVARTYPEID_GAME));
    table.addGlobalPointer(global->addVar("Debug", ZVARTYPEID_DEBUG));
    table.addGlobalPointer(global->addVar("Audio", ZVARTYPEID_AUDIO));
    table.addGlobalPointer(global->addVar("Text", ZVARTYPEID_TEXT));
    table.addGlobalPointer(global->addVar("NPCData", ZVARTYPEID_NPCDATA));
    table.addGlobalPointer(global->addVar("ComboData", ZVARTYPEID_COMBOS));
    table.addGlobalPointer(global->addVar("SpriteData", ZVARTYPEID_SPRITEDATA));
    table.addGlobalPointer(global->addVar("Graphics", ZVARTYPEID_GRAPHICS));
    table.addGlobalPointer(global->addVar("Input", ZVARTYPEID_INPUT));
    table.addGlobalPointer(global->addVar("MapData", ZVARTYPEID_MAPDATA));
    table.addGlobalPointer(global->addVar("DMapData", ZVARTYPEID_DMAPDATA));
    table.addGlobalPointer(global->addVar("MessageData", ZVARTYPEID_ZMESSAGE));
    table.addGlobalPointer(global->addVar("ShopData", ZVARTYPEID_SHOPDATA));
    table.addGlobalPointer(global->addVar("Untyped", ZVARTYPEID_UNTYPED));
    table.addGlobalPointer(global->addVar("dropdata->", ZVARTYPEID_DROPSET));
    table.addGlobalPointer(global->addVar("ponddata->", ZVARTYPEID_PONDS));
    table.addGlobalPointer(global->addVar("warpring->", ZVARTYPEID_WARPRING));
    table.addGlobalPointer(global->addVar("doorset->", ZVARTYPEID_DOORSET));
    table.addGlobalPointer(global->addVar("misccolors->", ZVARTYPEID_ZUICOLOURS));
    table.addGlobalPointer(global->addVar("rgbdata->", ZVARTYPEID_RGBDATA));
    table.addGlobalPointer(global->addVar("palette->", ZVARTYPEID_PALETTE));
    table.addGlobalPointer(global->addVar("musictrack->", ZVARTYPEID_TUNES));
    table.addGlobalPointer(global->addVar("palcycle->", ZVARTYPEID_PALCYCLE));
    table.addGlobalPointer(global->addVar("gamedata->", ZVARTYPEID_GAMEDATA));
    table.addGlobalPointer(global->addVar("cheats->", ZVARTYPEID_CHEATS));

	return global;
}

Scope::Scope(SymbolTable& table) : table(table) {}

Scope& Scope::getOrMakeChild(string const& name)
{
	Scope* child = getChild(name);
	if (child == NULL) child = makeChild(name);
	return *child;
}

// Types

int Scope::getTypeId(string const& nspace, string const& name) const
{
	if (nspace == "") return getTypeId(name);

	vector<string> names;
	names.push_back(nspace);
	names.push_back(name);
	return getTypeId(names);
}

int Scope::getTypeId(vector<string> const& names) const
{
	if (names.size() < 1) return -1;
	if (names.size() == 1) return getTypeId(names[0]);

	Scope* child = getChild(names[0]);
	if (child)
	{
		vector<string> childNames(names.begin() + 1, names.end());
		int typeId = child->getTypeIdNoParent(childNames);
		if (typeId != -1) return typeId;
	}

	Scope* parent = getParent();
	if (parent) return parent->getTypeId(names);
	return -1;
}

int Scope::getTypeIdNoParent(vector<string> const& names) const
{
	if (names.size() < 1) return -1;
	if (names.size() == 1) return getTypeIdNoParent(names[0]);

	Scope* child = getChild(names[0]);
	if (child)
	{
		vector<string> childNames(names.begin() + 1, names.end());
		int typeId = child->getTypeIdNoParent(childNames);
		if (typeId != -1) return typeId;
	}

	return -1;
}

ZVarType* Scope::getType(string const& nspace, string const& name) const
{
	ZVarTypeId typeId = getTypeId(nspace, name);
	if (typeId == -1) return NULL;
	return table.getType(typeId);
}

ZVarType* Scope::getType(vector<string> const& names) const
{
	ZVarTypeId typeId = getTypeId(names);
	if (typeId == -1) return NULL;
	return table.getType(typeId);
}

ZVarType* Scope::getType(string const& name) const
{
	ZVarTypeId typeId = getTypeId(name);
	if (typeId == -1) return NULL;
	return table.getType(typeId);
}

int Scope::addType(string const& name, ZVarType const& type, AST* node)
{
	return addType(name, table.getOrAssignTypeId(type), node);
}

int Scope::addType(string const& name, ZVarTypeId typeId)
{
	return addType(name, typeId, NULL);
}

int Scope::addType(string const& name, ZVarType const& type)
{
	return addType(name, table.getOrAssignTypeId(type), NULL);
}

// Variables

int Scope::getVarId(string const& nspace, string const& name) const
{
	if (nspace == "") return getVarId(name);

	vector<string> names;
	names.push_back(nspace);
	names.push_back(name);
	return getVarId(names);
}

int Scope::getVarId(vector<string> const& names) const
{
	if (names.size() < 1) return -1;
	if (names.size() == 1) return getVarId(names[0]);

	Scope* child = getChild(names[0]);
	if (child)
	{
		vector<string> childNames(names.begin() + 1, names.end());
		int varId = child->getVarIdNoParent(childNames);
		if (varId != -1) return varId;
	}

	Scope* parent = getParent();
	if (parent) return parent->getVarId(names);
	return -1;
}

int Scope::getVarIdNoParent(vector<string> const& names) const
{
	if (names.size() < 1) return -1;
	if (names.size() == 1) return getVarIdNoParent(names[0]);

	Scope* child = getChild(names[0]);
	if (child)
	{
		vector<string> childNames(names.begin() + 1, names.end());
		int varId = child->getVarIdNoParent(childNames);
		if (varId != -1) return varId;
	}

	return -1;
}

int Scope::addVar(string const& name, ZVarType const& type, AST* node)
{
	return addVar(name, table.getOrAssignTypeId(type), node);
}

int Scope::addVar(string const& name, ZVarTypeId typeId)
{
	return addVar(name, typeId, NULL);
}

int Scope::addVar(string const& name, ZVarType const& type)
{
	return addVar(name, table.getOrAssignTypeId(type), NULL);
}

// Functions

vector<int> Scope::getFuncIds(string const& nspace, string const& name) const
{
	if (nspace == "") return getFuncIds(name);

	vector<string> names;
	names.push_back(nspace);
	names.push_back(name);
	return getFuncIds(names);
}

vector<int> Scope::getFuncIds(vector<string> const& names) const
{
	vector<int> ids;
	getFuncIds(ids, names);
	return ids;
}

void Scope::getFuncIds(vector<int>& ids, vector<string> const& names) const
{
	if (names.size() < 1) return;
	if (names.size() == 1)
	{
		getFuncIds(ids, names[0]);
		return;
	}

	Scope* child = getChild(names[0]);
	if (child)
	{
		vector<string> childNames(names.begin() + 1, names.end());
		child->getFuncIdsNoParent(ids, childNames);
	}

	Scope* parent = getParent();
	if (parent) parent->getFuncIds(ids, names);
}

void Scope::getFuncIdsNoParent(vector<int>& ids, vector<string> const& names) const
{
	if (names.size() < 1) return;
	if (names.size() == 1)
	{
		getFuncIdsNoParent(ids, names[0]);
		return;
	}

	Scope* child = getChild(names[0]);
	if (child)
	{
		vector<string> childNames(names.begin() + 1, names.end());
		child->getFuncIdsNoParent(ids, childNames);
	}
}

vector<int> Scope::getFuncIds(string const& name) const
{
	vector<int> ids;
	getFuncIds(ids, name);
	return ids;
}

int Scope::getFuncId(string const& nspace, FunctionSignature const& signature) const
{
	if (nspace == "") return getFuncId(signature);

	vector<string> names;
	names.push_back(nspace);
	return getFuncId(names, signature);
}

int Scope::getFuncId(vector<string> const& names, FunctionSignature const& signature) const
{
	if (names.size() == 0) return getFuncId(signature);

	Scope* child = getChild(names[0]);
	if (child)
	{
		vector<string> childNames(names.begin() + 1, names.end());
		int funcId = child->getFuncId(childNames, signature);
		if (funcId != -1) return funcId;
	}

	Scope* parent = getParent();
	if (parent) return parent->getFuncId(names, signature);
	return -1;
}

int Scope::addFunc(string const& name, ZVarTypeId returnTypeId, vector<ZVarTypeId> const& paramTypeIds)
{
	return addFunc(name, returnTypeId, paramTypeIds, NULL);
}

int Scope::addFunc(string const& name, ZVarType const& returnType, vector<ZVarType*> const& paramTypes)
{
	return addFunc(name, returnType, paramTypes, NULL);
}

int Scope::addFunc(string const& name, ZVarType const& returnType, vector<ZVarType*> const& paramTypes, AST* node)
{
	vector<ZVarTypeId> paramTypeIds;
	for (vector<ZVarType*>::const_iterator it = paramTypes.begin(); it != paramTypes.end(); ++it)
		paramTypeIds.push_back(table.getOrAssignTypeId(**it));
	return addFunc(name, table.getOrAssignTypeId(returnType), paramTypeIds, node);
}

////////////////////////////////////////////////////////////////
// Basic Scope

BasicScope::BasicScope(Scope* parent) : Scope(parent->getTable()), parent(parent) {}

BasicScope::BasicScope(SymbolTable& table) : Scope(table), parent(NULL) {}

BasicScope::~BasicScope()
{
	for (map<string, Scope*>::iterator it = children.begin(); it != children.end(); ++it)
		delete it->second;
}

// Children

Scope* BasicScope::getParent() const {return parent;}

Scope* BasicScope::makeChild(string const& name)
{
	map<string, Scope*>::const_iterator it = children.find(name);
	if (it != children.end()) return NULL;
	children[name] = new BasicScope(this);
	return children[name];
}

Scope* BasicScope::getChild(string const& name) const
{
	map<string, Scope*>::const_iterator it = children.find(name);
	if (it == children.end()) return NULL;
	return it->second;
}

// Types

int BasicScope::getTypeId(string const& name) const
{
	map<string, int>::const_iterator it = types.find(name);
	if (it != types.end()) return it->second;
	if (parent) return parent->getTypeId(name);
	return -1;
}

int BasicScope::getTypeIdNoParent(string const& name) const
{
	map<string, int>::const_iterator it = types.find(name);
	if (it != types.end()) return it->second;
	return -1;
}

int BasicScope::addType(string const& name, ZVarTypeId typeId, AST* node)
{
	map<string, int>::const_iterator it = types.find(name);
	if (it != types.end()) return -1;
	types[name] = typeId;
	if (node) table.putNodeId(node, typeId);
	return typeId;
}

// Variables

int BasicScope::getVarId(string const& name) const
{
	map<string, int>::const_iterator it = variables.find(name);
	if (it != variables.end()) return it->second;
	if (parent) return parent->getVarId(name);
	return -1;
}

int BasicScope::getVarIdNoParent(string const& name) const
{
	map<string, int>::const_iterator it = variables.find(name);
	if (it != variables.end()) return it->second;
	return -1;
}

int BasicScope::addVar(string const& name, ZVarTypeId typeId, AST* node)
{
	map<string, int>::const_iterator it = variables.find(name);
	if (it != variables.end()) return -1;
	int varId = ScriptParser::getUniqueVarID();
	variables[name] = varId;
	table.putVarTypeId(varId, typeId);
	if (node) table.putNodeId(node, varId);
	return varId;
}

// Functions

void BasicScope::getFuncIds(vector<int>& ids, string const& name) const
{
	map<string, vector<int> >::const_iterator it = functionsByName.find(name);
	if (it != functionsByName.end())
		ids.insert(ids.end(), it->second.begin(), it->second.end());

	Scope* parent = getParent();
	if (parent) parent->getFuncIds(ids, name);
}

void BasicScope::getFuncIdsNoParent(vector<int>& ids, string const& name) const
{
	map<string, vector<int> >::const_iterator it = functionsByName.find(name);
	if (it != functionsByName.end())
		ids.insert(ids.end(), it->second.begin(), it->second.end());
}

int BasicScope::getFuncId(FunctionSignature const& signature) const
{
	map<FunctionSignature, int>::const_iterator it = functionsBySignature.find(signature);
	if (it != functionsBySignature.end()) return it->second;
	if (parent) return parent->getFuncId(signature);
	return -1;
}

int BasicScope::addFunc(string const& name, ZVarTypeId returnTypeId, vector<ZVarTypeId> const& paramTypeIds, AST* node)
{
	FunctionSignature signature(name, paramTypeIds);
	map<FunctionSignature, int>::const_iterator it = functionsBySignature.find(signature);
	if (it != functionsBySignature.end()) return -1;

	int funcId = ScriptParser::getUniqueFuncID();
	functionsByName[name].push_back(funcId);
	functionsBySignature[signature] = funcId;
	table.putFuncTypeIds(funcId, returnTypeId, paramTypeIds);
	if (node) table.putNodeId(node, funcId);
	return funcId;
}

