#ifndef BYTECODE_H_
#define BYTECODE_H_

#include "zasm/defines.h"
#include "parser/Opcode.h"

/*
 SP - stack pointer
 D4 - stack frame pointer
 D6 - stack frame offset accumulator
 D2 - expression accumulator #1
 D3 - expression accumulator #2
 D0 - array index accumulator
 D1 - secondary array index accumulator
 D5 - pure SETR sink
 */
#define INDEX                   rINDEX
#define INDEX2                  rINDEX2
#define EXP1                    rEXP1
#define EXP2                    rEXP2
#define SFRAME                  rSFRAME
#define NUL                     rNUL
#define SFTEMP                  rSFTEMP
#define WHAT_NO_7               rWHAT_NO_7

namespace ZScript {
	class ONoOp : public Opcode
	{
	public:
		ONoOp() = default;
		ONoOp(int lbl) : ONoOp()
		{
			setLabel(lbl);
		}
		std::string toString() const {return "NOP";}
		Opcode* clone() const
		{
			return new ONoOp();
		}
	};

	// Please keep grouped by number of arguments, and sorted.

	typedef Opcode0<QUIT> OQuit;
	typedef Opcode0<RETURNFUNC> OReturnFunc;
	
	typedef Opcode1<ABS, VarArgument> OAbsRegister;
	typedef Opcode1<ARRAYSIZE, VarArgument> OArraySize;
	typedef Opcode1<BITNOT, VarArgument> ONot;
	typedef Opcode1<BITNOT32, VarArgument> O32BitNot;
	typedef Opcode1<CALLFUNC, LabelArgument> OCallFunc;
	typedef Opcode1<CASTBOOLF, VarArgument> OCastBoolF;
	typedef Opcode1<CASTBOOLI, VarArgument> OCastBoolI;
	typedef Opcode1<GOTO, LabelArgument> OGotoImmediate;
	typedef Opcode1<GOTOFALSE, LabelArgument> OGotoFalseImmediate;
	typedef Opcode1<GOTOLESS, LabelArgument> OGotoLessImmediate;
	typedef Opcode1<GOTOMORE, LabelArgument> OGotoMoreImmediate;
	typedef Opcode1<GOTOR, LabelArgument> OGotoRegister;
	typedef Opcode1<GOTOTRUE, LabelArgument> OGotoTrueImmediate;
	typedef Opcode1<MAKEVARGARRAY, LiteralArgument> OMakeVargArray;
	typedef Opcode1<PEEK, VarArgument> OPeek;
	typedef Opcode1<POP, VarArgument> OPopRegister;
	typedef Opcode1<PUSHR, VarArgument> OPushRegister;
	typedef Opcode1<PUSHV, LiteralArgument> OPushImmediate;
	typedef Opcode1<PUSHVARGR, VarArgument> OPushVargR;
	typedef Opcode1<PUSHVARGV, LiteralArgument> OPushVargV;
	typedef Opcode1<REF_AUTORELEASE, VarArgument> ORefAutorelease;
	typedef Opcode1<REF_INC, LiteralArgument> ORefInc;
	typedef Opcode1<REF_REMOVE, LiteralArgument> ORefRemove;
	typedef Opcode1<STARTDESTRUCTOR, StringArgument> ODestructor;
	typedef Opcode1<TRACER, VarArgument> OTraceRegister;
	typedef Opcode1<TRACEV, LiteralArgument> OTraceImmediate;
	typedef Opcode1<TRUNCATE, VarArgument> OTruncate;
	typedef Opcode1<ZCLASS_FREE, VarArgument> OFreeObject;
	typedef Opcode1<ZCLASS_MARK_TYPE, VectorArgument> OMarkTypeClass;

