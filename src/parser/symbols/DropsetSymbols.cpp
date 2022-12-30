#include "SymbolDefs.h"

DropsetSymbols DropsetSymbols::singleton = DropsetSymbols();

static AccessorTable DropsetTable[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
	{ "getItems[]",                 0,         ZTID_FLOAT,   DROPSETITEMS,              0,  { ZTID_DROPSET, ZTID_FLOAT },{} },
	{ "setItems[]",                 0,          ZTID_VOID,   DROPSETITEMS,              0,  { ZTID_DROPSET, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getChances[]",               0,         ZTID_FLOAT,   DROPSETCHANCES,            0,  { ZTID_DROPSET, ZTID_FLOAT },{} },
	{ "setChances[]",               0,          ZTID_VOID,   DROPSETCHANCES,            0,  { ZTID_DROPSET, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getNothingChance",           0,         ZTID_FLOAT,   DROPSETNULLCHANCE,         0,  { ZTID_DROPSET },{} },
	{ "setNothingChance",           0,          ZTID_VOID,   DROPSETNULLCHANCE,         0,  { ZTID_DROPSET, ZTID_FLOAT },{} },
	{ "_getChoose",                 0,         ZTID_FLOAT,   DROPSETCHOOSE,             0,  { ZTID_DROPSET },{} },
//	{ "Create",                     0,          ZTID_ITEM,   -1,                        0,  { ZTID_DROPSET, ZTID_FLOAT, ZTID_FLOAT },{} },
//	{ "Create",                     1,          ZTID_ITEM,   -1,                        0,  { ZTID_DROPSET, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
};

DropsetSymbols::DropsetSymbols()
{
	table = DropsetTable;
	refVar = REFDROPS;
}

void DropsetSymbols::generateCode()
{
}

