#include "ScriptParser.h"
#include "ParseError.h"
#include "y.tab.hpp"
#include "TypeChecker.h"
#include "GlobalSymbols.h"
#include "ByteCode.h"
#include "../zsyssimple.h"
#include <iostream>
#include <assert.h>
#include <string>
using namespace std;
//#define PARSER_DEBUG

AST *resAST;

ASTProgram::~ASTProgram() {delete decls;}

ASTDeclList::~ASTDeclList() 
{
	list<ASTDecl *>::iterator it;
	for(it = decls.begin(); it != decls.end(); it++)
	{
		delete *it;
	}
	decls.clear();
}

void ASTDeclList::addDeclaration(ASTDecl *newdecl)
{
	decls.push_front(newdecl);
}

ASTFuncDecl::~ASTFuncDecl()
{
	delete rettype;
	delete block;
	list<ASTVarDecl *>::iterator it;
	for(it = params.begin(); it != params.end(); it++)
	{
		delete *it;
	}
	params.clear();
}

void ASTFuncDecl::addParam(ASTVarDecl *param)
{
	params.push_front(param);
}

ASTVarDecl::~ASTVarDecl()
{
	delete type;
}

ASTVarDeclInitializer::~ASTVarDeclInitializer()
{
	delete initial;
}


ASTFuncCall::~ASTFuncCall()
{
	list<ASTExpr *>::iterator it;
	for(it = params.begin(); it != params.end(); it++)
	{
		delete *it;
	}
	params.clear();
	delete name;
}

void ASTBlock::addStatement(ASTStmt *stmt)
{
	statements.push_front(stmt);
}

ASTBlock::~ASTBlock()
{
	list<ASTStmt *>::iterator it;
	for(it=statements.begin(); it != statements.end(); it++)
	{
		delete *it;
	}
	statements.clear();
}

ASTScript::~ASTScript()
{
	delete sblock;
	delete type;
}

