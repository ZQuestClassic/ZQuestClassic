#include "../precompiled.h" // Always first.
#include "ASTVisitors.h"

#include "../zsyssimple.h"
#include "CompileError.h"
#include "Scope.h"
#include "parserDefs.h"
#include <assert.h>
#include <cstdarg>

using std::list;
using std::vector;
using namespace ZScript;

struct tag {};
void* const RecursiveVisitor::paramNone = new tag();
void* const RecursiveVisitor::paramRead = new tag();
void* const RecursiveVisitor::paramWrite = new tag();
void* const RecursiveVisitor::paramReadWrite = new tag();

uint32_t zscript_failcode = 0;
bool zscript_had_warn_err = false;

////////////////////////////////////////////////////////////////
// RecursiveVisitor

bool RecursiveVisitor::breakRecursion(AST& host, void* param) const
{
	return host.errorDisabled || failure_temp || failure_halt || breakNode;
}

bool RecursiveVisitor::breakRecursion(void* param) const
{
	return failure_temp || failure_halt || breakNode;
}

void RecursiveVisitor::handleError(CompileError const& error)
{
	bool hard_error = (scope && *ZScript::lookupOption(*scope, CompileOption::OPT_NO_ERROR_HALT) == 0);
	// Scan through the node stack looking for a handler.
	for (vector<AST*>::const_reverse_iterator it = recursionStack.rbegin();
		 it != recursionStack.rend(); ++it)
	{
		AST& ancestor = **it;
		for (vector<ASTExprConst*>::iterator it =
				 ancestor.compileErrorCatches.begin();
			 it != ancestor.compileErrorCatches.end(); ++it)
		{
			ASTExprConst& idNode = **it;
			optional<int32_t> errorId = idNode.getCompileTimeValue(this, scope);
			assert(errorId);
			// If we've found a handler, remove that handler from the node's
			// list of handlers and disable the current node (if not a
			// warning).
			if (*errorId == *error.getId() * 10000L)
			{
				ancestor.compileErrorCatches.erase(it);
				if (error.isStrict())
				{
					ancestor.errorDisabled = true;
					breakNode = &ancestor;
				}
				return;
			}
		}
	}

	// Actually handle the error.
	std::string err_str = error.toString();
	char const* err_str_ptr = err_str.c_str();
	while(err_str_ptr[0]==' '||err_str_ptr[0]=='\r'||err_str_ptr[0]=='\n') ++err_str_ptr;
	
	zscript_had_warn_err = true;
	
	if (error.isStrict())
	{
		if(hard_error) failure_halt = true;
		failure = true;
		failure_temp = true;
		if(!zscript_failcode)
			zscript_failcode = *error.getId();
		zconsole_error("%s",err_str_ptr);
	}
	else zconsole_warn("%s",err_str_ptr);
	log_error(error);
}

void RecursiveVisitor::visit(AST& node, void* param)
{
	if(node.isDisabled()) return; //Don't visit disabled nodes.
	if (breakRecursion(node, param)) return;
	recursionStack.push_back(&node);
	node.execute(*this, param);
	recursionStack.pop_back();
	if (breakNode == &node) breakNode = NULL;
}

void RecursiveVisitor::visit(AST* node, void* param)
{
	if (node) visit(*node, param);
}

void RecursiveVisitor::checkCast(
		DataType const& sourceType, DataType const& targetType, AST* node, bool twoWay)
{
	if (sourceType.canCastTo(targetType)) return;
	if (twoWay && targetType.canCastTo(sourceType)) return;
	handleError(
		CompileError::IllegalCast(
			node, sourceType.getName(), targetType.getName()));
}

////////////////////////////////////////////////////////////////
// Cases

void RecursiveVisitor::caseFile(ASTFile& host, void* param)
{
	block_visit(host, host.options, param);
	if (breakRecursion(host, param)) return;
	block_visit(host, host.use, param);
	if (breakRecursion(host, param)) return;
	block_visit(host, host.dataTypes, param);
	if (breakRecursion(host, param)) return;
	block_visit(host, host.scriptTypes, param);
	if (breakRecursion(host, param)) return;
	block_visit(host, host.imports, param);
	if (breakRecursion(host, param)) return;
	block_visit(host, host.condimports, param);
	if (breakRecursion(host, param)) return;
	block_visit(host, host.variables, param);
	if (breakRecursion(host, param)) return;
	block_visit(host, host.functions, param);
	if (breakRecursion(host, param)) return;
	block_visit(host, host.namespaces, param);
	if (breakRecursion(host, param)) return;
	block_visit(host, host.scripts, param);
	if (breakRecursion(host, param)) return;
	block_visit(host, host.asserts, param);
}

