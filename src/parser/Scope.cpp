#include "parserDefs.h"
#include "Scope.h"

#include <set>
#include "CompileError.h"
#include "LibrarySymbols.h"
#include "Types.h"
#include "ZScript.h"
#include <sstream>

using namespace ZScript;
using namespace util;
using std::set;
using std::unique_ptr;
////////////////////////////////////////////////////////////////
// Scope

Scope::Scope(TypeStore& typeStore)
	: typeStore_(typeStore), name_(std::nullopt)
{
	id = ScopeID++;
}

Scope::Scope(TypeStore& typeStore, string const& name)
	: typeStore_(typeStore), name_(name)
{
	id = ScopeID++;
}

void Scope::invalidateStackSize()
{
	if (Scope* parent = getParent())
		parent->invalidateStackSize();
}

// Inheritance

Scope* ZScript::getDescendant(Scope const& scope, vector<string> const& names, vector<string> const& delimiters)
{
	Scope* child = const_cast<Scope*>(&scope);
	vector<string>::const_iterator del = delimiters.begin();
	for (vector<string>::const_iterator it = names.begin();
	     child && it != names.end(); ++it)
	{
		child = child->getChild(*it);
		string str = *del;
		//Only allow `.` for scripts, and `::` for namespaces, when delimiting scope. -V
		if(!child || (child->isScript() && str.compare(".")) || (child->isNamespace() && str.compare("::")))
			return NULL;
		++del;
	}
	return child;
}

Scope* ZScript::lookupScope(Scope const& scope, string const& name, bool noUsing, AST& host, CompileErrorHandler* errorHandler)
{
	Scope* current = const_cast<Scope*>(&scope);
	Scope* first = current;
	Scope* found = NULL;
	while (current && !found)
	{
		if (Scope* child = current->getChild(name))
		{
			if(current->isFile() || current->isRoot()) //Only continue if the scope was file/root
				found = child;
			else return child;
		}

		current = current->getParent();
	}
	if(!noUsing)//handle Using Namespaces
	{
		vector<NamespaceScope*> namespaces = lookupUsingNamespaces(*first);
		for(vector<NamespaceScope*>::iterator it = namespaces.begin();
			it != namespaces.end(); ++it)
		{
			Scope* usingscope = *it;
			if (Scope* child = usingscope->getChild(name))
			{
				if(found && found != child)
					errorHandler->handleError(CompileError::TooManyVar(&host, name));

				found = child;
			}
		}
	}
	return found;
}

Scope* ZScript::lookupScope(Scope const& scope, vector<string> const& names, vector<string> const& delimiters, bool noUsing, AST& host, CompileErrorHandler* errorHandler)
{
	// Travel as far up the tree as needed for the first scope.
	Scope* current = lookupScope(scope, names.front(), noUsing, host, errorHandler);
	if (!current) return NULL;
	//string str = ;
	if(current->isScript() && delimiters.front().compare(".") || current->isNamespace() && delimiters.front().compare("::"))
			return NULL;
	// Descend with the rest of the names from the first found scope.
	vector<string> restOfNames(++names.begin(), names.end());
	vector<string> restOfDelimiters(++delimiters.begin(), delimiters.end());
	return getDescendant(*current, restOfNames, restOfDelimiters);
}

vector<Scope*> ZScript::lookupScopes(Scope const& scope, vector<string> const& names, vector<std::string> const& delimiters, bool noUsing)
{
	vector<Scope*> scopes;
	for (Scope* current = const_cast<Scope*>(&scope);
	     current; current = current->getParent())
	{
		if(current->isRoot() && scopes.size() != 0)
			break;
		if (Scope* descendant = getDescendant(*current, names, delimiters))
			scopes.push_back(descendant);
	}
	if(!noUsing)
	{
		vector<NamespaceScope*> namespaces = lookupUsingNamespaces(scope);
		for(vector<NamespaceScope*>::iterator it = namespaces.begin();
			it != namespaces.end(); ++it)
		{
			if (Scope* descendant = getDescendant(**it, names, delimiters))
				scopes.push_back(descendant);
		}
	}
	return scopes;
}

vector<Scope*> ZScript::lookupUsingScopes(Scope const& scope, vector<string> const& names, vector<std::string> const& delimiters)
{
	vector<Scope*> scopes;
	vector<NamespaceScope*> namespaces = lookupUsingNamespaces(scope);
	for(vector<NamespaceScope*>::iterator it = namespaces.begin();
		it != namespaces.end(); ++it)
	{
		if (Scope* descendant = getDescendant(**it, names, delimiters))
			scopes.push_back(descendant);
	}
	return scopes;
}

RootScope* ZScript::getRoot(Scope const& scope)
{
	Scope* current = const_cast<Scope*>(&scope);
	while (Scope* parent = current->getParent())
		current = parent;
	return dynamic_cast<RootScope*>(current);
}

// Lookup

DataType const* ZScript::lookupDataType(
	Scope const& scope, string const& name, ASTExprIdentifier& host, CompileErrorHandler* errorHandler, bool isTypedefCheck, bool forceSkipUsing)
{
	DataType const* type = NULL;
	Scope const* current = &scope;
	for (; current; current = current->getParent())
	{
		DataType const* temp = current->getLocalDataType(name);
		if(!type)
		{
			//Only continue if this var was found at the file scope or higher.
			if(current->isFile() || current->isRoot())
				type = temp;
			else if(temp)
				return temp;
		}
		else if(temp)
		{
			//Don't give a duplication warning on RootScope! -V
			if(current->isRoot())
				break;
			if(type->compare(*temp))
			{
				if(!isTypedefCheck)errorHandler->handleError(CompileError::TooManyType(&host, name));
				return NULL;
			}
		}
		if(isTypedefCheck && !current->isFile()) return type; //Only check current scope, and root for file, for typedefs
	}
	if(host.noUsing || forceSkipUsing || isTypedefCheck) return type; //End early
	vector<NamespaceScope*> namespaces = lookupUsingNamespaces(scope);
	for(vector<NamespaceScope*>::iterator it = namespaces.begin();
		it != namespaces.end(); ++it)
	{
		NamespaceScope* nsscope = *it;
		DataType const* temp = nsscope->getLocalDataType(name);
		if(!type)
			type = temp;
		else if(temp)
		{
			if(type->compare(*temp))
			{
				errorHandler->handleError(CompileError::TooManyType(&host, name));
				return NULL;
			}
		}
	}
	return type;
}

DataType const* ZScript::lookupDataType(
	Scope const& scope, ASTExprIdentifier& host, CompileErrorHandler* errorHandler, bool isTypedefCheck)
{
	vector<string> names = host.components;
	if (names.size() == 0)
		return NULL;
	else if (names.size() == 1)
		return lookupDataType(scope, names[0], host, errorHandler, isTypedefCheck);
	vector<string> childNames(names.begin(), --names.end());
	if (Scope* child = lookupScope(scope, childNames, host.delimiters, host.noUsing, host, errorHandler))
		return lookupDataType(*child, names.back(), host, errorHandler, isTypedefCheck, true); //lookupScope() handles UsingNamespaces; don't allow using to occur again! -V

	return NULL;
}

ParserScriptType ZScript::lookupScriptType(Scope const& scope, string const& name)
{
	for (Scope const* current = &scope;
	     current; current = current->getParent())
		if (std::optional<ParserScriptType> type = current->getLocalScriptType(name))
			return *type;
	return ParserScriptType::invalid;
}

ZClass* ZScript::lookupClass(Scope const& scope, string const& name)
{
	for (Scope const* current = &scope;
	     current; current = current->getParent())
		if (ZClass* klass = current->getLocalClass(name))
			return klass;
	return NULL;
}

