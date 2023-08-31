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

BuildOpcodes::BuildOpcodes(Scope* curScope)
	: returnlabelid(-1), continuelabelids(), breaklabelids(), 
	  returnRefCount(0), continueRefCounts(), breakRefCounts()
{
	opcodeTargets.push_back(&result);
	scope = curScope;
}

void addOpcode2(vector<shared_ptr<Opcode>>& v, Opcode* code)
{
	shared_ptr<Opcode> op(code);
	v.push_back(op);
}

void BuildOpcodes::visit(AST& node, void* param)
{
	if(node.isDisabled()) return; //Don't visit disabled nodes.
	RecursiveVisitor::visit(node, param);
	for (auto it = node.compileErrorCatches.cbegin(); it != node.compileErrorCatches.cend(); ++it)
	{
		ASTExprConst& idNode = **it;
		std::optional<int32_t> errorId = idNode.getCompileTimeValue(NULL, scope);
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
	if(host.isDecl())
	{
		if(!host.getScope())
		{
			host.setScope(scope->makeChild());
		}
		scope = host.getScope();
		int32_t startRefCount = arrayRefs.size();
		
		if(std::optional<int32_t> val = host.declaration->getInitializer()->getCompileTimeValue(this, scope))
		{
			if((host.isInverted()) == (*val==0)) //True, so go straight to the 'then'
			{
				literalVisit(host.declaration.get(), param); 
				visit(host.thenStatement.get(), param);
				deallocateRefsUntilCount(startRefCount);
				
				while ((int32_t)arrayRefs.size() > startRefCount)
					arrayRefs.pop_back();
				
				scope = scope->getParent();
			} //Either true or false, it's constant, so no checks required.
			return;
		}
		
		int32_t endif = ScriptParser::getUniqueLabelID();
		literalVisit(host.declaration.get(), param);
		
		//The condition should be reading the value just processed from the initializer
		visit(host.condition.get(), param);
		//
		addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
		if(host.isInverted())
			addOpcode(new OGotoFalseImmediate(new LabelArgument(endif)));
		else
			addOpcode(new OGotoTrueImmediate(new LabelArgument(endif)));
		
		visit(host.thenStatement.get(), param);
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
		if(std::optional<int32_t> val = host.condition->getCompileTimeValue(this, scope))
		{
			if((host.isInverted()) == (*val==0)) //True, so go straight to the 'then'
			{
				visit(host.thenStatement.get(), param);
			} //Either true or false, it's constant, so no checks required.
			return;
		}
		//run the test
		int32_t startRefCount = arrayRefs.size(); //Store ref count
		literalVisit(host.condition.get(), param);
		//Deallocate string/array literals from within the condition
		deallocateRefsUntilCount(startRefCount);
		while ((int32_t)arrayRefs.size() > startRefCount)
			arrayRefs.pop_back();
		//Continue
		int32_t endif = ScriptParser::getUniqueLabelID();
		addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
		if(host.isInverted())
			addOpcode(new OGotoFalseImmediate(new LabelArgument(endif)));
		else
			addOpcode(new OGotoTrueImmediate(new LabelArgument(endif)));
		//run the block
		visit(host.thenStatement.get(), param);
		//nop
		Opcode *next = new ONoOp();
		next->setLabel(endif);
		addOpcode(next);
	}
}

void BuildOpcodes::caseStmtIfElse(ASTStmtIfElse &host, void *param)
{
	if(host.isDecl())
	{
		if(!host.getScope())
		{
			host.setScope(scope->makeChild());
		}
		scope = host.getScope();
		int32_t startRefCount = arrayRefs.size();
		
		if(std::optional<int32_t> val = host.declaration->getInitializer()->getCompileTimeValue(this, scope))
		{
			if((host.isInverted()) == (*val==0)) //True, so go straight to the 'then'
			{
				literalVisit(host.declaration.get(), param); 
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
				visit(host.elseStatement.get(), param);
			}
			//Either way, ignore the rest and return.
			return;
		}
		
		int32_t endif = ScriptParser::getUniqueLabelID();
		int32_t elseif = ScriptParser::getUniqueLabelID();
		literalVisit(host.declaration.get(), param);
		
		//The condition should be reading the value just processed from the initializer
		visit(host.condition.get(), param);
		//
		addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(1)));
		if(host.isInverted())
			addOpcode(new OGotoFalseImmediate(new LabelArgument(elseif)));
		else
			addOpcode(new OGotoTrueImmediate(new LabelArgument(elseif)));
		
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
		visit(host.elseStatement.get(), param);
		
		next = new ONoOp();
		next->setLabel(endif);
		addOpcode(next);
	}
	else
	{
		if(std::optional<int32_t> val = host.condition->getCompileTimeValue(this, scope))
		{
			if((host.isInverted()) == (*val==0)) //True, so go straight to the 'then'
			{
				visit(host.thenStatement.get(), param);
			}
			else //False, so go straight to the 'else'
			{
				visit(host.elseStatement.get(), param);
			}
			//Either way, ignore the rest and return.
			return;
		}
		//run the test
		int32_t startRefCount = arrayRefs.size(); //Store ref count
		literalVisit(host.condition.get(), param);
		//Deallocate string/array literals from within the condition
		deallocateRefsUntilCount(startRefCount);
		while ((int32_t)arrayRefs.size() > startRefCount)
			arrayRefs.pop_back();
		//Continue
		int32_t elseif = ScriptParser::getUniqueLabelID();
		int32_t endif = ScriptParser::getUniqueLabelID();
		addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
		if(host.isInverted())
			addOpcode(new OGotoFalseImmediate(new LabelArgument(elseif)));
		else
			addOpcode(new OGotoTrueImmediate(new LabelArgument(elseif)));
		//run if blocl
		visit(host.thenStatement.get(), param);
		addOpcode(new OGotoImmediate(new LabelArgument(endif)));
		Opcode *next = new ONoOp();
		next->setLabel(elseif);
		addOpcode(next);
		visit(host.elseStatement.get(), param);
		next = new ONoOp();
		next->setLabel(endif);
		addOpcode(next);
	}
}

