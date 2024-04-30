#include "ASTVisitors.h"

#include "zsyssimple.h"
#include "CompileError.h"
#include "Scope.h"
#include "parserDefs.h"
#include <assert.h>
#include <cstdarg>
#include "ZScript.h"

using std::list;
using std::vector;
using namespace ZScript;

struct tag {};
void* const RecursiveVisitor::paramNone = new tag();
void* const RecursiveVisitor::paramRead = new tag();
void* const RecursiveVisitor::paramWrite = new tag();
void* const RecursiveVisitor::paramReadWrite = new tag();

uint32_t zscript_failcode = 0;
bool zscript_error_out = false;

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

void RecursiveVisitor::handleError(CompileError const& error, std::string const* inf)
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
			std::optional<int32_t> errorId = idNode.getCompileTimeValue(this, scope);
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
	
	if (error.isStrict())
	{
		if(hard_error) failure_halt = true;
		failure = true;
		failure_temp = true;
		if(!zscript_failcode)
			zscript_failcode = *error.getId();
		if(inf && inf->size())
			zconsole_error("%s\nINFO: %s",err_str_ptr,inf->c_str());
		else zconsole_error("%s",err_str_ptr);
	}
	else
	{
		if(inf && inf->size())
			zconsole_warn("%s\nINFO: %s",err_str_ptr,inf->c_str());
		else zconsole_warn("%s",err_str_ptr);
	}
	//log_error(error);
}

void RecursiveVisitor::deprecWarn(Function* func, AST* host, std::string const& s1, std::string const& s2)
{
	switch(*ZScript::lookupOption(*scope, CompileOption::OPT_WARN_DEPRECATED)/10000)
	{
		case 0: //No warn
			break;
		case 2: //Error
			if(func->shouldShowDepr(true))
			{
				handleError(CompileError::DeprecatedError(host, s1, s2), &func->getInfo());
			}
			break;
		default: //Warn
			if(func->shouldShowDepr(false))
			{
				handleError(CompileError::DeprecatedWarn(host, s1, s2), &func->getInfo());
			}
			break;
	}
}
void RecursiveVisitor::deprecWarn(AST* host, std::string const& s1, std::string const& s2, std::string const& info)
{
	switch(*ZScript::lookupOption(*scope, CompileOption::OPT_WARN_DEPRECATED)/10000)
	{
		case 0: //No warn
			break;
		case 2: //Error
			handleError(CompileError::DeprecatedError(host, s1, s2), info.empty() ? nullptr : &info);
			break;
		default: //Warn
			handleError(CompileError::DeprecatedWarn(host, s1, s2), info.empty() ? nullptr : &info);
			break;
	}
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

void RecursiveVisitor::_visit_internals(Function& func)
{
	if(func.isTemplateSkip())
	{
		auto& vec = func.get_applied_funcs();
		auto& sz = template_function_data[&func];
		if(sz >= vec.size())
			return;
		for(size_t q = sz; q < vec.size(); ++q)
			template_function_queue.insert(vec[q].get());
		sz = vec.size();
	}
	else
	{
		Scope* oldscope = scope;
		scope = func.getInternalScope();
		analyzeFunctionInternals(func);
		scope = oldscope;
	}
}
void RecursiveVisitor::visitFunctionInternals(ZScript::Program& program)
{
	vector<Function*> functions = program.getUserGlobalFunctions();
	
	for (auto& func : functions)
		_visit_internals(*func);
	
	for (vector<Script*>::iterator it = program.scripts.begin();
		 it != program.scripts.end(); ++it)
	{
		Script& script = **it;
		scope = &script.getScope();
		functions = scope->getLocalFunctions();
		for (auto& func : functions)
			_visit_internals(*func);
		scope = scope->getParent();
	}
	
	for (vector<Namespace*>::iterator it = program.namespaces.begin();
		 it != program.namespaces.end(); ++it)
	{
		Namespace& namesp = **it;
		scope = &namesp.getScope();
		functions = scope->getLocalFunctions();
		for (auto& func : functions)
			_visit_internals(*func);
		scope = scope->getParent();
	}
	
	for (vector<UserClass*>::iterator it = program.classes.begin();
		 it != program.classes.end(); ++it)
	{
		UserClass& user_class = **it;
		ClassScope* cscope = &user_class.getScope();
		scope = cscope;
		
		DataType const* thisType = &cscope->user_class.getNode()->type->resolve(*cscope,this);
		DataType const* constType = thisType->getConstType();

		functions = cscope->getConstructors();
		parsing_user_class = puc_construct;
		for (auto& func : functions)
			_visit_internals(*func);
		
		functions = scope->getLocalFunctions();
		for (auto& func : functions)
		{
			if(func->getFlag(FUNCFLAG_STATIC))
				parsing_user_class = puc_none;
			else
				parsing_user_class = puc_funcs;
			_visit_internals(*func);
		}
		
		functions = cscope->getDestructor();
		parsing_user_class = puc_destruct;
		for (auto& func : functions)
			_visit_internals(*func);
		parsing_user_class = puc_none;
		
		scope = scope->getParent();
	}


	for (auto& pair : template_function_data)
		_visit_internals(*pair.first);
	while(!template_function_queue.empty())
	{
		while(!template_function_queue.empty())
		{
			Function* func = *template_function_queue.begin();
			_visit_internals(*func);
			template_function_queue.erase(template_function_queue.begin());
		}
		for (auto& pair : template_function_data)
			_visit_internals(*pair.first);
	}
}

void RecursiveVisitor::checkCast(
		DataType const& sourceType, DataType const& targetType, AST* node, bool twoWay)
{
	if (sourceType.canCastTo(targetType, scope)) return;
	if (twoWay && targetType.canCastTo(sourceType, scope)) return;
	handleError(
		CompileError::IllegalCast(
			node, sourceType.getName(), targetType.getName()));
}

////////////////////////////////////////////////////////////////
// Cases

void RecursiveVisitor::caseFile(ASTFile& host, void* param)
{
	block_visit_vec(host.options, param);
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.use, param);
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.dataTypes, param);
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.scriptTypes, param);
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.imports, param);
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.condimports, param);
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.variables, param);
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.functions, param);
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.namespaces, param);
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.scripts, param);
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.classes, param);
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.asserts, param);
}

