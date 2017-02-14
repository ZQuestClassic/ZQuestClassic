
#include "../precompiled.h" //always first

#include "UtilVisitors.h"
#include <assert.h>
#include "../zsyssimple.h"
#include "ParseError.h"

////////////////////////////////////////////////////////////////
// RecursiveVisitor

void RecursiveVisitor::caseProgram(ASTProgram &host, void *param)
{
	host.getDeclarations()->execute(*this, param);
}

void RecursiveVisitor::caseProgram(ASTProgram &host)
{
	host.getDeclarations()->execute(*this);
}

// Statements

void RecursiveVisitor::caseBlock(ASTBlock &host, void *param)
{
	list<ASTStmt *> l = host.getStatements();

	for(list<ASTStmt *>::iterator it = l.begin(); it != l.end(); it++)
		(*it)->execute(*this,param);
}

void RecursiveVisitor::caseBlock(ASTBlock &host)
{
	list<ASTStmt *> l = host.getStatements();

	for(list<ASTStmt *>::iterator it = l.begin(); it != l.end(); it++)
		(*it)->execute(*this);
}

void RecursiveVisitor::caseStmtAssign(ASTStmtAssign &host, void *param)
{
	host.getLVal()->execute(*this, param);
	host.getRVal()->execute(*this, param);
}

void RecursiveVisitor::caseStmtAssign(ASTStmtAssign &host)
{
	host.getLVal()->execute(*this);
	host.getRVal()->execute(*this);
}

void RecursiveVisitor::caseStmtIf(ASTStmtIf &host, void *param)
{
	host.getCondition()->execute(*this, param);
	host.getStmt()->execute(*this, param);
}

void RecursiveVisitor::caseStmtIf(ASTStmtIf &host)
{
	host.getCondition()->execute(*this);
	host.getStmt()->execute(*this);
}

void RecursiveVisitor::caseStmtIfElse(ASTStmtIfElse &host, void *param)
{
	host.getCondition()->execute(*this, param);
	host.getStmt()->execute(*this, param);
	host.getElseStmt()->execute(*this, param);
}

void RecursiveVisitor::caseStmtIfElse(ASTStmtIfElse &host)
{
	host.getCondition()->execute(*this);
	host.getStmt()->execute(*this);
	host.getElseStmt()->execute(*this);
}

void RecursiveVisitor::caseStmtSwitch(ASTStmtSwitch & host, void* param)
{
	host.getKey()->execute(*this, param);

	vector<ASTSwitchCases*> & cases = host.getCases();
	for (vector<ASTSwitchCases*>::iterator it = cases.begin(); it != cases.end(); ++it)
		(*it)->execute(*this, param);
}

void RecursiveVisitor::caseStmtSwitch(ASTStmtSwitch & host)
{
	host.getKey()->execute(*this);

	vector<ASTSwitchCases*> & cases = host.getCases();
	for (vector<ASTSwitchCases*>::iterator it = cases.begin(); it != cases.end(); ++it)
		(*it)->execute(*this);
}

void RecursiveVisitor::caseSwitchCases(ASTSwitchCases & host, void* param)
{
	vector<ASTExprConst*> & cases = host.getCases();
	for (vector<ASTExprConst*>::iterator it = cases.begin();
		 it != cases.end();
		 ++it)
	{
		(*it)->execute(*this, param);
	}

	host.getBlock()->execute(*this, param);
}

void RecursiveVisitor::caseSwitchCases(ASTSwitchCases & host)
{
	vector<ASTExprConst*> & cases = host.getCases();
	for (vector<ASTExprConst*>::iterator it = cases.begin();
		 it != cases.end();
		 ++it)
	{
		(*it)->execute(*this);
	}

	host.getBlock()->execute(*this);
}

void RecursiveVisitor::caseStmtFor(ASTStmtFor &host, void *param)
{
	host.getPrecondition()->execute(*this, param);
	host.getIncrement()->execute(*this, param);
	host.getTerminationCondition()->execute(*this, param);
	host.getStmt()->execute(*this, param);
}

