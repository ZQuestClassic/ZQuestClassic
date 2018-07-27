#include "../precompiled.h"
#include "CompileError.h"
#include "GlobalSymbols.h"
#include "Scope.h"
#include "Types.h"
#include "ZScript.h"

using namespace std;
using namespace ZScript;

////////////////////////////////////////////////////////////////
// Scope

Scope::Scope(SymbolTable& table)
	: table(table), name(nullopt), varDeclsDeprecated(false)
{}

Scope::Scope(SymbolTable& table, string const& name)
	: table(table), name(name), varDeclsDeprecated(false)
{}

void Scope::invalidateStackSize()
{
	if (Scope* parent = getParent())
		parent->invalidateStackSize();
}

// Inheritance

Scope* ZScript::getDescendant(Scope const& scope, vector<string> const& names)
{
	Scope* child = const_cast<Scope*>(&scope);
	for (vector<string>::const_iterator it = names.begin();
	     child && it != names.end(); ++it)
		child = child->getChild(*it);
	return child;
}

Scope* ZScript::lookupScope(Scope const& scope, string const& name)
{
	Scope* current = const_cast<Scope*>(&scope);
	while (current)
	{
		if (Scope* child = current->getChild(name))
	return child;

		if (optional<string> const& currentName = current->getName())
			if (*currentName == name)
				return current;

		current = current->getParent();
	}

	return NULL;
}

Scope* ZScript::lookupScope(Scope const& scope, vector<string> const& names)
{
	// Travel as far up the tree as needed for the first scope.
	Scope* current = lookupScope(scope, names.front());
	if (!current) return NULL;
	if (names.size() == 1) return current;

	// Descend with the rest of the names from the first found scope.
	vector<string> restOfNames(++names.begin(), names.end());
	return getDescendant(*current, restOfNames);
}

vector<Scope*> ZScript::lookupScopes(Scope const& scope, vector<string> const& names)
{
	vector<Scope*> scopes;
	for (Scope* current = const_cast<Scope*>(&scope);
	     current; current = current->getParent())
		if (Scope* descendant = getDescendant(*current, names))
			scopes.push_back(descendant);
	return scopes;
}

// Lookup

ZVarType const* ZScript::lookupType(Scope const& scope, string const& name)
{
	for (Scope const* current = &scope;
	     current; current = current->getParent())
		if (ZVarType const* type = scope.getLocalType(name))
			return type;
	return NULL;
}

ZClass* ZScript::lookupClass(Scope const& scope, string const& name)
{
	for (Scope const* current = &scope;
	     current; current = current->getParent())
		if (ZClass* klass = current->getLocalClass(name))
			return klass;
	return NULL;
}

Datum* ZScript::lookupDatum(Scope const& scope, string const& name)
{
	for (Scope const* current = &scope;
	     current; current = current->getParent())
		if (Datum* datum = current->getLocalDatum(name))
			return datum;
	return NULL;
}

Datum* ZScript::lookupDatum(Scope const& scope, vector<string> const& names)
{
	if (names.size() == 0)
		return NULL;
	else if (names.size() == 1)
		return lookupDatum(scope, names[0]);

	vector<string> childNames(names.begin(), --names.end());
	if (Scope* child = lookupScope(scope, childNames))
		return lookupDatum(*child, names.back());

	return NULL;
}

Function* ZScript::lookupGetter(Scope const& scope, string const& name)
{
	for (Scope const* current = &scope;
	     current; current = current->getParent())
		if (Function* getter = current->getLocalGetter(name))
			return getter;
	return NULL;
}

Function* ZScript::lookupSetter(Scope const& scope, string const& name)
{
	for (Scope const* current = &scope;
	     current; current = current->getParent())
		if (Function* setter = current->getLocalSetter(name))
			return setter;
	return NULL;
}

Function* ZScript::lookupFunction(Scope const& scope,
                         Function::Signature const& signature)
{
	for (Scope const* current = &scope;
	     current; current = current->getParent())
		if (Function* function = current->getLocalFunction(signature))
			return function;
	return NULL;
}

