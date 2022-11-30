#ifndef _ZSCRIPT_SYMBOLS_H
#define _ZSCRIPT_SYMBOLS_H

#include "../../precompiled.h" //always first
#include <assert.h>
#include "../../zsyssimple.h"
#include "../ByteCode.h"
#include "../base/zdefs.h" //Putting this higher has issues?
#include "../parserdefs.h"
#include "../Scope.h"
#include "../ZScript.h"
#include "../CompilerUtils.h"
#include "../LibrarySymbols.h"
using namespace ZScript;
using std::shared_ptr;

#define MAXDMAPS         512
#define MAXLEVELS        512

//sanity underflow
#define typeVOID ZVARTYPEID_VOID
#define S ZVARTYPEID_SCREEN
#define F ZVARTYPEID_FLOAT

#define tV ZVARTYPEID_VOID
#define tFF ZVARTYPEID_FFC
#define tF ZVARTYPEID_FLOAT
#define tN ZVARTYPEID_NPC
#define tB ZVARTYPEID_BOOL
#define tI ZVARTYPEID_ITEM
#define tID ZVARTYPEID_ITEMCLASS
#define tE ZVARTYPEID_EWEAPON
#define tL ZVARTYPEID_LWEAPON
#define tLI ZVARTYPEID_PLAYER
#define tG ZVARTYPEID_GAME
#define tS ZVARTYPEID_SCREEN

void addOpcode2(std::vector<std::shared_ptr<Opcode>>& v, Opcode* code);
//New Types

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
	Set the IFUNCFLAG_SKIPPOINTER.
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

//{ Older defines
#define ARGS_4(t, arg1, arg2, arg3, arg4) \
	{ t, arg1, arg2, arg3, arg4, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }
#define ARGS_6(t, arg1, arg2, arg3, arg4, arg5, arg6) \
	{ t, arg1, arg2, arg3, arg4, arg5, arg6, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }
#define ARGS_8(t, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) \
	{ t, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }
#define ARGS_12(t, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12) { t, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8,arg9,arg10,arg11,arg12,-1,-1,-1,-1,-1,-1,-1 }
#define ARGS_13(t, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13) { t, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8,arg9,arg10,arg11,arg12,arg13,-1,-1,-1,-1,-1,-1 }
#define ARGS_15(t, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15) { t, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8,arg9,arg10,arg11,arg12,arg13,arg14,arg15,-1,-1,-1,-1 }

#define ARGS_16(t, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16) { t, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8,arg9,arg10,arg11,arg12,arg13,arg14,arg15,arg16,-1,-1,-1 }

#define ARGS_17(t, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17) { t, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8,arg9,arg10,arg11,arg12,arg13,arg14,arg15,arg16,arg17,-1,-1 }

#define SetIdentifier(label) \
{ \
	Function* function = functions[label]; \
	int32_t label = function->getLabel(); \
	vector<shared_ptr<Opcode>> code; \
} \

#define IgnorePointer() \
{ \
	addOpcode2 (code, new OPopRegister(new VarArgument(NUL))); \
} \

#define PopFirstArg(reg_index) \
{ \
	addOpcode2 (code, new OPopRegister(new VarArgument(reg_index))); \
	LABELBACK(label); \
} \

#define SetRefVar(reg_index, ref_index2, ref_var) \
{ \
	addOpcode2 (code, new OSetRegister(new VarArgument(reg_index), new VarArgument(ref_var))); \
	addOpcode2 (code, new OPopRegister(new VarArgument(ref_index2))); \
} \

#define AddArgument(register_index) \
{ \
	addOpcode2 (code, new OPopRegister(new VarArgument(register_index))); \
} \

#define InstructionToRegister(instruction, reg_index) \
{ \
	addOpcode2 (code, new OSetRegister(new VarArgument(instruction), new VarArgument(reg_index))); \
} \

