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
	SymbolTable(map<string, long> *consts) : varTypes(), funcTypes(), astToID(), funcParams(), constants(consts) {}
	int getVarType(int varID);
	int getFuncType(int funcID) {return funcTypes[funcID];}
	void putVar(int ID, int type) {varTypes[ID]=type;}
	void putFunc(int ID, int type);
	void putFuncDecl(int ID, vector<int> params) {funcParams[ID]=params;}
	void putAST(AST *obj, int ID);
	void putAmbiguousFunc(AST *func, vector<int> possibleIDs) {astToAmbiguousFuncIDs[func]=possibleIDs;}
	int getVarType(AST *obj);
	int getFuncType(AST *obj);
	vector<int> getFuncParams(int funcID) {return funcParams[funcID];}
	vector<int> getAmbiguousFuncs(AST *func) {return astToAmbiguousFuncIDs[func];}
	int getID(AST *obj) {return astToID[obj];}
	void printDiagnostics();
	vector<int> &getGlobalPointers(void) {return globalPointers;}
	void addGlobalPointer(int vid) {globalPointers.push_back(vid);}
	bool isConstant(string name);
	long getConstantVal(string name);
private:
	map<int, int> varTypes;
	map<int, int> funcTypes;
	map<AST *, int> astToID;
	map<AST *, vector<int> > astToAmbiguousFuncIDs;
	map<int, vector<int> > funcParams;
	vector<int> globalPointers;
	map<string, long> *constants;
};

class Scope
{
public:
	Scope(Scope *Parent) : namedChildren(), parent(Parent), vars(), funcs() {}
	~Scope();
	VariableSymbols &getVarSymbols() {return vars;}
	FunctionSymbols &getFuncSymbols() {return funcs;}
	bool addNamedChild(string name, Scope *child);
	int getVarInScope(string nspace, string name);
	vector<int> getFuncsInScope(string nspace, string name);
	Scope *getNamedChild(string name);
private:
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
	map<ASTScript *, int> scriptTypes;
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
	map<string, int> scriptTypes;
	map<string, int> thisPtr;
};

struct IntermediateData
{
	map<int, vector<Opcode *> > funcs;
	vector<Opcode *> globalsInit;
	vector<Opcode *> globalasInit;
	map<string, int> scriptRunLabels;
	map<string, int> numParams;
	map<string, int> scriptTypes;
	map<string, int> thisPtr;
};

class LinkTable
{
public:
	int functionToLabel(int fid);
	int getGlobalID(int vid);
	int addGlobalVar(int vid);
	void addGlobalPointer(int vid) {globalIDs[vid]=0;}
private:
	map<int, int> funcLabels;
	map<int, int> globalIDs;
};

class StackFrame
{
public:
	void addToFrame(int vid, int offset) {stackoffset[vid] = offset;}
	int getOffset(int vid);
private:
	map<int, int> stackoffset;
};

struct OpcodeContext
{
	StackFrame *stackframe;
	LinkTable *linktable;
	SymbolTable *symbols;
};

struct BFSParam
{
	Scope *scope;
	SymbolTable *table;
	int type;
};

#endif
 
