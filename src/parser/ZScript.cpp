#include "../precompiled.h"
#include "CompilerUtils.h"
#include "CompileError.h"
#include "DataStructs.h"
#include "Types.h"
#include "Scope.h"
#include "ZScript.h"

//using namespace std;
using std::string;
using std::vector;
using std::map;

using namespace ZScript;

////////////////////////////////////////////////////////////////
// ZScript::Program

Program::Program(
		ASTProgram& node,
		TypeStore& typeStore,
		CompileErrorHandler& errorHandler)
	: node(node),
	  typeStore(typeStore),
	  globalScope(new GlobalScope(typeStore))
{
	// Create a ZScript::Script for every script in the program.
	for (vector<ASTScript*>::const_iterator it = node.scripts.begin();
		 it != node.scripts.end(); ++it)
	{
		Script* script = createScript(*this, **it, errorHandler);
		if (!script) continue;
		
		scripts.push_back(script);
		scriptsByName[script->getName()] = script;
		scriptsByNode[*it] = script;
	}

	// Create the ~Init script.
	if (Script* initScript =
	    createScript(*this, ScriptType::getGlobal(), "~Init", errorHandler))
	{
		scripts.push_back(initScript);
		scriptsByName[initScript->getName()] = initScript;
	}
}

Program::~Program()
{
	deleteElements(scripts);
	delete globalScope;
}

Script* Program::getScript(string const& name) const
{
	map<string, Script*>::const_iterator it = scriptsByName.find(name);
	if (it == scriptsByName.end()) return NULL;
	return it->second;
}

Script* Program::getScript(ASTScript* node) const
{
	map<ASTScript*, Script*>::const_iterator it = scriptsByNode.find(node);
	if (it == scriptsByNode.end()) return NULL;
	return it->second;
}

vector<Function*> Program::getUserGlobalFunctions() const
{
	vector<Function*> functions = globalScope->getLocalFunctions();
	for (vector<Function*>::iterator it = functions.begin(); it != functions.end();)
	{
		Function& function = **it;
		if (!function.node) it = functions.erase(it);
		else ++it;
	}
	return functions;
}

vector<Function*> Program::getUserFunctions() const
{
	vector<Function*> functions = getFunctions(*this);
	for (vector<Function*>::iterator it = functions.begin();
	     it != functions.end();)
	{
		Function& function = **it;
		if (!function.node) it = functions.erase(it);
		else ++it;
	}
	return functions;
}

vector<Function*> ZScript::getFunctions(Program const& program)
{
	vector<Function*> functions = getFunctionsInBranch(program.getScope());
	appendElements(functions, getClassFunctions(program.getTypeStore()));
	return functions;
}

////////////////////////////////////////////////////////////////
// ZScript::Script

// ZScript::Script

Script::Script(Program& program) : program(program) {}

// ZScript::UserScript

UserScript::UserScript(Program& program, ASTScript& node)
	: Script(program), node(node)
{}

// ZScript::BuiltinScript

BuiltinScript::BuiltinScript(
		Program& program, ScriptType type, string const& name)
	: Script(program), type(type), name(name)
{}

// ZScript

UserScript* ZScript::createScript(
		Program& program, ASTScript& node,
		CompileErrorHandler& errorHandler)
{
	UserScript* script = new UserScript(program, node);

	ScriptScope* scope = program.getScope().makeScriptChild(*script);
	if (!scope)
	{
		errorHandler.handleError(
				CompileError::ScriptRedef, &node, node.name.c_str());
		delete script;
		return NULL;
	}
	scope->varDeclsDeprecated = true;
	script->scope = scope;

	if (node.type->type.isNull())
	{
		errorHandler.handleError(
				CompileError::ScriptBadType, &node, node.name.c_str());
		delete script;
		return NULL;
	}

	return script;
}

BuiltinScript* ZScript::createScript(
		Program& program, ScriptType type, string const& name,
		CompileErrorHandler& errorHandler)
{
	BuiltinScript* script = new BuiltinScript(program, type, name);

	ScriptScope* scope = program.getScope().makeScriptChild(*script);
	if (!scope)
	{
		errorHandler.handleError(
				CompileError::ScriptRedef, NULL, name.c_str());
		delete script;
		return NULL;
	}
	scope->varDeclsDeprecated = true;
	script->scope = scope;

	if (type.isNull())
	{
		errorHandler.handleError(
				CompileError::ScriptBadType, NULL, name.c_str());
		delete script;
		return NULL;
	}

	return script;
}

Function* ZScript::getRunFunction(Script const& script)
{
	return getOnly<Function*>(script.getScope().getLocalFunctions("run"))
		.value_or(NULL);
}

optional<int> ZScript::getLabel(Script const& script)
{
	if (Function* run = getRunFunction(script))
		return run->getLabel();
	return nullopt;
}

////////////////////////////////////////////////////////////////
// ZScript::Datum

Datum::Datum(Scope& scope, DataType const& type)
	: scope(scope), type(type), id(ScriptParser::getUniqueVarID())
{}

bool Datum::tryAddToScope(CompileErrorHandler& errorHandler)
{
	return scope.add(*this, errorHandler);
}

bool ZScript::isGlobal(Datum const& datum)
{
	return (datum.scope.isGlobal() || datum.scope.isScript())
		&& datum.getName();
}

optional<int> ZScript::getStackOffset(Datum const& datum)
{
	return lookupStackPosition(datum.scope, datum);
}

// ZScript::Literal

