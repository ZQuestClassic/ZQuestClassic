/**
 * Code path handler, to check for unreachable code / missing 'return' statements
 * Begun: 12th Dec, 2023
 * Author: Emily
 */

#ifndef RETURNVISITOR_H
#define RETURNVISITOR_H

#include "ASTVisitors.h"
#include "DataStructs.h"
#include "ZScript.h"

namespace ZScript
{
	#define VNODE_FLAG_BREAK_TO       0x01
	#define VNODE_FLAG_BREAK_PAST     0x02
	#define VNODE_FLAG_CONTINUE_TO    0x04
	#define VNODE_FLAG_CONTINUE_PAST  0x08
	#define VNODE_FLAG_BRANCH         0x10
	#define VNODE_FLAG_HASRETURN      0x20
	
	#define VNODE_FLAG_BREAK     (VNODE_FLAG_BREAK_TO|VNODE_FLAG_BREAK_PAST)
	#define VNODE_FLAG_CONTINUE  (VNODE_FLAG_CONTINUE_TO|VNODE_FLAG_CONTINUE_PAST)
	#define VNODE_FLAG_CONTBREAK (VNODE_FLAG_BREAK|VNODE_FLAG_CONTINUE)
	#define VNODE_FLAG_EXITED    (VNODE_FLAG_BREAK|VNODE_FLAG_CONTINUE_PAST)
	struct VisitNode
	{
		AST* node;
		uint8_t flags;
		VisitNode* parent;
		vector<std::shared_ptr<VisitNode>> children;
		optional<bool> terminated;
		
		VisitNode(AST* host, VisitNode* parent = nullptr)
			: node(host), parent(parent), flags(0)
		{}
		VisitNode* create(AST* node)
		{
			children.emplace_back(std::make_shared<VisitNode>(node, this));
			return children.back().get();
		}
		
		bool is_continue_node() const
		{
			if(dynamic_cast<ASTStmtFor*>(node))
				return true;
			if(dynamic_cast<ASTStmtForEach*>(node))
				return true;
			if(dynamic_cast<ASTStmtWhile*>(node))
				return true;
			if(dynamic_cast<ASTStmtDo*>(node))
				return true;
			return false;
		}
		bool is_break_node() const
		{
			if(dynamic_cast<ASTStmtSwitch*>(node))
				return true;
			return is_continue_node();
		}
		bool terminates()
		{
			if(!terminated)
				check_terminate();
			return *terminated;
		}
		bool check_terminate()
		{
			bool do_term = true;
			if(flags & VNODE_FLAG_BRANCH) //noterm if any child doesn't terminate
			{
				for(auto child : children)
				{
					if(!child->terminates())
					{
						do_term = false;
						break;
					}
				}
			}
			else //noterm if EVERY child doesn't terminate
			{
				do_term = false;
				for(auto child : children)
				{
					if(child->terminates())
					{
						do_term = true;
						break;
					}
				}
			}
			terminated = do_term;
			return do_term;
		}
		bool check_terminate(size_t since)
		{
			bool do_term = true;
			if(flags & VNODE_FLAG_BRANCH) //noterm if any child doesn't terminate
			{
				if(terminated)
					do_term = *terminated;
				for(auto q = since; q < children.size(); ++q)
				{
					if(!children[q]->terminates())
					{
						do_term = false;
						break;
					}
				}
			}
			else //noterm if EVERY child doesn't terminate
			{
				do_term = false;
				if(terminated)
					do_term = *terminated;
				for(auto q = since; q < children.size(); ++q)
				{
					if(children[q]->terminates())
					{
						do_term = true;
						break;
					}
				}
			}
			terminated = do_term;
			return do_term;
		}
		size_t child_index() const
		{
			return children.size();
		}
		bool get_flag(int flag) const
		{
			return flags&flag;
		}
		
		void do_break(uint count = 1)
		{
			if(count < 1) return;
			VisitNode* curr = this;
			while(count)
			{
				curr = curr->parent;
				if(!curr) return;
				if(curr->is_break_node())
				{
					if(--count)
						curr->flags |= VNODE_FLAG_BREAK_PAST;
				}
			}
			curr->flags |= VNODE_FLAG_BREAK_TO;
		}
		void do_continue(uint count = 1)
		{
			if(count < 1) return;
			VisitNode* curr = this;
			while(count)
			{
				curr = curr->parent;
				if(!curr) return;
				if(curr->is_continue_node())
				{
					if(--count)
						curr->flags |= VNODE_FLAG_CONTINUE_PAST;
				}
			}
			curr->flags |= VNODE_FLAG_CONTINUE_TO;
		}
		void force_term(bool term)
		{
			terminated = term;
		}
		void mark_branch(bool branch = true)
		{
			SETFLAG(flags, VNODE_FLAG_BRANCH, branch);
		}
	};
	class ReturnVisitor : public RecursiveVisitor
	{
	public:
		ReturnVisitor(ZScript::Program& program);
		using RecursiveVisitor::visit;
		void visit(AST& node, void* param = NULL);
		////////////////
		// Cases
		void caseDefault(AST& host, void* param = NULL);
		void caseBlock(ASTBlock& host, void* param = NULL);
		void caseStmtReturn(ASTStmtReturn& host, void* param = NULL);
		void caseStmtReturnVal(ASTStmtReturnVal& host, void* param = NULL);
		void caseStmtBreak(ASTStmtBreak &host, void* param = NULL);
		void caseStmtContinue(ASTStmtContinue &host, void* param = NULL);
		void caseStmtIf(ASTStmtIf& host, void* param = NULL);
		void caseStmtIfElse(ASTStmtIfElse& host, void* param = NULL);
		void caseStmtFor(ASTStmtFor& host, void* param = NULL);
		void caseStmtForEach(ASTStmtForEach& host, void* param = NULL);
		void caseStmtWhile(ASTStmtWhile& host, void* param = NULL);
		void caseStmtDo(ASTStmtDo& host, void* param = NULL);
		void caseStmtSwitch(ASTStmtSwitch& host, void* param = NULL);
		void caseSwitchCases(ASTSwitchCases & host, void* param = NULL);
		//functions
		void caseExprCall(ASTExprCall& host, void* param = NULL);
		//internals
		virtual void analyzeFunctionInternals(ZScript::Function& function);
		
		template <class Container>
		bool block_retvisit(AST& host, Container const& nodes, void* param = NULL);
		bool block_retvisit(AST& host, void* param = NULL);
		bool block_retvisit(AST* host, void* param = NULL)
		{
			if(!host) return false;
			return block_retvisit(*host, param);
		}
	protected:
		void markReachable(AST& host)
		{
			host.mark_reachable();
		}
		bool reachable(AST& node) const;
	private:
		ZScript::Program& program;
		bool extra_pass, marked_never_ret, missing_ret;
		
		void analyzeUnaryExpr(ASTUnaryExpr& host);
		void analyzeBinaryExpr(ASTBinaryExpr& host);
	};
}

#endif

