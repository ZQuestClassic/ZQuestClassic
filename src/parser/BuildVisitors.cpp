
#include "../precompiled.h" //always first

#include "BuildVisitors.h"
#include "CompileError.h"
#include "ZScript.h"
#include <assert.h>

using namespace ZScript;

/////////////////////////////////////////////////////////////////////////////////
// BuildOpcodes

BuildOpcodes::BuildOpcodes()
	: returnlabelid(-1), continuelabelid(-1), breaklabelid(-1), 
	  returnRefCount(0), continueRefCount(0), breakRefCount(0)
{
	opcodeTargets.push_back(&result);
}

void BuildOpcodes::visit(AST& node, void* param)
{
	RecursiveVisitor::visit(node, param);
	for (list<ASTExpr*>::const_iterator it = node.compileErrorCatches.begin();
		 it != node.compileErrorCatches.end(); ++it)
	{
		ASTExpr& idNode = **it;
		optional<long> errorId = idNode.getCompileTimeValue();
		assert(errorId);
		handleError(CompileError::MissingCompileError, &node,
					int(*errorId / 10000L));
	}
}

void BuildOpcodes::caseDefault(AST&, void*)
{
    // Unreachable
    assert(false);
}

void BuildOpcodes::addOpcode(Opcode* code)
{
	opcodeTargets.back()->push_back(code);
}

template <class Container>
void BuildOpcodes::addOpcodes(Container const& container)
{
	for (typename Container::const_iterator it = container.begin();
		 it != container.end(); ++it)
		addOpcode(*it);
}

void BuildOpcodes::deallocateArrayRef(long arrayRef)
{
	addOpcode(new OSetRegister(new VarArgument(SFTEMP), new VarArgument(SFRAME)));
	addOpcode(new OAddImmediate(new VarArgument(SFTEMP), new LiteralArgument(arrayRef)));
	addOpcode(new OLoadIndirect(new VarArgument(EXP2), new VarArgument(SFTEMP)));
	addOpcode(new ODeallocateMemRegister(new VarArgument(EXP2)));
}

void BuildOpcodes::deallocateRefsUntilCount(int count)
{
	count = arrayRefs.size() - count;
    for (list<long>::reverse_iterator it = arrayRefs.rbegin();
		 it != arrayRefs.rend() && count > 0;
		 it++, count--)
	{
		deallocateArrayRef(*it);
	}
}

// Statements

void BuildOpcodes::caseBlock(ASTBlock &host, void *param)
{
	OpcodeContext *c = (OpcodeContext *)param;

	int startRefCount = arrayRefs.size();

    for (vector<ASTStmt*>::iterator it = host.statements.begin();
		 it != host.statements.end(); ++it)
	{
		int initIndex = result.size();
        visit(*it, param);
		result.insert(result.begin() + initIndex, c->initCode.begin(), c->initCode.end());
		c->initCode.clear();
	}

	deallocateRefsUntilCount(startRefCount);
	while ((int)arrayRefs.size() > startRefCount)
		arrayRefs.pop_back();
}

void BuildOpcodes::caseStmtIf(ASTStmtIf &host, void *param)
{
    //run the test
    visit(host.condition, param);
    int endif = ScriptParser::getUniqueLabelID();
    addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
    addOpcode(new OGotoTrueImmediate(new LabelArgument(endif)));
    //run the block
    visit(host.thenStatement, param);
    //nop
    Opcode *next = new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0));
    next->setLabel(endif);
    addOpcode(next);
}

void BuildOpcodes::caseStmtIfElse(ASTStmtIfElse &host, void *param)
{
    //run the test
    visit(host.condition, param);
    int elseif = ScriptParser::getUniqueLabelID();
    int endif = ScriptParser::getUniqueLabelID();
    addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
    addOpcode(new OGotoTrueImmediate(new LabelArgument(elseif)));
    //run if blocl
    visit(host.thenStatement, param);
    addOpcode(new OGotoImmediate(new LabelArgument(endif)));
    Opcode *next = new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(0));
    next->setLabel(elseif);
    addOpcode(next);
    visit(host.elseStatement, param);
    next = new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(0));
    next->setLabel(endif);
    addOpcode(next);
}

