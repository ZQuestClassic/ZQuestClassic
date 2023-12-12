/**
 * Code path handler, to check for unreachable code / missing 'return' statements
 * Begun: 12th Dec, 2023
 * Author: Emily
 */

#include "parserDefs.h"
#include "ReturnVisitor.h"
#include <cassert>
#include <sstream>
#include "Scope.h"
#include "CompileError.h"

#include "zc/ffscript.h"
extern FFScript FFCore;
using std::ostringstream;
using namespace ZScript;
using std::unique_ptr;
////////////////////////////////////////////////////////////////
// ReturnVisitor

ReturnVisitor::ReturnVisitor(Program& program)
	: program(program)
{
	// Analyze function internals.
	visitFunctionInternals(program);
}

void ReturnVisitor::visit(AST& node, void* param)
{
	if(node.isDisabled()) return; //Don't visit disabled nodes.
	if(reachable(node)) return; //Don't double-pass
	RecursiveVisitor::visit(node, param);
}

void ReturnVisitor::caseDefault(AST& host, void* param)
{
	markReachable(host);
}

void ReturnVisitor::analyzeFunctionInternals(Function& function)
{
	Scope* oldscope = scope;
	scope = function.getInternalScope();
	ASTFuncDecl* node = function.node;
	ASTBlock* block = node->block.get();
	if(!node) return;
	if(!block) return;
	failure_temp = failure_halt = false;
	//
	auto& stmts = block->statements;
	size_t indx = 0;
	bool earlyterm = false;
	VisitNode rootnode(node);
	for(auto it = stmts.begin(); it != stmts.end(); ++it)
	{
		ASTStmt& stmt = **it;
		visit(stmt, (void*)&rootnode);
		markReachable(stmt);
		earlyterm = rootnode.check_terminate(indx);
		indx = rootnode.child_index();
		if(earlyterm)
			break;
	}
	
	if(!earlyterm && !function.returnType->isVoid())
		handleError(CompileError::MissingReturn(node, node->name));
	//
	scope = oldscope;
}

void ReturnVisitor::caseBlock(ASTBlock& host, void* param)
{
	VisitNode* paramNode = (VisitNode*)param;
	paramNode = paramNode->create(&host);
	
	block_retvisit(host, host.statements, paramNode);
	markReachable(host);
}

void do_return(AST& host, void* param)
{
	VisitNode* paramNode = (VisitNode*)param;
	paramNode = paramNode->create(&host);
	paramNode->force_term(true);
	for(VisitNode* curr = paramNode; curr; curr = curr->parent)
		curr->flags |= VNODE_FLAG_HASRETURN;
}

void ReturnVisitor::caseStmtReturn(ASTStmtReturn& host, void* param)
{
	do_return(host, param);
	markReachable(host);
}

void ReturnVisitor::caseStmtReturnVal(ASTStmtReturnVal& host, void* param)
{
    RecursiveVisitor::caseStmtReturnVal(host);
	
	do_return(host, param);
	markReachable(host);
}

void ReturnVisitor::caseStmtBreak(ASTStmtBreak& host, void* param)
{
	if(host.count.get())
	{
		if(std::optional<int32_t> v = host.count->getCompileTimeValue(this, scope))
		{
			int32_t val = *v;
			if(val < 0)
			{
				handleError(CompileError::BreakBadCount(&host, val));
				val = 10000;
			}
			host.breakCount = size_t(val/10000);
		}
	}
	VisitNode* paramNode = (VisitNode*)param;
	paramNode = paramNode->create(&host);
	paramNode->force_term(true);
	paramNode->do_break(host.breakCount);
	markReachable(host);
}

