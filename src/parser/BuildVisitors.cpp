#include <assert.h>

#include "BuildVisitors.h"
#include "CompileError.h"
#include "Types.h"
#include "ZScript.h"

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

#define INITC_VISIT(node) \
do \
	visit(node, (void*)&initContext); \
while(false)
#define INITC_INIT() \
do \
	result.insert(result.begin() + initIndex, initContext.initCode.begin(), initContext.initCode.end()); \
while(false)
#define INITC_DEALLOC() \
do \
	result.insert(result.end(), initContext.deallocCode.begin(), initContext.deallocCode.end()); \
while(false)

#define INITC_CTXT ((void*)&initContext)

/////////////////////////////////////////////////////////////////////////////////
// BuildOpcodes

BuildOpcodes::BuildOpcodes()
	: returnlabelid(-1), returnRefCount(0), continuelabelids(), 
	  continueRefCounts(), breaklabelids(), breakRefCounts(),
	  break_past_counts(), break_to_counts(), break_depth(0),
	  continue_past_counts(), continue_to_counts(), continue_depth(0)
{
	opcodeTargets.push_back(&result);
}
BuildOpcodes::BuildOpcodes(Scope* curScope)
	: BuildOpcodes()
{
	scope = curScope;
}

BuildOpcodes::BuildOpcodes(LValBOHelper* helper)
	: BuildOpcodes()
{
	scope = helper->scope;
	parsing_user_class = helper->parsing_user_class;
	in_func_body = helper->in_func_body;
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
	RecursiveVisitor::visit(node, param);
	for (auto it = node.compileErrorCatches.cbegin(); it != node.compileErrorCatches.cend(); ++it)
	{
		ASTExprConst& idNode = **it;
		auto errorId = idNode.getCompileTimeValue(this, scope);
		assert(errorId);
		handleError(CompileError::MissingCompileError(&node, int32_t(*errorId / 10000L)));
	}
}

void BuildOpcodes::literalVisit(AST& node, void* param)
{
	if(node.isDisabled()) return; //Don't visit disabled nodes.
	int32_t initIndex = result.size();
	OpcodeContext *parentContext = (OpcodeContext*)param;
	OpcodeContext prm(parentContext->typeStore);
	OpcodeContext *c = &prm;
	visit(node, (void*)c);
	//Handle literals
	result.insert(result.begin() + initIndex, c->initCode.begin(), c->initCode.end());
	result.insert(result.end(), c->deallocCode.begin(), c->deallocCode.end());
}

