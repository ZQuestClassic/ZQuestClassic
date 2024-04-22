#include "SymbolDefs.h"

NPCDataSymbols NPCDataSymbols::singleton = NPCDataSymbols();

static AccessorTable NPCDataTable[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
//	datatype variables
	{ "getTile",                    0,         ZTID_FLOAT,   NPCDATATILE,               0,  { ZTID_NPCDATA },{} },
	{ "setTile",                    0,          ZTID_VOID,   NPCDATATILE,               0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getFlags",                   0,         ZTID_FLOAT,   NPCDATAFLAGS,              0,  { ZTID_NPCDATA },{} },
	{ "setFlags",                   0,          ZTID_VOID,   NPCDATAFLAGS,              0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getFlags2",                  0,         ZTID_FLOAT,   NPCDATAFLAGS2,             0,  { ZTID_NPCDATA },{} },
	{ "setFlags2",                  0,          ZTID_VOID,   NPCDATAFLAGS2,             0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getWidth",                   0,         ZTID_FLOAT,   NPCDATAWIDTH,              0,  { ZTID_NPCDATA },{} },
	{ "setWidth",                   0,          ZTID_VOID,   NPCDATAWIDTH,              0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getHeight",                  0,         ZTID_FLOAT,   NPCDATAHEIGHT,             0,  { ZTID_NPCDATA },{} },
	{ "setHeight",                  0,          ZTID_VOID,   NPCDATAHEIGHT,             0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getSTile",                   0,         ZTID_FLOAT,   NPCDATASTILE,              0,  { ZTID_NPCDATA },{} },
	{ "setSTile",                   0,          ZTID_VOID,   NPCDATASTILE,              0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getSWidth",                  0,         ZTID_FLOAT,   NPCDATASWIDTH,             0,  { ZTID_NPCDATA },{} },
	{ "setSWidth",                  0,          ZTID_VOID,   NPCDATASWIDTH,             0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getSHeight",                 0,         ZTID_FLOAT,   NPCDATASHEIGHT,            0,  { ZTID_NPCDATA },{} },
	{ "setSHeight",                 0,          ZTID_VOID,   NPCDATASHEIGHT,            0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getExTile",                  0,         ZTID_FLOAT,   NPCDATAETILE,              0,  { ZTID_NPCDATA },{} },
	{ "setExTile",                  0,          ZTID_VOID,   NPCDATAETILE,              0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getExWidth",                 0,         ZTID_FLOAT,   NPCDATAEWIDTH,             0,  { ZTID_NPCDATA },{} },
	{ "setExWidth",                 0,          ZTID_VOID,   NPCDATAEWIDTH,             0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getExHeight",                0,         ZTID_FLOAT,   NPCDATAEHEIGHT,            0,  { ZTID_NPCDATA },{} },
	{ "setExHeight",                0,          ZTID_VOID,   NPCDATAEHEIGHT,            0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getHP",                      0,         ZTID_FLOAT,   NPCDATAHP,                 0,  { ZTID_NPCDATA },{} },
	{ "setHP",                      0,          ZTID_VOID,   NPCDATAHP,                 0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getFamily",                  0,         ZTID_FLOAT,   NPCDATAFAMILY,             0,  { ZTID_NPCDATA },{} },
	{ "setFamily",                  0,          ZTID_VOID,   NPCDATAFAMILY,             0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getCSet",                    0,         ZTID_FLOAT,   NPCDATACSET,               0,  { ZTID_NPCDATA },{} },
	{ "setCSet",                    0,          ZTID_VOID,   NPCDATACSET,               0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getAnim",                    0,         ZTID_FLOAT,   NPCDATAANIM,               0,  { ZTID_NPCDATA },{} },
	{ "setAnim",                    0,          ZTID_VOID,   NPCDATAANIM,               0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getExAnim",                  0,         ZTID_FLOAT,   NPCDATAEANIM,              0,  { ZTID_NPCDATA },{} },
	{ "setExAnim",                  0,          ZTID_VOID,   NPCDATAEANIM,              0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getFramerate",               0,         ZTID_FLOAT,   NPCDATAFRAMERATE,          0,  { ZTID_NPCDATA },{} },
	{ "setFramerate",               0,          ZTID_VOID,   NPCDATAFRAMERATE,          0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getExFramerate",             0,         ZTID_FLOAT,   NPCDATAEFRAMERATE,         0,  { ZTID_NPCDATA },{} },
	{ "setExFramerate",             0,          ZTID_VOID,   NPCDATAEFRAMERATE,         0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getTouchDamage",             0,         ZTID_FLOAT,   NPCDATATOUCHDAMAGE,        0,  { ZTID_NPCDATA },{} },
	{ "setTouchDamage",             0,          ZTID_VOID,   NPCDATATOUCHDAMAGE,        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getWeaponDamage",            0,         ZTID_FLOAT,   NPCDATAWEAPONDAMAGE,       0,  { ZTID_NPCDATA },{} },
	{ "setWeaponDamage",            0,          ZTID_VOID,   NPCDATAWEAPONDAMAGE,       0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getWeapon",                  0,         ZTID_FLOAT,   NPCDATAWEAPON,             0,  { ZTID_NPCDATA },{} },
	{ "setWeapon",                  0,          ZTID_VOID,   NPCDATAWEAPON,             0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getRandom",                  0,         ZTID_FLOAT,   NPCDATARANDOM,             0,  { ZTID_NPCDATA },{} },
	{ "setRandom",                  0,          ZTID_VOID,   NPCDATARANDOM,             0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getHaltrate",                0,         ZTID_FLOAT,   NPCDATAHALT,               0,  { ZTID_NPCDATA },{} },
	{ "setHaltrate",                0,          ZTID_VOID,   NPCDATAHALT,               0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getStep",                    0,         ZTID_FLOAT,   NPCDATASTEP,               0,  { ZTID_NPCDATA },{} },
	{ "setStep",                    0,          ZTID_VOID,   NPCDATASTEP,               0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getHoming",                  0,         ZTID_FLOAT,   NPCDATAHOMING,             0,  { ZTID_NPCDATA },{} },
	{ "setHoming",                  0,          ZTID_VOID,   NPCDATAHOMING,             0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getHunger",                  0,         ZTID_FLOAT,   NPCDATAHUNGER,             0,  { ZTID_NPCDATA },{} },
	{ "setHunger",                  0,          ZTID_VOID,   NPCDATAHUNGER,             0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getDropset",                 0,         ZTID_FLOAT,   NPCDATADROPSET,            0,  { ZTID_NPCDATA },{} },
	{ "setDropset",                 0,          ZTID_VOID,   NPCDATADROPSET,            0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getBGSFX",                   0,         ZTID_FLOAT,   NPCDATABGSFX,              0,  { ZTID_NPCDATA },{} },
	{ "setBGSFX",                   0,          ZTID_VOID,   NPCDATABGSFX,              0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getDeathSFX",                0,         ZTID_FLOAT,   NPCDATADEATHSFX,           0,  { ZTID_NPCDATA },{} },
	{ "setDeathSFX",                0,          ZTID_VOID,   NPCDATADEATHSFX,           0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getHitSFX",                  0,         ZTID_FLOAT,   NPCDATAHITSFX,             0,  { ZTID_NPCDATA },{} },
	{ "setHitSFX",                  0,          ZTID_VOID,   NPCDATAHITSFX,             0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
//	{ "getFireSFX",                 0,         ZTID_FLOAT,   SPRITEDATATYPE,            0,  { ZTID_NPCDATA },{} },
//	{ "setFireSFX",                 0,          ZTID_VOID,   SPRITEDATATYPE,            0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getDrawXOffset",             0,         ZTID_FLOAT,   NPCDATAXOFS,               0,  { ZTID_NPCDATA },{} },
	{ "setDrawXOffset",             0,          ZTID_VOID,   NPCDATAXOFS,               0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getDrawYOffset",             0,         ZTID_FLOAT,   NPCDATAYOFS,               0,  { ZTID_NPCDATA },{} },
	{ "setDrawYOffset",             0,          ZTID_VOID,   NPCDATAYOFS,               0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getDrawZOffset",             0,         ZTID_FLOAT,   NPCDATAZOFS,               0,  { ZTID_NPCDATA },{} },
	{ "setDrawZOffset",             0,          ZTID_VOID,   NPCDATAZOFS,               0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getHitXOffset",              0,         ZTID_FLOAT,   NPCDATAHXOFS,              0,  { ZTID_NPCDATA },{} },
	{ "setHitXOffset",              0,          ZTID_VOID,   NPCDATAHXOFS,              0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getHitYOffset",              0,         ZTID_FLOAT,   NPCDATAHYOFS,              0,  { ZTID_NPCDATA },{} },
	{ "setHitYOffset",              0,          ZTID_VOID,   NPCDATAHYOFS,              0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getHitWidth",                0,         ZTID_FLOAT,   NPCDATAHITWIDTH,           0,  { ZTID_NPCDATA },{} },
	{ "setHitWidth",                0,          ZTID_VOID,   NPCDATAHITWIDTH,           0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getHitHeight",               0,         ZTID_FLOAT,   NPCDATAHITHEIGHT,          0,  { ZTID_NPCDATA },{} },
	{ "setHitHeight",               0,          ZTID_VOID,   NPCDATAHITHEIGHT,          0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getHitZHeight",              0,         ZTID_FLOAT,   NPCDATAHITZ,               0,  { ZTID_NPCDATA },{} },
	{ "setHitZHeight",              0,          ZTID_VOID,   NPCDATAHITZ,               0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getTileWidth",               0,         ZTID_FLOAT,   NPCDATATILEWIDTH,          0,  { ZTID_NPCDATA },{} },
	{ "setTileWidth",               0,          ZTID_VOID,   NPCDATATILEWIDTH,          0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getTileHeight",              0,         ZTID_FLOAT,   NPCDATATILEHEIGHT,         0,  { ZTID_NPCDATA },{} },
	{ "setTileHeight",              0,          ZTID_VOID,   NPCDATATILEHEIGHT,         0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getWeaponSprite",            0,         ZTID_FLOAT,   NPCDATAWPNSPRITE,          0,  { ZTID_NPCDATA },{} },
	{ "setWeaponSprite",            0,          ZTID_VOID,   NPCDATAWPNSPRITE,          0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getDefense[]",               0,         ZTID_FLOAT,   NPCDATADEFENSE,            0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "setDefense[]",               0,          ZTID_VOID,   NPCDATADEFENSE,            0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getSizeFlag[]",              0,          ZTID_BOOL,   NPCDATASIZEFLAG,           0,  { ZTID_FFC, ZTID_FLOAT },{} },
	{ "setSizeFlag[]",              0,          ZTID_VOID,   NPCDATASIZEFLAG,           0,  { ZTID_FFC, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getAttributes[]",            0,         ZTID_FLOAT,   NPCDATAATTRIBUTE,          0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "setAttributes[]",            0,          ZTID_VOID,   NPCDATAATTRIBUTE,          0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getShield[]",                0,          ZTID_BOOL,   NPCDATASHIELD,             0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "setShield[]",                0,          ZTID_VOID,   NPCDATASHIELD,             0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getBFlags[]",                0,          ZTID_BOOL,   NPCDATABEHAVIOUR,          0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "setBFlags[]",                0,          ZTID_VOID,   NPCDATABEHAVIOUR,          0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getFrozenTile",              0,         ZTID_FLOAT,   NPCDATAFROZENTILE,         0,  { ZTID_NPCDATA },{} },
	{ "setFrozenTile",              0,          ZTID_VOID,   NPCDATAFROZENTILE,         0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getFrozenCSet",              0,         ZTID_FLOAT,   NPCDATAFROZENCSET,         0,  { ZTID_NPCDATA },{} },
	{ "setFrozenCSet",              0,          ZTID_VOID,   NPCDATAFROZENCSET,         0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getScript",                  0,         ZTID_FLOAT,   NPCDATASCRIPT,             0,  { ZTID_NPCDATA },{} },
	{ "setScript",                  0,          ZTID_VOID,   NPCDATASCRIPT,             0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getInitD[]",                 0,         ZTID_FLOAT,   NPCDATAINITD,              0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "setInitD[]",                 0,          ZTID_VOID,   NPCDATAINITD,              0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getWeaponScript",            0,         ZTID_FLOAT,   NPCDATAWEAPONSCRIPT,       0,  { ZTID_NPCDATA },{} },
	{ "setWeaponScript",            0,          ZTID_VOID,   NPCDATAWEAPONSCRIPT,       0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getWeaponInitD[]",           0,         ZTID_FLOAT,   NPCDATAWEAPONINITD,        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "setWeaponInitD[]",           0,          ZTID_VOID,   NPCDATAWEAPONINITD,        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getShadowSprite",            0,         ZTID_FLOAT,   NPCDSHADOWSPR,             0,  { ZTID_NPCDATA },{} },
	{ "setShadowSprite",            0,          ZTID_VOID,   NPCDSHADOWSPR,             0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getSpawnSprite",             0,         ZTID_FLOAT,   NPCDSPAWNSPR,              0,  { ZTID_NPCDATA },{} },
	{ "setSpawnSprite",             0,          ZTID_VOID,   NPCDSPAWNSPR,              0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "getDeathSprite",             0,         ZTID_FLOAT,   NPCDDEATHSPR,              0,  { ZTID_NPCDATA },{} },
	{ "setDeathSprite",             0,          ZTID_VOID,   NPCDDEATHSPR,              0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	
//	{ "GetInitDLabel",              0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
//	{ "MatchInitDLabel",            0,          ZTID_BOOL,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetName",                    0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	
//{	Functions
//	one inout, no return
	{ "GetTile",                    0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetEHeight",                 0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetFlags",                   0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetFlags2",                  0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetWidth",                   0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetHeight",                  0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetSTile",                   0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetSWidth",                  0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetSHeight",                 0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetETile",                   0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetEWidth",                  0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetHP",                      0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetFamily",                  0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetCSet",                    0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetAnim",                    0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetEAnim",                   0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetFramerate",               0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetEFramerate",              0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetTouchDamage",             0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetWeaponDamage",            0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetWeapon",                  0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetRandom",                  0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetHaltRate",                0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetStep",                    0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetHoming",                  0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetHunger",                  0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetDropset",                 0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetBGSFX",                   0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetHitSFX",                  0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetDeathSFX",                0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetDrawXOffset",             0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetDrawYOffset",             0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetDrawZOffset",             0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetHitXOffset",              0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetHitYOffset",              0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetHitWidth",                0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetHitHeight",               0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetHitZHeight",              0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetTileWidth",               0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetTileHeight",              0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
	{ "GetWeaponSprite",            0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT },{} },
//	two inouts, one return

	{ "GetScriptDefense",           0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetDefense",                 0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetSizeFlag",                0,          ZTID_BOOL,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetAttribute",               0,         ZTID_FLOAT,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },

//	two inputs, no return
	{ "SetTile",                    0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetEHeight",                 0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetFlags",                   0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetFlags2",                  0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetWidth",                   0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetHeight",                  0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetSTile",                   0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetSWidth",                  0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetSHeight",                 0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetETile",                   0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetEWidth",                  0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetHP",                      0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetFamily",                  0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetCSet",                    0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetAnim",                    0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetEAnim",                   0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetFramerate",               0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetEFramerate",              0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetTouchDamage",             0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetWeaponDamage",            0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetWeapon",                  0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetRandom",                  0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetHaltRate",                0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetStep",                    0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetHoming",                  0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetHunger",                  0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetDropset",                 0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetBGSFX",                   0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetHitSFX",                  0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetDeathSFX",                0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetDrawXOffset",             0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetDrawYOffset",             0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetDrawZOffset",             0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetHitXOffset",              0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetHitYOffset",              0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetHitWidth",                0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetHitHeight",               0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetHitZHeight",              0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetTileWidth",               0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetTileHeight",              0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetWeaponSprite",            0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
//	two inputs, no return

	{ "SetScriptDefense",           0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetDefense",                 0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetSizeFlag",                0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "SetAttribute",               0,          ZTID_VOID,   -1,                        0,  { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
//}
	
	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
};

NPCDataSymbols::NPCDataSymbols()
{
	table = NPCDataTable;
	refVar = REFNPCCLASS;
}

void NPCDataSymbols::generateCode()
{
	//GetTile(NPCData, int32_t)
	{
		Function* function = getFunction("GetTile");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataBaseTile(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	
	//void GetName(npcdata, int32_t)
	{
		Function* function = getFunction("GetName");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OGetNPCDataName(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void GetInitDLabel(npc, int32_t buffer[], int32_t d)
	/* {
		Function* function = getFunction("GetInitDLabel");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OGetNPCDataInitDLabel(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//bool MatchInitDLabel(npc, "label", d)
	{
		Function* function = getFunction("MatchInitDLabel");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(NPCMATCHINITDLABEL)));
		RETURN();
		function->giveCode(code);
	}*/
	
	//GetEHeight(NPCData, int32_t)
	{
		Function* function = getFunction("GetEHeight");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		POPREF();
		addOpcode2 (code, new ONDataEHeight(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}

	//int32_t GetScriptDefense((NPCData, int32_t, int32_t)
	{
		Function* function = getFunction("GetScriptDefense");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataScriptDef(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t GetDefense(NPCData, int32_t, int32_t)
	{
		Function* function = getFunction("GetDefense");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataDefense(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t GetSizeFlag(NPCData, int32_t, int32_t)
	{
		Function* function = getFunction("GetSizeFlag");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataSizeFlag(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t GetAttribute(NPCData, int32_t, int32_t)
	{
		Function* function = getFunction("GetAttribute");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDatattributes(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	
	
	//three inputs, no return
	
	//void SetScriptDefense(NPCData, int32_t,int32_t,int32_t)
	{
		Function* function = getFunction("SetScriptDefense");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSetRegister(new VarArgument(SETNPCDATASCRIPTDEF), new VarArgument(SFTEMP)));
		RETURN();
		function->giveCode(code);
	}
	//three inputs, no return
	
	//void SetDefense(NPCData, int32_t,int32_t,int32_t)
	{
		Function* function = getFunction("SetDefense");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSetRegister(new VarArgument(SETNPCDATADEFENSE), new VarArgument(SFTEMP)));
		RETURN();
		function->giveCode(code);
	}
	//three inputs, no return
	
	//void SetSizeFlag(NPCData, int32_t,int32_t,int32_t)
	{
		Function* function = getFunction("SetSizeFlag");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSetRegister(new VarArgument(SETNPCDATASIZEFLAG), new VarArgument(SFTEMP)));
		RETURN();
		function->giveCode(code);
	}
	//three inputs, no return
	
	//void SetAttribute(NPCData, int32_t,int32_t,int32_t)
	{
		Function* function = getFunction("SetAttribute");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSetRegister(new VarArgument(SETNPCDATAATTRIBUTE), new VarArgument(SFTEMP)));
		RETURN();
		function->giveCode(code);
	}
	//GetFlags(NPCData, int32_t)
	{
		Function* function = getFunction("GetFlags");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		POPREF();
		addOpcode2 (code, new ONDataFlags(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	
	//GetFlags2(NPCData, int32_t)
	{
		Function* function = getFunction("GetFlags2");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataFlags2(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetWidth(game, int32_t)
	{
		Function* function = getFunction("GetWidth");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		POPREF();
		addOpcode2 (code, new ONDataWidth(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetHeight(NPCData, int32_t)
	{
		Function* function = getFunction("GetHeight");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		POPREF();
		addOpcode2 (code, new ONDataHeight(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetSTile(NPCData, int32_t)
	{
		Function* function = getFunction("GetSTile");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		POPREF();
		addOpcode2 (code, new ONDataTile(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetSWidth(NPCData, int32_t)
	{
		Function* function = getFunction("GetSWidth");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataSWidth(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetSHeight(NPCData, int32_t)
	{
		Function* function = getFunction("GetSHeight");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataSHeight(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetETile(NPCData, int32_t)
	{
		Function* function = getFunction("GetETile");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataETile(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetEWidth(NPCData, int32_t)
	{
		Function* function = getFunction("GetEWidth");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataEWidth(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetHP(NPCData, int32_t)
	{
		Function* function = getFunction("GetHP");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataHP(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetFamily(NPCData, int32_t)
	{
		Function* function = getFunction("GetFamily");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataFamily(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetCSet(NPCData, int32_t)
	{
		Function* function = getFunction("GetCSet");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataCSet(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//SetDMapIntro(NPCData, int32_t)
	{
		Function* function = getFunction("GetAnim");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataAnim(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetEAnim(NPCData, int32_t)
	{
		Function* function = getFunction("GetEAnim");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataEAnim(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetFramerate(NPCData, int32_t)
	{
		Function* function = getFunction("GetFramerate");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataFramerate(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetEFramerate(NPCData, int32_t)
	{
		Function* function = getFunction("GetEFramerate");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataEFramerate(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetTouchDamage(NPCData,, int32_t)
	{
		Function* function = getFunction("GetTouchDamage");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataTouchDamage(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetWeaponDamage(NPCData, int32_t)
	{
		Function* function = getFunction("GetWeaponDamage");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataWeaponDamage(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetWeapon(NPCData, int32_t)
	{
		Function* function = getFunction("GetWeapon");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataWeapon(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetRandom(NPCData, int32_t)
	{
		Function* function = getFunction("GetRandom");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataRandom(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetHaltRate(NPCData, int32_t)
	{
		Function* function = getFunction("GetHaltRate");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataHalt(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetStep(NPCData, int32_t)
	{
		Function* function = getFunction("GetStep");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataStep(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetHoming(NPCData, int32_t)
	{
		Function* function = getFunction("GetHoming");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataHoming(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetHunger(NPCData, int32_t)
	{
		Function* function = getFunction("GetHunger");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataHunger(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetDropset(NPCData, int32_t)
	{
		Function* function = getFunction("GetDropset");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataropset(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetBGSFX(NPCData, int32_t)
	{
		Function* function = getFunction("GetBGSFX");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataBGSound(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetHitSFX(NPCData, int32_t)
	{
		Function* function = getFunction("GetHitSFX");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataHitSound(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetDeathSFX(NPCData, int32_t)
	{
		Function* function = getFunction("GetDeathSFX");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF(); 
		addOpcode2 (code, new ONDataDeathSound(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetDrawXOffset(NPCData, int32_t)
	{
		Function* function = getFunction("GetDrawXOffset");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataXofs(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetDrawYOffset(NPCData, int32_t)
	{
		Function* function = getFunction("GetDrawYOffset");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataYofs(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetDrawZOffset(NPCData,int32_t)
	{
		Function* function = getFunction("GetDrawZOffset");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataZofs(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetHitXOffset(NPCData, int32_t)
	{
		Function* function = getFunction("GetHitXOffset");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataHitXOfs(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetHitYOffset(NPCData, int32_t)
	{
		Function* function = getFunction("GetHitYOffset");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataHYOfs(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetHitWidth(NPCData, int32_t)
	{
		Function* function = getFunction("GetHitWidth");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataHitWidth(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetHitHeight(NPCData, int32_t)
	{
		Function* function = getFunction("GetHitHeight");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataHitHeight(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetHitZHeight(NPCData, int32_t)
	{
		Function* function = getFunction("GetHitZHeight");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataHitZ(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetTileWidth(NPCData, int32_t)
	{
		Function* function = getFunction("GetTileWidth");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataTileWidth(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetTileHeight(NPCData, int32_t)
	{
		Function* function = getFunction("GetTileHeight");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataTileHeight(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetWeaponSprite(NPCData, int32_t)
	{
		Function* function = getFunction("GetWeaponSprite");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataWeapSprite(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//SetFlags(NPCData, int32_t, int32_t)
	{
		Function* function = getFunction("SetFlags");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new ONDataSetFlags(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//SetTile(NPCData, int32_t, int32_t)
	{
		Function* function = getFunction("SetTile");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new ONDataSetBaseTile(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//SetEHeight(NPCData, int32_t, int32_t)
	{
		Function* function = getFunction("SetEHeight");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new ONDataSetEHeight(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//SetFlags2(NPCData, int32_t, int32_t)
	{
		Function* function = getFunction("SetFlags2");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new ONDataSetFlags2(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//SetWidth(NPCData, int32_t, int32_t)
	{
		Function* function = getFunction("SetWidth");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new ONDataSetWidth(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//SetHeight(NPCData, int32_t, int32_t)
	{
		Function* function = getFunction("SetHeight");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new ONDataSetHeight(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//SetSTile(NPCData, int32_t, int32_t)
	{
		Function* function = getFunction("SetSTile");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new ONDataSetTile(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//SetSWidth(NPCData, int32_t, int32_t)
	{
		Function* function = getFunction("SetSWidth");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new ONDataSetSWidth(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//SetSHeight(NPCData, int32_t, int32_t)
	{
		Function* function = getFunction("SetSHeight");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new ONDataSetSHeight(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//SetETile(NPCData, int32_t, int32_t)
	{
		Function* function = getFunction("SetETile");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new ONDataSetETile(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//SetEWidth(NPCData, int32_t, int32_t)
	{
		Function* function = getFunction("SetEWidth");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new ONDataSetEWidth(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//SetHP(NPCData, int32_t, int32_t)
	{
		Function* function = getFunction("SetHP");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new ONDataSetHP(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//SetFamily(NPCData, int32_t, int32_t)
	{
		Function* function = getFunction("SetFamily");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new ONDataSetFamily(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//SetCSet(NPCData, int32_t, int32_t)
	{
		Function* function = getFunction("SetCSet");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new ONDataSetCSet(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//SetDMapIntro(NPCData, int32_t, int32_t)
	{
		Function* function = getFunction("SetAnim");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new ONDataSetAnim(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetEAnim");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new ONDataSetEAnim(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
}

