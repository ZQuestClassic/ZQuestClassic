#include "SymbolDefs.h"

SubscreenDataSymbols SubscreenDataSymbols::singleton = SubscreenDataSymbols();

static AccessorTable SubscreenDataTable[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
	// Any Subscreen
	{ "GetName",                    0,          ZTID_VOID,   -1,                        0,  { ZTID_SUBSCREENDATA, ZTID_CHAR },{} },
	{ "SetName",                    0,          ZTID_VOID,   -1,                        0,  { ZTID_SUBSCREENDATA, ZTID_CHAR },{} },
	{ "getCurPage",                 0,         ZTID_FLOAT,   SUBDATACURPG,              0,  { ZTID_SUBSCREENDATA },{} },
	{ "setCurPage",                 0,          ZTID_VOID,   SUBDATACURPG,              0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getNumPages",                0,         ZTID_FLOAT,   SUBDATANUMPG,              0,  { ZTID_SUBSCREENDATA },{} },
	{ "setNumPages",                0,          ZTID_VOID,   SUBDATANUMPG,              0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getPages[]",                 0, ZTID_SUBSCREENPAGE,   SUBDATAPAGES,              0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setPages[]",                 0,          ZTID_VOID,   SUBDATAPAGES,      FL_RDONLY,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_SUBSCREENPAGE },{} },
	{ "getType",                    0,         ZTID_FLOAT,   SUBDATATYPE,               0,  { ZTID_SUBSCREENDATA },{} },
	{ "setType",                    0,          ZTID_VOID,   SUBDATATYPE,       FL_RDONLY,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getFlags[]",                 0,          ZTID_BOOL,   SUBDATAFLAGS,              0,  { ZTID_SUBSCREENDATA },{} },
	{ "setFlags[]",                 0,          ZTID_VOID,   SUBDATAFLAGS,              0,  { ZTID_SUBSCREENDATA, ZTID_BOOL },{} },
	
	//Active Only
	{ "SwapPages",                  0,          ZTID_VOID,   -1,                        0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getCursorPos",               0,         ZTID_FLOAT,   SUBDATACURSORPOS,          0,  { ZTID_SUBSCREENDATA },{} },
	{ "setCursorPos",               0,          ZTID_VOID,   SUBDATACURSORPOS,          0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getScript",                  0,         ZTID_FLOAT,   SUBDATASCRIPT,             0,  { ZTID_SUBSCREENDATA },{} },
	{ "setScript",                  0,          ZTID_VOID,   SUBDATASCRIPT,             0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getInitD[]",                 0,       ZTID_UNTYPED,   SUBDATAINITD,              0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setInitD[]",                 0,          ZTID_VOID,   SUBDATAINITD,              0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getBtnPageLeft[]",           0,          ZTID_BOOL,   SUBDATABTNLEFT,            0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setBtnPageLeft[]",           0,          ZTID_VOID,   SUBDATABTNLEFT,            0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getBtnPageRight[]",          0,          ZTID_BOOL,   SUBDATABTNRIGHT,           0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setBtnPageRight[]",          0,          ZTID_VOID,   SUBDATABTNRIGHT,           0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_BOOL },{} },
	
	{ "getTransLeftType",           0,         ZTID_FLOAT,   SUBDATATRANSLEFTTY,        0,  { ZTID_SUBSCREENDATA },{} },
	{ "setTransLeftType",           0,          ZTID_VOID,   SUBDATATRANSLEFTTY,        0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getTransLeftSFX",            0,         ZTID_FLOAT,   SUBDATATRANSLEFTSFX,       0,  { ZTID_SUBSCREENDATA },{} },
	{ "setTransLeftSFX",            0,          ZTID_VOID,   SUBDATATRANSLEFTSFX,       0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getTransLeftFlags[]",        0,          ZTID_BOOL,   SUBDATATRANSLEFTFLAGS,     0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setTransLeftFlags[]",        0,          ZTID_VOID,   SUBDATATRANSLEFTFLAGS,     0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getTransLeftArgs[]",         0,         ZTID_FLOAT,   SUBDATATRANSLEFTARGS,      0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setTransLeftArgs[]",         0,          ZTID_VOID,   SUBDATATRANSLEFTARGS,      0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getTransRightType",          0,         ZTID_FLOAT,   SUBDATATRANSRIGHTTY,       0,  { ZTID_SUBSCREENDATA },{} },
	{ "setTransRightType",          0,          ZTID_VOID,   SUBDATATRANSRIGHTTY,       0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getTransRightSFX",           0,         ZTID_FLOAT,   SUBDATATRANSRIGHTSFX,      0,  { ZTID_SUBSCREENDATA },{} },
	{ "setTransRightSFX",           0,          ZTID_VOID,   SUBDATATRANSRIGHTSFX,      0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getTransRightFlags[]",       0,          ZTID_BOOL,   SUBDATATRANSRIGHTFLAGS,    0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setTransRightFlags[]",       0,          ZTID_VOID,   SUBDATATRANSRIGHTFLAGS,    0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getTransRightArgs[]",        0,         ZTID_FLOAT,   SUBDATATRANSRIGHTARGS,     0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setTransRightArgs[]",        0,          ZTID_VOID,   SUBDATATRANSRIGHTARGS,     0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getSelectorDestX",           0,         ZTID_FLOAT,   SUBDATASELECTORDSTX,       0,  { ZTID_SUBSCREENDATA },{} },
	{ "setSelectorDestX",           0,          ZTID_VOID,   SUBDATASELECTORDSTX,       0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getSelectorDestY",           0,         ZTID_FLOAT,   SUBDATASELECTORDSTY,       0,  { ZTID_SUBSCREENDATA },{} },
	{ "setSelectorDestY",           0,          ZTID_VOID,   SUBDATASELECTORDSTY,       0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getSelectorDestW",           0,         ZTID_FLOAT,   SUBDATASELECTORDSTW,       0,  { ZTID_SUBSCREENDATA },{} },
	{ "setSelectorDestW",           0,          ZTID_VOID,   SUBDATASELECTORDSTW,       0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getSelectorDestH",           0,         ZTID_FLOAT,   SUBDATASELECTORDSTH,       0,  { ZTID_SUBSCREENDATA },{} },
	{ "setSelectorDestH",           0,          ZTID_VOID,   SUBDATASELECTORDSTH,       0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	
	{ "getSelectorWid[]",           0,         ZTID_FLOAT,   SUBDATASELECTORWID,        0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setSelectorWid[]",           0,          ZTID_VOID,   SUBDATASELECTORWID,        0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getSelectorHei[]",           0,         ZTID_FLOAT,   SUBDATASELECTORHEI,        0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setSelectorHei[]",           0,          ZTID_VOID,   SUBDATASELECTORHEI,        0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getSelectorTile[]",          0,         ZTID_FLOAT,   SUBDATASELECTORTILE,       0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setSelectorTile[]",          0,          ZTID_VOID,   SUBDATASELECTORTILE,       0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getSelectorCSet[]",          0,         ZTID_FLOAT,   SUBDATASELECTORCSET,       0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setSelectorCSet[]",          0,          ZTID_VOID,   SUBDATASELECTORCSET,       0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getSelectorFlashCSet[]",     0,         ZTID_FLOAT,   SUBDATASELECTORFLASHCSET,  0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setSelectorFlashCSet[]",     0,          ZTID_VOID,   SUBDATASELECTORFLASHCSET,  0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getSelectorFrames[]",        0,         ZTID_FLOAT,   SUBDATASELECTORFRM,        0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setSelectorFrames[]",        0,          ZTID_VOID,   SUBDATASELECTORFRM,        0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getSelectorASpeed[]",        0,         ZTID_FLOAT,   SUBDATASELECTORASPD,       0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setSelectorASpeed[]",        0,          ZTID_VOID,   SUBDATASELECTORASPD,       0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getSelectorDelay[]",         0,         ZTID_FLOAT,   SUBDATASELECTORDELAY,      0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setSelectorDelay[]",         0,          ZTID_VOID,   SUBDATASELECTORDELAY,      0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	// Current Transition info
	{ "getTransClock",              0,         ZTID_FLOAT,   SUBDATATRANSCLK,           0,  { ZTID_SUBSCREENDATA },{} },
	{ "setTransClock",              0,          ZTID_VOID,   SUBDATATRANSCLK,           0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getTransType",               0,         ZTID_FLOAT,   SUBDATATRANSTY,            0,  { ZTID_SUBSCREENDATA },{} },
	{ "setTransType",               0,          ZTID_VOID,   SUBDATATRANSTY,            0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getTransFlags[]",            0,          ZTID_BOOL,   SUBDATATRANSFLAGS,         0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setTransFlags[]",            0,          ZTID_VOID,   SUBDATATRANSFLAGS,         0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getTransArgs[]",             0,         ZTID_FLOAT,   SUBDATATRANSARGS,          0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setTransArgs[]",             0,          ZTID_VOID,   SUBDATATRANSARGS,          0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getTransFromPage",           0,         ZTID_FLOAT,   SUBDATATRANSFROMPG,        0,  { ZTID_SUBSCREENDATA },{} },
	{ "setTransFromPage",           0,          ZTID_VOID,   SUBDATATRANSFROMPG,        0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getTransToPage",             0,         ZTID_FLOAT,   SUBDATATRANSTOPG,          0,  { ZTID_SUBSCREENDATA },{} },
	{ "setTransToPage",             0,          ZTID_VOID,   SUBDATATRANSTOPG,          0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	
	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
};

SubscreenDataSymbols::SubscreenDataSymbols()
{
	table = SubscreenDataTable;
	refVar = REFSUBSCREEN;
}

void SubscreenDataSymbols::generateCode()
{
	//void GetName(subscreendata, char)
	{
		Function* function = getFunction("GetName");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new OGetSubscreenName(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void SetName(subscreendata, char)
	{
		Function* function = getFunction("SetName");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new OSetSubscreenName(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void SwapPages(subscreendata, int, int)
	{
		Function* function = getFunction("SwapPages");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OSubscrSwapPages());
		LABELBACK(label);
		POP_ARGS(3,NUL);
		RETURN();
		function->giveCode(code);
	}
}



SubscreenPageSymbols SubscreenPageSymbols::singleton = SubscreenPageSymbols();

static AccessorTable SubscreenPageTable[] =
{
	//name,                       tag,              rettype,   var,               funcFlags,  params,optparams
	// Any Page
	{ "getIndex",                   0,           ZTID_FLOAT,   SUBPGINDEX,                0,  { ZTID_SUBSCREENPAGE },{} },
	{ "setIndex",                   0,            ZTID_VOID,   SUBPGINDEX,        FL_RDONLY,  { ZTID_SUBSCREENPAGE, ZTID_FLOAT },{} },
	{ "getNumWidgets",              0,           ZTID_FLOAT,   SUBPGNUMWIDG,              0,  { ZTID_SUBSCREENPAGE },{} },
	{ "setNumWidgets",              0,            ZTID_VOID,   SUBPGNUMWIDG,      FL_RDONLY,  { ZTID_SUBSCREENPAGE, ZTID_FLOAT },{} },
	{ "getWidgets[]",               0, ZTID_SUBSCREENWIDGET,   SUBPGWIDGETS,              0,  { ZTID_SUBSCREENPAGE, ZTID_FLOAT },{} },
	{ "setWidgets[]",               0,            ZTID_VOID,   SUBPGWIDGETS,      FL_RDONLY,  { ZTID_SUBSCREENPAGE, ZTID_FLOAT, ZTID_SUBSCREENPAGE },{} },
	{ "getSubData",                 0,   ZTID_SUBSCREENDATA,   SUBPGSUBDATA,              0,  { ZTID_SUBSCREENPAGE },{} },
	{ "setSubData",                 0,            ZTID_VOID,   SUBPGSUBDATA,      FL_RDONLY,  { ZTID_SUBSCREENPAGE, ZTID_SUBSCREENDATA },{} },
	
	// Active Only
	{ "getCursorPos",               0,           ZTID_FLOAT,   SUBPGCURSORPOS,            0,  { ZTID_SUBSCREENPAGE },{} },
	{ "setCursorPos",               0,            ZTID_VOID,   SUBPGCURSORPOS,            0,  { ZTID_SUBSCREENPAGE, ZTID_FLOAT },{} },
	{ "CreateWidget",               0, ZTID_SUBSCREENWIDGET,   -1,                        0,  { ZTID_SUBSCREENPAGE, ZTID_FLOAT },{} },
	{ "SwapWidgets",                0,            ZTID_VOID,   -1,                        0,  { ZTID_SUBSCREENPAGE, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "FindWidget",                 0, ZTID_SUBSCREENWIDGET,   -1,                        0,  { ZTID_SUBSCREENPAGE, ZTID_FLOAT },{} },
	{ "SelectorMove",               0,           ZTID_FLOAT,   -1,                        0,  { ZTID_SUBSCREENPAGE, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "Delete",                     0,            ZTID_VOID,   -1,                        0,  { ZTID_SUBSCREENPAGE },{} },
	
	{ "",                           0,            ZTID_VOID,   -1,                        0,  {},{} }
};

SubscreenPageSymbols::SubscreenPageSymbols()
{
	table = SubscreenPageTable;
	refVar = REFSUBSCREENPAGE;
}

void SubscreenPageSymbols::generateCode()
{
	//void FindWidget(subscreenpage, int)
	{
		Function* function = getFunction("FindWidget");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OSubscrPgFindWidget());
		LABELBACK(label);
		POP_ARGS(2,NUL);
		RETURN();
		function->giveCode(code);
	}
	//void SelectorMove(subscreenpage, int, int, int)
	{
		Function* function = getFunction("SelectorMove");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OSubscrPgMvCursor());
		LABELBACK(label);
		POP_ARGS(4,NUL);
		RETURN();
		function->giveCode(code);
	}
	//void SwapWidgets(subscreenpage, int, int)
	{
		Function* function = getFunction("SwapWidgets");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OSubscrPgSwapWidgets());
		LABELBACK(label);
		POP_ARGS(3,NUL);
		RETURN();
		function->giveCode(code);
	}
	//void CreateWidget(subscreenpage, int)
	{
		Function* function = getFunction("CreateWidget");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OSubscrPgNewWidget());
		LABELBACK(label);
		POP_ARGS(2,NUL);
		RETURN();
		function->giveCode(code);
	}
	//void Delete(subscreenpage)
	{
		Function* function = getFunction("Delete");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new OSubscrPgDelete());
		RETURN();
		function->giveCode(code);
	}
}



SubscreenWidgetSymbols SubscreenWidgetSymbols::singleton = SubscreenWidgetSymbols();

static AccessorTable SubscreenWidgetTable[] =
{
	//name,                       tag,              rettype,   var,               funcFlags,  params,optparams
	// Any Widget
	/*{ "getIndex",                   0,           ZTID_FLOAT,   SUBWIDGINDEX,              0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setIndex",                   0,            ZTID_VOID,   SUBWIDGINDEX,      FL_RDONLY,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getPos",                     0,           ZTID_FLOAT,   SUBWIDGPOS,                0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setPos",                     0,            ZTID_VOID,   SUBWIDGPOS,                0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getType",                    0,           ZTID_FLOAT,   SUBWIDGTYPE,               0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setType",                    0,            ZTID_VOID,   SUBWIDGTYPE,       FL_RDONLY,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getPage",                    0,   ZTID_SUBSCREENPAGE,   SUBWIDGPAGE,               0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setPage",                    0,            ZTID_VOID,   SUBWIDGPAGE,       FL_RDONLY,  { ZTID_SUBSCREENWIDGET, ZTID_SUBSCREENPAGE },{} },
	*/{ "",                           0,            ZTID_VOID,   -1,                        0,  {},{} }
};

SubscreenWidgetSymbols::SubscreenWidgetSymbols()
{
	table = SubscreenWidgetTable;
	refVar = REFSUBSCREENWIDG;
}

void SubscreenWidgetSymbols::generateCode()
{}