void RecursiveVisitor::caseSetOption(ASTSetOption& host, void* param)
{
	visit(host.expr.get(), param);
}

// Statements

void RecursiveVisitor::caseBlock(ASTBlock& host, void* param)
{
	block_visit(host, host.options, param);
	if (breakRecursion(host, param)) return;
	block_visit(host, host.statements, param);
}

void RecursiveVisitor::caseStmtIf(ASTStmtIf& host, void* param)
{
	if(host.isDecl())
		visit(host.declaration.get(), param);
	visit(host.condition.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.thenStatement.get(), param);
}

void RecursiveVisitor::caseStmtIfElse(ASTStmtIfElse& host, void* param)
{
	caseStmtIf(host, param);
	if (breakRecursion(host, param)) return;
	visit(host.elseStatement.get(), param);
}

void RecursiveVisitor::caseStmtSwitch(ASTStmtSwitch& host, void* param)
{
	visit(host.key.get(), param);
	if (breakRecursion(host, param)) return;
	block_visit(host, host.cases, param);
}

void RecursiveVisitor::caseSwitchCases(ASTSwitchCases& host, void* param)
{
	block_visit(host, host.ranges, param);
	if (breakRecursion(host, param)) return;
	block_visit(host, host.cases, param);
	if (breakRecursion(host, param)) return;
	visit(host.block.get(), param);
}

void RecursiveVisitor::caseRange(ASTRange& host, void* param)
{
	visit(host.start.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.end.get(), param);
}

void RecursiveVisitor::caseStmtFor(ASTStmtFor& host, void* param)
{
	visit(host.setup.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.test.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.increment.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.body.get(), param);
}

void RecursiveVisitor::caseStmtWhile(ASTStmtWhile& host, void* param)
{
	visit(host.test.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.body.get(), param);
}

void RecursiveVisitor::caseStmtDo(ASTStmtDo& host, void* param)
{
	visit(host.body.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.test.get(), param);
}

void RecursiveVisitor::caseStmtRepeat(ASTStmtRepeat& host, void* param)
{
	visit(*host.iter, param);
	if(breakRecursion(host, param)) return;
	optional<int32_t> repeats = (*host.iter).getCompileTimeValue(this, scope);
	if(host.bodies.size() == 0)
	{
		if(repeats)
		{
			int32_t rep = *repeats / 10000L;
			if(rep>0)
			{
				for(int32_t q = 0; q < rep; ++q)
				{
					host.bodies.push_back((*host.body).clone());
				}
			}
			else if(rep < 0)
			{
				handleError(CompileError::ConstantBadSize(&*host.iter, ">= 0"));
			}
		}
		else
		{
			handleError(CompileError::ExprNotConstant(&*host.iter));
		}
	}
	visit(host, host.bodies, param);
}

void RecursiveVisitor::caseStmtReturnVal(ASTStmtReturnVal& host, void* param)
{
	visit(host.value.get(), param);
}

// Declarations

void RecursiveVisitor::caseScript(ASTScript& host, void* param)
{
	visit(host.type.get(), param);
	if (breakRecursion(host, param)) return;
	block_visit(host, host.options, param);
	if (breakRecursion(host, param)) return;
	block_visit(host, host.use, param);
	if (breakRecursion(host, param)) return;
	block_visit(host, host.types, param);
	if (breakRecursion(host, param)) return;
	block_visit(host, host.variables, param);
	if (breakRecursion(host, param)) return;
	block_visit(host, host.functions, param);
	if (breakRecursion(host, param)) return;
	block_visit(host, host.asserts, param);
}