void BuildOpcodes::literalVisit(AST* node, void* param)
{
	if(node) literalVisit(*node, param);
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

void BuildOpcodes::deallocateArrayRef(int32_t arrayRef)
{
	addOpcode(new OLoadDirect(new VarArgument(EXP2), new LiteralArgument(arrayRef)));
	addOpcode(new ODeallocateMemRegister(new VarArgument(EXP2)));
}
void BuildOpcodes::deallocateArrayRef(int32_t arrayRef, std::vector<std::shared_ptr<Opcode>>& code)
{
	addOpcode2(code, new OLoadDirect(new VarArgument(EXP2), new LiteralArgument(arrayRef)));
	addOpcode2(code, new ODeallocateMemRegister(new VarArgument(EXP2)));
}

void BuildOpcodes::deallocateRefsUntilCount(int32_t count)
{
	count = arrayRefs.size() - count;
	for (auto it = arrayRefs.rbegin(); it != arrayRefs.rend() && count > 0; ++it, --count)
	{
		deallocateArrayRef(*it);
	}
}
void BuildOpcodes::deallocateRefsUntilCount(int32_t count, std::vector<std::shared_ptr<Opcode>>& code)
{
	count = arrayRefs.size() - count;
	for (auto it = arrayRefs.rbegin(); it != arrayRefs.rend() && count > 0; ++it, --count)
	{
		deallocateArrayRef(*it, code);
	}
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
	scope = host.getScope();
	
	OpcodeContext *c = (OpcodeContext *)param;

	int32_t startRefCount = arrayRefs.size();

	for (auto it = host.statements.begin();
		 it != host.statements.end(); ++it)
	{
		literalVisit(*it, param);
	}

	deallocateRefsUntilCount(startRefCount);
	while ((int32_t)arrayRefs.size() > startRefCount)
		arrayRefs.pop_back();
	
	scope = scope->getParent();
}

void BuildOpcodes::caseStmtIf(ASTStmtIf &host, void *param)
{
	int32_t ifid = ScriptParser::getUniqueLabelID();
	bool inv = host.isInverted();
	string ifstr = inv ? "unless" : "if",
		truestr = inv ? "false" : "true",
		falsestr = inv ? "true" : "false";
	if(host.isDecl())
	{
		string const& declname = host.declaration->name;
		if(!host.getScope())
		{
			host.setScope(scope->makeChild());
		}
		scope = host.getScope();
		int32_t startRefCount = arrayRefs.size();
		
		if(auto val = host.declaration->getInitializer()->getCompileTimeValue(this, scope))
		{
			if((host.isInverted()) == (*val==0)) //True, so go straight to the 'then'
			{
				auto targ_sz = commentTarget();
				literalVisit(host.declaration.get(), param);
				commentAt(targ_sz, fmt::format("{}({}={}) #{} [Opt:AlwaysOn]",ifstr,declname,truestr,ifid));
				visit(host.thenStatement.get(), param);
				deallocateRefsUntilCount(startRefCount);
				
				while ((int32_t)arrayRefs.size() > startRefCount)
					arrayRefs.pop_back();
				
				scope = scope->getParent();
			} //Either true or false, it's constant, so no checks required.
			return;
		}
		
		int32_t endif = ifid;
		auto targ_sz = commentTarget();
		literalVisit(host.declaration.get(), param);
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
		//nop
		Opcode *next = new ONoOp();
		next->setLabel(endif);
		addOpcode(next);
		
		deallocateRefsUntilCount(startRefCount);
		
		while ((int32_t)arrayRefs.size() > startRefCount)
			arrayRefs.pop_back();
		
		scope = scope->getParent();
	}
	else
	{
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
		int32_t startRefCount = arrayRefs.size(); //Store ref count
		auto targ_sz = commentTarget();
		literalVisit(host.condition.get(), param);
		commentAt(targ_sz, fmt::format("{}() #{} Test",ifstr,ifid));
		//Deallocate string/array literals from within the condition
		deallocateRefsUntilCount(startRefCount);
		while ((int32_t)arrayRefs.size() > startRefCount)
			arrayRefs.pop_back();
		//Continue
		int32_t endif = ifid;
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
		Opcode *next = new ONoOp();
		next->setLabel(endif);
		addOpcode(next);
	}
}

void BuildOpcodes::caseStmtIfElse(ASTStmtIfElse &host, void *param)
{
	int32_t ifid = ScriptParser::getUniqueLabelID();
	bool inv = host.isInverted();
	string ifstr = inv ? "unless" : "if",
		truestr = inv ? "false" : "true",
		falsestr = inv ? "true" : "false";
	if(host.isDecl())
	{
		string const& declname = host.declaration->name;
		if(!host.getScope())
		{
			host.setScope(scope->makeChild());
		}
		scope = host.getScope();
		int32_t startRefCount = arrayRefs.size();
		
		if(auto val = host.declaration->getInitializer()->getCompileTimeValue(this, scope))
		{
			if((host.isInverted()) == (*val==0)) //True, so go straight to the 'then'
			{
				auto targ_sz = commentTarget();
				literalVisit(host.declaration.get(), param);
				commentAt(targ_sz, fmt::format("{}({}={}) #{} [Opt:AlwaysOn]",ifstr,declname,truestr,ifid));
				visit(host.thenStatement.get(), param);
				//Deallocate after then block
				deallocateRefsUntilCount(startRefCount);
				
				while ((int32_t)arrayRefs.size() > startRefCount)
					arrayRefs.pop_back();
				
				scope = scope->getParent();
			}
			else //False, so go straight to the 'else'
			{
				//Deallocate before else block
				deallocateRefsUntilCount(startRefCount);
				
				while ((int32_t)arrayRefs.size() > startRefCount)
					arrayRefs.pop_back();
				
				scope = scope->getParent();
				//
				auto targ_sz = commentTarget();
				visit(host.elseStatement.get(), param);
				commentStartEnd(targ_sz, fmt::format("{}({}={}) #{} Else [Opt:AlwaysOff]",ifstr,declname,falsestr,ifid));
			}
			//Either way, ignore the rest and return.
			return;
		}
		
		int32_t elseif = ifid;
		int32_t endif = ScriptParser::getUniqueLabelID();
		auto targ_sz = commentTarget();
		literalVisit(host.declaration.get(), param);
		commentAt(targ_sz, fmt::format("{}({}) #{} Decl",ifstr,declname,ifid));
		
		//The condition should be reading the value just processed from the initializer
		targ_sz = commentTarget();
		visit(host.condition.get(), param);
		commentAt(targ_sz, fmt::format("{}({}) #{} Test",ifstr,declname,ifid));
		//
		addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(1)));
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
		//nop
		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
		Opcode *next = new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0));
		next->setLabel(elseif);
		addOpcode(next);
		
		deallocateRefsUntilCount(startRefCount);
		
		while ((int32_t)arrayRefs.size() > startRefCount)
			arrayRefs.pop_back();
		
		scope = scope->getParent();
		
		addOpcode(new OGotoTrueImmediate(new LabelArgument(endif)));
		commentStartEnd(targ_sz, fmt::format("{}({}) #{} Body",ifstr,declname,ifid));
		targ_sz = commentTarget();
		visit(host.elseStatement.get(), param);
		commentStartEnd(targ_sz, fmt::format("{}({}) #{} Else",ifstr,declname,ifid));
		
		next = new ONoOp();
		next->setLabel(endif);
		addOpcode(next);
	}
	else
	{
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
		int32_t startRefCount = arrayRefs.size(); //Store ref count
		auto targ_sz = commentTarget();
		literalVisit(host.condition.get(), param);
		commentAt(targ_sz, fmt::format("{}() #{} Test",ifstr,ifid));
		//Deallocate string/array literals from within the condition
		deallocateRefsUntilCount(startRefCount);
		while ((int32_t)arrayRefs.size() > startRefCount)
			arrayRefs.pop_back();
		//Continue
		int32_t elseif = ifid;
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
		Opcode *next = new ONoOp();
		next->setLabel(elseif);
		addOpcode(next);
		targ_sz = commentTarget();
		visit(host.elseStatement.get(), param);
		commentStartEnd(targ_sz, fmt::format("{}() #{} Else",ifstr,ifid));
		next = new ONoOp();
		next->setLabel(endif);
		addOpcode(next);
	}
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
	
	int32_t switchid = ScriptParser::getUniqueLabelID();
	int32_t end_label = switchid;
	auto default_label = end_label;
	
	// save and override break label.
	push_break(end_label, arrayRefs.size());
	
	// Evaluate the key.
	auto keyval = host.key->getCompileTimeValue(this, scope);
	if(!keyval)
	{
		int32_t startRefCount = arrayRefs.size(); //Store ref count
		auto targ_sz = commentTarget();
		literalVisit(host.key.get(), param);
		commentAt(targ_sz, fmt::format("switch() #{} Key", switchid));
		//Deallocate string/array literals from within the key
		deallocateRefsUntilCount(startRefCount);
		while ((int32_t)arrayRefs.size() > startRefCount)
			arrayRefs.pop_back();
	}
	
	if(cases.size() == 1 && cases.back()->isDefault) //Only default case
	{
		auto targ_sz = commentTarget();
		visit(cases.back()->block.get(), param);
		commentAt(targ_sz, fmt::format("switch() #{} Default [Opt:DefaultOnly]", switchid));
		// Add ending label, for 'break;'
		Opcode* next = new ONoOp();
		next->setLabel(end_label);
		addOpcode(next);
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
				// Test this individual case.
				if(val)
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
				if(startval)  //Compare key to lower bound
				{
					addOpcode(new OCompareImmediate(new VarArgument(SWITCHKEY), new LiteralArgument(*startval)));
				}
				else //Shouldn't ever happen?
				{
					visit(*range.start, param);
					addOpcode(new OCompareRegister(new VarArgument(SWITCHKEY), new VarArgument(EXP1)));
				}
				addOpcode(new OSetMore(new VarArgument(EXP1))); //Set if key is IN the bound
				addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0))); //Compare if key is OUT of the bound
				addOpcode(new OGotoTrueImmediate(new LabelArgument(skipLabel))); //Skip if key is OUT of the bound
				commentBack(fmt::format("case '{0}...{1}', key<{0}", OPT_STR(startval), OPT_STR(endval)));
				
				if(endval)  //Compare key to upper bound
				{
					addOpcode(new OCompareImmediate(new VarArgument(SWITCHKEY), new LiteralArgument(*endval)));
				}
				else //Shouldn't ever happen?
				{
					visit(*range.end, param);
					addOpcode(new OCompareRegister(new VarArgument(SWITCHKEY), new VarArgument(EXP1)));
				}
				addOpcode(new OSetLess(new VarArgument(EXP1)	)); //Set if key is IN the bound
				addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0))); //Compare if key is OUT of the bound
				addOpcode(new OGotoFalseImmediate(new LabelArgument(label))); //If key is in bounds, jump to its label
				commentBack(fmt::format("case '{0}...{1}', {0}<key<{1}", OPT_STR(startval), OPT_STR(endval)));
				Opcode *end = new ONoOp(); //Just here so the skip label can be placed
				end->setLabel(skipLabel);
				addOpcode(end); //add the skip label
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
			Opcode* next = new ONoOp();
			next->setLabel(labels[cases]);
			addOpcode(next);
			commentBack("Case block");
			// Add block.
			visit(cases->block.get(), param);
		}
	}
	
	if(needsEndLabel)
	{
		// Add ending label.
		Opcode *next = new ONoOp();
		next->setLabel(end_label);
		addOpcode(next);
	}
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
	push_break(end_label, arrayRefs.size());
	
	// Evaluate the key.
	int32_t startRefCount = arrayRefs.size(); //Store ref count
	auto targ_sz = commentTarget();
	literalVisit(host.key.get(), param);
	commentAt(targ_sz, fmt::format("switch(\"\") #{} Key", switchid));
	//Deallocate string/array literals from within the key
	deallocateRefsUntilCount(startRefCount);
	while ((int32_t)arrayRefs.size() > startRefCount)
		arrayRefs.pop_back();
	
	if(cases.size() == 1 && cases.back()->isDefault) //Only default case
	{
		targ_sz = commentTarget();
		visit(cases.back()->block.get(), param);
		commentAt(targ_sz, fmt::format("switch(\"\") #{} Default [Opt:DefaultOnly]", switchid));
		// Add ending label, for 'break;'
		Opcode *next = new ONoOp();
		next->setLabel(end_label);
		addOpcode(next);
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
			int32_t litRefCount = arrayRefs.size(); //Store ref count
			INITC_STORE(); //store init-related values
			INITC_VISIT(*it); //visit with the initc params
			INITC_INIT(); //initialize the literal
			
			// Compare the strings
			if(*lookupOption(*scope, CompileOption::OPT_STRING_SWITCH_CASE_INSENSITIVE))
				addOpcode(new OInternalInsensitiveStringCompare(new VarArgument(SWITCHKEY), new VarArgument(EXP1)));
			else
				addOpcode(new OInternalStringCompare(new VarArgument(SWITCHKEY), new VarArgument(EXP1)));
			commentBack(fmt::format("case \"{}\"", (*it)->value));
			
			INITC_DEALLOC(); //deallocate the literal
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
		Opcode* next = new ONoOp();
		next->setLabel(labels[cases]);
		addOpcode(next);
		commentBack("Case block");
		// Add block.
		visit(cases->block.get(), param);
	}
	
	// Add ending label.
	Opcode *next = new ONoOp();
	next->setLabel(end_label);
	addOpcode(next);
	
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
	auto targ_sz = commentTarget();
	//run the precondition
	int32_t setupRefCount = arrayRefs.size(); //Store ref count
	literalVisit(host.setup.get(), param);
	//Deallocate string/array literals from within the setup
	deallocateRefsUntilCount(setupRefCount);
	int32_t forid = ScriptParser::getUniqueLabelID();
	commentAt(targ_sz, fmt::format("for() #{} setup",forid));
	while ((int32_t)arrayRefs.size() > setupRefCount)
		arrayRefs.pop_back();
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
			return;
		}
	}
	//Continue
	int32_t loopstart = forid;
	int32_t loopend = ScriptParser::getUniqueLabelID();
	int32_t loopincr = ScriptParser::getUniqueLabelID();
	int32_t elselabel = host.hasElse() ? ScriptParser::getUniqueLabelID() : loopend;
	
	Opcode *next = new ONoOp();
	next->setLabel(loopstart);
	addOpcode(next);
	commentBack(fmt::format("for() #{} LoopTest",forid));
	//test the termination condition
	int32_t testRefCount = arrayRefs.size(); //Store ref count
	literalVisit(host.test.get(), param);
	//Deallocate string/array literals from within the test
	deallocateRefsUntilCount(testRefCount);
	while ((int32_t)arrayRefs.size() > testRefCount)
		arrayRefs.pop_back();
	//Continue
	addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
	addOpcode(new OGotoTrueImmediate(new LabelArgument(elselabel)));
	commentBack(fmt::format("for() #{} TestFail?",forid));
	//run the loop body
	//save the old break and continue values

	push_break(loopend, arrayRefs.size());
	push_cont(loopincr, arrayRefs.size());
	
	targ_sz = commentTarget();
	visit(host.body.get(), param);
	commentStartEnd(targ_sz, fmt::format("for() #{} Body",forid));
	
	pop_break();
	pop_cont();

	//run the increment
	next = new ONoOp();
	next->setLabel(loopincr);
	addOpcode(next);
	commentBack(fmt::format("for() #{} LoopIncrement",forid));
	int32_t incRefCount = arrayRefs.size(); //Store ref count
	literalVisit(host.increment.get(), param);
	//Deallocate string/array literals from within the increment
	deallocateRefsUntilCount(incRefCount);
	while ((int32_t)arrayRefs.size() > incRefCount)
		arrayRefs.pop_back();
	//Continue
	addOpcode(new OGotoImmediate(new LabelArgument(loopstart)));
	commentBack(fmt::format("for() #{} End",forid));
	
	scope = scope->getParent();
	
	if(host.hasElse())
	{
		next = new ONoOp();
		next->setLabel(elselabel);
		addOpcode(next);
		targ_sz = commentTarget();
		visit(host.elseBlock.get(), param);
		commentStartEnd(targ_sz, fmt::format("for() #{} Else",forid));
	}
	
	//nop
	next = new ONoOp();
	next->setLabel(loopend);
	addOpcode(next);
}

