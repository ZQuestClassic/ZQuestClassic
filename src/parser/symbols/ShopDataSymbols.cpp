#include "SymbolDefs.h"

ShopDataSymbols ShopDataSymbols::singleton = ShopDataSymbols();

static AccessorTable ShopDataTable[] =
{
//	All of these return a function label error when used:
//	  name,                     rettype,                  setorget,     var,               numindex,      funcFlags,                            numParams,   params
	{ "getType",                ZTID_FLOAT,         GETTER,       SHOPDATATYPE,       1,             0,                                    1,           { ZTID_SHOPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getItem[]",              ZTID_FLOAT,         GETTER,       SHOPDATAITEM,      3,             0,                                    2,           { ZTID_SHOPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItem[]",              ZTID_VOID,          SETTER,       SHOPDATAITEM,      3,             0,                                    3,           { ZTID_SHOPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getHasItem[]",           ZTID_BOOL,          GETTER,       SHOPDATAHASITEM,   3,             0,                                    2,           { ZTID_SHOPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHasItem[]",           ZTID_VOID,          SETTER,       SHOPDATAHASITEM,   3,             0,                                    3,           { ZTID_SHOPDATA, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getPrice[]",             ZTID_FLOAT,         GETTER,       SHOPDATAPRICE,     3,             0,                                    2,           { ZTID_SHOPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPrice[]",             ZTID_VOID,          SETTER,       SHOPDATAPRICE,     3,             0,                                    3,           { ZTID_SHOPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getString[]",            ZTID_FLOAT,         GETTER,       SHOPDATASTRING,    3,             0,                                    2,           { ZTID_SHOPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setString[]",            ZTID_VOID,          SETTER,       SHOPDATASTRING,    3,             0,                                    3,           { ZTID_SHOPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "",                       -1,                       -1,           -1,                -1,            0,                                    1,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

ShopDataSymbols::ShopDataSymbols()
{
	table = ShopDataTable;
	refVar = REFSHOPDATA;
}

void ShopDataSymbols::generateCode()
{
}

