#include "../precompiled.h" //always first

#include "SymbolVisitors.h"
#include "DataStructs.h"
#include "UtilVisitors.h"
#include "ParseError.h"
#include <assert.h>

////////////////////////////////////////////////////////////////
// BuildScriptSymbols

void BuildScriptSymbols::caseDefault(void *) {}

// Declarations

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
	Scope& scope = *(Scope*)param;
	SymbolTable& symbolTable = scope.getTable();
    string name = host.getName();
    vector<ZVarTypeId> paramTypeIds;
    list<ASTVarDecl *> vds = host.getParams();

    for (list<ASTVarDecl*>::iterator it = vds.begin(); it != vds.end(); it++)
    {
		ZVarType const& type = (*it)->getType()->resolve(scope);
        paramTypeIds.push_back(symbolTable.getOrAssignTypeId(type));

        if (type == ZVarType::VOID)
        {
            failure = true;
            printErrorMsg(&host, FUNCTIONVOIDPARAM, name);
        }
    }

	ZVarType const& returnType = host.getReturnType()->resolve(scope);
	ZVarTypeId returnTypeId = symbolTable.getOrAssignTypeId(returnType);
    int id = scope.addFunc(name, returnTypeId, paramTypeIds, &host);

    if (id == -1)
    {
        failure = true;
        printErrorMsg(&host, FUNCTIONREDEF, name);
        return;
    }
}

void BuildScriptSymbols::caseArrayDecl(ASTArrayDecl &host, void *param)
{
	Scope& scope = *(Scope*)param;
	SymbolTable& symbolTable = scope.getTable();
    string name = host.getName();

    ZVarType const& type = host.getType()->resolve(scope);

    if (type == ZVarType::VOID)
    {
        failure = true;
        printErrorMsg(&host, VOIDARR, name);
    }

    if (type == ZVarType::FFC || type == ZVarType::ITEM || type == ZVarType::ITEMCLASS
		|| type == ZVarType::NPC || type == ZVarType::LWPN || type == ZVarType::EWPN)
    {
        failure = true;
        printErrorMsg(&host, REFARR, name);
    }

    // var is always visible
	ZVarTypeId typeId = symbolTable.getOrAssignTypeId(type);
    int id = scope.addVar(name, typeId, &host);

    if (id == -1)
    {
        failure = true;
        printErrorMsg(&host, ARRREDEF, name);
        return;
    }

    if (this->deprecateGlobals)
    {
        printErrorMsg(&host, DEPRECATEDGLOBAL, name);
    }

	((ASTExpr *)host.getSize())->execute(*this, param);

    if(host.getList() != NULL)
    {
        for(list<ASTExpr *>::iterator it = host.getList()->getList().begin(); it != host.getList()->getList().end(); it++)
        {
            (*it)->execute(*this, param);
        }
    }
}

void BuildScriptSymbols::caseVarDecl(ASTVarDecl &host, void *param)
{
	Scope& scope = *(Scope*)param;
	SymbolTable& symbolTable = scope.getTable();
    string name = host.getName();

	ZVarType const& type = host.getType()->resolve(scope);
	ZVarTypeId typeId = symbolTable.getOrAssignTypeId(type);

    if (type == ZVarType::VOID)
    {
        failure = true;
        printErrorMsg(&host, VOIDVAR, name);
    }

    if (type == ZVarType::FFC || type == ZVarType::ITEM || type == ZVarType::ITEMCLASS
		|| type == ZVarType::NPC || type == ZVarType::LWPN || type == ZVarType::EWPN)
    {
        failure = true;
        printErrorMsg(&host, REFVAR, name);
    }

    // Var is always visible.
    int id = scope.addVar(name, typeId, &host);

    if (id == -1)
    {
        failure = true;
        printErrorMsg(&host, VARREDEF, name);
        return;
    }

    if (this->deprecateGlobals)
    {
        printErrorMsg(&host, DEPRECATEDGLOBAL, name);
    }
}

void BuildScriptSymbols::caseVarDeclInitializer(ASTVarDeclInitializer &host, void *param)
{
    host.getInitializer()->execute(*this, param);
    caseVarDecl(host, param);
}

void BuildScriptSymbols::caseTypeDef(ASTTypeDef& host, void* param)
{
	Scope& scope = *(Scope*)param;
	SymbolTable& table = scope.getTable();

	ZVarType const& type = host.getType()->resolve(scope);
	ZVarTypeId typeId = table.getOrAssignTypeId(type);
	scope.addType(host.getName(), typeId, &host);
}

// Expressions

void BuildScriptSymbols::caseStringConstant(ASTStringConstant& host, void* param)
{
	Scope& scope = *(Scope*)param;
	BuildFunctionSymbols bfs;
	BFSParam subParam(scope);
	host.execute(bfs, &subParam);

	if (!bfs.isOK()) failure = true;
}

