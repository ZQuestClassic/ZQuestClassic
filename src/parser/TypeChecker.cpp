//2.53 Updated to 16th Jan, 2017
#include "../precompiled.h" //always first

#include "TypeChecker.h"
#include "ParseError.h"
#include "GlobalSymbols.h"
#include "../zsyssimple.h"
#include <assert.h>
#include <string>

////////////////////////////////////////////////////////////////
// TypeCheck
    
TypeCheck::TypeCheck(SymbolTable& symbolTable)
	: symbolTable(symbolTable), returnType(ZVarType::VOID), failure(false)
{}

TypeCheck::TypeCheck(SymbolTable& symbolTable, ZVarTypeId returnTypeId)
	: symbolTable(symbolTable), returnType(*symbolTable.getType(returnTypeId)), failure(false)
{}

TypeCheck::TypeCheck(SymbolTable& symbolTable, ZVarType const& returnType)
	: symbolTable(symbolTable), returnType(returnType), failure(false)
{}


// Statements

void TypeCheck::caseStmtAssign(ASTStmtAssign& host)
{
    host.getRVal()->execute(*this);
    if (failure) return;

	ZVarTypeId ltype = getLValTypeId(*host.getLVal());
    if (failure) return;

    ZVarTypeId rtype = host.getRVal()->getType();

    if (!standardCheck(ltype, rtype, &host))
        failure = true;

	if (ltype == symbolTable.getTypeId(ZVarType::CONST_FLOAT))
	{
		printErrorMsg(&host, CONSTASSIGN);
		failure = true;
	}
}

void TypeCheck::caseStmtIf(ASTStmtIf &host)
{
    RecursiveVisitor::caseStmtIf(host);
    if (failure) return;

    ZVarTypeId type = host.getCondition()->getType();

    if (!standardCheck(ZVARTYPEID_BOOL, type, &host))
        failure = true;
}
        
void TypeCheck::caseStmtIfElse(ASTStmtIfElse &host)
{
    caseStmtIf(host);
    host.getElseStmt()->execute(*this);
}
        
void TypeCheck::caseStmtSwitch(ASTStmtSwitch &host)
{
	RecursiveVisitor::caseStmtSwitch(host);
	if (failure) return;

	ZVarTypeId type = host.getKey()->getType();
	if (!standardCheck(ZVARTYPEID_FLOAT, type, &host))
		failure = true;
}

void TypeCheck::caseStmtFor(ASTStmtFor &host)
{
    RecursiveVisitor::caseStmtFor(host);
    if (failure) return;

    ZVarTypeId type = host.getTerminationCondition()->getType();
    if (!standardCheck(ZVARTYPEID_BOOL, type, &host))
        failure = true;
}
        
void TypeCheck::caseStmtWhile(ASTStmtWhile &host)
{
    RecursiveVisitor::caseStmtWhile(host);
    if (failure) return;

    ZVarTypeId type = host.getCond()->getType();
    if (!standardCheck(ZVARTYPEID_BOOL, type, &host))
        failure = true;
}
    
void TypeCheck::caseStmtReturn(ASTStmtReturn &host)
{
    if (returnType != ZVarType::VOID)
    {
        printErrorMsg(&host, FUNCBADRETURN, returnType.getName());
        failure = true;
    }
}

void TypeCheck::caseStmtReturnVal(ASTStmtReturnVal &host)
{
    host.getReturnValue()->execute(*this);
    if (failure) return;

    if (!standardCheck(symbolTable.getTypeId(returnType), host.getReturnValue()->getType(), &host))
        failure = true;
}

// Declarations

void TypeCheck::caseArrayDecl(ASTArrayDecl &host)
{
	ASTExpr *size = host.getSize();
	size->execute(*this);
    
	if (size->getType() != ZVARTYPEID_FLOAT)
	{
		printErrorMsg(&host, NONINTEGERARRAYSIZE, "");
		failure = true;
		return;
	}

    if (host.getList() != NULL)
    {
		ZVarTypeId arraytype = symbolTable.getVarTypeId(&host);
        list<ASTExpr *> l = host.getList()->getList();

        for(list<ASTExpr *>::iterator it = l.begin(); it != l.end(); it++)
        {
            (*it)->execute(*this);
            if (failure) return;

            if (!standardCheck(arraytype, (*it)->getType(), &host))
            {
                failure = true;
                return;
            }
        }
    }
}

