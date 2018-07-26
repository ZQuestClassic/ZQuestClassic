#include "../precompiled.h" // Always first.
#include "ASTVisitors.h"

#include "../zsyssimple.h"
#include "CompileError.h"
#include <assert.h>
#include <cstdarg>

////////////////////////////////////////////////////////////////
// RecursiveVisitor

bool RecursiveVisitor::breakRecursion(AST& host, void* param) const
{
	if (failure) return true;
	if (currentCompileError) return true;
	return false;
}

void RecursiveVisitor::handleError(
		CompileError const& error, AST const* node, ...)
{
	// Scan through the error handlers backwards to see if we can find
	// one matching this error.
	for (vector<ASTStmtCompileError*>::reverse_iterator it
			 = compileErrorHandlers.rbegin();
		 it != compileErrorHandlers.rend(); ++it)
	{
		// When we find a valid handler, save the error as the current one and
		// return.
		if ((*it)->canHandle(error))
		{
			currentCompileError = &error;
			return;
		}
	}

	// If there was no handler, fail if it's not a warning and then print it
	// out.
	va_list args;
	va_start(args, node);
	error.vprint(node, args);
	if (!error.warning) failure = true;
	va_end(args);
}

void RecursiveVisitor::visit(AST& node, void* param)
{
	node.execute(*this, param);
}

void RecursiveVisitor::visit(AST* node, void* param)
{
	if (node) visit(*node, param);
}

////////////////////////////////////////////////////////////////
// Cases

void RecursiveVisitor::caseStmtCompileError(ASTStmtCompileError& host, void* param)
{
	// If we've already been triggered, don't run.
	if (host.errorTriggered) return;
	
	// First visit on error id.
	visit(host.errorId, param);
	if (breakRecursion(host, param)) return;

	// Visit on statement with host as a handler.
	compileErrorHandlers.push_back(&host);
	visit(host.statement, param);
	compileErrorHandlers.pop_back();

	// If we can handle the current error, erase it and continue.
	if (currentCompileError && host.canHandle(*currentCompileError))
	{
		currentCompileError = NULL;
		host.errorTriggered = true;
	}
}

void RecursiveVisitor::caseProgram(ASTProgram& host, void* param)
{
	visit(host, host.imports, param);
	if (breakRecursion(host, param)) return;
	visit(host, host.types, param);
	if (breakRecursion(host, param)) return;
	visit(host, host.variables, param);
	if (breakRecursion(host, param)) return;
	visit(host, host.functions, param);
	if (breakRecursion(host, param)) return;
	visit(host, host.scripts, param);
}

// Statements

void RecursiveVisitor::caseBlock(ASTBlock& host, void* param)
{
	visit(host, host.statements, param);
}

void RecursiveVisitor::caseStmtIf(ASTStmtIf& host, void* param)
{
	visit(host.condition, param);
	if (breakRecursion(host, param)) return;
	visit(host.thenStatement, param);
}

void RecursiveVisitor::caseStmtIfElse(ASTStmtIfElse& host, void* param)
{
	caseStmtIf(host, param);
	if (breakRecursion(host, param)) return;
	visit(host.elseStatement, param);
}

void RecursiveVisitor::caseStmtSwitch(ASTStmtSwitch& host, void* param)
{
	visit(host.key, param);
	if (breakRecursion(host, param)) return;
	visit(host, host.cases, param);
}

void RecursiveVisitor::caseSwitchCases(ASTSwitchCases& host, void* param)
{
	visit(host, host.cases, param);
	if (breakRecursion(host, param)) return;
	visit(host.block, param);
}

void RecursiveVisitor::caseStmtFor(ASTStmtFor& host, void* param)
{
	visit(host.setup, param);
	if (breakRecursion(host, param)) return;
	visit(host.test, param);
	if (breakRecursion(host, param)) return;
	visit(host.increment, param);
	if (breakRecursion(host, param)) return;
	visit(host.body, param);
}