	typedef Opcode2<ADDR, VarArgument, VarArgument> OAddRegister;
	typedef Opcode2<ADDV, VarArgument, LiteralArgument> OAddImmediate;
	typedef Opcode2<ANDR, VarArgument, VarArgument> OAndRegister;
	typedef Opcode2<ANDR32, VarArgument, VarArgument> O32BitAndRegister;
	typedef Opcode2<ANDV, VarArgument, LiteralArgument> OAndImmediate;
	typedef Opcode2<ANDV32, VarArgument, LiteralArgument> O32BitAndImmediate;
	typedef Opcode2<COMPARER, VarArgument, VarArgument> OCompareRegister;
	typedef Opcode2<COMPAREV, VarArgument, LiteralArgument> OCompareImmediate;
	typedef Opcode2<COMPAREV2, LiteralArgument, VarArgument> OCompareImmediate2;
	typedef Opcode2<DIVR, VarArgument, VarArgument> ODivRegister;
	typedef Opcode2<DIVV, VarArgument, LiteralArgument> ODivImmediate;
	typedef Opcode2<DIVV2, LiteralArgument, VarArgument> ODivImmediate2;
	typedef Opcode2<GOTOCMP, LabelArgument, CompareArgument> OGotoCompare;
	typedef Opcode2<LOAD_INTERNAL_ARRAY, VarArgument, LiteralVarArgument> OLoadInternalArray;
	typedef Opcode2<LOAD, VarArgument, LiteralArgument> OLoad;
	typedef Opcode2<LPOWERR, VarArgument, VarArgument> OLPowRegister;
	typedef Opcode2<LPOWERV, VarArgument, LiteralArgument> OLPowImmediate;
	typedef Opcode2<LPOWERV2, LiteralArgument, VarArgument> OLPowImmediate2;
	typedef Opcode2<LSHIFTR, VarArgument, VarArgument> OLShiftRegister;
	typedef Opcode2<LSHIFTR32, VarArgument, VarArgument> O32BitLShiftRegister;
	typedef Opcode2<LSHIFTV, VarArgument, LiteralArgument> OLShiftImmediate;
	typedef Opcode2<LSHIFTV32, VarArgument, LiteralArgument> O32BitLShiftImmediate;
	typedef Opcode2<MARK_TYPE_REG, GlobalArgument, LiteralArgument> OMarkTypeRegister;
	typedef Opcode2<MARK_TYPE_STACK, LiteralArgument, LiteralArgument> OMarkTypeStack;
	typedef Opcode2<MODR, VarArgument, VarArgument> OModuloRegister;
	typedef Opcode2<MODV, VarArgument, LiteralArgument> OModuloImmediate;
	typedef Opcode2<MODV2, LiteralArgument, VarArgument> OModuloImmediate2;
	typedef Opcode2<MULTR, VarArgument, VarArgument> OMultRegister;
	typedef Opcode2<MULTV, VarArgument, LiteralArgument> OMultImmediate;
	typedef Opcode2<ORR, VarArgument, VarArgument> OOrRegister;
	typedef Opcode2<ORR32, VarArgument, VarArgument> O32BitOrRegister;
	typedef Opcode2<ORV, VarArgument, LiteralArgument> OOrImmediate;
	typedef Opcode2<ORV32, VarArgument, LiteralArgument> O32BitOrImmediate;
	typedef Opcode2<PEEKATV, VarArgument, LiteralArgument> OPeekAtImmediate;
	typedef Opcode2<POPARGS, VarArgument, LiteralArgument> OPopArgsRegister;
	typedef Opcode2<POWERR, VarArgument, VarArgument> OPowRegister;
	typedef Opcode2<POWERV, VarArgument, LiteralArgument> OPowImmediate;
	typedef Opcode2<POWERV2, LiteralArgument, VarArgument> OPowImmediate2;
	typedef Opcode2<PUSHARGSR, VarArgument, LiteralArgument> OPushArgsRegister;
	typedef Opcode2<PUSHARGSV, LiteralArgument, LiteralArgument> OPushArgsImmediate;
	typedef Opcode2<PUSHVARGSR, VarArgument, LiteralArgument> OPushVargsR;
	typedef Opcode2<PUSHVARGSV, LiteralArgument, LiteralArgument> OPushVargsV;
	typedef Opcode2<READPODARRAYR, VarArgument, VarArgument> OReadPODArrayR;
	typedef Opcode2<READPODARRAYV, VarArgument, LiteralArgument> OReadPODArrayI;
	typedef Opcode2<RSHIFTR, VarArgument, VarArgument> ORShiftRegister;
	typedef Opcode2<RSHIFTR32, VarArgument, VarArgument> O32BitRShiftRegister;
	typedef Opcode2<RSHIFTV, VarArgument, LiteralArgument> ORShiftImmediate;
	typedef Opcode2<RSHIFTV32, VarArgument, LiteralArgument> O32BitRShiftImmediate;
	typedef Opcode2<SET_OBJECT, VarArgument, VarArgument> OSetObject;
	typedef Opcode2<SETCMP, VarArgument, CompareArgument> OSetCompare;
	typedef Opcode2<SETR, VarArgument, VarArgument> OSetRegister;
	typedef Opcode2<SETV, VarArgument, LabelArgument> OSetImmediateLabel;
	typedef Opcode2<SETV, VarArgument, LiteralArgument> OSetImmediate;
	typedef Opcode2<STACKWRITEATRV, VarArgument, LiteralArgument> OStackWriteAtRV;
	typedef Opcode2<STACKWRITEATVV, LiteralArgument, LiteralArgument> OStackWriteAtVV;
	typedef Opcode2<STORE_OBJECT, VarArgument, LiteralArgument> OStoreObject;
	typedef Opcode2<STORE, VarArgument, LiteralArgument> OStore;
	typedef Opcode2<STORED, VarArgument, VarArgument> OStoreDirect;
	typedef Opcode2<STOREV, LiteralArgument, LiteralArgument> OStoreV;
	typedef Opcode2<STRCMPR, VarArgument, VarArgument> OInternalStringCompare;
	typedef Opcode2<STRICMPR, VarArgument, VarArgument> OInternalInsensitiveStringCompare;
	typedef Opcode2<SUBR, VarArgument, VarArgument> OSubRegister;
	typedef Opcode2<SUBV, VarArgument, LiteralArgument> OSubImmediate;
	typedef Opcode2<SUBV2, LiteralArgument, VarArgument> OSubImmediate2;
	typedef Opcode2<WRITEPODARRAY, VarArgument, VectorArgument> OWritePODArray;
	typedef Opcode2<WRITEPODARRAYRR, VarArgument, VarArgument> OWritePODArrayRR;
	typedef Opcode2<WRITEPODARRAYVR, LiteralArgument, VarArgument> OWritePODArrayIR;
	typedef Opcode2<WRITEPODSTRING, VarArgument, StringArgument> OWritePODString;
	typedef Opcode2<XORR, VarArgument, VarArgument> OXorRegister;
	typedef Opcode2<XORR32, VarArgument, VarArgument> O32BitXorRegister;
	typedef Opcode2<XORV, VarArgument, LiteralArgument> OXorImmediate;
	typedef Opcode2<XORV32, VarArgument, LiteralArgument> O32BitXorImmediate;
	typedef Opcode2<ZCLASS_CONSTRUCT, VarArgument, VectorArgument> OConstructClass;
	typedef Opcode2<ZCLASS_READ, VarArgument, LiteralArgument> OReadObject;
	typedef Opcode2<ZCLASS_WRITE, VarArgument, LiteralArgument> OWriteObject;

	typedef Opcode3<ALLOCATEGMEMV, VarArgument, LiteralArgument, LiteralArgument> OAllocateGlobalMemImmediate;
	typedef Opcode3<ALLOCATEMEMV, VarArgument, LiteralArgument, LiteralArgument> OAllocateMemImmediate;
	typedef Opcode3<LOAD_INTERNAL_ARRAY_REF, VarArgument, LiteralVarArgument, VarArgument> OLoadInternalArrayRef;
	typedef Opcode3<STACKWRITEATVV_IF, LiteralArgument, LiteralArgument, CompareArgument> OStackWriteAtVV_If;
}

#endif
