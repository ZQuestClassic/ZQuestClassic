
#include "../precompiled.h" //always first
#include <assert.h>

#include "BuildVisitors.h"
#include "CompileError.h"
#include "Opcode.h"
#include "Types.h"
#include "ZScript.h"

using std::string;
using namespace ZScript;
using namespace ZAsm;

void comment(Opcode& opcode, AST const& node)
{
	opcode.withComment(getLineString(node));
}

/////////////////////////////////////////////////////////////////////////////////
// BuildOpcodes

BuildOpcodes::BuildOpcodes(TypeStore& typeStore)
	: typeStore(typeStore),
	  returnlabelid(-1), continuelabelid(-1), breaklabelid(-1), 
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

void BuildOpcodes::addOpcode(Opcode const& code)
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
	addOpcode(opSETR(varSFTemp(), varSFrame()));
	addOpcode(opADDV(varSFTemp(), arrayRef));
	addOpcode(opLOADI(varExp2(), varSFTemp()));
	addOpcode(opDEALLOCATEMEMR(varExp2()));
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
	int start = result.size();

    for (vector<ASTStmt*>::iterator it = host.statements.begin();
		 it != host.statements.end(); ++it)
	{
		int initIndex = result.size();
        visit(*it, param);
		result.insert(result.begin() + initIndex, c->initCode.begin(), c->initCode.end());
		c->initCode.clear();
		if (initIndex < result.size()) comment(result[initIndex], **it);
	}

	deallocateRefsUntilCount(startRefCount);
	while ((int)arrayRefs.size() > startRefCount)
		arrayRefs.pop_back();

	/*
	if (!result.empty())
	{
		result[start]->appendComment(host.asString());
		result.back()->appendComment("    END " + host.asString());
	}
	*/
}

void BuildOpcodes::caseStmtIf(ASTStmtIf &host, void *param)
{
    //run the test
    visit(host.condition, param);
    int endif = ScriptParser::getUniqueLabelID();
    addOpcode(opCOMPAREV(varExp1(), 0));
    addOpcode(opGOTOTRUE(endif));
    //run the block
    visit(host.thenStatement, param);
    //nop
    addOpcode(opSETV(varExp1(), 0).withLabel(endif));
}

void BuildOpcodes::caseStmtIfElse(ASTStmtIfElse &host, void *param)
{
	//run the test
    visit(host.condition, param);
    int elseif = ScriptParser::getUniqueLabelID();
    int endif = ScriptParser::getUniqueLabelID();
    addOpcode(opCOMPAREV(varExp1(), 0));
    addOpcode(opGOTOTRUE(elseif));
    //run if blocl
    visit(host.thenStatement, param);
    addOpcode(opGOTO(endif));
    addOpcode(opSETV(varExp2(), 0).withLabel(elseif));
    visit(host.elseStatement, param);
    addOpcode(opSETV(varExp2(), 0).withLabel(endif));
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
	result.push_back(opSETR(varExp2(), varExp1()));

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
			result.push_back(opPUSHR(varExp2()));
			visit(*it, param);
			result.push_back(opPOP(varExp2()));
			// If the test succeeds, jump to its label.
			result.push_back(opCOMPARER(varExp1(), varExp2()));
			result.push_back(opGOTOTRUE(label));
		}

		// If this set includes the default case, mark it.
		if (cases->isDefault)
			default_label = label;
	}

	// Add direct jump to default case (or end if there isn't one.).
	result.push_back(opGOTO(default_label));

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
			result.push_back(opSETV(varExp2(), 0));
		// Set label to start of block.
		result[block_start_index].withLabel(labels[cases]);
	}

	// Add ending label.
    result.push_back(opSETV(varExp2(), 0).withLabel(end_label));

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
    addOpcode(opSETV(varExp1(), 0).withLabel(loopstart));
    //test the termination condition
    visit(host.test, param);
    addOpcode(opCOMPAREV(varExp1(), 0));
    addOpcode(opGOTOTRUE(loopend));
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
    addOpcode(opSETV(varExp2(), 0).withLabel(loopincr));
    visit(host.increment, param);
    addOpcode(opGOTO(loopstart));
    //nop
    addOpcode(opSETV(varExp2(), 0).withLabel(loopend));
}