vector<Function*> ZScript::lookupFunctions(Scope const& scope, string const& name)
{
	vector<Function*> functions;
	for (Scope const* current = &scope;
	     current; current = current->getParent())
	{
		vector<Function*> currentFunctions = current->getLocalFunctions(name);
		functions.insert(functions.end(),
		                 currentFunctions.begin(), currentFunctions.end());
	}
	return functions;
}

vector<Function*> ZScript::lookupFunctions(
		Scope const& scope, vector<string> const& names)
{
	if (names.size() == 0)
		return vector<Function*>();
	else if (names.size() == 1)
		return lookupFunctions(scope, names[0]);

	vector<Function*> functions;
	string const& name = names.back();

	vector<string> ancestry(names.begin(), --names.end());
	vector<Scope*> scopes = lookupScopes(scope, ancestry);
	for (vector<Scope*>::const_iterator it = scopes.begin();
	     it != scopes.end(); ++it)
	{
		Scope& current = **it;
		vector<Function*> currentFunctions = current.getLocalFunctions(name);
		functions.insert(functions.end(),
		                 currentFunctions.begin(), currentFunctions.end());
	}

	return functions;
}

// Stack

bool ZScript::isStackRoot(Scope const& scope)
{
	return scope.getRootStackSize();
}

optional<int> ZScript::lookupStackOffset(
		Scope const& scope, Datum const& datum)
{
	Scope* s = const_cast<Scope*>(&scope);
	while (s)
	{
		if (optional<int> offset = s->getLocalStackOffset(datum))
			return offset;

		if (isStackRoot(*s)) return nullopt;

		s = s->getParent();
	}
	return nullopt;
}

optional<int> ZScript::lookupStackSize(Scope const& scope)
{
	Scope* s = const_cast<Scope*>(&scope);
	while (s)
	{
		if (optional<int> size = s->getRootStackSize())
			return size;

		s = s->getParent();
	}
	return nullopt;
}

optional<int> ZScript::lookupStackPosition(
		Scope const& scope, Datum const& datum)
{
	optional<int> offset = lookupStackOffset(scope, datum);
	optional<int> size = lookupStackSize(scope);
	if (offset && size)
		return *size - 1 - *offset;
	return nullopt;
}

// Get all in branch

vector<Function*> ZScript::getFunctionsInBranch(Scope const& scope)
{
	typedef vector<Function*> (Scope::*Call)() const;
	Call call = static_cast<Call>(&Scope::getLocalFunctions);
	return getInBranch<Function*>(scope, call);
}

////////////////////////////////////////////////////////////////
// Basic Scope

BasicScope::BasicScope(Scope* parent)
	: Scope(parent->getTable()), parent(parent),
	  stackDepth(parent->getLocalStackDepth())
{}

BasicScope::BasicScope(Scope* parent, string const& name)
	: Scope(parent->getTable(), name), parent(parent),
	  stackDepth(parent->getLocalStackDepth())
{}

BasicScope::BasicScope(SymbolTable& table)
	: Scope(table), parent(NULL), stackDepth(0)
{}

BasicScope::BasicScope(SymbolTable& table, string const& name)
	: Scope(table, name), parent(NULL), stackDepth(0)
{}

BasicScope::~BasicScope()
{
	deleteSeconds(children);
	deleteElements(anonymousChildren);
	deleteElements(anonymousData);
	deleteSeconds(namedData);
	deleteSeconds(getters);
	deleteSeconds(setters);
	deleteSeconds(functionsBySignature);
}

// Inheritance

Scope* BasicScope::getChild(string const& name) const
{
	return find<Scope*>(children, name).value_or(NULL);
}

vector<Scope*> BasicScope::getChildren() const
{
	vector<Scope*> results = anonymousChildren;
	appendElements(results, getSeconds<Scope*>(children));
	return results;
}

// Lookup Local

ZVarType const* BasicScope::getLocalType(string const& name) const
{
	return find<ZVarType const*>(types, name).value_or(NULL);
}

ZClass* BasicScope::getLocalClass(string const& name) const
{
	return find<ZClass*>(classes, name).value_or(NULL);
}

Datum* BasicScope::getLocalDatum(string const& name) const
{
	return find<Datum*>(namedData, name).value_or(NULL);
}

