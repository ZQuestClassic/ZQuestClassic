#include "SymbolDefs.h"

GameSymbols GameSymbols::singleton = GameSymbols();

static AccessorTable gameTable[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
	{ "_getIncrementQuest",         0,         ZTID_FLOAT,   INCQST,                    0,  { ZTID_GAME },{} },
	{ "_getMaxNPCs",                0,         ZTID_FLOAT,   SPRITEMAXNPC,              0,  { ZTID_GAME },{} },
	{ "_setMaxNPCs",                0,          ZTID_VOID,   SPRITEMAXNPC,              0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "_getMaxLWeapons",            0,         ZTID_FLOAT,   SPRITEMAXLWPN,             0,  { ZTID_GAME },{} },
	{ "_setMaxLWeapons",            0,          ZTID_VOID,   SPRITEMAXLWPN,             0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "_getMaxEWeapons",            0,         ZTID_FLOAT,   SPRITEMAXEWPN,             0,  { ZTID_GAME },{} },
	{ "_setMaxEWeapons",            0,          ZTID_VOID,   SPRITEMAXEWPN,             0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "_getMaxItemsprites",         0,         ZTID_FLOAT,   SPRITEMAXITEM,             0,  { ZTID_GAME },{} },
	{ "_setMaxItemsprites",         0,          ZTID_VOID,   SPRITEMAXITEM,             0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "_getGetCurScreen",           0,         ZTID_FLOAT,   CURSCR,                    0,  { ZTID_GAME },{} },
	{ "_getGetCurDMapScreen",       0,         ZTID_FLOAT,   CURDSCR,                   0,  { ZTID_GAME },{} },
	{ "_getGetCurMap",              0,         ZTID_FLOAT,   CURMAP,                    0,  { ZTID_GAME },{} },
	{ "_getGetCurDMap",             0,         ZTID_FLOAT,   CURDMAP,                   0,  { ZTID_GAME },{} },
	{ "_getGetCurLevel",            0,         ZTID_FLOAT,   CURLEVEL,                  0,  { ZTID_GAME },{} },
	{ "getCurScreen",               0,         ZTID_FLOAT,   CURSCR,                    0,  { ZTID_GAME },{} },
	{ "getCurDMapScreen",           0,         ZTID_FLOAT,   CURDSCR,                   0,  { ZTID_GAME },{} },
	{ "getCurDMScreen",             0,         ZTID_FLOAT,   CURDSCR,                   0,  { ZTID_GAME },{} },
	{ "getCurMap",                  0,         ZTID_FLOAT,   CURMAP,                    0,  { ZTID_GAME },{} },
	{ "getCurDMap",                 0,         ZTID_FLOAT,   CURDMAP,                   0,  { ZTID_GAME },{} },
	{ "getCurLevel",                0,         ZTID_FLOAT,   CURLEVEL,                  0,  { ZTID_GAME },{} },
	{ "getNumDeaths",               0,         ZTID_FLOAT,   GAMEDEATHS,                0,  { ZTID_GAME },{} },
	{ "setNumDeaths",               0,          ZTID_VOID,   GAMEDEATHS,                0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "getCheat",                   0,         ZTID_FLOAT,   GAMECHEAT,                 0,  { ZTID_GAME },{} },
	{ "setCheat",                   0,          ZTID_VOID,   GAMECHEAT,                 0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "getMaxCheat",                0,         ZTID_FLOAT,   GAMEMAXCHEAT,              0,  { ZTID_GAME },{} },
	{ "setMaxCheat",                0,          ZTID_VOID,   GAMEMAXCHEAT,              0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "getSubscreenSpeed",          0,          ZTID_LONG,   ACTIVESSSPEED,             0,  { ZTID_GAME },{} },
	{ "setSubscreenSpeed",          0,          ZTID_VOID,   ACTIVESSSPEED,             0,  { ZTID_GAME, ZTID_LONG },{} },
	{ "getTime",                    0,          ZTID_LONG,   GAMETIME,                  0,  { ZTID_GAME },{} },
	{ "setTime",                    0,          ZTID_VOID,   GAMETIME,                  0,  { ZTID_GAME, ZTID_LONG },{} },
	{ "getHasPlayed",               0,          ZTID_BOOL,   GAMEHASPLAYED,             0,  { ZTID_GAME },{} },
	{ "setHasPlayed",               0,          ZTID_VOID,   GAMEHASPLAYED,             0,  { ZTID_GAME, ZTID_BOOL },{} },
	{ "getTypingMode",              0,          ZTID_BOOL,   TYPINGMODE,                0,  { ZTID_GAME },{} },
	{ "setTypingMode",              0,          ZTID_VOID,   TYPINGMODE,                0,  { ZTID_GAME, ZTID_BOOL },{} },
	{ "getTimeValid",               0,          ZTID_BOOL,   GAMETIMEVALID,             0,  { ZTID_GAME },{} },
	{ "setTimeValid",               0,          ZTID_VOID,   GAMETIMEVALID,             0,  { ZTID_GAME, ZTID_BOOL },{} },
	{ "getGuyCount[]",              0,         ZTID_FLOAT,   GAMEGUYCOUNT,              0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setGuyCount[]",              0,          ZTID_VOID,   GAMEGUYCOUNT,              0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getContinueScreen",          0,         ZTID_FLOAT,   GAMECONTSCR,               0,  { ZTID_GAME },{} },
	{ "setContinueScreen",          0,          ZTID_VOID,   GAMECONTSCR,               0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "getContinueDMap",            0,         ZTID_FLOAT,   GAMECONTDMAP,              0,  { ZTID_GAME },{} },
	{ "setContinueDMap",            0,          ZTID_VOID,   GAMECONTDMAP,              0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "getCounter[]",               0,         ZTID_FLOAT,   GAMECOUNTERD,              0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setCounter[]",               0,          ZTID_VOID,   GAMECOUNTERD,              0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getMCounter[]",              0,         ZTID_FLOAT,   GAMEMCOUNTERD,             0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setMCounter[]",              0,          ZTID_VOID,   GAMEMCOUNTERD,             0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getDCounter[]",              0,         ZTID_FLOAT,   GAMEDCOUNTERD,             0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setDCounter[]",              0,          ZTID_VOID,   GAMEDCOUNTERD,             0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getGeneric[]",               0,       ZTID_UNTYPED,   GAMEGENERICD,              0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setGeneric[]",               0,          ZTID_VOID,   GAMEGENERICD,              0,  { ZTID_GAME, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getMisc[]",                  0,       ZTID_UNTYPED,   GAMEMISC,                  0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setMisc[]",                  0,          ZTID_VOID,   GAMEMISC,                  0,  { ZTID_GAME, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getLItems[]",                0,         ZTID_FLOAT,   GAMELITEMSD,               0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setLItems[]",                0,          ZTID_VOID,   GAMELITEMSD,               0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getLKeys[]",                 0,         ZTID_FLOAT,   GAMELKEYSD,                0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setLKeys[]",                 0,          ZTID_VOID,   GAMELKEYSD,                0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getLSwitches[]",             0,          ZTID_LONG,   GAMELSWITCH,               0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setLSwitches[]",             0,          ZTID_VOID,   GAMELSWITCH,               0,  { ZTID_GAME, ZTID_FLOAT, ZTID_LONG },{} },
	{ "getGSwitch[]",               0,         ZTID_FLOAT,   GAMEGSWITCH,               0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setGSwitch[]",               0,          ZTID_VOID,   GAMEGSWITCH,               0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getBottleState[]",           0,         ZTID_FLOAT,   GAMEBOTTLEST,              0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setBottleState[]",           0,          ZTID_VOID,   GAMEBOTTLEST,              0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetScreenD",                 0,       ZTID_UNTYPED,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetScreenD",                 0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "GetDMapScreenD",             0,       ZTID_UNTYPED,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetDMapScreenD",             0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "LoadItemData",               0,     ZTID_ITEMCLASS,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "_getGetMIDI",                0,         ZTID_FLOAT,   GETMIDI,                   0,  { ZTID_GAME },{} },
	{ "Save",                       0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GAME },{} },
	{ "End",                        0,          ZTID_VOID,   -1,           FL_EXIT|FL_INL,  { ZTID_GAME },{} },
	{ "Reload",                     0,          ZTID_VOID,   -1,           FL_EXIT|FL_INL,  { ZTID_GAME },{} },
	{ "Continue",                   0,          ZTID_VOID,   -1,           FL_EXIT|FL_INL,  { ZTID_GAME },{} },
	{ "SaveAndQuit",                0,          ZTID_VOID,   -1,           FL_EXIT|FL_INL,  { ZTID_GAME },{} },
	{ "SaveAndContinue",            0,          ZTID_VOID,   -1,           FL_EXIT|FL_INL,  { ZTID_GAME },{} },
	{ "ShowContinueScreen",         0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GAME },{} },
	{ "GetSaveName",                0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "SetSaveName",                0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "getStandalone",              0,          ZTID_BOOL,   GAMESTANDALONE,            0,  { ZTID_GAME },{} },
	{ "setStandalone",              0,          ZTID_VOID,   GAMESTANDALONE,            0,  { ZTID_GAME, ZTID_BOOL },{} },
	{ "ShowSaveScreen",             0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_GAME },{} },
	{ "ShowSaveQuitScreen",         0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GAME },{} },
	{ "getLastEntranceScreen",      0,         ZTID_FLOAT,   GAMEENTRSCR,               0,  { ZTID_GAME },{} },
	{ "setLastEntranceScreen",      0,          ZTID_VOID,   GAMEENTRSCR,               0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "getLastEntranceDMap",        0,         ZTID_FLOAT,   GAMEENTRDMAP,              0,  { ZTID_GAME },{} },
	{ "setLastEntranceDMap",        0,          ZTID_VOID,   GAMEENTRDMAP,              0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "getClickToFreezeEnabled",    0,          ZTID_BOOL,   GAMECLICKFREEZE,           0,  { ZTID_GAME },{} },
	{ "setClickToFreezeEnabled",    0,          ZTID_VOID,   GAMECLICKFREEZE,           0,  { ZTID_GAME, ZTID_BOOL },{} },
	{ "GetFFCScript",               0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetComboScript",             0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
//	Get an item script ID, similar to GetFFCScript()
	{ "GetItemScript",              0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetNPCScript",               0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetLWeaponScript",           0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetEWeaponScript",           0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetPlayerScript",            0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetGlobalScript",            0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetDMapScript",              0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetScreenScript",            0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetItemSpriteScript",        0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetNPC",                     0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetItem",                    0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetCombo",                   0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetDMap",                    0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetGenericScript",           0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT },{} },
//	Monochrome mode
	{ "getFFRules[]",               0,          ZTID_BOOL,   FFRULE,                    0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setFFRules[]",               0,          ZTID_VOID,   FFRULE,                    0,  { ZTID_GAME, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getSkipCredits",             0,          ZTID_BOOL,   SKIPCREDITS,               0,  { ZTID_GAME },{} },
	{ "setSkipCredits",             0,          ZTID_VOID,   SKIPCREDITS,               0,  { ZTID_GAME, ZTID_BOOL },{} },
	{ "getSkipF6",                  0,          ZTID_BOOL,   SKIPF6,                    0,  { ZTID_GAME },{} },
	{ "setSkipF6",                  0,          ZTID_VOID,   SKIPF6,                    0,  { ZTID_GAME, ZTID_BOOL },{} },
	{ "getSuspend[]",               0,          ZTID_BOOL,   GAMESUSPEND,               0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setSuspend[]",               0,          ZTID_VOID,   GAMESUSPEND,               0,  { ZTID_GAME, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getDisableItem[]",           0,          ZTID_BOOL,   DISABLEDITEM,              0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setDisableItem[]",           0,          ZTID_VOID,   DISABLEDITEM,              0,  { ZTID_GAME, ZTID_FLOAT, ZTID_BOOL },{} },
//	Get the map count 
	{ "_getMapCount",               0,         ZTID_FLOAT,   GAMEMAXMAPS,               0,  { ZTID_GAME },{} },
//	Versioning
	{ "getZScriptVersion",          0,         ZTID_FLOAT,   ZSCRIPTVERSION,            0,  { ZTID_GAME },{} },
	{ "getVersion",                 0,         ZTID_FLOAT,   ZELDAVERSION,              0,  { ZTID_GAME },{} },
	{ "getBuild",                   0,         ZTID_FLOAT,   ZELDABUILD,                FL_DEPR,  { ZTID_GAME },{} },
	{ "getBeta",                    0,         ZTID_FLOAT,   ZELDABETA,                 0,  { ZTID_GAME },{} },
	{ "getBetaType",                0,         ZTID_FLOAT,   ZELDABETATYPE,             0,  { ZTID_GAME },{} },
	{ "getDisableActiveSubscreen",  0,          ZTID_BOOL,   NOACTIVESUBSC,             0,  { ZTID_GAME },{} },
	{ "setDisableActiveSubscreen",  0,          ZTID_VOID,   NOACTIVESUBSC,             0,  { ZTID_GAME, ZTID_BOOL },{} },
	{ "getNumMessages",             0,         ZTID_FLOAT,   GAMENUMMESSAGES,           0,  { ZTID_GAME },{} },
	
//	New Datatypes
	{ "LoadNPCData",                0,       ZTID_NPCDATA,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "LoadComboData",              0,        ZTID_COMBOS,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "LoadMapData",                0,       ZTID_MAPDATA,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "LoadTempScreen",             0,       ZTID_MAPDATA,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "LoadScrollingScreen",        0,       ZTID_MAPDATA,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "LoadSpriteData",             0,    ZTID_SPRITEDATA,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "LoadShopData",               0,      ZTID_SHOPDATA,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "LoadInfoShopData",           0,      ZTID_SHOPDATA,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "LoadBitmapID",               0,        ZTID_BITMAP,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "LoadMessageData",            0,      ZTID_ZMESSAGE,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "LoadDMapData",               0,      ZTID_DMAPDATA,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "LoadDropset",                0,       ZTID_DROPSET,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "LoadRNG",                    0,           ZTID_RNG,   -1,                   FL_INL,  { ZTID_GAME },{} },
	{ "LoadStack",                  0,         ZTID_STACK,   -1,                   FL_INL,  { ZTID_GAME },{} },
	{ "LoadBottleData",             0,    ZTID_BOTTLETYPE,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "LoadBottleShopData",         0,    ZTID_BOTTLESHOP,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "LoadGenericData",            0,   ZTID_GENERICDATA,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "CreateBitmap",               0,        ZTID_BITMAP,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{ 2560000, 2560000 } },
	{ "_getAllocateBitmap",         0,        ZTID_BITMAP,   ALLOCATEBITMAPR,           0,  { ZTID_GAME },{} },
	{ "getGravity[]",               0,         ZTID_FLOAT,   GAMEGRAVITY,               0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setGravity[]",               0,          ZTID_VOID,   GAMEGRAVITY,               0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getScrolling[]",             0,         ZTID_FLOAT,   GAMESCROLLING,             0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "getMiscSprites[]",           0,         ZTID_FLOAT,   GAMEMISCSPR,               0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setMiscSprites[]",           0,          ZTID_VOID,   GAMEMISCSPR,               0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getMiscSFX[]",               0,         ZTID_FLOAT,   GAMEMISCSFX,               0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setMiscSFX[]",               0,          ZTID_VOID,   GAMEMISCSFX,               0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getEventData[]",             0,       ZTID_UNTYPED,   GAMEEVENTDATA,             0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setEventData[]",             0,          ZTID_VOID,   GAMEEVENTDATA,             0,  { ZTID_GAME, ZTID_FLOAT, ZTID_UNTYPED },{} },
	
	{ "getMisc2[]",                 0,       ZTID_UNTYPED,   GHOSTARR,                  0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setMisc2[]",                 0,          ZTID_VOID,   GHOSTARR,                  0,  { ZTID_GAME, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getMisc3[]",                 0,       ZTID_UNTYPED,   TANGOARR,                  0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setMisc3[]",                 0,          ZTID_VOID,   TANGOARR,                  0,  { ZTID_GAME, ZTID_FLOAT, ZTID_UNTYPED },{} },
	
	{ "getMouseCursor",             0,         ZTID_FLOAT,   GAMEMOUSECURSOR,           0,  { ZTID_GAME },{} },
	{ "setMouseCursor",             0,          ZTID_VOID,   GAMEMOUSECURSOR,           0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "SetCustomCursor",            0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL, ZTID_BOOL },{ 0, 0 } },
	
	{ "_getNumSavedPortals",        0,         ZTID_FLOAT,   SAVEDPORTALCOUNT,          0,  { ZTID_GAME },{} },
	{ "LoadSavedPortal",            0,     ZTID_SAVPORTAL,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "CreateSavedPortal",          0,     ZTID_SAVPORTAL,   -1,                   FL_INL,  { ZTID_GAME },{} },
	{ "constSavedPortals",          0,     ZTID_SAVPORTAL,   INTARR_SAVPRTL*10000, FL_ARR,  { ZTID_GAME },{} },
	
	{ "getTrigGroups[]",            0,         ZTID_FLOAT,   GAMETRIGGROUPS,            0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setTrigGroups[]",            0,          ZTID_VOID,   GAMETRIGGROUPS,            0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getTriggerGroups[]",         0,         ZTID_FLOAT,   GAMETRIGGROUPS,            0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setTriggerGroups[]",         0,          ZTID_VOID,   GAMETRIGGROUPS,            0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getOverrideItems[]",         0,         ZTID_FLOAT,   GAMEOVERRIDEITEMS,         0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setOverrideItems[]",         0,          ZTID_VOID,   GAMEOVERRIDEITEMS,         0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "CurrentItemID",              0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{ 0x01 } },
	
	{ "getActiveSubscreenOpen",     0,          ZTID_BOOL,   GAMEASUBOPEN,         FL_INL,  { ZTID_GAME },{} },
	{ "setActiveSubscreenOpen",     0,          ZTID_VOID,   GAMEASUBOPEN,      FL_RDONLY,  { ZTID_GAME, ZTID_BOOL },{} },
	{ "getActiveSubscreenY",        0,         ZTID_FLOAT,   GAMEASUBYOFF,         FL_INL,  { ZTID_GAME },{} },
	{ "setActiveSubscreenY",        0,          ZTID_VOID,   GAMEASUBYOFF,      FL_RDONLY,  { ZTID_GAME, ZTID_BOOL },{} },
	{ "LoadASubData",               0, ZTID_SUBSCREENDATA,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "LoadPSubData",               0, ZTID_SUBSCREENDATA,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "LoadOSubData",               0, ZTID_SUBSCREENDATA,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "SwapActiveSubscreens",       0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SwapPassiveSubscreens",      0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SwapOverlaySubscreens",      0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getNumActiveSubscreens",     0,         ZTID_FLOAT,   GAMENUMASUB,          FL_INL,  { ZTID_GAME },{} },
	{ "setNumActiveSubscreens",     0,          ZTID_VOID,   GAMENUMASUB,          FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "getNumPassiveSubscreens",    0,         ZTID_FLOAT,   GAMENUMPSUB,          FL_INL,  { ZTID_GAME },{} },
	{ "setNumPassiveSubscreens",    0,          ZTID_VOID,   GAMENUMPSUB,          FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "getNumOverlaySubscreens",    0,         ZTID_FLOAT,   GAMENUMOSUB,          FL_INL,  { ZTID_GAME },{} },
	{ "setNumOverlaySubscreens",    0,          ZTID_VOID,   GAMENUMOSUB,          FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	
	//Intentionally undocumented
	{ "getSTD[]",                   0,       ZTID_UNTYPED,   STDARR,                    0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setSTD[]",                   0,          ZTID_VOID,   STDARR,                    0,  { ZTID_GAME, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "GetHeroScript",              0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetLinkScript",              0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "getGhost[]",                 0,       ZTID_UNTYPED,   GHOSTARR,                  0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setGhost[]",                 0,          ZTID_VOID,   GHOSTARR,                  0,  { ZTID_GAME, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getTango[]",                 0,       ZTID_UNTYPED,   TANGOARR,                  0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setTango[]",                 0,          ZTID_VOID,   TANGOARR,                  0,  { ZTID_GAME, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "setGameOverScreen[]",        0,          ZTID_VOID,   SETGAMEOVERELEMENT,        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "setGameOverStrings[]",       0,          ZTID_VOID,   SETGAMEOVERSTRING,         0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetMessage",                 0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetMessage",                 0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getHighestStringID",         0,         ZTID_FLOAT,   GAMENUMMESSAGES,           0,  { ZTID_GAME },{} },
	{ "GreyscaleOn",                0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GAME },{} },
	{ "GreyscaleOff",               0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GAME },{} },
	{ "ComboTile",                  0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	
	{ "PlaySound",                  0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT },{},0,"Use 'Audio->' instead!" },
	{ "PlayMIDI",                   0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT },{},0,"Use 'Audio->' instead!" },
	{ "PlayEnhancedMusic",          0,          ZTID_BOOL,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'Audio->' instead!" },
	{ "PlayOgg",                    0,          ZTID_BOOL,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetOggPos",                  0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME },{} },
	{ "SetOggPos",                  0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "SetOggSpeed",                0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT },{} },
	
	{ "getReadKey[]",               0,          ZTID_BOOL,   READKEY,             FL_DEPR,  { ZTID_GAME, ZTID_FLOAT },{},0,"Use 'Input->KeyPress[]' instead!" },
	{ "GetScreenEnemy",             0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "SetScreenEnemy",             0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "GetScreenDoor",              0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "SetScreenDoor",              0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	
	{ "SetScreenWidth",             0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetScreenWidth",             0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetScreenHeight",            0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetScreenHeight",            0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetScreenViewX",             0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetScreenViewX",             0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetScreenViewY",             0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetScreenViewY",             0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetScreenGuy",               0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "GetScreenGuy",               0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "SetScreenString",            0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "GetScreenString",            0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "SetScreenRoomType",          0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "GetScreenRoomType",          0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "SetScreenEntryX",            0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "GetScreenEntryX",            0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	
	{ "SetScreenEntryY",            0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "GetScreenEntryY",            0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "SetScreenItem",              0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "GetScreenItem",              0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "SetScreenUndercombo",        0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "GetScreenUndercombo",        0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "SetScreenUnderCSet",         0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "GetScreenUnderCSet",         0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "SetScreenCatchall",          0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "GetScreenCatchall",          0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "SetScreenLayerOpacity",      0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "GetScreenLayerOpacity",      0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "SetScreenSecretCombo",       0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "GetScreenSecretCombo",       0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "SetScreenSecretCSet",        0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "GetScreenSecretCSet",        0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "SetScreenSecretFlag",        0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "GetScreenSecretFlag",        0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "SetScreenLayerMap",          0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "GetScreenLayerMap",          0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "SetScreenLayerScreen",       0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "GetScreenLayerScreen",       0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "SetScreenPath",              0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "GetScreenPath",              0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "SetScreenWarpReturnX",       0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "GetScreenWarpReturnX",       0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "SetScreenWarpReturnY",       0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "GetScreenWarpReturnY",       0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "GetScreenFlags",             0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "GetScreenEFlags",            0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "GetScreenState",             0,          ZTID_BOOL,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->' vars instead!" },
	{ "SetScreenState",             0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL },{},0,"Use 'mapdata->' vars instead!" },
	
	{ "GetComboData",               0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->ComboD' instead!" },
	{ "SetComboData",               0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->ComboD'  instead!" },
	{ "GetComboCSet",               0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->ComboC'  instead!" },
	{ "SetComboCSet",               0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->ComboC'  instead!" },
	{ "GetComboFlag",               0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->ComboF'  instead!" },
	{ "SetComboFlag",               0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->ComboF'  instead!" },
	{ "GetComboType",               0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->ComboT'  instead!" },
	{ "SetComboType",               0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->ComboT'  instead!" },
	{ "GetComboInherentFlag",       0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->ComboI'  instead!" },
	{ "SetComboInherentFlag",       0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->ComboI'  instead!" },
	{ "GetComboSolid",              0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->ComboS'  instead!" },
	{ "SetComboSolid",              0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'mapdata->ComboS'  instead!" },
	
	{ "getJoypadPress[]",           0,          ZTID_BOOL,   JOYPADPRESS,         FL_DEPR,  { ZTID_GAME, ZTID_FLOAT },{},0,"Use 'Input->Joypad[]' instead!" },
	{ "getButtonPress[]",           0,          ZTID_BOOL,   BUTTONPRESS,         FL_DEPR,  { ZTID_GAME, ZTID_FLOAT },{},0,"Use 'Input->Press[]' instead!" },
	{ "setButtonPress[]",           0,          ZTID_VOID,   BUTTONPRESS,         FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_BOOL },{},0,"Use 'Input->Press[]' instead!" },
	{ "getButtonInput[]",           0,          ZTID_BOOL,   BUTTONINPUT,         FL_DEPR,  { ZTID_GAME, ZTID_FLOAT },{},0,"Use 'Input->Button[]' instead!" },
	{ "setButtonInput[]",           0,          ZTID_VOID,   BUTTONINPUT,         FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_BOOL },{},0,"Use 'Input->Button[]' instead!" },
	{ "getButtonHeld[]",            0,          ZTID_BOOL,   BUTTONHELD,          FL_DEPR,  { ZTID_GAME, ZTID_FLOAT },{},0,"Use 'Input->Button[]' instead!" },
	{ "setButtonHeld[]",            0,          ZTID_VOID,   BUTTONHELD,          FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_BOOL },{},0,"Use 'Input->Button[]' instead!" },
	{ "GetUntypedScript",           0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetSubscreenScript",         0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT },{} },
	
	{ "getDMapPalette[]",           0,         ZTID_FLOAT,   DMAPLEVELPAL,        FL_DEPR,  { ZTID_GAME, ZTID_FLOAT },{},0,"Use 'dmapdata->Palette' instead!" },
	{ "setDMapPalette[]",           0,          ZTID_VOID,   DMAPLEVELPAL,        FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'dmapdata->Palette' instead!" },
	{ "GetDMapMusicFilename",       0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'dmapdata->GetMusic()' instead!" },
	{ "GetDMapMusicTrack",          0,         ZTID_FLOAT,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT },{},0,"Use 'dmapdata->MusicTrack' instead!" },
	{ "SetDMapEnhancedMusic",       0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'dmapdata->SetMusic()' instead!" },
	{ "getDMapFlags[]",             0,         ZTID_FLOAT,   DMAPFLAGSD,          FL_DEPR,  { ZTID_GAME, ZTID_FLOAT },{},0,"Use 'dmapdata->Flagset[]' instead!" },
	{ "setDMapFlags[]",             0,          ZTID_VOID,   DMAPFLAGSD,          FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'dmapdata->Flagset[]' instead!" },
	{ "getDMapLevel[]",             0,         ZTID_FLOAT,   DMAPLEVELD,          FL_DEPR,  { ZTID_GAME, ZTID_FLOAT },{},0,"Use 'dmapdata->Level' instead!" },
	{ "setDMapLevel[]",             0,          ZTID_VOID,   DMAPLEVELD,          FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'dmapdata->Level' instead!" },
	{ "getDMapCompass[]",           0,         ZTID_FLOAT,   DMAPCOMPASSD,        FL_DEPR,  { ZTID_GAME, ZTID_FLOAT },{},0,"Use 'dmapdata->Compass' instead!" },
	{ "setDMapCompass[]",           0,          ZTID_VOID,   DMAPCOMPASSD,        FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'dmapdata->Compass' instead!" },
	{ "getDMapContinue[]",          0,         ZTID_FLOAT,   DMAPCONTINUED,       FL_DEPR,  { ZTID_GAME, ZTID_FLOAT },{},0,"Use 'dmapdata->Continue' instead!" },
	{ "setDMapContinue[]",          0,          ZTID_VOID,   DMAPCONTINUED,       FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'dmapdata->Continue' instead!" },
	{ "getDMapMIDI[]",              0,         ZTID_FLOAT,   DMAPMIDID,           FL_DEPR,  { ZTID_GAME, ZTID_FLOAT },{},0,"Use 'dmapdata->MIDI' instead!" },
	{ "setDMapMIDI[]",              0,          ZTID_VOID,   DMAPMIDID,           FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'dmapdata->MIDI' instead!" },
	{ "GetDMapName",                0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'dmapdata->GetName()' instead!" },
	{ "SetDMapName",                0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'dmapdata->SetName()' instead!" },
	{ "GetDMapTitle",               0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'dmapdata->GetTitle()' instead!" },
	{ "SetDMapTitle",               0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'dmapdata->SetTitle()' instead!" },
	{ "GetDMapIntro",               0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'dmapdata->GetIntro()' instead!" },
	{ "SetDMapIntro",               0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'dmapdata->SetIntro()' instead!" },
	{ "getDMapOffset[]",            0,         ZTID_FLOAT,   DMAPOFFSET,          FL_DEPR,  { ZTID_GAME, ZTID_FLOAT },{},0,"Use 'dmapdata->Offset' instead!" },
	{ "setDMapOffset[]",            0,          ZTID_VOID,   DMAPOFFSET,          FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'dmapdata->Offset' instead!" },
	{ "getDMapMap[]",               0,         ZTID_FLOAT,   DMAPMAP,             FL_DEPR,  { ZTID_GAME, ZTID_FLOAT },{},0,"Use 'dmapdata->Map' instead!" },
	{ "setDMapMap[]",               0,          ZTID_VOID,   DMAPMAP,             FL_DEPR,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{},0,"Use 'dmapdata->Map' instead!" },
	
	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
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
		Function* function = getFunction("LoadItemData");
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
		Function* function = getFunction("LoadNPCData");
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
		Function* function = getFunction("LoadDMapData");
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
	
	//LoadASubData
	{
		Function* function = getFunction("LoadASubData");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OLoadSubscreenDataRV(new VarArgument(EXP1), new LiteralArgument(0*10000)));
		RETURN();
		function->giveCode(code);
	}
	
	//LoadPSubData
	{
		Function* function = getFunction("LoadPSubData");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OLoadSubscreenDataRV(new VarArgument(EXP1), new LiteralArgument(1*10000)));
		RETURN();
		function->giveCode(code);
	}
	
	//LoadOSubData
	{
		Function* function = getFunction("LoadOSubData");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OLoadSubscreenDataRV(new VarArgument(EXP1), new LiteralArgument(2*10000)));
		RETURN();
		function->giveCode(code);
	}
	
	//SwapActiveSubscreens
	{
		Function* function = getFunction("SwapActiveSubscreens");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		ASSERT_NUL();
		addOpcode2 (code, new OSwapSubscrV(new LiteralArgument(0*10000)));
		LABELBACK(label);
		RETURN();
		function->giveCode(code);
	}
	//SwapPassiveSubscreens
	{
		Function* function = getFunction("SwapPassiveSubscreens");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		ASSERT_NUL();
		addOpcode2 (code, new OSwapSubscrV(new LiteralArgument(1*10000)));
		LABELBACK(label);
		RETURN();
		function->giveCode(code);
	}
	//SwapOverlaySubscreens
	{
		Function* function = getFunction("SwapOverlaySubscreens");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		ASSERT_NUL();
		addOpcode2 (code, new OSwapSubscrV(new LiteralArgument(2*10000)));
		LABELBACK(label);
		RETURN();
		function->giveCode(code);
	}
	
	//Dropset
	{
		Function* function = getFunction("LoadDropset");
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
		Function* function = getFunction("LoadRNG");
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
		Function* function = getFunction("LoadStack");
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
		Function* function = getFunction("LoadBottleData");
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
		Function* function = getFunction("LoadBottleShopData");
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
		Function* function = getFunction("LoadGenericData");
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
		Function* function = getFunction("LoadMessageData");
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
		Function* function = getFunction("LoadComboData");
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
	//int32_t LoadMapData(mapdata, int32_t map,int32_t scr)
	{
		Function* function = getFunction("LoadMapData");
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
		Function* function = getFunction("LoadTempScreen");
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
		Function* function = getFunction("LoadScrollingScreen");
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
		Function* function = getFunction("CreateBitmap");
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
		
	Function* function = getFunction("LoadSpriteData");
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
		Function* function = getFunction("LoadShopData");
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
		Function* function = getFunction("LoadInfoShopData");
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
	//Bitmap
	{
		Function* function = getFunction("LoadBitmapID");
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
		Function* function = getFunction("GetScreenState");
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
		RETURN();
		LABELBACK(done);
		function->giveCode(code);
	}
	//void SetScreenState(game, int32_t,int32_t,int32_t,bool)
	{
		Function* function = getFunction("SetScreenState");
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
		Function* function = getFunction("GetScreenD");
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
		Function* function = getFunction("SetScreenD");
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
		Function* function = getFunction("GetDMapScreenD");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
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
		Function* function = getFunction("SetDMapScreenD");
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
		Function* function = getFunction("PlaySound");
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
		Function* function = getFunction("PlayMIDI");
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
		Function* function = getFunction("PlayEnhancedMusic");
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
		Function* function = getFunction("GetDMapMusicFilename");
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
		Function* function = getFunction("GetDMapMusicTrack");
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
		Function* function = getFunction("SetDMapEnhancedMusic");
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
		Function* function = getFunction("GetComboData");
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
		Function* function = getFunction("SetComboData");
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
		Function* function = getFunction("GetComboCSet");
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
		Function* function = getFunction("SetComboCSet");
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
		Function* function = getFunction("GetComboFlag");
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
		Function* function = getFunction("SetComboFlag");
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
		Function* function = getFunction("GetComboType");
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
		Function* function = getFunction("SetComboType");
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
		Function* function = getFunction("GetComboInherentFlag");
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
		Function* function = getFunction("SetComboInherentFlag");
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
		Function* function = getFunction("GetComboSolid");
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
		Function* function = getFunction("SetComboSolid");
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
		Function* function = getFunction("GetScreenFlags");
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
		Function* function = getFunction("GetScreenEFlags");
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
		Function* function = getFunction("Save");
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
		Function* function = getFunction("End");
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
		Function* function = getFunction("Continue");
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
		Function* function = getFunction("SaveAndQuit");
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
		Function* function = getFunction("SaveAndContinue");
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
		Function* function = getFunction("ShowContinueScreen");
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
		Function* function = getFunction("ComboTile");
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
		Function* function = getFunction("GetSaveName");
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
		Function* function = getFunction("SetSaveName");
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
		Function* function = getFunction("GetMessage");
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
		Function* function = getFunction("GetDMapName");
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
		Function* function = getFunction("GetDMapTitle");
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
		Function* function = getFunction("GetDMapIntro");
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
		Function* function = getFunction("GreyscaleOn");
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
		Function* function = getFunction("GreyscaleOff");
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
		Function* function = getFunction("SetMessage");
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
		Function* function = getFunction("SetDMapName");
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
		Function* function = getFunction("SetDMapTitle");
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
		Function* function = getFunction("SetDMapIntro");
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
		Function* function = getFunction("ShowSaveScreen");
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
		Function* function = getFunction("ShowSaveQuitScreen");
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
		Function* function = getFunction("GetFFCScript");
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
		Function* function = getFunction("GetComboScript");
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
		Function* function = getFunction("GetItemScript");
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
		Function* function = getFunction("GetNPCScript");
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
		Function* function = getFunction("GetLWeaponScript");
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
		Function* function = getFunction("GetEWeaponScript");
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
		Function* function = getFunction("GetGenericScript");
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
		Function* function = getFunction("GetHeroScript");
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
		Function* function = getFunction("GetPlayerScript");
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
		Function* function = getFunction("GetLinkScript");
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
		Function* function = getFunction("GetGlobalScript");
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
		Function* function = getFunction("GetDMapScript");
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
		Function* function = getFunction("GetScreenScript");
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
		Function* function = getFunction("GetItemSpriteScript");
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
		Function* function = getFunction("GetUntypedScript");
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
		Function* function = getFunction("GetSubscreenScript");
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
		Function* function = getFunction("GetNPC");
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
		Function* function = getFunction("GetItem");
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
		Function* function = getFunction("GetCombo");
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
		Function* function = getFunction("GetDMap");
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
		Function* function = getFunction("GetScreenEnemy");
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
		Function* function = getFunction("GetScreenDoor");
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
		Function* function = getFunction("SetScreenEnemy");
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
		Function* function = getFunction("SetScreenDoor");
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
		Function* function = getFunction("SetScreenWidth");
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
		Function* function = getFunction("GetScreenWidth");
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
		Function* function = getFunction("SetScreenHeight");
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
		Function* function = getFunction("GetScreenHeight");
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
		Function* function = getFunction("SetScreenViewX");
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
		Function* function = getFunction("GetScreenViewX");
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
		Function* function = getFunction("SetScreenViewY");
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
		Function* function = getFunction("GetScreenViewY");
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
		Function* function = getFunction("SetScreenGuy");
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
		Function* function = getFunction("GetScreenGuy");
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
		Function* function = getFunction("SetScreenString");
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
		Function* function = getFunction("GetScreenString");
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
		Function* function = getFunction("SetScreenRoomType");
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
		Function* function = getFunction("GetScreenRoomType");
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
		Function* function = getFunction("SetScreenEntryX");
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
		Function* function = getFunction("GetScreenEntryX");
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
		Function* function = getFunction("SetScreenEntryY");
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
		Function* function = getFunction("GetScreenEntryY");
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
		Function* function = getFunction("SetScreenItem");
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
		Function* function = getFunction("GetScreenItem");
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
		Function* function = getFunction("SetScreenUndercombo");
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
		Function* function = getFunction("GetScreenUndercombo");
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
		Function* function = getFunction("SetScreenUnderCSet");
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
		Function* function = getFunction("GetScreenUnderCSet");
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
		Function* function = getFunction("SetScreenCatchall");
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
		Function* function = getFunction("GetScreenCatchall");
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
		Function* function = getFunction("SetScreenLayerOpacity");
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
		Function* function = getFunction("GetScreenLayerOpacity");
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
		Function* function = getFunction("SetScreenSecretCombo");
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
		Function* function = getFunction("GetScreenSecretCombo");
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
		Function* function = getFunction("SetScreenSecretCSet");
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
		Function* function = getFunction("GetScreenSecretCSet");
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
		Function* function = getFunction("SetScreenSecretFlag");
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
		Function* function = getFunction("GetScreenSecretFlag");
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
		Function* function = getFunction("SetScreenLayerMap");
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
		Function* function = getFunction("GetScreenLayerMap");
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
		Function* function = getFunction("SetScreenLayerScreen");
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
		Function* function = getFunction("GetScreenLayerScreen");
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
		Function* function = getFunction("SetScreenPath");
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
		Function* function = getFunction("GetScreenPath");
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
		Function* function = getFunction("SetScreenWarpReturnX");
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
		Function* function = getFunction("GetScreenWarpReturnX");
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
		Function* function = getFunction("SetScreenWarpReturnY");
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
	//int32_t GetScreenWarpReturnY(game,int32_t,int32_t,int32_t)
	{
		Function* function = getFunction("GetScreenWarpReturnY");
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
		Function* function = getFunction("PlayOgg");
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
		Function* function = getFunction("GetOggPos");
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
		Function* function = getFunction("SetOggPos");
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
		Function* function = getFunction("SetOggSpeed");
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
		Function* function = getFunction("Reload");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer, and ignore it
		ASSERT_NUL();
		addOpcode2 (code, new OGameReload());
		LABELBACK(label);
		RETURN();
		function->giveCode(code);
	}
	
	//void SetCustomCursor(game, bitmap, int, int, bool, bool)
	{
		Function* function = getFunction("SetCustomCursor");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		ASSERT_NUL();
		addOpcode2 (code, new OSetCustomCursor());
		LABELBACK(label);
		POP_ARGS(5, NUL);
		RETURN();
		function->giveCode(code);
	}
	//savedportal LoadSavedPortal(game, int32_t)
	{
		Function* function = getFunction("LoadSavedPortal");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OLoadSavPortalRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//portal CreateSavedPortal(game)
	{
		Function* function = getFunction("CreateSavedPortal");
		
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		ASSERT_NUL();
		addOpcode2 (code, new OCreateSavPortal());
		LABELBACK(label);
		RETURN();
		function->giveCode(code);
	}
	//int CurrentItemID(game, int itype, int flags)
	{
		Function* function = getFunction("CurrentItemID");
		
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		ASSERT_NUL();
		addOpcode2 (code, new OCurrentItemID());
		LABELBACK(label);
		POP_ARGS(NUL, 2);
		RETURN();
		function->giveCode(code);
	}
}