void ReturnVisitor::caseStmtContinue(ASTStmtContinue &host, void* param)
{
	if(host.count.get())
	{
		if(std::optional<int32_t> v = host.count->getCompileTimeValue(this, scope))
		{
			int32_t val = *v;
			if(val < 0)
			{
				handleError(CompileError::BreakBadCount(&host, val));
				val = 10000;
			}
			host.contCount = size_t(val/10000);
		}
	}
	VisitNode* paramNode = (VisitNode*)param;
	paramNode = paramNode->create(&host);
	paramNode->force_term(true);
	paramNode->do_continue(host.contCount);
	markReachable(host);
}

void ReturnVisitor::caseStmtIf(ASTStmtIf& host, void* param)
{
	VisitNode* paramNode = (VisitNode*)param;
	paramNode = paramNode->create(&host);
	paramNode->force_term(false); //'if' without 'else' never terminates
	
	if(host.isDecl())
		visit(host.declaration.get(), paramNode);
	visit(host.condition.get(), paramNode);
	visit(host.thenStatement.get(), paramNode);
	markReachable(host);
}

void ReturnVisitor::caseStmtIfElse(ASTStmtIfElse& host, void* param)
{
	VisitNode* paramNode = (VisitNode*)param;
	paramNode = paramNode->create(&host);
	paramNode->mark_branch(); // is a branch of its' 2 child nodes
	VisitNode* thenNode = paramNode->create(host.thenStatement.get());
	VisitNode* elseNode = paramNode->create(host.elseStatement.get());
	
	if(host.isDecl())
		visit(host.declaration.get(), thenNode);
	visit(host.condition.get(), thenNode);
	visit(host.thenStatement.get(), thenNode);
	visit(host.elseStatement.get(), elseNode);
	markReachable(host);
}

void ReturnVisitor::caseStmtFor(ASTStmtFor& host, void* param)
{
	VisitNode* paramNode = (VisitNode*)param;
	paramNode = paramNode->create(&host);
	paramNode->mark_branch(); // is a branch of its' 2 child nodes
	VisitNode* thenNode = paramNode->create(host.body.get());
	
	visit(host.setup.get(), thenNode);
	visit(host.test.get(), thenNode);
	visit(host.increment.get(), thenNode);
	visit(host.body.get(), thenNode);
	
	auto val = host.test->getCompileTimeValue(this, scope);
	bool infloop = (val && *val);
	//if a break/continue moved out of it, don't count it as a terminator
	if(thenNode->get_flag(VNODE_FLAG_EXITED))
		paramNode->force_term(false);
	//an infinite loop that can return can count as a terminator
	else if(infloop && thenNode->get_flag(VNODE_FLAG_HASRETURN))
		paramNode->force_term(true);
	else if(!infloop)
	{
		VisitNode* elseNode = paramNode->create(host.elseBlock.get());
		if(host.hasElse())
			visit(host.elseBlock.get(), elseNode);
	}
	markReachable(host);
}

void ReturnVisitor::caseStmtForEach(ASTStmtForEach& host, void* param)
{
	VisitNode* paramNode = (VisitNode*)param;
	paramNode = paramNode->create(&host);
	paramNode->mark_branch(); // is a branch of its' 2 child nodes
	VisitNode* thenNode = paramNode->create(host.body.get());
	VisitNode* elseNode = paramNode->create(host.elseBlock.get());
	
	visit(host.arrExpr.get(), thenNode);
	visit(host.body.get(), thenNode);
	if(host.indxdecl)
		visit(host.indxdecl.get(), thenNode);
	if(host.arrdecl)
		visit(host.arrdecl.get(), thenNode);
	if(host.decl)
		visit(host.decl.get(), thenNode);
	if(host.hasElse())
		visit(host.elseBlock.get(), elseNode);
	markReachable(host);
}

