#include "SymbolDefs.h"

ShopDataSymbols ShopDataSymbols::singleton = ShopDataSymbols();

static AccessorTable ShopDataTable[] =
{
//	All of these return a function label error when used:
//	  name,                     rettype,                  setorget,     var,               numindex,      funcFlags,                            numParams,   params
	{ "getType",                ZVARTYPEID_FLOAT,         GETTER,       SHOPDATATYPE,       1,             0,                                    1,           { ZVARTYPEID_SHOPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getItem[]",              ZVARTYPEID_FLOAT,         GETTER,       SHOPDATAITEM,      3,             0,                                    2,           { ZVARTYPEID_SHOPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItem[]",              ZVARTYPEID_VOID,          SETTER,       SHOPDATAITEM,      3,             0,                                    3,           { ZVARTYPEID_SHOPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getHasItem[]",           ZVARTYPEID_BOOL,          GETTER,       SHOPDATAHASITEM,   3,             0,                                    2,           { ZVARTYPEID_SHOPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHasItem[]",           ZVARTYPEID_VOID,          SETTER,       SHOPDATAHASITEM,   3,             0,                                    3,           { ZVARTYPEID_SHOPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getPrice[]",             ZVARTYPEID_FLOAT,         GETTER,       SHOPDATAPRICE,     3,             0,                                    2,           { ZVARTYPEID_SHOPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPrice[]",             ZVARTYPEID_VOID,          SETTER,       SHOPDATAPRICE,     3,             0,                                    3,           { ZVARTYPEID_SHOPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getString[]",            ZVARTYPEID_FLOAT,         GETTER,       SHOPDATASTRING,    3,             0,                                    2,           { ZVARTYPEID_SHOPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setString[]",            ZVARTYPEID_VOID,          SETTER,       SHOPDATASTRING,    3,             0,                                    3,           { ZVARTYPEID_SHOPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
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

