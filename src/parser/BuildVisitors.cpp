#include <assert.h>
#include <memory>

#include "BuildVisitors.h"
#include "CompileError.h"
#include "Types.h"
#include "ZScript.h"
#include "parser/AST.h"
#include "parser/ASTVisitors.h"
#include "parser/ByteCode.h"
#include "parser/Scope.h"
#include "parser/Opcode.h"
#include "parser/parserDefs.h"

using namespace ZScript;
using std::map;
using std::pair;
using std::string;
using std::vector;
using std::list;
using std::shared_ptr;

#define CONST_VAL(val) addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(val)));


#define INITC_STORE() \
int32_t initIndex = result.size(); \
OpcodeContext initContext(((OpcodeContext*)param)->typeStore)

#define INITCTX ((void*)&initContext)

#define INITC_INIT() \
do \
	result.insert(result.begin() + initIndex, initContext.initCode.begin(), initContext.initCode.end()); \
while(false)

#define INITC_CTXT ((void*)&initContext)


#define VISIT_USEVAL(...) \
{ \
	auto tmpb = sidefx_only; \
	sidefx_only = false; \
	visit(__VA_ARGS__); \
	sidefx_only = tmpb; \
}

#define SIDEFX_CHECK(...) \
do { \
	if(sidefx_only) \
	{ \
		vector<ASTExpr*> vec = {__VA_ARGS__}; \
		sidefx_visit_vec(vec, param); \
		return; \
	} \
} while(false)

#define SIDEFX_BINOP() SIDEFX_CHECK(host.left.get(), host.right.get())
#define SIDEFX_UNOP() SIDEFX_CHECK(host.operand.get())

class MiniStackMgr
{
public:
	vector<uint> peekinds;
	uint push()
	{
		for(uint& q : peekinds)
			++q;
		peekinds.push_back(0);
		return peekinds.size()-1;
	}
	uint at(uint ind)
	{
		return peekinds.at(ind);
	}
	bool pop()
	{
		if(peekinds.empty())
			return false;
		peekinds.pop_back();
		for(uint& q : peekinds)
			--q;
		return true;
	}
	uint pop_all()
	{
		uint sz = peekinds.size();
		peekinds.clear();
		return sz;
	}
	uint count()
	{
		return peekinds.size();
	}
};
#define VISIT_PUSH(node, ind) \
do { \
	visit(node, param); \
	addOpcode(new OPushRegister(new VarArgument(EXP1))); \
	ind = mgr.push(); \
} while(false)

/////////////////////////////////////////////////////////////////////////////////
// BuildOpcodes

BuildOpcodes::BuildOpcodes(Program& program)
	: RecursiveVisitor(program), returnlabelid(-1), continuelabelids(),
	  breaklabelids(), breakScopes(), break_to_counts(),
	  cur_scopes(), break_depth(0), continue_depth(0)
{
	opcodeTargets.push_back(&result);
}
BuildOpcodes::BuildOpcodes(Program& program, Scope* curScope)
	: BuildOpcodes(program)
{
	scope = curScope;
}

BuildOpcodes::BuildOpcodes(Program& program, LValBOHelper* helper)
	: BuildOpcodes(program)
{
	scope = helper->scope;
	parsing_user_class = helper->parsing_user_class;
	in_func = helper->in_func;
}

void addOpcode2(vector<shared_ptr<Opcode>>& v, Opcode* code)
{
	shared_ptr<Opcode> op(code);
	v.push_back(op);
}

void BuildOpcodes::visit(AST& node, void* param)
{
	if(node.isDisabled()) return; //Don't visit disabled nodes.
	if(!node.reachable()) return; //Don't visit unreachable nodes for ZASM generation
	if(sidefx_only) return sidefx_visit(node, param);
	RecursiveVisitor::visit(node, param);
	for (auto it = node.compileErrorCatches.cbegin(); it != node.compileErrorCatches.cend(); ++it)
	{
		ASTExprConst& idNode = **it;
		auto errorId = idNode.getCompileTimeValue(this, scope);
		assert(errorId);
		handleError(CompileError::MissingCompileError(&node, int32_t(*errorId / 10000L)));
	}
}

void BuildOpcodes::literal_visit(AST& node, void* param)
{
	if(node.isDisabled()) return; //Don't visit disabled nodes.
	if(!node.reachable()) return; //Don't visit unreachable nodes for ZASM generation
	int32_t initIndex = result.size();
	OpcodeContext *parentContext = (OpcodeContext*)param;
	OpcodeContext prm(parentContext->typeStore);
	OpcodeContext *c = &prm;
	visit(node, (void*)c);
	//Handle literals
	result.insert(result.begin() + initIndex, c->initCode.begin(), c->initCode.end());
}

void BuildOpcodes::literal_visit(AST* node, void* param)
{
	if(node) literal_visit(*node, param);
}

template <class Container>
void BuildOpcodes::literal_visit_vec(Container const& nodes, void* param)
{
	for (auto it = nodes.cbegin();
		 it != nodes.cend(); ++it)
	{
		failure_temp = false;
		literal_visit(**it, param);
		if(failure_halt) return;
	}
}

void BuildOpcodes::sidefx_visit(AST& node, void* param)
{
	if(node.isDisabled()) return; //Don't visit disabled nodes.
	if(!node.reachable()) return; //Don't visit unreachable nodes for ZASM generation
	
	auto tmpb = sidefx_only;
	
	//Special side-effects checks
	if(ASTExpr* expr = dynamic_cast<ASTExpr*>(&node))
	{
		if(expr->getCompileTimeValue(this, scope))
			return; //compile-constants have no side-effects
		sidefx_only = true;
	}
	
	RecursiveVisitor::visit(node, param);
	for (auto it = node.compileErrorCatches.cbegin(); it != node.compileErrorCatches.cend(); ++it)
	{
		ASTExprConst& idNode = **it;
		auto errorId = idNode.getCompileTimeValue(this, scope);
		assert(errorId);
		handleError(CompileError::MissingCompileError(&node, int32_t(*errorId / 10000L)));
	}
	sidefx_only = tmpb;
}

void BuildOpcodes::sidefx_visit(AST* node, void* param)
{
	if(node) sidefx_visit(*node, param);
}

template <class Container>
void BuildOpcodes::sidefx_visit_vec(Container const& nodes, void* param)
{
	for (auto it = nodes.cbegin();
		 it != nodes.cend(); ++it)
	{
		failure_temp = false;
		sidefx_visit(**it, param);
		if(failure_halt) return;
	}
}

void BuildOpcodes::caseDefault(AST&, void*)
{
	// Unreachable
	assert(false);
}

void BuildOpcodes::addOpcode(Opcode* code)
{
	std::shared_ptr<Opcode> op(code);
	opcodeTargets.back()->push_back(op);
}

void BuildOpcodes::addOpcode(std::shared_ptr<Opcode> &code)
{
	opcodeTargets.back()->push_back(code);
}
Opcode* BuildOpcodes::backOpcode()
{
	return opcodeTargets.back()->back().get();
}
vector<std::shared_ptr<Opcode>>& BuildOpcodes::backTarget()
{
	return *opcodeTargets.back();
}
size_t BuildOpcodes::commentTarget() const
{
	return opcodeTargets.back()->size();
}
void BuildOpcodes::commentAt(size_t indx, string const& comment)
{
	auto targ = backTarget();
	if(indx >= targ.size())
		return;
	targ[indx]->mergeComment(comment);
}
void BuildOpcodes::commentBack(string const& comment)
{
	backOpcode()->mergeComment(comment);
}
void BuildOpcodes::commentStartEnd(size_t indx, string const& comment)
{
	auto sz = commentTarget();
	if(sz > indx)
	{
		if(sz == indx+1)
			commentBack(comment);
		else
		{
			commentAt(indx, fmt::format("{} Start",comment));
			commentBack(fmt::format("{} End",comment));
		}
	}
}

template <class Container>
void BuildOpcodes::addOpcodes(Container const& container)
{
	for (auto ptr: container)
		addOpcode(ptr);
}

void BuildOpcodes::caseSetOption(ASTSetOption&, void*)
{
	// Do nothing, not even recurse.
}

void BuildOpcodes::caseUsing(ASTUsingDecl& host, void*)
{
	// Do nothing, not even recurse.
}

// Statements

void BuildOpcodes::caseBlock(ASTBlock &host, void *param)
{
	if(!host.getScope())
	{
		host.setScope(scope->makeChild());
	}

	ScopeReverter sr(&scope);
	scope = host.getScope();

	cur_scopes.push_back(scope);

	for (auto it = host.statements.begin(); it != host.statements.end(); ++it)
		literal_visit(*it, param);

	cur_scopes.pop_back();

	if (host.getScope() != scope)
		throw compile_exception("host.getScope() != scope");

	mark_ref_remove_if_needed_for_block(&host);
}

void BuildOpcodes::caseStmtIf(ASTStmtIf &host, void *param)
{
	uint ifid = host.get_comment_id();
	bool inv = host.isInverted();
	string ifstr = inv ? "unless" : "if",
		truestr = inv ? "false" : "true",
		falsestr = inv ? "true" : "false";
	if(host.isDecl())
	{
		string const& declname = host.declaration->getName();
		if(!host.getScope())
		{
			host.setScope(scope->makeChild());
		}
		ScopeReverter sr(&scope);
		scope = host.getScope();

		cur_scopes.push_back(scope);
		
		if(auto val = host.declaration->getInitializer()->getCompileTimeValue(this, scope))
		{
			if((host.isInverted()) == (*val==0)) //True, so go straight to the 'then'
			{
				auto targ_sz = commentTarget();
				literal_visit(host.declaration.get(), param);
				commentAt(targ_sz, fmt::format("{}({}={}) #{} [Opt:AlwaysOn]",ifstr,declname,truestr,ifid));
				visit(host.thenStatement.get(), param);
			} //Either true or false, it's constant, so no checks required.

			cur_scopes.pop_back();
			return;
		}
		
		int32_t endif = ScriptParser::getUniqueLabelID();
		auto targ_sz = commentTarget();
		literal_visit(host.declaration.get(), param);
		commentAt(targ_sz, fmt::format("{}({}) #{} Decl",ifstr,declname,ifid));
		
		//The condition should be reading the value just processed from the initializer
		targ_sz = commentTarget();
		visit(host.condition.get(), param);
		commentAt(targ_sz, fmt::format("{}({}) #{} Test",ifstr,declname,ifid));
		//
		addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
		if(inv)
		{
			addOpcode(new OGotoFalseImmediate(new LabelArgument(endif)));
			commentBack("Test 'unless'");
		}
		else
		{
			addOpcode(new OGotoTrueImmediate(new LabelArgument(endif)));
			commentBack("Test 'if'");
		}
		targ_sz = commentTarget();
		visit(host.thenStatement.get(), param);
		commentStartEnd(targ_sz, fmt::format("{}({}) #{} Body",ifstr,declname,ifid));
		mark_ref_remove_if_needed_for_scope(host.getScope());
		//nop
		addOpcode(new ONoOp(endif));

		cur_scopes.pop_back();
	}
	else
	{
		ScopeReverter sr(&scope);

		if(auto val = host.condition->getCompileTimeValue(this, scope))
		{
			if((inv) == (*val==0)) //True, so go straight to the 'then'
			{
				auto targ_sz = commentTarget();
				visit(host.thenStatement.get(), param);
				commentAt(targ_sz, fmt::format("{}({}) #{} [Opt:AlwaysOn]",ifstr,truestr,ifid));
			} //Either true or false, it's constant, so no checks required.
			return;
		}

		//run the test
		auto targ_sz = commentTarget();
		literal_visit(host.condition.get(), param);
		commentAt(targ_sz, fmt::format("{}() #{} Test",ifstr,ifid));
		//Continue
		int32_t endif = ScriptParser::getUniqueLabelID();
		addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
		if(inv)
		{
			addOpcode(new OGotoFalseImmediate(new LabelArgument(endif)));
			commentBack("Test 'unless'");
		}
		else
		{
			addOpcode(new OGotoTrueImmediate(new LabelArgument(endif)));
			commentBack("Test 'if'");
		}
		//run the block
		targ_sz = commentTarget();
		visit(host.thenStatement.get(), param);
		commentStartEnd(targ_sz, fmt::format("{}() #{} Body",ifstr,ifid));
		//nop
		addOpcode(new ONoOp(endif));
	}
}

void BuildOpcodes::caseStmtIfElse(ASTStmtIfElse &host, void *param)
{
	uint ifid = host.get_comment_id();
	bool inv = host.isInverted();
	string ifstr = inv ? "unless" : "if",
		truestr = inv ? "false" : "true",
		falsestr = inv ? "true" : "false";
	if(host.isDecl())
	{
		string const& declname = host.declaration->getName();
		if(!host.getScope())
		{
			host.setScope(scope->makeChild());
		}
		ScopeReverter sr(&scope);
		scope = host.getScope();

		cur_scopes.push_back(scope);
		
		if(auto val = host.declaration->getInitializer()->getCompileTimeValue(this, scope))
		{
			if((host.isInverted()) == (*val==0)) //True, so go straight to the 'then'
			{
				auto targ_sz = commentTarget();
				literal_visit(host.declaration.get(), param);
				commentAt(targ_sz, fmt::format("{}({}={}) #{} [Opt:AlwaysOn]",ifstr,declname,truestr,ifid));
				visit(host.thenStatement.get(), param);
			}
			else //False, so go straight to the 'else'
			{
				auto targ_sz = commentTarget();
				visit(host.elseStatement.get(), param);
				commentStartEnd(targ_sz, fmt::format("{}({}={}) #{} Else [Opt:AlwaysOff]",ifstr,declname,falsestr,ifid));
			}
			//Either way, ignore the rest and return.
			cur_scopes.pop_back();
			return;
		}
		
		int32_t elseif = ScriptParser::getUniqueLabelID();;
		int32_t endif = ScriptParser::getUniqueLabelID();
		auto targ_sz = commentTarget();
		literal_visit(host.declaration.get(), param);
		commentAt(targ_sz, fmt::format("{}({}) #{} Decl",ifstr,declname,ifid));
		
		//The condition should be reading the value just processed from the initializer
		targ_sz = commentTarget();
		visit(host.condition.get(), param);
		commentAt(targ_sz, fmt::format("{}({}) #{} Test",ifstr,declname,ifid));
		//
		addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
		if(inv)
		{
			addOpcode(new OGotoFalseImmediate(new LabelArgument(elseif)));
			commentBack("Test 'unless'");
		}
		else
		{
			addOpcode(new OGotoTrueImmediate(new LabelArgument(elseif)));
			commentBack("Test 'if'");
		}
		
		targ_sz = commentTarget();
		visit(host.thenStatement.get(), param);
		addOpcode(new OGotoImmediate(new LabelArgument(endif)));
		//nop
		addOpcode(new ONoOp(elseif));
		
		scope = scope->getParent();
		
		commentStartEnd(targ_sz, fmt::format("{}({}) #{} Body",ifstr,declname,ifid));
		targ_sz = commentTarget();
		visit(host.elseStatement.get(), param);
		commentStartEnd(targ_sz, fmt::format("{}({}) #{} Else",ifstr,declname,ifid));
		
		addOpcode(new ONoOp(endif));

		cur_scopes.pop_back();
	}
	else
	{
		ScopeReverter sr(&scope);

		if(auto val = host.condition->getCompileTimeValue(this, scope))
		{
			auto targ_sz = commentTarget();
			if((host.isInverted()) == (*val==0)) //True, so go straight to the 'then'
			{
				visit(host.thenStatement.get(), param);
				commentAt(targ_sz, fmt::format("{}({}) #{} [Opt:AlwaysOn]",ifstr,truestr,ifid));
			}
			else //False, so go straight to the 'else'
			{
				visit(host.elseStatement.get(), param);
				commentAt(targ_sz, fmt::format("{}({}) #{} [Opt:AlwaysOff]",ifstr,falsestr,ifid));
			}
			//Either way, ignore the rest and return.
			return;
		}
		//run the test
		auto targ_sz = commentTarget();
		literal_visit(host.condition.get(), param);
		commentAt(targ_sz, fmt::format("{}() #{} Test",ifstr,ifid));
		//Continue
		int32_t elseif = ScriptParser::getUniqueLabelID();
		int32_t endif = ScriptParser::getUniqueLabelID();
		addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
		if(inv)
		{
			addOpcode(new OGotoFalseImmediate(new LabelArgument(elseif)));
			commentBack("Test 'unless'");
		}
		else
		{
			addOpcode(new OGotoTrueImmediate(new LabelArgument(elseif)));
			commentBack("Test 'if'");
		}
		//run if block
		targ_sz = commentTarget();
		visit(host.thenStatement.get(), param);
		addOpcode(new OGotoImmediate(new LabelArgument(endif)));
		commentStartEnd(targ_sz, fmt::format("{}() #{} Body",ifstr,ifid));
		addOpcode(new ONoOp(elseif));

		targ_sz = commentTarget();
		visit(host.elseStatement.get(), param);
		commentStartEnd(targ_sz, fmt::format("{}() #{} Else",ifstr,ifid));
		addOpcode(new ONoOp(endif));
	}
}

template<typename T>
static std::vector<Scope*> getBreakableScopesForStatement(T* node)
{
	if (auto block = dynamic_cast<ASTBlock*>(node->body.get()))
		return {block->getScope()};

	return {};
}

static std::vector<Scope*> getBreakableScopesForStatement(ASTStmtSwitch* node)
{
	std::vector<Scope*> blocks;

	for (auto case_node : node->cases)
		blocks.push_back(case_node->block.get()->getScope());

	return blocks;
}

