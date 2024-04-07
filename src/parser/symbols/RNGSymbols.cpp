#include "SymbolDefs.h"

RNGSymbols RNGSymbols::singleton = RNGSymbols();

static AccessorTable RNGTable[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
//	{ "DirExists",                  0,          ZTID_BOOL,   -1,                        0,  { ZTID_FILESYSTEM, ZTID_CHAR },{} },
	{ "Rand",                       0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_RNG },{} },
	{ "Rand",                       1,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_RNG, ZTID_FLOAT },{} },
	{ "Rand",                       2,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_RNG, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "LRand",                      0,          ZTID_LONG,   -1,                   FL_INL,  { ZTID_RNG },{} },
	{ "LRand",                      1,          ZTID_LONG,   -1,                   FL_INL,  { ZTID_RNG, ZTID_LONG },{} },
	{ "LRand",                      2,          ZTID_LONG,   -1,                   FL_INL,  { ZTID_RNG, ZTID_LONG, ZTID_LONG },{} },
	{ "SRand",                      0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_RNG, ZTID_LONG },{} },
	{ "SRand",                      1,          ZTID_LONG,   -1,                   FL_INL,  { ZTID_RNG },{} },
	{ "Free",                       0,          ZTID_VOID,   -1,           FL_INL|FL_DEPR,  { ZTID_RNG },{},0,"Free() no longer does anything as of ZC 3.0. Objects are now freed automatically." },
	{ "Own",                        0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_RNG },{} },
	
	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
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
		Function* function = getFunction("Rand");
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
		Function* function = getFunction("Rand", 1);
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
		Function* function = getFunction("Rand", 2);
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
		Function* function = getFunction("LRand");
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
		Function* function = getFunction("LRand", 1);
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
		Function* function = getFunction("LRand", 2);
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
		Function* function = getFunction("SRand");
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
		Function* function = getFunction("Free");
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
		Function* function = getFunction("Own");
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

