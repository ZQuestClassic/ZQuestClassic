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

class ResetVisitor : public RecursiveVisitor
{
public:
	using RecursiveVisitor::visit;
	void visit(AST& node, void* param = NULL)
	{
		node.mark_reachable(false);
		RecursiveVisitor::visit(node, param);
	}
};

////////////////////////////////////////////////////////////////
// ReturnVisitor

ReturnVisitor::ReturnVisitor(Program& program)
	: program(program), marked_never_ret(false), missing_ret(false),
	var_map()
{
	mode = MODE_START;
	visitFunctionInternals(program);
	mode = MODE_EXPASS;
	while(marked_never_ret)
	{
		if(missing_ret)
			break;
		marked_never_ret = false;
		visitFunctionInternals(program);
	}
	mode = MODE_FINISH;
	visitFunctionInternals(program);
}

void ReturnVisitor::visit(AST& node, void* param)
{
	if(node.isDisabled()) return; //Don't visit disabled nodes.
	if(reachable(node)) return; //Don't double-pass
	RecursiveVisitor::visit(node, param);
	markReachable(node);
}

template <class Container>
bool ReturnVisitor::block_retvisit_vec(Container const& nodes, void* param)
{
	VisitNode* paramNode = (VisitNode*)param;
	bool check_early_ret = !paramNode->get_flag(VNODE_FLAG_BRANCH);
	size_t indx = paramNode->child_index();
	for (typename Container::const_iterator it = nodes.cbegin();
		 it != nodes.cend(); ++it)
	{
		failure_temp = false;
		visit(**it, param);
		if(check_early_ret)
		{
			paramNode->check_terminate(indx);
			indx = paramNode->child_index();
			if(paramNode->terminates())
				return false;
		}
		if(failure_halt) return false;
	}
	return true;
}
bool ReturnVisitor::block_retvisit(AST& host, void* param)
{
	bool ret = true;
	if(ASTBlock* block = dynamic_cast<ASTBlock*>(&host))
	{
		VisitNode* paramNode = (VisitNode*)param;
		paramNode = paramNode->create(&host);
		
		ret = block_retvisit_vec(block->statements, paramNode);
		markReachable(host);
	}
	else
	{
		VisitNode* paramNode = (VisitNode*)param;
		bool check_early_ret = !paramNode->get_flag(VNODE_FLAG_BRANCH);
		size_t indx = paramNode->child_index();
		failure_temp = false;
		visit(host, param);
		if(check_early_ret)
		{
			paramNode->check_terminate(indx);
			if(paramNode->terminates())
				return false;
		}
		if(failure_halt) return false;
	}
	return ret;
}

void ReturnVisitor::caseDefault(AST& host, void* param)
{
	markReachable(host);
}

