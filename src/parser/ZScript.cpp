#include "ZScript.h"

#include <sstream>
#include "CompilerUtils.h"
#include "CompileError.h"
#include "DataStructs.h"
#include "Types.h"
#include "Scope.h"
#include <fmt/ranges.h>


using namespace ZScript;
using std::vector;
using std::string;
using std::ostringstream;
using std::shared_ptr;

extern bool is_json_output;

////////////////////////////////////////////////////////////////
// ZScript::Program

Program::Program(ASTFile& root, CompileErrorHandler* errorHandler_)
	: rootScope_(new RootScope(typeStore_)), root_(root)
{
	// Create the ~Init script.
	if (Script* initScript =
	    	createScript(
				*this, *rootScope_, ParserScriptType::global,
				"~Init", errorHandler_))
	{
		scripts.push_back(initScript);
		scriptsByName_[initScript->getName()] = initScript;
	}

	errorHandler = errorHandler_;
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
	return functions;
}

////////////////////////////////////////////////////////////////
// ZScript::UserClass

UserClass::UserClass(Program& program, ASTClass& user_class)
	: classType(nullptr), program(program), node(user_class), parentClass(nullptr)
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
				CompileError::ClassRedef(node.identifier.get(), user_class->getName().c_str()));
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
	: name(namesp.getName())
{}

Namespace* ZScript::createNamespace(
		Program& program, Scope& parentScope, ASTNamespace& node,
		CompileErrorHandler* errorHandler)
{
	NamespaceScope* scope = parentScope.makeNamespaceChild(node);
	if (!scope) return nullptr;
	Namespace* namesp = scope->namesp;

	return namesp;
}

////////////////////////////////////////////////////////////////
// ZScript::Datum

Datum::Datum(Scope& scope, DataType const& type)
	: scope(scope), type(type), id(ScriptParser::getUniqueVarID()),
	erased(false)
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
	if(datum.is_erased())
		return nullopt;
	return lookupStackPosition(datum.scope, datum);
}
int32_t Datum::getStackOffset(bool i10k) const
{
	return (i10k ? 10000 : 1) * *ZScript::getStackOffset(*this);
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
		ASTExpr* init = node.getInitializer();
		if (auto expr_const = dynamic_cast<ASTExprVarInitializer*>(init))
			return expr_const->value;
		return std::nullopt;
	}
	return std::nullopt;
}

// ZScript::InternalVariable

InternalVariable* InternalVariable::create(
		Scope& scope, ASTDataDecl& node, DataType const& type,
		CompileErrorHandler* errorHandler)
{
	InternalVariable* variable = new InternalVariable(scope, node, type);
	if (variable->tryAddToScope(errorHandler)) return variable;
	delete variable;
	return NULL;
}

InternalVariable::InternalVariable(Scope& scope, ASTDataDecl& node, DataType const& type)
	: Datum(scope, type), readfn(nullptr), writefn(nullptr), node(node)
{
	node.manager = this;
}

// ZScript::UserClassVar

