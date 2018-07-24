//2.53 Updated to 16th Jan, 2017
#include "../precompiled.h" //always first

#include "TypeChecker.h"
#include "ParseError.h"
#include "GlobalSymbols.h"
#include "Scope.h"
#include "../zsyssimple.h"
#include "ZScript.h"
#include <assert.h>
#include <string>

using namespace ZScript;

////////////////////////////////////////////////////////////////
// TypeCheck
    
TypeCheck::TypeCheck(SymbolTable& symbolTable)
	: symbolTable(symbolTable), returnType(ZVarType::ZVOID), failure(false)
{}

TypeCheck::TypeCheck(SymbolTable& symbolTable, ZVarTypeId returnTypeId)
	: symbolTable(symbolTable), returnType(*symbolTable.getType(returnTypeId)), failure(false)
{}

TypeCheck::TypeCheck(SymbolTable& symbolTable, ZVarType const& returnType)
	: symbolTable(symbolTable), returnType(returnType), failure(false)
{}


// Statements

void TypeCheck::caseStmtIf(ASTStmtIf &host)
{
    RecursiveVisitor::caseStmtIf(host);
    if (failure) return;

    ZVarType const& type = *host.getCondition()->getVarType();

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

	ZVarType const& type = *host.getKey()->getVarType();
	if (!standardCheck(ZVARTYPEID_FLOAT, type, &host))
		failure = true;
}

void TypeCheck::caseStmtFor(ASTStmtFor &host)
{
    RecursiveVisitor::caseStmtFor(host);
    if (failure) return;

    ZVarType const& type = *host.getTerminationCondition()->getVarType();
    if (!standardCheck(ZVARTYPEID_BOOL, type, &host))
        failure = true;
}
        
void TypeCheck::caseStmtWhile(ASTStmtWhile &host)
{
    RecursiveVisitor::caseStmtWhile(host);
    if (failure) return;

    ZVarType const& type = *host.getCond()->getVarType();
    if (!standardCheck(ZVARTYPEID_BOOL, type, &host))
        failure = true;
}
    
void TypeCheck::caseStmtReturn(ASTStmtReturn &host)
{
    if (returnType != ZVarType::ZVOID)
    {
        printErrorMsg(&host, FUNCBADRETURN, returnType.getName());
        failure = true;
    }
}

void TypeCheck::caseStmtReturnVal(ASTStmtReturnVal &host)
{
    host.getReturnValue()->execute(*this);
    if (failure) return;

    if (!standardCheck(symbolTable.getTypeId(returnType), *host.getReturnValue()->getVarType(), &host))
        failure = true;
}

// Declarations

void TypeCheck::caseDataDecl(ASTDataDecl& host)
{
	RecursiveVisitor::caseDataDecl(host);
	if (failure) return;

	Variable& variable = *host.manager;
    
	// Constants are treated special.
	if (*variable.type == ZVarType::CONST_FLOAT)
	{
		// A constant without an initializer doesn't make sense.
		if (!host.getInitializer())
	{
			printErrorMsg(&host, CONSTUNITIALIZED);
		failure = true;
		return;
	}

		// Inline the constant if possible.
		if (host.getInitializer()->hasDataValue())
    {
			symbolTable.inlineConstant(&host, host.getInitializer()->getDataValue());
			variable.inlined = true;
		}
	}

	// Does it have an initializer?
	if (host.getInitializer())
        {
		// Make sure we can cast the initializer to the type.
		ZVarType const& initType = *host.getInitializer()->getVarType();
		if (!standardCheck(*variable.type, initType, &host))
            {
                failure = true;
                return;
            }

		// If it's an array, do an extra check for array count and sizes.
		if (variable.type->typeClassId() == ZVARTYPE_CLASSID_ARRAY)
	{
			if (*variable.type != initType)
			{
				string msg = initType.getName() + " to " + variable.type->getName();
				printErrorMsg(&host, ILLEGALCAST, msg);
		failure = true;
				return;
			}
		}
	}
}

