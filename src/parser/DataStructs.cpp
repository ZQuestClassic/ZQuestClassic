//2.53 Updated to 16th Jan, 2017
#include "../precompiled.h" //always first

#include "DataStructs.h"
#include "Types.h"
#include "../zsyssimple.h"
#include <assert.h>
#include <iostream>

using std::cout;
using std::endl;

////////////////////////////////////////////////////////////////
// FunctionSignature

FunctionSignature::FunctionSignature(string const& name, vector<ZVarTypeId> const& paramTypeIds)
	: name(name), paramTypeIds(paramTypeIds)
{}

int FunctionSignature::compare(FunctionSignature const& other) const
{
	int c = name.compare(other.name);
	if (c) return c;
	c = paramTypeIds.size() - other.paramTypeIds.size();
	if (c) return c;
	for (int i = 0; i < paramTypeIds.size(); ++i)
	{
		c = paramTypeIds[i] - other.paramTypeIds[i];
		if (c) return c;
	}
	return 0;
}

////////////////////////////////////////////////////////////////
// FunctionTypeIds

FunctionTypeIds::FunctionTypeIds(ZVarTypeId returnTypeId, vector<ZVarTypeId> const& paramTypeIds)
	: returnTypeId(returnTypeId), paramTypeIds(paramTypeIds)
{}

int FunctionTypeIds::compare(FunctionTypeIds const& other) const
{
	int c = returnTypeId - other.returnTypeId;
	if (c) return c;
	c = paramTypeIds.size() - other.paramTypeIds.size();
	if (c) return c;
	for (int i = 0; i < paramTypeIds.size(); ++i)
	{
		c = paramTypeIds[i] - other.paramTypeIds[i];
		if (c) return c;
	}
	return 0;
}

FunctionTypeIds const FunctionTypeIds::null;

////////////////////////////////////////////////////////////////
// SymbolTable

SymbolTable::SymbolTable(map<string, long> *consts)
	: nodeIds(), possibleNodeFuncIds(), types(), typeIds(),
	  varTypes(), funcTypes(), constants(consts)
{
	for (ZVarTypeId id = 0; id < ZVARTYPEID_END; ++id)
		assignTypeId(*ZVarType::get(id));
}

SymbolTable::~SymbolTable()
{
	for (vector<ZVarType*>::iterator it = types.begin(); it != types.end(); ++it)
		delete *it;
}

// Nodes

int SymbolTable::getNodeId(AST* node) const
{
	map<AST*, int>::const_iterator it = nodeIds.find(node);
	if (it == nodeIds.end()) return -1;
	return it->second;
}
    
void SymbolTable::putNodeId(AST* node, int id)
    {
    nodeIds[node] = id;
}

vector<int> SymbolTable::getPossibleNodeFuncIds(AST* node) const
{
	map<AST*, vector<int> >::const_iterator it = possibleNodeFuncIds.find(node);
	if (it == possibleNodeFuncIds.end()) return vector<int>();
	return it->second;
    }
    
void SymbolTable::putPossibleNodeFuncIds(AST* node, vector<int> possibleFuncIds)
{
	possibleNodeFuncIds[node] = possibleFuncIds;
}

// Types
ZVarType* SymbolTable::getType(ZVarTypeId typeId) const
{
	if (typeId < 0 || typeId > types.size()) return NULL;
	return types[typeId];
}

ZVarTypeId SymbolTable::getTypeId(ZVarType const& type) const
{
	map<ZVarType*, ZVarTypeId, ZVarType::PointerLess>::const_iterator it = typeIds.find((ZVarType*)&type);
	if (it == typeIds.end()) return -1;
	return it->second;
}

ZVarTypeId SymbolTable::assignTypeId(ZVarType const& type)
{
	map<ZVarType*, ZVarTypeId, ZVarType::PointerLess>::const_iterator it = typeIds.find((ZVarType*)&type);
	if (it != typeIds.end()) return -1;
	ZVarTypeId id = types.size();
	ZVarType* storedType = type.clone();
	types.push_back(storedType);
	typeIds[storedType] = id;
	return id;
}

ZVarTypeId SymbolTable::getOrAssignTypeId(ZVarType const& type)
{
	map<ZVarType*, ZVarTypeId, ZVarType::PointerLess>::const_iterator it = typeIds.find((ZVarType*)&type);
	if (it != typeIds.end()) return it->second;
	ZVarTypeId id = types.size();
	ZVarType* storedType = type.clone();
	types.push_back(storedType);
	typeIds[storedType] = id;
	return id;
}

// Variables

