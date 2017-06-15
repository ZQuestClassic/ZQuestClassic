
#include "GlobalSymbols.h"
#include "Scope.h"
#include "Types.h"
#include "ZScript.h"

using namespace ZScript;

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

vector<Scope*> Scope::getLocalChildren() const
{
	vector<Scope*> children;
	getLocalChildren(children);
	return children;
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

Variable* Scope::getVariable(string const& name) const
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

Variable* Scope::getVariable(vector<string> const& names) const
{
	vector<string> namespaces = names;
	namespaces.pop_back();
	Scope* scope = getChild(namespaces);
	if (scope == NULL) return NULL;
	return scope->getVariable(names.back());
}

// Properties

Function* Scope::getGetter(string const& name) const
{
	Function* fun = getLocalGetter(name);
	if (!fun)
	{
		Scope* parent = getParent();
		if (parent) fun = parent->getGetter(name);
	}
	return fun;
}

Function* Scope::getSetter(string const& name) const
{
	Function* fun = getLocalSetter(name);
	if (!fun)
	{
		Scope* parent = getParent();
		if (parent) fun = parent->getSetter(name);
	}
	return fun;
}

// Functions

vector<Function*> Scope::getLocalFunctions() const
{
	vector<Function*> functions;
	getLocalFunctions(functions);
	return functions;
}

vector<Function*> Scope::getLocalFunctions(string const& name) const
{
	vector<Function*> functions;
	getLocalFunctions(name, functions);
	return functions;
}

vector<Function*> Scope::getAllFunctions() const
{
	vector<Function*> functions;
	getAllFunctions(functions);
	return functions;
}

void Scope::getAllFunctions(vector<ZScript::Function*>& outFunctions) const
{
	// Get local functions.
	getLocalFunctions(outFunctions);
	// Recurse on children.
	vector<Scope*> children = getLocalChildren();
	for (vector<Scope*>::iterator it = children.begin();
		 it != children.end(); ++it)
		(*it)->getAllFunctions(outFunctions);
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

vector<Function*> Scope::getFunctions(string const& name) const
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

vector<Function*> Scope::getFunctions(vector<string> const& names) const
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

Function* Scope::getFunction(FunctionSignature const& signature) const
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

Function* Scope::addFunction(ZVarTypeId returnTypeId, string const& name,
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
	for (vector<Scope*>::iterator it = anonymousChildren.begin(); it != anonymousChildren.end(); ++it)
		delete *it;
	for (vector<Literal*>::iterator it = literals.begin(); it != literals.end(); ++it)
		delete *it;
	for (map<string, Variable*>::iterator it = variables.begin(); it != variables.end(); ++it)
		delete it->second;
	for (map<FunctionSignature, Function*>::iterator it = functionsBySignature.begin();
		 it != functionsBySignature.end(); ++it)
		delete it->second;
	for (map<string, Function*>::iterator it = getters.begin(); it != getters.end(); ++it)
		delete it->second;
	for (map<string, Function*>::iterator it = setters.begin(); it != setters.end(); ++it)
		delete it->second;
}

// Inheritance

Scope* BasicScope::getParent() const {return parent;}

Scope* BasicScope::makeChild()
{
	Scope* child = new BasicScope(this);
	anonymousChildren.push_back(child);
	return child;
}

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

vector<Scope*> BasicScope::getAnonymousChildren() const
{
	return anonymousChildren;
}

void BasicScope::getLocalChildren(vector<Scope*>& outChildren) const
{
	// Grab anonymous children.
	outChildren.insert(
			outChildren.end(),
			anonymousChildren.begin(),
			anonymousChildren.end());
	// Grab named children.
	for (map<string, Scope*>::const_iterator it = children.begin();
		 it != children.end(); ++it)
		outChildren.push_back(it->second);
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

// Literals

vector<Literal*> BasicScope::getLocalLiterals() const
{
	return literals;
}

Literal* BasicScope::addLiteral(ASTLiteral& node, ZVarType const* type)
{
	int id = ScriptParser::getUniqueVarID();
	getTable().putNodeId(&node, id);
	Literal* literal = new Literal(&node, type, id);
	literals.push_back(literal);
	return literal;
}

// Variables

vector<Variable*> BasicScope::getLocalVariables() const
{
	vector<Variable*> ret;
	for (map<string, Variable*>::const_iterator it = variables.begin();
		 it != variables.end(); ++it)
		ret.push_back(it->second);
	return ret;
}

Variable* BasicScope::getLocalVariable(string const& name) const
{
	map<string, Variable*>::const_iterator it = variables.find(name);
	if (it == variables.end()) return NULL;
	return it->second;
}

Variable* BasicScope::addVariable(ZVarType const& type, string const& name, AST* node)
{
	// Return null if variable with name already exists locally.
	map<string, Variable*>::const_iterator it = variables.find(name);
	if (it != variables.end()) return NULL;

	Variable* var = new Variable((ASTDataDecl*)node, &type, name, ScriptParser::getUniqueVarID());
	variables[name] = var;
	table.putVarTypeId(var->id, table.getOrAssignTypeId(type));
	if (node) table.putNodeId(node, var->id);
	return var;
}

// Properties

Function* BasicScope::getLocalGetter(string const& name) const
{
	map<string, Function*>::const_iterator it = getters.find(name);
	if (it == getters.end()) return NULL;
	return it->second;
}

Function*  BasicScope::getLocalSetter(string const& name) const
{
	map<string, Function*>::const_iterator it = setters.find(name);
	if (it == setters.end()) return NULL;
	return it->second;
}

Function* BasicScope::addGetter(
		ZVarType const* returnType, string const& name,
		vector<ZVarType const*> const& paramTypes, AST* node)
{
	// Return null if getter with name already exists locally.
	map<string, Function*>::const_iterator it = getters.find(name);
	if (it != getters.end()) return NULL;

	Function* fun = new Function(returnType, name, paramTypes, ScriptParser::getUniqueFuncID());
	getters[name] = fun;
	table.putFuncTypes(fun->id, returnType, paramTypes);
	if (node) table.putNodeId(node, fun->id);
	return fun;
}

Function* BasicScope::addSetter(
		ZVarType const* returnType, string const& name,
		vector<ZVarType const*> const& paramTypes, AST* node)
{
	// Return null if setter with name already exists locally.
	map<string, Function*>::const_iterator it = setters.find(name);
	if (it != setters.end()) return NULL;

	Function* fun = new Function(returnType, name, paramTypes, ScriptParser::getUniqueFuncID());
	setters[name] = fun;
	table.putFuncTypes(fun->id, returnType, paramTypes);
	if (node) table.putNodeId(node, fun->id);
	return fun;
}

// Functions

void BasicScope::getLocalFunctions(vector<Function*>& functions) const
{
	for (map<FunctionSignature, Function*>::const_iterator it = functionsBySignature.begin();
		 it != functionsBySignature.end(); ++it)
		functions.push_back(it->second);
}

void BasicScope::getLocalFunctions(string const& name, vector<Function*>& out) const
{
	map<string, vector<Function*> >::const_iterator it = functionsByName.find(name);
	if (it != functionsByName.end())
		out.insert(out.end(), it->second.begin(), it->second.end());
}

Function* BasicScope::getLocalFunction(FunctionSignature const& signature) const
{
	map<FunctionSignature, Function*>::const_iterator it = functionsBySignature.find(signature);
	if (it == functionsBySignature.end()) return NULL;
	return it->second;
}

Function* BasicScope::addFunction(ZVarType const* returnType, string const& name, vector<ZVarType const*> const& paramTypes, AST* node)
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
    table.addGlobalPointer(addVariable(ZVarType::LINK, "Link")->id);
    table.addGlobalPointer(addVariable(ZVarType::SCREEN, "Screen")->id);
    table.addGlobalPointer(addVariable(ZVarType::GAME, "Game")->id);
	table.addGlobalPointer(addVariable(ZVarType::AUDIO, "Audio")->id);
	table.addGlobalPointer(addVariable(ZVarType::DEBUG, "Debug")->id);
}

ScriptScope* GlobalScope::makeScriptChild(Script& script)
{
	string name = script.getName();
	map<string, Scope*>::const_iterator it = children.find(name);
	if (it != children.end()) return NULL;
	ScriptScope* child = new ScriptScope(this, script);
	children[name] = child;
	return child;
}


////////////////////////////////////////////////////////////////
// ScriptScope

////////////////////////////////////////////////////////////////
// ZClass

ZClass::ZClass(SymbolTable& table, string const& name, int id)
	: BasicScope(table), name(name), id(id)
{}