Function* BasicScope::getLocalGetter(string const& name) const
{
	return find<Function*>(getters, name).value_or(NULL);
}

Function* BasicScope::getLocalSetter(string const& name) const
{
	return find<Function*>(setters, name).value_or(NULL);
}

Function* BasicScope::getLocalFunction(
		Function::Signature const& signature) const
{
	return find<Function*>(functionsBySignature, signature).value_or(NULL);
}

vector<Function*> BasicScope::getLocalFunctions(string const& name) const
{
	return find<vector<Function*> >(functionsByName, name)
		.value_or(vector<Function*>());
}

// Get All Local

vector<Datum*> BasicScope::getLocalData() const
{
	vector<Datum*> results = getSeconds<Datum*>(namedData);
	appendElements(results, anonymousData);
	return results;
}

vector<Function*> BasicScope::getLocalFunctions() const
{
	return getSeconds<Function*>(functionsBySignature);
}

// Add

Scope* BasicScope::makeChild()
{
	Scope* child = new BasicScope(this);
	anonymousChildren.push_back(child);
	return child;
}

Scope* BasicScope::makeChild(string const& name)
{
	if (find<Scope*>(children, name)) return NULL;
	Scope* child = new BasicScope(this, name);
	children[name] = child;
	return child;
}

FunctionScope* BasicScope::makeFunctionChild(Function& function)
{
	FunctionScope* child = new FunctionScope(this, function);
	anonymousChildren.push_back(child);
	return child;
}

ZVarType const* BasicScope::addType(
		string const& name, ZVarType const* type, AST* node)
{
	if (find<ZVarType const*>(types, name)) return NULL;
	type = table.getCanonicalType(*type);
	types[name] = type;
	return type;
}

bool BasicScope::add(Datum& datum, CompileErrorHandler& errorHandler)
{
	if (optional<string> name = datum.getName())
	{
		if (find<Datum*>(namedData, *name))
	{
			errorHandler.handleError(CompileError::VarRedef,
			                         datum.getNode(), name->c_str());
			return false;
		}
		namedData[*name] = &datum;
	}
	else anonymousData.push_back(&datum);

	if (!ZScript::isGlobal(datum))
	{
		stackOffsets[&datum] = stackDepth++;
		invalidateStackSize();
	}
	
	return true;
}

Function* BasicScope::addGetter(
		ZVarType const* returnType, string const& name,
		vector<ZVarType const*> const& paramTypes, AST* node)
{
	if (find<Function*>(getters, name)) return NULL;

	Function* fun = new Function(
			returnType, name, paramTypes, ScriptParser::getUniqueFuncID());
	getters[name] = fun;
	return fun;
}

Function* BasicScope::addSetter(
		ZVarType const* returnType, string const& name,
		vector<ZVarType const*> const& paramTypes, AST* node)
{
	if (find<Function*>(setters, name)) return NULL;

	Function* fun = new Function(
			returnType, name, paramTypes, ScriptParser::getUniqueFuncID());
	setters[name] = fun;
	return fun;
}

Function* BasicScope::addFunction(
		ZVarType const* returnType, string const& name,
		vector<ZVarType const*> const& paramTypes, AST* node)
{
	Function::Signature signature(name, paramTypes);
	if (find<Function*>(functionsBySignature, signature))
		return NULL;

	Function* fun = new Function(
			returnType, name, paramTypes, ScriptParser::getUniqueFuncID());
	functionsByName[name].push_back(fun);
	functionsBySignature[signature] = fun;
	return fun;
}

// Stack

optional<int> BasicScope::getLocalStackOffset(Datum const& datum) const
{
	Datum* key = const_cast<Datum*>(&datum);
	return find<int>(stackOffsets, key);
}

////////////////////////////////////////////////////////////////
// StackRoot

int calculateStackSize(Scope* scope)
{
	int greatestSize = scope->getLocalStackDepth();
	vector<Scope*> children = scope->getChildren();
	for (vector<Scope*>::const_iterator it = children.begin();
	     it != children.end(); ++it)
	{
		int size = calculateStackSize(*it);
		if (greatestSize < size) greatestSize = size;
	}
	return greatestSize;
}