void BuildOpcodes::caseStmtWhile(ASTStmtWhile &host, void *param)
{
    int startlabel = ScriptParser::getUniqueLabelID();
    int endlabel = ScriptParser::getUniqueLabelID();
    //run the test
    //nop to label start
    addOpcode(opSETV(varExp1(), 0).withLabel(startlabel));
    visit(host.test, param);
    addOpcode(opCOMPAREV(varExp1(), 0));
    addOpcode(opGOTOTRUE(endlabel));

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

    addOpcode(opGOTO(startlabel));
    //nop to end while
    addOpcode(opSETV(varExp1(), 0).withLabel(endlabel));
}

void BuildOpcodes::caseStmtDo(ASTStmtDo &host, void *param)
{
    int startlabel = ScriptParser::getUniqueLabelID();
    int endlabel = ScriptParser::getUniqueLabelID();
    int continuelabel = ScriptParser::getUniqueLabelID();
    //nop to label start
    addOpcode(opSETV(varExp1(), 0).withLabel(startlabel));

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

	addOpcode(opSETV(varNull(), 0).withLabel(continuelabel));
    visit(host.test, param);
    addOpcode(opCOMPAREV(varExp1(), 0));
    addOpcode(opGOTOTRUE(endlabel));
    addOpcode(opGOTO(startlabel));
    //nop to end dowhile
    addOpcode(opSETV(varExp1(), 0).withLabel(endlabel));
}

void BuildOpcodes::caseStmtReturn(ASTStmtReturn& host, void*)
{
	deallocateRefsUntilCount(0);
    addOpcode(opGOTO(returnlabelid));
}

void BuildOpcodes::caseStmtReturnVal(ASTStmtReturnVal &host, void *param)
{
	visit(host.value, param);
	deallocateRefsUntilCount(0);
    addOpcode(opGOTO(returnlabelid));
}

void BuildOpcodes::caseStmtBreak(ASTStmtBreak &host, void *)
{
    if (breaklabelid == -1)
    {
        handleError(CompileError::BreakBad, &host);
        return;
    }

	deallocateRefsUntilCount(breakRefCount);
    addOpcode(opGOTO(breaklabelid));
}

void BuildOpcodes::caseStmtContinue(ASTStmtContinue &host, void *)
{
    if (continuelabelid == -1)
    {
        handleError(CompileError::ContinueBad, &host);
        return;
    }

	deallocateRefsUntilCount(continueRefCount);
    addOpcode(opGOTO(continuelabelid));
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
	if (manager.type.isArray())
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
			addOpcode(opSETR(varGD(*globalId), varExp1()));
		else
			addOpcode(opSETV(varGD(*globalId), 0));
	}
	else
	{
		int offset = 10000L * *getStackOffset(manager);
		addOpcode(opSETR(varSFTemp(), varSFrame()));
		addOpcode(opADDV(varSFTemp(), offset));
		if (!host.initializer())
			addOpcode(opSETV(varExp1(), 0));
		addOpcode(opSTOREI(varExp1(), varSFTemp()));
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
		addOpcode(opSETR(varGD(*globalId), varExp1()));
	}
	else
	{
		int offset = 10000L * *getStackOffset(manager);
		addOpcode(opSETR(varSFTemp(), varSFrame()));
		addOpcode(opADDV(varSFTemp(), offset));
		addOpcode(opSTOREI(varExp1(), varSFTemp()));
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
		addOpcode(opALLOCATEGMEMV(varExp1(), totalSize));
		addOpcode(opSETR(varGD(*globalId), varExp1()));
	}
	else
	{
		addOpcode(opALLOCATEMEMV(varExp1(), totalSize));
		int offset = 10000L * *getStackOffset(manager);
		addOpcode(opSETR(varSFTemp(), varSFrame()));
		addOpcode(opADDV(varSFTemp(), offset));
		addOpcode(opSTOREI(varExp1(), varSFTemp()));
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
    LValBOHelper helper(typeStore);
    host.left->execute(helper, param);
	addOpcodes(helper.getResult());
}

