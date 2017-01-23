#ifndef TYPECHECKER_H //2.53 Updated to 16th Jan, 2017
#define TYPECHECKER_H

#include "UtilVisitors.h"

class TypeCheck : public RecursiveVisitor
{
public:
    TypeCheck() : failure(false) {}

    void caseDefault(void *) {}
	// Statements
    void caseStmtAssign(ASTStmtAssign &host, void *param);
    void caseStmtIf(ASTStmtIf &host, void *param);
    void caseStmtIfElse(ASTStmtIfElse &host, void *param);
	void caseStmtSwitch(ASTStmtSwitch &host, void* param);
    void caseStmtFor(ASTStmtFor &host, void *param);
    void caseStmtWhile(ASTStmtWhile &host, void *param);
    void caseStmtReturn(ASTStmtReturn &host, void *param);
    void caseStmtReturnVal(ASTStmtReturnVal &host, void *param);
	// Declarations
    void caseArrayDecl(ASTArrayDecl &host, void *param);
    void caseVarDeclInitializer(ASTVarDeclInitializer &host, void *param);
	// Expressions
	void caseExprConst(ASTExprConst &host, void *param);
    void caseNumConstant(ASTNumConstant &host, void *param);
    void caseBoolConstant(ASTBoolConstant &host, void *param);
    void caseStringConstant(ASTStringConstant &host, void *param);
    void caseExprDot(ASTExprDot &host, void *param);
    void caseExprArrow(ASTExprArrow &host, void *param);
    void caseExprArray(ASTExprArray &host, void *param);
    void caseFuncCall(ASTFuncCall &host, void *param);
    void caseExprNegate(ASTExprNegate &host, void *param);
    void caseExprNot(ASTExprNot &host, void *param);
    void caseExprBitNot(ASTExprBitNot &host, void *param);
    void caseExprIncrement(ASTExprIncrement &host, void *param);
    void caseExprPreIncrement(ASTExprPreIncrement &host, void *param);
    void caseExprDecrement(ASTExprDecrement &host, void *param);
    void caseExprPreDecrement(ASTExprPreDecrement &host, void *param);
    void caseExprAnd(ASTExprAnd &host, void *param);
    void caseExprOr(ASTExprOr &host, void *param);
    void caseExprGT(ASTExprGT &host, void *param);
    void caseExprGE(ASTExprGE &host, void *param);
    void caseExprLT(ASTExprLT &host, void *param);
    void caseExprLE(ASTExprLE &host, void *param);
    void caseExprEQ(ASTExprEQ &host, void *param);
    void caseExprNE(ASTExprNE &host, void *param);
    void caseExprPlus(ASTExprPlus &host, void *param);
    void caseExprMinus(ASTExprMinus &host, void *param);
    void caseExprTimes(ASTExprTimes &host, void *param);
    void caseExprDivide(ASTExprDivide &host, void *param);
    void caseExprModulo(ASTExprModulo &host, void *param);
    void caseExprBitAnd(ASTExprBitAnd &host, void *param);
    void caseExprBitOr(ASTExprBitOr &host, void *param);
    void caseExprBitXor(ASTExprBitXor &host, void *param);
    void caseExprLShift(ASTExprLShift &host, void *param);
    void caseExprRShift(ASTExprRShift &host, void *param);

    bool isOK() {return !failure;}
    void fail() {failure = true;}
    friend class GetLValType;
private:
    bool failure;
    bool standardCheck(int firsttype, int secondtype, AST *toblame);
};

class GetLValType : public ASTVisitor
{
public:
    void caseDefault(void *param);
    void caseVarDecl(ASTVarDecl &host, void *param);
    void caseExprDot(ASTExprDot &host, void *param);
    void caseExprArrow(ASTExprArrow &host, void *param);
    void caseExprArray(ASTExprArray &host, void *param);
};

#endif

