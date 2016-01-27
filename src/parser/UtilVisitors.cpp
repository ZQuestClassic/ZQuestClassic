
#include "../precompiled.h" //always first

#include "UtilVisitors.h"
#include <assert.h>
#include "../zsyssimple.h"
#include "ParseError.h"

void Clone::caseDefault(void *param)
{
    //these are here to bypass compiler warnings about unused arguments
    param=param;
    
    //unreachable
    assert(false);
}

void Clone::caseProgram(ASTProgram &host, void *param)
{
    host.getDeclarations()->execute(*this,param);
    result = new ASTProgram((ASTDeclList *)result, host.getLocation());
}
void Clone::caseFloat(ASTFloat &host, void *param)
{
    //these are here to bypass compiler warnings about unused arguments
    param=param;
    
    result = new ASTFloat(host.getValue().c_str(),host.getType(),host.getLocation());
}
void Clone::caseString(ASTString &host, void *param)
{
    //these are here to bypass compiler warnings about unused arguments
    param=param;
    
    result = new ASTString(host.getValue().c_str(),host.getLocation());
}
void Clone::caseDeclList(ASTDeclList &host, void *param)
{
    ASTDeclList *dl = new ASTDeclList(host.getLocation());
    list<ASTDecl *> decls = host.getDeclarations();
    list<ASTDecl *>::reverse_iterator it;
    
    for(it = decls.rbegin(); it != decls.rend(); it++)
    {
        (*it)->execute(*this,param);
        dl->addDeclaration((ASTDecl *)result);
    }
    
    result = dl;
}
void Clone::caseImportDecl(ASTImportDecl &host, void *param)
{
    //these are here to bypass compiler warnings about unused arguments
    param=param;
    
    result = new ASTImportDecl(host.getFilename(),host.getLocation());
}
void Clone::caseConstDecl(ASTConstDecl &host, void *param)
{
    host.getValue()->execute(*this,param);
    result = new ASTConstDecl(host.getName(),(ASTFloat *)result,host.getLocation());
}
void Clone::caseFuncDecl(ASTFuncDecl &host, void *param)
{
    ASTFuncDecl *af = new ASTFuncDecl(host.getLocation());
    host.getReturnType()->execute(*this,param);
    ASTType *rettype = (ASTType *)result;
    host.getBlock()->execute(*this,param);
    ASTBlock *block = (ASTBlock *)result;
    af->setName(host.getName());
    af->setBlock(block);
    af->setReturnType(rettype);
    list<ASTVarDecl *> params = host.getParams();
    list<ASTVarDecl *>::reverse_iterator it;
    
    for(it = params.rbegin(); it != params.rend(); it++)
    {
        (*it)->execute(*this,param);
        af->addParam((ASTVarDecl *)result);
    }
    
    result = af;
}
void Clone::caseTypeFloat(ASTTypeFloat &host, void *param)
{
    //these are here to bypass compiler warnings about unused arguments
    param=param;
    
    result = new ASTTypeFloat(host.getLocation());
}
void Clone::caseTypeBool(ASTTypeBool &host, void *param)
{
    //these are here to bypass compiler warnings about unused arguments
    param=param;
    
    result = new ASTTypeBool(host.getLocation());
}
void Clone::caseTypeVoid(ASTTypeVoid &host, void *param)
{
    //these are here to bypass compiler warnings about unused arguments
    param=param;
    
    result = new ASTTypeVoid(host.getLocation());
}
void Clone::caseTypeFFC(ASTTypeFFC &host, void *param)
{
    //these are here to bypass compiler warnings about unused arguments
    param=param;
    
    result = new ASTTypeFFC(host.getLocation());
}
void Clone::caseTypeGlobal(ASTTypeGlobal &host, void *param)
{
    //these are here to bypass compiler warnings about unused arguments
    param=param;
    
    result = new ASTTypeGlobal(host.getLocation());
}
void Clone::caseTypeItem(ASTTypeItem &host, void *param)
{
    //these are here to bypass compiler warnings about unused arguments
    param=param;
    
    result = new ASTTypeItem(host.getLocation());
}
void Clone::caseTypeItemclass(ASTTypeItemclass &host, void *param)
{
    //these are here to bypass compiler warnings about unused arguments
    param=param;
    
    result = new ASTTypeItemclass(host.getLocation());
}
void Clone::caseTypeNPC(ASTTypeNPC &host, void *)
{
    result = new ASTTypeNPC(host.getLocation());
}

