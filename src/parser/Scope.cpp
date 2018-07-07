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
    table.addGlobalPointer(global->addVariable("Link", ZVARTYPEID_LINK));
    table.addGlobalPointer(global->addVariable("Screen", ZVARTYPEID_SCREEN));
    table.addGlobalPointer(global->addVariable("Game", ZVARTYPEID_GAME));
    table.addGlobalPointer(global->addVariable("Debug", ZVARTYPEID_DEBUG));
    table.addGlobalPointer(global->addVariable("Audio", ZVARTYPEID_AUDIO));
    table.addGlobalPointer(global->addVariable("Text", ZVARTYPEID_TEXT));
    table.addGlobalPointer(global->addVariable("NPCData", ZVARTYPEID_NPCDATA));
    table.addGlobalPointer(global->addVariable("ComboData", ZVARTYPEID_COMBOS));
    table.addGlobalPointer(global->addVariable("SpriteData", ZVARTYPEID_SPRITEDATA));
    table.addGlobalPointer(global->addVariable("Graphics", ZVARTYPEID_GRAPHICS));
    table.addGlobalPointer(global->addVariable("Input", ZVARTYPEID_INPUT));
    table.addGlobalPointer(global->addVariable("MapData", ZVARTYPEID_MAPDATA));
    table.addGlobalPointer(global->addVariable("DMapData", ZVARTYPEID_DMAPDATA));
    table.addGlobalPointer(global->addVariable("MessageData", ZVARTYPEID_ZMESSAGE));
    table.addGlobalPointer(global->addVariable("ShopData", ZVARTYPEID_SHOPDATA));
    table.addGlobalPointer(global->addVariable("Untyped", ZVARTYPEID_UNTYPED));
    table.addGlobalPointer(global->addVariable("dropdata->", ZVARTYPEID_DROPSET));
    table.addGlobalPointer(global->addVariable("ponddata->", ZVARTYPEID_PONDS));
    table.addGlobalPointer(global->addVariable("warpring->", ZVARTYPEID_WARPRING));
    table.addGlobalPointer(global->addVariable("doorset->", ZVARTYPEID_DOORSET));
    table.addGlobalPointer(global->addVariable("misccolors->", ZVARTYPEID_ZUICOLOURS));
    table.addGlobalPointer(global->addVariable("rgbdata->", ZVARTYPEID_RGBDATA));
    table.addGlobalPointer(global->addVariable("palette->", ZVARTYPEID_PALETTE));
    table.addGlobalPointer(global->addVariable("musictrack->", ZVARTYPEID_TUNES));
    table.addGlobalPointer(global->addVariable("palcycle->", ZVARTYPEID_PALCYCLE));
    table.addGlobalPointer(global->addVariable("gamedata->", ZVARTYPEID_GAMEDATA));
    table.addGlobalPointer(global->addVariable("cheats->", ZVARTYPEID_CHEATS));

	return global;
}

Scope::Scope(SymbolTable& table) : table(table) {}

// Inheritance

Scope& Scope::getOrMakeLocalChild(string const& name)
{
	Scope* child = getLocalChild(name);
	if (child == NULL) child = makeChild(name);
	return *child;
}

Scope* Scope::getNamespace(string const& name) const
{
	Scope* child = getLocalChild(name);
	Scope* parent = getParent();
	if (!child && parent) child = parent->getNamespace(name);
	return child;
}

// Types

int Scope::getTypeId(string const& name) const
{
	int typeId = getLocalTypeId(name);
	Scope* parent = getParent();
	if (typeId == -1 && parent)
		typeId = parent->getTypeId(name);
	return typeId;
}

ZVarType* Scope::getLocalType(string const& name) const
{
	ZVarTypeId typeId = getLocalTypeId(name);
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

int Scope::getVariableId(string const& name) const
{
	int variableId = getLocalVariableId(name);
	Scope* parent = getParent();
	if (variableId == -1 && parent)
		variableId = parent->getVariableId(name);
	return variableId;
}

int Scope::addVariable(string const& name, ZVarType const& type, AST* node)
{
	return addVariable(name, table.getOrAssignTypeId(type), node);
}

int Scope::addVariable(string const& name, ZVarTypeId typeId)
{
	return addVariable(name, typeId, NULL);
}

int Scope::addVariable(string const& name, ZVarType const& type)
{
	return addVariable(name, table.getOrAssignTypeId(type), NULL);
}

// Functions

vector<int> Scope::getLocalFunctionIds(string const& name) const
{
	vector<int> ids;
	getLocalFunctionIds(ids, name);
	return ids;
}

void Scope::getFunctionIds(vector<int>& ids, string const& name) const
{
	getLocalFunctionIds(ids, name);
	Scope* parent = getParent();
	if (parent) parent->getFunctionIds(ids, name);
}

vector<int> Scope::getFunctionIds(string const& name) const
{
	vector<int> ids;
	getFunctionIds(ids, name);
	return ids;
}

int Scope::getFunctionId(FunctionSignature const& signature) const
{
	int functionId = getLocalFunctionId(signature);
	Scope* parent = getParent();
	if (functionId == -1 && parent)
		functionId = parent->getFunctionId(signature);
	return functionId;
}

int Scope::addFunction(string const& name, ZVarTypeId returnTypeId, vector<ZVarTypeId> const& paramTypeIds)
{
	return addFunction(name, returnTypeId, paramTypeIds, NULL);
}

int Scope::addFunction(string const& name, ZVarType const& returnType, vector<ZVarType*> const& paramTypes)
{
	return addFunction(name, returnType, paramTypes, NULL);
}

int Scope::addFunction(string const& name, ZVarType const& returnType, vector<ZVarType*> const& paramTypes, AST* node)
{
	vector<ZVarTypeId> paramTypeIds;
	for (vector<ZVarType*>::const_iterator it = paramTypes.begin(); it != paramTypes.end(); ++it)
		paramTypeIds.push_back(table.getOrAssignTypeId(**it));
	return addFunction(name, table.getOrAssignTypeId(returnType), paramTypeIds, node);
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

// Inheritance

Scope* BasicScope::getParent() const {return parent;}

Scope* BasicScope::makeChild(string const& name)
{
	map<string, Scope*>::const_iterator it = children.find(name);
	if (it != children.end()) return NULL;
	children[name] = new BasicScope(this);
	return children[name];
}

Scope* BasicScope::getLocalChild(string const& name) const
{
	map<string, Scope*>::const_iterator it = children.find(name);
	if (it == children.end()) return NULL;
	return it->second;
}

// Types

int BasicScope::getLocalTypeId(string const& name) const
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

int BasicScope::getLocalVariableId(string const& name) const
{
	map<string, int>::const_iterator it = variables.find(name);
	if (it != variables.end()) return it->second;
	return -1;
}

int BasicScope::addVariable(string const& name, ZVarTypeId typeId, AST* node)
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

void BasicScope::getLocalFunctionIds(vector<int>& ids, string const& name) const
{
	map<string, vector<int> >::const_iterator it = functionsByName.find(name);
	if (it != functionsByName.end())
		ids.insert(ids.end(), it->second.begin(), it->second.end());
}

int BasicScope::getLocalFunctionId(FunctionSignature const& signature) const
{
	map<FunctionSignature, int>::const_iterator it = functionsBySignature.find(signature);
	if (it != functionsBySignature.end()) return it->second;
	return -1;
}

int BasicScope::addFunction(string const& name, ZVarTypeId returnTypeId, vector<ZVarTypeId> const& paramTypeIds, AST* node)
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