Datum* ZScript::lookupDatum(Scope& scope, std::string const& name, ASTExprIdentifier& host, CompileErrorHandler* errorHandler, bool forceSkipUsing)
{
	Datum* datum = NULL;
	Scope const* current = &scope;
	for (; current; current = current->getParent())
	{
		Datum* temp = current->getLocalDatum(name);
		if(!datum)
		{
			//Only continue if this var was found at the file scope or higher.
			if(current->isFile() || current->isRoot())
				datum = temp;
			else if(temp)
				return temp;
		}
		else if(temp)
		{
			//Don't give a duplication warning on RootScope! -V
			if(current->isRoot())
				break;
			if(&datum != &temp)
				errorHandler->handleError(CompileError::TooManyVar(&host, name));
		}
	}
	if(host.noUsing || forceSkipUsing) return datum; //End early
	vector<NamespaceScope*> namespaces = lookupUsingNamespaces(scope);
	for(vector<NamespaceScope*>::iterator it = namespaces.begin();
		it != namespaces.end(); ++it)
	{
		NamespaceScope* nsscope = *it;
		Datum* temp = nsscope->getLocalDatum(name);
		if(!datum)
			datum = temp;
		else if(temp)
		{
			if(&datum != &temp)
				errorHandler->handleError(CompileError::TooManyVar(&host, name));
		}
	}
	return datum;
}

Datum* ZScript::lookupDatum(Scope& scope, ASTExprIdentifier& host, CompileErrorHandler* errorHandler)
{
	vector<string> names = host.components;
	if (names.size() == 0)
		return NULL;
	else if (names.size() == 1)
		return lookupDatum(scope, names[0], host, errorHandler);
	vector<string> childNames(names.begin(), --names.end());
	if (Scope* child = lookupScope(scope, childNames, host.delimiters, host.noUsing, host, errorHandler))
		return lookupDatum(*child, names.back(), host, errorHandler, true); //lookupScope() handles UsingNamespaces; don't allow using to occur again! -V

	return NULL;
}