void TypeCheck::caseVarDecl(ASTVarDecl &host)
{
	// Constants must be initialized.
	if (*symbolTable.getVarType(&host) == ZVarType::CONST_FLOAT)
	{
		printErrorMsg(&host, CONSTUNITIALIZED);
		failure = true;
	}
}

void TypeCheck::caseVarDeclInitializer(ASTVarDeclInitializer &host)
{
    ASTExpr* init = host.getInitializer();
    init->execute(*this);
    if (failure) return;

    ZVarTypeId type = init->getType();
    ZVarTypeId ltype = symbolTable.getVarTypeId(&host);

    if (!standardCheck(ltype, type, &host))
        failure = true;

	// If a constant, save it as an inlined value.
	if (*symbolTable.getVarType(&host) == ZVarType::CONST_FLOAT)
	{
		if (init->hasIntValue())
			symbolTable.inlineConstant(&host, init->getIntValue());
	}
}

// Expressions

void TypeCheck::caseExprConst(ASTExprConst &host)
{
	ASTExpr* content = host.getContent();
	content->execute(*this);

	if (!host.isConstant())
	{
		failure = true;
        printErrorMsg(&host, EXPRNOTCONSTANT);
		return;
	}

	host.setType(content->getType());
	if (content->hasIntValue())
		host.setIntValue(content->getIntValue());
}

void TypeCheck::caseNumConstant(ASTNumConstant &host)
{
    host.setType(ZVARTYPEID_FLOAT);
    pair<string,string> parts = host.getValue()->parseValue();
    pair<long, bool> val = ScriptParser::parseLong(parts);

    if (!val.second)
        printErrorMsg(&host, CONSTTRUNC, host.getValue()->getValue());

    host.setIntValue(val.first);
}
        
void TypeCheck::caseBoolConstant(ASTBoolConstant &host)
{
    host.setType(ZVARTYPEID_BOOL);
    host.setIntValue(host.getValue() ? 1L : 0L);
}
            
void TypeCheck::caseStringConstant(ASTStringConstant& host)
{
	host.setType(ZVARTYPEID_FLOAT);
}

void TypeCheck::caseExprDot(ASTExprDot &host)
{
    if (symbolTable.isInlinedConstant(&host))
	{
        host.setType(symbolTable.getTypeId(ZVarType::CONST_FLOAT));
		host.setIntValue(symbolTable.getInlinedValue(&host));
	}
    else
    {
        host.setType(symbolTable.getVarTypeId(&host));
    }
}
    
void TypeCheck::caseExprArrow(ASTExprArrow &host)
{
    // Annoyingly enough I have to treat arrowed variables as function calls
    // Get the left-hand type.
    host.getLVal()->execute(*this);
    if (failure) return;

    bool isIndexed = (host.getIndex() != NULL);
    if (isIndexed)
    {
        host.getIndex()->execute(*this);
        if (failure) return;

        if (!standardCheck(ZVARTYPEID_FLOAT, host.getIndex()->getType(), host.getIndex()))
        {
            failure = true;
            return;
        }
    }

    ZVarTypeId typeId = host.getLVal()->getType();
	LibrarySymbols* lib = LibrarySymbols::getTypeInstance(typeId);
	if (!lib)
    {
	    failure = true;
	    printErrorMsg(&host, ARROWNOTPOINTER);
	    return;
    }

	int functionId = lib->matchGetter(host.getName());

	vector<ZVarTypeId> functionParams = symbolTable.getFuncParamTypeIds(functionId);
    if (functionId == -1 || functionParams.size() != (isIndexed ? 2 : 1) || functionParams[0] != typeId)
    {
        failure = true;
        printErrorMsg(&host, ARROWNOVAR, host.getName() + (isIndexed ? "[]" : ""));
        return;
    }

    symbolTable.putNodeId(&host, functionId);
    host.setType(symbolTable.getFuncReturnTypeId(functionId));
}