void RecursiveVisitor::caseSetOption(ASTSetOption& host, void* param)
{
	visit(host.expr.get(), param);
}

// Statements

void RecursiveVisitor::caseBlock(ASTBlock& host, void* param)
{
	block_visit_vec(host.options, param);
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.statements, param);
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
	block_visit_vec(host.cases, param);
}

void RecursiveVisitor::caseSwitchCases(ASTSwitchCases& host, void* param)
{
	block_visit_vec(host.ranges, param);
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.cases, param);
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
	visit_vec(host.increments, param);
	if (breakRecursion(host, param)) return;
	visit(host.body.get(), param);
	if (breakRecursion(host, param)) return;
	if(host.hasElse())
		visit(host.elseBlock.get(), param);
}
void RecursiveVisitor::caseStmtForEach(ASTStmtForEach& host, void* param)
{
	visit(host.arrExpr.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.body.get(), param);
	if(host.indxdecl)
	{
		if (breakRecursion(host, param)) return;
		visit(host.indxdecl.get(), param);
	}
	if(host.arrdecl)
	{
		if (breakRecursion(host, param)) return;
		visit(host.arrdecl.get(), param);
	}
	if(host.decl)
	{
		if (breakRecursion(host, param)) return;
		visit(host.decl.get(), param);
	}
	if(host.hasElse())
	{
		if (breakRecursion(host, param)) return;
		visit(host.elseBlock.get(), param);
	}
}

void RecursiveVisitor::caseStmtRangeLoop(ASTStmtRangeLoop& host, void* param)
{
	visit(host.type.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.range.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.increment.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.body.get(), param);
	if(host.decl)
	{
		if (breakRecursion(host, param)) return;
		visit(host.decl.get(), param);
	}
	if(host.hasElse())
	{
		if (breakRecursion(host, param)) return;
		visit(host.elseBlock.get(), param);
	}
}

void RecursiveVisitor::caseStmtWhile(ASTStmtWhile& host, void* param)
{
	visit(host.test.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.body.get(), param);
	if (breakRecursion(host, param)) return;
	if(host.hasElse())
		visit(host.elseBlock.get(), param);
}

