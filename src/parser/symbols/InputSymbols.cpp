#include "SymbolDefs.h"

InputSymbols InputSymbols::singleton = InputSymbols();

static AccessorTable InputTable[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
	{ "getPress[]",                 0,          ZTID_BOOL,   BUTTONPRESS,               0,  { ZTID_INPUT, ZTID_FLOAT },{} },
	{ "setPress[]",                 0,          ZTID_VOID,   BUTTONPRESS,               0,  { ZTID_INPUT, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getButton[]",                0,          ZTID_BOOL,   BUTTONINPUT,               0,  { ZTID_INPUT, ZTID_FLOAT },{} },
	{ "setButton[]",                0,          ZTID_VOID,   BUTTONINPUT,               0,  { ZTID_INPUT, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getKeyPress[]",              0,          ZTID_BOOL,   KEYPRESS,                  0,  { ZTID_INPUT, ZTID_FLOAT },{} },
	{ "setKeyPress[]",              0,          ZTID_VOID,   KEYPRESS,                  0,  { ZTID_INPUT, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getKey[]",                   0,          ZTID_BOOL,   KEYINPUT,                  0,  { ZTID_INPUT, ZTID_FLOAT },{} },
	{ "setKey[]",                   0,          ZTID_VOID,   KEYINPUT,                  0,  { ZTID_INPUT, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getMouse[]",                 0,       ZTID_UNTYPED,   MOUSEARR,                  0,  { ZTID_INPUT, ZTID_FLOAT },{} },
	{ "setMouse[]",                 0,          ZTID_VOID,   MOUSEARR,                  0,  { ZTID_INPUT, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getKeyBindings[]",           0,         ZTID_FLOAT,   KEYBINDINGS,               0,  { ZTID_INPUT, ZTID_FLOAT },{} },
	{ "setKeyBindings[]",           0,          ZTID_VOID,   KEYBINDINGS,               0,  { ZTID_INPUT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getModifierKeys",            0,         ZTID_FLOAT,   KEYMODIFIERS,              0,  { ZTID_INPUT },{} },
	{ "setModifierKeys",            0,          ZTID_VOID,   KEYMODIFIERS,              0,  { ZTID_INPUT, ZTID_FLOAT },{} },
	
	{ "getDisableKey[]",            0,          ZTID_BOOL,   DISABLEKEY,                0,  { ZTID_INPUT, ZTID_FLOAT },{} },
	{ "setDisableKey[]",            0,          ZTID_VOID,   DISABLEKEY,                0,  { ZTID_INPUT, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getDisableButton[]",         0,          ZTID_BOOL,   DISABLEBUTTON,             0,  { ZTID_INPUT, ZTID_FLOAT },{} },
	{ "setDisableButton[]",         0,          ZTID_VOID,   DISABLEBUTTON,             0,  { ZTID_INPUT, ZTID_FLOAT, ZTID_BOOL },{} },
	
	//Not documenting in web docs
	{ "getJoypad[]",                0,          ZTID_BOOL,   JOYPADPRESS,               0,  { ZTID_INPUT, ZTID_FLOAT },{} },
	{ "setJoypad[]",                0,          ZTID_VOID,   JOYPADPRESS,       FL_RDONLY,  { ZTID_INPUT, ZTID_FLOAT, ZTID_BOOL },{} },
//	{ "getSimulateKeypress[]",      0,          ZTID_BOOL,   SIMULATEKEYPRESS,          0,  { ZTID_INPUT, ZTID_FLOAT },{} },
	{ "setSimulateKeypress[]",      0,          ZTID_VOID,   SIMULATEKEYPRESS,          0,  { ZTID_INPUT, ZTID_FLOAT, ZTID_BOOL },{} },

	//Intentionally Undocumented
	{ "getReadKey[]",               0,          ZTID_BOOL,   READKEY,             FL_DEPR,  { ZTID_INPUT, ZTID_FLOAT },{},0,"Use '->KeyPress[]' instead!" },
	{ "setReadKey[]",               0,          ZTID_VOID,   READKEY,   FL_DEPR|FL_RDONLY,  { ZTID_INPUT, ZTID_FLOAT, ZTID_BOOL },{},0,"Use '->KeyPress[]' instead!" },
	{ "getHold[]",                  0,          ZTID_BOOL,   BUTTONHELD,          FL_DEPR,  { ZTID_INPUT, ZTID_FLOAT },{},0,"Use '->Button[]' instead!" },
	{ "setHold[]",                  0,          ZTID_VOID,   BUTTONHELD,          FL_DEPR,  { ZTID_INPUT, ZTID_FLOAT, ZTID_BOOL },{},0,"Use '->Button[]' instead!" },
	{ "getKeyRaw[]",                0,          ZTID_BOOL,   RAWKEY,              FL_DEPR,  { ZTID_INPUT, ZTID_FLOAT },{},0,"Unsafe!" },
	{ "setKeyRaw[]",                0,          ZTID_VOID,   RAWKEY,              FL_DEPR,  { ZTID_INPUT, ZTID_FLOAT, ZTID_BOOL },{},0,"Unsafe!" },
	
	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
};

InputSymbols::InputSymbols()
{
	table = InputTable;
	refVar = NUL;
}

void InputSymbols::generateCode()
{
}