void RecursiveVisitor::caseStmtFor(ASTStmtFor &host)
{
	host.getPrecondition()->execute(*this);
	host.getIncrement()->execute(*this);
	host.getTerminationCondition()->execute(*this);
	host.getStmt()->execute(*this);
}

void RecursiveVisitor::caseStmtWhile(ASTStmtWhile &host, void *param)
{
	host.getCond()->execute(*this, param);
	host.getStmt()->execute(*this, param);
}

void RecursiveVisitor::caseStmtWhile(ASTStmtWhile &host)
{
	host.getCond()->execute(*this);
	host.getStmt()->execute(*this);
}

void RecursiveVisitor::caseStmtDo(ASTStmtDo &host, void *param)
{
	host.getStmt()->execute(*this, param);
	host.getCond()->execute(*this, param);
}

void RecursiveVisitor::caseStmtDo(ASTStmtDo &host)
{
	host.getStmt()->execute(*this);
	host.getCond()->execute(*this);
}

void RecursiveVisitor::caseStmtReturnVal(ASTStmtReturnVal &host, void *param)
{
	host.getReturnValue()->execute(*this, param);
}

void RecursiveVisitor::caseStmtReturnVal(ASTStmtReturnVal &host)
{
	host.getReturnValue()->execute(*this);
}

// Declarations

void RecursiveVisitor::caseScript(ASTScript &host, void *param)
{
	host.getType()->execute(*this, param);
	host.getScriptBlock()->execute(*this, param);
}

void RecursiveVisitor::caseScript(ASTScript &host)
{
	host.getType()->execute(*this);
	host.getScriptBlock()->execute(*this);
}

void RecursiveVisitor::caseDeclList(ASTDeclList &host, void *param)
{
	list<ASTDecl*> l = host.getDeclarations();

	for(list<ASTDecl*>::iterator it = l.begin(); it != l.end(); it++)
	{
		(*it)->execute(*this, param);
	}
}

void RecursiveVisitor::caseDeclList(ASTDeclList &host)
{
	list<ASTDecl*> l = host.getDeclarations();

	for(list<ASTDecl*>::iterator it = l.begin(); it != l.end(); it++)
	{
		(*it)->execute(*this);
	}
}

void RecursiveVisitor::caseFuncDecl(ASTFuncDecl &host, void *param)
{
	host.getReturnType()->execute(*this, param);
	list<ASTVarDecl *> l = host.getParams();

	for(list<ASTVarDecl *>::iterator it = l.begin(); it != l.end(); it++)
	{
		(*it)->execute(*this, param);
	}

	host.getBlock()->execute(*this, param);
}

void RecursiveVisitor::caseFuncDecl(ASTFuncDecl &host)
{
	host.getReturnType()->execute(*this);
	list<ASTVarDecl *> l = host.getParams();

	for(list<ASTVarDecl *>::iterator it = l.begin(); it != l.end(); it++)
	{
		(*it)->execute(*this);
	}

	host.getBlock()->execute(*this);
}

void RecursiveVisitor::caseArrayDecl(ASTArrayDecl &host, void *param)
{
	host.getType()->execute(*this, param);

	((ASTExpr *) host.getSize())->execute(*this, param);

	if (host.getList() != NULL)
	{
		for (list<ASTExpr *>::iterator it = host.getList()->getList().begin(); it != host.getList()->getList().end(); it++)
		{
			(*it)->execute(*this, param);
		}
	}
}

void RecursiveVisitor::caseArrayDecl(ASTArrayDecl &host)
{
	host.getType()->execute(*this);

	((ASTExpr *) host.getSize())->execute(*this);

	if (host.getList() != NULL)
	{
		for (list<ASTExpr *>::iterator it = host.getList()->getList().begin(); it != host.getList()->getList().end(); it++)
		{
			(*it)->execute(*this);
		}
	}
}

void RecursiveVisitor::caseVarDecl(ASTVarDecl &host, void *param)
{
	host.getType()->execute(*this, param);
}

void RecursiveVisitor::caseVarDecl(ASTVarDecl &host)
{
	host.getType()->execute(*this);
}