void TypeCheck::caseDataDeclExtraArray(ASTDataDeclExtraArray& host)
{
	// Type Check size expressions.
	RecursiveVisitor::caseDataDeclExtraArray(host);

	// Iterate over sizes.
	for (vector<ASTExpr*>::const_iterator it = host.dimensions.begin();
		 it != host.dimensions.end(); ++it)
	{
		ASTExpr& size = **it;

		// Make sure each size can cast to float.
		if (!size.getVarType()->canCastTo(ZVarType::FLOAT))
		{
			printErrorMsg(&host, NONINTEGERARRAYSIZE);
        failure = true;
			return;
		}

		// Make sure that the size is constant.
		if (!size.hasDataValue())
	{
			printErrorMsg(&host, EXPRNOTCONSTANT);
			failure = true;
			return;
		}
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

	host.setVarType(content->getVarType());
	if (content->hasDataValue())
		host.setDataValue(content->getDataValue());
}

void TypeCheck::caseExprAssign(ASTExprAssign& host)
{
    host.getRVal()->execute(*this);
    if (failure) return;

	ZVarTypeId ltypeid = getLValTypeId(*host.getLVal());
    if (failure) return;

    ZVarType const& rtype = *host.getRVal()->getVarType();
	host.setVarType(rtype);

    if (!standardCheck(ltypeid, rtype, &host))
        failure = true;

	if (ltypeid == ZVARTYPEID_CONST_FLOAT)
	{
		printErrorMsg(&host, CONSTASSIGN);
		failure = true;
	}
}

void TypeCheck::caseExprIdentifier(ASTExprIdentifier &host)
{
    if (symbolTable.isInlinedConstant(&host))
	{
        host.setVarType(symbolTable.getType(symbolTable.getTypeId(ZVarType::CONST_FLOAT)));
		host.setDataValue(symbolTable.getInlinedValue(&host));
	}
    else
    {
        host.setVarType(symbolTable.getType(symbolTable.getVarTypeId(&host)));
    }
}
    
void TypeCheck::caseExprArrow(ASTExprArrow &host)
{
    // Annoyingly enough I have to treat arrowed variables as function calls
    // Get the left-hand type.
    host.getLeft()->execute(*this);
    if (failure) return;

	// Don't need to check index here, since it'll be checked in the above
	// ASTExprIndex.
	bool isIndexed = host.getIndex() != NULL;

	// Make sure the left side is an object.
    ZVarTypeId leftTypeId = symbolTable.getTypeId(*host.getLeft()->getVarType());
	if (symbolTable.getType(leftTypeId)->typeClassId() != ZVARTYPE_CLASSID_CLASS)
    {
	    failure = true;
	    printErrorMsg(&host, ARROWNOTPOINTER);
	    return;
    }

	ZVarTypeClass& leftType = *(ZVarTypeClass*)symbolTable.getType(leftTypeId);
	ZClass& leftClass = *symbolTable.getClass(leftType.getClassId());

	Function* function = leftClass.getGetter(host.getRight());
	if (!function)
	{
		failure = true;
        printErrorMsg(&host, ARROWNOVAR, host.getRight() + (isIndexed ? "[]" : ""));
        return;
	}
	vector<ZVarTypeId> functionParams = symbolTable.getFuncParamTypeIds(function->id);
    if (functionParams.size() != (isIndexed ? 2 : 1) || functionParams[0] != leftTypeId)
    {
        failure = true;
        printErrorMsg(&host, ARROWNOVAR, host.getRight() + (isIndexed ? "[]" : ""));
        return;
    }

    symbolTable.putNodeId(&host, function->id);
    host.setVarType(symbolTable.getType(symbolTable.getFuncReturnTypeId(function->id)));
}

void TypeCheck::caseExprIndex(ASTExprIndex &host)
{
	host.getArray()->execute(*this);
	host.setVarType(host.getArray()->getVarType());

	// The index must be a number.
    if (host.getIndex())
    {
        host.getIndex()->execute(*this);
        if (failure) return;

        if (!standardCheck(ZVARTYPEID_FLOAT, *host.getIndex()->getVarType(), host.getIndex()))
        {
            failure = true;
            return;
        }
    }
}
    
void TypeCheck::caseExprCall(ASTExprCall &host)
{
    // Yuck. Time to disambiguate these damn functions

    // Build the param types
    list<ASTExpr*> params = host.getParams();
    vector<ZVarTypeId> paramtypes;
    vector<int> possibleFuncIds;

    // If this is a simple function, we already have what we need otherwise we
    // need the type of the thing being arrowed.
    if (host.getLeft()->isTypeArrow())
    {
        ASTExprArrow* lval = (ASTExprArrow*)host.getLeft();
        lval->getLeft()->execute(*this);
        if (failure) return;
        ZVarType const& lvaltype = *lval->getLeft()->getVarType();

		if (lvaltype.typeClassId() != ZVARTYPE_CLASSID_CLASS)
        {
	        printErrorMsg(lval, ARROWNOTPOINTER);
	        failure = true;
	        return;
        }

        // Prepend that type to the function parameters, as that is implicitly passed
        paramtypes.push_back(symbolTable.getTypeId(lvaltype));
    }

    // Now add the normal parameters.
    for (list<ASTExpr*>::iterator it = params.begin(); it != params.end(); it++)
    {
        (*it)->execute(*this);
        if (failure) return;

        paramtypes.push_back(symbolTable.getTypeId(*(*it)->getVarType()));
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

    if (host.getLeft()->isTypeIdentifier())
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

        string fullname = host.getLeft()->asString();

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
        host.setVarType(symbolTable.getType(symbolTable.getFuncReturnTypeId(bestmatch[0])));
        symbolTable.putNodeId(&host, bestmatch[0]);
    }
    else
    {
        // Still have to deal with the (%&# arrow functions
        // Luckily I will here assert that each type's functions MUST be unique
		ASTExprArrow& arrow = *(ASTExprArrow*)host.getLeft();
        string name = arrow.getRight();

		// Make sure the left side is an object.
        ZVarTypeId leftTypeId = symbolTable.getTypeId(*arrow.getLeft()->getVarType());
		if (symbolTable.getType(leftTypeId)->typeClassId() != ZVARTYPE_CLASSID_CLASS)
		{
			failure = true;
			printErrorMsg(&host, ARROWNOTPOINTER);
			return;
		}

		ZVarTypeClass& leftType = *(ZVarTypeClass*)symbolTable.getType(leftTypeId);
		ZClass& leftClass = *symbolTable.getClass(leftType.getClassId());

		int functionId = -1;
		vector<int> functionIds = leftClass.getFunctionIds(name);
		if (functionIds.size() > 0) functionId = functionIds[0];
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

        symbolTable.putNodeId(&host, functionId);
        host.setVarType(symbolTable.getType(symbolTable.getFuncReturnTypeId(functionId)));
    }
}

void TypeCheck::caseExprNegate(ASTExprNegate &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT)) return;
    host.setVarType(ZVarType::FLOAT);

    if (host.getOperand()->hasDataValue())
    {
        long val = host.getOperand()->getDataValue();
        host.setDataValue(-val);
    }
}