void BuildOpcodes::caseStmtSwitch(ASTStmtSwitch &host, void* param)
{
	if(host.isString)
	{
		caseStmtStrSwitch(host, param);
		return;
	}
	
	map<ASTSwitchCases*, int32_t> labels;
	vector<ASTSwitchCases*> cases = host.cases.data();
	
	int32_t end_label = ScriptParser::getUniqueLabelID();
	auto default_label = end_label;
	
	// save and override break label.
	breaklabelids.push_back(end_label);
	breakRefCounts.push_back(arrayRefs.size());
	
	// Evaluate the key.
	std::optional<int32_t> keyval = host.key->getCompileTimeValue(NULL, scope);
	if(!keyval)
	{
		int32_t startRefCount = arrayRefs.size(); //Store ref count
		literalVisit(host.key.get(), param);
		//Deallocate string/array literals from within the key
		deallocateRefsUntilCount(startRefCount);
		while ((int32_t)arrayRefs.size() > startRefCount)
			arrayRefs.pop_back();
	}
	
	if(cases.size() == 1 && cases.back()->isDefault) //Only default case
	{
		visit(cases.back()->block.get(), param);
		// Add ending label, for 'break;'
		std::shared_ptr<Opcode> next = std::make_shared<ONoOp>();
		next->setLabel(end_label);
		result.push_back(next);
		return;
	}
	
	//Continue
	
	bool needsEndLabel = true;
	if(keyval)
	{
		ASTSwitchCases* defcase = nullptr;
		ASTSwitchCases* foundcase = nullptr;
		// Add the tests and jumps.
		for (auto it = cases.begin(); it != cases.end(); ++it)
		{
			ASTSwitchCases* cases = *it;
			bool found = false;
			// Run the tests for these cases.
			for (auto it = cases->cases.begin(); !found && it != cases->cases.end(); ++it)
			{
				// Test this individual case.
				if(std::optional<int32_t> val = (*it)->getCompileTimeValue(this, scope))
				{
					if(*val == *keyval)
						found = true;
				}
			}
			for (auto it = cases->ranges.begin(); !found && it != cases->ranges.end(); ++it)
			{
				ASTRange& range = **it;
				//Test each full range
				std::optional<int32_t> low_val = (*range.start).getCompileTimeValue(this, scope);
				std::optional<int32_t> high_val = (*range.end).getCompileTimeValue(this, scope);
				
				if(low_val && high_val && (*low_val <= *keyval && *high_val >= *keyval))
				{
					found = true;
				}
			}

			// If this set includes the default case, mark it.
			if (cases->isDefault)
				defcase = cases;
			if(found)
			{
				foundcase = cases;
				break;
			}
		}
		if(!foundcase) foundcase = defcase;
		
		if(foundcase)
			visit(foundcase->block.get(), param);
		else needsEndLabel = false;
	}
	else
	{
		addOpcode2(result, new OSetRegister(new VarArgument(SWITCHKEY), new VarArgument(EXP1)));

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
				// Test this individual case.
				if(std::optional<int32_t> val = (*it)->getCompileTimeValue(this, scope))
				{
					addOpcode2(result, new OCompareImmediate(new VarArgument(SWITCHKEY), new LiteralArgument(*val)));
				}
				// If the test succeeds, jump to its label.
				addOpcode2(result, new OGotoTrueImmediate(new LabelArgument(label)));
			}
			for (auto it = cases->ranges.begin(); it != cases->ranges.end(); ++it)
			{
				ASTRange& range = **it;
				int32_t skipLabel = ScriptParser::getUniqueLabelID();
				//Test each full range
				if(std::optional<int32_t> val = (*range.start).getCompileTimeValue(this, scope))  //Compare key to lower bound
				{
					addOpcode2(result, new OCompareImmediate(new VarArgument(SWITCHKEY), new LiteralArgument(*val)));
				}
				else //Shouldn't ever happen?
				{
					visit(*range.start, param);
					addOpcode2(result, new OCompareRegister(new VarArgument(SWITCHKEY), new VarArgument(EXP1)));
				}
				addOpcode2(result, new OSetMore(new VarArgument(EXP1))); //Set if key is IN the bound
				addOpcode2(result, new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0))); //Compare if key is OUT of the bound
				addOpcode2(result, new OGotoTrueImmediate(new LabelArgument(skipLabel))); //Skip if key is OUT of the bound
				
				if(std::optional<int32_t> val = (*range.end).getCompileTimeValue(this, scope))  //Compare key to upper bound
				{
					addOpcode2(result, new OCompareImmediate(new VarArgument(SWITCHKEY), new LiteralArgument(*val)));
				}
				else //Shouldn't ever happen?
				{
					visit(*range.end, param);
					addOpcode2(result, new OCompareRegister(new VarArgument(SWITCHKEY), new VarArgument(EXP1)));
				}
				addOpcode2(result, new OSetLess(new VarArgument(EXP1)	)); //Set if key is IN the bound
				addOpcode2(result, new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0))); //Compare if key is OUT of the bound
				addOpcode2(result, new OGotoFalseImmediate(new LabelArgument(label))); //If key is in bounds, jump to its label
				Opcode *end = new ONoOp(); //Just here so the skip label can be placed
				end->setLabel(skipLabel);
				addOpcode2(result, end); //add the skip label
			}

			// If this set includes the default case, mark it.
			if (cases->isDefault)
				default_label = label;
		}

		// Add direct jump to default case (or end if there isn't one.).
		addOpcode2(result, new OGotoImmediate(new LabelArgument(default_label)));

		// Add the actual code branches.
		for (auto it = cases.begin(); it != cases.end(); ++it)
		{
			ASTSwitchCases* cases = *it;

			// Mark start of the block we're adding.
			int32_t block_start_index = result.size();
			// Make a nop for starting the block.
			addOpcode2(result, new ONoOp());
			result[block_start_index]->setLabel(labels[cases]);
			// Add block.
			visit(cases->block.get(), param);
		}
	}
	
	if(needsEndLabel)
	{
		// Add ending label.
		Opcode *next = new ONoOp();
		next->setLabel(end_label);
		addOpcode2(result, next);
	}
	// Restore break label.
	breaklabelids.pop_back();
	breakRefCounts.pop_back();
}