void RecursiveVisitor::caseVarDeclInitializer(ASTVarDeclInitializer &host, void *param)
{
	host.getType()->execute(*this, param);
	host.getInitializer()->execute(*this, param);
}

void RecursiveVisitor::caseVarDeclInitializer(ASTVarDeclInitializer &host)
{
	host.getType()->execute(*this);
	host.getInitializer()->execute(*this);
}

void RecursiveVisitor::caseTypeDef(ASTTypeDef& host, void* param)
{
	host.getType()->execute(*this, param);
}

void RecursiveVisitor::caseTypeDef(ASTTypeDef& host)
{
	host.getType()->execute(*this);
}

// Expressions

void RecursiveVisitor::caseExprConst(ASTExprConst &host, void *param)
{
	host.getContent()->execute(*this, param);
}

void RecursiveVisitor::caseExprConst(ASTExprConst &host)
{
	host.getContent()->execute(*this);
}

void RecursiveVisitor::caseNumConstant(ASTNumConstant &host, void *param)
{
	host.getValue()->execute(*this, param);
}

void RecursiveVisitor::caseNumConstant(ASTNumConstant &host)
{
	host.getValue()->execute(*this);
}

void RecursiveVisitor::caseExprArrow(ASTExprArrow &host, void*param)
{
	host.getLVal()->execute(*this, param);

	if(host.getIndex())
		host.getIndex()->execute(*this, param);
}

void RecursiveVisitor::caseExprArrow(ASTExprArrow &host)
{
	host.getLVal()->execute(*this);

	if(host.getIndex())
		host.getIndex()->execute(*this);
}

void RecursiveVisitor::caseExprArray(ASTExprArray &host, void *param)
{
	if(host.getIndex()) host.getIndex()->execute(*this, param);
}

void RecursiveVisitor::caseExprArray(ASTExprArray &host)
{
	if(host.getIndex()) host.getIndex()->execute(*this);
}

void RecursiveVisitor::caseFuncCall(ASTFuncCall &host, void *param)
{
	list<ASTExpr *> l = host.getParams();

	for(list<ASTExpr *>::iterator it = l.begin(); it != l.end(); it++)
	{
		(*it)->execute(*this, param);
	}
}

void RecursiveVisitor::caseFuncCall(ASTFuncCall &host)
{
	list<ASTExpr *> l = host.getParams();

	for(list<ASTExpr *>::iterator it = l.begin(); it != l.end(); it++)
	{
		(*it)->execute(*this);
	}
}

