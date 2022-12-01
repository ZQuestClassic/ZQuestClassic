#include "SymbolDefs.h"

RNGSymbols RNGSymbols::singleton = RNGSymbols();

static AccessorTable RNGTable[] =
{
//	  name,                     rettype,                     setorget,     var,   numindex,   funcFlags,                            numParams,   params
//	{ "DirExists",              ZTID_BOOL,             FUNCTION,     0,     1,          0,                                    2,           { ZTID_FILESYSTEM, ZTID_CHAR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Rand",                   ZTID_FLOAT,            FUNCTION,     0,     1,          FUNCFLAG_INLINE,                      1,           { ZTID_RNG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Rand",                   ZTID_FLOAT,            FUNCTION,     0,     1,          FUNCFLAG_INLINE,                      2,           { ZTID_RNG, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Rand",                   ZTID_FLOAT,            FUNCTION,     0,     1,          FUNCFLAG_INLINE,                      3,           { ZTID_RNG, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LRand",                  ZTID_LONG,             FUNCTION,     0,     1,          FUNCFLAG_INLINE,                      1,           { ZTID_RNG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LRand",                  ZTID_LONG,             FUNCTION,     0,     1,          FUNCFLAG_INLINE,                      2,           { ZTID_RNG, ZTID_LONG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LRand",                  ZTID_LONG,             FUNCTION,     0,     1,          FUNCFLAG_INLINE,                      3,           { ZTID_RNG, ZTID_LONG, ZTID_LONG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SRand",                  ZTID_VOID,             FUNCTION,     0,     1,          FUNCFLAG_INLINE,                      2,           { ZTID_RNG, ZTID_LONG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SRand",                  ZTID_LONG,             FUNCTION,     0,     1,          FUNCFLAG_INLINE,                      1,           { ZTID_RNG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Free",                   ZTID_VOID,             FUNCTION,     0,     1,          FUNCFLAG_INLINE,                      1,           { ZTID_RNG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Own",                    ZTID_VOID,             FUNCTION,     0,     1,          FUNCFLAG_INLINE,                      1,           { ZTID_RNG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "",                       -1,                       -1,           -1,               -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

RNGSymbols::RNGSymbols()
{
    table = RNGTable;
    refVar = REFRNG;
}

void RNGSymbols::generateCode()
{
	//int32_t Rand()
	{
		Function* function = getFunction("Rand", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new ORNGRand1());
		RETURN();
		function->giveCode(code);
	}
	//int32_t Rand(int32_t bound)
	{
		Function* function = getFunction("Rand", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new ORNGRand2(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t Rand(int32_t bound1, int32_t bound2)
	{
		Function* function = getFunction("Rand", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		//pop pointer
		POPREF();
		addOpcode2 (code, new ORNGRand3(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t LRand()
	{
		Function* function = getFunction("LRand", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new ORNGLRand1());
		RETURN();
		function->giveCode(code);
	}
	//int32_t LRand(int32_t bound)
	{
		Function* function = getFunction("LRand", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new ORNGLRand2(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t LRand(int32_t bound1, int32_t bound2)
	{
		Function* function = getFunction("LRand", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		//pop pointer
		POPREF();
		addOpcode2 (code, new ORNGLRand3(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//void SRand(int32_t seed)
    {
	    Function* function = getFunction("SRand", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop seed
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
		POPREF();
        addOpcode2 (code, new ORNGSeed(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t SRand()
    {
	    Function* function = getFunction("SRand", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
		ASSERT_NON_NUL();
		POPREF();
        LABELBACK(label);
        addOpcode2 (code, new ORNGRSeed());
        RETURN();
        function->giveCode(code);
    }
	//void Free()
	{
		Function* function = getFunction("Free", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new ORNGFree());
		RETURN();
		function->giveCode(code);
	}
	//void Own()
	{
		Function* function = getFunction("Own", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new ORNGOwn());
		RETURN();
		function->giveCode(code);
	}
}

