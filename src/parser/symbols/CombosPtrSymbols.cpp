#include "SymbolDefs.h"

CombosPtrSymbols CombosPtrSymbols::singleton = CombosPtrSymbols();

static AccessorTable CombosTable[] =
{
//	  name,                       rettype,                  setorget,     var,                          numindex,      funcFlags,                            numParams,   params
	
//	newcombo struct
	{ "PosX",                     ZVARTYPEID_FLOAT,         GETTER,       COMBOXR,                      1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getX",                     ZVARTYPEID_FLOAT,         GETTER,       COMBOXR,                      1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getY",                     ZVARTYPEID_FLOAT,         GETTER,       COMBOYR,                      1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "PosY",                     ZVARTYPEID_FLOAT,         GETTER,       COMBOYR,                      1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Pos",                      ZVARTYPEID_FLOAT,         GETTER,       COMBOPOSR,                    1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPos",                   ZVARTYPEID_FLOAT,         GETTER,       COMBOPOSR,                    1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Layer",                    ZVARTYPEID_FLOAT,         GETTER,       COMBOLAYERR,                  1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLayer",                 ZVARTYPEID_FLOAT,         GETTER,       COMBOLAYERR,                  1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getInitD[]",               ZVARTYPEID_UNTYPED,       GETTER,       COMBODATAINITD,               2,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInitD[]",               ZVARTYPEID_VOID,          SETTER,       COMBODATAINITD,               2,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getID",                    ZVARTYPEID_FLOAT,         GETTER,       COMBODATAID,                  1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getOriginalTile",          ZVARTYPEID_FLOAT,         GETTER,       COMBODOTILE,                  1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setOriginalTile",          ZVARTYPEID_VOID,          SETTER,       COMBODOTILE,                  1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFrame",                 ZVARTYPEID_FLOAT,         GETTER,       COMBODFRAME,                  1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFrame",                 ZVARTYPEID_VOID,          SETTER,       COMBODFRAME,                  1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAClk",                  ZVARTYPEID_FLOAT,         GETTER,       COMBODACLK,                   1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAClk",                  ZVARTYPEID_VOID,          SETTER,       COMBODACLK,                   1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTile",                  ZVARTYPEID_FLOAT,         GETTER,       COMBODTILE,                   1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTile",                  ZVARTYPEID_VOID,          SETTER,       COMBODTILE,                   1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScript",                  ZVARTYPEID_FLOAT,         GETTER,       COMBODATASCRIPT,                   1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScript",                  ZVARTYPEID_VOID,          SETTER,       COMBODATASCRIPT,                   1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getASpeed",                ZVARTYPEID_FLOAT,         GETTER,       COMBODASPEED,                 1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setASpeed",                ZVARTYPEID_VOID,          SETTER,       COMBODASPEED,                 1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlip",                  ZVARTYPEID_FLOAT,         GETTER,       COMBODFLIP,                   1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlip",                  ZVARTYPEID_VOID,          SETTER,       COMBODFLIP,                   1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWalk",                  ZVARTYPEID_FLOAT,         GETTER,       COMBODWALK,                   1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWalk",                  ZVARTYPEID_VOID,          SETTER,       COMBODWALK,                   1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getEffect",                ZVARTYPEID_FLOAT,         GETTER,       COMBODEFFECT,                 1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setEffect",                ZVARTYPEID_VOID,          SETTER,       COMBODEFFECT,                 1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getType",                  ZVARTYPEID_FLOAT,         GETTER,       COMBODTYPE,                   1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setType",                  ZVARTYPEID_VOID,          SETTER,       COMBODTYPE,                   1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCSet",                  ZVARTYPEID_FLOAT,         GETTER,       COMBODCSET,                   1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCSet",                  ZVARTYPEID_VOID,          SETTER,       COMBODCSET,                   1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCSet2",                 ZVARTYPEID_FLOAT,         GETTER,       COMBODCSET,                   1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCSet2",                 ZVARTYPEID_VOID,          SETTER,       COMBODCSET,                   1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCSet2Flags",            ZVARTYPEID_FLOAT,         GETTER,       COMBODCSET2FLAGS,             1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCSet2Flags",            ZVARTYPEID_VOID,          SETTER,       COMBODCSET2FLAGS,             1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFoo",                   ZVARTYPEID_FLOAT,         GETTER,       COMBODFOO,                    1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFoo",                   ZVARTYPEID_VOID,          SETTER,       COMBODFOO,                    1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFrames",                ZVARTYPEID_FLOAT,         GETTER,       COMBODFRAMES,                 1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFrames",                ZVARTYPEID_VOID,          SETTER,       COMBODFRAMES,                 1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getNextData",              ZVARTYPEID_FLOAT,         GETTER,       COMBODNEXTD,                  1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNextData",              ZVARTYPEID_VOID,          SETTER,       COMBODNEXTD,                  1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getNextCSet",              ZVARTYPEID_FLOAT,         GETTER,       COMBODNEXTC,                  1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNextCSet",              ZVARTYPEID_VOID,          SETTER,       COMBODNEXTC,                  1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlag",                  ZVARTYPEID_FLOAT,         GETTER,       COMBODFLAG,                   1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlag",                  ZVARTYPEID_VOID,          SETTER,       COMBODFLAG,                   1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSkipAnim",              ZVARTYPEID_FLOAT,         GETTER,       COMBODSKIPANIM,               1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSkipAnim",              ZVARTYPEID_VOID,          SETTER,       COMBODSKIPANIM,               1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getNextTimer",             ZVARTYPEID_FLOAT,         GETTER,       COMBODNEXTTIMER,              1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNextTimer",             ZVARTYPEID_VOID,          SETTER,       COMBODNEXTTIMER,              1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSkipAnimY",             ZVARTYPEID_FLOAT,         GETTER,       COMBODAKIMANIMY,              1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSkipAnimY",             ZVARTYPEID_VOID,          SETTER,       COMBODAKIMANIMY,              1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAnimFlags",             ZVARTYPEID_FLOAT,         GETTER,       COMBODANIMFLAGS,              1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAnimFlags",             ZVARTYPEID_VOID,          SETTER,       COMBODANIMFLAGS,              1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getExpansion[]",           ZVARTYPEID_FLOAT,         GETTER,       COMBODEXPANSION,              6,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setExpansion[]",           ZVARTYPEID_VOID,          SETTER,       COMBODEXPANSION,              6,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAttributes[]",          ZVARTYPEID_FLOAT,         GETTER,       COMBODATTRIBUTES,             4,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAttributes[]",          ZVARTYPEID_VOID,          SETTER,       COMBODATTRIBUTES,             4,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAttribytes[]",          ZVARTYPEID_FLOAT,         GETTER,       COMBODATTRIBYTES,             8,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAttribytes[]",          ZVARTYPEID_VOID,          SETTER,       COMBODATTRIBYTES,             8,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAttrishorts[]",         ZVARTYPEID_FLOAT,         GETTER,       COMBODATTRISHORTS,            8,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAttrishorts[]",         ZVARTYPEID_VOID,          SETTER,       COMBODATTRISHORTS,            8,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTriggerFlags[]",        ZVARTYPEID_FLOAT,         GETTER,       COMBODTRIGGERFLAGS,           3,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTriggerFlags[]",        ZVARTYPEID_VOID,          SETTER,       COMBODTRIGGERFLAGS,           3,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTrigFlags[]",           ZVARTYPEID_BOOL,          GETTER,       COMBODTRIGGERFLAGS2,         96,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTrigFlags[]",           ZVARTYPEID_VOID,          SETTER,       COMBODTRIGGERFLAGS2,         96,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTriggerButton[]",       ZVARTYPEID_BOOL,          GETTER,       COMBODTRIGGERBUTTON,          8,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTriggerButton[]",       ZVARTYPEID_VOID,          SETTER,       COMBODTRIGGERBUTTON,          8,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getUserFlags",             ZVARTYPEID_FLOAT,         GETTER,       COMBODUSRFLAGS,               1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setUserFlags",             ZVARTYPEID_VOID,          SETTER,       COMBODUSRFLAGS,               1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlags[]",               ZVARTYPEID_BOOL,          GETTER,       COMBODUSRFLAGARR,            16,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlags[]",               ZVARTYPEID_VOID,          SETTER,       COMBODUSRFLAGARR,            16,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getGenFlags[]",            ZVARTYPEID_BOOL,          GETTER,       COMBODGENFLAGARR,             2,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setGenFlags[]",            ZVARTYPEID_VOID,          SETTER,       COMBODGENFLAGARR,             2,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTriggerLevel",          ZVARTYPEID_FLOAT,         GETTER,       COMBODTRIGGERLEVEL,           1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTriggerLevel",          ZVARTYPEID_VOID,          SETTER,       COMBODTRIGGERLEVEL,           1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTriggerItem",           ZVARTYPEID_FLOAT,         GETTER,       COMBODTRIGGERITEM,            1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTriggerItem",           ZVARTYPEID_VOID,          SETTER,       COMBODTRIGGERITEM,            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTriggerTimer",          ZVARTYPEID_FLOAT,         GETTER,       COMBODTRIGGERTIMER,           1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTriggerTimer",          ZVARTYPEID_VOID,          SETTER,       COMBODTRIGGERTIMER,           1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTriggerSFX",            ZVARTYPEID_FLOAT,         GETTER,       COMBODTRIGGERSFX,             1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTriggerSFX",            ZVARTYPEID_VOID,          SETTER,       COMBODTRIGGERSFX,             1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTriggerChange",         ZVARTYPEID_FLOAT,         GETTER,       COMBODTRIGGERCHANGECMB,       1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTriggerChange",         ZVARTYPEID_VOID,          SETTER,       COMBODTRIGGERCHANGECMB,       1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTriggerProximity",      ZVARTYPEID_FLOAT,         GETTER,       COMBODTRIGGERPROX,            1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTriggerProximity",      ZVARTYPEID_VOID,          SETTER,       COMBODTRIGGERPROX,            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTriggerLightBeam",      ZVARTYPEID_FLOAT,         GETTER,       COMBODTRIGGERLIGHTBEAM,       1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTriggerLightBeam",      ZVARTYPEID_VOID,          SETTER,       COMBODTRIGGERLIGHTBEAM,       1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTriggerCounter",        ZVARTYPEID_FLOAT,         GETTER,       COMBODTRIGGERCTR,             1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTriggerCounter",        ZVARTYPEID_VOID,          SETTER,       COMBODTRIGGERCTR,             1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTriggerCtrAmount",      ZVARTYPEID_FLOAT,         GETTER,       COMBODTRIGGERCTRAMNT,         1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTriggerCtrAmount",      ZVARTYPEID_VOID,          SETTER,       COMBODTRIGGERCTRAMNT,         1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTrigCooldown",          ZVARTYPEID_FLOAT,         GETTER,       COMBODTRIGGERCOOLDOWN,        1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTrigCooldown",          ZVARTYPEID_VOID,          SETTER,       COMBODTRIGGERCOOLDOWN,        1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTrigCopycat",           ZVARTYPEID_FLOAT,         GETTER,       COMBODTRIGGERCOPYCAT,         1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTrigCopycat",           ZVARTYPEID_VOID,          SETTER,       COMBODTRIGGERCOPYCAT,         1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTrigSpawnItemPickup",   ZVARTYPEID_FLOAT,         GETTER,       COMBODTRIGITEMPICKUP,         1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTrigSpawnItemPickup",   ZVARTYPEID_VOID,          SETTER,       COMBODTRIGITEMPICKUP,         1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTrigExState",           ZVARTYPEID_FLOAT,         GETTER,       COMBODTRIGEXSTATE,            1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTrigExState",           ZVARTYPEID_VOID,          SETTER,       COMBODTRIGEXSTATE,            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTrigSpawnEnemy",        ZVARTYPEID_FLOAT,         GETTER,       COMBODTRIGSPAWNENEMY,         1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTrigSpawnEnemy",        ZVARTYPEID_VOID,          SETTER,       COMBODTRIGSPAWNENEMY,         1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTrigSpawnItem",         ZVARTYPEID_FLOAT,         GETTER,       COMBODTRIGSPAWNITEM,          1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTrigSpawnItem",         ZVARTYPEID_VOID,          SETTER,       COMBODTRIGSPAWNITEM,          1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTrigCSetChange",        ZVARTYPEID_FLOAT,         GETTER,       COMBODTRIGCSETCHANGE,         1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTrigCSetChange",        ZVARTYPEID_VOID,          SETTER,       COMBODTRIGCSETCHANGE,         1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLiftGFXCombo",          ZVARTYPEID_FLOAT,         GETTER,       COMBODLIFTGFXCOMBO,           1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLiftGFXCombo",          ZVARTYPEID_VOID,          SETTER,       COMBODLIFTGFXCOMBO,           1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLiftGFXCSet",           ZVARTYPEID_FLOAT,         GETTER,       COMBODLIFTGFXCCSET,           1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLiftGFXCSet",           ZVARTYPEID_VOID,          SETTER,       COMBODLIFTGFXCCSET,           1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLiftUnderCombo",        ZVARTYPEID_FLOAT,         GETTER,       COMBODLIFTUNDERCMB,           1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLiftUnderCombo",        ZVARTYPEID_VOID,          SETTER,       COMBODLIFTUNDERCMB,           1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLiftUnderCSet",         ZVARTYPEID_FLOAT,         GETTER,       COMBODLIFTUNDERCS,            1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLiftUnderCSet",         ZVARTYPEID_VOID,          SETTER,       COMBODLIFTUNDERCS,            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLiftDamage",            ZVARTYPEID_FLOAT,         GETTER,       COMBODLIFTDAMAGE,             1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLiftDamage",            ZVARTYPEID_VOID,          SETTER,       COMBODLIFTDAMAGE,             1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLiftLevel",             ZVARTYPEID_FLOAT,         GETTER,       COMBODLIFTLEVEL,              1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLiftLevel",             ZVARTYPEID_VOID,          SETTER,       COMBODLIFTLEVEL,              1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLiftItem",              ZVARTYPEID_FLOAT,         GETTER,       COMBODLIFTITEM,               1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLiftItem",              ZVARTYPEID_VOID,          SETTER,       COMBODLIFTITEM,               1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLiftFlags[]",           ZVARTYPEID_BOOL,          GETTER,       COMBODLIFTFLAGS,              1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLiftFlags[]",           ZVARTYPEID_VOID,          SETTER,       COMBODLIFTFLAGS,              1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLiftGFXType",           ZVARTYPEID_FLOAT,         GETTER,       COMBODLIFTGFXTYPE,            1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLiftGFXType",           ZVARTYPEID_VOID,          SETTER,       COMBODLIFTGFXTYPE,            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLiftGFXSprite",         ZVARTYPEID_FLOAT,         GETTER,       COMBODLIFTGFXSPRITE,          1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLiftGFXSprite",         ZVARTYPEID_VOID,          SETTER,       COMBODLIFTGFXSPRITE,          1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLiftSFX",               ZVARTYPEID_FLOAT,         GETTER,       COMBODLIFTSFX,                1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLiftSFX",               ZVARTYPEID_VOID,          SETTER,       COMBODLIFTSFX,                1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLiftBreakSprite",       ZVARTYPEID_FLOAT,         GETTER,       COMBODLIFTBREAKSPRITE,        1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLiftBreakSprite",       ZVARTYPEID_VOID,          SETTER,       COMBODLIFTBREAKSPRITE,        1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLiftBreakSFX",          ZVARTYPEID_FLOAT,         GETTER,       COMBODLIFTBREAKSFX,           1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLiftBreakSFX",          ZVARTYPEID_VOID,          SETTER,       COMBODLIFTBREAKSFX,           1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLiftHeight",            ZVARTYPEID_FLOAT,         GETTER,       COMBODLIFTHEIGHT,             1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLiftHeight",            ZVARTYPEID_VOID,          SETTER,       COMBODLIFTHEIGHT,             1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLiftTime",              ZVARTYPEID_FLOAT,         GETTER,       COMBODLIFTTIME,               1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLiftTime",              ZVARTYPEID_VOID,          SETTER,       COMBODLIFTTIME,               1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	
	
//	comboclass struct
//	"Name" -> Needs to be a function, GetName(int32_t string[])
	{ "getBlockNPC",              ZVARTYPEID_FLOAT,         GETTER,       COMBODBLOCKNPC,               1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setBlockNPC",              ZVARTYPEID_VOID,          SETTER,       COMBODBLOCKNPC,               1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getBlockHole",             ZVARTYPEID_FLOAT,         GETTER,       COMBODBLOCKHOLE,              1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setBlockHole",             ZVARTYPEID_VOID,          SETTER,       COMBODBLOCKHOLE,              1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getBlockTrigger",          ZVARTYPEID_FLOAT,         GETTER,       COMBODBLOCKTRIG,              1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setBlockTrigger",          ZVARTYPEID_VOID,          SETTER,       COMBODBLOCKTRIG,              1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getBlockWeapon[]",         ZVARTYPEID_FLOAT,         GETTER,       COMBODBLOCKWEAPON,            32,            0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setBlockWeapon[]",         ZVARTYPEID_VOID,          SETTER,       COMBODBLOCKWEAPON,            32,            0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getConveyorSpeedX",        ZVARTYPEID_FLOAT,         GETTER,       COMBODCONVXSPEED,             1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setConveyorSpeedX",        ZVARTYPEID_VOID,          SETTER,       COMBODCONVXSPEED,             1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getConveyorSpeedY",        ZVARTYPEID_FLOAT,         GETTER,       COMBODCONVYSPEED,             1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setConveyorSpeedY",        ZVARTYPEID_VOID,          SETTER,       COMBODCONVYSPEED,             1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSpawnNPC",              ZVARTYPEID_FLOAT,         GETTER,       COMBODSPAWNNPC,               1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSpawnNPC",              ZVARTYPEID_VOID,          SETTER,       COMBODSPAWNNPC,               1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSpawnNPCWhen",          ZVARTYPEID_FLOAT,         GETTER,       COMBODSPAWNNPCWHEN,           1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSpawnNPCWhen",          ZVARTYPEID_VOID,          SETTER,       COMBODSPAWNNPCWHEN,           1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSpawnNPCCHange",        ZVARTYPEID_FLOAT,         GETTER,       COMBODSPAWNNPCCHANGE,         1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSpawnNPCChange",        ZVARTYPEID_VOID,          SETTER,       COMBODSPAWNNPCCHANGE,         1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDirChange",             ZVARTYPEID_FLOAT,         GETTER,       COMBODDIRCHANGETYPE,          1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDirChange",             ZVARTYPEID_VOID,          SETTER,       COMBODDIRCHANGETYPE,          1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDistanceChangeTiles",   ZVARTYPEID_FLOAT,         GETTER,       COMBODDISTANCECHANGETILES,    1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDistanceChangeTiles",   ZVARTYPEID_VOID,          SETTER,       COMBODDISTANCECHANGETILES,    1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDiveItem",              ZVARTYPEID_FLOAT,         GETTER,       COMBODDIVEITEM,               1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDiveItem",              ZVARTYPEID_VOID,          SETTER,       COMBODDIVEITEM,               1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDock",                  ZVARTYPEID_FLOAT,         GETTER,       COMBODDOCK,                   1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDock",                  ZVARTYPEID_VOID,          SETTER,       COMBODDOCK,                   1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFairy",                 ZVARTYPEID_FLOAT,         GETTER,       COMBODFAIRY,                  1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFairy",                 ZVARTYPEID_VOID,          SETTER,       COMBODFAIRY,                  1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFFCAttributeChange",    ZVARTYPEID_FLOAT,         GETTER,       COMBODFFATTRCHANGE,           1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCAttributeChange",    ZVARTYPEID_VOID,          SETTER,       COMBODFFATTRCHANGE,           1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDecorationTile",        ZVARTYPEID_FLOAT,         GETTER,       COMBODFOORDECOTILE,           1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDecorationTile",        ZVARTYPEID_VOID,          SETTER,       COMBODFOORDECOTILE,           1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDecorationType",        ZVARTYPEID_FLOAT,         GETTER,       COMBODFOORDECOTYPE,           1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDecorationType",        ZVARTYPEID_VOID,          SETTER,       COMBODFOORDECOTYPE,           1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHookshot",              ZVARTYPEID_FLOAT,         GETTER,       COMBODHOOKSHOTPOINT,          1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHookshot",              ZVARTYPEID_VOID,          SETTER,       COMBODHOOKSHOTPOINT,          1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLadder",                ZVARTYPEID_FLOAT,         GETTER,       COMBODLADDERPASS,             1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLadder",                ZVARTYPEID_VOID,          SETTER,       COMBODLADDERPASS,             1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLockBlock",             ZVARTYPEID_FLOAT,         GETTER,       COMBODLOCKBLOCK,              1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLockBlock",             ZVARTYPEID_VOID,          SETTER,       COMBODLOCKBLOCK,              1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLockBlockChange",       ZVARTYPEID_FLOAT,         GETTER,       COMBODLOCKBLOCKCHANGE,        1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLockBlockChange",       ZVARTYPEID_VOID,          SETTER,       COMBODLOCKBLOCKCHANGE,        1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMirror",                ZVARTYPEID_FLOAT,         GETTER,       COMBODMAGICMIRROR,            1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMirror",                ZVARTYPEID_VOID,          SETTER,       COMBODMAGICMIRROR,            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDamageAmount",          ZVARTYPEID_FLOAT,         GETTER,       COMBODMODHPAMOUNT,            1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDamageAmount",          ZVARTYPEID_VOID,          SETTER,       COMBODMODHPAMOUNT,            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDamageDelay",           ZVARTYPEID_FLOAT,         GETTER,       COMBODMODHPDELAY,             1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDamageDelay",           ZVARTYPEID_VOID,          SETTER,       COMBODMODHPDELAY,             1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDamageType",            ZVARTYPEID_FLOAT,         GETTER,       COMBODMODHPTYPE,              1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDamageType",            ZVARTYPEID_VOID,          SETTER,       COMBODMODHPTYPE,              1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMagicAmount",           ZVARTYPEID_FLOAT,         GETTER,       COMBODNMODMPAMOUNT,           1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMagicAmount",           ZVARTYPEID_VOID,          SETTER,       COMBODNMODMPAMOUNT,           1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMagicDelay",            ZVARTYPEID_FLOAT,         GETTER,       COMBODMODMPDELAY,             1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMagicDelay",            ZVARTYPEID_VOID,          SETTER,       COMBODMODMPDELAY,             1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMagicType",             ZVARTYPEID_FLOAT,         GETTER,       COMBODMODMPTYPE,              1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMagicType",             ZVARTYPEID_VOID,          SETTER,       COMBODMODMPTYPE,              1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getNoPushBlocks",          ZVARTYPEID_FLOAT,         GETTER,       COMBODNOPUSHBLOCK,            1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNoPushBlocks",          ZVARTYPEID_VOID,          SETTER,       COMBODNOPUSHBLOCK,            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getOverhead",              ZVARTYPEID_FLOAT,         GETTER,       COMBODOVERHEAD,               1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setOverhead",              ZVARTYPEID_VOID,          SETTER,       COMBODOVERHEAD,               1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPlaceNPC",              ZVARTYPEID_FLOAT,         GETTER,       COMBODPLACENPC,               1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPlaceNPC",              ZVARTYPEID_VOID,          SETTER,       COMBODPLACENPC,               1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPushDir",               ZVARTYPEID_FLOAT,         GETTER,       COMBODPUSHDIR,                1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPushDir",               ZVARTYPEID_VOID,          SETTER,       COMBODPUSHDIR,                1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPushDelay",             ZVARTYPEID_FLOAT,         GETTER,       COMBODPUSHWAIT,               42,            0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPushDelay",             ZVARTYPEID_VOID,          SETTER,       COMBODPUSHWAIT,               42,            0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPushHeavy",             ZVARTYPEID_FLOAT,         GETTER,       COMBODPUSHHEAVY,              1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPushHeavy",             ZVARTYPEID_VOID,          SETTER,       COMBODPUSHHEAVY,              1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPushed",                ZVARTYPEID_FLOAT,         GETTER,       COMBODPUSHED,                 1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPushed",                ZVARTYPEID_VOID,          SETTER,       COMBODPUSHED,                 1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRaft",                  ZVARTYPEID_FLOAT,         GETTER,       COMBODRAFT,                   1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRaft",                  ZVARTYPEID_VOID,          SETTER,       COMBODRAFT,                   1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getResetRoom",             ZVARTYPEID_FLOAT,         GETTER,       COMBODRESETROOM,              1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setResetRoom",             ZVARTYPEID_VOID,          SETTER,       COMBODRESETROOM,              1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSavePoint",             ZVARTYPEID_FLOAT,         GETTER,       COMBODSAVEPOINTTYPE,          1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSavePoint",             ZVARTYPEID_VOID,          SETTER,       COMBODSAVEPOINTTYPE,          1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFreezeScreen",          ZVARTYPEID_FLOAT,         GETTER,       COMBODSCREENFREEZETYPE,       1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFreezeScreen",          ZVARTYPEID_VOID,          SETTER,       COMBODSCREENFREEZETYPE,       1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSecretCombo",           ZVARTYPEID_FLOAT,         GETTER,       COMBODSECRETCOMBO,            1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSecretCombo",           ZVARTYPEID_VOID,          SETTER,       COMBODSECRETCOMBO,            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSingular",              ZVARTYPEID_FLOAT,         GETTER,       COMBODSINGULAR,               1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSingular",              ZVARTYPEID_VOID,          SETTER,       COMBODSINGULAR,               1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSlowWalk",              ZVARTYPEID_FLOAT,         GETTER,       COMBODSLOWWALK,               1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSlowWalk",              ZVARTYPEID_VOID,          SETTER,       COMBODSLOWWALK,               1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStatue",                ZVARTYPEID_FLOAT,         GETTER,       COMBODSTATUETYPE,             1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStatue",                ZVARTYPEID_VOID,          SETTER,       COMBODSTATUETYPE,             1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStep",                  ZVARTYPEID_FLOAT,         GETTER,       COMBODSTEPTYPE,               1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStep",                  ZVARTYPEID_VOID,          SETTER,       COMBODSTEPTYPE,               1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStepChange",            ZVARTYPEID_FLOAT,         GETTER,       COMBODSTEPCHANGEINTO,         1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStepChange",            ZVARTYPEID_VOID,          SETTER,       COMBODSTEPCHANGEINTO,         1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStrike[]",              ZVARTYPEID_FLOAT,         GETTER,       COMBODSTRIKEWEAPONS,          32,            0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStrike[]",              ZVARTYPEID_VOID,          SETTER,       COMBODSTRIKEWEAPONS,          32,            0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStrikeRemnants",        ZVARTYPEID_FLOAT,         GETTER,       COMBODSTRIKEREMNANTS,         1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStrikeRemnants",        ZVARTYPEID_VOID,          SETTER,       COMBODSTRIKEREMNANTS,         1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStrikeRemnantsType",    ZVARTYPEID_FLOAT,         GETTER,       COMBODSTRIKEREMNANTSTYPE,     1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStrikeRemnantsType",    ZVARTYPEID_VOID,          SETTER,       COMBODSTRIKEREMNANTSTYPE,     1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStrikeChange",          ZVARTYPEID_FLOAT,         GETTER,       COMBODSTRIKECHANGE,           1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStrikeChange",          ZVARTYPEID_VOID,          SETTER,       COMBODSTRIKECHANGE,           1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStrikeItem",            ZVARTYPEID_FLOAT,         GETTER,       COMBODTOUCHITEM,              1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStrikeItem",            ZVARTYPEID_VOID,          SETTER,       COMBODTOUCHITEM,              1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTouchItem",             ZVARTYPEID_FLOAT,         GETTER,       COMBODTOUCHITEM,              1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTouchItem",             ZVARTYPEID_VOID,          SETTER,       COMBODTOUCHITEM,              1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTouchStairs",           ZVARTYPEID_FLOAT,         GETTER,       COMBODTOUCHSTAIRS,            1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTouchStairs",           ZVARTYPEID_VOID,          SETTER,       COMBODTOUCHSTAIRS,            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTriggerType",           ZVARTYPEID_FLOAT,         GETTER,       COMBODTRIGGERTYPE,            1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTriggerType",           ZVARTYPEID_VOID,          SETTER,       COMBODTRIGGERTYPE,            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTriggerSensitivity",    ZVARTYPEID_FLOAT,         GETTER,       COMBODTRIGGERSENS,            1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTriggerSensitivity",    ZVARTYPEID_VOID,          SETTER,       COMBODTRIGGERSENS,            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWarp",                  ZVARTYPEID_FLOAT,         GETTER,       COMBODWARPTYPE,               1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWarp",                  ZVARTYPEID_VOID,          SETTER,       COMBODWARPTYPE,               1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWarpSensitivity",       ZVARTYPEID_FLOAT,         GETTER,       COMBODWARPSENS,               1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWarpSensitivity",       ZVARTYPEID_VOID,          SETTER,       COMBODWARPSENS,               1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWarpDirect",            ZVARTYPEID_FLOAT,         GETTER,       COMBODWARPDIRECT,             1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWarpDirect",            ZVARTYPEID_VOID,          SETTER,       COMBODWARPDIRECT,             1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWarpLocation",          ZVARTYPEID_FLOAT,         GETTER,       COMBODWARPLOCATION,           1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWarpLocation",          ZVARTYPEID_VOID,          SETTER,       COMBODWARPLOCATION,           1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWater",                 ZVARTYPEID_FLOAT,         GETTER,       COMBODWATER,                  1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWater",                 ZVARTYPEID_VOID,          SETTER,       COMBODWATER,                  1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWhistle",               ZVARTYPEID_FLOAT,         GETTER,       COMBODWHISTLE,                1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWhistle",               ZVARTYPEID_VOID,          SETTER,       COMBODWHISTLE,                1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWinGame",               ZVARTYPEID_FLOAT,         GETTER,       COMBODWINGAME,                1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWinGame",               ZVARTYPEID_VOID,          SETTER,       COMBODWINGAME,                1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getBlockWeaponLevel",      ZVARTYPEID_FLOAT,         GETTER,       COMBODBLOCKWPNLEVEL,          1,             0,                                    1,           { ZVARTYPEID_COMBOS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setBlockWeaponLevel",      ZVARTYPEID_VOID,          SETTER,       COMBODBLOCKWPNLEVEL,          1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
//	Functions
//	{ "GetName",                  ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "SetName",                  ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	
//	one input, one return
	{ "GetBlockEnemies",          ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetBlockHole",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetBlockTrigger",          ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetConveyorSpeedX",        ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetConveyorSpeedY",        ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetCreateEnemy",           ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetCreateEnemyWhen",       ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetCreateEnemyChnge",      ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetDirChangeType",         ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetDistanceChangeTiles",   ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetDiveItem",              ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetDock",                  ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetFairy",                 ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetFFComboChangeAttrib",   ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetFootDecorationsTile",   ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetFootDecorationsType",   ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetHookshotGrab",          ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetLadderPass",            ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetLockBlockType",         ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetLockBlockChange",       ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetMagicMirror",           ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetModifyHPAmount",        ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetModifyHPDelay",         ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetModifyHPType",          ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetModifyMPAmount",        ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetModifyMPDelay",         ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetModifyMPType",          ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetNoPushBlocks",          ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetOverhead",              ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetPlaceEnemy",            ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetPushDirection",         ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetPushWeight",            ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetPushWait",              ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetPushed",                ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetRaft",                  ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetResetRoom",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetSavePoint",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetScreenFreeze",          ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetSecretCombo",           ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetSingular",              ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetSlowMove",              ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetStatue",                ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetStepType",              ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetStepChangeTo",          ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetStrikeRemnants",        ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetStrikeRemnantsType",    ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetStrikeChange",          ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetStrikeItem",            ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetTouchItem",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetTouchStairs",           ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetTriggerType",           ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetTriggerSens",           ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetWarpType",              ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetWarpSens",              ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetWarpDirect",            ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetWarpLocation",          ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetWater",                 ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetWhistle",               ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetWinGame",               ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetBlockWeaponLevel",      ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetTile",                  ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetFlip",                  ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetWalkability",           ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetType",                  ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetCSets",                 ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetFoo",                   ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetFrames",                ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetSpeed",                 ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetNextCombo",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetNextCSet",              ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetFlag",                  ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetSkipAnim",              ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetNextTimer",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetSkipAnimY",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetAnimFlags",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    2,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
//	two inputs, one return
	{ "GetBlockWeapon",           ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetExpansion",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetStrikeWeapons",         ZVARTYPEID_FLOAT,         FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
//	two inputs, no return
	{ "SetBlockEnemies",          ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetBlockHole",             ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetBlockTrigger",          ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetConveyorSpeedX",        ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetConveyorSpeedY",        ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetCreateEnemy",           ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetCreateEnemyWhen",       ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetCreateEnemyChnge",      ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetDirChangeType",         ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetDistanceChangeTiles",   ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetDiveItem",              ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetDock",                  ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetFairy",                 ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetFFComboChangeAttrib",   ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetFootDecorationsTile",   ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetFootDecorationsType",   ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetHookshotGrab",          ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetLadderPass",            ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetLockBlockType",         ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetLockBlockChange",       ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetMagicMirror",           ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetModifyHPAmount",        ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetModifyHPDelay",         ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetModifyHPType",          ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetModifyMPAmount",        ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetModifyMPDelay",         ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetModifyMPType",          ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetNoPushBlocks",          ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetOverhead",              ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetPlaceEnemy",            ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetPushDirection",         ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetPushWeight",            ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetPushWait",              ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetPushed",                ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetRaft",                  ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetResetRoom",             ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetSavePoint",             ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetScreenFreeze",          ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetSecretCombo",           ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetSingular",              ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetSlowMove",              ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetStatue",                ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetStepType",              ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetStepChangeTo",          ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetStrikeRemnants",        ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetStrikeRemnantsType",    ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetStrikeChange",          ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetStrikeItem",            ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetTouchItem",             ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetTouchStairs",           ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetTriggerType",           ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetTriggerSens",           ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetWarpType",              ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetWarpSens",              ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetWarpDirect",            ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetWarpLocation",          ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetWater",                 ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetWhistle",               ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetWinGame",               ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetBlockWeaponLevel",      ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetTile",                  ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetFlip",                  ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetWalkability",           ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetType",                  ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetCSets",                 ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetFoo",                   ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetFrames",                ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetSpeed",                 ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetNextCombo",             ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetNextCSet",              ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetFlag",                  ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetSkipAnim",              ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetNextTimer",             ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetSkipAnimY",             ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetAnimFlags",             ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    3,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
//	three inputs, no return
	{ "SetBlockWeapon",           ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    4,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetExpansion",             ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    4,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetStrikeWeapons",         ZVARTYPEID_VOID,          FUNCTION,     0,                            1,             0,                                    4,           { ZVARTYPEID_COMBOS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	
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
        ONE_INPUT_ONE_RETURN("GetBlockEnemies",OCDataBlockEnemy);
    }
    {
        ONE_INPUT_ONE_RETURN("GetBlockHole",OCDataBlockHole);
    }
    {
        ONE_INPUT_ONE_RETURN("GetBlockTrigger",OCDataBlockTrig);
    }
    {
        ONE_INPUT_ONE_RETURN("GetConveyorSpeedX",OCDataConveyX);
    }
    {
        ONE_INPUT_ONE_RETURN("GetConveyorSpeedY",OCDataConveyY);
    }
    {
        ONE_INPUT_ONE_RETURN("GetCreateEnemy",OCDataCreateNPC);
    }
    {
        ONE_INPUT_ONE_RETURN("GetCreateEnemyWhen",OCDataCreateEnemW);
    }
    {
        ONE_INPUT_ONE_RETURN("GetCreateEnemyChnge",OCDataCreateEnemC);
    }
    {
        ONE_INPUT_ONE_RETURN("GetDirChangeType",OCDataDirch);
    }
    {
        ONE_INPUT_ONE_RETURN("GetDistanceChangeTiles",OCDataDistTiles);
    }
    {
        ONE_INPUT_ONE_RETURN("GetDiveItem",OCDataDiveItem);
    }
    {
        ONE_INPUT_ONE_RETURN("GetDock",OCDataDock);
    }
    {
        ONE_INPUT_ONE_RETURN("GetFairy",OCDataFairy);
    }
    {
        ONE_INPUT_ONE_RETURN("GetFFComboChangeAttrib",OCDataAttrib);
    }
    {
        ONE_INPUT_ONE_RETURN("GetFootDecorationsTile",OCDataDecoTile);
    }
    {
        ONE_INPUT_ONE_RETURN("GetFootDecorationsType",OCDataDecoType);
    }
    {
        ONE_INPUT_ONE_RETURN("GetHookshotGrab",OCDataHookshotGrab);
    }
    {
        ONE_INPUT_ONE_RETURN("GetLadderPass",OCDataLadderPass);
    }
    {
        ONE_INPUT_ONE_RETURN("GetLockBlockType",OCDataLockBlock);
    }
    {
        ONE_INPUT_ONE_RETURN("GetLockBlockChange",OCDataLockBlockChange);
    }
    {
        ONE_INPUT_ONE_RETURN("GetMagicMirror",OCDataMagicMirror);
    }
    {
        ONE_INPUT_ONE_RETURN("GetModifyHPAmount",OCDataModHP);
    }
    {
        ONE_INPUT_ONE_RETURN("GetModifyHPDelay",OCDataModHPDelay);
    }
    {
        ONE_INPUT_ONE_RETURN("GetModifyHPType",OCDataModHpType);
    }
    {
        ONE_INPUT_ONE_RETURN("GetModifyMPAmount",OCDataModMP);
    }
    {
        ONE_INPUT_ONE_RETURN("GetModifyMPDelay",OCDataMpdMPDelay);
    }
    {
        ONE_INPUT_ONE_RETURN("GetModifyMPType",OCDataModMPType);
    }
    {
        ONE_INPUT_ONE_RETURN("GetNoPushBlocks",OCDataNoPush);
    }
    {
        ONE_INPUT_ONE_RETURN("GetOverhead",OCDataOverhead);
    }
    {
        ONE_INPUT_ONE_RETURN("GetPlaceEnemy",OCDataEnemyLoc);
    }
    {
        ONE_INPUT_ONE_RETURN("GetPushDirection",OCDataPushDir);
    }
    {
        ONE_INPUT_ONE_RETURN("GetPushWeight",OCDataPushWeight);
    }
    {
        ONE_INPUT_ONE_RETURN("GetPushWait",OCDataPushWait);
    }
    {
        ONE_INPUT_ONE_RETURN("GetPushed",OCDataPushed);
    }
    {
        ONE_INPUT_ONE_RETURN("GetRaft",OCDataRaft);
    }
    {
        ONE_INPUT_ONE_RETURN("GetResetRoom",OCDataResetRoom);
    }
    {
        ONE_INPUT_ONE_RETURN("GetSavePoint",OCDataSavePoint);
    }
    {
        ONE_INPUT_ONE_RETURN("GetScreenFreeze",OCDataFreeezeScreen);
    }
    {
        ONE_INPUT_ONE_RETURN("GetSecretCombo",OCDataSecretCombo);
    }
    {
        ONE_INPUT_ONE_RETURN("GetSingular",OCDataSingular);
    }
    {
        ONE_INPUT_ONE_RETURN("GetSlowMove",OCDataSlowMove);
    }
    {
        ONE_INPUT_ONE_RETURN("GetStatue",OCDataStatue);
    }
    {
        ONE_INPUT_ONE_RETURN("GetStepType",OCDataStepType);
    }
    {
        ONE_INPUT_ONE_RETURN("GetStepChangeTo",OCDataSteoChange);
    }
    {
        ONE_INPUT_ONE_RETURN("GetStrikeRemnants",OCDataStrikeRem);
    }
    {
        ONE_INPUT_ONE_RETURN("GetStrikeRemnantsType",OCDataStrikeRemType);
    }
    {
        ONE_INPUT_ONE_RETURN("GetStrikeChange",OCDataStrikeChange);
    }
    {
        ONE_INPUT_ONE_RETURN("GetStrikeItem",OCDataStrikeChangeItem);
    }
    {
        ONE_INPUT_ONE_RETURN("GetTouchItem",OCDataTouchItem);
    }
    {
        ONE_INPUT_ONE_RETURN("GetTouchStairs",OCDataTouchStairs);
    }
    {
        ONE_INPUT_ONE_RETURN("GetTriggerType",OCDataTriggerType);
    }
    {
        ONE_INPUT_ONE_RETURN("GetTriggerSens",OCDataTriggerSens);
    }
    {
        ONE_INPUT_ONE_RETURN("GetWarpType",OCDataWarpType);
    }
    {
        ONE_INPUT_ONE_RETURN("GetWarpSens",OCDataWarpSens);
    }
    {
        ONE_INPUT_ONE_RETURN("GetWarpDirect",OCDataWarpDirect);
    }
    {
        ONE_INPUT_ONE_RETURN("GetWarpLocation",OCDataWarpLoc);
    }
    {
        ONE_INPUT_ONE_RETURN("GetWater",OCDataWater);
    }
    {
        ONE_INPUT_ONE_RETURN("GetWhistle",OCDataWhistle);
    }
    {
        ONE_INPUT_ONE_RETURN("GetWinGame",OCDataWinGame);
    }
    {
        ONE_INPUT_ONE_RETURN("GetBlockWeaponLevel",OCDataWeapBlockLevel);
    }
    {
        ONE_INPUT_ONE_RETURN("GetTile",OCDataTile);
    }
    {
        ONE_INPUT_ONE_RETURN("GetFlip",OCDataFlip);
    }
    {
        ONE_INPUT_ONE_RETURN("GetWalkability",OCDataWalkability);
    }
    {
        ONE_INPUT_ONE_RETURN("GetType",OCDataType);
    }
    {
        ONE_INPUT_ONE_RETURN("GetCSets",OCDataCSets);
    }
    {
        ONE_INPUT_ONE_RETURN("GetFoo",OCDataFoo);
    }
    {
        ONE_INPUT_ONE_RETURN("GetFrames",OCDataFrames);
    }
    {
        ONE_INPUT_ONE_RETURN("GetSpeed",OCDataSpeed);
    }
    {
        ONE_INPUT_ONE_RETURN("GetNextCombo",OCDataNext);
    }
    {
        ONE_INPUT_ONE_RETURN("GetNextCSet",OCDataNextCSet);
    }
    {
        ONE_INPUT_ONE_RETURN("GetFlag",OCDataFlag);
    }
    {
        ONE_INPUT_ONE_RETURN("GetSkipAnim",OCSetDataSkipAnim);
    }
    {
        ONE_INPUT_ONE_RETURN("GetNextTimer",OCDataTimer);
    }
    {
        ONE_INPUT_ONE_RETURN("GetSkipAnimY",OCDataAnimY);
    }
    {
        ONE_INPUT_ONE_RETURN("GetAnimFlags",OCDataAnimFlags);
    }
    
    {
        TWO_INPUT_NO_RETURN("SetBlockEnemies",OCSetDataBlockEnemy);
    }
    {
        TWO_INPUT_NO_RETURN("SetBlockHole",OCSetDataBlockHole);
    }
    {
        TWO_INPUT_NO_RETURN("SetBlockTrigger",OCSetDataBlockTrig);
    }
    {
        TWO_INPUT_NO_RETURN("SetConveyorSpeedX",OCSetDataConveyX);
    }
    {
        TWO_INPUT_NO_RETURN("SetConveyorSpeedY",OCSetDataConveyY);
    }
    {
        TWO_INPUT_NO_RETURN("SetCreateEnemy",OCSetDataCreateNPC);
    }
    {
        TWO_INPUT_NO_RETURN("SetCreateEnemyWhen",OCSetDataCreateEnemW);
    }
    {
        TWO_INPUT_NO_RETURN("SetCreateEnemyChnge",OCSetDataCreateEnemC);
    }
    {
        TWO_INPUT_NO_RETURN("SetDirChangeType",OCSetDataDirch);
    }
    {
        TWO_INPUT_NO_RETURN("SetDistanceChangeTiles",OCSetDataDistTiles);
    }
    {
        TWO_INPUT_NO_RETURN("SetDiveItem",OCSetDataDiveItem);
    }
    {
        TWO_INPUT_NO_RETURN("SetDock",OCSetDataDock);
    }
    {
        TWO_INPUT_NO_RETURN("SetFairy",OCSetDataFairy);
    }
    {
        TWO_INPUT_NO_RETURN("SetFFComboChangeAttrib",OCSetDataAttrib);
    }
    {
        TWO_INPUT_NO_RETURN("SetFootDecorationsTile",OCSetDataDecoTile);
    }
    {
        TWO_INPUT_NO_RETURN("SetFootDecorationsType",OCSetDataDecoType);
    }
    {
        TWO_INPUT_NO_RETURN("SetHookshotGrab",OCSetDataHookshotGrab);
    }
    {
        TWO_INPUT_NO_RETURN("SetLadderPass",OCSetDataLadderPass);
    }
    {
        TWO_INPUT_NO_RETURN("SetLockBlockType",OCSetDataLockBlock);
    }
    {
        TWO_INPUT_NO_RETURN("SetLockBlockChange",OCSetDataLockBlockChange);
    }
    {
        TWO_INPUT_NO_RETURN("SetMagicMirror",OCSetDataMagicMirror);
    }
    {
        TWO_INPUT_NO_RETURN("SetModifyHPAmount",OCSetDataModHP);
    }
    {
        TWO_INPUT_NO_RETURN("SetModifyHPDelay",OCSetDataModHPDelay);
    }
    {
        TWO_INPUT_NO_RETURN("SetModifyHPType",OCSetDataModHpType);
    }
    {
        TWO_INPUT_NO_RETURN("SetModifyMPAmount",OCSetDataModMP);
    }
    {
        TWO_INPUT_NO_RETURN("SetModifyMPDelay",OCSetDataMpdMPDelay);
    }
    {
        TWO_INPUT_NO_RETURN("SetModifyMPType",OCSetDataModMPType);
    }
    {
        TWO_INPUT_NO_RETURN("SetNoPushBlocks",OCSetDataNoPush);
    }
    {
        TWO_INPUT_NO_RETURN("SetOverhead",OCSetDataOverhead);
    }
    {
        TWO_INPUT_NO_RETURN("SetPlaceEnemy",OCSetDataEnemyLoc);
    }
    {
        TWO_INPUT_NO_RETURN("SetPushDirection",OCSetDataPushDir);
    }
    {
        TWO_INPUT_NO_RETURN("SetPushWeight",OCSetDataPushWeight);
    }
    {
        TWO_INPUT_NO_RETURN("SetPushWait",OCSetDataPushWait);
    }
    {
        TWO_INPUT_NO_RETURN("SetPushed",OCSetDataPushed);
    }
    {
        TWO_INPUT_NO_RETURN("SetRaft",OCSetDataRaft);
    }
    {
        TWO_INPUT_NO_RETURN("SetResetRoom",OCSetDataResetRoom);
    }
    {
        TWO_INPUT_NO_RETURN("SetSavePoint",OCSetDataSavePoint);
    }
    {
        TWO_INPUT_NO_RETURN("SetScreenFreeze",OCSetDataFreeezeScreen);
    }
    {
        TWO_INPUT_NO_RETURN("SetSecretCombo",OCSetDataSecretCombo);
    }
    {
        TWO_INPUT_NO_RETURN("SetSingular",OCSetDataSingular);
    }
    {
        TWO_INPUT_NO_RETURN("SetSlowMove",OCSetDataSlowMove);
    }
    {
        TWO_INPUT_NO_RETURN("SetStatue",OCSetDataStatue);
    }
    {
        TWO_INPUT_NO_RETURN("SetStepType",OCSetDataStepType);
    }
    {
        TWO_INPUT_NO_RETURN("SetStepChangeTo",OCSetDataSteoChange);
    }
    {
        TWO_INPUT_NO_RETURN("SetStrikeRemnants",OCSetDataStrikeRem);
    }
    {
        TWO_INPUT_NO_RETURN("SetStrikeRemnantsType",OCSetDataStrikeRemType);
    }
    {
        TWO_INPUT_NO_RETURN("SetStrikeChange",OCSetDataStrikeChange);
    }
    {
        TWO_INPUT_NO_RETURN("SetStrikeItem",OCSetDataStrikeChangeItem);
    }
    {
        TWO_INPUT_NO_RETURN("SetTouchItem",OCSetDataTouchItem);
    }
    {
        TWO_INPUT_NO_RETURN("SetTouchStairs",OCSetDataTouchStairs);
    }
    {
        TWO_INPUT_NO_RETURN("SetTriggerType",OCSetDataTriggerType);
    }
    {
        TWO_INPUT_NO_RETURN("SetTriggerSens",OCSetDataTriggerSens);
    }
    {
        TWO_INPUT_NO_RETURN("SetWarpType",OCSetDataWarpType);
    }
    {
        TWO_INPUT_NO_RETURN("SetWarpSens",OCSetDataWarpSens);
    }
    {
        TWO_INPUT_NO_RETURN("SetWarpDirect",OCSetDataWarpDirect);
    }
    {
        TWO_INPUT_NO_RETURN("SetWarpLocation",OCSetDataWarpLoc);
    }
    {
        TWO_INPUT_NO_RETURN("SetWater",OCSetDataWater);
    }
    {
        TWO_INPUT_NO_RETURN("SetWhistle",OCSetDataWhistle);
    }
    {
        TWO_INPUT_NO_RETURN("SetWinGame",OCSetDataWinGame);
    }
    {
        TWO_INPUT_NO_RETURN("SetBlockWeaponLevel",OCSetDataWeapBlockLevel);
    }
    {
        TWO_INPUT_NO_RETURN("SetTile",OCSetDataTile);
    }
    {
        TWO_INPUT_NO_RETURN("SetFlip",OCSetDataFlip);
    }
    {
        TWO_INPUT_NO_RETURN("SetWalkability",OCSetDataWalkability);
    }
    {
        TWO_INPUT_NO_RETURN("SetType",OCSetDataType);
    }
    {
        TWO_INPUT_NO_RETURN("SetCSets",OCSetDataCSets);
    }
    {
        TWO_INPUT_NO_RETURN("SetFoo",OCSetDataFoo);
    }
    {
        TWO_INPUT_NO_RETURN("SetFrames",OCSetDataFrames);
    }
    {
        TWO_INPUT_NO_RETURN("SetSpeed",OCSetDataSpeed);
    }
    {
        TWO_INPUT_NO_RETURN("SetNextCombo",OCSetDataNext);
    }
    {
        TWO_INPUT_NO_RETURN("SetNextCSet",OCSetDataNextCSet);
    }
    {
        TWO_INPUT_NO_RETURN("SetFlag",OCSetDataFlag);
    }
    {
        TWO_INPUT_NO_RETURN("SetSkipAnim",OCSetDataSkipAnim);
    }
    {
        TWO_INPUT_NO_RETURN("SetNextTimer",OCSetDataTimer);
    }
    {
        TWO_INPUT_NO_RETURN("SetSkipAnimY",OCSetDataAnimY);
    }
    {
        TWO_INPUT_NO_RETURN("SetAnimFlags",OCSetDataAnimFlags);
    }
    
    {
        TWO_INPUT_ONE_RETURN("GetBlockWeapon",OCDataBlockWeapon);
    }
    {
        TWO_INPUT_ONE_RETURN("GetExpansion",OCDataExpansion);
    }
    {
        TWO_INPUT_ONE_RETURN("GetStrikeWeapons",OCDataStrikeWeapon);
    }
    {
	THREE_INPUT_NO_RETURN("SetBlockWeapon", SCDBLOCKWEAPON);
    }
    /*
    {
	THREE_INPUT_NO_RETURN("SetExpansion", SCDEXPANSION);
    }
    */
    {
	THREE_INPUT_NO_RETURN("SetStrikeWeapons", SCDSTRIKEWEAPONS);
    }
}

