#ifndef ZPARSER_SCOPE_H
#define ZPARSER_SCOPE_H

#include "DataStructs.h"

class Scope
{
public:
	Scope(SymbolTable& table);
	Scope(Scope* parent);
	~Scope();
	// SymbolTable
	SymbolTable const& getTable() const {return table;}
	SymbolTable& getTable() {return table;}
	// Children
	Scope* makeChild(string const& name);
	Scope* getChild(string const& name) const;
	Scope& getOrMakeChild(string const& name);
	// Types
	int getTypeId(string const& nspace, string const& name) const;
	int getTypeId(vector<string> const& names) const;
	int getTypeId(string const& name) const;
	ZVarType* getType(string const& nspace, string const& name) const;
	ZVarType* getType(vector<string> const& names) const;
	ZVarType* getType(string const& name) const;
	int addType(string const& name, ZVarTypeId typeId, AST* node);
	int addType(string const& name, ZVarType const& type, AST* node);
	int addType(string const& name, ZVarTypeId typeId);
	int addType(string const& name, ZVarType const& type);
	// Variables
	int getVarId(string const& nspace, string const& name) const;
	int getVarId(vector<string> const& names) const;
	int getVarId(string const& name) const;
	int addVar(string const& name, ZVarTypeId typeId, AST* node);
	int addVar(string const& name, ZVarType const& type, AST* node);
	int addVar(string const& name, ZVarTypeId typeId);
	int addVar(string const& name, ZVarType const& type);
	// Functions
	vector<int> getFuncIds(string const& nspace, string const& name) const;
	vector<int> getFuncIds(vector<string> const& names) const;
	vector<int> getFuncIds(string const& name) const;
	int getFuncId(string const& nspace, FunctionSignature const& signature) const;
	int getFuncId(vector<string> const& names, FunctionSignature const& signature) const;
	int getFuncId(FunctionSignature const& signature) const;
	int addFunc(string const& name, ZVarTypeId returnTypeId, vector<ZVarTypeId> const& paramTypeIds, AST* node);
	int addFunc(string const& name, ZVarType const& returnType, vector<ZVarType*> const& paramTypes, AST* node);
	int addFunc(string const& name, ZVarTypeId returnTypeId, vector<ZVarTypeId> const& paramTypeIds);
	int addFunc(string const& name, ZVarType const& returnType, vector<ZVarType*> const& paramTypes);
private:
	SymbolTable& table;
    Scope* parent;
    map<string, Scope*> children;
	map<string, int> types;
	map<string, int> variables;
	map<string, vector<int> > functionsByName;
	map<FunctionSignature, int> functionsBySignature;

	int getTypeIdNoParent(vector<string> const& names) const;
	int getTypeIdNoParent(string const& name) const;
	int getVarIdNoParent(vector<string> const& names) const;
	int getVarIdNoParent(string const& name) const;
	void getFuncIds(vector<int>& ids, vector<string> const& names) const;
	void getFuncIds(vector<int>& ids, string const& name) const;
	void getFuncIdsNoParent(vector<int>& ids, vector<string> const& names) const;
	void getFuncIdsNoParent(vector<int>& ids, string const& name) const;
};

#endif