UserClassVar* UserClassVar::create(
		Scope& scope, ASTDataDecl& node, DataType const& type,
		CompileErrorHandler* errorHandler)
{
	UserClassVar* ucv = new UserClassVar(scope, node, type);
	if (ucv->tryAddToScope(errorHandler))
	{
		ucv->is_readonly = node.list->readonly;
		ucv->is_arr = !node.extraArrays.empty();

		if (node.list->internal)
		{
			ucv->is_internal = true;
			return ucv;
		}

		ClassScope* cscope = scope.getClass();
		UserClass& user_class = cscope->user_class;
		if(ucv->is_arr)
		{
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
	  is_internal(false), is_readonly(false), _index(0), node(node)
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

std::optional<string> Constant::getName() const {return node.getName();}


// ZScript::FunctionSignature

FunctionSignature::FunctionSignature(string const& name,
	vector<DataType const*> const& parameterTypes, DataType const* returnType )
	: name(name), prefix(false), destructor(false),
	parameterTypes(parameterTypes), returnType(returnType)
{}

FunctionSignature::FunctionSignature(Function const& function, bool useret)
	: name(function.name), prefix(function.hasPrefixType), destructor(function.getFlag(FUNCFLAG_DESTRUCTOR)),
	parameterTypes(function.paramTypes), returnType(useret ? function.returnType : nullptr)
{
	if (function.getClass())
		name = function.getClass()->getName() + "::" + name;
}
		
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
	if(destructor)
		oss << "~";
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
				   vector<DataType const*> paramTypes, vector<shared_ptr<const string>> paramNames, int32_t id,
				   int32_t flags, int32_t internal_flags, bool prototype, optional<int32_t> defaultReturn)
	: returnType(returnType), name(name), hasPrefixType(false), isFromTypeTemplate(false),
	  extra_vargs(0), paramTypes(paramTypes), paramNames(paramNames), numOptionalParams(), id(id),
	  node(NULL), data_decl_source_node(NULL), internalScope(NULL), externalScope(NULL), thisVar(NULL),
	  internal_flags(internal_flags), prototype(prototype),
	  defaultReturn(defaultReturn), label(std::nullopt), flags(flags),
	  aliased_func(nullptr), paramDatum(), templ_bound_ts()
{
	assert(returnType);
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
	if (!getExternalScope()) return NULL;
	Scope* parentScope = getExternalScope()->getParent();
	if (!parentScope) return NULL;
	if (!parentScope->isScript()) return NULL;
	ScriptScope* scriptScope =
		dynamic_cast<ScriptScope*>(parentScope);
	return &scriptScope->script;
}
UserClass* Function::getClass() const
{
	if (!getExternalScope()) return NULL;
	Scope* parentScope = getExternalScope()->getParent();
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
	return getFlag(FUNCFLAG_DEPRECATED);
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
			assert(paramTypes[q]->canCastTo(*func->paramTypes[q], func->internalScope));
		}
		assert(hasPrefixType == func->hasPrefixType);
		assert(returnType->canCastTo(*func->returnType, func->internalScope));
		//Ensure the function had no owned info of its' own
		assert(ownedCode.empty());
		assert(!(label || altlabel));
	}
}

static void type_replace(DataType const** ptr, DataType const& to_repl, DataType const& new_type)
{
	DataType const* new_const_ty = new_type.isConstant() ? &new_type : new_type.getConstType();
	DataType const* new_mut_ty = new_type.isConstant() ? new_type.getMutType() : &new_type;
	if (**ptr == to_repl)
	{
		if ((*ptr)->isConstant())
			*ptr = new_const_ty;
		else *ptr = new_mut_ty;
	}
	else
	{
		uint depth = 0;
		auto ty = *ptr;
		while(auto arrptr = dynamic_cast<DataTypeArray const*>(ty))
		{
			ty = &arrptr->getElementType();
			++depth;
			if(*ty == to_repl)
				break;
		}
		if (*ty == to_repl)
		{
			if(ty->isConstant())
				*ptr = DataTypeArray::create_depth(*new_const_ty, depth);
			else *ptr = DataTypeArray::create_depth(*new_mut_ty, depth);
		}
	}
}
Function* Function::apply_templ_func(vector<DataType const*> const& bound_ts)
{
	for(shared_ptr<Function> func : applied_funcs)
	{
		bool mismatch = false;
		if(bound_ts.size() != func->templ_bound_ts.size())
			continue;
		for(size_t q = 0; q < bound_ts.size(); ++q)
			if(*bound_ts[q] != *func->templ_bound_ts[q])
			{
				mismatch = true;
				break;
			}
		if(!mismatch)
			return func.get();
	}
	Scope *outside, *external;
	Function* templ = new Function(*this);
	bool skip = isTemplateSkip();
	if(skip)
	{
		outside = externalScope->getParent();
		external = outside->makeChild();
		templ->setExternalScope(external);
	}
	for(size_t q = 0; q < node->template_types.size(); ++q)
	{
		DataTypeTemplate* tmp_type = node->template_types[q].get();
		DataType const* new_type = bound_ts[q];
		
		//Typedef the template to the real type, for the internals of the func
		if(skip)
			external->addDataType(tmp_type->getName(), new_type, nullptr);
		//Replace template types in the signature
		type_replace(&templ->returnType, *tmp_type, *new_type);
		for(size_t q = 0; q < paramTypes.size(); ++q)
			type_replace(&templ->paramTypes[q], *tmp_type, *new_type);
	}
	templ->isFromTypeTemplate = true;
	templ->templ_bound_ts = bound_ts;
	if(!skip)
	{
		templ->aliased_func = this;
	}
	else
	{
		templ->setInternalScope(external->makeFunctionChild(*templ));
		templ->node = templ->node->clone();
		templ->node->parentScope = external;
		templ->node->func = templ;
		templ->node->parameters = templ->node->param_template;
		for(size_t q = 0; q < paramTypes.size(); ++q)
			templ->node->parameters[q]->resolvedType = templ->paramTypes[q];
	}
	applied_funcs.emplace_back(templ);
	return templ;
}