void BuildOpcodes::caseStmtSwitch(ASTStmtSwitch &host, void* param)
{
	map<ASTSwitchCases*, int> labels;
	vector<ASTSwitchCases*>& cases = host.cases;

	int end_label = ScriptParser::getUniqueLabelID();;
	int default_label = end_label;

	// save and override break label.
	int old_break_label = breaklabelid;
	int oldBreakRefCount = breakRefCount;
	breaklabelid = end_label;
	breakRefCount = arrayRefs.size();

	// Evaluate the key.
	ASTExpr* key = host.key;
	visit(key, param);
	result.push_back(new OSetRegister(new VarArgument(EXP2), new VarArgument(EXP1)));

	// Add the tests and jumps.
	for (vector<ASTSwitchCases*>::iterator it = cases.begin(); it != cases.end(); ++it)
	{
		ASTSwitchCases* cases = *it;

		// Make the target label.
		int label = ScriptParser::getUniqueLabelID();
		labels[cases] = label;

		// Run the tests for these cases.
		for (vector<ASTExprConst*>::iterator it = cases->cases.begin();
			 it != cases->cases.end();
			 ++it)
		{
			// Test this individual case.
			result.push_back(new OPushRegister(new VarArgument(EXP2)));
			visit(*it, param);
			result.push_back(new OPopRegister(new VarArgument(EXP2)));
			// If the test succeeds, jump to its label.
			result.push_back(new OCompareRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
			result.push_back(new OGotoTrueImmediate(new LabelArgument(label)));
		}

		// If this set includes the default case, mark it.
		if (cases->isDefault)
			default_label = label;
	}

	// Add direct jump to default case (or end if there isn't one.).
	result.push_back(new OGotoImmediate(new LabelArgument(default_label)));

	// Add the actual code branches.
	for (vector<ASTSwitchCases*>::iterator it = cases.begin(); it != cases.end(); ++it)
	{
		ASTSwitchCases* cases = *it;

		// Mark start of the block we're adding.
		int block_start_index = result.size();
		// Add block.
		visit(cases->block, param);
		// If nothing was added, put in a nop to point to.
		if (result.size() == block_start_index)
			result.push_back(new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(0)));
		// Set label to start of block.
		result[block_start_index]->setLabel(labels[cases]);
	}

	// Add ending label.
    Opcode *next = new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(0));
    next->setLabel(end_label);
	result.push_back(next);

	// Restore break label.
	breaklabelid = old_break_label;
	breakRefCount = oldBreakRefCount;
}

void BuildOpcodes::caseStmtFor(ASTStmtFor &host, void *param)
{
    //run the precondition
    visit(host.setup, param);
    int loopstart = ScriptParser::getUniqueLabelID();
    int loopend = ScriptParser::getUniqueLabelID();
    int loopincr = ScriptParser::getUniqueLabelID();
    //nop
    Opcode *next = new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0));
    next->setLabel(loopstart);
    addOpcode(next);
    //test the termination condition
    visit(host.test, param);
    addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
    addOpcode(new OGotoTrueImmediate(new LabelArgument(loopend)));
    //run the loop body
    //save the old break and continue values

    int oldbreak = breaklabelid;
	int oldBreakRefCount = breakRefCount;
    breaklabelid = loopend;
	breakRefCount = arrayRefs.size();
    int oldcontinue = continuelabelid;
	int oldContinueRefCount = continueRefCount;
    continuelabelid = loopincr;
	continueRefCount = arrayRefs.size();

    visit(host.body, param);

    breaklabelid = oldbreak;
    breakRefCount = oldBreakRefCount;
    continuelabelid = oldcontinue;
	continueRefCount = oldContinueRefCount;

    //run the increment
    //nop
    next = new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(0));
    next->setLabel(loopincr);
    addOpcode(next);
    visit(host.increment, param);
    addOpcode(new OGotoImmediate(new LabelArgument(loopstart)));
    //nop
    next = new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(0));
    next->setLabel(loopend);
    addOpcode(next);
}

void BuildOpcodes::caseStmtWhile(ASTStmtWhile &host, void *param)
{
    int startlabel = ScriptParser::getUniqueLabelID();
    int endlabel = ScriptParser::getUniqueLabelID();
    //run the test
    //nop to label start
    Opcode *start = new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0));
    start->setLabel(startlabel);
    addOpcode(start);
    visit(host.test, param);
    addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
    addOpcode(new OGotoTrueImmediate(new LabelArgument(endlabel)));

    int oldbreak = breaklabelid;
	int oldBreakRefCount = breakRefCount;
    breaklabelid = endlabel;
	breakRefCount = arrayRefs.size();
    int oldcontinue = continuelabelid;
	int oldContinueRefCount = continueRefCount;
    continuelabelid = startlabel;
	continueRefCount = arrayRefs.size();

    visit(host.body, param);

    breaklabelid = oldbreak;
	breakRefCount = oldBreakRefCount;
    continuelabelid = oldcontinue;
	continueRefCount = oldContinueRefCount;

    addOpcode(new OGotoImmediate(new LabelArgument(startlabel)));
    //nop to end while
    Opcode *end = new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0));
    end->setLabel(endlabel);
    addOpcode(end);
}

void BuildOpcodes::caseStmtDo(ASTStmtDo &host, void *param)
{
    int startlabel = ScriptParser::getUniqueLabelID();
    int endlabel = ScriptParser::getUniqueLabelID();
    int continuelabel = ScriptParser::getUniqueLabelID();
    //nop to label start
    Opcode *start = new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0));
    start->setLabel(startlabel);
    addOpcode(start);

    int oldbreak = breaklabelid;
	int oldBreakRefCount = breakRefCount;
    breaklabelid = endlabel;
	breakRefCount = arrayRefs.size();
    int oldcontinue = continuelabelid;
	int oldContinueRefCount = continueRefCount;
    continuelabelid = continuelabel;
	continueRefCount = arrayRefs.size();

    visit(host.body, param);

    breaklabelid = oldbreak;
    continuelabelid = oldcontinue;
    breakRefCount = oldBreakRefCount;
	continueRefCount = oldContinueRefCount;

    start = new OSetImmediate(new VarArgument(NUL), new LiteralArgument(0));
    start->setLabel(continuelabel);
    addOpcode(start);
    visit(host.test, param);
    addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
    addOpcode(new OGotoTrueImmediate(new LabelArgument(endlabel)));
    addOpcode(new OGotoImmediate(new LabelArgument(startlabel)));
    //nop to end dowhile
    Opcode *end = new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0));
    end->setLabel(endlabel);
    addOpcode(end);
}

