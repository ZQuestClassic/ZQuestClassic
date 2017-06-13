
#include "../precompiled.h" //always first

#include "BuildVisitors.h"
#include <assert.h>
#include "ParseError.h"

/////////////////////////////////////////////////////////////////////////////////
// BuildOpcodes

BuildOpcodes::BuildOpcodes()
	: returnlabelid(-1), continuelabelid(-1), breaklabelid(-1), 
	  returnRefCount(0), continueRefCount(0), breakRefCount(0),
	  failure(false)
{
	opcodeTargets.push_back(&result);
}

void BuildOpcodes::caseDefault(void *)
{
    //unreachable
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
	list<ASTStmt*> l = host.getStatements();

	int startRefCount = arrayRefs.size();

    for (list<ASTStmt*>::iterator it = l.begin(); it != l.end(); it++)
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
    host.getCondition()->execute(*this,param);
    int endif = ScriptParser::getUniqueLabelID();
    addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
    addOpcode(new OGotoTrueImmediate(new LabelArgument(endif)));
    //run the block
    host.getStmt()->execute(*this,param);
    //nop
    Opcode *next = new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0));
    next->setLabel(endif);
    addOpcode(next);
}

void BuildOpcodes::caseStmtIfElse(ASTStmtIfElse &host, void *param)
{
    //run the test
    host.getCondition()->execute(*this,param);
    int elseif = ScriptParser::getUniqueLabelID();
    int endif = ScriptParser::getUniqueLabelID();
    addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
    addOpcode(new OGotoTrueImmediate(new LabelArgument(elseif)));
    //run if blocl
    host.getStmt()->execute(*this,param);
    addOpcode(new OGotoImmediate(new LabelArgument(endif)));
    Opcode *next = new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(0));
    next->setLabel(elseif);
    addOpcode(next);
    host.getElseStmt()->execute(*this,param);
    next = new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(0));
    next->setLabel(endif);
    addOpcode(next);
}

void BuildOpcodes::caseStmtSwitch(ASTStmtSwitch &host, void* param)
{
	map<ASTSwitchCases*, int> labels;
	vector<ASTSwitchCases*> cases = host.getCases();

	int end_label = ScriptParser::getUniqueLabelID();;
	int default_label = end_label;

	// save and override break label.
	int old_break_label = breaklabelid;
	int oldBreakRefCount = breakRefCount;
	breaklabelid = end_label;
	breakRefCount = arrayRefs.size();

	// Evaluate the key.
	ASTExpr* key = host.getKey();
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
		for (vector<ASTExprConst*>::iterator it = cases->getCases().begin();
			 it != cases->getCases().end();
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
		if (cases->isDefaultCase())
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
		cases->getBlock()->execute(*this, param);
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
    host.getPrecondition()->execute(*this,param);
    int loopstart = ScriptParser::getUniqueLabelID();
    int loopend = ScriptParser::getUniqueLabelID();
    int loopincr = ScriptParser::getUniqueLabelID();
    //nop
    Opcode *next = new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0));
    next->setLabel(loopstart);
    addOpcode(next);
    //test the termination condition
    host.getTerminationCondition()->execute(*this,param);
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

    host.getStmt()->execute(*this,param);

    breaklabelid = oldbreak;
    breakRefCount = oldBreakRefCount;
    continuelabelid = oldcontinue;
	continueRefCount = oldContinueRefCount;

    //run the increment
    //nop
    next = new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(0));
    next->setLabel(loopincr);
    addOpcode(next);
    host.getIncrement()->execute(*this,param);
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
    host.getCond()->execute(*this,param);
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

    host.getStmt()->execute(*this,param);

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

    host.getStmt()->execute(*this, param);

    breaklabelid = oldbreak;
    continuelabelid = oldcontinue;
    breakRefCount = oldBreakRefCount;
	continueRefCount = oldContinueRefCount;

    start = new OSetImmediate(new VarArgument(NUL), new LiteralArgument(0));
    start->setLabel(continuelabel);
    addOpcode(start);
    host.getCond()->execute(*this,param);
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
    host.getReturnValue()->execute(*this, param);
	deallocateRefsUntilCount(0);
    addOpcode(new OGotoImmediate(new LabelArgument(returnlabelid)));
}

