//2.53 Updated to 16th Jan, 2017
#include "../precompiled.h" //always first
#include "UtilVisitors.h"

#include "../zsyssimple.h"
#include "CompileError.h"
#include <assert.h>
#include <cstdarg>

////////////////////////////////////////////////////////////////
// RecursiveVisitor

bool RecursiveVisitor::breakRecursion(AST& host) const
{
	if (failure) return true;
	if (currentCompileError) return true;
	return false;
}

void RecursiveVisitor::compileError(AST& host, CompileError const* error, ...)
{
	// Scan through the error handlers backwards to see if we can find
	// one matching this error.
	for (vector<ASTCompileError*>::reverse_iterator it
			 = compileErrorHandlers.rbegin();
		 it != compileErrorHandlers.rend(); ++it)
	{
		// When we find a valid handler, save the error as the current one and
		// return.
		if ((*it)->canHandle(*error))
	{
			currentCompileError = error;
			return;
		}
	}

	// If there was no handler, fail if it's not a warning and then print it
	// out.
	va_list args;
	va_start(args, *error);
	error->vprint(&host, args);
	va_end(args);
}

void RecursiveVisitor::recurse(AST& node, void* param)
{
	node.execute(*this, param);
}

void RecursiveVisitor::recurse(AST* node, void* param)
{
	if (node) node->execute(*this, param);
}

template <class Node> void RecursiveVisitor::recurse(
		AST& host, void* param, vector<Node*> nodes)
{
	for (typename vector<Node*>::const_iterator it = nodes.begin();
		 it != nodes.end(); ++it)
	{
		(*it)->execute(*this, param);
		if (breakRecursion(host)) return;
	}
}

template <class Node> void RecursiveVisitor::recurse(
		AST& host, void* param, list<Node*> nodes)
{
	for (typename list<Node*>::const_iterator it = nodes.begin();
		 it != nodes.end(); ++it)
	{
		(*it)->execute(*this, param);
		if (breakRecursion(host)) return;
	}
}

////

void RecursiveVisitor::caseCompileError(ASTCompileError& host, void* param)
{
	// If we've already been triggered, don't run.
	if (host.errorTriggered) return;

	// First recurse on error id.
	recurse(host.errorId, param);
	if (breakRecursion(host)) return;

	// Recurse on statement with host as a handler.
	compileErrorHandlers.push_back(&host);
	recurse(host.statement, param);
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
	recurse(host, param, host.imports);
	if (breakRecursion(host)) return;
	recurse(host, param, host.types);
	if (breakRecursion(host)) return;
	recurse(host, param, host.variables);
	if (breakRecursion(host)) return;
	recurse(host, param, host.functions);
	if (breakRecursion(host)) return;
	recurse(host, param, host.scripts);
}

// Statements

void RecursiveVisitor::caseBlock(ASTBlock& host, void* param)
{
	recurse(host, param, host.statements);
}

void RecursiveVisitor::caseStmtIf(ASTStmtIf& host, void* param)
{
	recurse(host.condition, param);
	if (breakRecursion(host)) return;
	recurse(host.thenStatement, param);
}

void RecursiveVisitor::caseStmtIfElse(ASTStmtIfElse& host, void* param)
{
	caseStmtIf(host, param);
	if (breakRecursion(host)) return;
	recurse(host.elseStatement, param);
}

void RecursiveVisitor::caseStmtSwitch(ASTStmtSwitch& host, void* param)
{
	recurse(host.key, param);
	if (breakRecursion(host)) return;
	recurse(host, param, host.cases);
}

void RecursiveVisitor::caseSwitchCases(ASTSwitchCases& host, void* param)
{
	recurse(host, param, host.cases);
	if (breakRecursion(host)) return;
	recurse(host.block, param);
}

void RecursiveVisitor::caseStmtFor(ASTStmtFor& host, void* param)
{
	recurse(host.setup, param);
	if (breakRecursion(host)) return;
	recurse(host.test, param);
	if (breakRecursion(host)) return;
	recurse(host.increment, param);
	if (breakRecursion(host)) return;
	recurse(host.body, param);
}