void BuildOpcodes::caseStmtForEach(ASTStmtForEach &host, void *param)
{
	//Force to sub-scope
	if(!host.getScope())
	{
		host.setScope(scope->makeChild());
	}
	scope = host.getScope();
	
	int32_t forid = ScriptParser::getUniqueLabelID();
	//Declare the local variable that will hold the array ptr
	auto targ_sz = commentTarget();
	literalVisit(host.arrdecl.get(), param);
	commentAt(targ_sz, fmt::format("for(each) #{} ArrDecl",forid));
	//Declare the local variable that will hold the iterator
	targ_sz = commentTarget();
	literalVisit(host.indxdecl.get(), param);
	commentAt(targ_sz, fmt::format("for(each) #{} IndxDecl",forid));
	//Declare the local variable that will hold the current loop value
	targ_sz = commentTarget();
	literalVisit(host.decl.get(), param);
	commentAt(targ_sz, fmt::format("for(each) #{} ValDecl",forid));
	
	int32_t decloffset = 10000L * *getStackOffset(*host.decl.get()->manager);
	int32_t arrdecloffset = 10000L * *getStackOffset(*host.arrdecl.get()->manager);
	int32_t indxdecloffset = 10000L * *getStackOffset(*host.indxdecl.get()->manager);
	
	int32_t loopstart = forid;
	int32_t loopend = ScriptParser::getUniqueLabelID();
	int32_t elselabel = host.hasElse() ? ScriptParser::getUniqueLabelID() : loopend;
	
	Opcode* next = new ONoOp();
	next->setLabel(loopstart);
	addOpcode(next);
	commentBack(fmt::format("for(each) #{} EndArrayCheck",forid));
	//Check if we've reached the end of the array
	addOpcode(new OLoadDirect(new VarArgument(INDEX), new LiteralArgument(arrdecloffset)));
	addOpcode(new OArraySize(new VarArgument(INDEX))); //get sizeofarray
	//Load the iterator
	addOpcode(new OLoadDirect(new VarArgument(EXP2), new LiteralArgument(indxdecloffset)));
	//If the iterator is >= the length
	addOpcode(new OCompareRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
	addOpcode(new OSetMore(new VarArgument(EXP1)));
	addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
	//Goto the 'else' (end without break)
	addOpcode(new OGotoFalseImmediate(new LabelArgument(elselabel)));
	commentBack(fmt::format("for(each) #{} to Else",forid));
	
	//Reaching here, we have a valid index! Load it.
	addOpcode(new OReadPODArrayR(new VarArgument(EXP1), new VarArgument(EXP2)));
	commentBack(fmt::format("for(each) #{} Next Index",forid));
	//... and store it in the local variable.
	addOpcode(new OStoreDirect(new VarArgument(EXP1), new LiteralArgument(decloffset)));
	//Now increment the iterator for the next loop
	addOpcode(new OAddImmediate(new VarArgument(EXP2), new LiteralArgument(10000)));
	addOpcode(new OStoreDirect(new VarArgument(EXP2), new LiteralArgument(indxdecloffset)));
	
	//...and run the inside of the loop.
	push_break(loopend, arrayRefs.size());
	push_cont(loopstart, arrayRefs.size());
	
	targ_sz = commentTarget();
	visit(host.body.get(), param);
	commentStartEnd(targ_sz, fmt::format("for(each) #{} Body",forid));
	
	pop_break();
	pop_cont();
	
	//Return to top of loop
	if(host.ends_loop)
		addOpcode(new OGotoImmediate(new LabelArgument(loopstart)));
	commentBack(fmt::format("for(each) #{} End",forid));
	
	scope = scope->getParent();
	
	if(host.hasElse())
	{
		next = new ONoOp();
		next->setLabel(elselabel);
		addOpcode(next);
		targ_sz = commentTarget();
		visit(host.elseBlock.get(), param);
		commentStartEnd(targ_sz, fmt::format("for(each) #{} Else",forid));
	}
	
	next = new ONoOp();
	next->setLabel(loopend);
	addOpcode(next);
}

void BuildOpcodes::caseStmtWhile(ASTStmtWhile &host, void *param)
{
	auto val = host.test->getCompileTimeValue(this, scope);
	bool inv = host.isInverted();
	string whilestr = inv ? "until" : "while",
		truestr = inv ? "false" : "true",
		falsestr = inv ? "true" : "false";
	int32_t whileid = ScriptParser::getUniqueLabelID();
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
	
	int32_t startlabel = whileid;
	int32_t endlabel = ScriptParser::getUniqueLabelID();
	int32_t elselabel = host.hasElse() ? ScriptParser::getUniqueLabelID() : endlabel;
	//run the test
	Opcode *next = new ONoOp();
	next->setLabel(startlabel);
	addOpcode(next);
	if(!val)
	{
		commentBack(fmt::format("{}() #{} Test",whilestr,whileid));
		int32_t startRefCount = arrayRefs.size(); //Store ref count
		literalVisit(host.test.get(), param);
		//Deallocate string/array literals from within the test
		deallocateRefsUntilCount(startRefCount);
		while ((int32_t)arrayRefs.size() > startRefCount)
			arrayRefs.pop_back();
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
	
	push_break(endlabel, arrayRefs.size());
	push_cont(startlabel, arrayRefs.size());
	
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
		next = new ONoOp();
		next->setLabel(elselabel);
		addOpcode(next);
		targ_sz = commentTarget();
		visit(host.elseBlock.get(), param);
		commentStartEnd(targ_sz, fmt::format("{}() #{} Else",whilestr,whileid));
	}
	if(!val || num_breaks) //no else / end label needed for inf loops unless they break
	{
		next = new ONoOp();
		next->setLabel(endlabel);
		addOpcode(next);
	}
}

void BuildOpcodes::caseStmtDo(ASTStmtDo &host, void *param)
{
	auto val = host.test->getCompileTimeValue(this, scope);
	bool inv = host.isInverted();
	bool truthyval = val && inv == (*val==0);
	bool deadloop = val && !truthyval;
	int32_t whileid = ScriptParser::getUniqueLabelID();
	int32_t startlabel = whileid;
	int32_t endlabel = ScriptParser::getUniqueLabelID();
	int32_t elselabel = host.hasElse() ? ScriptParser::getUniqueLabelID() : endlabel;
	int32_t continuelabel = ScriptParser::getUniqueLabelID();
	string whilestr = inv ? "do-until" : "do-while",
		truestr = inv ? "false" : "true",
		falsestr = inv ? "true" : "false";
	Opcode* next;
	if(!deadloop)
	{
		//nop to label start
		next = new ONoOp();
		next->setLabel(startlabel);
		addOpcode(next);
	}
	
	push_break(endlabel, arrayRefs.size());
	push_cont(continuelabel, arrayRefs.size());
	
	auto targ_sz = commentTarget();
	visit(host.body.get(), param);
	commentStartEnd(targ_sz, fmt::format("{}() #{} Body",whilestr,whileid));
	
	pop_break();
	pop_cont();
	
	next = new ONoOp();
	next->setLabel(continuelabel);
	addOpcode(next);
	
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
		int32_t startRefCount = arrayRefs.size(); //Store ref count
		literalVisit(host.test.get(), param);
		//Deallocate string/array literals from within the test
		deallocateRefsUntilCount(startRefCount);
		while ((int32_t)arrayRefs.size() > startRefCount)
			arrayRefs.pop_back();
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
		
		next = new ONoOp();
		next->setLabel(elselabel);
		addOpcode(next);
		targ_sz = commentTarget();
		visit(host.elseBlock.get(), param);
		commentStartEnd(targ_sz, fmt::format("{}() #{} Else",whilestr,whileid));
	}
	
	next = new ONoOp();
	next->setLabel(endlabel);
	addOpcode(next);
}

void BuildOpcodes::caseStmtReturn(ASTStmtReturn&, void*)
{
	deallocateRefsUntilCount(0);
	addOpcode(new OGotoImmediate(new LabelArgument(returnlabelid)));
	commentBack("return (Void)");
}