void BuildOpcodes::caseExprIdentifier(ASTExprIdentifier& host, void* param)
{
    OpcodeContext* c = (OpcodeContext*)param;

	// If a constant, just load its value.
    if (optional<long> value = host.binding->getCompileTimeValue())
    {
        addOpcode(opSETV(varExp1(), *value));
		host.markConstant();
        return;
    }

    int vid = host.binding->id;

    if (optional<int> globalId = host.binding->getGlobalId())
    {
        // Global variable, so just get its value.
        addOpcode(opSETR(varExp1(), varGD(*globalId)));
        return;
    }

    // Local variable, get its value from the stack.
    int offset = 10000L * *getStackOffset(*host.binding);
    addOpcode(opSETR(varSFTemp(), varSFrame()));
    addOpcode(opADDV(varSFTemp(), offset));
    addOpcode(opLOADI(varExp1(), varSFTemp()));
}

void BuildOpcodes::caseExprArrow(ASTExprArrow& host, void* param)
{
    OpcodeContext* c = (OpcodeContext*)param;
    bool isIndexed = host.index != NULL;
    //this is actually a function call
    //to the appropriate gettor method
    //so, set that up:
    //push the stack frame
    addOpcode(opPUSHR(varSFrame()));
    int returnlabel = ScriptParser::getUniqueLabelID();
    //push the return address
    addOpcode(opSETV(varExp1(), Label(returnlabel)));
    addOpcode(opPUSHR(varExp1()));
    //get the rhs of the arrow
    visit(host.left, param);
    addOpcode(opPUSHR(varExp1()));

    //if indexed, push the index
    if(isIndexed)
    {
        visit(host.index, param);
        addOpcode(opPUSHR(varExp1()));
    }

    //call the function
    int label = host.readFunction->getLabel();
    addOpcode(opGOTO(label));
    //pop the stack frame
    addOpcode(opPOP(varSFrame()).withLabel(returnlabel));
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
	addOpcode(opPUSHR(varExp1()));

	// Load the index into INDEX2.
	visit(host.index, param);
	addOpcode(opSETR(varIndex2(), varExp1()));

	// Pop array into INDEX.
	addOpcode(opPOP(varIndex1()));

	// Return GLOBALRAM to indicate an array access.
	//   (As far as I can tell, there's no difference between GLOBALRAM and
	//    SCRIPTRAM, so I'll use GLOBALRAM here instead of checking.)
	addOpcode(opSETR(varExp1(), varGLOBALRAM()));
}

void BuildOpcodes::caseExprCall(ASTExprCall& host, void* param)
{
    OpcodeContext* c = (OpcodeContext*)param;
    int funclabel = host.binding->getLabel();
    //push the stack frame pointer
    addOpcode(opPUSHR(varSFrame()));
    //push the return address
    int returnaddr = ScriptParser::getUniqueLabelID();
    addOpcode(opSETV(varExp1(), Label(returnaddr)));
    addOpcode(opPUSHR(varExp1()));

    // If the function is a pointer function (->func()) we need to push the
    // left-hand-side.
    if (host.left->isTypeArrow())
    {
        //load the value of the left-hand of the arrow into EXP1
        visit(static_cast<ASTExprArrow&>(*host.left).left, param);
        //visit(host.getLeft(), param);
        //push it onto the stack
        addOpcode(opPUSHR(varExp1()));
    }

    //push the parameters, in forward order
    for (vector<ASTExpr*>::iterator it = host.parameters.begin();
		it != host.parameters.end(); ++it)
    {
        visit(*it, param);
        addOpcode(opPUSHR(varExp1()));
    }

    //goto
    addOpcode(opGOTO(funclabel).withComment(getLineString(host)));
    //pop the stack frame pointer
    addOpcode(opPOP(varSFrame()).withLabel(returnaddr));
}

void BuildOpcodes::caseExprNegate(ASTExprNegate& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(opSETV(varExp1(), *host.getCompileTimeValue(this)));
        return;
    }

    visit(host.operand, param);
    addOpcode(opSETV(varExp2(), 0));
    addOpcode(opSUBR(varExp2(), varExp1()));
    addOpcode(opSETR(varExp1(), varExp2()));
}