void TypeCheck::caseExprNot(ASTExprNot &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_BOOL)) return;
    host.setVarType(ZVarType::BOOL);

    if (host.getOperand()->hasDataValue())
    {
        long val = host.getOperand()->getDataValue();
		host.setDataValue(!host.getOperand()->getDataValue());
    }
}

void TypeCheck::caseExprBitNot(ASTExprBitNot &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT)) return;
    host.setVarType(ZVarType::FLOAT);

    if (host.getOperand()->hasDataValue())
    {
        long val = host.getOperand()->getDataValue();
        host.setDataValue((~(val/10000))*10000);
    }
}

void TypeCheck::caseExprIncrement(ASTExprIncrement &host)
{
    host.getOperand()->execute(*this);
    if (failure) return;

	ASTExpr& operand = *host.getOperand();
    if (operand.isTypeArrow() || operand.isTypeIndex())
    {
		ASTExprArrow* arrow;
		if (operand.isTypeArrow()) arrow = &(ASTExprArrow&)operand;
		else arrow = (ASTExprArrow*)((ASTExprIndex&)operand).getArray();
        int fid = symbolTable.getNodeId(arrow);
        symbolTable.putNodeId(&host, fid);
    }

	ZVarTypeId ltype = getLValTypeId(operand);
    if (failure) return;

    if (!standardCheck(ZVARTYPEID_FLOAT, ltype, &host))
    {
        failure = true;
        return;
    }

    host.setVarType(ZVarType::FLOAT);
}
    