void TypeCheck::caseExprArray(ASTExprArray &host)
{
    ZVarTypeId type  = symbolTable.getVarTypeId(&host);
    host.setType(type);

    if (host.getIndex())
    {
        host.getIndex()->execute(*this);
        if (failure) return;

        if (!standardCheck(ZVARTYPEID_FLOAT, host.getIndex()->getType(), host.getIndex()))
        {
            failure = true;
            return;
        }
    }
}
    
void TypeCheck::caseFuncCall(ASTFuncCall &host)
{
    // Yuck. Time to disambiguate these damn functions

    // Build the param types
    list<ASTExpr*> params = host.getParams();
    vector<ZVarTypeId> paramtypes;
    vector<int> possibleFuncIds;

    bool isdotexpr;
    IsDotExpr temp;
    host.getName()->execute(temp, &isdotexpr);

    // If this is a simple function, we already have what we need otherwise we
    // need the type of the thing being arrowed
    if (!isdotexpr)
    {
        ASTExprArrow* lval = (ASTExprArrow *)host.getName();
        lval->getLVal()->execute(*this);
        if (failure) return;
        ZVarTypeId lvaltype = lval->getLVal()->getType();

        if (!(lvaltype == ZVARTYPEID_FFC
              || lvaltype == ZVARTYPEID_LINK
			  || lvaltype == ZVARTYPEID_SCREEN
              || lvaltype == ZVARTYPEID_ITEM
			  || lvaltype == ZVARTYPEID_ITEMCLASS
              || lvaltype == ZVARTYPEID_GAME
              || lvaltype == ZVARTYPEID_NPC
              || lvaltype == ZVARTYPEID_LWPN
              || lvaltype == ZVARTYPEID_EWPN
              || lvaltype == ZVARTYPEID_NPCDATA
              || lvaltype == ZVARTYPEID_DEBUG
              || lvaltype == ZVARTYPEID_AUDIO
              || lvaltype == ZVARTYPEID_COMBOS
              || lvaltype == ZVARTYPEID_SPRITEDATA
              || lvaltype == ZVARTYPEID_GRAPHICS
              || lvaltype == ZVARTYPEID_TEXT
              || lvaltype == ZVARTYPEID_INPUT
              || lvaltype == ZVARTYPEID_MAPDATA
              || lvaltype == ZVARTYPEID_DMAPDATA
              || lvaltype == ZVARTYPEID_ZMESSAGE
              || lvaltype == ZVARTYPEID_SHOPDATA 
              || lvaltype == ZVARTYPEID_UNTYPED
              || lvaltype == ZVARTYPEID_DROPSET
              || lvaltype == ZVARTYPEID_PONDS
              || lvaltype == ZVARTYPEID_WARPRING
              || lvaltype == ZVARTYPEID_DOORSET
              || lvaltype == ZVARTYPEID_ZUICOLOURS
              || lvaltype == ZVARTYPEID_RGBDATA
              || lvaltype == ZVARTYPEID_PALETTE
              || lvaltype == ZVARTYPEID_TUNES
              || lvaltype == ZVARTYPEID_PALCYCLE
              || lvaltype == ZVARTYPEID_GAMEDATA
              || lvaltype == ZVARTYPEID_CHEATS
              ))
        {
	        printErrorMsg(lval, ARROWNOTPOINTER);
	        failure = true;
	        return;
        }

        // Prepend that type to the function parameters, as that is implicitly passed
        paramtypes.push_back(lvaltype);
    }

    // Now add the normal parameters.
    for (list<ASTExpr*>::iterator it = params.begin(); it != params.end(); it++)
    {
        (*it)->execute(*this);
        if (failure) return;

        paramtypes.push_back((*it)->getType());
    }

    string paramstring = "(";
    bool firsttype = true;
    for(vector<ZVarTypeId>::iterator it = paramtypes.begin(); it != paramtypes.end(); it++)
    {
        if (firsttype)
            firsttype = false;
        else
            paramstring += ", ";

        paramstring += symbolTable.getType(*it)->getName();
    }
    paramstring += ")";

    if (isdotexpr)
    {
        possibleFuncIds = symbolTable.getPossibleNodeFuncIds(&host);

        vector<pair<int, int> > matchedfuncs;

        for (vector<int>::iterator it = possibleFuncIds.begin(); it != possibleFuncIds.end(); it++)
        {
            vector<ZVarTypeId> itParams = symbolTable.getFuncParamTypeIds(*it);

            // See if they match.
            if (itParams.size() != paramtypes.size())
                continue;

            bool matched = true;
            int diffs = 0;

            for (unsigned int i = 0; i < itParams.size(); i++)
            {
                if (!standardCheck(itParams[i], paramtypes[i], NULL))
                {
                    matched = false;
                    break;
                }

                if (itParams[i] != paramtypes[i])
                    diffs++;
            }

            if (matched)
            {
                matchedfuncs.push_back(pair<int,int>(*it, diffs));
            }
        }

        // Now find the closest match *sigh*
        vector<int> bestmatch;
        int bestdiffs = 10000;

        for (vector<pair<int, int> >::iterator it = matchedfuncs.begin(); it != matchedfuncs.end(); it++)
        {
            if ((*it).second < bestdiffs)
            {
                bestdiffs = (*it).second;
                bestmatch.clear();
                bestmatch.push_back((*it).first);
            }
            else if ((*it).second == bestdiffs)
            {
                bestmatch.push_back((*it).first);
            }
        }

        string fullname;

        if (((ASTExprDot *)host.getName())->getNamespace() == "")
            fullname = ((ASTExprDot*)host.getName())->getName();
        else
            fullname = ((ASTExprDot *)host.getName())->getNamespace() + "." + ((ASTExprDot *)host.getName())->getName();

        if (bestmatch.size() == 0)
        {
            printErrorMsg(&host, NOFUNCMATCH, fullname + paramstring);
            failure = true;
            return;
        }
        else if (bestmatch.size() > 1)
        {
            printErrorMsg(&host, TOOFUNCMATCH, fullname+paramstring);
            failure = true;
            return;
        }

        // WHEW!
        host.setType(symbolTable.getFuncReturnTypeId(bestmatch[0]));
        symbolTable.putNodeId(&host, bestmatch[0]);
    }
    else
    {
        // Still have to deal with the (%&# arrow functions
        // Luckily I will here assert that each type's functions MUST be unique
		ASTExprArrow& arrow = *(ASTExprArrow*)host.getName();
        string name = arrow.getName();
        ZVarTypeId typeId = arrow.getLVal()->getType();

		LibrarySymbols* lib = LibrarySymbols::getTypeInstance(typeId);
		assert(lib);

		int functionId = lib->matchFunction(name);
        if (functionId == -1)
        {
            failure = true;
            printErrorMsg(&host, ARROWNOFUNC, name);
            return;
        }

		vector<ZVarTypeId> functionParams = symbolTable.getFuncParamTypeIds(functionId);
        if (paramtypes.size() != functionParams.size())
        {
            failure = true;
            printErrorMsg(&host, NOFUNCMATCH, name + paramstring);
            return;
        }

        for (unsigned int i = 0; i < paramtypes.size(); i++)
        {
            if (!standardCheck(functionParams[i], paramtypes[i], NULL))
            {
                failure = true;
                printErrorMsg (&host, NOFUNCMATCH, name + paramstring);
                return;
            }
        }

        host.setType(symbolTable.getFuncReturnTypeId(functionId));
        symbolTable.putNodeId(&host, functionId);
    }
}

