/**
 * Registration handler, to ensure that anything global can be referenced in any order
 * Begun: 26th June, 2019
 * Author: Venrob
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
		RegistrationVisitor(ZScript::Program& program);
		////////////////
		// Cases
		void caseDefault(AST& host, void* param = NULL);
		void caseRoot(ASTFile& host, void* = NULL);
		void caseFile(ASTFile& host, void* = NULL);
		void caseSetOption(ASTSetOption& host, void* param = NULL);
		void caseUsing(ASTUsingDecl& host, void* param = NULL);
		// Statements
		//Ommitted. Should never be visited by this, as statements occur only in functions.
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
		void caseNamespace(ASTNamespace& host, void* = NULL);
		void caseImportDecl(ASTImportDecl& host, void* = NULL);
		// Expressions -- Needed for constant evaluation
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
		void caseExprTernary(ASTTernaryExpr& host, void* = NULL);
		// Literals
		//Ommitted. Variables with literal initializers cannot be global constants at this time, so no need during registration.
	protected:
		
		////////////////////////////////////////////////////////////////
		// Convenience Functions
		// Quickly checks if a node, or container of nodes, is all registered
		bool registered(AST& node, void* param = NULL) const
		{
			return node.registered();
		}
		template <class Container>
		bool registered(Container const& nodes, void* param = NULL) const
		{
			for(typename Container::const_iterator it = nodes.begin();
				it != nodes.end(); ++it)
			{
				if(!registered(**it, param)) return false;
			}
			return true;
		}
	private:
		ZScript::Program& program;
	}
}

#endif