void TypeCheck::caseExprPreIncrement(ASTExprPreIncrement &host)
{
    host.getOperand()->execute(*this);
    if (failure) return;

	ASTExpr& operand = *host.getOperand();
    if (operand.isTypeArrow() || operand.isTypeIndex())
    {
		ASTExprArrow* arrow;
		if (operand.isTypeArrow()) arrow = &(ASTExprArrow&)operand;
		else arrow = (ASTExprArrow*)((ASTExprIndex&)operand).getArray();
        int fid = symbolTable.getNodeId(arrow);
        symbolTable.putNodeId(&host, fid);
    }

	ZVarTypeId ltype = getLValTypeId(operand);
    if (failure) return;

    if (!standardCheck(ZVARTYPEID_FLOAT, ltype, &host))
    {
        failure = true;
        return;
    }

    host.setVarType(ZVarType::FLOAT);
}

void TypeCheck::caseExprDecrement(ASTExprDecrement &host)
{
    host.getOperand()->execute(*this);
    if (failure) return;

	ASTExpr& operand = *host.getOperand();
    if (operand.isTypeArrow() || operand.isTypeIndex())
    {
		ASTExprArrow* arrow;
		if (operand.isTypeArrow()) arrow = &(ASTExprArrow&)operand;
		else arrow = (ASTExprArrow*)((ASTExprIndex&)operand).getArray();
        int fid = symbolTable.getNodeId(arrow);
        symbolTable.putNodeId(&host, fid);
    }

	ZVarTypeId ltype = getLValTypeId(operand);
    if (failure) return;

    if (!standardCheck(ZVARTYPEID_FLOAT, ltype, &host))
    {
        failure = true;
        return;
    }

    host.setVarType(ZVarType::FLOAT);
}
    
void TypeCheck::caseExprPreDecrement(ASTExprPreDecrement &host)
{
    host.getOperand()->execute(*this);
    if (failure) return;

	ASTExpr& operand = *host.getOperand();
    if (operand.isTypeArrow() || operand.isTypeIndex())
    {
		ASTExprArrow* arrow;
		if (operand.isTypeArrow()) arrow = &(ASTExprArrow&)operand;
		else arrow = (ASTExprArrow*)((ASTExprIndex&)operand).getArray();
        int fid = symbolTable.getNodeId(arrow);
        symbolTable.putNodeId(&host, fid);
    }

	ZVarTypeId ltype = getLValTypeId(operand);
    if (failure) return;

    if (!standardCheck(ZVARTYPEID_FLOAT, ltype, &host))
    {
        failure = true;
        return;
    }

    host.setVarType(ZVarType::FLOAT);
}

void TypeCheck::caseExprAnd(ASTExprAnd &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_BOOL, ZVARTYPEID_BOOL)) return;
    host.setVarType(ZVarType::BOOL);

	if (host.getFirstOperand()->hasDataValue() && host.getSecondOperand()->hasDataValue())
    {
        long firstval = host.getFirstOperand()->getDataValue();
        long secondval = host.getSecondOperand()->getDataValue();
		host.setDataValue(firstval && secondval);
    }
}

