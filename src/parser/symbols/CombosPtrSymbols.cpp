#include "SymbolDefs.h"

CombosPtrSymbols CombosPtrSymbols::singleton = CombosPtrSymbols();

static AccessorTable CombosTable[] =
{
//	  name,                       rettype,                  setorget,     var,                          numindex,      funcFlags,                            numParams,   params
	
//	newcombo struct
	{ "PosX",                     ZTID_FLOAT,         GETTER,       COMBOXR,                      1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getX",                     ZTID_FLOAT,         GETTER,       COMBOXR,                      1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getY",                     ZTID_FLOAT,         GETTER,       COMBOYR,                      1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "PosY",                     ZTID_FLOAT,         GETTER,       COMBOYR,                      1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Pos",                      ZTID_FLOAT,         GETTER,       COMBOPOSR,                    1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPos",                   ZTID_FLOAT,         GETTER,       COMBOPOSR,                    1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Layer",                    ZTID_FLOAT,         GETTER,       COMBOLAYERR,                  1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLayer",                 ZTID_FLOAT,         GETTER,       COMBOLAYERR,                  1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getInitD[]",               ZTID_UNTYPED,       GETTER,       COMBODATAINITD,               2,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInitD[]",               ZTID_VOID,          SETTER,       COMBODATAINITD,               2,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getID",                    ZTID_FLOAT,         GETTER,       COMBODATAID,                  1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getOriginalTile",          ZTID_FLOAT,         GETTER,       COMBODOTILE,                  1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setOriginalTile",          ZTID_VOID,          SETTER,       COMBODOTILE,                  1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFrame",                 ZTID_FLOAT,         GETTER,       COMBODFRAME,                  1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFrame",                 ZTID_VOID,          SETTER,       COMBODFRAME,                  1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAClk",                  ZTID_FLOAT,         GETTER,       COMBODACLK,                   1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAClk",                  ZTID_VOID,          SETTER,       COMBODACLK,                   1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTile",                  ZTID_FLOAT,         GETTER,       COMBODTILE,                   1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTile",                  ZTID_VOID,          SETTER,       COMBODTILE,                   1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScript",                  ZTID_FLOAT,         GETTER,       COMBODATASCRIPT,                   1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScript",                  ZTID_VOID,          SETTER,       COMBODATASCRIPT,                   1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getASpeed",                ZTID_FLOAT,         GETTER,       COMBODASPEED,                 1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setASpeed",                ZTID_VOID,          SETTER,       COMBODASPEED,                 1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlip",                  ZTID_FLOAT,         GETTER,       COMBODFLIP,                   1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlip",                  ZTID_VOID,          SETTER,       COMBODFLIP,                   1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWalk",                  ZTID_FLOAT,         GETTER,       COMBODWALK,                   1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWalk",                  ZTID_VOID,          SETTER,       COMBODWALK,                   1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getEffect",                ZTID_FLOAT,         GETTER,       COMBODEFFECT,                 1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setEffect",                ZTID_VOID,          SETTER,       COMBODEFFECT,                 1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getType",                  ZTID_FLOAT,         GETTER,       COMBODTYPE,                   1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setType",                  ZTID_VOID,          SETTER,       COMBODTYPE,                   1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCSet",                  ZTID_FLOAT,         GETTER,       COMBODCSET,                   1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCSet",                  ZTID_VOID,          SETTER,       COMBODCSET,                   1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCSet2",                 ZTID_FLOAT,         GETTER,       COMBODCSET,                   1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCSet2",                 ZTID_VOID,          SETTER,       COMBODCSET,                   1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCSet2Flags",            ZTID_FLOAT,         GETTER,       COMBODCSET2FLAGS,             1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCSet2Flags",            ZTID_VOID,          SETTER,       COMBODCSET2FLAGS,             1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFoo",                   ZTID_FLOAT,         GETTER,       COMBODFOO,                    1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFoo",                   ZTID_VOID,          SETTER,       COMBODFOO,                    1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFrames",                ZTID_FLOAT,         GETTER,       COMBODFRAMES,                 1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFrames",                ZTID_VOID,          SETTER,       COMBODFRAMES,                 1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getNextData",              ZTID_FLOAT,         GETTER,       COMBODNEXTD,                  1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNextData",              ZTID_VOID,          SETTER,       COMBODNEXTD,                  1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getNextCSet",              ZTID_FLOAT,         GETTER,       COMBODNEXTC,                  1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNextCSet",              ZTID_VOID,          SETTER,       COMBODNEXTC,                  1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlag",                  ZTID_FLOAT,         GETTER,       COMBODFLAG,                   1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlag",                  ZTID_VOID,          SETTER,       COMBODFLAG,                   1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSkipAnim",              ZTID_FLOAT,         GETTER,       COMBODSKIPANIM,               1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSkipAnim",              ZTID_VOID,          SETTER,       COMBODSKIPANIM,               1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getNextTimer",             ZTID_FLOAT,         GETTER,       COMBODNEXTTIMER,              1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNextTimer",             ZTID_VOID,          SETTER,       COMBODNEXTTIMER,              1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSkipAnimY",             ZTID_FLOAT,         GETTER,       COMBODAKIMANIMY,              1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSkipAnimY",             ZTID_VOID,          SETTER,       COMBODAKIMANIMY,              1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAnimFlags",             ZTID_FLOAT,         GETTER,       COMBODANIMFLAGS,              1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAnimFlags",             ZTID_VOID,          SETTER,       COMBODANIMFLAGS,              1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getExpansion[]",           ZTID_FLOAT,         GETTER,       COMBODEXPANSION,              6,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setExpansion[]",           ZTID_VOID,          SETTER,       COMBODEXPANSION,              6,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAttributes[]",          ZTID_FLOAT,         GETTER,       COMBODATTRIBUTES,             4,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAttributes[]",          ZTID_VOID,          SETTER,       COMBODATTRIBUTES,             4,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAttribytes[]",          ZTID_FLOAT,         GETTER,       COMBODATTRIBYTES,             8,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAttribytes[]",          ZTID_VOID,          SETTER,       COMBODATTRIBYTES,             8,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAttrishorts[]",         ZTID_FLOAT,         GETTER,       COMBODATTRISHORTS,            8,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAttrishorts[]",         ZTID_VOID,          SETTER,       COMBODATTRISHORTS,            8,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTriggerFlags[]",        ZTID_FLOAT,         GETTER,       COMBODTRIGGERFLAGS,           3,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTriggerFlags[]",        ZTID_VOID,          SETTER,       COMBODTRIGGERFLAGS,           3,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTrigFlags[]",           ZTID_BOOL,          GETTER,       COMBODTRIGGERFLAGS2,         96,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTrigFlags[]",           ZTID_VOID,          SETTER,       COMBODTRIGGERFLAGS2,         96,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTriggerButton[]",       ZTID_BOOL,          GETTER,       COMBODTRIGGERBUTTON,          8,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTriggerButton[]",       ZTID_VOID,          SETTER,       COMBODTRIGGERBUTTON,          8,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getUserFlags",             ZTID_FLOAT,         GETTER,       COMBODUSRFLAGS,               1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setUserFlags",             ZTID_VOID,          SETTER,       COMBODUSRFLAGS,               1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlags[]",               ZTID_BOOL,          GETTER,       COMBODUSRFLAGARR,            16,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlags[]",               ZTID_VOID,          SETTER,       COMBODUSRFLAGARR,            16,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getGenFlags[]",            ZTID_BOOL,          GETTER,       COMBODGENFLAGARR,             2,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setGenFlags[]",            ZTID_VOID,          SETTER,       COMBODGENFLAGARR,             2,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTriggerLevel",          ZTID_FLOAT,         GETTER,       COMBODTRIGGERLEVEL,           1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTriggerLevel",          ZTID_VOID,          SETTER,       COMBODTRIGGERLEVEL,           1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTriggerItem",           ZTID_FLOAT,         GETTER,       COMBODTRIGGERITEM,            1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTriggerItem",           ZTID_VOID,          SETTER,       COMBODTRIGGERITEM,            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTriggerTimer",          ZTID_FLOAT,         GETTER,       COMBODTRIGGERTIMER,           1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTriggerTimer",          ZTID_VOID,          SETTER,       COMBODTRIGGERTIMER,           1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTriggerSFX",            ZTID_FLOAT,         GETTER,       COMBODTRIGGERSFX,             1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTriggerSFX",            ZTID_VOID,          SETTER,       COMBODTRIGGERSFX,             1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTriggerChange",         ZTID_FLOAT,         GETTER,       COMBODTRIGGERCHANGECMB,       1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTriggerChange",         ZTID_VOID,          SETTER,       COMBODTRIGGERCHANGECMB,       1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTriggerProximity",      ZTID_FLOAT,         GETTER,       COMBODTRIGGERPROX,            1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTriggerProximity",      ZTID_VOID,          SETTER,       COMBODTRIGGERPROX,            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTriggerLightBeam",      ZTID_FLOAT,         GETTER,       COMBODTRIGGERLIGHTBEAM,       1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTriggerLightBeam",      ZTID_VOID,          SETTER,       COMBODTRIGGERLIGHTBEAM,       1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTriggerCounter",        ZTID_FLOAT,         GETTER,       COMBODTRIGGERCTR,             1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTriggerCounter",        ZTID_VOID,          SETTER,       COMBODTRIGGERCTR,             1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTriggerCtrAmount",      ZTID_FLOAT,         GETTER,       COMBODTRIGGERCTRAMNT,         1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTriggerCtrAmount",      ZTID_VOID,          SETTER,       COMBODTRIGGERCTRAMNT,         1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTrigCooldown",          ZTID_FLOAT,         GETTER,       COMBODTRIGGERCOOLDOWN,        1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTrigCooldown",          ZTID_VOID,          SETTER,       COMBODTRIGGERCOOLDOWN,        1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTrigCopycat",           ZTID_FLOAT,         GETTER,       COMBODTRIGGERCOPYCAT,         1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTrigCopycat",           ZTID_VOID,          SETTER,       COMBODTRIGGERCOPYCAT,         1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTrigSpawnItemPickup",   ZTID_FLOAT,         GETTER,       COMBODTRIGITEMPICKUP,         1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTrigSpawnItemPickup",   ZTID_VOID,          SETTER,       COMBODTRIGITEMPICKUP,         1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTrigExState",           ZTID_FLOAT,         GETTER,       COMBODTRIGEXSTATE,            1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTrigExState",           ZTID_VOID,          SETTER,       COMBODTRIGEXSTATE,            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTrigSpawnEnemy",        ZTID_FLOAT,         GETTER,       COMBODTRIGSPAWNENEMY,         1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTrigSpawnEnemy",        ZTID_VOID,          SETTER,       COMBODTRIGSPAWNENEMY,         1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTrigSpawnItem",         ZTID_FLOAT,         GETTER,       COMBODTRIGSPAWNITEM,          1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTrigSpawnItem",         ZTID_VOID,          SETTER,       COMBODTRIGSPAWNITEM,          1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTrigCSetChange",        ZTID_FLOAT,         GETTER,       COMBODTRIGCSETCHANGE,         1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTrigCSetChange",        ZTID_VOID,          SETTER,       COMBODTRIGCSETCHANGE,         1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLiftGFXCombo",          ZTID_FLOAT,         GETTER,       COMBODLIFTGFXCOMBO,           1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLiftGFXCombo",          ZTID_VOID,          SETTER,       COMBODLIFTGFXCOMBO,           1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLiftGFXCSet",           ZTID_FLOAT,         GETTER,       COMBODLIFTGFXCCSET,           1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLiftGFXCSet",           ZTID_VOID,          SETTER,       COMBODLIFTGFXCCSET,           1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLiftUnderCombo",        ZTID_FLOAT,         GETTER,       COMBODLIFTUNDERCMB,           1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLiftUnderCombo",        ZTID_VOID,          SETTER,       COMBODLIFTUNDERCMB,           1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLiftUnderCSet",         ZTID_FLOAT,         GETTER,       COMBODLIFTUNDERCS,            1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLiftUnderCSet",         ZTID_VOID,          SETTER,       COMBODLIFTUNDERCS,            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLiftDamage",            ZTID_FLOAT,         GETTER,       COMBODLIFTDAMAGE,             1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLiftDamage",            ZTID_VOID,          SETTER,       COMBODLIFTDAMAGE,             1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLiftLevel",             ZTID_FLOAT,         GETTER,       COMBODLIFTLEVEL,              1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLiftLevel",             ZTID_VOID,          SETTER,       COMBODLIFTLEVEL,              1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLiftItem",              ZTID_FLOAT,         GETTER,       COMBODLIFTITEM,               1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLiftItem",              ZTID_VOID,          SETTER,       COMBODLIFTITEM,               1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLiftFlags[]",           ZTID_BOOL,          GETTER,       COMBODLIFTFLAGS,              1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLiftFlags[]",           ZTID_VOID,          SETTER,       COMBODLIFTFLAGS,              1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLiftGFXType",           ZTID_FLOAT,         GETTER,       COMBODLIFTGFXTYPE,            1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLiftGFXType",           ZTID_VOID,          SETTER,       COMBODLIFTGFXTYPE,            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLiftGFXSprite",         ZTID_FLOAT,         GETTER,       COMBODLIFTGFXSPRITE,          1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLiftGFXSprite",         ZTID_VOID,          SETTER,       COMBODLIFTGFXSPRITE,          1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLiftSFX",               ZTID_FLOAT,         GETTER,       COMBODLIFTSFX,                1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLiftSFX",               ZTID_VOID,          SETTER,       COMBODLIFTSFX,                1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLiftBreakSprite",       ZTID_FLOAT,         GETTER,       COMBODLIFTBREAKSPRITE,        1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLiftBreakSprite",       ZTID_VOID,          SETTER,       COMBODLIFTBREAKSPRITE,        1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLiftBreakSFX",          ZTID_FLOAT,         GETTER,       COMBODLIFTBREAKSFX,           1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLiftBreakSFX",          ZTID_VOID,          SETTER,       COMBODLIFTBREAKSFX,           1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLiftHeight",            ZTID_FLOAT,         GETTER,       COMBODLIFTHEIGHT,             1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLiftHeight",            ZTID_VOID,          SETTER,       COMBODLIFTHEIGHT,             1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLiftTime",              ZTID_FLOAT,         GETTER,       COMBODLIFTTIME,               1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLiftTime",              ZTID_VOID,          SETTER,       COMBODLIFTTIME,               1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	
	
//	comboclass struct
//	"Name" -> Needs to be a function, GetName(int32_t string[])
	{ "getBlockNPC",              ZTID_FLOAT,         GETTER,       COMBODBLOCKNPC,               1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setBlockNPC",              ZTID_VOID,          SETTER,       COMBODBLOCKNPC,               1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getBlockHole",             ZTID_FLOAT,         GETTER,       COMBODBLOCKHOLE,              1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setBlockHole",             ZTID_VOID,          SETTER,       COMBODBLOCKHOLE,              1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getBlockTrigger",          ZTID_FLOAT,         GETTER,       COMBODBLOCKTRIG,              1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setBlockTrigger",          ZTID_VOID,          SETTER,       COMBODBLOCKTRIG,              1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getBlockWeapon[]",         ZTID_FLOAT,         GETTER,       COMBODBLOCKWEAPON,            32,            0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setBlockWeapon[]",         ZTID_VOID,          SETTER,       COMBODBLOCKWEAPON,            32,            0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getConveyorSpeedX",        ZTID_FLOAT,         GETTER,       COMBODCONVXSPEED,             1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setConveyorSpeedX",        ZTID_VOID,          SETTER,       COMBODCONVXSPEED,             1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getConveyorSpeedY",        ZTID_FLOAT,         GETTER,       COMBODCONVYSPEED,             1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setConveyorSpeedY",        ZTID_VOID,          SETTER,       COMBODCONVYSPEED,             1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSpawnNPC",              ZTID_FLOAT,         GETTER,       COMBODSPAWNNPC,               1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSpawnNPC",              ZTID_VOID,          SETTER,       COMBODSPAWNNPC,               1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSpawnNPCWhen",          ZTID_FLOAT,         GETTER,       COMBODSPAWNNPCWHEN,           1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSpawnNPCWhen",          ZTID_VOID,          SETTER,       COMBODSPAWNNPCWHEN,           1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSpawnNPCCHange",        ZTID_FLOAT,         GETTER,       COMBODSPAWNNPCCHANGE,         1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSpawnNPCChange",        ZTID_VOID,          SETTER,       COMBODSPAWNNPCCHANGE,         1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDirChange",             ZTID_FLOAT,         GETTER,       COMBODDIRCHANGETYPE,          1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDirChange",             ZTID_VOID,          SETTER,       COMBODDIRCHANGETYPE,          1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDistanceChangeTiles",   ZTID_FLOAT,         GETTER,       COMBODDISTANCECHANGETILES,    1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDistanceChangeTiles",   ZTID_VOID,          SETTER,       COMBODDISTANCECHANGETILES,    1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDiveItem",              ZTID_FLOAT,         GETTER,       COMBODDIVEITEM,               1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDiveItem",              ZTID_VOID,          SETTER,       COMBODDIVEITEM,               1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDock",                  ZTID_FLOAT,         GETTER,       COMBODDOCK,                   1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDock",                  ZTID_VOID,          SETTER,       COMBODDOCK,                   1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFairy",                 ZTID_FLOAT,         GETTER,       COMBODFAIRY,                  1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFairy",                 ZTID_VOID,          SETTER,       COMBODFAIRY,                  1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFFCAttributeChange",    ZTID_FLOAT,         GETTER,       COMBODFFATTRCHANGE,           1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCAttributeChange",    ZTID_VOID,          SETTER,       COMBODFFATTRCHANGE,           1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDecorationTile",        ZTID_FLOAT,         GETTER,       COMBODFOORDECOTILE,           1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDecorationTile",        ZTID_VOID,          SETTER,       COMBODFOORDECOTILE,           1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDecorationType",        ZTID_FLOAT,         GETTER,       COMBODFOORDECOTYPE,           1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDecorationType",        ZTID_VOID,          SETTER,       COMBODFOORDECOTYPE,           1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHookshot",              ZTID_FLOAT,         GETTER,       COMBODHOOKSHOTPOINT,          1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHookshot",              ZTID_VOID,          SETTER,       COMBODHOOKSHOTPOINT,          1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLadder",                ZTID_FLOAT,         GETTER,       COMBODLADDERPASS,             1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLadder",                ZTID_VOID,          SETTER,       COMBODLADDERPASS,             1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLockBlock",             ZTID_FLOAT,         GETTER,       COMBODLOCKBLOCK,              1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLockBlock",             ZTID_VOID,          SETTER,       COMBODLOCKBLOCK,              1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLockBlockChange",       ZTID_FLOAT,         GETTER,       COMBODLOCKBLOCKCHANGE,        1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLockBlockChange",       ZTID_VOID,          SETTER,       COMBODLOCKBLOCKCHANGE,        1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMirror",                ZTID_FLOAT,         GETTER,       COMBODMAGICMIRROR,            1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMirror",                ZTID_VOID,          SETTER,       COMBODMAGICMIRROR,            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDamageAmount",          ZTID_FLOAT,         GETTER,       COMBODMODHPAMOUNT,            1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDamageAmount",          ZTID_VOID,          SETTER,       COMBODMODHPAMOUNT,            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDamageDelay",           ZTID_FLOAT,         GETTER,       COMBODMODHPDELAY,             1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDamageDelay",           ZTID_VOID,          SETTER,       COMBODMODHPDELAY,             1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDamageType",            ZTID_FLOAT,         GETTER,       COMBODMODHPTYPE,              1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDamageType",            ZTID_VOID,          SETTER,       COMBODMODHPTYPE,              1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMagicAmount",           ZTID_FLOAT,         GETTER,       COMBODNMODMPAMOUNT,           1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMagicAmount",           ZTID_VOID,          SETTER,       COMBODNMODMPAMOUNT,           1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMagicDelay",            ZTID_FLOAT,         GETTER,       COMBODMODMPDELAY,             1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMagicDelay",            ZTID_VOID,          SETTER,       COMBODMODMPDELAY,             1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMagicType",             ZTID_FLOAT,         GETTER,       COMBODMODMPTYPE,              1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMagicType",             ZTID_VOID,          SETTER,       COMBODMODMPTYPE,              1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getNoPushBlocks",          ZTID_FLOAT,         GETTER,       COMBODNOPUSHBLOCK,            1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNoPushBlocks",          ZTID_VOID,          SETTER,       COMBODNOPUSHBLOCK,            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getOverhead",              ZTID_FLOAT,         GETTER,       COMBODOVERHEAD,               1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setOverhead",              ZTID_VOID,          SETTER,       COMBODOVERHEAD,               1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPlaceNPC",              ZTID_FLOAT,         GETTER,       COMBODPLACENPC,               1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPlaceNPC",              ZTID_VOID,          SETTER,       COMBODPLACENPC,               1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPushDir",               ZTID_FLOAT,         GETTER,       COMBODPUSHDIR,                1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPushDir",               ZTID_VOID,          SETTER,       COMBODPUSHDIR,                1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPushDelay",             ZTID_FLOAT,         GETTER,       COMBODPUSHWAIT,               42,            0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPushDelay",             ZTID_VOID,          SETTER,       COMBODPUSHWAIT,               42,            0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPushHeavy",             ZTID_FLOAT,         GETTER,       COMBODPUSHHEAVY,              1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPushHeavy",             ZTID_VOID,          SETTER,       COMBODPUSHHEAVY,              1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPushed",                ZTID_FLOAT,         GETTER,       COMBODPUSHED,                 1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPushed",                ZTID_VOID,          SETTER,       COMBODPUSHED,                 1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRaft",                  ZTID_FLOAT,         GETTER,       COMBODRAFT,                   1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRaft",                  ZTID_VOID,          SETTER,       COMBODRAFT,                   1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getResetRoom",             ZTID_FLOAT,         GETTER,       COMBODRESETROOM,              1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setResetRoom",             ZTID_VOID,          SETTER,       COMBODRESETROOM,              1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSavePoint",             ZTID_FLOAT,         GETTER,       COMBODSAVEPOINTTYPE,          1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSavePoint",             ZTID_VOID,          SETTER,       COMBODSAVEPOINTTYPE,          1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFreezeScreen",          ZTID_FLOAT,         GETTER,       COMBODSCREENFREEZETYPE,       1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFreezeScreen",          ZTID_VOID,          SETTER,       COMBODSCREENFREEZETYPE,       1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSecretCombo",           ZTID_FLOAT,         GETTER,       COMBODSECRETCOMBO,            1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSecretCombo",           ZTID_VOID,          SETTER,       COMBODSECRETCOMBO,            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSingular",              ZTID_FLOAT,         GETTER,       COMBODSINGULAR,               1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSingular",              ZTID_VOID,          SETTER,       COMBODSINGULAR,               1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSlowWalk",              ZTID_FLOAT,         GETTER,       COMBODSLOWWALK,               1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSlowWalk",              ZTID_VOID,          SETTER,       COMBODSLOWWALK,               1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStatue",                ZTID_FLOAT,         GETTER,       COMBODSTATUETYPE,             1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStatue",                ZTID_VOID,          SETTER,       COMBODSTATUETYPE,             1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStep",                  ZTID_FLOAT,         GETTER,       COMBODSTEPTYPE,               1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStep",                  ZTID_VOID,          SETTER,       COMBODSTEPTYPE,               1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStepChange",            ZTID_FLOAT,         GETTER,       COMBODSTEPCHANGEINTO,         1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStepChange",            ZTID_VOID,          SETTER,       COMBODSTEPCHANGEINTO,         1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStrike[]",              ZTID_FLOAT,         GETTER,       COMBODSTRIKEWEAPONS,          32,            0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStrike[]",              ZTID_VOID,          SETTER,       COMBODSTRIKEWEAPONS,          32,            0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStrikeRemnants",        ZTID_FLOAT,         GETTER,       COMBODSTRIKEREMNANTS,         1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStrikeRemnants",        ZTID_VOID,          SETTER,       COMBODSTRIKEREMNANTS,         1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStrikeRemnantsType",    ZTID_FLOAT,         GETTER,       COMBODSTRIKEREMNANTSTYPE,     1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStrikeRemnantsType",    ZTID_VOID,          SETTER,       COMBODSTRIKEREMNANTSTYPE,     1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStrikeChange",          ZTID_FLOAT,         GETTER,       COMBODSTRIKECHANGE,           1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStrikeChange",          ZTID_VOID,          SETTER,       COMBODSTRIKECHANGE,           1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStrikeItem",            ZTID_FLOAT,         GETTER,       COMBODTOUCHITEM,              1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStrikeItem",            ZTID_VOID,          SETTER,       COMBODTOUCHITEM,              1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTouchItem",             ZTID_FLOAT,         GETTER,       COMBODTOUCHITEM,              1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTouchItem",             ZTID_VOID,          SETTER,       COMBODTOUCHITEM,              1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTouchStairs",           ZTID_FLOAT,         GETTER,       COMBODTOUCHSTAIRS,            1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTouchStairs",           ZTID_VOID,          SETTER,       COMBODTOUCHSTAIRS,            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTriggerType",           ZTID_FLOAT,         GETTER,       COMBODTRIGGERTYPE,            1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTriggerType",           ZTID_VOID,          SETTER,       COMBODTRIGGERTYPE,            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTriggerSensitivity",    ZTID_FLOAT,         GETTER,       COMBODTRIGGERSENS,            1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTriggerSensitivity",    ZTID_VOID,          SETTER,       COMBODTRIGGERSENS,            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWarp",                  ZTID_FLOAT,         GETTER,       COMBODWARPTYPE,               1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWarp",                  ZTID_VOID,          SETTER,       COMBODWARPTYPE,               1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWarpSensitivity",       ZTID_FLOAT,         GETTER,       COMBODWARPSENS,               1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWarpSensitivity",       ZTID_VOID,          SETTER,       COMBODWARPSENS,               1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWarpDirect",            ZTID_FLOAT,         GETTER,       COMBODWARPDIRECT,             1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWarpDirect",            ZTID_VOID,          SETTER,       COMBODWARPDIRECT,             1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWarpLocation",          ZTID_FLOAT,         GETTER,       COMBODWARPLOCATION,           1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWarpLocation",          ZTID_VOID,          SETTER,       COMBODWARPLOCATION,           1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWater",                 ZTID_FLOAT,         GETTER,       COMBODWATER,                  1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWater",                 ZTID_VOID,          SETTER,       COMBODWATER,                  1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWhistle",               ZTID_FLOAT,         GETTER,       COMBODWHISTLE,                1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWhistle",               ZTID_VOID,          SETTER,       COMBODWHISTLE,                1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWinGame",               ZTID_FLOAT,         GETTER,       COMBODWINGAME,                1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWinGame",               ZTID_VOID,          SETTER,       COMBODWINGAME,                1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getBlockWeaponLevel",      ZTID_FLOAT,         GETTER,       COMBODBLOCKWPNLEVEL,          1,             0,                                    1,           { ZTID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setBlockWeaponLevel",      ZTID_VOID,          SETTER,       COMBODBLOCKWPNLEVEL,          1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
//	Functions
//	{ "GetName",                  ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "SetName",                  ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	
//	one input, one return
	{ "GetBlockEnemies",          ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetBlockHole",             ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetBlockTrigger",          ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetConveyorSpeedX",        ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetConveyorSpeedY",        ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetCreateEnemy",           ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetCreateEnemyWhen",       ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetCreateEnemyChnge",      ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetDirChangeType",         ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetDistanceChangeTiles",   ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetDiveItem",              ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetDock",                  ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetFairy",                 ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetFFComboChangeAttrib",   ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetFootDecorationsTile",   ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetFootDecorationsType",   ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetHookshotGrab",          ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetLadderPass",            ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetLockBlockType",         ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetLockBlockChange",       ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetMagicMirror",           ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetModifyHPAmount",        ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetModifyHPDelay",         ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetModifyHPType",          ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetModifyMPAmount",        ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetModifyMPDelay",         ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetModifyMPType",          ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetNoPushBlocks",          ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetOverhead",              ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetPlaceEnemy",            ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetPushDirection",         ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetPushWeight",            ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetPushWait",              ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetPushed",                ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetRaft",                  ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetResetRoom",             ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetSavePoint",             ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetScreenFreeze",          ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetSecretCombo",           ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetSingular",              ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetSlowMove",              ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetStatue",                ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetStepType",              ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetStepChangeTo",          ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetStrikeRemnants",        ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetStrikeRemnantsType",    ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetStrikeChange",          ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetStrikeItem",            ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetTouchItem",             ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetTouchStairs",           ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetTriggerType",           ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetTriggerSens",           ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetWarpType",              ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetWarpSens",              ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetWarpDirect",            ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetWarpLocation",          ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetWater",                 ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetWhistle",               ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetWinGame",               ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetBlockWeaponLevel",      ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetTile",                  ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetFlip",                  ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetWalkability",           ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetType",                  ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetCSets",                 ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetFoo",                   ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetFrames",                ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetSpeed",                 ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetNextCombo",             ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetNextCSet",              ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetFlag",                  ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetSkipAnim",              ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetNextTimer",             ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetSkipAnimY",             ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetAnimFlags",             ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZTID_COMBOS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
//	two inputs, one return
	{ "GetBlockWeapon",           ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetExpansion",             ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetStrikeWeapons",         ZTID_FLOAT,         FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
//	two inputs, no return
	{ "SetBlockEnemies",          ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetBlockHole",             ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetBlockTrigger",          ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetConveyorSpeedX",        ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetConveyorSpeedY",        ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetCreateEnemy",           ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetCreateEnemyWhen",       ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetCreateEnemyChnge",      ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetDirChangeType",         ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetDistanceChangeTiles",   ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetDiveItem",              ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetDock",                  ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetFairy",                 ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetFFComboChangeAttrib",   ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetFootDecorationsTile",   ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetFootDecorationsType",   ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetHookshotGrab",          ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetLadderPass",            ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetLockBlockType",         ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetLockBlockChange",       ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetMagicMirror",           ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetModifyHPAmount",        ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetModifyHPDelay",         ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetModifyHPType",          ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetModifyMPAmount",        ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetModifyMPDelay",         ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetModifyMPType",          ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetNoPushBlocks",          ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetOverhead",              ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetPlaceEnemy",            ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetPushDirection",         ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetPushWeight",            ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetPushWait",              ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetPushed",                ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetRaft",                  ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetResetRoom",             ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetSavePoint",             ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetScreenFreeze",          ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetSecretCombo",           ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetSingular",              ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetSlowMove",              ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetStatue",                ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetStepType",              ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetStepChangeTo",          ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetStrikeRemnants",        ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetStrikeRemnantsType",    ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetStrikeChange",          ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetStrikeItem",            ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetTouchItem",             ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetTouchStairs",           ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetTriggerType",           ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetTriggerSens",           ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetWarpType",              ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetWarpSens",              ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetWarpDirect",            ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetWarpLocation",          ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetWater",                 ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetWhistle",               ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetWinGame",               ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetBlockWeaponLevel",      ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetTile",                  ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetFlip",                  ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetWalkability",           ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetType",                  ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetCSets",                 ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetFoo",                   ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetFrames",                ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetSpeed",                 ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetNextCombo",             ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetNextCSet",              ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetFlag",                  ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetSkipAnim",              ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetNextTimer",             ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetSkipAnimY",             ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetAnimFlags",             ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
//	three inputs, no return
	{ "SetBlockWeapon",           ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    4,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetExpansion",             ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    4,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetStrikeWeapons",         ZTID_VOID,          FUNCTION,     0,                            1,             0,                                    4,           { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	
	{ "",                         -1,                       -1,           -1,                           -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

CombosPtrSymbols::CombosPtrSymbols()
{
	table = CombosTable;
	refVar = REFCOMBODATA; //NUL;
}

void CombosPtrSymbols::generateCode()
{
	{
		Function* function = getFunction("GetBlockEnemies", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataBlockEnemy(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetBlockHole", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataBlockHole(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetBlockTrigger", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataBlockTrig(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetConveyorSpeedX", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataConveyX(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetConveyorSpeedY", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataConveyY(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetCreateEnemy", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataCreateNPC(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetCreateEnemyWhen", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataCreateEnemW(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetCreateEnemyChnge", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataCreateEnemC(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetDirChangeType", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataDirch(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetDistanceChangeTiles", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataDistTiles(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetDiveItem", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataDiveItem(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetDock", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataDock(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetFairy", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataFairy(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetFFComboChangeAttrib", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataAttrib(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetFootDecorationsTile", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataDecoTile(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetFootDecorationsType", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataDecoType(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetHookshotGrab", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataHookshotGrab(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetLadderPass", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataLadderPass(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetLockBlockType", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataLockBlock(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetLockBlockChange", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataLockBlockChange(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetMagicMirror", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataMagicMirror(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetModifyHPAmount", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataModHP(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetModifyHPDelay", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataModHPDelay(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetModifyHPType", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataModHpType(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetModifyMPAmount", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataModMP(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetModifyMPDelay", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataMpdMPDelay(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetModifyMPType", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataModMPType(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetNoPushBlocks", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataNoPush(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetOverhead", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataOverhead(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetPlaceEnemy", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataEnemyLoc(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetPushDirection", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataPushDir(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetPushWeight", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataPushWeight(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetPushWait", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataPushWait(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetPushed", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataPushed(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetRaft", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataRaft(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetResetRoom", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataResetRoom(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetSavePoint", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataSavePoint(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetScreenFreeze", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataFreeezeScreen(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetSecretCombo", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataSecretCombo(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetSingular", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataSingular(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetSlowMove", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataSlowMove(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetStatue", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataStatue(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetStepType", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataStepType(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetStepChangeTo", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataSteoChange(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetStrikeRemnants", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataStrikeRem(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetStrikeRemnantsType", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataStrikeRemType(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetStrikeChange", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataStrikeChange(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetStrikeItem", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataStrikeChangeItem(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetTouchItem", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataTouchItem(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetTouchStairs", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataTouchStairs(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetTriggerType", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataTriggerType(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetTriggerSens", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataTriggerSens(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetWarpType", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataWarpType(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetWarpSens", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataWarpSens(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetWarpDirect", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataWarpDirect(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetWarpLocation", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataWarpLoc(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetWater", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataWater(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetWhistle", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataWhistle(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetWinGame", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataWinGame(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetBlockWeaponLevel", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataWeapBlockLevel(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetTile", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataTile(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetFlip", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataFlip(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetWalkability", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataWalkability(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetType", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataType(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetCSets", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataCSets(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetFoo", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataFoo(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetFrames", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataFrames(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetSpeed", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataSpeed(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetNextCombo", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataNext(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetNextCSet", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataNextCSet(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetFlag", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataFlag(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetSkipAnim", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCSetDataSkipAnim(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetNextTimer", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataTimer(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetSkipAnimY", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataAnimY(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetAnimFlags", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataAnimFlags(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("SetBlockEnemies", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataBlockEnemy(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetBlockHole", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataBlockHole(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetBlockTrigger", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataBlockTrig(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetConveyorSpeedX", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataConveyX(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetConveyorSpeedY", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataConveyY(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetCreateEnemy", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataCreateNPC(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetCreateEnemyWhen", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataCreateEnemW(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetCreateEnemyChnge", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataCreateEnemC(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetDirChangeType", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataDirch(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetDistanceChangeTiles", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataDistTiles(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetDiveItem", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataDiveItem(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetDock", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataDock(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetFairy", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataFairy(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetFFComboChangeAttrib", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataAttrib(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetFootDecorationsTile", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataDecoTile(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetFootDecorationsType", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataDecoType(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetHookshotGrab", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataHookshotGrab(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetLadderPass", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataLadderPass(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetLockBlockType", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataLockBlock(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetLockBlockChange", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataLockBlockChange(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetMagicMirror", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataMagicMirror(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetModifyHPAmount", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataModHP(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetModifyHPDelay", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataModHPDelay(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetModifyHPType", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataModHpType(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetModifyMPAmount", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataModMP(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetModifyMPDelay", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataMpdMPDelay(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetModifyMPType", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataModMPType(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetNoPushBlocks", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataNoPush(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetOverhead", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataOverhead(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetPlaceEnemy", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataEnemyLoc(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetPushDirection", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataPushDir(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetPushWeight", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataPushWeight(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetPushWait", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataPushWait(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetPushed", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataPushed(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetRaft", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataRaft(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetResetRoom", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataResetRoom(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetSavePoint", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataSavePoint(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetScreenFreeze", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataFreeezeScreen(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetSecretCombo", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataSecretCombo(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetSingular", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataSingular(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetSlowMove", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataSlowMove(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetStatue", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataStatue(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetStepType", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataStepType(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetStepChangeTo", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataSteoChange(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetStrikeRemnants", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataStrikeRem(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetStrikeRemnantsType", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataStrikeRemType(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetStrikeChange", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataStrikeChange(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetStrikeItem", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataStrikeChangeItem(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetTouchItem", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataTouchItem(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetTouchStairs", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataTouchStairs(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetTriggerType", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataTriggerType(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetTriggerSens", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataTriggerSens(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetWarpType", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataWarpType(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetWarpSens", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataWarpSens(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetWarpDirect", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataWarpDirect(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetWarpLocation", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataWarpLoc(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetWater", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataWater(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetWhistle", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataWhistle(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetWinGame", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataWinGame(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetBlockWeaponLevel", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataWeapBlockLevel(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetTile", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataTile(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetFlip", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataFlip(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetWalkability", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataWalkability(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetType", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataType(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetCSets", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataCSets(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetFoo", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataFoo(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetFrames", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataFrames(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetSpeed", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataSpeed(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetNextCombo", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataNext(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetNextCSet", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataNextCSet(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetFlag", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataFlag(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetSkipAnim", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataSkipAnim(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetNextTimer", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataTimer(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetSkipAnimY", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataAnimY(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetAnimFlags", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataAnimFlags(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("GetBlockWeapon", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		POPREF();
		addOpcode2 (code, new OCDataBlockWeapon(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("GetExpansion", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		POPREF();
		addOpcode2 (code, new OCDataExpansion(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("GetStrikeWeapons", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		POPREF();
		addOpcode2 (code, new OCDataStrikeWeapon(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetBlockWeapon", 4);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		POPREF();
		addOpcode2 (code, new OSetRegister(new VarArgument(SCDBLOCKWEAPON), new VarArgument(SFTEMP)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetStrikeWeapons", 4);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		POPREF();
		addOpcode2 (code, new OSetRegister(new VarArgument(SCDSTRIKEWEAPONS), new VarArgument(SFTEMP)));
		RETURN();
		function->giveCode(code);
	}
}

