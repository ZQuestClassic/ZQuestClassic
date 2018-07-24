//2.53 Updated to 16th Jan, 2017
#include "../precompiled.h" //always first

#include "../zsyssimple.h"
#include "CompileError.h"
#include "GlobalSymbols.h"
#include "Scope.h"
#include "TypeChecker.h"
#include "ZScript.h"
#include <assert.h>
#include <string>

using namespace ZScript;

////////////////////////////////////////////////////////////////
// TypeCheck
    
TypeCheck::TypeCheck(SymbolTable& symbolTable)
	: symbolTable(symbolTable), returnType(ZVarType::ZVOID)
{}

TypeCheck::TypeCheck(SymbolTable& symbolTable, ZVarTypeId returnTypeId)
	: symbolTable(symbolTable), returnType(*symbolTable.getType(returnTypeId))
{}

TypeCheck::TypeCheck(SymbolTable& symbolTable, ZVarType const& returnType)
	: symbolTable(symbolTable), returnType(returnType)
{}


// Statements

void TypeCheck::caseStmtIf(ASTStmtIf& host, void*)
{
    RecursiveVisitor::caseStmtIf(host);
    if (failure) return;

    ZVarType const& type = *host.condition->getVarType();

    if (!standardCheck(ZVARTYPEID_BOOL, type, &host))
        failure = true;
}
        
void TypeCheck::caseStmtIfElse(ASTStmtIfElse& host, void*)
{
    caseStmtIf(host);
    host.elseStatement->execute(*this);
}
        
void TypeCheck::caseStmtSwitch(ASTStmtSwitch& host, void*)
{
	RecursiveVisitor::caseStmtSwitch(host);
	if (failure) return;

	ZVarType const& type = *host.key->getVarType();
	if (!standardCheck(ZVARTYPEID_FLOAT, type, &host))
		failure = true;
}

void TypeCheck::caseStmtFor(ASTStmtFor& host, void*)
{
    RecursiveVisitor::caseStmtFor(host);
    if (failure) return;

    ZVarType const& type = *host.test->getVarType();
    if (!standardCheck(ZVARTYPEID_BOOL, type, &host))
        failure = true;
}
        
void TypeCheck::caseStmtWhile(ASTStmtWhile& host, void*)
{
    RecursiveVisitor::caseStmtWhile(host);
    if (failure) return;

    ZVarType const& type = *host.test->getVarType();
    if (!standardCheck(ZVARTYPEID_BOOL, type, &host))
        failure = true;
}
    
void TypeCheck::caseStmtReturn(ASTStmtReturn& host, void*)
{
    if (returnType != ZVarType::ZVOID)
        compileError(
				host, &CompileError::FuncBadReturn,
				returnType.getName().c_str());
}

void TypeCheck::caseStmtReturnVal(ASTStmtReturnVal& host, void*)
{
    host.value->execute(*this);
    if (failure) return;

    if (!standardCheck(symbolTable.getTypeId(returnType), *host.value->getVarType(), &host))
        failure = true;
}

// Declarations

void TypeCheck::caseDataDecl(ASTDataDecl& host, void*)
{
	RecursiveVisitor::caseDataDecl(host);
	if (failure) return;

	Variable& variable = *host.manager;
    
	// Constants are treated special.
	if (*variable.type == ZVarType::CONST_FLOAT)
	{
		// A constant without an initializer doesn't make sense.
		if (!host.initializer())
	{
			compileError(host, &CompileError::ConstUninitialized);
		return;
	}

		// Inline the constant if possible.
		if (host.initializer()->hasDataValue())
    {
			symbolTable.inlineConstant(&host, host.initializer()->getDataValue());
			variable.inlined = true;
		}
	}

	// Does it have an initializer?
	if (host.initializer())
        {
		// Make sure we can cast the initializer to the type.
		ZVarType const& initType = *host.initializer()->getVarType();
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
				compileError(host, &CompileError::IllegalCast, msg.c_str());
		failure = true;
				return;
			}
		}
	}
}

void TypeCheck::caseDataDeclExtraArray(ASTDataDeclExtraArray& host, void*)
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
			compileError(host, &CompileError::NonIntegerArraySize);
			return;
		}

		// Make sure that the size is constant.
		if (!size.hasDataValue())
	{
			compileError(host, &CompileError::ExprNotConstant);
			return;
		}
	}
}

