#ifndef ZPARSER_SCOPE_H
#define ZPARSER_SCOPE_H

#include <assert.h>
#include "DataStructs.h"

namespace ZScript
{
	class Variable;
	class Function;
};

class Scope
{
public:
	static Scope* makeGlobalScope(SymbolTable& table);

	Scope(SymbolTable& table);

	// SymbolTable
	SymbolTable const& getTable() const {return table;}
	SymbolTable& getTable() {return table;}

	// Others
	virtual bool isGlobal() const {return false;}
	virtual bool isScript() const {return false;}

	////////////////
	// Virtual Methods

	// Inheritance
	virtual Scope* getParent() const = 0;
	virtual Scope* makeChild() = 0;
	virtual Scope* makeChild(string const& name) = 0;
	virtual Scope* getLocalChild(string const& name) const = 0;
	virtual vector<Scope*> getAnonymousChildren() const = 0;
	virtual void getLocalChildren(vector<Scope*>&) const = 0;
	// Types
	virtual int getLocalTypeId(string const& name) const = 0;
	virtual int addType(string const& name, ZVarTypeId typeId, AST* node) = 0;
	// Classes
	virtual int getLocalClassId(string const& name) const = 0;
	virtual int addClass(string const& name, AST* node) = 0;
	// Variables
	virtual vector<ZScript::Variable*> getLocalVariables() const = 0;
	virtual ZScript::Variable* getLocalVariable(string const& name) const = 0;
	virtual ZScript::Variable* addVariable(ZVarType const& type, string const& name, AST* node = NULL) = 0;
	// Properties
	virtual ZScript::Function* getLocalGetter(string const& name) const = 0;
	virtual ZScript::Function* getLocalSetter(string const& name) const = 0;
	virtual ZScript::Function* addGetter(ZVarType const* returnType, string const& name,
								vector<ZVarType const*> const& paramTypes, AST* node = NULL) = 0;
	virtual ZScript::Function* addSetter(ZVarType const* returnType, string const& name,
								vector<ZVarType const*> const& paramTypes, AST* node = NULL) = 0;
	// Functions
	virtual void getLocalFunctions(vector<ZScript::Function*>&) const = 0;
	virtual void getLocalFunctions(string const& name, vector<ZScript::Function*>& out) const = 0;
	virtual ZScript::Function* getLocalFunction(FunctionSignature const& signature) const = 0;
	virtual ZScript::Function* addFunction(
			ZVarType const* returnType, string const& name,
			vector<ZVarType const*> const& paramTypes, AST* node = NULL) = 0;

	////////////////
	// Convenience Methods

	// Inheritance
	Scope& getOrMakeLocalChild(string const& name);
	Scope* getNamespace(string const& name) const;
	Scope* getLocalChild(vector<string> const& names) const;
	Scope* getChild(vector<string>const& names)const;
	vector<Scope*> getLocalChildren() const;
	// Types
	int getTypeId(string const& name) const;
	ZVarType* getLocalType(string const& name) const;
	ZVarType* getType(string const& name) const;
	int addType(string const& name, ZVarType const& type, AST* node);
	int addType(string const& name, ZVarTypeId typeId);
	int addType(string const& name, ZVarType const& type);
	// Classes
	int getClassId(string const& name) const;
	ZClass* getLocalClass(string const& name) const;
	ZClass* getClass(string const& name) const;
	int addClass(string const& name);
	// Variables
	ZScript::Variable* getVariable(string const& name) const;
	ZScript::Variable* getVariable(vector<string> const& name) const;
	// Properties
	ZScript::Function* getGetter(string const& name) const;
	ZScript::Function* getSetter(string const& name) const;
	// Functions
	vector<ZScript::Function*> getLocalFunctions() const;
	vector<ZScript::Function*> getLocalFunctions(string const& name) const;
	vector<ZScript::Function*> getAllFunctions() const;
	void getAllFunctions(vector<ZScript::Function*>&) const;
	void getLocalFunctionIds(string const& name, vector<int>& out) const;
	vector<int> getLocalFunctionIds(string const& name) const;
	void getFunctions(string const& name, vector<ZScript::Function*>& out) const;
	void getFunctionIds(string const& name, vector<int>& out) const;
	vector<ZScript::Function*> getFunctions(string const& name) const;
	vector<int> getFunctionIds(string const& name) const;
	vector<ZScript::Function*> getFunctions(vector<string> const& names)const;
	vector<int> getFunctionIds(vector<string> const& names)const;
	ZScript::Function* getFunction(FunctionSignature const& signature) const;
	int getFunctionId(FunctionSignature const& signature) const;
	ZScript::Function* addFunction(ZVarTypeId returnTypeId, string const& name, vector<ZVarTypeId> const& paramTypeIds, AST* node = NULL);