ZVarTypeId SymbolTable::getVarTypeId(int varId) const
{
    map<int, ZVarTypeId>::const_iterator it = varTypes.find(varId);
    
    if(it == varTypes.end())
    {
        box_out("Internal Error: Can't find the variable type!");
        box_eol();
        return -1;
    }
    
    return it->second;
}

ZVarTypeId SymbolTable::getVarTypeId(AST* obj) const
{
    map<AST*, int>::const_iterator it = nodeIds.find(obj);

    if (it == nodeIds.end())
    {
        box_out("Internal Error: Can't find the AST ID!");
        box_eol();
        return -1;
}

    return getVarTypeId(it->second);
}

void SymbolTable::putVarTypeId(int varId, ZVarTypeId typeId)
{
	varTypes[varId] = typeId;
}

void SymbolTable::putVarType(int varId, ZVarType const& type)
{
	varTypes[varId] = getOrAssignTypeId(type);
}

// Functions

ZVarTypeId SymbolTable::getFuncReturnTypeId(int funcId) const
{
	map<int, FunctionTypeIds>::const_iterator it = funcTypes.find(funcId);
	if (it == funcTypes.end()) return -1;
	return it->second.returnTypeId;
}

ZVarTypeId SymbolTable::getFuncReturnTypeId(AST* node) const
{
    return getFuncReturnTypeId(getNodeId(node));
}

vector<ZVarTypeId> SymbolTable::getFuncParamTypeIds(int funcId) const
{
	map<int, FunctionTypeIds>::const_iterator it = funcTypes.find(funcId);
	if (it == funcTypes.end()) return vector<ZVarTypeId>();
	return it->second.paramTypeIds;
}

void SymbolTable::putFuncTypeIds(int funcId, ZVarTypeId returnTypeId, vector<ZVarTypeId> const& paramTypeIds)
{
	funcTypes[funcId] = FunctionTypeIds(returnTypeId, paramTypeIds);
}

// 

bool SymbolTable::isConstant(string name) const
{
    return constants->find(name) != constants->end();
}

long SymbolTable::getConstantVal(string name) const
{
    return (*constants)[name];
}

void SymbolTable::printDiagnostics()
{
    cout << (unsigned int)varTypes.size() << " variable symbols" << endl;
    cout << (unsigned int)funcTypes.size() << " function symbols" << endl;
}

////////////////////////////////////////////////////////////////
// Scope

Scope::Scope(SymbolTable& table)
	: table(table), children(), parent(NULL), variables(), functionsBySignature(), functionsByName()
{}

Scope::Scope(Scope* parent)
	: table(parent->table), children(), parent(parent), variables(), functionsBySignature(), functionsByName()
{}

Scope::~Scope()
{
	for (map<string, Scope*>::iterator it = children.begin(); it != children.end(); ++it)
		delete it->second;
}
    
// Children

Scope* Scope::makeChild(string const& name)
{
	map<string, Scope*>::const_iterator it = children.find(name);
	if (it != children.end()) return NULL;
	children[name] = new Scope(this);
	return children[name];
}
        
Scope* Scope::getChild(string const& name) const
{
	map<string, Scope*>::const_iterator it = children.find(name);
	if (it == children.end()) return NULL;
	return it->second;
}

Scope& Scope::getOrMakeChild(string const& name)
{
	map<string, Scope*>::const_iterator it = children.find(name);
	if (it != children.end()) return *it->second;
	children[name] = new Scope(this);
	return *children[name];
}

// Variables

int Scope::getVarId(string const& nspace, string const& name) const
{
	if (nspace == "") return getVarId(name);
        
	vector<string> names;
	names.push_back(nspace);
	names.push_back(name);
	return getVarId(names);
}

int Scope::getVarId(vector<string> const& names) const
{
	if (names.size() < 1) return -1;
	if (names.size() == 1) return getVarId(names[0]);

	Scope* child = getChild(names[0]);
    if (child)
    {
		vector<string> childNames(names.begin() + 1, names.end());
		int varId = child->getVarId(childNames);
		if (varId != -1) return varId;
    }
    
	if (parent) return parent->getVarId(names);
	return -1;
}

int Scope::getVarId(string const& name) const
{
	map<string, int>::const_iterator it = variables.find(name);
	if (it != variables.end()) return it->second;
	if (parent) return parent->getVarId(name);
	return -1;
}

int Scope::addVar(string const& name, ZVarTypeId typeId, AST* node)
{
	map<string, int>::const_iterator it = variables.find(name);
	if (it != variables.end()) return -1;
	int varId = ScriptParser::getUniqueVarID();
	variables[name] = varId;
	table.putVarTypeId(varId, typeId);
	if (node) table.putNodeId(node, varId);
	return varId;
}