void BuildOpcodes::caseExprNot(ASTExprNot& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(opSETV(varExp1(), *host.getCompileTimeValue(this)));
        return;
    }

    visit(host.operand, param);
    addOpcode(opCOMPAREV(varExp1(), 0));
    addOpcode(opSETTRUE(varExp1()));
}

void BuildOpcodes::caseExprBitNot(ASTExprBitNot& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(opSETV(varExp1(), *host.getCompileTimeValue(this)));
        return;
    }

    visit(host.operand, param);
    addOpcode(opBITNOT(varExp1()));
}

void BuildOpcodes::caseExprIncrement(ASTExprIncrement& host, void* param)
{
    OpcodeContext* c = (OpcodeContext*)param;

    // Load value of the variable into EXP1 and push.
	visit(host.operand, param);
    addOpcode(opPUSHR(varExp1()));

    // Increment EXP1
    addOpcode(opADDV(varExp1(), 10000));
	
    // Store it
    LValBOHelper helper(typeStore);
    host.operand->execute(helper, param);
    addOpcodes(helper.getResult());
	
    // Pop EXP1
    addOpcode(opPOP(varExp1()));
}

void BuildOpcodes::caseExprPreIncrement(ASTExprPreIncrement& host, void* param)
{
    OpcodeContext* c = (OpcodeContext*)param;

    // Load value of the variable into EXP1.
	visit(host.operand, param);

    // Increment EXP1
    addOpcode(opADDV(varExp1(), 10000));

    // Store it
    LValBOHelper helper(typeStore);
    host.operand->execute(helper, param);
	addOpcodes(helper.getResult());
}

void BuildOpcodes::caseExprPreDecrement(ASTExprPreDecrement& host, void* param)
{
    OpcodeContext* c = (OpcodeContext*)param;

    // Load value of the variable into EXP1.
	visit(host.operand, param);

    // Decrement EXP1.
    addOpcode(opSUBV(varExp1(), 10000));

    // Store it.
    LValBOHelper helper(typeStore);
    host.operand->execute(helper, param);
	addOpcodes(helper.getResult());
}

void BuildOpcodes::caseExprDecrement(ASTExprDecrement& host, void* param)
{
    OpcodeContext* c = (OpcodeContext*)param;

    // Load value of the variable into EXP1 and push.
	visit(host.operand, param);
    addOpcode(opPUSHR(varExp1()));

    // Decrement EXP1.
    addOpcode(opSUBV(varExp1(), 10000));
    // Store it.
    LValBOHelper helper(typeStore);
    host.operand->execute(helper, param);
	addOpcodes(helper.getResult());

    // Pop EXP1.
    addOpcode(opPOP(varExp1()));
}

void BuildOpcodes::caseExprAnd(ASTExprAnd& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(opSETV(varExp1(), *host.getCompileTimeValue(this)));
        return;
    }

    //compute both sides
    visit(host.left, param);
    addOpcode(opPUSHR(varExp1()));
    visit(host.right, param);
    addOpcode(opPOP(varExp2()));
    castFromBool(result, varExp1());
    castFromBool(result, varExp2());
    addOpcode(opADDR(varExp1(), varExp2()));
    addOpcode(opCOMPAREV(varExp1(), 2));
    addOpcode(opSETMORE(varExp1()));
}

void BuildOpcodes::caseExprOr(ASTExprOr& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(opSETV(varExp1(), *host.getCompileTimeValue(this)));
        return;
    }

    //compute both sides
    visit(host.left, param);
    addOpcode(opPUSHR(varExp1()));
    visit(host.right, param);
    addOpcode(opPOP(varExp2()));
    addOpcode(opADDR(varExp1(), varExp2()));
    addOpcode(opCOMPAREV(varExp1(), 1));
    addOpcode(opSETMORE(varExp1()));
}