void RecursiveVisitor::caseExprNegate(ASTExprNegate &host, void *param)
{
	host.getOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprNegate(ASTExprNegate &host)
{
	host.getOperand()->execute(*this);
}

void RecursiveVisitor::caseExprNot(ASTExprNot &host, void *param)
{
	host.getOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprNot(ASTExprNot &host)
{
	host.getOperand()->execute(*this);
}

void RecursiveVisitor::caseExprBitNot(ASTExprBitNot &host, void *param)
{
	host.getOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprBitNot(ASTExprBitNot &host)
{
	host.getOperand()->execute(*this);
}

void RecursiveVisitor::caseExprIncrement(ASTExprIncrement &host, void *param)
{
	host.getOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprIncrement(ASTExprIncrement &host)
{
	host.getOperand()->execute(*this);
}

void RecursiveVisitor::caseExprPreIncrement(ASTExprPreIncrement &host, void *param)
{
	host.getOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprPreIncrement(ASTExprPreIncrement &host)
{
	host.getOperand()->execute(*this);
}

void RecursiveVisitor::caseExprDecrement(ASTExprDecrement &host, void *param)
{
	host.getOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprDecrement(ASTExprDecrement &host)
{
	host.getOperand()->execute(*this);
}

void RecursiveVisitor::caseExprPreDecrement(ASTExprPreDecrement &host, void *param)
{
	host.getOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprPreDecrement(ASTExprPreDecrement &host)
{
	host.getOperand()->execute(*this);
}

void RecursiveVisitor::caseExprAnd(ASTExprAnd &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprAnd(ASTExprAnd &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprOr(ASTExprOr &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprOr(ASTExprOr &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprGT(ASTExprGT &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprGT(ASTExprGT &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprGE(ASTExprGE &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprGE(ASTExprGE &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprLT(ASTExprLT &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprLT(ASTExprLT &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprLE(ASTExprLE &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprLE(ASTExprLE &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprEQ(ASTExprEQ &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprEQ(ASTExprEQ &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprNE(ASTExprNE &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprNE(ASTExprNE &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprPlus(ASTExprPlus &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprPlus(ASTExprPlus &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprMinus(ASTExprMinus &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprMinus(ASTExprMinus &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprTimes(ASTExprTimes &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprTimes(ASTExprTimes &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprDivide(ASTExprDivide &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprDivide(ASTExprDivide &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprModulo(ASTExprModulo &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprModulo(ASTExprModulo &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprBitAnd(ASTExprBitAnd &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprBitAnd(ASTExprBitAnd &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprBitOr(ASTExprBitOr &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprBitOr(ASTExprBitOr &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprBitXor(ASTExprBitXor &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprBitXor(ASTExprBitXor &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprLShift(ASTExprLShift &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprLShift(ASTExprLShift &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprRShift(ASTExprRShift &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprRShift(ASTExprRShift &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

////////////////////////////////////////////////////////////////////////////////
// GetImports

void GetImports::caseDefault(void *param)
{
    if(param != NULL)
        *(bool *)param = false;
}

void GetImports::caseProgram(ASTProgram &host, void *)
{
    host.getDeclarations()->execute(*this, NULL);
}

void GetImports::caseDeclList(ASTDeclList &host, void *)
{
    list<ASTDecl *> &l = host.getDeclarations();

    for(list<ASTDecl *>::iterator it = l.begin(); it != l.end();)
    {
        bool isimport;
        (*it)->execute(*this, &isimport);

        if(isimport)
        {
            result.push_back((ASTImportDecl *)*it);
            it=l.erase(it);
        }
        else
            it++;
    }
}

void GetImports::caseImportDecl(ASTImportDecl &, void *param)
{
    if(param != NULL)
        *(bool *)param = true;
}

////////////////////////////////////////////////////////////////////////////////
// GetConsts

void GetConsts::caseDefault(void *param)
{
    if(param != NULL)
        *(bool *)param = false;
}

void GetConsts::caseProgram(ASTProgram &host, void *)
{
    host.getDeclarations()->execute(*this, NULL);
}

void GetConsts::caseDeclList(ASTDeclList &host, void *)
{
    list<ASTDecl *> &l = host.getDeclarations();

    for(list<ASTDecl *>::iterator it = l.begin(); it != l.end();)
    {
        bool isconst;
        (*it)->execute(*this, &isconst);

        if(isconst)
        {
            result.push_back((ASTConstDecl *)*it);
            it=l.erase(it);
        }
        else
            it++;
    }
}

void GetConsts::caseConstDecl(ASTConstDecl &, void *param)
{
    if(param != NULL)
        *(bool *)param = true;
}

////////////////////////////////////////////////////////////////////////////////
// GetGlobalVars

void GetGlobalVars::caseDefault(void *param)
{
    if(param != NULL)
        *(int *)param = 0;
}

void GetGlobalVars::caseProgram(ASTProgram &host, void *)
{
    host.getDeclarations()->execute(*this, NULL);
}

void GetGlobalVars::caseDeclList(ASTDeclList &host, void *)
{
    list<ASTDecl *> &l = host.getDeclarations();

    for(list<ASTDecl *>::iterator it = l.begin(); it != l.end();)
    {
        int dectype;
        (*it)->execute(*this, &dectype);

        if (dectype == 1)
        {
            result.push_back((ASTVarDecl *)(*it));
            it = l.erase(it);
        }
        else if (dectype == 2)
        {
            resultA.push_back((ASTArrayDecl *)(*it));
            it = l.erase(it);
        }
        else
            it++;
    }
}

void GetGlobalVars::caseArrayDecl(ASTArrayDecl &, void *param)
{
    if(param != NULL)
        *(int *)param = 2;
}

void GetGlobalVars::caseVarDecl(ASTVarDecl &, void *param)
{
    if(param != NULL)
        *(int *)param = 1;
}

void GetGlobalVars::caseVarDeclInitializer(ASTVarDeclInitializer &, void *param)
{
    if(param != NULL)
        *(int *)param = 1;
}

////////////////////////////////////////////////////////////////////////////////
// GetGlobalFuncs

void GetGlobalFuncs::caseDefault(void *param)
{
    if(param != NULL)
        *(bool *)param = false;
}

void GetGlobalFuncs::caseProgram(ASTProgram &host, void *)
{
    host.getDeclarations()->execute(*this,NULL);
}

void GetGlobalFuncs::caseDeclList(ASTDeclList &host, void *)
{
    list<ASTDecl *> &l = host.getDeclarations();

    for(list<ASTDecl *>::iterator it = l.begin(); it != l.end();)
    {
        bool isfuncdecl;
        (*it)->execute(*this, &isfuncdecl);

        if(isfuncdecl)
        {
            result.push_back((ASTFuncDecl *)*it);
            it = l.erase(it);
        }
        else
            it++;
    }
}

void GetGlobalFuncs::caseFuncDecl(ASTFuncDecl &, void *param)
{
    if(param != NULL)
        *(bool *)param = true;
}

////////////////////////////////////////////////////////////////
// GetGlobalTypes

void GetGlobalTypes::caseDefault(void *param)
{
	if (param != NULL) *(bool*)param = false;
}

void GetGlobalTypes::caseProgram(ASTProgram &host, void*)
{
    host.getDeclarations()->execute(*this, NULL);
}

void GetGlobalTypes::caseDeclList(ASTDeclList &host, void*)
{
    list<ASTDecl*> &l = host.getDeclarations();

    for (list<ASTDecl*>::iterator it = l.begin(); it != l.end();)
    {
        bool istypedef;
        (*it)->execute(*this, &istypedef);

        if (istypedef)
        {
            result.push_back((ASTTypeDef*)*it);
            it = l.erase(it);
        }
        else
            it++;
    }
}

void GetGlobalTypes::caseTypeDef(ASTTypeDef &, void *param)
{
    if (param != NULL) *(bool*)param = true;
}

////////////////////////////////////////////////////////////////////////////////
// GetScripts

void GetScripts::caseDefault(void *)
{
    // There should be nothing left in here now
    box_out("Should be nothing but scripts left at this point.");
    box_eol();
    assert(false);
}

void GetScripts::caseProgram(ASTProgram &host, void *param)
{
    host.getDeclarations()->execute(*this,param);
}

void GetScripts::caseDeclList(ASTDeclList &host, void *param)
{
    list<ASTDecl *> &l = host.getDeclarations();

    for(list<ASTDecl *>::iterator it = l.begin(); it != l.end();)
    {
        (*it)->execute(*this, param);
        result.push_back((ASTScript *)*it);
        it = l.erase(it);
    }
}

void GetScripts::caseScript(ASTScript &, void *) {}

////////////////////////////////////////////////////////////////////////////////
// MergeASTs

void MergeASTs::caseDefault(void *)
{
    box_out("Something BAD BROKEN in the parser code!");
    box_eol();
    assert(false);
}

void MergeASTs::caseProgram(ASTProgram &host, void *param)
{
    assert(param);
    ASTProgram *other = (ASTProgram *)param;
    list<ASTDecl *> &decls = other->getDeclarations()->getDeclarations();

    for(list<ASTDecl *>::iterator it = decls.begin(); it != decls.end();)
    {
        host.getDeclarations()->addDeclaration((*it));
        it = decls.erase(it);
    }

    delete other;
}

////////////////////////////////////////////////////////////////////////////////
// CheckForExtraneousImports

void CheckForExtraneousImports::caseDefault(void *) {}

void CheckForExtraneousImports::caseImportDecl(ASTImportDecl &host, void *)
{
    ok = false;
    printErrorMsg(&host, IMPORTBADSCOPE);
}