////////////////////////////////////////////////////////////////////////////////////////
void Clone::caseDefault(void *param)
{
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
	result = new ASTFloat(host.getValue().c_str(),host.getLocation());
}
void Clone::caseString(ASTString &host, void *param)
{
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
	result = new ASTImportDecl(host.getFilename(),host.getLocation());
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
	result = new ASTTypeFloat(host.getLocation());
}
void Clone::caseTypeBool(ASTTypeBool &host, void *param)
{
	result = new ASTTypeBool(host.getLocation());
}
void Clone::caseTypeVoid(ASTTypeVoid &host, void *param)
{
	result = new ASTTypeVoid(host.getLocation());
}
void Clone::caseTypeFFC(ASTTypeFFC &host, void *param)
{
	result = new ASTTypeFFC(host.getLocation());
}
void Clone::caseTypeGlobal(ASTTypeGlobal &host, void *param)
{
	result = new ASTTypeGlobal(host.getLocation());
}
void Clone::caseTypeItem(ASTTypeItem &host, void *param)
{
	result = new ASTTypeItem(host.getLocation());
}
void Clone::caseTypeItemclass(ASTTypeItemclass &host, void *param)
{
	result = new ASTTypeItemclass(host.getLocation());
}
void Clone::caseVarDecl(ASTVarDecl &host, void *param)
{
	host.getType()->execute(*this,param);
	ASTType *t = (ASTType *)result;
	result = new ASTVarDecl(t,host.getName(), host.getLocation());
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
void Clone::caseExprDecrement(ASTExprDecrement&host, void *param)
{
	host.getOperand()->execute(*this,param);
	ASTExpr *f = (ASTExpr *)result;
	ASTExprDecrement *res = new ASTExprDecrement(host.getLocation());
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
	fc->setName(fc->getName());
	list<ASTExpr *> params = fc->getParams();
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
	result = new ASTExprDot(host.getNamespace(),host.getName(),host.getLocation());
}

void Clone::caseExprArrow(ASTExprArrow &host, void *param)
{
	host.getLVal()->execute(*this,param);
	ASTExprArrow *arrow = new ASTExprArrow((ASTExpr *)result, host.getName(), host.getLocation());
	if(host.getIndex())
	{
		host.getIndex()->execute(*this,param);
        ((ASTExprArrow *)result)->setIndex((ASTExpr *)result);
	}
	result = arrow;
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
	result = new ASTStmtReturn(host.getLocation());
}
void Clone::caseStmtReturnVal(ASTStmtReturnVal &host, void *param)
{
	host.getReturnValue()->execute(*this,param);
	result = new ASTStmtReturnVal((ASTExpr *)result,host.getLocation());
}
void Clone::caseStmtEmpty(ASTStmtEmpty &host, void *param)
{
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
////////////////////////////////////////////////////////////////////////////////
void GetImports::caseDefault(void *param) 
{
	if(param != NULL)
		*(bool *)param = false;
}
void GetImports::caseImportDecl(ASTImportDecl &host, void *param)
{
	if(param != NULL)
		*(bool *)param = true;
}
void GetImports::caseDeclList(ASTDeclList &host, void *param)
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
	host.getDeclarations()->execute(*this,NULL);
}

void GetGlobalFuncs::caseDefault(void *param) 
{
	if(param != NULL)
		*(bool *)param = false;
}
void GetGlobalFuncs::caseFuncDecl(ASTFuncDecl &host, void *param)
{
	if(param != NULL)
		*(bool *)param = true;
}
void GetGlobalFuncs::caseDeclList(ASTDeclList &host, void *param)
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
void GetGlobalFuncs::caseProgram(ASTProgram &host, void *param)
{
	host.getDeclarations()->execute(*this,NULL);
}

void GetScripts::caseDefault(void *param)
{
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

void GetScripts::caseScript(ASTScript &host, void *param) {}

void MergeASTs::caseDefault(void *param)
{
	cerr << "Something BAD BROKEN in the parser code!" << endl;
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

void CheckForExtraneousImports::caseDefault(void *param)
{
}

void CheckForExtraneousImports::caseImportDecl(ASTImportDecl &host, void *param)
{
	ok = false;
	printErrorMsg(&host, IMPORTBADSCOPE);
}

void ExtractType::caseDefault(void *param)
{
	//unreachable
	assert(false);
}

void ExtractType::caseTypeBool(ASTTypeBool &host, void *param)
{
	*(int *)param = ScriptParser::TYPE_BOOL;
}

void ExtractType::caseTypeFloat(ASTTypeFloat &host, void *param)
{
	*(int *)param = ScriptParser::TYPE_FLOAT;
}

void ExtractType::caseTypeVoid(ASTTypeVoid &host, void *param)
{
	*(int *)param = ScriptParser::TYPE_VOID;
}

void ExtractType::caseTypeFFC(ASTTypeFFC &host, void *param)
{
	*(int *)param = ScriptParser::TYPE_FFC;
}
void ExtractType::caseTypeGlobal(ASTTypeGlobal &host, void *param)
{
	*(int *)param = ScriptParser::TYPE_GLOBAL;
}
void ExtractType::caseTypeItem(ASTTypeItem &host, void *param)
{
	*(int *)param = ScriptParser::TYPE_ITEM;
}

void ExtractType::caseTypeItemclass(ASTTypeItemclass &host, void *param)
{
	*(int *)param = ScriptParser::TYPE_ITEMCLASS;
}

void BuildScriptSymbols::caseDefault(void *param)
{
	//unreachable
	assert(false);
}

void BuildScriptSymbols::caseScript(ASTScript &host,void *param)
{
	list<ASTDecl *> ad = host.getScriptBlock()->getDeclarations();
	for(list<ASTDecl *>::iterator it = ad.begin(); it != ad.end(); it++)
	{
		(*it)->execute(*this,param);
	}
}

void BuildScriptSymbols::caseFuncDecl(ASTFuncDecl &host, void *param)
{
	pair<Scope *, SymbolTable *> *p = (pair<Scope *, SymbolTable *> *)param;
	string name = host.getName();
	vector<int> params;
	list<ASTVarDecl *> vds = host.getParams();
	for(list<ASTVarDecl *>::iterator it = vds.begin(); it != vds.end(); it++)
	{
		int type;
		ExtractType temp;
		(*it)->getType()->execute(temp, &type);
		params.push_back(type);
		if(type == ScriptParser::TYPE_VOID)
		{
			failure=true;
			printErrorMsg(&host, FUNCTIONVOIDPARAM, name);
		}
	}
	int rettype;
	ExtractType temp;
	host.getReturnType()->execute(temp, &rettype);
	int id = p->first->getFuncSymbols().addFunction(name,rettype,params);
	if(id == -1)
	{
		failure = true;
		printErrorMsg(&host, FUNCTIONREDEF, name);
		return;
	}
	p->second->putAST(&host, id);
	p->second->putFunc(id, rettype);
	p->second->putFuncDecl(id, params);
}

void BuildScriptSymbols::caseVarDecl(ASTVarDecl &host, void *param)
{
	pair<Scope *, SymbolTable *> *p = (pair<Scope *, SymbolTable *> *)param;
	string name = host.getName();
	int type;
	ExtractType temp;
	host.getType()->execute(temp, &type);
	if(type == ScriptParser::TYPE_VOID)
	{
		failure = true;
		printErrorMsg(&host, VOIDVAR, name);
	}
	if(type == ScriptParser::TYPE_FFC || type == ScriptParser::TYPE_ITEM
		|| type == ScriptParser::TYPE_ITEMCLASS)
	{
		failure = true;
		printErrorMsg(&host, REFVAR, name);
	}
	//var is always visible
	int id = p->first->getVarSymbols().addVariable(name, type);
	if(id == -1)
	{
		failure = true;
		printErrorMsg(&host, VARREDEF, name);
		return;
	}
	p->second->putAST(&host, id);
	p->second->putVar(id, type);
}

void BuildFunctionSymbols::caseFuncDecl(ASTFuncDecl &host, void *param)
{
	pair<Scope *, SymbolTable *> *p = (pair<Scope *, SymbolTable *> *)param;
	//push in the scope
	Scope *subscope = new Scope(p->first);
	pair<Scope *, SymbolTable *> newparam = pair<Scope *, SymbolTable *>(subscope,p->second);
	//add the params
	list<ASTVarDecl *> vars = host.getParams();
	for(list<ASTVarDecl *>::iterator it = vars.begin(); it != vars.end(); it++)
	{
		string name = (*it)->getName();
		int type;
		ExtractType temp;
		(*it)->getType()->execute(temp, &type);
		int id = p->first->getVarSymbols().addVariable(name,type);
		if(id == -1)
		{
			printErrorMsg(*it, VARREDEF, name);
			failure = true;
			delete subscope;
			return;
		}
		p->second->putVar(id, type);
		p->second->putAST(*it, id);
	}
	//shortcut the block
	list<ASTStmt *> stmts = host.getBlock()->getStatements();
	for(list<ASTStmt *>::iterator it = stmts.begin(); it != stmts.end(); it++)
	{
		(*it)->execute(*this, &newparam);
	}
	delete subscope;
}

void BuildFunctionSymbols::caseVarDecl(ASTVarDecl &host, void *param)
{
	pair<Scope *, SymbolTable *> *p = (pair<Scope *, SymbolTable *> *)param;
	string name = host.getName();
	int type;
	ExtractType temp;
	host.getType()->execute(temp, &type);
	int id = p->first->getVarSymbols().addVariable(name, type);
	if(id == -1)
	{
		printErrorMsg(&host, VARREDEF, name);
		failure = true;
		return;
	}
	p->second->putAST(&host, id);
	p->second->putVar(id, type);
}

void BuildFunctionSymbols::caseBlock(ASTBlock &host, void *param)
{
	//push in  a new scope
	pair<Scope *, SymbolTable *> *p = (pair<Scope *, SymbolTable *> *)param;
	Scope *newscope = new Scope(p->first);
	pair<Scope *, SymbolTable *> *newparam = new pair<Scope *, SymbolTable *>(newscope, p->second);
	list<ASTStmt *> stmts = host.getStatements();
	for(list<ASTStmt *>::iterator it = stmts.begin(); it != stmts.end(); it++)
	{
		(*it)->execute(*this, newparam);
	}
	delete newparam;
	delete newscope;
}

void BuildFunctionSymbols::caseStmtFor(ASTStmtFor &host, void *param)
{
	//push in new scope
	//in accordance with C++ scoping
	pair<Scope *, SymbolTable *> *p = (pair<Scope *, SymbolTable *> *)param;
	Scope *newscope = new Scope(p->first);
	pair<Scope *, SymbolTable *> *newparam = new pair<Scope *, SymbolTable *>(newscope, p->second);
	host.getPrecondition()->execute(*this, newparam);
	host.getTerminationCondition()->execute(*this, newparam);
	host.getIncrement()->execute(*this, newparam);
	ASTStmt * stmt = host.getStmt();
	
	stmt->execute(*this,newparam);
	delete newparam;
	delete newscope;
}

void BuildFunctionSymbols::caseFuncCall(ASTFuncCall &host, void *param)
{
	pair<Scope *, SymbolTable *> *p = (pair<Scope *, SymbolTable *> *)param;
	ASTExpr *ident = host.getName();
	//ident could be a dotexpr, or an arrow exp.
	//if an arrow exp, it is not my problem right now
	bool isdot = false;
	IsDotExpr temp;
	ident->execute(temp, &isdot);
	if(!isdot)
	{
		//recur to get any dotexprs inside
		ident->execute(*this,param);
	}
	else
	{
		//resolve the possible functions
		ASTExprDot *dotname = (ASTExprDot *)host.getName();
		string name = dotname->getName();
		string nspace = dotname->getNamespace();
		vector<int> possibleFuncs = p->first->getFuncsInScope(nspace, name);
		if(possibleFuncs.size() == 0)
		{
			string fullname;
			if(nspace == "")
				fullname=name;
			else
				fullname = nspace + "." + name;
			printErrorMsg(&host, FUNCUNDECLARED, fullname);
			failure = true;
			return;
		}
		p->second->putAmbiguousFunc(&host, possibleFuncs);
	}
	
	for(list<ASTExpr *>::iterator it = host.getParams().begin(); it != host.getParams().end(); it++)
	{
		(*it)->execute(*this,param);
	}
}

void BuildFunctionSymbols::caseExprDot(ASTExprDot &host, void *param)
{
	pair<Scope *, SymbolTable *> *p = (pair<Scope *, SymbolTable *> *)param;
	string name = host.getName();
	string nspace = host.getNamespace();
	int id = p->first->getVarInScope(nspace, name);
	if(id == -1)
	{
		string fullname;
		if(nspace == "")
			fullname=name;
		else
			fullname = nspace + "." + name;
		printErrorMsg(&host, VARUNDECLARED, fullname);
		failure = true;
		return;
	}
	p->second->putAST(&host, id);
}

void BuildFunctionSymbols::caseExprArrow(ASTExprArrow &host, void *param)
{
	//recur on the name
	host.getLVal()->execute(*this,param);
	//wait for type-checking to do rest of work
}
////////////////////////////////////////////////////////////////////////////////
ScriptsData * compile(char *filename);

#ifdef PARSER_DEBUG
int main(int argc, char *argv[])
{
	if(argc < 2)
		return -1;
	compile(argv[1]);
}
#endif

ScriptsData * compile(char *filename)
{
	ScriptParser::resetState();
#ifndef SCRIPTPARSER_COMPILE
	box_out("Pass 1: Parsing");
	box_eol();
#endif
	if(go(filename) != 0 || !resAST)
	{
		printErrorMsg(NULL, CANTOPENSOURCE);
		return NULL;
	}
	AST *theAST = resAST;
#ifndef SCRIPTPARSER_COMPILE
	box_out("Pass 2: Preprocessing");
	box_eol();
#endif
	if(!ScriptParser::preprocess(theAST, RECURSIONLIMIT))
	{
		delete theAST;
		return NULL;
	}
#ifndef SCRIPTPARSER_COMPILE
	box_out("Pass 3: Building symbol tables");
	box_eol();
#endif
	SymbolData *d = ScriptParser::buildSymbolTable(theAST);
	if(d==NULL)
	{
		return NULL;
	}
	//d->symbols->printDiagnostics();
#ifndef SCRIPTPARSER_COMPILE
	box_out("Pass 4: Type-checking/Completing function symbol tables/Constant folding");
	box_eol();
#endif
	FunctionData *fd = ScriptParser::typeCheck(d);
	if(fd == NULL)
		return NULL;
#ifndef SCRIPTPARSER_COMPILE
	box_out("Pass 5: Generating object code");
	box_eol();
#endif
	IntermediateData *id = ScriptParser::generateOCode(fd);
#ifndef SCRIPTPARSER_COMPILE
	box_out("Pass 6: Assembling");
	box_eol();
#endif
	ScriptsData *final = ScriptParser::assemble(id);
	
	printf("success\n");
	return final;
}

int ScriptParser::vid = 0;
int ScriptParser::fid = 0;
int ScriptParser::gid = 1;
int ScriptParser::lid = 0;

// The following is NOT AT ALL compliant with the C++ standard
// but apparently required by the MingW gcc...
#ifndef _MSC_VER
const int ScriptParser::TYPE_FLOAT;
const int ScriptParser::TYPE_BOOL;
const int ScriptParser::TYPE_VOID;
const int ScriptParser::TYPE_LINK;
const int ScriptParser::TYPE_FFC;
const int ScriptParser::TYPE_ITEM;
const int ScriptParser::TYPE_ITEMCLASS;
const int ScriptParser::TYPE_SCREEN;
const int ScriptParser::TYPE_GLOBAL;
const int ScriptParser::TYPE_GAME;
#endif 

string ScriptParser::trimQuotes(string quoteds)
{
	string rval = quoteds.substr(1,quoteds.size()-2);
	return rval;
}

pair<string, string> ScriptParser::parseFloat(string f)
{
	string intpart;
	string fpart;
	bool founddot = false;
	for(unsigned int i=0; i<f.size(); i++)
	{
		if(f.at(i) == '.')
		{
			intpart = f.substr(0, i);
						
			fpart = f.substr(i+1,f.size()-i-1);
			founddot = true;
			break;
		}
	}
	if(!founddot)
	{
		intpart = f;
		fpart = "";
	}
	return pair<string,string>(intpart, fpart);
}

bool ScriptParser::preprocess(AST *theAST, int reclimit)
{
	if(reclimit == 0)
	{
		printErrorMsg(NULL, IMPORTRECURSION);
		return false;
	}
	GetImports gi;
	theAST->execute(gi, NULL);
	vector<ASTImportDecl *> imports = gi.getResult();
	for(vector<ASTImportDecl *>::iterator it = imports.begin(); it != imports.end(); it++)
	{
		string fn = trimQuotes((*it)->getFilename());
		if(go(fn.c_str()) != 0 || !resAST)
		{
			printErrorMsg(*it,CANTOPENIMPORT, fn);
			for(vector<ASTImportDecl *>::iterator it2 = imports.begin(); it2 != imports.end(); it2++)
			{
				delete *it2;
			}
			return false;
		}
		AST *recAST = resAST;
		if(!preprocess(recAST, reclimit-1))
		{
			delete recAST;
			return false;
		}
		MergeASTs temp;
		theAST->execute(temp, recAST);
	}
	//check that there are no more stupidly placed imports in the file
	CheckForExtraneousImports c;
	theAST->execute(c, NULL);
	return c.isOK();
}

SymbolData *ScriptParser::buildSymbolTable(AST *theAST)
{
	SymbolData *rval = new SymbolData();
	SymbolTable *t = new SymbolTable();
	Scope *globalScope = new Scope(NULL);
	bool failure = false;
	//ADD LIBRARY FUNCTIONS TO THE GLOBAL SCOPE HERE
	//t->putFuncDecl etc
	//globalScope->getFuncSymbols().addFunction("Rand", TYPE_FLOAT, vector<int>());
	GlobalSymbols::getInst().addSymbolsToScope(globalScope, t);
	FFCSymbols::getInst().addSymbolsToScope(globalScope,t);
	ItemSymbols::getInst().addSymbolsToScope(globalScope,t);
	ItemclassSymbols::getInst().addSymbolsToScope(globalScope,t);
	LinkSymbols::getInst().addSymbolsToScope(globalScope,t);
	ScreenSymbols::getInst().addSymbolsToScope(globalScope,t);
	GameSymbols::getInst().addSymbolsToScope(globalScope,t);

	//strip the global functions from the AST
	GetGlobalFuncs gc;
	theAST->execute(gc, NULL);
	vector<ASTFuncDecl *> fds = gc.getResult();
	//add these functions to the global scope
	for(vector<ASTFuncDecl *>::iterator it = fds.begin(); it != fds.end(); it++)
	{
		vector<int> params;
		for(list<ASTVarDecl *>::iterator it2 = (*it)->getParams().begin();
			it2 != (*it)->getParams().end(); it2++)
		{
			int type;
			ExtractType temp;
			(*it2)->getType()->execute(temp, &type);
			if(type == ScriptParser::TYPE_VOID)
			{
				printErrorMsg(*it2, FUNCTIONVOIDPARAM, (*it2)->getName());
				failure=true;
			}
			params.push_back(type);
		}
		int rettype;
		ExtractType temp;
		(*it)->getReturnType()->execute(temp, &rettype);
		int id = globalScope->getFuncSymbols().addFunction((*it)->getName(), rettype, params);
		if(id == -1)
		{
			printErrorMsg(*it, FUNCTIONREDEF, (*it)->getName());
			failure=true;
		}
		if(failure)
		{
			for(vector<ASTFuncDecl *>::iterator it2 = fds.begin(); it2 != fds.end(); it2++)
			{
				delete *it2;
			}
			delete globalScope;
			delete t;
			delete rval;
			delete theAST;
			return NULL;
		}
		t->putAST(*it, id);
		t->putFunc(id, rettype);
		t->putFuncDecl(id, params);
		
	}
	rval->globalFuncs = fds;
	//put script variables and functions in their script subscopes
	GetScripts gs;
	theAST->execute(gs, NULL);
	vector<ASTScript *> scripts = gs.getResult();
	for(vector<ASTScript *>::iterator it = scripts.begin(); it != scripts.end(); it++)
	{
		int scripttype;
		ExtractType et;
		(*it)->getType()->execute(et, &scripttype);
		if(!(scripttype == ScriptParser::TYPE_FFC || scripttype == ScriptParser::TYPE_ITEM
			|| scripttype == ScriptParser::TYPE_GLOBAL))
		{
			printErrorMsg(*it, SCRIPTBADTYPE, (*it)->getName());
			failure = true;
			continue;
		}
		Scope *subscope = new Scope(globalScope);
		if(!globalScope->addNamedChild((*it)->getName(), subscope))
		{
			printErrorMsg(*it, SCRIPTREDEF, (*it)->getName());
			failure = true;
			delete subscope;
			continue;
		}
		//add the "this" pointer
		int vid = subscope->getVarSymbols().addVariable("this", scripttype);
		t->putVar(vid, scripttype);
		rval->thisPtr[*it]=vid;
		//t->addGlobalPointer(vid);
		//add a Link global variable
		vid = subscope->getVarSymbols().addVariable("Link", ScriptParser::TYPE_LINK);
		t->putVar(vid, ScriptParser::TYPE_LINK);
		t->addGlobalPointer(vid);
		//add a Screen global variable
		vid = subscope->getVarSymbols().addVariable("Screen", ScriptParser::TYPE_SCREEN);
		t->putVar(vid, ScriptParser::TYPE_SCREEN);
		t->addGlobalPointer(vid);
		//add a Game global variable
		vid = subscope->getVarSymbols().addVariable("Game", ScriptParser::TYPE_GAME);
		t->putVar(vid, ScriptParser::TYPE_GAME);
		t->addGlobalPointer(vid);

		pair<Scope *, SymbolTable *> param(subscope, t);
		BuildScriptSymbols bss;
		(*it)->execute(bss, &param);
		if(!bss.isOK())
			failure=true;
		else
		{
			//find the start symbol
			vector<int> possibleruns = subscope->getFuncsInScope((*it)->getName(), "run");
			int runid = -1;
			if(possibleruns.size() > 1)
			{
				printErrorMsg(*it, TOOMANYRUN, (*it)->getName());
				failure = true;
			}
			else if(possibleruns.size() == 1)
				runid = possibleruns[0];
			if(!failure)
			{
				if(runid == -1)
				{
					printErrorMsg(*it, SCRIPTNORUN, (*it)->getName());
					failure = true;
				}
				else
				{
					int type = t->getFuncType(runid);
					if(type != ScriptParser::TYPE_VOID)
					{
						printErrorMsg(*it, SCRIPTRUNNOTVOID, (*it)->getName());
						failure = true;
					}
					else
					{
						rval->runsymbols[*it] = runid;
						rval->numParams[*it] = (int)t->getFuncParams(runid).size();
						rval->scriptTypes[*it] = scripttype;
					}
				}
			}
		}
	}
	//all non-local variables have been added to the table, so process all function
	//declarations and add their local variables. As well, look up the symbol id of
	//all variables and functions accessed and called within that function

	//starting with global functions
	if(!failure)
	{
		for(vector<ASTFuncDecl *>::iterator it = fds.begin(); it != fds.end(); it++)
		{
			Scope *subscope = new Scope(globalScope);
			pair<Scope *, SymbolTable *> param(subscope, t);
			BuildFunctionSymbols bfs;
			(*it)->execute(bfs, &param);
			if(!bfs.isOK())
				failure = true;
			delete subscope;
		}
	}
	//now do script function
	if(!failure)
	{
		for(vector<ASTScript *>::iterator it = scripts.begin(); it != scripts.end(); it++)
		{
			Scope *subscope = globalScope->getNamedChild((*it)->getName());
			Scope *newscope = new Scope(subscope);
			pair<Scope *, SymbolTable *> param(newscope, t);
			list<ASTDecl *> decls = (*it)->getScriptBlock()->getDeclarations();
			for(list<ASTDecl *>::iterator it2 = decls.begin(); it2 != decls.end(); it2++)
			{
				bool isfuncdecl;
				IsFuncDecl temp;
				(*it2)->execute(temp, &isfuncdecl);
				if(isfuncdecl)
				{
					BuildFunctionSymbols bfs;
					(*it2)->execute(bfs, &param);
					if(!bfs.isOK())
					failure = true;
				}
				
			}
			delete newscope;
		}
	}
	if(failure)
	{
		for(vector<ASTFuncDecl *>::iterator it2 = fds.begin(); it2 != fds.end(); it2++)
		{
			delete *it2;
		}
		for(vector<ASTScript *>::iterator it2 = scripts.begin(); it2 != scripts.end(); it2++)
		{
			delete *it2;
		}
		delete globalScope;
		delete t;
		delete rval;
		delete theAST;
		return NULL;
	}
	delete globalScope;
	delete theAST;
	rval->symbols = t;
	rval->scripts = scripts;
	rval->globalFuncs = fds;

	return rval;
}

FunctionData *ScriptParser::typeCheck(SymbolData *sdata)
{
	//build the functiondata
	FunctionData *fd = new FunctionData;
	fd->symbols = sdata->symbols;
	vector<ASTScript *> scripts = sdata->scripts;
	vector<ASTFuncDecl *> funcs = sdata->globalFuncs;
	map<ASTScript *, int> runsymbols = sdata->runsymbols;
	map<ASTScript *, int> numparams = sdata->numParams;
	map<ASTScript *, int> scripttypes = sdata->scriptTypes;
	map<ASTScript *, int> thisptr = sdata->thisPtr;
	delete sdata;
	bool failure =false;
	map<int, bool> usednums;
	//strip var and func decls from the scripts
	for(vector<ASTScript *>::iterator it = scripts.begin(); it != scripts.end(); it++)
	{
		fd->scriptRunSymbols[(*it)->getName()] = runsymbols[*it];
		fd->numParams[(*it)->getName()] = numparams[*it];
		fd->scriptTypes[(*it)->getName()] = scripttypes[*it];
		fd->thisPtr[(*it)->getName()] = thisptr[*it];
		//strip vars and funcs
		list<ASTDecl *> &stuff = (*it)->getScriptBlock()->getDeclarations();
		for(list<ASTDecl *>::iterator it2 = stuff.begin(); it2 != stuff.end();)
		{
			bool isFunc = false;
			IsFuncDecl temp;
			(*it2)->execute(temp, &isFunc);
			if(isFunc)
			{
				fd->functions.push_back((ASTFuncDecl *)*it2);
			}
			else
			{
				fd->globalVars.push_back((ASTVarDecl *)*it2);
			}
			it2 = stuff.erase(it2);
		}
	}
	for(vector<ASTScript *>::iterator it = scripts.begin(); it != scripts.end(); it++)
	{
		delete *it;
	}
	for(vector<ASTFuncDecl *>::iterator it = funcs.begin(); it != funcs.end(); it++)
	{
		fd->functions.push_back(*it);
	}
	if(failure)
	{
		//delete stuff
		for(vector<ASTVarDecl *>::iterator it = fd->globalVars.begin(); it != fd->globalVars.end(); it++)
		{
			delete *it;
		}
		for(vector<ASTFuncDecl *>::iterator it = fd->functions.begin(); it != fd->functions.end(); it++)
		{
			delete *it;
		}
		delete fd->symbols;
		delete fd;
		return NULL;
	}
	//fd is now loaded with all the info
	//so run type-checker visitor
	for(vector<ASTVarDecl *>::iterator it = fd->globalVars.begin(); it != fd->globalVars.end(); it++)
	{
		pair<SymbolTable *, int> param = pair<SymbolTable *, int>(fd->symbols, -1);
		TypeCheck tc;
		(*it)->execute(tc, &param);
		if(!tc.isOK())
			failure = true;
	}
	for(vector<ASTFuncDecl *>::iterator it = fd->functions.begin(); it != fd->functions.end(); it++)
	{
		int rettype = fd->symbols->getFuncType(*it);
		pair<SymbolTable *, int> param = pair<SymbolTable *, int>(fd->symbols, rettype);
		TypeCheck tc;
		(*it)->execute(tc, &param);
		if(!tc.isOK())
			failure = true;
	}
	if(fd->globalVars.size() > 256)
	{
		printErrorMsg(NULL, TOOMANYGLOBAL);
		failure = true;
	}
	if(failure)
	{
		//delete stuff
		for(vector<ASTVarDecl *>::iterator it = fd->globalVars.begin(); it != fd->globalVars.end(); it++)
		{
			delete *it;
		}
		for(vector<ASTFuncDecl *>::iterator it = fd->functions.begin(); it != fd->functions.end(); it++)
		{
			delete *it;
		}
		delete fd->symbols;
		delete fd;
		return NULL;
	}
	return fd;
}

IntermediateData *ScriptParser::generateOCode(FunctionData *fdata)
{
	vector<ASTFuncDecl *> funcs = fdata->functions;
	vector<ASTVarDecl *> globals = fdata->globalVars;
	map<string, int> runsymbols = fdata->scriptRunSymbols;
	SymbolTable *symbols = fdata->symbols;
	map<string, int> numparams = fdata->numParams;
	map<string, int> scripttypes = fdata->scriptTypes;
	map<string, int> thisptr = fdata->thisPtr;
	delete fdata;
	LinkTable lt;
	for(vector<ASTVarDecl *>::iterator it = globals.begin(); it != globals.end(); it++)
	{
		int vid = symbols->getID(*it);
		lt.addGlobalVar(vid);
	}
	//and add the this pointers
	for(vector<int>::iterator it = symbols->getGlobalPointers().begin(); it != symbols->getGlobalPointers().end(); it++)
	{
		lt.addGlobalPointer(*it);
	}
	for(vector<ASTFuncDecl *>::iterator it = funcs.begin(); it != funcs.end(); it++)
	{
		int fid = symbols->getID(*it);
		lt.functionToLabel(fid);
	}
	//we now have labels for the functions and ids for the global variables.
	//we can now generate the code to intialize the globals
	IntermediateData *rval = new IntermediateData();
	//Link against the global symbols, and add their labels
	map<int, vector<Opcode *> > globalcode = GlobalSymbols::getInst().addSymbolsCode(lt);
	for(map<int, vector<Opcode *> >::iterator it = globalcode.begin(); it != globalcode.end(); it++)
	{
		rval->funcs[it->first] = it->second;
	}
	globalcode = FFCSymbols::getInst().addSymbolsCode(lt);
	for(map<int, vector<Opcode *> >::iterator it = globalcode.begin(); it != globalcode.end(); it++)
	{
		rval->funcs[it->first] = it->second;
	}
	globalcode = ItemSymbols::getInst().addSymbolsCode(lt);
	for(map<int, vector<Opcode *> >::iterator it = globalcode.begin(); it != globalcode.end(); it++)
	{
		rval->funcs[it->first] = it->second;
	}
	globalcode = ItemclassSymbols::getInst().addSymbolsCode(lt);
	for(map<int, vector<Opcode *> >::iterator it = globalcode.begin(); it != globalcode.end(); it++)
	{
		rval->funcs[it->first] = it->second;
	}
	globalcode = LinkSymbols::getInst().addSymbolsCode(lt);
	for(map<int, vector<Opcode *> >::iterator it = globalcode.begin(); it != globalcode.end(); it++)
	{
		rval->funcs[it->first] = it->second;
	}
	globalcode = ScreenSymbols::getInst().addSymbolsCode(lt);
	for(map<int, vector<Opcode *> >::iterator it = globalcode.begin(); it != globalcode.end(); it++)
	{
		rval->funcs[it->first] = it->second;
	}
	globalcode = GameSymbols::getInst().addSymbolsCode(lt);
	for(map<int, vector<Opcode *> >::iterator it = globalcode.begin(); it != globalcode.end(); it++)
	{
		rval->funcs[it->first] = it->second;
	}

	for(vector<ASTVarDecl *>::iterator it = globals.begin(); it != globals.end(); it++)
	{
		OpcodeContext oc;
		oc.linktable = &lt;
		oc.symbols = symbols;
		oc.stackframe = NULL;
		BuildOpcodes bo;
		(*it)->execute(bo, &oc);
		vector<Opcode *> code = bo.getResult();
		for(vector<Opcode *>::iterator it2 = code.begin(); it2!= code.end(); it2++)
		{
			rval->globalsInit.push_back(*it2);
		}
		delete *it; //say so long to our lovely data structure the AST
	}
	
	//globals have been initialized, now we repeat for the functions
	for(vector<ASTFuncDecl *>::iterator it = funcs.begin(); it != funcs.end(); it++)
	{
		bool isarun = false;
		string scriptname;
		for(map<string,int>::iterator it2 = runsymbols.begin(); it2 != runsymbols.end(); it2++)
		{
			if(it2->second == symbols->getID(*it))
			{
				isarun=true;
				scriptname = it2->first;
				break;
			}
		}
		vector<Opcode *> funccode;
		//count the number of stack-allocated variables
		vector<int> stackvars;
		pair<vector<int> *, SymbolTable *> param = pair<vector<int> *, SymbolTable *>(&stackvars,symbols);
		CountStackSymbols temp;
		(*it)->execute(temp, &param);
		int offset = 0;
		StackFrame sf;
		
		//the params are now the first elements of this list
		//so assign them depths in reverse order
		for(vector<int>::reverse_iterator it2 = stackvars.rbegin(); it2 != stackvars.rend(); it2++)
		{
			sf.addToFrame(*it2, offset);
			offset+=10000;
		}
		//if this is a run, there is the this pointer
		if(isarun)
		{
			sf.addToFrame(thisptr[scriptname], offset);
			offset+=10000;
		}
		//start of the function
		Opcode *first = new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0));
		first->setLabel(lt.functionToLabel(symbols->getID(*it)));
		funccode.push_back(first);
		//push on the this, if a script
		if(isarun)
		{
			switch(scripttypes[scriptname])
			{
			case ScriptParser::TYPE_FFC:
				funccode.push_back(new OSetRegister(new VarArgument(EXP2), new VarArgument(REFFFC)));
				break;
			case ScriptParser::TYPE_ITEM:
				funccode.push_back(new OSetRegister(new VarArgument(EXP2), new VarArgument(REFITEM)));
				break;
			case ScriptParser::TYPE_GLOBAL:
				//don't care, we don't have a valid this pointer
				break;
			}
			funccode.push_back(new OPushRegister(new VarArgument(EXP2)));
		}
		//push on the 0s
		int numtoallocate = (unsigned int)stackvars.size()-(unsigned int)symbols->getFuncParams(symbols->getID(*it)).size();
		for(int i=0;i<numtoallocate; i++)
		{
			funccode.push_back(new OPushRegister(new VarArgument(EXP1)));
		}
		//set up the stack frame register
		funccode.push_back(new OSetRegister(new VarArgument(SFRAME), new VarArgument(SP)));
		OpcodeContext oc;
		oc.linktable = &lt;
		oc.symbols = symbols;
		oc.stackframe = &sf;
		BuildOpcodes bo;
		(*it)->execute(bo, &oc);
		vector<Opcode *> code = bo.getResult();
		for(vector<Opcode *>::iterator it2 = code.begin(); it2 != code.end(); it2++)
		{
			funccode.push_back(*it2);
		}
		//add appendix code
		//nop label
		Opcode *next = new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(0));
		next->setLabel(bo.getReturnLabelID());
		funccode.push_back(next);
		//pop off everything
		for(unsigned int i=0; i< stackvars.size(); i++)
		{
			funccode.push_back(new OPopRegister(new VarArgument(EXP2)));
		}
		//if it's a main script, quit.
		if(isarun)
			funccode.push_back(new OQuit());
		else
		{
			//pop off the return address
			funccode.push_back(new OPopRegister(new VarArgument(EXP2)));
			//and return
			funccode.push_back(new OGotoRegister(new VarArgument(EXP2)));
		}
		rval->funcs[lt.functionToLabel(symbols->getID(*it))]=funccode;
		delete *it;
	}
	
	//update the run symbols
	for(map<string, int>::iterator it = runsymbols.begin(); it != runsymbols.end(); it++)
	{
		int labelid = lt.functionToLabel(it->second);
		rval->scriptRunLabels[it->first] = labelid;
		rval->numParams[it->first] = numparams[it->first];
		rval->scriptTypes[it->first] = scripttypes[it->first];
	}
	delete symbols; //and so long to our beloved ;) symbol table
	return rval;
}