void BuildOpcodes::caseExprGT(ASTExprGT& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(opSETV(varExp1(), *host.getCompileTimeValue(this)));
        return;
    }

    //compute both sides
    visit(host.left, param);
    addOpcode(opPUSHR(varExp1()));
    visit(host.right, param);
    addOpcode(opPOP(varExp2()));
    addOpcode(opCOMPARER(varExp2(), varExp1()));
    addOpcode(opSETLESS(varExp1()));
    addOpcode(opCOMPAREV(varExp1(), 0));
    addOpcode(opSETTRUE(varExp1()));
}

void BuildOpcodes::caseExprGE(ASTExprGE& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(opSETV(varExp1(), *host.getCompileTimeValue(this)));
        return;
    }

    //compute both sides
    visit(host.left, param);
    addOpcode(opPUSHR(varExp1()));
    visit(host.right, param);
    addOpcode(opPOP(varExp2()));
    addOpcode(opCOMPARER(varExp2(), varExp1()));
    addOpcode(opSETMORE(varExp1()));
}

void BuildOpcodes::caseExprLT(ASTExprLT& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(opSETV(varExp1(), *host.getCompileTimeValue(this)));
        return;
    }

    //compute both sides
    visit(host.left, param);
    addOpcode(opPUSHR(varExp1()));
    visit(host.right, param);
    addOpcode(opPOP(varExp2()));
    addOpcode(opCOMPARER(varExp2(), varExp1()));
    addOpcode(opSETMORE(varExp1()));
    addOpcode(opCOMPAREV(varExp1(), 0));
    addOpcode(opSETTRUE(varExp1()));
}

void BuildOpcodes::caseExprLE(ASTExprLE& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(opSETV(varExp1(), *host.getCompileTimeValue(this)));
        return;
    }

    // Compute both sides.
    visit(host.left, param);
    addOpcode(opPUSHR(varExp1()));
    visit(host.right, param);
    addOpcode(opPOP(varExp2()));
    addOpcode(opCOMPARER(varExp2(), varExp1()));
    addOpcode(opSETLESS(varExp1()));
}

void BuildOpcodes::caseExprEQ(ASTExprEQ& host, void* param)
{
    // Special case for booleans.
	bool isBoolean =
		(*host.left->getReadType(typeStore) == typeStore.getBool());

    if (host.getCompileTimeValue())
    {
        addOpcode(opSETV(varExp1(), *host.getCompileTimeValue(this)));
        return;
    }

    // Compute both sides.
    visit(host.left, param);
    addOpcode(opPUSHR(varExp1()));
    visit(host.right, param);
    addOpcode(opPOP(varExp2()));

    if (isBoolean)
    {
        castFromBool(result, varExp1());
        castFromBool(result, varExp2());
    }

    addOpcode(opCOMPARER(varExp1(), varExp2()));
    addOpcode(opSETTRUE(varExp1()));
}

void BuildOpcodes::caseExprNE(ASTExprNE& host, void* param)
{
    // Special case for booleans.
    bool isBoolean =
	    (*host.left->getReadType(typeStore) == typeStore.getBool());

    if (host.getCompileTimeValue())
    {
        addOpcode(opSETV(varExp1(), *host.getCompileTimeValue(this)));
        return;
    }

    // Compute both sides.
    visit(host.left, param);
    addOpcode(opPUSHR(varExp1()));
    visit(host.right, param);
    addOpcode(opPOP(varExp2()));

    if (isBoolean)
    {
        castFromBool(result, varExp1());
        castFromBool(result, varExp2());
    }

    addOpcode(opCOMPARER(varExp1(), varExp2()));
    addOpcode(opSETFALSE(varExp1()));
}

void BuildOpcodes::caseExprPlus(ASTExprPlus& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(opSETV(varExp1(), *host.getCompileTimeValue(this)));
        return;
    }

    // Compute both sides.
    visit(host.left, param);
    addOpcode(opPUSHR(varExp1()));
    visit(host.right, param);
    addOpcode(opPOP(varExp2()));
    addOpcode(opADDR(varExp1(), varExp2()));
}

void BuildOpcodes::caseExprMinus(ASTExprMinus& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(opSETV(varExp1(), *host.getCompileTimeValue(this)));
        return;
    }

    // Compute both sides.
    visit(host.left, param);
    addOpcode(opPUSHR(varExp1()));
    visit(host.right, param);
    addOpcode(opPOP(varExp2()));
    addOpcode(opSUBR(varExp2(), varExp1()));
    addOpcode(opSETR(varExp1(), varExp2()));
}

