#include "../precompiled.h"
#include "ZScript.h"

#include <sstream>
#include "CompilerUtils.h"
#include "CompileError.h"
#include "DataStructs.h"
#include "Types.h"
#include "Scope.h"
#include "../ffscript.h"
extern FFScript FFCore;


using namespace ZScript;
using std::vector;
using std::string;
using std::ostringstream;

////////////////////////////////////////////////////////////////
// ZScript::Program

Program::Program(ASTFile& root, CompileErrorHandler* errorHandler)
	: root_(root), rootScope_(new RootScope(typeStore_))
{
	// Create the ~Init script.
	if (Script* initScript =
	    	createScript(
				*this, *rootScope_, ScriptType::global,
				"~Init", errorHandler))
	{
		scripts.push_back(initScript);
		scriptsByName_[initScript->getName()] = initScript;
	}
}

Program::~Program()
{
	deleteElements(scripts);
	delete rootScope_;
}

Script* Program::getScript(string const& name) const
{
	return find<Script*>(scriptsByName_, name).value_or(boost::add_pointer<Script>::type());
}

Script* Program::getScript(ASTScript* node) const
{
	return find<Script*>(scriptsByNode_, node).value_or(boost::add_pointer<Script>::type());
}

Script* Program::addScript(
		ASTScript& node, Scope& parentScope, CompileErrorHandler* handler)
{
	Script* script = createScript(*this, parentScope, node, handler);
	if (!script) return NULL;

	scripts.push_back(script);
	scriptsByName_[script->getName()] = script;
	scriptsByNode_[&node] = script;
	return script;
}

Namespace* Program::addNamespace(ASTNamespace& node, Scope& parentScope, CompileErrorHandler* handler)
{
	Namespace* namesp = createNamespace(*this, parentScope, node, handler);
	if(!namesp) return NULL;
	
	for(vector<Namespace*>::iterator it = namespaces.begin();
		it != namespaces.end(); ++it)
		if(namesp == *it) return namesp; //Already registered, don't re-register.
	namespaces.push_back(namesp);
	return namesp;
}

vector<Function*> Program::getUserGlobalFunctions() const
{
	vector<Function*> functions = rootScope_->getLocalFunctions();
	for (vector<Function*>::iterator it = functions.begin();
	     it != functions.end();)
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
		if (function.isInternal()) it = functions.erase(it);
		else ++it;
	}
	return functions;
}