void TypeCheck::caseExprNegate(ASTExprNegate &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT)) return;
    host.setType(ZVARTYPEID_FLOAT);

    if (host.getOperand()->hasIntValue())
    {
        long val = host.getOperand()->getIntValue();
        host.setIntValue(-val);
    }
}

void TypeCheck::caseExprNot(ASTExprNot &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_BOOL)) return;
    host.setType(ZVARTYPEID_BOOL);

    if (host.getOperand()->hasIntValue())
    {
        long val = host.getOperand()->getIntValue();
		host.setIntValue(!host.getOperand()->getIntValue());
    }
}

void TypeCheck::caseExprBitNot(ASTExprBitNot &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT)) return;
    host.setType(ZVARTYPEID_FLOAT);

    if (host.getOperand()->hasIntValue())
    {
        long val = host.getOperand()->getIntValue();
        host.setIntValue((~(val/10000))*10000);
    }
}

void TypeCheck::caseExprIncrement(ASTExprIncrement &host)
{
    host.getOperand()->execute(*this);
    if (failure) return;

    bool isexprdot;
    IsDotExpr temp;
    host.getOperand()->execute(temp, &isexprdot);

    if (!isexprdot)
    {
        int fid = symbolTable.getNodeId(host.getOperand());
        symbolTable.putNodeId(&host, fid);
    }

	ZVarTypeId ltype = getLValTypeId(*host.getOperand());
    if (failure) return;

    if (!standardCheck(ZVARTYPEID_FLOAT, ltype, &host))
    {
        failure = true;
        return;
    }

    host.setType(ZVARTYPEID_FLOAT);
}
    
