#include "SymbolDefs.h"

GameSymbols GameSymbols::singleton = GameSymbols();

static AccessorTable gameTable[] =
{
//	  name,                            rettype,                  setorget,     var,                  numindex,       funcFlags,                            numParams,   params
	{ "IncrementQuest",                   ZVARTYPEID_FLOAT,         GETTER,       INCQST,                 1,             0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MaxNPCs",                   ZVARTYPEID_FLOAT,         GETTER,       SPRITEMAXNPC,                 1,             0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MaxNPCs",                   ZVARTYPEID_VOID,          SETTER,       SPRITEMAXNPC,                 1,             0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MaxLWeapons",                   ZVARTYPEID_FLOAT,         GETTER,       SPRITEMAXLWPN,                 1,             0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MaxLWeapons",                   ZVARTYPEID_VOID,          SETTER,       SPRITEMAXLWPN,                 1,             0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MaxEWeapons",                   ZVARTYPEID_FLOAT,         GETTER,       SPRITEMAXEWPN,                 1,             0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MaxEWeapons",                   ZVARTYPEID_VOID,          SETTER,       SPRITEMAXEWPN,                 1,             0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MaxItemsprites",                   ZVARTYPEID_FLOAT,         GETTER,       SPRITEMAXITEM,                 1,             0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MaxItemsprites",                   ZVARTYPEID_VOID,          SETTER,       SPRITEMAXITEM,                 1,             0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSTD[]",                 ZVARTYPEID_UNTYPED,         GETTER,       STDARR,         256,              0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSTD[]",                 ZVARTYPEID_VOID,          SETTER,       STDARR,         256,              0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getGhost[]",                 ZVARTYPEID_UNTYPED,         GETTER,       GHOSTARR,         256,              0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setGhost[]",                 ZVARTYPEID_VOID,          SETTER,       GHOSTARR,         256,              0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTango[]",                 ZVARTYPEID_UNTYPED,         GETTER,       TANGOARR,         256,              0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTango[]",                 ZVARTYPEID_VOID,          SETTER,       TANGOARR,         256,              0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetCurScreen",                  ZVARTYPEID_FLOAT,         GETTER,       CURSCR,               1,              0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetCurDMapScreen",              ZVARTYPEID_FLOAT,         GETTER,       CURDSCR,              1,              0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetCurMap",                     ZVARTYPEID_FLOAT,         GETTER,       CURMAP,               1,              0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetCurDMap",                    ZVARTYPEID_FLOAT,         GETTER,       CURDMAP,              1,              0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetCurLevel",                   ZVARTYPEID_FLOAT,         GETTER,       CURLEVEL,             1,              0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getNumDeaths",                  ZVARTYPEID_FLOAT,         GETTER,       GAMEDEATHS,           1,              0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNumDeaths",                  ZVARTYPEID_VOID,          SETTER,       GAMEDEATHS,           1,              0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCheat",                      ZVARTYPEID_FLOAT,         GETTER,       GAMECHEAT,            1,              0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCheat",                      ZVARTYPEID_VOID,          SETTER,       GAMECHEAT,            1,              0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMaxCheat",                   ZVARTYPEID_FLOAT,         GETTER,       GAMEMAXCHEAT,            1,              0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMaxCheat",                   ZVARTYPEID_VOID,          SETTER,       GAMEMAXCHEAT,            1,              0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSubscreenSpeed",                       ZVARTYPEID_LONG,          GETTER,       ACTIVESSSPEED,             1,              0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSubscreenSpeed",                       ZVARTYPEID_VOID,          SETTER,       ACTIVESSSPEED,             1,              0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_LONG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTime",                       ZVARTYPEID_LONG,          GETTER,       GAMETIME,             1,              0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTime",                       ZVARTYPEID_VOID,          SETTER,       GAMETIME,             1,              0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_LONG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHasPlayed",                  ZVARTYPEID_BOOL,          GETTER,       GAMEHASPLAYED,        1,              0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHasPlayed",                  ZVARTYPEID_VOID,          SETTER,       GAMEHASPLAYED,        1,              0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTypingMode",                 ZVARTYPEID_BOOL,          GETTER,       TYPINGMODE,           1,              0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTypingMode",                 ZVARTYPEID_VOID,          SETTER,       TYPINGMODE,           1,              0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTimeValid",                  ZVARTYPEID_BOOL,          GETTER,       GAMETIMEVALID,        1,              0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTimeValid",                  ZVARTYPEID_VOID,          SETTER,       GAMETIMEVALID,        1,              0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getGuyCount[]",                 ZVARTYPEID_FLOAT,         GETTER,       GAMEGUYCOUNT,         2,              0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setGuyCount[]",                 ZVARTYPEID_VOID,          SETTER,       GAMEGUYCOUNT,         2,              0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getContinueScreen",             ZVARTYPEID_FLOAT,         GETTER,       GAMECONTSCR,          1,              0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setContinueScreen",             ZVARTYPEID_VOID,          SETTER,       GAMECONTSCR,          1,              0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getContinueDMap",               ZVARTYPEID_FLOAT,         GETTER,       GAMECONTDMAP,         1,              0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setContinueDMap",               ZVARTYPEID_VOID,          SETTER,       GAMECONTDMAP,         1,              0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCounter[]",                  ZVARTYPEID_FLOAT,         GETTER,       GAMECOUNTERD,         32,             0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCounter[]",                  ZVARTYPEID_VOID,          SETTER,       GAMECOUNTERD,         32,             0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMCounter[]",                 ZVARTYPEID_FLOAT,         GETTER,       GAMEMCOUNTERD,        32,             0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMCounter[]",                 ZVARTYPEID_VOID,          SETTER,       GAMEMCOUNTERD,        32,             0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDCounter[]",                 ZVARTYPEID_FLOAT,         GETTER,       GAMEDCOUNTERD,        32,             0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDCounter[]",                 ZVARTYPEID_VOID,          SETTER,       GAMEDCOUNTERD,        32,             0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getGeneric[]",                  ZVARTYPEID_FLOAT,         GETTER,       GAMEGENERICD,         256,            0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setGeneric[]",                  ZVARTYPEID_VOID,          SETTER,       GAMEGENERICD,         256,            0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMisc[]",                     ZVARTYPEID_UNTYPED,       GETTER,       GAMEMISC,             32,             0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMisc[]",                     ZVARTYPEID_VOID,          SETTER,       GAMEMISC,             32,             0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLItems[]",                   ZVARTYPEID_FLOAT,         GETTER,       GAMELITEMSD,          512,            0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLItems[]",                   ZVARTYPEID_VOID,          SETTER,       GAMELITEMSD,          512,            0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLKeys[]",                    ZVARTYPEID_FLOAT,         GETTER,       GAMELKEYSD,           512,            0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLKeys[]",                    ZVARTYPEID_VOID,          SETTER,       GAMELKEYSD,           512,            0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLSwitches[]",                ZVARTYPEID_LONG,          GETTER,       GAMELSWITCH,          512,            0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLSwitches[]",                ZVARTYPEID_VOID,          SETTER,       GAMELSWITCH,          512,            0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_LONG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getGSwitch[]",                  ZVARTYPEID_FLOAT,         GETTER,       GAMEGSWITCH,          256,            0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setGSwitch[]",                  ZVARTYPEID_VOID,          SETTER,       GAMEGSWITCH,          256,            0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getBottleState[]",              ZVARTYPEID_FLOAT,         GETTER,       GAMEBOTTLEST,         256,            0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setBottleState[]",              ZVARTYPEID_VOID,          SETTER,       GAMEBOTTLEST,         256,            0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetScreenState",                ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetScreenState",                ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    5,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetScreenD",                    ZVARTYPEID_UNTYPED,       FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetScreenD",                    ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetDMapScreenD",                ZVARTYPEID_UNTYPED,       FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetDMapScreenD",                ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    5,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadItemData",                  ZVARTYPEID_ITEMCLASS,     FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "PlaySound",                     ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "PlayMIDI",                      ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "PlayEnhancedMusic",             ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetDMapMusicFilename",          ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetDMapMusicTrack",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetDMapEnhancedMusic",          ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetComboData",                  ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetComboData",                  ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    5,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetComboCSet",                  ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetComboCSet",                  ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    5,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetComboFlag",                  ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetComboFlag",                  ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    5,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetComboType",                  ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetComboType",                  ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    5,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetComboInherentFlag",          ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetComboInherentFlag",          ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    5,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetComboSolid",                 ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetComboSolid",                 ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    5,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetMIDI",                       ZVARTYPEID_FLOAT,         GETTER,       GETMIDI,              1,              0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetScreenFlags",                ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetScreenEFlags",               ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDMapFlags[]",                ZVARTYPEID_FLOAT,         GETTER,       DMAPFLAGSD,           512,            0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDMapFlags[]",                ZVARTYPEID_VOID,          SETTER,       DMAPFLAGSD,           512,            0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDMapLevel[]",                ZVARTYPEID_FLOAT,         GETTER,       DMAPLEVELD,           512,            0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDMapLevel[]",                ZVARTYPEID_VOID,          SETTER,       DMAPLEVELD,           512,            0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDMapCompass[]",              ZVARTYPEID_FLOAT,         GETTER,       DMAPCOMPASSD,         512,            0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDMapCompass[]",              ZVARTYPEID_VOID,          SETTER,       DMAPCOMPASSD,         512,            0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDMapContinue[]",             ZVARTYPEID_FLOAT,         GETTER,       DMAPCONTINUED,        512,            0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDMapContinue[]",             ZVARTYPEID_VOID,          SETTER,       DMAPCONTINUED,        512,            0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDMapMIDI[]",                 ZVARTYPEID_FLOAT,         GETTER,       DMAPMIDID,            512,            0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDMapMIDI[]",                 ZVARTYPEID_VOID,          SETTER,       DMAPMIDID,            512,            0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Save",                          ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "End",                           ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Continue",                      ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SaveAndQuit",                   ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SaveAndContinue",               ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "ShowContinueScreen",            ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "ComboTile",                     ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetSaveName",                   ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetSaveName",                   ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetMessage",                    ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetDMapName",                   ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetDMapTitle",                  ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetDMapIntro",                  ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStandalone",                 ZVARTYPEID_BOOL,          GETTER,       GAMESTANDALONE,       1,              0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStandalone",                 ZVARTYPEID_VOID,          SETTER,       GAMESTANDALONE,       1,              0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "ShowSaveScreen",                ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "ShowSaveQuitScreen",            ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLastEntranceScreen",         ZVARTYPEID_FLOAT,         GETTER,       GAMEENTRSCR,          1,              0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLastEntranceScreen",         ZVARTYPEID_VOID,          SETTER,       GAMEENTRSCR,          1,              0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLastEntranceDMap",           ZVARTYPEID_FLOAT,         GETTER,       GAMEENTRDMAP,         1,              0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLastEntranceDMap",           ZVARTYPEID_VOID,          SETTER,       GAMEENTRDMAP,         1,              0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getClickToFreezeEnabled",       ZVARTYPEID_BOOL,          GETTER,       GAMECLICKFREEZE,      1,              0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setClickToFreezeEnabled",       ZVARTYPEID_VOID,          SETTER,       GAMECLICKFREEZE,      1,              0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDMapOffset[]",               ZVARTYPEID_FLOAT,         GETTER,       DMAPOFFSET,           512,            0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDMapOffset[]",               ZVARTYPEID_VOID,          SETTER,       DMAPOFFSET,           512,            0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDMapMap[]",                  ZVARTYPEID_FLOAT,         GETTER,       DMAPMAP,              512,            0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDMapMap[]",                  ZVARTYPEID_VOID,          SETTER,       DMAPMAP,              512,            0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetFFCScript",                  ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetComboScript",                  ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	Get an item script ID, similar to GetFFCScript()
	{ "GetItemScript",                 ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetNPCScript",                 ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetLWeaponScript",                 ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetEWeaponScript",                 ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetPlayerScript",                 ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetLinkScript",                 ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetGlobalScript",                 ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetDMapScript",                 ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetScreenScript",                 ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetItemSpriteScript",                 ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetUntypedScript",                 ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetSubscreenScript",                 ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetNPC",                 ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetItem",                 ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetCombo",                 ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetDMap",                 ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetHeroScript",                 ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetGenericScript",                 ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	Monochrome mode
	{ "GreyscaleOn",                   ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GreyscaleOff",                  ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	Joypad and Keyboard. -Z
	{ "getFFRules[]",                  ZVARTYPEID_BOOL,          GETTER,       FFRULE,               800,            0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFRules[]",                  ZVARTYPEID_VOID,          SETTER,       FFRULE,               800,            0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getButtonPress[]",              ZVARTYPEID_BOOL,          GETTER,       BUTTONPRESS,          18,             0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setButtonPress[]",              ZVARTYPEID_VOID,          SETTER,       BUTTONPRESS,          18,             0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getButtonInput[]",              ZVARTYPEID_BOOL,          GETTER,       BUTTONINPUT,          18,             0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setButtonInput[]",              ZVARTYPEID_VOID,          SETTER,       BUTTONINPUT,          18,             0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getButtonHeld[]",               ZVARTYPEID_BOOL,          GETTER,       BUTTONHELD,           18,             0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setButtonHeld[]",               ZVARTYPEID_VOID,          SETTER,       BUTTONHELD,           18,             0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getReadKey[]",                  ZVARTYPEID_BOOL,          GETTER,       READKEY,              127,            0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "ReadKey",                       ZVARTYPEID_BOOL,          GETTER,       READKEY,              1,              0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSkipCredits",                ZVARTYPEID_BOOL,          GETTER,       SKIPCREDITS,          1,              0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSkipCredits",                ZVARTYPEID_VOID,          SETTER,       SKIPCREDITS,          1,              0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSkipF6",                     ZVARTYPEID_BOOL,          GETTER,       SKIPF6,               1,              0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSkipF6",                     ZVARTYPEID_VOID,          SETTER,       SKIPF6,               1,              0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSuspend[]",              ZVARTYPEID_BOOL,          GETTER,       GAMESUSPEND,         68,            0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSuspend[]",              ZVARTYPEID_VOID,          SETTER,       GAMESUSPEND,         68,            0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	// { "getResume[]",              ZVARTYPEID_BOOL,          GETTER,       GAMERESUME,         36,            0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	// { "setResume[]",              ZVARTYPEID_VOID,          SETTER,       GAMERESUME,         36,            0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDisableItem[]",              ZVARTYPEID_BOOL,          GETTER,       DISABLEDITEM,         256,            0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDisableItem[]",              ZVARTYPEID_VOID,          SETTER,       DISABLEDITEM,         256,            0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getJoypadPress[]",              ZVARTYPEID_BOOL,          GETTER,       JOYPADPRESS,          18,             0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	Read or change the palette (level palette) for any given DMap
	{ "getDMapPalette[]",              ZVARTYPEID_FLOAT,         GETTER,       DMAPLEVELPAL,         512,            0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDMapPalette[]",              ZVARTYPEID_VOID,          SETTER,       DMAPLEVELPAL,         512,            0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	Get the map count 
	{ "MapCount",                      ZVARTYPEID_FLOAT,         GETTER,       GAMEMAXMAPS,          1,              0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	Versioning
	{ "getZScriptVersion",             ZVARTYPEID_FLOAT,         GETTER,       ZSCRIPTVERSION,         1,              0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getVersion",                    ZVARTYPEID_FLOAT,         GETTER,       ZELDAVERSION,         1,              0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getBuild",                      ZVARTYPEID_FLOAT,         GETTER,       ZELDABUILD,           1,              0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getBeta",                       ZVARTYPEID_FLOAT,         GETTER,       ZELDABETA,            1,              0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDisableActiveSubscreen",     ZVARTYPEID_BOOL,          GETTER,       NOACTIVESUBSC,        1,              0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDisableActiveSubscreen",     ZVARTYPEID_VOID,          SETTER,       NOACTIVESUBSC,        1,              0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetMessage",                    ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetDMapName",                   ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetDMapTitle",                  ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetDMapIntro",                  ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetScreenEnemy",                ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetScreenEnemy",                ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    5,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetScreenDoor",                 ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetScreenDoor",                 ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    5,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "SetScreenWidth",                ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetScreenWidth",                ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetScreenHeight",               ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetScreenHeight",               ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetScreenViewX",                ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetScreenViewX",                ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetScreenViewY",                ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetScreenViewY",                ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetScreenGuy",                  ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetScreenGuy",                  ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetScreenString",               ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetScreenString",               ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetScreenRoomType",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetScreenRoomType",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetScreenEntryX",               ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetScreenEntryX",               ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "SetScreenEntryY",               ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetScreenEntryY",               ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetScreenItem",                 ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetScreenItem",                 ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetScreenUndercombo",           ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetScreenUndercombo",           ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetScreenUnderCSet",            ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetScreenUnderCSet",            ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetScreenCatchall",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetScreenCatchall",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetScreenLayerOpacity",         ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    5,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetScreenLayerOpacity",         ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetScreenSecretCombo",          ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    5,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetScreenSecretCombo",          ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetScreenSecretCSet",           ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    5,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetScreenSecretCSet",           ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetScreenSecretFlag",           ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    5,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetScreenSecretFlag",           ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetScreenLayerMap",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    5,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetScreenLayerMap",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetScreenLayerScreen",          ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    5,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetScreenLayerScreen",          ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetScreenPath",                 ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    5,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetScreenPath",                 ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetScreenWarpReturnX",          ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    5,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetScreenWarpReturnX",          ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetScreenWarpReturnY",          ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    5,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetScreenWarpReturnY",          ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              0,                                    4,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHighestStringID",            ZVARTYPEID_FLOAT,         GETTER,       GAMENUMMESSAGES,      1,              0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getNumMessages",                ZVARTYPEID_FLOAT,         GETTER,       GAMENUMMESSAGES,      1,              0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "setGameOverScreen[]",           ZVARTYPEID_VOID,          SETTER,       SETGAMEOVERELEMENT,   12,             0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setGameOverStrings[]",          ZVARTYPEID_VOID,          SETTER,       SETGAMEOVERSTRING,    3,              0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
//	{ "SetContinueScreen[]",           ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "SetContinueString[]",           ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
/*	
	{ "getJoypad[]",                   ZVARTYPEID_FLOAT,         GETTER,       JOYPADPRESS,          18,             0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPress[]",                    ZVARTYPEID_BOOL,          GETTER,       BUTTONPRESS,          18,             0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPress[]",                    ZVARTYPEID_VOID,          SETTER,       BUTTONPRESS,          18,             0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getButton[]",                   ZVARTYPEID_BOOL,          GETTER,       BUTTONINPUT,          18,             0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setButton[]",                   ZVARTYPEID_VOID,          SETTER,       BUTTONINPUT,          18,             0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHold[]",                     ZVARTYPEID_BOOL,          GETTER,       BUTTONHELD,           18,             0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHold[]",                     ZVARTYPEID_VOID,          SETTER,       BUTTONHELD,           18,             0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getReadKey[]",                  ZVARTYPEID_BOOL,          GETTER,       READKEY,              127,            0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getKey[]",                      ZVARTYPEID_BOOL,          GETTER,       RAWKEY,             127,            0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setKey[]",                      ZVARTYPEID_VOID,          SETTER,       RAWKEY,             127,            0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMouse[]",                    ZVARTYPEID_FLOAT,         GETTER,       MOUSEARR,             6,              0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMouse[]",                    ZVARTYPEID_VOID,          SETTER,       MOUSEARR,             6,              0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
*/	
//	New Datatypes
	{ "LoadNPCData",                   ZVARTYPEID_NPCDATA,       FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadComboData",                 ZVARTYPEID_COMBOS,        FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadMapData",                   ZVARTYPEID_MAPDATA,       FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadTempScreen",                ZVARTYPEID_MAPDATA,       FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadScrollingScreen",           ZVARTYPEID_MAPDATA,       FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadSpriteData",                ZVARTYPEID_SPRITEDATA,    FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadShopData",                  ZVARTYPEID_SHOPDATA,      FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadInfoShopData",              ZVARTYPEID_SHOPDATA,      FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "LoadScreenData",                ZVARTYPEID_ITEMCLASS,     FUNCTION,     0,                    1,              0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadBitmapID",                  ZVARTYPEID_BITMAP,        FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadMessageData",               ZVARTYPEID_ZMESSAGE,      FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadDMapData",                  ZVARTYPEID_DMAPDATA,      FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadDropset",                   ZVARTYPEID_DROPSET,       FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadRNG",                       ZVARTYPEID_RNG,           FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadStack",                     ZVARTYPEID_STACK,         FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadBottleData",                ZVARTYPEID_BOTTLETYPE,    FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadBottleShopData",            ZVARTYPEID_BOTTLESHOP,    FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadGenericData",               ZVARTYPEID_GENERICDATA,   FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "CreateBitmap",                  ZVARTYPEID_BITMAP,        FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "PlayOgg",                       ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetOggPos",                     ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetOggPos",                     ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetOggSpeed",                   ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "AllocateBitmap",                ZVARTYPEID_BITMAP,        GETTER,       ALLOCATEBITMAPR,      1,              0,                                    1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getGravity[]",                  ZVARTYPEID_FLOAT,         GETTER,       GAMEGRAVITY,          3,              0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setGravity[]",                  ZVARTYPEID_VOID,          SETTER,       GAMEGRAVITY,          3,              0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScrolling[]",                ZVARTYPEID_FLOAT,         GETTER,       GAMESCROLLING,        5,              0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Reload",                        ZVARTYPEID_VOID,          FUNCTION,     0,                    1,              FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_GAME, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMiscSprites[]",              ZVARTYPEID_FLOAT,         GETTER,       GAMEMISCSPR,          256,            0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMiscSprites[]",              ZVARTYPEID_VOID,          SETTER,       GAMEMISCSPR,          256,            0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMiscSFX[]",                  ZVARTYPEID_FLOAT,         GETTER,       GAMEMISCSFX,          256,            0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMiscSFX[]",                  ZVARTYPEID_VOID,          SETTER,       GAMEMISCSFX,          256,            0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getEventData[]",                ZVARTYPEID_UNTYPED,       GETTER,       GAMEEVENTDATA,        214748,         0,                                    2,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setEventData[]",                ZVARTYPEID_VOID,          SETTER,       GAMEEVENTDATA,        214748,         0,                                    3,           { ZVARTYPEID_GAME, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "",                              -1,                       -1,           -1,                   -1,             0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

GameSymbols::GameSymbols()
{
    table = gameTable;
    refVar = NUL;
}

void GameSymbols::generateCode()
{
    //itemclass LoadItemData(game, int32_t)
    {
	    Function* function = getFunction("LoadItemData", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OLoadItemDataRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(REFITEMCLASS)));
        RETURN();
        function->giveCode(code);
    }
    //NPCData
    {
	    Function* function = getFunction("LoadNPCData", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OLoadNPCDataRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(REFNPCCLASS)));
        RETURN();
        function->giveCode(code);
    }
    
    //DMapdata
    {
	    Function* function = getFunction("LoadDMapData", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OLoadDMapDataRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(REFDMAPDATA)));
        RETURN();
        function->giveCode(code);
    }
    
    //Dropset
    {
	    Function* function = getFunction("LoadDropset", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OLoadDropsetRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(REFDROPS)));
        RETURN();
        function->giveCode(code);
    }
    
    //randgen LoadRNG(Game)
    {
	    Function* function = getFunction("LoadRNG", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop pointer
        POPREF();
        addOpcode2 (code, new OLoadRNG());
        LABELBACK(label);
        RETURN();
        function->giveCode(code);
    }
	
	//stack LoadStack(Game)
    {
	    Function* function = getFunction("LoadStack", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop pointer
        POPREF();
        addOpcode2 (code, new OLoadStack());
        LABELBACK(label);
        RETURN();
        function->giveCode(code);
    }
    //BottleData
    {
	    Function* function = getFunction("LoadBottleData", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OLoadBottleTypeRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(REFBOTTLETYPE)));
        RETURN();
        function->giveCode(code);
    }
	//BottleShopData
    {
	    Function* function = getFunction("LoadBottleShopData", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OLoadBShopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(REFBOTTLESHOP)));
        RETURN();
        function->giveCode(code);
    }
	//GenericData
    {
	    Function* function = getFunction("LoadGenericData", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OLoadGenericDataR(new VarArgument(EXP1)));
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(REFGENERICDATA)));
        RETURN();
        function->giveCode(code);
    }
	
	//Messagedata
    {
	    Function* function = getFunction("LoadMessageData", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OLoadMessageDataRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(REFMSGDATA)));
        RETURN();
        function->giveCode(code);
    }
    //ComboData
    {
	    Function* function = getFunction("LoadComboData", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OLoadComboDataRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(REFCOMBODATA)));
        RETURN();
        function->giveCode(code); 
    }
    //MapData
    /*
    { //LoadMapData(int32_t map, int32_t screen)
	Function* function = getFunction("LoadMapData", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
	addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OLoadMapDataRegister(new VarArgument(EXP1), new VarArgument(INDEX)));
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(REFMAPDATA)));
        RETURN();
        function->giveCode(code);     
	//LOAD_REFDATA("LoadMapData", OLoadMapDataRegister, REFMAPDATA);
    }
    */
    
    //int32_t LoadMapData(mapdata, int32_t map,int32_t scr)
    {
        Function* function = getFunction("LoadMapData", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(LOADMAPDATA)));
        RETURN();
        function->giveCode(code);
    }
    
    //int32_t LoadTempScreen(game, int32_t layer)
    {
        Function* function = getFunction("LoadTempScreen", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OLoadTmpScr(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    
    //int32_t LoadScrollingScreen(game, int32_t layer)
    {
        Function* function = getFunction("LoadScrollingScreen", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OLoadScrollScr(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
	
    //int32_t Create(bitmap, int32_t map,int32_t scr)
	{
		Function* function = getFunction("CreateBitmap", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(CREATEBITMAP)));
		RETURN();
		function->giveCode(code);
	}
   
    //SpriteData
    {
	    
	Function* function = getFunction("LoadSpriteData", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OLoadSpriteDataRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(REFSPRITEDATA)));
        RETURN();
        function->giveCode(code);    
    }
    //ShopData
    {
	    Function* function = getFunction("LoadShopData", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OLoadShopDataRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(REFSHOPDATA)));
        RETURN();
        function->giveCode(code);    
    }
    //InfoShopData
    {
	    Function* function = getFunction("LoadInfoShopData", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OLoadInfoShopDataRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(REFSHOPDATA)));
        RETURN();
        function->giveCode(code);    
    }
    //ScreenData
    /*
    {
	LOAD_REFDATA("LoadScreenData", OLoadScreenDataRegister, NUL); //Change when we set this up! -Z
    }
    */
    //Bitmap
    {
	    Function* function = getFunction("LoadBitmapID", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OLoadBitmapDataRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(REFBITMAP)));
        RETURN();
        function->giveCode(code);   
    }
    
    //bool GetScreenState(game, int32_t,int32_t,int32_t)
    {
        Function* function = getFunction("GetScreenState", 4);
        int32_t label = function->getLabel();
        int32_t done = ScriptParser::getUniqueLabelID();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OSubImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
        addOpcode2 (code, new OMultImmediate(new VarArgument(EXP1), new LiteralArgument(1360000)));
        addOpcode2 (code, new OAddRegister(new VarArgument(INDEX), new VarArgument(EXP1)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(SCREENSTATEDD)));
        addOpcode2 (code, new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
        addOpcode2 (code, new OGotoTrueImmediate(new LabelArgument(done)));
        addOpcode2 (code, new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
        addOpcode2 (code, new OGotoImmediate(new LabelArgument(done)));
        addOpcode2 (code, new OReturn());
        LABELBACK(done);
        function->giveCode(code);
    }
    //void SetScreenState(game, int32_t,int32_t,int32_t,bool)
    {
        Function* function = getFunction("SetScreenState", 5);
        int32_t label = function->getLabel();
        int32_t done = ScriptParser::getUniqueLabelID();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OSubImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
        addOpcode2 (code, new OMultImmediate(new VarArgument(EXP1), new LiteralArgument(1360000)));
        addOpcode2 (code, new OAddRegister(new VarArgument(INDEX), new VarArgument(EXP1)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OCompareImmediate(new VarArgument(SFTEMP), new LiteralArgument(0)));
        addOpcode2 (code, new OGotoTrueImmediate(new LabelArgument(done)));
        addOpcode2 (code, new OSetImmediate(new VarArgument(SFTEMP), new LiteralArgument(10000)));
        addOpcode2 (code, new OSetRegister(new VarArgument(SCREENSTATEDD), new VarArgument(SFTEMP)));
        LABELBACK(done);
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetScreenD(game, int32_t,int32_t)
    {
        Function* function = getFunction("GetScreenD", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(SDDD)));
        RETURN();
        function->giveCode(code);
    }
    //void SetScreenD(game, int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetScreenD", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(SDDD), new VarArgument(SFTEMP)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetDMapScreenD(game, int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("GetDMapScreenD", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(SDDDD)));
        RETURN();
        function->giveCode(code);
    }
    //void SetDMapScreenD(game, int32_t,int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetDMapScreenD", 5);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(SDDDD), new VarArgument(SFTEMP)));
        RETURN();
        function->giveCode(code);
    }
    //void PlaySound(game, int32_t)
    {
	    Function* function = getFunction("PlaySound", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OPlaySoundRegister(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //void PlayMIDI(game, int32_t)
    {
	    Function* function = getFunction("PlayMIDI", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OPlayMIDIRegister(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //void PlayEnhancedMusic(game, int32_t, int32_t)
    {
	    Function* function = getFunction("PlayEnhancedMusic", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OPlayEnhancedMusic(new VarArgument(EXP2), new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //void GetDMapMusicFilename(game, int32_t, int32_t)
    {
	    Function* function = getFunction("GetDMapMusicFilename", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGetDMapMusicFilename(new VarArgument(EXP2), new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetDMapMusicTrack(game, int32_t)
    {
	    Function* function = getFunction("GetDMapMusicTrack", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGetDMapMusicTrack(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //void SetDMapEnhancedMusic(game, int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetDMapEnhancedMusic", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OSetDMapEnhancedMusic());
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        //pop pointer, and ignore it
        POPREF();
        
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetComboData(int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("GetComboData", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(COMBODDM)));
        RETURN();
        function->giveCode(code);
    }
    //void SetComboData(int32_t,int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetComboData", 5);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(COMBODDM), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetComboCSet(int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("GetComboCSet", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(COMBOCDM)));
        RETURN();
        function->giveCode(code);
    }
    //void SetComboCSet(int32_t,int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetComboCSet", 5);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(COMBOCDM), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetComboFlag(int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("GetComboFlag", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(COMBOFDM)));
        RETURN();
        function->giveCode(code);
    }
    //void SetComboFlag(int32_t,int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetComboFlag", 5);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(COMBOFDM), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetComboType(int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("GetComboType", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(COMBOTDM)));
        RETURN();
        function->giveCode(code);
    }
    //void SetComboType(int32_t,int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetComboType", 5);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(COMBOTDM), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetComboInherentFlag(int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("GetComboInherentFlag", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(COMBOIDM)));
        RETURN();
        function->giveCode(code);
    }
    //void SetComboInherentFlag(int32_t,int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetComboInherentFlag", 5);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(COMBOIDM), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetComboCollision(int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("GetComboSolid", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(COMBOSDM)));
        RETURN();
        function->giveCode(code);
    }
    //void SetComboCollision(int32_t,int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetComboSolid", 5);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(COMBOSDM), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetScreenFlags(game,int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("GetScreenFlags", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGetScreenFlags(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetScreenEFlags(game,int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("GetScreenEFlags", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGetScreenEFlags(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //void Save(game)
    {
	    Function* function = getFunction("Save", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop pointer, and ignore it
		ASSERT_NUL();
        addOpcode2 (code, new OSave());
        LABELBACK(label);
        RETURN();
        function->giveCode(code);
    }
    //void End(game)
    {
	    Function* function = getFunction("End", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop pointer, and ignore it
		ASSERT_NUL();
        addOpcode2 (code, new OEnd());
        LABELBACK(label);
        RETURN();
        function->giveCode(code);
    }
    
    //void Continue(game)
    {
	    Function* function = getFunction("Continue", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop pointer, and ignore it
		ASSERT_NUL();
        addOpcode2 (code, new OGameContinue());
        LABELBACK(label);
        RETURN();
        function->giveCode(code);
    }
    
    //void SaveAndQuit(game)
    {
	    Function* function = getFunction("SaveAndQuit", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop pointer, and ignore it
        addOpcode2 (code, new OPopRegister(new VarArgument(refVar)));
        LABELBACK(label);
        addOpcode2 (code, new OGameSaveQuit());
        RETURN();
        function->giveCode(code);
    }
    
    //void SaveAndContinue(game)
    {
	    Function* function = getFunction("SaveAndContinue", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop pointer, and ignore it
        addOpcode2 (code, new OPopRegister(new VarArgument(refVar)));
        LABELBACK(label);
        addOpcode2 (code, new OGameSaveContinue());
        RETURN();
        function->giveCode(code);
    }
    //void ShowContinueScreen(game)
    {
	    Function* function = getFunction("ShowContinueScreen", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop pointer, and ignore it
		ASSERT_NUL();
        addOpcode2 (code, new OShowF6Screen());
        LABELBACK(label);
        RETURN();
        function->giveCode(code);
    }
    //int32_t ComboTile(game,int32_t)
    {
	    Function* function = getFunction("ComboTile", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OComboTile(new VarArgument(EXP1),new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //void GetSaveName(game, int32_t)
    {
	    Function* function = getFunction("GetSaveName", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGetSaveName(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //void GetSaveName(game, int32_t)
    {
	    Function* function = getFunction("SetSaveName", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetSaveName(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //GetMessage(game, int32_t, int32_t)
    {
	    Function* function = getFunction("GetMessage", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        //pop pointer, and ignore it
        addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
        addOpcode2 (code, new OGetMessage(new VarArgument(EXP2), new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //GetDMapName(game, int32_t, int32_t)
    {
	    Function* function = getFunction("GetDMapName", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        //pop pointer, and ignore it
        addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
        addOpcode2 (code, new OGetDMapName(new VarArgument(EXP2), new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //GetDMapTitle(game, int32_t, int32_t)
    {
	    Function* function = getFunction("GetDMapTitle", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        //pop pointer, and ignore it
        addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
        addOpcode2 (code, new OGetDMapTitle(new VarArgument(EXP2), new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //GetDMapIntro(game, int32_t, int32_t)
    {
	    Function* function = getFunction("GetDMapIntro", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        //pop pointer, and ignore it
        addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
        addOpcode2 (code, new OGetDMapIntro(new VarArgument(EXP2), new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    
    
    
    //void GreyscaleOn(game)
    {
	    Function* function = getFunction("GreyscaleOn", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop pointer, and ignore it
		ASSERT_NUL();
        addOpcode2 (code, new OGreyscaleOn());
        LABELBACK(label);
        RETURN();
        function->giveCode(code);
    }
            
	//void GreyscaleOff(game)
    {
	    Function* function = getFunction("GreyscaleOff", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop pointer, and ignore it
		ASSERT_NUL();
        addOpcode2 (code, new OGreyscaleOff());
        LABELBACK(label);
        RETURN();
        function->giveCode(code);
    }
    
    
   // SetMessage(game, int32_t, int32_t)
    {
	    Function* function = getFunction("SetMessage", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        //pop pointer, and ignore it
        addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
        addOpcode2 (code, new OSetMessage(new VarArgument(EXP2), new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //SetDMapName(game, int32_t, int32_t)
    {
	    Function* function = getFunction("SetDMapName", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        //pop pointer, and ignore it
        addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
        addOpcode2 (code, new OSetDMapName(new VarArgument(EXP2), new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //SetDMapTitle(game, int32_t, int32_t)
    {
	    Function* function = getFunction("SetDMapTitle", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        //pop pointer, and ignore it
        addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
        addOpcode2 (code, new OSetDMapTitle(new VarArgument(EXP2), new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //SetDMapIntro(game, int32_t, int32_t)
    {
	    Function* function = getFunction("SetDMapIntro", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        //pop pointer, and ignore it
        addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
        addOpcode2 (code, new OSetDMapIntro(new VarArgument(EXP2), new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    
    //bool ShowSaveScreen(game)
    {
	    Function* function = getFunction("ShowSaveScreen", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop pointer, and ignore it
		ASSERT_NUL();
        addOpcode2 (code, new OShowSaveScreen(new VarArgument(EXP1)));
        LABELBACK(label);
        RETURN();
        function->giveCode(code);
    }
    
    //void ShowSaveQuitScreen(game)
    {
	    Function* function = getFunction("ShowSaveQuitScreen", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop pointer, and ignore it
		ASSERT_NUL();
        addOpcode2 (code, new OShowSaveQuitScreen());
        LABELBACK(label);
        RETURN();
        function->giveCode(code);
    }
    
    //int32_t GetFFCScript(game, int32_t)
    {
	    Function* function = getFunction("GetFFCScript", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGetFFCScript(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    
    //int32_t GetComboScript(game, int32_t)
    {
	    Function* function = getFunction("GetComboScript", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGetComboScript(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    
    
     //int32_t GetItemScript(game, int32_t)
    {
	    Function* function = getFunction("GetItemScript", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGetItemScript(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    
     //int32_t GetNPCScript(game, int32_t)
    {
	    Function* function = getFunction("GetNPCScript", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGETNPCSCRIPT(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetLWeaponScript(game, int32_t)
    {
	    Function* function = getFunction("GetLWeaponScript", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGETLWEAPONSCRIPT(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetEWeaponScript(game, int32_t)
    {
	    Function* function = getFunction("GetEWeaponScript", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGETEWEAPONSCRIPT(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetGenericScript(game, int32_t)
    {
	    Function* function = getFunction("GetGenericScript", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGETGENERICSCRIPT(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetHeroScript(game, int32_t)
    {
	    Function* function = getFunction("GetHeroScript", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGETHEROSCRIPT(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetPlayerScript(game, int32_t)
    {
	    Function* function = getFunction("GetPlayerScript", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGETHEROSCRIPT(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetLinkScript(game, int32_t)
    {
	    Function* function = getFunction("GetLinkScript", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGETHEROSCRIPT(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetGlobalScript(game, int32_t)
    {
	    Function* function = getFunction("GetGlobalScript", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGETGLOBALSCRIPT(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetDMapScript(game, int32_t)
    {
	    Function* function = getFunction("GetDMapScript", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGETDMAPSCRIPT(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetScreenScript(game, int32_t)
    {
	    Function* function = getFunction("GetScreenScript", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGETSCREENSCRIPT(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetItemSpriteScript(game, int32_t)
    {
	    Function* function = getFunction("GetItemSpriteScript", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGETSPRITESCRIPT(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetUntypedScript(game, int32_t)
    {
	    Function* function = getFunction("GetUntypedScript", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGETUNTYPEDSCRIPT(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetSubscreenScript(game, int32_t)
    {
	    Function* function = getFunction("GetSubscreenScript", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGETSUBSCREENSCRIPT(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetNPC(game, int32_t)
    {
	    Function* function = getFunction("GetNPC", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGETNPCBYNAME(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetItem(game, int32_t)
    {
	    Function* function = getFunction("GetItem", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGETITEMBYNAME(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetCombo(game, int32_t)
    {
	    Function* function = getFunction("GetCombo", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGETCOMBOBYNAME(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetDMap(game, int32_t)
    {
	    Function* function = getFunction("GetDMap", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGETDMAPBYNAME(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    
     //int32_t GetScreenEnemy(game,int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("GetScreenEnemy", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGetScreenEnemy(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
     //int32_t GetScreenDoor(game,int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("GetScreenDoor", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGetScreenDoor(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //void SetScreenEnemy(int32_t,int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetScreenEnemy", 5);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(SETSCREENENEMY), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //void SetScreenDoor(int32_t,int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetScreenDoor", 5);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(SETSCREENDOOR), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    
    //void SetScreenWidth(game, int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetScreenWidth", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(SCREENWIDTH), new VarArgument(SFTEMP)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetScreenWidth(game, int32_t,int32_t)
    {
	    Function* function = getFunction("GetScreenWidth", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(SCREENWIDTH)));
        RETURN();
        function->giveCode(code);
    }
    
    //void SetScreenHeight(game, int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetScreenHeight", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(SCREENHEIGHT), new VarArgument(SFTEMP)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetScreenHeight(game, int32_t,int32_t)
    {
	    Function* function = getFunction("GetScreenHeight", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(SCREENHEIGHT)));
        RETURN();
        function->giveCode(code);
    }
    //void SetScreenViewX(game, int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetScreenViewX", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(SCREENVIEWX), new VarArgument(SFTEMP)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetScreenViewX(game, int32_t,int32_t)
    {
	    Function* function = getFunction("GetScreenViewX", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(SCREENVIEWX)));
        RETURN();
        function->giveCode(code);
    }
     //void SetScreenViewY(game, int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetScreenViewY", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(SCREENVIEWY), new VarArgument(SFTEMP)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetScreenViewY(game, int32_t,int32_t)
    {
	    Function* function = getFunction("GetScreenViewY", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(SCREENVIEWY)));
        RETURN();
        function->giveCode(code);
    }
    //void SetScreenGuy(game, int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetScreenGuy", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(SCREENGUY), new VarArgument(SFTEMP)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetScreenGuy(game, int32_t,int32_t)
    {
	    Function* function = getFunction("GetScreenGuy", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(SCREENGUY)));
        RETURN();
        function->giveCode(code);
    }
    //void SetScreenString(game, int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetScreenString", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(SCREENSTRING), new VarArgument(SFTEMP)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetScreenString(game, int32_t,int32_t)
    {
	    Function* function = getFunction("GetScreenString", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(SCREENSTRING)));
        RETURN();
        function->giveCode(code);
    }
    //void SetScreenRoomType(game, int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetScreenRoomType", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(SCREENROOM), new VarArgument(SFTEMP)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetScreenRoomType(game, int32_t,int32_t)
    {
	    Function* function = getFunction("GetScreenRoomType", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(SCREENROOM)));
        RETURN();
        function->giveCode(code);
    }
    //void SetScreenEntryX(game, int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetScreenEntryX", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(SCREENENTX), new VarArgument(SFTEMP)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetScreenEntryX(game, int32_t,int32_t)
    {
	    Function* function = getFunction("GetScreenEntryX", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(SCREENENTX)));
        RETURN();
        function->giveCode(code);
    }
    //void SetScreenEntryY(game, int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetScreenEntryY", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(SCREENENTY), new VarArgument(SFTEMP)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetScreenEntryY(game, int32_t,int32_t)
    {
	    Function* function = getFunction("GetScreenEntryY", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(SCREENENTY)));
        RETURN();
        function->giveCode(code);
    }
     //void SetScreenItem(game, int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetScreenItem", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(SCREENITEM), new VarArgument(SFTEMP)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetScreenItem(game, int32_t,int32_t)
    {
	    Function* function = getFunction("GetScreenItem", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(SCREENITEM)));
        RETURN();
        function->giveCode(code);
    }
     //void SetScreenUndercombo(game, int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetScreenUndercombo", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(SCREENUNDCMB), new VarArgument(SFTEMP)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetScreenUndercombo(game, int32_t,int32_t)
    {
	    Function* function = getFunction("GetScreenUndercombo", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(SCREENUNDCMB)));
        RETURN();
        function->giveCode(code);
    }
    //void SetScreenUnderCSet(game, int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetScreenUnderCSet", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(SCREENUNDCST), new VarArgument(SFTEMP)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetScreenUnderCSet(game, int32_t,int32_t)
    {
	    Function* function = getFunction("GetScreenUnderCSet", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(SCREENUNDCST)));
        RETURN();
        function->giveCode(code);
    }
    //void SetScreenCatchall(game, int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetScreenCatchall", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(SCREENCATCH), new VarArgument(SFTEMP)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetScreenCatchall(game, int32_t,int32_t)
    {
	    Function* function = getFunction("GetScreenCatchall", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(SCREENCATCH)));
        RETURN();
        function->giveCode(code);
    }
    
    //void SetScreenLayerOpacity(int32_t,int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetScreenLayerOpacity", 5);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(SETSCREENLAYOP), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetScreenLayerOpacity(game,int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("GetScreenLayerOpacity", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGetScreenLayerOpacity(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }

     //void SetScreenSecretCombo(int32_t,int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetScreenSecretCombo", 5);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(SETSCREENSECCMB), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetScreenSecretCombo(game,int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("GetScreenSecretCombo", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGetScreenSecretCombo(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }

     //void SetScreenSecretCSet(int32_t,int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetScreenSecretCSet", 5);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(SETSCREENSECCST), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetScreenSecretCSet(game,int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("GetScreenSecretCSet", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGetScreenSecretCSet(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //void SetScreenSecretFlag(int32_t,int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetScreenSecretFlag", 5);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(SETSCREENSECFLG), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetScreenSecretFlag(game,int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("GetScreenSecretFlag", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGetScreenSecretFlag(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }

     //void SetScreenLayerMap(int32_t,int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetScreenLayerMap", 5);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(SETSCREENLAYMAP), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetScreenLayerMap(game,int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("GetScreenLayerMap", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGetScreenLayerMap(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }

    
    //void SetScreenLayerScreen(int32_t,int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetScreenLayerScreen", 5);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(SETSCREENLAYSCR), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetScreenLayerScreen(game,int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("GetScreenLayerScreen", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGetScreenLayerScreen(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }

    //void SetScreenPath(int32_t,int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetScreenPath", 5);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(SETSCREENPATH), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetScreenPath(game,int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("GetScreenPath", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGetScreenPath(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //void SetScreenWarpReturnX(int32_t,int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetScreenWarpReturnX", 5);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(SETSCREENWARPRX), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetScreenWarpReturnX(game,int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("GetScreenWarpReturnX", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGetScreenWarpReturnX(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //void SetScreenWarpReturnY(int32_t,int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("SetScreenWarpReturnY", 5);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(SETSCREENWARPRY), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    /*
    {
        TWO_INPUT_NO_RETURN("SetContinueScreen",OSSetContinueScreen);
    }
    */
    /*
    {
        TWO_INPUT_NO_RETURN("SetContinueString",OSSetContinueString);
    }
    */
    //int32_t GetScreenWarpReturnY(game,int32_t,int32_t,int32_t)
    {
	    Function* function = getFunction("GetScreenWarpReturnY", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGetScreenWarpReturnY(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //void PlayOgg(game, int32_t, int32_t)
    {
	    Function* function = getFunction("PlayOgg", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OPlayEnhancedMusicEx(new VarArgument(EXP2), new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetOggPos(game)
{
	    Function* function = getFunction("GetOggPos", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop pointer, and ignore it
		ASSERT_NUL();
        addOpcode2 (code, new OGetEnhancedMusicPos(new VarArgument(EXP1)));
        LABELBACK(label);
        RETURN();
        function->giveCode(code);
}
     //void SetOggPos(game, int32_t)
    {
	    Function* function = getFunction("SetOggPos", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetEnhancedMusicPos(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    
    //void SetOggSpeed(game, int32_t)
    {
	    Function* function = getFunction("SetOggSpeed", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetEnhancedMusicSpeed(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
	
	//void Reload(game)
    {
	    Function* function = getFunction("Reload", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop pointer, and ignore it
		ASSERT_NUL();
        addOpcode2 (code, new OGameReload());
        LABELBACK(label);
        RETURN();
        function->giveCode(code);
    }
}