void ReturnVisitor::analyzeFunctionInternals(Function& function)
{
	ASTFuncDecl* node = function.node;
	ASTBlock* block = node->block.get();
	if(!node || node->isDisabled()) return;
	if(!block) return;
	auto func_var_map = var_map[&function];
	if(mode == MODE_FINISH)
	{
		for(auto& pair : func_var_map)
		{
			Variable* var = pair.first;
			ASTDataDecl* node = var->getNode();
			if(!pair.second) //unused
				ZScript::eraseDatum(var->scope, *var);
		}
	}
	else
	{
		auto& stmts = block->statements;
		if(mode == MODE_EXPASS) //already parsed through at least once
		{
			if(function.isNil())
				return; //nothing more to possibly do here
		}
		ResetVisitor resetter;
		resetter.visit(*block); //reset the 'reachable' state to false
		func_var_map.clear();
		
		markReachable(*node);
		markReachable(*block);
		failure_temp = failure_halt = false;
		bool no_ret = false;
		if(stmts.empty()) //Function is completely empty; optimize
		{
			function.setFlag(FUNCFLAG_NIL);
			no_ret = true; //still a missing return error if not void/constructor
		}
		else
		{
			ASTStmtReturn* op_ret = dynamic_cast<ASTStmtReturn*>(stmts.front());
			ASTStmtReturnVal* op_retv = dynamic_cast<ASTStmtReturnVal*>(stmts.front());
			bool done = false;
			if(op_ret || op_retv) //function just returns... nil with a def val
			{
				done = true;
				if(op_retv)
				{
					if(auto val = op_retv->value->getCompileTimeValue(this, scope))
						function.defaultReturn = val;
					else done = false;
				}
				else function.defaultReturn.reset();
				if(done)
					function.setFlag(FUNCFLAG_NIL);
			}
			if(!done) //Visit the function's statements
			{
				size_t indx = 0;
				bool earlyterm = false;
				VisitNode rootnode(node);
				in_func = &function;
				for(auto it = stmts.begin(); it != stmts.end(); ++it)
				{
					ASTStmt& stmt = **it;
					visit(stmt, (void*)&rootnode);
					earlyterm = rootnode.check_terminate(indx);
					indx = rootnode.child_index();
					if(earlyterm)
						break;
				}
				in_func = nullptr;
				
				if(earlyterm)
				{
					if(!rootnode.get_flag(VNODE_FLAG_HASRETURN))
					{
						no_ret = true;
						//Terminates without return; infinite loop, or script quit?
						if(!function.getFlag(FUNCFLAG_NEVER_RETURN))
						{
							marked_never_ret = true;
							function.setFlag(FUNCFLAG_NEVER_RETURN);
						}
					}
				}
				else no_ret = true;
			}
		}
		//Void functions && constructors can miss out on returns
		if(!(function.returnType->isVoid() || function.getFlag(FUNCFLAG_CONSTRUCTOR)) && no_ret)
		{
			switch(*ZScript::lookupOption(*scope, CompileOption::OPT_ON_MISSING_RETURN)/10000)
			{
				case 0: //No warn
					break;
				case 3: //Warn
					handleError(CompileError::MissingReturnWarn(node, node->getName()));
					break;
				default: //Error
					handleError(CompileError::MissingReturnError(node, node->getName()));
					node->disable();
					missing_ret = true;
					break;
			}
		}
	}
}

void ReturnVisitor::caseBlock(ASTBlock& host, void* param)
{
	VisitNode* paramNode = (VisitNode*)param;
	paramNode = paramNode->create(&host);
	
	block_retvisit_vec(host.statements, paramNode);
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
	RecursiveVisitor::caseStmtReturn(host, param);
	do_return(host, param);
	markReachable(host);
}

void ReturnVisitor::caseStmtReturnVal(ASTStmtReturnVal& host, void* param)
{
    RecursiveVisitor::caseStmtReturnVal(host, param);
	
	do_return(host, param);
	markReachable(host);
}

void ReturnVisitor::caseStmtBreak(ASTStmtBreak& host, void* param)
{
	RecursiveVisitor::caseStmtBreak(host, param);
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
	RecursiveVisitor::caseStmtContinue(host, param);
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
	
	optional<int32_t> val;
	if(host.isDecl())
		val = host.declaration->getInitializer()->getCompileTimeValue(this, scope);
	else val = host.condition->getCompileTimeValue(this, scope);
	if(val && host.isInverted())
		val = *val ? 0 : 1;
	if(val && *val) //always true can terminate
		;
	else //'if' without 'else' never terminates
	{
		paramNode = paramNode->create(&host);
		paramNode->force_term(false);
	}
	
	if(host.isDecl())
		visit(host.declaration.get(), paramNode);
	visit(host.condition.get(), paramNode);
	if(!val || *val) //variable or constant true
		visit(host.thenStatement.get(), paramNode);
	markReachable(host);
}

void ReturnVisitor::caseStmtIfElse(ASTStmtIfElse& host, void* param)
{
	VisitNode* paramNode = (VisitNode*)param;
	
	optional<int32_t> val;
	if(host.isDecl())
		val = host.declaration->getInitializer()->getCompileTimeValue(this, scope);
	else val = host.condition->getCompileTimeValue(this, scope);
	if(val && host.isInverted())
		val = *val ? 0 : 1;
	
	paramNode = paramNode->create(&host);
	paramNode->mark_branch(); // is a branch of its' 2 child nodes
	
	if(host.isDecl())
		visit(host.declaration.get(), paramNode);
	visit(host.condition.get(), paramNode);
	
	if(!val || *val) //variable or constant true
		visit(host.thenStatement.get(), paramNode->create(host.thenStatement.get()));
	if(!val || !*val) //variable or constant false
		visit(host.elseStatement.get(), paramNode->create(host.elseStatement.get()));
	markReachable(host);
}