void BuildOpcodes::caseExprTimes(ASTExprTimes& host, void *param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(opSETV(varExp1(), *host.getCompileTimeValue(this)));
        return;
    }

    // Compute both sides.
    visit(host.left, param);
    addOpcode(opPUSHR(varExp1()));
    visit(host.right, param);
    addOpcode(opPOP(varExp2()));
    addOpcode(opMULTR(varExp1(), varExp2()));
}

void BuildOpcodes::caseExprDivide(ASTExprDivide& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(opSETV(varExp1(), *host.getCompileTimeValue(this)));
        return;
    }

    // Compute both sides.
    visit(host.left, param);
    addOpcode(opPUSHR(varExp1()));
    visit(host.right, param);
    addOpcode(opPOP(varExp2()));
    addOpcode(opDIVR(varExp2(), varExp1()));
    addOpcode(opSETR(varExp1(), varExp2()));
}

void BuildOpcodes::caseExprModulo(ASTExprModulo& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(opSETV(varExp1(), *host.getCompileTimeValue(this)));
        return;
    }

    // Compute both sides.
    visit(host.left, param);
    addOpcode(opPUSHR(varExp1()));
    visit(host.right, param);
    addOpcode(opPOP(varExp2()));
    addOpcode(opMODR(varExp2(), varExp1()));
    addOpcode(opSETR(varExp1(), varExp2()));
}

void BuildOpcodes::caseExprBitAnd(ASTExprBitAnd& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(opSETV(varExp1(), *host.getCompileTimeValue(this)));
        return;
    }

    // Compute both sides.
    visit(host.left, param);
    addOpcode(opPUSHR(varExp1()));
    visit(host.right, param);
    addOpcode(opPOP(varExp2()));
    addOpcode(opANDR(varExp1(), varExp2()));
}

void BuildOpcodes::caseExprBitOr(ASTExprBitOr& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(opSETV(varExp1(), *host.getCompileTimeValue(this)));
        return;
    }

    // Compute both sides.
    visit(host.left, param);
    addOpcode(opPUSHR(varExp1()));
    visit(host.right, param);
    addOpcode(opPOP(varExp2()));
    addOpcode(opORR(varExp1(), varExp2()));
}

void BuildOpcodes::caseExprBitXor(ASTExprBitXor& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(opSETV(varExp1(), *host.getCompileTimeValue(this)));
        return;
    }

    // Compute both sides.
    visit(host.left, param);
    addOpcode(opPUSHR(varExp1()));
    visit(host.right, param);
    addOpcode(opPOP(varExp2()));
    addOpcode(opXORR(varExp1(), varExp2()));
}

void BuildOpcodes::caseExprLShift(ASTExprLShift& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(opSETV(varExp1(), *host.getCompileTimeValue(this)));
        return;
    }

    // Compute both sides.
    visit(host.left, param);
    addOpcode(opPUSHR(varExp1()));
    visit(host.right, param);
    addOpcode(opPOP(varExp2()));
    addOpcode(opLSHIFTR(varExp2(), varExp1()));
    addOpcode(opSETR(varExp1(), varExp2()));
}

void BuildOpcodes::caseExprRShift(ASTExprRShift& host, void* param)
{
    if (host.getCompileTimeValue())
    {
        addOpcode(opSETV(varExp1(), *host.getCompileTimeValue(this)));
        return;
    }

    // Compute both sides.
    visit(host.left, param);
    addOpcode(opPUSHR(varExp1()));
    visit(host.right, param);
    addOpcode(opPOP(varExp2()));
    addOpcode(opRSHIFTR(varExp2(), varExp1()));
    addOpcode(opSETR(varExp1(), varExp2()));
}

// Literals

void BuildOpcodes::caseNumberLiteral(ASTNumberLiteral& host, void*)
{
    if (host.getCompileTimeValue())
        addOpcode(opSETV(varExp1(), *host.getCompileTimeValue(this)));
    else
    {
        pair<long, bool> val = ScriptParser::parseLong(host.value->parseValue());

        if (!val.second)
            handleError(CompileError::ConstTrunc, &host,
						host.value->value);

        addOpcode(opSETV(varExp1(), val.first));
    }
}