Literal* Literal::create(
		Scope& scope, ASTLiteral& node, DataType const& type,
		CompileErrorHandler& errorHandler)
{
	Literal* literal = new Literal(scope, node, type);
	if (literal->tryAddToScope(errorHandler)) return literal;
	delete literal;
	return NULL;
}

Literal::Literal(Scope& scope, ASTLiteral& node, DataType const& type)
	: Datum(scope, type), node(node)
{
	node.manager = this;
}

// ZScript::Variable

Variable* Variable::create(
		Scope& scope, ASTDataDecl& node, DataType const& type,
		CompileErrorHandler& errorHandler)
{
	Variable* variable = new Variable(scope, node, type);
	if (variable->tryAddToScope(errorHandler)) return variable;
	delete variable;
	return NULL;
}

Variable::Variable(
		Scope& scope, ASTDataDecl& node, DataType const& type)
	: Datum(scope, type),
	  node(node),
	  globalId((scope.isGlobal() || scope.isScript())
	           ? optional<int>(ScriptParser::getUniqueGlobalID())
	           : nullopt)
{
	node.manager = this;
}

// ZScript::BuiltinVariable

BuiltinVariable* BuiltinVariable::create(
		Scope& scope, DataType const& type, string const& name,
		CompileErrorHandler& errorHandler)
{
	BuiltinVariable* builtin = new BuiltinVariable(scope, type, name);
	if (builtin->tryAddToScope(errorHandler)) return builtin;
	delete builtin;
	return NULL;
}

BuiltinVariable::BuiltinVariable(
		Scope& scope, DataType const& type, string const& name)
	: Datum(scope, type),
	  name(name),
	  globalId((scope.isGlobal() || scope.isScript())
	           ? optional<int>(ScriptParser::getUniqueGlobalID())
	           : nullopt)
{}

// ZScript::Constant

Constant* Constant::create(
		Scope& scope, ASTDataDecl& node, DataType const& type, long value,
		CompileErrorHandler& errorHandler)
{
	Constant* constant = new Constant(scope, node, type, value);
	if (constant->tryAddToScope(errorHandler)) return constant;
	delete constant;
	return NULL;
}

Constant::Constant(
		Scope& scope, ASTDataDecl& node, DataType const& type, long value)
	: Datum(scope, type), node(node), value(value)
{
	node.manager = this;
}

optional<string> Constant::getName() const {return node.name;}

// ZScript::BuiltinConstant


BuiltinConstant* BuiltinConstant::create(
		Scope& scope, DataType const& type, string const& name, long value,
		CompileErrorHandler& errorHandler)
{
	BuiltinConstant* builtin = new BuiltinConstant(scope, type, name, value);
	if (builtin->tryAddToScope(errorHandler)) return builtin;
	delete builtin;
	return NULL;
}

BuiltinConstant::BuiltinConstant(
		Scope& scope, DataType const& type, string const& name, long value)
	: Datum(scope, type), name(name), value(value)
{}

// ZScript::Function::Signature

Function::Signature::Signature(
		string const& name, vector<DataType> const& parameterTypes)
	: name(name), parameterTypes(parameterTypes)
{}

Function::Signature::Signature(Function const& function)
	: name(function.name), parameterTypes(function.paramTypes)
{}
		
int Function::Signature::compare(Function::Signature const& other) const
{
	int c = name.compare(other.name);
	if (c) return c;
	c = parameterTypes.size() - other.parameterTypes.size();
	if (c) return c;
	for (int i = 0; i < (int)parameterTypes.size(); ++i)
	{
		c = parameterTypes[i].compare(other.parameterTypes[i]);
		if (c) return c;
	}
	return 0;
}

bool Function::Signature::operator==(Function::Signature const& other) const
{
	return compare(other) == 0;
}

bool Function::Signature::operator<(Function::Signature const& other) const
{
	return compare(other) < 0;
}

string Function::Signature::asString() const
{
	string result;
	result += name;
	result += "(";
	bool comma = false;
	for (vector<DataType>::const_iterator it = parameterTypes.begin();
		 it != parameterTypes.end(); ++it)
	{
		if (comma) {result += ", ";}
		comma = true;
		result += it->getName();
	}
	result += ")";
	return result;
}

// ZScript::Function

Function::Function(DataType const& returnType, string const& name,
				   vector<DataType> const& paramTypes, int id)
	: node(NULL), internalScope(NULL), thisVar(NULL),
	  returnType(returnType), name(name), paramTypes(paramTypes), id(id)
{}

void Function::setCode(vector<Opcode> const& code)
{
	code_ = code;
	code_.front()
		.withLabel(getLabel())
		.withComment("FUNCTION " + getSignature().asString());
}

Script* Function::getScript() const
{
	if (!internalScope) return NULL;
	Scope* parentScope = internalScope->getParent();
	if (!parentScope) return NULL;
	if (!parentScope->isScript()) return NULL;
	ScriptScope* scriptScope =
		dynamic_cast<ScriptScope*>(parentScope);
	return &scriptScope->script;
}

int Function::getLabel() const
{
	if (!label_) label_ = ScriptParser::getUniqueLabelID();
	return *label_;
}

bool ZScript::isRun(Function const& function)
{
	TypeStore& typeStore = function.returnType.getTypeStore();
	return function.internalScope->getParent()->isScript()
		&& function.returnType == typeStore.getVoid()
		&& function.name == "run";
}

int ZScript::getStackSize(Function const& function)
{
	return *lookupStackSize(*function.internalScope);
}

int ZScript::getParameterCount(Function const& function)
{
	return function.paramTypes.size() + (isRun(function) ? 1 : 0);
}