#define OPT_STR(v) (v?to_string(*v):"nullopt")
void BuildOpcodes::caseStmtSwitch(ASTStmtSwitch &host, void* param)
{
	if(host.isString)
	{
		caseStmtStrSwitch(host, param);
		return;
	}
	
	map<ASTSwitchCases*, int32_t> labels;
	vector<ASTSwitchCases*> cases = host.cases.data();
	
	uint switchid = host.get_comment_id();
	int32_t end_label = ScriptParser::getUniqueLabelID();
	auto default_label = end_label;
	
	// save and override break label.
	push_break(end_label, 0, getBreakableScopesForStatement(&host));
	
	// Evaluate the key.
	auto keyval = host.key->getCompileTimeValue(this, scope);
	if(!keyval)
	{
		auto targ_sz = commentTarget();
		literal_visit(host.key.get(), param);
		commentAt(targ_sz, fmt::format("switch() #{} Key", switchid));
	}
	
	if(cases.size() == 1 && cases.back()->isDefault) //Only default case
	{
		auto targ_sz = commentTarget();
		visit(cases.back()->block.get(), param);
		commentAt(targ_sz, fmt::format("switch() #{} Default [Opt:DefaultOnly]", switchid));
		// Add ending label, for 'break;'
		addOpcode(new ONoOp(end_label));
		return;
	}
	
	//Continue
	
	bool needsEndLabel = true;
	auto opt_vec = host.getCompileTimeCases(this, scope);
	if(keyval && opt_vec)
	{
		auto vec = *opt_vec;
		
		if(vec.size())
		{
			auto targ_sz = commentTarget();
			
			for(auto swcase : vec)
				visit(swcase->block.get(), param);
			
			commentAt(targ_sz, fmt::format("switch({}) #{} [Opt:ConstVal]", *keyval, switchid));
		}
		else needsEndLabel = false;
	}
	else
	{
		addOpcode(new OSetRegister(new VarArgument(SWITCHKEY), new VarArgument(EXP1)));
		commentBack("Store key");

		// Add the tests and jumps.
		for (auto it = cases.begin(); it != cases.end(); ++it)
		{
			ASTSwitchCases* cases = *it;

			// Make the target label.
			int32_t label = ScriptParser::getUniqueLabelID();
			labels[cases] = label;

			// Run the tests for these cases.
			for (auto it = cases->cases.begin(); it != cases->cases.end(); ++it)
			{
				auto val = (*it)->getCompileTimeValue(this, scope);
				assert(val); //ASTExprConst
				// Test this individual case.
				addOpcode(new OCompareImmediate(new VarArgument(SWITCHKEY), new LiteralArgument(*val)));
				// If the test succeeds, jump to its label.
				addOpcode(new OGotoTrueImmediate(new LabelArgument(label)));
				commentBack(fmt::format("case '{}'", OPT_STR(val)));
			}
			for (auto it = cases->ranges.begin(); it != cases->ranges.end(); ++it)
			{
				ASTRange& range = **it;
				int32_t skipLabel = ScriptParser::getUniqueLabelID();
				//Test each full range
				auto startval = (*range.start).getCompileTimeValue(this, scope);
				auto endval = (*range.end).getCompileTimeValue(this, scope);
				assert(startval && endval); //switch-case uses ASTExprConst ranges
				//The logic below entirely assumes the RANGE_LR (equality allowed on both sides) type
				//Convert the values if it isn't of that type...
				if(!(range.type & ASTRange::RANGE_L))
					startval = *startval+1;
				if(!(range.type & ASTRange::RANGE_R))
					endval = *endval-1;
				//Compare key to lower bound
				addOpcode(new OCompareImmediate(new VarArgument(SWITCHKEY), new LiteralArgument(*startval)));
				addOpcode(new OGotoCompare(new LabelArgument(skipLabel), new CompareArgument(CMP_LT))); //Skip if key is OUT of the bound
				commentBack(fmt::format("case '{0}...{1}', key<{0}", OPT_STR(startval), OPT_STR(endval)));
				
				//Compare key to upper bound
				addOpcode(new OCompareImmediate(new VarArgument(SWITCHKEY), new LiteralArgument(*endval)));
				addOpcode(new OGotoCompare(new LabelArgument(label), new CompareArgument(CMP_LE))); //If key is in bounds, jump to its label
				commentBack(fmt::format("case '{0}...{1}', {0}<key<{1}", OPT_STR(startval), OPT_STR(endval)));
				addOpcode(new ONoOp(skipLabel)); //add the skip label
			}

			// If this set includes the default case, mark it.
			if (cases->isDefault)
				default_label = label;
		}

		// Add direct jump to default case (or end if there isn't one.).
		addOpcode(new OGotoImmediate(new LabelArgument(default_label)));
		commentBack(fmt::format("switch() #{} NoMatch", switchid));
		
		// Add the actual code branches.
		for (auto it = cases.begin(); it != cases.end(); ++it)
		{
			ASTSwitchCases* cases = *it;
			
			// Make a nop for starting the block.
			addOpcode(new ONoOp(labels[cases]));
			commentBack("Case block");
			// Add block.
			visit(cases->block.get(), param);
		}
	}
	
	if(needsEndLabel)
		addOpcode(new ONoOp(end_label));

	// Restore break label.
	pop_break();
}

void BuildOpcodes::caseStmtStrSwitch(ASTStmtSwitch &host, void* param)
{
	map<ASTSwitchCases*, int32_t> labels;
	vector<ASTSwitchCases*> cases = host.cases.data();
	
	int32_t switchid = ScriptParser::getUniqueLabelID();
	int32_t end_label = switchid;
	int32_t default_label = end_label;
	
	// save and override break label.
	push_break(end_label, 0, getBreakableScopesForStatement(&host));
	
	// Evaluate the key.
	auto targ_sz = commentTarget();
	literal_visit(host.key.get(), param);
	commentAt(targ_sz, fmt::format("switch(\"\") #{} Key", switchid));
	
	if(cases.size() == 1 && cases.back()->isDefault) //Only default case
	{
		targ_sz = commentTarget();
		visit(cases.back()->block.get(), param);
		commentAt(targ_sz, fmt::format("switch(\"\") #{} Default [Opt:DefaultOnly]", switchid));
		// Add ending label, for 'break;'
		addOpcode(new ONoOp(end_label));
		return;
	}
	
	//Continue
	addOpcode(new OSetRegister(new VarArgument(SWITCHKEY), new VarArgument(EXP1)));
	commentBack("Store key");
	
	// Add the tests and jumps.
	for (auto it = cases.begin(); it != cases.end(); ++it)
	{
		ASTSwitchCases* cases = *it;

		// Make the target label.
		int32_t label = ScriptParser::getUniqueLabelID();
		labels[cases] = label;

		// Run the tests for these cases.
		for (auto it = cases->str_cases.begin(); it != cases->str_cases.end(); ++it)
		{
			// Test this individual case.
			//Allocate the string literal
			INITC_STORE(); //store init-related values
			visit(*it, INITCTX); //visit with the initc params
			INITC_INIT(); //initialize the literal
			
			// Compare the strings
			if(*lookupOption(*scope, CompileOption::OPT_STRING_SWITCH_CASE_INSENSITIVE))
				addOpcode(new OInternalInsensitiveStringCompare(new VarArgument(SWITCHKEY), new VarArgument(EXP1)));
			else
				addOpcode(new OInternalStringCompare(new VarArgument(SWITCHKEY), new VarArgument(EXP1)));
			commentBack(fmt::format("case \"{}\"", (*it)->value));
			
			//
			addOpcode(new OGotoTrueImmediate(new LabelArgument(label)));
		}

		// If this set includes the default case, mark it.
		if (cases->isDefault)
			default_label = label;
	}
	
	// Add direct jump to default case (or end if there isn't one.).
	addOpcode(new OGotoImmediate(new LabelArgument(default_label)));
	commentBack(fmt::format("switch(\"\") #{} NoMatch", switchid));
	
	// Add the actual code branches.
	for (auto it = cases.begin(); it != cases.end(); ++it)
	{
		ASTSwitchCases* cases = *it;

		// Make a nop for starting the block.
		addOpcode(new ONoOp(labels[cases]));
		commentBack("Case block");
		// Add block.
		visit(cases->block.get(), param);
	}
	
	// Add ending label.
	addOpcode(new ONoOp(end_label));
	
	// Restore break label.
	pop_break();
}

void BuildOpcodes::caseStmtFor(ASTStmtFor &host, void *param)
{
	if(!host.getScope())
	{
		host.setScope(scope->makeChild());
	}
	scope = host.getScope();
	cur_scopes.push_back(scope);
	auto targ_sz = commentTarget();
	//run the precondition
	literal_visit(host.setup.get(), param);

	uint forid = host.get_comment_id();
	commentAt(targ_sz, fmt::format("for() #{} setup",forid));
	//Check for a constant FALSE condition
	if(auto val = host.test->getCompileTimeValue(this, scope))
	{
		if(*val == 0) //False, so run else, restore scope, and exit
		{
			scope = scope->getParent();
			if(host.hasElse())
			{
				targ_sz = commentTarget();
				visit(host.elseBlock.get(), param);
				commentStartEnd(targ_sz, fmt::format("for() #{} Else [Opt:AlwaysFalse]",forid));
			}
			cur_scopes.pop_back();
			return;
		}
	}
	//Continue
	int32_t loopstart = ScriptParser::getUniqueLabelID();
	int32_t loopend = ScriptParser::getUniqueLabelID();
	int32_t loopincr = ScriptParser::getUniqueLabelID();
	int32_t elselabel = host.hasElse() ? ScriptParser::getUniqueLabelID() : loopend;
	
	addOpcode(new ONoOp(loopstart));
	commentBack(fmt::format("for() #{} LoopTest",forid));
	//test the termination condition
	literal_visit(host.test.get(), param);
	//Continue
	addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
	addOpcode(new OGotoTrueImmediate(new LabelArgument(elselabel)));
	commentBack(fmt::format("for() #{} TestFail?",forid));
	//run the loop body
	//save the old break and continue values

	push_break(loopend, 0, getBreakableScopesForStatement(&host));
	push_cont(loopincr);
	
	targ_sz = commentTarget();
	visit(host.body.get(), param);
	commentStartEnd(targ_sz, fmt::format("for() #{} Body",forid));
	
	pop_break();
	pop_cont();

	//run the increment
	addOpcode(new ONoOp(loopincr));
	commentBack(fmt::format("for() #{} LoopIncrement",forid));
	literal_visit_vec(host.increments, param);
	//Continue
	addOpcode(new OGotoImmediate(new LabelArgument(loopstart)));
	commentBack(fmt::format("for() #{} End",forid));
	
	scope = scope->getParent();
	
	if(host.hasElse())
	{
		addOpcode(new ONoOp(elselabel));
		targ_sz = commentTarget();
		visit(host.elseBlock.get(), param);
		commentStartEnd(targ_sz, fmt::format("for() #{} Else",forid));
	}
	
	//nop
	addOpcode(new ONoOp(loopend));

	cur_scopes.pop_back();
}