void BuildOpcodes::caseBoolLiteral(ASTBoolLiteral& host, void*)
{
    addOpcode(opSETV(varExp1(), *host.getCompileTimeValue(this)));
}

// TODO add implicit stackframe to global case for literals.
void BuildOpcodes::caseStringLiteral(ASTStringLiteral& host, void* param)
{
	OpcodeContext* c = (OpcodeContext*)param;
	int id = host.manager->id;

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
	c->initCode.push_back(opALLOCATEMEMV(varExp1(), size));
	int offset = 10000L * *getStackOffset(*host.manager);
	c->initCode.push_back(opSETR(varSFTemp(), varSFrame()));
	c->initCode.push_back(opADDV(varSFTemp(), offset));
	c->initCode.push_back(opSTOREI(varExp1(), varSFTemp()));

	// Initialize.
	c->initCode.push_back(opSETR(varIndex1(), varExp1()));
	for (int i = 0; i < (int)data.size(); ++i)
	{
		c->initCode.push_back(opSETV(varIndex2(), i * 10000L));
		long value = data[i] * 10000L;
		c->initCode.push_back(opSETV(varSCRIPTRAM(), value));
	}
	c->initCode.push_back(opSETV(varIndex2(), data.size() * 10000L));
	c->initCode.push_back(opSETV(varSCRIPTRAM(), 0));

	////////////////////////////////////////////////////////////////
	// Actual Code.

	// Local variable, get its value from the stack.
	addOpcode(opSETR(varSFTemp(), varSFrame()));
	addOpcode(opADDV(varSFTemp(), offset));
	addOpcode(opLOADI(varExp1(), varSFTemp()));

	////////////////////////////////////////////////////////////////
	// Register for cleanup.

	arrayRefs.push_back(offset);
}

void BuildOpcodes::caseArrayLiteral(ASTArrayLiteral& host, void* param)
{
	OpcodeContext& context = *(OpcodeContext*)param;
	Literal& manager = *host.manager;

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

	int offset = 10000L * *getStackOffset(manager);
	
	////////////////////////////////////////////////////////////////
	// Initialization Code.

	// Allocate.
	context.initCode.push_back(opALLOCATEMEMV(varExp1(), size * 10000L));
	context.initCode.push_back(opSETR(varSFTemp(), varSFrame()));
	context.initCode.push_back(opADDV(varSFTemp(), offset));
	context.initCode.push_back(opSTOREI(varExp1(), varSFTemp()));

	// Initialize.
	context.initCode.push_back(opSETR(varIndex1(), varExp1()));
	long i = 0;
	vector<ASTExpr*> elements = host.elements;
	for (vector<ASTExpr*>::iterator it = elements.begin();
		 it != elements.end(); ++it, i += 10000L)
	{
		context.initCode.push_back(opPUSHR(varIndex1()));
		opcodeTargets.push_back(&context.initCode);
		visit(*it, param);
		opcodeTargets.pop_back();
		context.initCode.push_back(opPOP(varIndex1()));
		context.initCode.push_back(opSETV(varIndex2(), i));
		context.initCode.push_back(opSETR(varSCRIPTRAM(), varExp1()));
	}

	////////////////////////////////////////////////////////////////
	// Actual Code.

	// Local variable, get its value from the stack.
	addOpcode(opSETR(varSFTemp(), varSFrame()));
	addOpcode(opADDV(varSFTemp(), offset));
	addOpcode(opLOADI(varExp1(), varSFTemp()));

	////////////////////////////////////////////////////////////////
	// Register for cleanup.

	arrayRefs.push_back(offset);
}

// Other

void BuildOpcodes::castFromBool(
		vector<Opcode>& res, ZAsm::Variable const& var)
{
	res.push_back(opCOMPAREV(var, 0));
	res.push_back(opSETFALSE(var));
}

/////////////////////////////////////////////////////////////////////////////////
// LValBOHelper

