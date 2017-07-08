#ifndef ZSCRIPT_ASTVISITORS_H
#define ZSCRIPT_ASTVISITORS_H

#include "AST.h"
#include "CompileError.h"

////////////////////////////////////////////////////////////////
// Standard AST Visitor.

class ASTVisitor
{
public:
    virtual ~ASTVisitor() {}
    virtual void caseDefault(void *param) = 0;
	// AST Subclasses
    virtual void caseProgram(ASTProgram&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseFloat(ASTFloat&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseString(ASTString&, void* param = NULL) {
		caseDefault(param);}
	// Statements
    virtual void caseBlock(ASTBlock&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseStmtIf(ASTStmtIf&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseStmtIfElse(ASTStmtIfElse&, void* param = NULL) {
		caseDefault(param);}
	virtual void caseStmtSwitch(ASTStmtSwitch&, void* param = NULL) {
		caseDefault(param);}
	virtual void caseSwitchCases(ASTSwitchCases&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseStmtFor(ASTStmtFor&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseStmtWhile(ASTStmtWhile&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseStmtDo(ASTStmtDo&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseStmtReturn(ASTStmtReturn&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseStmtReturnVal(ASTStmtReturnVal&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseStmtBreak(ASTStmtBreak&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseStmtContinue(ASTStmtContinue&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseStmtEmpty(ASTStmtEmpty&, void* param = NULL) {
		caseDefault(param);}
	virtual void caseStmtCompileError(ASTStmtCompileError& node, void* param = NULL) {
		caseDefault(param);}
	// Declarations
    virtual void caseScript(ASTScript&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseImportDecl(ASTImportDecl&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseFuncDecl(ASTFuncDecl&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseDataDeclList(ASTDataDeclList&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseDataDecl(ASTDataDecl&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseDataDeclExtraArray(
			ASTDataDeclExtraArray&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseTypeDef(ASTTypeDef&, void* param = NULL) {
		caseDefault(param);}
	// Expressions
    virtual void caseExprConst(ASTExprConst&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprAssign(ASTExprAssign&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprIdentifier(ASTExprIdentifier&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprArrow(ASTExprArrow&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprIndex(ASTExprIndex&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprCall(ASTExprCall&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprNegate(ASTExprNegate&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprNot(ASTExprNot&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprBitNot(ASTExprBitNot&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprIncrement(ASTExprIncrement&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprPreIncrement(
			ASTExprPreIncrement&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprDecrement(ASTExprDecrement&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprPreDecrement(ASTExprPreDecrement&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprAnd(ASTExprAnd&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprOr(ASTExprOr&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprGT(ASTExprGT&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprGE(ASTExprGE&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprLT(ASTExprLT&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprLE(ASTExprLE&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprEQ(ASTExprEQ&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprNE(ASTExprNE&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprPlus(ASTExprPlus&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprMinus(ASTExprMinus&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprTimes(ASTExprTimes&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprDivide(ASTExprDivide&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprModulo(ASTExprModulo&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprBitAnd(ASTExprBitAnd&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprBitOr(ASTExprBitOr&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprBitXor(ASTExprBitXor&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprLShift(ASTExprLShift&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprRShift(ASTExprRShift&, void* param = NULL) {
		caseDefault(param);}
	// Literals
    virtual void caseNumberLiteral(ASTNumberLiteral&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseBoolLiteral(ASTBoolLiteral&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseStringLiteral(ASTStringLiteral&, void* param = NULL) {
		caseDefault(param);}
	virtual void caseArrayLiteral(ASTArrayLiteral& node, void* param = NULL) {
		caseDefault(param);}
	// Types
	virtual void caseScriptType(ASTScriptType&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseVarType(ASTVarType&, void* param = NULL) {
		caseDefault(param);}
};

////////////////////////////////////////////////////////////////
// AST Visitor that recurses on elements and handles errors.

class RecursiveVisitor : public ASTVisitor, public CompileErrorHandler
{
public:
	RecursiveVisitor() : failure(false), currentCompileError(NULL) {}
	
	// If any errors have occured.
	bool hasFailed() const {return failure;}

	// Mark as having failed.
	void fail() {failure = true;}
	
	// Used to signal that a compile error has occured.
	void handleError(CompileError const& error, AST const* node, ...);
	
    virtual void caseDefault(void*) {}
	virtual void caseStmtCompileError(
			ASTStmtCompileError& host, void* param = NULL);

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

	// A stack of active ASTStmtCompileErrors.
	vector<ASTStmtCompileError*> compileErrorHandlers;

	// The current compile error. While this is set breakRecursion will return
	// true. This lets us move up the call stack until we reach the proper
	// handler for it.
	CompileError const* currentCompileError;

	// Set to true if any errors have occured.
	bool failure;
	
};

#endif

