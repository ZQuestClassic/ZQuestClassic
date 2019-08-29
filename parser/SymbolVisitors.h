#ifndef SYMBOLVISITORS_H
#define SYMBOLVISITORS_H

#include "AST.h"
#include "UtilVisitors.h"

//builds the global symbols (functions and variables) for a script.
//param should be a pair<Scope, SymbolTable> pointer.
class BuildScriptSymbols : public ASTVisitor
{
public:
	BuildScriptSymbols() : failure(false) {}
	virtual void caseDefault(void *param);
	virtual void caseScript(ASTScript &host,void *param);
	virtual void caseVarDecl(ASTVarDecl &host, void *param);
	virtual void caseVarDeclInitializer(ASTVarDeclInitializer &host, void *param)
	{
		caseVarDecl(host, param);
	}
	virtual void caseFuncDecl(ASTFuncDecl &host, void *param);
	bool isOK() {return !failure;}
private:
	bool failure;
};

class BuildFunctionSymbols : public RecursiveVisitor
{
public:
	BuildFunctionSymbols() : failure(false) {}
	virtual void caseDefault(void *param) {param=param; /*these are here to bypass compiler warnings about unused arguments*/  }
	virtual void caseFuncDecl(ASTFuncDecl &host, void *param);
	virtual void caseVarDecl(ASTVarDecl &host, void *param);
	virtual void caseVarDeclInitializer(ASTVarDeclInitializer &host, void *param)
	{
		host.getInitializer()->execute(*this,param);
		caseVarDecl(host, param);
	}
	virtual void caseBlock(ASTBlock &host, void *param);
	virtual void caseStmtFor(ASTStmtFor &host, void *param);
	virtual void caseFuncCall(ASTFuncCall &host, void *param);
	virtual void caseExprDot(ASTExprDot &host, void *param);
	virtual void caseExprArrow(ASTExprArrow &host, void *param);
	bool isOK() {return !failure;}
private:
	bool failure;
};

#endif
