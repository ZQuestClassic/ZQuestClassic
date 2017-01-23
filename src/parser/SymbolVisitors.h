#ifndef SYMBOLVISITORS_H //2.53 Updated to 16th Jan, 2017
#define SYMBOLVISITORS_H

#include "AST.h"
#include "UtilVisitors.h"

//builds the global symbols (functions and variables) for a script.
//param should be a pair<Scope, SymbolTable> pointer.
class BuildScriptSymbols : public RecursiveVisitor
{
public:
    BuildScriptSymbols() : failure(false), deprecateGlobals(false) {}
    void caseDefault(void *);
	// Declarations
    void caseScript(ASTScript &host,void *param);
    void caseFuncDecl(ASTFuncDecl &host, void *param);
    void caseArrayDecl(ASTArrayDecl &host, void *param);
    void caseVarDecl(ASTVarDecl &host, void *param);
    void caseVarDeclInitializer(ASTVarDeclInitializer &host, void *param);
	// Expressions
	void caseStringConstant(ASTStringConstant& host, void* param);
    void caseExprDot(ASTExprDot &host, void *param);
    void caseExprArrow(ASTExprArrow &host, void *param);
    void caseExprArray(ASTExprArray &host, void *param);
    void caseFuncCall(ASTFuncCall &host, void *param);

    bool isOK() {return !failure;}
    void enableDeprecationWarnings() {deprecateGlobals = true;}
private:
    bool failure;
    bool deprecateGlobals;
};

class BuildFunctionSymbols : public RecursiveVisitor
{
public:
    BuildFunctionSymbols() : failure(false), thisvid(-1) {}

    void caseDefault(void *) {}
	// Statements
    void caseBlock(ASTBlock &host, void *param);
    void caseStmtFor(ASTStmtFor &host, void *param);
	// Declarations
    void caseFuncDecl(ASTFuncDecl &host, void *param);
    void caseArrayDecl(ASTArrayDecl &host, void *param);
    void caseVarDecl(ASTVarDecl &host, void *param);
    void caseVarDeclInitializer(ASTVarDeclInitializer &host, void *param);
	// Expressions
	void caseStringConstant(ASTStringConstant& host, void* param);
    void caseFuncCall(ASTFuncCall &host, void *param);
    void caseExprDot(ASTExprDot &host, void *param);
    void caseExprArrow(ASTExprArrow &host, void *param);
    void caseExprArray(ASTExprArray &host, void *param);

    bool isOK() {return !failure;}
    int getThisVID() {return thisvid;}
private:
    bool failure;
    int thisvid;
};

#endif