void BuildOpcodes::caseStmtBreak(ASTStmtBreak &host, void *)
{
    if (breaklabelid == -1)
    {
        printErrorMsg(&host, BREAKBAD);
        failure = true;
        return;
    }

	deallocateRefsUntilCount(breakRefCount);
    addOpcode(new OGotoImmediate(new LabelArgument(breaklabelid)));
}

void BuildOpcodes::caseStmtContinue(ASTStmtContinue &host, void *)
{
    if (continuelabelid == -1)
    {
        printErrorMsg(&host, CONTINUEBAD);
        failure = true;
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

    host.getBlock()->execute(*this, param);
}

void BuildOpcodes::caseArrayDecl(ASTArrayDecl &host, void *param)
{
    OpcodeContext *c = (OpcodeContext *) param;

    // Check if the array is global or not.
	int id = c->symbols->getNodeId(&host);
    int globalid = c->linktable->getGlobalID(id);
    int RAMtype = (globalid == -1) ? SCRIPTRAM: GLOBALRAM;

    // Size is an expression.
	/*
    if(host.isRegister())
    {
        // Push size.
        ((ASTExpr *) host.getSize())->execute(*this, param);

        // Allocate.
        if(RAMtype == GLOBALRAM)
        {
            addOpcode(new OAllocateGlobalMemRegister(new VarArgument(EXP1), new VarArgument(EXP1)));
            addOpcode(new OSetRegister(new GlobalArgument(globalid), new VarArgument(EXP1)));
        }
        else
        {
            addOpcode(new OAllocateMemRegister(new VarArgument(EXP1), new VarArgument(EXP1)));
            int offset = c->stackframe->getOffset(c->symbols->getNodeId(&host));
            addOpcode(new OSetRegister(new VarArgument(SFTEMP), new VarArgument(SFRAME)));
            addOpcode(new OAddImmediate(new VarArgument(SFTEMP), new LiteralArgument(offset)));
            addOpcode(new OStoreIndirect(new VarArgument(EXP1), new VarArgument(SFTEMP)));
        }
    }
	*/

	// Get size.

	if (!host.getSize()->hasDataValue())
	{
		failure = true;
		printErrorMsg(&host, EXPRNOTCONSTANT);
		return;
	}

	long size = host.getSize()->getDataValue();

	if (size < 10000)
	{
		failure = true;
		printErrorMsg(&host, ARRAYTOOSMALL, "");
		return;
	}

	// Check if we've allocated enough memory for the initialiser.
	if (host.getList() != NULL && host.getList()->getList().size() >= size_t(size))
	{
		failure = true;

		if(host.getList()->isString())
			printErrorMsg(&host, ARRAYLISTSTRINGTOOLARGE, "");
		else
			printErrorMsg(&host, ARRAYLISTTOOLARGE, "");

		return;
	}

	// Allocate.
	if (RAMtype == GLOBALRAM)
	{
		addOpcode(new OAllocateGlobalMemImmediate(new VarArgument(EXP1), new LiteralArgument(size)));
		addOpcode(new OSetRegister(new GlobalArgument(globalid), new VarArgument(EXP1)));
	}
	else
	{
		addOpcode(new OAllocateMemImmediate(new VarArgument(EXP1), new LiteralArgument(size)));
		int offset = c->stackframe->getOffset(c->symbols->getNodeId(&host));
		addOpcode(new OSetRegister(new VarArgument(SFTEMP), new VarArgument(SFRAME)));
		addOpcode(new OAddImmediate(new VarArgument(SFTEMP), new LiteralArgument(offset)));
		addOpcode(new OStoreIndirect(new VarArgument(EXP1), new VarArgument(SFTEMP)));
	}

    // Initialise.
    if(host.getList() != NULL)
    {
        addOpcode(new OSetRegister(new VarArgument(INDEX), new VarArgument(EXP1)));

        int i = 0;
        for(list<ASTExpr *>::iterator it = host.getList()->getList().begin();
          it != host.getList()->getList().end();
          it++, i+=10000)
        {
            addOpcode(new OPushRegister(new VarArgument(INDEX)));
            (*it)->execute(*this, param);
            addOpcode(new OPopRegister(new VarArgument(INDEX)));
            addOpcode(new OSetImmediate(new VarArgument(INDEX2), new LiteralArgument(i)));
            addOpcode(new OSetRegister(new VarArgument(RAMtype), new VarArgument(EXP1)));
        }
    }

	// Register for cleanup.
	if (globalid == -1)
	{
		int offset = c->stackframe->getOffset(id);
		arrayRefs.push_back(offset);
	}
}

void BuildOpcodes::caseVarDecl(ASTVarDecl &host, void *param)
{
    //initialize to 0
    OpcodeContext *c = (OpcodeContext *)param;

	// Don't need to initialize a constant.
	// (Although a constant without an initializer doesn't make sense.)
	if (c->symbols->isInlinedConstant(&host)) return;

    int globalid = c->linktable->getGlobalID(c->symbols->getNodeId(&host));
    if (globalid != -1)
    {
        //it's a global var
        //just set it to 0
        addOpcode(new OSetImmediate(new GlobalArgument(globalid), new LiteralArgument(0)));
        return;
    }

    //it's a local var
    //set it to 0 on the stack
    int offset = c->stackframe->getOffset(c->symbols->getNodeId(&host));
    addOpcode(new OSetRegister(new VarArgument(SFTEMP), new VarArgument(SFRAME)));
    addOpcode(new OAddImmediate(new VarArgument(SFTEMP), new LiteralArgument(offset)));
    addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
    addOpcode(new OStoreIndirect(new VarArgument(EXP1), new VarArgument(SFTEMP)));
}

void BuildOpcodes::caseVarDeclInitializer(ASTVarDeclInitializer &host, void *param)
{
    OpcodeContext *c = (OpcodeContext *) param;

	// Don't need to initialize a constant.
	if (c->symbols->isInlinedConstant(&host)) return;

	// Compute the value of the initializer
    host.getInitializer()->execute(*this,param);
    //value of expr now stored in EXP1
    int globalid = c->linktable->getGlobalID(c->symbols->getNodeId(&host));

    if(globalid != -1)
    {
        //it's a global var
        addOpcode(new OSetRegister(new GlobalArgument(globalid), new VarArgument(EXP1)));
        return;
    }

    //it's a local var
    //set it on the stack
    int offset = c->stackframe->getOffset(c->symbols->getNodeId(&host));
    addOpcode(new OSetRegister(new VarArgument(SFTEMP), new VarArgument(SFRAME)));
    addOpcode(new OAddImmediate(new VarArgument(SFTEMP), new LiteralArgument(offset)));
    addOpcode(new OStoreIndirect(new VarArgument(EXP1), new VarArgument(SFTEMP)));
}

void BuildOpcodes::caseTypeDef(ASTTypeDef&, void*) {}

// Expressions

void BuildOpcodes::caseExprAssign(ASTExprAssign &host, void *param)
{
    //load the rval into EXP1
    host.getRVal()->execute(*this,param);
    //and store it
    LValBOHelper helper;
    host.getLVal()->execute(helper, param);
    vector<Opcode *> subcode = helper.getResult();

    for(vector<Opcode *>::iterator it = subcode.begin(); it != subcode.end(); it++)
    {
        addOpcode(*it);
    }
}

void BuildOpcodes::caseExprConst(ASTExprConst &host, void *param)
{
	host.getContent()->execute(*this, param);
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
    bool isIndexed = host.getIndex() != NULL;
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
    host.getLeft()->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));

    //if indexed, push the index
    if(isIndexed)
    {
        host.getIndex()->execute(*this,param);
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
	if (host.getArray()->isTypeArrow())
	{
		host.getArray()->execute(*this, param);
		return;
	}

	// First, push the array.
	host.getArray()->execute(*this, param);
	addOpcode(new OPushRegister(new VarArgument(EXP1)));

	// Load the index into INDEX2.
	host.getIndex()->execute(*this, param);
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
    if (host.getLeft()->isTypeArrow())
    {
        //load the value of the left-hand of the arrow into EXP1
        ((ASTExprArrow*)host.getLeft())->getLeft()->execute(*this, param);
        //host.getLeft()->execute(*this,param);
        //push it onto the stack
        addOpcode(new OPushRegister(new VarArgument(EXP1)));
    }

    //push the parameters, in forward order
    for(list<ASTExpr *>::iterator it = host.getParams().begin(); it != host.getParams().end(); it++)
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
    if (host.hasDataValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getDataValue())));
        return;
    }

    host.getOperand()->execute(*this, param);
    addOpcode(new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(0)));
    addOpcode(new OSubRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
    addOpcode(new OSetRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprNot(ASTExprNot& host, void* param)
{
    if (host.hasDataValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getDataValue())));
        return;
    }

    host.getOperand()->execute(*this, param);
    addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
    addOpcode(new OSetTrue(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprBitNot(ASTExprBitNot& host, void* param)
{
    if (host.hasDataValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getDataValue())));
        return;
    }

    host.getOperand()->execute(*this, param);
    addOpcode(new ONot(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprIncrement(ASTExprIncrement& host, void* param)
{
    OpcodeContext* c = (OpcodeContext*)param;

    // Load value of the variable into EXP1.  Except if it is an arrow expr, in
    // which case the gettor function is stored in this AST*.
	ASTExpr& operand = *host.getOperand();
	if (operand.isTypeArrow() || operand.isTypeIndex())
	{
		ASTExprArrow* arrow;
		if (operand.isTypeArrow()) arrow = &(ASTExprArrow&)operand;
		else arrow = (ASTExprArrow*)((ASTExprIndex&)operand).getArray();

        int oldid = c->symbols->getNodeId(arrow);
        c->symbols->putNodeId(arrow, c->symbols->getNodeId(&host));
        host.getOperand()->execute(*this,param);
        c->symbols->putNodeId(arrow, oldid);
    }
    else if (host.getOperand()->isTypeIdentifier())
    {
        host.getOperand()->execute(*this, param);
    }

    addOpcode(new OPushRegister(new VarArgument(EXP1)));

    //increment EXP1
    addOpcode(new OAddImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
    //store it
    LValBOHelper helper;
    host.getOperand()->execute(helper, param);
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
	ASTExpr& operand = *host.getOperand();
	if (operand.isTypeArrow() || operand.isTypeIndex())
	{
		ASTExprArrow* arrow;
		if (operand.isTypeArrow()) arrow = &(ASTExprArrow&)operand;
		else arrow = (ASTExprArrow*)((ASTExprIndex&)operand).getArray();

        int oldid = c->symbols->getNodeId(arrow);
        c->symbols->putNodeId(arrow, c->symbols->getNodeId(&host));
        host.getOperand()->execute(*this,param);
        c->symbols->putNodeId(arrow, oldid);
    }
    else if (host.getOperand()->isTypeIdentifier())
    {
        host.getOperand()->execute(*this, param);
    }

    //increment EXP1
    addOpcode(new OAddImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
    //store it
    LValBOHelper helper;
    host.getOperand()->execute(helper, param);
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
	ASTExpr& operand = *host.getOperand();
	if (operand.isTypeArrow() || operand.isTypeIndex())
	{
		ASTExprArrow* arrow;
		if (operand.isTypeArrow()) arrow = &(ASTExprArrow&)operand;
		else arrow = (ASTExprArrow*)((ASTExprIndex&)operand).getArray();

        int oldid = c->symbols->getNodeId(arrow);
        c->symbols->putNodeId(arrow, c->symbols->getNodeId(&host));
        host.getOperand()->execute(*this,param);
        c->symbols->putNodeId(arrow, oldid);
    }
    else if (host.getOperand()->isTypeIdentifier())
    {
        host.getOperand()->execute(*this, param);
    }

    //dencrement EXP1
    addOpcode(new OSubImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
    //store it
    LValBOHelper helper;
    host.getOperand()->execute(helper, param);
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
	ASTExpr& operand = *host.getOperand();
	if (operand.isTypeArrow() || operand.isTypeIndex())
	{
		ASTExprArrow* arrow;
		if (operand.isTypeArrow()) arrow = &(ASTExprArrow&)operand;
		else arrow = (ASTExprArrow*)((ASTExprIndex&)operand).getArray();

        int oldid = c->symbols->getNodeId(arrow);
        c->symbols->putNodeId(arrow, c->symbols->getNodeId(&host));
        host.getOperand()->execute(*this,param);
        c->symbols->putNodeId(arrow, oldid);
    }
    else if (host.getOperand()->isTypeIdentifier())
    {
        host.getOperand()->execute(*this, param);
    }

    addOpcode(new OPushRegister(new VarArgument(EXP1)));

    //decrement EXP1
    addOpcode(new OSubImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
    //store it
    LValBOHelper helper;
    host.getOperand()->execute(helper, param);
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
    if (host.hasDataValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getDataValue())));
        return;
    }

    //compute both sides
    host.getFirstOperand()->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.getSecondOperand()->execute(*this, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    castFromBool(result, EXP1);
    castFromBool(result, EXP2);
    addOpcode(new OAddRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
    addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(2)));
    addOpcode(new OSetMore(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprOr(ASTExprOr& host, void* param)
{
    if (host.hasDataValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getDataValue())));
        return;
    }

    //compute both sides
    host.getFirstOperand()->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.getSecondOperand()->execute(*this,param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    addOpcode(new OAddRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
    addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(1)));
    addOpcode(new OSetMore(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprGT(ASTExprGT& host, void* param)
{
    if (host.hasDataValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getDataValue())));
        return;
    }

    //compute both sides
    host.getFirstOperand()->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.getSecondOperand()->execute(*this, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    addOpcode(new OCompareRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
    addOpcode(new OSetLess(new VarArgument(EXP1)));
    addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
    addOpcode(new OSetTrue(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprGE(ASTExprGE& host, void* param)
{
    if (host.hasDataValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getDataValue())));
        return;
    }

    //compute both sides
    host.getFirstOperand()->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.getSecondOperand()->execute(*this, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    addOpcode(new OCompareRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
    addOpcode(new OSetMore(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprLT(ASTExprLT& host, void* param)
{
    if (host.hasDataValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getDataValue())));
        return;
    }

    //compute both sides
    host.getFirstOperand()->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.getSecondOperand()->execute(*this, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    addOpcode(new OCompareRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
    addOpcode(new OSetMore(new VarArgument(EXP1)));
    addOpcode(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
    addOpcode(new OSetTrue(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprLE(ASTExprLE& host, void* param)
{
    if (host.hasDataValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getDataValue())));
        return;
    }

    // Compute both sides.
    host.getFirstOperand()->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.getSecondOperand()->execute(*this, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    addOpcode(new OCompareRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
    addOpcode(new OSetLess(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprEQ(ASTExprEQ& host, void* param)
{
    // Special case for booleans.
    bool isBoolean = (*host.getFirstOperand()->getVarType() == ZVarType::BOOL);

    if (host.hasDataValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getDataValue())));
        return;
    }

    // Compute both sides.
    host.getFirstOperand()->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.getSecondOperand()->execute(*this, param);
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
    bool isBoolean = (*host.getFirstOperand()->getVarType() == ZVarType::BOOL);

    if (host.hasDataValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getDataValue())));
        return;
    }

    // Compute both sides.
    host.getFirstOperand()->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.getSecondOperand()->execute(*this, param);
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
    if (host.hasDataValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getDataValue())));
        return;
    }

    // Compute both sides.
    host.getFirstOperand()->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.getSecondOperand()->execute(*this, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    addOpcode(new OAddRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprMinus(ASTExprMinus& host, void* param)
{
    if (host.hasDataValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getDataValue())));
        return;
    }

    // Compute both sides.
    host.getFirstOperand()->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.getSecondOperand()->execute(*this, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    addOpcode(new OSubRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
    addOpcode(new OSetRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprTimes(ASTExprTimes& host, void *param)
{
    if (host.hasDataValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getDataValue())));
        return;
    }

    // Compute both sides.
    host.getFirstOperand()->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.getSecondOperand()->execute(*this, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    addOpcode(new OMultRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprDivide(ASTExprDivide& host, void* param)
{
    if (host.hasDataValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getDataValue())));
        return;
    }

    // Compute both sides.
    host.getFirstOperand()->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.getSecondOperand()->execute(*this, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    addOpcode(new ODivRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
    addOpcode(new OSetRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprModulo(ASTExprModulo& host, void* param)
{
    if (host.hasDataValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getDataValue())));
        return;
    }

    // Compute both sides.
    host.getFirstOperand()->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.getSecondOperand()->execute(*this, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    addOpcode(new OModuloRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
    addOpcode(new OSetRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprBitAnd(ASTExprBitAnd& host, void* param)
{
    if (host.hasDataValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getDataValue())));
        return;
    }

    // Compute both sides.
    host.getFirstOperand()->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.getSecondOperand()->execute(*this, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    addOpcode(new OAndRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprBitOr(ASTExprBitOr& host, void* param)
{
    if (host.hasDataValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getDataValue())));
        return;
    }

    // Compute both sides.
    host.getFirstOperand()->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.getSecondOperand()->execute(*this, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    addOpcode(new OOrRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprBitXor(ASTExprBitXor& host, void* param)
{
    if (host.hasDataValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getDataValue())));
        return;
    }

    // Compute both sides.
    host.getFirstOperand()->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.getSecondOperand()->execute(*this, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    addOpcode(new OXorRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprLShift(ASTExprLShift& host, void* param)
{
    if (host.hasDataValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getDataValue())));
        return;
    }

    // Compute both sides.
    host.getFirstOperand()->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.getSecondOperand()->execute(*this, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    addOpcode(new OLShiftRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
    addOpcode(new OSetRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprRShift(ASTExprRShift& host, void* param)
{
    if (host.hasDataValue())
    {
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getDataValue())));
        return;
    }

    // Compute both sides.
    host.getFirstOperand()->execute(*this, param);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));
    host.getSecondOperand()->execute(*this, param);
    addOpcode(new OPopRegister(new VarArgument(EXP2)));
    addOpcode(new ORShiftRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
    addOpcode(new OSetRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

// Literals

void BuildOpcodes::caseNumberLiteral(ASTNumberLiteral& host, void*)
{
    if (host.hasDataValue())
        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getDataValue())));
    else
    {
        pair<long, bool> val = ScriptParser::parseLong(host.getValue()->parseValue());

        if (!val.second)
            printErrorMsg(&host, CONSTTRUNC, host.getValue()->getValue());

        addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(val.first)));
    }
}

void BuildOpcodes::caseBoolLiteral(ASTBoolLiteral& host, void*)
{
    addOpcode(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getDataValue())));
}

void BuildOpcodes::caseStringLiteral(ASTStringLiteral& host, void* param)
{
	OpcodeContext* c = (OpcodeContext*)param;
	int id = c->symbols->getNodeId(&host);
    int globalid = c->linktable->getGlobalID(id);
    int RAMtype = (globalid == -1) ? SCRIPTRAM: GLOBALRAM;

	////////////////////////////////////////////////////////////////
	// Initialization Code.

	string data = host.getValue();
	long size = (data.size() + 1) * 10000L;

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
	OpcodeContext* c = (OpcodeContext*)param;
	int id = c->symbols->getNodeId(&host);
    int globalid = c->linktable->getGlobalID(id);
    int RAMtype = (globalid == -1) ? SCRIPTRAM: GLOBALRAM;

	int size = -1;

	// If there's an explicit size, grab it.
	if (host.getSize())
	{
		// Make sure the size has been resloved.
		if (!host.getSize()->hasDataValue())
		{
			printErrorMsg(&host, EXPRNOTCONSTANT);
			failure = true;
			return;
		}

		// Grab the size.
		size = host.getSize()->getDataValue() / 10000;

		// Make sure the chosen size has enough space.
		if (size < host.getElements().size())
		{
			printErrorMsg(&host, ARRAYLISTTOOLARGE);
			failure = true;
			return;
		}
	}
	// Otherwise, grab the number of elements.
	else size = host.getElements().size();

	////////////////////////////////////////////////////////////////
	// Initialization Code.

	// Allocate.
	if (RAMtype == GLOBALRAM)
	{
		c->initCode.push_back(new OAllocateGlobalMemImmediate(new VarArgument(EXP1), new LiteralArgument(size * 10000L)));
		c->initCode.push_back(new OSetRegister(new GlobalArgument(globalid), new VarArgument(EXP1)));
	}
	else
	{
		c->initCode.push_back(new OAllocateMemImmediate(new VarArgument(EXP1), new LiteralArgument(size * 10000L)));
		int offset = c->stackframe->getOffset(id);
		c->initCode.push_back(new OSetRegister(new VarArgument(SFTEMP), new VarArgument(SFRAME)));
		c->initCode.push_back(new OAddImmediate(new VarArgument(SFTEMP), new LiteralArgument(offset)));
		c->initCode.push_back(new OStoreIndirect(new VarArgument(EXP1), new VarArgument(SFTEMP)));
	}

	// Initialize.
	c->initCode.push_back(new OSetRegister(new VarArgument(INDEX), new VarArgument(EXP1)));
	long i = 0;
	vector<ASTExpr*> elements = host.getElements();
	for (vector<ASTExpr*>::iterator it = elements.begin();
		 it != elements.end(); ++it, i += 10000L)
	{
		c->initCode.push_back(new OPushRegister(new VarArgument(INDEX)));
		opcodeTargets.push_back(&c->initCode);
		(*it)->execute(*this, param);
		opcodeTargets.pop_back();
		c->initCode.push_back(new OPopRegister(new VarArgument(INDEX)));
		c->initCode.push_back(new OSetImmediate(new VarArgument(INDEX2), new LiteralArgument(i)));
		c->initCode.push_back(new OSetRegister(new VarArgument(RAMtype), new VarArgument(EXP1)));
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


void LValBOHelper::caseVarDecl(ASTVarDecl &host, void *param)
{
    //cannot be a global variable, so just stuff it in the stack
    OpcodeContext *c = (OpcodeContext *)param;
    int vid = c->symbols->getNodeId(&host);
    int offset = c->stackframe->getOffset(vid);
    addOpcode(new OSetRegister(new VarArgument(SFTEMP), new VarArgument(SFRAME)));
    addOpcode(new OAddImmediate(new VarArgument(SFTEMP), new LiteralArgument(offset)));
    addOpcode(new OStoreIndirect(new VarArgument(EXP1), new VarArgument(SFTEMP)));
}

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
    int isIndexed = (host.getIndex() != NULL);
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
    host.getLeft()->execute(oc, param);
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
        host.getIndex()->execute(oc2, param);
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
	if (host.getArray()->isTypeArrow())
	{
		host.getArray()->execute(*this, param);
		return;
	}

	vector<Opcode*> opcodes;

	// Push the value.
    addOpcode(new OPushRegister(new VarArgument(EXP1)));

	// Get and push the array pointer.
	BuildOpcodes buildOpcodes1;
	host.getArray()->execute(buildOpcodes1, param);
	opcodes = buildOpcodes1.getResult();
	for (vector<Opcode*>::iterator it = opcodes.begin(); it != opcodes.end(); ++it)
		addOpcode(*it);
    addOpcode(new OPushRegister(new VarArgument(EXP1)));

	// Get the index.
	BuildOpcodes buildOpcodes2;
	host.getIndex()->execute(buildOpcodes2, param);
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

