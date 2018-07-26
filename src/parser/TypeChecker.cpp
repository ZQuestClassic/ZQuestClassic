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

struct tag {};
void* const TypeCheck::paramRead = new tag();
void* const TypeCheck::paramWrite = new tag();
void* const TypeCheck::paramReadWrite = new tag();

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
    if (breakRecursion(host)) return;

    ZVarType const& type = *host.condition->getReadType();

    if (!standardCheck(ZVARTYPEID_BOOL, type, &host))
        failure = true;
}
        
void TypeCheck::caseStmtIfElse(ASTStmtIfElse& host, void*)
{
    caseStmtIf(host);
    visit(host.elseStatement);
}
        
void TypeCheck::caseStmtSwitch(ASTStmtSwitch& host, void*)
{
	RecursiveVisitor::caseStmtSwitch(host);
	if (breakRecursion(host)) return;

	ZVarType const& type = *host.key->getReadType();
	if (!standardCheck(ZVARTYPEID_FLOAT, type, &host))
		failure = true;
}

void TypeCheck::caseStmtFor(ASTStmtFor& host, void*)
{
    RecursiveVisitor::caseStmtFor(host);
    if (breakRecursion(host)) return;

    ZVarType const& type = *host.test->getReadType();
    if (!standardCheck(ZVARTYPEID_BOOL, type, &host))
        failure = true;
}
        
void TypeCheck::caseStmtWhile(ASTStmtWhile& host, void*)
{
    RecursiveVisitor::caseStmtWhile(host);
    if (breakRecursion(host)) return;

    ZVarType const& type = *host.test->getReadType();
    if (!standardCheck(ZVARTYPEID_BOOL, type, &host))
        failure = true;
}
    
void TypeCheck::caseStmtReturn(ASTStmtReturn& host, void*)
{
    if (returnType != ZVarType::ZVOID)
	    handleError(CompileError::FuncBadReturn, &host,
				returnType.getName().c_str());
}

void TypeCheck::caseStmtReturnVal(ASTStmtReturnVal& host, void*)
{
    visit(host.value);
    if (breakRecursion(host)) return;

    if (!standardCheck(symbolTable.getTypeId(returnType),
					   *host.value->getReadType(), &host))
        failure = true;
}

// Declarations

void TypeCheck::caseDataDecl(ASTDataDecl& host, void*)
{
	RecursiveVisitor::caseDataDecl(host);
	if (breakRecursion(host)) return;

	Variable& variable = *host.manager;
    
	// Constants are treated special.
	if (*variable.type == ZVarType::CONST_FLOAT)
	{
		// A constant without an initializer doesn't make sense.
		if (!host.initializer())
	{
			handleError(CompileError::ConstUninitialized, &host);
		return;
	}

		// Inline the constant if possible.
		if (host.initializer()->getCompileTimeValue(this))
    {
			symbolTable.inlineConstant(&host, *host.initializer()->getCompileTimeValue(this));
			variable.compileTimeValue = host.initializer()->getCompileTimeValue(this);
		}
	}

	// Does it have an initializer?
	if (host.initializer())
        {
		// Make sure we can cast the initializer to the type.
		ZVarType const& initType = *host.initializer()->getReadType();
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
				handleError(CompileError::IllegalCast, &host, msg.c_str());
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
		if (!size.getReadType()->canCastTo(ZVarType::FLOAT))
		{
			handleError(CompileError::NonIntegerArraySize, &host);
			return;
		}

		// Make sure that the size is constant.
		if (!size.getCompileTimeValue(this))
	{
			handleError(CompileError::ExprNotConstant, &host);
			return;
		}
	}
}

// Expressions

void TypeCheck::caseExprConst(ASTExprConst& host, void*)
{
	ASTExpr* content = host.content;
	visit(content);

	if (!host.isConstant())
	{
		failure = true;
		handleError(CompileError::ExprNotConstant, &host);
		return;
	}
}

void TypeCheck::caseExprAssign(ASTExprAssign& host, void*)
{
	visit(host.left, paramWrite);
    if (breakRecursion(host)) return;

    visit(host.right, paramRead);
    if (breakRecursion(host)) return;

	ZVarType const& ltype = *host.left->getWriteType();
    ZVarType const& rtype = *host.right->getReadType();

    if (!standardCheck(ltype, rtype, &host))
        failure = true;

	if (ltype == ZVarType::CONST_FLOAT)
		handleError(CompileError::ConstAssign, &host);
}

void TypeCheck::caseExprIdentifier(ASTExprIdentifier& host, void* param)
{
	RecursiveVisitor::caseExprIdentifier(host);
    if (breakRecursion(host)) return;

	if (param == paramWrite || param == paramReadWrite)
	{
		if (*host.binding->type == ZVarType::CONST_FLOAT)
		{
			handleError(CompileError::LValConst, &host,
						host.asString().c_str());
			return;
		}
	}
}