void TypeCheck::caseExprOr(ASTExprOr &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_BOOL, ZVARTYPEID_BOOL)) return;
    host.setVarType(ZVarType::BOOL);

	if (host.getFirstOperand()->hasDataValue() && host.getSecondOperand()->hasDataValue())
    {
        long firstval = host.getFirstOperand()->getDataValue();
        long secondval = host.getSecondOperand()->getDataValue();
		host.setDataValue(firstval || secondval);
    }
}

void TypeCheck::caseExprGT(ASTExprGT &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setVarType(ZVarType::BOOL);

    if (host.getFirstOperand()->hasDataValue() && host.getSecondOperand()->hasDataValue())
    {
        long firstval = host.getFirstOperand()->getDataValue();
        long secondval = host.getSecondOperand()->getDataValue();
		host.setDataValue(firstval > secondval);
    }
}

void TypeCheck::caseExprGE(ASTExprGE &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setVarType(ZVarType::BOOL);

    if (host.getFirstOperand()->hasDataValue() && host.getSecondOperand()->hasDataValue())
    {
        long firstval = host.getFirstOperand()->getDataValue();
        long secondval = host.getSecondOperand()->getDataValue();
		host.setDataValue(firstval >= secondval);
    }
}

void TypeCheck::caseExprLT(ASTExprLT &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setVarType(ZVarType::BOOL);

    if (host.getFirstOperand()->hasDataValue() && host.getSecondOperand()->hasDataValue())
    {
        long firstval = host.getFirstOperand()->getDataValue();
        long secondval = host.getSecondOperand()->getDataValue();
		host.setDataValue(firstval < secondval);
    }
}

void TypeCheck::caseExprLE(ASTExprLE &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setVarType(ZVarType::BOOL);

    if (host.getFirstOperand()->hasDataValue() && host.getSecondOperand()->hasDataValue())
    {
        long firstval = host.getFirstOperand()->getDataValue();
        long secondval = host.getSecondOperand()->getDataValue();
		host.setDataValue(firstval <= secondval);
    }
}

void TypeCheck::caseExprEQ(ASTExprEQ &host)
{
	host.getFirstOperand()->execute(*this);
	if (failure) return;
	host.getSecondOperand()->execute(*this);
	if (failure) return;

	if (!standardCheck(*host.getFirstOperand()->getVarType(), *host.getSecondOperand()->getVarType(), &host))
	{
		failure = true;
		return;
	}

    host.setVarType(ZVarType::BOOL);

    if (host.getFirstOperand()->hasDataValue() && host.getSecondOperand()->hasDataValue())
    {
        long firstval = host.getFirstOperand()->getDataValue();
        long secondval = host.getSecondOperand()->getDataValue();
		host.setDataValue(firstval == secondval);
    }
}

void TypeCheck::caseExprNE(ASTExprNE &host)
{
	host.getFirstOperand()->execute(*this);
	if (failure) return;
	host.getSecondOperand()->execute(*this);
	if (failure) return;

	if (!standardCheck(*host.getFirstOperand()->getVarType(), *host.getSecondOperand()->getVarType(), &host))
	{
		failure = true;
		return;
	}

    host.setVarType(ZVarType::BOOL);

    if (host.getFirstOperand()->hasDataValue() && host.getSecondOperand()->hasDataValue())
    {
        long firstval = host.getFirstOperand()->getDataValue();
        long secondval = host.getSecondOperand()->getDataValue();
		host.setDataValue(firstval != secondval);
    }
}

void TypeCheck::caseExprPlus(ASTExprPlus &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setVarType(ZVarType::FLOAT);

    if (host.getFirstOperand()->hasDataValue() && host.getSecondOperand()->hasDataValue())
    {
        long firstval = host.getFirstOperand()->getDataValue();
        long secondval = host.getSecondOperand()->getDataValue();
        host.setDataValue(firstval + secondval);
    }
}

