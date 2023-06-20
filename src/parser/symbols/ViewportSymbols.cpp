#include "SymbolDefs.h"

ViewportSymbols ViewportSymbols::singleton = ViewportSymbols();

static AccessorTable ViewportTable[] =
{
	//name,                       tag,            rettype,   var,                       funcFlags,  params,optparams
	{ "getMode",                    0,          ZTID_LONG,   VIEWPORT_MODE,                    0,  { ZTID_VIEWPORT }, {} },
	{ "setMode",                    0,          ZTID_VOID,   VIEWPORT_MODE,                    0,  { ZTID_VIEWPORT, ZTID_LONG }, {} },
	{ "getX",                       0,          ZTID_LONG,   VIEWPORT_X,                       0,  { ZTID_VIEWPORT }, {} },
	{ "setX",                       0,          ZTID_VOID,   VIEWPORT_X,                       0,  { ZTID_VIEWPORT, ZTID_LONG }, {} },
	{ "getY",                       0,          ZTID_LONG,   VIEWPORT_Y,                       0,  { ZTID_VIEWPORT }, {} },
	{ "setY",                       0,          ZTID_VOID,   VIEWPORT_Y,                       0,  { ZTID_VIEWPORT, ZTID_LONG }, {} },
	{ "getWidth",                   0,          ZTID_LONG,   VIEWPORT_WIDTH,                   0,  { ZTID_VIEWPORT }, {} },
	{ "setWidth",                   0,          ZTID_VOID,   VIEWPORT_WIDTH,                   0,  { ZTID_VIEWPORT, ZTID_LONG }, {} },
	{ "getHeight",                  0,          ZTID_LONG,   VIEWPORT_HEIGHT,                  0,  { ZTID_VIEWPORT }, {} },
	{ "setHeight",                  0,          ZTID_VOID,   VIEWPORT_HEIGHT,                  0,  { ZTID_VIEWPORT, ZTID_LONG }, {} },

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