void LValBOHelper::caseDefault(void *)
{
    //Shouldn't happen
    assert(false);
}

void LValBOHelper::addOpcode(Opcode const& code)
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
    addOpcode(opSETR(varSFTemp(), varSFrame()));
    addOpcode(opADDV(varSFTemp(), offset));
    addOpcode(opSTOREI(varExp1(), varSFTemp()));
}
*/

void LValBOHelper::caseExprIdentifier(ASTExprIdentifier& host, void* param)
{
    OpcodeContext* c = (OpcodeContext*)param;
    int vid = host.binding->id;

    if (optional<int> globalId = host.binding->getGlobalId())
    {
        // Global variable.
        addOpcode(opSETR(varGD(*globalId), varExp1()));
        return;
    }

    // Set the stack.
    int offset = 10000L * *getStackOffset(*host.binding);

    addOpcode(opSETR(varSFTemp(), varSFrame()));
    addOpcode(opADDV(varSFTemp(), offset));
    addOpcode(opSTOREI(varExp1(), varSFTemp()));
}

void LValBOHelper::caseExprArrow(ASTExprArrow &host, void *param)
{
    OpcodeContext *c = (OpcodeContext *)param;
    int isIndexed = (host.index != NULL);
    // This is actually implemented as a settor function call.

    // Push the stack frame.
    addOpcode(opPUSHR(varSFrame()));

    int returnlabel = ScriptParser::getUniqueLabelID();
    //push the return address
    addOpcode(opSETV(varExp2(), Label(returnlabel)));
    addOpcode(opPUSHR(varExp2()));
    //push the lhs of the arrow
    //but first save the value of EXP1
    addOpcode(opPUSHR(varExp1()));

    BuildOpcodes oc(typeStore);
    oc.visit(host.left, param);
	addOpcodes(oc.getResult());
    
    //pop the old value of EXP1
    addOpcode(opPOP(varExp2()));
    //and push the lhs
    addOpcode(opPUSHR(varExp1()));
    //and push the old value of EXP1
    addOpcode(opPUSHR(varExp2()));
    
    //and push the index, if indexed
    if(isIndexed)
    {
	    BuildOpcodes oc2(typeStore);
        oc2.visit(host.index, param);
		addOpcodes(oc2.getResult());
        addOpcode(opPUSHR(varExp1()));
    }
    
    //finally, goto!
    int label = host.writeFunction->getLabel();
    addOpcode(opGOTO(label));

    // Pop the stack frame
    addOpcode(opPOP(varSFrame()).withLabel(returnlabel));
}

void LValBOHelper::caseExprIndex(ASTExprIndex& host, void* param)
{
	// Arrows just fall back on the arrow implementation.
	if (host.array->isTypeArrow())
	{
		caseExprArrow(static_cast<ASTExprArrow&>(*host.array), param);
		return;
	}

	vector<Opcode> opcodes;

	// Push the value.
    addOpcode(opPUSHR(varExp1()));

	// Get and push the array pointer.
    BuildOpcodes buildOpcodes1(typeStore);
	buildOpcodes1.visit(host.array, param);
	opcodes = buildOpcodes1.getResult();
	for (vector<Opcode>::iterator it = opcodes.begin();
	     it != opcodes.end(); ++it)
		addOpcode(*it);
    addOpcode(opPUSHR(varExp1()));

	// Get the index.
    BuildOpcodes buildOpcodes2(typeStore);
	buildOpcodes2.visit(host.index, param);
	opcodes = buildOpcodes2.getResult();
	for (vector<Opcode>::iterator it = opcodes.begin();
	     it != opcodes.end(); ++it)
		addOpcode(*it);

	// Setup array indices.
    addOpcode(opPOP(varIndex1()));
    addOpcode(opSETR(varIndex2(), varExp1()));

	// Pop and assign the value.
	//   (As far as I can tell, there's no difference between GLOBALRAM and
	//    SCRIPTRAM, so I'll use GLOBALRAM here instead of checking.)
    addOpcode(opPOP(varExp1())); // Pop the value
    addOpcode(opSETR(varGLOBALRAM(), varExp1()));
}
