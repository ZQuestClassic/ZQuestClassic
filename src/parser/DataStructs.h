#ifndef DATASTRUCTS_H
#define DATASTRUCTS_H

#include <map>
#include <string>
#include <vector>
#include "AST.h"
#include "ZScript.h"

using std::string;
using std::map;
using std::vector;
using std::pair;

namespace ZScript
{
	class ZClass;
}

struct FunctionData
{
	FunctionData(ZScript::Program& program);
	ZScript::Program& program;
	vector<ZScript::Datum*> globalData;
	vector<ZScript::Datum*> globalVariables;
};

struct IntermediateData
{
	IntermediateData(FunctionData const& functionData);
	ZScript::Program& program;
    vector<Opcode> globalsInit;
};

struct OpcodeContext
{
    TypeStore* typeStore;
	vector<Opcode> initCode;
};

#endif

