#include "SymbolDefs.h"

DropsetSymbols DropsetSymbols::singleton = DropsetSymbols();

static AccessorTable DropsetTable[] =
{
//	All of these return a function label error when used:
//	  name,                     rettype,                  setorget,     var,                   numindex,      funcFlags,                            numParams,   params
//	{ "getTest",                ZTID_FLOAT,         GETTER,       DEBUGREFFFC,           1,             0,                                    1,           { ZTID_DROPSET, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getItems[]",             ZTID_FLOAT,         GETTER,       DROPSETITEMS,          10,            0,                                    2,           { ZTID_DROPSET, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItems[]",             ZTID_VOID,          SETTER,       DROPSETITEMS,          10,            0,                                    3,           { ZTID_DROPSET, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getChances[]",           ZTID_FLOAT,         GETTER,       DROPSETCHANCES,        10,            0,                                    2,           { ZTID_DROPSET, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setChances[]",           ZTID_VOID,          SETTER,       DROPSETCHANCES,        10,            0,                                    3,           { ZTID_DROPSET, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getNothingChance",       ZTID_FLOAT,         GETTER,       DROPSETNULLCHANCE,     1,             0,                                    1,           { ZTID_DROPSET, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNothingChance",       ZTID_VOID,          SETTER,       DROPSETNULLCHANCE,     1,             0,                                    2,           { ZTID_DROPSET, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Choose",                 ZTID_FLOAT,         GETTER,       DROPSETCHOOSE,         1,             0,                                    1,           { ZTID_DROPSET, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "Create",                 ZTID_ITEM,          FUNCTION,     0,                     1,             0,                                    3,           { ZTID_DROPSET, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "Create",                 ZTID_ITEM,          FUNCTION,     0,                     1,             0,                                    4,           { ZTID_DROPSET, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
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

