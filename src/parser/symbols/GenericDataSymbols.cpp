#include "SymbolDefs.h"

GenericDataSymbols GenericDataSymbols::singleton = GenericDataSymbols();

static AccessorTable GenericDataTable[] =
{
	//name,                     rettype,                  setorget,     var,                numindex,      funcFlags,                            numParams,   params
	{ "RunFrozen",              ZVARTYPEID_BOOL,          FUNCTION,     0,                  1,             FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_GENERICDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRunning",             ZVARTYPEID_BOOL,          GETTER,       GENDATARUNNING,     1,             0,                                    1,           { ZVARTYPEID_GENERICDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRunning",             ZVARTYPEID_VOID,          SETTER,       GENDATARUNNING,     1,             0,                                    2,           { ZVARTYPEID_GENERICDATA, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDataSize",            ZVARTYPEID_FLOAT,         GETTER,       GENDATASIZE,        1,             0,                                    1,           { ZVARTYPEID_GENERICDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDataSize",            ZVARTYPEID_VOID,          SETTER,       GENDATASIZE,        1,             0,                                    2,           { ZVARTYPEID_GENERICDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getExitState[]",         ZVARTYPEID_BOOL,          GETTER,       GENDATAEXITSTATE,   1,             0,                                    2,           { ZVARTYPEID_GENERICDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setExitState[]",         ZVARTYPEID_VOID,          SETTER,       GENDATAEXITSTATE,   1,             0,                                    3,           { ZVARTYPEID_GENERICDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getReloadState[]",       ZVARTYPEID_BOOL,          GETTER,       GENDATARELOADSTATE, 1,             0,                                    2,           { ZVARTYPEID_GENERICDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setReloadState[]",       ZVARTYPEID_VOID,          SETTER,       GENDATARELOADSTATE, 1,             0,                                    3,           { ZVARTYPEID_GENERICDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getEventListen[]",       ZVARTYPEID_BOOL,          GETTER,       GENDATAEVENTSTATE,  1,             0,                                    2,           { ZVARTYPEID_GENERICDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setEventListen[]",       ZVARTYPEID_VOID,          SETTER,       GENDATAEVENTSTATE,  1,             0,                                    3,           { ZVARTYPEID_GENERICDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getData[]",              ZVARTYPEID_UNTYPED,       GETTER,       GENDATADATA,        214748,        0,                                    2,           { ZVARTYPEID_GENERICDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setData[]",              ZVARTYPEID_VOID,          SETTER,       GENDATADATA,        214748,        0,                                    3,           { ZVARTYPEID_GENERICDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getInitD[]",             ZVARTYPEID_UNTYPED,       GETTER,       GENDATAINITD,       8,             0,                                    2,           { ZVARTYPEID_GENERICDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInitD[]",             ZVARTYPEID_VOID,          SETTER,       GENDATAINITD,       8,             0,                                    3,           { ZVARTYPEID_GENERICDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "",                       -1,                       -1,           -1,                 -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
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
		Function* function = getFunction("RunFrozen", 1);
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
