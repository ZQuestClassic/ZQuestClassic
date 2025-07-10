#ifndef ZSCRIPT_DATASTRUCTS_H_
#define ZSCRIPT_DATASTRUCTS_H_

#include <vector>
#include "AST.h"
#include "ZScript.h"

namespace ZScript
{
	struct FunctionData
	{
		FunctionData(ZScript::Program& program);
		ZScript::Program& program;
		std::vector<ZScript::Datum*> globalData;
		std::vector<ZScript::Datum*> globalVariables;
	};

	struct IntermediateData
	{
		IntermediateData(FunctionData const& functionData);
		ZScript::Program& program;
		std::vector<std::shared_ptr<Opcode>> globalsInit;
	};

	struct OpcodeContext
	{
		OpcodeContext(ZScript::TypeStore*);
		ZScript::TypeStore* typeStore;
		std::vector<std::shared_ptr<Opcode>> initCode;
	};
}

#endif

