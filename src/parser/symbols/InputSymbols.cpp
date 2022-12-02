#include "SymbolDefs.h"

InputSymbols InputSymbols::singleton = InputSymbols();

static AccessorTable InputTable[] =
{
//	All of these return a function label error when used:
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
	{ "getJoypad[]",                0,          ZTID_BOOL,   JOYPADPRESS,               0,  { ZTID_INPUT, ZTID_FLOAT },{} },
	{ "getPress[]",                 0,          ZTID_BOOL,   BUTTONPRESS,               0,  { ZTID_INPUT, ZTID_FLOAT },{} },
	{ "setPress[]",                 0,          ZTID_VOID,   BUTTONPRESS,               0,  { ZTID_INPUT, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getButton[]",                0,          ZTID_BOOL,   BUTTONINPUT,               0,  { ZTID_INPUT, ZTID_FLOAT },{} },
	{ "setButton[]",                0,          ZTID_VOID,   BUTTONINPUT,               0,  { ZTID_INPUT, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getHold[]",                  0,          ZTID_BOOL,   BUTTONHELD,                0,  { ZTID_INPUT, ZTID_FLOAT },{} },
	{ "setHold[]",                  0,          ZTID_VOID,   BUTTONHELD,                0,  { ZTID_INPUT, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getReadKey[]",               0,          ZTID_BOOL,   READKEY,                   0,  { ZTID_INPUT, ZTID_FLOAT },{} },
//	{ "getSimulateKeypress[]",      0,          ZTID_BOOL,   SIMULATEKEYPRESS,          0,  { ZTID_INPUT, ZTID_FLOAT },{} },
	{ "setSimulateKeypress[]",      0,          ZTID_VOID,   SIMULATEKEYPRESS,          0,  { ZTID_INPUT, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getKeyRaw[]",                0,          ZTID_BOOL,   RAWKEY,                    0,  { ZTID_INPUT, ZTID_FLOAT },{} },
	{ "setKeyRaw[]",                0,          ZTID_VOID,   RAWKEY,                    0,  { ZTID_INPUT, ZTID_FLOAT, ZTID_BOOL },{} },
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

