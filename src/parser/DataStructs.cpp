
#include "../precompiled.h" //always first
#include <assert.h>
#include <iostream>
#include "../zsyssimple.h"
#include "CompileError.h"
#include "DataStructs.h"
#include "GlobalSymbols.h"
#include "Scope.h"
#include "Types.h"
#include "ZScript.h"

using std::cout;
using std::endl;
using namespace ZScript;

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
	for (int i = 0; i < (int)paramTypeIds.size(); ++i)
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

// Types

ZVarType* SymbolTable::getType(ZVarTypeId typeId) const
{
	if (typeId < 0 || typeId > (int)types.size()) return NULL;
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
	if (!type.isResolved())
	{
		CompileError::UnresolvedType.print(NULL, type.getName());
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
		CompileError::UnresolvedType.print(NULL, type.getName());
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
	if (classId < 0 || classId > (int)classes.size()) return NULL;
	return classes[classId];
}

ZClass* SymbolTable::createClass(string const& name)
{
	ZClass* klass = new ZClass(*this, name, classes.size());
	classes.push_back(klass);
	return klass;
}

////////////////////////////////////////////////////////////////
// FunctionData

FunctionData::FunctionData(Program& program)
	: program(program),
	  globalData(program.getScope().getLocalData())
{
	for (vector<Datum*>::const_iterator it = globalData.begin();
	     it != globalData.end(); ++it)
	{
		Datum& datum = **it;
		if (!datum.getCompileTimeValue())
			globalVariables.push_back(&datum);
	}

	for (vector<Script*>::const_iterator it = program.scripts.begin();
	     it != program.scripts.end(); ++it)
	{
		ScriptScope& scope = (*it)->getScope();
		vector<Datum*> data = scope.getLocalData();
		globalVariables.insert(globalVariables.end(),
		                       data.begin(), data.end());
	}
}

////////////////////////////////////////////////////////////////
// IntermediateData

IntermediateData::IntermediateData(FunctionData const& functionData)
	: program(functionData.program)
{}

