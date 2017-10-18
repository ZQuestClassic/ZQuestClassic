#ifndef SYMBOLVISITORS_H
#define SYMBOLVISITORS_H

#include "AST.h"
#include "UtilVisitors.h"

//builds the global symbols (functions and variables) for a script.
//param should be a pair<Scope, SymbolTable> pointer.
class BuildScriptSymbols : public RecursiveVisitor
{
public:
    BuildScriptSymbols() : failure(false), deprecateGlobals(false) {}
    virtual void caseDefault(void *);
    virtual void caseScript(ASTScript &host,void *param);
    virtual void caseArrayDecl(ASTArrayDecl &host, void *param);
    virtual void caseVarDecl(ASTVarDecl &host, void *param);
    virtual void caseVarDeclInitializer(ASTVarDeclInitializer &host, void *param);
    virtual void caseFuncDecl(ASTFuncDecl &host, void *param);
    virtual void caseExprDot(ASTExprDot &host, void *param);
    virtual void caseExprArrow(ASTExprArrow &host, void *param);
    virtual void caseExprArray(ASTExprArray &host, void *param);
    virtual void caseFuncCall(ASTFuncCall &host, void *param);
    bool isOK()
    {
        return !failure;
    }
    void enableDeprecationWarnings()
    {
        deprecateGlobals = true;
    }
private:
    bool failure;
    bool deprecateGlobals;
};

class BuildFunctionSymbols : public RecursiveVisitor
{
public:
    BuildFunctionSymbols() : failure(false), thisvid(-1) {}
    virtual void caseDefault(void *param)
    {
        param=param; /*these are here to bypass compiler warnings about unused arguments*/
    }
    virtual void caseFuncDecl(ASTFuncDecl &host, void *param);
    virtual void caseVarDecl(ASTVarDecl &host, void *param);
    virtual void caseArrayDecl(ASTArrayDecl &host, void *param);
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
    virtual void caseExprArray(ASTExprArray &host, void *param);
    bool isOK()
    {
        return !failure;
    }
    int getThisVID()
    {
        return thisvid;
    }
private:
    bool failure;
    int thisvid;
};

#endif