void Clone::caseTypeLWpn(ASTTypeLWpn &host, void *)
{
    result = new ASTTypeLWpn(host.getLocation());
}

void Clone::caseTypeEWpn(ASTTypeEWpn &host, void *)
{
    result = new ASTTypeEWpn(host.getLocation());
}

void Clone::caseVarDecl(ASTVarDecl &host, void *param)
{
    host.getType()->execute(*this,param);
    ASTType *t = (ASTType *)result;
    result = new ASTVarDecl(t,host.getName(), host.getLocation());
}
void Clone::caseArrayDecl(ASTArrayDecl &host, void *param)
{
    ASTArrayList *l = NULL;
    host.getType()->execute(*this,param);
    ASTType *t = (ASTType *) result;
    
    if(host.getList())
    {
        l = new ASTArrayList(host.getLocation());
        
        for(list<ASTExpr *>::iterator it = host.getList()->getList().begin(); it != host.getList()->getList().end(); it++)
        {
            (*it)->execute(*this,param);
            l->addParam((ASTExpr *) result);
        }
    }
    
    AST *s;
    
    if(host.isRegister())
    {
        ((ASTExpr *) host.getSize())->execute(*this, param);
        s = (ASTExpr *) result;
    }
    else
    {
        ((ASTFloat *) host.getSize())->execute(*this, param);
        s = (ASTFloat *) result;
    }
    
    result = new ASTArrayDecl(t,host.getName(),s,host.isRegister(),l,host.getLocation());
}

void Clone::caseVarDeclInitializer(ASTVarDeclInitializer &host, void *param)
{
    host.getType()->execute(*this,param);
    ASTType *t = (ASTType *)result;
    host.getInitializer()->execute(*this,param);
    ASTExpr *e = (ASTExpr *)result;
    result = new ASTVarDeclInitializer(t,host.getName(),e, host.getLocation());
}