// Expressions

void TypeCheck::caseExprConst(ASTExprConst& host, void*)
{
	ASTExpr* content = host.content;
	content->execute(*this);

	if (!host.isConstant())
	{
		failure = true;
        compileError(host, &CompileError::ExprNotConstant);
		return;
	}

	host.setVarType(content->getVarType());
	if (!host.hasDataValue() && content->hasDataValue())
		host.setDataValue(content->getDataValue());
}

void TypeCheck::caseExprAssign(ASTExprAssign& host, void*)
{
    host.right->execute(*this);
    if (failure) return;

	ZVarTypeId ltypeid = getLValTypeId(*host.left);
    if (failure) return;

    ZVarType const& rtype = *host.right->getVarType();
	host.setVarType(rtype);

    if (!standardCheck(ltypeid, rtype, &host))
        failure = true;

	if (ltypeid == ZVARTYPEID_CONST_FLOAT)
		compileError(host, &CompileError::ConstAssign);
}

void TypeCheck::caseExprIdentifier(ASTExprIdentifier& host, void*)
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
    
void TypeCheck::caseExprArrow(ASTExprArrow& host, void*)
{
    // Recurse on left.
    host.left->execute(*this);
    if (breakRecursion(host)) return;

	// Don't need to check index here, since it'll be checked in the above
	// ASTExprIndex.
	bool isIndexed = host.index != NULL;

	// Make sure the left side is an object.
    ZVarTypeId leftTypeId = symbolTable.getTypeId(*host.left->getVarType());
	if (symbolTable.getType(leftTypeId)->typeClassId() != ZVARTYPE_CLASSID_CLASS)
    {
        compileError(host, &CompileError::ArrowNotPointer);
	    return;
    }

	ZVarTypeClass& leftType = *(ZVarTypeClass*)symbolTable.getType(leftTypeId);
	ZClass& leftClass = *symbolTable.getClass(leftType.getClassId());

	Function* function = leftClass.getGetter(host.right);
	if (!function)
	{
        compileError(host, &CompileError::ArrowNoVar,
					 (host.right + (isIndexed ? "[]" : "")).c_str());
        return;
	}
	vector<ZVarTypeId> functionParams = symbolTable.getFuncParamTypeIds(function->id);
    if (functionParams.size() != (isIndexed ? 2 : 1) || functionParams[0] != leftTypeId)
    {
        compileError(host, &CompileError::ArrowNoVar,
					 (host.right + (isIndexed ? "[]" : "")).c_str());
        return;
    }

    symbolTable.putNodeId(&host, function->id);
    host.setVarType(symbolTable.getType(symbolTable.getFuncReturnTypeId(function->id)));
}

void TypeCheck::caseExprIndex(ASTExprIndex& host, void*)
{
	host.array->execute(*this);
	host.setVarType(host.array->getVarType());

	// The index must be a number.
    if (host.index)
    {
        host.index->execute(*this);
        if (failure) return;

        if (!standardCheck(ZVARTYPEID_FLOAT, *host.index->getVarType(), host.index))
        {
            failure = true;
            return;
        }
    }
}
    
