#ifndef ZPARSER_SEMANTIC_ANALYZER_H
#define ZPARSER_SEMANTIC_ANALYZER_H

#include "AST.h"
#include "DataStructs.h"
#include "UtilVisitors.h"
#include "ZScript.h"

class SemanticAnalyzer : public RecursiveVisitor
{
public:
	SemanticAnalyzer(ZScript::Program& program);

	////////////////
	// Cases
	void caseProgram(ASTProgram& host, void* = NULL);
	// Statements
	void caseBlock(ASTBlock& host, void* = NULL);
	void caseStmtFor(ASTStmtFor& host, void* = NULL);
	// Declarations
	void caseTypeDef(ASTTypeDef& host, void* = NULL);
	void caseDataDeclList(ASTDataDeclList& host, void* = NULL);
	void caseDataDecl(ASTDataDecl& host, void* = NULL);
    void caseFuncDecl(ASTFuncDecl& host, void* = NULL);
    void caseScript(ASTScript& host, void* = NULL);
	// Expressions
	void caseExprConst(ASTExprConst& host, void* = NULL);
	void caseExprAssign(ASTExprAssign& host, void* = NULL);
	void caseExprCall(ASTExprCall& host, void* = NULL);
	void caseExprIdentifier(ASTExprIdentifier& host, void* = NULL);
	// void caseExprArrow(ASTExprArrow& host, void* = NULL); // No need to redefine.
	void caseExprIndex(ASTExprIndex& host, void* = NULL);
	// Literals
    void caseNumberLiteral(ASTNumberLiteral& host, void* = NULL);
    void caseBoolLiteral(ASTBoolLiteral& host, void* = NULL);
	void caseStringLiteral(ASTStringLiteral& host, void* = NULL);
	void caseArrayLiteral(ASTArrayLiteral& host, void* = NULL);

	////////////////
	bool hasFailed() const {return failure;}

private:
	Scope* scope;
	ZScript::Program& program;

	bool deprecateGlobals;

	void analyzeFunctionInternals(ZScript::Function& function);
};

#endif

