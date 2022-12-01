#include "SymbolDefs.h"

GenericDataSymbols GenericDataSymbols::singleton = GenericDataSymbols();

static AccessorTable GenericDataTable[] =
{
	//name,                     rettype,                  setorget,     var,                numindex,      funcFlags,                            numParams,   params
	{ "RunFrozen",              ZTID_BOOL,          FUNCTION,     0,                  1,             FUNCFLAG_INLINE,                      1,           { ZTID_GENERICDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRunning",             ZTID_BOOL,          GETTER,       GENDATARUNNING,     1,             0,                                    1,           { ZTID_GENERICDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRunning",             ZTID_VOID,          SETTER,       GENDATARUNNING,     1,             0,                                    2,           { ZTID_GENERICDATA, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDataSize",            ZTID_FLOAT,         GETTER,       GENDATASIZE,        1,             0,                                    1,           { ZTID_GENERICDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDataSize",            ZTID_VOID,          SETTER,       GENDATASIZE,        1,             0,                                    2,           { ZTID_GENERICDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getExitState[]",         ZTID_BOOL,          GETTER,       GENDATAEXITSTATE,   1,             0,                                    2,           { ZTID_GENERICDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setExitState[]",         ZTID_VOID,          SETTER,       GENDATAEXITSTATE,   1,             0,                                    3,           { ZTID_GENERICDATA, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getReloadState[]",       ZTID_BOOL,          GETTER,       GENDATARELOADSTATE, 1,             0,                                    2,           { ZTID_GENERICDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setReloadState[]",       ZTID_VOID,          SETTER,       GENDATARELOADSTATE, 1,             0,                                    3,           { ZTID_GENERICDATA, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getEventListen[]",       ZTID_BOOL,          GETTER,       GENDATAEVENTSTATE,  1,             0,                                    2,           { ZTID_GENERICDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setEventListen[]",       ZTID_VOID,          SETTER,       GENDATAEVENTSTATE,  1,             0,                                    3,           { ZTID_GENERICDATA, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getData[]",              ZTID_UNTYPED,       GETTER,       GENDATADATA,        214748,        0,                                    2,           { ZTID_GENERICDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setData[]",              ZTID_VOID,          SETTER,       GENDATADATA,        214748,        0,                                    3,           { ZTID_GENERICDATA, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getInitD[]",             ZTID_UNTYPED,       GETTER,       GENDATAINITD,       8,             0,                                    2,           { ZTID_GENERICDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInitD[]",             ZTID_VOID,          SETTER,       GENDATAINITD,       8,             0,                                    3,           { ZTID_GENERICDATA, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
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