void RecursiveVisitor::caseStmtWhile(ASTStmtWhile& host, void* param)
{
	visit(host.test, param);
	if (breakRecursion(host, param)) return;
	visit(host.body, param);
}

void RecursiveVisitor::caseStmtDo(ASTStmtDo& host, void* param)
{
	visit(host.body, param);
	if (breakRecursion(host, param)) return;
	visit(host.test, param);
}

void RecursiveVisitor::caseStmtReturnVal(ASTStmtReturnVal& host, void* param)
{
	visit(host.value, param);
}

// Declarations

void RecursiveVisitor::caseScript(ASTScript& host, void* param)
{
	visit(host.type, param);
	if (breakRecursion(host, param)) return;
	visit(host, host.types, param);
	if (breakRecursion(host, param)) return;
	visit(host, host.variables, param);
	if (breakRecursion(host, param)) return;
	visit(host, host.functions, param);
}

void RecursiveVisitor::caseFuncDecl(ASTFuncDecl& host, void* param)
{
	visit(host.returnType, param);
	if (breakRecursion(host, param)) return;
	visit(host, host.parameters, param);
	if (breakRecursion(host, param)) return;
	visit(host.block, param);
}

void RecursiveVisitor::caseDataDeclList(ASTDataDeclList& host, void* param)
{
	visit(host.baseType, param);
	if (breakRecursion(host, param)) return;
	visit(host, host.declarations(), param);
}

void RecursiveVisitor::caseDataDecl(ASTDataDecl& host, void* param)
{
	visit(host.baseType, param);
	if (breakRecursion(host, param)) return;
	visit(host, host.extraArrays, param);
	if (breakRecursion(host, param)) return;
	visit(host.initializer(), param);
}

void RecursiveVisitor::caseDataDeclExtraArray(
		ASTDataDeclExtraArray& host, void* param)
{
	visit(host, host.dimensions, param);
}

void RecursiveVisitor::caseTypeDef(ASTTypeDef& host, void* param)
{
	visit(host.type, param);
}

// Expressions

void RecursiveVisitor::caseExprConst(ASTExprConst& host, void* param)
{
	visit(host.content, param);
}

void RecursiveVisitor::caseExprAssign(ASTExprAssign& host, void* param)
{
	visit(host.left, param);
	if (breakRecursion(host, param)) return;
	visit(host.right, param);
}

void RecursiveVisitor::caseExprArrow(ASTExprArrow& host, void* param)
{
	visit(host.left, param);
	if (breakRecursion(host, param)) return;
	visit(host.index, param);
}

void RecursiveVisitor::caseExprIndex(ASTExprIndex& host, void* param)
{
	visit(host.array, param);
	if (breakRecursion(host, param)) return;
	visit(host.index, param);
}

void RecursiveVisitor::caseExprCall(ASTExprCall& host, void* param)
{
	//visit(host.left, param);
	//if (breakRecursion(host, param)) return;
	visit(host, host.parameters, param);
}

void RecursiveVisitor::caseExprNegate(ASTExprNegate& host, void* param)
{
	visit(host.operand, param);
}

void RecursiveVisitor::caseExprNot(ASTExprNot& host, void* param)
{
	visit(host.operand, param);
}

void RecursiveVisitor::caseExprBitNot(ASTExprBitNot& host, void* param)
{
	visit(host.operand, param);
}

void RecursiveVisitor::caseExprIncrement(ASTExprIncrement& host, void* param)
{
	visit(host.operand, param);
}

void RecursiveVisitor::caseExprPreIncrement(
		ASTExprPreIncrement& host, void* param)
{
	visit(host.operand, param);
}

void RecursiveVisitor::caseExprDecrement(ASTExprDecrement& host, void* param)
{
	visit(host.operand, param);
}

void RecursiveVisitor::caseExprPreDecrement(
		ASTExprPreDecrement& host, void* param)
{
	visit(host.operand, param);
}

