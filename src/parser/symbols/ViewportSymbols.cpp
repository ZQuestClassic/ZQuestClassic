#include "SymbolDefs.h"

ViewportSymbols ViewportSymbols::singleton = ViewportSymbols();

static AccessorTable ViewportTable[] =
{
	//name,                       tag,            rettype,   var,                       funcFlags,  params,optparams
	{ "getX",                       0,          ZTID_FLOAT,   VIEWPORT_X,               0,  { ZTID_VIEWPORT }, {} },
	{ "getY",                       0,          ZTID_FLOAT,   VIEWPORT_Y,               0,  { ZTID_VIEWPORT }, {} },
	{ "getWidth",                   0,          ZTID_FLOAT,   VIEWPORT_WIDTH,           0,  { ZTID_VIEWPORT }, {} },
	{ "getHeight",                  0,          ZTID_FLOAT,   VIEWPORT_HEIGHT,          0,  { ZTID_VIEWPORT }, {} },

	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
};

ViewportSymbols::ViewportSymbols()
{
	table = ViewportTable;
	refVar = NUL;
}

void ViewportSymbols::generateCode()
{
}