void Clone::caseExprAnd(ASTExprAnd &host, void *param)
{
    host.getFirstOperand()->execute(*this,param);
    ASTExpr *f = (ASTExpr *)result;
    host.getSecondOperand()->execute(*this,param);
    ASTExpr *s = (ASTExpr *)result;
    ASTExprAnd *res = new ASTExprAnd(host.getLocation());
    res->setFirstOperand(f);
    res->setSecondOperand(s);
    result = res;
}
void Clone::caseExprOr(ASTExprOr &host, void *param)
{
    host.getFirstOperand()->execute(*this,param);
    ASTExpr *f = (ASTExpr *)result;
    host.getSecondOperand()->execute(*this,param);
    ASTExpr *s = (ASTExpr *)result;
    ASTExprOr *res = new ASTExprOr(host.getLocation());
    res->setFirstOperand(f);
    res->setSecondOperand(s);
    result = res;
}
void Clone::caseExprGT(ASTExprGT &host, void *param)
{
    host.getFirstOperand()->execute(*this,param);
    ASTExpr *f = (ASTExpr *)result;
    host.getSecondOperand()->execute(*this,param);
    ASTExpr *s = (ASTExpr *)result;
    ASTExprGT *res = new ASTExprGT(host.getLocation());
    res->setFirstOperand(f);
    res->setSecondOperand(s);
    result = res;
}
void Clone::caseExprGE(ASTExprGE &host, void *param)
{
    host.getFirstOperand()->execute(*this,param);
    ASTExpr *f = (ASTExpr *)result;
    host.getSecondOperand()->execute(*this,param);
    ASTExpr *s = (ASTExpr *)result;
    ASTExprGE *res = new ASTExprGE(host.getLocation());
    res->setFirstOperand(f);
    res->setSecondOperand(s);
    result = res;
}
void Clone::caseExprLT(ASTExprLT &host, void *param)
{
    host.getFirstOperand()->execute(*this,param);
    ASTExpr *f = (ASTExpr *)result;
    host.getSecondOperand()->execute(*this,param);
    ASTExpr *s = (ASTExpr *)result;
    ASTExprLT *res = new ASTExprLT(host.getLocation());
    res->setFirstOperand(f);
    res->setSecondOperand(s);
    result = res;
}
void Clone::caseExprLE(ASTExprLE &host, void *param)
{
    host.getFirstOperand()->execute(*this,param);
    ASTExpr *f = (ASTExpr *)result;
    host.getSecondOperand()->execute(*this,param);
    ASTExpr *s = (ASTExpr *)result;
    ASTExprLE *res = new ASTExprLE(host.getLocation());
    res->setFirstOperand(f);
    res->setSecondOperand(s);
    result = res;
}
void Clone::caseExprEQ(ASTExprEQ &host, void *param)
{
    host.getFirstOperand()->execute(*this,param);
    ASTExpr *f = (ASTExpr *)result;
    host.getSecondOperand()->execute(*this,param);
    ASTExpr *s = (ASTExpr *)result;
    ASTExprEQ *res = new ASTExprEQ(host.getLocation());
    res->setFirstOperand(f);
    res->setSecondOperand(s);
    result = res;
}
void Clone::caseExprNE(ASTExprNE &host, void *param)
{
    host.getFirstOperand()->execute(*this,param);
    ASTExpr *f = (ASTExpr *)result;
    host.getSecondOperand()->execute(*this,param);
    ASTExpr *s = (ASTExpr *)result;
    ASTExprNE *res = new ASTExprNE(host.getLocation());
    res->setFirstOperand(f);
    res->setSecondOperand(s);
    result = res;
}
void Clone::caseExprPlus(ASTExprPlus &host, void *param)
{
    host.getFirstOperand()->execute(*this,param);
    ASTExpr *f = (ASTExpr *)result;
    host.getSecondOperand()->execute(*this,param);
    ASTExpr *s = (ASTExpr *)result;
    ASTExprPlus *res = new ASTExprPlus(host.getLocation());
    res->setFirstOperand(f);
    res->setSecondOperand(s);
    result = res;
}
void Clone::caseExprMinus(ASTExprMinus &host, void *param)
{
    host.getFirstOperand()->execute(*this,param);
    ASTExpr *f = (ASTExpr *)result;
    host.getSecondOperand()->execute(*this,param);
    ASTExpr *s = (ASTExpr *)result;
    ASTExprMinus *res = new ASTExprMinus(host.getLocation());
    res->setFirstOperand(f);
    res->setSecondOperand(s);
    result = res;
}
void Clone::caseExprTimes(ASTExprTimes &host, void *param)
{
    host.getFirstOperand()->execute(*this,param);
    ASTExpr *f = (ASTExpr *)result;
    host.getSecondOperand()->execute(*this,param);
    ASTExpr *s = (ASTExpr *)result;
    ASTExprTimes *res = new ASTExprTimes(host.getLocation());
    res->setFirstOperand(f);
    res->setSecondOperand(s);
    result = res;
}
void Clone::caseExprDivide(ASTExprDivide &host, void *param)
{
    host.getFirstOperand()->execute(*this,param);
    ASTExpr *f = (ASTExpr *)result;
    host.getSecondOperand()->execute(*this,param);
    ASTExpr *s = (ASTExpr *)result;
    ASTExprDivide *res = new ASTExprDivide(host.getLocation());
    res->setFirstOperand(f);
    res->setSecondOperand(s);
    result = res;
}
void Clone::caseExprBitOr(ASTExprBitOr &host, void *param)
{
    host.getFirstOperand()->execute(*this,param);
    ASTExpr *f = (ASTExpr *)result;
    host.getSecondOperand()->execute(*this,param);
    ASTExpr *s = (ASTExpr *)result;
    ASTExprBitOr *res = new ASTExprBitOr(host.getLocation());
    res->setFirstOperand(f);
    res->setSecondOperand(s);
    result = res;
}
void Clone::caseExprBitXor(ASTExprBitXor &host, void *param)
{
    host.getFirstOperand()->execute(*this,param);
    ASTExpr *f = (ASTExpr *)result;
    host.getSecondOperand()->execute(*this,param);
    ASTExpr *s = (ASTExpr *)result;
    ASTExprBitXor *res = new ASTExprBitXor(host.getLocation());
    res->setFirstOperand(f);
    res->setSecondOperand(s);
    result = res;
}
void Clone::caseExprBitAnd(ASTExprBitAnd &host, void *param)
{
    host.getFirstOperand()->execute(*this,param);
    ASTExpr *f = (ASTExpr *)result;
    host.getSecondOperand()->execute(*this,param);
    ASTExpr *s = (ASTExpr *)result;
    ASTExprBitAnd *res = new ASTExprBitAnd(host.getLocation());
    res->setFirstOperand(f);
    res->setSecondOperand(s);
    result = res;
}
void Clone::caseExprLShift(ASTExprLShift &host, void *param)
{
    host.getFirstOperand()->execute(*this,param);
    ASTExpr *f = (ASTExpr *)result;
    host.getSecondOperand()->execute(*this,param);
    ASTExpr *s = (ASTExpr *)result;
    ASTExprLShift *res = new ASTExprLShift(host.getLocation());
    res->setFirstOperand(f);
    res->setSecondOperand(s);
    result = res;
}
void Clone::caseExprRShift(ASTExprRShift &host, void *param)
{
    host.getFirstOperand()->execute(*this,param);
    ASTExpr *f = (ASTExpr *)result;
    host.getSecondOperand()->execute(*this,param);
    ASTExpr *s = (ASTExpr *)result;
    ASTExprRShift *res = new ASTExprRShift(host.getLocation());
    res->setFirstOperand(f);
    res->setSecondOperand(s);
    result = res;
}
void Clone::caseExprModulo(ASTExprModulo &host, void *param)
{
    host.getFirstOperand()->execute(*this,param);
    ASTExpr *f = (ASTExpr *)result;
    host.getSecondOperand()->execute(*this,param);
    ASTExpr *s = (ASTExpr *)result;
    ASTExprModulo *res = new ASTExprModulo(host.getLocation());
    res->setFirstOperand(f);
    res->setSecondOperand(s);
    result = res;
}
void Clone::caseExprNot(ASTExprNot &host, void *param)
{
    host.getOperand()->execute(*this,param);
    ASTExpr *f = (ASTExpr *)result;
    ASTExprNot *res = new ASTExprNot(host.getLocation());
    res->setOperand(f);
    result = res;
}
void Clone::caseExprNegate(ASTExprNegate &host, void *param)
{
    host.getOperand()->execute(*this,param);
    ASTExpr *f = (ASTExpr *)result;
    ASTExprNegate *res = new ASTExprNegate(host.getLocation());
    res->setOperand(f);
    result = res;
}
void Clone::caseExprBitNot(ASTExprBitNot &host, void *param)
{
    host.getOperand()->execute(*this,param);
    ASTExpr *f = (ASTExpr *)result;
    ASTExprBitNot *res = new ASTExprBitNot(host.getLocation());
    res->setOperand(f);
    result = res;
}
void Clone::caseExprIncrement(ASTExprIncrement &host, void *param)
{
    host.getOperand()->execute(*this,param);
    ASTExpr *f = (ASTExpr *)result;
    ASTExprIncrement *res = new ASTExprIncrement(host.getLocation());
    res->setOperand(f);
    result = res;
}
void Clone::caseExprPreIncrement(ASTExprPreIncrement &host, void *param)
{
    host.getOperand()->execute(*this,param);
    ASTExpr *f = (ASTExpr *)result;
    ASTExprPreIncrement *res = new ASTExprPreIncrement(host.getLocation());
    res->setOperand(f);
    result = res;
}
void Clone::caseExprDecrement(ASTExprDecrement&host, void *param)
{
    host.getOperand()->execute(*this,param);
    ASTExpr *f = (ASTExpr *)result;
    ASTExprDecrement *res = new ASTExprDecrement(host.getLocation());
    res->setOperand(f);
    result = res;
}
void Clone::caseExprPreDecrement(ASTExprPreDecrement &host, void *param)
{
    host.getOperand()->execute(*this,param);
    ASTExpr *f = (ASTExpr *)result;
    ASTExprPreDecrement *res = new ASTExprPreDecrement(host.getLocation());
    res->setOperand(f);
    result = res;
}
void Clone::caseNumConstant(ASTNumConstant &host, void *param)
{
    host.getValue()->execute(*this,param);
    ASTFloat *f = (ASTFloat *)result;
    result = new ASTNumConstant(f, host.getLocation());
}
void Clone::caseFuncCall(ASTFuncCall &host, void *param)
{
    ASTFuncCall *fc = new ASTFuncCall(host.getLocation());
    host.getName()->execute(*this,param);
    ASTExpr *name = (ASTExpr *)result;
    fc->setName(name);
    list<ASTExpr *> params = host.getParams();
    list<ASTExpr *>::reverse_iterator it;
    
    for(it = params.rbegin(); it != params.rend(); it++)
    {
        (*it)->execute(*this,param);
        fc->addParam((ASTExpr *)result);
    }
    
    result = fc;
}