void RecursiveVisitor::caseNamespace(ASTNamespace& host, void* param)
{
	block_visit(host, host.options, param);
	if (breakRecursion(host, param)) return;
	block_visit(host, host.dataTypes, param);
	if (breakRecursion(host, param)) return;
	block_visit(host, host.scriptTypes, param);
	if (breakRecursion(host, param)) return;
	block_visit(host, host.use, param);
	if (breakRecursion(host, param)) return;
	block_visit(host, host.variables, param);
	if (breakRecursion(host, param)) return;
	block_visit(host, host.functions, param);
	if (breakRecursion(host, param)) return;
	block_visit(host, host.namespaces, param);
	if (breakRecursion(host, param)) return;
	block_visit(host, host.scripts, param);
	if (breakRecursion(host, param)) return;
	block_visit(host, host.asserts, param);
}

void RecursiveVisitor::caseImportDecl(ASTImportDecl& host, void* param)
{
	visit(host.getTree(), param);
}

void RecursiveVisitor::caseImportCondDecl(ASTImportCondDecl& host, void* param)
{
	visit(*host.cond, param);
	if(breakRecursion(host, param)) return;
	optional<int32_t> val = host.cond->getCompileTimeValue(this, scope);
	if(val && (*val != 0))
	{
		if(!host.preprocessed)
		{
			ScriptParser::preprocess_one(*host.import, ScriptParser::recursionLimit);
			host.preprocessed = true;
		}
		visit(*host.import, param);
	}
}

void RecursiveVisitor::caseFuncDecl(ASTFuncDecl& host, void* param)
{
	if(host.getFlag(FUNCFLAG_INVALID))
	{
		handleError(CompileError::BadFuncModifiers(&host, host.invalidMsg));
		return;
	}
	visit(host.returnType.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host, host.parameters, param);
	if (breakRecursion(host, param)) return;
	if(host.prototype)
		visit(host.defaultReturn.get(), param);
	else visit(host.block.get(), param);
}

void RecursiveVisitor::caseDataDeclList(ASTDataDeclList& host, void* param)
{
	visit(host.baseType.get(), param);
	if (breakRecursion(host, param)) return;
	block_visit(host, host.getDeclarations(), param);
}

void RecursiveVisitor::caseDataEnum(ASTDataEnum& host, void* param)
{
	visit(host.baseType.get(), param);
	if (breakRecursion(host, param)) return;
	block_visit(host, host.getDeclarations(), param);
}

void RecursiveVisitor::caseDataDecl(ASTDataDecl& host, void* param)
{
	visit(host.baseType.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host, host.extraArrays, param);
	if (breakRecursion(host, param)) return;
	visit(host.getInitializer(), param);
}

void RecursiveVisitor::caseDataDeclExtraArray(
		ASTDataDeclExtraArray& host, void* param)
{
	visit(host, host.dimensions, param);
}

void RecursiveVisitor::caseDataTypeDef(ASTDataTypeDef& host, void* param)
{
	visit(host.type.get(), param);
}

void RecursiveVisitor::caseCustomDataTypeDef(ASTCustomDataTypeDef& host, void* param)
{
	visit(host.type.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.definition.get(), param);
}

void RecursiveVisitor::caseAssert(ASTAssert& host, void* param)
{
	//Ignored, except in SemanticAnalyzer
}

// Expressions

void RecursiveVisitor::caseExprConst(ASTExprConst& host, void* param)
{
	visit(host.content.get(), param);
}

void RecursiveVisitor::caseVarInitializer(ASTExprVarInitializer& host, void* param)
{
	caseExprConst(host, param);
}

void RecursiveVisitor::caseExprAssign(ASTExprAssign& host, void* param)
{
	visit(host.left.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.right.get(), param);
}

void RecursiveVisitor::caseExprArrow(ASTExprArrow& host, void* param)
{
	visit(host.left.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.index.get(), param);
}

void RecursiveVisitor::caseExprIndex(ASTExprIndex& host, void* param)
{
	visit(host.array.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.index.get(), param);
}

void RecursiveVisitor::caseExprCall(ASTExprCall& host, void* param)
{
	//visit(host.left, param);
	//if (breakRecursion(host, param)) return;
	visit(host, host.parameters, param);
}

void RecursiveVisitor::caseExprNegate(ASTExprNegate& host, void* param)
{
	visit(host.operand.get(), param);
}

void RecursiveVisitor::caseExprNot(ASTExprNot& host, void* param)
{
	visit(host.operand.get(), param);
}

void RecursiveVisitor::caseExprBitNot(ASTExprBitNot& host, void* param)
{
	visit(host.operand.get(), param);
}

