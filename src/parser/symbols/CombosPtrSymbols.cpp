#include "SymbolDefs.h"

CombosPtrSymbols CombosPtrSymbols::singleton = CombosPtrSymbols();

static AccessorTable2 CombosTable2[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
	
//	newcombo struct
	{ "_getPosX",                   0,         ZTID_FLOAT,   COMBOXR,                   0,  { ZTID_COMBOS },{} },
	{ "getX",                       0,         ZTID_FLOAT,   COMBOXR,                   0,  { ZTID_COMBOS },{} },
	{ "getY",                       0,         ZTID_FLOAT,   COMBOYR,                   0,  { ZTID_COMBOS },{} },
	{ "_getPosY",                   0,         ZTID_FLOAT,   COMBOYR,                   0,  { ZTID_COMBOS },{} },
	{ "_getPos",                    0,         ZTID_FLOAT,   COMBOPOSR,                 0,  { ZTID_COMBOS },{} },
	{ "getPos",                     0,         ZTID_FLOAT,   COMBOPOSR,                 0,  { ZTID_COMBOS },{} },
	{ "_getLayer",                  0,         ZTID_FLOAT,   COMBOLAYERR,               0,  { ZTID_COMBOS },{} },
	{ "getLayer",                   0,         ZTID_FLOAT,   COMBOLAYERR,               0,  { ZTID_COMBOS },{} },
	{ "getInitD[]",                 0,       ZTID_UNTYPED,   COMBODATAINITD,            0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "setInitD[]",                 0,          ZTID_VOID,   COMBODATAINITD,            0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getID",                      0,         ZTID_FLOAT,   COMBODATAID,               0,  { ZTID_COMBOS },{} },
	{ "getOriginalTile",            0,         ZTID_FLOAT,   COMBODOTILE,               0,  { ZTID_COMBOS },{} },
	{ "setOriginalTile",            0,          ZTID_VOID,   COMBODOTILE,               0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getFrame",                   0,         ZTID_FLOAT,   COMBODFRAME,               0,  { ZTID_COMBOS },{} },
	{ "setFrame",                   0,          ZTID_VOID,   COMBODFRAME,               0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getAClk",                    0,         ZTID_FLOAT,   COMBODACLK,                0,  { ZTID_COMBOS },{} },
	{ "setAClk",                    0,          ZTID_VOID,   COMBODACLK,                0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTile",                    0,         ZTID_FLOAT,   COMBODTILE,                0,  { ZTID_COMBOS },{} },
	{ "setTile",                    0,          ZTID_VOID,   COMBODTILE,                0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getScript",                  0,         ZTID_FLOAT,   COMBODATASCRIPT,           0,  { ZTID_COMBOS },{} },
	{ "setScript",                  0,          ZTID_VOID,   COMBODATASCRIPT,           0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getASpeed",                  0,         ZTID_FLOAT,   COMBODASPEED,              0,  { ZTID_COMBOS },{} },
	{ "setASpeed",                  0,          ZTID_VOID,   COMBODASPEED,              0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getFlip",                    0,         ZTID_FLOAT,   COMBODFLIP,                0,  { ZTID_COMBOS },{} },
	{ "setFlip",                    0,          ZTID_VOID,   COMBODFLIP,                0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getWalk",                    0,         ZTID_FLOAT,   COMBODWALK,                0,  { ZTID_COMBOS },{} },
	{ "setWalk",                    0,          ZTID_VOID,   COMBODWALK,                0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getEffect",                  0,         ZTID_FLOAT,   COMBODEFFECT,              0,  { ZTID_COMBOS },{} },
	{ "setEffect",                  0,          ZTID_VOID,   COMBODEFFECT,              0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getType",                    0,         ZTID_FLOAT,   COMBODTYPE,                0,  { ZTID_COMBOS },{} },
	{ "setType",                    0,          ZTID_VOID,   COMBODTYPE,                0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getCSet",                    0,         ZTID_FLOAT,   COMBODCSET,                0,  { ZTID_COMBOS },{} },
	{ "setCSet",                    0,          ZTID_VOID,   COMBODCSET,                0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getCSet2",                   0,         ZTID_FLOAT,   COMBODCSET,                0,  { ZTID_COMBOS },{} },
	{ "setCSet2",                   0,          ZTID_VOID,   COMBODCSET,                0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getCSet2Flags",              0,         ZTID_FLOAT,   COMBODCSET2FLAGS,          0,  { ZTID_COMBOS },{} },
	{ "setCSet2Flags",              0,          ZTID_VOID,   COMBODCSET2FLAGS,          0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getFoo",                     0,         ZTID_FLOAT,   COMBODFOO,                 0,  { ZTID_COMBOS },{} },
	{ "setFoo",                     0,          ZTID_VOID,   COMBODFOO,                 0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getFrames",                  0,         ZTID_FLOAT,   COMBODFRAMES,              0,  { ZTID_COMBOS },{} },
	{ "setFrames",                  0,          ZTID_VOID,   COMBODFRAMES,              0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getNextData",                0,         ZTID_FLOAT,   COMBODNEXTD,               0,  { ZTID_COMBOS },{} },
	{ "setNextData",                0,          ZTID_VOID,   COMBODNEXTD,               0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getNextCSet",                0,         ZTID_FLOAT,   COMBODNEXTC,               0,  { ZTID_COMBOS },{} },
	{ "setNextCSet",                0,          ZTID_VOID,   COMBODNEXTC,               0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getFlag",                    0,         ZTID_FLOAT,   COMBODFLAG,                0,  { ZTID_COMBOS },{} },
	{ "setFlag",                    0,          ZTID_VOID,   COMBODFLAG,                0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getSkipAnim",                0,         ZTID_FLOAT,   COMBODSKIPANIM,            0,  { ZTID_COMBOS },{} },
	{ "setSkipAnim",                0,          ZTID_VOID,   COMBODSKIPANIM,            0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getNextTimer",               0,         ZTID_FLOAT,   COMBODNEXTTIMER,           0,  { ZTID_COMBOS },{} },
	{ "setNextTimer",               0,          ZTID_VOID,   COMBODNEXTTIMER,           0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getSkipAnimY",               0,         ZTID_FLOAT,   COMBODAKIMANIMY,           0,  { ZTID_COMBOS },{} },
	{ "setSkipAnimY",               0,          ZTID_VOID,   COMBODAKIMANIMY,           0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getAnimFlags",               0,         ZTID_FLOAT,   COMBODANIMFLAGS,           0,  { ZTID_COMBOS },{} },
	{ "setAnimFlags",               0,          ZTID_VOID,   COMBODANIMFLAGS,           0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getExpansion[]",             0,         ZTID_FLOAT,   COMBODEXPANSION,           0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "setExpansion[]",             0,          ZTID_VOID,   COMBODEXPANSION,           0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getAttributes[]",            0,         ZTID_FLOAT,   COMBODATTRIBUTES,          0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "setAttributes[]",            0,          ZTID_VOID,   COMBODATTRIBUTES,          0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getAttribytes[]",            0,         ZTID_FLOAT,   COMBODATTRIBYTES,          0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "setAttribytes[]",            0,          ZTID_VOID,   COMBODATTRIBYTES,          0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getAttrishorts[]",           0,         ZTID_FLOAT,   COMBODATTRISHORTS,         0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "setAttrishorts[]",           0,          ZTID_VOID,   COMBODATTRISHORTS,         0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getTriggerFlags[]",          0,         ZTID_FLOAT,   COMBODTRIGGERFLAGS,        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "setTriggerFlags[]",          0,          ZTID_VOID,   COMBODTRIGGERFLAGS,        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getTrigFlags[]",             0,          ZTID_BOOL,   COMBODTRIGGERFLAGS2,       0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "setTrigFlags[]",             0,          ZTID_VOID,   COMBODTRIGGERFLAGS2,       0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getTriggerButton[]",         0,          ZTID_BOOL,   COMBODTRIGGERBUTTON,       0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "setTriggerButton[]",         0,          ZTID_VOID,   COMBODTRIGGERBUTTON,       0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getUserFlags",               0,         ZTID_FLOAT,   COMBODUSRFLAGS,            0,  { ZTID_COMBOS },{} },
	{ "setUserFlags",               0,          ZTID_VOID,   COMBODUSRFLAGS,            0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getFlags[]",                 0,          ZTID_BOOL,   COMBODUSRFLAGARR,          0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "setFlags[]",                 0,          ZTID_VOID,   COMBODUSRFLAGARR,          0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getGenFlags[]",              0,          ZTID_BOOL,   COMBODGENFLAGARR,          0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "setGenFlags[]",              0,          ZTID_VOID,   COMBODGENFLAGARR,          0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getTriggerLevel",            0,         ZTID_FLOAT,   COMBODTRIGGERLEVEL,        0,  { ZTID_COMBOS },{} },
	{ "setTriggerLevel",            0,          ZTID_VOID,   COMBODTRIGGERLEVEL,        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTriggerItem",             0,         ZTID_FLOAT,   COMBODTRIGGERITEM,         0,  { ZTID_COMBOS },{} },
	{ "setTriggerItem",             0,          ZTID_VOID,   COMBODTRIGGERITEM,         0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTriggerTimer",            0,         ZTID_FLOAT,   COMBODTRIGGERTIMER,        0,  { ZTID_COMBOS },{} },
	{ "setTriggerTimer",            0,          ZTID_VOID,   COMBODTRIGGERTIMER,        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTriggerSFX",              0,         ZTID_FLOAT,   COMBODTRIGGERSFX,          0,  { ZTID_COMBOS },{} },
	{ "setTriggerSFX",              0,          ZTID_VOID,   COMBODTRIGGERSFX,          0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTriggerChange",           0,         ZTID_FLOAT,   COMBODTRIGGERCHANGECMB,    0,  { ZTID_COMBOS },{} },
	{ "setTriggerChange",           0,          ZTID_VOID,   COMBODTRIGGERCHANGECMB,    0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTriggerProximity",        0,         ZTID_FLOAT,   COMBODTRIGGERPROX,         0,  { ZTID_COMBOS },{} },
	{ "setTriggerProximity",        0,          ZTID_VOID,   COMBODTRIGGERPROX,         0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTriggerLightBeam",        0,         ZTID_FLOAT,   COMBODTRIGGERLIGHTBEAM,    0,  { ZTID_COMBOS },{} },
	{ "setTriggerLightBeam",        0,          ZTID_VOID,   COMBODTRIGGERLIGHTBEAM,    0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTriggerCounter",          0,         ZTID_FLOAT,   COMBODTRIGGERCTR,          0,  { ZTID_COMBOS },{} },
	{ "setTriggerCounter",          0,          ZTID_VOID,   COMBODTRIGGERCTR,          0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTriggerCtrAmount",        0,         ZTID_FLOAT,   COMBODTRIGGERCTRAMNT,      0,  { ZTID_COMBOS },{} },
	{ "setTriggerCtrAmount",        0,          ZTID_VOID,   COMBODTRIGGERCTRAMNT,      0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTrigCooldown",            0,         ZTID_FLOAT,   COMBODTRIGGERCOOLDOWN,     0,  { ZTID_COMBOS },{} },
	{ "setTrigCooldown",            0,          ZTID_VOID,   COMBODTRIGGERCOOLDOWN,     0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTrigCopycat",             0,         ZTID_FLOAT,   COMBODTRIGGERCOPYCAT,      0,  { ZTID_COMBOS },{} },
	{ "setTrigCopycat",             0,          ZTID_VOID,   COMBODTRIGGERCOPYCAT,      0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTrigSpawnItemPickup",     0,         ZTID_FLOAT,   COMBODTRIGITEMPICKUP,      0,  { ZTID_COMBOS },{} },
	{ "setTrigSpawnItemPickup",     0,          ZTID_VOID,   COMBODTRIGITEMPICKUP,      0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTrigExState",             0,         ZTID_FLOAT,   COMBODTRIGEXSTATE,         0,  { ZTID_COMBOS },{} },
	{ "setTrigExState",             0,          ZTID_VOID,   COMBODTRIGEXSTATE,         0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTrigSpawnEnemy",          0,         ZTID_FLOAT,   COMBODTRIGSPAWNENEMY,      0,  { ZTID_COMBOS },{} },
	{ "setTrigSpawnEnemy",          0,          ZTID_VOID,   COMBODTRIGSPAWNENEMY,      0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTrigSpawnItem",           0,         ZTID_FLOAT,   COMBODTRIGSPAWNITEM,       0,  { ZTID_COMBOS },{} },
	{ "setTrigSpawnItem",           0,          ZTID_VOID,   COMBODTRIGSPAWNITEM,       0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTrigCSetChange",          0,         ZTID_FLOAT,   COMBODTRIGCSETCHANGE,      0,  { ZTID_COMBOS },{} },
	{ "setTrigCSetChange",          0,          ZTID_VOID,   COMBODTRIGCSETCHANGE,      0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getLiftGFXCombo",            0,         ZTID_FLOAT,   COMBODLIFTGFXCOMBO,        0,  { ZTID_COMBOS },{} },
	{ "setLiftGFXCombo",            0,          ZTID_VOID,   COMBODLIFTGFXCOMBO,        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getLiftGFXCSet",             0,         ZTID_FLOAT,   COMBODLIFTGFXCCSET,        0,  { ZTID_COMBOS },{} },
	{ "setLiftGFXCSet",             0,          ZTID_VOID,   COMBODLIFTGFXCCSET,        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getLiftUnderCombo",          0,         ZTID_FLOAT,   COMBODLIFTUNDERCMB,        0,  { ZTID_COMBOS },{} },
	{ "setLiftUnderCombo",          0,          ZTID_VOID,   COMBODLIFTUNDERCMB,        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getLiftUnderCSet",           0,         ZTID_FLOAT,   COMBODLIFTUNDERCS,         0,  { ZTID_COMBOS },{} },
	{ "setLiftUnderCSet",           0,          ZTID_VOID,   COMBODLIFTUNDERCS,         0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getLiftDamage",              0,         ZTID_FLOAT,   COMBODLIFTDAMAGE,          0,  { ZTID_COMBOS },{} },
	{ "setLiftDamage",              0,          ZTID_VOID,   COMBODLIFTDAMAGE,          0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getLiftLevel",               0,         ZTID_FLOAT,   COMBODLIFTLEVEL,           0,  { ZTID_COMBOS },{} },
	{ "setLiftLevel",               0,          ZTID_VOID,   COMBODLIFTLEVEL,           0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getLiftItem",                0,         ZTID_FLOAT,   COMBODLIFTITEM,            0,  { ZTID_COMBOS },{} },
	{ "setLiftItem",                0,          ZTID_VOID,   COMBODLIFTITEM,            0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getLiftFlags[]",             0,          ZTID_BOOL,   COMBODLIFTFLAGS,           0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "setLiftFlags[]",             0,          ZTID_VOID,   COMBODLIFTFLAGS,           0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getLiftGFXType",             0,         ZTID_FLOAT,   COMBODLIFTGFXTYPE,         0,  { ZTID_COMBOS },{} },
	{ "setLiftGFXType",             0,          ZTID_VOID,   COMBODLIFTGFXTYPE,         0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getLiftGFXSprite",           0,         ZTID_FLOAT,   COMBODLIFTGFXSPRITE,       0,  { ZTID_COMBOS },{} },
	{ "setLiftGFXSprite",           0,          ZTID_VOID,   COMBODLIFTGFXSPRITE,       0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getLiftSFX",                 0,         ZTID_FLOAT,   COMBODLIFTSFX,             0,  { ZTID_COMBOS },{} },
	{ "setLiftSFX",                 0,          ZTID_VOID,   COMBODLIFTSFX,             0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getLiftBreakSprite",         0,         ZTID_FLOAT,   COMBODLIFTBREAKSPRITE,     0,  { ZTID_COMBOS },{} },
	{ "setLiftBreakSprite",         0,          ZTID_VOID,   COMBODLIFTBREAKSPRITE,     0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getLiftBreakSFX",            0,         ZTID_FLOAT,   COMBODLIFTBREAKSFX,        0,  { ZTID_COMBOS },{} },
	{ "setLiftBreakSFX",            0,          ZTID_VOID,   COMBODLIFTBREAKSFX,        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getLiftHeight",              0,         ZTID_FLOAT,   COMBODLIFTHEIGHT,          0,  { ZTID_COMBOS },{} },
	{ "setLiftHeight",              0,          ZTID_VOID,   COMBODLIFTHEIGHT,          0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getLiftTime",                0,         ZTID_FLOAT,   COMBODLIFTTIME,            0,  { ZTID_COMBOS },{} },
	{ "setLiftTime",                0,          ZTID_VOID,   COMBODLIFTTIME,            0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	
	
	
//	comboclass struct
//	"Name" -> Needs to be a function, GetName(int32_t string[])
	{ "getBlockNPC",                0,         ZTID_FLOAT,   COMBODBLOCKNPC,            0,  { ZTID_COMBOS },{} },
	{ "setBlockNPC",                0,          ZTID_VOID,   COMBODBLOCKNPC,            0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getBlockHole",               0,         ZTID_FLOAT,   COMBODBLOCKHOLE,           0,  { ZTID_COMBOS },{} },
	{ "setBlockHole",               0,          ZTID_VOID,   COMBODBLOCKHOLE,           0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getBlockTrigger",            0,         ZTID_FLOAT,   COMBODBLOCKTRIG,           0,  { ZTID_COMBOS },{} },
	{ "setBlockTrigger",            0,          ZTID_VOID,   COMBODBLOCKTRIG,           0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getBlockWeapon[]",           0,         ZTID_FLOAT,   COMBODBLOCKWEAPON,         0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "setBlockWeapon[]",           0,          ZTID_VOID,   COMBODBLOCKWEAPON,         0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getConveyorSpeedX",          0,         ZTID_FLOAT,   COMBODCONVXSPEED,          0,  { ZTID_COMBOS },{} },
	{ "setConveyorSpeedX",          0,          ZTID_VOID,   COMBODCONVXSPEED,          0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getConveyorSpeedY",          0,         ZTID_FLOAT,   COMBODCONVYSPEED,          0,  { ZTID_COMBOS },{} },
	{ "setConveyorSpeedY",          0,          ZTID_VOID,   COMBODCONVYSPEED,          0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getSpawnNPC",                0,         ZTID_FLOAT,   COMBODSPAWNNPC,            0,  { ZTID_COMBOS },{} },
	{ "setSpawnNPC",                0,          ZTID_VOID,   COMBODSPAWNNPC,            0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getSpawnNPCWhen",            0,         ZTID_FLOAT,   COMBODSPAWNNPCWHEN,        0,  { ZTID_COMBOS },{} },
	{ "setSpawnNPCWhen",            0,          ZTID_VOID,   COMBODSPAWNNPCWHEN,        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getSpawnNPCCHange",          0,         ZTID_FLOAT,   COMBODSPAWNNPCCHANGE,      0,  { ZTID_COMBOS },{} },
	{ "setSpawnNPCChange",          0,          ZTID_VOID,   COMBODSPAWNNPCCHANGE,      0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getDirChange",               0,         ZTID_FLOAT,   COMBODDIRCHANGETYPE,       0,  { ZTID_COMBOS },{} },
	{ "setDirChange",               0,          ZTID_VOID,   COMBODDIRCHANGETYPE,       0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getDistanceChangeTiles",     0,         ZTID_FLOAT,   COMBODDISTANCECHANGETILES, 0,  { ZTID_COMBOS },{} },
	{ "setDistanceChangeTiles",     0,          ZTID_VOID,   COMBODDISTANCECHANGETILES, 0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getDiveItem",                0,         ZTID_FLOAT,   COMBODDIVEITEM,            0,  { ZTID_COMBOS },{} },
	{ "setDiveItem",                0,          ZTID_VOID,   COMBODDIVEITEM,            0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getDock",                    0,         ZTID_FLOAT,   COMBODDOCK,                0,  { ZTID_COMBOS },{} },
	{ "setDock",                    0,          ZTID_VOID,   COMBODDOCK,                0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getFairy",                   0,         ZTID_FLOAT,   COMBODFAIRY,               0,  { ZTID_COMBOS },{} },
	{ "setFairy",                   0,          ZTID_VOID,   COMBODFAIRY,               0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getFFCAttributeChange",      0,         ZTID_FLOAT,   COMBODFFATTRCHANGE,        0,  { ZTID_COMBOS },{} },
	{ "setFFCAttributeChange",      0,          ZTID_VOID,   COMBODFFATTRCHANGE,        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getDecorationTile",          0,         ZTID_FLOAT,   COMBODFOORDECOTILE,        0,  { ZTID_COMBOS },{} },
	{ "setDecorationTile",          0,          ZTID_VOID,   COMBODFOORDECOTILE,        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getDecorationType",          0,         ZTID_FLOAT,   COMBODFOORDECOTYPE,        0,  { ZTID_COMBOS },{} },
	{ "setDecorationType",          0,          ZTID_VOID,   COMBODFOORDECOTYPE,        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getHookshot",                0,         ZTID_FLOAT,   COMBODHOOKSHOTPOINT,       0,  { ZTID_COMBOS },{} },
	{ "setHookshot",                0,          ZTID_VOID,   COMBODHOOKSHOTPOINT,       0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getLadder",                  0,         ZTID_FLOAT,   COMBODLADDERPASS,          0,  { ZTID_COMBOS },{} },
	{ "setLadder",                  0,          ZTID_VOID,   COMBODLADDERPASS,          0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getLockBlock",               0,         ZTID_FLOAT,   COMBODLOCKBLOCK,           0,  { ZTID_COMBOS },{} },
	{ "setLockBlock",               0,          ZTID_VOID,   COMBODLOCKBLOCK,           0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getLockBlockChange",         0,         ZTID_FLOAT,   COMBODLOCKBLOCKCHANGE,     0,  { ZTID_COMBOS },{} },
	{ "setLockBlockChange",         0,          ZTID_VOID,   COMBODLOCKBLOCKCHANGE,     0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getMirror",                  0,         ZTID_FLOAT,   COMBODMAGICMIRROR,         0,  { ZTID_COMBOS },{} },
	{ "setMirror",                  0,          ZTID_VOID,   COMBODMAGICMIRROR,         0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getDamageAmount",            0,         ZTID_FLOAT,   COMBODMODHPAMOUNT,         0,  { ZTID_COMBOS },{} },
	{ "setDamageAmount",            0,          ZTID_VOID,   COMBODMODHPAMOUNT,         0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getDamageDelay",             0,         ZTID_FLOAT,   COMBODMODHPDELAY,          0,  { ZTID_COMBOS },{} },
	{ "setDamageDelay",             0,          ZTID_VOID,   COMBODMODHPDELAY,          0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getDamageType",              0,         ZTID_FLOAT,   COMBODMODHPTYPE,           0,  { ZTID_COMBOS },{} },
	{ "setDamageType",              0,          ZTID_VOID,   COMBODMODHPTYPE,           0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getMagicAmount",             0,         ZTID_FLOAT,   COMBODNMODMPAMOUNT,        0,  { ZTID_COMBOS },{} },
	{ "setMagicAmount",             0,          ZTID_VOID,   COMBODNMODMPAMOUNT,        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getMagicDelay",              0,         ZTID_FLOAT,   COMBODMODMPDELAY,          0,  { ZTID_COMBOS },{} },
	{ "setMagicDelay",              0,          ZTID_VOID,   COMBODMODMPDELAY,          0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getMagicType",               0,         ZTID_FLOAT,   COMBODMODMPTYPE,           0,  { ZTID_COMBOS },{} },
	{ "setMagicType",               0,          ZTID_VOID,   COMBODMODMPTYPE,           0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getNoPushBlocks",            0,         ZTID_FLOAT,   COMBODNOPUSHBLOCK,         0,  { ZTID_COMBOS },{} },
	{ "setNoPushBlocks",            0,          ZTID_VOID,   COMBODNOPUSHBLOCK,         0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getOverhead",                0,         ZTID_FLOAT,   COMBODOVERHEAD,            0,  { ZTID_COMBOS },{} },
	{ "setOverhead",                0,          ZTID_VOID,   COMBODOVERHEAD,            0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getPlaceNPC",                0,         ZTID_FLOAT,   COMBODPLACENPC,            0,  { ZTID_COMBOS },{} },
	{ "setPlaceNPC",                0,          ZTID_VOID,   COMBODPLACENPC,            0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getPushDir",                 0,         ZTID_FLOAT,   COMBODPUSHDIR,             0,  { ZTID_COMBOS },{} },
	{ "setPushDir",                 0,          ZTID_VOID,   COMBODPUSHDIR,             0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getPushDelay",               0,         ZTID_FLOAT,   COMBODPUSHWAIT,            0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "setPushDelay",               0,          ZTID_VOID,   COMBODPUSHWAIT,            0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getPushHeavy",               0,         ZTID_FLOAT,   COMBODPUSHHEAVY,           0,  { ZTID_COMBOS },{} },
	{ "setPushHeavy",               0,          ZTID_VOID,   COMBODPUSHHEAVY,           0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getPushed",                  0,         ZTID_FLOAT,   COMBODPUSHED,              0,  { ZTID_COMBOS },{} },
	{ "setPushed",                  0,          ZTID_VOID,   COMBODPUSHED,              0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getRaft",                    0,         ZTID_FLOAT,   COMBODRAFT,                0,  { ZTID_COMBOS },{} },
	{ "setRaft",                    0,          ZTID_VOID,   COMBODRAFT,                0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getResetRoom",               0,         ZTID_FLOAT,   COMBODRESETROOM,           0,  { ZTID_COMBOS },{} },
	{ "setResetRoom",               0,          ZTID_VOID,   COMBODRESETROOM,           0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getSavePoint",               0,         ZTID_FLOAT,   COMBODSAVEPOINTTYPE,       0,  { ZTID_COMBOS },{} },
	{ "setSavePoint",               0,          ZTID_VOID,   COMBODSAVEPOINTTYPE,       0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getFreezeScreen",            0,         ZTID_FLOAT,   COMBODSCREENFREEZETYPE,    0,  { ZTID_COMBOS },{} },
	{ "setFreezeScreen",            0,          ZTID_VOID,   COMBODSCREENFREEZETYPE,    0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getSecretCombo",             0,         ZTID_FLOAT,   COMBODSECRETCOMBO,         0,  { ZTID_COMBOS },{} },
	{ "setSecretCombo",             0,          ZTID_VOID,   COMBODSECRETCOMBO,         0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getSingular",                0,         ZTID_FLOAT,   COMBODSINGULAR,            0,  { ZTID_COMBOS },{} },
	{ "setSingular",                0,          ZTID_VOID,   COMBODSINGULAR,            0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getSlowWalk",                0,         ZTID_FLOAT,   COMBODSLOWWALK,            0,  { ZTID_COMBOS },{} },
	{ "setSlowWalk",                0,          ZTID_VOID,   COMBODSLOWWALK,            0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getStatue",                  0,         ZTID_FLOAT,   COMBODSTATUETYPE,          0,  { ZTID_COMBOS },{} },
	{ "setStatue",                  0,          ZTID_VOID,   COMBODSTATUETYPE,          0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getStep",                    0,         ZTID_FLOAT,   COMBODSTEPTYPE,            0,  { ZTID_COMBOS },{} },
	{ "setStep",                    0,          ZTID_VOID,   COMBODSTEPTYPE,            0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getStepChange",              0,         ZTID_FLOAT,   COMBODSTEPCHANGEINTO,      0,  { ZTID_COMBOS },{} },
	{ "setStepChange",              0,          ZTID_VOID,   COMBODSTEPCHANGEINTO,      0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getStrike[]",                0,         ZTID_FLOAT,   COMBODSTRIKEWEAPONS,       0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "setStrike[]",                0,          ZTID_VOID,   COMBODSTRIKEWEAPONS,       0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getStrikeRemnants",          0,         ZTID_FLOAT,   COMBODSTRIKEREMNANTS,      0,  { ZTID_COMBOS },{} },
	{ "setStrikeRemnants",          0,          ZTID_VOID,   COMBODSTRIKEREMNANTS,      0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getStrikeRemnantsType",      0,         ZTID_FLOAT,   COMBODSTRIKEREMNANTSTYPE,  0,  { ZTID_COMBOS },{} },
	{ "setStrikeRemnantsType",      0,          ZTID_VOID,   COMBODSTRIKEREMNANTSTYPE,  0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getStrikeChange",            0,         ZTID_FLOAT,   COMBODSTRIKECHANGE,        0,  { ZTID_COMBOS },{} },
	{ "setStrikeChange",            0,          ZTID_VOID,   COMBODSTRIKECHANGE,        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getStrikeItem",              0,         ZTID_FLOAT,   COMBODTOUCHITEM,           0,  { ZTID_COMBOS },{} },
	{ "setStrikeItem",              0,          ZTID_VOID,   COMBODTOUCHITEM,           0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTouchItem",               0,         ZTID_FLOAT,   COMBODTOUCHITEM,           0,  { ZTID_COMBOS },{} },
	{ "setTouchItem",               0,          ZTID_VOID,   COMBODTOUCHITEM,           0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTouchStairs",             0,         ZTID_FLOAT,   COMBODTOUCHSTAIRS,         0,  { ZTID_COMBOS },{} },
	{ "setTouchStairs",             0,          ZTID_VOID,   COMBODTOUCHSTAIRS,         0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTriggerType",             0,         ZTID_FLOAT,   COMBODTRIGGERTYPE,         0,  { ZTID_COMBOS },{} },
	{ "setTriggerType",             0,          ZTID_VOID,   COMBODTRIGGERTYPE,         0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTriggerSensitivity",      0,         ZTID_FLOAT,   COMBODTRIGGERSENS,         0,  { ZTID_COMBOS },{} },
	{ "setTriggerSensitivity",      0,          ZTID_VOID,   COMBODTRIGGERSENS,         0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getWarp",                    0,         ZTID_FLOAT,   COMBODWARPTYPE,            0,  { ZTID_COMBOS },{} },
	{ "setWarp",                    0,          ZTID_VOID,   COMBODWARPTYPE,            0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getWarpSensitivity",         0,         ZTID_FLOAT,   COMBODWARPSENS,            0,  { ZTID_COMBOS },{} },
	{ "setWarpSensitivity",         0,          ZTID_VOID,   COMBODWARPSENS,            0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getWarpDirect",              0,         ZTID_FLOAT,   COMBODWARPDIRECT,          0,  { ZTID_COMBOS },{} },
	{ "setWarpDirect",              0,          ZTID_VOID,   COMBODWARPDIRECT,          0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getWarpLocation",            0,         ZTID_FLOAT,   COMBODWARPLOCATION,        0,  { ZTID_COMBOS },{} },
	{ "setWarpLocation",            0,          ZTID_VOID,   COMBODWARPLOCATION,        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getWater",                   0,         ZTID_FLOAT,   COMBODWATER,               0,  { ZTID_COMBOS },{} },
	{ "setWater",                   0,          ZTID_VOID,   COMBODWATER,               0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getWhistle",                 0,         ZTID_FLOAT,   COMBODWHISTLE,             0,  { ZTID_COMBOS },{} },
	{ "setWhistle",                 0,          ZTID_VOID,   COMBODWHISTLE,             0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getWinGame",                 0,         ZTID_FLOAT,   COMBODWINGAME,             0,  { ZTID_COMBOS },{} },
	{ "setWinGame",                 0,          ZTID_VOID,   COMBODWINGAME,             0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getBlockWeaponLevel",        0,         ZTID_FLOAT,   COMBODBLOCKWPNLEVEL,       0,  { ZTID_COMBOS },{} },
	{ "setBlockWeaponLevel",        0,          ZTID_VOID,   COMBODBLOCKWPNLEVEL,       0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	
//	Functions
//	{ "GetName",                    0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
//	{ "SetName",                    0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	
	
//	one input, one return
	{ "GetBlockEnemies",            0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetBlockHole",               0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetBlockTrigger",            0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetConveyorSpeedX",          0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetConveyorSpeedY",          0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetCreateEnemy",             0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetCreateEnemyWhen",         0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetCreateEnemyChnge",        0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetDirChangeType",           0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetDistanceChangeTiles",     0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetDiveItem",                0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetDock",                    0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetFairy",                   0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetFFComboChangeAttrib",     0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetFootDecorationsTile",     0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetFootDecorationsType",     0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetHookshotGrab",            0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetLadderPass",              0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetLockBlockType",           0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetLockBlockChange",         0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetMagicMirror",             0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetModifyHPAmount",          0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetModifyHPDelay",           0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetModifyHPType",            0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetModifyMPAmount",          0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetModifyMPDelay",           0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetModifyMPType",            0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetNoPushBlocks",            0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetOverhead",                0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetPlaceEnemy",              0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetPushDirection",           0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetPushWeight",              0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetPushWait",                0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetPushed",                  0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetRaft",                    0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetResetRoom",               0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetSavePoint",               0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetScreenFreeze",            0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetSecretCombo",             0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetSingular",                0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetSlowMove",                0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetStatue",                  0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetStepType",                0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetStepChangeTo",            0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetStrikeRemnants",          0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetStrikeRemnantsType",      0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetStrikeChange",            0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetStrikeItem",              0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetTouchItem",               0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetTouchStairs",             0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetTriggerType",             0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetTriggerSens",             0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetWarpType",                0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetWarpSens",                0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetWarpDirect",              0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetWarpLocation",            0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetWater",                   0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetWhistle",                 0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetWinGame",                 0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetBlockWeaponLevel",        0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetTile",                    0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetFlip",                    0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetWalkability",             0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetType",                    0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetCSets",                   0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetFoo",                     0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetFrames",                  0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetSpeed",                   0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetNextCombo",               0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetNextCSet",                0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetFlag",                    0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetSkipAnim",                0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetNextTimer",               0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetSkipAnimY",               0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "GetAnimFlags",               0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	
//	two inputs, one return
	{ "GetBlockWeapon",             0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetExpansion",               0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetStrikeWeapons",           0,         ZTID_FLOAT,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	
//	two inputs, no return
	{ "SetBlockEnemies",            0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetBlockHole",               0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetBlockTrigger",            0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetConveyorSpeedX",          0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetConveyorSpeedY",          0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetCreateEnemy",             0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetCreateEnemyWhen",         0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetCreateEnemyChnge",        0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetDirChangeType",           0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetDistanceChangeTiles",     0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetDiveItem",                0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetDock",                    0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetFairy",                   0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetFFComboChangeAttrib",     0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetFootDecorationsTile",     0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetFootDecorationsType",     0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetHookshotGrab",            0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetLadderPass",              0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetLockBlockType",           0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetLockBlockChange",         0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetMagicMirror",             0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetModifyHPAmount",          0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetModifyHPDelay",           0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetModifyHPType",            0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetModifyMPAmount",          0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetModifyMPDelay",           0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetModifyMPType",            0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetNoPushBlocks",            0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetOverhead",                0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetPlaceEnemy",              0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetPushDirection",           0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetPushWeight",              0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetPushWait",                0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetPushed",                  0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetRaft",                    0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetResetRoom",               0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetSavePoint",               0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetScreenFreeze",            0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetSecretCombo",             0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetSingular",                0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetSlowMove",                0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetStatue",                  0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetStepType",                0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetStepChangeTo",            0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetStrikeRemnants",          0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetStrikeRemnantsType",      0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetStrikeChange",            0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetStrikeItem",              0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetTouchItem",               0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetTouchStairs",             0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetTriggerType",             0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetTriggerSens",             0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetWarpType",                0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetWarpSens",                0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetWarpDirect",              0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetWarpLocation",            0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetWater",                   0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetWhistle",                 0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetWinGame",                 0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetBlockWeaponLevel",        0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetTile",                    0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetFlip",                    0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetWalkability",             0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetType",                    0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetCSets",                   0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetFoo",                     0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetFrames",                  0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetSpeed",                   0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetNextCombo",               0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetNextCSet",                0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetFlag",                    0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetSkipAnim",                0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetNextTimer",               0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetSkipAnimY",               0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetAnimFlags",               0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT },{} },
	
//	three inputs, no return
	{ "SetBlockWeapon",             0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetExpansion",               1,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetStrikeWeapons",           0,          ZTID_VOID,   -1,                        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	
	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
};

CombosPtrSymbols::CombosPtrSymbols()
{
	table2 = CombosTable2;
	refVar = REFCOMBODATA; //NUL;
}

void CombosPtrSymbols::generateCode()
{
	{
		Function* function = getFunction2("GetBlockEnemies");
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
		Function* function = getFunction2("GetBlockHole");
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
		Function* function = getFunction2("GetBlockTrigger");
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
		Function* function = getFunction2("GetConveyorSpeedX");
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
		Function* function = getFunction2("GetConveyorSpeedY");
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
		Function* function = getFunction2("GetCreateEnemy");
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
		Function* function = getFunction2("GetCreateEnemyWhen");
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
		Function* function = getFunction2("GetCreateEnemyChnge");
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
		Function* function = getFunction2("GetDirChangeType");
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
		Function* function = getFunction2("GetDistanceChangeTiles");
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
		Function* function = getFunction2("GetDiveItem");
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
		Function* function = getFunction2("GetDock");
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
		Function* function = getFunction2("GetFairy");
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
		Function* function = getFunction2("GetFFComboChangeAttrib");
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
		Function* function = getFunction2("GetFootDecorationsTile");
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
		Function* function = getFunction2("GetFootDecorationsType");
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
		Function* function = getFunction2("GetHookshotGrab");
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
		Function* function = getFunction2("GetLadderPass");
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
		Function* function = getFunction2("GetLockBlockType");
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
		Function* function = getFunction2("GetLockBlockChange");
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
		Function* function = getFunction2("GetMagicMirror");
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
		Function* function = getFunction2("GetModifyHPAmount");
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
		Function* function = getFunction2("GetModifyHPDelay");
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
		Function* function = getFunction2("GetModifyHPType");
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
		Function* function = getFunction2("GetModifyMPAmount");
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
		Function* function = getFunction2("GetModifyMPDelay");
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
		Function* function = getFunction2("GetModifyMPType");
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
		Function* function = getFunction2("GetNoPushBlocks");
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
		Function* function = getFunction2("GetOverhead");
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
		Function* function = getFunction2("GetPlaceEnemy");
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
		Function* function = getFunction2("GetPushDirection");
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
		Function* function = getFunction2("GetPushWeight");
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
		Function* function = getFunction2("GetPushWait");
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
		Function* function = getFunction2("GetPushed");
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
		Function* function = getFunction2("GetRaft");
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
		Function* function = getFunction2("GetResetRoom");
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
		Function* function = getFunction2("GetSavePoint");
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
		Function* function = getFunction2("GetScreenFreeze");
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
		Function* function = getFunction2("GetSecretCombo");
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
		Function* function = getFunction2("GetSingular");
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
		Function* function = getFunction2("GetSlowMove");
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
		Function* function = getFunction2("GetStatue");
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
		Function* function = getFunction2("GetStepType");
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
		Function* function = getFunction2("GetStepChangeTo");
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
		Function* function = getFunction2("GetStrikeRemnants");
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
		Function* function = getFunction2("GetStrikeRemnantsType");
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
		Function* function = getFunction2("GetStrikeChange");
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
		Function* function = getFunction2("GetStrikeItem");
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
		Function* function = getFunction2("GetTouchItem");
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
		Function* function = getFunction2("GetTouchStairs");
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
		Function* function = getFunction2("GetTriggerType");
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
		Function* function = getFunction2("GetTriggerSens");
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
		Function* function = getFunction2("GetWarpType");
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
		Function* function = getFunction2("GetWarpSens");
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
		Function* function = getFunction2("GetWarpDirect");
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
		Function* function = getFunction2("GetWarpLocation");
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
		Function* function = getFunction2("GetWater");
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
		Function* function = getFunction2("GetWhistle");
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
		Function* function = getFunction2("GetWinGame");
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
		Function* function = getFunction2("GetBlockWeaponLevel");
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
		Function* function = getFunction2("GetTile");
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
		Function* function = getFunction2("GetFlip");
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
		Function* function = getFunction2("GetWalkability");
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
		Function* function = getFunction2("GetType");
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
		Function* function = getFunction2("GetCSets");
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
		Function* function = getFunction2("GetFoo");
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
		Function* function = getFunction2("GetFrames");
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
		Function* function = getFunction2("GetSpeed");
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
		Function* function = getFunction2("GetNextCombo");
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
		Function* function = getFunction2("GetNextCSet");
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
		Function* function = getFunction2("GetFlag");
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
		Function* function = getFunction2("GetSkipAnim");
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
		Function* function = getFunction2("GetNextTimer");
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
		Function* function = getFunction2("GetSkipAnimY");
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
		Function* function = getFunction2("GetAnimFlags");
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
		Function* function = getFunction2("SetBlockEnemies");
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
		Function* function = getFunction2("SetBlockHole");
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
		Function* function = getFunction2("SetBlockTrigger");
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
		Function* function = getFunction2("SetConveyorSpeedX");
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
		Function* function = getFunction2("SetConveyorSpeedY");
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
		Function* function = getFunction2("SetCreateEnemy");
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
		Function* function = getFunction2("SetCreateEnemyWhen");
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
		Function* function = getFunction2("SetCreateEnemyChnge");
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
		Function* function = getFunction2("SetDirChangeType");
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
		Function* function = getFunction2("SetDistanceChangeTiles");
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
		Function* function = getFunction2("SetDiveItem");
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
		Function* function = getFunction2("SetDock");
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
		Function* function = getFunction2("SetFairy");
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
		Function* function = getFunction2("SetFFComboChangeAttrib");
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
		Function* function = getFunction2("SetFootDecorationsTile");
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
		Function* function = getFunction2("SetFootDecorationsType");
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
		Function* function = getFunction2("SetHookshotGrab");
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
		Function* function = getFunction2("SetLadderPass");
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
		Function* function = getFunction2("SetLockBlockType");
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
		Function* function = getFunction2("SetLockBlockChange");
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
		Function* function = getFunction2("SetMagicMirror");
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
		Function* function = getFunction2("SetModifyHPAmount");
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
		Function* function = getFunction2("SetModifyHPDelay");
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
		Function* function = getFunction2("SetModifyHPType");
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
		Function* function = getFunction2("SetModifyMPAmount");
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
		Function* function = getFunction2("SetModifyMPDelay");
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
		Function* function = getFunction2("SetModifyMPType");
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
		Function* function = getFunction2("SetNoPushBlocks");
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
		Function* function = getFunction2("SetOverhead");
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
		Function* function = getFunction2("SetPlaceEnemy");
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
		Function* function = getFunction2("SetPushDirection");
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
		Function* function = getFunction2("SetPushWeight");
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
		Function* function = getFunction2("SetPushWait");
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
		Function* function = getFunction2("SetPushed");
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
		Function* function = getFunction2("SetRaft");
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
		Function* function = getFunction2("SetResetRoom");
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
		Function* function = getFunction2("SetSavePoint");
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
		Function* function = getFunction2("SetScreenFreeze");
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
		Function* function = getFunction2("SetSecretCombo");
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
		Function* function = getFunction2("SetSingular");
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
		Function* function = getFunction2("SetSlowMove");
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
		Function* function = getFunction2("SetStatue");
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
		Function* function = getFunction2("SetStepType");
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
		Function* function = getFunction2("SetStepChangeTo");
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
		Function* function = getFunction2("SetStrikeRemnants");
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
		Function* function = getFunction2("SetStrikeRemnantsType");
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
		Function* function = getFunction2("SetStrikeChange");
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
		Function* function = getFunction2("SetStrikeItem");
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
		Function* function = getFunction2("SetTouchItem");
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
		Function* function = getFunction2("SetTouchStairs");
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
		Function* function = getFunction2("SetTriggerType");
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
		Function* function = getFunction2("SetTriggerSens");
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
		Function* function = getFunction2("SetWarpType");
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
		Function* function = getFunction2("SetWarpSens");
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
		Function* function = getFunction2("SetWarpDirect");
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
		Function* function = getFunction2("SetWarpLocation");
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
		Function* function = getFunction2("SetWater");
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
		Function* function = getFunction2("SetWhistle");
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
		Function* function = getFunction2("SetWinGame");
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
		Function* function = getFunction2("SetBlockWeaponLevel");
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
		Function* function = getFunction2("SetTile");
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
		Function* function = getFunction2("SetFlip");
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
		Function* function = getFunction2("SetWalkability");
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
		Function* function = getFunction2("SetType");
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
		Function* function = getFunction2("SetCSets");
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
		Function* function = getFunction2("SetFoo");
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
		Function* function = getFunction2("SetFrames");
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
		Function* function = getFunction2("SetSpeed");
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
		Function* function = getFunction2("SetNextCombo");
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
		Function* function = getFunction2("SetNextCSet");
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
		Function* function = getFunction2("SetFlag");
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
		Function* function = getFunction2("SetSkipAnim");
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
		Function* function = getFunction2("SetNextTimer");
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
		Function* function = getFunction2("SetSkipAnimY");
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
		Function* function = getFunction2("SetAnimFlags");
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
		Function* function = getFunction2("GetBlockWeapon");
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
		Function* function = getFunction2("GetExpansion");
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
		Function* function = getFunction2("GetStrikeWeapons");
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
		Function* function = getFunction2("SetBlockWeapon");
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
		Function* function = getFunction2("SetStrikeWeapons");
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