void BuildOpcodes::caseStmtReturnVal(ASTStmtReturnVal &host, void *param)
{
	auto targ_sz = commentTarget();
	INITC_STORE();
	INITC_VISIT(host.value.get());
	INITC_INIT();
	INITC_DEALLOC();
	deallocateRefsUntilCount(0);
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
	int32_t refcount = breakRefCounts.at(breakRefCounts.size()-host.breakCount);
	int32_t breaklabel = breaklabelids.at(breaklabelids.size()-host.breakCount);
	deallocateRefsUntilCount(refcount);
	addOpcode(new OGotoImmediate(new LabelArgument(breaklabel)));
	commentBack(fmt::format("break #{}",breaklabel));
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

	int32_t refcount = continueRefCounts.at(continueRefCounts.size()-host.contCount);
	int32_t contlabel = continuelabelids.at(continuelabelids.size()-host.contCount);
	deallocateRefsUntilCount(refcount);
	addOpcode(new OGotoImmediate(new LabelArgument(contlabel)));
	commentBack(fmt::format("continue #{}",contlabel));
	inc_cont(host.contCount);
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
	returnRefCount = arrayRefs.size();
	
	in_func_body = true;
	visit(host.block.get(), param);
	in_func_body = false;
}

void BuildOpcodes::caseDataDecl(ASTDataDecl& host, void* param)
{
	if(parsing_user_class == puc_vars) return;
	OpcodeContext& context = *(OpcodeContext*)param;
	Datum& manager = *host.manager;
	ASTExpr* init = host.getInitializer();

	// Ignore inlined values.
	if (manager.getCompileTimeValue()) return;

	// Switch off to the proper helper function.
	if (manager.type.isArray()
		|| (init && (init->isArrayLiteral()
					 || init->isStringLiteral())))
	{
		if (init) buildArrayInit(host, context);
		else buildArrayUninit(host, context);
	}
	else buildVariable(host, context);
}

void BuildOpcodes::buildVariable(ASTDataDecl& host, OpcodeContext& context)
{
	Datum& manager = *host.manager;

	// Load initializer into EXP1, if present.
	visit(host.getInitializer(), &context);

	// Set variable to EXP1 or 0, depending on the initializer.
	if (auto globalId = manager.getGlobalId())
	{
		if (host.getInitializer())
			addOpcode(new OSetRegister(new GlobalArgument(*globalId),
									   new VarArgument(EXP1)));
		else
			addOpcode(new OSetImmediate(new GlobalArgument(*globalId),
										new LiteralArgument(0)));
	}
	else
	{
		int32_t offset = 10000L * *getStackOffset(manager);
		if (!host.getInitializer())
			addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
		addOpcode(new OStoreDirect(new VarArgument(EXP1), new LiteralArgument(offset)));
	}
}

void BuildOpcodes::buildArrayInit(ASTDataDecl& host, OpcodeContext& context)
{
	Datum& manager = *host.manager;

	// Initializer should take care of everything.
	visit(host.getInitializer(), &context);
}

void BuildOpcodes::buildArrayUninit(
		ASTDataDecl& host, OpcodeContext& context)
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

	// Allocate the array.
	if (auto globalId = manager.getGlobalId())
	{
		addOpcode(new OAllocateGlobalMemImmediate(
						  new VarArgument(EXP1),
						  new LiteralArgument(totalSize)));
		addOpcode(new OSetRegister(new GlobalArgument(*globalId),
								   new VarArgument(EXP1)));
	}
	else
	{
		addOpcode(new OAllocateMemImmediate(new VarArgument(EXP1), new LiteralArgument(totalSize)));
		int32_t offset = 10000L * *getStackOffset(manager);
		addOpcode(new OStoreDirect(new VarArgument(EXP1), new LiteralArgument(offset)));
		// Register for cleanup.
		arrayRefs.push_back(offset);
	}
}

void BuildOpcodes::caseDataTypeDef(ASTDataTypeDef&, void*) {}

void BuildOpcodes::caseCustomDataTypeDef(ASTDataTypeDef&, void*) {}

// Expressions

void BuildOpcodes::caseExprAssign(ASTExprAssign &host, void *param)
{
	//load the rval into EXP1
	visit(host.right.get(), param);
	//and store it
	LValBOHelper helper(this);
	helper.parsing_user_class = parsing_user_class;
	host.left->execute(helper, param);
	addOpcodes(helper.getResult());
}

void BuildOpcodes::caseExprIdentifier(ASTExprIdentifier& host, void* param)
{
	if(parsing_user_class > puc_vars)
	{
		if(host.asString() == "this")
		{
			addOpcode(new OSetRegister(new VarArgument(EXP1), new VarArgument(CLASS_THISKEY)));	
			return;
		}
	}
	OpcodeContext* c = (OpcodeContext*)param;
	
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

	int32_t vid = host.binding->id;

	if (auto globalId = host.binding->getGlobalId())
	{
		// Global variable, so just get its value.
		addOpcode(new OSetRegister(new VarArgument(EXP1),
								   new GlobalArgument(*globalId)));
		return;
	}

	// Local variable, get its value from the stack.
	int32_t offset = 10000L * *getStackOffset(*host.binding);
	addOpcode(new OLoadDirect(new VarArgument(EXP1), new LiteralArgument(offset)));
}

