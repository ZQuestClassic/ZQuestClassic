#include "SymbolDefs.h"

InputSymbols InputSymbols::singleton = InputSymbols();

static AccessorTable InputTable[] =
{
//	All of these return a function label error when used:
//	  name,                     rettype,                  setorget,     var,              numindex,      funcFlags,                            numParams,   params
	 { "getJoypad[]",            ZVARTYPEID_BOOL,          GETTER,       JOYPADPRESS,      18,            0,                                    2,           { ZVARTYPEID_INPUT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPress[]",             ZVARTYPEID_BOOL,          GETTER,       BUTTONPRESS,      18,            0,                                    2,           { ZVARTYPEID_INPUT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPress[]",             ZVARTYPEID_VOID,          SETTER,       BUTTONPRESS,      18,            0,                                    3,           { ZVARTYPEID_INPUT, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getButton[]",            ZVARTYPEID_BOOL,          GETTER,       BUTTONINPUT,      18,            0,                                    2,           { ZVARTYPEID_INPUT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setButton[]",            ZVARTYPEID_VOID,          SETTER,       BUTTONINPUT,      18,            0,                                    3,           { ZVARTYPEID_INPUT, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getHold[]",              ZVARTYPEID_BOOL,          GETTER,       BUTTONHELD,       18,            0,                                    2,           { ZVARTYPEID_INPUT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setHold[]",              ZVARTYPEID_VOID,          SETTER,       BUTTONHELD,       18,            0,                                    3,           { ZVARTYPEID_INPUT, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getReadKey[]",           ZVARTYPEID_BOOL,          GETTER,       READKEY,          127,           0,                                    2,           { ZVARTYPEID_INPUT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "getSimulateKeypress[]",  ZVARTYPEID_BOOL,          GETTER,       SIMULATEKEYPRESS, 127,           0,                                    2,           { ZVARTYPEID_INPUT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setSimulateKeypress[]",  ZVARTYPEID_VOID,          SETTER,       SIMULATEKEYPRESS, 127,           0,                                    3,           { ZVARTYPEID_INPUT, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getKeyRaw[]",            ZVARTYPEID_BOOL,          GETTER,       RAWKEY,           127,           0,                                    2,           { ZVARTYPEID_INPUT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setKeyRaw[]",            ZVARTYPEID_VOID,          SETTER,       RAWKEY,           127,           0,                                    3,           { ZVARTYPEID_INPUT, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getKeyPress[]",          ZVARTYPEID_BOOL,          GETTER,       KEYPRESS,         127,           0,                                    2,           { ZVARTYPEID_INPUT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setKeyPress[]",          ZVARTYPEID_VOID,          SETTER,       KEYPRESS,         127,           0,                                    3,           { ZVARTYPEID_INPUT, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getKey[]",               ZVARTYPEID_BOOL,          GETTER,       KEYINPUT,         127,           0,                                    2,           { ZVARTYPEID_INPUT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setKey[]",               ZVARTYPEID_VOID,          SETTER,       KEYINPUT,         127,           0,                                    3,           { ZVARTYPEID_INPUT, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMouse[]",             ZVARTYPEID_UNTYPED,       GETTER,       MOUSEARR,         6,             0,                                    2,           { ZVARTYPEID_INPUT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMouse[]",             ZVARTYPEID_VOID,          SETTER,       MOUSEARR,         6,             0,                                    3,           { ZVARTYPEID_INPUT, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getKeyBindings[]",       ZVARTYPEID_FLOAT,         GETTER,       KEYBINDINGS,      14,            0,                                    2,           { ZVARTYPEID_INPUT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setKeyBindings[]",       ZVARTYPEID_VOID,          SETTER,       KEYBINDINGS,      14,            0,                                    3,           { ZVARTYPEID_INPUT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getModifierKeys",        ZVARTYPEID_FLOAT,         GETTER,       KEYMODIFIERS,     1,             0,                                    1,           { ZVARTYPEID_INPUT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setModifierKeys",        ZVARTYPEID_VOID,          SETTER,       KEYMODIFIERS,     1,             0,                                    2,           { ZVARTYPEID_INPUT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getDisableKey[]",        ZVARTYPEID_BOOL,          GETTER,       DISABLEKEY,       127,           0,                                    2,           { ZVARTYPEID_INPUT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDisableKey[]",        ZVARTYPEID_VOID,          SETTER,       DISABLEKEY,       127,           0,                                    3,           { ZVARTYPEID_INPUT, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDisableButton[]",     ZVARTYPEID_BOOL,          GETTER,       DISABLEBUTTON,    18,            0,                                    2,           { ZVARTYPEID_INPUT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDisableButton[]",     ZVARTYPEID_VOID,          SETTER,       DISABLEBUTTON,    18,            0,                                    3,           { ZVARTYPEID_INPUT, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
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