void RecursiveVisitor::caseStmtDo(ASTStmtDo& host, void* param)
{
	visit(host.body.get(), param);
	if (breakRecursion(host, param)) return;
	visit(host.test.get(), param);
	if (breakRecursion(host, param)) return;
	if(host.hasElse())
		visit(host.elseBlock.get(), param);
}

void RecursiveVisitor::caseStmtRepeat(ASTStmtRepeat& host, void* param)
{
	visit(*host.iter, param);
	if(breakRecursion(host, param)) return;
	std::optional<int32_t> repeats = (*host.iter).getCompileTimeValue(this, scope);
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
	visit_vec(host.bodies, param);
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
	block_visit_vec(host.options, param);
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.use, param);
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.types, param);
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.variables, param);
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.functions, param);
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.asserts, param);
}
void RecursiveVisitor::caseClass(ASTClass& host, void* param)
{
	block_visit_vec(host.options, param);
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.use, param);
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.types, param);
	if (breakRecursion(host, param)) return;
	parsing_user_class = puc_vars;
	block_visit_vec(host.variables, param);
	parsing_user_class = puc_none;
	if (breakRecursion(host, param)) return;
	parsing_user_class = puc_funcs;
	block_visit_vec(host.functions, param);
	parsing_user_class = puc_none;
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.asserts, param);
	if (breakRecursion(host, param)) return;
	parsing_user_class = puc_construct;
	block_visit_vec(host.constructors, param);
	parsing_user_class = puc_none;
	if (breakRecursion(host, param)) return;
	parsing_user_class = puc_destruct;
	visit(host.destructor.get(), param);
	parsing_user_class = puc_none;
}

void RecursiveVisitor::caseNamespace(ASTNamespace& host, void* param)
{
	block_visit_vec(host.options, param);
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.dataTypes, param);
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.scriptTypes, param);
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.use, param);
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.variables, param);
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.functions, param);
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.namespaces, param);
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.scripts, param);
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.classes, param);
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.asserts, param);
}

void RecursiveVisitor::caseImportDecl(ASTImportDecl& host, void* param)
{
	visit(host.getTree(), param);
}
void RecursiveVisitor::caseIncludePath(ASTIncludePath& host, void* param)
{}

void RecursiveVisitor::caseImportCondDecl(ASTImportCondDecl& host, void* param)
{
	visit(*host.cond, param);
	if(breakRecursion(host, param)) return;
	std::optional<int32_t> val = host.cond->getCompileTimeValue(this, scope);
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
	visit_vec(host.parameters, param);
	if (breakRecursion(host, param)) return;
	visit_vec(host.optparams, param);
	if (breakRecursion(host, param)) return;
	if(host.prototype)
		visit(host.defaultReturn.get(), param);
	else visit(host.block.get(), param);
}

void RecursiveVisitor::caseDataDeclList(ASTDataDeclList& host, void* param)
{
	visit(host.baseType.get(), param);
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.getDeclarations(), param);
}

void RecursiveVisitor::caseDataEnum(ASTDataEnum& host, void* param)
{
	visit(host.baseType.get(), param);
	if (breakRecursion(host, param)) return;
	block_visit_vec(host.getDeclarations(), param);
}

void RecursiveVisitor::caseDataDecl(ASTDataDecl& host, void* param)
{
	visit(host.baseType.get(), param);
	if (breakRecursion(host, param)) return;
	visit_vec(host.extraArrays, param);
	if (breakRecursion(host, param)) return;
	visit(host.getInitializer(), param);
}

void RecursiveVisitor::caseDataDeclExtraArray(
		ASTDataDeclExtraArray& host, void* param)
{
	visit_vec(host.dimensions, param);
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
	visit(host.left.get(), param);
	if (breakRecursion(host, param)) return;
	visit_vec(host.parameters, param);
}

void RecursiveVisitor::caseExprDelete(ASTExprDelete& host, void* param)
{
	visit(host.operand.get(), param);
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

void RecursiveVisitor::caseExprDecrement(ASTExprDecrement& host, void* param)
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

void RecursiveVisitor::caseExprExpn(ASTExprExpn& host, void* param)
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
	visit_vec(host.elements, param);
}