void RecursiveVisitor::caseExprIncrement(ASTExprIncrement& host, void* param)
{
	visit(host.operand.get(), param);
}

void RecursiveVisitor::caseExprPreIncrement(
		ASTExprPreIncrement& host, void* param)
{
	visit(host.operand.get(), param);
}

void RecursiveVisitor::caseExprDecrement(ASTExprDecrement& host, void* param)
{
	visit(host.operand.get(), param);
}

void RecursiveVisitor::caseExprPreDecrement(
		ASTExprPreDecrement& host, void* param)
{
	visit(host.operand.get(), param);
}

void RecursiveVisitor::caseExprCast(ASTExprCast& host, void* param)
{
	visit(host.operand.get(), param);
}

void RecursiveVisitor::caseExprAnd(ASTExprAnd& host, void* param)
{
	visit(host.left.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.right.get(), param);
}

void RecursiveVisitor::caseExprOr(ASTExprOr& host, void* param)
{
	visit(host.left.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.right.get(), param);
}

void RecursiveVisitor::caseExprGT(ASTExprGT& host, void* param)
{
	visit(host.left.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.right.get(), param);
}

void RecursiveVisitor::caseExprGE(ASTExprGE& host, void* param)
{
	visit(host.left.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.right.get(), param);
}

void RecursiveVisitor::caseExprLT(ASTExprLT& host, void* param)
{
	visit(host.left.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.right.get(), param);
}

void RecursiveVisitor::caseExprLE(ASTExprLE& host, void* param)
{
	visit(host.left.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.right.get(), param);
}

void RecursiveVisitor::caseExprEQ(ASTExprEQ& host, void* param)
{
	visit(host.left.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.right.get(), param);
}

void RecursiveVisitor::caseExprNE(ASTExprNE& host, void* param)
{
	visit(host.left.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.right.get(), param);
}

void RecursiveVisitor::caseExprAppxEQ(ASTExprAppxEQ& host, void* param)
{
	visit(host.left.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.right.get(), param);
}

void RecursiveVisitor::caseExprXOR(ASTExprXOR& host, void* param)
{
	visit(host.left.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.right.get(), param);
}

void RecursiveVisitor::caseExprPlus(ASTExprPlus& host, void* param)
{
	visit(host.left.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.right.get(), param);
}

void RecursiveVisitor::caseExprMinus(ASTExprMinus& host, void* param)
{
	visit(host.left.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.right.get(), param);
}

void RecursiveVisitor::caseExprTimes(ASTExprTimes& host, void* param)
{
	visit(host.left.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.right.get(), param);
}

void RecursiveVisitor::caseExprDivide(ASTExprDivide& host, void* param)
{
	visit(host.left.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.right.get(), param);
}

void RecursiveVisitor::caseExprModulo(ASTExprModulo& host, void* param)
{
	visit(host.left.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.right.get(), param);
}

void RecursiveVisitor::caseExprBitAnd(ASTExprBitAnd& host, void* param)
{
	visit(host.left.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.right.get(), param);
}

void RecursiveVisitor::caseExprBitOr(ASTExprBitOr& host, void* param)
{
	visit(host.left.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.right.get(), param);
}

void RecursiveVisitor::caseExprBitXor(ASTExprBitXor& host, void* param)
{
	visit(host.left.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.right.get(), param);
}

void RecursiveVisitor::caseExprLShift(ASTExprLShift& host, void* param)
{
	visit(host.left.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.right.get(), param);
}

void RecursiveVisitor::caseExprRShift(ASTExprRShift& host, void* param)
{
	visit(host.left.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.right.get(), param);
}

void RecursiveVisitor::caseExprTernary(ASTTernaryExpr& host, void* param)
{
	visit(host.left.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.middle.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.right.get(), param);
}

// Literals

void RecursiveVisitor::caseNumberLiteral(ASTNumberLiteral& host, void* param)
{
	visit(host.value.get(), param);
}

void RecursiveVisitor::caseCharLiteral(ASTCharLiteral& host, void* param)
{
	visit(host.value.get(), param);
}

void RecursiveVisitor::caseArrayLiteral(ASTArrayLiteral& host, void* param)
{
	visit(host.type.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.size.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host, host.elements, param);
}
