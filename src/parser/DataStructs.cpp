
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

