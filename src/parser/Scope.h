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
	// Children
	virtual Scope* getParent() const = 0;
	virtual Scope* makeChild(string const& name) = 0;
	virtual Scope* getChild(string const& name) const = 0;
	Scope& getOrMakeChild(string const& name);
	// Types
	int getTypeId(string const& nspace, string const& name) const;
	int getTypeId(vector<string> const& names) const;
	virtual int getTypeId(string const& name) const = 0;
	ZVarType* getType(string const& nspace, string const& name) const;
	ZVarType* getType(vector<string> const& names) const;
	ZVarType* getType(string const& name) const;
	virtual int addType(string const& name, ZVarTypeId typeId, AST* node) = 0;
	int addType(string const& name, ZVarType const& type, AST* node);
	int addType(string const& name, ZVarTypeId typeId);
	int addType(string const& name, ZVarType const& type);
	// Variables
	int getVarId(string const& nspace, string const& name) const;
	int getVarId(vector<string> const& names) const;
	virtual int getVarId(string const& name) const = 0;
	virtual int addVar(string const& name, ZVarTypeId typeId, AST* node) = 0;
	int addVar(string const& name, ZVarType const& type, AST* node);
	int addVar(string const& name, ZVarTypeId typeId);
	int addVar(string const& name, ZVarType const& type);
	// Functions
	vector<int> getFuncIds(string const& nspace, string const& name) const;
	vector<int> getFuncIds(vector<string> const& names) const;
	vector<int> getFuncIds(string const& name) const;
	void getFuncIds(vector<int>& ids, vector<string> const& names) const;
	virtual void getFuncIds(vector<int>& ids, string const& name) const = 0;
	int getFuncId(string const& nspace, FunctionSignature const& signature) const;
	int getFuncId(vector<string> const& names, FunctionSignature const& signature) const;
	virtual int getFuncId(FunctionSignature const& signature) const = 0;
	virtual int addFunc(string const& name, ZVarTypeId returnTypeId, vector<ZVarTypeId> const& paramTypeIds, AST* node) = 0;
	int addFunc(string const& name, ZVarType const& returnType, vector<ZVarType*> const& paramTypes, AST* node);
	int addFunc(string const& name, ZVarTypeId returnTypeId, vector<ZVarTypeId> const& paramTypeIds);
	int addFunc(string const& name, ZVarType const& returnType, vector<ZVarType*> const& paramTypes);
protected:
	SymbolTable& table;
	int getTypeIdNoParent(vector<string> const& names) const;
	virtual int getTypeIdNoParent(string const& name) const = 0;
	int getVarIdNoParent(vector<string> const& names) const;
	virtual int getVarIdNoParent(string const& name) const = 0;
	void getFuncIdsNoParent(vector<int>& ids, vector<string> const& names) const;
	virtual void getFuncIdsNoParent(vector<int>& ids, string const& name) const = 0;
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
	Scope* getChild(string const& name) const;
	// Types
	using Scope::getTypeId;
	int getTypeId(string const& name) const;
	using Scope::addType;
	int addType(string const& name, ZVarTypeId typeId, AST* node);
	// Variables
	using Scope::getVarId;
	int getVarId(string const& name) const;
	using Scope::addVar;
	int addVar(string const& name, ZVarTypeId typeId, AST* node);
	// Functions
	using Scope::getFuncIds;
	void getFuncIds(vector<int>& ids, string const& name) const;
	using Scope::getFuncId;
	int getFuncId(FunctionSignature const& signature) const;
	using Scope::addFunc;
	int addFunc(string const& name, ZVarTypeId returnTypeId, vector<ZVarTypeId> const& paramTypeIds, AST* node);
protected:
	int getTypeIdNoParent(string const& name) const;
	int getVarIdNoParent(string const& name) const;
	void getFuncIdsNoParent(vector<int>& ids, string const& name) const;
private:
    Scope* parent;
    map<string, Scope*> children;
	map<string, int> types;
	map<string, int> variables;
	map<string, vector<int> > functionsByName;
	map<FunctionSignature, int> functionsBySignature;
};

#endif
