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
	void caseProgram(ASTProgram& host);
	// Statements
	void caseBlock(ASTBlock& host);
	void caseStmtFor(ASTStmtFor& host);
	// Declarations
	void caseTypeDef(ASTTypeDef& host);
    void caseVarDecl(ASTVarDecl& host);
    void caseVarDeclInitializer(ASTVarDeclInitializer& host);
    void caseArrayDecl(ASTArrayDecl& host);
    void caseFuncDecl(ASTFuncDecl& host);
    void caseScript(ASTScript& host);
	// Expressions
	void caseExprAssign(ASTExprAssign& host);
	void caseExprCall(ASTExprCall& host);
	void caseExprIdentifier(ASTExprIdentifier& host);
	// void caseExprArrow(ASTExprArrow& host); // No need to redefine.
	void caseExprIndex(ASTExprIndex& host);
	// Literals
	void caseStringLiteral(ASTStringLiteral& host);

	////////////////
	bool hasFailed() const {return failure;}

private:
	bool failure;
	Scope* scope;
	ZScript::Program& program;

	bool deprecateGlobals;

	void analyzeFunctionInternals(ZScript::Function& function);
};

#endif