void Clone::caseBoolConstant(ASTBoolConstant &host, void *param)
{
    //these are here to bypass compiler warnings about unused arguments
    param=param;
    
    result = new ASTBoolConstant(host.getValue(),host.getLocation());
}

void Clone::caseBlock(ASTBlock &host, void *param)
{
    ASTBlock *b = new ASTBlock(host.getLocation());
    list<ASTStmt *> stmts = host.getStatements();
    list<ASTStmt *>::reverse_iterator it;
    
    for(it = stmts.rbegin(); it != stmts.rend(); it++)
    {
        (*it)->execute(*this,param);
        b->addStatement((ASTStmt *)result);
    }
    
    result = b;
}

void Clone::caseStmtAssign(ASTStmtAssign &host, void *param)
{
    host.getLVal()->execute(*this,param);
    ASTExpr *left = (ASTExpr *)result;
    host.getRVal()->execute(*this,param);
    ASTExpr *right = (ASTExpr *)result;
    result = new ASTStmtAssign(left,right,host.getLocation());
}

void Clone::caseExprDot(ASTExprDot &host, void *param)
{
    //these are here to bypass compiler warnings about unused arguments
    param=param;
    
    result = new ASTExprDot(host.getNamespace(),host.getName(),host.getLocation());
}

void Clone::caseExprArrow(ASTExprArrow &host, void *param)
{
    host.getLVal()->execute(*this,param);
    ASTExprArrow *arrow = new ASTExprArrow((ASTExpr *)result, host.getName(), host.getLocation());
    
    if(host.getIndex())
    {
        host.getIndex()->execute(*this,param);
        arrow->setIndex((ASTExpr *)result);
    }
    
    result = arrow;
}

