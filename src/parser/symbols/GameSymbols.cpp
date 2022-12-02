#include "SymbolDefs.h"

GameSymbols GameSymbols::singleton = GameSymbols();

static AccessorTable2 gameTable2[] =
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
	{ "getSTD[]",                   0,       ZTID_UNTYPED,   STDARR,                    0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setSTD[]",                   0,          ZTID_VOID,   STDARR,                    0,  { ZTID_GAME, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getGhost[]",                 0,       ZTID_UNTYPED,   GHOSTARR,                  0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setGhost[]",                 0,          ZTID_VOID,   GHOSTARR,                  0,  { ZTID_GAME, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getTango[]",                 0,       ZTID_UNTYPED,   TANGOARR,                  0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setTango[]",                 0,          ZTID_VOID,   TANGOARR,                  0,  { ZTID_GAME, ZTID_FLOAT, ZTID_UNTYPED },{} },
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
	{ "getGeneric[]",               0,         ZTID_FLOAT,   GAMEGENERICD,              0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setGeneric[]",               0,          ZTID_VOID,   GAMEGENERICD,              0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
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
	{ "GetScreenState",             0,          ZTID_BOOL,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetScreenState",             0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "GetScreenD",                 0,       ZTID_UNTYPED,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "SetScreenD",                 0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetDMapScreenD",             0,       ZTID_UNTYPED,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetDMapScreenD",             0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "LoadItemData",               0,     ZTID_ITEMCLASS,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "PlaySound",                  0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "PlayMIDI",                   0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "PlayEnhancedMusic",          0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetDMapMusicFilename",       0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetDMapMusicTrack",          0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "SetDMapEnhancedMusic",       0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetComboData",               0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetComboData",               0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetComboCSet",               0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetComboCSet",               0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetComboFlag",               0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetComboFlag",               0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetComboType",               0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetComboType",               0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetComboInherentFlag",       0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetComboInherentFlag",       0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetComboSolid",              0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetComboSolid",              0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "_getGetMIDI",                0,         ZTID_FLOAT,   GETMIDI,                   0,  { ZTID_GAME },{} },
	{ "GetScreenFlags",             0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetScreenEFlags",            0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getDMapFlags[]",             0,         ZTID_FLOAT,   DMAPFLAGSD,                0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setDMapFlags[]",             0,          ZTID_VOID,   DMAPFLAGSD,                0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getDMapLevel[]",             0,         ZTID_FLOAT,   DMAPLEVELD,                0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setDMapLevel[]",             0,          ZTID_VOID,   DMAPLEVELD,                0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getDMapCompass[]",           0,         ZTID_FLOAT,   DMAPCOMPASSD,              0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setDMapCompass[]",           0,          ZTID_VOID,   DMAPCOMPASSD,              0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getDMapContinue[]",          0,         ZTID_FLOAT,   DMAPCONTINUED,             0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setDMapContinue[]",          0,          ZTID_VOID,   DMAPCONTINUED,             0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getDMapMIDI[]",              0,         ZTID_FLOAT,   DMAPMIDID,                 0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setDMapMIDI[]",              0,          ZTID_VOID,   DMAPMIDID,                 0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "Save",                       0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GAME },{} },
	{ "End",                        0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GAME },{} },
	{ "Continue",                   0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GAME },{} },
	{ "SaveAndQuit",                0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GAME },{} },
	{ "SaveAndContinue",            0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GAME },{} },
	{ "ShowContinueScreen",         0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GAME },{} },
	{ "ComboTile",                  0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetSaveName",                0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "SetSaveName",                0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetMessage",                 0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetDMapName",                0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetDMapTitle",               0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetDMapIntro",               0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
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
	{ "getDMapOffset[]",            0,         ZTID_FLOAT,   DMAPOFFSET,                0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setDMapOffset[]",            0,          ZTID_VOID,   DMAPOFFSET,                0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getDMapMap[]",               0,         ZTID_FLOAT,   DMAPMAP,                   0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setDMapMap[]",               0,          ZTID_VOID,   DMAPMAP,                   0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetFFCScript",               0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetComboScript",             0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
//	Get an item script ID, similar to GetFFCScript()
	{ "GetItemScript",              0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetNPCScript",               0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetLWeaponScript",           0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetEWeaponScript",           0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetPlayerScript",            0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetLinkScript",              0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetGlobalScript",            0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetDMapScript",              0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetScreenScript",            0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetItemSpriteScript",        0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetUntypedScript",           0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetSubscreenScript",         0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetNPC",                     0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetItem",                    0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetCombo",                   0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetDMap",                    0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetHeroScript",              0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "GetGenericScript",           0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT },{} },
//	Monochrome mode
	{ "GreyscaleOn",                0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GAME },{} },
	{ "GreyscaleOff",               0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GAME },{} },
