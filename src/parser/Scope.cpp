#include "GlobalSymbols.h"
#include "Scope.h"

////////////////////////////////////////////////////////////////
// Scope

Scope::Scope(SymbolTable& table) : table(table), parent(NULL)
{
	// Add basic types to the top level scope.
	for (int id = ZVARTYPEID_VOID; id <= ZVARTYPEID_EWPN; ++id)
	{
		ZVarType const& type = *ZVarType::get(id);
		types[type.getName()] = id;
	}

	// Add const float type.
	ZVarTypeConstFloat constType;
	addType("const int", ZVarType::CONST_FLOAT);
	addType("const float", ZVarType::CONST_FLOAT);

	// Add library functions to the top level scope.
    GlobalSymbols::getInst().addSymbolsToScope(*this);
    FFCSymbols::getInst().addSymbolsToScope(*this);
    ItemSymbols::getInst().addSymbolsToScope(*this);
    ItemclassSymbols::getInst().addSymbolsToScope(*this);
    LinkSymbols::getInst().addSymbolsToScope(*this);
    ScreenSymbols::getInst().addSymbolsToScope(*this);
    GameSymbols::getInst().addSymbolsToScope(*this);
    NPCSymbols::getInst().addSymbolsToScope(*this);
    LinkWeaponSymbols::getInst().addSymbolsToScope(*this);
    EnemyWeaponSymbols::getInst().addSymbolsToScope(*this);
    TextPtrSymbols::getInst().addSymbolsToScope(*this);
	GfxPtrSymbols::getInst().addSymbolsToScope(*this);
	SpriteDataSymbols::getInst().addSymbolsToScope(*this);
	CombosPtrSymbols::getInst().addSymbolsToScope(*this);
	AudioSymbols::getInst().addSymbolsToScope(*this);
	DebugSymbols::getInst().addSymbolsToScope(*this);
	NPCDataSymbols::getInst().addSymbolsToScope(*this);
	InputSymbols::getInst().addSymbolsToScope(*this);
	MapDataSymbols::getInst().addSymbolsToScope(*this);
	DMapDataSymbols::getInst().addSymbolsToScope(*this);
	MessageDataSymbols::getInst().addSymbolsToScope(*this);
	ShopDataSymbols::getInst().addSymbolsToScope(*this);
	UntypedSymbols::getInst().addSymbolsToScope(*this);
	DropsetSymbols::getInst().addSymbolsToScope(*this);
	PondSymbols::getInst().addSymbolsToScope(*this);
	WarpringSymbols::getInst().addSymbolsToScope(*this);
	DoorsetSymbols::getInst().addSymbolsToScope(*this);
	MiscColourSymbols::getInst().addSymbolsToScope(*this);
	RGBSymbols::getInst().addSymbolsToScope(*this);
	PaletteSymbols::getInst().addSymbolsToScope(*this);
	TunesSymbols::getInst().addSymbolsToScope(*this);
	PalCycleSymbols::getInst().addSymbolsToScope(*this);
	GamedataSymbols::getInst().addSymbolsToScope(*this);
	CheatsSymbols::getInst().addSymbolsToScope(*this);

	// Add global pointers.
    table.addGlobalPointer(addVar("Link", ZVARTYPEID_LINK));
    table.addGlobalPointer(addVar("Screen", ZVARTYPEID_SCREEN));
    table.addGlobalPointer(addVar("Game", ZVARTYPEID_GAME));
    table.addGlobalPointer(addVar("Debug", ZVARTYPEID_DEBUG));
    table.addGlobalPointer(addVar("Audio", ZVARTYPEID_AUDIO));
    table.addGlobalPointer(addVar("Text", ZVARTYPEID_TEXT));
    table.addGlobalPointer(addVar("NPCData", ZVARTYPEID_NPCDATA));
    table.addGlobalPointer(addVar("ComboData", ZVARTYPEID_COMBOS));
    table.addGlobalPointer(addVar("SpriteData", ZVARTYPEID_SPRITEDATA));
    table.addGlobalPointer(addVar("Graphics", ZVARTYPEID_GRAPHICS));
    table.addGlobalPointer(addVar("Input", ZVARTYPEID_INPUT));
    table.addGlobalPointer(addVar("MapData", ZVARTYPEID_MAPDATA));
    table.addGlobalPointer(addVar("DMapData", ZVARTYPEID_DMAPDATA));
    table.addGlobalPointer(addVar("MessageData", ZVARTYPEID_ZMESSAGE));
    table.addGlobalPointer(addVar("ShopData", ZVARTYPEID_SHOPDATA));
    table.addGlobalPointer(addVar("Untyped", ZVARTYPEID_UNTYPED));
    table.addGlobalPointer(addVar("dropdata->", ZVARTYPEID_DROPSET));
    table.addGlobalPointer(addVar("ponddata->", ZVARTYPEID_PONDS));
    table.addGlobalPointer(addVar("warpring->", ZVARTYPEID_WARPRING));
    table.addGlobalPointer(addVar("doorset->", ZVARTYPEID_DOORSET));
    table.addGlobalPointer(addVar("misccolors->", ZVARTYPEID_ZUICOLOURS));
    table.addGlobalPointer(addVar("rgbdata->", ZVARTYPEID_RGBDATA));
    table.addGlobalPointer(addVar("palette->", ZVARTYPEID_PALETTE));
    table.addGlobalPointer(addVar("musictrack->", ZVARTYPEID_TUNES));
    table.addGlobalPointer(addVar("palcycle->", ZVARTYPEID_PALCYCLE));
    table.addGlobalPointer(addVar("gamedata->", ZVARTYPEID_GAMEDATA));
    table.addGlobalPointer(addVar("cheats->", ZVARTYPEID_CHEATS));
}