void TypeCheck::caseExprCall(ASTExprCall& host, void*)
{
    // Yuck. Time to disambiguate these damn functions

    // Build the param types
    vector<ASTExpr*> params = host.parameters;
    vector<ZVarTypeId> paramtypes;
    vector<int> possibleFuncIds;

    // If this is a simple function, we already have what we need otherwise we
    // need the type of the thing being arrowed.
    if (host.left->isTypeArrow())
    {
        ASTExprArrow* lval = (ASTExprArrow*)host.left;
        lval->left->execute(*this);
        if (failure) return;
        ZVarType const& lvaltype = *lval->left->getVarType();

		if (lvaltype.typeClassId() != ZVARTYPE_CLASSID_CLASS)
        {
            compileError(*lval, &CompileError::ArrowNotPointer);
	        return;
        }

        // Prepend that type to the function parameters, as that is implicitly passed
        paramtypes.push_back(symbolTable.getTypeId(lvaltype));
    }

    // Now add the normal parameters.
    for (vector<ASTExpr*>::iterator it = params.begin(); it != params.end(); it++)
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

    if (host.left->isTypeIdentifier())
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

        string fullname = host.left->asString();

        if (bestmatch.size() == 0)
        {
            compileError(host, &CompileError::NoFuncMatch,
						 (fullname + paramstring).c_str());
            return;
        }
        else if (bestmatch.size() > 1)
        {
            compileError(host, &CompileError::TooFuncMatch,
						 (fullname + paramstring).c_str());
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
		ASTExprArrow& arrow = *(ASTExprArrow*)host.left;
        string name = arrow.right;

		// Make sure the left side is an object.
        ZVarTypeId leftTypeId = symbolTable.getTypeId(*arrow.left->getVarType());
		if (symbolTable.getType(leftTypeId)->typeClassId() != ZVARTYPE_CLASSID_CLASS)
		{
			compileError(host, &CompileError::ArrowNotPointer);
			return;
		}

		ZVarTypeClass& leftType = *(ZVarTypeClass*)symbolTable.getType(leftTypeId);
		ZClass& leftClass = *symbolTable.getClass(leftType.getClassId());

		int functionId = -1;
		vector<int> functionIds = leftClass.getFunctionIds(name);
		if (functionIds.size() > 0) functionId = functionIds[0];
        if (functionId == -1)
        {
			compileError(host, &CompileError::ArrowNoFunc, name.c_str());
            return;
        }

		vector<ZVarTypeId> functionParams = symbolTable.getFuncParamTypeIds(functionId);
        if (paramtypes.size() != functionParams.size())
        {
            compileError(host, &CompileError::NoFuncMatch,
						 (name + paramstring).c_str());
            return;
        }

        for (unsigned int i = 0; i < paramtypes.size(); i++)
        {
            if (!standardCheck(functionParams[i], paramtypes[i], NULL))
            {
                compileError(host, &CompileError::NoFuncMatch,
							 (name + paramstring).c_str());
                return;
            }
        }

        symbolTable.putNodeId(&host, functionId);
        host.setVarType(symbolTable.getType(symbolTable.getFuncReturnTypeId(functionId)));
    }
}

void TypeCheck::caseExprNegate(ASTExprNegate& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT)) return;
    host.setVarType(ZVarType::FLOAT);

    if (host.operand->hasDataValue())
    {
        long val = host.operand->getDataValue();
        host.setDataValue(-val);
    }
}

void TypeCheck::caseExprNot(ASTExprNot& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_BOOL)) return;
    host.setVarType(ZVarType::BOOL);

    if (host.operand->hasDataValue())
    {
        long val = host.operand->getDataValue();
		host.setDataValue(!host.operand->getDataValue());
    }
}

void TypeCheck::caseExprBitNot(ASTExprBitNot& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT)) return;
    host.setVarType(ZVarType::FLOAT);

    if (host.operand->hasDataValue())
    {
        long val = host.operand->getDataValue();
        host.setDataValue((~(val/10000))*10000);
    }
}