#define OpcodeToRegister(ocode, reg_index) \
{ \
	addOpcode2 (code, new ocode(new VarArgument(reg_index))); \
} \

//Loads a refvaar to reg_index, then pops it to reg_index2

//LoadRefData
#define LOAD_REFDATA(flabel, ffins, ref_var, numparam) \
{ \
        Function* function = getFunction(flabel, numparam); \
        int32_t label = function->getLabel(); \
        vector<shared_ptr<Opcode>> code; \
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1))); \
        LABELBACK(label); \
        POPREF(); \
        addOpcode2 (code, new ffins(new VarArgument(EXP1))); \
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(ref_var))); \
        RETURN(); \
        function->giveCode(code); \
} \
    
//Guydata member with one input, one return
#define GET_GUYDATA_MEMBER(flabel, ffins) \
{ \
	Function* function = getFunction(flabel, 2);\
	int32_t label = function->getLabel(); \
	vector<shared_ptr<Opcode>> code; \
	addOpcode2 (code, new OPopRegister(new VarArgument(EXP2))); \
	LABELBACK(label); \
	POPREF(); \
	addOpcode2 (code, new ffins(new VarArgument(EXP1),new VarArgument(EXP2))); \
	RETURN(); \
	function->giveCode(code); \
} \

//Dataclass member with three inputs
#define SET_DATACLASS_MEMBER(flabel, ffins) \
{ \
	Function* function = getFunction(flabel, 4); \
	int32_t label = function->getLabel(); \
	vector<shared_ptr<Opcode>> code; \
	addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP))); \
	LABELBACK(label); \
	addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2))); \
	addOpcode2 (code, new OPopRegister(new VarArgument(INDEX))); \
	POPREF(); \
	addOpcode2 (code, new OSetRegister(new VarArgument(ffins), new VarArgument(SFTEMP))); \
	RETURN(); \
	function->giveCode(code); \
} \

//Dataclass Member with four inputs (int32_t, int32_t, int32_t, int32_t)
#define SET_DATACLASS_ARRAY(flabel, ffins) \
{ \
	Function* function = getFunction(flabel, 5); \
	int32_t label = function->getLabel(); \
	vector<shared_ptr<Opcode>> code; \
	addOpcode2 (code, new OPopRegister(new VarArgument(EXP2))); \
	LABELBACK(label); \
	addOpcode2 (code, new OPopRegister(new VarArgument(INDEX))); \
	addOpcode2 (code, new OPopRegister(new VarArgument(EXP1))); \
	addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2))); \
	POPREF(); \
	addOpcode2 (code, new OSetRegister(new VarArgument(ffins), new VarArgument(EXP2))); \
	RETURN(); \
	function->giveCode(code); \
} \

//Dataclass member with two inputs, one return 
#define GET_DATACLASS_MEMBER(flabel, ffins) \
{ \
	Function* function = getFunction(flabel, 3); \
	int32_t label = function->getLabel(); \
	vector<shared_ptr<Opcode>> code; \
	addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2))); \
	LABELBACK(label); \
	addOpcode2 (code, new OPopRegister(new VarArgument(INDEX))); \
	POPREF(); \
	addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(ffins))); \
	RETURN(); \
	function->giveCode(code); \
} \

//Dataclass member with three inputs, one return
#define GET_DATACLASS_ARRAY(flabel, ocode) \
{ \
	Function* function = getFunction(flabel, 4); \
	int32_t label = function->getLabel(); \
	vector<shared_ptr<Opcode>> code; \
	addOpcode2 (code, new OPopRegister(new VarArgument(INDEX))); \
	LABELBACK(label); \
	addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2))); \
	addOpcode2 (code, new OPopRegister(new VarArgument(EXP1))); \
	POPREF(); \
	addOpcode2 (code, new ocode(new VarArgument(EXP1))); \
	RETURN(); \
	function->giveCode(code); \
} \