void RecursiveVisitor::caseStmtWhile(ASTStmtWhile& host, void* param)
{
	recurse(host.test, param);
	if (breakRecursion(host)) return;
	recurse(host.body, param);
}

void RecursiveVisitor::caseStmtDo(ASTStmtDo& host, void* param)
{
	recurse(host.body, param);
	if (breakRecursion(host)) return;
	recurse(host.test, param);
}

void RecursiveVisitor::caseStmtReturnVal(ASTStmtReturnVal& host, void* param)
{
	recurse(host.value, param);
}

// Declarations

void RecursiveVisitor::caseScript(ASTScript& host, void* param)
{
	recurse(host.type, param);
	if (breakRecursion(host)) return;
	recurse(host, param, host.types);
	if (breakRecursion(host)) return;
	recurse(host, param, host.variables);
	if (breakRecursion(host)) return;
	recurse(host, param, host.functions);
}

void RecursiveVisitor::caseFuncDecl(ASTFuncDecl& host, void* param)
{
	recurse(host.returnType, param);
	if (breakRecursion(host)) return;
	recurse(host, param, host.parameters);
	if (breakRecursion(host)) return;
	recurse(host.block, param);
}

void RecursiveVisitor::caseDataDeclList(ASTDataDeclList& host, void* param)
{
	recurse(host.baseType, param);
	if (breakRecursion(host)) return;
	recurse(host, param, host.declarations());
}

void RecursiveVisitor::caseDataDecl(ASTDataDecl& host, void* param)
{
	recurse(host.baseType, param);
	if (breakRecursion(host)) return;
	recurse(host, param, host.extraArrays);
	if (breakRecursion(host)) return;
	recurse(host.initializer(), param);
}

void RecursiveVisitor::caseDataDeclExtraArray(
		ASTDataDeclExtraArray& host, void* param)
{
	recurse(host, param, host.dimensions);
}

void RecursiveVisitor::caseTypeDef(ASTTypeDef& host, void* param)
{
	recurse(host.type, param);
}

// Expressions

void RecursiveVisitor::caseExprConst(ASTExprConst& host, void* param)
{
	recurse(host.content, param);
}

void RecursiveVisitor::caseExprAssign(ASTExprAssign& host, void* param)
{
	recurse(host.left, param);
	if (breakRecursion(host)) return;
	recurse(host.right, param);
}

void RecursiveVisitor::caseExprArrow(ASTExprArrow& host, void* param)
{
	recurse(host.left, param);
	if (breakRecursion(host)) return;
	recurse(host.index, param);
}

void RecursiveVisitor::caseExprIndex(ASTExprIndex& host, void* param)
{
	recurse(host.array, param);
	if (breakRecursion(host)) return;
	recurse(host.index, param);
}

void RecursiveVisitor::caseExprCall(ASTExprCall& host, void* param)
{
	//recurse(host.left, param);
	//if (breakRecursion(host)) return;
	recurse(host, param, host.parameters);
}

void RecursiveVisitor::caseExprNegate(ASTExprNegate& host, void* param)
{
	recurse(host.operand, param);
}

void RecursiveVisitor::caseExprNot(ASTExprNot& host, void* param)
{
	recurse(host.operand, param);
}

void RecursiveVisitor::caseExprBitNot(ASTExprBitNot& host, void* param)
{
	recurse(host.operand, param);
}

void RecursiveVisitor::caseExprIncrement(ASTExprIncrement& host, void* param)
{
	recurse(host.operand, param);
}

void RecursiveVisitor::caseExprPreIncrement(
		ASTExprPreIncrement& host, void* param)
{
	recurse(host.operand, param);
}

void RecursiveVisitor::caseExprDecrement(ASTExprDecrement& host, void* param)
{
	recurse(host.operand, param);
}

void RecursiveVisitor::caseExprPreDecrement(
		ASTExprPreDecrement& host, void* param)
{
	recurse(host.operand, param);
}

void RecursiveVisitor::caseExprAnd(ASTExprAnd& host, void* param)
{
	recurse(host.left, param);
	if (breakRecursion(host)) return;
	recurse(host.right, param);
}