ScriptsData *ScriptParser::assemble(IntermediateData *id)
{
	//finally, finish off this bitch
	ScriptsData *rval = new ScriptsData;
	map<int, vector<Opcode *> > funcs = id->funcs;
	vector<Opcode *> ginit = id->globalsInit;
	map<string, int> scripts = id->scriptRunLabels;
	map<string, int> numparams = id->numParams;
	map<string, int> scripttypes = id->scriptTypes;
	delete id;
	//do the global inits
	//if there's a global script called "Init", append it to ~Init:
	map<string, int>::iterator it = scripts.find("Init");
	if(it != scripts.end() && scripttypes["Init"] == ScriptParser::TYPE_GLOBAL)
	{
		//append
		//get label
		int label = funcs[scripts["Init"]][0]->getLabel();
		ginit.push_back(new OGotoImmediate(new LabelArgument(label)));
	}
	rval->theScripts["~Init"] = assembleOne(ginit, funcs,0);
	rval->scriptTypes["~Init"] = ScriptParser::TYPE_GLOBAL;

	for(map<string, int>::iterator it = scripts.begin(); it != scripts.end(); it++)
	{
		vector<Opcode *> code = funcs[it->second];
		rval->theScripts[it->first] = assembleOne(code,funcs,numparams[it->first]);
		rval->scriptTypes[it->first] = scripttypes[it->first];
	}
	for(vector<Opcode *>::iterator it = ginit.begin(); it != ginit.end(); it++)
	{
		delete *it;
	}
	for(map<int, vector<Opcode *> >::iterator it = funcs.begin(); it != funcs.end(); it++)
	{
		for(vector<Opcode *>::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++)
		{
			delete *it2;
		}
	}
	return rval;
}