//void function(int32_t) -- follows pattern of cleartile() and trace()
#define ONE_INPUT_NO_RETURN(flabel, ocode) \
{ \
        id = getFunction(flabel, 1)->id; \
        int32_t label = function->getLabel(); \
        vector<shared_ptr<Opcode>> code; \
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2))); \
        LABELBACK(label); \
        addOpcode2 (code, new ocode(new VarArgument(EXP2))); \
	RETURN(); \
        function->giveCode(code); \
} \

//int32_t function(int32_t)
#define ONE_INPUT_ONE_RETURN(flabel, ocode) \
{ \
	Function* function = getFunction(flabel, 2); \
	int32_t label = function->getLabel(); \
        vector<shared_ptr<Opcode>> code; \
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1))); \
        LABELBACK(label); \
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2))); \
        POPREF(); \
        addOpcode2 (code, new ocode(new VarArgument(EXP1),new VarArgument(EXP2))); \
	RETURN(); \
        function->giveCode(code); \
} \


//void function(int32_t, int32_t)
#define TWO_INPUT_NO_RETURN(flabel, ocode) \
{ \
	Function* function = getFunction(flabel, 3); \
        int32_t label = function->getLabel(); \
        vector<shared_ptr<Opcode>> code; \
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2))); \
        LABELBACK(label); \
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2))); \
        addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP))); \
        addOpcode2 (code, new ocode(new VarArgument(EXP2), new VarArgument(EXP1))); \
        RETURN();                                  \
        function->giveCode(code); \
} \
    
	
	
//int32_t function(int32_t, int32_t)
#define TWO_INPUT_ONE_RETURN(flabel, ocode) \
{ \
        Function* function = getFunction(flabel, 3); \
        int32_t label = function->getLabel(); \
        vector<shared_ptr<Opcode>> code; \
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2))); \
        LABELBACK(label); \
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX))); \
        POPREF(); \
        addOpcode2 (code, new ocode(new VarArgument(EXP1))); \
        RETURN();                    \
        function->giveCode(code); \
} \

//void function(int32_t, int32_t, int32_t)
#define THREE_INPUT_NO_RETURN(flabel,zasmid) \
{ \
        Function* function = getFunction(flabel, 4); \
        int32_t label = function->getLabel(); \
        vector<shared_ptr<Opcode>> code; \
        addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP))); \
        LABELBACK(label); \
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2))); \
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX))); \
        POPREF(); \
        addOpcode2 (code, new OSetRegister(new VarArgument(zasmid), new VarArgument(SFTEMP))); \
        RETURN();                                  \
        function->giveCode(code); \
} \

//Three Inputs, One Return -- based on int32_t GetScreenEFlags(int32_t map, int32_t screen, int32_t flagset);	
#define THREE_INPUTS_ONE_RETURN(flabel, ocode, numparam) \
{ \
	Function* function = getFunction(flabel, numparam); \
	int32_t label = function->getLabel(); \
	vector<shared_ptr<Opcode>> code; \
	addOpcode2 (code, new OPopRegister(new VarArgument(INDEX))); \
	LABELBACK(label); \
	addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2))); \
	addOpcode2 (code, new OPopRegister(new VarArgument(EXP1))); \
	POPREF(); \
	addOpcode2 (code, new ocode(new VarArgument(EXP1))); \
	RETURN(); \
	function->giveCode(code); \
} \

//This will trace the float value of any pointer type to allegro.log. 
#define TRACING_FUNCTION(flabel, numparam) \
{ \
	Function* function = getFunction(flabel, numparam); \
	int32_t label = function->getLabel(); \
	vector<shared_ptr<Opcode>> code; \
	addOpcode2 (code, new OPopRegister(new VarArgument(EXP2))); \
	LABELBACK(label); \
	addOpcode2 (code, new OTraceRegister(new VarArgument(EXP2))); \
	RETURN(); \
	function->giveCode(code); \
} \

//}
//}

#endif

