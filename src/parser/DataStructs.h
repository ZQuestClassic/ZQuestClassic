#ifndef DATASTRUCTS_H //2.53 Updated to 16th Jan, 2017
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

class VariableSymbols
{
public:
    VariableSymbols() : symbols() {}
    int addVariable(string name, int type);
    bool containsVariable(string name);
    int getID(string name);
private:
    map<string, pair<int, int> > symbols;
};

class FunctionSymbols
{
public:
    FunctionSymbols() : symbols(), ambiguous() {}
    int addFunction(string name, int rettype, vector<int> paramtype);
    bool containsFunction(string name, vector<int> &params);
    int getID(string name, vector<int> &params);
    vector<int> getFuncIDs(string name);
private:
    map<pair<string, vector<int> >, pair<int,int> > symbols;
    map<string, vector<int> > ambiguous;
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
    void putVarTypeId(int ID, int type);
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
	Scope(SymbolTable& table) : table(table), namedChildren(), parent(NULL), vars(), funcs() {}
	Scope(Scope* parent) : table(parent->table), namedChildren(), parent(parent), vars(), funcs() {}
    ~Scope();
	SymbolTable const& getTable() const {return table;}
	SymbolTable& getTable() {return table;}
    VariableSymbols &getVarSymbols()
    {
        return vars;
    }
    FunctionSymbols &getFuncSymbols()
    {
        return funcs;
    }
    bool addNamedChild(string name, Scope *child);
    int getVarInScope(string nspace, string name);
    vector<int> getFuncsInScope(string nspace, string name);
    Scope *getNamedChild(string name);
private:
	SymbolTable& table;
    map<string, Scope *> namedChildren;
    Scope *parent;
    VariableSymbols vars;
    FunctionSymbols funcs;
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