void ReturnVisitor::caseStmtWhile(ASTStmtWhile& host, void* param)
{
	VisitNode* paramNode = (VisitNode*)param;
	paramNode = paramNode->create(&host);
	paramNode->mark_branch(); // is a branch of its' 2 child nodes
	VisitNode* thenNode = paramNode->create(host.body.get());
	
	visit(host.test.get(), thenNode);
	visit(host.body.get(), thenNode);
	
	auto val = host.test->getCompileTimeValue(this, scope);
	bool inv = host.isInverted();
	bool infloop = (val && (inv == !*val));
	//if a break/continue moved out of it, don't count it as a terminator
	if(thenNode->get_flag(VNODE_FLAG_EXITED))
		paramNode->force_term(false);
	//an infinite loop that can return can count as a terminator
	else if(infloop && thenNode->get_flag(VNODE_FLAG_HASRETURN))
		paramNode->force_term(true);
	else if(!infloop)
	{
		VisitNode* elseNode = paramNode->create(host.elseBlock.get());
		if(host.hasElse())
			visit(host.elseBlock.get(), elseNode);
	}
	markReachable(host);
}

void ReturnVisitor::caseStmtDo(ASTStmtDo& host, void* param)
{
	VisitNode* paramNode = (VisitNode*)param;
	paramNode = paramNode->create(&host);
	paramNode->mark_branch(); // is a branch of its' 2 child nodes
	VisitNode* thenNode = paramNode->create(host.body.get());
	
	visit(host.body.get(), thenNode);
	visit(host.test.get(), thenNode);
	
	auto val = host.test->getCompileTimeValue(this, scope);
	bool inv = host.isInverted();
	bool infloop = (val && (inv == !*val));
	//if a break/continue moved out of it, don't count it as a terminator
	if(thenNode->get_flag(VNODE_FLAG_EXITED))
		paramNode->force_term(false);
	//an infinite loop that can return can count as a terminator
	else if(infloop && thenNode->get_flag(VNODE_FLAG_HASRETURN))
		paramNode->force_term(true);
	else if(!infloop)
	{
		VisitNode* elseNode = paramNode->create(host.elseBlock.get());
		if(host.hasElse())
			visit(host.elseBlock.get(), elseNode);
	}
	markReachable(host);
}

void ReturnVisitor::caseStmtSwitch(ASTStmtSwitch& host, void* param)
{
	VisitNode* paramNode = (VisitNode*)param;
	paramNode = paramNode->create(&host);
	//is NOT a branch of its' child nodes, due to fallthrough
	
	visit(host.key.get(), paramNode);
	block_visit(host, host.cases, paramNode);
	
	//if a break/continue moved out of it, don't count it as a terminator
	if(paramNode->get_flag(VNODE_FLAG_EXITED))
		paramNode->force_term(false);
	else
	{
		bool has_default = false;
		for(auto _case : host.cases)
			if(_case->isDefault)
			{
				has_default = true;
				break;
			}
		if(!has_default)
			paramNode->force_term(false);
		else if(paramNode->children.size())
			paramNode->force_term(paramNode->children.back()->terminates());
		else paramNode->force_term(false);
	}
	markReachable(host);
}

void ReturnVisitor::caseSwitchCases(ASTSwitchCases& host, void* param)
{
	VisitNode* paramNode = (VisitNode*)param;
	paramNode = paramNode->create(&host);
	
	block_visit(host, host.ranges, paramNode);
	block_visit(host, host.cases, paramNode);
	visit(host.block.get(), paramNode);
	markReachable(host);
}

void ReturnVisitor::caseExprCall(ASTExprCall& host, void* param)
{
	visit(host, host.parameters, param);
	auto& func = *host.binding;
	if(func.getFlag(FUNCFLAG_EXITS))
		do_return(host, param);
	markReachable(host);
}

//Helper Functions
bool ReturnVisitor::reachable(AST& node) const
{
	return node.reachable();
}

bool ReturnVisitor::reachable(AST* node) const
{
	if(node) return reachable(*node);
	return true;
}

template <class Container>
bool ReturnVisitor::reachable(AST& host, Container const& nodes) const
{
	for(auto it = nodes.cbegin(); it != nodes.cend(); ++it)
	{
		if(!reachable(*it)) return false;
	}
	return true;
}