vector<Function*> Program::getInternalFunctions() const
{
	vector<Function*> functions = getFunctions(*this);
	for (vector<Function*>::iterator it = functions.begin();
	     it != functions.end();)
	{
		Function& function = **it;
		if (function.isInternal()) ++it;
		else it = functions.erase(it);
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

Script::Script(Program& program)
	: program(program), runFunc(NULL)
{}

Script::~Script()
{
	deleteElements(code);
}

bool Script::isPrototypeRun() const
{
	if(runFunc && runFunc->prototype) return true;
	return false;
}

// ZScript::UserScript

UserScript::UserScript(Program& program, ASTScript& node)
	: Script(program), node(node)
{}

ScriptType UserScript::getType() const
{
	return resolveScriptType(*node.type, *scope->getParent());
}

// ZScript::BuiltinScript
const string BuiltinScript::builtin_author = "ZQ_PARSER";
BuiltinScript::BuiltinScript(
		Program& program, ScriptType type, string const& name)
	: Script(program), type(type), name(name)
{}

// ZScript

UserScript* ZScript::createScript(
		Program& program, Scope& parentScope, ASTScript& node,
		CompileErrorHandler* errorHandler)
{
	UserScript* script = new UserScript(program, node);

	ScriptScope* scope = parentScope.makeScriptChild(*script);
	if (!scope)
	{
		if (errorHandler)
			errorHandler->handleError(
				CompileError::ScriptRedef(&node, node.name));
		delete script;
		return NULL;
	}
	script->scope = scope;

	if (!resolveScriptType(*node.type, parentScope).isValid())
	{
		if (errorHandler)
			errorHandler->handleError(
				CompileError::ScriptBadType(&node, node.name));
		delete script;
		return NULL;
	}

	return script;
}

BuiltinScript* ZScript::createScript(
		Program& program, Scope& parentScope, ScriptType type,
		string const& name, CompileErrorHandler* errorHandler)
{
	BuiltinScript* script = new BuiltinScript(program, type, name);

	ScriptScope* scope = parentScope.makeScriptChild(*script);
	if (!scope)
	{
		if (errorHandler)
			errorHandler->handleError(CompileError::ScriptRedef(NULL, name));
		delete script;
		return NULL;
	}
	script->scope = scope;

	if (!type.isValid())
	{
		if (errorHandler)
			errorHandler->handleError(
				CompileError::ScriptBadType(NULL, name));
		delete script;
		return NULL;
	}

	return script;
}

optional<int32_t> ZScript::getLabel(Script const& script)
{
	if (Function* run = script.getRun())
		return run->getLabel();
	return nullopt;
}

////////////////////////////////////////////////////////////////
// ZScript::Namespace

Namespace::Namespace(ASTNamespace& namesp)
	: name(namesp.name)
{}

Namespace* ZScript::createNamespace(
		Program& program, Scope& parentScope, ASTNamespace& node,
		CompileErrorHandler* errorHandler)
{
	NamespaceScope* scope = parentScope.makeNamespaceChild(node);
	Namespace* namesp = scope->namesp;

	return namesp;
}

////////////////////////////////////////////////////////////////
// ZScript::Datum

Datum::Datum(Scope& scope, DataType const& type)
	: scope(scope), type(type), id(ScriptParser::getUniqueVarID())
{}

bool Datum::tryAddToScope(CompileErrorHandler* errorHandler)
{
	return scope.add(*this, errorHandler);
}

bool ZScript::isGlobal(Datum const& datum)
{
	return (datum.scope.isGlobal() || datum.scope.isScript())
		&& datum.getName();
}

optional<int32_t> ZScript::getStackOffset(Datum const& datum)
{
	return lookupStackPosition(datum.scope, datum);
}

// ZScript::Literal

Literal* Literal::create(
		Scope& scope, ASTLiteral& node, DataType const& type,
		CompileErrorHandler* errorHandler)
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
		CompileErrorHandler* errorHandler)
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
	           ? optional<int32_t>(ScriptParser::getUniqueGlobalID())
	           : nullopt)
{
	node.manager = this;
}

optional<int32_t> Variable::getCompileTimeValue(bool getinitvalue) const
{
	if(getinitvalue)
	{
		ASTExprVarInitializer* init = node.getInitializer();
		return init ? init->value : nullopt;
	}
	return nullopt;
}

// ZScript::BuiltinVariable

BuiltinVariable* BuiltinVariable::create(
		Scope& scope, DataType const& type, string const& name,
		CompileErrorHandler* errorHandler)
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
	           ? optional<int32_t>(ScriptParser::getUniqueGlobalID())
	           : nullopt)
{}

// ZScript::Constant

Constant* Constant::create(
		Scope& scope, ASTDataDecl& node, DataType const& type, int32_t value,
		CompileErrorHandler* errorHandler)
{
	Constant* constant = new Constant(scope, node, type, value);
	if (constant->tryAddToScope(errorHandler)) return constant;
	delete constant;
	return NULL;
}

Constant::Constant(
		Scope& scope, ASTDataDecl& node, DataType const& type, int32_t value)
	: Datum(scope, type), node(node), value(value)
{
	node.manager = this;
}

optional<string> Constant::getName() const {return node.name;}

// ZScript::BuiltinConstant


