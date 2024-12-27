#include "SymbolDefs.h"

RegionSymbols RegionSymbols::singleton = RegionSymbols();

static AccessorTable RegionTable[] =
{
	//name,                       tag,            rettype,   var,                       funcFlags,  params,optparams
	{ "getWidth",                   0,          ZTID_LONG,   REGION_WIDTH,                       0,  { ZTID_REGION }, {} },
	{ "getHeight",                  0,          ZTID_LONG,   REGION_HEIGHT,                      0,  { ZTID_REGION }, {} },
	{ "getScreenWidth",             0,          ZTID_LONG,   REGION_SCREEN_WIDTH,                0,  { ZTID_REGION }, {} },
	{ "getScreenHeight",            0,          ZTID_LONG,   REGION_SCREEN_HEIGHT,               0,  { ZTID_REGION }, {} },
	{ "getNumCombos",               0,          ZTID_LONG,   REGION_NUM_COMBOS,                  0,  { ZTID_REGION }, {} },
	{ "getID",                      0,          ZTID_LONG,   REGION_ID,                          0,  { ZTID_REGION }, {} },
	{ "getOriginScreenIndex",       0,          ZTID_LONG,   REGION_ORIGIN_SCREEN,               0,  { ZTID_REGION }, {} },
	{ "getOriginScreen",            0,          ZTID_SCREEN, REGION_ORIGIN_SCREEN,               0,  { ZTID_REGION }, {} },

	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
};

RegionSymbols::RegionSymbols()
{
	table = RegionTable;
	refVar = NUL;
}

void RegionSymbols::generateCode()
{
}
