#ifndef UTILVISITORS_H //2.53 Updated to 16th Jan, 2017
#define UTILVISITORS_H

#include "AST.h"

class RecursiveVisitor : public ASTVisitor
{
public:
    virtual void caseDefault(void*) {}
    virtual void caseProgram(ASTProgram &host, void *param);
    virtual void caseProgram(ASTProgram &host);
	// Statements
    virtual void caseBlock(ASTBlock &host, void *param);
    virtual void caseBlock(ASTBlock &host);
    virtual void caseStmtAssign(ASTStmtAssign &host, void *param);
    virtual void caseStmtAssign(ASTStmtAssign &host);
    virtual void caseStmtIf(ASTStmtIf &host, void *param);
    virtual void caseStmtIf(ASTStmtIf &host);
    virtual void caseStmtIfElse(ASTStmtIfElse &host, void *param);
    virtual void caseStmtIfElse(ASTStmtIfElse &host);
	virtual void caseStmtSwitch(ASTStmtSwitch & host, void* param);
	virtual void caseStmtSwitch(ASTStmtSwitch & host);
	virtual void caseSwitchCases(ASTSwitchCases & host, void* param);
	virtual void caseSwitchCases(ASTSwitchCases & host);
    virtual void caseStmtFor(ASTStmtFor &host, void* param);
    virtual void caseStmtFor(ASTStmtFor &host);
    virtual void caseStmtWhile(ASTStmtWhile &host, void *param);
    virtual void caseStmtWhile(ASTStmtWhile &host);
    virtual void caseStmtDo(ASTStmtDo &host, void *param);
    virtual void caseStmtDo(ASTStmtDo &host);
    virtual void caseStmtReturnVal(ASTStmtReturnVal &host, void *param);
    virtual void caseStmtReturnVal(ASTStmtReturnVal &host);
	// Declarations
    virtual void caseScript(ASTScript &host, void *param);
    virtual void caseScript(ASTScript &host);
    virtual void caseDeclList(ASTDeclList &host, void *param);
    virtual void caseDeclList(ASTDeclList &host);
    virtual void caseFuncDecl(ASTFuncDecl &host, void *param);
    virtual void caseFuncDecl(ASTFuncDecl &host);
    virtual void caseArrayDecl(ASTArrayDecl &host, void *param);
    virtual void caseArrayDecl(ASTArrayDecl &host);
    virtual void caseVarDecl(ASTVarDecl &host, void *param);
    virtual void caseVarDecl(ASTVarDecl &host);
    virtual void caseVarDeclInitializer(ASTVarDeclInitializer &host, void *param);
    virtual void caseVarDeclInitializer(ASTVarDeclInitializer &host);
    virtual void caseTypeDef(ASTTypeDef&, void* param);
    virtual void caseTypeDef(ASTTypeDef& node);
	// Expressions
	virtual void caseExprConst(ASTExprConst &host, void *param);
	virtual void caseExprConst(ASTExprConst &host);
    virtual void caseNumConstant(ASTNumConstant &host, void *param);
    virtual void caseNumConstant(ASTNumConstant &host);
    virtual void caseExprArrow(ASTExprArrow &host, void *param);
    virtual void caseExprArrow(ASTExprArrow &host);
    virtual void caseExprIndex(ASTExprIndex &host, void *param);
    virtual void caseExprIndex(ASTExprIndex &host);
    virtual void caseFuncCall(ASTFuncCall &host, void *param);
    virtual void caseFuncCall(ASTFuncCall &host);
    virtual void caseExprNegate(ASTExprNegate &host, void *param);
    virtual void caseExprNegate(ASTExprNegate &host);
    virtual void caseExprNot(ASTExprNot &host, void *param);
    virtual void caseExprNot(ASTExprNot &host);
    virtual void caseExprBitNot(ASTExprBitNot &host, void *param);
    virtual void caseExprBitNot(ASTExprBitNot &host);
    virtual void caseExprIncrement(ASTExprIncrement &host, void *param);
    virtual void caseExprIncrement(ASTExprIncrement &host);
    virtual void caseExprPreIncrement(ASTExprPreIncrement &host, void *param);
    virtual void caseExprPreIncrement(ASTExprPreIncrement &host);
    virtual void caseExprDecrement(ASTExprDecrement &host, void *param);
    virtual void caseExprDecrement(ASTExprDecrement &host);
    virtual void caseExprPreDecrement(ASTExprPreDecrement &host, void *param);
    virtual void caseExprPreDecrement(ASTExprPreDecrement &host);
    virtual void caseExprAnd(ASTExprAnd &host, void *param);
    virtual void caseExprAnd(ASTExprAnd &host);
    virtual void caseExprOr(ASTExprOr &host, void *param);
    virtual void caseExprOr(ASTExprOr &host);
    virtual void caseExprGT(ASTExprGT &host, void *param);
    virtual void caseExprGT(ASTExprGT &host);
    virtual void caseExprGE(ASTExprGE &host, void *param);
    virtual void caseExprGE(ASTExprGE &host);
    virtual void caseExprLT(ASTExprLT &host, void *param);
    virtual void caseExprLT(ASTExprLT &host);
    virtual void caseExprLE(ASTExprLE &host, void *param);
    virtual void caseExprLE(ASTExprLE &host);
    virtual void caseExprEQ(ASTExprEQ &host, void *param);
    virtual void caseExprEQ(ASTExprEQ &host);
    virtual void caseExprNE(ASTExprNE &host, void *param);
    virtual void caseExprNE(ASTExprNE &host);
    virtual void caseExprPlus(ASTExprPlus &host, void *param);
    virtual void caseExprPlus(ASTExprPlus &host);
    virtual void caseExprMinus(ASTExprMinus &host, void *param);
    virtual void caseExprMinus(ASTExprMinus &host);
    virtual void caseExprTimes(ASTExprTimes &host, void *param);
    virtual void caseExprTimes(ASTExprTimes &host);
    virtual void caseExprDivide(ASTExprDivide &host, void *param);
    virtual void caseExprDivide(ASTExprDivide &host);
    virtual void caseExprModulo(ASTExprModulo &host, void *param);
    virtual void caseExprModulo(ASTExprModulo &host);
    virtual void caseExprBitAnd(ASTExprBitAnd &host, void *param);
    virtual void caseExprBitAnd(ASTExprBitAnd &host);
    virtual void caseExprBitOr(ASTExprBitOr &host, void *param);
    virtual void caseExprBitOr(ASTExprBitOr &host);
    virtual void caseExprBitXor(ASTExprBitXor &host, void *param);
    virtual void caseExprBitXor(ASTExprBitXor &host);
    virtual void caseExprLShift(ASTExprLShift &host, void *param);
    virtual void caseExprLShift(ASTExprLShift &host);
    virtual void caseExprRShift(ASTExprRShift &host, void *param);
    virtual void caseExprRShift(ASTExprRShift &host);
};

class CheckForExtraneousImports : public RecursiveVisitor
{
public:
    CheckForExtraneousImports() : ok(true) {}

    void caseDefault(void *param);
    void caseImportDecl(ASTImportDecl &host, void *param);

	bool isOK() const {return ok;}
private:
    bool ok;
};

class IsArrayDecl : public ASTVisitor
{
public:
    void caseDefault(void *param) {*(bool *)param = false;}
    void caseArrayDecl(ASTArrayDecl &, void *param) {*(bool *)param = true;}
};

class IsBlock : public ASTVisitor
{
public:
    void caseDefault(void *param) {*(bool *)param = false;}
    void caseBlock(ASTBlock &, void *param) {*(bool *)param = true;}
};

class IsFuncDecl : public ASTVisitor
{
public:
    void caseDefault(void *param) {*(bool *)param = false;}
    virtual void caseFuncDecl(ASTFuncDecl &, void *param) {*(bool *)param = true;}
};

class IsTypeDef : public ASTVisitor
{
public:
    void caseDefault(void *param) {*(bool*)param = false;}
    void caseTypeDef(ASTTypeDef &, void *param) {*(bool*)param = true;}
};

#endif

