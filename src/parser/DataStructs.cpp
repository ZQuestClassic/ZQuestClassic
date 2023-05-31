//2.53 Updated to 16th Jan, 2017
#include <assert.h>
#include <iostream>
#include "zsyssimple.h"
#include "CompileError.h"
#include "DataStructs.h"
#include "LibrarySymbols.h"
#include "Scope.h"
#include "Types.h"
#include "ZScript.h"

using std::cout;
using std::endl;
using namespace ZScript;

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
	
	for (vector<Namespace*>::const_iterator it = program.namespaces.begin();
		it != program.namespaces.end(); ++it)
	{
		NamespaceScope& scope = (*it)->getScope();
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

////////////////////////////////////////////////////////////////
// OpcodeContext

OpcodeContext::OpcodeContext(ZScript::TypeStore* typeStore)
	: typeStore(typeStore)
{}

