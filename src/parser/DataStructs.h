#ifndef ZSCRIPT_DATASTRUCTS_H
#define ZSCRIPT_DATASTRUCTS_H

#include <vector>
#include "AST.h"
#include "ZScript.h"

namespace ZScript
{
	class ZClass;
}

class FunctionTypeIds
{
public:
	FunctionTypeIds() : returnTypeId(-1), paramTypeIds() {}
	FunctionTypeIds(ZScript::DataTypeId returnTypeId,
	                std::vector<ZScript::DataTypeId> const& paramTypeIds);
	int compare(FunctionTypeIds const& other) const;
	bool operator==(FunctionTypeIds const& other) const {
		return compare(other) == 0;}
	bool operator<(FunctionTypeIds const& other) const {
		return compare(other) < 0;}
	ZScript::DataTypeId returnTypeId;
	std::vector<ZScript::DataTypeId> paramTypeIds;
	static FunctionTypeIds const null;
};

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
    std::vector<Opcode *> globalsInit;
};

struct OpcodeContext
{
	OpcodeContext(ZScript::TypeStore*);
	ZScript::TypeStore* typeStore;
	std::vector<Opcode*> initCode;
};

#endif

