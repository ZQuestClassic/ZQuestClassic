#include "SymbolVisitors.h"
#include "DataStructs.h"
#include "UtilVisitors.h"
#include "ParseError.h"
#include <assert.h>


void BuildScriptSymbols::caseDefault(void *param)
{
  //these are here to bypass compiler warnings about unused arguments
  param=param;

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
		|| type == ScriptParser::TYPE_ITEMCLASS || type == ScriptParser::TYPE_NPC)
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
	if(id == -1 && !(nspace == "" && p->second->isConstant(name)))
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
	//recur on the index, if it exists
	if(host.getIndex())
		host.getIndex()->execute(*this,param);
	//wait for type-checking to do rest of work
}
