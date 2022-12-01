#include "SymbolDefs.h"

DropsetSymbols DropsetSymbols::singleton = DropsetSymbols();

static AccessorTable DropsetTable[] =
{
//	All of these return a function label error when used:
//	  name,                     rettype,                  setorget,     var,                   numindex,      funcFlags,                            numParams,   params
//	{ "getTest",                ZVARTYPEID_FLOAT,         GETTER,       DEBUGREFFFC,           1,             0,                                    1,           { ZVARTYPEID_DROPSET, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getItems[]",             ZVARTYPEID_FLOAT,         GETTER,       DROPSETITEMS,          10,            0,                                    2,           { ZVARTYPEID_DROPSET, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItems[]",             ZVARTYPEID_VOID,          SETTER,       DROPSETITEMS,          10,            0,                                    3,           { ZVARTYPEID_DROPSET, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getChances[]",           ZVARTYPEID_FLOAT,         GETTER,       DROPSETCHANCES,        10,            0,                                    2,           { ZVARTYPEID_DROPSET, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setChances[]",           ZVARTYPEID_VOID,          SETTER,       DROPSETCHANCES,        10,            0,                                    3,           { ZVARTYPEID_DROPSET, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getNothingChance",       ZVARTYPEID_FLOAT,         GETTER,       DROPSETNULLCHANCE,     1,             0,                                    1,           { ZVARTYPEID_DROPSET, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNothingChance",       ZVARTYPEID_VOID,          SETTER,       DROPSETNULLCHANCE,     1,             0,                                    2,           { ZVARTYPEID_DROPSET, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Choose",                 ZVARTYPEID_FLOAT,         GETTER,       DROPSETCHOOSE,         1,             0,                                    1,           { ZVARTYPEID_DROPSET, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "Create",                 ZVARTYPEID_ITEM,          FUNCTION,     0,                     1,             0,                                    3,           { ZVARTYPEID_DROPSET, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "Create",                 ZVARTYPEID_ITEM,          FUNCTION,     0,                     1,             0,                                    4,           { ZVARTYPEID_DROPSET, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "",                       -1,                       -1,           -1,                    -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

DropsetSymbols::DropsetSymbols()
{
    table = DropsetTable;
    refVar = REFDROPS;
}

void DropsetSymbols::generateCode()
{
}

