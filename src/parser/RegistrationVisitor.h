/**
 * Registration handler, to ensure that anything global can be referenced in any order
 * Begun: 26th June, 2019
 * Author: Emily
 */

#ifndef REGISTRATIONVISITOR_H
#define REGISTRATIONVISITOR_H

#include "ASTVisitors.h"
#include "DataStructs.h"
#include "ZScript.h"

namespace ZScript
{
	class RegistrationVisitor : public RecursiveVisitor
	{
	public:
		RegistrationVisitor(ZScript::Program& program);
		using RecursiveVisitor::visit;
		void visit(AST& node, void* param = NULL);
		////////////////
		// Cases
		void caseDefault(AST& host, void* param = NULL);
		void caseRoot(ASTFile& host, void* param = NULL);
		void caseFile(ASTFile& host, void* param = NULL);
		void caseSetOption(ASTSetOption& host, void* param = NULL);
		// Declarations
		void caseScript(ASTScript& host, void* param = NULL);
		void caseClass(ASTClass& host, void* param = NULL);
		void caseNamespace(ASTNamespace& host, void* param = NULL);
		void caseImportDecl(ASTImportDecl& host, void* param = NULL);
		void caseIncludePath(ASTIncludePath& host, void* param = NULL);
		void caseImportCondDecl(ASTImportCondDecl& host, void* param = NULL);
		void caseUsing(ASTUsingDecl& host, void* param = NULL);
		void caseDataTypeDef(ASTDataTypeDef& host, void* param = NULL);
		void caseCustomDataTypeDef(ASTCustomDataTypeDef& host, void* param = NULL);
		void caseScriptTypeDef(ASTScriptTypeDef& host, void* param = NULL);
		void caseDataDeclList(ASTDataDeclList& host, void* param = NULL);
		void caseDataEnum(ASTDataEnum& host, void* param = NULL);
		void caseDataDecl(ASTDataDecl& host, void* param = NULL);
		void caseDataDeclExtraArray(ASTDataDeclExtraArray& host, void* param = NULL);
		void caseFuncDecl(ASTFuncDecl& host, void* param = NULL);
		// Expressions -- Needed for constant evaluation
		void caseExprConst(ASTExprConst& host, void* param = NULL);
		void caseVarInitializer(ASTExprVarInitializer& host, void* param = NULL);
		void caseExprAssign(ASTExprAssign& host, void* param = NULL);
		void caseExprIdentifier(ASTExprIdentifier& host, void* param = NULL);
		void caseExprArrow(ASTExprArrow& host, void* param = NULL);
		void caseExprIndex(ASTExprIndex& host, void* param = NULL);
		void caseExprCall(ASTExprCall& host, void* param = NULL);
		void caseExprNegate(ASTExprNegate& host, void* param = NULL);
		void caseExprDelete(ASTExprDelete& host, void* param = NULL);
		void caseExprNot(ASTExprNot& host, void* param = NULL);
		void caseExprBitNot(ASTExprBitNot& host, void* param = NULL);
		void caseExprIncrement(ASTExprIncrement& host, void* param = NULL);
		void caseExprDecrement(ASTExprDecrement& host, void* param = NULL);
		void caseExprCast(ASTExprCast& host, void* param = NULL);
		void caseExprAnd(ASTExprAnd& host, void* param = NULL);
		void caseExprOr(ASTExprOr& host, void* param = NULL);
		void caseExprGT(ASTExprGT& host, void* param = NULL);
		void caseExprGE(ASTExprGE& host, void* param = NULL);
		void caseExprLT(ASTExprLT& host, void* param = NULL);
		void caseExprLE(ASTExprLE& host, void* param = NULL);
		void caseExprEQ(ASTExprEQ& host, void* param = NULL);
		void caseExprNE(ASTExprNE& host, void* param = NULL);
		void caseExprAppxEQ(ASTExprAppxEQ& host, void* param = NULL);
		void caseExprPlus(ASTExprPlus& host, void* param = NULL);
		void caseExprMinus(ASTExprMinus& host, void* param = NULL);
		void caseExprTimes(ASTExprTimes& host, void* param = NULL);
		void caseExprExpn(ASTExprExpn& host, void* param = NULL);
		void caseExprDivide(ASTExprDivide& host, void* param = NULL);
		void caseExprModulo(ASTExprModulo& host, void* param = NULL);
		void caseExprBitAnd(ASTExprBitAnd& host, void* param = NULL);
		void caseExprBitOr(ASTExprBitOr& host, void* param = NULL);
		void caseExprBitXor(ASTExprBitXor& host, void* param = NULL);
		void caseExprLShift(ASTExprLShift& host, void* param = NULL);
		void caseExprRShift(ASTExprRShift& host, void* param = NULL);
		void caseExprTernary(ASTTernaryExpr& host, void* param = NULL);
		//Types
		void caseScriptType(ASTScriptType& host, void* param = NULL);
		void caseDataType(ASTDataType& host, void* param = NULL);
		//Literals
		void caseNumberLiteral(ASTNumberLiteral& host, void* param = NULL){
			caseDefault(host,param);}
		void caseCharLiteral(ASTCharLiteral& host, void* param = NULL){
			caseDefault(host,param);}
		void caseArrayLiteral(ASTArrayLiteral& host, void* param = NULL);
		void caseStringLiteral(ASTStringLiteral& host, void* param = NULL);
	protected:
		
		void doRegister(AST& host)
		{
			host.mark_registered();
			hasChanged = true;
		}
		////////////////////////////////////////////////////////////////
		// Convenience Functions
		// Quickly checks if a node, or container of nodes, is all registered
		bool registered(AST& node) const;
		//Shortcut for pointer
		bool registered(AST* node) const;
		template <class Container>
		bool registered(AST& host, Container const& nodes) const;
		//Visiting
		template <class Container>
		void block_regvisit(AST& host, Container const& nodes, void* param = NULL);
	private:
		ZScript::Program& program;
		
		void analyzeUnaryExpr(ASTUnaryExpr& host);
		void analyzeBinaryExpr(ASTBinaryExpr& host);
		
		bool hasChanged;
	};
}

#endif