void ReturnVisitor::caseStmtFor(ASTStmtFor& host, void* param)
{
	VisitNode* paramNode = (VisitNode*)param;
	paramNode = paramNode->create(&host);
	paramNode->mark_branch(); // is a branch of its' 2 child nodes
	VisitNode* thenNode = paramNode->create(host.body.get());
	
	auto val = host.test->getCompileTimeValue(this, scope);
	bool infloop = (val && *val);
	
	visit(host.setup.get(), paramNode);
	visit(host.test.get(), paramNode);
	visit_vec(host.increments, paramNode);
	
	if(!val || *val)
		host.ends_loop = block_retvisit(host.body.get(), thenNode);
	
	//if a break/continue moved out of it, don't count it as a terminator
	if(paramNode->get_flag(VNODE_FLAG_EXITED))
		paramNode->force_term(false);
	//an infinite loop that can return can count as a terminator
	else if(infloop) // && thenNode->get_flag(VNODE_FLAG_HASRETURN)
		paramNode->force_term(true);
	if(!infloop)
	{
		VisitNode* elseNode = paramNode->create(host.elseBlock.get());
		if(host.hasElse())
			host.ends_else = block_retvisit(host.elseBlock.get(), elseNode);
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
	host.ends_loop = block_retvisit(host.body.get(), thenNode);
	if(host.indxdecl)
	{
		visit(host.indxdecl.get(), thenNode);
		if(auto varptr = dynamic_cast<Variable*>(host.indxdecl->manager))
		{
			auto& vmap = var_map[in_func];
			auto it = vmap.find(varptr);
			if(it != vmap.end())
				it->second = true; // Mark param as used
		}
	}
	if(host.arrdecl)
	{
		visit(host.arrdecl.get(), thenNode);
		if(auto varptr = dynamic_cast<Variable*>(host.arrdecl->manager))
		{
			auto& vmap = var_map[in_func];
			auto it = vmap.find(varptr);
			if(it != vmap.end())
				it->second = true; // Mark param as used
		}
	}
	if(host.decl)
	{
		visit(host.decl.get(), thenNode);
		if(auto varptr = dynamic_cast<Variable*>(host.decl->manager))
		{
			auto& vmap = var_map[in_func];
			auto it = vmap.find(varptr);
			if(it != vmap.end())
				it->second = true; // Mark param as used
		}
	}
	if(host.hasElse())
		host.ends_else = block_retvisit(host.elseBlock.get(), elseNode);
	markReachable(host);
}

void ReturnVisitor::caseStmtRangeLoop(ASTStmtRangeLoop& host, void* param)
{
	VisitNode* paramNode = (VisitNode*)param;
	paramNode = paramNode->create(&host);
	paramNode->mark_branch(); // is a branch of its' 2 child nodes
	VisitNode* thenNode = paramNode->create(host.body.get());
	VisitNode* elseNode = paramNode->create(host.elseBlock.get());
	
	visit(host.type.get(), thenNode);
	visit(host.range.get(), thenNode);
	visit(host.increment.get(), thenNode);
	host.ends_loop = block_retvisit(host.body.get(), thenNode);
	if(host.decl)
	{
		visit(host.decl.get(), thenNode);
		if(auto varptr = dynamic_cast<Variable*>(host.decl->manager))
		{
			auto& vmap = var_map[in_func];
			auto it = vmap.find(varptr);
			if(it != vmap.end())
				it->second = true; // Mark param as used
		}
	}
	if(host.hasElse())
		host.ends_else = block_retvisit(host.elseBlock.get(), elseNode);
	markReachable(host);
}

void ReturnVisitor::caseStmtWhile(ASTStmtWhile& host, void* param)
{
	VisitNode* paramNode = (VisitNode*)param;
	paramNode = paramNode->create(&host);
	paramNode->mark_branch(); // is a branch of its' 2 child nodes
	VisitNode* thenNode = paramNode->create(host.body.get());
	
	auto val = host.test->getCompileTimeValue(this, scope);
	bool inv = host.isInverted();
	bool infloop = (val && (inv == !*val));
	
	visit(host.test.get(), thenNode);
	if(!val || (inv == !*val)) //Never runs, block unreachable
		host.ends_loop = block_retvisit(host.body.get(), thenNode);
	
	//if a break/continue moved out of it, don't count it as a terminator
	if(paramNode->get_flag(VNODE_FLAG_EXITED))
		paramNode->force_term(false);
	//an infinite loop can count as a terminator
	else if(infloop) // && thenNode->get_flag(VNODE_FLAG_HASRETURN)
		paramNode->force_term(true);
	if(!infloop)
	{
		VisitNode* elseNode = paramNode->create(host.elseBlock.get());
		if(host.hasElse())
			host.ends_else = block_retvisit(host.elseBlock.get(), elseNode);
	}
	markReachable(host);
}

void ReturnVisitor::caseStmtDo(ASTStmtDo& host, void* param)
{
	VisitNode* paramNode = (VisitNode*)param;
	paramNode = paramNode->create(&host);
	paramNode->mark_branch(); // is a branch of its' 2 child nodes
	VisitNode* thenNode = paramNode->create(host.body.get());
	
	auto val = host.test->getCompileTimeValue(this, scope);
	bool inv = host.isInverted();
	bool infloop = (val && (inv == !*val));
	
	host.ends_loop = block_retvisit(host.body.get(), thenNode);
	if(!val || *val)
		visit(host.test.get(), thenNode);
	//if a break/continue moved out of it, don't count it as a terminator
	if(paramNode->get_flag(VNODE_FLAG_EXITED))
		paramNode->force_term(false);
	//an infinite loop that can return can count as a terminator
	else if(infloop) // && thenNode->get_flag(VNODE_FLAG_HASRETURN)
		paramNode->force_term(true);
	if(!infloop)
	{
		VisitNode* elseNode = paramNode->create(host.elseBlock.get());
		if(host.hasElse())
			host.ends_else = block_retvisit(host.elseBlock.get(), elseNode);
	}
	markReachable(host);
}

void ReturnVisitor::caseStmtSwitch(ASTStmtSwitch& host, void* param)
{
	VisitNode* paramNode = (VisitNode*)param;
	paramNode = paramNode->create(&host);
	//is NOT a branch of its' child nodes, due to fallthrough
	
	visit(host.key.get(), paramNode);
	
	auto opt_vec = host.getCompileTimeCases(this, scope);
	if(opt_vec) //compile-time constant optimizations
	{
		auto vec = *opt_vec;
		//continue from the start to first terminator, respecting fallthrough
		block_retvisit_vec(vec, paramNode);
	}
	else block_visit_vec(host.cases, paramNode);
	
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
	
	block_visit_vec(host.ranges, paramNode);
	block_visit_vec(host.cases, paramNode);
	visit(host.block.get(), paramNode);
	markReachable(host);
}

void ReturnVisitor::caseExprCall(ASTExprCall& host, void* param)
{
	if(host.left->isTypeArrow())
		visit(host.left.get(), param);
	visit_vec(host.parameters, param);
	auto& func = *host.binding;
	bool exiting_call = func.getFlag(FUNCFLAG_EXITS)
	//This should work, but, FUNCFLAG_NEVER_RETURN won't be set for all
	// functions yet as it gets set DURING this pass... -Em
		|| func.getFlag(FUNCFLAG_NEVER_RETURN);
	if(exiting_call)
	{
		VisitNode* paramNode = (VisitNode*)param;
		paramNode = paramNode->create(&host);
		paramNode->force_term(true);
	}
	markReachable(host);
}

void ReturnVisitor::caseDataDecl(ASTDataDecl& host, void* param)
{
	RecursiveVisitor::caseDataDecl(host, param);
	Datum* ptr = host.manager;
	Variable* varptr = dynamic_cast<Variable*>(ptr);
	if(in_func && varptr && ptr->getNode() && !ptr->getGlobalId())
		var_map[in_func][varptr] = false;
}
void ReturnVisitor::caseExprIdentifier(ASTExprIdentifier& host, void* param)
{
	RecursiveVisitor::caseExprIdentifier(host, param);
	Datum* ptr = host.binding;
	Variable* varptr = dynamic_cast<Variable*>(ptr);
	if(varptr && in_func)
	{
		auto& vmap = var_map[in_func];
		auto it = vmap.find(varptr);
		if(it != vmap.end())
			it->second = true; // Mark param as used
	}
}

//Helper Functions
bool ReturnVisitor::reachable(AST& node) const
{
	return node.reachable();
}