void TypeCheck::caseExprArrow(ASTExprArrow& host, void* param)
{
	// Recurse on left.
	visit(host.left);
	if (breakRecursion(host)) return;

	// Grab the left side's class.
	ZVarType const& leftType = *host.left->getReadType();
	if (leftType.typeClassId() != ZVARTYPE_CLASSID_CLASS)
	{
		handleError(CompileError::ArrowNotPointer, &host);
		return;
	}
	ZClass& leftClass = *symbolTable.getClass(
			static_cast<ZVarTypeClass const&>(leftType).getClassId());

	// Find read function.
	if (!param || param == paramRead || param == paramReadWrite)
	{
		host.readFunction = leftClass.getGetter(host.right);
		if (!host.readFunction)
		{
			handleError(CompileError::ArrowNoVar, &host,
			            (host.right + (host.index ? "[]" : "")).c_str());
			return;
		}
		vector<ZVarType const*>& paramTypes = host.readFunction->paramTypes;
		if (paramTypes.size() != (host.index ? 2 : 1) || *paramTypes[0] != leftType)
		{
			handleError(CompileError::ArrowNoVar, &host,
			            (host.right + (host.index ? "[]" : "")).c_str());
			return;
		}
		symbolTable.putNodeId(&host, host.readFunction->id);
	}

	// Find write function.
	if (param == paramWrite || param == paramReadWrite)
	{
		host.writeFunction = leftClass.getSetter(host.right);
		if (!host.writeFunction)
		{
			handleError(CompileError::ArrowNoVar, &host,
			            (host.right + (host.index ? "[]" : "")).c_str());
			return;
		}
		vector<ZVarType const*>& paramTypes = host.writeFunction->paramTypes;
		if (paramTypes.size() != (host.index ? 3 : 2) || *paramTypes[0] != leftType)
		{
			handleError(CompileError::ArrowNoVar, &host,
			            (host.right + (host.index ? "[]" : "")).c_str());
			return;
		}
		symbolTable.putNodeId(&host, host.writeFunction->id);
	}

	if (host.index)
	{
		visit(host.index);
		if (breakRecursion(host)) return;

		standardCheck(
				ZVarType::FLOAT, *host.index->getReadType(), host.index);
		if (breakRecursion(host)) return;
	}
}

void TypeCheck::caseExprIndex(ASTExprIndex& host, void* param)
{
	// Arrow handles its own indexing.
	if (host.array->isTypeArrow())
	{
		visit(host.array, param);
		host.setVarType(host.array->getReadType());
		return;
	}
	
	RecursiveVisitor::caseExprIndex(host);
	host.setVarType(host.array->getReadType());

	// The index must be a number.
    if (host.index)
    {
        if (breakRecursion(host)) return;

        if (!standardCheck(ZVARTYPEID_FLOAT, *host.index->getReadType(), host.index))
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
        visit(lval->left);
        if (breakRecursion(host)) return;
        ZVarType const& lvaltype = *lval->left->getReadType();

		if (lvaltype.typeClassId() != ZVARTYPE_CLASSID_CLASS)
        {
	        handleError(CompileError::ArrowNotPointer, lval);
	        return;
        }

        // Prepend that type to the function parameters, as that is implicitly passed
        paramtypes.push_back(symbolTable.getTypeId(lvaltype));
    }

    // Now add the normal parameters.
    for (vector<ASTExpr*>::iterator it = params.begin(); it != params.end(); it++)
    {
        visit(*it);
        if (breakRecursion(host)) return;

        paramtypes.push_back(symbolTable.getTypeId(*(*it)->getReadType()));
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
	        handleError(CompileError::NoFuncMatch, &host,
						 (fullname + paramstring).c_str());
            return;
        }
        else if (bestmatch.size() > 1)
        {
	        handleError(CompileError::TooFuncMatch, &host,
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
		ASTExprArrow& arrow = *static_cast<ASTExprArrow*>(host.left);
        string name = arrow.right;

		// Make sure the left side is an object.
        ZVarType const& leftType = *arrow.left->getReadType();
		if (leftType.typeClassId() != ZVARTYPE_CLASSID_CLASS)
		{
			handleError(CompileError::ArrowNotPointer, &host);
			return;
		}

		ZClass& leftClass = *symbolTable.getClass(
				static_cast<ZVarTypeClass const&>(leftType).getClassId());

		vector<Function*> functions = leftClass.getFunctions(name);
		if (functions.size() == 1) arrow.readFunction = functions[0];
		else
        {
			handleError(CompileError::ArrowNoFunc, &host, name.c_str());
            return;
        }

        if (paramtypes.size() != arrow.readFunction->paramTypes.size())
        {
	        handleError(CompileError::NoFuncMatch, &host,
						 (name + paramstring).c_str());
            return;
        }

        for (unsigned int i = 0; i < paramtypes.size(); i++)
        {
            if (!standardCheck(*arrow.readFunction->paramTypes[i], paramtypes[i], NULL))
            {
	            handleError(CompileError::NoFuncMatch, &host,
							 (name + paramstring).c_str());
                return;
            }
        }

        symbolTable.putNodeId(&host, arrow.readFunction->id);
    }
}

void TypeCheck::caseExprNegate(ASTExprNegate& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT)) return;
}

