
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

void BuildOpcodes::caseDefault(AST&, void*)
{
    // Unreachable
    assert(false);
}

void BuildOpcodes::addOpcode(Opcode* code)
{
	opcodeTargets.back()->push_back(code);
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
        (*it)->execute(*this, param);
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
    host.condition->execute(*this,param);
    int endif = ScriptParser::getUniqueLabelID();
    addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
    addOpcode(new OGotoTrueImmediate(new LabelArgument(endif)));
    //run the block
    host.thenStatement->execute(*this,param);
    //nop
    Opcode *next = new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0));
    next->setLabel(endif);
    addOpcode(next);
}

void BuildOpcodes::caseStmtIfElse(ASTStmtIfElse &host, void *param)
{
    //run the test
    host.condition->execute(*this,param);
    int elseif = ScriptParser::getUniqueLabelID();
    int endif = ScriptParser::getUniqueLabelID();
    addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
    addOpcode(new OGotoTrueImmediate(new LabelArgument(elseif)));
    //run if blocl
    host.thenStatement->execute(*this,param);
    addOpcode(new OGotoImmediate(new LabelArgument(endif)));
    Opcode *next = new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(0));
    next->setLabel(elseif);
    addOpcode(next);
    host.elseStatement->execute(*this,param);
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
	key->execute(*this, param);
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
			(*it)->execute(*this, param);
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
		cases->block->execute(*this, param);
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
    host.setup->execute(*this,param);
    int loopstart = ScriptParser::getUniqueLabelID();
    int loopend = ScriptParser::getUniqueLabelID();
    int loopincr = ScriptParser::getUniqueLabelID();
    //nop
    Opcode *next = new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0));
    next->setLabel(loopstart);
    addOpcode(next);
    //test the termination condition
    host.test->execute(*this,param);
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

    host.body->execute(*this,param);

    breaklabelid = oldbreak;
    breakRefCount = oldBreakRefCount;
    continuelabelid = oldcontinue;
	continueRefCount = oldContinueRefCount;

    //run the increment
    //nop
    next = new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(0));
    next->setLabel(loopincr);
    addOpcode(next);
    host.increment->execute(*this,param);
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
    host.test->execute(*this,param);
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

    host.body->execute(*this,param);

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

    host.body->execute(*this, param);

    breaklabelid = oldbreak;
    continuelabelid = oldcontinue;
    breakRefCount = oldBreakRefCount;
	continueRefCount = oldContinueRefCount;

    start = new OSetImmediate(new VarArgument(NUL), new LiteralArgument(0));
    start->setLabel(continuelabel);
    addOpcode(start);
    host.test->execute(*this,param);
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
    host.value->execute(*this, param);
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

    host.block->execute(*this, param);
}

void BuildOpcodes::caseDataDecl(ASTDataDecl& host, void* param)
{
    OpcodeContext& context = *(OpcodeContext*)param;
	Variable& manager = *host.manager;

	// Ignore inlined values.
	if (manager.compileTimeValue) return;

	// Switch off to the proper helper function.
	if (manager.type->typeClassId() == ZVARTYPE_CLASSID_ARRAY)
	{
		if (host.initializer()) buildArrayInit(host, context);
		else buildArrayUninit(host, context);
	}
	else buildVariable(host, context);
}