void TypeCheck::caseExprMinus(ASTExprMinus &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setVarType(ZVarType::FLOAT);

    if (host.getFirstOperand()->hasDataValue() && host.getSecondOperand()->hasDataValue())
    {
        long firstval = host.getFirstOperand()->getDataValue();
        long secondval = host.getSecondOperand()->getDataValue();
        host.setDataValue(firstval - secondval);
    }
    }
    
void TypeCheck::caseExprTimes(ASTExprTimes &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setVarType(ZVarType::FLOAT);

    if (host.getFirstOperand()->hasDataValue() && host.getSecondOperand()->hasDataValue())
    {
        long firstval = host.getFirstOperand()->getDataValue();
        long secondval = host.getSecondOperand()->getDataValue();
        double temp = ((double)secondval)/10000.0;
        host.setDataValue((long)(firstval * temp));
    }
}

void TypeCheck::caseExprDivide(ASTExprDivide &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setVarType(ZVarType::FLOAT);

    if (host.getFirstOperand()->hasDataValue() && host.getSecondOperand()->hasDataValue())
    {
        long firstval = host.getFirstOperand()->getDataValue();
        long secondval = host.getSecondOperand()->getDataValue();

        if (secondval == 0)
        {
            printErrorMsg(&host, DIVBYZERO);
            failure = true;
            return;
        }

        host.setDataValue((firstval / secondval) * 10000);
    }
}

void TypeCheck::caseExprModulo(ASTExprModulo &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setVarType(ZVarType::FLOAT);

    if (host.getFirstOperand()->hasDataValue() && host.getSecondOperand()->hasDataValue())
    {
        long firstval = host.getFirstOperand()->getDataValue();
        long secondval = host.getSecondOperand()->getDataValue();

        if (secondval == 0)
        {
            printErrorMsg(&host, DIVBYZERO);
            failure = true;
            return;
        }

        host.setDataValue(firstval % secondval);
    }
}

void TypeCheck::caseExprBitAnd(ASTExprBitAnd &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setVarType(ZVarType::FLOAT);

    if (host.getFirstOperand()->hasDataValue() && host.getSecondOperand()->hasDataValue())
    {
        long firstval = host.getFirstOperand()->getDataValue();
        long secondval = host.getSecondOperand()->getDataValue();
        host.setDataValue(((firstval/10000)&(secondval/10000))*10000);
    }
}
    
void TypeCheck::caseExprBitOr(ASTExprBitOr &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setVarType(ZVarType::FLOAT);

    if (host.getFirstOperand()->hasDataValue() && host.getSecondOperand()->hasDataValue())
    {
        long firstval = host.getFirstOperand()->getDataValue();
        long secondval = host.getSecondOperand()->getDataValue();
        host.setDataValue(((firstval/10000)|(secondval/10000))*10000);
    }
}

void TypeCheck::caseExprBitXor(ASTExprBitXor &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setVarType(ZVarType::FLOAT);

    if (host.getFirstOperand()->hasDataValue() && host.getSecondOperand()->hasDataValue())
    {
        long firstval = host.getFirstOperand()->getDataValue();
        long secondval = host.getSecondOperand()->getDataValue();
        host.setDataValue(((firstval/10000)^(secondval/10000))*10000);
    }
}
void TypeCheck::caseExprLShift(ASTExprLShift &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;

    if (host.getSecondOperand()->hasDataValue())
    {
        if (host.getSecondOperand()->getDataValue() % 10000)
        {
            printErrorMsg(&host, SHIFTNOTINT);
            host.getSecondOperand()->setDataValue(10000*(host.getSecondOperand()->getDataValue()/10000));
        }
    }
    host.setVarType(ZVarType::FLOAT);

    if (host.getFirstOperand()->hasDataValue() && host.getSecondOperand()->hasDataValue())
    {
        long firstval = host.getFirstOperand()->getDataValue();
        int secondval = host.getSecondOperand()->getDataValue();
        host.setDataValue(((firstval/10000)<<(secondval/10000))*10000);
    }
}
void TypeCheck::caseExprRShift(ASTExprRShift &host)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;

    if (host.getSecondOperand()->hasDataValue())
    {
        if (host.getSecondOperand()->getDataValue() % 10000)
        {
            printErrorMsg(&host, SHIFTNOTINT);
            host.getSecondOperand()->setDataValue(10000*(host.getSecondOperand()->getDataValue()/10000));
        }
    }
    host.setVarType(ZVarType::FLOAT);

    if (host.getFirstOperand()->hasDataValue() && host.getSecondOperand()->hasDataValue())
    {
        long firstval = host.getFirstOperand()->getDataValue();
        int secondval = host.getSecondOperand()->getDataValue();
        host.setDataValue(((firstval/10000)>>(secondval/10000))*10000);
            }
        }
        
