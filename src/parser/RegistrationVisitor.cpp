/**
 * Registration handler, to ensure that anything global can be referenced in any order
 * Begun: 26th June, 2019
 * Author: Venrob
 */

#include "RegistrationVisitor.h"
#include <cassert>
#include "Scope.h"
#include "CompileError.h"

using std::string;
using std::vector;
using namespace ZScript;

////////////////////////////////////////////////////////////////
// RegistrationVisitor

RegistrationVisitor::RegistrationVisitor(Program& program)
	: program(program)
{
	scope = &program.getScope();
	caseRoot(program.getRoot());
	assert(dynamic_cast<RootScope*>(scope)); //Ensure no scope corruption
}

void RegistrationVisitor::visit(AST& node, void* param)
{
	if(node.isDisabled()) return; //Don't visit disabled nodes.
	if(registered(node)) return; //Don't double-register
	RecursiveVisitor::visit(node, param);
}

void RegistrationVisitor::caseDefault(AST& host, void* param)
{
	assert(false); //This should never be reached.
}

//Handle the root file specially!
void RegistrationVisitor::caseRoot(ASTFile& host, void* param)
{
	int recursionLimit = 50;
	while(--recursionLimit)
	{
		caseFile(host, param);
		if(registered(host)) return;
	}
	//Failed recursionLimit
	//VENROBTODO Compile Warning Here! SemanticAnalyzer should error, somewhere, as well.
}

void RegistrationVisitor::caseFile(ASTFile& host, void* param)
{
	//VENROBTODO do stuff here!
}

void RegistrationVisitor::caseSetOption(ASTSetOption& host, void* param)
{
	//VENROBTODO do stuff here!
}

void RegistrationVisitor::caseUsing(ASTUsingDecl& host, void* param)
{
	//VENROBTODO do stuff here!
}

// Declarations
void RegistrationVisitor::caseDataTypeDef(ASTDataTypeDef& host, void* param)
{
	//VENROBTODO do stuff here!
}

void RegistrationVisitor::caseCustomDataTypeDef(ASTCustomDataTypeDef& host, void* param)
{
	//VENROBTODO do stuff here!
}

void RegistrationVisitor::caseScriptTypeDef(ASTScriptTypeDef& host, void* param)
{
	//VENROBTODO do stuff here!
}

void RegistrationVisitor::caseDataDeclList(ASTDataDeclList& host, void* param)
{
	//VENROBTODO do stuff here!
}

void RegistrationVisitor::caseDataEnum(ASTDataEnum& host, void* param)
{
	//VENROBTODO do stuff here!
}

void RegistrationVisitor::caseDataDecl(ASTDataDecl& host, void* param)
{
	//VENROBTODO do stuff here!
}

void RegistrationVisitor::caseDataDeclExtraArray(ASTDataDeclExtraArray& host, void* param)
{
	//VENROBTODO do stuff here!
}

void RegistrationVisitor::caseFuncDecl(ASTFuncDecl& host, void* param)
{
	//VENROBTODO do stuff here!
}

void RegistrationVisitor::caseScript(ASTScript& host, void* param)
{
	//VENROBTODO do stuff here!
}

void RegistrationVisitor::caseNamespace(ASTNamespace& host, void* param)
{
	//VENROBTODO do stuff here!
}

void RegistrationVisitor::caseImportDecl(ASTImportDecl& host, void* param)
{
	//VENROBTODO do stuff here!
}

// Expressions -- Needed for constant evaluation
void RegistrationVisitor::caseExprConst(ASTExprConst& host, void* param)
{
	//VENROBTODO do stuff here!
}

void RegistrationVisitor::caseExprAssign(ASTExprAssign& host, void* param)
{
	//VENROBTODO do stuff here!
}

void RegistrationVisitor::caseExprIdentifier(ASTExprIdentifier& host, void* param)
{
	//VENROBTODO do stuff here!
}

void RegistrationVisitor::caseExprArrow(ASTExprArrow& host, void* param)
{
	//VENROBTODO do stuff here!
}

void RegistrationVisitor::caseExprIndex(ASTExprIndex& host, void* param)
{
	//VENROBTODO do stuff here!
}

void RegistrationVisitor::caseExprCall(ASTExprCall& host, void* param)
{
	//VENROBTODO Error here. Calls should not be found in global initializers.
}

void RegistrationVisitor::caseExprNegate(ASTExprNegate& host, void* param)
{
	analyzeUnaryExpr(host);
}

void RegistrationVisitor::caseExprNot(ASTExprNot& host, void* param)
{
	analyzeUnaryExpr(host);
}

void RegistrationVisitor::caseExprBitNot(ASTExprBitNot& host, void* param)
{
	analyzeUnaryExpr(host);
}

void RegistrationVisitor::caseExprIncrement(ASTExprIncrement& host, void* param)
{
	analyzeUnaryExpr(host);
}

void RegistrationVisitor::caseExprPreIncrement(ASTExprPreIncrement& host, void* param)
{
	analyzeUnaryExpr(host);
}

void RegistrationVisitor::caseExprDecrement(ASTExprDecrement& host, void* param)
{
	analyzeUnaryExpr(host);
}

void RegistrationVisitor::caseExprPreDecrement(ASTExprPreDecrement& host, void* param)
{
	analyzeUnaryExpr(host);
}

void RegistrationVisitor::caseExprAnd(ASTExprAnd& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprOr(ASTExprOr& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprGT(ASTExprGT& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprGE(ASTExprGE& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprLT(ASTExprLT& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprLE(ASTExprLE& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprEQ(ASTExprEQ& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprNE(ASTExprNE& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprPlus(ASTExprPlus& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprMinus(ASTExprMinus& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprTimes(ASTExprTimes& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprDivide(ASTExprDivide& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprModulo(ASTExprModulo& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprBitAnd(ASTExprBitAnd& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprBitOr(ASTExprBitOr& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprBitXor(ASTExprBitXor& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprLShift(ASTExprLShift& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprRShift(ASTExprRShift& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprTernary(ASTTernaryExpr& host, void* param)
{
	visit(host.left.get());
	syncDisable(host, *host.left);
	if (breakRecursion(host)) return;
	visit(host.middle.get());
	syncDisable(host, *host.middle);
	if (breakRecursion(host)) return;
	visit(host.right.get());
	syncDisable(host, *host.right);
	if (breakRecursion(host)) return;
	if(registered(*host.left) && registered(*host.middle) && registered(*host.right)) host.Register();
}

//Helper Functions
void RegistrationVisitor::analyzeUnaryExpr(ASTUnaryExpr& host)
{
	visit(host.operand.get());
	syncDisable(host, *host.operand);
	if (breakRecursion(host)) return;
	if(registered(*host.operand))host.Register();
}

void RegistrationVisitor::analyzeBinaryExpr(ASTBinaryExpr& host)
{
	visit(host.left.get());
	syncDisable(host, *host.left);
	if (breakRecursion(host)) return;
	visit(host.right.get());
	syncDisable(host, *host.right);
	if (breakRecursion(host)) return;
	if(registered(*host.left) && registered(*host.right)) host.Register();
}