void BuildOpcodes::caseStmtStrSwitch(ASTStmtSwitch &host, void* param)
{
	map<ASTSwitchCases*, int32_t> labels;
	vector<ASTSwitchCases*> cases = host.cases.data();
	
	int32_t end_label = ScriptParser::getUniqueLabelID();;
	int32_t default_label = end_label;
	
	// save and override break label.
	breaklabelids.push_back(end_label);
	breakRefCounts.push_back(arrayRefs.size());
	
	// Evaluate the key.
	int32_t startRefCount = arrayRefs.size(); //Store ref count
	literalVisit(host.key.get(), param);
	//Deallocate string/array literals from within the key
	deallocateRefsUntilCount(startRefCount);
	while ((int32_t)arrayRefs.size() > startRefCount)
		arrayRefs.pop_back();
	
	if(cases.size() == 1 && cases.back()->isDefault) //Only default case
	{
		visit(cases.back()->block.get(), param);
		// Add ending label, for 'break;'
		Opcode *next = new ONoOp();
		next->setLabel(end_label);
		addOpcode2(result, next);
		return;
	}
	
	//Continue
	addOpcode2(result, new OSetRegister(new VarArgument(SWITCHKEY), new VarArgument(EXP1)));
	
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
				addOpcode2(result, new OInternalInsensitiveStringCompare(new VarArgument(SWITCHKEY), new VarArgument(EXP1)));
			else
				addOpcode2(result, new OInternalStringCompare(new VarArgument(SWITCHKEY), new VarArgument(EXP1)));
			
			INITC_DEALLOC(); //deallocate the literal
			//
			addOpcode2(result, new OGotoTrueImmediate(new LabelArgument(label)));
		}

		// If this set includes the default case, mark it.
		if (cases->isDefault)
			default_label = label;
	}
	
	// Add direct jump to default case (or end if there isn't one.).
	addOpcode2(result, new OGotoImmediate(new LabelArgument(default_label)));
	
	// Add the actual code branches.
	for (auto it = cases.begin(); it != cases.end(); ++it)
	{
		ASTSwitchCases* cases = *it;

		// Mark start of the block we're adding.
		int32_t block_start_index = result.size();
		// Make a nop for starting the block.
		addOpcode2(result, new ONoOp());
		result[block_start_index]->setLabel(labels[cases]);
		// Add block.
		visit(cases->block.get(), param);
	}
	
	// Add ending label.
	Opcode *next = new ONoOp();
	next->setLabel(end_label);
	addOpcode2(result, next);
	
	// Restore break label.
	breaklabelids.pop_back();
	breakRefCounts.pop_back();
}