// Literals

void TypeCheck::caseNumberLiteral(ASTNumberLiteral& host)
{
    host.setVarType(ZVarType::FLOAT);
    pair<string,string> parts = host.getValue()->parseValue();
    pair<long, bool> val = ScriptParser::parseLong(parts);

    if (!val.second)
        printErrorMsg(&host, CONSTTRUNC, host.getValue()->getValue());

    host.setDataValue(val.first);
}

void TypeCheck::caseBoolLiteral(ASTBoolLiteral& host)
{
    host.setVarType(ZVarType::BOOL);
    host.setDataValue(host.getValue() ? 1L : 0L);
}

void TypeCheck::caseStringLiteral(ASTStringLiteral& host) {}

void TypeCheck::caseArrayLiteral(ASTArrayLiteral& host)
{
	// First, check elements.
	RecursiveVisitor::caseArrayLiteral(host);
	if (failure) return;

	// Check the explicit size is a number, if present.
	ASTExpr* size = host.getSize();
	if (size && !size->getVarType()->canCastTo(ZVarType::FLOAT))
	{
		printErrorMsg(&host, NONINTEGERARRAYSIZE);
		failure = true;
		return;
	}

	// Don't allow an explicit size if we're part of a declaration.
	if (size && host.declaration)
	{
		printErrorMsg(&host, ARRAYLITERALRESIZE);
		failure = true;
		return;
	}

	// If we don't have a type assigned, grab it from the first element.
	ZVarTypeArray const* type = (ZVarTypeArray const*)host.getVarType();
	ZVarType const* elementType;
	if (type)
		elementType = &type->getElementType();
	else
	{
		elementType = host.getElements()[0]->getVarType();
		type = (ZVarTypeArray const*)symbolTable.getCanonicalType(ZVarTypeArray(*elementType));
		host.setVarType(type);
	}

	// Check each element to make sure it adheres to type.
	vector<ASTExpr*> elements = host.getElements();
	for (vector<ASTExpr*>::iterator it = elements.begin();
		 it != elements.end(); ++it)
	{
		if (!standardCheck(*elementType, *(*it)->getVarType(), &host))
		{
			failure = true;
			return;
		}
	}

	// Update literal type.
	host.manager->type = type;
}


// Other

bool TypeCheck::standardCheck(ZVarTypeId targetTypeId, ZVarTypeId sourceTypeId, AST* toBlame)
{
	ZVarType const& sourceType = *symbolTable.getType(sourceTypeId);
	ZVarType const& targetType = *symbolTable.getType(targetTypeId);
	return standardCheck(targetType, sourceType, toBlame);
}

bool TypeCheck::standardCheck(ZVarType const& targetType, ZVarTypeId sourceTypeId, AST* toBlame)
{
	ZVarType const& sourceType = *symbolTable.getType(sourceTypeId);
	return standardCheck(targetType, sourceType, toBlame);
}