void TypeCheck::caseExprPreIncrement(ASTExprPreIncrement &host)
{
    host.getOperand()->execute(*this);
    if (failure) return;

    bool isexprdot;
    IsDotExpr temp;
    host.getOperand()->execute(temp, &isexprdot);

    if (!isexprdot)
    {
        int fid = symbolTable.getNodeId(host.getOperand());
        symbolTable.putNodeId(&host, fid);
    }

	ZVarTypeId ltype = getLValTypeId(*host.getOperand());
    if (failure) return;

    if (!standardCheck(ZVARTYPEID_FLOAT, ltype, &host))
    {
        failure = true;
        return;
    }

    host.setType(ZVARTYPEID_FLOAT);
}

void TypeCheck::caseExprDecrement(ASTExprDecrement &host)
{
    host.getOperand()->execute(*this);
    if (failure) return;

    bool isexprdot;
    IsDotExpr temp;
    host.getOperand()->execute(temp, &isexprdot);

    if (!isexprdot)
    {
        int fid = symbolTable.getNodeId(host.getOperand());
        symbolTable.putNodeId(&host, fid);
    }

	ZVarTypeId ltype = getLValTypeId(*host.getOperand());
    if (failure) return;

    if (!standardCheck(ZVARTYPEID_FLOAT, ltype, &host))
    {
        failure = true;
        return;
    }

    host.setType(ZVARTYPEID_FLOAT);
}
    
void TypeCheck::caseExprPreDecrement(ASTExprPreDecrement &host)
{
    host.getOperand()->execute(*this);
    if (failure) return;

    bool isexprdot;
    IsDotExpr temp;
    host.getOperand()->execute(temp, &isexprdot);

    if (!isexprdot)
    {
        int fid = symbolTable.getNodeId(host.getOperand());
        symbolTable.putNodeId(&host, fid);
    }

	ZVarTypeId ltype = getLValTypeId(*host.getOperand());
    if (failure) return;

    if (!standardCheck(ZVARTYPEID_FLOAT, ltype, &host))
    {
        failure = true;
        return;
    }

    host.setType(ZVARTYPEID_FLOAT);
}

void TypeCheck::caseExprAnd(ASTExprAnd &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_BOOL, ZVARTYPEID_BOOL)) return;
    host.setType(ZVARTYPEID_BOOL);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();
		host.setIntValue(firstval && secondval);
    }
}

void TypeCheck::caseExprOr(ASTExprOr &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_BOOL, ZVARTYPEID_BOOL)) return;
    host.setType(ZVARTYPEID_BOOL);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();
		host.setIntValue(firstval || secondval);
    }
}

void TypeCheck::caseExprGT(ASTExprGT &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setType(ZVARTYPEID_BOOL);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();
		host.setIntValue(firstval > secondval);
    }
}

void TypeCheck::caseExprGE(ASTExprGE &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setType(ZVARTYPEID_BOOL);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();
		host.setIntValue(firstval >= secondval);
    }
}

void TypeCheck::caseExprLT(ASTExprLT &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setType(ZVARTYPEID_BOOL);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();
		host.setIntValue(firstval < secondval);
    }
}

void TypeCheck::caseExprLE(ASTExprLE &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setType(ZVARTYPEID_BOOL);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();
		host.setIntValue(firstval <= secondval);
    }
}

void TypeCheck::caseExprEQ(ASTExprEQ &host)
{
	host.getFirstOperand()->execute(*this);
	if (failure) return;
	host.getSecondOperand()->execute(*this);
	if (failure) return;

	if (!standardCheck(host.getFirstOperand()->getType(), host.getSecondOperand()->getType(), &host))
	{
		failure = true;
		return;
	}

    host.setType(ZVARTYPEID_BOOL);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();
		host.setIntValue(firstval == secondval);
    }
}

