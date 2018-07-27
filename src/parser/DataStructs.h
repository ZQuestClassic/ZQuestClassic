#ifndef DATASTRUCTS_H //2.53 Updated to 16th Jan, 2017
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
	// Nodes
    int getNodeId(AST* node) const;
    void putNodeId(AST* node, int id);
    vector<int> getPossibleNodeFuncIds(AST* node) const;
    void putPossibleNodeFuncIds(AST* node, vector<int> possibleFuncIds);
	// Types
	ZVarType* getType(ZVarTypeId typeId) const;
	ZVarType* getType(AST* node) const;
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
	// Variables
    ZVarTypeId getVarTypeId(int varId) const;
    ZVarTypeId getVarTypeId(AST* node) const;
    ZVarType* getVarType(int varId) const;
    ZVarType* getVarType(AST* node) const;
    void putVarTypeId(int varId, ZVarTypeId typeId);
    void putVarType(int varId, ZVarType const& type);
	// Inlined Constants
	void inlineConstant(int varId, long value);
	void inlineConstant(AST* node, long value);
	bool isInlinedConstant(int varId) const;
	bool isInlinedConstant(AST* node) const;
	long getInlinedValue(int varId) const;
	long getInlinedValue(AST* node) const;
	// Functions
    ZVarTypeId getFuncReturnTypeId(int funcId) const;
    ZVarTypeId getFuncReturnTypeId(AST *node) const;
    vector<ZVarTypeId> getFuncParamTypeIds(int funcId) const;
    void putFuncTypeIds(int funcId, ZVarTypeId returnTypeId,
                        vector<ZVarTypeId> const& paramTypeIds);
    void putFuncTypes(int funcId, ZVarType const* returnType,
                      vector<ZVarType const*> const& paramTypes);
	// Global Pointers
    vector<int> const& getGlobalPointers() const {return globalPointers;}
    vector<int>& getGlobalPointers() {return globalPointers;}
    void addGlobalPointer(int varId) {globalPointers.push_back(varId);}
	// Other
    void printDiagnostics();
private:
    map<AST*, int> nodeIds;
	vector<ZVarType*> types;
	map<ZVarType*, ZVarTypeId, ZVarType::PointerLess> typeIds;
	vector<ZScript::ZClass*> classes;
    map<AST*, vector<int> > possibleNodeFuncIds;
    map<int, ZVarTypeId> varTypes;
    map<int, long> inlinedConstants;
    map<int, FunctionTypeIds> funcTypes;
    vector<int> globalPointers;
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
    map<string, int> scriptRunLabels;
};

class LinkTable
{
public:
    int getGlobalID(int vid);
    int addGlobalVar(int vid);
    void addGlobalPointer(int vid)
    {
        globalIDs[vid]=0;
    }
private:
    map<int, int> globalIDs;
};

class StackFrame
{
public:
    void addToFrame(int vid, int offset)
    {
        stackoffset[vid] = offset;
    }
    int getOffset(int vid);
private:
    map<int, int> stackoffset;
};

struct OpcodeContext
{
    StackFrame *stackframe;
    LinkTable *linktable;
    SymbolTable *symbols;
	vector<Opcode*> initCode;
};

#endif

