#ifndef ZPARSER_SEMANTIC_ANALYZER_H
#define ZPARSER_SEMANTIC_ANALYZER_H

#include "AST.h"
#include "DataStructs.h"
#include "UtilVisitors.h"

class SemanticAnalyzer : public RecursiveVisitor
{
public:
	SemanticAnalyzer();

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
	void caseStringConstant(ASTStringConstant& host);
	void caseFuncCall(ASTFuncCall& host);
	void caseExprDot(ASTExprDot& host);
	// void caseExprArrow(ASTExprArrow& host); // No need to redefine.
	// void caseExprIndex(ASTExprIndex& host); // No need to redefine.

	////////////////
	bool hasFailed() const {return failure;}
	SymbolData& getResults() {return results;}

private:
	bool failure;
	SymbolTable data;
	SymbolData results;
	Scope* scope;

	bool deprecateGlobals;

	void analyzeFunctionInternals(ASTFuncDecl& function) {analyzeFunctionInternals(NULL, function);}
	void analyzeFunctionInternals(ASTScript* script, ASTFuncDecl& function);
};

#endif