void BuildOpcodes::caseStmtFor(ASTStmtFor &host, void *param)
{
	if(!host.getScope())
	{
		host.setScope(scope->makeChild());
	}
	scope = host.getScope();
	//run the precondition
	int32_t setupRefCount = arrayRefs.size(); //Store ref count
	literalVisit(host.setup.get(), param);
	//Deallocate string/array literals from within the setup
	deallocateRefsUntilCount(setupRefCount);
	while ((int32_t)arrayRefs.size() > setupRefCount)
		arrayRefs.pop_back();
	//Check for a constant FALSE condition
	if(std::optional<int32_t> val = host.test->getCompileTimeValue(this, scope))
	{
		if(*val == 0) //False, so run else, restore scope, and exit
		{
			scope = scope->getParent();
			if(host.hasElse())
				visit(host.elseBlock.get(), param);
			return;
		}
	}
	//Continue
	int32_t loopstart = ScriptParser::getUniqueLabelID();
	int32_t loopend = ScriptParser::getUniqueLabelID();
	int32_t loopincr = ScriptParser::getUniqueLabelID();
	int32_t elselabel = host.hasElse() ? ScriptParser::getUniqueLabelID() : loopend;
	
	Opcode *next = new ONoOp();
	next->setLabel(loopstart);
	addOpcode(next);
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
	//run the loop body
	//save the old break and continue values

	breaklabelids.push_back(loopend);
	breakRefCounts.push_back(arrayRefs.size());
	continuelabelids.push_back(loopincr);
	continueRefCounts.push_back(arrayRefs.size());

	visit(host.body.get(), param);

	breaklabelids.pop_back();
	continuelabelids.pop_back();
	breakRefCounts.pop_back();
	continueRefCounts.pop_back();

	//run the increment
	next = new ONoOp();
	next->setLabel(loopincr);
	addOpcode(next);
	int32_t incRefCount = arrayRefs.size(); //Store ref count
	literalVisit(host.increment.get(), param);
	//Deallocate string/array literals from within the increment
	deallocateRefsUntilCount(incRefCount);
	while ((int32_t)arrayRefs.size() > incRefCount)
		arrayRefs.pop_back();
	//Continue
	addOpcode(new OGotoImmediate(new LabelArgument(loopstart)));
	
	scope = scope->getParent();
	
	if(host.hasElse())
	{
		next = new ONoOp();
		next->setLabel(elselabel);
		addOpcode(next);
		visit(host.elseBlock.get(), param);
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
	
	//Declare the local variable that will hold the array ptr
	literalVisit(host.arrdecl.get(), param);
	//Declare the local variable that will hold the iterator
	literalVisit(host.indxdecl.get(), param);
	//Declare the local variable that will hold the current loop value
	literalVisit(host.decl.get(), param);
	
	int32_t decloffset = 10000L * *getStackOffset(*host.decl.get()->manager);
	int32_t arrdecloffset = 10000L * *getStackOffset(*host.arrdecl.get()->manager);
	int32_t indxdecloffset = 10000L * *getStackOffset(*host.indxdecl.get()->manager);
	
	int32_t loopstart = ScriptParser::getUniqueLabelID();
	int32_t loopend = ScriptParser::getUniqueLabelID();
	int32_t elselabel = host.hasElse() ? ScriptParser::getUniqueLabelID() : loopend;
	
	Opcode* next = new ONoOp();
	next->setLabel(loopstart);
	addOpcode(next);
	
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
	
	//Reaching here, we have a valid index! Load it.
	addOpcode(new OReadPODArrayR(new VarArgument(EXP1), new VarArgument(EXP2)));
	//... and store it in the local variable.
	addOpcode(new OStoreDirect(new VarArgument(EXP1), new LiteralArgument(decloffset)));
	//Now increment the iterator for the next loop
	addOpcode(new OAddImmediate(new VarArgument(EXP2), new LiteralArgument(10000)));
	addOpcode(new OStoreDirect(new VarArgument(EXP2), new LiteralArgument(indxdecloffset)));
	
	//...and run the inside of the loop.
	breaklabelids.push_back(loopend);
	breakRefCounts.push_back(arrayRefs.size());
	continuelabelids.push_back(loopstart);
	continueRefCounts.push_back(arrayRefs.size());
	
	visit(host.body.get(), param);
	
	breaklabelids.pop_back();
	continuelabelids.pop_back();
	breakRefCounts.pop_back();
	continueRefCounts.pop_back();
	
	//Return to top of loop
	addOpcode(new OGotoImmediate(new LabelArgument(loopstart)));
	
	scope = scope->getParent();
	
	if(host.hasElse())
	{
		next = new ONoOp();
		next->setLabel(elselabel);
		addOpcode(next);
		visit(host.elseBlock.get(), param);
	}
	
	next = new ONoOp();
	next->setLabel(loopend);
	addOpcode(next);
}

void BuildOpcodes::caseStmtWhile(ASTStmtWhile &host, void *param)
{
	std::optional<int32_t> val = host.test->getCompileTimeValue(this, scope);
	if(val && (host.isInverted() != !*val)) //never runs, handle else only
	{
		if(host.hasElse())
			visit(host.elseBlock.get(), param);
		return;
	}
	
	int32_t startlabel = ScriptParser::getUniqueLabelID();
	int32_t endlabel = ScriptParser::getUniqueLabelID();
	int32_t elselabel = host.hasElse() ? ScriptParser::getUniqueLabelID() : endlabel;
	//run the test
	Opcode *next = new ONoOp();
	next->setLabel(startlabel);
	addOpcode(next);
	if(!val)
	{
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
		}
		else
		{
			addOpcode(new OGotoTrueImmediate(new LabelArgument(elselabel)));
		}
	}

	breaklabelids.push_back(endlabel);
	breakRefCounts.push_back(arrayRefs.size());
	continuelabelids.push_back(startlabel);
	continueRefCounts.push_back(arrayRefs.size());

	visit(host.body.get(), param);

	breaklabelids.pop_back();
	continuelabelids.pop_back();
	breakRefCounts.pop_back();
	continueRefCounts.pop_back();

	addOpcode(new OGotoImmediate(new LabelArgument(startlabel)));
	
	if(host.hasElse())
	{
		next = new ONoOp();
		next->setLabel(elselabel);
		addOpcode(next);
		visit(host.elseBlock.get(), param);
	}
	next = new ONoOp();
	next->setLabel(endlabel);
	addOpcode(next);
}

void BuildOpcodes::caseStmtDo(ASTStmtDo &host, void *param)
{
	std::optional<int32_t> val = host.test->getCompileTimeValue(this, scope);
	bool truthyval = val && host.isInverted() == (*val==0);
	bool deadloop = val && !truthyval;
	int32_t startlabel;
	int32_t endlabel = ScriptParser::getUniqueLabelID();
	int32_t elselabel = host.hasElse() ? ScriptParser::getUniqueLabelID() : endlabel;
	int32_t continuelabel = ScriptParser::getUniqueLabelID();
	Opcode* next;
	if(!deadloop)
	{
		startlabel = ScriptParser::getUniqueLabelID();
		//nop to label start
		next = new ONoOp();
		next->setLabel(startlabel);
		addOpcode(next);
	}
	
	breaklabelids.push_back(endlabel);
	breakRefCounts.push_back(arrayRefs.size());
	continuelabelids.push_back(continuelabel);
	continueRefCounts.push_back(arrayRefs.size());

	visit(host.body.get(), param);

	breaklabelids.pop_back();
	continuelabelids.pop_back();
	breakRefCounts.pop_back();
	continueRefCounts.pop_back();
	
	next = new ONoOp();
	next->setLabel(continuelabel);
	addOpcode(next);
	
	if(val)
	{
		if(truthyval) //infinite loop
			addOpcode(new OGotoImmediate(new LabelArgument(startlabel)));
		else if(host.hasElse())
			visit(host.elseBlock.get(), param);
	}
	else
	{
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
		}
		else
		{
			addOpcode(new OGotoTrueImmediate(new LabelArgument(elselabel)));
		}
		addOpcode(new OGotoImmediate(new LabelArgument(startlabel)));
		
		next = new ONoOp();
		next->setLabel(elselabel);
		addOpcode(next);
		visit(host.elseBlock.get(), param);
	}
	
	next = new ONoOp();
	next->setLabel(endlabel);
	addOpcode(next);
}