void BuildOpcodes::caseStmtReturn(ASTStmtReturn&, void*)
{
	deallocateRefsUntilCount(0);
    addOpcode(new OGotoImmediate(new LabelArgument(returnlabelid)));
}

void BuildOpcodes::caseStmtReturnVal(ASTStmtReturnVal &host, void *param)
{
    visit(host.value, param);
	deallocateRefsUntilCount(0);
    addOpcode(new OGotoImmediate(new LabelArgument(returnlabelid)));
}

void BuildOpcodes::caseStmtBreak(ASTStmtBreak &host, void *)
{
    if (breaklabelid == -1)
    {
        handleError(CompileError::BreakBad, &host);
        return;
    }

	deallocateRefsUntilCount(breakRefCount);
    addOpcode(new OGotoImmediate(new LabelArgument(breaklabelid)));
}

void BuildOpcodes::caseStmtContinue(ASTStmtContinue &host, void *)
{
    if (continuelabelid == -1)
    {
        handleError(CompileError::ContinueBad, &host);
        return;
    }

	deallocateRefsUntilCount(continueRefCount);
    addOpcode(new OGotoImmediate(new LabelArgument(continuelabelid)));
}

void BuildOpcodes::caseStmtEmpty(ASTStmtEmpty &, void *)
{
    // empty
}

// Declarations

void BuildOpcodes::caseFuncDecl(ASTFuncDecl &host, void *param)
{
	int oldreturnlabelid = returnlabelid;
	int oldReturnRefCount = returnRefCount;
    returnlabelid = ScriptParser::getUniqueLabelID();
	returnRefCount = arrayRefs.size();

    visit(host.block, param);
}

void BuildOpcodes::caseDataDecl(ASTDataDecl& host, void* param)
{
    OpcodeContext& context = *(OpcodeContext*)param;
	Datum& manager = *host.manager;

	// Ignore inlined values.
	if (manager.getCompileTimeValue()) return;

	// Switch off to the proper helper function.
	if (manager.type.typeClassId() == ZVARTYPE_CLASSID_ARRAY)
	{
		if (host.initializer()) buildArrayInit(host, context);
		else buildArrayUninit(host, context);
	}
	else buildVariable(host, context);
}

void BuildOpcodes::buildVariable(ASTDataDecl& host, OpcodeContext& context)
{
	Datum& manager = *host.manager;

	// Load initializer into EXP1, if present.
	visit(host.initializer(), &context);

	// Set variable to EXP1 or 0, depending on the initializer.
	if (optional<int> globalId = manager.getGlobalId())
	{
		if (host.initializer())
			addOpcode(new OSetRegister(new GlobalArgument(*globalId),
			                           new VarArgument(EXP1)));
		else
			addOpcode(new OSetImmediate(new GlobalArgument(*globalId),
			                            new LiteralArgument(0)));
	}
	else
	{
		int offset = context.stackframe->getOffset(manager.id);
		addOpcode(new OSetRegister(new VarArgument(SFTEMP), new VarArgument(SFRAME)));
		addOpcode(new OAddImmediate(new VarArgument(SFTEMP), new LiteralArgument(offset)));
		if (!host.initializer())
			addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
		addOpcode(new OStoreIndirect(new VarArgument(EXP1), new VarArgument(SFTEMP)));
	}
}

void BuildOpcodes::buildArrayInit(ASTDataDecl& host, OpcodeContext& context)
{
	Datum& manager = *host.manager;

	// Initializer creates the array and loads the array id into EXP1.
	visit(host.initializer(), &context);

	// Set variable to EXP1.
	if (optional<int> globalId = manager.getGlobalId())
	{
		addOpcode(new OSetRegister(new GlobalArgument(*globalId),
		                           new VarArgument(EXP1)));
	}
	else
	{
		int offset = context.stackframe->getOffset(manager.id);
		addOpcode(new OSetRegister(new VarArgument(SFTEMP),
		                           new VarArgument(SFRAME)));
		addOpcode(new OAddImmediate(new VarArgument(SFTEMP),
		                            new LiteralArgument(offset)));
		addOpcode(new OStoreIndirect(new VarArgument(EXP1),
		                             new VarArgument(SFTEMP)));
	}
}

void BuildOpcodes::buildArrayUninit(
		ASTDataDecl& host, OpcodeContext& context)
{
	Datum& manager = *host.manager;

	// Right now, don't support nested arrays.
	if (host.extraArrays.size() != 1)
	{
		handleError(CompileError::DimensionMismatch, &host);
		return;
	}

	// Get size of the array.
	long totalSize;
	if (optional<int> size = host.extraArrays[0]->getCompileTimeSize(this))
		totalSize = *size * 10000L;
	else
	{
		handleError(CompileError::ExprNotConstant, host.extraArrays[0]);
		return;
	}

	// Allocate the array.
	if (optional<int> globalId = manager.getGlobalId())
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
		int offset = context.stackframe->getOffset(manager.id);
		addOpcode(new OSetRegister(new VarArgument(SFTEMP), new VarArgument(SFRAME)));
		addOpcode(new OAddImmediate(new VarArgument(SFTEMP), new LiteralArgument(offset)));
		addOpcode(new OStoreIndirect(new VarArgument(EXP1), new VarArgument(SFTEMP)));
		// Register for cleanup.
		arrayRefs.push_back(offset);
	}
}