//	Joypad and Keyboard. -Z
	{ "getFFRules[]",               0,          ZTID_BOOL,   FFRULE,                    0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setFFRules[]",               0,          ZTID_VOID,   FFRULE,                    0,  { ZTID_GAME, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getButtonPress[]",           0,          ZTID_BOOL,   BUTTONPRESS,               0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setButtonPress[]",           0,          ZTID_VOID,   BUTTONPRESS,               0,  { ZTID_GAME, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getButtonInput[]",           0,          ZTID_BOOL,   BUTTONINPUT,               0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setButtonInput[]",           0,          ZTID_VOID,   BUTTONINPUT,               0,  { ZTID_GAME, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getButtonHeld[]",            0,          ZTID_BOOL,   BUTTONHELD,                0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setButtonHeld[]",            0,          ZTID_VOID,   BUTTONHELD,                0,  { ZTID_GAME, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getReadKey[]",               0,          ZTID_BOOL,   READKEY,                   0,  { ZTID_GAME, ZTID_FLOAT },{} },
//	{ "_getReadKey",                0,          ZTID_BOOL,   READKEY,                   0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "getSkipCredits",             0,          ZTID_BOOL,   SKIPCREDITS,               0,  { ZTID_GAME },{} },
	{ "setSkipCredits",             0,          ZTID_VOID,   SKIPCREDITS,               0,  { ZTID_GAME, ZTID_BOOL },{} },
	{ "getSkipF6",                  0,          ZTID_BOOL,   SKIPF6,                    0,  { ZTID_GAME },{} },
	{ "setSkipF6",                  0,          ZTID_VOID,   SKIPF6,                    0,  { ZTID_GAME, ZTID_BOOL },{} },
	{ "getSuspend[]",               0,          ZTID_BOOL,   GAMESUSPEND,               0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setSuspend[]",               0,          ZTID_VOID,   GAMESUSPEND,               0,  { ZTID_GAME, ZTID_FLOAT, ZTID_BOOL },{} },
//	{ "getResume[]",                0,          ZTID_BOOL,   GAMERESUME,                0,  { ZTID_GAME, ZTID_FLOAT },{} },
//	{ "setResume[]",                0,          ZTID_VOID,   GAMERESUME,                0,  { ZTID_GAME, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getDisableItem[]",           0,          ZTID_BOOL,   DISABLEDITEM,              0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setDisableItem[]",           0,          ZTID_VOID,   DISABLEDITEM,              0,  { ZTID_GAME, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getJoypadPress[]",           0,          ZTID_BOOL,   JOYPADPRESS,               0,  { ZTID_GAME, ZTID_FLOAT },{} },
//	Read or change the palette (level palette) for any given DMap
	{ "getDMapPalette[]",           0,         ZTID_FLOAT,   DMAPLEVELPAL,              0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setDMapPalette[]",           0,          ZTID_VOID,   DMAPLEVELPAL,              0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
//	Get the map count 
	{ "_getMapCount",               0,         ZTID_FLOAT,   GAMEMAXMAPS,               0,  { ZTID_GAME },{} },
//	Versioning
	{ "getZScriptVersion",          0,         ZTID_FLOAT,   ZSCRIPTVERSION,            0,  { ZTID_GAME },{} },
	{ "getVersion",                 0,         ZTID_FLOAT,   ZELDAVERSION,              0,  { ZTID_GAME },{} },
	{ "getBuild",                   0,         ZTID_FLOAT,   ZELDABUILD,                0,  { ZTID_GAME },{} },
	{ "getBeta",                    0,         ZTID_FLOAT,   ZELDABETA,                 0,  { ZTID_GAME },{} },
	{ "getDisableActiveSubscreen",  0,          ZTID_BOOL,   NOACTIVESUBSC,             0,  { ZTID_GAME },{} },
	{ "setDisableActiveSubscreen",  0,          ZTID_VOID,   NOACTIVESUBSC,             0,  { ZTID_GAME, ZTID_BOOL },{} },
	{ "SetMessage",                 0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetDMapName",                0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetDMapTitle",               0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetDMapIntro",               0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetScreenEnemy",             0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetScreenEnemy",             0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetScreenDoor",              0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetScreenDoor",              0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "SetScreenWidth",             0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetScreenWidth",             0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetScreenHeight",            0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetScreenHeight",            0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetScreenViewX",             0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetScreenViewX",             0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetScreenViewY",             0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetScreenViewY",             0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetScreenGuy",               0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetScreenGuy",               0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetScreenString",            0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetScreenString",            0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetScreenRoomType",          0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetScreenRoomType",          0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetScreenEntryX",            0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetScreenEntryX",            0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "SetScreenEntryY",            0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetScreenEntryY",            0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetScreenItem",              0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetScreenItem",              0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetScreenUndercombo",        0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetScreenUndercombo",        0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetScreenUnderCSet",         0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetScreenUnderCSet",         0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetScreenCatchall",          0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetScreenCatchall",          0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetScreenLayerOpacity",      0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetScreenLayerOpacity",      0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetScreenSecretCombo",       0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetScreenSecretCombo",       0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetScreenSecretCSet",        0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetScreenSecretCSet",        0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetScreenSecretFlag",        0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetScreenSecretFlag",        0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetScreenLayerMap",          0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetScreenLayerMap",          0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetScreenLayerScreen",       0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetScreenLayerScreen",       0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetScreenPath",              0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetScreenPath",              0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetScreenWarpReturnX",       0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetScreenWarpReturnX",       0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetScreenWarpReturnY",       0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetScreenWarpReturnY",       0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getHighestStringID",         0,         ZTID_FLOAT,   GAMENUMMESSAGES,           0,  { ZTID_GAME },{} },
	{ "getNumMessages",             0,         ZTID_FLOAT,   GAMENUMMESSAGES,           0,  { ZTID_GAME },{} },
	
	{ "setGameOverScreen[]",        0,          ZTID_VOID,   SETGAMEOVERELEMENT,        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "setGameOverStrings[]",       0,          ZTID_VOID,   SETGAMEOVERSTRING,         0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	
//	{ "SetContinueScreen[]",        0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
//	{ "SetContinueString[]",        0,          ZTID_VOID,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
/*	
	{ "getJoypad[]",                0,         ZTID_FLOAT,   JOYPADPRESS,               0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "getPress[]",                 0,          ZTID_BOOL,   BUTTONPRESS,               0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setPress[]",                 0,          ZTID_VOID,   BUTTONPRESS,               0,  { ZTID_GAME, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getButton[]",                0,          ZTID_BOOL,   BUTTONINPUT,               0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setButton[]",                0,          ZTID_VOID,   BUTTONINPUT,               0,  { ZTID_GAME, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getHold[]",                  0,          ZTID_BOOL,   BUTTONHELD,                0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setHold[]",                  0,          ZTID_VOID,   BUTTONHELD,                0,  { ZTID_GAME, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getReadKey[]",               1,          ZTID_BOOL,   READKEY,                   0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "getKey[]",                   0,          ZTID_BOOL,   RAWKEY,                    0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setKey[]",                   0,          ZTID_VOID,   RAWKEY,                    0,  { ZTID_GAME, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getMouse[]",                 0,         ZTID_FLOAT,   MOUSEARR,                  0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setMouse[]",                 0,          ZTID_VOID,   MOUSEARR,                  0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
*/	
//	New Datatypes
	{ "LoadNPCData",                0,       ZTID_NPCDATA,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "LoadComboData",              0,        ZTID_COMBOS,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "LoadMapData",                0,       ZTID_MAPDATA,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "LoadTempScreen",             0,       ZTID_MAPDATA,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "LoadScrollingScreen",        0,       ZTID_MAPDATA,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "LoadSpriteData",             0,    ZTID_SPRITEDATA,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "LoadShopData",               0,      ZTID_SHOPDATA,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "LoadInfoShopData",           0,      ZTID_SHOPDATA,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
//	{ "LoadScreenData",             0,     ZTID_ITEMCLASS,   -1,                        0,  { ZTID_GAME, ZTID_FLOAT },{} },
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
	{ "PlayOgg",                    0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetOggPos",                  0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GAME },{} },
	{ "SetOggPos",                  0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "SetOggSpeed",                0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "_getAllocateBitmap",         0,        ZTID_BITMAP,   ALLOCATEBITMAPR,           0,  { ZTID_GAME },{} },
	{ "getGravity[]",               0,         ZTID_FLOAT,   GAMEGRAVITY,               0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setGravity[]",               0,          ZTID_VOID,   GAMEGRAVITY,               0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getScrolling[]",             0,         ZTID_FLOAT,   GAMESCROLLING,             0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "Reload",                     0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GAME },{} },
	{ "getMiscSprites[]",           0,         ZTID_FLOAT,   GAMEMISCSPR,               0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setMiscSprites[]",           0,          ZTID_VOID,   GAMEMISCSPR,               0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getMiscSFX[]",               0,         ZTID_FLOAT,   GAMEMISCSFX,               0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setMiscSFX[]",               0,          ZTID_VOID,   GAMEMISCSFX,               0,  { ZTID_GAME, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getEventData[]",             0,       ZTID_UNTYPED,   GAMEEVENTDATA,             0,  { ZTID_GAME, ZTID_FLOAT },{} },
	{ "setEventData[]",             0,          ZTID_VOID,   GAMEEVENTDATA,             0,  { ZTID_GAME, ZTID_FLOAT, ZTID_UNTYPED },{} },
	
	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
};

GameSymbols::GameSymbols()
{
	table2 = gameTable2;
	refVar = NUL;
}

void GameSymbols::generateCode()
{
	//itemclass LoadItemData(game, int32_t)
	{
		Function* function = getFunction2("LoadItemData");
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
		Function* function = getFunction2("LoadNPCData");
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
		Function* function = getFunction2("LoadDMapData");
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
		Function* function = getFunction2("LoadDropset");
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
		Function* function = getFunction2("LoadRNG");
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
		Function* function = getFunction2("LoadStack");
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
		Function* function = getFunction2("LoadBottleData");
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
		Function* function = getFunction2("LoadBottleShopData");
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
		Function* function = getFunction2("LoadGenericData");
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
		Function* function = getFunction2("LoadMessageData");
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
		Function* function = getFunction2("LoadComboData");
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
	}
	*/
	
	//int32_t LoadMapData(mapdata, int32_t map,int32_t scr)
	{
		Function* function = getFunction2("LoadMapData");
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
		Function* function = getFunction2("LoadTempScreen");
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
		Function* function = getFunction2("LoadScrollingScreen");
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
		Function* function = getFunction2("CreateBitmap");
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
		
	Function* function = getFunction2("LoadSpriteData");
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
		Function* function = getFunction2("LoadShopData");
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
		Function* function = getFunction2("LoadInfoShopData");
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
		Function* function = getFunction2("LoadBitmapID");
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
		Function* function = getFunction2("GetScreenState");
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
		Function* function = getFunction2("SetScreenState");
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
		Function* function = getFunction2("GetScreenD");
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
		Function* function = getFunction2("SetScreenD");
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
		Function* function = getFunction2("GetDMapScreenD");
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
		Function* function = getFunction2("SetDMapScreenD");
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
		Function* function = getFunction2("PlaySound");
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
		Function* function = getFunction2("PlayMIDI");
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
		Function* function = getFunction2("PlayEnhancedMusic");
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
		Function* function = getFunction2("GetDMapMusicFilename");
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
		Function* function = getFunction2("GetDMapMusicTrack");
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
		Function* function = getFunction2("SetDMapEnhancedMusic");
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
		Function* function = getFunction2("GetComboData");
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
		Function* function = getFunction2("SetComboData");
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
		Function* function = getFunction2("GetComboCSet");
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
		Function* function = getFunction2("SetComboCSet");
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
		Function* function = getFunction2("GetComboFlag");
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
		Function* function = getFunction2("SetComboFlag");
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
		Function* function = getFunction2("GetComboType");
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
		Function* function = getFunction2("SetComboType");
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
		Function* function = getFunction2("GetComboInherentFlag");
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
		Function* function = getFunction2("SetComboInherentFlag");
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
		Function* function = getFunction2("GetComboSolid");
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
		Function* function = getFunction2("SetComboSolid");
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
		Function* function = getFunction2("GetScreenFlags");
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
		Function* function = getFunction2("GetScreenEFlags");
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
		Function* function = getFunction2("Save");
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
		Function* function = getFunction2("End");
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
		Function* function = getFunction2("Continue");
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
		Function* function = getFunction2("SaveAndQuit");
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
		Function* function = getFunction2("SaveAndContinue");
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
		Function* function = getFunction2("ShowContinueScreen");
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
		Function* function = getFunction2("ComboTile");
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
		Function* function = getFunction2("GetSaveName");
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
		Function* function = getFunction2("SetSaveName");
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
		Function* function = getFunction2("GetMessage");
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
		Function* function = getFunction2("GetDMapName");
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
		Function* function = getFunction2("GetDMapTitle");
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
		Function* function = getFunction2("GetDMapIntro");
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
		Function* function = getFunction2("GreyscaleOn");
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
		Function* function = getFunction2("GreyscaleOff");
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
		Function* function = getFunction2("SetMessage");
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
		Function* function = getFunction2("SetDMapName");
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
		Function* function = getFunction2("SetDMapTitle");
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
		Function* function = getFunction2("SetDMapIntro");
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
		Function* function = getFunction2("ShowSaveScreen");
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
		Function* function = getFunction2("ShowSaveQuitScreen");
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
		Function* function = getFunction2("GetFFCScript");
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
		Function* function = getFunction2("GetComboScript");
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
		Function* function = getFunction2("GetItemScript");
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
		Function* function = getFunction2("GetNPCScript");
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
		Function* function = getFunction2("GetLWeaponScript");
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
		Function* function = getFunction2("GetEWeaponScript");
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
		Function* function = getFunction2("GetGenericScript");
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
		Function* function = getFunction2("GetHeroScript");
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
		Function* function = getFunction2("GetPlayerScript");
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
		Function* function = getFunction2("GetLinkScript");
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
		Function* function = getFunction2("GetGlobalScript");
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
		Function* function = getFunction2("GetDMapScript");
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
		Function* function = getFunction2("GetScreenScript");
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
		Function* function = getFunction2("GetItemSpriteScript");
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
		Function* function = getFunction2("GetUntypedScript");
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
		Function* function = getFunction2("GetSubscreenScript");
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
		Function* function = getFunction2("GetNPC");
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
		Function* function = getFunction2("GetItem");
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
		Function* function = getFunction2("GetCombo");
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
		Function* function = getFunction2("GetDMap");
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
		Function* function = getFunction2("GetScreenEnemy");
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
		Function* function = getFunction2("GetScreenDoor");
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
		Function* function = getFunction2("SetScreenEnemy");
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
		Function* function = getFunction2("SetScreenDoor");
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
		Function* function = getFunction2("SetScreenWidth");
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
		Function* function = getFunction2("GetScreenWidth");
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
		Function* function = getFunction2("SetScreenHeight");
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
		Function* function = getFunction2("GetScreenHeight");
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
		Function* function = getFunction2("SetScreenViewX");
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
		Function* function = getFunction2("GetScreenViewX");
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
		Function* function = getFunction2("SetScreenViewY");
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
		Function* function = getFunction2("GetScreenViewY");
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
		Function* function = getFunction2("SetScreenGuy");
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
		Function* function = getFunction2("GetScreenGuy");
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
		Function* function = getFunction2("SetScreenString");
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
		Function* function = getFunction2("GetScreenString");
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
		Function* function = getFunction2("SetScreenRoomType");
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
		Function* function = getFunction2("GetScreenRoomType");
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
		Function* function = getFunction2("SetScreenEntryX");
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
		Function* function = getFunction2("GetScreenEntryX");
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
		Function* function = getFunction2("SetScreenEntryY");
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
		Function* function = getFunction2("GetScreenEntryY");
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
		Function* function = getFunction2("SetScreenItem");
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
		Function* function = getFunction2("GetScreenItem");
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
		Function* function = getFunction2("SetScreenUndercombo");
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
		Function* function = getFunction2("GetScreenUndercombo");
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
		Function* function = getFunction2("SetScreenUnderCSet");
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
		Function* function = getFunction2("GetScreenUnderCSet");
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
		Function* function = getFunction2("SetScreenCatchall");
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
		Function* function = getFunction2("GetScreenCatchall");
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
		Function* function = getFunction2("SetScreenLayerOpacity");
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
		Function* function = getFunction2("GetScreenLayerOpacity");
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
		Function* function = getFunction2("SetScreenSecretCombo");
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
		Function* function = getFunction2("GetScreenSecretCombo");
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
		Function* function = getFunction2("SetScreenSecretCSet");
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
		Function* function = getFunction2("GetScreenSecretCSet");
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
		Function* function = getFunction2("SetScreenSecretFlag");
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
		Function* function = getFunction2("GetScreenSecretFlag");
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
		Function* function = getFunction2("SetScreenLayerMap");
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
		Function* function = getFunction2("GetScreenLayerMap");
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
		Function* function = getFunction2("SetScreenLayerScreen");
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
		Function* function = getFunction2("GetScreenLayerScreen");
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
		Function* function = getFunction2("SetScreenPath");
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
		Function* function = getFunction2("GetScreenPath");
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
		Function* function = getFunction2("SetScreenWarpReturnX");
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
		Function* function = getFunction2("GetScreenWarpReturnX");
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
		Function* function = getFunction2("SetScreenWarpReturnY");
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
		Function* function = getFunction2("GetScreenWarpReturnY");
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
		Function* function = getFunction2("PlayOgg");
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
		Function* function = getFunction2("GetOggPos");
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
		Function* function = getFunction2("SetOggPos");
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
		Function* function = getFunction2("SetOggSpeed");
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
		Function* function = getFunction2("Reload");
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

