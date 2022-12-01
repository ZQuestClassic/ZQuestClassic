#include "SymbolDefs.h"

InputSymbols InputSymbols::singleton = InputSymbols();

static AccessorTable InputTable[] =
{
//	All of these return a function label error when used:
//	  name,                     rettype,                  setorget,     var,              numindex,      funcFlags,                            numParams,   params
	 { "getJoypad[]",            ZTID_BOOL,          GETTER,       JOYPADPRESS,      18,            0,                                    2,           { ZTID_INPUT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPress[]",             ZTID_BOOL,          GETTER,       BUTTONPRESS,      18,            0,                                    2,           { ZTID_INPUT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPress[]",             ZTID_VOID,          SETTER,       BUTTONPRESS,      18,            0,                                    3,           { ZTID_INPUT, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getButton[]",            ZTID_BOOL,          GETTER,       BUTTONINPUT,      18,            0,                                    2,           { ZTID_INPUT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setButton[]",            ZTID_VOID,          SETTER,       BUTTONINPUT,      18,            0,                                    3,           { ZTID_INPUT, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getHold[]",              ZTID_BOOL,          GETTER,       BUTTONHELD,       18,            0,                                    2,           { ZTID_INPUT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setHold[]",              ZTID_VOID,          SETTER,       BUTTONHELD,       18,            0,                                    3,           { ZTID_INPUT, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getReadKey[]",           ZTID_BOOL,          GETTER,       READKEY,          127,           0,                                    2,           { ZTID_INPUT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "getSimulateKeypress[]",  ZTID_BOOL,          GETTER,       SIMULATEKEYPRESS, 127,           0,                                    2,           { ZTID_INPUT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setSimulateKeypress[]",  ZTID_VOID,          SETTER,       SIMULATEKEYPRESS, 127,           0,                                    3,           { ZTID_INPUT, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getKeyRaw[]",            ZTID_BOOL,          GETTER,       RAWKEY,           127,           0,                                    2,           { ZTID_INPUT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setKeyRaw[]",            ZTID_VOID,          SETTER,       RAWKEY,           127,           0,                                    3,           { ZTID_INPUT, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getKeyPress[]",          ZTID_BOOL,          GETTER,       KEYPRESS,         127,           0,                                    2,           { ZTID_INPUT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setKeyPress[]",          ZTID_VOID,          SETTER,       KEYPRESS,         127,           0,                                    3,           { ZTID_INPUT, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getKey[]",               ZTID_BOOL,          GETTER,       KEYINPUT,         127,           0,                                    2,           { ZTID_INPUT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setKey[]",               ZTID_VOID,          SETTER,       KEYINPUT,         127,           0,                                    3,           { ZTID_INPUT, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMouse[]",             ZTID_UNTYPED,       GETTER,       MOUSEARR,         6,             0,                                    2,           { ZTID_INPUT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMouse[]",             ZTID_VOID,          SETTER,       MOUSEARR,         6,             0,                                    3,           { ZTID_INPUT, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getKeyBindings[]",       ZTID_FLOAT,         GETTER,       KEYBINDINGS,      14,            0,                                    2,           { ZTID_INPUT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setKeyBindings[]",       ZTID_VOID,          SETTER,       KEYBINDINGS,      14,            0,                                    3,           { ZTID_INPUT, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getModifierKeys",        ZTID_FLOAT,         GETTER,       KEYMODIFIERS,     1,             0,                                    1,           { ZTID_INPUT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setModifierKeys",        ZTID_VOID,          SETTER,       KEYMODIFIERS,     1,             0,                                    2,           { ZTID_INPUT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getDisableKey[]",        ZTID_BOOL,          GETTER,       DISABLEKEY,       127,           0,                                    2,           { ZTID_INPUT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDisableKey[]",        ZTID_VOID,          SETTER,       DISABLEKEY,       127,           0,                                    3,           { ZTID_INPUT, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDisableButton[]",     ZTID_BOOL,          GETTER,       DISABLEBUTTON,    18,            0,                                    2,           { ZTID_INPUT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDisableButton[]",     ZTID_VOID,          SETTER,       DISABLEBUTTON,    18,            0,                                    3,           { ZTID_INPUT, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "",                       -1,                       -1,           -1,               -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

InputSymbols::InputSymbols()
{
    table = InputTable;
    refVar = NUL;
}

void InputSymbols::generateCode()
{
}