////////////////////////////////////////////////////////////////
// GlobalScope

GlobalScope::GlobalScope(SymbolTable& table)
	: BasicScope(table, "global")
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

	// Add builtin pointers.
	BuiltinConstant::create(*this, ZVarType::LINK, "Link", 0);
	BuiltinConstant::create(*this, ZVarType::SCREEN, "Screen", 0);
	BuiltinConstant::create(*this, ZVarType::GAME, "Game", 0);
	BuiltinConstant::create(*this, ZVarType::AUDIO, "Audio", 0);
	BuiltinConstant::create(*this, ZVarType::DEBUG, "Debug", 0);
	BuiltinConstant::create(*this, ZVarType::NPCDATA, "NPCData", 0);
	BuiltinConstant::create(*this, ZVarType::TEXT, "Text", 0);
	BuiltinConstant::create(*this, ZVarType::COMBOS, "ComboData", 0);
	BuiltinConstant::create(*this, ZVarType::SPRITEDATA, "SpriteData", 0);
	BuiltinConstant::create(*this, ZVarType::GRAPHICS, "Graphics", 0);
	BuiltinConstant::create(*this, ZVarType::INPUT, "Input", 0);
	BuiltinConstant::create(*this, ZVarType::MAPDATA, "MapData", 0);
	BuiltinConstant::create(*this, ZVarType::DMAPDATA, "DMapData", 0);
	BuiltinConstant::create(*this, ZVarType::ZMESSAGE, "MessageData", 0);
	BuiltinConstant::create(*this, ZVarType::SHOPDATA, "ShopData", 0);
	BuiltinConstant::create(*this, ZVarType::UNTYPED, "Untyped", 0);
	BuiltinConstant::create(*this, ZVarType::DROPSET, "DropData", 0);
	BuiltinConstant::create(*this, ZVarType::PONDS, "PondData", 0);
	BuiltinConstant::create(*this, ZVarType::WARPRING, "WarpRing", 0);
	BuiltinConstant::create(*this, ZVarType::DOORSET, "DoorSet", 0);
	BuiltinConstant::create(*this, ZVarType::ZUICOLOURS, "MiscColors", 0);
	BuiltinConstant::create(*this, ZVarType::RGBDATA, "RGBData", 0);
	BuiltinConstant::create(*this, ZVarType::PALETTE, "Palette", 0);
	BuiltinConstant::create(*this, ZVarType::TUNES, "MusicTrack", 0);
	BuiltinConstant::create(*this, ZVarType::PALCYCLE, "PalCycle", 0);
	BuiltinConstant::create(*this, ZVarType::GAMEDATA, "GameData", 0);
	BuiltinConstant::create(*this, ZVarType::CHEATS, "Cheats", 0);
}

ScriptScope* GlobalScope::makeScriptChild(Script& script)
{
	string name = script.getName();
	if (find<Scope*>(children, name)) return NULL;
	ScriptScope* child = new ScriptScope(this, script);
	children[name] = child;
	return child;
}

optional<int> GlobalScope::getRootStackSize() const
{
	if (!stackSize)
	{
		GlobalScope* mutableThis = const_cast<GlobalScope*>(this);
		stackSize = calculateStackSize(mutableThis);
	}
	return stackSize;
}

////////////////////////////////////////////////////////////////
// ScriptScope

ScriptScope::ScriptScope(GlobalScope* parent, Script& script)
	: BasicScope(parent, script.getName()), script(script)
{}

////////////////////////////////////////////////////////////////
// FunctionScope

FunctionScope::FunctionScope(Scope* parent, Function& function)
	: BasicScope(parent, function.name), function(function)
{
	// Functions have their own stack.
	stackDepth = 0;
}

optional<int> FunctionScope::getRootStackSize() const
{
	if (!stackSize)
	{
		FunctionScope* mutableThis = const_cast<FunctionScope*>(this);
		stackSize = calculateStackSize(mutableThis);
	}
	return stackSize;
}

////////////////////////////////////////////////////////////////
// ZClass

ZClass::ZClass(SymbolTable& table, string const& name, int id)
	: BasicScope(table), name(name), id(id)
{}