void BuildOpcodes::caseExprArrow(ASTExprArrow& host, void* param)
{
	if(UserClassVar* ucv = host.u_datum)
	{
		visit(host.left.get(), param);
		addOpcode(new OReadObject(new VarArgument(EXP1), new LiteralArgument(ucv->getIndex())));
		return;
	}
	OpcodeContext *c = (OpcodeContext *)param;
	bool isarray = host.arrayFunction;
	bool isIndexed = isarray ? false : host.index;
	Function* readfunc = isarray ? host.arrayFunction : host.readFunction;
	assert(readfunc->isInternal());
	
	if(readfunc->getFlag(FUNCFLAG_NIL))
	{
		bool skipptr = readfunc->getIntFlag(IFUNCFLAG_SKIPPOINTER);
		if (!skipptr)
		{
			//visit the lhs of the arrow
			visit(host.left.get(), param);
		}
		
		if(isIndexed)
		{
			visit(host.index.get(), param);
		}
		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
	}
	else if(readfunc->getFlag(FUNCFLAG_INLINE))
	{
		if (!(readfunc->getIntFlag(IFUNCFLAG_SKIPPOINTER)))
		{
			//push the lhs of the arrow
			visit(host.left.get(), param);
			addOpcode(new OPushRegister(new VarArgument(EXP1)));
		}
		
		if(isIndexed)
		{
			visit(host.index.get(), param);
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
			visit(host.left.get(), param);
			addOpcode(new OPushRegister(new VarArgument(EXP1)));
		}

		//if indexed, push the index
		if(isIndexed)
		{
			visit(host.index.get(), param);
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
		if(!arrow->arrayFunction && !arrow->isTypeArrowUsrClass())
		{
			caseExprArrow(static_cast<ASTExprArrow&>(*host.array), param);
			return;
		}
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
	int32_t startRefCount = arrayRefs.size(); //Store ref count
	const string func_comment = fmt::format("Func[{}]",func.getUnaliasedSignature(true).asString());
	auto targ_sz = commentTarget();
	bool never_ret = func.getFlag(FUNCFLAG_NEVER_RETURN);
	if(func.isNil()) //Prototype/Nil function
	{
		//Visit each parameter, in case there are side-effects; but don't push the results, as they are unneeded.
		for (auto it = host.parameters.begin();
			it != host.parameters.end(); ++it)
		{
			INITC_VISIT(*it);
		}
		commentStartEnd(targ_sz, fmt::format("Proto{} Visit Params",func_comment));
		
		//Set the return to the default value
		if(classfunc && func.getFlag(FUNCFLAG_CONSTRUCTOR) && parsing_user_class <= puc_vars)
		{
			ClassScope* cscope = func.getInternalScope()->getClass();
			UserClass& user_class = cscope->user_class;
			vector<Function*> destr = cscope->getDestructor();
			Function* destructor = destr.size() == 1 ? destr.at(0) : nullptr;
			if(destructor && !destructor->isNil())
			{
				Function* destructor = destr[0];
				addOpcode(new OSetImmediate(new VarArgument(EXP1),
					new LabelArgument(destructor->getLabel(), true)));
			}
			else addOpcode(new OSetImmediate(new VarArgument(EXP1),
				new LiteralArgument(0)));
			commentBack(fmt::format("Proto{} Set Destructor",func_comment));
			addOpcode(new OConstructClass(new VarArgument(EXP1),
				new VectorArgument(user_class.members)));
			commentBack(fmt::format("Proto{} Default Construct",func_comment));
		}
		else
		{
			DataType const& retType = *func.returnType;
			if(!retType.isVoid())
			{
				int32_t retval = 0;
				if (auto val = func.defaultReturn->getCompileTimeValue(this, scope))
					retval = *val;
				addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(retval)));
				commentBack(fmt::format("Proto{} Default RetVal",func_comment));
			}
		}
	}
	else if(func.getFlag(FUNCFLAG_INLINE) && func.isInternal()) //Inline function
	{
		// User functions actually can't really benefit from any optimization like this... -Em
		size_t num_actual_params = func.paramTypes.size() - func.extra_vargs;
		size_t num_used_params = host.parameters.size();
		
		if (host.left->isTypeArrow())
		{
			++num_used_params;
			if (!(func.getIntFlag(IFUNCFLAG_SKIPPOINTER)))
			{
				//load the value of the left-hand of the arrow into EXP1
				INITC_VISIT(static_cast<ASTExprArrow&>(*host.left).left.get());
				//INITC_VISIT(host.getLeft());
				//push it onto the stack
				addOpcode(new OPushRegister(new VarArgument(EXP1)));
			}
		}
		
		bool vargs = func.getFlag(FUNCFLAG_VARARGS);
		int v = num_used_params-num_actual_params;
		
		size_t vargcount = 0;
		size_t used_opt_params = 0;
		(v>0 ? vargcount : used_opt_params) = abs(v);
		//push the parameters, in forward order
		size_t param_indx = 0;
		for (; param_indx < host.parameters.size()-vargcount; ++param_indx)
		{
			auto& arg = host.parameters.at(param_indx);
			//Compile-time constants can be optimized slightly...
			if(auto val = arg->getCompileTimeValue(this, scope))
				addOpcode(new OPushImmediate(new LiteralArgument(*val)));
			else
			{
				INITC_VISIT(arg);
				//Optimize
				Opcode* lastop = optarg->back().get();
				if (OSetRegister* tmp = dynamic_cast<OSetRegister*>(lastop))
				{
					VarArgument* destreg = static_cast<VarArgument*>(tmp->getFirstArgument());
					if (destreg->ID == EXP1)
					{
						Argument* arg = tmp->getSecondArgument()->clone();
						optarg->pop_back();
						addOpcode(new OPushRegister(arg));
						continue;
					}
				}
				else if (OSetImmediate* tmp = dynamic_cast<OSetImmediate*>(lastop))
				{
					VarArgument* destreg = static_cast<VarArgument*>(tmp->getFirstArgument());
					if (destreg->ID == EXP1)
					{
						Argument* arg = tmp->getSecondArgument()->clone();
						optarg->pop_back();
						addOpcode(new OPushImmediate(arg));
						continue;
					}
				}
				addOpcode(new OPushRegister(new VarArgument(EXP1)));
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
					INITC_VISIT(arg);
					//Optimize
					Opcode* lastop = optarg->back().get();
					if(OSetRegister* tmp = dynamic_cast<OSetRegister*>(lastop))
					{
						VarArgument* destreg = static_cast<VarArgument*>(tmp->getFirstArgument());
						if(destreg->ID == EXP1)
						{
							Argument* arg = tmp->getSecondArgument()->clone();
							optarg->pop_back();
							addOpcode(new OPushVargR(arg));
							continue;
						}
					}
					else if(OSetImmediate* tmp = dynamic_cast<OSetImmediate*>(lastop))
					{
						VarArgument* destreg = static_cast<VarArgument*>(tmp->getFirstArgument());
						if(destreg->ID == EXP1)
						{
							Argument* arg = tmp->getSecondArgument()->clone();
							optarg->pop_back();
							addOpcode(new OPushVargV(arg));
							continue;
						}
					}
					addOpcode(new OPushVargR(new VarArgument(EXP1)));
				}
			}
		}
		else if(used_opt_params)
		{
			auto opt_param_count = func.opt_vals.size();
			auto skipped_optional_params = opt_param_count - used_opt_params;
			//push any optional parameter values
			for(auto q = skipped_optional_params; q < opt_param_count; ++q)
			{
				addOpcode(new OPushImmediate(new LiteralArgument(func.opt_vals[q])));
			}
		}
		std::vector<std::shared_ptr<Opcode>> const& funcCode = func.getCode();
		auto it = funcCode.begin();
		while(OPopRegister* ocode = dynamic_cast<OPopRegister*>(it->get()))
		{
			VarArgument const* destreg = static_cast<VarArgument*>(ocode->getArgument());
			//Optimize
			Opcode* lastop = optarg->back().get();
			if(OPushRegister* tmp = dynamic_cast<OPushRegister*>(lastop))
			{
				VarArgument const* arg = static_cast<VarArgument*>(tmp->getArgument());
				if(arg->ID == destreg->ID) //Same register!
				{
					optarg->pop_back();
				}
				else //Different register
				{
					Argument* a = arg->clone();
					optarg->pop_back();
					addOpcode(new OSetRegister(destreg->clone(), a));
				}
				if(++it == funcCode.end())
					break;
				continue;
			}
			else if(OPushImmediate* tmp = dynamic_cast<OPushImmediate*>(lastop))
			{
				Argument* arg = tmp->getArgument()->clone();
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
					LValBOHelper helper(this);
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
		size_t vargcount = 0;
		size_t used_opt_params = 0;
		int v = num_used_params-num_actual_params;
		(v>0 ? vargcount : used_opt_params) = abs(v);
		size_t pushcount = 0;
		if(vargs)
		{
			//push the vargs, in forward order
			for (size_t param_indx = num_used_params-vargcount; param_indx < num_used_params; ++param_indx)
			{
				auto& arg = host.parameters.at(param_indx);
				//Compile-time constants can be optimized slightly...
				if(auto val = arg->getCompileTimeValue(this, scope))
					addOpcode(new OPushVargV(new LiteralArgument(*val)));
				else
				{
					INITC_VISIT(arg);
					//Optimize
					Opcode* lastop = optarg->back().get();
					if(OSetRegister* tmp = dynamic_cast<OSetRegister*>(lastop))
					{
						VarArgument* destreg = static_cast<VarArgument*>(tmp->getFirstArgument());
						if(destreg->ID == EXP1)
						{
							Argument* arg = tmp->getSecondArgument()->clone();
							optarg->pop_back();
							addOpcode(new OPushVargR(arg));
							continue;
						}
					}
					else if(OSetImmediate* tmp = dynamic_cast<OSetImmediate*>(lastop))
					{
						VarArgument* destreg = static_cast<VarArgument*>(tmp->getFirstArgument());
						if(destreg->ID == EXP1)
						{
							Argument* arg = tmp->getSecondArgument()->clone();
							optarg->pop_back();
							addOpcode(new OPushVargV(arg));
							continue;
						}
					}
					addOpcode(new OPushVargR(new VarArgument(EXP1)));
				}
			}
			addOpcode(new OMakeVargArray());
			commentBack("Allocate Vargs array");
			addOpcode(new OPushRegister(new VarArgument(EXP1)));
			commentBack("Push the Vargs array pointer");
			++pushcount;
		}
		commentStartEnd(targ_sz, fmt::format("Class{} Vargs",func_comment));
		//push the this key/stack frame pointer
		if(!never_ret)
		{
			addOpcode(new OPushRegister(new VarArgument(CLASS_THISKEY)));
			addOpcode(new OPushRegister(new VarArgument(SFRAME)));
			pushcount += 2;
		}
		
		targ_sz = commentTarget();
		//push the parameters, in forward order
		size_t param_indx = 0;
		for (; param_indx < num_used_params-vargcount; ++param_indx)
		{
			auto& arg = host.parameters.at(param_indx);
			//Compile-time constants can be optimized slightly...
			if(auto val = arg->getCompileTimeValue(this, scope))
				addOpcode(new OPushImmediate(new LiteralArgument(*val)));
			else
			{
				INITC_VISIT(arg);
				//Optimize
				Opcode* lastop = optarg->back().get();
				if(OSetRegister* tmp = dynamic_cast<OSetRegister*>(lastop))
				{
					VarArgument* destreg = static_cast<VarArgument*>(tmp->getFirstArgument());
					if(destreg->ID == EXP1)
					{
						Argument* arg = tmp->getSecondArgument()->clone();
						optarg->pop_back();
						addOpcode(new OPushRegister(arg));
						continue;
					}
				}
				else if(OSetImmediate* tmp = dynamic_cast<OSetImmediate*>(lastop))
				{
					VarArgument* destreg = static_cast<VarArgument*>(tmp->getFirstArgument());
					if(destreg->ID == EXP1)
					{
						Argument* arg = tmp->getSecondArgument()->clone();
						optarg->pop_back();
						addOpcode(new OPushImmediate(arg));
						continue;
					}
				}
				addOpcode(new OPushRegister(new VarArgument(EXP1)));
			}
		}
		pushcount += (num_used_params-vargcount);
		if(vargs)
		{
			if(auto offs = pushcount-1)
				addOpcode(new OPeekAtImmediate(new VarArgument(EXP1), new LiteralArgument(offs)));
			else addOpcode(new OPeek(new VarArgument(EXP1)));
			commentBack("Peek the Vargs array pointer");
			addOpcode(new OPushRegister(new VarArgument(EXP1)));
		}
		else if(used_opt_params)
		{
			//push any optional parameter values
			auto num_actual_params = func.paramTypes.size();
			auto used_opt_params = num_actual_params - host.parameters.size();
			auto opt_param_count = func.opt_vals.size();
			auto skipped_optional_params = opt_param_count - used_opt_params;
			for(auto q = skipped_optional_params; q < opt_param_count; ++q)
			{
				addOpcode(new OPushImmediate(new LiteralArgument(func.opt_vals[q])));
			}
		}
		if (host.left->isTypeArrow())
		{
			//load the value of the left-hand of the arrow into EXP1
			INITC_VISIT(static_cast<ASTExprArrow&>(*host.left).left.get());
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
			if(vargs)
			{
				addOpcode(new OPopRegister(new VarArgument(EXP2)));
				addOpcode(new ODeallocateMemRegister(new VarArgument(EXP2)));
				commentBack("Deallocate Vargs array");
			}
		}
	}
	else
	{
		const string comment_pref = func.isInternal() ? "Int." : "Usr";
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
		size_t pushcount = 0;
		
		if(user_vargs)
		{
			//push the vargs, in forward order
			for (size_t param_indx = num_used_params-vargcount; param_indx < num_used_params; ++param_indx)
			{
				auto& arg = host.parameters.at(param_indx);
				//Compile-time constants can be optimized slightly...
				if(auto val = arg->getCompileTimeValue(this, scope))
					addOpcode(new OPushVargV(new LiteralArgument(*val)));
				else
				{
					INITC_VISIT(arg);
					//Optimize
					Opcode* lastop = optarg->back().get();
					if(OSetRegister* tmp = dynamic_cast<OSetRegister*>(lastop))
					{
						VarArgument* destreg = static_cast<VarArgument*>(tmp->getFirstArgument());
						if(destreg->ID == EXP1)
						{
							Argument* arg = tmp->getSecondArgument()->clone();
							optarg->pop_back();
							addOpcode(new OPushVargR(arg));
							continue;
						}
					}
					else if(OSetImmediate* tmp = dynamic_cast<OSetImmediate*>(lastop))
					{
						VarArgument* destreg = static_cast<VarArgument*>(tmp->getFirstArgument());
						if(destreg->ID == EXP1)
						{
							Argument* arg = tmp->getSecondArgument()->clone();
							optarg->pop_back();
							addOpcode(new OPushVargV(arg));
							continue;
						}
					}
					addOpcode(new OPushVargR(new VarArgument(EXP1)));
				}
			}
			addOpcode(new OMakeVargArray());
			commentBack("Allocate Vargs array");
			addOpcode(new OPushRegister(new VarArgument(EXP1)));
			commentBack("Push the Vargs array pointer");
			++pushcount;
		}
		commentStartEnd(targ_sz, fmt::format("{}{} Vargs",comment_pref,func_comment));
		//push the stack frame pointer
		if(!never_ret)
		{
			addOpcode(new OPushRegister(new VarArgument(SFRAME)));
			++pushcount;
		}
		targ_sz = commentTarget();
		// If the function is a pointer function (->func()) we need to push the
		// left-hand-side.
		if (host.left->isTypeArrow() && !(func.getIntFlag(IFUNCFLAG_SKIPPOINTER)))
		{
			//load the value of the left-hand of the arrow into EXP1
			INITC_VISIT(static_cast<ASTExprArrow&>(*host.left).left.get());
			//INITC_VISIT(host.getLeft());
			//push it onto the stack
			addOpcode(new OPushRegister(new VarArgument(EXP1)));
			++pushcount;
		}
		
		//push the parameters, in forward order
		size_t param_indx = 0;
		for (; param_indx < num_used_params-vargcount; ++param_indx)
		{
			auto& arg = host.parameters.at(param_indx);
			//Compile-time constants can be optimized slightly...
			if(auto val = arg->getCompileTimeValue(this, scope))
				addOpcode(new OPushImmediate(new LiteralArgument(*val)));
			else
			{
				INITC_VISIT(arg);
				//Optimize
				Opcode* lastop = optarg->back().get();
				if(OSetRegister* tmp = dynamic_cast<OSetRegister*>(lastop))
				{
					VarArgument* destreg = static_cast<VarArgument*>(tmp->getFirstArgument());
					if(destreg->ID == EXP1)
					{
						Argument* arg = tmp->getSecondArgument()->clone();
						optarg->pop_back();
						addOpcode(new OPushRegister(arg));
						continue;
					}
				}
				else if(OSetImmediate* tmp = dynamic_cast<OSetImmediate*>(lastop))
				{
					VarArgument* destreg = static_cast<VarArgument*>(tmp->getFirstArgument());
					if(destreg->ID == EXP1)
					{
						Argument* arg = tmp->getSecondArgument()->clone();
						optarg->pop_back();
						addOpcode(new OPushImmediate(arg));
						continue;
					}
				}
				addOpcode(new OPushRegister(new VarArgument(EXP1)));
			}
		}
		pushcount += (num_used_params-vargcount);
		if(vargs && (vargcount || user_vargs))
		{
			if(user_vargs)
			{
				if(auto offs = pushcount-1)
					addOpcode(new OPeekAtImmediate(new VarArgument(EXP1), new LiteralArgument(offs)));
				else addOpcode(new OPeek(new VarArgument(EXP1)));
				commentBack("Peek the Vargs array pointer");
				addOpcode(new OPushRegister(new VarArgument(EXP1)));
			}
			else
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
						INITC_VISIT(arg);
						//Optimize
						Opcode* lastop = optarg->back().get();
						if(OSetRegister* tmp = dynamic_cast<OSetRegister*>(lastop))
						{
							VarArgument* destreg = static_cast<VarArgument*>(tmp->getFirstArgument());
							if(destreg->ID == EXP1)
							{
								Argument* arg = tmp->getSecondArgument()->clone();
								optarg->pop_back();
								addOpcode(new OPushVargR(arg));
								continue;
							}
						}
						else if(OSetImmediate* tmp = dynamic_cast<OSetImmediate*>(lastop))
						{
							VarArgument* destreg = static_cast<VarArgument*>(tmp->getFirstArgument());
							if(destreg->ID == EXP1)
							{
								Argument* arg = tmp->getSecondArgument()->clone();
								optarg->pop_back();
								addOpcode(new OPushVargV(arg));
								continue;
							}
						}
						addOpcode(new OPushVargR(new VarArgument(EXP1)));
					}
				}
			}
		}
		else if(used_opt_params)
		{
			//push any optional parameter values
			auto opt_param_count = func.opt_vals.size();
			auto skipped_optional_params = opt_param_count - used_opt_params;
			for(auto q = skipped_optional_params; q < opt_param_count; ++q)
			{
				addOpcode(new OPushImmediate(new LiteralArgument(func.opt_vals[q])));
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
			if(user_vargs)
			{
				addOpcode(new OPopRegister(new VarArgument(EXP2)));
				addOpcode(new ODeallocateMemRegister(new VarArgument(EXP2)));
				commentBack("Deallocate Vargs array");
			}
			
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
						LValBOHelper helper(this);
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
	INITC_DEALLOC();
	
	//Deallocate string/array literals from within the parameters
	deallocateRefsUntilCount(startRefCount);
	while ((int32_t)arrayRefs.size() > startRefCount)
		arrayRefs.pop_back();
}

void BuildOpcodes::caseExprNegate(ASTExprNegate& host, void* param)
{
	if (auto val = host.getCompileTimeValue(this, scope))
	{
		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*val)));
		return;
	}

	visit(host.operand.get(), param);
	addOpcode(new OSubImmediate2(new LiteralArgument(0), new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprDelete(ASTExprDelete& host, void* param)
{
	visit(host.operand.get(), param);
	addOpcode(new OFreeObject(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprNot(ASTExprNot& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
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
		if(decret)
			addOpcode(new OSetTrue(new VarArgument(EXP1)));
		else
			addOpcode(new OSetTrueI(new VarArgument(EXP1)));
	}
}

void BuildOpcodes::caseExprBitNot(ASTExprBitNot& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}

	visit(host.operand.get(), param);
	
	if(*lookupOption(*scope, CompileOption::OPT_BINARY_32BIT)
	   || host.operand.get()->isLong(scope, this))
		addOpcode(new O32BitNot(new VarArgument(EXP1)));
	else
		addOpcode(new ONot(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprIncrement(ASTExprIncrement& host, void* param)
{
	OpcodeContext* c = (OpcodeContext*)param;

	// Load value of the variable into EXP1 and push.
	visit(host.operand.get(), param);
	addOpcode(new OPushRegister(new VarArgument(EXP1)));

	// Increment EXP1
	addOpcode(new OAddImmediate(new VarArgument(EXP1),
								new LiteralArgument(10000)));
	
	// Store it
	LValBOHelper helper(this);
	helper.parsing_user_class = parsing_user_class;
	host.operand->execute(helper, param);
	addOpcodes(helper.getResult());
	
	// Pop EXP1
	addOpcode(new OPopRegister(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprPreIncrement(ASTExprPreIncrement& host, void* param)
{
	OpcodeContext* c = (OpcodeContext*)param;

	// Load value of the variable into EXP1.
	visit(host.operand.get(), param);

	// Increment EXP1
	addOpcode(new OAddImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));

	// Store it
	LValBOHelper helper(this);
	helper.parsing_user_class = parsing_user_class;
	host.operand->execute(helper, param);
	addOpcodes(helper.getResult());
}

void BuildOpcodes::caseExprPreDecrement(ASTExprPreDecrement& host, void* param)
{
	OpcodeContext* c = (OpcodeContext*)param;

	// Load value of the variable into EXP1.
	visit(host.operand.get(), param);

	// Decrement EXP1.
	addOpcode(new OSubImmediate(new VarArgument(EXP1),
								new LiteralArgument(10000)));

	// Store it.
	LValBOHelper helper(this);
	helper.parsing_user_class = parsing_user_class;
	host.operand->execute(helper, param);
	addOpcodes(helper.getResult());
}

void BuildOpcodes::caseExprDecrement(ASTExprDecrement& host, void* param)
{
	OpcodeContext* c = (OpcodeContext*)param;

	// Load value of the variable into EXP1 and push.
	visit(host.operand.get(), param);
	addOpcode(new OPushRegister(new VarArgument(EXP1)));

	// Decrement EXP1.
	addOpcode(new OSubImmediate(new VarArgument(EXP1),
								new LiteralArgument(10000)));
	// Store it.
	LValBOHelper helper(this);
	helper.parsing_user_class = parsing_user_class;
	host.operand->execute(helper, param);
	addOpcodes(helper.getResult());

	// Pop EXP1.
	addOpcode(new OPopRegister(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprAnd(ASTExprAnd& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
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
		addOpcode(new OCastBoolF(new VarArgument(EXP1))); //Don't break boolean ops on negative numbers on the RHS.
		Opcode* ocode =  new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(1));
		ocode->setLabel(skip);
		addOpcode(ocode);
	}
	else
	{
		//Get left
		visit(host.left.get(), param);
		//Store left for later
		addOpcode(new OPushRegister(new VarArgument(EXP1)));
		//Get right
		visit(host.right.get(), param);
		//Retrieve left
		addOpcode(new OPopRegister(new VarArgument(EXP2)));
		addOpcode(new OCastBoolF(new VarArgument(EXP1)));
		addOpcode(new OCastBoolF(new VarArgument(EXP2)));
		addOpcode(new OAddRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
		addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(2)));
	}
	if(decret)
		addOpcode(new OSetMore(new VarArgument(EXP1)));
	else
		addOpcode(new OSetMoreI(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprOr(ASTExprOr& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
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
		addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(1)));
		addOpcode(new OGotoMoreImmediate(new LabelArgument(skip)));
		//Get rightx
		//Get right
		visit(host.right.get(), param);
		addOpcode(new OCastBoolF(new VarArgument(EXP1))); //Don't break boolean ops on negative numbers on the RHS.
		addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(1)));
		//Set output
		Opcode* ocode = decret ? (Opcode*)(new OSetMore(new VarArgument(EXP1))) : (Opcode*)(new OSetMoreI(new VarArgument(EXP1)));
		ocode->setLabel(skip);
		addOpcode(ocode);
	}
	else
	{
		//Get left
		visit(host.left.get(), param);
		//Store left for later
		addOpcode(new OPushRegister(new VarArgument(EXP1)));
		//Get right
		visit(host.right.get(), param);
		//Retrieve left
		addOpcode(new OPopRegister(new VarArgument(EXP2)));
		addOpcode(new OCastBoolF(new VarArgument(EXP1)));
		addOpcode(new OCastBoolF(new VarArgument(EXP2)));
		addOpcode(new OAddRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
		addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(1)));
		if(decret)
			addOpcode(new OSetMore(new VarArgument(EXP1)));
		else
			addOpcode(new OSetMoreI(new VarArgument(EXP1)));
	}
}

void BuildOpcodes::caseExprGT(ASTExprGT& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	bool decret = *lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL)!=0;
	compareExprs(host.left.get(), host.right.get(), param);
	addOpcode(new OSetLess(new VarArgument(EXP1)));
	addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
	if(decret)
		addOpcode(new OSetTrue(new VarArgument(EXP1)));
	else
		addOpcode(new OSetTrueI(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprGE(ASTExprGE& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	bool decret = *lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL)!=0;
	compareExprs(host.left.get(), host.right.get(), param);
	if(decret)
		addOpcode(new OSetMore(new VarArgument(EXP1)));
	else
		addOpcode(new OSetMoreI(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprLT(ASTExprLT& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	bool decret = *lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL)!=0;
	compareExprs(host.left.get(), host.right.get(), param);
	addOpcode(new OSetMore(new VarArgument(EXP1)));
	addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
	if(*lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL))
		addOpcode(new OSetTrue(new VarArgument(EXP1)));
	else
		addOpcode(new OSetTrueI(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprLE(ASTExprLE& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	bool decret = *lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL)!=0;
	compareExprs(host.left.get(), host.right.get(), param);
	if(decret)
		addOpcode(new OSetLess(new VarArgument(EXP1)));
	else
		addOpcode(new OSetLessI(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprEQ(ASTExprEQ& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	
	// Special case for booleans.
	DataType const* ltype = host.left->getReadType(scope, this);
	DataType const* rtype = host.right->getReadType(scope, this);
	bool isBoolean = (*ltype == DataType::BOOL || *rtype == DataType::BOOL || *ltype == DataType::CBOOL || *rtype == DataType::CBOOL);
	
	bool decret = *lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL)!=0;
	compareExprs(host.left.get(), host.right.get(), param, isBoolean);
	if(decret)
		addOpcode(new OSetTrue(new VarArgument(EXP1)));
	else
		addOpcode(new OSetTrueI(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprNE(ASTExprNE& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	
	// Special case for booleans.
	DataType const* ltype = host.left->getReadType(scope, this);
	DataType const* rtype = host.right->getReadType(scope, this);
	bool isBoolean = (*ltype == DataType::BOOL || *rtype == DataType::BOOL || *ltype == DataType::CBOOL || *rtype == DataType::CBOOL);
	
	bool decret = *lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL)!=0;
	compareExprs(host.left.get(), host.right.get(), param, isBoolean);
	if(decret)
		addOpcode(new OSetFalse(new VarArgument(EXP1)));
	else
		addOpcode(new OSetFalseI(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprAppxEQ(ASTExprAppxEQ& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}

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
	if(*lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL))
		addOpcode(new OSetLess(new VarArgument(EXP1)));
	else
		addOpcode(new OSetLessI(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprXOR(ASTExprXOR& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
	
	bool decret = *lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL)!=0;
	compareExprs(host.left.get(), host.right.get(), param, true);
	if(decret)
		addOpcode(new OSetFalse(new VarArgument(EXP1)));
	else
		addOpcode(new OSetFalseI(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprPlus(ASTExprPlus& host, void* param)
{
	if (auto v = host.getCompileTimeValue(this, scope))
	{
		CONST_VAL(*v);
		return;
	}
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
			handleError(CompileError::DivByZero(&host,"divide"));
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
			handleError(CompileError::DivByZero(&host,"modulo"));
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
			visit(host.middle.get());
		else visit(host.right.get());
	}
	else
	{
		auto mval = host.middle->getCompileTimeValue(this, scope);
		auto rval = host.right->getCompileTimeValue(this, scope);
		
		visit(host.left.get(), param);
		int32_t elseif = ScriptParser::getUniqueLabelID();
		int32_t endif = ScriptParser::getUniqueLabelID();
		addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
		addOpcode(new OGotoTrueImmediate(new LabelArgument(elseif)));
		if(mval)
			addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*mval)));
		else visit(host.middle.get(), param); //Use middle section
		addOpcode(new OGotoImmediate(new LabelArgument(endif))); //Skip right
		Opcode *next;
		if(rval)
		{
			next = new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*rval));
		}
		else next = new ONoOp();
		next->setLabel(elseif);
		addOpcode(next); //Add label for between middle and right
		if(!rval) visit(host.right.get(), param); //Use right section
		next = new ONoOp();
		next->setLabel(endif);
		addOpcode(next); //Add label for after right
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
	if (auto globalId = manager.getGlobalId())
	{
		addOpcode(new OAllocateGlobalMemImmediate(
						  new VarArgument(EXP1),
						  new LiteralArgument(size * 10000L)));
		addOpcode(new OSetRegister(new GlobalArgument(*globalId),
								   new VarArgument(EXP1)));
	}
	else
	{
		addOpcode(new OAllocateMemImmediate(new VarArgument(EXP1),
											new LiteralArgument(size * 10000L)));
		int32_t offset = 10000L * *getStackOffset(manager);
		addOpcode(new OStoreDirect(new VarArgument(EXP1), new LiteralArgument(offset)));
		// Register for cleanup.
		arrayRefs.push_back(offset);
	}

	addOpcode(new OWritePODString(new VarArgument(EXP1), new StringArgument(data)));
	// // Initialize array.
	// addOpcode(new OSetRegister(new VarArgument(INDEX),
							   // new VarArgument(EXP1)));
	// for (size_t i = 0; i < data.size(); ++i)
	// {
		// addOpcode(new OWritePODArrayII(
						  // new LiteralArgument(i * 10000L),
						  // new LiteralArgument(data[i] * 10000L)));
	// }
	// //Add nullchar
	// addOpcode(new OWritePODArrayII(
					  // new LiteralArgument(data.size() * 10000L),
					  // new LiteralArgument(0)));
}

void BuildOpcodes::stringLiteralFree(
		ASTStringLiteral& host, OpcodeContext& context)
{
	Literal& manager = *host.manager;
	string data = host.value;
	int32_t size = data.size() + 1;
	int32_t offset = *getStackOffset(manager) * 10000L;
	vector<shared_ptr<Opcode>>& init = context.initCode;
	vector<shared_ptr<Opcode>>& dealloc = context.deallocCode;

	////////////////////////////////////////////////////////////////
	// Initialization Code.

	// Allocate.
	addOpcode2(init, new OAllocateMemImmediate(
						   new VarArgument(EXP1),
						   new LiteralArgument(size * 10000L)));
	addOpcode2(init, new OStoreDirect(new VarArgument(EXP1),
									  new LiteralArgument(offset)));

	// Initialize.
	addOpcode2(init, new OWritePODString(new VarArgument(EXP1), new StringArgument(data)));
	// addOpcode2(init, new OSetRegister(new VarArgument(INDEX),
									// new VarArgument(EXP1)));
	// for (int32_t i = 0; i < (int32_t)data.size(); ++i)
	// {
		// addOpcode2(init, new OWritePODArrayII(
							   // new LiteralArgument(i * 10000L),
							   // new LiteralArgument(data[i] * 10000L)));
	// }
	// //Add nullchar
	// addOpcode2(init, new OWritePODArrayII(
						   // new LiteralArgument(data.size() * 10000L),
						   // new LiteralArgument(0)));

	////////////////////////////////////////////////////////////////
	// Actual Code.

	// Local variable, get its value from the stack.
	addOpcode(new OLoadDirect(new VarArgument(EXP1),
								new LiteralArgument(offset)));

	////////////////////////////////////////////////////////////////
	// Register for cleanup.
	
	deallocateArrayRef(offset, dealloc);
	//arrayRefs.push_back(offset); //Replaced by deallocCode
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

	// Create the array and store its id.
	if (auto globalId = manager.getGlobalId())
	{
		addOpcode(new OAllocateGlobalMemImmediate(
						  new VarArgument(EXP1),
						  new LiteralArgument(size * 10000L)));
		addOpcode(new OSetRegister(new GlobalArgument(*globalId),
								   new VarArgument(EXP1)));
	}
	else
	{
		addOpcode(new OAllocateMemImmediate(new VarArgument(EXP1),
											new LiteralArgument(size * 10000L)));
		int32_t offset = 10000L * *getStackOffset(manager);
		addOpcode(new OStoreDirect(new VarArgument(EXP1), new LiteralArgument(offset)));
		// Register for cleanup.
		arrayRefs.push_back(offset);
	}

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
}

void BuildOpcodes::arrayLiteralFree(
		ASTArrayLiteral& host, OpcodeContext& context)
{
	Literal& manager = *host.manager;

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

	int32_t offset = 10000L * *getStackOffset(manager);
	
	////////////////////////////////////////////////////////////////
	// Initialization Code.

	// Allocate.

	addOpcode2(context.initCode,
			new OAllocateMemImmediate(new VarArgument(EXP1),
									  new LiteralArgument(size * 10000L)));
	addOpcode2(context.initCode,
			new OStoreDirect(new VarArgument(EXP1),
							   new LiteralArgument(offset)));

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
		addOpcode2(context.initCode, new OWritePODArray(new VarArgument(EXP1), new VectorArgument(constelements)));
	}
	if(varelem)
	{
		addOpcode2(context.initCode, new OSetRegister(new VarArgument(INDEX), new VarArgument(EXP1)));
		int32_t i = 0;
		for (auto it = host.elements.begin(); it != host.elements.end(); ++it, i += 10000L)
		{
			if (!(*it)->getCompileTimeValue(this, scope))
			{
				addOpcode2(context.initCode, new OPushRegister(new VarArgument(INDEX)));
				opcodeTargets.push_back(&context.initCode);
				visit(*it, &context);
				opcodeTargets.pop_back();
				addOpcode2(context.initCode, new OPopRegister(new VarArgument(INDEX)));
				addOpcode2(context.initCode, new OWritePODArrayIR(new LiteralArgument(i), new VarArgument(EXP1)));
			}
		}
	}

	////////////////////////////////////////////////////////////////
	// Actual Code.

	// Local variable, get its value from the stack.
	addOpcode(new OLoadDirect(new VarArgument(EXP1),
								new LiteralArgument(offset)));

	////////////////////////////////////////////////////////////////
	// Register for cleanup.

	deallocateArrayRef(offset, context.deallocCode);
	//arrayRefs.push_back(offset); //Replaced by deallocCode
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

void BuildOpcodes::compareExprs(ASTExpr* left, ASTExpr* right, void* param, bool boolMode)
{
	auto lval = left->getCompileTimeValue(this, scope);
	auto rval = right->getCompileTimeValue(this, scope);
	if(lval)
	{
		visit(right, param);
		if(boolMode)
		{
			addOpcode(new OCastBoolF(new VarArgument(EXP1)));
			lval = (*lval) ? 1 : 0;
		}
		addOpcode(new OCompareImmediate2(new LiteralArgument(*lval), new VarArgument(EXP1)));
	}
	else if (rval)
	{
		visit(left, param);
		if(boolMode)
		{
			addOpcode(new OCastBoolF(new VarArgument(EXP1)));
			rval = (*rval) ? 1 : 0;
		}
		addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(*rval)));
	}
	else
	{
		//compute both sides
		visit(left, param);
		addOpcode(new OPushRegister(new VarArgument(EXP1)));
		visit(right, param);
		addOpcode(new OPopRegister(new VarArgument(EXP2)));
		if(boolMode)
		{
			addOpcode(new OCastBoolF(new VarArgument(EXP1)));
			addOpcode(new OCastBoolF(new VarArgument(EXP2)));
		}
		addOpcode(new OCompareRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
	}
}

/////////////////////////////////////////////////////////////////////////////////
// LValBOHelper

LValBOHelper::LValBOHelper(Scope* scope)
{
	ASTVisitor::scope = scope;
}
LValBOHelper::LValBOHelper(BuildOpcodes* bo)
{
	ASTVisitor::scope = bo->scope;
	parsing_user_class = bo->parsing_user_class;
	in_func_body = bo->in_func_body;
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

	if (auto globalId = host.binding->getGlobalId())
	{
		// Global variable.
		addOpcode(new OSetRegister(new GlobalArgument(*globalId),
								   new VarArgument(EXP1)));
		return;
	}

	// Set the stack.
	int32_t offset = 10000L * *getStackOffset(*host.binding);

	addOpcode(new OStoreDirect(new VarArgument(EXP1),new LiteralArgument(offset)));
}

void LValBOHelper::caseExprArrow(ASTExprArrow &host, void *param)
{
	if(UserClassVar* ucv = host.u_datum)
	{
		BuildOpcodes oc(this);
		oc.parsing_user_class = parsing_user_class;
		if(ucv->is_arr)
		{
			oc.visit(host.left.get(), param); //incase side effects
			addOpcodes(oc.getResult());
			return; //No overwriting object arrays!
		}
		addOpcode(new OPushRegister(new VarArgument(EXP1)));
		oc.visit(host.left.get(), param);
		addOpcodes(oc.getResult());
		addOpcode(new OSetRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
		addOpcode(new OPopRegister(new VarArgument(EXP1)));
		addOpcode(new OWriteObject(new VarArgument(EXP2), new LiteralArgument(ucv->getIndex())));
		return;
	}
	OpcodeContext *c = (OpcodeContext *)param;
	int32_t isIndexed = (host.index != NULL);
	assert(host.writeFunction->isInternal());
	
	if(host.writeFunction->getFlag(FUNCFLAG_NIL))
	{
		bool skipptr = host.writeFunction->getIntFlag(IFUNCFLAG_SKIPPOINTER);
		bool needs_pushpop = isIndexed || !skipptr;
		if(needs_pushpop)
			addOpcode(new OPushRegister(new VarArgument(EXP1)));
		if (!skipptr)
		{
			//Get lval
			BuildOpcodes oc(this);
			oc.parsing_user_class = parsing_user_class;
			oc.visit(host.left.get(), param);
			addOpcodes(oc.getResult());
		}
		
		if(isIndexed)
		{
			BuildOpcodes oc2(this);
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
			BuildOpcodes oc(this);
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
			BuildOpcodes oc2(this);
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
			BuildOpcodes oc(this);
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
			BuildOpcodes oc2(this);
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
		if(!arrow->arrayFunction && !arrow->isTypeArrowUsrClass())
		{
			caseExprArrow(static_cast<ASTExprArrow&>(*host.array), param);
			return;
		}
	}

	vector<shared_ptr<Opcode>> opcodes;
	BuildOpcodes bo(this);
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
		BuildOpcodes buildOpcodes1(this);
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
		BuildOpcodes buildOpcodes2(this);
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