bool TypeCheck::standardCheck(ZVarTypeId targetTypeId, ZVarType const& sourceType, AST* toBlame)
{
	ZVarType const& targetType = *symbolTable.getType(targetTypeId);
	return standardCheck(targetType, sourceType, toBlame);
}

bool TypeCheck::standardCheck(ZVarType const& targetType, ZVarType const& sourceType, AST* toBlame)
{
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
	failure = !standardCheck(type, *expr.getOperand()->getVarType(), &expr);
	return !failure;
}

bool TypeCheck::checkExprTypes(ASTBinaryExpr& expr, ZVarTypeId firstType, ZVarTypeId secondType)
{
	expr.getFirstOperand()->execute(*this);
	if (failure) return false;
	failure = !standardCheck(firstType, *expr.getFirstOperand()->getVarType(), &expr);
	if (failure) return false;

	expr.getSecondOperand()->execute(*this);
	if (failure) return false;
	failure = !standardCheck(secondType, *expr.getSecondOperand()->getVarType(), &expr);
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

void GetLValType::caseExprArrow(ASTExprArrow &host)
{
	SymbolTable& symbolTable = typeCheck.symbolTable;

    host.getLeft()->execute(typeCheck);
    if (!typeCheck.isOK()) return;

	// Don't need to check index here, since it'll be checked in the above
	// ASTExprIndex.
	bool isIndexed = host.getIndex() != NULL;

	// Make sure the left side is an object.
    ZVarTypeId leftTypeId = symbolTable.getTypeId(*host.getLeft()->getVarType());
	if (symbolTable.getType(leftTypeId)->typeClassId() != ZVARTYPE_CLASSID_CLASS)
    {
		typeCheck.fail();
        printErrorMsg(&host, ARROWNOTPOINTER);
        return;
    }

	ZVarTypeClass& leftType = *(ZVarTypeClass*)symbolTable.getType(leftTypeId);
	ZClass& leftClass = *symbolTable.getClass(leftType.getClassId());

	Function* function = leftClass.getSetter(host.getRight());
    if (!function)
    {
        typeCheck.fail();
        printErrorMsg(&host, ARROWNOVAR, host.getRight() + (isIndexed ? "[]" : ""));
        return;
    }
	vector<ZVarTypeId> functionParams = symbolTable.getFuncParamTypeIds(function->id);
    if (functionParams.size() != (isIndexed ? 3 : 2) || functionParams[0] != leftTypeId)
    {
        typeCheck.fail();
        printErrorMsg(&host, ARROWNOVAR, host.getRight() + (isIndexed ? "[]" : ""));
        return;
    }

    symbolTable.putNodeId(&host, function->id);
    typeId = functionParams[isIndexed ? 2 : 1];
	host.setVarType(symbolTable.getType(typeId));
}

void GetLValType::caseExprIdentifier(ASTExprIdentifier& host)
{
    host.execute(typeCheck);
    int vid = typeCheck.symbolTable.getNodeId(&host);

    if (vid == -1)
    {
        printErrorMsg(&host, LVALCONST, host.asString());
        typeCheck.fail();
        return;
    }

    typeId = typeCheck.symbolTable.getVarTypeId(&host);
}

void GetLValType::caseExprIndex(ASTExprIndex& host)
{
	// Arrows just fall back on the arrow implementation.
	if (host.getArray()->isTypeArrow()) host.getArray()->execute(*this);

	else
    {
		host.execute(typeCheck);
		typeId = typeCheck.symbolTable.getTypeId(*host.getArray()->getVarType());
    }

	// The index must be a number.
    if (host.getIndex())
    {
        host.getIndex()->execute(typeCheck);

        if (!typeCheck.isOK()) return;

		if (!typeCheck.standardCheck(ZVARTYPEID_FLOAT, *host.getIndex()->getVarType(), host.getIndex()))
        {
            typeCheck.fail();
            return;
        }
    }

	host.setVarType(typeCheck.symbolTable.getType(typeId));
}

        
    

