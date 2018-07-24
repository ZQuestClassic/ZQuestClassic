#ifndef UTILVISITORS_H //2.53 Updated to 16th Jan, 2017
#define UTILVISITORS_H

#include "AST.h"
#include "CompileError.h"

class RecursiveVisitor : public ASTVisitor, public CompileErrorHandler
{
public:
	RecursiveVisitor() : failure(false), currentCompileError(NULL) {}
	
	// If any errors have occured.
	bool hasFailed() const {return failure;}

	// Mark as having failed.
	void fail() {failure = true;}
	
	// Used to signal that a compile error has occured.
	void handleError(CompileError const& error, AST* node, ...);
	
    virtual void caseDefault(void*) {}
	virtual void caseCompileError(ASTCompileError& host, void* param = NULL);

    virtual void caseProgram(ASTProgram& host, void* param = NULL);
	// Statements
    virtual void caseBlock(ASTBlock& host, void* param = NULL);
    virtual void caseStmtIf(ASTStmtIf& host, void* param = NULL);
    virtual void caseStmtIfElse(ASTStmtIfElse& host, void* param = NULL);
	virtual void caseStmtSwitch(ASTStmtSwitch & host, void* param = NULL);
	virtual void caseSwitchCases(ASTSwitchCases & host, void* param = NULL);
    virtual void caseStmtFor(ASTStmtFor& host, void* param = NULL);
    virtual void caseStmtWhile(ASTStmtWhile& host, void* param = NULL);
    virtual void caseStmtDo(ASTStmtDo& host, void* param = NULL);
    virtual void caseStmtReturnVal(
			ASTStmtReturnVal& host, void* param = NULL);
	// Declarations
    virtual void caseScript(ASTScript& host, void* param = NULL);
    virtual void caseFuncDecl(ASTFuncDecl& host, void* param = NULL);
	virtual void caseDataDeclList(ASTDataDeclList& host, void* param = NULL);
	virtual void caseDataDecl(ASTDataDecl& host, void* param = NULL);
	virtual void caseDataDeclExtraArray(
			ASTDataDeclExtraArray& host, void* param = NULL);
    virtual void caseTypeDef(ASTTypeDef&, void* param = NULL);
	// Expressions
	virtual void caseExprConst(ASTExprConst& host, void* param = NULL);
    virtual void caseExprAssign(ASTExprAssign& host, void* param = NULL);
    virtual void caseExprArrow(ASTExprArrow& host, void* param = NULL);
    virtual void caseExprIndex(ASTExprIndex& host, void* param = NULL);
    virtual void caseExprCall(ASTExprCall& host, void* param = NULL);
    virtual void caseExprNegate(ASTExprNegate& host, void* param = NULL);
    virtual void caseExprNot(ASTExprNot& host, void* param = NULL);
    virtual void caseExprBitNot(ASTExprBitNot& host, void* param = NULL);
    virtual void caseExprIncrement(
			ASTExprIncrement& host, void* param = NULL);
    virtual void caseExprPreIncrement(
			ASTExprPreIncrement& host, void* param = NULL);
    virtual void caseExprDecrement(
			ASTExprDecrement& host, void* param = NULL);
    virtual void caseExprPreDecrement(
			ASTExprPreDecrement& host, void* param = NULL);
    virtual void caseExprAnd(ASTExprAnd& host, void* param = NULL);
    virtual void caseExprOr(ASTExprOr& host, void* param = NULL);
    virtual void caseExprGT(ASTExprGT& host, void* param = NULL);
    virtual void caseExprGE(ASTExprGE& host, void* param = NULL);
    virtual void caseExprLT(ASTExprLT& host, void* param = NULL);
    virtual void caseExprLE(ASTExprLE& host, void* param = NULL);
    virtual void caseExprEQ(ASTExprEQ& host, void* param = NULL);
    virtual void caseExprNE(ASTExprNE& host, void* param = NULL);
    virtual void caseExprPlus(ASTExprPlus& host, void* param = NULL);
    virtual void caseExprMinus(ASTExprMinus& host, void* param = NULL);
    virtual void caseExprTimes(ASTExprTimes& host, void* param = NULL);
    virtual void caseExprDivide(ASTExprDivide& host, void* param = NULL);
    virtual void caseExprModulo(ASTExprModulo& host, void* param = NULL);
    virtual void caseExprBitAnd(ASTExprBitAnd& host, void* param = NULL);
    virtual void caseExprBitOr(ASTExprBitOr& host, void* param = NULL);
    virtual void caseExprBitXor(ASTExprBitXor& host, void* param = NULL);
    virtual void caseExprLShift(ASTExprLShift& host, void* param = NULL);
    virtual void caseExprRShift(ASTExprRShift& host, void* param = NULL);
	// Literals
    virtual void caseNumberLiteral(
			ASTNumberLiteral& host, void* param = NULL);
	virtual void caseArrayLiteral(ASTArrayLiteral& host, void* param = NULL);

protected:

	// Returns true if we have failed or for some other reason must break out
	// of recursion. Should be passed the current node.
	bool breakRecursion(AST& host) const;

	// Recurse on a single node.
	void recurse(AST& node, void* param);
	// Recurse on a single node. Does nothing if it's null.
	void recurse(AST* node, void* param);
	// Recurse on a vector of nodes.
	template <class Node> void recurse(
			AST& host, void* param, vector<Node*> nodes);
	// Recurse on a list of nodes.
	template <class Node> void recurse(
			AST& host, void* param, list<Node*> nodes);

	// A stack of active ASTCompileErrors.
	vector<ASTCompileError*> compileErrorHandlers;

	// The current compile error. While this is set breakRecursion will return
	// true. This lets us move up the call stack until we reach the proper
	// handler for it.
	CompileError const* currentCompileError;

	// Set to true if any errors have occured.
	bool failure;
	
};

class CheckForExtraneousImports : public RecursiveVisitor
{
public:
    CheckForExtraneousImports() : ok(true) {}

    void caseDefault(void *param);
    void caseImportDecl(ASTImportDecl &host, void *param);

	bool isOK() const {return ok;}
private:
    bool ok;
};

class IsBlock : public ASTVisitor
{
public:
    void caseDefault(void *param) {*(bool *)param = false;}
    void caseBlock(ASTBlock &, void *param) {*(bool *)param = true;}
};

class IsFuncDecl : public ASTVisitor
{
public:
    void caseDefault(void *param) {*(bool *)param = false;}
    virtual void caseFuncDecl(ASTFuncDecl &, void *param) {*(bool *)param = true;}
};

class IsTypeDef : public ASTVisitor
{
public:
    void caseDefault(void *param) {*(bool*)param = false;}
    void caseTypeDef(ASTTypeDef &, void *param) {*(bool*)param = true;}
};

#endif