bool ZScript::isRun(Function const& function)
{
	return function.getExternalScope()->getParent()->isScript()
		&& *function.returnType == DataType::ZVOID
		&& (!( strcmp(function.name.c_str(), "run" )))
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

bool ZScript::is_test()
{
	static bool state = std::getenv("TEST_ZSCRIPT") != nullptr;
	return state;
}

struct cache_entry
{
	std::string contents;
	std::vector<uint32_t> lines;
};
static std::map<std::string, cache_entry> sourceContentsCache;
static const cache_entry* getSourceCodeCacheEntry(const std::string& fname)
{
	cache_entry* entry;
	auto it = sourceContentsCache.find(fname);
	if (it != sourceContentsCache.end())
		return &it->second;

	sourceContentsCache[fname] = {util::read_text_file(fname), {}};
	entry = &sourceContentsCache[fname];

	uint32_t count = 0;
	std::vector<std::string> lines = util::split(entry->contents, "\n");
	for (auto& line : lines)
	{
		entry->lines.push_back(count);
		count += line.size() + 1;
	}
	entry->lines.push_back(count);

	return entry;
}

int ZScript::getSourceCodeNumLines(const LocationData& loc)
{
	return getSourceCodeCacheEntry(loc.fname)->lines.size();
}

std::string ZScript::getSourceCodeSnippet(const LocationData& loc)
{
	auto entry = getSourceCodeCacheEntry(loc.fname);
	uint32_t start = entry->lines[loc.first_line - 1] + loc.first_column - 1;
	uint32_t end = loc.last_line - 1 < entry->lines.size() ?
		entry->lines[loc.last_line - 1] + loc.last_column - 1 :
		entry->contents.size();
	return entry->contents.substr(start, end - start);
}

std::string ZScript::getErrorContext(const LocationData& loc)
{
	// Don't print context for IDE usages (ex: vscode extension), too much work and is not needed.
	bool should_print_context = !is_json_output || is_test();
	if (!should_print_context)
		return "";

	int num_lines_context = 2;
	int num_lines_prev = std::min(loc.first_line - 1, num_lines_context);

	LocationData expanded_loc = loc;
	expanded_loc.first_line -= num_lines_prev;
	expanded_loc.last_line = std::min(expanded_loc.last_line, getSourceCodeNumLines(loc)) + 1;
	expanded_loc.first_column = 1;
	expanded_loc.last_column = 1;

	std::string context = getSourceCodeSnippet(expanded_loc);
	if (context.empty())
		return "";

	auto lines = util::split(context, "\n");

	// Add underline. Example:
	//    auto[][] numbers6 = {1, 2, 3};
	//             ^~~~~~~~
	bool show_underline = loc.first_line == loc.last_line;
	if (show_underline)
	{
		// Increase width by "num of leading tabs * 3" of target line.
		int prefix_w = loc.first_column - 1;
		const std::string& target_line = lines[num_lines_prev];
		for (int i = 0; i < loc.first_column; i++)
		{
			char c = target_line[i];
			if (c == '\t') prefix_w += 3;
			else if (!isspace(c)) break;
		}

		std::string underline =
			fmt::format("{}{:~<{}}", std::string(prefix_w, ' '), "^", loc.last_column - loc.first_column);
		lines.push_back(underline);
	}

	for (auto& line : lines)
		util::replstr(line, "\t", "    ");

	// Trim leading whitespace.
	int min_ws = INT_MAX;
	for (const auto& line : lines)
	{
		int ws = 0;
		for (const char& c : line)
		{
			if (c == ' ')
				ws++;
			else break;
		}
		min_ws = std::min(min_ws, ws);
	}
	if (min_ws > 0)
	{
		for (auto& line : lines)
			line = line.substr(min_ws);
	}

	// Add line numbers.
	int prefix_w = fmt::format("{}", expanded_loc.last_line).size();
	int line_num = expanded_loc.first_line;
	bool ignore_next = false;
	for (auto& line : lines)
	{
		if (ignore_next)
		{
			line = fmt::format("{: >{}}    {}", "", prefix_w, line);
			ignore_next = false;
			continue;
		}

		line = fmt::format("{: >{}}    {}", line_num, prefix_w, line);
		ignore_next = show_underline && line_num == loc.first_line;
		line_num += 1;
	}

	return fmt::format("\n\n{}\n\n", fmt::join(lines, "\n"));
}
