#include "ZScript.h"

#include <sstream>
#include "CompilerUtils.h"
#include "CompileError.h"
#include "DataStructs.h"
#include "Types.h"
#include "Scope.h"
#include "zc/ffscript.h"
extern FFScript FFCore;


using namespace ZScript;
using std::vector;
using std::string;
using std::ostringstream;
using std::shared_ptr;

////////////////////////////////////////////////////////////////
// ZScript::Program

Program::Program(ASTFile& root, CompileErrorHandler* errorHandler)
	: rootScope_(new RootScope(typeStore_)), root_(root)
{
	// Create the ~Init script.
	if (Script* initScript =
	    	createScript(
				*this, *rootScope_, ParserScriptType::global,
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
	return find<Script*>(scriptsByName_, name).value_or(std::add_pointer<Script>::type());
}

Script* Program::getScript(ASTScript* node) const
{
	return find<Script*>(scriptsByNode_, node).value_or(std::add_pointer<Script>::type());
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

UserClass* Program::getClass(string const& name) const
{
	return find<UserClass*>(classesByName_, name).value_or(std::add_pointer<UserClass>::type());
}

UserClass* Program::getClass(ASTClass* node) const
{
	return find<UserClass*>(classesByNode_, node).value_or(std::add_pointer<UserClass>::type());
}
UserClass* Program::addClass(
		ASTClass& node, Scope& parentScope, CompileErrorHandler* handler)
{
	UserClass* user_class = createClass(*this, parentScope, node, handler);
	if (!user_class) return NULL;

	classes.push_back(user_class);
	classesByName_[user_class->getName()] = user_class;
	classesByNode_[&node] = user_class;
	return user_class;
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

vector<Function*> Program::getUserClassConstructors() const
{
	vector<Function*> functions;
	for(auto it = classes.begin(); it != classes.end(); ++it)
	{
		UserClass* user_class = *it;
		appendElements(functions, user_class->getScope().getConstructors());
	}
	return functions;
}
vector<Function*> Program::getUserClassDestructors() const
{
	vector<Function*> functions;
	for(auto it = classes.begin(); it != classes.end(); ++it)
	{
		UserClass* user_class = *it;
		appendElements(functions, user_class->getScope().getDestructor());
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
// ZScript::UserClass

UserClass::UserClass(Program& program, ASTClass& user_class)
	: classType(nullptr), program(program), node(user_class)
{
	members.push_back(0);
}

UserClass::~UserClass()
{}

UserClass* ZScript::createClass(
		Program& program, Scope& parentScope, ASTClass& node,
		CompileErrorHandler* errorHandler)
{
	UserClass* user_class = new UserClass(program, node);

	ClassScope* scope = parentScope.makeClassChild(*user_class);
	if (!scope)
	{
		if (errorHandler)
			errorHandler->handleError(
				CompileError::ClassRedef(&node, user_class->getName().c_str()));
		delete user_class;
		return NULL;
	}
	user_class->scope = scope;
	
	return user_class;
}

////////////////////////////////////////////////////////////////
// ZScript::Script

// ZScript::Script

Script::Script(Program& program)
	: runFunc(NULL), program(program)
{}

Script::~Script()
{
	//deleteElements(code);
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

ParserScriptType UserScript::getType() const
{
	return resolveScriptType(*node.type, *scope->getParent());
}

// ZScript::BuiltinScript
const string BuiltinScript::builtin_author = "ZQ_PARSER";
BuiltinScript::BuiltinScript(
		Program& program, ParserScriptType type, string const& name)
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
				CompileError::ScriptRedef(&node, script->getName().c_str()));
		delete script;
		return NULL;
	}
	script->scope = scope;

	if (!resolveScriptType(*node.type, parentScope).isValid())
	{
		if (errorHandler)
			errorHandler->handleError(
				CompileError::ScriptBadType(&node, script->getName().c_str()));
		delete script;
		return NULL;
	}

	return script;
}

BuiltinScript* ZScript::createScript(
		Program& program, Scope& parentScope, ParserScriptType type,
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

std::optional<int32_t> ZScript::getLabel(Script const& script)
{
	if (Function* run = script.getRun())
		return run->getLabel();
	return std::nullopt;
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

std::optional<int32_t> ZScript::getStackOffset(Datum const& datum)
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
	           ? std::optional<int32_t>(ScriptParser::getUniqueGlobalID())
	           : std::nullopt)
{
	node.manager = this;
}

std::optional<int32_t> Variable::getCompileTimeValue(bool getinitvalue) const
{
	if(getinitvalue)
	{
		ASTExprVarInitializer* init = node.getInitializer();
		return init ? init->value : std::nullopt;
	}
	return std::nullopt;
}

// ZScript::UserClassVar

UserClassVar* UserClassVar::create(
		Scope& scope, ASTDataDecl& node, DataType const& type,
		CompileErrorHandler* errorHandler)
{
	UserClassVar* ucv = new UserClassVar(scope, node, type);
	if (ucv->tryAddToScope(errorHandler))
	{
		ClassScope* cscope = scope.getClass();
		UserClass& user_class = cscope->user_class;
		if(type.isArray())
		{
			ucv->is_arr = true;
			int32_t totalSize = -1;
			if (std::optional<int32_t> size = node.extraArrays[0]->getCompileTimeSize(errorHandler, &scope))
				totalSize = *size;
			user_class.members.push_back(totalSize);
		}
		else
		{
			user_class.members[0]++;
		}
		return ucv;
	}
	delete ucv;
	return NULL;
}
UserClassVar::UserClassVar(
		Scope& scope, ASTDataDecl& node, DataType const& type)
	: Datum(scope, type), is_arr(false),
	  _index(0), node(node)
{
	node.manager = this;
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
	           ? std::optional<int32_t>(ScriptParser::getUniqueGlobalID())
	           : std::nullopt)
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

std::optional<string> Constant::getName() const {return node.name;}

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

FunctionSignature::FunctionSignature(string const& name,
	vector<DataType const*> const& parameterTypes, DataType const* returnType )
	: name(name), prefix(false), parameterTypes(parameterTypes), returnType(returnType)
{}

FunctionSignature::FunctionSignature(Function const& function, bool useret)
	: name(function.name), prefix(function.hasPrefixType),
	parameterTypes(function.paramTypes), returnType(useret ? function.returnType : nullptr)
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
	if(returnType && other.returnType)
	{
		c = returnType->compare(*other.returnType);
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
	if(returnType)
		oss << returnType->getName() << " ";
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
	: returnType(returnType), name(name), hasPrefixType(false),
	  extra_vargs(0), paramTypes(paramTypes), paramNames(paramNames), opt_vals(), id(id),
	  node(NULL), internalScope(NULL), thisVar(NULL), internal_flags(internal_flags), prototype(prototype),
	  defaultReturn(defaultReturn), label(std::nullopt), flags(flags), shown_depr(false),
	  aliased_func(nullptr)
{
	assert(returnType);
}


Function::~Function()
{
	//deleteElements(ownedCode);
	deleteElements(paramNames);
}

std::vector<std::shared_ptr<Opcode>> Function::takeCode()
{
	if(aliased_func)
		return aliased_func->takeCode();
	std::vector<std::shared_ptr<Opcode>> code = ownedCode;
	ownedCode.clear();
	return code;
}

void Function::giveCode(vector<shared_ptr<Opcode>>& code)
{
	if(aliased_func)
		return aliased_func->giveCode(code);
	appendElements(ownedCode, code);
	code.clear();
}

Script* Function::getScript() const
{
	if (!getInternalScope()) return NULL;
	Scope* parentScope = getInternalScope()->getParent();
	if (!parentScope) return NULL;
	if (!parentScope->isScript()) return NULL;
	ScriptScope* scriptScope =
		dynamic_cast<ScriptScope*>(parentScope);
	return &scriptScope->script;
}
UserClass* Function::getClass() const
{
	if (!getInternalScope()) return NULL;
	Scope* parentScope = getInternalScope()->getParent();
	if (!parentScope) return NULL;
	if (!parentScope->isClass()) return NULL;
	ClassScope* classScope =
		dynamic_cast<ClassScope*>(parentScope);
	return &classScope->user_class;
}

int32_t Function::getLabel() const
{
	if(aliased_func)
		return aliased_func->getLabel();
	if (!label) label = ScriptParser::getUniqueLabelID();
	return *label;
}
int32_t Function::getAltLabel() const
{
	if(aliased_func)
		return aliased_func->getAltLabel();
	if (!altlabel) altlabel = ScriptParser::getUniqueLabelID();
	return *altlabel;
}

bool Function::isTracing() const
{
	std::string prefix = name.substr(0, 5);
	return *returnType == DataType::ZVOID
		&& (prefix == "Trace" || prefix == "print");
}

//Return true the first time it is called, if func is deprecated
bool Function::shouldShowDepr(bool err) const
{
	if(!getFlag(FUNCFLAG_DEPRECATED)) return false;
	if(err) return shown_depr < 2;
	return !shown_depr;
}
void Function::ShownDepr(bool err)
{
	if(err)
		shown_depr = 2;
	else if(shown_depr < 2)
		shown_depr = 1;
}

void Function::alias(Function* func, bool force)
{
	assert(!aliased_func); //Should never change once set
	aliased_func = func;
	
	if(!returnType)
		force = true;
	if(force)
	{
		for(auto type_ptr : paramTypes)
			delete type_ptr;
		paramTypes.clear();
		for(auto type_ptr : func->paramTypes)
			paramTypes.push_back(type_ptr->clone());
		hasPrefixType = func->hasPrefixType;
		delete returnType;
		returnType = func->returnType;
		ownedCode.clear();
		label.reset();
		altlabel.reset();
	}
	else
	{
		//Ensure the function is a valid match
		assert(paramTypes.size() == func->paramTypes.size());
		for(size_t q = 0; q < paramTypes.size(); ++q)
		{
			assert(paramTypes[q]->canCastTo(*func->paramTypes[q]));
		}
		assert(hasPrefixType == func->hasPrefixType);
		assert(returnType->canCastTo(*func->returnType));
		//Ensure the function had no owned info of its' own
		assert(ownedCode.empty());
		assert(!(label || altlabel));
	}
}

bool ZScript::isRun(Function const& function)
{
	//al_trace("Parser sees run string as: %s\n", FFCore.scriptRunString);
	return function.getInternalScope()->getParent()->isScript()
		&& *function.returnType == DataType::ZVOID
		&& (!( strcmp(function.name.c_str(), FFCore.scriptRunString )))
		&& (!(function.getFlag(FUNCFLAG_INLINE))) ;
}

int32_t ZScript::getStackSize(Function const& function)
{
	return *lookupStackSize(*function.getInternalScope());
}

int32_t ZScript::getParameterCount(Function const& function)
{
	return function.paramTypes.size() + (isRun(function) ? 1 : 0);
}