void BuildOpcodes::buildVariable(ASTDataDecl& host, OpcodeContext& context)
{
	Variable& manager = *host.manager;

	// Load initializer into EXP1, if present.
	AST::execute(host.initializer(), *this, &context);

	// Set variable to EXP1 or 0, depending on the initializer.
	if (manager.global)
	{
		int globalid = context.linktable->getGlobalID(manager.id);
		if (host.initializer())
			addOpcode(new OSetRegister(new GlobalArgument(globalid), new VarArgument(EXP1)));
		else
			addOpcode(new OSetImmediate(new GlobalArgument(globalid), new LiteralArgument(0)));
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
	Variable& manager = *host.manager;

	// Initializer creates the array and loads the array id into EXP1.
	host.initializer()->execute(*this, &context);

	// Set variable to EXP1.
	if (manager.global)
	{
		int globalid = context.linktable->getGlobalID(manager.id);
		addOpcode(new OSetRegister(new GlobalArgument(globalid), new VarArgument(EXP1)));
	}
	else
	{
		int offset = context.stackframe->getOffset(manager.id);
		addOpcode(new OSetRegister(new VarArgument(SFTEMP), new VarArgument(SFRAME)));
		addOpcode(new OAddImmediate(new VarArgument(SFTEMP), new LiteralArgument(offset)));
		addOpcode(new OStoreIndirect(new VarArgument(EXP1), new VarArgument(SFTEMP)));
	}
}

void BuildOpcodes::buildArrayUninit(ASTDataDecl& host, OpcodeContext& context)
{
	Variable& manager = *host.manager;

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
	if (manager.global)
	{
		int globalid = context.linktable->getGlobalID(manager.id);
		addOpcode(new OAllocateGlobalMemImmediate(new VarArgument(EXP1), new LiteralArgument(totalSize)));
		addOpcode(new OSetRegister(new GlobalArgument(globalid), new VarArgument(EXP1)));
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

void BuildOpcodes::caseStmtCompileError(ASTStmtCompileError& host, void*)
{
	// If we haven't been triggered, throw a warning.
	if (!host.errorTriggered)
		CompileError::MissingCompileError.print(&host, host.getErrorId());

	// Otherwise, we don't want to actually generate any code.
}

void BuildOpcodes::caseExprAssign(ASTExprAssign &host, void *param)
{
    //load the rval into EXP1
    host.right->execute(*this, param);
    //and store it
    LValBOHelper helper;
    host.left->execute(helper, param);
    vector<Opcode *> subcode = helper.getResult();

    for(vector<Opcode *>::iterator it = subcode.begin(); it != subcode.end(); it++)
    {
        addOpcode(*it);
    }
}

void BuildOpcodes::caseExprConst(ASTExprConst &host, void *param)
{
	AST::execute(host.content, *this, param);
}

void BuildOpcodes::caseExprIdentifier(ASTExprIdentifier& host, void* param)
{
    OpcodeContext* c = (OpcodeContext*)param;

	// If a constant, just load its value.
    if (c->symbols->isInlinedConstant(&host))
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(c->symbols->getInlinedValue(&host))));
		host.markConstant();
        return;
    }

    int vid = c->symbols->getNodeId(&host);
    int globalid = c->linktable->getGlobalID(vid);

    if (globalid != -1)
    {
        // Global variable, so just get its value.
        addOpcode(new OSetRegister(new VarArgument(EXP1), new GlobalArgument(globalid)));
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
    AST::execute(host.left, *this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));

    //if indexed, push the index
    if(isIndexed)
    {
        AST::execute(host.index, *this, param);
        addOpcode(new OPushRegister(new VarArgument(EXP1)));
    }

    //call the function
    int label = c->linktable->functionToLabel(c->symbols->getNodeId(&host));
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
		host.array->execute(*this, param);
		return;
	}

	// First, push the array.
	AST::execute(host.array, *this, param);
	addOpcode(new OPushRegister(new VarArgument(EXP1)));

	// Load the index into INDEX2.
	AST::execute(host.index, *this, param);
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
    int funclabel = c->linktable->functionToLabel(c->symbols->getNodeId(&host));
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
        ((ASTExprArrow*)host.left)->left->execute(*this, param);
        //host.getLeft()->execute(*this,param);
        //push it onto the stack
        addOpcode(new OPushRegister(new VarArgument(EXP1)));
    }

    //push the parameters, in forward order
    for (vector<ASTExpr*>::iterator it = host.parameters.begin();
		it != host.parameters.end(); ++it)
    {
        (*it)->execute(*this, param);
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

    host.operand->execute(*this, param);
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

    host.operand->execute(*this, param);
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

    host.operand->execute(*this, param);
    addOpcode(new ONot(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprIncrement(ASTExprIncrement& host, void* param)
{
    OpcodeContext* c = (OpcodeContext*)param;

    // Load value of the variable into EXP1.  Except if it is an arrow expr, in
    // which case the gettor function is stored in this AST*.
	ASTExpr& operand = *host.operand;
	if (operand.isTypeArrow() || operand.isTypeIndex())
	{
		ASTExprArrow* arrow;
		if (operand.isTypeArrow()) arrow = &(ASTExprArrow&)operand;
		else arrow = (ASTExprArrow*)((ASTExprIndex&)operand).array;

        int oldid = c->symbols->getNodeId(arrow);
        c->symbols->putNodeId(arrow, c->symbols->getNodeId(&host));
        host.operand->execute(*this,param);
        c->symbols->putNodeId(arrow, oldid);
    }
    else if (host.operand->isTypeIdentifier())
    {
        host.operand->execute(*this, param);
    }

    addOpcode(new OPushRegister(new VarArgument(EXP1)));

    //increment EXP1
    addOpcode(new OAddImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
    //store it
    LValBOHelper helper;
    host.operand->execute(helper, param);
    vector<Opcode *> subcode = helper.getResult();

    for(vector<Opcode *>::iterator it = subcode.begin(); it != subcode.end(); it++)
    {
        addOpcode(*it);
    }

    //pop EXP1
    addOpcode(new OPopRegister(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprPreIncrement(ASTExprPreIncrement& host, void* param)
{
    OpcodeContext* c = (OpcodeContext*)param;

    // Load value of the variable into EXP1.  Except if it is an arrow expr, in
    // which case the gettor function is stored in this AST*.
	ASTExpr& operand = *host.operand;
	if (operand.isTypeArrow() || operand.isTypeIndex())
	{
		ASTExprArrow* arrow;
		if (operand.isTypeArrow()) arrow = &(ASTExprArrow&)operand;
		else arrow = (ASTExprArrow*)((ASTExprIndex&)operand).array;

        int oldid = c->symbols->getNodeId(arrow);
        c->symbols->putNodeId(arrow, c->symbols->getNodeId(&host));
        host.operand->execute(*this,param);
        c->symbols->putNodeId(arrow, oldid);
    }
    else if (host.operand->isTypeIdentifier())
    {
        host.operand->execute(*this, param);
    }

    //increment EXP1
    addOpcode(new OAddImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
    //store it
    LValBOHelper helper;
    host.operand->execute(helper, param);
    vector<Opcode *> subcode = helper.getResult();

    for(vector<Opcode *>::iterator it = subcode.begin(); it != subcode.end(); it++)
    {
        addOpcode(*it);
    }
}

void BuildOpcodes::caseExprPreDecrement(ASTExprPreDecrement& host, void* param)
{
    OpcodeContext* c = (OpcodeContext*)param;
    // Load value of the variable into EXP1 Except if it is an arrow expr, in
    // which case the gettor function is stored in this AST*.
	ASTExpr& operand = *host.operand;
	if (operand.isTypeArrow() || operand.isTypeIndex())
	{
		ASTExprArrow* arrow;
		if (operand.isTypeArrow()) arrow = &(ASTExprArrow&)operand;
		else arrow = (ASTExprArrow*)((ASTExprIndex&)operand).array;

        int oldid = c->symbols->getNodeId(arrow);
        c->symbols->putNodeId(arrow, c->symbols->getNodeId(&host));
        host.operand->execute(*this,param);
        c->symbols->putNodeId(arrow, oldid);
    }
    else if (host.operand->isTypeIdentifier())
    {
        host.operand->execute(*this, param);
    }

    //dencrement EXP1
    addOpcode(new OSubImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
    //store it
    LValBOHelper helper;
    host.operand->execute(helper, param);
    vector<Opcode *> subcode = helper.getResult();

    for(vector<Opcode *>::iterator it = subcode.begin(); it != subcode.end(); it++)
    {
        addOpcode(*it);
    }
}

void BuildOpcodes::caseExprDecrement(ASTExprDecrement& host, void* param)
{
    OpcodeContext* c = (OpcodeContext*)param;
    // Load value of the variable into EXP1 except if it is an arrow expr, in
    // which case the gettor function is stored in this AST*.
	ASTExpr& operand = *host.operand;
	if (operand.isTypeArrow() || operand.isTypeIndex())
	{
		ASTExprArrow* arrow;
		if (operand.isTypeArrow()) arrow = &(ASTExprArrow&)operand;
		else arrow = (ASTExprArrow*)((ASTExprIndex&)operand).array;

        int oldid = c->symbols->getNodeId(arrow);
        c->symbols->putNodeId(arrow, c->symbols->getNodeId(&host));
        host.operand->execute(*this,param);
        c->symbols->putNodeId(arrow, oldid);
    }
    else if (host.operand->isTypeIdentifier())
    {
        host.operand->execute(*this, param);
    }

    addOpcode(new OPushRegister(new VarArgument(EXP1)));

    //decrement EXP1
    addOpcode(new OSubImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
    //store it
    LValBOHelper helper;
    host.operand->execute(helper, param);
    vector<Opcode *> subcode = helper.getResult();

    for(vector<Opcode *>::iterator it = subcode.begin(); it != subcode.end(); it++)
    {
        addOpcode(*it);
    }

    //pop EXP1
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
    host.left->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.right->execute(*this, param);
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
    host.left->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.right->execute(*this,param);
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
    host.left->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.right->execute(*this, param);
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
    host.left->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.right->execute(*this, param);
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
    host.left->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.right->execute(*this, param);
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
    host.left->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.right->execute(*this, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    addOpcode(new OCompareRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
    addOpcode(new OSetLess(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprEQ(ASTExprEQ& host, void* param)
{
    // Special case for booleans.
    bool isBoolean = (*host.left->getVarType() == ZVarType::BOOL);

    if (host.getCompileTimeValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this))));
        return;
    }

    // Compute both sides.
    host.left->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.right->execute(*this, param);
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
    bool isBoolean = (*host.left->getVarType() == ZVarType::BOOL);

    if (host.getCompileTimeValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(*host.getCompileTimeValue(this))));
        return;
    }

    // Compute both sides.
    host.left->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.right->execute(*this, param);
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
    host.left->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.right->execute(*this, param);
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
    host.left->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.right->execute(*this, param);
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
    host.left->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.right->execute(*this, param);
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
    host.left->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.right->execute(*this, param);
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
    host.left->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.right->execute(*this, param);
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
    host.left->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.right->execute(*this, param);
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
    host.left->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.right->execute(*this, param);
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
    host.left->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.right->execute(*this, param);
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
    host.left->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.right->execute(*this, param);
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
    host.left->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.right->execute(*this, param);
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

void BuildOpcodes::caseStringLiteral(ASTStringLiteral& host, void* param)
{
	OpcodeContext* c = (OpcodeContext*)param;
	int id = c->symbols->getNodeId(&host);
    int globalid = c->linktable->getGlobalID(id);
    int RAMtype = (globalid == -1) ? SCRIPTRAM: GLOBALRAM;

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
	if (RAMtype == GLOBALRAM)
	{
		c->initCode.push_back(new OAllocateGlobalMemImmediate(new VarArgument(EXP1), new LiteralArgument(size)));
		c->initCode.push_back(new OSetRegister(new GlobalArgument(globalid), new VarArgument(EXP1)));
	}
	else
	{
		c->initCode.push_back(new OAllocateMemImmediate(new VarArgument(EXP1), new LiteralArgument(size)));
		int offset = c->stackframe->getOffset(id);
		c->initCode.push_back(new OSetRegister(new VarArgument(SFTEMP), new VarArgument(SFRAME)));
		c->initCode.push_back(new OAddImmediate(new VarArgument(SFTEMP), new LiteralArgument(offset)));
		c->initCode.push_back(new OStoreIndirect(new VarArgument(EXP1), new VarArgument(SFTEMP)));
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

	if (globalid != -1)
	{
        // Global variable, so just get its value.
        addOpcode(new OSetRegister(new VarArgument(EXP1), new GlobalArgument(globalid)));
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

	if (globalid == -1)
	{
		int offset = c->stackframe->getOffset(id);
		arrayRefs.push_back(offset);
	}
}

void BuildOpcodes::caseArrayLiteral(ASTArrayLiteral& host, void* param)
{
	OpcodeContext& context = *(OpcodeContext*)param;
	Literal& manager = *host.manager;
    int globalid = context.linktable->getGlobalID(manager.id);
    int RAMtype = (globalid == -1) ? SCRIPTRAM: GLOBALRAM;

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
	else if (host.getSize())
	{
		ASTExpr& sizeExpr = *host.getSize();
		if (optional<long> s = sizeExpr.getCompileTimeValue(this))
			size = *s / 10000L;
		else
		{
			handleError(CompileError::ExprNotConstant, &sizeExpr);
			return;
		}
	}

	// Otherwise, grab the number of elements.
	if (size == -1) size = host.getElements().size();

	// Make sure the chosen size has enough space.
	if (size < (int)host.getElements().size())
	{
		handleError(CompileError::ArrayListTooLarge, &host);
		return;
	}

	////////////////////////////////////////////////////////////////
	// Initialization Code.

	// Allocate.
	if (RAMtype == GLOBALRAM)
	{
		context.initCode.push_back(new OAllocateGlobalMemImmediate(new VarArgument(EXP1), new LiteralArgument(size * 10000L)));
		context.initCode.push_back(new OSetRegister(new GlobalArgument(globalid), new VarArgument(EXP1)));
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
	vector<ASTExpr*> elements = host.getElements();
	for (vector<ASTExpr*>::iterator it = elements.begin();
		 it != elements.end(); ++it, i += 10000L)
	{
		context.initCode.push_back(new OPushRegister(new VarArgument(INDEX)));
		opcodeTargets.push_back(&context.initCode);
		(*it)->execute(*this, param);
		opcodeTargets.pop_back();
		context.initCode.push_back(new OPopRegister(new VarArgument(INDEX)));
		context.initCode.push_back(new OSetImmediate(new VarArgument(INDEX2), new LiteralArgument(i)));
		context.initCode.push_back(new OSetRegister(new VarArgument(RAMtype), new VarArgument(EXP1)));
	}

	////////////////////////////////////////////////////////////////
	// Actual Code.

	if (globalid != -1)
	{
        // Global variable, so just get its value.
        addOpcode(new OSetRegister(new VarArgument(EXP1), new GlobalArgument(globalid)));
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

	if (globalid == -1)
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
    int globalid = c->linktable->getGlobalID(vid);

    if (globalid != -1)
    {
        // Global variable.
        addOpcode(new OSetRegister(new GlobalArgument(globalid), new VarArgument(EXP1)));
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
    vector<Opcode *> toadd;
    BuildOpcodes oc;
    host.left->execute(oc, param);
    toadd = oc.getResult();
    
    for(vector<Opcode *>::iterator it = toadd.begin(); it != toadd.end(); it++)
    {
        addOpcode(*it);
    }
    
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
        host.index->execute(oc2, param);
        toadd = oc2.getResult();
        
        for(vector<Opcode *>::iterator it = toadd.begin(); it != toadd.end(); it++)
        {
            addOpcode(*it);
        }
        
        addOpcode(new OPushRegister(new VarArgument(EXP1)));
    }
    
    //finally, goto!
    int label = c->linktable->functionToLabel(c->symbols->getNodeId(&host));
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
		host.array->execute(*this, param);
		return;
	}

	vector<Opcode*> opcodes;

	// Push the value.
    addOpcode(new OPushRegister(new VarArgument(EXP1)));

	// Get and push the array pointer.
	BuildOpcodes buildOpcodes1;
	host.array->execute(buildOpcodes1, param);
	opcodes = buildOpcodes1.getResult();
	for (vector<Opcode*>::iterator it = opcodes.begin(); it != opcodes.end(); ++it)
		addOpcode(*it);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));

	// Get the index.
	BuildOpcodes buildOpcodes2;
	host.index->execute(buildOpcodes2, param);
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