	bool varDeclsDeprecated;
protected:
	SymbolTable& table;
};

class BasicScope : public Scope
{
public:
	BasicScope(SymbolTable& table);
	BasicScope(Scope* parent);
	~BasicScope();
	// Children.
	Scope* getParent() const;
	Scope* makeChild();
	Scope* makeChild(string const& name);
	Scope* getLocalChild(string const& name) const;
	vector<Scope*> getAnonymousChildren() const;
	using Scope::getLocalChildren;
	void getLocalChildren(vector<Scope*>&) const;
	// Types
	int getLocalTypeId(string const& name) const;
	using Scope::addType;
	int addType(string const& name, ZVarTypeId typeId, AST* node);
	// Classes
	int getLocalClassId(string const& name) const;
	int addClass(string const& name, AST* node);
	// Variables
	vector<ZScript::Variable*> getLocalVariables() const;
	ZScript::Variable* getLocalVariable(string const& name) const;
	using Scope::addVariable;
	ZScript::Variable* addVariable(ZVarType const& type, string const& name, AST* node = NULL);
	// Properties
	ZScript::Function* getLocalGetter(string const& name) const;
	ZScript::Function* getLocalSetter(string const& name) const;
	ZScript::Function* addGetter(ZVarType const* returnType, string const& name, vector<ZVarType const*> const& paramTypes, AST* node = NULL);
	ZScript::Function* addSetter(ZVarType const* returnType, string const& name, vector<ZVarType const*> const& paramTypes, AST* node = NULL);
	// Functions
	using Scope::getLocalFunctions;
	void getLocalFunctions(vector<ZScript::Function*>&) const;
	void getLocalFunctions(string const& name, vector<ZScript::Function*>& out) const;
	ZScript::Function* getLocalFunction(FunctionSignature const& signature) const;
	using Scope::addFunction;
	ZScript::Function* addFunction(ZVarType const* returnType, string const& name,
						  vector<ZVarType const*> const& paramTypes, AST* node = NULL);
protected:
    Scope* parent;
    map<string, Scope*> children;
	vector<Scope*> anonymousChildren;
	map<string, int> types;
	map<string, int> classes;
	map<string, ZScript::Variable*> variables;
	map<string, ZScript::Function*> getters;
	map<string, ZScript::Function*> setters;
	map<string, vector<ZScript::Function*> > functionsByName;
	map<FunctionSignature, ZScript::Function*> functionsBySignature;

	// Disabled since it's easy to call by accident instead of the Scope*
	// constructor.
	BasicScope(BasicScope const& base) : Scope(base.table) {assert(false);}
};

class ScriptScope;
class GlobalScope : public BasicScope
{
public:
	GlobalScope(SymbolTable& table);
	bool isGlobal() const {return true;}
	ScriptScope* makeScriptChild(ZScript::Script& script);
};

class ScriptScope : public BasicScope
{
public:
	ScriptScope(GlobalScope* scope, ZScript::Script& script) : BasicScope(scope), script(script) {}
	bool isScript() const {return true;}
	ZScript::Script& script;
};

enum ZClassIdBuiltin
{
	ZCLASSID_START = 0,
    ZCLASSID_GAME = 0, ZCLASSID_LINK, ZCLASSID_SCREEN,
    ZCLASSID_FFC, ZCLASSID_ITEM, ZCLASSID_ITEMCLASS, ZCLASSID_NPC, ZCLASSID_LWPN, ZCLASSID_EWPN,
	ZCLASSID_END
};

class ZClass : public BasicScope
{
public:
	ZClass(SymbolTable& table, string const& name, int id);
	string const name;
	int const id;
};

#endif