void BuildOpcodes::caseStmtForEach(ASTStmtForEach &host, void *param)
{
	//Force to sub-scope
	if(!host.getScope())
	{
		host.setScope(scope->makeChild());
	}
	scope = host.getScope();
	cur_scopes.push_back(scope);
	
	uint forid = host.get_comment_id();
	//Declare the local variable that will hold the array ptr
	auto targ_sz = commentTarget();
	literal_visit(host.arrdecl.get(), param);
	commentAt(targ_sz, fmt::format("for(each) #{} ArrDecl",forid));
	//Declare the local variable that will hold the iterator
	targ_sz = commentTarget();
	literal_visit(host.indxdecl.get(), param);
	commentAt(targ_sz, fmt::format("for(each) #{} IndxDecl",forid));
	//Declare the local variable that will hold the current loop value
	targ_sz = commentTarget();
	literal_visit(host.decl.get(), param);
	commentAt(targ_sz, fmt::format("for(each) #{} ValDecl",forid));
	
	int32_t decloffset = host.decl->manager->getStackOffset(false);
	int32_t arrdecloffset = host.arrdecl->manager->getStackOffset(false);
	int32_t indxdecloffset = host.indxdecl->manager->getStackOffset(false);
	
	int32_t loopstart = ScriptParser::getUniqueLabelID();
	int32_t loopend = ScriptParser::getUniqueLabelID();
	int32_t elselabel = host.hasElse() ? ScriptParser::getUniqueLabelID() : loopend;
	
	addOpcode(new ONoOp(loopstart));
	commentBack(fmt::format("for(each) #{} EndArrayCheck",forid));
	//Check if we've reached the end of the array
	addOpcode(new OLoad(new VarArgument(INDEX), new LiteralArgument(arrdecloffset)));
	addOpcode(new OArraySize(new VarArgument(INDEX))); //get sizeofarray
	//Load the iterator
	addOpcode(new OLoad(new VarArgument(EXP2), new LiteralArgument(indxdecloffset)));
	//If the iterator is >= the length, goto the 'else' (end without break)
	addOpcode(new OCompareRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
	addOpcode(new OGotoCompare(new LabelArgument(elselabel), new CompareArgument(CMP_GE)));
	commentBack(fmt::format("for(each) #{} to Else",forid));
	
	//Reaching here, we have a valid index! Load it.
	addOpcode(new OReadPODArrayR(new VarArgument(EXP1), new VarArgument(EXP2)));
	commentBack(fmt::format("for(each) #{} Next Index",forid));
	//... and store it in the local variable.
	addOpcode(new OStore(new VarArgument(EXP1), new LiteralArgument(decloffset)));
	//Now increment the iterator for the next loop
	addOpcode(new OAddImmediate(new VarArgument(EXP2), new LiteralArgument(10000)));
	addOpcode(new OStore(new VarArgument(EXP2), new LiteralArgument(indxdecloffset)));
	
	//...and run the inside of the loop.
	push_break(loopend, 0, getBreakableScopesForStatement(&host));
	push_cont(loopstart);
	
	targ_sz = commentTarget();
	visit(host.body.get(), param);
	commentStartEnd(targ_sz, fmt::format("for(each) #{} Body",forid));
	
	pop_break();
	pop_cont();
	
	//Return to top of loop
	if(host.ends_loop)
		addOpcode(new OGotoImmediate(new LabelArgument(loopstart)));
	commentBack(fmt::format("for(each) #{} End",forid));

	cur_scopes.pop_back();
	
	scope = scope->getParent();
	
	if(host.hasElse())
	{
		addOpcode(new ONoOp(elselabel));
		targ_sz = commentTarget();
		visit(host.elseBlock.get(), param);
		commentStartEnd(targ_sz, fmt::format("for(each) #{} Else",forid));
	}
	
	addOpcode(new ONoOp(loopend));
}

void BuildOpcodes::caseStmtRangeLoop(ASTStmtRangeLoop &host, void *param)
{
	//Force to sub-scope
	if(!host.getScope())
	{
		host.setScope(scope->makeChild());
	}
	ScopeReverter sr(&scope);
	scope = host.getScope();
	cur_scopes.push_back(scope);
	
	uint loopid = host.get_comment_id();
	//Declare the local variable that will hold the current loop value
	auto targ_sz = commentTarget();
	literal_visit(host.decl.get(), param);
	commentAt(targ_sz, fmt::format("loop() #{} ValDecl",loopid));
	
	int32_t decloffset = host.decl->manager->getStackOffset(false);
	
	int32_t loopstart = ScriptParser::getUniqueLabelID();
	int32_t loopcont = ScriptParser::getUniqueLabelID();
	int32_t loopend = ScriptParser::getUniqueLabelID();
	int32_t elselabel = host.hasElse() ? ScriptParser::getUniqueLabelID() : loopend;
	
	DataType const& declty = host.decl->resolveType(scope, this);
	bool const_indx = declty.isConstant();
	
	auto startval_exact = host.range->getStartVal(false, this, scope);
	auto endval_exact = host.range->getEndVal(false, this, scope);
	auto startval = host.range->getStartVal(true, this, scope);
	auto endval = host.range->getEndVal(true, this, scope);
	auto incrval = host.increment->getCompileTimeValue(this, scope);
	MiniStackMgr mgr;
	optional<uint> start_peekind, end_peekind, incr_peekind, indx_peekind, overflow_peekind;
	bool infloop = incrval && !*incrval;
	if(infloop) host.overflow = ASTStmtRangeLoop::OVERFLOW_ALLOW;
	if(host.overflow != ASTStmtRangeLoop::OVERFLOW_ALLOW)
	{
		addOpcode(new OPushImmediate(new LiteralArgument(0)));
		overflow_peekind = mgr.push();
	}
	if(!startval)
	{
		visit(host.range->start.get(), param);
		if(!(host.range->type & ASTRange::RANGE_L))
			addOpcode(new OAddImmediate(new VarArgument(EXP1), new LiteralArgument(1)));
		
		addOpcode(new OPushRegister(new VarArgument(EXP1)));
		start_peekind = mgr.push();
		if(incrval && *incrval > 0)
			addOpcode(new OStore(new VarArgument(EXP1), new LiteralArgument(decloffset)));
	}
	if(!endval)
	{
		visit(host.range->end.get(), param);
		if(!(host.range->type & ASTRange::RANGE_R))
			addOpcode(new OAddImmediate(new VarArgument(EXP1), new LiteralArgument(-1)));
		
		addOpcode(new OPushRegister(new VarArgument(EXP1)));
		end_peekind = mgr.push();
		if(incrval && *incrval < 0)
			addOpcode(new OStore(new VarArgument(EXP1), new LiteralArgument(decloffset)));
	}
	if(!startval || !endval) //ensure the range is valid to run at all
	{
		if(startval)
			addOpcode(new OCompareImmediate2(new LiteralArgument(*startval), new VarArgument(EXP1)));
		else if(endval)
			addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(*endval)));
		else
		{
			addOpcode(new OPeekAtImmediate(new VarArgument(EXP2), new LiteralArgument(mgr.at(*start_peekind))));
			addOpcode(new OCompareRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
		}
		addOpcode(new OGotoCompare(new LabelArgument(loopend), new CompareArgument(CMP_GT)));
		commentBack(fmt::format("loop() #{} Range Invalid?",loopid));
	}
	if(!incrval)
	{
		VISIT_PUSH(host.increment.get(), incr_peekind);
		addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
		auto lbl = ScriptParser::getUniqueLabelID();
		// Depending on the increment, the starting value changes
		if(startval) //If the startval is constant, it will already be initialized.
		{
			addOpcode(new OGotoCompare(new LabelArgument(lbl), new CompareArgument(CMP_GE)));
			if(endval)
				addOpcode(new OStoreV(new LiteralArgument(*endval), new LiteralArgument(decloffset)));
			else
			{
				addOpcode(new OPeekAtImmediate(new VarArgument(EXP1), new LiteralArgument(mgr.at(*end_peekind))));
				addOpcode(new OStore(new VarArgument(EXP1), new LiteralArgument(decloffset)));
			}
			addOpcode(new ONoOp(lbl));
		}
		else //Otherwise we need to initailize either way
		{
			auto lbl2 = ScriptParser::getUniqueLabelID();
			addOpcode(new OGotoCompare(new LabelArgument(lbl), new CompareArgument(CMP_LT)));
			if(startval)
				addOpcode(new OStoreV(new LiteralArgument(*startval), new LiteralArgument(decloffset)));
			else
			{
				addOpcode(new OPeekAtImmediate(new VarArgument(EXP1), new LiteralArgument(mgr.at(*start_peekind))));
				addOpcode(new OStore(new VarArgument(EXP1), new LiteralArgument(decloffset)));
			}
			addOpcode(new OGotoImmediate(new LabelArgument(lbl2)));
			addOpcode(new ONoOp(lbl));
			
			if(endval)
				addOpcode(new OStoreV(new LiteralArgument(*endval), new LiteralArgument(decloffset)));
			else
			{
				addOpcode(new OPeekAtImmediate(new VarArgument(EXP1), new LiteralArgument(mgr.at(*end_peekind))));
				addOpcode(new OStore(new VarArgument(EXP1), new LiteralArgument(decloffset)));
			}
			addOpcode(new ONoOp(lbl2));
		}
	}
	
	if(!const_indx)
	{
		if(!incrval && !startval)
			;//already in EXP1
		else addOpcode(new OLoad(new VarArgument(EXP1), new LiteralArgument(decloffset)));
		addOpcode(new OPushRegister(new VarArgument(EXP1)));
		indx_peekind = mgr.push();
	}
	
	addOpcode(new ONoOp(loopstart));
	
	//run the inside of the loop.
	push_break(loopend, mgr.count(), getBreakableScopesForStatement(&host));
	push_cont(loopcont);
	
	targ_sz = commentTarget();
	visit(host.body.get(), param);
	commentStartEnd(targ_sz, fmt::format("loop() #{} Body",loopid));
	
	pop_break();
	pop_cont();
	
	//Load
	if(const_indx)
		addOpcode(new OLoad(new VarArgument(EXP2), new LiteralArgument(decloffset)));
	else addOpcode(new OPeekAtImmediate(new VarArgument(EXP2), new LiteralArgument(mgr.at(*indx_peekind))));
	backOpcode()->setLabel(loopcont);
	commentBack(fmt::format("loop() #{} Incr",loopid));
	if(host.overflow != ASTStmtRangeLoop::OVERFLOW_ALLOW)
	{
		//Cache the value we just ran the loop for...
		addOpcode(new OStackWriteAtRV(new VarArgument(EXP2), new LiteralArgument(mgr.at(*overflow_peekind))));
	}
	if(!infloop)
	{
		if(incrval)
			addOpcode(new OAddImmediate(new VarArgument(EXP2), new LiteralArgument(*incrval)));
		else
		{
			addOpcode(new OPeekAtImmediate(new VarArgument(EXP1), new LiteralArgument(mgr.at(*incr_peekind))));
			addOpcode(new OAddRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
		}
		if(!const_indx)
			addOpcode(new OStackWriteAtRV(new VarArgument(EXP2), new LiteralArgument(mgr.at(*indx_peekind))));
	}
	addOpcode(new OStore(new VarArgument(EXP2), new LiteralArgument(decloffset)));
	
	targ_sz = commentTarget();
	if(!incrval)
	{
		auto endtestlabel = ScriptParser::getUniqueLabelID();
		if(startval)
		{
			addOpcode(new OCompareImmediate(new VarArgument(EXP2), new LiteralArgument(*startval)));
		}
		else
		{
			addOpcode(new OPeekAtImmediate(new VarArgument(EXP1), new LiteralArgument(mgr.at(*start_peekind))));
			addOpcode(new OCompareRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
		}
		addOpcode(new OGotoCompare(new LabelArgument(endtestlabel), new CompareArgument(CMP_LT)));
		commentBack(fmt::format("{} start, skip to else/end",CMP_STR(CMP_LT)));
		if(endval)
		{
			addOpcode(new OCompareImmediate(new VarArgument(EXP2), new LiteralArgument(*endval)));
		}
		else
		{
			addOpcode(new OPeekAtImmediate(new VarArgument(EXP1), new LiteralArgument(mgr.at(*end_peekind))));
			addOpcode(new OCompareRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
		}
		addOpcode(new OGotoCompare(new LabelArgument(loopstart), new CompareArgument(CMP_LE)));
		commentBack(fmt::format("{} end, keep looping",CMP_STR(CMP_LE)));
		addOpcode(new ONoOp(endtestlabel));
	}
	else if(*incrval < 0)
	{
		if(startval)
		{
			addOpcode(new OCompareImmediate(new VarArgument(EXP2), new LiteralArgument(*startval)));
		}
		else
		{
			addOpcode(new OPeekAtImmediate(new VarArgument(EXP1), new LiteralArgument(mgr.at(*start_peekind))));
			addOpcode(new OCompareRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
		}
		addOpcode(new OGotoCompare(new LabelArgument(loopstart), new CompareArgument(CMP_GE)));
		commentBack(fmt::format("{} start, keep looping",CMP_STR(CMP_GE)));
	}
	else if(*incrval > 0)
	{
		if(endval)
		{
			addOpcode(new OCompareImmediate(new VarArgument(EXP2), new LiteralArgument(*endval)));
		}
		else
		{
			addOpcode(new OPeekAtImmediate(new VarArgument(EXP1), new LiteralArgument(mgr.at(*end_peekind))));
			addOpcode(new OCompareRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
		}
		addOpcode(new OGotoCompare(new LabelArgument(loopstart), new CompareArgument(CMP_LE)));
		commentBack(fmt::format("{} end, keep looping",CMP_STR(CMP_LE)));
	}
	else //constant 0 increment, infinite loop
	{
		addOpcode(new OGotoImmediate(new LabelArgument(loopstart)));
	}
	commentAt(targ_sz, fmt::format("loop() #{} Test",loopid));
	
	if(host.overflow != ASTStmtRangeLoop::OVERFLOW_ALLOW)
	{
		vector<shared_ptr<Opcode>> op_endval, op_startval;
		auto& targ = backTarget();
		optional<int> sv, ev, targv;
		//calculate and store the code to grab target value
		if(host.overflow == ASTStmtRangeLoop::OVERFLOW_LONG)
		{
			if(startval)
				addOpcode2(op_startval, new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*(sv = startval))));
			else
				addOpcode2(op_startval, new OPeekAtImmediate(new VarArgument(EXP1), new LiteralArgument(mgr.at(*start_peekind))));
			
			if(endval)
				addOpcode2(op_endval, new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*(ev = endval))));
			else
				addOpcode2(op_endval, new OPeekAtImmediate(new VarArgument(EXP1), new LiteralArgument(mgr.at(*end_peekind))));
		}
		else if(host.overflow == ASTStmtRangeLoop::OVERFLOW_INT)
		{
			if(startval)
			{
				auto val = *startval;
				val = zslongToFix(val).getTrunc()*10000;
				sv = val;
				addOpcode2(op_startval, new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(val)));
			}
			else
			{
				addOpcode2(op_startval, new OPeekAtImmediate(new VarArgument(EXP1), new LiteralArgument(mgr.at(*start_peekind))));
				addOpcode2(op_startval, new OTruncate(new VarArgument(EXP1)));
			}
			if(endval)
			{
				auto val = *endval;
				val = zslongToFix(val).getTrunc()*10000;
				ev = val;
				addOpcode2(op_endval, new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(val)));
			}
			else
			{
				addOpcode2(op_endval, new OPeekAtImmediate(new VarArgument(EXP1), new LiteralArgument(mgr.at(*end_peekind))));
				addOpcode2(op_endval, new OTruncate(new VarArgument(EXP1)));
			}
		}
		if(incrval) //we know which target to use
		{
			if(*incrval > 0) //targ is endval
			{
				if(ev)
					targv = ev;
				else targ.insert(targ.end(), op_endval.begin(), op_endval.end());
			}
			else if(*incrval < 0)
			{
				if(sv)
					targv = sv;
				else targ.insert(targ.end(), op_startval.begin(), op_startval.end());
			}
			else assert(false); //unreachable
		}
		else //We need to calculate which target to use
		{
			addOpcode(new OPeekAtImmediate(new VarArgument(EXP1), new LiteralArgument(mgr.at(*incr_peekind))));
			addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
			auto skiplabel = ScriptParser::getUniqueLabelID();
			auto skiplabel2 = ScriptParser::getUniqueLabelID();
			addOpcode(new OGotoCompare(new LabelArgument(skiplabel), new CompareArgument(CMP_LT)));
			targ.insert(targ.end(), op_endval.begin(), op_endval.end());
			addOpcode(new OGotoImmediate(new LabelArgument(skiplabel2)));
			addOpcode(new ONoOp(skiplabel));
			targ.insert(targ.end(), op_startval.begin(), op_startval.end());
			addOpcode(new ONoOp(skiplabel2));
		}
		//...now, we need to check that we didn't ALREADY just run that value exactly
		addOpcode(new OPeekAtImmediate(new VarArgument(EXP2), new LiteralArgument(mgr.at(*overflow_peekind))));
		if(targv)
			addOpcode(new OCompareImmediate(new VarArgument(EXP2), new LiteralArgument(*targv)));
		else addOpcode(new OCompareRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
		addOpcode(new OGotoCompare(new LabelArgument(elselabel), new CompareArgument(CMP_EQ)));
		//If we haven't run the value yet, set it and run the loop one last time
		if(!const_indx)
			addOpcode(new OStackWriteAtRV(new VarArgument(EXP2), new LiteralArgument(mgr.at(*indx_peekind))));
		if(targv)
			addOpcode(new OStoreV(new LiteralArgument(*targv), new LiteralArgument(decloffset)));
		else addOpcode(new OStore(new VarArgument(EXP1), new LiteralArgument(decloffset)));
		addOpcode(new OGotoImmediate(new LabelArgument(loopstart)));
	}

	cur_scopes.pop_back();
	
	scope = scope->getParent();
	
	addOpcode(new OPopArgsRegister(new VarArgument(NUL), new LiteralArgument(mgr.pop_all())));
	
	if(host.hasElse())
	{
		addOpcode(new ONoOp(elselabel));
		targ_sz = commentTarget();
		visit(host.elseBlock.get(), param);
		commentStartEnd(targ_sz, fmt::format("loop() #{} Else",loopid));
	}
	
	addOpcode(new ONoOp(loopend));
}

void BuildOpcodes::caseStmtWhile(ASTStmtWhile &host, void *param)
{
	auto val = host.test->getCompileTimeValue(this, scope);
	bool inv = host.isInverted();
	string whilestr = inv ? "until" : "while",
		truestr = inv ? "false" : "true",
		falsestr = inv ? "true" : "false";
	uint whileid = host.get_comment_id();
	if(val && (inv != !*val)) //never runs, handle else only
	{
		if(host.hasElse())
		{
			auto targ_sz = commentTarget();
			visit(host.elseBlock.get(), param);
			commentStartEnd(targ_sz, fmt::format("{}({}) #{} Else [Opt:AlwaysOff]",whilestr,falsestr,whileid));
		}
		return;
	}
	
	int32_t startlabel = ScriptParser::getUniqueLabelID();
	int32_t endlabel = ScriptParser::getUniqueLabelID();
	int32_t elselabel = host.hasElse() ? ScriptParser::getUniqueLabelID() : endlabel;
	//run the test
	addOpcode(new ONoOp(startlabel));
	if(!val)
	{
		commentBack(fmt::format("{}() #{} Test",whilestr,whileid));
		literal_visit(host.test.get(), param);

		//Continue
		addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
		if(host.isInverted()) //Is this `until` or `while`?
		{
			addOpcode(new OGotoFalseImmediate(new LabelArgument(elselabel)));
			commentBack("Test 'until'");
		}
		else
		{
			addOpcode(new OGotoTrueImmediate(new LabelArgument(elselabel)));
			commentBack("Test 'while'");
		}
	}
	
	push_break(endlabel, 0, getBreakableScopesForStatement(&host));
	push_cont(startlabel);
	
	auto targ_sz = commentTarget();
	visit(host.body.get(), param);
	commentStartEnd(targ_sz, fmt::format("{}({}) #{} Body{}",whilestr,val?truestr:"",whileid,val?" [Opt:AlwaysOn]":""));
	
	uint num_breaks = break_to_counts.back();
	pop_break();
	pop_cont();
	
	if(host.ends_loop)
		addOpcode(new OGotoImmediate(new LabelArgument(startlabel)));
	commentBack(fmt::format("{}() #{} End",whilestr,whileid));
	
	if(host.hasElse() && !val)
	{
		addOpcode(new ONoOp(elselabel));
		targ_sz = commentTarget();
		visit(host.elseBlock.get(), param);
		commentStartEnd(targ_sz, fmt::format("{}() #{} Else",whilestr,whileid));
	}
	if(!val || num_breaks) //no else / end label needed for inf loops unless they break
		addOpcode(new ONoOp(endlabel));
}

void BuildOpcodes::caseStmtDo(ASTStmtDo &host, void *param)
{
	auto val = host.test->getCompileTimeValue(this, scope);
	bool inv = host.isInverted();
	bool truthyval = val && inv == (*val==0);
	bool deadloop = val && !truthyval;
	uint whileid = host.get_comment_id();
	int32_t startlabel = ScriptParser::getUniqueLabelID();
	int32_t endlabel = ScriptParser::getUniqueLabelID();
	int32_t elselabel = host.hasElse() ? ScriptParser::getUniqueLabelID() : endlabel;
	int32_t continuelabel = ScriptParser::getUniqueLabelID();
	string whilestr = inv ? "do-until" : "do-while",
		truestr = inv ? "false" : "true",
		falsestr = inv ? "true" : "false";
	if(!deadloop)
		addOpcode(new ONoOp(startlabel));
	
	push_break(endlabel, 0, getBreakableScopesForStatement(&host));
	push_cont(continuelabel);
	
	auto targ_sz = commentTarget();
	visit(host.body.get(), param);
	commentStartEnd(targ_sz, fmt::format("{}() #{} Body",whilestr,whileid));
	
	pop_break();
	pop_cont();
	
	addOpcode(new ONoOp(continuelabel));
	
	if(val)
	{
		if(truthyval) //infinite loop
		{
			commentBack(fmt::format("{}({}) #{} Loop [Opt:AlwaysOn]",whilestr,truestr,whileid));
			addOpcode(new OGotoImmediate(new LabelArgument(startlabel)));
		}
		else if(host.hasElse())
		{
			commentBack(fmt::format("{}({}) #{} Loop [Opt:AlwaysOff]",whilestr,falsestr,whileid));
			visit(host.elseBlock.get(), param);
		}
	}
	else
	{
		commentBack(fmt::format("{}() #{} Test",whilestr,whileid));
		literal_visit(host.test.get(), param);
		//Continue
		addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
		if(host.isInverted()) //Is this `until` or `while`?
		{
			addOpcode(new OGotoFalseImmediate(new LabelArgument(elselabel)));
			commentBack("Test 'until'");
		}
		else
		{
			addOpcode(new OGotoTrueImmediate(new LabelArgument(elselabel)));
			commentBack("Test 'while'");
		}
		addOpcode(new OGotoImmediate(new LabelArgument(startlabel)));
		
		addOpcode(new ONoOp(elselabel));
		targ_sz = commentTarget();
		visit(host.elseBlock.get(), param);
		commentStartEnd(targ_sz, fmt::format("{}() #{} Else",whilestr,whileid));
	}
	
	addOpcode(new ONoOp(endlabel));
}

void BuildOpcodes::caseStmtReturn(ASTStmtReturn&, void*)
{
	// For the scopes that won't end normally because this return is skipping over them,
	// remove their object references.
	assert(in_func);
	assert(in_func->getInternalScope());

	Scope* exiting_scope = in_func->getInternalScope();
	if (exiting_scope)
	{
		for (auto it = cur_scopes.rbegin(); it != cur_scopes.rend(); it++)
		{
			auto scope = *it;
			mark_ref_remove_if_needed_for_scope(scope);

			if (scope == exiting_scope)
				break;
		}
	}

	if(uint pops = scope_pops_count())
		pop_params(pops);
	addOpcode(new OGotoImmediate(new LabelArgument(returnlabelid)));
	commentBack("return (Void)");
}

void BuildOpcodes::caseStmtReturnVal(ASTStmtReturnVal &host, void *param)
{
	auto targ_sz = commentTarget();
	INITC_STORE();
	visit(host.value.get(), INITCTX);
	INITC_INIT();

	auto ret_type = host.value->getReadType(scope, this);
	if (ret_type && ret_type->isObject())
		addOpcode(new ORefAutorelease(new VarArgument(EXP1)));

	// For the scopes that won't end normally because this return is skipping over them,
	// remove their object references.
	assert(in_func);
	assert(in_func->getInternalScope());

	Scope* exiting_scope = in_func->getInternalScope();
	if (exiting_scope)
	{
		for (auto it = cur_scopes.rbegin(); it != cur_scopes.rend(); it++)
		{
			auto scope = *it;
			mark_ref_remove_if_needed_for_scope(scope);

			if (scope == exiting_scope)
				break;
		}
	}

	if(uint pops = scope_pops_count())
		pop_params(pops);
	addOpcode(new OGotoImmediate(new LabelArgument(returnlabelid)));
	commentStartEnd(targ_sz,"return");
}

void BuildOpcodes::caseStmtBreak(ASTStmtBreak &host, void *)
{
	if(!host.breakCount) return;
	if (break_depth < host.breakCount)
	{
		handleError(CompileError::BreakBad(&host,host.breakCount));
		return;
	}
	int32_t breaklabel = breaklabelids.at(breaklabelids.size()-host.breakCount);

	// For the scopes that won't end normally because this break is skipping over them,
	// remove their object references.
	auto exiting_scopes = breakScopes.at(breakScopes.size()-host.breakCount);
	if (!exiting_scopes.empty())
	{
		for (auto it = cur_scopes.rbegin(); it != cur_scopes.rend(); it++)
		{
			auto scope = *it;
			mark_ref_remove_if_needed_for_scope(scope);

			if (std::find(exiting_scopes.begin(), exiting_scopes.end(), scope) != exiting_scopes.end())
				break;
		}
	}

	if(uint pops = scope_pops_back(host.breakCount))
		pop_params(pops);
	addOpcode(new OGotoImmediate(new LabelArgument(breaklabel)));
	commentBack(fmt::format("break {};",host.breakCount));
	inc_break(host.breakCount);
}