void TypeCheck::caseExprNot(ASTExprNot& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_BOOL)) return;
}

void TypeCheck::caseExprBitNot(ASTExprBitNot& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT)) return;
}

void TypeCheck::caseExprIncrement(ASTExprIncrement& host, void*)
{
    visit(host.operand, paramReadWrite);
    if (breakRecursion(host)) return;

	ASTExpr& operand = *host.operand;
	if (operand.isTypeArrow())
	{
		ASTExprArrow& arrow = static_cast<ASTExprArrow&>(operand);
		symbolTable.putNodeId(&host, arrow.readFunction->id);
	}
	if (operand.isTypeIndex())
	{
		ASTExprIndex& index = static_cast<ASTExprIndex&>(operand);
		if (index.array->isTypeArrow())
		{
			ASTExprArrow& arrow = static_cast<ASTExprArrow&>(*index.array);
			symbolTable.putNodeId(&host, arrow.readFunction->id);
		}
	}

    standardCheck(ZVarType::FLOAT, *operand.getReadType(), &host);
    if (breakRecursion(host)) return;
}
    
void TypeCheck::caseExprPreIncrement(ASTExprPreIncrement& host, void*)
{
    visit(host.operand, paramReadWrite);
    if (breakRecursion(host)) return;

	ASTExpr& operand = *host.operand;
	if (operand.isTypeArrow())
	{
		ASTExprArrow& arrow = static_cast<ASTExprArrow&>(operand);
		symbolTable.putNodeId(&host, arrow.readFunction->id);
	}
	if (operand.isTypeIndex())
	{
		ASTExprIndex& index = static_cast<ASTExprIndex&>(operand);
		if (index.array->isTypeArrow())
		{
			ASTExprArrow& arrow = static_cast<ASTExprArrow&>(*index.array);
			symbolTable.putNodeId(&host, arrow.readFunction->id);
		}
	}

    standardCheck(ZVarType::FLOAT, *operand.getReadType(), &host);
    if (breakRecursion(host)) return;
}

void TypeCheck::caseExprDecrement(ASTExprDecrement& host, void*)
{
    visit(host.operand, paramReadWrite);
    if (breakRecursion(host)) return;

	ASTExpr& operand = *host.operand;
	if (operand.isTypeArrow())
	{
		ASTExprArrow& arrow = static_cast<ASTExprArrow&>(operand);
		symbolTable.putNodeId(&host, arrow.readFunction->id);
	}
	if (operand.isTypeIndex())
	{
		ASTExprIndex& index = static_cast<ASTExprIndex&>(operand);
		if (index.array->isTypeArrow())
		{
			ASTExprArrow& arrow = static_cast<ASTExprArrow&>(*index.array);
			symbolTable.putNodeId(&host, arrow.readFunction->id);
		}
	}

    standardCheck(ZVarType::FLOAT, *operand.getReadType(), &host);
    if (breakRecursion(host)) return;
}
    
void TypeCheck::caseExprPreDecrement(ASTExprPreDecrement& host, void*)
{
    visit(host.operand, paramReadWrite);
    if (breakRecursion(host)) return;

	ASTExpr& operand = *host.operand;
	if (operand.isTypeArrow())
	{
		ASTExprArrow& arrow = static_cast<ASTExprArrow&>(operand);
		symbolTable.putNodeId(&host, arrow.readFunction->id);
	}
	if (operand.isTypeIndex())
	{
		ASTExprIndex& index = static_cast<ASTExprIndex&>(operand);
		if (index.array->isTypeArrow())
		{
			ASTExprArrow& arrow = static_cast<ASTExprArrow&>(*index.array);
			symbolTable.putNodeId(&host, arrow.readFunction->id);
		}
	}

    standardCheck(ZVarType::FLOAT, *operand.getReadType(), &host);
    if (breakRecursion(host)) return;
}

void TypeCheck::caseExprAnd(ASTExprAnd& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_BOOL, ZVARTYPEID_BOOL)) return;
}