void BuildScriptSymbols::caseExprDot(ASTExprDot &host, void *param)
{
	Scope& scope = *(Scope*)param;
    BuildFunctionSymbols bfs;
	BFSParam subParam(scope);
	host.execute(bfs, &subParam);

    if (!bfs.isOK()) failure = true;
}

void BuildScriptSymbols::caseExprArray(ASTExprArray &host, void *param)
{
	Scope& scope = *(Scope*)param;
    BuildFunctionSymbols bfs;
    BFSParam newp(scope);
    host.execute(bfs, &newp);

    if (!bfs.isOK()) failure = true;

    host.getIndex()->execute(bfs, &newp);
}

void BuildScriptSymbols::caseExprArrow(ASTExprArrow &host, void*)
{
    printErrorMsg(&host, BADGLOBALINIT, "");
    failure = true;
}

void BuildScriptSymbols::caseFuncCall(ASTFuncCall &host, void*)
{
    printErrorMsg(&host, BADGLOBALINIT, "");
    failure = true;
}

//////////////////////////////////////////////////////////////
// BuildFunctionSymbols

// Statements

void BuildFunctionSymbols::caseBlock(ASTBlock& host, void* param)
{
    // Push in a new scope.
    BFSParam& p = *(BFSParam*)param;
    Scope blockScope(&p.scope);
    BFSParam newParam(blockScope, p.type);

    list<ASTStmt*> stmts = host.getStatements();
    for (list<ASTStmt*>::iterator it = stmts.begin(); it != stmts.end(); it++)
        (*it)->execute(*this, &newParam);
}

void BuildFunctionSymbols::caseStmtFor(ASTStmtFor &host, void *param)
{
    // Push in new scope (in accordance with C++ scoping).
    BFSParam& p = *(BFSParam*)param;
    Scope blockScope(&p.scope);
    BFSParam newParam(blockScope, p.type);
    host.getPrecondition()->execute(*this, &newParam);
    host.getTerminationCondition()->execute(*this, &newParam);
    host.getIncrement()->execute(*this, &newParam);
    ASTStmt* stmt = host.getStmt();
    stmt->execute(*this, &newParam);
}

// Declarations

void BuildFunctionSymbols::caseFuncDecl(ASTFuncDecl &host, void *param)
{
    BFSParam& p = *(BFSParam*)param;
    // Push in the scope.
    Scope blockScope(&p.scope);
	SymbolTable& table = p.scope.getTable();
    BFSParam newParam(blockScope, p.type);

    // If it's a run method, add "this".
	ZVarType const& returnType = host.getReturnType()->resolve(p.scope);

    if (host.getName() == "run" && returnType == ZVarType::VOID)
    {
		ZVarTypeId thisTypeId = ScriptParser::getThisType(p.type);
        int vid = blockScope.addVar("this", thisTypeId);
        table.putVarTypeId(vid, thisTypeId);
        thisvid = vid;
    }

    // Add the params.
    list<ASTVarDecl*> vars = host.getParams();

    for (list<ASTVarDecl*>::iterator it = vars.begin(); it != vars.end(); it++)
    {
        string name = (*it)->getName();
		ZVarType const& type = (*it)->getType()->resolve(blockScope);
		ZVarTypeId typeId = table.getOrAssignTypeId(type);
        int id = blockScope.addVar(name, typeId);

        if (id == -1)
        {
            printErrorMsg(*it, VARREDEF, name);
            failure = true;
            return;
        }

        table.putVarTypeId(id, typeId);
        table.putNodeId(*it, id);
    }

    // Shortcut the block.
    list<ASTStmt*> stmts = host.getBlock()->getStatements();
    for(list<ASTStmt *>::iterator it = stmts.begin(); it != stmts.end(); it++)
        (*it)->execute(*this, &newParam);
}

void BuildFunctionSymbols::caseArrayDecl(ASTArrayDecl &host, void *param)
{
    BFSParam& p = *(BFSParam*)param;
	Scope& scope = p.scope;
	SymbolTable& table = scope.getTable();
    string name = host.getName();
	ZVarType const& type = host.getType()->resolve(scope);
	ZVarTypeId typeId = table.getOrAssignTypeId(type);
    int id = scope.addVar(name, typeId);

    if (id == -1)
    {
        printErrorMsg(&host, ARRREDEF, name);
        failure = true;
        return;
    }

    table.putNodeId(&host, id);
    table.putVarTypeId(id, typeId);

	((ASTExpr*)host.getSize())->execute(*this, param);

    if (host.getList() != NULL)
    {
        for (list<ASTExpr *>::iterator it = host.getList()->getList().begin(); it != host.getList()->getList().end(); it++)
        {
            (*it)->execute(*this, param);
        }
    }
}