void RecursiveVisitor::caseExprAnd(ASTExprAnd& host, void* param)
{
	visit(host.left, param);
	if (breakRecursion(host, param)) return;
	visit(host.right, param);
}

void RecursiveVisitor::caseExprOr(ASTExprOr& host, void* param)
{
	visit(host.left, param);
	if (breakRecursion(host, param)) return;
	visit(host.right, param);
}

void RecursiveVisitor::caseExprGT(ASTExprGT& host, void* param)
{
	visit(host.left, param);
	if (breakRecursion(host, param)) return;
	visit(host.right, param);
}

void RecursiveVisitor::caseExprGE(ASTExprGE& host, void* param)
{
	visit(host.left, param);
	if (breakRecursion(host, param)) return;
	visit(host.right, param);
}

void RecursiveVisitor::caseExprLT(ASTExprLT& host, void* param)
{
	visit(host.left, param);
	if (breakRecursion(host, param)) return;
	visit(host.right, param);
}

void RecursiveVisitor::caseExprLE(ASTExprLE& host, void* param)
{
	visit(host.left, param);
	if (breakRecursion(host, param)) return;
	visit(host.right, param);
}

void RecursiveVisitor::caseExprEQ(ASTExprEQ& host, void* param)
{
	visit(host.left, param);
	if (breakRecursion(host, param)) return;
	visit(host.right, param);
}

void RecursiveVisitor::caseExprNE(ASTExprNE& host, void* param)
{
	visit(host.left, param);
	if (breakRecursion(host, param)) return;
	visit(host.right, param);
}

void RecursiveVisitor::caseExprPlus(ASTExprPlus& host, void* param)
{
	visit(host.left, param);
	if (breakRecursion(host, param)) return;
	visit(host.right, param);
}

void RecursiveVisitor::caseExprMinus(ASTExprMinus& host, void* param)
{
	visit(host.left, param);
	if (breakRecursion(host, param)) return;
	visit(host.right, param);
}

void RecursiveVisitor::caseExprTimes(ASTExprTimes& host, void* param)
{
	visit(host.left, param);
	if (breakRecursion(host, param)) return;
	visit(host.right, param);
}

void RecursiveVisitor::caseExprDivide(ASTExprDivide& host, void* param)
{
	visit(host.left, param);
	if (breakRecursion(host, param)) return;
	visit(host.right, param);
}

void RecursiveVisitor::caseExprModulo(ASTExprModulo& host, void* param)
{
	visit(host.left, param);
	if (breakRecursion(host, param)) return;
	visit(host.right, param);
}

void RecursiveVisitor::caseExprBitAnd(ASTExprBitAnd& host, void* param)
{
	visit(host.left, param);
	if (breakRecursion(host, param)) return;
	visit(host.right, param);
}

void RecursiveVisitor::caseExprBitOr(ASTExprBitOr& host, void* param)
{
	visit(host.left, param);
	if (breakRecursion(host, param)) return;
	visit(host.right, param);
}

void RecursiveVisitor::caseExprBitXor(ASTExprBitXor& host, void* param)
{
	visit(host.left, param);
	if (breakRecursion(host, param)) return;
	visit(host.right, param);
}

void RecursiveVisitor::caseExprLShift(ASTExprLShift& host, void* param)
{
	visit(host.left, param);
	if (breakRecursion(host, param)) return;
	visit(host.right, param);
}

void RecursiveVisitor::caseExprRShift(ASTExprRShift& host, void* param)
{
	visit(host.left, param);
	if (breakRecursion(host, param)) return;
	visit(host.right, param);
}

// Literals

void RecursiveVisitor::caseNumberLiteral(ASTNumberLiteral& host, void* param)
{
	visit(host.value, param);
}

void RecursiveVisitor::caseArrayLiteral(ASTArrayLiteral& host, void* param)
{
	visit(host.type, param);
	if (breakRecursion(host, param)) return;
	visit(host.size, param);
	if (breakRecursion(host, param)) return;
	visit(host, host.elements, param);
}
