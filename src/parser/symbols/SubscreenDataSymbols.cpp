#include "SymbolDefs.h"

SubscreenDataSymbols SubscreenDataSymbols::singleton = SubscreenDataSymbols();

static AccessorTable SubscreenDataTable[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
	// Any Subscreen
	{ "getCurPage",                 0,         ZTID_FLOAT,   SUBDATACURPG,              0,  { ZTID_SUBSCREENDATA },{} },
	{ "setCurPage",                 0,          ZTID_VOID,   SUBDATACURPG,              0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getNumPages",                0,         ZTID_FLOAT,   SUBDATANUMPG,              0,  { ZTID_SUBSCREENDATA },{} },
	{ "setNumPages",                0,          ZTID_VOID,   SUBDATANUMPG,              0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getPages[]",                 0, ZTID_SUBSCREENPAGE,   SUBDATAPAGES,              0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setPages[]",                 0,          ZTID_VOID,   SUBDATAPAGES,      FL_RDONLY,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_SUBSCREENPAGE },{} },
	
	//Active Only
	{ "getCursorPos",               0,         ZTID_FLOAT,   SUBDATACURSORPOS,          0,  { ZTID_SUBSCREENDATA },{} },
	{ "setCursorPos",               0,          ZTID_VOID,   SUBDATACURSORPOS,          0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getScript",                  0,         ZTID_FLOAT,   SUBDATASCRIPT,             0,  { ZTID_SUBSCREENDATA },{} },
	{ "setScript",                  0,          ZTID_VOID,   SUBDATASCRIPT,             0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getInitD[]",                 0,       ZTID_UNTYPED,   SUBDATAINITD,              0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setInitD[]",                 0,          ZTID_VOID,   SUBDATAINITD,              0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_UNTYPED },{} },

	
	
	// { "RunFrozen",                  0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_GENERICDATA },{} },
	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
};

SubscreenDataSymbols::SubscreenDataSymbols()
{
	table = SubscreenDataTable;
	refVar = REFSUBSCREEN;
}

void SubscreenDataSymbols::generateCode()
{}



SubscreenPageSymbols SubscreenPageSymbols::singleton = SubscreenPageSymbols();

static AccessorTable SubscreenPageTable[] =
{
	//name,                       tag,              rettype,   var,               funcFlags,  params,optparams
	// Any Page
	{ "getIndex",                   0,           ZTID_FLOAT,   SUBPGINDEX,                0,  { ZTID_SUBSCREENPAGE },{} },
	{ "setIndex",                   0,            ZTID_VOID,   SUBPGINDEX,        FL_RDONLY,  { ZTID_SUBSCREENPAGE, ZTID_FLOAT },{} },
	{ "getNumWidgets",              0,           ZTID_FLOAT,   SUBPGNUMWIDG,              0,  { ZTID_SUBSCREENPAGE },{} },
	{ "setNumWidgets",              0,            ZTID_VOID,   SUBPGNUMWIDG,              0,  { ZTID_SUBSCREENPAGE, ZTID_FLOAT },{} },
	{ "getWidgets[]",               0, ZTID_SUBSCREENWIDGET,   SUBPGWIDGETS,              0,  { ZTID_SUBSCREENPAGE, ZTID_FLOAT },{} },
	{ "setWidgets[]",               0,            ZTID_VOID,   SUBPGWIDGETS,      FL_RDONLY,  { ZTID_SUBSCREENPAGE, ZTID_FLOAT, ZTID_SUBSCREENPAGE },{} },
	{ "getSubData",                 0,   ZTID_SUBSCREENDATA,   SUBPGSUBDATA,              0,  { ZTID_SUBSCREENPAGE },{} },
	{ "setSubData",                 0,            ZTID_VOID,   SUBPGSUBDATA,      FL_RDONLY,  { ZTID_SUBSCREENPAGE, ZTID_SUBSCREENDATA },{} },
	
	// Active Only
	{ "getCursorPos",               0,           ZTID_FLOAT,   SUBPGCURSORPOS,            0,  { ZTID_SUBSCREENPAGE },{} },
	{ "setCursorPos",               0,            ZTID_VOID,   SUBPGCURSORPOS,            0,  { ZTID_SUBSCREENPAGE, ZTID_FLOAT },{} },
	
	{ "",                           0,            ZTID_VOID,   -1,                        0,  {},{} }
};

SubscreenPageSymbols::SubscreenPageSymbols()
{
	table = SubscreenPageTable;
	refVar = REFSUBSCREENPAGE;
}

void SubscreenPageSymbols::generateCode()
{}



SubscreenWidgetSymbols SubscreenWidgetSymbols::singleton = SubscreenWidgetSymbols();

static AccessorTable SubscreenWidgetTable[] =
{
	//name,                       tag,              rettype,   var,               funcFlags,  params,optparams
	// Any Widget
	{ "getIndex",                   0,           ZTID_FLOAT,   SUBWIDGINDEX,              0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setIndex",                   0,            ZTID_VOID,   SUBWIDGINDEX,      FL_RDONLY,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getPos",                     0,           ZTID_FLOAT,   SUBWIDGPOS,                0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setPos",                     0,            ZTID_VOID,   SUBWIDGPOS,                0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getType",                    0,           ZTID_FLOAT,   SUBWIDGTYPE,               0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setType",                    0,            ZTID_VOID,   SUBWIDGTYPE,       FL_RDONLY,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getPage",                    0,   ZTID_SUBSCREENPAGE,   SUBWIDGPAGE,               0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setPage",                    0,            ZTID_VOID,   SUBWIDGPAGE,       FL_RDONLY,  { ZTID_SUBSCREENWIDGET, ZTID_SUBSCREENPAGE },{} },
	{ "",                           0,            ZTID_VOID,   -1,                        0,  {},{} }
};

SubscreenWidgetSymbols::SubscreenWidgetSymbols()
{
	table = SubscreenWidgetTable;
	refVar = REFSUBSCREENWIDG;
}

void SubscreenWidgetSymbols::generateCode()
{}


