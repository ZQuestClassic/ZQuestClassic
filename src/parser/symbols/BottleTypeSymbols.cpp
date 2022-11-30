#include "SymbolDefs.h"

BottleTypeSymbols BottleTypeSymbols::singleton = BottleTypeSymbols();

static AccessorTable BottleTypeTable[] =
{
	//name,                     rettype,                  setorget,     var,              numindex,      funcFlags,                            numParams,   params
	{ "GetName",                ZVARTYPEID_VOID,          FUNCTION,     0,                1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_BOTTLETYPE, ZVARTYPEID_CHAR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetName",                ZVARTYPEID_VOID,          FUNCTION,     0,                1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_BOTTLETYPE, ZVARTYPEID_CHAR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCounter[]",           ZVARTYPEID_FLOAT,         GETTER,       BOTTLECOUNTER,    3,             0,                                    2,           { ZVARTYPEID_BOTTLETYPE, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCounter[]",           ZVARTYPEID_VOID,          SETTER,       BOTTLECOUNTER,    3,             0,                                    3,           { ZVARTYPEID_BOTTLETYPE, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAmount[]",            ZVARTYPEID_FLOAT,         GETTER,       BOTTLEAMOUNT,     3,             0,                                    2,           { ZVARTYPEID_BOTTLETYPE, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAmount[]",            ZVARTYPEID_VOID,          SETTER,       BOTTLEAMOUNT,     3,             0,                                    3,           { ZVARTYPEID_BOTTLETYPE, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getIsPercent[]",         ZVARTYPEID_BOOL,          GETTER,       BOTTLEPERCENT,    3,             0,                                    2,           { ZVARTYPEID_BOTTLETYPE, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setIsPercent[]",         ZVARTYPEID_VOID,          SETTER,       BOTTLEPERCENT,    3,             0,                                    3,           { ZVARTYPEID_BOTTLETYPE, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlags[]",             ZVARTYPEID_BOOL,          GETTER,       BOTTLEFLAGS,      4,             0,                                    2,           { ZVARTYPEID_BOTTLETYPE, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlags[]",             ZVARTYPEID_VOID,          SETTER,       BOTTLEFLAGS,      4,             0,                                    3,           { ZVARTYPEID_BOTTLETYPE, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getNextType",            ZVARTYPEID_FLOAT,         GETTER,       BOTTLENEXT,       1,             0,                                    1,           { ZVARTYPEID_BOTTLETYPE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNextType",            ZVARTYPEID_VOID,          SETTER,       BOTTLENEXT,       1,             0,                                    2,           { ZVARTYPEID_BOTTLETYPE, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "",                       -1,                       -1,           -1,               -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

BottleTypeSymbols::BottleTypeSymbols()
{
    table = BottleTypeTable;
    refVar = REFBOTTLETYPE;
}

void BottleTypeSymbols::generateCode()
{
    //void GetName(bottledata, int32_t)
    {
		Function* function = getFunction("GetName", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer
        POPREF();
        addOpcode2 (code, new OGetBottleName(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //void SetName(bottledata, int32_t)
    {
		Function* function = getFunction("SetName", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer
        POPREF();
        addOpcode2 (code, new OSetBottleName(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
}

