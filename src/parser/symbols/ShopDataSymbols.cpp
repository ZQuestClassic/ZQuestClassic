#include "SymbolDefs.h"

ShopDataSymbols ShopDataSymbols::singleton = ShopDataSymbols();

static AccessorTable ShopDataTable[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
	{ "getType",                    0,         ZTID_FLOAT,   SHOPDATATYPE,              0,  { ZTID_SHOPDATA },{} },
	{ "getItem[]",                  0,         ZTID_FLOAT,   SHOPDATAITEM,              0,  { ZTID_SHOPDATA, ZTID_FLOAT },{} },
	{ "setItem[]",                  0,          ZTID_VOID,   SHOPDATAITEM,              0,  { ZTID_SHOPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getHasItem[]",               0,          ZTID_BOOL,   SHOPDATAHASITEM,           0,  { ZTID_SHOPDATA, ZTID_FLOAT },{} },
	{ "setHasItem[]",               0,          ZTID_VOID,   SHOPDATAHASITEM,           0,  { ZTID_SHOPDATA, ZTID_FLOAT, ZTID_BOOL },{} },
	
	{ "getPrice[]",                 0,         ZTID_FLOAT,   SHOPDATAPRICE,             0,  { ZTID_SHOPDATA, ZTID_FLOAT },{} },
	{ "setPrice[]",                 0,          ZTID_VOID,   SHOPDATAPRICE,             0,  { ZTID_SHOPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getString[]",                0,         ZTID_FLOAT,   SHOPDATASTRING,            0,  { ZTID_SHOPDATA, ZTID_FLOAT },{} },
	{ "setString[]",                0,          ZTID_VOID,   SHOPDATASTRING,            0,  { ZTID_SHOPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
};

ShopDataSymbols::ShopDataSymbols()
{
	table = ShopDataTable;
	refVar = REFSHOPDATA;
}

void ShopDataSymbols::generateCode()
{
}