void BuildOpcodes::caseTypeDef(ASTTypeDef&, void*) {}

// Expressions

void BuildOpcodes::caseExprAssign(ASTExprAssign &host, void *param)
{
    //load the rval into EXP1
    visit(host.right, param);
    //and store it
    LValBOHelper helper;
    host.left->execute(helper, param);
	addOpcodes(helper.getResult());
}

void BuildOpcodes::caseExprIdentifier(ASTExprIdentifier& host, void* param)
{
    OpcodeContext* c = (OpcodeContext*)param;

	// If a constant, just load its value.
    if (optional<long> value = host.binding->getCompileTimeValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1),
                                    new LiteralArgument(*value)));
		host.markConstant();
        return;
    }

    int vid = host.binding->id;

    if (optional<int> globalId = host.binding->getGlobalId())
    {
        // Global variable, so just get its value.
        addOpcode(new OSetRegister(new VarArgument(EXP1),
                                   new GlobalArgument(*globalId)));
        return;
    }

    // Local variable, get its value from the stack.
    int offset = c->stackframe->getOffset(vid);
    addOpcode(new OSetRegister(new VarArgument(SFTEMP), new VarArgument(SFRAME)));
    addOpcode(new OAddImmediate(new VarArgument(SFTEMP), new LiteralArgument(offset)));
    addOpcode(new OLoadIndirect(new VarArgument(EXP1), new VarArgument(SFTEMP)));
}

void BuildOpcodes::caseExprArrow(ASTExprArrow& host, void* param)
{
    OpcodeContext* c = (OpcodeContext*)param;
    bool isIndexed = host.index != NULL;
    //this is actually a function call
    //to the appropriate gettor method
    //so, set that up:
    //push the stack frame
    addOpcode(new OPushRegister(new VarArgument(SFRAME)));
    int returnlabel = ScriptParser::getUniqueLabelID();
    //push the return address
    addOpcode(new OSetImmediate(new VarArgument(EXP1), new LabelArgument(returnlabel)));
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    //get the rhs of the arrow
    visit(host.left, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));

    //if indexed, push the index
    if(isIndexed)
    {
        visit(host.index, param);
        addOpcode(new OPushRegister(new VarArgument(EXP1)));
    }

    //call the function
    int label = host.readFunction->getLabel();
    addOpcode(new OGotoImmediate(new LabelArgument(label)));
    //pop the stack frame
    Opcode *next = new OPopRegister(new VarArgument(SFRAME));
    next->setLabel(returnlabel);
    addOpcode(next);
}

void BuildOpcodes::caseExprIndex(ASTExprIndex& host, void* param)
{
	// If the left hand side is an arrow, then we'll let it run instead.
	if (host.array->isTypeArrow())
	{
		caseExprArrow(static_cast<ASTExprArrow&>(*host.array), param);
		return;
	}

	// First, push the array.
	visit(host.array, param);
	addOpcode(new OPushRegister(new VarArgument(EXP1)));

	// Load the index into INDEX2.
	visit(host.index, param);
	addOpcode(new OSetRegister(new VarArgument(INDEX2), new VarArgument(EXP1)));

	// Pop array into INDEX.
	addOpcode(new OPopRegister(new VarArgument(INDEX)));

	// Return GLOBALRAM to indicate an array access.
	//   (As far as I can tell, there's no difference between GLOBALRAM and
	//    SCRIPTRAM, so I'll use GLOBALRAM here instead of checking.)
	addOpcode(new OSetRegister(new VarArgument(EXP1), new VarArgument(GLOBALRAM)));
}

void BuildOpcodes::caseExprCall(ASTExprCall& host, void* param)
{
    OpcodeContext* c = (OpcodeContext*)param;
    int funclabel = host.binding->getLabel();
    //push the stack frame pointer
    addOpcode(new OPushRegister(new VarArgument(SFRAME)));
    //push the return address
    int returnaddr = ScriptParser::getUniqueLabelID();
    addOpcode(new OSetImmediate(new VarArgument(EXP1), new LabelArgument(returnaddr)));
    addOpcode(new OPushRegister(new VarArgument(EXP1)));

    // If the function is a pointer function (->func()) we need to push the
    // left-hand-side.
    if (host.left->isTypeArrow())
    {
        //load the value of the left-hand of the arrow into EXP1
        visit(static_cast<ASTExprArrow&>(*host.left).left, param);
        //visit(host.getLeft(), param);
        //push it onto the stack
        addOpcode(new OPushRegister(new VarArgument(EXP1)));
    }

    //push the parameters, in forward order
    for (vector<ASTExpr*>::iterator it = host.parameters.begin();
		it != host.parameters.end(); ++it)
    {
        visit(*it, param);
        addOpcode(new OPushRegister(new VarArgument(EXP1)));
    }

    //goto
    addOpcode(new OGotoImmediate(new LabelArgument(funclabel)));
    //pop the stack frame pointer
    Opcode *next = new OPopRegister(new VarArgument(SFRAME));
    next->setLabel(returnaddr);
    addOpcode(next);
}