void BuildOpcodes::caseStmtContinue(ASTStmtContinue &host, void *)
{
	if(!host.contCount) return;
	if (continue_depth < host.contCount)
	{
		handleError(CompileError::ContinueBad(&host,host.contCount));
		return;
	}

	int32_t contlabel = continuelabelids.at(continuelabelids.size()-host.contCount);
	if(uint pops = scope_pops_back(host.contCount-1))
		pop_params(pops);
	addOpcode(new OGotoImmediate(new LabelArgument(contlabel)));
	commentBack(fmt::format("continue {};",host.contCount));
}

void BuildOpcodes::caseStmtEmpty(ASTStmtEmpty &, void *)
{
	// empty
}

// Declarations

void BuildOpcodes::caseFuncDecl(ASTFuncDecl &host, void *param)
{
	if(host.getFlag(FUNCFLAG_INLINE)) return; //Skip inline func decls, they are handled at call location -V
	if(host.prototype) return; //Same for prototypes
	returnlabelid = ScriptParser::getUniqueLabelID();
	
	in_func = host.func;
	visit(host.block.get(), param);
	in_func = nullptr;
}

void BuildOpcodes::caseDataDecl(ASTDataDecl& host, void* param)
{
	if(parsing_user_class == puc_vars) return;
	OpcodeContext& context = *(OpcodeContext*)param;
	Datum& manager = *host.manager;
	if(manager.is_erased()) //var unused, optimized away
	{
		sidefx_visit(host.getInitializer(), param);
		return;
	}

	// Ignore inlined values.
	if (manager.getCompileTimeValue()) return;

	buildVariable(host, context);
}

void BuildOpcodes::buildVariable(ASTDataDecl& host, OpcodeContext& context)
{
	if (host.list && host.list->internal)
		return;

	Datum& manager = *host.manager;
	
	// Load initializer, if present.
	auto init = host.getInitializer();

	std::optional<int> comptime_val;
	if (init)
		comptime_val = init->getCompileTimeValue(this, scope);
	if (!comptime_val)
	{
		if (init)
			visit(init, &context);
		else if (!host.extraArrays.empty())
			buildArrayUninit(host, context);
		else if (host.getFlag(ASTDataDecl::FL_SKIP_EMPTY_INIT))
			return;
		else
			comptime_val = 0;
	}

	auto writeType = &host.resolveType(scope, this);
	bool is_object = writeType && writeType->isObject();

	// Set variable to EXP1 or comptime_val, depending on the initializer.
	if (auto globalId = manager.getGlobalId())
	{
		if (is_object)
			addOpcode(new OMarkTypeRegister(new GlobalArgument(*globalId), new LiteralArgument((int)writeType->getScriptObjectTypeId())));

		if (comptime_val)
			addOpcode(new OSetImmediate(new GlobalArgument(*globalId), new LiteralArgument(*comptime_val)));
		else if (is_object)
			addOpcode(new OSetObject(new GlobalArgument(*globalId), new VarArgument(EXP1)));
		else
			addOpcode(new OSetRegister(new GlobalArgument(*globalId), new VarArgument(EXP1)));
	}
	else
	{
		int32_t offset = manager.getStackOffset(false);
		if (comptime_val)
		{
			// I tried to optimize this away in some circumstances, it lead to only problems -Em
			addOpcode(new OStoreV(new LiteralArgument(*comptime_val), new LiteralArgument(offset)));
		}
		else if (is_object)
		{
			// This command decrements the reference of the object currently stored at the position before
			// setting the new object and incrementing its reference. Since we set the initial value for
			// stack variables to 0 (aka a null object), this is fine.
			addOpcode(new OStoreObject(new VarArgument(EXP1), new LiteralArgument(offset)));
		}
		else addOpcode(new OStore(new VarArgument(EXP1), new LiteralArgument(offset)));
	}
}

void BuildOpcodes::buildArrayUninit(ASTDataDecl& host, OpcodeContext& context)
{
	Datum& manager = *host.manager;

	// Right now, don't support nested arrays.
	if (host.extraArrays.size() != 1)
	{
		handleError(CompileError::DimensionMismatch(&host));
		return;
	}

	// Get size of the array.
	int32_t totalSize;
	if (auto size = host.extraArrays[0]->getCompileTimeSize(this, scope))
		totalSize = *size * 10000L;
	else
	{
		handleError(
				CompileError::ExprNotConstant(host.extraArrays[0]));
		return;
	}

	auto& type = host.resolveType(scope, this);
	if (!dynamic_cast<const DataTypeArray*>(&type))
	{
		handleError(CompileError::Error(&host, fmt::format("Expected an array type")));
		return;
	}

	auto array_type = dynamic_cast<const DataTypeArray*>(&type);
	int element_script_object_type_id = (int)array_type->getElementType().getScriptObjectTypeId();

	// Allocate the array.
	if (auto globalId = manager.getGlobalId())
	{
		addOpcode(new OAllocateGlobalMemImmediate(
						  new VarArgument(EXP1),
						  new LiteralArgument(totalSize),
						  new LiteralArgument(element_script_object_type_id)));
	}
	else
	{
		addOpcode(new OAllocateMemImmediate(new VarArgument(EXP1), new LiteralArgument(totalSize), new LiteralArgument(element_script_object_type_id)));
	}
}

void BuildOpcodes::caseDataTypeDef(ASTDataTypeDef&, void*) {}

void BuildOpcodes::caseCustomDataTypeDef(ASTDataTypeDef&, void*) {}

// Expressions

void BuildOpcodes::caseExprAssign(ASTExprAssign &host, void *param)
{
	//load the rval into EXP1
	VISIT_USEVAL(host.right.get(), param);
	//and store it
	LValBOHelper helper(program, this);
	helper.parsing_user_class = parsing_user_class;
	host.left->execute(helper, param);
	addOpcodes(helper.getResult());
}

void BuildOpcodes::caseExprIdentifier(ASTExprIdentifier& host, void* param)
{
	if(sidefx_only)
		return;
	if(parsing_user_class > puc_vars)
	{
		if(host.asString() == "this")
		{
			addOpcode(new OSetRegister(new VarArgument(EXP1), new VarArgument(CLASS_THISKEY)));	
			return;
		}
	}

	if (auto ivar = dynamic_cast<InternalVariable*>(host.binding))
	{
		assert(ivar->readfn);
		std::vector<std::shared_ptr<Opcode>> const& funcCode = ivar->readfn->getCode();
		for (auto it = funcCode.begin(); it != funcCode.end(); ++it)
			addOpcode((*it)->makeClone(false));
		return;
	}

	if(UserClassVar* ucv = dynamic_cast<UserClassVar*>(host.binding))
	{
		UserClass& user_class = *ucv->getClass();
		addOpcode(new OReadObject(new VarArgument(CLASS_THISKEY), new LiteralArgument(ucv->getIndex())));
		return;
	}
	
	// If a constant, just load its value.
	if (auto value = host.binding->getCompileTimeValue(scope->isGlobal() || scope->isScript()))
	{
		addOpcode(new OSetImmediate(new VarArgument(EXP1),
									new LiteralArgument(*value)));
		host.markConstant();
		return;
	}

	if (auto globalId = host.binding->getGlobalId())
	{
		// Global variable, so just get its value.
		addOpcode(new OSetRegister(new VarArgument(EXP1),
								   new GlobalArgument(*globalId)));
		return;
	}

	// Local variable, get its value from the stack.
	int32_t offset = host.binding->getStackOffset(false);
	addOpcode(new OLoad(new VarArgument(EXP1), new LiteralArgument(offset)));
}

void BuildOpcodes::caseExprArrow(ASTExprArrow& host, void* param)
{
	if (UserClassVar* ucv = host.u_datum; ucv && !ucv->is_internal)
	{
		SIDEFX_CHECK(host.left.get());
		visit(host.left.get(), param);
		addOpcode(new OReadObject(new VarArgument(EXP1), new LiteralArgument(ucv->getIndex())));
		return;
	}

	bool isIndexed = host.index;
	Function* readfunc = host.readFunction;
	
	if(readfunc->isNil())
	{
		bool skipptr = readfunc->getIntFlag(IFUNCFLAG_SKIPPOINTER);
		
		if (!skipptr)
			sidefx_visit(host.left.get(), param);
		
		if(isIndexed)
			sidefx_visit(host.index.get(), param);
		
		if(!sidefx_only)
			addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
	}
	else if(readfunc->getFlag(FUNCFLAG_INLINE))
	{
		if (!(readfunc->getIntFlag(IFUNCFLAG_SKIPPOINTER)))
		{
			//push the lhs of the arrow
			VISIT_USEVAL(host.left.get(), param);
			addOpcode(new OPushRegister(new VarArgument(EXP1)));
		}
		
		if(isIndexed)
		{
			VISIT_USEVAL(host.index.get(), param);
			addOpcode(new OPushRegister(new VarArgument(EXP1)));
		}
		
		std::vector<std::shared_ptr<Opcode>> const& funcCode = readfunc->getCode();
		for(auto it = funcCode.begin();
			it != funcCode.end(); ++it)
		{
			addOpcode((*it)->makeClone(false));
		}
	}
	else
	{
		//this is 	actually a function call
		//to the appropriate gettor method
		//so, set that up:
		//push the stack frame
		bool never_ret = readfunc->getFlag(FUNCFLAG_NEVER_RETURN);
		if(!never_ret)
			addOpcode(new OPushRegister(new VarArgument(SFRAME)));
		if (!(readfunc->getIntFlag(IFUNCFLAG_SKIPPOINTER)))
		{
			//push the lhs of the arrow
			VISIT_USEVAL(host.left.get(), param);
			addOpcode(new OPushRegister(new VarArgument(EXP1)));
		}

		//if indexed, push the index
		if(isIndexed)
		{
			VISIT_USEVAL(host.index.get(), param);
			addOpcode(new OPushRegister(new VarArgument(EXP1)));
		}

		//call the function
		int32_t label = readfunc->getLabel();
		addOpcode(new OCallFunc(new LabelArgument(label, true)));
		//pop the stack frame
		if(!never_ret)
			addOpcode(new OPopRegister(new VarArgument(SFRAME)));
	}
}

void BuildOpcodes::caseExprIndex(ASTExprIndex& host, void* param)
{
	// If the left hand side is an arrow, then we'll let it run instead.
	if (host.array->isTypeArrow())
	{
		ASTExprArrow* arrow = static_cast<ASTExprArrow*>(host.array.get());
		if(!arrow->isTypeArrowUsrClass())
		{
			caseExprArrow(static_cast<ASTExprArrow&>(*host.array), param);
			return;
		}
		if (arrow->u_datum->is_internal)
		{
			assert(arrow->u_datum->is_arr);
			caseExprArrow(static_cast<ASTExprArrow&>(*host.array), param);
			return;
		}
	}
	
	if(sidefx_only)
	{
		sidefx_visit(host.array.get(), param);
		sidefx_visit(host.index.get(), param);
		return;
	}
	
	auto arrVal = host.array->getCompileTimeValue(this,scope);
	auto indxVal = host.index->getCompileTimeValue(this,scope);
	
	if(!arrVal)
	{
		// First, push the array.
		visit(host.array.get(), param);
		addOpcode(new OPushRegister(new VarArgument(EXP1)));
	}

	if(!indxVal)
	{
		//Load the index
		visit(host.index.get(), param);
	}
	// Pop array into INDEX.
	if(arrVal) addOpcode(new OSetImmediate(new VarArgument(INDEX), new LiteralArgument(*arrVal)));
	else addOpcode(new OPopRegister(new VarArgument(INDEX)));
	
	if(indxVal) addOpcode(new OReadPODArrayI(new VarArgument(EXP1), new LiteralArgument(*indxVal)));
	else addOpcode(new OReadPODArrayR(new VarArgument(EXP1), new VarArgument(EXP1)));
}

