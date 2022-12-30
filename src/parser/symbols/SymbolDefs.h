#ifndef _ZSCRIPT_SYMBOLS_H
#define _ZSCRIPT_SYMBOLS_H

#include "../../precompiled.h" //always first
#include <assert.h>
#include "../../zsyssimple.h"
#include "../ByteCode.h"
#include "../base/zdefs.h" //Putting this higher has issues?
#include "../parserDefs.h"
#include "../Scope.h"
#include "../ZScript.h"
#include "../CompilerUtils.h"
#include "../LibrarySymbols.h"
using namespace ZScript;
using std::shared_ptr;

#define MAXDMAPS         512
#define MAXLEVELS        512

#define FL_ARR     FUNCFLAG_INTARRAY
#define FL_INL     FUNCFLAG_INLINE
#define FL_VARG    FUNCFLAG_VARARGS
#define FL_DEPR    FUNCFLAG_DEPRECATED
#define FL_NOCAST  FUNCFLAG_NOCAST


void addOpcode2(std::vector<std::shared_ptr<Opcode>>& v, Opcode* code);

//{ Defines
/*
	Pop to refVar, unless refVar is NUL.
	If refVar is NUL, instead set the IFUNCFLAG_SKIPPOINTER
*/
#define POPREF() \
if(refVar == NUL) \
{ \
	function->internal_flags |= IFUNCFLAG_SKIPPOINTER; \
} \
else \
	addOpcode2 (code, new OPopRegister(new VarArgument(refVar)))

/*
	Assert that the refVar IS NUL.
	Set the IFUNCFLAG_SKIPPOINTER.
*/
#define ASSERT_NUL() \
assert(refVar == NUL); \
function->internal_flags |= IFUNCFLAG_SKIPPOINTER

/*
	Assert that the refVar is NON-NUL.
*/
#define ASSERT_NON_NUL() \
assert(refVar != NUL)

/*
	Forces any function to be inline where inline gives strict gain; i.e. has no negative effects.
	Presently, this is defined as any function with < 5 opcodes, before adding 'RETURN'.
*/
#define INLINE_CHECK() \
if(code.size() < 5) function->setFlag(FUNCFLAG_INLINE)

/*
	Return from the function. Automatically skips OReturn() on inline functions.
*/
#define RETURN() \
INLINE_CHECK(); \
if(!(function->getFlag(FUNCFLAG_INLINE))) \
	addOpcode2 (code, new OReturn())

/*
	Adds the label passed to the back of the 'code' vector<shared_ptr<Opcode>>
*/
#define LABELBACK(LBL) \
code.back()->setLabel(LBL)

/*
	Reassigns the pointer that was referenced to the passed register.
*/
#define REASSIGN_PTR(reg) \
ASSERT_NON_NUL(); \
if(reg!=EXP2) addOpcode2 (code, new OSetRegister(new VarArgument(EXP2), new VarArgument(reg))); \
function->internal_flags |= IFUNCFLAG_REASSIGNPTR

/*
	Pop multiple args to 1 register; mostly used to clear the stack after drawing commands.
*/
#define POP_ARGS(num_args, t) \
	addOpcode2 (code, new OPopArgsRegister(new VarArgument(t), new LiteralArgument(num_args)))

#define POP_ARG(t) \
	addOpcode2 (code, new OPopRegister(new VarArgument(t)))

//}

#endif

