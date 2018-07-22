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

int Scope::getVariableId(string const& name) const
{
	int variableId = getLocalVariableId(name);
	Scope* parent = getParent();
	if (variableId == -1 && parent)
		variableId = parent->getVariableId(name);
	return variableId;
}

int Scope::getVariableId(vector<string> const& names) const
{
	vector<string> namespaces = names;
	namespaces.pop_back();
	Scope* scope = getChild(namespaces);
	if (scope == NULL) return -1;
	return scope->getVariableId(names.back());
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

Scope::VariableAccess Scope::getRead(string const& name) const
{
	VariableAccess r;
	r.variableId = getVariableId(name);
	if (r.variableId != -1)
		r.functionId = getGetterId(r.variableId);
	return r;
}

Scope::VariableAccess Scope::getWrite(string const& name) const
{
	VariableAccess r;
	r.variableId = getVariableId(name);
	if (r.variableId != -1)
		r.functionId = getSetterId(r.variableId);
	return r;
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

vector<int> Scope::getFunctionIds(vector<string> const& names)const
{
	vector<string> namespaces = names;
	namespaces.pop_back();
	Scope* scope = getChild(namespaces);
	if (scope == NULL) return vector<int>();
	return scope->getFunctionIds(names.back());
}

int Scope::getFunctionId(FunctionSignature const& signature) const
{
	int functionId = getLocalFunctionId(signature);
	Scope* parent = getParent();
	if (functionId == -1 && parent)
		functionId = parent->getFunctionId(signature);
	return functionId;
}

int Scope::addFunction(string const& name, ZVarType const& returnType, vector<ZVarType*> const& paramTypes, AST* node)
{
	vector<ZVarTypeId> paramTypeIds;
	for (vector<ZVarType*>::const_iterator it = paramTypes.begin(); it != paramTypes.end(); ++it)
		paramTypeIds.push_back(table.getOrAssignTypeId(**it));
	return addFunction(name, table.getOrAssignTypeId(returnType), paramTypeIds, node);
}

int Scope::addFunction(string const& name, ZVarTypeId returnTypeId, vector<ZVarTypeId> const& paramTypeIds)
{
	return addFunction(name, returnTypeId, paramTypeIds, NULL);
}

int Scope::addFunction(string const& name, ZVarType const& returnType, vector<ZVarType*> const& paramTypes)
{
	return addFunction(name, returnType, paramTypes, NULL);
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
	table.putFuncTypeIds(setterId, table.getTypeId(ZVarType::VOID), paramTypeIds);
	if (node) table.putNodeId(node, setterId);
	return setterId;
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
    table.addGlobalPointer(addVariable("Link", ZVARTYPEID_LINK));
    table.addGlobalPointer(addVariable("Screen", ZVARTYPEID_SCREEN));
    table.addGlobalPointer(addVariable("Game", ZVARTYPEID_GAME));
    table.addGlobalPointer(addVariable("Debug", ZVARTYPEID_DEBUG));
    table.addGlobalPointer(addVariable("Audio", ZVARTYPEID_AUDIO));
    table.addGlobalPointer(addVariable("Text", ZVARTYPEID_TEXT));
    table.addGlobalPointer(addVariable("NPCData", ZVARTYPEID_NPCDATA));
    table.addGlobalPointer(addVariable("ComboData", ZVARTYPEID_COMBOS));
    table.addGlobalPointer(addVariable("SpriteData", ZVARTYPEID_SPRITEDATA));
    table.addGlobalPointer(addVariable("Graphics", ZVARTYPEID_GRAPHICS));
    table.addGlobalPointer(addVariable("Input", ZVARTYPEID_INPUT));
    table.addGlobalPointer(addVariable("MapData", ZVARTYPEID_MAPDATA));
    table.addGlobalPointer(addVariable("DMapData", ZVARTYPEID_DMAPDATA));
    table.addGlobalPointer(addVariable("MessageData", ZVARTYPEID_ZMESSAGE));
    table.addGlobalPointer(addVariable("ShopData", ZVARTYPEID_SHOPDATA));
    table.addGlobalPointer(addVariable("Untyped", ZVARTYPEID_UNTYPED));
    table.addGlobalPointer(addVariable("dropdata->", ZVARTYPEID_DROPSET));
    table.addGlobalPointer(addVariable("ponddata->", ZVARTYPEID_PONDS));
    table.addGlobalPointer(addVariable("warpring->", ZVARTYPEID_WARPRING));
    table.addGlobalPointer(addVariable("doorset->", ZVARTYPEID_DOORSET));
    table.addGlobalPointer(addVariable("misccolors->", ZVARTYPEID_ZUICOLOURS));
    table.addGlobalPointer(addVariable("rgbdata->", ZVARTYPEID_RGBDATA));
    table.addGlobalPointer(addVariable("palette->", ZVARTYPEID_PALETTE));
    table.addGlobalPointer(addVariable("musictrack->", ZVARTYPEID_TUNES));
    table.addGlobalPointer(addVariable("palcycle->", ZVARTYPEID_PALCYCLE));
    table.addGlobalPointer(addVariable("gamedata->", ZVARTYPEID_GAMEDATA));
    table.addGlobalPointer(addVariable("cheats->", ZVARTYPEID_CHEATS));
}

////////////////////////////////////////////////////////////////
// ZClass

ZClass::ZClass(SymbolTable& table, string const& name, int id)
	: BasicScope(table), name(name), id(id)
{}