void Clone::caseExprArray(ASTExprArray &host, void *param)
{
    //these are here to bypass compiler warnings about unused arguments
    param=param;
    
    ASTExprArray *arr = new ASTExprArray(host.getNamespace(), host.getName(), host.getLocation());
    
    if(host.getIndex())
    {
        host.getIndex()->execute(*this,param);
        arr->setIndex((ASTExpr *)result);
    }
    
    result = arr; //new ASTExprArray(host.getNamespace(),host.getName(),host.getLocation());
}

void Clone::caseStmtFor(ASTStmtFor &host, void *param)
{
    host.getStmt()->execute(*this,param);
    ASTBlock *block = (ASTBlock *)result;
    host.getPrecondition()->execute(*this,param);
    ASTStmt *prec = (ASTStmt *)result;
    host.getTerminationCondition()->execute(*this,param);
    ASTExpr *cond = (ASTExpr *)result;
    host.getIncrement()->execute(*this,param);
    ASTStmt *incr = (ASTStmt *)result;
    result = new ASTStmtFor(prec,cond,incr,block,host.getLocation());
}
void Clone::caseStmtIf(ASTStmtIf &host, void *param)
{
    host.getCondition()->execute(*this,param);
    ASTExpr *cond = (ASTExpr *)result;
    host.getStmt()->execute(*this,param);
    result = new ASTStmtIf(cond, (ASTBlock *)result,host.getLocation());
}
void Clone::caseStmtIfElse(ASTStmtIfElse &host, void *param)
{
    host.getCondition()->execute(*this,param);
    ASTExpr *cond = (ASTExpr *)result;
    host.getStmt()->execute(*this,param);
    ASTBlock *block =(ASTBlock *)result;
    host.getElseStmt()->execute(*this,param);
    result = new ASTStmtIfElse(cond, block, (ASTBlock *)result,host.getLocation());
}

