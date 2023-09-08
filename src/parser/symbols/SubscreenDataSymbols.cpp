#include "SymbolDefs.h"

SubscreenDataSymbols SubscreenDataSymbols::singleton = SubscreenDataSymbols();

static AccessorTable SubscreenDataTable[] =
{
	//name,                       tag,              rettype,   var,               funcFlags,  params,optparams
	// Any Subscreen
	{ "GetName",                    0,            ZTID_VOID,   -1,                        0,  { ZTID_SUBSCREENDATA, ZTID_CHAR },{} },
	{ "SetName",                    0,            ZTID_VOID,   -1,                        0,  { ZTID_SUBSCREENDATA, ZTID_CHAR },{} },
	{ "getCurPage",                 0,           ZTID_FLOAT,   SUBDATACURPG,              0,  { ZTID_SUBSCREENDATA },{} },
	{ "setCurPage",                 0,            ZTID_VOID,   SUBDATACURPG,              0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getNumPages",                0,           ZTID_FLOAT,   SUBDATANUMPG,              0,  { ZTID_SUBSCREENDATA },{} },
	{ "setNumPages",                0,            ZTID_VOID,   SUBDATANUMPG,              0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getPages[]",                 0,   ZTID_SUBSCREENPAGE,   SUBDATAPAGES,              0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setPages[]",                 0,            ZTID_VOID,   SUBDATAPAGES,      FL_RDONLY,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_SUBSCREENPAGE },{} },
	{ "getType",                    0,           ZTID_FLOAT,   SUBDATATYPE,               0,  { ZTID_SUBSCREENDATA },{} },
	{ "setType",                    0,            ZTID_VOID,   SUBDATATYPE,       FL_RDONLY,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getFlags[]",                 0,            ZTID_BOOL,   SUBDATAFLAGS,              0,  { ZTID_SUBSCREENDATA },{} },
	{ "setFlags[]",                 0,            ZTID_VOID,   SUBDATAFLAGS,              0,  { ZTID_SUBSCREENDATA, ZTID_BOOL },{} },
	
	//Active Only
	{ "SwapPages",                  0,            ZTID_VOID,   -1,                        0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getCursorPos",               0,           ZTID_FLOAT,   SUBDATACURSORPOS,          0,  { ZTID_SUBSCREENDATA },{} },
	{ "setCursorPos",               0,            ZTID_VOID,   SUBDATACURSORPOS,          0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getScript",                  0,           ZTID_FLOAT,   SUBDATASCRIPT,             0,  { ZTID_SUBSCREENDATA },{} },
	{ "setScript",                  0,            ZTID_VOID,   SUBDATASCRIPT,             0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getInitD[]",                 0,         ZTID_UNTYPED,   SUBDATAINITD,              0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setInitD[]",                 0,            ZTID_VOID,   SUBDATAINITD,              0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getBtnPageLeft[]",           0,            ZTID_BOOL,   SUBDATABTNLEFT,            0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setBtnPageLeft[]",           0,            ZTID_VOID,   SUBDATABTNLEFT,            0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getBtnPageRight[]",          0,            ZTID_BOOL,   SUBDATABTNRIGHT,           0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setBtnPageRight[]",          0,            ZTID_VOID,   SUBDATABTNRIGHT,           0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_BOOL },{} },
	
	{ "getTransLeftType",           0,           ZTID_FLOAT,   SUBDATATRANSLEFTTY,        0,  { ZTID_SUBSCREENDATA },{} },
	{ "setTransLeftType",           0,            ZTID_VOID,   SUBDATATRANSLEFTTY,        0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getTransLeftSFX",            0,           ZTID_FLOAT,   SUBDATATRANSLEFTSFX,       0,  { ZTID_SUBSCREENDATA },{} },
	{ "setTransLeftSFX",            0,            ZTID_VOID,   SUBDATATRANSLEFTSFX,       0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getTransLeftFlags[]",        0,            ZTID_BOOL,   SUBDATATRANSLEFTFLAGS,     0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setTransLeftFlags[]",        0,            ZTID_VOID,   SUBDATATRANSLEFTFLAGS,     0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getTransLeftArgs[]",         0,           ZTID_FLOAT,   SUBDATATRANSLEFTARGS,      0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setTransLeftArgs[]",         0,            ZTID_VOID,   SUBDATATRANSLEFTARGS,      0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getTransRightType",          0,           ZTID_FLOAT,   SUBDATATRANSRIGHTTY,       0,  { ZTID_SUBSCREENDATA },{} },
	{ "setTransRightType",          0,            ZTID_VOID,   SUBDATATRANSRIGHTTY,       0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getTransRightSFX",           0,           ZTID_FLOAT,   SUBDATATRANSRIGHTSFX,      0,  { ZTID_SUBSCREENDATA },{} },
	{ "setTransRightSFX",           0,            ZTID_VOID,   SUBDATATRANSRIGHTSFX,      0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getTransRightFlags[]",       0,            ZTID_BOOL,   SUBDATATRANSRIGHTFLAGS,    0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setTransRightFlags[]",       0,            ZTID_VOID,   SUBDATATRANSRIGHTFLAGS,    0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getTransRightArgs[]",        0,           ZTID_FLOAT,   SUBDATATRANSRIGHTARGS,     0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setTransRightArgs[]",        0,            ZTID_VOID,   SUBDATATRANSRIGHTARGS,     0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getSelectorDestX",           0,           ZTID_FLOAT,   SUBDATASELECTORDSTX,       0,  { ZTID_SUBSCREENDATA },{} },
	{ "setSelectorDestX",           0,            ZTID_VOID,   SUBDATASELECTORDSTX,       0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getSelectorDestY",           0,           ZTID_FLOAT,   SUBDATASELECTORDSTY,       0,  { ZTID_SUBSCREENDATA },{} },
	{ "setSelectorDestY",           0,            ZTID_VOID,   SUBDATASELECTORDSTY,       0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getSelectorDestW",           0,           ZTID_FLOAT,   SUBDATASELECTORDSTW,       0,  { ZTID_SUBSCREENDATA },{} },
	{ "setSelectorDestW",           0,            ZTID_VOID,   SUBDATASELECTORDSTW,       0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getSelectorDestH",           0,           ZTID_FLOAT,   SUBDATASELECTORDSTH,       0,  { ZTID_SUBSCREENDATA },{} },
	{ "setSelectorDestH",           0,            ZTID_VOID,   SUBDATASELECTORDSTH,       0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	
	{ "getSelectorWid[]",           0,           ZTID_FLOAT,   SUBDATASELECTORWID,        0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setSelectorWid[]",           0,            ZTID_VOID,   SUBDATASELECTORWID,        0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getSelectorHei[]",           0,           ZTID_FLOAT,   SUBDATASELECTORHEI,        0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setSelectorHei[]",           0,            ZTID_VOID,   SUBDATASELECTORHEI,        0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getSelectorTile[]",          0,           ZTID_FLOAT,   SUBDATASELECTORTILE,       0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setSelectorTile[]",          0,            ZTID_VOID,   SUBDATASELECTORTILE,       0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getSelectorCSet[]",          0,           ZTID_FLOAT,   SUBDATASELECTORCSET,       0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setSelectorCSet[]",          0,            ZTID_VOID,   SUBDATASELECTORCSET,       0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getSelectorFlashCSet[]",     0,           ZTID_FLOAT,   SUBDATASELECTORFLASHCSET,  0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setSelectorFlashCSet[]",     0,            ZTID_VOID,   SUBDATASELECTORFLASHCSET,  0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getSelectorFrames[]",        0,           ZTID_FLOAT,   SUBDATASELECTORFRM,        0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setSelectorFrames[]",        0,            ZTID_VOID,   SUBDATASELECTORFRM,        0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getSelectorASpeed[]",        0,           ZTID_FLOAT,   SUBDATASELECTORASPD,       0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setSelectorASpeed[]",        0,            ZTID_VOID,   SUBDATASELECTORASPD,       0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getSelectorDelay[]",         0,           ZTID_FLOAT,   SUBDATASELECTORDELAY,      0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setSelectorDelay[]",         0,            ZTID_VOID,   SUBDATASELECTORDELAY,      0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	// Current Transition info
	{ "getTransClock",              0,           ZTID_FLOAT,   SUBDATATRANSCLK,           0,  { ZTID_SUBSCREENDATA },{} },
	{ "setTransClock",              0,            ZTID_VOID,   SUBDATATRANSCLK,           0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getTransType",               0,           ZTID_FLOAT,   SUBDATATRANSTY,            0,  { ZTID_SUBSCREENDATA },{} },
	{ "setTransType",               0,            ZTID_VOID,   SUBDATATRANSTY,            0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getTransFlags[]",            0,            ZTID_BOOL,   SUBDATATRANSFLAGS,         0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setTransFlags[]",            0,            ZTID_VOID,   SUBDATATRANSFLAGS,         0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getTransArgs[]",             0,           ZTID_FLOAT,   SUBDATATRANSARGS,          0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "setTransArgs[]",             0,            ZTID_VOID,   SUBDATATRANSARGS,          0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getTransFromPage",           0,           ZTID_FLOAT,   SUBDATATRANSFROMPG,        0,  { ZTID_SUBSCREENDATA },{} },
	{ "setTransFromPage",           0,            ZTID_VOID,   SUBDATATRANSFROMPG,        0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	{ "getTransToPage",             0,           ZTID_FLOAT,   SUBDATATRANSTOPG,          0,  { ZTID_SUBSCREENDATA },{} },
	{ "setTransToPage",             0,            ZTID_VOID,   SUBDATATRANSTOPG,          0,  { ZTID_SUBSCREENDATA, ZTID_FLOAT },{} },
	
	{ "",                           0,            ZTID_VOID,   -1,                        0,  {},{} }
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
	{ "getType",                    0,           ZTID_FLOAT,   SUBWIDGTYPE,               0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setType",                    0,            ZTID_VOID,   SUBWIDGTYPE,       FL_RDONLY,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getIndex",                   0,           ZTID_FLOAT,   SUBWIDGINDEX,              0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setIndex",                   0,            ZTID_VOID,   SUBWIDGINDEX,      FL_RDONLY,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getPage",                    0,   ZTID_SUBSCREENPAGE,   SUBWIDGPAGE,               0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setPage",                    0,            ZTID_VOID,   SUBWIDGPAGE,       FL_RDONLY,  { ZTID_SUBSCREENWIDGET, ZTID_SUBSCREENPAGE },{} },
	{ "getPos",                     0,           ZTID_FLOAT,   SUBWIDGPOS,                0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setPos",                     0,            ZTID_VOID,   SUBWIDGPOS,                0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getPosDirs[]",               0,           ZTID_FLOAT,   SUBWIDGPOSES,              0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setPosDirs[]",               0,            ZTID_VOID,   SUBWIDGPOSES,              0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getVisibleFlags[]",          0,            ZTID_BOOL,   SUBWIDGPOSFLAG,            0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setVisibleFlags[]",          0,            ZTID_VOID,   SUBWIDGPOSFLAG,            0,  { ZTID_SUBSCREENWIDGET, ZTID_BOOL },{} },
	{ "getX",                       0,           ZTID_FLOAT,   SUBWIDGX,                  0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setX",                       0,            ZTID_VOID,   SUBWIDGX,                  0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getY",                       0,           ZTID_FLOAT,   SUBWIDGY,                  0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setY",                       0,            ZTID_VOID,   SUBWIDGY,                  0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getW",                       0,           ZTID_FLOAT,   SUBWIDGW,                  0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setW",                       0,            ZTID_VOID,   SUBWIDGW,                  0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getH",                       0,           ZTID_FLOAT,   SUBWIDGH,                  0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setH",                       0,            ZTID_VOID,   SUBWIDGH,                  0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getGenFlags[]",              0,            ZTID_BOOL,   SUBWIDGGENFLAG,            0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setGenFlags[]",              0,            ZTID_VOID,   SUBWIDGGENFLAG,            0,  { ZTID_SUBSCREENWIDGET, ZTID_BOOL },{} },
	{ "getFlags[]",                 0,            ZTID_BOOL,   SUBWIDGFLAG,               0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setFlags[]",                 0,            ZTID_VOID,   SUBWIDGFLAG,               0,  { ZTID_SUBSCREENWIDGET, ZTID_BOOL },{} },
	
	//Selector Override
	{ "getSelectorDestX",           0,           ZTID_FLOAT,   SUBWIDGSELECTORDSTX,       0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setSelectorDestX",           0,            ZTID_VOID,   SUBWIDGSELECTORDSTX,       0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getSelectorDestY",           0,           ZTID_FLOAT,   SUBWIDGSELECTORDSTY,       0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setSelectorDestY",           0,            ZTID_VOID,   SUBWIDGSELECTORDSTY,       0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getSelectorDestW",           0,           ZTID_FLOAT,   SUBWIDGSELECTORDSTW,       0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setSelectorDestW",           0,            ZTID_VOID,   SUBWIDGSELECTORDSTW,       0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getSelectorDestH",           0,           ZTID_FLOAT,   SUBWIDGSELECTORDSTH,       0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setSelectorDestH",           0,            ZTID_VOID,   SUBWIDGSELECTORDSTH,       0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getSelectorWid[]",           0,           ZTID_FLOAT,   SUBWIDGSELECTORWID,        0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "setSelectorWid[]",           0,            ZTID_VOID,   SUBWIDGSELECTORWID,        0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getSelectorHei[]",           0,           ZTID_FLOAT,   SUBWIDGSELECTORHEI,        0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "setSelectorHei[]",           0,            ZTID_VOID,   SUBWIDGSELECTORHEI,        0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getSelectorTile[]",          0,           ZTID_FLOAT,   SUBWIDGSELECTORTILE,       0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "setSelectorTile[]",          0,            ZTID_VOID,   SUBWIDGSELECTORTILE,       0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getSelectorCSet[]",          0,           ZTID_FLOAT,   SUBWIDGSELECTORCSET,       0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "setSelectorCSet[]",          0,            ZTID_VOID,   SUBWIDGSELECTORCSET,       0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getSelectorFlashCSet[]",     0,           ZTID_FLOAT,   SUBWIDGSELECTORFLASHCSET,  0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "setSelectorFlashCSet[]",     0,            ZTID_VOID,   SUBWIDGSELECTORFLASHCSET,  0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getSelectorFrames[]",        0,           ZTID_FLOAT,   SUBWIDGSELECTORFRM,        0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "setSelectorFrames[]",        0,            ZTID_VOID,   SUBWIDGSELECTORFRM,        0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getSelectorASpeed[]",        0,           ZTID_FLOAT,   SUBWIDGSELECTORASPD,       0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "setSelectorASpeed[]",        0,            ZTID_VOID,   SUBWIDGSELECTORASPD,       0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getSelectorDelay[]",         0,           ZTID_FLOAT,   SUBWIDGSELECTORDELAY,      0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "setSelectorDelay[]",         0,            ZTID_VOID,   SUBWIDGSELECTORDELAY,      0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	//Selected text override
	{ "GetSelTextOverride",         0,            ZTID_VOID,   -1,                        0,  { ZTID_SUBSCREENWIDGET, ZTID_CHAR },{} },
	{ "SetSelTextOverride",         0,            ZTID_VOID,   -1,                        0,  { ZTID_SUBSCREENWIDGET, ZTID_CHAR },{} },
	
	//OnPress Generic Script
	{ "getPressScript",             0,           ZTID_FLOAT,   SUBWIDGPRESSSCRIPT,        0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setPressScript",             0,            ZTID_VOID,   SUBWIDGPRESSSCRIPT,        0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getPressInitD[]",            0,         ZTID_UNTYPED,   SUBWIDGPRESSINITD,         0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "setPressInitD[]",            0,            ZTID_VOID,   SUBWIDGPRESSINITD,         0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getBtnPressScript[]",        0,            ZTID_BOOL,   SUBWIDGBTNPRESS,           0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "setBtnPressScript[]",        0,            ZTID_VOID,   SUBWIDGBTNPRESS,           0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT, ZTID_BOOL },{} },
	
	//OnPress Page Change
	{ "getBtnPageChange[]",         0,            ZTID_BOOL,   SUBWIDGBTNPG,              0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "setBtnPageChange[]",         0,            ZTID_VOID,   SUBWIDGBTNPG,              0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getPageMode",                0,           ZTID_FLOAT,   SUBWIDGPGMODE,             0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setPageMode",                0,            ZTID_VOID,   SUBWIDGPGMODE,             0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getTargetPage",              0,           ZTID_FLOAT,   SUBWIDGPGTARG,             0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setTargetPage",              0,            ZTID_VOID,   SUBWIDGPGTARG,             0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getPageTransType",           0,           ZTID_FLOAT,   SUBWIDGTRANSPGTY,          0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setPageTransType",           0,            ZTID_VOID,   SUBWIDGTRANSPGTY,          0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getPageTransSFX",            0,           ZTID_FLOAT,   SUBWIDGTRANSPGSFX,         0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setPageTransSFX",            0,            ZTID_VOID,   SUBWIDGTRANSPGSFX,         0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getPageTransFlags[]",        0,            ZTID_BOOL,   SUBWIDGTRANSPGFLAGS,       0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "setPageTransFlags[]",        0,            ZTID_VOID,   SUBWIDGTRANSPGFLAGS,       0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getPageTransArgs[]",         0,           ZTID_FLOAT,   SUBWIDGTRANSPGARGS,        0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "setPageTransArgs[]",         0,            ZTID_VOID,   SUBWIDGTRANSPGARGS,        0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	//Type Specific
	{ "getCSet[]",                  0,           ZTID_FLOAT,   SUBWIDGTY_CSET,            0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "setCSet[]",                  0,            ZTID_VOID,   SUBWIDGTY_CSET,            0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getTile[]",                  0,           ZTID_FLOAT,   SUBWIDGTY_TILE,            0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "setTile[]",                  0,            ZTID_VOID,   SUBWIDGTY_TILE,            0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getFont",                    0,           ZTID_FLOAT,   SUBWIDGTY_FONT,            0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setFont",                    0,            ZTID_VOID,   SUBWIDGTY_FONT,            0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getAlign",                   0,           ZTID_FLOAT,   SUBWIDGTY_ALIGN,           0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setAlign",                   0,            ZTID_VOID,   SUBWIDGTY_ALIGN,           0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getShadowType",              0,           ZTID_FLOAT,   SUBWIDGTY_SHADOWTY,        0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setShadowType",              0,            ZTID_VOID,   SUBWIDGTY_SHADOWTY,        0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getColorText",               0,           ZTID_FLOAT,   SUBWIDGTY_COLOR_TXT,       0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setColorText",               0,            ZTID_VOID,   SUBWIDGTY_COLOR_TXT,       0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getColorShadow",             0,           ZTID_FLOAT,   SUBWIDGTY_COLOR_SHD,       0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setColorShadow",             0,            ZTID_VOID,   SUBWIDGTY_COLOR_SHD,       0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getColorBG",                 0,           ZTID_FLOAT,   SUBWIDGTY_COLOR_BG,        0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setColorBG",                 0,            ZTID_VOID,   SUBWIDGTY_COLOR_BG,        0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "GetText",                    0,            ZTID_VOID,   -1,                        0,  { ZTID_SUBSCREENWIDGET, ZTID_CHAR },{} },
	{ "SetText",                    0,            ZTID_VOID,   -1,                        0,  { ZTID_SUBSCREENWIDGET, ZTID_CHAR },{} },
	{ "getColorOutline",            0,           ZTID_FLOAT,   SUBWIDGTY_COLOR_OLINE,     0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setColorOutline",            0,            ZTID_VOID,   SUBWIDGTY_COLOR_OLINE,     0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getColorFill",               0,           ZTID_FLOAT,   SUBWIDGTY_COLOR_FILL,      0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setColorFill",               0,            ZTID_VOID,   SUBWIDGTY_COLOR_FILL,      0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getButton",                  0,           ZTID_FLOAT,   SUBWIDGTY_BUTTON,          0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setButton",                  0,            ZTID_VOID,   SUBWIDGTY_BUTTON,          0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getCounter[]",               0,           ZTID_FLOAT,   SUBWIDGTY_COUNTERS,        0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "setCounter[]",               0,            ZTID_VOID,   SUBWIDGTY_COUNTERS,        0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getMinDigits",               0,           ZTID_FLOAT,   SUBWIDGTY_MINDIG,          0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setMinDigits",               0,            ZTID_VOID,   SUBWIDGTY_MINDIG,          0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getMaxDigits",               0,           ZTID_FLOAT,   SUBWIDGTY_MAXDIG,          0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setMaxDigits",               0,            ZTID_VOID,   SUBWIDGTY_MAXDIG,          0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getInfiniteItem",            0,           ZTID_FLOAT,   SUBWIDGTY_INFITM,          0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setInfiniteItem",            0,            ZTID_VOID,   SUBWIDGTY_INFITM,          0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getInfiniteChar",            0,            ZTID_CHAR,   SUBWIDGTY_INFCHAR,         0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setInfiniteChar",            0,            ZTID_VOID,   SUBWIDGTY_INFCHAR,         0,  { ZTID_SUBSCREENWIDGET, ZTID_CHAR },{} },
	{ "getCostIndex",               0,           ZTID_FLOAT,   SUBWIDGTY_COSTIND,         0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setCostIndex",               0,            ZTID_VOID,   SUBWIDGTY_COSTIND,         0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getColorPlayer",             0,           ZTID_FLOAT,   SUBWIDGTY_COLOR_PLAYER,    0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setColorPlayer",             0,            ZTID_VOID,   SUBWIDGTY_COLOR_PLAYER,    0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getColorCompassBlink",       0,           ZTID_FLOAT,   SUBWIDGTY_COLOR_CMPBLNK,   0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setColorCompassBlink",       0,            ZTID_VOID,   SUBWIDGTY_COLOR_CMPBLNK,   0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getColorCompassOff",         0,           ZTID_FLOAT,   SUBWIDGTY_COLOR_CMPOFF,    0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setColorCompassOff",         0,            ZTID_VOID,   SUBWIDGTY_COLOR_CMPOFF,    0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getColorRoom",               0,           ZTID_FLOAT,   SUBWIDGTY_COLOR_ROOM,      0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setColorRoom",               0,            ZTID_VOID,   SUBWIDGTY_COLOR_ROOM,      0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getItemClass",               0,           ZTID_FLOAT,   SUBWIDGTY_ITEMCLASS,       0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setItemClass",               0,            ZTID_VOID,   SUBWIDGTY_ITEMCLASS,       0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getItemID",                  0,           ZTID_FLOAT,   SUBWIDGTY_ITEMID,          0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setItemID",                  0,            ZTID_VOID,   SUBWIDGTY_ITEMID,          0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getFrameTile",               0,           ZTID_FLOAT,   SUBWIDGTY_FRAMETILE,       0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setFrameTile",               0,            ZTID_VOID,   SUBWIDGTY_FRAMETILE,       0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getFrameCSet",               0,           ZTID_FLOAT,   SUBWIDGTY_FRAMECSET,       0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setFrameCSet",               0,            ZTID_VOID,   SUBWIDGTY_FRAMECSET,       0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getPieceTile",               0,           ZTID_FLOAT,   SUBWIDGTY_PIECETILE,       0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setPieceTile",               0,            ZTID_VOID,   SUBWIDGTY_PIECETILE,       0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getPieceCSet",               0,           ZTID_FLOAT,   SUBWIDGTY_PIECECSET,       0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setPieceCSet",               0,            ZTID_VOID,   SUBWIDGTY_PIECECSET,       0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getFlip",                    0,           ZTID_FLOAT,   SUBWIDGTY_FLIP,            0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setFlip",                    0,            ZTID_VOID,   SUBWIDGTY_FLIP,            0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getNumber",                  0,           ZTID_FLOAT,   SUBWIDGTY_NUMBER,          0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setNumber",                  0,            ZTID_VOID,   SUBWIDGTY_NUMBER,          0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getCorner[]",                0,           ZTID_FLOAT,   SUBWIDGTY_CORNER,          0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "setCorner[]",                0,            ZTID_VOID,   SUBWIDGTY_CORNER,          0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getFrames",                  0,           ZTID_FLOAT,   SUBWIDGTY_FRAMES,          0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setFrames",                  0,            ZTID_VOID,   SUBWIDGTY_FRAMES,          0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getSpeed",                   0,           ZTID_FLOAT,   SUBWIDGTY_SPEED,           0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setSpeed",                   0,            ZTID_VOID,   SUBWIDGTY_SPEED,           0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getDelay",                   0,           ZTID_FLOAT,   SUBWIDGTY_DELAY,           0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setDelay",                   0,            ZTID_VOID,   SUBWIDGTY_DELAY,           0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getContainer",               0,           ZTID_FLOAT,   SUBWIDGTY_CONTAINER,       0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setContainer",               0,            ZTID_VOID,   SUBWIDGTY_CONTAINER,       0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getGaugeWid",                0,           ZTID_FLOAT,   SUBWIDGTY_GAUGE_WID,       0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setGaugeWid",                0,            ZTID_VOID,   SUBWIDGTY_GAUGE_WID,       0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getGaugeHei",                0,           ZTID_FLOAT,   SUBWIDGTY_GAUGE_HEI,       0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setGaugeHei",                0,            ZTID_VOID,   SUBWIDGTY_GAUGE_HEI,       0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getUnits",                   0,           ZTID_FLOAT,   SUBWIDGTY_UNITS,           0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setUnits",                   0,            ZTID_VOID,   SUBWIDGTY_UNITS,           0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getHSpace",                  0,           ZTID_FLOAT,   SUBWIDGTY_HSPACE,          0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setHSpace",                  0,            ZTID_VOID,   SUBWIDGTY_HSPACE,          0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getVSpace",                  0,           ZTID_FLOAT,   SUBWIDGTY_VSPACE,          0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setVSpace",                  0,            ZTID_VOID,   SUBWIDGTY_VSPACE,          0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getGridX",                   0,           ZTID_FLOAT,   SUBWIDGTY_GRIDX,           0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setGridX",                   0,            ZTID_VOID,   SUBWIDGTY_GRIDX,           0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getGridY",                   0,           ZTID_FLOAT,   SUBWIDGTY_GRIDY,           0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setGridY",                   0,            ZTID_VOID,   SUBWIDGTY_GRIDY,           0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getAnimVal",                 0,           ZTID_FLOAT,   SUBWIDGTY_ANIMVAL,         0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setAnimVal",                 0,            ZTID_VOID,   SUBWIDGTY_ANIMVAL,         0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getShowDrain",               0,           ZTID_FLOAT,   SUBWIDGTY_SHOWDRAIN,       0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setShowDrain",               0,            ZTID_VOID,   SUBWIDGTY_SHOWDRAIN,       0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getPerContainer",            0,           ZTID_FLOAT,   SUBWIDGTY_PERCONTAINER,    0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setPerContainer",            0,            ZTID_VOID,   SUBWIDGTY_PERCONTAINER,    0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	{ "getTabSize",                 0,           ZTID_FLOAT,   SUBWIDGTY_TABSIZE,         0,  { ZTID_SUBSCREENWIDGET },{} },
	{ "setTabSize",                 0,            ZTID_VOID,   SUBWIDGTY_TABSIZE,         0,  { ZTID_SUBSCREENWIDGET, ZTID_FLOAT },{} },
	
	{ "",                           0,            ZTID_VOID,   -1,                        0,  {},{} }
};

SubscreenWidgetSymbols::SubscreenWidgetSymbols()
{
	table = SubscreenWidgetTable;
	refVar = REFSUBSCREENWIDG;
}

void SubscreenWidgetSymbols::generateCode()
{
	//void GetSelTextOverride(subscreenwidget, char)
	{
		Function* function = getFunction("GetSelTextOverride");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new OGetSubWidgSelTxtOverride(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void SetSelTextOverride(subscreenwidget, char)
	{
		Function* function = getFunction("SetSelTextOverride");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new OSetSubWidgSelTxtOverride(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void GetText(subscreenwidget, char)
	{
		Function* function = getFunction("GetText");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new OSubWidgTy_GetText(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void SetText(subscreenwidget, char)
	{
		Function* function = getFunction("SetText");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new OSubWidgTy_SetText(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
}