int Scope::addVar(string const& name, ZVarType const& type, AST* node)
{
	return addVar(name, table.getOrAssignTypeId(type), node);
}

int Scope::addVar(string const& name, ZVarTypeId typeId)
{
	return addVar(name, typeId, NULL);
}

int Scope::addVar(string const& name, ZVarType const& type)
{
	return addVar(name, table.getOrAssignTypeId(type), NULL);
}

// Functions

vector<int> Scope::getFuncIds(string const& nspace, string const& name) const
{
	if (nspace == "") return getFuncIds(name);
    
	vector<string> names;
	names.push_back(nspace);
	names.push_back(name);
	return getFuncIds(names);
}

vector<int> Scope::getFuncIds(vector<string> const& names) const
{
	vector<int> ids;
	getFuncIds(ids, names);
	return ids;
}

void Scope::getFuncIds(vector<int>& ids, vector<string> const& names) const
{
	if (names.size() < 1) return;
	if (names.size() == 1)
        {
		getFuncIds(ids, names[0]);
		return;
    }
    
	Scope* child = getChild(names[0]);
    if (child)
    {
		vector<string> childNames(names.begin() + 1, names.end());
		child->getFuncIds(ids, childNames);
    }
    
	if (parent) parent->getFuncIds(ids, names);
}

vector<int> Scope::getFuncIds(string const& name) const
{
	vector<int> ids;
	getFuncIds(ids, name);
	return ids;
}

void Scope::getFuncIds(vector<int>& ids, string const& name) const
{
	map<string, vector<int> >::const_iterator it = functionsByName.find(name);
	if (it != functionsByName.end())
		ids.insert(ids.end(), it->second.begin(), it->second.end());

	if (parent) parent->getFuncIds(ids, name);
}

int Scope::getFuncId(string const& nspace, FunctionSignature const& signature) const
{
	if (nspace == "") return getFuncId(signature);

	vector<string> names;
	names.push_back(nspace);
	return getFuncId(names, signature);
}

int Scope::getFuncId(vector<string> const& names, FunctionSignature const& signature) const
{
	if (names.size() == 0) return getFuncId(signature);

	Scope* child = getChild(names[0]);
	if (child)
        {
		vector<string> childNames(names.begin() + 1, names.end());
		int funcId = child->getFuncId(childNames, signature);
		if (funcId != -1) return funcId;
    }
    
	if (parent) return parent->getFuncId(names, signature);
	return -1;
}

int Scope::getFuncId(FunctionSignature const& signature) const
{
	map<FunctionSignature, int>::const_iterator it = functionsBySignature.find(signature);
	if (it != functionsBySignature.end()) return it->second;
	if (parent) return parent->getFuncId(signature);
	return -1;
}

int Scope::addFunc(string const& name, ZVarTypeId returnTypeId, vector<ZVarTypeId> const& paramTypeIds, AST* node)
{
	FunctionSignature signature(name, paramTypeIds);
	map<FunctionSignature, int>::const_iterator it = functionsBySignature.find(signature);
	if (it != functionsBySignature.end()) return -1;

	int funcId = ScriptParser::getUniqueFuncID();
	functionsByName[name].push_back(funcId);
	functionsBySignature[signature] = funcId;
	table.putFuncTypeIds(funcId, returnTypeId, paramTypeIds);
	if (node) table.putNodeId(node, funcId);
	return funcId;
}

int Scope::addFunc(string const& name, ZVarType const& returnType, vector<ZVarType*> const& paramTypes, AST* node)
{
	vector<ZVarTypeId> paramTypeIds;
	for (vector<ZVarType*>::const_iterator it = paramTypes.begin(); it != paramTypes.end(); ++it)
		paramTypeIds.push_back(table.getOrAssignTypeId(**it));
	return addFunc(name, table.getOrAssignTypeId(returnType), paramTypeIds, node);
}

int Scope::addFunc(string const& name, ZVarTypeId returnTypeId, vector<ZVarTypeId> const& paramTypeIds)
{
	return addFunc(name, returnTypeId, paramTypeIds, NULL);
}

int Scope::addFunc(string const& name, ZVarType const& returnType, vector<ZVarType*> const& paramTypes)
{
	return addFunc(name, returnType, paramTypes, NULL);
}

////////////////////////////////////////////////////////////////

int StackFrame::getOffset(int vid)
{
    map<int, int>::iterator it = stackoffset.find(vid);
    
    if(it == stackoffset.end())
    {
        box_out("Internal Error: Can't find stack offset for variable!");
        box_eol();
        return 0;
    }
    
    return stackoffset[vid];
}