void Clone::caseStmtReturn(ASTStmtReturn &host, void *param)
{
    //these are here to bypass compiler warnings about unused arguments
    param=param;
    
    result = new ASTStmtReturn(host.getLocation());
}

void Clone::caseStmtReturnVal(ASTStmtReturnVal &host, void *param)
{
    host.getReturnValue()->execute(*this,param);
    result = new ASTStmtReturnVal((ASTExpr *)result,host.getLocation());
}

void Clone::caseStmtEmpty(ASTStmtEmpty &host, void *param)
{
    //these are here to bypass compiler warnings about unused arguments
    param=param;
    
    result = new ASTStmtEmpty(host.getLocation());
}

void Clone::caseScript(ASTScript &host, void *param)
{
    host.getType()->execute(*this,param);
    ASTType *type = (ASTType *)result;
    host.getScriptBlock()->execute(*this,param);
    result = new ASTScript(type, host.getName(), (ASTDeclList *)result, host.getLocation());
}

void Clone::caseStmtWhile(ASTStmtWhile &host, void *param)
{
    host.getCond()->execute(*this,param);
    ASTExpr *cond = (ASTExpr *)result;
    host.getStmt()->execute(*this,param);
    result = new ASTStmtWhile(cond, (ASTStmt *)result, host.getLocation());
}

void Clone::caseStmtDo(ASTStmtDo &host, void *param)
{
    host.getCond()->execute(*this,param);
    ASTExpr *cond = (ASTExpr *)result;
    host.getStmt()->execute(*this,param);
    result = new ASTStmtDo(cond, (ASTStmt *)result, host.getLocation());
}

void Clone::caseStmtBreak(ASTStmtBreak &host, void *param)
{
    //these are here to bypass compiler warnings about unused arguments
    param=param;
    
    result = new ASTStmtBreak(host.getLocation());
}

void Clone::caseStmtContinue(ASTStmtContinue &host, void *param)
{
    //these are here to bypass compiler warnings about unused arguments
    param=param;
    
    result = new ASTStmtContinue(host.getLocation());
}
////////////////////////////////////////////////////////////////////////////////
void GetImports::caseDefault(void *param)
{
    if(param != NULL)
        *(bool *)param = false;
}

