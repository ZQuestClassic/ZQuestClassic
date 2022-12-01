#include "SymbolDefs.h"

BottleShopSymbols BottleShopSymbols::singleton = BottleShopSymbols();

static AccessorTable BottleShopTable[] =
{
	//name,                     rettype,                  setorget,     var,              numindex,      funcFlags,                            numParams,   params
	{ "GetName",                ZVARTYPEID_VOID,          FUNCTION,     0,                1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_BOTTLESHOP, ZVARTYPEID_CHAR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetName",                ZVARTYPEID_VOID,          FUNCTION,     0,                1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_BOTTLESHOP, ZVARTYPEID_CHAR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFill[]",              ZVARTYPEID_FLOAT,         GETTER,       BSHOPFILL,        3,             0,                                    2,           { ZVARTYPEID_BOTTLESHOP, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFill[]",              ZVARTYPEID_VOID,          SETTER,       BSHOPFILL,        3,             0,                                    3,           { ZVARTYPEID_BOTTLESHOP, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCombo[]",             ZVARTYPEID_FLOAT,         GETTER,       BSHOPCOMBO,       3,             0,                                    2,           { ZVARTYPEID_BOTTLESHOP, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCombo[]",             ZVARTYPEID_VOID,          SETTER,       BSHOPCOMBO,       3,             0,                                    3,           { ZVARTYPEID_BOTTLESHOP, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCSet[]",              ZVARTYPEID_FLOAT,         GETTER,       BSHOPCSET,        3,             0,                                    2,           { ZVARTYPEID_BOTTLESHOP, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCSet[]",              ZVARTYPEID_VOID,          SETTER,       BSHOPCSET,        3,             0,                                    3,           { ZVARTYPEID_BOTTLESHOP, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPrice[]",             ZVARTYPEID_FLOAT,         GETTER,       BSHOPPRICE,       3,             0,                                    2,           { ZVARTYPEID_BOTTLESHOP, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPrice[]",             ZVARTYPEID_VOID,          SETTER,       BSHOPPRICE,       3,             0,                                    3,           { ZVARTYPEID_BOTTLESHOP, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getInfoString[]",        ZVARTYPEID_FLOAT,         GETTER,       BSHOPSTR,         3,             0,                                    2,           { ZVARTYPEID_BOTTLESHOP, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInfoString[]",        ZVARTYPEID_VOID,          SETTER,       BSHOPSTR,         3,             0,                                    3,           { ZVARTYPEID_BOTTLESHOP, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "",                       -1,                       -1,           -1,               -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

BottleShopSymbols::BottleShopSymbols()
{
    table = BottleShopTable;
    refVar = REFBOTTLESHOP;
}

void BottleShopSymbols::generateCode()
{
    //void GetName(bottleshopdata, int32_t)
    {
		Function* function = getFunction("GetName", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer
        POPREF();
        addOpcode2 (code, new OGetBottleShopName(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //void SetName(bottleshopdata, int32_t)
    {
		Function* function = getFunction("SetName", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer
        POPREF();
        addOpcode2 (code, new OSetBottleShopName(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
}