BuiltinConstant* BuiltinConstant::create(
		Scope& scope, DataType const& type, string const& name, int32_t value,
		CompileErrorHandler* errorHandler)
{
	BuiltinConstant* builtin = new BuiltinConstant(scope, type, name, value);
	if (builtin->tryAddToScope(errorHandler)) return builtin;
	delete builtin;
	return NULL;
}

BuiltinConstant::BuiltinConstant(
		Scope& scope, DataType const& type, string const& name, int32_t value)
	: Datum(scope, type), name(name), value(value)
{}

// ZScript::FunctionSignature

FunctionSignature::FunctionSignature(
		string const& name, vector<DataType const*> const& parameterTypes)
	: name(name), parameterTypes(parameterTypes), prefix(false)
{}

FunctionSignature::FunctionSignature(Function const& function)
	: name(function.name), parameterTypes(function.paramTypes), prefix(function.hasPrefixType)
{}
		
int32_t FunctionSignature::compare(FunctionSignature const& other) const
{
	int32_t c = name.compare(other.name);
	if (c) return c;
	c = parameterTypes.size() - other.parameterTypes.size();
	if (c) return c;
	for (int32_t i = 0; i < (int32_t)parameterTypes.size(); ++i)
	{
		c = parameterTypes[i]->compare(*other.parameterTypes[i]);
		if (c) return c;
	}
	return 0;
}

bool FunctionSignature::operator==(FunctionSignature const& other) const
{
	return compare(other) == 0;
}

bool FunctionSignature::operator<(FunctionSignature const& other) const
{
	return compare(other) < 0;
}

string FunctionSignature::asString() const
{
	ostringstream oss;
	vector<DataType const*>::const_iterator it = parameterTypes.begin();
	if(prefix)
	{
		oss << (*it)->getName() << "->";
		++it;
	}
	oss << name << "(";
	if(it != parameterTypes.end())
	{
		//Add the first type; the loop adds a comma before it.
		oss << (*it)->getName();
		++it;
	}
	for (; it != parameterTypes.end(); ++it)
	{
		oss << ", ";
		oss << (*it)->getName();
	}
	oss << ")";
	return oss.str();
}

// ZScript::Function

Function::Function(DataType const* returnType, string const& name,
				   vector<DataType const*> paramTypes, vector<string const*> paramNames, int32_t id,
				   int32_t flags, int32_t internal_flags, bool prototype, ASTExprConst* defaultReturn)
	: node(NULL), internalScope(NULL), thisVar(NULL),
	  returnType(returnType), name(name), paramTypes(paramTypes), paramNames(paramNames),
	  id(id), label(nullopt), flags(flags), internal_flags(internal_flags), hasPrefixType(false),
	  prototype(prototype), defaultReturn(defaultReturn)
{}

Function::~Function()
{
	deleteElements(ownedCode);
	deleteElements(paramNames);
}

vector<Opcode*> Function::takeCode()
{
	vector<Opcode*> code = ownedCode;
	ownedCode.clear();
	return code;
}

void Function::giveCode(vector<Opcode*>& code)
{
	appendElements(ownedCode, code);
	code.clear();
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

int32_t Function::getLabel() const
{
	if (!label) label = ScriptParser::getUniqueLabelID();
	return *label;
}

bool Function::isTracing() const
{
	std::string prefix = name.substr(0, 5);
	return *returnType == DataType::ZVOID
		&& (prefix == "Trace" || prefix == "print");
}

bool ZScript::isRun(Function const& function)
{
	//al_trace("Parser sees run string as: %s\n", FFCore.scriptRunString);
	return function.internalScope->getParent()->isScript()
		&& *function.returnType == DataType::ZVOID
		&& (!( strcmp(function.name.c_str(), FFCore.scriptRunString )))
		&& (!(function.getFlag(FUNCFLAG_INLINE))) ;
}

int32_t ZScript::getStackSize(Function const& function)
{
	return *lookupStackSize(*function.internalScope);
}

int32_t ZScript::getParameterCount(Function const& function)
{
	return function.paramTypes.size() + (isRun(function) ? 1 : 0);
}