void RecursiveVisitor::caseExprOr(ASTExprOr& host, void* param)
{
	recurse(host.left, param);
	if (breakRecursion(host)) return;
	recurse(host.right, param);
}

void RecursiveVisitor::caseExprGT(ASTExprGT& host, void* param)
{
	recurse(host.left, param);
	if (breakRecursion(host)) return;
	recurse(host.right, param);
}

void RecursiveVisitor::caseExprGE(ASTExprGE& host, void* param)
{
	recurse(host.left, param);
	if (breakRecursion(host)) return;
	recurse(host.right, param);
}

void RecursiveVisitor::caseExprLT(ASTExprLT& host, void* param)
{
	recurse(host.left, param);
	if (breakRecursion(host)) return;
	recurse(host.right, param);
}

void RecursiveVisitor::caseExprLE(ASTExprLE& host, void* param)
{
	recurse(host.left, param);
	if (breakRecursion(host)) return;
	recurse(host.right, param);
}

void RecursiveVisitor::caseExprEQ(ASTExprEQ& host, void* param)
{
	recurse(host.left, param);
	if (breakRecursion(host)) return;
	recurse(host.right, param);
}

void RecursiveVisitor::caseExprNE(ASTExprNE& host, void* param)
{
	recurse(host.left, param);
	if (breakRecursion(host)) return;
	recurse(host.right, param);
}

void RecursiveVisitor::caseExprPlus(ASTExprPlus& host, void* param)
{
	recurse(host.left, param);
	if (breakRecursion(host)) return;
	recurse(host.right, param);
}

void RecursiveVisitor::caseExprMinus(ASTExprMinus& host, void* param)
{
	recurse(host.left, param);
	if (breakRecursion(host)) return;
	recurse(host.right, param);
}

void RecursiveVisitor::caseExprTimes(ASTExprTimes& host, void* param)
{
	recurse(host.left, param);
	if (breakRecursion(host)) return;
	recurse(host.right, param);
}

void RecursiveVisitor::caseExprDivide(ASTExprDivide& host, void* param)
{
	recurse(host.left, param);
	if (breakRecursion(host)) return;
	recurse(host.right, param);
}

void RecursiveVisitor::caseExprModulo(ASTExprModulo& host, void* param)
{
	recurse(host.left, param);
	if (breakRecursion(host)) return;
	recurse(host.right, param);
}

void RecursiveVisitor::caseExprBitAnd(ASTExprBitAnd& host, void* param)
{
	recurse(host.left, param);
	if (breakRecursion(host)) return;
	recurse(host.right, param);
}

void RecursiveVisitor::caseExprBitOr(ASTExprBitOr& host, void* param)
{
	recurse(host.left, param);
	if (breakRecursion(host)) return;
	recurse(host.right, param);
}

void RecursiveVisitor::caseExprBitXor(ASTExprBitXor& host, void* param)
{
	recurse(host.left, param);
	if (breakRecursion(host)) return;
	recurse(host.right, param);
}

void RecursiveVisitor::caseExprLShift(ASTExprLShift& host, void* param)
{
	recurse(host.left, param);
	if (breakRecursion(host)) return;
	recurse(host.right, param);
}

void RecursiveVisitor::caseExprRShift(ASTExprRShift& host, void* param)
{
	recurse(host.left, param);
	if (breakRecursion(host)) return;
	recurse(host.right, param);
}

// Literals

void RecursiveVisitor::caseNumberLiteral(ASTNumberLiteral& host, void* param)
{
	recurse(host.value, param);
}

void RecursiveVisitor::caseArrayLiteral(ASTArrayLiteral& host, void* param)
{
	recurse(host.type, param);
	if (breakRecursion(host)) return;
	recurse(host.size, param);
	if (breakRecursion(host)) return;
	recurse(host, param, host.elements);
}

////////////////////////////////////////////////////////////////////////////////
// CheckForExtraneousImports

void CheckForExtraneousImports::caseDefault(void *) {}

void CheckForExtraneousImports::caseImportDecl(ASTImportDecl &host, void *)
{
    ok = false;
	CompileError::ImportBadScope.print(&host);
}

