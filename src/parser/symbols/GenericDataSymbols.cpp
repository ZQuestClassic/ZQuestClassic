#include "SymbolDefs.h"

GenericDataSymbols GenericDataSymbols::singleton = GenericDataSymbols();

static AccessorTable GenericDataTable[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
	{ "RunFrozen",                  0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_GENERICDATA },{} },
	{ "getRunning",                 0,          ZTID_BOOL,   GENDATARUNNING,            0,  { ZTID_GENERICDATA },{} },
	{ "setRunning",                 0,          ZTID_VOID,   GENDATARUNNING,            0,  { ZTID_GENERICDATA, ZTID_BOOL },{} },
	{ "getDataSize",                0,         ZTID_FLOAT,   GENDATASIZE,               0,  { ZTID_GENERICDATA },{} },
	{ "setDataSize",                0,          ZTID_VOID,   GENDATASIZE,               0,  { ZTID_GENERICDATA, ZTID_FLOAT },{} },
	{ "getExitState[]",             0,          ZTID_BOOL,   GENDATAEXITSTATE,          0,  { ZTID_GENERICDATA, ZTID_FLOAT },{} },
	{ "setExitState[]",             0,          ZTID_VOID,   GENDATAEXITSTATE,          0,  { ZTID_GENERICDATA, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getReloadState[]",           0,          ZTID_BOOL,   GENDATARELOADSTATE,        0,  { ZTID_GENERICDATA, ZTID_FLOAT },{} },
	{ "setReloadState[]",           0,          ZTID_VOID,   GENDATARELOADSTATE,        0,  { ZTID_GENERICDATA, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getEventListen[]",           0,          ZTID_BOOL,   GENDATAEVENTSTATE,         0,  { ZTID_GENERICDATA, ZTID_FLOAT },{} },
	{ "setEventListen[]",           0,          ZTID_VOID,   GENDATAEVENTSTATE,         0,  { ZTID_GENERICDATA, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getData[]",                  0,       ZTID_UNTYPED,   GENDATADATA,               0,  { ZTID_GENERICDATA, ZTID_FLOAT },{} },
	{ "setData[]",                  0,          ZTID_VOID,   GENDATADATA,               0,  { ZTID_GENERICDATA, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getInitD[]",                 0,       ZTID_UNTYPED,   GENDATAINITD,              0,  { ZTID_GENERICDATA, ZTID_FLOAT },{} },
	{ "setInitD[]",                 0,          ZTID_VOID,   GENDATAINITD,              0,  { ZTID_GENERICDATA, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
};

GenericDataSymbols::GenericDataSymbols()
{
	table = GenericDataTable;
	refVar = REFGENERICDATA;
}

void GenericDataSymbols::generateCode()
{
	//RunFrozen(genericdata)
	{
		Function* function = getFunction("RunFrozen");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new ORunGenericFrozenScript(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
}
