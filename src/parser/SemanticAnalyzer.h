#ifndef ZSCRIPT_SEMANTIC_ANALYZER_H
#define ZSCRIPT_SEMANTIC_ANALYZER_H

#include "ASTVisitors.h"
#include "DataStructs.h"
#include "ZScript.h"

namespace ZScript
{
	class SemanticAnalyzer : public RecursiveVisitor
	{
	public:

		SemanticAnalyzer(ZScript::Program& program);

		////////////////
		// Cases
		void caseFile(ASTFile& host, void* = NULL);
		void caseSetOption(ASTSetOption& host, void* param = NULL);
		void caseUsing(ASTUsingDecl& host, void* param = NULL);
		// Statements
		void caseBlock(ASTBlock& host, void* = NULL);
		void caseStmtIf(ASTStmtIf& host, void* = NULL);
		void caseStmtIfElse(ASTStmtIfElse& host, void* = NULL);
		void caseStmtSwitch(ASTStmtSwitch& host, void* = NULL);
		void caseRange(ASTRange& host, void* = NULL);
		void caseStmtFor(ASTStmtFor& host, void* = NULL);
		void caseStmtForEach(ASTStmtForEach& host, void* = NULL);
		void caseStmtRangeLoop(ASTStmtRangeLoop& host, void* = NULL);
		void caseStmtWhile(ASTStmtWhile& host, void* = NULL);
		void caseStmtReturn(ASTStmtReturn& host, void* = NULL);
		void caseStmtReturnVal(ASTStmtReturnVal& host, void* = NULL);
		void caseStmtBreak(ASTStmtBreak &host, void* = NULL);
		void caseStmtContinue(ASTStmtContinue &host, void* = NULL);
		// Declarations
		void caseDataTypeDef(ASTDataTypeDef& host, void* = NULL);
		void caseCustomDataTypeDef(ASTCustomDataTypeDef& host, void* = NULL);
		void caseScriptTypeDef(ASTScriptTypeDef& host, void* = NULL);
		void caseDataDeclList(ASTDataDeclList& host, void* = NULL);
		void caseDataEnum(ASTDataEnum& host, void* = NULL);
		void caseDataDecl(ASTDataDecl& host, void* = NULL);
		void caseDataDeclExtraArray(ASTDataDeclExtraArray& host, void* = NULL);
		void caseFuncDecl(ASTFuncDecl& host, void* = NULL);
		void caseScript(ASTScript& host, void* = NULL);
		void caseClass(ASTClass& host, void* = NULL);
		void caseNamespace(ASTNamespace& host, void* = NULL);
		void caseImportDecl(ASTImportDecl& host, void* = NULL);
		void caseIncludePath(ASTIncludePath& host, void* = NULL);
		void caseImportCondDecl(ASTImportCondDecl& host, void* = NULL);
		void caseAssert(ASTAssert& host, void* = NULL);
		// Expressions
		void caseExprConst(ASTExprConst& host, void* = NULL);
		void caseVarInitializer(ASTExprVarInitializer& host, void* param = NULL);
		void caseExprAssign(ASTExprAssign& host, void* = NULL);
		void caseExprIdentifier(ASTExprIdentifier& host, void* = NULL);
		void caseExprArrow(ASTExprArrow& host, void* = NULL);
		void caseExprIndex(ASTExprIndex& host, void* = NULL);
		void caseExprCall(ASTExprCall& host, void* = NULL);
		void caseExprNegate(ASTExprNegate& host, void* = NULL);
		void caseExprDelete(ASTExprDelete& host, void* = NULL);
		void caseExprNot(ASTExprNot& host, void* = NULL);
		void caseExprBitNot(ASTExprBitNot& host, void* = NULL);
		void caseExprIncrement(ASTExprIncrement& host, void* = NULL);
		void caseExprPreIncrement(ASTExprPreIncrement& host, void* = NULL);
		void caseExprDecrement(ASTExprDecrement& host, void* = NULL);
		void caseExprPreDecrement(ASTExprPreDecrement& host, void* = NULL);
		void caseExprCast(ASTExprCast& host, void* = NULL);
		void caseExprAnd(ASTExprAnd& host, void* = NULL);
		void caseExprOr(ASTExprOr& host, void* = NULL);
		void caseExprXOR(ASTExprXOR& host, void* = NULL);
		void caseExprGT(ASTExprGT& host, void* = NULL);
		void caseExprGE(ASTExprGE& host, void* = NULL);
		void caseExprLT(ASTExprLT& host, void* = NULL);
		void caseExprLE(ASTExprLE& host, void* = NULL);
		void caseExprEQ(ASTExprEQ& host, void* = NULL);
		void caseExprNE(ASTExprNE& host, void* = NULL);
		void caseExprAppxEQ(ASTExprAppxEQ& host, void* = NULL);
		void caseExprPlus(ASTExprPlus& host, void* = NULL);
		void caseExprMinus(ASTExprMinus& host, void* = NULL);
		void caseExprTimes(ASTExprTimes& host, void* = NULL);
		void caseExprExpn(ASTExprExpn& host, void* = NULL);
		void caseExprDivide(ASTExprDivide& host, void* = NULL);
		void caseExprModulo(ASTExprModulo& host, void* = NULL);
		void caseExprBitAnd(ASTExprBitAnd& host, void* = NULL);
		void caseExprBitOr(ASTExprBitOr& host, void* = NULL);
		void caseExprBitXor(ASTExprBitXor& host, void* = NULL);
		void caseExprLShift(ASTExprLShift& host, void* = NULL);
		void caseExprRShift(ASTExprRShift& host, void* = NULL);
		void caseExprTernary(ASTTernaryExpr& host, void* = NULL);
		// Literals
		void caseStringLiteral(ASTStringLiteral& host, void* = NULL);
		void caseArrayLiteral(ASTArrayLiteral& host, void* = NULL);
		void caseOptionValue(ASTOptionValue& host, void* = NULL);
		void caseIsIncluded(ASTIsIncluded& host, void* = NULL);
		
		//internals
		virtual void analyzeFunctionInternals(ZScript::Function& function);
	private:
		ZScript::Program& program;
		// Current function return type.
		ZScript::DataType const* returnType;

		std::vector<Function*> inlineStack;
		
		bool deprecateGlobals;

		////////////////////////////////////////////////////////////////
		// Helper Functions.

		void analyzeUnaryExpr(ASTUnaryExpr& host, ZScript::DataType const& type);
		void analyzeIncrement(ASTUnaryExpr& host);
		void analyzeBinaryExpr(
				ASTBinaryExpr& host, ZScript::DataType const& leftType,
				ZScript::DataType const& rightType);

	};
}	
#endif