vector<Opcode *> ScriptParser::assembleOne(vector<Opcode *> script, map<int, vector<Opcode *> > &otherfuncs, int numparams)
{
	vector<Opcode *> rval;
	//first, push on the params to the run
	int i;
	for(i=0; i<numparams && i<9; i++)
	{
		rval.push_back(new OPushRegister(new VarArgument(i)));
	}
	for(;i<numparams; i++)
	{
		rval.push_back(new OPushRegister(new VarArgument(EXP1)));
	}
	//next, find all labels jumped to by the script code
	map<int,bool> labels;
	for(vector<Opcode *>::iterator it = script.begin(); it != script.end(); it++)
	{
		GetLabels temp;
		(*it)->execute(temp, &labels);
	}
	//do some fixed-point bullshit
	size_t oldnumlabels = 0;
	while(oldnumlabels != labels.size())
	{
		oldnumlabels = labels.size();
		for(map<int,bool>::iterator lit = labels.begin(); lit != labels.end(); lit++)
		{
			map<int, vector<Opcode *> >::iterator it = otherfuncs.find(lit->first);
			if(it == otherfuncs.end())
				continue;
			for(vector<Opcode *>::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++)
			{
				GetLabels temp;
				(*it2)->execute(temp, &labels);
			}
		}
	}
	//make the rval
	for(vector<Opcode *>::iterator it = script.begin(); it != script.end(); it++)
	{
		rval.push_back((*it)->makeClone());
	}
	for(map<int,bool>::iterator it = labels.begin(); it != labels.end(); it++)
	{
		map<int, vector<Opcode *> >::iterator it2 = otherfuncs.find(it->first);
		if(it2 != otherfuncs.end())
		{
			for(vector<Opcode *>::iterator it3 = (*it2).second.begin(); it3 != (*it2).second.end(); it3++)
			{
				rval.push_back((*it3)->makeClone());
			}
		}
	}
	//set the label line numbers
	map<int, int> linenos;
	int lineno=1;
	for(vector<Opcode *>::iterator it = rval.begin(); it != rval.end(); it++)
	{
		if((*it)->getLabel() != -1)
		{
			linenos[(*it)->getLabel()]=lineno;
		}
		lineno++;
	}
	//now fill in those labels
	for(vector<Opcode *>::iterator it = rval.begin(); it != rval.end(); it++)
	{
		SetLabels temp;
		(*it)->execute(temp, &linenos);
	}
	return rval;
}

int StackFrame::getOffset(int vid)
{
	map<int, int>::iterator it = stackoffset.find(vid);
	if(it == stackoffset.end())
	{
		box_out("Internal Error: Can't find variable stack offset!");
		box_eol();
		return 0;
	}
	return stackoffset[vid];
}