void TypeCheck::caseExprNE(ASTExprNE &host)
{
	host.getFirstOperand()->execute(*this);
	if (failure) return;
	host.getSecondOperand()->execute(*this);
	if (failure) return;

	if (!standardCheck(host.getFirstOperand()->getType(), host.getSecondOperand()->getType(), &host))
	{
		failure = true;
		return;
	}

    host.setType(ZVARTYPEID_BOOL);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();
		host.setIntValue(firstval != secondval);
    }
}

void TypeCheck::caseExprPlus(ASTExprPlus &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setType(ZVARTYPEID_FLOAT);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();
        host.setIntValue(firstval + secondval);
    }
}

void TypeCheck::caseExprMinus(ASTExprMinus &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setType(ZVARTYPEID_FLOAT);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();
        host.setIntValue(firstval - secondval);
    }
    }
    
void TypeCheck::caseExprTimes(ASTExprTimes &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setType(ZVARTYPEID_FLOAT);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();
        double temp = ((double)secondval)/10000.0;
        host.setIntValue((long)(firstval * temp));
    }
}

void TypeCheck::caseExprDivide(ASTExprDivide &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setType(ZVARTYPEID_FLOAT);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();

        if (secondval == 0)
        {
            printErrorMsg(&host, DIVBYZERO);
            failure = true;
            return;
        }

        host.setIntValue((firstval / secondval) * 10000);
    }
}

void TypeCheck::caseExprModulo(ASTExprModulo &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setType(ZVARTYPEID_FLOAT);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();

        if (secondval == 0)
        {
            printErrorMsg(&host, DIVBYZERO);
            failure = true;
            return;
        }

        host.setIntValue(firstval % secondval);
    }
}

void TypeCheck::caseExprBitAnd(ASTExprBitAnd &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setType(ZVARTYPEID_FLOAT);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();
        host.setIntValue(((firstval/10000)&(secondval/10000))*10000);
    }
}
    
void TypeCheck::caseExprBitOr(ASTExprBitOr &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setType(ZVARTYPEID_FLOAT);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();
        host.setIntValue(((firstval/10000)|(secondval/10000))*10000);
    }
}

void TypeCheck::caseExprBitXor(ASTExprBitXor &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setType(ZVARTYPEID_FLOAT);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();
        host.setIntValue(((firstval/10000)^(secondval/10000))*10000);
    }
}
void TypeCheck::caseExprLShift(ASTExprLShift &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;

    if (host.getSecondOperand()->hasIntValue())
    {
        if (host.getSecondOperand()->getIntValue() % 10000)
        {
            printErrorMsg(&host, SHIFTNOTINT);
            host.getSecondOperand()->setIntValue(10000*(host.getSecondOperand()->getIntValue()/10000));
        }
    }
    host.setType(ZVARTYPEID_FLOAT);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        int secondval = host.getSecondOperand()->getIntValue();
        host.setIntValue(((firstval/10000)<<(secondval/10000))*10000);
    }
}
void TypeCheck::caseExprRShift(ASTExprRShift &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;

    if (host.getSecondOperand()->hasIntValue())
    {
        if (host.getSecondOperand()->getIntValue() % 10000)
        {
            printErrorMsg(&host, SHIFTNOTINT);
            host.getSecondOperand()->setIntValue(10000*(host.getSecondOperand()->getIntValue()/10000));
        }
    }
    host.setType(ZVARTYPEID_FLOAT);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        int secondval = host.getSecondOperand()->getIntValue();
        host.setIntValue(((firstval/10000)>>(secondval/10000))*10000);
            }
        }
        
// Other

bool TypeCheck::standardCheck(ZVarTypeId targetTypeId, ZVarTypeId sourceTypeId, AST* toBlame)
{
	ZVarType const& sourceType = *symbolTable.getType(sourceTypeId);
	ZVarType const& targetType = *symbolTable.getType(targetTypeId);
	if (sourceType.canCastTo(targetType)) return true;

	if (toBlame)
	{
		string msg = sourceType.getName() + " to " + targetType.getName();
		printErrorMsg(toBlame, ILLEGALCAST, msg);
	}
	return false;
}