void BuildOpcodes::caseExprNegate(ASTExprNegate& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this))));
        return;
    }

    visit(host.operand, param);
    addOpcode(new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(0)));
    addOpcode(new OSubRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
    addOpcode(new OSetRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprNot(ASTExprNot& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this))));
        return;
    }

    visit(host.operand, param);
    addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
    addOpcode(new OSetTrue(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprBitNot(ASTExprBitNot& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this))));
        return;
    }

    visit(host.operand, param);
    addOpcode(new ONot(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprIncrement(ASTExprIncrement& host, void* param)
{
    OpcodeContext* c = (OpcodeContext*)param;

    // Load value of the variable into EXP1 and push.
	visit(host.operand, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));

    // Increment EXP1
    addOpcode(new OAddImmediate(new VarArgument(EXP1),
								new LiteralArgument(10000)));
	
    // Store it
    LValBOHelper helper;
    host.operand->execute(helper, param);
    addOpcodes(helper.getResult());
	
    // Pop EXP1
    addOpcode(new OPopRegister(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprPreIncrement(ASTExprPreIncrement& host, void* param)
{
    OpcodeContext* c = (OpcodeContext*)param;

    // Load value of the variable into EXP1.
	visit(host.operand, param);

    // Increment EXP1
    addOpcode(new OAddImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));

    // Store it
    LValBOHelper helper;
    host.operand->execute(helper, param);
	addOpcodes(helper.getResult());
}

void BuildOpcodes::caseExprPreDecrement(ASTExprPreDecrement& host, void* param)
{
    OpcodeContext* c = (OpcodeContext*)param;

    // Load value of the variable into EXP1.
	visit(host.operand, param);

    // Decrement EXP1.
    addOpcode(new OSubImmediate(new VarArgument(EXP1),
								new LiteralArgument(10000)));

    // Store it.
    LValBOHelper helper;
    host.operand->execute(helper, param);
	addOpcodes(helper.getResult());
}

void BuildOpcodes::caseExprDecrement(ASTExprDecrement& host, void* param)
{
    OpcodeContext* c = (OpcodeContext*)param;

    // Load value of the variable into EXP1 and push.
	visit(host.operand, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));

    // Decrement EXP1.
    addOpcode(new OSubImmediate(new VarArgument(EXP1),
								new LiteralArgument(10000)));
    // Store it.
    LValBOHelper helper;
    host.operand->execute(helper, param);
	addOpcodes(helper.getResult());

    // Pop EXP1.
    addOpcode(new OPopRegister(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprAnd(ASTExprAnd& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this))));
        return;
    }

    //compute both sides
    visit(host.left, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    visit(host.right, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    castFromBool(result, EXP1);
    castFromBool(result, EXP2);
    addOpcode(new OAddRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
    addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(2)));
    addOpcode(new OSetMore(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprOr(ASTExprOr& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this))));
        return;
    }

    //compute both sides
    visit(host.left, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    visit(host.right, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    addOpcode(new OAddRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
    addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(1)));
    addOpcode(new OSetMore(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprGT(ASTExprGT& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this))));
        return;
    }

    //compute both sides
    visit(host.left, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    visit(host.right, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    addOpcode(new OCompareRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
    addOpcode(new OSetLess(new VarArgument(EXP1)));
    addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
    addOpcode(new OSetTrue(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprGE(ASTExprGE& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this))));
        return;
    }

    //compute both sides
    visit(host.left, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    visit(host.right, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    addOpcode(new OCompareRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
    addOpcode(new OSetMore(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprLT(ASTExprLT& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this))));
        return;
    }

    //compute both sides
    visit(host.left, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    visit(host.right, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    addOpcode(new OCompareRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
    addOpcode(new OSetMore(new VarArgument(EXP1)));
    addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
    addOpcode(new OSetTrue(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprLE(ASTExprLE& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this))));
        return;
    }

    // Compute both sides.
    visit(host.left, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    visit(host.right, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    addOpcode(new OCompareRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
    addOpcode(new OSetLess(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprEQ(ASTExprEQ& host, void* param)
{
    // Special case for booleans.
    bool isBoolean = (*host.left->getReadType() == ZVarType::BOOL);

    if (host.getCompileTimeValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this))));
        return;
    }

    // Compute both sides.
    visit(host.left, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    visit(host.right, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));

    if (isBoolean)
    {
        castFromBool(result, EXP1);
        castFromBool(result, EXP2);
    }

    addOpcode(new OCompareRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
    addOpcode(new OSetTrue(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprNE(ASTExprNE& host, void* param)
{
    // Special case for booleans.
    bool isBoolean = (*host.left->getReadType() == ZVarType::BOOL);

    if (host.getCompileTimeValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this))));
        return;
    }

    // Compute both sides.
    visit(host.left, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    visit(host.right, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));

    if (isBoolean)
    {
        castFromBool(result, EXP1);
        castFromBool(result, EXP2);
    }

    addOpcode(new OCompareRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
    addOpcode(new OSetFalse(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprPlus(ASTExprPlus& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this))));
        return;
    }

    // Compute both sides.
    visit(host.left, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    visit(host.right, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    addOpcode(new OAddRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprMinus(ASTExprMinus& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this))));
        return;
    }

    // Compute both sides.
    visit(host.left, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    visit(host.right, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    addOpcode(new OSubRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
    addOpcode(new OSetRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprTimes(ASTExprTimes& host, void *param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this))));
        return;
    }

    // Compute both sides.
    visit(host.left, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    visit(host.right, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    addOpcode(new OMultRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprDivide(ASTExprDivide& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this))));
        return;
    }

    // Compute both sides.
    visit(host.left, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    visit(host.right, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    addOpcode(new ODivRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
    addOpcode(new OSetRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprModulo(ASTExprModulo& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this))));
        return;
    }

    // Compute both sides.
    visit(host.left, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    visit(host.right, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    addOpcode(new OModuloRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
    addOpcode(new OSetRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprBitAnd(ASTExprBitAnd& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this))));
        return;
    }

    // Compute both sides.
    visit(host.left, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    visit(host.right, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    addOpcode(new OAndRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprBitOr(ASTExprBitOr& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this))));
        return;
    }

    // Compute both sides.
    visit(host.left, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    visit(host.right, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    addOpcode(new OOrRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprBitXor(ASTExprBitXor& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this))));
        return;
    }

    // Compute both sides.
    visit(host.left, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    visit(host.right, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    addOpcode(new OXorRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprLShift(ASTExprLShift& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this))));
        return;
    }

    // Compute both sides.
    visit(host.left, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    visit(host.right, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    addOpcode(new OLShiftRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
    addOpcode(new OSetRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprRShift(ASTExprRShift& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this))));
        return;
    }

    // Compute both sides.
    visit(host.left, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    visit(host.right, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    addOpcode(new ORShiftRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
    addOpcode(new OSetRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

// Literals

void BuildOpcodes::caseNumberLiteral(ASTNumberLiteral& host, void*)
{
    if (host.getCompileTimeValue())
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this))));
    else
    {
        pair<long, bool> val = ScriptParser::parseLong(host.value->parseValue());

        if (!val.second)
            handleError(CompileError::ConstTrunc, &host,
						host.value->value);

        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(val.first)));
    }
}