void BuildOpcodes::caseStmtReturn(ASTStmtReturn&, void*)
{
	deallocateRefsUntilCount(0);
	addOpcode(new OGotoImmediate(new LabelArgument(returnlabelid)));
}

void BuildOpcodes::caseStmtReturnVal(ASTStmtReturnVal &host, void *param)
{
	INITC_STORE();
	INITC_VISIT(host.value.get());
	INITC_INIT();
	INITC_DEALLOC();
	deallocateRefsUntilCount(0);
	addOpcode(new OGotoImmediate(new LabelArgument(returnlabelid)));
}

void BuildOpcodes::caseStmtBreak(ASTStmtBreak &host, void *)
{
	if(!host.breakCount) return;
	if (breaklabelids.size() < host.breakCount)
	{
		handleError(CompileError::BreakBad(&host,host.breakCount));
		return;
	}
	int32_t refcount = breakRefCounts.at(breakRefCounts.size()-host.breakCount);
	int32_t breaklabel = breaklabelids.at(breaklabelids.size()-host.breakCount);
	deallocateRefsUntilCount(refcount);
	addOpcode(new OGotoImmediate(new LabelArgument(breaklabel)));
}

void BuildOpcodes::caseStmtContinue(ASTStmtContinue &host, void *)
{
	if(!host.contCount) return;
	if (continuelabelids.size() < host.contCount)
	{
		handleError(CompileError::ContinueBad(&host,host.contCount));
		return;
	}

	int32_t refcount = continueRefCounts.at(continueRefCounts.size()-host.contCount);
	int32_t contlabel = continuelabelids.at(continuelabelids.size()-host.contCount);
	deallocateRefsUntilCount(refcount);
	addOpcode(new OGotoImmediate(new LabelArgument(contlabel)));
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
	int32_t oldreturnlabelid = returnlabelid;
	int32_t oldReturnRefCount = returnRefCount;
	returnlabelid = ScriptParser::getUniqueLabelID();
	returnRefCount = arrayRefs.size();

	visit(host.block.get(), param);
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
	if (std::optional<int32_t> globalId = manager.getGlobalId())
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
	if (std::optional<int32_t> size = host.extraArrays[0]->getCompileTimeSize(this, scope))
		totalSize = *size * 10000L;
	else
	{
		handleError(
				CompileError::ExprNotConstant(host.extraArrays[0]));
		return;
	}

	// Allocate the array.
	if (std::optional<int32_t> globalId = manager.getGlobalId())
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
	LValBOHelper helper(scope);
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
	if (std::optional<int32_t> value = host.binding->getCompileTimeValue(scope->isGlobal() || scope->isScript()))
	{
		addOpcode(new OSetImmediate(new VarArgument(EXP1),
									new LiteralArgument(*value)));
		host.markConstant();
		return;
	}

	int32_t vid = host.binding->id;

	if (std::optional<int32_t> globalId = host.binding->getGlobalId())
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
	
	if(readfunc->getFlag(FUNCFLAG_INLINE))
	{
		if (!(readfunc->internal_flags & IFUNCFLAG_SKIPPOINTER))
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
		addOpcode(new OPushRegister(new VarArgument(SFRAME)));
		int32_t returnlabel = ScriptParser::getUniqueLabelID();
		//push the return address
		addOpcode(new OPushImmediate(new LabelArgument(returnlabel)));
		if (!(readfunc->internal_flags & IFUNCFLAG_SKIPPOINTER))
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
		addOpcode(new OGotoImmediate(new LabelArgument(label)));
		//pop the stack frame
		Opcode *next = new OPopRegister(new VarArgument(SFRAME));
		next->setLabel(returnlabel);
		addOpcode(next);
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
	std::optional<int32_t> arrVal = host.array->getCompileTimeValue(this,scope);
	std::optional<int32_t> indxVal = host.index->getCompileTimeValue(this,scope);
	
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
	INITC_STORE();
	auto& func = *host.binding;
	bool classfunc = func.getFlag(FUNCFLAG_CLASSFUNC) && !func.getFlag(FUNCFLAG_STATIC);
	
	auto* optarg = opcodeTargets.back();
	int32_t startRefCount = arrayRefs.size(); //Store ref count
	if(func.prototype) //Prototype function
	{
		//Visit each parameter, in case there are side-effects; but don't push the results, as they are unneeded.
		for (auto it = host.parameters.begin();
			it != host.parameters.end(); ++it)
		{
			INITC_VISIT(*it);
		}
		
		//Set the return to the default value
		if(classfunc && func.getFlag(FUNCFLAG_CONSTRUCTOR) && parsing_user_class <= puc_vars)
		{
			ClassScope* cscope = func.internalScope->getClass();
			UserClass& user_class = cscope->user_class;
			vector<Function*> destr = cscope->getDestructor();
			Function* destructor = destr.size() == 1 ? destr.at(0) : nullptr;
			if(destructor && !destructor->prototype)
			{
				Function* destructor = destr[0];
				addOpcode(new OSetImmediate(new VarArgument(EXP1),
					new LabelArgument(destructor->getLabel())));
			}
			else addOpcode(new OSetImmediate(new VarArgument(EXP1),
				new LiteralArgument(0)));
			addOpcode(new OConstructClass(new VarArgument(EXP1),
				new VectorArgument(user_class.members)));
		}
		else
		{
			DataType const& retType = *func.returnType;
			if(retType != DataType::ZVOID)
			{
				if (std::optional<int32_t> val = func.defaultReturn->getCompileTimeValue(NULL, scope))
				{
					addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*val)));
				}
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
			if (!(func.internal_flags & IFUNCFLAG_SKIPPOINTER))
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
			if(std::optional<int32_t> val = arg->getCompileTimeValue(this, scope))
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
				if(std::optional<int32_t> val = arg->getCompileTimeValue(this, scope))
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
		for(;it != funcCode.end(); ++it)
		{
			addOpcode((*it)->makeClone(false));
		}
	
		if(host.left->isTypeArrow())
		{
			ASTExprArrow* arr = static_cast<ASTExprArrow*>(host.left.get());
			if(arr->left->getWriteType(scope, this) && !arr->left->isConstant())
			{
				if(func.internal_flags & IFUNCFLAG_REASSIGNPTR)
				{
					bool isVoid = func.returnType->isVoid();
					if(!isVoid) addOpcode(new OPushRegister(new VarArgument(EXP1)));
					addOpcode(new OSetRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
					LValBOHelper helper(scope);
					helper.parsing_user_class = parsing_user_class;
					arr->left->execute(helper, INITC_CTXT);
					addOpcodes(helper.getResult());
					if(!isVoid) addOpcode(new OPopRegister(new VarArgument(EXP1)));
				}
			}
			else if(func.internal_flags & IFUNCFLAG_REASSIGNPTR) //This is likely a mistake in the script... give the user a warning.
			{
				handleError(CompileError::BadReassignCall(&host, func.getSignature().asString()));
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
				if(std::optional<int32_t> val = arg->getCompileTimeValue(this, scope))
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
			addOpcode(new OPushRegister(new VarArgument(EXP1)));
			++pushcount;
		}
		
		//push the this key/stack frame pointer
		addOpcode(new OPushRegister(new VarArgument(CLASS_THISKEY)));
		addOpcode(new OPushRegister(new VarArgument(SFRAME)));
		//push the return address
		int32_t returnaddr = ScriptParser::getUniqueLabelID();
		addOpcode(new OPushImmediate(new LabelArgument(returnaddr)));
		pushcount += 3;

		//push the parameters, in forward order
		size_t param_indx = 0;
		for (; param_indx < num_used_params-vargcount; ++param_indx)
		{
			auto& arg = host.parameters.at(param_indx);
			//Compile-time constants can be optimized slightly...
			if(std::optional<int32_t> val = arg->getCompileTimeValue(this, scope))
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
		//goto
		if(parsing_user_class == puc_construct && func.getFlag(FUNCFLAG_CONSTRUCTOR)
			&& !host.isConstructor())
		{
			//A constructor calling another constructor to inherit it's code
			//Use the alt label of the constructor, which is after the constructy bits
			addOpcode(new OSetRegister(new VarArgument(CLASS_THISKEY2), new VarArgument(CLASS_THISKEY)));
			addOpcode(new OGotoImmediate(new LabelArgument(func.getAltLabel())));
		}
		else addOpcode(new OGotoImmediate(new LabelArgument(funclabel)));
		//pop the stack frame pointer
		Opcode *next = new OPopRegister(new VarArgument(SFRAME));
		next->setLabel(returnaddr);
		addOpcode(next);
		addOpcode(new OPopRegister(new VarArgument(CLASS_THISKEY)));
		if(vargs)
		{
			addOpcode(new OPopRegister(new VarArgument(EXP2)));
			addOpcode(new ODeallocateMemRegister(new VarArgument(EXP2)));
		}
	}
	else
	{
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
				if(std::optional<int32_t> val = arg->getCompileTimeValue(this, scope))
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
			addOpcode(new OPushRegister(new VarArgument(EXP1)));
			++pushcount;
		}
		
		//push the stack frame pointer
		addOpcode(new OPushRegister(new VarArgument(SFRAME)));
		//push the return address
		int32_t returnaddr = ScriptParser::getUniqueLabelID();
		addOpcode(new OPushImmediate(new LabelArgument(returnaddr)));
		pushcount += 2;
		
		// If the function is a pointer function (->func()) we need to push the
		// left-hand-side.
		if (host.left->isTypeArrow() && !(func.internal_flags & IFUNCFLAG_SKIPPOINTER))
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
			if(std::optional<int32_t> val = arg->getCompileTimeValue(this, scope))
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
				addOpcode(new OPushRegister(new VarArgument(EXP1)));
			}
			else
			{
				//push the vargs, in forward order
				for (; param_indx < host.parameters.size(); ++param_indx)
				{
					auto& arg = host.parameters.at(param_indx);
					//Compile-time constants can be optimized slightly...
					if(std::optional<int32_t> val = arg->getCompileTimeValue(this, scope))
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
		//goto
		addOpcode(new OGotoImmediate(new LabelArgument(funclabel)));
		//pop the stack frame pointer
		Opcode *next = new OPopRegister(new VarArgument(SFRAME));
		next->setLabel(returnaddr);
		addOpcode(next);
		if(user_vargs)
		{
			addOpcode(new OPopRegister(new VarArgument(EXP2)));
			addOpcode(new ODeallocateMemRegister(new VarArgument(EXP2)));
		}
		
		if(host.left->isTypeArrow())
		{
			ASTExprArrow* arr = static_cast<ASTExprArrow*>(host.left.get());
			if(arr->left->getWriteType(scope, this) && !arr->left->isConstant())
			{
				if(func.internal_flags & IFUNCFLAG_REASSIGNPTR)
				{
					bool isVoid = func.returnType->isVoid();
					if(!isVoid) addOpcode(new OPushRegister(new VarArgument(EXP1)));
					addOpcode(new OSetRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
					LValBOHelper helper(scope);
					helper.parsing_user_class = parsing_user_class;
					arr->left->execute(helper, INITC_CTXT);
					addOpcodes(helper.getResult());
					if(!isVoid) addOpcode(new OPopRegister(new VarArgument(EXP1)));
				}
			}
			else if(func.internal_flags & IFUNCFLAG_REASSIGNPTR) //This is likely a mistake in the script... give the user a warning.
			{
				handleError(CompileError::BadReassignCall(&host, func.getSignature().asString()));
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
	if (std::optional<int32_t> val = host.getCompileTimeValue(NULL, scope))
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
	if (host.getCompileTimeValue(NULL, scope))
	{
		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this, scope))));
		return;
	}

	visit(host.operand.get(), param);
	addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
	if(*lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL))
		addOpcode(new OSetTrue(new VarArgument(EXP1)));
	else
		addOpcode(new OSetTrueI(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprBitNot(ASTExprBitNot& host, void* param)
{
	if (host.getCompileTimeValue(NULL, scope))
	{
		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this, scope))));
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
	LValBOHelper helper(scope);
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
	LValBOHelper helper(scope);
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
	LValBOHelper helper(scope);
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
	LValBOHelper helper(scope);
	helper.parsing_user_class = parsing_user_class;
	host.operand->execute(helper, param);
	addOpcodes(helper.getResult());

	// Pop EXP1.
	addOpcode(new OPopRegister(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprAnd(ASTExprAnd& host, void* param)
{
	if (host.getCompileTimeValue(NULL, scope))
	{
		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this, scope))));
		return;
	}
	bool decret = *lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL)!=0;
	bool short_circuit = *lookupOption(*scope, CompileOption::OPT_SHORT_CIRCUIT) != 0;
	if(std::optional<int32_t> val = host.left->getCompileTimeValue(NULL, scope))
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
	else if(std::optional<int32_t> val = host.right->getCompileTimeValue(NULL, scope))
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
	if (host.getCompileTimeValue(NULL, scope))
	{
		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this, scope))));
		return;
	}
	bool decret = *lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL)!=0;
	bool short_circuit = *lookupOption(*scope, CompileOption::OPT_SHORT_CIRCUIT) != 0;
	if(std::optional<int32_t> val = host.left->getCompileTimeValue(NULL, scope))
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
	else if(std::optional<int32_t> val = host.right->getCompileTimeValue(NULL, scope))
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
	if (host.getCompileTimeValue(NULL, scope))
	{
		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this, scope))));
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
	if (host.getCompileTimeValue(NULL, scope))
	{
		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this, scope))));
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
	if (host.getCompileTimeValue(NULL, scope))
	{
		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this, scope))));
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
	if (host.getCompileTimeValue(NULL, scope))
	{
		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this, scope))));
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
	if (host.getCompileTimeValue(NULL, scope))
	{
		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this, scope))));
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
	if (host.getCompileTimeValue(NULL, scope))
	{
		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this, scope))));
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
	if (host.getCompileTimeValue(NULL, scope))
	{
		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this, scope))));
		return;
	}

	std::optional <int32_t> lval = host.left->getCompileTimeValue(NULL, scope);
	std::optional <int32_t> rval = host.right->getCompileTimeValue(NULL, scope);
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
	if (host.getCompileTimeValue(NULL, scope))
	{
		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this, scope))));
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
	if (host.getCompileTimeValue(NULL, scope))
	{
		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this, scope))));
		return;
	}
	std::optional <int32_t> lval = host.left->getCompileTimeValue(NULL, scope);
	std::optional <int32_t> rval = host.right->getCompileTimeValue(NULL, scope);
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
	if (host.getCompileTimeValue(NULL, scope))
	{
		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this, scope))));
		return;
	}
	std::optional <int32_t> lval = host.left->getCompileTimeValue(NULL, scope);
	std::optional <int32_t> rval = host.right->getCompileTimeValue(NULL, scope);
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
	if (host.getCompileTimeValue(NULL, scope))
	{
		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this, scope))));
		return;
	}
	std::optional <int32_t> lval = host.left->getCompileTimeValue(NULL, scope);
	std::optional <int32_t> rval = host.right->getCompileTimeValue(NULL, scope);
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
	if (host.getCompileTimeValue(NULL, scope))
	{
		CONST_VAL(*host.getCompileTimeValue(this, scope));
		return;
	}
	bool do_long = host.left.get()->isLong(scope, this) || host.right.get()->isLong(scope, this);
	std::optional <int32_t> lval = host.left->getCompileTimeValue(NULL, scope);
	std::optional <int32_t> rval = host.right->getCompileTimeValue(NULL, scope);
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
	if (host.getCompileTimeValue(NULL, scope))
	{
		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this, scope))));
		return;
	}
	
	std::optional <int32_t> lval = host.left->getCompileTimeValue(NULL, scope);
	std::optional <int32_t> rval = host.right->getCompileTimeValue(NULL, scope);
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
	if (host.getCompileTimeValue(NULL, scope))
	{
		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this, scope))));
		return;
	}
	
	std::optional <int32_t> lval = host.left->getCompileTimeValue(NULL, scope);
	std::optional <int32_t> rval = host.right->getCompileTimeValue(NULL, scope);
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
	if (host.getCompileTimeValue(NULL, scope))
	{
		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this, scope))));
		return;
	}
	bool do_long = *lookupOption(*scope, CompileOption::OPT_BINARY_32BIT)
		|| host.left.get()->isLong(scope, this) || host.right.get()->isLong(scope, this);
	
	std::optional <int32_t> lval = host.left->getCompileTimeValue(NULL, scope);
	std::optional <int32_t> rval = host.right->getCompileTimeValue(NULL, scope);
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
	if (host.getCompileTimeValue(NULL, scope))
	{
		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this, scope))));
		return;
	}
	bool do_long = *lookupOption(*scope, CompileOption::OPT_BINARY_32BIT)
		|| host.left.get()->isLong(scope, this) || host.right.get()->isLong(scope, this);
	
	std::optional <int32_t> lval = host.left->getCompileTimeValue(NULL, scope);
	std::optional <int32_t> rval = host.right->getCompileTimeValue(NULL, scope);
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
	if (host.getCompileTimeValue(NULL, scope))
	{
		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this, scope))));
		return;
	}
	bool do_long = *lookupOption(*scope, CompileOption::OPT_BINARY_32BIT)
		|| host.left.get()->isLong(scope, this) || host.right.get()->isLong(scope, this);
	
	std::optional <int32_t> lval = host.left->getCompileTimeValue(NULL, scope);
	std::optional <int32_t> rval = host.right->getCompileTimeValue(NULL, scope);
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
	if (host.getCompileTimeValue(NULL, scope))
	{
		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this, scope))));
		return;
	}
	bool do_long = *lookupOption(*scope, CompileOption::OPT_BINARY_32BIT)
		|| host.left.get()->isLong(scope, this);
	
	std::optional <int32_t> lval = host.left->getCompileTimeValue(NULL, scope);
	std::optional <int32_t> rval = host.right->getCompileTimeValue(NULL, scope);
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
	if (host.getCompileTimeValue(NULL, scope))
	{
		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this, scope))));
		return;
	}
	bool do_long = *lookupOption(*scope, CompileOption::OPT_BINARY_32BIT)
		|| host.left.get()->isLong(scope, this);
	
	std::optional <int32_t> lval = host.left->getCompileTimeValue(NULL, scope);
	std::optional <int32_t> rval = host.right->getCompileTimeValue(NULL, scope);
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
	if (host.getCompileTimeValue(NULL, scope))
	{
		addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this, scope))));
		return;
	}
	std::optional <int32_t> lval = host.left->getCompileTimeValue(NULL, scope);
	if(lval)
	{
		if(*lval)
			visit(host.middle.get());
		else visit(host.right.get());
	}
	else
	{
		std::optional <int32_t> mval = host.middle->getCompileTimeValue(NULL, scope);
		std::optional <int32_t> rval = host.right->getCompileTimeValue(NULL, scope);
		
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
	if (std::optional<int32_t> cval = host.getCompileTimeValue(this, scope))
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
	if (std::optional<int32_t> cval = host.getCompileTimeValue(this, scope))
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
	addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this, scope))));
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
		if (std::optional<int32_t> totalSize = extraArray.getCompileTimeSize(this, scope))
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
	if (std::optional<int32_t> globalId = manager.getGlobalId())
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
		if (std::optional<int32_t> s = host.size->getCompileTimeValue(this, scope))
			size = *s / 10000L;
	// From the declaration?
	if (size == -1 && declaration.extraArrays.size() == 1)
	{
		ASTDataDeclExtraArray& extraArray = *declaration.extraArrays[0];
		if (std::optional<int32_t> totalSize = extraArray.getCompileTimeSize(this, scope))
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
	if (std::optional<int32_t> globalId = manager.getGlobalId())
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
		if (std::optional<int32_t> val = (*it)->getCompileTimeValue(this, scope))
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
		if (std::optional<int32_t> s = host.size->getCompileTimeValue(this, scope))
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
		if (std::optional<int32_t> val = (*it)->getCompileTimeValue(this, scope))
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
	std::optional <int32_t> lval = left->getCompileTimeValue(NULL, scope);
	std::optional <int32_t> rval = right->getCompileTimeValue(NULL, scope);
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
	std::optional <int32_t> lval = left->getCompileTimeValue(NULL, scope);
	std::optional <int32_t> rval = right->getCompileTimeValue(NULL, scope);
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

	if (std::optional<int32_t> globalId = host.binding->getGlobalId())
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
		BuildOpcodes oc(scope);
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
	
	if(host.writeFunction->getFlag(FUNCFLAG_INLINE))
	{
		if (!(host.writeFunction->internal_flags & IFUNCFLAG_SKIPPOINTER))
		{
			//Push rval
			addOpcode(new OPushRegister(new VarArgument(EXP1)));
			//Get lval
			BuildOpcodes oc(scope);
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
			BuildOpcodes oc2(scope);
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

		// Push the stack frame.
		addOpcode(new OPushRegister(new VarArgument(SFRAME)));

		int32_t returnlabel = ScriptParser::getUniqueLabelID();
		//push the return address
		addOpcode(new OPushImmediate(new LabelArgument(returnlabel)));
		
		if (!(host.writeFunction->internal_flags & IFUNCFLAG_SKIPPOINTER))
		{
			//Push rval
			addOpcode(new OPushRegister(new VarArgument(EXP1)));
			//Get lval
			BuildOpcodes oc(scope);
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
			BuildOpcodes oc2(scope);
			oc2.parsing_user_class = parsing_user_class;
			oc2.visit(host.index.get(), param);
			addOpcodes(oc2.getResult());
			addOpcode(new OPushRegister(new VarArgument(EXP1)));
		}
		
		//finally, goto!
		int32_t label = host.writeFunction->getLabel();
		addOpcode(new OGotoImmediate(new LabelArgument(label)));

		// Pop the stack frame
		Opcode* next = new OPopRegister(new VarArgument(SFRAME));
		next->setLabel(returnlabel);
		addOpcode(next);
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
	BuildOpcodes bo(scope);
	bo.parsing_user_class = parsing_user_class;
	std::optional<int32_t> arrVal = host.array->getCompileTimeValue(&bo, scope);
	std::optional<int32_t> indxVal = host.index->getCompileTimeValue(&bo, scope);
	if(!arrVal || !indxVal)
	{
		// Push the value.
		addOpcode(new OPushRegister(new VarArgument(EXP1)));
	}
	
	if(!arrVal)
	{
		// Get and push the array pointer.
		BuildOpcodes buildOpcodes1(scope);
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
		BuildOpcodes buildOpcodes2(scope);
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

