#include "GlobalSymbols.h"
#include "Scope.h"
#include "Types.h"

////////////////////////////////////////////////////////////////
// Scope

Scope::Scope(SymbolTable& table) : table(table), varDeclsDeprecated(false) {}

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

Scope* Scope::getLocalChild(vector<string> const& names) const
{
	if (names.size() == 0) return (Scope*)this;

	Scope* child = (Scope*)this;
	for (vector<string>::const_iterator it = names.begin();
	   it != names.end() && child != NULL;
	   ++it)
	{
		child = child->getLocalChild(*it);
	}

	return child;
}

Scope* Scope::getChild(vector<string>const& names) const
{
	Scope* child = getLocalChild(names);
	Scope* parent = getParent();
	if (!child && parent) child = parent->getChild(names);
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

// Classes

int Scope::getClassId(string const& name) const
{
	int classId = getLocalClassId(name);
	Scope* parent = getParent();
	if (classId == -1 && parent)
		classId = parent->getClassId(name);
	return classId;
}

ZClass* Scope::getLocalClass(string const& name) const
{
	int classId = getLocalClassId(name);
	if (classId == -1) return NULL;
	return table.getClass(classId);
}

ZClass* Scope::getClass(string const& name) const
{
	int classId = getClassId(name);
	if (classId == -1) return NULL;
	return table.getClass(classId);
}

int Scope::addClass(string const& name)
{
	return addClass(name, NULL);
}

// Variables

int Scope::getLocalVariableId(string const& name) const
{
	Variable* var = getLocalVariable(name);
	if (var == NULL) return -1;
	return var->id;
}

Scope::Variable* Scope::getVariable(string const& name) const
{
	Variable* var = getLocalVariable(name);
	if (var == NULL)
	{
	Scope* parent = getParent();
		if (parent != NULL)
			var = parent->getVariable(name);
	}
	return var;
}

int Scope::getVariableId(string const& name) const
{
	Variable* var = getVariable(name);
	if (var == NULL) return -1;
	return var->id;
}

Scope::Variable* Scope::getVariable(vector<string> const& names) const
{
	vector<string> namespaces = names;
	namespaces.pop_back();
	Scope* scope = getChild(namespaces);
	if (scope == NULL) return NULL;
	return scope->getVariable(names.back());
}

int Scope::getVariableId(vector<string> const& name) const
{
	Variable* var = getVariable(name);
	if (var == NULL) return -1;
	return var->id;
}

Scope::Variable* Scope::addVariable(ZVarTypeId typeId, string const& name, AST* node)
{
	return addVariable(*table.getType(typeId), name, node);
}

// Properties

int Scope::getGetterId(int varId) const
{
	int id = getLocalGetterId(varId);
	Scope* parent = getParent();
	if (id == -1 && parent)
		id = parent->getGetterId(varId);
	return id;
}

int Scope::getGetterId(string const& name) const
{
	int varId = getVariableId(name);
	if (varId == -1) return -1;
	return getGetterId(varId);
}

int Scope::getSetterId(int varId) const
{
	int id = getLocalSetterId(varId);
	Scope* parent = getParent();
	if (id == -1 && parent)
		id = parent->getSetterId(varId);
	return id;
}

int Scope::getSetterId(string const& name) const
{
	int varId = getVariableId(name);
	if (varId == -1) return -1;
	return getSetterId(varId);
}

int Scope::addGetter(int varId, vector<ZVarTypeId> const& paramTypeIds)
{
	return addGetter(varId, paramTypeIds, NULL);
}

int Scope::addSetter(int varId, vector<ZVarTypeId> const& paramTypeIds)
{
	return addSetter(varId, paramTypeIds, NULL);
}

// Functions

vector<Scope::Function*> Scope::getLocalFunctions(string const& name) const
{
	vector<Function*> functions;
	getLocalFunctions(name, functions);
	return functions;
}

void Scope::getLocalFunctionIds(string const& name, vector<int>& out) const
{
	vector<Function*> functions;
	getLocalFunctions(name, functions);
	for (vector<Function*>::const_iterator it = functions.begin(); it != functions.end(); ++it)
		out.push_back((*it)->id);
}

vector<int> Scope::getLocalFunctionIds(string const& name) const
{
	vector<int> ids;
	getLocalFunctionIds(name, ids);
	return ids;
}

void Scope::getFunctions(string const& name, vector<Function*>& out) const
{
	getLocalFunctions(name, out);
	Scope* parent = getParent();
	if (parent) parent->getFunctions(name, out);
}

void Scope::getFunctionIds(string const& name, vector<int>& out) const
{
	getLocalFunctionIds(name, out);
	Scope* parent = getParent();
	if (parent) parent->getFunctionIds(name, out);
}

vector<Scope::Function*> Scope::getFunctions(string const& name) const
{
	vector<Function*> functions;
	getFunctions(name, functions);
	return functions;
}

vector<int> Scope::getFunctionIds(string const& name) const
{
	vector<int> ids;
	getFunctionIds(name, ids);
	return ids;
}

vector<Scope::Function*> Scope::getFunctions(vector<string> const& names) const
{
	vector<string> namespaces = names;
	namespaces.pop_back();
	Scope* scope = getChild(namespaces);
	if (scope == NULL) return vector<Function*>();
	return scope->getFunctions(names.back());
}

vector<int> Scope::getFunctionIds(vector<string> const& names)const
{
	vector<string> namespaces = names;
	namespaces.pop_back();
	Scope* scope = getChild(namespaces);
	if (scope == NULL) return vector<int>();
	return scope->getFunctionIds(names.back());
}

Scope::Function* Scope::getFunction(FunctionSignature const& signature) const
{
	Function* function = getLocalFunction(signature);
	if (!function)
	{
	Scope* parent = getParent();
		if (parent) function = parent->getFunction(signature);
	}
	return function;
}

int Scope::getFunctionId(FunctionSignature const& signature) const
{
	Function* function = getLocalFunction(signature);
	if (!function)
	{
		Scope* parent = getParent();
		if (parent) function = parent->getFunction(signature);
	}
	if (function) return function->id;
	return -1;
}

Scope::Function* Scope::addFunction(ZVarTypeId returnTypeId, string const& name,
									vector<ZVarTypeId> const& paramTypeIds, AST* node)
{
	vector<ZVarType const*> paramTypes;
	for (vector<ZVarTypeId>::const_iterator it = paramTypeIds.begin();
	   it != paramTypeIds.end(); ++it)
	{
		paramTypes.push_back((ZVarType const*)table.getType(*it));
	}
	return addFunction((ZVarType const*)table.getType(returnTypeId),
					   name, paramTypes, node);
}

////////////////////////////////////////////////////////////////
// Basic Scope

BasicScope::BasicScope(Scope* parent) : Scope(parent->getTable()), parent(parent) {}

BasicScope::BasicScope(SymbolTable& table) : Scope(table), parent(NULL) {}

BasicScope::~BasicScope()
{
	for (map<string, Scope*>::iterator it = children.begin(); it != children.end(); ++it)
		delete it->second;
	for (map<string, Variable*>::iterator it = variables.begin(); it != variables.end(); ++it)
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

// Classes

int BasicScope::getLocalClassId(string const& name) const
{
	map<string, int>::const_iterator it = classes.find(name);
	if (it != classes.end()) return it->second;
	return -1;
}

int BasicScope::addClass(string const& name, AST* node)
{
	map<string, int>::const_iterator it = classes.find(name);
	if (it != classes.end()) return -1;

	int classId = table.createClass(name)->id;
	classes[name] = classId;
	if (node) table.putNodeId(node, classId);
	return classId;
}

// Variables

Scope::Variable* BasicScope::getLocalVariable(string const& name) const
{
	map<string, Variable*>::const_iterator it = variables.find(name);
	if (it == variables.end()) return NULL;
	return it->second;
}

Scope::Variable* BasicScope::addVariable(ZVarType const& type, string const& name, AST* node)
{
	// Return null if variable with name already exists locally.
	map<string, Variable*>::const_iterator it = variables.find(name);
	if (it != variables.end()) return NULL;

	Variable* var = new Variable(&type, name, ScriptParser::getUniqueVarID());
	variables[name] = var;
	table.putVarTypeId(var->id, table.getOrAssignTypeId(type));
	if (node) table.putNodeId(node, var->id);
	return var;
}

// Properties

int BasicScope::getLocalGetterId(int varId) const
{
	map<int, int>::const_iterator it = getters.find(varId);
	if (it != getters.end()) return it->second;
	return -1;
}

int BasicScope::getLocalSetterId(int varId) const
{
	map<int, int>::const_iterator it = setters.find(varId);
	if (it != setters.end()) return it->second;
	return -1;
}

int BasicScope::addGetter(int varId, vector<ZVarTypeId> const& paramTypeIds, AST* node)
{
	map<int, int>::const_iterator it = getters.find(varId);
	if (it != getters.end()) return -1;
	ZVarTypeId returnTypeId = table.getVarTypeId(varId);
	int getterId = ScriptParser::getUniqueFuncID();
	getters[varId] = getterId;
	table.putFuncTypeIds(getterId, returnTypeId, paramTypeIds);
	if (node) table.putNodeId(node, getterId);
	return getterId;
}

int BasicScope::addSetter(int varId, vector<ZVarTypeId> const& paramTypeIds, AST* node)
{
	map<int, int>::const_iterator it = setters.find(varId);
	if (it != setters.end()) return -1;
	int setterId = ScriptParser::getUniqueFuncID();
	setters[varId] = setterId;
	table.putFuncTypeIds(setterId, table.getTypeId(ZVarType::ZVOID), paramTypeIds);
	if (node) table.putNodeId(node, setterId);
	return setterId;
}

// Functions

void BasicScope::getLocalFunctions(string const& name, vector<Function*>& out) const
{
	map<string, vector<Function*> >::const_iterator it = functionsByName.find(name);
	if (it != functionsByName.end())
		out.insert(out.end(), it->second.begin(), it->second.end());
}

Scope::Function* BasicScope::getLocalFunction(FunctionSignature const& signature) const
{
	map<FunctionSignature, Function*>::const_iterator it = functionsBySignature.find(signature);
	if (it == functionsBySignature.end()) return NULL;
	return it->second;
}

Scope::Function* BasicScope::addFunction(ZVarType const* returnType, string const& name, vector<ZVarType const*> const& paramTypes, AST* node)
{
	// Return null if function with signature already exists locally.
	FunctionSignature signature(name, paramTypes);
	map<FunctionSignature, Function*>::const_iterator it = functionsBySignature.find(signature);
	if (it != functionsBySignature.end()) return NULL;

	Function* fun = new Function(returnType, name, paramTypes, ScriptParser::getUniqueFuncID());
	functionsByName[name].push_back(fun);
	functionsBySignature[signature] = fun;
	table.putFuncTypes(fun->id, returnType, paramTypes);
	if (node) table.putNodeId(node, fun->id);
	return fun;
}

////////////////////////////////////////////////////////////////
// GlobalScope

GlobalScope::GlobalScope(SymbolTable& table) : BasicScope(table)
{
	// Add global library functions.
    GlobalSymbols::getInst().addSymbolsToScope(*this);

	// Create builtin classes (skip void, float, and bool).
	for (ZVarTypeId typeId = ZVARTYPEID_CLASS_START; typeId < ZVARTYPEID_CLASS_END; ++typeId)
	{
		ZVarTypeClass const& type = *(ZVarTypeClass const*)ZVarType::get(typeId);
		ZClass& klass = *table.getClass(type.getClassId());
		LibrarySymbols& library = *LibrarySymbols::getTypeInstance(typeId);
		library.addSymbolsToScope(klass);
	}

	// Add global pointers.
    table.addGlobalPointer(addVariable(ZVARTYPEID_LINK, "Link")->id);
    table.addGlobalPointer(addVariable(ZVARTYPEID_SCREEN, "Screen")->id);
    table.addGlobalPointer(addVariable(ZVARTYPEID_GAME, "Game")->id);
    table.addGlobalPointer(addVariable(ZVARTYPEID_DEBUG, "Debug")->id);
    table.addGlobalPointer(addVariable(ZVARTYPEID_AUDIO, "Audio")->id);
    table.addGlobalPointer(addVariable(ZVARTYPEID_TEXT, "Text")->id);
    table.addGlobalPointer(addVariable(ZVARTYPEID_NPCDATA, "NPCData")->id);
    table.addGlobalPointer(addVariable(ZVARTYPEID_COMBOS, "ComboData")->id);
    table.addGlobalPointer(addVariable(ZVARTYPEID_SPRITEDATA, "SpriteData")->id);
    table.addGlobalPointer(addVariable(ZVARTYPEID_GRAPHICS, "Graphics")->id);
    table.addGlobalPointer(addVariable(ZVARTYPEID_INPUT, "Input")->id);
    table.addGlobalPointer(addVariable(ZVARTYPEID_MAPDATA, "MapData")->id);
    table.addGlobalPointer(addVariable(ZVARTYPEID_DMAPDATA, "DMapData")->id);
    table.addGlobalPointer(addVariable(ZVARTYPEID_ZMESSAGE, "MessageData")->id);
    table.addGlobalPointer(addVariable(ZVARTYPEID_SHOPDATA, "ShopData")->id);
    table.addGlobalPointer(addVariable(ZVARTYPEID_UNTYPED, "Untyped")->id);
    table.addGlobalPointer(addVariable(ZVARTYPEID_DROPSET, "DropData")->id);
    table.addGlobalPointer(addVariable(ZVARTYPEID_PONDS, "PondData")->id);
    table.addGlobalPointer(addVariable(ZVARTYPEID_WARPRING, "WarpRing")->id);
    table.addGlobalPointer(addVariable(ZVARTYPEID_DOORSET, "DoorSet")->id);
    table.addGlobalPointer(addVariable(ZVARTYPEID_ZUICOLOURS, "MiscColors")->id);
    table.addGlobalPointer(addVariable(ZVARTYPEID_RGBDATA, "RGBData")->id);
    table.addGlobalPointer(addVariable(ZVARTYPEID_PALETTE, "Palette")->id);
    table.addGlobalPointer(addVariable(ZVARTYPEID_TUNES, "MusicTrack")->id);
    table.addGlobalPointer(addVariable(ZVARTYPEID_PALCYCLE, "PalCycle")->id);
    table.addGlobalPointer(addVariable(ZVARTYPEID_GAMEDATA, "GameData")->id);
    table.addGlobalPointer(addVariable(ZVARTYPEID_CHEATS, "Cheats")->id);
}

////////////////////////////////////////////////////////////////
// ZClass

ZClass::ZClass(SymbolTable& table, string const& name, int id)
	: BasicScope(table), name(name), id(id)
{}


