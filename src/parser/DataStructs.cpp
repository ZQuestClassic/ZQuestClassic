
#include "../precompiled.h" //always first

#include "DataStructs.h"
#include "Types.h"
#include "ParseError.h"
#include "GlobalSymbols.h"
#include "Scope.h"
#include "../zsyssimple.h"
#include <assert.h>
#include <iostream>

using std::cout;
using std::endl;

////////////////////////////////////////////////////////////////
// FunctionSignature

FunctionSignature::FunctionSignature(string const& name_, vector<ZVarType const*> const& paramTypes)
	: paramTypes(paramTypes)
{
	name.push_back(name_);
}

FunctionSignature::FunctionSignature(vector<string> const& name, vector<ZVarType const*> const& paramTypes)
	: name(name), paramTypes(paramTypes)
{}

int FunctionSignature::compare(FunctionSignature const& other) const
{
	int c = name.size() - other.name.size();
	if (c) return c;
	for (int i = 0; i < name.size(); ++i)
	{
		c = name[i].compare(other.name[i]);
		if (c) return c;
	}
	c = paramTypes.size() - other.paramTypes.size();
	if (c) return c;
	for (int i = 0; i < paramTypes.size(); ++i)
	{
		c = paramTypes[i]->compare(*other.paramTypes[i]);
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

SymbolTable::SymbolTable()
{
	// Assign builtin types.
	for (ZVarTypeId id = ZVARTYPEID_START; id < ZVARTYPEID_END; ++id)
		assignTypeId(*ZVarType::get(id));

	// Assign builtin classes.
	for (int id = ZVARTYPEID_CLASS_START; id < ZVARTYPEID_CLASS_END; ++id)
	{
		ZVarTypeClass& type = *(ZVarTypeClass*)ZVarType::get(id);
		assert(type.getClassId() == classes.size());
		classes.push_back(new ZClass(*this, type.getClassName(), type.getClassId()));
	}
}

SymbolTable::~SymbolTable()
{
	for (vector<ZVarType*>::iterator it = types.begin(); it != types.end(); ++it)
		delete *it;
	for (vector<ZClass*>::iterator it = classes.begin(); it != classes.end(); ++it)
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
	map<AST*, vector<int>>::const_iterator it = possibleNodeFuncIds.find(node);
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

ZVarType* SymbolTable::getType(AST* node) const
{
    return getType(getNodeId(node));
}

ZVarTypeId SymbolTable::getTypeId(ZVarType const& type) const
{
	map<ZVarType*, ZVarTypeId, ZVarType::PointerLess>::const_iterator it = typeIds.find((ZVarType*)&type);
	if (it == typeIds.end()) return -1;
	return it->second;
}

ZVarTypeId SymbolTable::assignTypeId(ZVarType const& type)
{
	if (!type.isResolved())
	{
		printErrorMsg(NULL, UNRESOLVEDTYPE, type.getName());
		return -1;
	}

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
	if (!type.isResolved())
	{
		printErrorMsg(NULL, UNRESOLVEDTYPE, type.getName());
		return -1;
	}

	map<ZVarType*, ZVarTypeId, ZVarType::PointerLess>::const_iterator it = typeIds.find((ZVarType*)&type);
	if (it != typeIds.end()) return it->second;
	ZVarTypeId id = types.size();
	ZVarType* storedType = type.clone();
	types.push_back(storedType);
	typeIds[storedType] = id;
	return id;
}

// Classes

ZClass* SymbolTable::getClass(int classId) const
{
	if (classId < 0 || classId > classes.size()) return NULL;
	return classes[classId];
}

ZClass* SymbolTable::createClass(string const& name)
{
	ZClass* klass = new ZClass(*this, name, classes.size());
	classes.push_back(klass);
	return klass;
}

// Variables

ZVarTypeId SymbolTable::getVarTypeId(int varId) const
{
    map<int, ZVarTypeId>::const_iterator it = varTypes.find(varId);

    if (it == varTypes.end())
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

ZVarType* SymbolTable::getVarType(int varId) const
{
	ZVarTypeId id = getVarTypeId(varId);
	if (id == -1) return NULL;
	return getType(id);
}

ZVarType* SymbolTable::getVarType(AST* node) const
{
	ZVarTypeId id = getVarTypeId(node);
	if (id == -1) return NULL;
	return getType(id);
}

void SymbolTable::putVarTypeId(int varId, ZVarTypeId typeId)
{
	varTypes[varId] = typeId;
}

void SymbolTable::putVarType(int varId, ZVarType const& type)
{
	varTypes[varId] = getOrAssignTypeId(type);
}

// Inlined Constants

void SymbolTable::inlineConstant(int varId, long value)
{
	if (varId == -1) return; 
	
	map<int, long>::const_iterator it = inlinedConstants.find(varId);
	if (it != inlinedConstants.end())
	{
		box_out("Internal Error: Constant already inlined.");
		box_eol();
		return;
	}

	inlinedConstants[varId] = value;
}

void SymbolTable::inlineConstant(AST* node, long value)
{
	int varId = getNodeId(node);
	if (varId == -1)
	{
		box_out("Internal Error (inlineConstant): Node does not have id.");
		box_eol();
		return;
	}

	inlineConstant(varId, value);
}

bool SymbolTable::isInlinedConstant(int varId) const
{
	return inlinedConstants.find(varId) != inlinedConstants.end();
}

bool SymbolTable::isInlinedConstant(AST* node) const
{
	int varId = getNodeId(node);
	if (varId == -1)
	{
		box_out("Internal Error (isInlinedConstant): Node does not have id.");
		box_eol();
		return false;
	}

	return isInlinedConstant(varId);
}

long SymbolTable::getInlinedValue(int varId) const
{
	map<int, long>::const_iterator it = inlinedConstants.find(varId);
	if (it == inlinedConstants.end()) return 0L;
	return it->second;
}

long SymbolTable::getInlinedValue(AST* node) const
{
	return getInlinedValue(getNodeId(node));
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

void SymbolTable::putFuncTypes(int funcId, ZVarType const* returnType, vector<ZVarType const*> const& paramTypes)
{
	ZVarTypeId returnTypeId = getOrAssignTypeId(*returnType);
	vector<ZVarTypeId> paramTypeIds;
	for (vector<ZVarType const*>::const_iterator it = paramTypes.begin(); it != paramTypes.end(); ++it)
		paramTypeIds.push_back(getOrAssignTypeId(**it));
	funcTypes[funcId] = FunctionTypeIds(returnTypeId, paramTypeIds);
}

//

void SymbolTable::printDiagnostics()
{
    cout << (unsigned int)varTypes.size() << " variable symbols" << endl;
    cout << (unsigned int)funcTypes.size() << " function symbols" << endl;
}

////////////////////////////////////////////////////////////////
// FunctionData

FunctionData::FunctionData(SymbolData const& symbolData)
	: program(symbolData.program)
{
	newGlobalVars = symbolData.globalVars;
	newGlobalArrays = symbolData.globalArrays;
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