void TypeCheck::caseExprIncrement(ASTExprIncrement& host, void*)
{
    host.operand->execute(*this);
    if (failure) return;

	ASTExpr& operand = *host.operand;
    if (operand.isTypeArrow() || operand.isTypeIndex())
    {
		ASTExprArrow* arrow;
		if (operand.isTypeArrow()) arrow = &(ASTExprArrow&)operand;
		else arrow = (ASTExprArrow*)((ASTExprIndex&)operand).array;
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
    
void TypeCheck::caseExprPreIncrement(ASTExprPreIncrement& host, void*)
{
    host.operand->execute(*this);
    if (failure) return;

	ASTExpr& operand = *host.operand;
    if (operand.isTypeArrow() || operand.isTypeIndex())
    {
		ASTExprArrow* arrow;
		if (operand.isTypeArrow()) arrow = &(ASTExprArrow&)operand;
		else arrow = (ASTExprArrow*)((ASTExprIndex&)operand).array;
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

void TypeCheck::caseExprDecrement(ASTExprDecrement& host, void*)
{
    host.operand->execute(*this);
    if (failure) return;

	ASTExpr& operand = *host.operand;
    if (operand.isTypeArrow() || operand.isTypeIndex())
    {
		ASTExprArrow* arrow;
		if (operand.isTypeArrow()) arrow = &(ASTExprArrow&)operand;
		else arrow = (ASTExprArrow*)((ASTExprIndex&)operand).array;
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
    
void TypeCheck::caseExprPreDecrement(ASTExprPreDecrement& host, void*)
{
    host.operand->execute(*this);
    if (failure) return;

	ASTExpr& operand = *host.operand;
    if (operand.isTypeArrow() || operand.isTypeIndex())
    {
		ASTExprArrow* arrow;
		if (operand.isTypeArrow()) arrow = &(ASTExprArrow&)operand;
		else arrow = (ASTExprArrow*)((ASTExprIndex&)operand).array;
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

void TypeCheck::caseExprAnd(ASTExprAnd& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_BOOL, ZVARTYPEID_BOOL)) return;
    host.setVarType(ZVarType::BOOL);

	if (host.left->hasDataValue() && host.right->hasDataValue())
    {
		long firstval = host.left->getDataValue();
		long secondval = host.right->getDataValue();
		host.setDataValue(firstval && secondval);
    }
}

void TypeCheck::caseExprOr(ASTExprOr& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_BOOL, ZVARTYPEID_BOOL)) return;
    host.setVarType(ZVarType::BOOL);

	if (host.left->hasDataValue() && host.right->hasDataValue())
    {
		long firstval = host.left->getDataValue();
		long secondval = host.right->getDataValue();
		host.setDataValue(firstval || secondval);
    }
}

void TypeCheck::caseExprGT(ASTExprGT& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setVarType(ZVarType::BOOL);

    if (host.left->hasDataValue() && host.right->hasDataValue())
    {
        long firstval = host.left->getDataValue();
        long secondval = host.right->getDataValue();
		host.setDataValue(firstval > secondval);
    }
}

void TypeCheck::caseExprGE(ASTExprGE& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setVarType(ZVarType::BOOL);

    if (host.left->hasDataValue() && host.right->hasDataValue())
    {
        long firstval = host.left->getDataValue();
        long secondval = host.right->getDataValue();
		host.setDataValue(firstval >= secondval);
    }
}

void TypeCheck::caseExprLT(ASTExprLT& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setVarType(ZVarType::BOOL);

    if (host.left->hasDataValue() && host.right->hasDataValue())
    {
        long firstval = host.left->getDataValue();
        long secondval = host.right->getDataValue();
		host.setDataValue(firstval < secondval);
    }
}

void TypeCheck::caseExprLE(ASTExprLE& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setVarType(ZVarType::BOOL);

    if (host.left->hasDataValue() && host.right->hasDataValue())
    {
        long firstval = host.left->getDataValue();
        long secondval = host.right->getDataValue();
		host.setDataValue(firstval <= secondval);
    }
}

void TypeCheck::caseExprEQ(ASTExprEQ& host, void*)
{
	host.left->execute(*this);
	if (failure) return;
	host.right->execute(*this);
	if (failure) return;

	if (!standardCheck(*host.left->getVarType(), *host.right->getVarType(), &host))
	{
		failure = true;
		return;
	}

    host.setVarType(ZVarType::BOOL);

    if (host.left->hasDataValue() && host.right->hasDataValue())
    {
        long firstval = host.left->getDataValue();
        long secondval = host.right->getDataValue();
		host.setDataValue(firstval == secondval);
    }
}

void TypeCheck::caseExprNE(ASTExprNE& host, void*)
{
	host.left->execute(*this);
	if (failure) return;
	host.right->execute(*this);
	if (failure) return;

	if (!standardCheck(*host.left->getVarType(), *host.right->getVarType(), &host))
	{
		failure = true;
		return;
	}

    host.setVarType(ZVarType::BOOL);

    if (host.left->hasDataValue() && host.right->hasDataValue())
    {
        long firstval = host.left->getDataValue();
        long secondval = host.right->getDataValue();
		host.setDataValue(firstval != secondval);
    }
}

void TypeCheck::caseExprPlus(ASTExprPlus& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setVarType(ZVarType::FLOAT);

    if (host.left->hasDataValue() && host.right->hasDataValue())
    {
        long firstval = host.left->getDataValue();
        long secondval = host.right->getDataValue();
        host.setDataValue(firstval + secondval);
    }
}

void TypeCheck::caseExprMinus(ASTExprMinus& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setVarType(ZVarType::FLOAT);

    if (host.left->hasDataValue() && host.right->hasDataValue())
    {
        long firstval = host.left->getDataValue();
        long secondval = host.right->getDataValue();
        host.setDataValue(firstval - secondval);
    }
    }
    
void TypeCheck::caseExprTimes(ASTExprTimes& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setVarType(ZVarType::FLOAT);

    if (host.left->hasDataValue() && host.right->hasDataValue())
    {
        long firstval = host.left->getDataValue();
        long secondval = host.right->getDataValue();
        double temp = ((double)secondval)/10000.0;
        host.setDataValue((long)(firstval * temp));
    }
}

void TypeCheck::caseExprDivide(ASTExprDivide& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setVarType(ZVarType::FLOAT);

    if (host.left->hasDataValue() && host.right->hasDataValue())
    {
        long firstval = host.left->getDataValue();
        long secondval = host.right->getDataValue();

        if (secondval == 0)
        {
            compileError(host, &CompileError::DivByZero);
            return;
        }

        host.setDataValue((firstval / secondval) * 10000);
    }
}

void TypeCheck::caseExprModulo(ASTExprModulo& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setVarType(ZVarType::FLOAT);

    if (host.left->hasDataValue() && host.right->hasDataValue())
    {
        long firstval = host.left->getDataValue();
        long secondval = host.right->getDataValue();

        if (secondval == 0)
        {
            compileError(host, &CompileError::DivByZero);
            failure = true;
            return;
        }

        host.setDataValue(firstval % secondval);
    }
}

void TypeCheck::caseExprBitAnd(ASTExprBitAnd& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setVarType(ZVarType::FLOAT);

    if (host.left->hasDataValue() && host.right->hasDataValue())
    {
        long firstval = host.left->getDataValue();
        long secondval = host.right->getDataValue();
        host.setDataValue(((firstval/10000)&(secondval/10000))*10000);
    }
}
    
void TypeCheck::caseExprBitOr(ASTExprBitOr& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setVarType(ZVarType::FLOAT);

    if (host.left->hasDataValue() && host.right->hasDataValue())
    {
        long firstval = host.left->getDataValue();
        long secondval = host.right->getDataValue();
        host.setDataValue(((firstval/10000)|(secondval/10000))*10000);
    }
}

void TypeCheck::caseExprBitXor(ASTExprBitXor& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setVarType(ZVarType::FLOAT);

    if (host.left->hasDataValue() && host.right->hasDataValue())
    {
        long firstval = host.left->getDataValue();
        long secondval = host.right->getDataValue();
        host.setDataValue(((firstval/10000)^(secondval/10000))*10000);
    }
}
void TypeCheck::caseExprLShift(ASTExprLShift& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;

    if (host.right->hasDataValue())
    {
        if (host.right->getDataValue() % 10000)
        {
            compileError(host, &CompileError::ShiftNotInt);
            host.right->setDataValue(10000*(host.right->getDataValue()/10000));
        }
    }
    host.setVarType(ZVarType::FLOAT);

    if (host.left->hasDataValue() && host.right->hasDataValue())
    {
        long firstval = host.left->getDataValue();
        int secondval = host.right->getDataValue();
        host.setDataValue(((firstval/10000)<<(secondval/10000))*10000);
    }
}
void TypeCheck::caseExprRShift(ASTExprRShift& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;

    if (host.right->hasDataValue())
    {
        if (host.right->getDataValue() % 10000)
        {
            compileError(host, &CompileError::ShiftNotInt);
            host.right->setDataValue(10000*(host.right->getDataValue()/10000));
        }
    }
    host.setVarType(ZVarType::FLOAT);

    if (host.left->hasDataValue() && host.right->hasDataValue())
    {
        long firstval = host.left->getDataValue();
        int secondval = host.right->getDataValue();
        host.setDataValue(((firstval/10000)>>(secondval/10000))*10000);
            }
        }
        
// Literals

void TypeCheck::caseStringLiteral(ASTStringLiteral& host, void*) {}

void TypeCheck::caseArrayLiteral(ASTArrayLiteral& host, void*)
{
	// First, check elements.
	RecursiveVisitor::caseArrayLiteral(host);
	if (failure) return;

	// Check the explicit size is a number, if present.
	ASTExpr* size = host.getSize();
	if (size && !size->getVarType()->canCastTo(ZVarType::FLOAT))
	{
		compileError(host, &CompileError::NonIntegerArraySize);
		return;
	}

	// Don't allow an explicit size if we're part of a declaration.
	if (size && host.declaration)
	{
		compileError(host, &CompileError::ArrayLiteralResize);
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
		compileError(*toBlame, &CompileError::IllegalCast, msg.c_str());
	}
	return false;
}

bool TypeCheck::checkExprTypes(ASTUnaryExpr& expr, ZVarTypeId type)
{
	expr.operand->execute(*this);
	if (failure) return false;
	failure = !standardCheck(type, *expr.operand->getVarType(), &expr);
	return !failure;
}

bool TypeCheck::checkExprTypes(ASTBinaryExpr& expr, ZVarTypeId firstType, ZVarTypeId secondType)
{
	expr.left->execute(*this);
	if (failure) return false;
	failure = !standardCheck(firstType, *expr.left->getVarType(), &expr);
	if (failure) return false;

	expr.right->execute(*this);
	if (failure) return false;
	failure = !standardCheck(secondType, *expr.right->getVarType(), &expr);
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
	return !tc.hasFailed();
}

bool TypeCheck::check(SymbolTable& symbolTable, AST& node)
{
	return TypeCheck::check(symbolTable, ZVARTYPEID_VOID, node);
}

////////////////////////////////////////////////////////////////
// GetLValType

GetLValType::GetLValType(TypeCheck& typeCheck) : typeCheck(typeCheck) {}

void GetLValType::caseExprArrow(ASTExprArrow& host, void*)
{
	SymbolTable& symbolTable = typeCheck.symbolTable;

    host.left->execute(typeCheck);
    if (typeCheck.hasFailed()) return;

	// Don't need to check index here, since it'll be checked in the above
	// ASTExprIndex.
	bool isIndexed = host.index != NULL;

	// Make sure the left side is an object.
    ZVarTypeId leftTypeId = symbolTable.getTypeId(*host.left->getVarType());
	if (symbolTable.getType(leftTypeId)->typeClassId() != ZVARTYPE_CLASSID_CLASS)
    {
        typeCheck.compileError(host, &CompileError::ArrowNotPointer);
        return;
    }

	ZVarTypeClass& leftType = *(ZVarTypeClass*)symbolTable.getType(leftTypeId);
	ZClass& leftClass = *symbolTable.getClass(leftType.getClassId());

	Function* function = leftClass.getSetter(host.right);
    if (!function)
    {
        typeCheck.compileError(
				host, &CompileError::ArrowNoVar,
				(host.right + (isIndexed ? "[]" : "")).c_str());
        return;
    }
	vector<ZVarTypeId> functionParams = symbolTable.getFuncParamTypeIds(function->id);
    if (functionParams.size() != (isIndexed ? 3 : 2) || functionParams[0] != leftTypeId)
    {
        typeCheck.compileError(
				host, &CompileError::ArrowNoVar,
				(host.right + (isIndexed ? "[]" : "")).c_str());
        return;
    }

    symbolTable.putNodeId(&host, function->id);
    typeId = functionParams[isIndexed ? 2 : 1];
	host.setVarType(symbolTable.getType(typeId));
}

void GetLValType::caseExprIdentifier(ASTExprIdentifier& host, void*)
{
    host.execute(typeCheck);
    int vid = typeCheck.symbolTable.getNodeId(&host);

    if (vid == -1)
    {
        typeCheck.compileError(host, &CompileError::LValConst,
							   host.asString().c_str());
        return;
    }

    typeId = typeCheck.symbolTable.getVarTypeId(&host);
}

void GetLValType::caseExprIndex(ASTExprIndex& host, void*)
{
	// Arrows just fall back on the arrow implementation.
	if (host.array->isTypeArrow()) host.array->execute(*this);

	else
    {
		host.execute(typeCheck);
		typeId = typeCheck.symbolTable.getTypeId(*host.array->getVarType());
    }

	// The index must be a number.
	if (host.index)
    {
		host.index->execute(typeCheck);

		if (typeCheck.hasFailed()) return;

		if (!typeCheck.standardCheck(ZVARTYPEID_FLOAT, *host.index->getVarType(), host.index))
        {
            typeCheck.fail();
            return;
        }
    }

	host.setVarType(typeCheck.symbolTable.getType(typeId));
}

        
    

