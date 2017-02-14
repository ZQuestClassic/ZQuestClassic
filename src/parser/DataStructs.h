#ifndef DATASTRUCTS_H
#define DATASTRUCTS_H

#include "AST.h"
#include <string>
#include <map>
#include <vector>

using std::string;
using std::map;
using std::vector;
using std::pair;

class FunctionSignature
{
public:
	FunctionSignature(string const& name, vector<ZVarTypeId> const& paramTypeIds);
	int compare(FunctionSignature const& other) const;
	bool operator==(FunctionSignature const& other) const {return compare(other) == 0;}
	bool operator<(FunctionSignature const& other) const {return compare(other) < 0;}
	string name;
	vector<ZVarTypeId> paramTypeIds;
};

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
    SymbolTable(map<string, long> *consts);
	~SymbolTable();
	// Nodes
    int getNodeId(AST* node) const;
    void putNodeId(AST* node, int id);
    vector<int> getPossibleNodeFuncIds(AST* node) const;
    void putPossibleNodeFuncIds(AST* node, vector<int> possibleFuncIds);
	// Types
	ZVarType* getType(ZVarTypeId typeId) const;
	ZVarTypeId getTypeId(ZVarType const& type) const;
	ZVarTypeId assignTypeId(ZVarType const& type);
	ZVarTypeId getOrAssignTypeId(ZVarType const& type);
	// Variables
    ZVarTypeId getVarTypeId(int varId) const;
    ZVarTypeId getVarTypeId(AST* node) const;
    void putVarTypeId(int varId, ZVarTypeId typeId);
    void putVarType(int varId, ZVarType const& type);
	// Functions
    ZVarTypeId getFuncReturnTypeId(int funcId) const;
    ZVarTypeId getFuncReturnTypeId(AST *node) const;
    vector<ZVarTypeId> getFuncParamTypeIds(int funcId) const;
    void putFuncTypeIds(int funcId, ZVarTypeId returnTypeId, vector<ZVarTypeId> const& paramTypeIds);
	// Global Pointers
    vector<int> const& getGlobalPointers() const {return globalPointers;}
    vector<int>& getGlobalPointers() {return globalPointers;}
    void addGlobalPointer(int varId) {globalPointers.push_back(varId);}
	// Other
    bool isConstant(string name) const;
    long getConstantVal(string name) const;
    void printDiagnostics();
private:
    map<AST*, int> nodeIds;
	vector<ZVarType*> types;
	map<ZVarType*, ZVarTypeId, ZVarType::PointerLess> typeIds;
    map<AST*, vector<int> > possibleNodeFuncIds;
    map<int, ZVarTypeId> varTypes;
    map<int, FunctionTypeIds> funcTypes;
    vector<int> globalPointers;
    map<string, long> *constants;
};

class Scope
{
public:
	Scope(SymbolTable& table);
	Scope(Scope* parent);
	~Scope();
	// SymbolTable
	SymbolTable const& getTable() const {return table;}
	SymbolTable& getTable() {return table;}
	// Children
	Scope* makeChild(string const& name);
	Scope* getChild(string const& name) const;
	Scope& getOrMakeChild(string const& name);
	// Variables
	int getVarId(string const& nspace, string const& name) const;
	int getVarId(vector<string> const& names) const;
	int getVarId(string const& name) const;
	int addVar(string const& name, ZVarTypeId typeId, AST* node);
	int addVar(string const& name, ZVarType const& type, AST* node);
	int addVar(string const& name, ZVarTypeId typeId);
	int addVar(string const& name, ZVarType const& type);
	// Functions
	vector<int> getFuncIds(string const& nspace, string const& name) const;
	vector<int> getFuncIds(vector<string> const& names) const;
	vector<int> getFuncIds(string const& name) const;
	int getFuncId(string const& nspace, FunctionSignature const& signature) const;
	int getFuncId(vector<string> const& names, FunctionSignature const& signature) const;
	int getFuncId(FunctionSignature const& signature) const;
	int addFunc(string const& name, ZVarTypeId returnTypeId, vector<ZVarTypeId> const& paramTypeIds, AST* node);
	int addFunc(string const& name, ZVarType const& returnType, vector<ZVarType*> const& paramTypes, AST* node);
	int addFunc(string const& name, ZVarTypeId returnTypeId, vector<ZVarTypeId> const& paramTypeIds);
	int addFunc(string const& name, ZVarType const& returnType, vector<ZVarType*> const& paramTypes);
private:
	SymbolTable& table;
    Scope* parent;
    map<string, Scope*> children;
	map<string, int> variables;
	map<string, vector<int> > functionsByName;
	map<FunctionSignature, int> functionsBySignature;

	void getFuncIds(vector<int>& ids, vector<string> const& names) const;
	void getFuncIds(vector<int>& ids, string const& name) const;
};

struct SymbolData
{
    SymbolTable *symbols;
    vector<ASTFuncDecl *> globalFuncs;
    vector<ASTVarDecl *> globalVars;
    vector<ASTArrayDecl *> globalArrays;
    vector<ASTScript *> scripts;
    map<ASTScript *, int> runsymbols;
    map<ASTScript *, int> numParams;
    map<ASTScript *, ScriptType> scriptTypes;
    map<ASTScript *, int> thisPtr;
};

struct FunctionData
{
    SymbolTable *symbols;
    vector<ASTFuncDecl *> functions;
    vector<ASTVarDecl *> globalVars;
    vector<ASTVarDecl *> newGlobalVars;
    vector<ASTArrayDecl *> globalArrays;
    vector<ASTArrayDecl *> newGlobalArrays;
    map<string, int> scriptRunSymbols;
    map<string, int> numParams;
    map<string, ScriptType> scriptTypes;
    map<string, int> thisPtr;
};

struct IntermediateData
{
    map<int, vector<Opcode *> > funcs;
    vector<Opcode *> globalsInit;
    vector<Opcode *> globalasInit;
    map<string, int> scriptRunLabels;
    map<string, int> numParams;
    map<string, ScriptType> scriptTypes;
    map<string, int> thisPtr;
};

class LinkTable
{
public:
    int functionToLabel(int fid);
    int getGlobalID(int vid);
    int addGlobalVar(int vid);
    void addGlobalPointer(int vid)
    {
        globalIDs[vid]=0;
    }
private:
    map<int, int> funcLabels;
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

struct BFSParam
{
	BFSParam(Scope& scope) : scope(scope), type(SCRIPTTYPE_VOID) {}
	BFSParam(Scope& scope, ScriptType type) : scope(scope), type(type) {}
    Scope& scope;
    ScriptType type;
};

#endif