void BuildFunctionSymbols::caseVarDecl(ASTVarDecl &host, void *param)
{
    BFSParam& p = *(BFSParam*)param;
	Scope& scope = p.scope;
	SymbolTable& table = scope.getTable();
    string name = host.getName();
	ZVarType const& type = host.getType()->resolve(scope);
	ZVarTypeId typeId = table.getOrAssignTypeId(type);
    int id = scope.addVar(name, typeId);

    if (id == -1)
    {
        printErrorMsg(&host, VARREDEF, name);
        failure = true;
        return;
    }

    table.putNodeId(&host, id);
    table.putVarTypeId(id, typeId);
}

void BuildFunctionSymbols::caseVarDeclInitializer(ASTVarDeclInitializer &host, void *param)
{
	host.getInitializer()->execute(*this,param);
	caseVarDecl(host, param);
}

void BuildFunctionSymbols::caseTypeDef(ASTTypeDef& host, void* param)
{
    BFSParam& p = *(BFSParam*)param;
	Scope& scope = p.scope;
	SymbolTable& table = scope.getTable();

	ZVarType const& type = host.getType()->resolve(scope);
	ZVarTypeId typeId = table.getOrAssignTypeId(type);
	scope.addType(host.getName(), typeId, &host);
}

// Expressions

void BuildFunctionSymbols::caseStringConstant(ASTStringConstant& host, void* param)
{
	BFSParam& p = *(BFSParam*)param;
	int id = ScriptParser::getUniqueVarID();
	p.scope.getTable().putNodeId(&host, id);
}

void BuildFunctionSymbols::caseFuncCall(ASTFuncCall &host, void *param)
{
    BFSParam& p = *(BFSParam*)param;
	Scope& scope = p.scope;
	SymbolTable& table = scope.getTable();

    ASTExpr* ident = host.getName();
    // ident could be a dotexpr, or an arrow exp. If it's an arrow exp, it is
    // not my problem right now.
    bool isdot = false;
    IsDotExpr temp;
    ident->execute(temp, &isdot);

    if (!isdot)
    {
        // Recur to get any dotexprs inside.
        ident->execute(*this, param);
    }
    else
    {
        // Resolve the possible functions.
        ASTExprDot* dotname = (ASTExprDot*)host.getName();
        string name = dotname->getName();
        string nspace = dotname->getNamespace();
        vector<int> possibleFuncs = scope.getFuncIds(nspace, name);

        if (possibleFuncs.size() == 0)
        {
            string fullname;

            if (nspace == "")
                fullname = name;
            else
                fullname = nspace + "." + name;

            printErrorMsg(&host, FUNCUNDECLARED, fullname);
            failure = true;
            return;
        }

        table.putPossibleNodeFuncIds(&host, possibleFuncs);
    }

    for (list<ASTExpr*>::iterator it = host.getParams().begin(); it != host.getParams().end(); it++)
        (*it)->execute(*this, param);
}

void BuildFunctionSymbols::caseExprDot(ASTExprDot &host, void *param)
{
    BFSParam& p = *(BFSParam*)param;
	Scope& scope = p.scope;
	SymbolTable& table = scope.getTable();

    string name = host.getName();
    string nspace = host.getNamespace();
    int id = scope.getVarId(nspace, name);
    if (id == -1 && !(nspace == "" && table.isConstant(name)))
    {
        string fullname;

        if (nspace == "")
            fullname = name;
        else
            fullname = nspace + "." + name;

        printErrorMsg(&host, VARUNDECLARED, fullname);
        failure = true;
        return;
    }

    table.putNodeId(&host, id);
}

void BuildFunctionSymbols::caseExprArrow(ASTExprArrow &host, void *param)
{
    // Recur on the name.
    host.getLVal()->execute(*this, param);

    // Recur on the index, if it exists.
    if (host.getIndex())
        host.getIndex()->execute(*this, param);

    // Wait for type-checking to do rest of work.
}

void BuildFunctionSymbols::caseExprArray(ASTExprArray &host, void *param)
{
    BFSParam& p = *(BFSParam*)param;
	Scope& scope = p.scope;
	SymbolTable& table = scope.getTable();

    string name = host.getName();
    string nspace = host.getNamespace();
    int id = scope.getVarId(nspace, name);

    if (id == -1 && !(nspace == "" && table.isConstant(name)))
    {
        string fullname;

        if (nspace == "")
            fullname = name;
        else
            fullname = nspace + "." + name;

        printErrorMsg(&host, VARUNDECLARED, fullname);
        failure = true;
        return;
    }

    table.putNodeId(&host, id);

    if (host.getIndex())
        host.getIndex()->execute(*this, param);
}

