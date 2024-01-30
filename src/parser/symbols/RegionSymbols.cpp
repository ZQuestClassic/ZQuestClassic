#include "SymbolDefs.h"

RegionSymbols RegionSymbols::singleton = RegionSymbols();

static AccessorTable RegionTable[] =
{
	//name,                       tag,            rettype,   var,                       funcFlags,  params,optparams
	{ "getWidth",                   0,          ZTID_LONG,   REGION_WIDTH,                       0,  { ZTID_REGION }, {} },
	{ "getHeight",                  0,          ZTID_LONG,   REGION_HEIGHT,                      0,  { ZTID_REGION }, {} },
	{ "getScreenWidth",             0,          ZTID_LONG,   REGION_SCREEN_WIDTH,                0,  { ZTID_REGION }, {} },
	{ "getScreenHeight",            0,          ZTID_LONG,   REGION_SCREEN_HEIGHT,               0,  { ZTID_REGION }, {} },
	{ "getNumCombos",               0,          ZTID_LONG,   REGION_NUM_COMBOS,                  0,  { ZTID_REGION }, {} },
	{ "getID",                      0,          ZTID_LONG,   REGION_ID,                          0,  { ZTID_REGION }, {} },
	{ "getOriginScreen",            0,          ZTID_LONG,   REGION_ORIGIN_SCREEN,               0,  { ZTID_REGION }, {} },
	
	
	{ "GetScreenForComoboPos",      0,          ZTID_FLOAT,  -1,                           FL_INL,   { ZTID_REGION, ZTID_FLOAT }, {} },
	{ "TriggerSecrets",             0,          ZTID_VOID,   -1,                           FL_INL,   { ZTID_REGION, ZTID_FLOAT }, {} },

	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
};

RegionSymbols::RegionSymbols()
{
	table = RegionTable;
	refVar = NUL;
}

void RegionSymbols::generateCode()
{
	//int32_t GetScreenForComboPos(region, int32_t)
	{
		Function* function = getFunction("GetScreenForComoboPos");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OGetScreenForComboPos(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void TriggerSecrets(region, float)
	{
		Function* function = getFunction("TriggerSecrets");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OTriggerSecretsFor(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
}