void TypeCheck::caseExprOr(ASTExprOr& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_BOOL, ZVARTYPEID_BOOL)) return;
}

void TypeCheck::caseExprGT(ASTExprGT& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
}

void TypeCheck::caseExprGE(ASTExprGE& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
}

void TypeCheck::caseExprLT(ASTExprLT& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
}

void TypeCheck::caseExprLE(ASTExprLE& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
}

void TypeCheck::caseExprEQ(ASTExprEQ& host, void*)
{
	visit(host.left);
	if (breakRecursion(host)) return;
	visit(host.right);
	if (breakRecursion(host)) return;

	if (!standardCheck(*host.left->getReadType(), *host.right->getReadType(),
					   &host))
	{
		failure = true;
		return;
	}
}

void TypeCheck::caseExprNE(ASTExprNE& host, void*)
{
	visit(host.left);
	if (breakRecursion(host)) return;
	visit(host.right);
	if (breakRecursion(host)) return;

	if (!standardCheck(*host.left->getReadType(), *host.right->getReadType(),
					   &host))
	{
		failure = true;
		return;
	}
}

void TypeCheck::caseExprPlus(ASTExprPlus& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
}

void TypeCheck::caseExprMinus(ASTExprMinus& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
}
    
void TypeCheck::caseExprTimes(ASTExprTimes& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
}

void TypeCheck::caseExprDivide(ASTExprDivide& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
}

void TypeCheck::caseExprModulo(ASTExprModulo& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
}

void TypeCheck::caseExprBitAnd(ASTExprBitAnd& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
}
    
void TypeCheck::caseExprBitOr(ASTExprBitOr& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
}

void TypeCheck::caseExprBitXor(ASTExprBitXor& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
}

void TypeCheck::caseExprLShift(ASTExprLShift& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
}

void TypeCheck::caseExprRShift(ASTExprRShift& host, void*)
{
	if (!checkExprTypes(host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
}
        
// Literals

void TypeCheck::caseStringLiteral(ASTStringLiteral& host, void*) {}

void TypeCheck::caseArrayLiteral(ASTArrayLiteral& host, void*)
{
	// First, check elements.
	RecursiveVisitor::caseArrayLiteral(host);
	if (breakRecursion(host)) return;

	// Check the explicit size is a number, if present.
	ASTExpr* size = host.getSize();
	if (size && !size->getReadType()->canCastTo(ZVarType::FLOAT))
	{
		handleError(CompileError::NonIntegerArraySize, &host);
		return;
	}

	// Don't allow an explicit size if we're part of a declaration.
	if (size && host.declaration)
	{
		handleError(CompileError::ArrayLiteralResize, &host);
		return;
	}

	// If we don't have a type assigned, grab it from the first element.
	ZVarTypeArray const* type = (ZVarTypeArray const*)host.getReadType();
	ZVarType const* elementType;
	if (type)
		elementType = &type->getElementType();
	else
	{
		elementType = host.getElements()[0]->getReadType();
		type = (ZVarTypeArray const*)symbolTable.getCanonicalType(ZVarTypeArray(*elementType));
		host.setVarType(type);
	}

	// Check each element to make sure it adheres to type.
	vector<ASTExpr*> elements = host.getElements();
	for (vector<ASTExpr*>::iterator it = elements.begin();
		 it != elements.end(); ++it)
	{
		if (!standardCheck(*elementType, *(*it)->getReadType(), &host))
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
		handleError(CompileError::IllegalCast, toBlame, msg.c_str());
	}
	return false;
}

bool TypeCheck::checkExprTypes(ASTUnaryExpr& expr, ZVarTypeId type)
{
	visit(expr.operand);
	if (failure) return false;
	failure = !standardCheck(type, *expr.operand->getReadType(), &expr);
	return !failure;
}

bool TypeCheck::checkExprTypes(ASTBinaryExpr& expr, ZVarTypeId firstType, ZVarTypeId secondType)
{
	visit(expr.left);
	if (failure) return false;
	failure = !standardCheck(firstType, *expr.left->getReadType(), &expr);
	if (failure) return false;

	visit(expr.right);
	if (failure) return false;
	failure = !standardCheck(secondType, *expr.right->getReadType(), &expr);
	return !failure;
}

bool TypeCheck::check(SymbolTable& symbolTable, ZVarTypeId returnTypeId, AST& node)
{
	TypeCheck tc(symbolTable, returnTypeId);
	tc.visit(node);
	return !tc.hasFailed();
}

bool TypeCheck::check(SymbolTable& symbolTable, AST& node)
{
	return TypeCheck::check(symbolTable, ZVARTYPEID_VOID, node);
}

