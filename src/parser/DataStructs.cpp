#include <assert.h>
#include <iostream>
#include "zsyssimple.h"
#include "CompileError.h"
#include "DataStructs.h"
#include "LibrarySymbols.h"
#include "Scope.h"
#include "Types.h"
#include "ZScript.h"

using namespace ZScript;

////////////////////////////////////////////////////////////////
// FunctionData

FunctionData::FunctionData(Program& program)
	: program(program),
	  globalData(program.getScope().getLocalData())
{
	for(Datum* datum : globalData)
	{
		if (!datum->getCompileTimeValue())
			globalVariables.push_back(datum);
	}

	for (Script* script : program.scripts)
	{
		ScriptScope& scope = script->getScope();
		for(Datum* datum : scope.getLocalData())
		{
			if (!datum->getCompileTimeValue())
				globalVariables.push_back(datum);
		}
	}
	
	for (Namespace* namesp : program.namespaces)
	{
		NamespaceScope& scope = namesp->getScope();
		for(Datum* datum : scope.getLocalData())
		{
			if (!datum->getCompileTimeValue())
				globalVariables.push_back(datum);
		}
	}
}

////////////////////////////////////////////////////////////////
// IntermediateData

IntermediateData::IntermediateData(FunctionData const& functionData)
	: program(functionData.program)
{}

////////////////////////////////////////////////////////////////
// OpcodeContext

OpcodeContext::OpcodeContext(ZScript::TypeStore* typeStore)
	: typeStore(typeStore)
{}

