#ifndef ZPARSER_SCOPE_H
#define ZPARSER_SCOPE_H

#include "DataStructs.h"

class Scope
{
public:
	static Scope* makeGlobalScope(SymbolTable& table);

	Scope(SymbolTable& table);

	// SymbolTable
	SymbolTable const& getTable() const {return table;}
	SymbolTable& getTable() {return table;}

	////////////////
	// Virtual Methods

	// Inheritance
	virtual Scope* getParent() const = 0;
	virtual Scope* makeChild(string const& name) = 0;
	virtual Scope* getLocalChild(string const& name) const = 0;
	// Types
	virtual int getLocalTypeId(string const& name) const = 0;
	virtual int addType(string const& name, ZVarTypeId typeId, AST* node) = 0;
	// Variables
	virtual int getLocalVariableId(string const& name) const = 0;
	virtual int addVariable(string const& name, ZVarTypeId typeId, AST* node) = 0;
	// Functions
	virtual void getLocalFunctionIds(vector<int>& ids, string const& name) const = 0;
	virtual int getLocalFunctionId(FunctionSignature const& signature) const = 0;
	virtual int addFunction(string const& name, ZVarTypeId returnTypeId, vector<ZVarTypeId> const& paramTypeIds, AST* node) = 0;

	////////////////
	// Convenience Methods

	// Inheritance
	Scope& getOrMakeLocalChild(string const& name);
	Scope* getNamespace(string const& name) const;
	// Types
	int getTypeId(string const& name) const;
	ZVarType* getLocalType(string const& name) const;
	ZVarType* getType(string const& name) const;
	int addType(string const& name, ZVarType const& type, AST* node);
	int addType(string const& name, ZVarTypeId typeId);
	int addType(string const& name, ZVarType const& type);
	// Variables
	int getVariableId(string const& name) const;
	int addVariable(string const& name, ZVarType const& type, AST* node);
	int addVariable(string const& name, ZVarTypeId typeId);
	int addVariable(string const& name, ZVarType const& type);
	// Functions
	vector<int> getLocalFunctionIds(string const& name) const;
	void getFunctionIds(vector<int>& ids, string const& name) const;
	vector<int> getFunctionIds(string const& name) const;
	int getFunctionId(FunctionSignature const& signature) const;
	int addFunction(string const& name, ZVarType const& returnType, vector<ZVarType*> const& paramTypes, AST* node);
	int addFunction(string const& name, ZVarTypeId returnTypeId, vector<ZVarTypeId> const& paramTypeIds);
	int addFunction(string const& name, ZVarType const& returnType, vector<ZVarType*> const& paramTypes);

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
	Scope* makeChild(string const& name);
	Scope* getLocalChild(string const& name) const;
	// Types
	int getLocalTypeId(string const& name) const;
	using Scope::addType;
	int addType(string const& name, ZVarTypeId typeId, AST* node);
	// Variables
	int getLocalVariableId(string const& name) const;
	using Scope::addVariable;
	int addVariable(string const& name, ZVarTypeId typeId, AST* node);
	// Functions
	using Scope::getLocalFunctionIds;
	void getLocalFunctionIds(vector<int>& ids, string const& name) const;
	int getLocalFunctionId(FunctionSignature const& signature) const;
	using Scope::addFunction;
	int addFunction(string const& name, ZVarTypeId returnTypeId, vector<ZVarTypeId> const& paramTypeIds, AST* node);
private:
    Scope* parent;
    map<string, Scope*> children;
	map<string, int> types;
	map<string, int> variables;
	map<string, vector<int> > functionsByName;
	map<FunctionSignature, int> functionsBySignature;
};

#endif