void BuildOpcodes::caseBoolLiteral(ASTBoolLiteral& host, void*)
{
    addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this))));
}

// TODO add implicit stackframe to global case for literals.
void BuildOpcodes::caseStringLiteral(ASTStringLiteral& host, void* param)
{
	OpcodeContext* c = (OpcodeContext*)param;
	int id = c->symbols->getNodeId(&host);
	bool isGlobal = !c->stackframe;
    int RAMtype = isGlobal ? SCRIPTRAM: GLOBALRAM;

	////////////////////////////////////////////////////////////////
	// Initialization Code.

	string data = host.value;
	long size = (data.size() + 1) * 10000L;

	// If this is part of an array declaration, grab the size info from it.
	if (host.declaration)
	{
		ASTDataDeclExtraArray& extraArray = *host.declaration->extraArrays[0];
		if (optional<int> totalSize = extraArray.getCompileTimeSize(this))
			size = *totalSize * 10000L;
		else if (extraArray.hasSize())
		{
			handleError(CompileError::ExprNotConstant, &host);
			return;
		}
	}

	// Allocate.
	if (isGlobal)
	{
		c->initCode.push_back(
				new OAllocateGlobalMemImmediate(new VarArgument(EXP1),
				                                new LiteralArgument(size)));
		c->initCode.push_back(
				new OSetRegister(new GlobalArgument(0),
				                 new VarArgument(EXP1)));
	}
	else
	{
		c->initCode.push_back(
				new OAllocateMemImmediate(new VarArgument(EXP1),
				                          new LiteralArgument(size)));
		int offset = c->stackframe->getOffset(id);
		c->initCode.push_back(new OSetRegister(new VarArgument(SFTEMP),
		                                       new VarArgument(SFRAME)));
		c->initCode.push_back(
				new OAddImmediate(new VarArgument(SFTEMP),
				                  new LiteralArgument(offset)));
		c->initCode.push_back(new OStoreIndirect(new VarArgument(EXP1),
		                                         new VarArgument(SFTEMP)));
	}

	// Initialize.
	c->initCode.push_back(new OSetRegister(new VarArgument(INDEX), new VarArgument(EXP1)));
	for (int i = 0; i < (int)data.size(); ++i)
	{
		c->initCode.push_back(new OSetImmediate(new VarArgument(INDEX2), new LiteralArgument(i * 10000L)));
		long value = data[i] * 10000L;
		c->initCode.push_back(new OSetImmediate(new VarArgument(RAMtype), new LiteralArgument(value)));
	}
	c->initCode.push_back(new OSetImmediate(new VarArgument(INDEX2), new LiteralArgument(data.size() * 10000L)));
	c->initCode.push_back(new OSetImmediate(new VarArgument(RAMtype), new LiteralArgument(0)));

	////////////////////////////////////////////////////////////////
	// Actual Code.

	if (isGlobal)
	{
        // Global variable, so just get its value.
        addOpcode(new OSetRegister(new VarArgument(EXP1),
                                   new GlobalArgument(0)));
	}
	else
	{
		// Local variable, get its value from the stack.
		int offset = c->stackframe->getOffset(id);
		addOpcode(new OSetRegister(new VarArgument(SFTEMP), new VarArgument(SFRAME)));
		addOpcode(new OAddImmediate(new VarArgument(SFTEMP), new LiteralArgument(offset)));
		addOpcode(new OLoadIndirect(new VarArgument(EXP1), new VarArgument(SFTEMP)));
	}

	////////////////////////////////////////////////////////////////
	// Register for cleanup.

	if (!isGlobal)
	{
		int offset = c->stackframe->getOffset(id);
		arrayRefs.push_back(offset);
	}
}

