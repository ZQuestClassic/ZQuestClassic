#ifndef TYPECHECKER_H
#define TYPECHECKER_H

#include "ASTVisitors.h"
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
    void caseStmtIf(ASTStmtIf& host, void* = NULL);
    void caseStmtIfElse(ASTStmtIfElse& host, void* = NULL);
	void caseStmtSwitch(ASTStmtSwitch& host, void* = NULL);
    void caseStmtFor(ASTStmtFor& host, void* = NULL);
    void caseStmtWhile(ASTStmtWhile& host, void* = NULL);
    void caseStmtReturn(ASTStmtReturn& host, void* = NULL);
    void caseStmtReturnVal(ASTStmtReturnVal& host, void* = NULL);
	// Declarations
	void caseDataDecl(ASTDataDecl& host, void* = NULL);
	void caseDataDeclExtraArray(ASTDataDeclExtraArray& host, void* = NULL);
	// Expressions
	void caseExprConst(ASTExprConst& host, void* = NULL);
    void caseExprAssign(ASTExprAssign& host, void* = NULL);
    void caseExprIdentifier(ASTExprIdentifier& host, void* = NULL);
    void caseExprArrow(ASTExprArrow& host, void* = NULL);
    void caseExprIndex(ASTExprIndex& host, void* = NULL);
    void caseExprCall(ASTExprCall& host, void* = NULL);
    void caseExprNegate(ASTExprNegate& host, void* = NULL);
    void caseExprNot(ASTExprNot& host, void* = NULL);
    void caseExprBitNot(ASTExprBitNot& host, void* = NULL);
    void caseExprIncrement(ASTExprIncrement& host, void* = NULL);
    void caseExprPreIncrement(ASTExprPreIncrement& host, void* = NULL);
    void caseExprDecrement(ASTExprDecrement& host, void* = NULL);
    void caseExprPreDecrement(ASTExprPreDecrement& host, void* = NULL);
    void caseExprAnd(ASTExprAnd& host, void* = NULL);
    void caseExprOr(ASTExprOr& host, void* = NULL);
    void caseExprGT(ASTExprGT& host, void* = NULL);
    void caseExprGE(ASTExprGE& host, void* = NULL);
    void caseExprLT(ASTExprLT& host, void* = NULL);
    void caseExprLE(ASTExprLE& host, void* = NULL);
    void caseExprEQ(ASTExprEQ& host, void* = NULL);
    void caseExprNE(ASTExprNE& host, void* = NULL);
    void caseExprPlus(ASTExprPlus& host, void* = NULL);
    void caseExprMinus(ASTExprMinus& host, void* = NULL);
    void caseExprTimes(ASTExprTimes& host, void* = NULL);
    void caseExprDivide(ASTExprDivide& host, void* = NULL);
    void caseExprModulo(ASTExprModulo& host, void* = NULL);
    void caseExprBitAnd(ASTExprBitAnd& host, void* = NULL);
    void caseExprBitOr(ASTExprBitOr& host, void* = NULL);
    void caseExprBitXor(ASTExprBitXor& host, void* = NULL);
    void caseExprLShift(ASTExprLShift& host, void* = NULL);
    void caseExprRShift(ASTExprRShift& host, void* = NULL);
	// Literals
    void caseStringLiteral(ASTStringLiteral& host, void* = NULL);
	void caseArrayLiteral(ASTArrayLiteral& host, void* = NULL);

	static bool check(SymbolTable& symbolTable, ZVarTypeId returnTypeId, AST& node);
	static bool check(SymbolTable& symbolTable, AST& node);
private:
	SymbolTable& symbolTable;
	ZVarType const& returnType;
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
    void caseExprIdentifier(ASTExprIdentifier& host, void* = NULL);
    void caseExprArrow(ASTExprArrow& host, void* = NULL);
    void caseExprIndex(ASTExprIndex& host, void* = NULL);
	ZVarTypeId typeId;
private:
	TypeCheck& typeCheck;
};

#endif

