#ifndef TYPECHECKER_H //2.53 Updated to 16th Jan, 2017
#define TYPECHECKER_H

#include "UtilVisitors.h"
#include "DataStructs.h"
#include <assert.h>

class TypeCheckParam;

class TypeCheck : public RecursiveVisitor
{
    friend class GetLValType;
public:
	TypeCheck(SymbolTable& symbolTable);
	TypeCheck(SymbolTable& symbolTable, ZVarTypeId returnTypeId);
	TypeCheck(SymbolTable& symbolTable, ZVarType const& returnType);

    void caseDefault(void*) {}
	// Statements
    void caseStmtIf(ASTStmtIf &host);
    void caseStmtIfElse(ASTStmtIfElse &host);
	void caseStmtSwitch(ASTStmtSwitch &host);
    void caseStmtFor(ASTStmtFor &host);
    void caseStmtWhile(ASTStmtWhile &host);
    void caseStmtReturn(ASTStmtReturn &host);
    void caseStmtReturnVal(ASTStmtReturnVal &host);
	// Declarations
	void caseDataDecl(ASTDataDecl& host);
	void caseDataDeclExtraArray(ASTDataDeclExtraArray& host);
	// Expressions
	void caseExprConst(ASTExprConst &host);
    void caseExprAssign(ASTExprAssign &host);
    void caseExprIdentifier(ASTExprIdentifier &host);
    void caseExprArrow(ASTExprArrow &host);
    void caseExprIndex(ASTExprIndex &host);
    void caseExprCall(ASTExprCall &host);
    void caseExprNegate(ASTExprNegate &host);
    void caseExprNot(ASTExprNot &host);
    void caseExprBitNot(ASTExprBitNot &host);
    void caseExprIncrement(ASTExprIncrement &host);
    void caseExprPreIncrement(ASTExprPreIncrement &host);
    void caseExprDecrement(ASTExprDecrement &host);
    void caseExprPreDecrement(ASTExprPreDecrement &host);
    void caseExprAnd(ASTExprAnd &host);
    void caseExprOr(ASTExprOr &host);
    void caseExprGT(ASTExprGT &host);
    void caseExprGE(ASTExprGE &host);
    void caseExprLT(ASTExprLT &host);
    void caseExprLE(ASTExprLE &host);
    void caseExprEQ(ASTExprEQ &host);
    void caseExprNE(ASTExprNE &host);
    void caseExprPlus(ASTExprPlus &host);
    void caseExprMinus(ASTExprMinus &host);
    void caseExprTimes(ASTExprTimes &host);
    void caseExprDivide(ASTExprDivide &host);
    void caseExprModulo(ASTExprModulo &host);
    void caseExprBitAnd(ASTExprBitAnd &host);
    void caseExprBitOr(ASTExprBitOr &host);
    void caseExprBitXor(ASTExprBitXor &host);
    void caseExprLShift(ASTExprLShift &host);
    void caseExprRShift(ASTExprRShift &host);
	// Literals
    void caseNumberLiteral(ASTNumberLiteral& host);
    void caseBoolLiteral(ASTBoolLiteral& host);
    void caseStringLiteral(ASTStringLiteral& host);
	void caseArrayLiteral(ASTArrayLiteral& host);

    bool isOK() {return !failure;}
    void fail() {failure = true;}
	static bool check(SymbolTable& symbolTable, ZVarTypeId returnTypeId, AST& node);
	static bool check(SymbolTable& symbolTable, AST& node);
private:
	SymbolTable& symbolTable;
	ZVarType const& returnType;
    bool failure;
    bool standardCheck(ZVarTypeId targetTypeId, ZVarTypeId sourceTypeId, AST* toBlame);
    bool standardCheck(ZVarType const& targetType, ZVarTypeId sourceTypeId, AST* toBlame);
    bool standardCheck(ZVarTypeId targetTypeId, ZVarType const& sourceType, AST* toBlame);
    bool standardCheck(ZVarType const& targetType, ZVarType const& sourceType, AST* toBlame);
	bool checkExprTypes(ASTUnaryExpr& expr, ZVarTypeId type);
	bool checkExprTypes(ASTBinaryExpr& expr, ZVarTypeId firstType, ZVarTypeId secondType);
	ZVarTypeId getLValTypeId(AST& lval);
};

class GetLValType : public ASTVisitor
{
public:
	GetLValType(TypeCheck& typeCheck);
    void caseDefault(void* param) {assert(false);}
    void caseExprIdentifier(ASTExprIdentifier& host);
    void caseExprArrow(ASTExprArrow& host);
    void caseExprIndex(ASTExprIndex& host);
	ZVarTypeId typeId;
private:
	TypeCheck& typeCheck;
};

#endif