Scope::Scope(Scope* parent) : table(parent->table), parent(parent)
{}

Scope::~Scope()
{
	for (map<string, Scope*>::iterator it = children.begin(); it != children.end(); ++it)
		delete it->second;
}

// Children

Scope* Scope::makeChild(string const& name)
{
	map<string, Scope*>::const_iterator it = children.find(name);
	if (it != children.end()) return NULL;
	children[name] = new Scope(this);
	return children[name];
}

Scope* Scope::getChild(string const& name) const
{
	map<string, Scope*>::const_iterator it = children.find(name);
	if (it == children.end()) return NULL;
	return it->second;
}

Scope& Scope::getOrMakeChild(string const& name)
{
	map<string, Scope*>::const_iterator it = children.find(name);
	if (it != children.end()) return *it->second;
	children[name] = new Scope(this);
	return *children[name];
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

int Scope::getTypeId(string const& name) const
{
	map<string, int>::const_iterator it = types.find(name);
	if (it != types.end()) return it->second;
	if (parent) return parent->getTypeId(name);
	return -1;
}

int Scope::getTypeIdNoParent(string const& name) const
{
	map<string, int>::const_iterator it = types.find(name);
	if (it != types.end()) return it->second;
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

int Scope::addType(string const& name, ZVarTypeId typeId, AST* node)
{
	map<string, int>::const_iterator it = types.find(name);
	if (it != types.end()) return -1;
	types[name] = typeId;
	if (node) table.putNodeId(node, typeId);
	return typeId;
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

int Scope::getVarId(string const& name) const
{
	map<string, int>::const_iterator it = variables.find(name);
	if (it != variables.end()) return it->second;
	if (parent) return parent->getVarId(name);
	return -1;
}

int Scope::getVarIdNoParent(string const& name) const
{
	map<string, int>::const_iterator it = variables.find(name);
	if (it != variables.end()) return it->second;
	return -1;
}

int Scope::addVar(string const& name, ZVarTypeId typeId, AST* node)
{
	map<string, int>::const_iterator it = variables.find(name);
	if (it != variables.end()) return -1;
	int varId = ScriptParser::getUniqueVarID();
	variables[name] = varId;
	table.putVarTypeId(varId, typeId);
	if (node) table.putNodeId(node, varId);
	return varId;
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

void Scope::getFuncIds(vector<int>& ids, string const& name) const
{
	map<string, vector<int> >::const_iterator it = functionsByName.find(name);
	if (it != functionsByName.end())
		ids.insert(ids.end(), it->second.begin(), it->second.end());

	if (parent) parent->getFuncIds(ids, name);
}

void Scope::getFuncIdsNoParent(vector<int>& ids, string const& name) const
{
	map<string, vector<int> >::const_iterator it = functionsByName.find(name);
	if (it != functionsByName.end())
		ids.insert(ids.end(), it->second.begin(), it->second.end());
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

	if (parent) return parent->getFuncId(names, signature);
	return -1;
}

int Scope::getFuncId(FunctionSignature const& signature) const
{
	map<FunctionSignature, int>::const_iterator it = functionsBySignature.find(signature);
	if (it != functionsBySignature.end()) return it->second;
	if (parent) return parent->getFuncId(signature);
	return -1;
}

int Scope::addFunc(string const& name, ZVarTypeId returnTypeId, vector<ZVarTypeId> const& paramTypeIds, AST* node)
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

int Scope::addFunc(string const& name, ZVarType const& returnType, vector<ZVarType*> const& paramTypes, AST* node)
{
	vector<ZVarTypeId> paramTypeIds;
	for (vector<ZVarType*>::const_iterator it = paramTypes.begin(); it != paramTypes.end(); ++it)
		paramTypeIds.push_back(table.getOrAssignTypeId(**it));
	return addFunc(name, table.getOrAssignTypeId(returnType), paramTypeIds, node);
}

int Scope::addFunc(string const& name, ZVarTypeId returnTypeId, vector<ZVarTypeId> const& paramTypeIds)
{
	return addFunc(name, returnTypeId, paramTypeIds, NULL);
}

int Scope::addFunc(string const& name, ZVarType const& returnType, vector<ZVarType*> const& paramTypes)
{
	return addFunc(name, returnType, paramTypes, NULL);
}