bool TypeCheck::checkExprTypes(ASTUnaryExpr& expr, ZVarTypeId type)
{
	expr.getOperand()->execute(*this);
	if (failure) return false;
	failure = !standardCheck(type, expr.getOperand()->getType(), &expr);
	return !failure;
}

bool TypeCheck::checkExprTypes(ASTBinaryExpr& expr, ZVarTypeId firstType, ZVarTypeId secondType)
{
	expr.getFirstOperand()->execute(*this);
	if (failure) return false;
	failure = !standardCheck(firstType, expr.getFirstOperand()->getType(), &expr);
	if (failure) return false;

	expr.getSecondOperand()->execute(*this);
	if (failure) return false;
	failure = !standardCheck(secondType, expr.getSecondOperand()->getType(), &expr);
	return !failure;
}

ZVarTypeId TypeCheck::getLValTypeId(AST& lval)
{
    GetLValType temp(*this);
    lval.execute(temp);
	return temp.typeId;
}

bool TypeCheck::check(SymbolTable& symbolTable, ZVarTypeId returnTypeId, AST& node)
{
	TypeCheck tc(symbolTable, returnTypeId);
	node.execute(tc);
	return tc.isOK();
}

bool TypeCheck::check(SymbolTable& symbolTable, AST& node)
{
	return TypeCheck::check(symbolTable, ZVARTYPEID_VOID, node);
}

////////////////////////////////////////////////////////////////
// GetLValType

GetLValType::GetLValType(TypeCheck& typeCheck) : typeCheck(typeCheck) {}

void GetLValType::caseDefault(void*)
{
    assert(false);
}

void GetLValType::caseVarDecl(ASTVarDecl& host)
{
    host.execute(typeCheck);
    typeId = typeCheck.symbolTable.getVarTypeId(&host);
}

void GetLValType::caseExprArrow(ASTExprArrow &host)
{
    host.getLVal()->execute(typeCheck);
    if (!typeCheck.isOK()) return;

    ZVarTypeId type = host.getLVal()->getType();
    bool isIndexed = (host.getIndex() != 0);

    if (isIndexed)
    {
        host.getIndex()->execute(typeCheck);
        if (!typeCheck.isOK()) return;

        if (!typeCheck.standardCheck(ZVARTYPEID_FLOAT, host.getIndex()->getType(), host.getIndex()))
        {
            typeCheck.fail();
            return;
        }
    }

	LibrarySymbols* lib = LibrarySymbols::getTypeInstance(type);
	if (!lib)
    {
        printErrorMsg(&host, ARROWNOTPOINTER);
		typeCheck.fail();
        return;
    }
	int functionId = lib->matchSetter(host.getName());

	vector<ZVarTypeId> functionParams = typeCheck.symbolTable.getFuncParamTypeIds(functionId);
    if (functionId == -1 || functionParams.size() != (isIndexed ? 3 : 2) || functionParams[0] != type)
    {
        printErrorMsg(&host, ARROWNOVAR, host.getName() + (isIndexed ? "[]" : ""));
        typeCheck.fail();
        return;
    }

    typeCheck.symbolTable.putNodeId(&host, functionId);
    typeId = functionParams[isIndexed ? 2 : 1];
}

void GetLValType::caseExprDot(ASTExprDot &host)
{
    host.execute(typeCheck);
    int vid = typeCheck.symbolTable.getNodeId(&host);

    if (vid == -1)
    {
        printErrorMsg(&host, LVALCONST, host.getName());
        typeCheck.fail();
        return;
    }

    typeId = typeCheck.symbolTable.getVarTypeId(&host);
}

void GetLValType::caseExprArray(ASTExprArray &host)
{
    host.execute(typeCheck);
    int vid = typeCheck.symbolTable.getNodeId(&host);

    if (vid == -1)
    {
        printErrorMsg(&host, LVALCONST, host.getName());
        typeCheck.fail();
        return;
    }

    typeId = typeCheck.symbolTable.getVarTypeId(&host);

    if (host.getIndex())
    {
        host.getIndex()->execute(typeCheck);

        if (!typeCheck.isOK()) return;

        if (!typeCheck.standardCheck(ZVARTYPEID_FLOAT, host.getIndex()->getType(), host.getIndex()))
        {
            typeCheck.fail();
            return;
        }
    }
}

        
    