void BuildOpcodes::caseArrayLiteral(ASTArrayLiteral& host, void* param)
{
	OpcodeContext& context = *(OpcodeContext*)param;
	Literal& manager = *host.manager;
	bool isGlobal = !context.stackframe;
    int RAMtype = isGlobal ? SCRIPTRAM: GLOBALRAM;

	int size = -1;

	// If this is part of an array declaration, grab the size info from it.
	if (host.declaration)
	{
		ASTDataDeclExtraArray& extraArray = *host.declaration->extraArrays[0];
		if (optional<int> totalSize = extraArray.getCompileTimeSize(this))
			size = *totalSize;
		else if (extraArray.hasSize())
		{
			handleError(CompileError::ExprNotConstant, &host);
			return;
		}
	}

	// If there's an explicit size, grab it.
	else if (host.size)
	{
		if (optional<long> s = host.size->getCompileTimeValue(this))
			size = *s / 10000L;
		else
		{
			handleError(CompileError::ExprNotConstant, host.size);
			return;
		}
	}

	// Otherwise, grab the number of elements.
	if (size == -1) size = host.elements.size();

	// Make sure the chosen size has enough space.
	if (size < int(host.elements.size()))
	{
		handleError(CompileError::ArrayListTooLarge, &host);
		return;
	}

	////////////////////////////////////////////////////////////////
	// Initialization Code.

	// Allocate.
	if (isGlobal)
	{
		context.initCode.push_back(
				new OAllocateGlobalMemImmediate(
						new VarArgument(EXP1),
						new LiteralArgument(size * 10000L)));
		context.initCode.push_back(new OSetRegister(new GlobalArgument(0),
		                                            new VarArgument(EXP1)));
	}
	else
	{
		context.initCode.push_back(new OAllocateMemImmediate(new VarArgument(EXP1), new LiteralArgument(size * 10000L)));
		int offset = context.stackframe->getOffset(manager.id);
		context.initCode.push_back(new OSetRegister(new VarArgument(SFTEMP), new VarArgument(SFRAME)));
		context.initCode.push_back(new OAddImmediate(new VarArgument(SFTEMP), new LiteralArgument(offset)));
		context.initCode.push_back(new OStoreIndirect(new VarArgument(EXP1), new VarArgument(SFTEMP)));
	}

	// Initialize.
	context.initCode.push_back(new OSetRegister(new VarArgument(INDEX), new VarArgument(EXP1)));
	long i = 0;
	vector<ASTExpr*> elements = host.elements;
	for (vector<ASTExpr*>::iterator it = elements.begin();
		 it != elements.end(); ++it, i += 10000L)
	{
		context.initCode.push_back(new OPushRegister(new VarArgument(INDEX)));
		opcodeTargets.push_back(&context.initCode);
		visit(*it, param);
		opcodeTargets.pop_back();
		context.initCode.push_back(new OPopRegister(new VarArgument(INDEX)));
		context.initCode.push_back(new OSetImmediate(new VarArgument(INDEX2), new LiteralArgument(i)));
		context.initCode.push_back(new OSetRegister(new VarArgument(RAMtype), new VarArgument(EXP1)));
	}

	////////////////////////////////////////////////////////////////
	// Actual Code.

	if (isGlobal)
	{
        // Global variable, so just get its value.
        addOpcode(new OSetRegister(new VarArgument(EXP1), new GlobalArgument(0)));
	}
	else
	{
		// Local variable, get its value from the stack.
		int offset = context.stackframe->getOffset(manager.id);
		addOpcode(new OSetRegister(new VarArgument(SFTEMP), new VarArgument(SFRAME)));
		addOpcode(new OAddImmediate(new VarArgument(SFTEMP), new LiteralArgument(offset)));
		addOpcode(new OLoadIndirect(new VarArgument(EXP1), new VarArgument(SFTEMP)));
	}

	////////////////////////////////////////////////////////////////
	// Register for cleanup.

	if (!isGlobal)
	{
		int offset = context.stackframe->getOffset(manager.id);
		arrayRefs.push_back(offset);
	}

}

// Other

void BuildOpcodes::castFromBool(vector<Opcode*>& res, int reg)
{
    res.push_back(new OCompareImmediate(new VarArgument(reg), new LiteralArgument(0)));
    res.push_back(new OSetFalse(new VarArgument(reg)));
}