UserClassVar* ZScript::lookupClassVars(Scope& scope, ASTExprIdentifier& host, CompileErrorHandler* errorHandler)
{
	vector<string> names = host.components;
	if (names.size() != 1)
		return nullptr;
	ClassScope* cscope = scope.getClass();
	if(!cscope) return nullptr;
	return cscope->getClassVar(names[0]);
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

/* Nothing calls this. Commenting it out so it can stay for reference. Not updating it, though. -V
Function* ZScript::lookupFunction(Scope const& scope,
                         FunctionSignature const& signature)
{
	for (Scope const* current = &scope;
	     current; current = current->getParent())
		if (Function* function = current->getLocalFunction(signature))
			return function;
	return NULL;
}*/

vector<Function*> ZScript::lookupFunctions(Scope& scope, string const& name, vector<DataType const*> const& parameterTypes, bool noUsing, bool isClass)
{
	set<Function*> functions;
	Scope const* current = &scope;
	bool foundFile = false;
	//Standard lookup loop
	for (; current; current = current->getParent())
	{
		if((current->isFile() || current->isRoot()) && !foundFile)
		{
			if(!functions.empty())
				noUsing = true;
		}
		if(current->isFile())
			foundFile = true;
		vector<Function*> currentFunctions = current->getLocalFunctions(name);
		trimBadFunctions(currentFunctions, parameterTypes, !isClass);
		functions.insert(currentFunctions.begin(), currentFunctions.end());
	}
	if(!noUsing)
	{
		vector<NamespaceScope*> namespaces = lookupUsingNamespaces(scope);
		for(vector<NamespaceScope*>::iterator it = namespaces.begin();
			it != namespaces.end(); ++it)
		{
			NamespaceScope* nsscope = *it;
			vector<Function*> currentFunctions = nsscope->getLocalFunctions(name);
			trimBadFunctions(currentFunctions, parameterTypes, !isClass);
			functions.insert(currentFunctions.begin(), currentFunctions.end());
		}
		current = &scope;
	}
	return vector<Function*>(functions.begin(), functions.end());
}

vector<Function*> ZScript::lookupFunctions(
		Scope& scope, vector<string> const& names, vector<string> const& delimiters, vector<DataType const*> const& parameterTypes, bool noUsing, bool isClass)
{
	if (names.size() == 0)
		return vector<Function*>();
	else if (names.size() == 1)
		return lookupFunctions(scope, names[0], parameterTypes, noUsing, isClass);

	vector<Function*> functions;
	string const& name = names.back();

	vector<string> ancestry(names.begin(), --names.end());
	vector<Scope*> scopes = lookupScopes(scope, ancestry, delimiters, true); //Don't include using scopes
	vector<Scope*> usingScopes = lookupUsingScopes(scope, ancestry, delimiters); //get ONLY using scopes
	//Check all non-using scopes for valid function matches
	bool foundFile = false;
	for (vector<Scope*>::const_iterator it = scopes.begin();
	     it != scopes.end(); ++it)
	{
		Scope& current = **it;
		if(current.isFile() || (current.isRoot() && !foundFile))
		{
			if(!functions.empty()) noUsing = true; //If there are local matches, don't check using
		}
		if(current.isFile()) foundFile = true;
		vector<Function*> currentFunctions = current.getLocalFunctions(name);
		trimBadFunctions(currentFunctions, parameterTypes, !isClass);
		functions.insert(functions.end(),
		                 currentFunctions.begin(), currentFunctions.end());
	}
	if(!noUsing)
	{
		//Check using functions
		for (vector<Scope*>::const_iterator it = usingScopes.begin();
			 it != usingScopes.end(); ++it)
		{
			Scope& current = **it;
			vector<Function*> currentFunctions = current.getLocalFunctions(name);
			trimBadFunctions(currentFunctions, parameterTypes, !isClass);
			functions.insert(functions.end(),
							 currentFunctions.begin(), currentFunctions.end());
		}
	}

	return functions;
}

UserClass* ZScript::lookupClass(Scope& scope, string const& name, bool noUsing)
{
	set<Function*> functions;
	Scope const* current = &scope;
	bool foundFile = false;
	ClassScope* cscope = nullptr;
	Scope* tscope = nullptr;
	//Standard lookup loop
	for (; current; current = current->getParent())
	{
		tscope = current->getChild(name);
		if(tscope && tscope->isClass())
		{
			cscope = static_cast<ClassScope*>(tscope);
			return &cscope->user_class;
		}
	}
	if(!noUsing)
	{
		vector<NamespaceScope*> namespaces = lookupUsingNamespaces(scope);
		for(vector<NamespaceScope*>::iterator it = namespaces.begin();
			it != namespaces.end(); ++it)
		{
			NamespaceScope* nsscope = *it;
			tscope = nsscope->getChild(name);
			if(tscope && tscope->isClass())
			{
				cscope = static_cast<ClassScope*>(tscope);
				return &cscope->user_class;
			}
		}
		current = &scope;
	}
	return nullptr;
}

UserClass* ZScript::lookupClass(Scope& scope, vector<string> const& names,
	vector<string> const& delimiters, bool noUsing)
{
	if (names.size() == 0)
		return nullptr;
	else if (names.size() == 1)
		return lookupClass(scope, names[0], noUsing);
	
	string const& name = names.back();
	vector<string> ancestry(names.begin(), --names.end());
	vector<Scope*> scopes = lookupScopes(scope, ancestry, delimiters, true); //Don't include using scopes
	vector<Scope*> usingScopes = lookupUsingScopes(scope, ancestry, delimiters); //get ONLY using scopes
	//Check all non-using scopes for valid function matches
	bool foundFile = false;
	ClassScope* cscope = nullptr;
	Scope* tscope = nullptr;
	for (vector<Scope*>::const_iterator it = scopes.begin();
	     it != scopes.end(); ++it)
	{
		Scope& current = **it;
		
		tscope = current.getChild(name);
		if(tscope && tscope->isClass())
		{
			cscope = static_cast<ClassScope*>(tscope);
			return &cscope->user_class;
		}
	}
	if(!noUsing)
	{
		//Check using functions
		for (vector<Scope*>::const_iterator it = usingScopes.begin();
			 it != usingScopes.end(); ++it)
		{
			Scope& current = **it;
			
			tscope = current.getChild(name);
			if(tscope && tscope->isClass())
			{
				cscope = static_cast<ClassScope*>(tscope);
				return &cscope->user_class;
			}
		}
	}

	return nullptr;
}

vector<Function*> ZScript::lookupConstructors(UserClass const& user_class, vector<DataType const*> const& parameterTypes)
{
	vector<Function*> functions = user_class.getScope().getConstructors();
	trimBadFunctions(functions, parameterTypes, false);
	return functions;
}
vector<Function*> ZScript::lookupClassFuncs(UserClass const& user_class,
	std::string const& name, vector<DataType const*> const& parameterTypes)
{
	vector<Function*> functions = user_class.getScope().getLocalFunctions(name);
	trimBadFunctions(functions, parameterTypes, false);
	for (vector<Function*>::iterator it = functions.begin();
		 it != functions.end();)
	{
		Function& function = **it;
		if(function.getFlag(FUNCFLAG_STATIC))
		{
			it = functions.erase(it);
			continue;
		}
		else ++it;
	}
	return functions;
}

inline void ZScript::trimBadFunctions(std::vector<Function*>& functions, std::vector<DataType const*> const& parameterTypes, bool trimClasses)
{
	// Filter out invalid functions.
	for (vector<Function*>::iterator it = functions.begin();
		 it != functions.end();)
	{
		Function& function = **it;
		if(trimClasses && function.internalScope->getClass() && !function.getFlag(FUNCFLAG_STATIC))
		{
			it = functions.erase(it);
			continue;
		}
		bool vargs = function.getFlag(FUNCFLAG_VARARGS);
		
		auto targetSize = parameterTypes.size();
		auto maxSize = function.paramTypes.size();
		auto minSize = maxSize - function.opt_vals.size();
		// Match against parameter count, including std::optional params.
		if (minSize > targetSize || (!vargs && maxSize < targetSize))
		{
			it = functions.erase(it);
			continue;
		}
		auto lowsize = zc_min(maxSize, parameterTypes.size());
		// Check parameter types.
		bool parametersMatch = true;
		if(function.getFlag(FUNCFLAG_NOCAST)) //no casting params
		{
			Scope* scope = function.internalScope;
			for (size_t i = 0; i < lowsize; ++i)
			{
				if (getNaiveType(*parameterTypes[i],scope)
					!= getNaiveType(*function.paramTypes[i],scope))
				{
					parametersMatch = false;
					break;
				}
			}
		}
		else
		{
			for (size_t i = 0; i < lowsize; ++i)
			{
				if (!parameterTypes[i]->canCastTo(*function.paramTypes[i]))
				{
					parametersMatch = false;
					break;
				}
			}
		}
		if (!parametersMatch)
		{
			it = functions.erase(it);
			continue;
		}

		// Keep function.
		++it;
	}
}

std::optional<int32_t> ZScript::lookupOption(Scope const& scope, CompileOption option)
{
	if (!option.isValid()) return std::nullopt;
	for (Scope const* current = &scope;
	     current; current = current->getParent())
	{
		CompileOptionSetting setting = current->getLocalOption(option);
		if (setting == CompileOptionSetting::Inherit) continue;
		if (setting == CompileOptionSetting::Default)
			return *option.getDefault();
		return *setting.getValue();
	}
	return *option.getDefault();
}

vector<NamespaceScope*> ZScript::lookupUsingNamespaces(Scope const& scope)
{
	Scope* first = const_cast<Scope*>(&scope);
	Scope* current = first;
	set<NamespaceScope*> namespaceSet;
	bool foundFile = false;
	for (; current; current = current->getParent())
	{
		if(current->isRoot()) break; //Don't check RootScope here!
		vector<NamespaceScope*> currentNamespaces = current->getUsingNamespaces();
		namespaceSet.insert(currentNamespaces.begin(), currentNamespaces.end());
		if(current->isFile())
		{
			foundFile = true;
			break; //Don't go to parent file!
		}
	}
	if(!foundFile && !first->isRoot()) //Get the file this is in, if it was not found through the looping. (i.e. this is within a namespace) - Also, don't get this for root. That crashes. -V
	{
		vector<NamespaceScope*> currentNamespaces = first->getFile()->getUsingNamespaces();
		namespaceSet.insert(currentNamespaces.begin(), currentNamespaces.end());
	}
	//Get `always using` things
	vector<NamespaceScope*> currentNamespaces = getRoot(*first)->getUsingNamespaces();
	namespaceSet.insert(currentNamespaces.begin(), currentNamespaces.end());
	vector<NamespaceScope*> result(namespaceSet.begin(), namespaceSet.end());
	return result;
}

// Stack

bool ZScript::isStackRoot(Scope const& scope)
{
	return scope.getRootStackSize().has_value();
}

std::optional<int32_t> ZScript::lookupStackOffset(
		Scope const& scope, Datum const& datum)
{
	Scope* s = const_cast<Scope*>(&scope);
	while (s)
	{
		if (std::optional<int32_t> offset = s->getLocalStackOffset(datum))
			return offset;

		if (isStackRoot(*s)) return std::nullopt;

		s = s->getParent();
	}
	return std::nullopt;
}

std::optional<int32_t> ZScript::lookupStackSize(Scope const& scope)
{
	Scope* s = const_cast<Scope*>(&scope);
	while (s)
	{
		if (std::optional<int32_t> size = s->getRootStackSize())
			return size;

		s = s->getParent();
	}
	return std::nullopt;
}

std::optional<int32_t> ZScript::lookupStackPosition(
		Scope const& scope, Datum const& datum)
{
	std::optional<int32_t> offset = lookupStackOffset(scope, datum);
	std::optional<int32_t> size = lookupStackSize(scope);
	if (offset && size)
		return *size - 1 - *offset;
	return std::nullopt;
}

// Get all in branch

vector<Function*> ZScript::getFunctionsInBranch(Scope const& scope)
{
	typedef vector<Function*> (Scope::*Call)() const;
	Call call = static_cast<Call>(&Scope::getLocalFunctions);
	return getInBranch<Function*>(scope, call, true);
}

////////////////////////////////////////////////////////////////
// Basic Scope

BasicScope::BasicScope(Scope* parent, FileScope* parentFile)
	: Scope(parent->getTypeStore()), parent_(parent), parentFile_(parentFile),
	  stackDepth_(parent->getLocalStackDepth()),
	  defaultOption_(CompileOptionSetting::Inherit)
{}

BasicScope::BasicScope(Scope* parent, FileScope* parentFile, string const& name)
	: Scope(parent->getTypeStore(), name), parent_(parent), parentFile_(parentFile),
	  stackDepth_(parent->getLocalStackDepth()),
	  defaultOption_(CompileOptionSetting::Inherit)
{}

BasicScope::BasicScope(TypeStore& typeStore)
	: Scope(typeStore), parent_(NULL), stackDepth_(0),
	  defaultOption_(CompileOptionSetting::Inherit)
{}

BasicScope::BasicScope(TypeStore& typeStore, string const& name)
	: Scope(typeStore, name), parent_(NULL), stackDepth_(0),
	  defaultOption_(CompileOptionSetting::Inherit)
{}

BasicScope::~BasicScope()
{
	deleteSeconds(children_);
	deleteElements(anonymousChildren_);
	deleteElements(anonymousData_);
	deleteSeconds(namedData_);
	deleteSeconds(getters_);
	deleteSeconds(setters_);
	deleteSeconds(functionsBySignature_);
}

// Inheritance

Scope* BasicScope::getChild(string const& name) const
{
	return find<Scope*>(children_, name).value_or(std::add_pointer<Scope>::type());
}

vector<Scope*> BasicScope::getChildren() const
{
	vector<Scope*> results = anonymousChildren_;
	appendElements(results, getSeconds<Scope*>(children_));
	return results;
}

ScriptScope* BasicScope::getScript()
{
	if(isScript())
	{
		Scope* temp = this;
		return static_cast<ScriptScope*>(temp);
	}
	for(Scope* parent = getParent(); parent; parent = parent->getParent())
	{
		if(parent->isScript()) return dynamic_cast<ScriptScope*>(parent);
	}
	return NULL;
}

ClassScope* BasicScope::getClass()
{
	if(isClass())
	{
		Scope* temp = this;
		return static_cast<ClassScope*>(temp);
	}
	for(Scope* parent = getParent(); parent; parent = parent->getParent())
	{
		if(parent->isClass()) return dynamic_cast<ClassScope*>(parent);
	}
	return NULL;
}

int32_t BasicScope::useNamespace(vector<std::string> names, vector<std::string> delimiters, bool noUsing)
{
	if (names.size() == 1)
		return useNamespace(names[0], noUsing);
	NamespaceScope* namesp = NULL;
	int32_t numMatches = 0;

	string const& name = names.back();

	vector<string> ancestry(names.begin(), --names.end());
	vector<Scope*> scopes = lookupScopes(*this, ancestry, delimiters, noUsing); //lookupScopes handles usingNamespaces!
	for (vector<Scope*>::const_iterator it = scopes.begin();
	     it != scopes.end(); ++it)
	{
		Scope& current = **it;
		Scope* tmp = current.getChild(name);
		if(!tmp || !tmp->isNamespace()) continue;
		namesp = static_cast<NamespaceScope*>(tmp);
		++numMatches;
	}
	//
	if(namesp)
	{
		vector<NamespaceScope*> namespaces = lookupUsingNamespaces(*this);
		for(vector<NamespaceScope*>::iterator it = namespaces.begin();
			it != namespaces.end(); ++it)
		{
			if(*it == namesp) return -1; //Already using this namespace! -V
		}
	}
	if(!namesp) return 0;
	if(numMatches == 1)
		usingNamespaces.push_back(namesp);
	return numMatches;
}

int32_t BasicScope::useNamespace(std::string name, bool noUsing)
{
	NamespaceScope* namesp = NULL;
	int32_t numMatches = 0;
	if(Scope* scope = getRoot(*this)->getChild(name))
	{
		if(scope->isNamespace())
		{
			namesp = static_cast<NamespaceScope*>(scope);
			++numMatches;
		}
	}
	if(!isFile() && !isRoot())
	{
		if(Scope* scope = getChild(name))
		{
			if(scope->isNamespace())
			{
				namesp = static_cast<NamespaceScope*>(scope);
				++numMatches;
			}
		}
	}
	if(!noUsing)
	{
		vector<NamespaceScope*> namespaces = lookupUsingNamespaces(*this); //Lookup parent using namespaces as well! -V
		for(vector<NamespaceScope*>::iterator it = namespaces.begin();
			it != namespaces.end(); ++it)
		{
			NamespaceScope* scope = *it;
			if(namesp)
			{
				//Return -1 for duplicate namespaces
				if(&scope == &namesp) return -1;
			}
			if(Scope* scope2 = scope->getChild(name))
			{
				if(scope2->isNamespace())
				{
					namesp = static_cast<NamespaceScope*>(scope2);
					++numMatches;
				}
			}
		}
	}
	if(!numMatches) return 0;
	if(numMatches == 1)
		usingNamespaces.push_back(namesp);
	return numMatches;
}

// Lookup Local

DataType const* BasicScope::getLocalDataType(string const& name) const
{
	return find<DataType const*>(dataTypes_, name).value_or(std::add_pointer<DataType const>::type());
}

std::optional<ParserScriptType> BasicScope::getLocalScriptType(string const& name) const
{
	return find<ParserScriptType>(scriptTypes_, name);
}

ZClass* BasicScope::getLocalClass(string const& name) const
{
	return find<ZClass*>(classes_, name).value_or(std::add_pointer<ZClass>::type());
}

Datum* BasicScope::getLocalDatum(string const& name) const
{
	return find<Datum*>(namedData_, name).value_or(std::add_pointer<Datum>::type());
}

Function* BasicScope::getLocalGetter(string const& name) const
{
	return find<Function*>(getters_, name).value_or(std::add_pointer<Function>::type());
}

Function* BasicScope::getLocalSetter(string const& name) const
{
	return find<Function*>(setters_, name).value_or(std::add_pointer<Function>::type());
}

Function* BasicScope::getLocalFunction(
		FunctionSignature const& signature) const
{
	return find<Function*>(functionsBySignature_, signature).value_or(std::add_pointer<Function>::type());
}

vector<Function*> BasicScope::getLocalFunctions(string const& name) const
{
	return find<vector<Function*> >(functionsByName_, name)
		.value_or(vector<Function*>());
}

CompileOptionSetting BasicScope::getLocalOption(CompileOption option) const
{
	if (std::optional<CompileOptionSetting> setting =
	    	find<CompileOptionSetting>(options_, option))
		return *setting;
	return defaultOption_;
}

// Get All Local

vector<Datum*> BasicScope::getLocalData() const
{
	vector<Datum*> results = getSeconds<Datum*>(namedData_);
	appendElements(results, anonymousData_);
	return results;
}

vector<Function*> BasicScope::getLocalFunctions() const
{
	return getSeconds<Function*>(functionsBySignature_);
}

vector<Function*> BasicScope::getLocalGetters() const
{
	return getSeconds<Function*>(getters_);
}

vector<Function*> BasicScope::getLocalSetters() const
{
	return getSeconds<Function*>(setters_);
}

map<CompileOption, CompileOptionSetting> BasicScope::getLocalOptions() const
{
	return options_;
}

// Add

Scope* BasicScope::makeChild()
{
	Scope* child = new BasicScope(this, getFile());
	anonymousChildren_.push_back(child);
	return child;
}

Scope* BasicScope::makeChild(string const& name)
{
	if (find<Scope*>(children_, name)) return NULL;
	Scope* child = new BasicScope(this, getFile(), name);
	children_[name] = child;
	return child;
}

FileScope* BasicScope::makeFileChild(string const& filename)
{
	FileScope* child = new FileScope(this, filename);
	//parentFile_ for a FileScope should be the FileScope itself, but this cannot be done within the constructor.
	//Thus, `setFile()` must be called AFTER the constructor, to set this correctly. Not doing so causes invalid data, and crashes. -V
	child->setFile();
	anonymousChildren_.push_back(child);
	return child;
}

ScriptScope* BasicScope::makeScriptChild(Script& script)
{
	string name = script.getName();
	if (find<Scope*>(children_, name)) return NULL;
	ScriptScope* child = new ScriptScope(this, getFile(), script);
	children_[name] = child;
	return child;
}

ClassScope* BasicScope::makeClassChild(UserClass& user_class)
{
	string name = user_class.getName();
	if (find<Scope*>(children_, name)) return NULL;
	ClassScope* child = new ClassScope(this, getFile(), user_class);
	children_[name] = child;
	return child;
}

NamespaceScope* BasicScope::makeNamespaceChild(ASTNamespace& node)
{
	string name = node.name;
	if (Scope* scope = getChild(name))
	{
		if(scope->isNamespace()) return static_cast<NamespaceScope*>(scope);
		else return NULL;
	}
	Namespace* namesp = new Namespace(node);
	NamespaceScope* result = new NamespaceScope(this, getFile(), namesp);
	namesp->setScope(result);
	children_[name] = result;
	return result;
}

FunctionScope* BasicScope::makeFunctionChild(Function& function)
{
	FunctionScope* child = new FunctionScope(this, getFile(), function);
	anonymousChildren_.push_back(child);
	return child;
}

DataType const* BasicScope::addDataType(
		string const& name, DataType const* type, AST* node)
{
	if (find<DataType const*>(dataTypes_, name)) return NULL;
	type = typeStore_.getCanonicalType(*type);
	dataTypes_[name] = type;
	return type;
}

bool BasicScope::addScriptType(
	string const& name, ParserScriptType type, AST* node)
{
	if (find<ParserScriptType>(scriptTypes_, name)) return false;
	scriptTypes_[name] = type;
	return true;
}

Function* BasicScope::addGetter(
		DataType const* returnType, string const& name,
		vector<DataType const*> const& paramTypes, vector<string const*> const& paramNames, int32_t flags, AST* node)
{
	if (find<Function*>(getters_, name)) return NULL;

	Function* fun = new Function(
			returnType, name, paramTypes, paramNames, ScriptParser::getUniqueFuncID(), flags);
	getters_[name] = fun;
	return fun;
}

Function* BasicScope::addSetter(
		DataType const* returnType, string const& name,
		vector<DataType const*> const& paramTypes, vector<string const*> const& paramNames, int32_t flags, AST* node)
{
	if (find<Function*>(setters_, name)) return NULL;

	Function* fun = new Function(
			returnType, name, paramTypes, paramNames, ScriptParser::getUniqueFuncID(), flags);
	setters_[name] = fun;
	return fun;
}

Function* BasicScope::addFunction(
		DataType const* returnType, string const& name,
		vector<DataType const*> const& paramTypes, vector<string const*> const& paramNames, int32_t flags, ASTFuncDecl* node, CompileErrorHandler* handler)
{
	bool prototype = false;
	ASTExprConst* defRet = NULL;
	if(node)
	{
		prototype = node->prototype;
		if(prototype)
		{
			defRet = node->defaultReturn.get();
		}
	}
	FunctionSignature signature(name, paramTypes);
	Function* foundFunc = NULL;
	std::optional<Function*> optFunc = find<Function*>(functionsBySignature_, signature);
	if(optFunc)
		foundFunc = *optFunc;
	else if(isFile() || isRoot())
	{
		std::optional<Function*> rootFunc = getRoot(*this)->getDescFuncBySig(signature);
		if(rootFunc)
			foundFunc = *rootFunc;
	}
	if (foundFunc)
	{
		if(foundFunc->prototype) //Prototype function declared
		{
			if(prototype) //Another identical prototype being declared
			{
				//Check default returns
				std::optional<int32_t> val = foundFunc->defaultReturn->getCompileTimeValue(handler, this);
				std::optional<int32_t> val2 = node->defaultReturn.get()->getCompileTimeValue(handler, this);
				if(!val || !val2 || (*val != *val2)) //Different or erroring default returns
				{
					handler->handleError(CompileError::BadDefaultReturn(node, node->name));
					return NULL;
				}
				else //Same default return; disable duplicate prototype without error
				{
					node->disable();
					return NULL; //NULL return gives no error if 'node->prototype'
				}
			}
			else //Function can be replaced by the new implementation of the prototype definition
			{
				//Remove the unneeded prototype function
				removeFunction(foundFunc);
				//Disable the node which defined the prototype function, and nullify it's pointer to the Function
				foundFunc->node->func = NULL;
				foundFunc->node->disable();
				//Delete the Function* to free memory
				delete foundFunc;
				//Continue to construct the new function
			}
		}
		else return NULL; //NULL return gives no error if 'node->prototype'
	}

	Function* fun = new Function(
			returnType, name, paramTypes, paramNames, ScriptParser::getUniqueFuncID(), flags, 0, prototype, defRet);
	fun->internalScope = makeFunctionChild(*fun);
	if(node)
	{
		for(auto it = node->optvals.begin(); it != node->optvals.end(); ++it)
		{
			fun->opt_vals.push_back(*it);
		}
	}

	functionsByName_[name].push_back(fun);
	functionsBySignature_[signature] = fun;
	return fun;
}

void BasicScope::removeFunction(Function* function)
{
	if(!function) return;
	FunctionSignature signature(function->name, function->paramTypes);
	functionsBySignature_.erase(signature); //Erase from signature map
	//Find in name map, and erase
	std::optional<vector<Function*> > foundVector = find<vector<Function*> >(functionsByName_, function->name);
	if(!foundVector) return;
	vector<Function*>& funcvector = *foundVector;
	if(funcvector.size() == 1 && funcvector.back() == function)
	{
		functionsByName_.erase(function->name);
		return;
	}
	for (vector<Function*>::iterator it = funcvector.begin(); it != funcvector.end();)
	{
		Function* f = *it;
		if(f == function) //Erase the function when found
		{
			it = funcvector.erase(it);
			return; //Found function, and erased
		}
	}
}

void BasicScope::setDefaultOption(CompileOptionSetting value)
{
	defaultOption_ = value;
}

void BasicScope::setOption(CompileOption option, CompileOptionSetting value)
{
	assert(option.isValid());
	options_[option] = value;
}

bool BasicScope::can_add(Datum& datum, CompileErrorHandler* errorHandler)
{
	if (std::optional<string> name = datum.getName())
	{
		if (find<Datum*>(namedData_, *name))
		{
			if (errorHandler)
				errorHandler->handleError(
					CompileError::VarRedef(datum.getNode(),
						name->c_str()));
			return false;
		}
	}
	return true;
}

bool BasicScope::add(Datum& datum, CompileErrorHandler* errorHandler)
{
	if (std::optional<string> name = datum.getName())
	{
		if (find<Datum*>(namedData_, *name))
		{
			if (errorHandler)
				errorHandler->handleError(
						CompileError::VarRedef(datum.getNode(),
						                       name->c_str()));
			return false;
		}
		namedData_[*name] = &datum;
	}
	else anonymousData_.push_back(&datum);

	if (!ZScript::isGlobal(datum))
	{
		stackOffsets_[&datum] = stackDepth_++;
		invalidateStackSize();
	}

	return true;
}

// Stack

std::optional<int32_t> BasicScope::getLocalStackOffset(Datum const& datum) const
{
	Datum* key = const_cast<Datum*>(&datum);
	return find<int32_t>(stackOffsets_, key);
}

////////////////////////////////////////////////////////////////
// FileScope

FileScope::FileScope(Scope* parent, string const& filename)
	: BasicScope(parent, NULL), filename_(filename)
{
	//defaultOption_ = CompileOptionSetting::Default; //No, let it default to `Inherit`. -V
}

Scope* FileScope::makeChild(std::string const& name)
{
	Scope* result = BasicScope::makeChild(name);
	if (!result) return NULL;
	if (!getRoot(*this)->registerChild(name, result))
		result = NULL;
	return result;
}

ScriptScope* FileScope::makeScriptChild(Script& script)
{
	ScriptScope* result = BasicScope::makeScriptChild(script);
	if (!result) return NULL;
	if (!getRoot(*this)->registerChild(script.getName(), result))
		result = NULL;
	return result;
}

ClassScope* FileScope::makeClassChild(UserClass& user_class)
{
	ClassScope* result = BasicScope::makeClassChild(user_class);
	if (!result) return NULL;
	if (!getRoot(*this)->registerChild(user_class.getName(), result))
		result = NULL;
	return result;
}

NamespaceScope* FileScope::makeNamespaceChild(ASTNamespace& node)
{
	string name = node.name;
	if (Scope* scope = find<Scope*>(children_, name).value_or(std::add_pointer<Scope>::type()))
	{
		if(scope->isNamespace())
		{
			NamespaceScope* result = static_cast<NamespaceScope*>(scope);
			return result;
		}
		else return NULL;
	}
	if (Scope* scope = getRoot(*this)->getChild(name))
	{
		if(scope->isNamespace())
		{
			NamespaceScope* result = static_cast<NamespaceScope*>(scope);
			return result;
		}
		else return NULL;
	}
	Namespace* namesp = new Namespace(node);
	NamespaceScope* result = new NamespaceScope(getRoot(*this), getFile(), namesp);
	namesp->setScope(result);
	children_[name] = result;
	getRoot(*this)->registerChild(name, result);
	return result;
}

DataType const* FileScope::addDataType(
		std::string const& name, DataType const* type, AST* node)
{
	DataType const* result = BasicScope::addDataType(name, type, node);
	if (!result) return NULL;
	if (!getRoot(*this)->registerDataType(name, result)) result = NULL;
	return result;
}

bool FileScope::addScriptType(string const& name, ParserScriptType type, AST* node)
{
	if (!BasicScope::addScriptType(name, type, node)) return false;
	if (!getRoot(*this)->registerScriptType(name, type)) return false;
	return true;
}

Function* FileScope::addGetter(
		DataType const* returnType, std::string const& name,
		std::vector<DataType const*> const& paramTypes, vector<string const*> const& paramNames, int32_t flags, AST* node)
{
	Function* result = BasicScope::addGetter(
			returnType, name, paramTypes, paramNames, flags, node);
	if (!result) return NULL;
	if (!getRoot(*this)->registerGetter(name, result))
		result = NULL;
	return result;
}

Function* FileScope::addSetter(
		DataType const* returnType, std::string const& name,
		std::vector<DataType const*> const& paramTypes, vector<string const*> const& paramNames, int32_t flags, AST* node)
{
	Function* result = BasicScope::addSetter(
			returnType, name, paramTypes, paramNames, flags, node);
	if (!result) return NULL;
	if (!getRoot(*this)->registerSetter(name, result))
		result = NULL;
	return result;
}

Function* FileScope::addFunction(
		DataType const* returnType, std::string const& name,
		std::vector<DataType const*> const& paramTypes, vector<string const*> const& paramNames, int32_t flags, ASTFuncDecl* node, CompileErrorHandler* handler)
{
	Function* result = BasicScope::addFunction(
			returnType, name, paramTypes, paramNames, flags, node, handler);
	if (!result) return NULL;
	if (!getRoot(*this)->registerFunction(result))
		result = NULL;
	return result;
}
void FileScope::removeFunction(Function* function)
{
	BasicScope::removeFunction(function);
	getRoot(*this)->removeFunction(function);
}
void FileScope::removeLocalFunction(Function* function)
{
	BasicScope::removeFunction(function);
}

bool FileScope::can_add(Datum& datum, CompileErrorHandler* errorHandler)
{
	if (!BasicScope::can_add(datum, errorHandler))
		return false;
	// Check in root scope if it's named.
	if (std::optional<string> name = datum.getName())
		if (getRoot(*this)->getLocalDatum(*name))
		{
			if (errorHandler)
				errorHandler->handleError(
					CompileError::VarRedef(datum.getNode(),
						name->c_str()));
			return false;
		}
	return true;
}
bool FileScope::add(Datum& datum, CompileErrorHandler* errorHandler)
{
	if (!can_add(datum, errorHandler))
		return false;

	BasicScope::add(datum, errorHandler);
	// Register in root scope if it's named.
	if (std::optional<string> name = datum.getName())
		getRoot(*this)->registerDatum(*name, &datum);

	return true;
}

////////////////////////////////////////////////////////////////
// RootScope

namespace // file local
{
	int32_t calculateStackSize(Scope* scope)
	{
		int32_t greatestSize = scope->getLocalStackDepth();
		vector<Scope*> children = scope->getChildren();
		for (vector<Scope*>::const_iterator it = children.begin();
		     it != children.end(); ++it)
		{
			int32_t size = calculateStackSize(*it);
			if (greatestSize < size) greatestSize = size;
		}
		return greatestSize;
	}
};

RootScope::RootScope(TypeStore& typeStore)
	: BasicScope(typeStore, "root")
{
	std::ostringstream errorstream;
	
	try
	{
		// Add global library functions.
		GlobalSymbols::getInst().addSymbolsToScope(*this);
	}
	catch(std::exception &e)
	{
		errorstream << e.what() << '\n';
	}
	
	// Create builtin classes (not primitives like void, float, and bool).
	for (DataTypeId typeId = ZTID_CLASS_START;
	     typeId < ZTID_CLASS_END; ++typeId)
	{
		try
		{
			DataTypeClass const& type =
				*static_cast<DataTypeClass const*>(DataType::get(typeId));
			ZClass& klass = *typeStore.getClass(type.getClassId());
			LibrarySymbols& library = *LibrarySymbols::getTypeInstance(typeId);
			library.addSymbolsToScope(klass);
		}
		catch(std::exception &e)
		{
			errorstream << e.what() << '\n';
		}
	}
	
	std::string errors = errorstream.str();
	if(!errors.empty())
		throw compile_exception(errors);

	// Add builtin pointers.
	BuiltinConstant::create(*this, DataType::PLAYER, "Link", 1); //compat
	BuiltinConstant::create(*this, DataType::PLAYER, "Hero", 1);
	BuiltinConstant::create(*this, DataType::PLAYER, "Player", 1);
	BuiltinConstant::create(*this, DataType::SCREEN, "Screen", 1);
	BuiltinConstant::create(*this, DataType::REGION, "Region", 1);
	BuiltinConstant::create(*this, DataType::VIEWPORT, "Viewport", 1);
	BuiltinConstant::create(*this, DataType::GAME, "Game", 1);
	BuiltinConstant::create(*this, DataType::AUDIO, "Audio", 1);
	BuiltinConstant::create(*this, DataType::DEBUG, "Debug", 1);
	BuiltinConstant::create(*this, DataType::NPCDATA, "NPCData", 1);
	BuiltinConstant::create(*this, DataType::TEXT, "Text", 1);
	BuiltinConstant::create(*this, DataType::COMBOS, "ComboData", 1);
	BuiltinConstant::create(*this, DataType::SPRITEDATA, "SpriteData", 1);
	BuiltinConstant::create(*this, DataType::INPUT, "Input", 1);
	BuiltinConstant::create(*this, DataType::GRAPHICS, "Graphics", 1);
	BuiltinConstant::create(*this, DataType::MAPDATA, "MapData", 1);
	BuiltinConstant::create(*this, DataType::DMAPDATA, "DMapData", 1);
	BuiltinConstant::create(*this, DataType::ZMESSAGE, "MessageData", 1);
	BuiltinConstant::create(*this, DataType::SHOPDATA, "ShopData", 1);
	BuiltinConstant::create(*this, DataType::DROPSET, "DropData", 1);
	BuiltinConstant::create(*this, DataType::PONDS, "PondData", 1);
	BuiltinConstant::create(*this, DataType::WARPRING, "WarpRing", 1);
	BuiltinConstant::create(*this, DataType::DOORSET, "DoorSet", 1);
	BuiltinConstant::create(*this, DataType::ZUICOLOURS, "MiscColors", 1);
	BuiltinConstant::create(*this, DataType::TUNES, "MusicTrack", 1);
	BuiltinConstant::create(*this, DataType::PALCYCLE, "PalCycle", 1);
	BuiltinConstant::create(*this, DataType::GAMEDATA, "GameData", 1);
	BuiltinConstant::create(*this, DataType::CHEATS, "Cheats", 1);
	BuiltinConstant::create(*this, DataType::FILESYSTEM, "FileSystem", 1);
	BuiltinConstant::create(*this, DataType::ZINFO, "Module", 1); //compat
	BuiltinConstant::create(*this, DataType::ZINFO, "ZInfo", 1);
	BuiltinConstant::create(*this, DataType::CRNG, "RandGen", 0); //Nullptr is valid for engine RNG
}

std::optional<int32_t> RootScope::getRootStackSize() const
{
	if (!stackSize_)
	{
		RootScope* mutableThis = const_cast<RootScope*>(this);
		stackSize_ = calculateStackSize(mutableThis);
	}
	return stackSize_;
}

// Single

Scope* RootScope::getChild(std::string const& name) const
{
	Scope* result = BasicScope::getChild(name);
	if (!result)
		result = find<Scope*>(descChildren_, name).value_or(std::add_pointer<Scope>::type());
	return result;
}

DataType const* RootScope::getLocalDataType(string const& name) const
{
	DataType const* result = BasicScope::getLocalDataType(name);
	if (!result)
		result = find<DataType const*>(descDataTypes_, name).value_or(std::add_pointer<DataType const>::type());
	return result;
}

std::optional<ParserScriptType> RootScope::getLocalScriptType(string const& name) const
{
	if (std::optional<ParserScriptType> result = BasicScope::getLocalScriptType(name))
		return result;
	return find<ParserScriptType>(descScriptTypes_, name);
}

ZClass* RootScope::getLocalClass(string const& name) const
{
	ZClass* result = BasicScope::getLocalClass(name);
	if (!result)
		result = find<ZClass*>(descClasses_, name).value_or(std::add_pointer<ZClass>::type());
	return result;
}

Datum* RootScope::getLocalDatum(string const& name) const
{
	Datum* result = BasicScope::getLocalDatum(name);
	if (!result)
		result = find<Datum*>(descData_, name).value_or(std::add_pointer<Datum>::type());
	return result;
}

Function* RootScope::getLocalGetter(string const& name) const
{
	Function* result = BasicScope::getLocalGetter(name);
	if (!result)
		result = find<Function*>(descGetters_, name).value_or(std::add_pointer<Function>::type());
	return result;
}

Function* RootScope::getLocalSetter(string const& name) const
{
	Function* result = BasicScope::getLocalSetter(name);
	if (!result)
		result = find<Function*>(descSetters_, name).value_or(std::add_pointer<Function>::type());
	return result;
}

Function* RootScope::getLocalFunction(
		FunctionSignature const& signature) const
{
	Function* result = BasicScope::getLocalFunction(signature);
	if (!result)
		result = find<Function*>(descFunctionsBySignature_, signature)
			.value_or(std::add_pointer<Function>::type());
	return result;
}

vector<Function*> RootScope::getLocalFunctions(string const& name) const
{
	vector<Function*> results(BasicScope::getLocalFunctions(name));
	if (std::optional<vector<Function*> > desc =
	    	find<vector<Function*> >(descFunctionsByName_, name))
		appendElements(results, *desc);
	return results;
}

// All

vector<Datum*> RootScope::getLocalData() const
{
	vector<Datum*> results(BasicScope::getLocalData());
	appendElements(results, getSeconds<Datum*>(descData_));
	return results;
}

vector<Function*> RootScope::getLocalFunctions() const
{
	vector<Function*> results(BasicScope::getLocalFunctions());
	appendElements(results, getSeconds<Function*>(descFunctionsBySignature_));
	return results;
}

vector<Function*> RootScope::getLocalGetters() const
{
	vector<Function*> results(BasicScope::getLocalGetters());
	appendElements(results, getSeconds<Function*>(descGetters_));
	return results;
}

vector<Function*> RootScope::getLocalSetters() const
{
	vector<Function*> results(BasicScope::getLocalSetters());
	appendElements(results, getSeconds<Function*>(descSetters_));
	return results;
}

// Register

bool RootScope::registerChild(string const& name, Scope* child)
{
	if (getChild(name)) return false;
	descChildren_[name] = child;
	return true;
}



bool RootScope::registerDataType(string const& name, DataType const* type)
{
	if (getLocalDataType(name)) return false;
	descDataTypes_[name] = type;
	return true;
}

bool RootScope::registerScriptType(std::string const& name, ParserScriptType type)
{
	if (getLocalScriptType(name)) return false;
	descScriptTypes_[name] = type;
	return true;
}

bool RootScope::registerClass(string const& name, ZClass* klass)
{
	if (getLocalClass(name)) return false;
	descClasses_[name] = klass;
	return true;
}

bool RootScope::registerDatum(string const& name, Datum* datum)
{
	if (getLocalDatum(name)) return false;
	descData_[name] = datum;
	return true;
}

bool RootScope::registerGetter(string const& name, Function* getter)
{
	if (getLocalGetter(name)) return false;
	descGetters_[name] = getter;
	return true;
}

bool RootScope::registerSetter(string const& name, Function* setter)
{
	if (getLocalSetter(name)) return false;
	descSetters_[name] = setter;
	return true;
}

bool RootScope::registerFunction(Function* function)
{
	FunctionSignature signature(*function);
	if (getLocalFunction(signature)) return false;
	descFunctionsByName_[signature.name].push_back(function);
	descFunctionsBySignature_[signature] = function;
	return true;
}

void RootScope::removeFunction(Function* function)
{
	if(!function) return;
	BasicScope::removeFunction(function); //Remove from basic scope maps
	//Make sure it is removed from it's parent file!
	function->internalScope->getFile()->removeLocalFunction(function);

	FunctionSignature signature(function->name, function->paramTypes);
	descFunctionsBySignature_.erase(signature); //Erase from signature map
	//Find in name map, and erase
	std::optional<vector<Function*> > foundVector = find<vector<Function*> >(descFunctionsByName_, function->name);
	if(!foundVector) return;
	vector<Function*>& funcvector = *foundVector;
	if(funcvector.size() == 1 && funcvector.back() == function)
	{
		descFunctionsByName_.erase(function->name);
		return;
	}
	for (vector<Function*>::iterator it = funcvector.begin(); it != funcvector.end();)
	{
		Function* f = *it;
		if(f == function) //Erase the function when found
		{
			it = funcvector.erase(it);
			return; //Found function, and erased
		}
	}
}

std::optional<Function*> RootScope::getDescFuncBySig(FunctionSignature& sig)
{
	return find<Function*>(descFunctionsBySignature_, sig);
}

bool RootScope::checkImport(ASTImportDecl* node, int32_t headerGuard, CompileErrorHandler* errorHandler)
{
	if(node->wasChecked()) return true;
	node->check();
	if(headerGuard == OPT_OFF) return true; //Don't check anything, behave as usual.
	string fname = node->getFilename();
	//lowerstr(fname);
	if(ASTImportDecl* first = find<ASTImportDecl*>(importsByName_, fname).value_or(std::add_pointer<ASTImportDecl>::type()))
	{
		node->disable(); //Disable node.
		switch(headerGuard)
		{
			case OPT_ERROR:
			{
				errorHandler->handleError(CompileError::HeaderGuardErr(node, node->getFilename()));
				return false; //Error, halt.
			}

			case OPT_WARN:
			{
				errorHandler->handleError(CompileError::HeaderGuardWarn(node, node->getFilename(), "Skipping"));
				return false; //Warn, and do not allow import
			}

			default: //OPT_ON, or any invalid value, if the user sets it as such.
			{
				return false; //No message, guard against the duplicate import.
			}

		}
	}
	//zconsole_db("Import '%s' checked and registered successfully", fname.c_str());
	importsByName_[fname] = node;
	return true; //Allow import
}

bool RootScope::isImported(string const& path)
{
	if(find<ASTImportDecl*>(importsByName_, path).value_or(std::add_pointer<ASTImportDecl>::type()))
		return true;
	return false;
}

////////////////////////////////////////////////////////////////
// ScriptScope

ScriptScope::ScriptScope(Scope* parent, FileScope* parentFile, Script& script)
	: BasicScope(parent, parentFile, script.getName()), script(script)
{}

////////////////////////////////////////////////////////////////
// ClassScope

ClassScope::ClassScope(Scope* parent, FileScope* parentFile, UserClass& user_class)
	: BasicScope(parent, parentFile, user_class.getName()), user_class(user_class), destructor_(nullptr)
{}

bool ClassScope::add(Datum& datum, CompileErrorHandler* errorHandler)
{
	if(UserClassVar* ucv = dynamic_cast<UserClassVar*>(&datum))
	{
		if (std::optional<string> name = ucv->getName())
		{
			if (find<UserClassVar*>(classData_, *name))
			{
				if (errorHandler)
					errorHandler->handleError(
							CompileError::VarRedef(ucv->getNode(),
												   name->c_str()));
				return false;
			}
			classData_[*name] = ucv;
			ucv->setOrder(classData_.size());
			if (!ZScript::isGlobal(datum))
			{
				stackOffsets_[&datum] = stackDepth_++;
				invalidateStackSize();
			}
			return true;
		}
		return false;
	}
	else return BasicScope::add(datum, errorHandler);
}

void ClassScope::parse_ucv()
{
	std::vector<UserClassVar*> ucvs = getSeconds<UserClassVar*>(classData_);
	
	//Sort them in proper order, or access will be wrong
	std::map<int32_t,UserClassVar*> ucv_map;
	for(auto ucv : ucvs)
		ucv_map[ucv->getOrder()] = ucv;
	ucvs = getSeconds<UserClassVar*>(ucv_map);
	
	int32_t ind = 0;
	for(auto ucv : ucvs) //Variables first
	{
		if(ucv->type.isArray()) continue;
		ucv->setIndex(ind++);
	}
	for(auto ucv : ucvs) //Then arrays
	{
		if(!ucv->type.isArray()) continue;
		ucv->setIndex(ind++);
	}
}

UserClassVar* ClassScope::getClassVar(std::string const& name)
{
	if (std::optional<UserClassVar*> var = find<UserClassVar*>(classData_, name))
	{
		return *var;
	}
	return nullptr;
}

std::vector<Function*> ClassScope::getConstructors() const
{
	return getSeconds<Function*>(constructorsBySignature_);
}
std::vector<Function*> ClassScope::getDestructor() const
{
	std::vector<Function*> v;
	if(destructor_) v.push_back(destructor_);
	return v;
}

Function* ClassScope::addFunction(
		DataType const* returnType, string const& name,
		vector<DataType const*> const& paramTypes, vector<string const*> const& paramNames, int32_t flags, ASTFuncDecl* node, CompileErrorHandler* handler)
{
	bool constructor = (flags&FUNCFLAG_CONSTRUCTOR);
	bool destructor = (flags&FUNCFLAG_DESTRUCTOR);
	bool condes = constructor||destructor;
	bool prototype = false;
	ASTExprConst* defRet = NULL;
	if(condes && name.compare(user_class.getName()))
	{
		if(constructor)
			handler->handleError(CompileError::NameMismatchC(node, name.c_str(), user_class.getName().c_str()));
		else handler->handleError(CompileError::NameMismatchD(node, name.c_str(), user_class.getName().c_str()));
		return NULL;
	}
	if(destructor && paramTypes.size())
	{
		handler->handleError(CompileError::DestructorParam(node, name.c_str()));
		return NULL;
	}
	if(node)
	{
		prototype = node->prototype;
		if(prototype)
		{
			defRet = node->defaultReturn.get();
		}
	}
	FunctionSignature signature(name, paramTypes);
	Function* foundFunc = NULL;
	std::optional<Function*> optFunc;
	if(constructor)
		optFunc = find<Function*>(constructorsBySignature_, signature);
	else if(destructor)
		optFunc = destructor_ ? std::optional<Function*>(destructor_) : std::optional<Function*>(std::nullopt);
	else
		optFunc = find<Function*>(functionsBySignature_, signature);
	
	if(optFunc)
		foundFunc = *optFunc;
	if (foundFunc)
	{
		if(foundFunc->prototype) //Prototype function declared
		{
			if(prototype) //Another identical prototype being declared
			{
				//Check default returns
				std::optional<int32_t> val = foundFunc->defaultReturn->getCompileTimeValue(handler, this);
				std::optional<int32_t> val2 = node->defaultReturn.get()->getCompileTimeValue(handler, this);
				if(!val || !val2 || (*val != *val2)) //Different or erroring default returns
				{
					handler->handleError(CompileError::BadDefaultReturn(node, node->name));
					return NULL;
				}
				else //Same default return; disable duplicate prototype without error
				{
					node->disable();
					return NULL; //NULL return gives no error if 'node->prototype'
				}
			}
			else //Function can be replaced by the new implementation of the prototype definition
			{
				//Remove the unneeded prototype function
				removeFunction(foundFunc);
				//Disable the node which defined the prototype function, and nullify it's pointer to the Function
				foundFunc->node->func = NULL;
				foundFunc->node->disable();
				//Delete the Function* to free memory
				delete foundFunc;
				//Continue to construct the new function
			}
		}
		else return NULL; //NULL return gives no error if 'node->prototype'
	}

	Function* fun = new Function(
			returnType, name, paramTypes, paramNames, ScriptParser::getUniqueFuncID(), flags, 0, prototype, defRet);
	fun->internalScope = makeFunctionChild(*fun);
	if(node)
	{
		for(auto it = node->optvals.begin(); it != node->optvals.end(); ++it)
		{
			fun->opt_vals.push_back(*it);
		}
	}
	
	if(constructor)
		constructorsBySignature_[signature] = fun;
	else if(destructor)
		destructor_ = fun;
	else
	{
		functionsByName_[name].push_back(fun);
		functionsBySignature_[signature] = fun;
	}
	return fun;
}

////////////////////////////////////////////////////////////////
// FunctionScope

FunctionScope::FunctionScope(Scope* parent, FileScope* parentFile, Function& function)
	: BasicScope(parent, parentFile, function.name), function(function)
{
	// Functions have their own stack.
	stackDepth_ = 0;
}

std::optional<int32_t> FunctionScope::getRootStackSize() const
{
	if (!stackSize)
	{
		FunctionScope* mutableThis = const_cast<FunctionScope*>(this);
		stackSize = calculateStackSize(mutableThis);
	}
	return stackSize;
}

////////////////////////////////////////////////////////////////
// NamespaceScope

NamespaceScope::NamespaceScope(Scope* parent, FileScope* parentFile, Namespace* namesp)
	: BasicScope(parent, parentFile, namesp->getName()), namesp(namesp)
{}

NamespaceScope::~NamespaceScope()
{
	delete namesp;
}

////////////////////////////////////////////////////////////////
// InlineScope

InlineScope::InlineScope(Scope* parent, FileScope* parentFile, ASTExprCall* node, ASTBlock* block)
	: BasicScope(parent, parentFile), node(node), block(block)
{}

////////////////////////////////////////////////////////////////
// ZClass

ZClass::ZClass(TypeStore& typeStore, string const& name, int32_t id)
	: BasicScope(typeStore), name(name), id(id)
{}