void GetImports::caseImportDecl(ASTImportDecl &, void *param)
{
    if(param != NULL)
        *(bool *)param = true;
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

void GetImports::caseProgram(ASTProgram &host, void *param)
{
    //these are here to bypass compiler warnings about unused arguments
    param=param;
    
    host.getDeclarations()->execute(*this,NULL);
}

////////////////////////////////////////////////////////////////////////////////
void GetConsts::caseDefault(void *param)
{
    if(param != NULL)
        *(bool *)param = false;
}

void GetConsts::caseConstDecl(ASTConstDecl &host, void *param)
{
    //these are here to bypass compiler warnings about unused arguments
    void *temp;
    temp=&host;
    
    if(param != NULL)
        *(bool *)param = true;
}

void GetConsts::caseDeclList(ASTDeclList &host, void *param)
{
    //these are here to bypass compiler warnings about unused arguments
    param=param;
    
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

void GetConsts::caseProgram(ASTProgram &host, void *param)
{
    //these are here to bypass compiler warnings about unused arguments
    param=param;
    
    host.getDeclarations()->execute(*this,NULL);
}

void GetGlobalVars::caseDefault(void *param)
{
    if(param != NULL)
        *(int *)param = 0;
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

void GetGlobalVars::caseArrayDecl(ASTArrayDecl &, void *param)
{
    if(param != NULL)
        *(int *)param = 2;
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
        
        if(dectype==1)
        {
            result.push_back((ASTVarDecl *)(*it));
            it=l.erase(it);
        }
        else if(dectype==2)
        {
            resultA.push_back((ASTArrayDecl *)(*it));
            it=l.erase(it);
        }
        else
            it++;
    }
}

void GetGlobalFuncs::caseDefault(void *param)
{
    if(param != NULL)
        *(bool *)param = false;
}

void GetGlobalFuncs::caseFuncDecl(ASTFuncDecl &, void *param)
{
    if(param != NULL)
        *(bool *)param = true;
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
            it=l.erase(it);
        }
        else
            it++;
    }
}

void GetGlobalFuncs::caseProgram(ASTProgram &host, void *)
{
    host.getDeclarations()->execute(*this,NULL);
}

void GetScripts::caseDefault(void *param)
{
    //these are here to bypass compiler warnings about unused arguments
    param=param;
    
    //there should be nothing left in here now
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
        it=l.erase(it);
    }
}

void GetScripts::caseScript(ASTScript &host, void *param)
{
    void *temp;
    temp=&host;
    param=param; /*these are here to bypass compiler warnings about unused arguments*/
}

void MergeASTs::caseDefault(void *param)
{
    //these are here to bypass compiler warnings about unused arguments
    param=param;
    
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

void CheckForExtraneousImports::caseDefault(void *)
{

}

void CheckForExtraneousImports::caseImportDecl(ASTImportDecl &host, void *)
{
    ok = false;
    printErrorMsg(&host, IMPORTBADSCOPE);
}

void ExtractType::caseDefault(void *)
{
    //unreachable
    assert(false);
}

void ExtractType::caseTypeBool(ASTTypeBool &, void *param)
{
    *(int *)param = ScriptParser::TYPE_BOOL;
}

void ExtractType::caseTypeFloat(ASTTypeFloat &, void *param)
{
    *(int *)param = ScriptParser::TYPE_FLOAT;
}

void ExtractType::caseTypeVoid(ASTTypeVoid &, void *param)
{
    *(int *)param = ScriptParser::TYPE_VOID;
}

void ExtractType::caseTypeFFC(ASTTypeFFC &, void *param)
{
    *(int *)param = ScriptParser::TYPE_FFC;
}

void ExtractType::caseTypeGlobal(ASTTypeGlobal &, void *param)
{
    *(int *)param = ScriptParser::TYPE_GLOBAL;
}

void ExtractType::caseTypeItem(ASTTypeItem &, void *param)
{
    *(int *)param = ScriptParser::TYPE_ITEM;
}

void ExtractType::caseTypeItemclass(ASTTypeItemclass &, void *param)
{
    *(int *)param = ScriptParser::TYPE_ITEMCLASS;
}

void ExtractType::caseTypeNPC(ASTTypeNPC &, void *param)
{
    *(int *)param = ScriptParser::TYPE_NPC;
}

void ExtractType::caseTypeLWpn(ASTTypeLWpn &, void *param)
{
    *(int *)param = ScriptParser::TYPE_LWPN;
}

void ExtractType::caseTypeEWpn(ASTTypeEWpn &, void *param)
{
    *(int *)param = ScriptParser::TYPE_EWPN;
}