/////////////////////////////////////////////////////////////////////////////////
// LValBOHelper

void LValBOHelper::caseDefault(void *)
{
    //Shouldn't happen
    assert(false);
}

void LValBOHelper::addOpcode(Opcode* code)
{
	result.push_back(code);
}

template <class Container>
void LValBOHelper::addOpcodes(Container const& container)
{
	for (typename Container::const_iterator it = container.begin();
		 it != container.end(); ++it)
		addOpcode(*it);
}

/*
void LValBOHelper::caseDataDecl(ASTDataDecl& host, void* param)
{
    // Cannot be a global variable, so just stuff it in the stack
    OpcodeContext* c = (OpcodeContext*)param;
    int vid = host.manager->id;
    int offset = c->stackframe->getOffset(vid);
    addOpcode(new OSetRegister(new VarArgument(SFTEMP), new VarArgument(SFRAME)));
    addOpcode(new OAddImmediate(new VarArgument(SFTEMP), new LiteralArgument(offset)));
    addOpcode(new OStoreIndirect(new VarArgument(EXP1), new VarArgument(SFTEMP)));
}
*/

void LValBOHelper::caseExprIdentifier(ASTExprIdentifier& host, void* param)
{
    OpcodeContext* c = (OpcodeContext*)param;
    int vid = c->symbols->getNodeId(&host);

    if (optional<int> globalId = host.binding->getGlobalId())
    {
        // Global variable.
        addOpcode(new OSetRegister(new GlobalArgument(*globalId),
                                   new VarArgument(EXP1)));
        return;
    }

    // Set the stack.
    int offset = c->stackframe->getOffset(vid);

    addOpcode(new OSetRegister(new VarArgument(SFTEMP), new VarArgument(SFRAME)));
    addOpcode(new OAddImmediate(new VarArgument(SFTEMP), new LiteralArgument(offset)));
    addOpcode(new OStoreIndirect(new VarArgument(EXP1), new VarArgument(SFTEMP)));
}

void LValBOHelper::caseExprArrow(ASTExprArrow &host, void *param)
{
    OpcodeContext *c = (OpcodeContext *)param;
    int isIndexed = (host.index != NULL);
    // This is actually implemented as a settor function call.

    // Push the stack frame.
    addOpcode(new OPushRegister(new VarArgument(SFRAME)));

    int returnlabel = ScriptParser::getUniqueLabelID();
    //push the return address
    addOpcode(new OSetImmediate(new VarArgument(EXP2), new LabelArgument(returnlabel)));
    addOpcode(new OPushRegister(new VarArgument(EXP2)));
    //push the lhs of the arrow
    //but first save the value of EXP1
    addOpcode(new OPushRegister(new VarArgument(EXP1)));

    BuildOpcodes oc;
    oc.visit(host.left, param);
	addOpcodes(oc.getResult());
    
    //pop the old value of EXP1
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    //and push the lhs
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    //and push the old value of EXP1
    addOpcode(new OPushRegister(new VarArgument(EXP2)));
    
    //and push the index, if indexed
    if(isIndexed)
    {
        BuildOpcodes oc2;
        oc2.visit(host.index, param);
		addOpcodes(oc2.getResult());
        addOpcode(new OPushRegister(new VarArgument(EXP1)));
    }
    
    //finally, goto!
    int label = host.writeFunction->getLabel();
    addOpcode(new OGotoImmediate(new LabelArgument(label)));

    // Pop the stack frame
    Opcode* next = new OPopRegister(new VarArgument(SFRAME));
    next->setLabel(returnlabel);
    addOpcode(next);
}

void LValBOHelper::caseExprIndex(ASTExprIndex& host, void* param)
{
	// Arrows just fall back on the arrow implementation.
	if (host.array->isTypeArrow())
	{
		caseExprArrow(static_cast<ASTExprArrow&>(*host.array), param);
		return;
	}

	vector<Opcode*> opcodes;

	// Push the value.
    addOpcode(new OPushRegister(new VarArgument(EXP1)));

	// Get and push the array pointer.
	BuildOpcodes buildOpcodes1;
	buildOpcodes1.visit(host.array, param);
	opcodes = buildOpcodes1.getResult();
	for (vector<Opcode*>::iterator it = opcodes.begin(); it != opcodes.end(); ++it)
		addOpcode(*it);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));

	// Get the index.
	BuildOpcodes buildOpcodes2;
	buildOpcodes2.visit(host.index, param);
	opcodes = buildOpcodes2.getResult();
	for (vector<Opcode*>::iterator it = opcodes.begin(); it != opcodes.end(); ++it)
		addOpcode(*it);

	// Setup array indices.
    addOpcode(new OPopRegister(new VarArgument(INDEX)));
    addOpcode(new OSetRegister(new VarArgument(INDEX2), new VarArgument(EXP1)));

	// Pop and assign the value.
	//   (As far as I can tell, there's no difference between GLOBALRAM and
	//    SCRIPTRAM, so I'll use GLOBALRAM here instead of checking.)
    addOpcode(new OPopRegister(new VarArgument(EXP1))); // Pop the value
    addOpcode(new OSetRegister(new VarArgument(GLOBALRAM), new VarArgument(EXP1)));
}

