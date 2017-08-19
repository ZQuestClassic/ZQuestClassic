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

class FunctionTypeIds
{
public:
	FunctionTypeIds() : returnTypeId(-1), paramTypeIds() {}
	FunctionTypeIds(ZVarTypeId returnTypeId, vector<ZVarTypeId> const& paramTypeIds);
	int compare(FunctionTypeIds const& other) const;
	bool operator==(FunctionTypeIds const& other) const {return compare(other) == 0;}
	bool operator<(FunctionTypeIds const& other) const {return compare(other) < 0;}
	ZVarTypeId returnTypeId;
	vector<ZVarTypeId> paramTypeIds;
	static FunctionTypeIds const null;
};

class SymbolTable
{
public:
    SymbolTable();
	~SymbolTable();
	// Types
	ZVarType* getType(ZVarTypeId typeId) const;
	ZVarTypeId getTypeId(ZVarType const& type) const;
	ZVarTypeId assignTypeId(ZVarType const& type);
	ZVarTypeId getOrAssignTypeId(ZVarType const& type);

	template <typename Type>
	Type const* getCanonicalType(Type const& type)
	{
		return static_cast<Type const*>(
				types[getOrAssignTypeId(type)]);
	}
	
	// Classes
	ZScript::ZClass* getClass(int classId) const;
	ZScript::ZClass* createClass(string const& name);

private:
	vector<ZVarType*> types;
	map<ZVarType*, ZVarTypeId, ZVarType::PointerLess> typeIds;
	vector<ZScript::ZClass*> classes;
};

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
    map<int, vector<Opcode *> > funcs;
    vector<Opcode *> globalsInit;
};

struct OpcodeContext
{
    SymbolTable *symbols;
	vector<Opcode*> initCode;
};

#endif