// TODO: refactor/simplify this function - function calling should mostly share the same code no matter if for
// a class, internal function, or free-function.
void BuildOpcodes::caseExprCall(ASTExprCall& host, void* param)
{
	if (host.isDisabled()) return;
	if (auto v = host.getCompileTimeValue(this, scope)) //constexpr function!
	{
		CONST_VAL(*v);
		return;
	}
	INITC_STORE();
	auto& func = *host.binding;
	bool classfunc = func.getFlag(FUNCFLAG_CLASSFUNC) && !func.getFlag(FUNCFLAG_STATIC);

	auto* optarg = opcodeTargets.back();
	const string func_comment = fmt::format("Func[{}]",func.getUnaliasedSignature(true).asString());
	auto targ_sz = commentTarget();
	bool never_ret = func.getFlag(FUNCFLAG_NEVER_RETURN);
	if(func.isNil()) //Prototype/Nil function
	{
		//Visit each parameter for side-effects only
		for (auto it = host.parameters.begin();
			it != host.parameters.end(); ++it)
		{
			sidefx_visit(*it, INITCTX);
		}
		commentStartEnd(targ_sz, fmt::format("Proto{} Visit Params",func_comment));
		
		//Set the return to the default value
		if(classfunc && func.getFlag(FUNCFLAG_CONSTRUCTOR))
		{
			ClassScope* cscope = func.getInternalScope()->getClass();
			UserClass& user_class = cscope->user_class;
			vector<Function*> destr = cscope->getDestructor();
			Function* destructor = destr.size() == 1 ? destr.at(0) : nullptr;
			if(destructor && !destructor->isNil())
			{
				Function* destructor = destr[0];
				addOpcode(new OSetImmediateLabel(new VarArgument(EXP1),
					new LabelArgument(destructor->getLabel(), true)));
			}
			else addOpcode(new OSetImmediate(new VarArgument(EXP1),
				new LiteralArgument(0)));
			commentBack(fmt::format("Proto{} Set Destructor",func_comment));
			addOpcode(new OConstructClass(new VarArgument(EXP1),
				new VectorArgument(user_class.members)));
			std::vector<int> object_indices;
			for (auto&& member : user_class.getScope().getClassData())
			{
				auto& type = member.second->getNode()->resolveType(scope, nullptr);
				if (type.isObject())
				{
					object_indices.push_back(member.second->getIndex());

					script_object_type object_type;
					if (type.isArray())
						object_type = static_cast<const DataTypeArray*>(&type)->getElementType().getScriptObjectTypeId();
					else
						object_type = type.getScriptObjectTypeId();
					object_indices.push_back((int)object_type);
				}
			}
			if (!object_indices.empty())
				addOpcode(new OMarkTypeClass(new VectorArgument(object_indices)));
			commentBack(fmt::format("Proto{} Default Construct",func_comment));
		}
		else
		{
			DataType const& retType = *func.returnType;
			if(!retType.isVoid() && !sidefx_only)
			{
				int32_t retval = 0;
				if (auto val = func.defaultReturn)
					retval = *val;
				addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(retval)));
				commentBack(fmt::format("Proto{} Default RetVal",func_comment));
			}
		}
	}
	else if(func.getFlag(FUNCFLAG_INLINE) && (func.isInternal() || func.getFlag(FUNCFLAG_INTERNAL))) //Inline function
	{
		// User functions actually can't really benefit from any optimization like this... -Em
		size_t num_actual_params = func.paramTypes.size() - func.extra_vargs;
		size_t num_used_params = host.parameters.size();
		
		if (host.left->isTypeArrow())
		{
			// Note: FUNCFLAG_INTERNAL are bindings written in .zs w/ 'internal' keyword.
			// But Function.isInternal is for symbols written in src/parser/symbols/*.cpp
			// Only the latter have an actual parameter of the LHS type in their param list.
			if (!func.getFlag(FUNCFLAG_INTERNAL))
				++num_used_params;
			if (!(func.getIntFlag(IFUNCFLAG_SKIPPOINTER)))
			{
				//load the value of the left-hand of the arrow into EXP1
				VISIT_USEVAL(static_cast<ASTExprArrow&>(*host.left).left.get(), INITCTX);
				//visit(host.getLeft(), INITCTX);
				//push it onto the stack
				addOpcode(new OPushRegister(new VarArgument(EXP1)));
			}
		}

		int v = num_used_params-num_actual_params;

		size_t vargcount = v;
		//push the parameters, in forward order
		size_t param_indx = 0;
		for (; param_indx < host.parameters.size()-vargcount; ++param_indx)
		{
			auto& arg = host.parameters.at(param_indx);
			bool unused = !func.isInternal() && !func.getFlag(FUNCFLAG_INTERNAL) && func.paramDatum[param_indx]->is_erased();
			//Compile-time constants can be optimized slightly...
			if(auto val = arg->getCompileTimeValue(this, scope))
			{
				if(!unused)
					addOpcode(new OPushImmediate(new LiteralArgument(*val)));
			}
			else if(unused)
			{
				sidefx_visit(arg, INITCTX);
			}
			else
			{
				VISIT_USEVAL(arg, INITCTX);
				push_param();
			}
		}
		if(vargcount)
		{
			//push the vargs, in forward order
			for (; param_indx < host.parameters.size(); ++param_indx)
			{
				auto& arg = host.parameters.at(param_indx);
				//Compile-time constants can be optimized slightly...
				if(auto val = arg->getCompileTimeValue(this, scope))
					addOpcode(new OPushVargV(new LiteralArgument(*val)));
				else
				{
					VISIT_USEVAL(arg, INITCTX);
					push_param(true);
				}
			}
		}

		std::vector<std::shared_ptr<Opcode>> const& funcCode = func.getCode();
		auto it = funcCode.begin();
		while(OPopRegister* ocode = dynamic_cast<OPopRegister*>(it->get()))
		{
			if (optarg->empty())
				break;

			VarArgument const* destreg = ocode->getArgument();
			//Optimize
			Opcode* lastop = optarg->back().get();
			if(OPushRegister* tmp = dynamic_cast<OPushRegister*>(lastop))
			{
				VarArgument const* arg = tmp->getArgument();
				if(*arg == *destreg) //Same register!
				{
					optarg->pop_back();
				}
				else //Different register
				{
					VarArgument* a = arg->clone();
					optarg->pop_back();
					addOpcode(new OSetRegister(destreg->clone(), a));
				}
				if(++it == funcCode.end())
					break;
				continue;
			}
			else if(OPushImmediate* tmp = dynamic_cast<OPushImmediate*>(lastop))
			{
				LiteralArgument* arg = tmp->getArgument()->clone();
				optarg->pop_back();
				addOpcode(new OSetImmediate(destreg->clone(), arg));
				if(++it == funcCode.end())
					break;
				continue;
			}
			else break;
		}
		commentStartEnd(targ_sz, fmt::format("Inline{} Params",func_comment));
		targ_sz = commentTarget();
		for(;it != funcCode.end(); ++it)
		{
			addOpcode((*it)->makeClone(false));
		}
		commentStartEnd(targ_sz, fmt::format("Inline{} Body",func_comment));
	
		if(host.left->isTypeArrow())
		{
			ASTExprArrow* arr = static_cast<ASTExprArrow*>(host.left.get());
			if(arr->left->getWriteType(scope, this) && !arr->left->isConstant())
			{
				if(func.getIntFlag(IFUNCFLAG_REASSIGNPTR))
				{
					bool isVoid = func.returnType->isVoid();
					if(!isVoid) addOpcode(new OPushRegister(new VarArgument(EXP1)));
					addOpcode(new OSetRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
					LValBOHelper helper(program, this);
					helper.parsing_user_class = parsing_user_class;
					arr->left->execute(helper, INITC_CTXT);
					addOpcodes(helper.getResult());
					if(!isVoid) addOpcode(new OPopRegister(new VarArgument(EXP1)));
				}
			}
			else if(func.getIntFlag(IFUNCFLAG_REASSIGNPTR)) //This is likely a mistake in the script... give the user a warning.
			{
				handleError(CompileError::BadReassignCall(&host, func.getUnaliasedSignature().asString()));
			}
		}
	}
	else if(classfunc)
	{
		int32_t funclabel = func.getLabel();
		
		bool vargs = func.getFlag(FUNCFLAG_VARARGS);
		auto num_used_params = host.parameters.size();
		auto num_actual_params = func.paramTypes.size() - (vargs ? 1 : 0);
		int v = num_used_params-num_actual_params;
		size_t vargcount = v > 0 ? v : 0;
		
		commentStartEnd(targ_sz, fmt::format("Class{} Vargs",func_comment));
		//push the this key/stack frame pointer
		if(!never_ret)
		{
			addOpcode(new OPushRegister(new VarArgument(CLASS_THISKEY)));
			addOpcode(new OPushRegister(new VarArgument(SFRAME)));
		}
		
		targ_sz = commentTarget();
		//push the parameters, in forward order
		size_t param_indx = 0;
		for (; param_indx < num_used_params-vargcount; ++param_indx)
		{
			auto& arg = host.parameters.at(param_indx);
			bool unused = !func.isInternal() && func.paramDatum[param_indx]->is_erased();
			//Compile-time constants can be optimized slightly...
			if(auto val = arg->getCompileTimeValue(this, scope))
			{
				if(!unused)
					addOpcode(new OPushImmediate(new LiteralArgument(*val)));
			}
			else if(unused)
			{
				sidefx_visit(arg, INITCTX);
			}
			else
			{
				VISIT_USEVAL(arg, INITCTX);
				push_param();
			}
		}

		if(vargs)
		{
			auto array_type = dynamic_cast<const DataTypeArray*>(func.paramTypes.back());
			int element_script_object_type_id = (int)array_type->getElementType().getScriptObjectTypeId();

			//push the vargs, in forward order
			for (size_t param_indx = num_used_params-vargcount; param_indx < num_used_params; ++param_indx)
			{
				auto& arg = host.parameters.at(param_indx);
				//Compile-time constants can be optimized slightly...
				if(auto val = arg->getCompileTimeValue(this, scope))
					addOpcode(new OPushVargV(new LiteralArgument(*val)));
				else
				{
					VISIT_USEVAL(arg, INITCTX);
					push_param(true);
				}
			}
			addOpcode(new OMakeVargArray(new LiteralArgument(element_script_object_type_id)));
			commentBack("Allocate Vargs array");
			addOpcode(new OPushRegister(new VarArgument(EXP1)));
			commentBack("Push the Vargs array pointer");
		}

		if (host.left->isTypeArrow())
		{
			//load the value of the left-hand of the arrow into EXP1
			VISIT_USEVAL(static_cast<ASTExprArrow&>(*host.left).left.get(), INITCTX);
			addOpcode(new OSetRegister(new VarArgument(CLASS_THISKEY), new VarArgument(EXP1)));
		}
		commentStartEnd(targ_sz, fmt::format("Class{} Params",func_comment));
		//goto
		string func_call_comment;
		if(parsing_user_class == puc_construct && func.getFlag(FUNCFLAG_CONSTRUCTOR)
			&& !host.isConstructor())
		{
			//A constructor calling another constructor to inherit its code
			//Use the alt label of the constructor, which is after the constructy bits
			addOpcode(new OSetRegister(new VarArgument(CLASS_THISKEY2), new VarArgument(CLASS_THISKEY)));
			addOpcode(new OCallFunc(new LabelArgument(func.getAltLabel(), true)));
			func_call_comment = fmt::format("Class{} Constructor Inheritance Call",func_comment);
		}
		else
		{
			addOpcode(new OCallFunc(new LabelArgument(funclabel, true)));
			if(host.isConstructor())
				func_call_comment = fmt::format("Class{} Constructor Call",func_comment);
			else func_call_comment = fmt::format("Class{} Call",func_comment);
		}
		commentBack(func_call_comment);

		if(func.getFlag(FUNCFLAG_NEVER_RETURN))
			commentBack("[Opt:NeverRet]");
		else
		{
			//pop the stack frame pointer
			addOpcode(new OPopRegister(new VarArgument(SFRAME)));
			addOpcode(new OPopRegister(new VarArgument(CLASS_THISKEY)));
		}
	}
	else
	{
		const string comment_pref = func.isInternal() || func.getFlag(FUNCFLAG_INTERNAL) ? "Int." : "Usr";
		int32_t funclabel = func.getLabel();
		
		bool vargs = func.getFlag(FUNCFLAG_VARARGS);
		bool user_vargs = vargs && !func.isInternal();
		auto num_used_params = host.parameters.size();
		auto num_actual_params = func.paramTypes.size() - (user_vargs ? 1 : 0);
		if(host.left->isTypeArrow())
			--num_actual_params; //Don't count the arrow param!
		size_t vargcount = 0;
		size_t used_opt_params = 0;
		int v = num_used_params-num_actual_params;
		(v>0 ? vargcount : used_opt_params) = abs(v);

		//push the stack frame pointer
		if(!never_ret)
		{
			addOpcode(new OPushRegister(new VarArgument(SFRAME)));
		}
		targ_sz = commentTarget();
		
		//push the parameters, in forward order
		size_t param_indx = 0;
		for (; param_indx < num_used_params-vargcount; ++param_indx)
		{
			auto& arg = host.parameters.at(param_indx);
			bool unused = !func.isInternal() && !func.getFlag(FUNCFLAG_INTERNAL) && func.paramDatum.size() > param_indx && func.paramDatum[param_indx]->is_erased();
			//Compile-time constants can be optimized slightly...
			if(auto val = arg->getCompileTimeValue(this, scope))
			{
				if(!unused)
					addOpcode(new OPushImmediate(new LiteralArgument(*val)));
			}
			else if(unused)
			{
				sidefx_visit(arg, INITCTX);
			}
			else
			{
				VISIT_USEVAL(arg, INITCTX);
				push_param();
			}
		}

		if(user_vargs)
		{
			auto array_type = dynamic_cast<const DataTypeArray*>(func.paramTypes.back());
			int element_script_object_type_id = (int)array_type->getElementType().getScriptObjectTypeId();

			//push the vargs, in forward order
			for (size_t param_indx = num_used_params-vargcount; param_indx < num_used_params; ++param_indx)
			{
				auto& arg = host.parameters.at(param_indx);
				//Compile-time constants can be optimized slightly...
				if(auto val = arg->getCompileTimeValue(this, scope))
					addOpcode(new OPushVargV(new LiteralArgument(*val)));
				else
				{
					VISIT_USEVAL(arg, INITCTX);
					push_param(true);
				}
			}
			addOpcode(new OMakeVargArray(new LiteralArgument(element_script_object_type_id)));
			commentBack("Allocate Vargs array");
			addOpcode(new OPushRegister(new VarArgument(EXP1)));
			commentBack("Push the Vargs array pointer");
		}

		if(vargs && vargcount && !user_vargs)
		{
			//push the vargs, in forward order
			for (; param_indx < host.parameters.size(); ++param_indx)
			{
				auto& arg = host.parameters.at(param_indx);
				//Compile-time constants can be optimized slightly...
				if(auto val = arg->getCompileTimeValue(this, scope))
					addOpcode(new OPushVargV(new LiteralArgument(*val)));
				else
				{
					VISIT_USEVAL(arg, INITCTX);
					push_param(true);
				}
			}
		}

		commentStartEnd(targ_sz, fmt::format("{}{} Params",comment_pref,func_comment));
		//goto
		addOpcode(new OCallFunc(new LabelArgument(funclabel, true)));
		commentBack(fmt::format("{}{} Call",comment_pref,func_comment));
		if(never_ret)
			commentBack("[Opt:NeverRet]");
		else
		{
			//pop the stack frame pointer
			addOpcode(new OPopRegister(new VarArgument(SFRAME)));
			
			if(host.left->isTypeArrow())
			{
				ASTExprArrow* arr = static_cast<ASTExprArrow*>(host.left.get());
				if(arr->left->getWriteType(scope, this) && !arr->left->isConstant())
				{
					if(func.getIntFlag(IFUNCFLAG_REASSIGNPTR))
					{
						bool isVoid = func.returnType->isVoid();
						if(!isVoid) addOpcode(new OPushRegister(new VarArgument(EXP1)));
						addOpcode(new OSetRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
						LValBOHelper helper(program, this);
						helper.parsing_user_class = parsing_user_class;
						arr->left->execute(helper, INITC_CTXT);
						addOpcodes(helper.getResult());
						if(!isVoid) addOpcode(new OPopRegister(new VarArgument(EXP1)));
					}
				}
				else if(func.getIntFlag(IFUNCFLAG_REASSIGNPTR)) //This is likely a mistake in the script... give the user a warning.
				{
					handleError(CompileError::BadReassignCall(&host, func.getUnaliasedSignature().asString()));
				}
			}
		}
	}
	
	//Allocate string/array literals retroactively
	INITC_INIT();
}

void BuildOpcodes::caseExprNegate(ASTExprNegate& host, void* param)
{
	if (auto val = host.getCompileTimeValue(this, scope))
	{
		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*val)));
		return;
	}
	SIDEFX_UNOP();
	
	visit(host.operand.get(), param);
	addOpcode(new OSubImmediate2(new LiteralArgument(0), new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprDelete(ASTExprDelete& host, void* param)
{
	VISIT_USEVAL(host.operand.get(), param);
	addOpcode(new OFreeObject(new VarArgument(EXP1)));
	deprecWarn(&host, "The operator", "delete", "This operator no longer does anything. Objects are freed automatically when they become unreachable");
}

void BuildOpcodes::caseExprNot(ASTExprNot& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	SIDEFX_UNOP();
	
	visit(host.operand.get(), param);
	
	bool decret = *lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL);
	if(host.isInverted())
	{
		if(decret)
			addOpcode(new OCastBoolF(new VarArgument(EXP1)));
		else
			addOpcode(new OCastBoolI(new VarArgument(EXP1)));
	}
	else
	{
		addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
		auto cmp = CMP_EQ;
		if(!decret)
			cmp |= CMP_SETI;
		addOpcode(new OSetCompare(new VarArgument(EXP1), new CompareArgument(cmp)));
	}
}

void BuildOpcodes::caseExprBitNot(ASTExprBitNot& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	SIDEFX_UNOP();
	
	visit(host.operand.get(), param);
	
	if(*lookupOption(*scope, CompileOption::OPT_BINARY_32BIT)
	   || host.operand.get()->isLong(scope, this))
		addOpcode(new O32BitNot(new VarArgument(EXP1)));
	else
		addOpcode(new ONot(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprIncrement(ASTExprIncrement& host, void* param)
{
	vector<shared_ptr<Opcode>> ops;
	
	// Increment EXP1
	addOpcode2(ops, new OAddImmediate(new VarArgument(EXP1),new LiteralArgument(10000)));
	
	if(host.is_pre)
		buildPreOp(host.operand.get(), param, ops);
	else buildPostOp(host.operand.get(), param, ops);
}

void BuildOpcodes::caseExprDecrement(ASTExprDecrement& host, void* param)
{
	vector<shared_ptr<Opcode>> ops;
	
	// Increment EXP1
	addOpcode2(ops, new OSubImmediate(new VarArgument(EXP1),new LiteralArgument(10000)));
	
	if(host.is_pre)
		buildPreOp(host.operand.get(), param, ops);
	else buildPostOp(host.operand.get(), param, ops);
}

static bool booltree_decret = false;
optional<bool> BuildOpcodes::rec_booltree_shortcircuit(BoolTreeNode& node, int parentMode, int truelbl, int falselbl, void* param)
{
	switch(node.mode)
	{
		case BoolTreeNode::MODE_LEAF:
		{
			bool _and = parentMode == BoolTreeNode::MODE_AND;
			auto val = node.leaf->getCompileTimeValue(this, scope);
			if(val)
			{
				if(_and ? *val : !*val)
					break; //just continue
				//fail, short-circuit
				if(_and)
				{
					addOpcode(new OGotoImmediate(new LabelArgument(falselbl)));
					return false;
				}
				else
				{
					addOpcode(new OGotoImmediate(new LabelArgument(truelbl)));
					return true;
				}
			}
			else
			{
				visit(node.leaf.get(), param);
				if(auto cmp = eatSetCompare())
				{
					int c = *cmp & ~CMP_SETI;
					if(_and)
						addOpcode(new OGotoCompare(new LabelArgument(falselbl), new CompareArgument(INVERT_CMP(c))));
					else addOpcode(new OGotoCompare(new LabelArgument(truelbl), new CompareArgument(c)));
				}
				else
				{
					addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
					if(_and)
						addOpcode(new OGotoCompare(new LabelArgument(falselbl), new CompareArgument(CMP_EQ)));
					else addOpcode(new OGotoCompare(new LabelArgument(truelbl), new CompareArgument(CMP_NE)));
				}
			}
			break;
		}
		case BoolTreeNode::MODE_AND:
		{
			optional<bool> ret;
			for(size_t q = 0; q < node.branch.size(); ++q)
			{
				auto& child = node.branch[q];
				bool last = q == node.branch.size()-1;
				if(last)
				{
					ret = rec_booltree_shortcircuit(child, node.mode, truelbl, falselbl, param);
					addOpcode(new OGotoImmediate(new LabelArgument(truelbl)));
				}
				else
				{
					auto nextlbl = ScriptParser::getUniqueLabelID();
					ret = rec_booltree_shortcircuit(child, node.mode, nextlbl, falselbl, param);
					addOpcode(new ONoOp(nextlbl));
				}
				if(ret)
					return *ret;
			}
			break;
		}
		case BoolTreeNode::MODE_OR:
		{
			optional<bool> ret;
			for(size_t q = 0; q < node.branch.size(); ++q)
			{
				auto& child = node.branch[q];
				bool last = q == node.branch.size()-1;
				if(last)
				{
					ret = rec_booltree_shortcircuit(child, node.mode, truelbl, falselbl, param);
					addOpcode(new OGotoImmediate(new LabelArgument(falselbl)));
				}
				else
				{
					auto nextlbl = ScriptParser::getUniqueLabelID();
					ret = rec_booltree_shortcircuit(child, node.mode, truelbl, nextlbl, param);
					addOpcode(new ONoOp(nextlbl));
				}
				if(ret)
					return *ret;
			}
			break;
		}
	}
	return nullopt;
}
void BuildOpcodes::rec_booltree_noshort(BoolTreeNode& node, int parentMode, void* param)
{
	bool _and = parentMode == BoolTreeNode::MODE_AND;
	bool _or = parentMode == BoolTreeNode::MODE_OR;
	switch(node.mode)
	{
		case BoolTreeNode::MODE_LEAF:
		{
			auto val = node.leaf->getCompileTimeValue(this, scope);
			if(val)
			{
				if(_and)
				{
					if(!*val)
						addOpcode(new OStackWriteAtVV(new LiteralArgument(0), new LiteralArgument(0)));
					return;
				}
				else if(_or)
				{
					if(*val)
						addOpcode(new OStackWriteAtVV(new LiteralArgument(booltree_decret ? 1 : 10000), new LiteralArgument(0)));
					return;
				}
			}
			else
			{
				visit(node.leaf.get(), param);
				int c = 0;
				if(auto cmp = eatSetCompare())
					c = *cmp & ~CMP_SETI;
				else
				{
					addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
					c = CMP_NE;
				}
				int val = booltree_decret ? 1 : 10000;
				if(_and)
				{
					val = 0;
					c = INVERT_CMP(c);
				}
				addOpcode(new OStackWriteAtVV_If(new LiteralArgument(val), new LiteralArgument(0), new CompareArgument(c)));
			}
			return;
		}
		case BoolTreeNode::MODE_AND:
		{
			addOpcode(new OPushImmediate(new LiteralArgument(booltree_decret ? 1 : 10000)));
			for(size_t q = 0; q < node.branch.size(); ++q)
				rec_booltree_noshort(node.branch[q], node.mode, param);
			addOpcode(new OPopRegister(new VarArgument(EXP1)));
			break;
		}
		case BoolTreeNode::MODE_OR:
		{
			addOpcode(new OPushImmediate(new LiteralArgument(0)));
			for(size_t q = 0; q < node.branch.size(); ++q)
				rec_booltree_noshort(node.branch[q], node.mode, param);
			addOpcode(new OPopRegister(new VarArgument(EXP1)));
			break;
		}
	}
	if(_and || _or)
	{
		addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
		int val = booltree_decret ? 1 : 10000;
		int c = CMP_NE;
		if(_and)
		{
			val = 0;
			c = INVERT_CMP(c);
		}
		addOpcode(new OStackWriteAtVV_If(new LiteralArgument(val), new LiteralArgument(0), new CompareArgument(c)));
	}
}

void BuildOpcodes::caseExprBoolTree(ASTExprBoolTree& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	BoolTreeNode& node = host.root;
	bool _and = node.mode == BoolTreeNode::MODE_AND;
	bool decret = booltree_decret = *lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL);
	
	bool short_circuit = *lookupOption(*scope, CompileOption::OPT_SHORT_CIRCUIT) != 0;
	
	auto sidefx = sidefx_only ;
	sidefx_only = false;
	if(short_circuit)
	{
		auto endlbl = ScriptParser::getUniqueLabelID();
		auto truelbl = sidefx ? endlbl : ScriptParser::getUniqueLabelID();
		auto falselbl = sidefx ? endlbl : ScriptParser::getUniqueLabelID();
		rec_booltree_shortcircuit(node, -1, truelbl, falselbl, param);
		if(sidefx)
			addOpcode(new ONoOp(endlbl));
		else if(_and)
		{
			addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(decret ? 1 : 10000)));
			backOpcode()->setLabel(truelbl);
			addOpcode(new OGotoImmediate(new LabelArgument(endlbl)));
			addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
			backOpcode()->setLabel(falselbl);
			addOpcode(new ONoOp(endlbl));
		}
		else
		{
			addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
			backOpcode()->setLabel(falselbl);
			addOpcode(new OGotoImmediate(new LabelArgument(endlbl)));
			addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(decret ? 1 : 10000)));
			backOpcode()->setLabel(truelbl);
			addOpcode(new ONoOp(endlbl));
		}
	}
	else
	{
		rec_booltree_noshort(node, -1, param);
	}
	sidefx_only = sidefx;
}
void BuildOpcodes::caseExprAnd(ASTExprAnd& host, void* param)
{
	if(host.tree) //should ALWAYS go in here now? -Em
	{
		visit(host.tree.get(), param);
		return;
	}
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	SIDEFX_BINOP();
	
	bool decret = *lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL)!=0;
	bool short_circuit = *lookupOption(*scope, CompileOption::OPT_SHORT_CIRCUIT) != 0;
	if(auto val = host.left->getCompileTimeValue(this, scope))
	{
		if(*val)
		{
			visit(host.right.get(), param);
			if(decret)
				addOpcode(new OCastBoolF(new VarArgument(EXP1)));
			else
				addOpcode(new OCastBoolI(new VarArgument(EXP1)));
			return;
		}
		else //if(!short_circuit) //if short circuit were true, the top early return would have triggered.
		{
			visit(host.right.get(), param); //Visit in case it has 'side effects'
			addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
			return;
		}
	}
	else if(auto val = host.right->getCompileTimeValue(this, scope))
	{
		if(*val)
		{
			visit(host.left.get(), param);
			if(decret)
				addOpcode(new OCastBoolF(new VarArgument(EXP1)));
			else
				addOpcode(new OCastBoolI(new VarArgument(EXP1)));
			return;
		}
		else
		{
			visit(host.left.get(), param); //Visit in case it has 'side effects'
			addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
			return;
		}
	}
	uint cmp;
	if(short_circuit)
	{
		int32_t skip = ScriptParser::getUniqueLabelID();
		//Get left
		visit(host.left.get(), param);
		//Check left, skip if false
		addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
		addOpcode(new OGotoTrueImmediate(new LabelArgument(skip)));
		//Get right
		visit(host.right.get(), param);
		Opcode* ocode =  new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0));
		ocode->setLabel(skip);
		addOpcode(ocode);
		cmp = CMP_NE;
		if(!decret)
			cmp |= CMP_SETI;
		addOpcode(new OSetCompare(new VarArgument(EXP1), new CompareArgument(cmp)));
	}
	else
	{
		addOpcode(new OPushImmediate(new LiteralArgument(decret ? 1 : 10000))); //push true
		//Get left
		visit(host.left.get(), param);
		//If false, set retval to 0
		int cmp = CMP_EQ;
		if(auto cmpval = eatSetCompare())
			cmp = INVERT_CMP(*cmpval) & ~CMP_SETI;
		else addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
		addOpcode(new OStackWriteAtVV_If(new LiteralArgument(0),
			new LiteralArgument(0), new CompareArgument(cmp)));
		//Get right
		visit(host.right.get(), param);
		//If false, set retval to 0
		cmp = CMP_EQ;
		if(auto cmpval = eatSetCompare())
			cmp = INVERT_CMP(*cmpval) & ~CMP_SETI;
		else addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
		addOpcode(new OStackWriteAtVV_If(new LiteralArgument(0),
			new LiteralArgument(0), new CompareArgument(cmp)));
		//Pop retval
		addOpcode(new OPopRegister(new VarArgument(EXP1)));
	}
}

void BuildOpcodes::caseExprOr(ASTExprOr& host, void* param)
{
	if(host.tree) //should ALWAYS go in here now? -Em
	{
		visit(host.tree.get(), param);
		return;
	}
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	SIDEFX_BINOP();
	
	bool decret = *lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL)!=0;
	bool short_circuit = *lookupOption(*scope, CompileOption::OPT_SHORT_CIRCUIT) != 0;
	if(auto val = host.left->getCompileTimeValue(this, scope))
	{
		if(*val) //if short circuit were true, the top early return would have triggered.
		{
			visit(host.right.get(), param); //Visit in case it has 'side effects'
			addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(decret ? 1 : 10000)));
			return;
		}
		else
		{
			visit(host.right.get(), param);
			if(decret)
				addOpcode(new OCastBoolF(new VarArgument(EXP1)));
			else
				addOpcode(new OCastBoolI(new VarArgument(EXP1)));
			return;
		}
	}
	else if(auto val = host.right->getCompileTimeValue(this, scope))
	{
		if(*val)
		{
			visit(host.left.get(), param); //Visit in case it has 'side effects'
			addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(decret ? 1 : 10000)));
			return;
		}
		else
		{
			visit(host.left.get(), param);
			if(decret)
				addOpcode(new OCastBoolF(new VarArgument(EXP1)));
			else
				addOpcode(new OCastBoolI(new VarArgument(EXP1)));
			return;
		}
	}
	if(short_circuit)
	{
		int32_t skip = ScriptParser::getUniqueLabelID();
		//Get left
		visit(host.left.get(), param);
		//Check left, skip if true
		addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
		addOpcode(new OGotoFalseImmediate(new LabelArgument(skip)));
		//Get rightx
		//Get right
		visit(host.right.get(), param);
		addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
		//Set output
		Opcode* ocode = new OSetCompare(new VarArgument(EXP1), new CompareArgument(CMP_NE | (decret?CMP_SETI:0)));
		ocode->setLabel(skip);
		addOpcode(ocode);
	}
	else
	{
		auto trueval = decret ? 1 : 10000;
		addOpcode(new OPushImmediate(new LiteralArgument(0))); //push false
		//Get left
		visit(host.left.get(), param);
		//If true, set retval to 1/10000
		int cmp = CMP_NE;
		if(auto cmpval = eatSetCompare())
			cmp = *cmpval & ~CMP_SETI;
		else addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
		addOpcode(new OStackWriteAtVV_If(new LiteralArgument(trueval),
			new LiteralArgument(0), new CompareArgument(cmp)));
		//Get right
		visit(host.right.get(), param);
		//If true, set retval to 1/10000
		cmp = CMP_NE;
		if(auto cmpval = eatSetCompare())
			cmp = *cmpval & ~CMP_SETI;
		else addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
		addOpcode(new OStackWriteAtVV_If(new LiteralArgument(trueval),
			new LiteralArgument(0), new CompareArgument(cmp)));
		//Pop retval
		addOpcode(new OPopRegister(new VarArgument(EXP1)));
	}
}

void BuildOpcodes::caseExprGT(ASTExprGT& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	SIDEFX_BINOP();
	
	bool decret = *lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL)!=0;
	compareExprs(host.left.get(), host.right.get(), param);
	auto cmp = CMP_GT;
	if(!decret)
		cmp |= CMP_SETI;
	addOpcode(new OSetCompare(new VarArgument(EXP1), new CompareArgument(cmp)));
}

void BuildOpcodes::caseExprGE(ASTExprGE& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	SIDEFX_BINOP();
	
	bool decret = *lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL)!=0;
	compareExprs(host.left.get(), host.right.get(), param);
	auto cmp = CMP_GE;
	if(!decret)
		cmp |= CMP_SETI;
	addOpcode(new OSetCompare(new VarArgument(EXP1), new CompareArgument(cmp)));
}

void BuildOpcodes::caseExprLT(ASTExprLT& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	SIDEFX_BINOP();
	
	bool decret = *lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL)!=0;
	compareExprs(host.left.get(), host.right.get(), param);
	auto cmp = CMP_LT;
	if(!decret)
		cmp |= CMP_SETI;
	addOpcode(new OSetCompare(new VarArgument(EXP1), new CompareArgument(cmp)));
}

void BuildOpcodes::caseExprLE(ASTExprLE& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	SIDEFX_BINOP();
	
	bool decret = *lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL)!=0;
	compareExprs(host.left.get(), host.right.get(), param);
	auto cmp = CMP_LE;
	if(!decret)
		cmp |= CMP_SETI;
	addOpcode(new OSetCompare(new VarArgument(EXP1), new CompareArgument(cmp)));
}

void BuildOpcodes::caseExprEQ(ASTExprEQ& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	SIDEFX_BINOP();
	
	// Special case for booleans.
	DataType const* ltype = host.left->getReadType(scope, this);
	DataType const* rtype = host.right->getReadType(scope, this);
	bool isBoolean = (*ltype == DataType::BOOL || *rtype == DataType::BOOL || *ltype == DataType::CBOOL || *rtype == DataType::CBOOL);
	
	bool decret = *lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL)!=0;
	compareExprs(host.left.get(), host.right.get(), param);
	auto cmp = CMP_EQ;
	if(isBoolean)
		cmp |= CMP_BOOL;
	if(!decret)
		cmp |= CMP_SETI;
	addOpcode(new OSetCompare(new VarArgument(EXP1), new CompareArgument(cmp)));
}

void BuildOpcodes::caseExprNE(ASTExprNE& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	SIDEFX_BINOP();
	
	// Special case for booleans.
	DataType const* ltype = host.left->getReadType(scope, this);
	DataType const* rtype = host.right->getReadType(scope, this);
	bool isBoolean = (*ltype == DataType::BOOL || *rtype == DataType::BOOL || *ltype == DataType::CBOOL || *rtype == DataType::CBOOL);
	
	bool decret = *lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL)!=0;
	compareExprs(host.left.get(), host.right.get(), param);
	auto cmp = CMP_NE;
	if(isBoolean)
		cmp |= CMP_BOOL;
	if(!decret)
		cmp |= CMP_SETI;
	addOpcode(new OSetCompare(new VarArgument(EXP1), new CompareArgument(cmp)));
}

void BuildOpcodes::caseExprAppxEQ(ASTExprAppxEQ& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	SIDEFX_BINOP();
	
	auto lval = host.left->getCompileTimeValue(this, scope);
	auto rval = host.right->getCompileTimeValue(this, scope);
	if(lval)
	{
		visit(host.right.get(), param);
		addOpcode(new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(*lval)));
	}
	else if (rval)
	{
		visit(host.left.get(), param);
		addOpcode(new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(*rval)));
	}
	else
	{
		//compute both sides
		visit(host.left.get(), param);
		addOpcode(new OPushRegister(new VarArgument(EXP1)));
		visit(host.right.get(), param);
		addOpcode(new OPopRegister(new VarArgument(EXP2)));
	}
	addOpcode(new OSubRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
	addOpcode(new OAbsRegister(new VarArgument(EXP1)));
	
	addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(*lookupOption(*scope, CompileOption::OPT_APPROX_EQUAL_MARGIN))));
	
	bool decret = *lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL);
	auto cmp = CMP_LE;
	if(!decret)
		cmp |= CMP_SETI;
	addOpcode(new OSetCompare(new VarArgument(EXP1), new CompareArgument(cmp)));
}

void BuildOpcodes::caseExprXOR(ASTExprXOR& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	SIDEFX_BINOP();
	
	bool decret = *lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL)!=0;
	
	compareExprs(host.left.get(), host.right.get(), param);
	
	auto cmp = CMP_NE | CMP_BOOL;
	if(!decret)
		cmp |= CMP_SETI;
	addOpcode(new OSetCompare(new VarArgument(EXP1), new CompareArgument(cmp)));
}

void BuildOpcodes::caseExprPlus(ASTExprPlus& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	SIDEFX_BINOP();
	
	auto lval = host.left->getCompileTimeValue(this, scope);
	auto rval = host.right->getCompileTimeValue(this, scope);
	if(lval)
	{
		visit(host.right.get(), param);
		if((*lval)==0) // 0 + y? Just do y!
			return;
		addOpcode(new OAddImmediate(new VarArgument(EXP1), new LiteralArgument(*lval)));
	}
	else if(rval)
	{
		visit(host.left.get(), param);
		if((*rval)==0) // x + 0? Just do x!
			return;
		addOpcode(new OAddImmediate(new VarArgument(EXP1), new LiteralArgument(*rval)));
	}
	else
	{
		// Compute both sides.
		visit(host.left.get(), param);
		addOpcode(new OPushRegister(new VarArgument(EXP1)));
		visit(host.right.get(), param);
		addOpcode(new OPopRegister(new VarArgument(EXP2)));
		addOpcode(new OAddRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
	}
}

void BuildOpcodes::caseExprMinus(ASTExprMinus& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	SIDEFX_BINOP();
	
	auto lval = host.left->getCompileTimeValue(this, scope);
	auto rval = host.right->getCompileTimeValue(this, scope);
	if(lval)
	{
		visit(host.right.get(), param);
		addOpcode(new OSubImmediate2(new LiteralArgument(*lval), new VarArgument(EXP1)));
	}
	else if(rval)
	{
		visit(host.left.get(), param);
		if((*rval)==0) // x - 0? Just do x!
			return;
		addOpcode(new OSubImmediate(new VarArgument(EXP1), new LiteralArgument(*rval)));
	}
	else
	{
		// Compute both sides.
		visit(host.left.get(), param);
		addOpcode(new OPushRegister(new VarArgument(EXP1)));
		visit(host.right.get(), param);
		addOpcode(new OPopRegister(new VarArgument(EXP2)));
		addOpcode(new OSubRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
		addOpcode(new OSetRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
	}
}

void BuildOpcodes::caseExprTimes(ASTExprTimes& host, void *param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	SIDEFX_BINOP();
	
	auto lval = host.left->getCompileTimeValue(this, scope);
	auto rval = host.right->getCompileTimeValue(this, scope);
	if(lval)
	{
		visit(host.right.get(), param);
		if((*lval)==10000L) // 1 * y? Just do y!
			return;
		addOpcode(new OMultImmediate(new VarArgument(EXP1), new LiteralArgument(*lval)));
	}
	if(rval)
	{
		visit(host.left.get(), param);
		if((*rval)==10000L) // x * 1? Just do x!
			return;
		addOpcode(new OMultImmediate(new VarArgument(EXP1), new LiteralArgument(*rval)));
	}
	else
	{
		// Compute both sides.
		visit(host.left.get(), param);
		addOpcode(new OPushRegister(new VarArgument(EXP1)));
		visit(host.right.get(), param);
		addOpcode(new OPopRegister(new VarArgument(EXP2)));
		addOpcode(new OMultRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
	}
}

void BuildOpcodes::caseExprExpn(ASTExprExpn& host, void *param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	SIDEFX_BINOP();
	
	bool do_long = host.left.get()->isLong(scope, this) || host.right.get()->isLong(scope, this);
	
	auto lval = host.left->getCompileTimeValue(this, scope);
	auto rval = host.right->getCompileTimeValue(this, scope);
	if(do_long)
	{
		if(lval)
		{
			if(*lval == 1) //1 ^ x? Always '1'
			{
				CONST_VAL(1);
				return;
			}
			visit(host.right.get(), param);
			addOpcode(new OLPowImmediate2(new LiteralArgument(*lval), new VarArgument(EXP1)));
		}
		if(rval)
		{
			if((*rval)==0) // x ^ 0? Always '1'. Yes even for 0^0, as we define it.
			{
				CONST_VAL(1);
				return;
			}
			visit(host.left.get(), param);
			addOpcode(new OLPowImmediate(new VarArgument(EXP1), new LiteralArgument(*rval)));
		}
		else
		{
			// Compute both sides.
			visit(host.right.get(), param);
			addOpcode(new OPushRegister(new VarArgument(EXP1)));
			visit(host.left.get(), param);
			addOpcode(new OPopRegister(new VarArgument(EXP2)));
			addOpcode(new OLPowRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
		}
	}
	else
	{
		if(lval)
		{
			if(*lval == 10000) //1 ^ x? Always '1'
			{
				CONST_VAL(10000);
				return;
			}
			visit(host.right.get(), param);
			addOpcode(new OPowImmediate2(new LiteralArgument(*lval), new VarArgument(EXP1)));
		}
		if(rval)
		{
			if((*rval)==0) // x ^ 0? Always '1'. Yes even for 0^0, as we define it.
			{
				CONST_VAL(10000);
				return;
			}
			visit(host.left.get(), param);
			addOpcode(new OPowImmediate(new VarArgument(EXP1), new LiteralArgument(*rval)));
		}
		else
		{
			// Compute both sides.
			visit(host.right.get(), param);
			addOpcode(new OPushRegister(new VarArgument(EXP1)));
			visit(host.left.get(), param);
			addOpcode(new OPopRegister(new VarArgument(EXP2)));
			addOpcode(new OPowRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
		}
	}
}

void BuildOpcodes::caseExprDivide(ASTExprDivide& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	SIDEFX_BINOP();
	
	auto lval = host.left->getCompileTimeValue(this, scope);
	auto rval = host.right->getCompileTimeValue(this, scope);
	if(lval)
	{
		visit(host.right.get(), param);
		addOpcode(new ODivImmediate2(new LiteralArgument(*lval), new VarArgument(EXP1)));
	}
	if(rval)
	{
		visit(host.left.get(), param);
		if((*rval)==10000L) // x / 1? Just do x!
			return;
		if((*rval)==0) //Div by 0! Warn
		{
			handleError(CompileError::DivByZero(&host));
		}
		addOpcode(new ODivImmediate(new VarArgument(EXP1), new LiteralArgument(*rval)));
	}
	else
	{
		// Compute both sides.
		visit(host.left.get(), param);
		addOpcode(new OPushRegister(new VarArgument(EXP1)));
		visit(host.right.get(), param);
		addOpcode(new OPopRegister(new VarArgument(EXP2)));
		addOpcode(new ODivRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
		addOpcode(new OSetRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
	}
}

void BuildOpcodes::caseExprModulo(ASTExprModulo& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	SIDEFX_BINOP();
	
	auto lval = host.left->getCompileTimeValue(this, scope);
	auto rval = host.right->getCompileTimeValue(this, scope);
	if(lval)
	{
		visit(host.right.get(), param);
		addOpcode(new OModuloImmediate2(new LiteralArgument(*lval), new VarArgument(EXP1)));
	}
	if(rval)
	{
		visit(host.left.get(), param);
		if((*rval)==0) //Mod by 0! Warn
		{
			handleError(CompileError::DivByZero(&host));
		}
		addOpcode(new OModuloImmediate(new VarArgument(EXP1), new LiteralArgument(*rval)));
	}
	else
	{
		// Compute both sides.
		visit(host.left.get(), param);
		addOpcode(new OPushRegister(new VarArgument(EXP1)));
		visit(host.right.get(), param);
		addOpcode(new OPopRegister(new VarArgument(EXP2)));
		addOpcode(new OModuloRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
		addOpcode(new OSetRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
	}
}

void BuildOpcodes::caseExprBitAnd(ASTExprBitAnd& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	SIDEFX_BINOP();
	
	bool do_long = *lookupOption(*scope, CompileOption::OPT_BINARY_32BIT)
		|| host.left.get()->isLong(scope, this) || host.right.get()->isLong(scope, this);
	
	auto lval = host.left->getCompileTimeValue(this, scope);
	auto rval = host.right->getCompileTimeValue(this, scope);
	if(lval)
	{
		visit(host.right.get(), param);
		if(do_long)
			addOpcode(new O32BitAndImmediate(new VarArgument(EXP1), new LiteralArgument(*lval)));
		else
			addOpcode(new OAndImmediate(new VarArgument(EXP1), new LiteralArgument(*lval)));
	}
	else if (rval)
	{
		visit(host.left.get(), param);
		if(do_long)
			addOpcode(new O32BitAndImmediate(new VarArgument(EXP1), new LiteralArgument(*rval)));
		else
			addOpcode(new OAndImmediate(new VarArgument(EXP1), new LiteralArgument(*rval)));
	}
	else
	{
		//compute both sides
		visit(host.left.get(), param);
		addOpcode(new OPushRegister(new VarArgument(EXP1)));
		visit(host.right.get(), param);
		addOpcode(new OPopRegister(new VarArgument(EXP2)));
		if(do_long)
			addOpcode(new O32BitAndRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
		else
			addOpcode(new OAndRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
	}
}

void BuildOpcodes::caseExprBitOr(ASTExprBitOr& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	SIDEFX_BINOP();
	
	bool do_long = *lookupOption(*scope, CompileOption::OPT_BINARY_32BIT)
		|| host.left.get()->isLong(scope, this) || host.right.get()->isLong(scope, this);
	
	auto lval = host.left->getCompileTimeValue(this, scope);
	auto rval = host.right->getCompileTimeValue(this, scope);
	if(lval)
	{
		visit(host.right.get(), param);
		if(do_long)
			addOpcode(new O32BitOrImmediate(new VarArgument(EXP1), new LiteralArgument(*lval)));
		else
			addOpcode(new OOrImmediate(new VarArgument(EXP1), new LiteralArgument(*lval)));
	}
	else if (rval)
	{
		visit(host.left.get(), param);
		if(do_long)
			addOpcode(new O32BitOrImmediate(new VarArgument(EXP1), new LiteralArgument(*rval)));
		else
			addOpcode(new OOrImmediate(new VarArgument(EXP1), new LiteralArgument(*rval)));
	}
	else
	{
		//compute both sides
		visit(host.left.get(), param);
		addOpcode(new OPushRegister(new VarArgument(EXP1)));
		visit(host.right.get(), param);
		addOpcode(new OPopRegister(new VarArgument(EXP2)));
		if(do_long)
			addOpcode(new O32BitOrRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
		else
			addOpcode(new OOrRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
	}
}

void BuildOpcodes::caseExprBitXor(ASTExprBitXor& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	SIDEFX_BINOP();
	
	bool do_long = *lookupOption(*scope, CompileOption::OPT_BINARY_32BIT)
		|| host.left.get()->isLong(scope, this) || host.right.get()->isLong(scope, this);
	
	auto lval = host.left->getCompileTimeValue(this, scope);
	auto rval = host.right->getCompileTimeValue(this, scope);
	if(lval)
	{
		visit(host.right.get(), param);
		if(do_long)
			addOpcode(new O32BitXorImmediate(new VarArgument(EXP1), new LiteralArgument(*lval)));
		else
			addOpcode(new OXorImmediate(new VarArgument(EXP1), new LiteralArgument(*lval)));
	}
	else if (rval)
	{
		visit(host.left.get(), param);
		if(do_long)
			addOpcode(new O32BitXorImmediate(new VarArgument(EXP1), new LiteralArgument(*rval)));
		else
			addOpcode(new OXorImmediate(new VarArgument(EXP1), new LiteralArgument(*rval)));
	}
	else
	{
		//compute both sides
		visit(host.left.get(), param);
		addOpcode(new OPushRegister(new VarArgument(EXP1)));
		visit(host.right.get(), param);
		addOpcode(new OPopRegister(new VarArgument(EXP2)));
		if(do_long)
			addOpcode(new O32BitXorRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
		else
			addOpcode(new OXorRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
	}
}

void BuildOpcodes::caseExprLShift(ASTExprLShift& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	SIDEFX_BINOP();
	
	bool do_long = *lookupOption(*scope, CompileOption::OPT_BINARY_32BIT)
		|| host.left.get()->isLong(scope, this);
	
	auto lval = host.left->getCompileTimeValue(this, scope);
	auto rval = host.right->getCompileTimeValue(this, scope);
	if(lval)
	{
		visit(host.right.get(), param);
		addOpcode(new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(*lval)));
	}
	else if (rval)
	{
		visit(host.left.get(), param);
		if(do_long)
			addOpcode(new O32BitLShiftImmediate(new VarArgument(EXP1), new LiteralArgument(*rval)));
		else
			addOpcode(new OLShiftImmediate(new VarArgument(EXP1), new LiteralArgument(*rval)));
		return;
	}
	else
	{
		//compute both sides
		visit(host.left.get(), param);
		addOpcode(new OPushRegister(new VarArgument(EXP1)));
		visit(host.right.get(), param);
		addOpcode(new OPopRegister(new VarArgument(EXP2)));
	}
	if(do_long)
		addOpcode(new O32BitLShiftRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
	else
		addOpcode(new OLShiftRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
	addOpcode(new OSetRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprRShift(ASTExprRShift& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	SIDEFX_BINOP();
	
	bool do_long = *lookupOption(*scope, CompileOption::OPT_BINARY_32BIT)
		|| host.left.get()->isLong(scope, this);
	
	auto lval = host.left->getCompileTimeValue(this, scope);
	auto rval = host.right->getCompileTimeValue(this, scope);
	if(lval)
	{
		visit(host.right.get(), param);
		addOpcode(new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(*lval)));
	}
	else if (rval)
	{
		visit(host.left.get(), param);
		if(do_long)
			addOpcode(new O32BitRShiftImmediate(new VarArgument(EXP1), new LiteralArgument(*rval)));
		else
			addOpcode(new ORShiftImmediate(new VarArgument(EXP1), new LiteralArgument(*rval)));
		return;
	}
	else
	{
		//compute both sides
		visit(host.left.get(), param);
		addOpcode(new OPushRegister(new VarArgument(EXP1)));
		visit(host.right.get(), param);
		addOpcode(new OPopRegister(new VarArgument(EXP2)));
	}
	if(do_long)
		addOpcode(new O32BitRShiftRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
	else
		addOpcode(new ORShiftRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
	addOpcode(new OSetRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprTernary(ASTTernaryExpr& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	auto lval = host.left->getCompileTimeValue(this, scope);
	if(lval)
	{
		if(*lval)
			visit(host.middle.get(), param);
		else visit(host.right.get(), param);
	}
	else
	{
		auto mval = host.middle->getCompileTimeValue(this, scope);
		auto rval = host.right->getCompileTimeValue(this, scope);
		
		VISIT_USEVAL(host.left.get(), param);
		int32_t elseif = ScriptParser::getUniqueLabelID();
		int32_t endif = ScriptParser::getUniqueLabelID();
		addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
		addOpcode(new OGotoTrueImmediate(new LabelArgument(elseif)));
		//Use middle section
		if(mval)
			addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*mval)));
		else visit(host.middle.get(), param);
		addOpcode(new OGotoImmediate(new LabelArgument(endif))); //Skip right
		//Use right section
		if(rval)
		{
			addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*rval)));
			backOpcode()->setLabel(elseif);
		}
		else
		{
			addOpcode(new ONoOp(elseif));
			visit(host.right.get(), param);
		}
		addOpcode(new ONoOp(endif)); //Add label for after right
	}
}

// Literals

void BuildOpcodes::caseNumberLiteral(ASTNumberLiteral& host, void*)
{
	if (auto cval = host.getCompileTimeValue(this, scope))
		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*cval)));
	else
	{
		pair<int32_t, bool> val = ScriptParser::parseLong(host.value->parseValue(scope), scope);

		if (!val.second)
			handleError(CompileError::ConstTrunc(&host, host.value->value));

		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(val.first)));
	}
}

void BuildOpcodes::caseCharLiteral(ASTCharLiteral& host, void*)
{
	if (auto cval = host.getCompileTimeValue(this, scope))
		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*cval)));
	else
	{
		pair<int32_t, bool> val = ScriptParser::parseLong(host.value->parseValue(scope), scope);

		if (!val.second)
			handleError(CompileError::ConstTrunc(&host, host.value->value));

		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(val.first)));
	}
}

void BuildOpcodes::caseBoolLiteral(ASTBoolLiteral& host, void*)
{
	CONST_VAL(*host.getCompileTimeValue(this, scope));
}

void BuildOpcodes::caseStringLiteral(ASTStringLiteral& host, void* param)
{
	OpcodeContext& context = *(OpcodeContext*)param;
	if (host.declaration) stringLiteralDeclaration(host, context);
	else stringLiteralFree(host, context);
}

void BuildOpcodes::stringLiteralDeclaration(
		ASTStringLiteral& host, OpcodeContext& context)
{
	ASTDataDecl& declaration = *host.declaration;
	Datum& manager = *declaration.manager;
	if(manager.is_erased()) //var unused, optimized away
		return;
	string const& data = host.value;

	// Grab the size from the declaration.
	int32_t size = -1;
	if (declaration.extraArrays.size() == 1)
	{
		ASTDataDeclExtraArray& extraArray = *declaration.extraArrays[0];
		if (auto totalSize = extraArray.getCompileTimeSize(this, scope))
			size = *totalSize;
		else if (extraArray.hasSize())
		{
			handleError(CompileError::ExprNotConstant(&host));
			return;
		}
	}

	// Otherwise, grab the number of elements as the size.
	if (size == -1) size = data.size() + 1;

	// Make sure the chosen size has enough space.
	if (size < int32_t(data.size() + 1))
	{
		handleError(CompileError::ArrayListStringTooLarge(&host));
		return;
	}

	// Create the array and store its id.
	if (manager.getGlobalId())
	{
		addOpcode(new OAllocateGlobalMemImmediate(
						  new VarArgument(EXP1),
						  new LiteralArgument(size * 10000L),
						  new LiteralArgument(0)));
	}
	else
	{
		addOpcode(new OAllocateMemImmediate(new VarArgument(EXP1),
											new LiteralArgument(size * 10000L),
											new LiteralArgument(0)));
	}

	addOpcode(new OWritePODString(new VarArgument(EXP1), new StringArgument(data)));
}

// "free" means it is not attached to a variable declaration (so it's either a param value, or used in an assignment)
void BuildOpcodes::stringLiteralFree(
		ASTStringLiteral& host, OpcodeContext& context)
{
	string data = host.value;
	int32_t size = data.size() + 1;

	addOpcode(new OAllocateMemImmediate(new VarArgument(EXP1), new LiteralArgument(size * 10000L), new LiteralArgument(0)));
	addOpcode(new OWritePODString(new VarArgument(EXP1), new StringArgument(data)));
}

void BuildOpcodes::caseArrayLiteral(ASTArrayLiteral& host, void* param)
{
	OpcodeContext& context = *(OpcodeContext*)param;
	if (host.declaration) arrayLiteralDeclaration(host, context);
	else arrayLiteralFree(host, context);
}

void BuildOpcodes::arrayLiteralDeclaration(
		ASTArrayLiteral& host, OpcodeContext& context)
{
	ASTDataDecl& declaration = *host.declaration;
	Datum& manager = *declaration.manager;
	if(manager.is_erased()) //var unused, optimized away
	{
		sidefx_visit_vec(host.elements, (void*)&context);
		return;
	}

	// Find the size.
	int32_t size = -1;
	// From this literal?
	if (host.size)
		if (auto s = host.size->getCompileTimeValue(this, scope))
			size = *s / 10000L;
	// From the declaration?
	if (size == -1 && declaration.extraArrays.size() == 1)
	{
		ASTDataDeclExtraArray& extraArray = *declaration.extraArrays[0];
		if (auto totalSize = extraArray.getCompileTimeSize(this, scope))
			size = *totalSize;
		else if (extraArray.hasSize())
		{
			handleError(CompileError::ExprNotConstant(&host));
			return;
		}
	}
	// Otherwise, grab the number of elements as the size.
	if (size == -1) size = host.elements.size();

	// Make sure we have a valid size.
	if (size < 1)
	{
		handleError(CompileError::ArrayTooSmall(&host));
		return;
	}
	
	// Make sure the chosen size has enough space.
	if (size < int32_t(host.elements.size()))
	{
		handleError(CompileError::ArrayListTooLarge(&host));
		return;
	}

	// TODO: host.readType_ is not being set for array literals. Can maybe fix in:
	// SemanticAnalyzer::caseArrayLiteral - see "// Otherwise, default to Untyped -Em"
	// For now just grab it here.
	auto& type = host.declaration->resolveType(scope, this);

	if (!dynamic_cast<const DataTypeArray*>(&type))
	{
		handleError(CompileError::Error(&host, fmt::format("Expected an array type")));
		return;
	}

	auto array_type = dynamic_cast<const DataTypeArray*>(&type);
	int element_script_object_type_id = (int)array_type->getElementType().getScriptObjectTypeId();

	// Create the array and store its id.
	if (manager.getGlobalId())
	{
		addOpcode(new OAllocateGlobalMemImmediate(
						  new VarArgument(EXP1),
						  new LiteralArgument(size * 10000L),
						  new LiteralArgument(element_script_object_type_id)));
	}
	else
	{
		addOpcode(new OAllocateMemImmediate(new VarArgument(EXP1),
											new LiteralArgument(size * 10000L),
											new LiteralArgument(element_script_object_type_id)));
	}

	addOpcode(new OPushRegister(new VarArgument(EXP1)));

	// Initialize array.
	std::vector<int32_t> constelements;
	bool varelem = false;
	bool constelem = false;
	for (auto it = host.elements.begin(); it != host.elements.end(); ++it)
	{
		if (auto val = (*it)->getCompileTimeValue(this, scope))
		{
			constelements.push_back(*val);
			constelem = true;
		}
		else
		{
			constelements.push_back(0);
			varelem = true;
		}
	}
	if(constelem)
	{
		addOpcode(new OWritePODArray(new VarArgument(EXP1), new VectorArgument(constelements)));
	}
	if(varelem)
	{
		addOpcode(new OSetRegister(new VarArgument(INDEX), new VarArgument(EXP1)));
		int32_t i = 0;
		for (auto it = host.elements.begin(); it != host.elements.end(); ++it, i += 10000L)
		{
			if (!(*it)->getCompileTimeValue(this, scope))
			{
				addOpcode(new OPushRegister(new VarArgument(INDEX)));
				visit(*it, &context);
				addOpcode(new OPopRegister(new VarArgument(INDEX)));
				addOpcode(new OWritePODArrayIR(new LiteralArgument(i), new VarArgument(EXP1)));
			}
		}
	}

	addOpcode(new OPopRegister(new VarArgument(EXP1)));
}

void BuildOpcodes::arrayLiteralFree(
		ASTArrayLiteral& host, OpcodeContext& context)
{
	int32_t size = -1;

	// If there's an explicit size, grab it.
	if (host.size)
	{
		if (auto s = host.size->getCompileTimeValue(this, scope))
			size = *s / 10000L;
		else
		{
			handleError(CompileError::ExprNotConstant(host.size.get()));
			return;
		}
	}

	// Otherwise, grab the number of elements.
	if (size == -1) size = host.elements.size();

	// Make sure the chosen size has enough space.
	if (size < int32_t(host.elements.size()))
	{
		handleError(CompileError::ArrayListTooLarge(&host));
		return;
	}

	auto type = host.getReadType(scope, this);

	if (!dynamic_cast<const DataTypeArray*>(type))
	{
		handleError(CompileError::Error(&host, fmt::format("Expected an array type")));
		return;
	}

	auto array_type = dynamic_cast<const DataTypeArray*>(type);
	int element_script_object_type_id = (int)array_type->getElementType().getScriptObjectTypeId();

	addOpcode(
			new OAllocateMemImmediate(new VarArgument(EXP1),
									  new LiteralArgument(size * 10000L),
									  new LiteralArgument(element_script_object_type_id)));
	addOpcode(new OPushRegister(new VarArgument(EXP1)));

	// Initialize.
	std::vector<int32_t> constelements;
	bool constelem = false;
	bool varelem = false;
	for (auto it = host.elements.begin(); it != host.elements.end(); ++it)
	{
		if (auto val = (*it)->getCompileTimeValue(this, scope))
		{
			constelements.push_back(*val);
			constelem = true;
		}
		else
		{
			constelements.push_back(0);
			varelem = true;
		}
	}
	if(constelem)
	{
		addOpcode(new OWritePODArray(new VarArgument(EXP1), new VectorArgument(constelements)));
	}
	if(varelem)
	{
		addOpcode(new OSetRegister(new VarArgument(INDEX), new VarArgument(EXP1)));
		int32_t i = 0;
		for (auto it = host.elements.begin(); it != host.elements.end(); ++it, i += 10000L)
		{
			if (!(*it)->getCompileTimeValue(this, scope))
			{
				addOpcode(new OPushRegister(new VarArgument(INDEX)));
				// opcodeTargets.push_back(&context.initCode);
				visit(*it, &context);
				// opcodeTargets.pop_back();
				addOpcode(new OPopRegister(new VarArgument(INDEX)));
				addOpcode(new OWritePODArrayIR(new LiteralArgument(i), new VarArgument(EXP1)));
			}
		}
	}

	addOpcode(new OPopRegister(new VarArgument(EXP1)));
}

void BuildOpcodes::caseOptionValue(ASTOptionValue& host, void*)
{
	addOpcode(new OSetImmediate(new VarArgument(EXP1),
		new LiteralArgument(*host.getCompileTimeValue(this, scope))));
}

void BuildOpcodes::caseIsIncluded(ASTIsIncluded& host, void*)
{
	addOpcode(new OSetImmediate(new VarArgument(EXP1),
		new LiteralArgument(*host.getCompileTimeValue(this, scope))));
}

//Helper functions
void BuildOpcodes::parseExprs(ASTExpr* left, ASTExpr* right, void* param, bool orderMatters)
{
	auto lval = left->getCompileTimeValue(this, scope);
	auto rval = right->getCompileTimeValue(this, scope);
	if(lval)
	{
		visit(right, param);
		addOpcode(new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(*lval)));
	}
	else if (rval)
	{
		visit(left, param);
		if(orderMatters)
		{
			addOpcode(new OSetRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
			addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*rval)));
		}
		else addOpcode(new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(*rval)));
	}
	else
	{
		//compute both sides
		visit(left, param);
		addOpcode(new OPushRegister(new VarArgument(EXP1)));
		visit(right, param);
		addOpcode(new OPopRegister(new VarArgument(EXP2)));
	}
}

void BuildOpcodes::compareExprs(ASTExpr* left, ASTExpr* right, void* param)
{
	auto lval = left->getCompileTimeValue(this, scope);
	auto rval = right->getCompileTimeValue(this, scope);
	if(lval)
	{
		visit(right, param);
		addOpcode(new OCompareImmediate2(new LiteralArgument(*lval), new VarArgument(EXP1)));
	}
	else if (rval)
	{
		visit(left, param);
		addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(*rval)));
	}
	else
	{
		//compute both sides
		visit(left, param);
		addOpcode(new OPushRegister(new VarArgument(EXP1)));
		visit(right, param);
		addOpcode(new OPopRegister(new VarArgument(EXP2)));
		addOpcode(new OCompareRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
	}
}

void BuildOpcodes::buildPreOp(ASTExpr* operand, void* param, vector<shared_ptr<Opcode>> const& ops)
{
	OpcodeContext* c = (OpcodeContext*)param;

	// Load value of the variable into EXP1.
	VISIT_USEVAL(operand, param);

	// Run the expr code
	addOpcodes(ops);

	// Store it
	LValBOHelper helper(program, this);
	helper.parsing_user_class = parsing_user_class;
	operand->execute(helper, param);
	addOpcodes(helper.getResult());
}
void BuildOpcodes::buildPostOp(ASTExpr* operand, void* param, vector<shared_ptr<Opcode>> const& ops)
{
	if(sidefx_only)
		return buildPreOp(operand, param, ops);
	
	OpcodeContext* c = (OpcodeContext*)param;

	// Load value of the variable into EXP1 and push.
	visit(operand, param);
	addOpcode(new OPushRegister(new VarArgument(EXP1)));

	// Run the expr code
	addOpcodes(ops);
	
	// Store it
	LValBOHelper helper(program, this);
	helper.parsing_user_class = parsing_user_class;
	operand->execute(helper, param);
	addOpcodes(helper.getResult());
	
	// Pop EXP1
	addOpcode(new OPopRegister(new VarArgument(EXP1)));
}

void BuildOpcodes::push_param(bool varg)
{
	VarArgument* reg = nullptr;
	LiteralArgument* lit = nullptr;
	//Try to optimize, instead of just blindly pushing EXP1
	auto* optarg = opcodeTargets.back();
	Opcode* lastop = optarg->back().get();
	if(OSetRegister* tmp = dynamic_cast<OSetRegister*>(lastop))
	{
		VarArgument* destreg = dynamic_cast<VarArgument*>(tmp->getFirstArgument());
		if(destreg && destreg->ID == EXP1)
		{
			reg = tmp->getSecondArgument()->clone();
			optarg->pop_back();
		}
	}
	else if(OSetImmediate* tmp = dynamic_cast<OSetImmediate*>(lastop))
	{
		VarArgument* destreg = dynamic_cast<VarArgument*>(tmp->getFirstArgument());
		if(destreg && destreg->ID == EXP1)
		{
			lit = tmp->getSecondArgument()->clone();
			optarg->pop_back();
		}
	}
	else if(OPopRegister* tmp = dynamic_cast<OPopRegister*>(lastop))
	{
		VarArgument* destreg = dynamic_cast<VarArgument*>(tmp->getArgument());
		if(destreg && destreg->ID == EXP1)
		{
			optarg->pop_back();
			return; //skip end
		}
	}
	if(!(lit || reg))
		reg = new VarArgument(EXP1);
	Opcode* op;
	if(lit)
	{
		if(varg)
			op = new OPushVargV(lit);
		else op = new OPushImmediate(lit);
	}
	else
	{
		if(varg)
			op = new OPushVargR(reg);
		else op = new OPushRegister(reg);
	}
	addOpcode(op);
}

optional<int> BuildOpcodes::eatSetCompare()
{
	if(OSetCompare* setcmp = dynamic_cast<OSetCompare*>(backOpcode()))
	{
		auto cmp = setcmp->getSecondArgument()->value;
		backTarget().pop_back(); //erase the OSetCompare
		return cmp;
	}
	return nullopt;
}

void BuildOpcodes::pop_params(uint count)
{
	if(count)
		addOpcode(new OPopArgsRegister(new VarArgument(NUL), new LiteralArgument(count)));
}

/////////////////////////////////////////////////////////////////////////////////
// LValBOHelper

LValBOHelper::LValBOHelper(Program& program, Scope* scope) : program(program)
{
	ASTVisitor::scope = scope;
}
LValBOHelper::LValBOHelper(Program& program, BuildOpcodes* bo) : program(program)
{
	ASTVisitor::scope = bo->scope;
	parsing_user_class = bo->parsing_user_class;
	in_func = bo->in_func;
}

void LValBOHelper::caseDefault(void *)
{
	//Shouldn't happen
	assert(false);
}

void LValBOHelper::addOpcode(Opcode* code)
{
	addOpcode2(result, code);
}

void LValBOHelper::addOpcode(std::shared_ptr<Opcode> &code)
{
	result.push_back(code);
}

template <class Container>
void LValBOHelper::addOpcodes(Container const& container)
{
	for (auto ptr: container)
		addOpcode(ptr);
}

void LValBOHelper::caseExprIdentifier(ASTExprIdentifier& host, void* param)
{
	OpcodeContext* c = (OpcodeContext*)param;
	int32_t vid = host.binding->id;
	
	if(UserClassVar* ucv = dynamic_cast<UserClassVar*>(host.binding))
	{
		UserClass& user_class = *ucv->getClass();
		addOpcode(new OWriteObject(new VarArgument(CLASS_THISKEY), new LiteralArgument(ucv->getIndex())));
		return;
	}

	if (auto ivar = dynamic_cast<InternalVariable*>(host.binding))
	{
		assert(ivar->writefn);
		std::vector<std::shared_ptr<Opcode>> const& funcCode = ivar->writefn->getCode();
		for (auto it = funcCode.begin(); it != funcCode.end(); ++it)
			addOpcode((*it)->makeClone(false));
		return;
	}

	bool is_object = false;
	if (auto type = host.getWriteType(scope, nullptr))
		is_object = type->isObject();

	if (auto globalId = host.binding->getGlobalId())
	{
		// Global variable.
		if (is_object)
			addOpcode(new OSetObject(new GlobalArgument(*globalId), new VarArgument(EXP1)));
		else
			addOpcode(new OSetRegister(new GlobalArgument(*globalId), new VarArgument(EXP1)));
		return;
	}

	// Set the stack.
	int32_t offset = host.binding->getStackOffset(false);

	if (is_object)
		addOpcode(new OStoreObject(new VarArgument(EXP1),new LiteralArgument(offset)));
	else
		addOpcode(new OStore(new VarArgument(EXP1),new LiteralArgument(offset)));
}

void LValBOHelper::caseExprArrow(ASTExprArrow &host, void *param)
{
	if(UserClassVar* ucv = host.u_datum; ucv && !ucv->is_internal)
	{
		BuildOpcodes oc(program, this);
		oc.parsing_user_class = parsing_user_class;
		addOpcode(new OPushRegister(new VarArgument(EXP1)));
		oc.visit(host.left.get(), param);
		addOpcodes(oc.getResult());
		addOpcode(new OSetRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
		addOpcode(new OPopRegister(new VarArgument(EXP1)));
		addOpcode(new OWriteObject(new VarArgument(EXP2), new LiteralArgument(ucv->getIndex())));
		return;
	}

	int32_t isIndexed = (host.index != NULL);
	assert(host.writeFunction->isInternal());
	
	if(host.writeFunction->isNil())
	{
		bool skipptr = host.writeFunction->getIntFlag(IFUNCFLAG_SKIPPOINTER);
		bool needs_pushpop = isIndexed || !skipptr;
		if(needs_pushpop)
			addOpcode(new OPushRegister(new VarArgument(EXP1)));
		if (!skipptr)
		{
			//Get lval
			BuildOpcodes oc(program, this);
			oc.parsing_user_class = parsing_user_class;
			oc.visit(host.left.get(), param);
			addOpcodes(oc.getResult());
		}
		
		if(isIndexed)
		{
			BuildOpcodes oc2(program, this);
			oc2.parsing_user_class = parsing_user_class;
			oc2.visit(host.index.get(), param);
			addOpcodes(oc2.getResult());
		}
		if(needs_pushpop)
			addOpcode(new OPopRegister(new VarArgument(EXP1)));
	}
	else if(host.writeFunction->getFlag(FUNCFLAG_INLINE))
	{
		if (!(host.writeFunction->getIntFlag(IFUNCFLAG_SKIPPOINTER)))
		{
			//Push rval
			addOpcode(new OPushRegister(new VarArgument(EXP1)));
			//Get lval
			BuildOpcodes oc(program, this);
			oc.parsing_user_class = parsing_user_class;
			oc.visit(host.left.get(), param);
			addOpcodes(oc.getResult());
			//Pop rval
			addOpcode(new OPopRegister(new VarArgument(EXP2)));
			//Push lval
			addOpcode(new OPushRegister(new VarArgument(EXP1)));
			//Push rval
			addOpcode(new OPushRegister(new VarArgument(EXP2)));
		}
		else
		{
			//Push rval
			addOpcode(new OPushRegister(new VarArgument(EXP1)));
		}
		
		if(isIndexed)
		{
			BuildOpcodes oc2(program, this);
			oc2.parsing_user_class = parsing_user_class;
			oc2.visit(host.index.get(), param);
			addOpcodes(oc2.getResult());
			addOpcode(new OPushRegister(new VarArgument(EXP1)));
		}
		
		std::vector<std::shared_ptr<Opcode>> const& funcCode = host.writeFunction->getCode();
		for(auto it = funcCode.begin();
			it != funcCode.end(); ++it)
		{
			addOpcode((*it)->makeClone(false));
		}
	}
	else
	{
		// This is actually implemented as a settor function call.
		bool never_ret = host.writeFunction->getFlag(FUNCFLAG_NEVER_RETURN);
		// Push the stack frame.
		if(!never_ret)
			addOpcode(new OPushRegister(new VarArgument(SFRAME)));
		
		if (!(host.writeFunction->getIntFlag(IFUNCFLAG_SKIPPOINTER)))
		{
			//Push rval
			addOpcode(new OPushRegister(new VarArgument(EXP1)));
			//Get lval
			BuildOpcodes oc(program, this);
			oc.parsing_user_class = parsing_user_class;
			oc.visit(host.left.get(), param);
			addOpcodes(oc.getResult());
			//Pop rval
			addOpcode(new OPopRegister(new VarArgument(EXP2)));
			//Push lval
			addOpcode(new OPushRegister(new VarArgument(EXP1)));
			//Push rval
			addOpcode(new OPushRegister(new VarArgument(EXP2)));
		}
		else
		{
			//Push rval
			addOpcode(new OPushRegister(new VarArgument(EXP1)));
		}
		
		//and push the index, if indexed
		if(isIndexed)
		{
			BuildOpcodes oc2(program, this);
			oc2.parsing_user_class = parsing_user_class;
			oc2.visit(host.index.get(), param);
			addOpcodes(oc2.getResult());
			addOpcode(new OPushRegister(new VarArgument(EXP1)));
		}
		
		//finally, goto!
		int32_t label = host.writeFunction->getLabel();
		addOpcode(new OCallFunc(new LabelArgument(label, true)));

		// Pop the stack frame
		if(!never_ret)
			addOpcode(new OPopRegister(new VarArgument(SFRAME)));
	}
}

void LValBOHelper::caseExprIndex(ASTExprIndex& host, void* param)
{
	// Arrows just fall back on the arrow implementation.
	if (host.array->isTypeArrow())
	{
		ASTExprArrow* arrow = static_cast<ASTExprArrow*>(host.array.get());
		if(!arrow->isTypeArrowUsrClass())
		{
			caseExprArrow(static_cast<ASTExprArrow&>(*host.array), param);
			return;
		}
	}

	vector<shared_ptr<Opcode>> opcodes;
	BuildOpcodes bo(program, this);
	bo.parsing_user_class = parsing_user_class;
	auto arrVal = host.array->getCompileTimeValue(&bo, scope);
	auto indxVal = host.index->getCompileTimeValue(&bo, scope);
	if(!arrVal || !indxVal)
	{
		// Push the value.
		addOpcode(new OPushRegister(new VarArgument(EXP1)));
	}
	
	if(!arrVal)
	{
		// Get and push the array pointer.
		BuildOpcodes buildOpcodes1(program, this);
		buildOpcodes1.parsing_user_class = parsing_user_class;
		buildOpcodes1.visit(host.array.get(), param);
		opcodes = buildOpcodes1.getResult();
		for (auto it = opcodes.begin(); it != opcodes.end(); ++it)
			addOpcode(*it);
		if(!indxVal)
		{
			addOpcode(new OPushRegister(new VarArgument(EXP1)));
		}
		else addOpcode(new OSetRegister(new VarArgument(INDEX), new VarArgument(EXP1)));
	}
	if(!indxVal)
	{
		// Get the index.
		BuildOpcodes buildOpcodes2(program, this);
		buildOpcodes2.parsing_user_class = parsing_user_class;
		buildOpcodes2.visit(host.index.get(), param);
		opcodes = buildOpcodes2.getResult();
		for (auto it = opcodes.begin(); it != opcodes.end(); ++it)
			addOpcode(*it);
		addOpcode(new OSetRegister(new VarArgument(EXP2), new VarArgument(EXP1))); //can't be helped, unforunately -V
	}
	// Setup array indices.
	if(arrVal)
		addOpcode(new OSetImmediate(new VarArgument(INDEX), new LiteralArgument(*arrVal)));
	else if(!indxVal) addOpcode(new OPopRegister(new VarArgument(INDEX)));
	
	if(!arrVal || !indxVal)
	{
		addOpcode(new OPopRegister(new VarArgument(EXP1))); // Pop the value
	}
	if(indxVal) addOpcode(new OWritePODArrayIR(new LiteralArgument(*indxVal), new VarArgument(EXP1)));
	else addOpcode(new OWritePODArrayRR(new VarArgument(EXP2), new VarArgument(EXP1)));
}


void CleanupVisitor::visit(AST& node, void* param)
{
	if(node.isDisabled()) return; //Don't visit disabled nodes.
	if(!node.reachable()) return; //Don't visit unreachable nodes for ZASM generation
	AST* nodeptr = &node;
	ASTExpr* exprptr = dynamic_cast<ASTExpr*>(nodeptr);
	ASTExprAnd* andnode = dynamic_cast<ASTExprAnd*>(nodeptr);
	ASTExprOr* ornode = dynamic_cast<ASTExprOr*>(nodeptr);
	bool is_boolexpr = andnode || ornode;
	ASTExprBoolTree* cache_tree = booltree;
	BoolTreeNode* cache_node = active_node;
	if(!is_boolexpr)
	{
		if(active_node)
		{
			exprptr = exprptr->clone();
			node.disable();
			active_node = &(active_node->branch.emplace_back());
			active_node->mode = BoolTreeNode::MODE_LEAF;
			active_node->leaf = exprptr;
		}
		booltree = nullptr;
		active_node = nullptr;
	}
	else
	{
		if(!booltree)
		{
			booltree = new ASTExprBoolTree(node.location);
			active_node = &booltree->root;
			if(andnode)
			{
				andnode->tree = booltree;
				active_node->mode = BoolTreeNode::MODE_AND;
			}
			else //if(ornode)
			{
				ornode->tree = booltree;
				active_node->mode = BoolTreeNode::MODE_OR;
			}
		}
		else if(andnode && active_node->mode == BoolTreeNode::MODE_OR)
		{
			active_node = &(active_node->branch.emplace_back());
			active_node->mode = BoolTreeNode::MODE_AND;
		}
		else if(ornode && active_node->mode == BoolTreeNode::MODE_AND)
		{
			active_node = &(active_node->branch.emplace_back());
			active_node->mode = BoolTreeNode::MODE_OR;
		}
	}
	RecursiveVisitor::visit(*nodeptr, param);
	booltree = cache_tree;
	active_node = cache_node;
}

