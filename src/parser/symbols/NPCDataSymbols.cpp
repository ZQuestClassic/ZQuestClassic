#include "SymbolDefs.h"

NPCDataSymbols NPCDataSymbols::singleton = NPCDataSymbols();

static AccessorTable NPCDataTable[] =
{
//	  name,                       rettype,                 setorget,     var,                  numindex,      funcFlags,                            numParams,   params
//	datatype variables
	{ "getTile",                  ZTID_FLOAT,        GETTER,       NPCDATATILE,          1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTile",                  ZTID_VOID,         SETTER,       NPCDATATILE,          1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlags",                 ZTID_FLOAT,        GETTER,       NPCDATAFLAGS,         1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlags",                 ZTID_VOID,         SETTER,       NPCDATAFLAGS,         1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlags2",                ZTID_FLOAT,        GETTER,       NPCDATAFLAGS2,        1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlags2",                ZTID_VOID,         SETTER,       NPCDATAFLAGS2,        1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWidth",                 ZTID_FLOAT,        GETTER,       NPCDATAWIDTH,         1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWidth",                 ZTID_VOID,         SETTER,       NPCDATAWIDTH,         1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHeight",                ZTID_FLOAT,        GETTER,       NPCDATAHEIGHT,        1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHeight",                ZTID_VOID,         SETTER,       NPCDATAHEIGHT,        1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSTile",                 ZTID_FLOAT,        GETTER,       NPCDATASTILE,         1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSTile",                 ZTID_VOID,         SETTER,       NPCDATASTILE,         1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSWidth",                ZTID_FLOAT,        GETTER,       NPCDATASWIDTH,        1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSWidth",                ZTID_VOID,         SETTER,       NPCDATASWIDTH,        1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSHeight",               ZTID_FLOAT,        GETTER,       NPCDATASHEIGHT,       1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSHeight",               ZTID_VOID,         SETTER,       NPCDATASHEIGHT,       1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getExTile",                ZTID_FLOAT,        GETTER,       NPCDATAETILE,         1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setExTile",                ZTID_VOID,         SETTER,       NPCDATAETILE,         1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getExWidth",               ZTID_FLOAT,        GETTER,       NPCDATAEWIDTH,        1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setExWidth",               ZTID_VOID,         SETTER,       NPCDATAEWIDTH,        1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getExHeight",              ZTID_FLOAT,        GETTER,       NPCDATAEHEIGHT,       1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setExHeight",              ZTID_VOID,         SETTER,       NPCDATAEHEIGHT,       1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHP",                    ZTID_FLOAT,        GETTER,       NPCDATAHP,            1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHP",                    ZTID_VOID,         SETTER,       NPCDATAHP,            1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFamily",                ZTID_FLOAT,        GETTER,       NPCDATAFAMILY,        1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFamily",                ZTID_VOID,         SETTER,       NPCDATAFAMILY,        1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCSet",                  ZTID_FLOAT,        GETTER,       NPCDATACSET,          1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCSet",                  ZTID_VOID,         SETTER,       NPCDATACSET,          1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAnim",                  ZTID_FLOAT,        GETTER,       NPCDATAANIM,          1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAnim",                  ZTID_VOID,         SETTER,       NPCDATAANIM,          1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getExAnim",                ZTID_FLOAT,        GETTER,       NPCDATAEANIM,         1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setExAnim",                ZTID_VOID,         SETTER,       NPCDATAEANIM,         1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFramerate",             ZTID_FLOAT,        GETTER,       NPCDATAFRAMERATE,     1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFramerate",             ZTID_VOID,         SETTER,       NPCDATAFRAMERATE,     1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getExFramerate",           ZTID_FLOAT,        GETTER,       NPCDATAEFRAMERATE,    1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setExFramerate",           ZTID_VOID,         SETTER,       NPCDATAEFRAMERATE,    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTouchDamage",           ZTID_FLOAT,        GETTER,       NPCDATATOUCHDAMAGE,   1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTouchDamage",           ZTID_VOID,         SETTER,       NPCDATATOUCHDAMAGE,   1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWeaponDamage",          ZTID_FLOAT,        GETTER,       NPCDATAWEAPONDAMAGE,  1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWeaponDamage",          ZTID_VOID,         SETTER,       NPCDATAWEAPONDAMAGE,  1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWeapon",                ZTID_FLOAT,        GETTER,       NPCDATAWEAPON,        1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWeapon",                ZTID_VOID,         SETTER,       NPCDATAWEAPON,        1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRandom",                ZTID_FLOAT,        GETTER,       NPCDATARANDOM,        1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRandom",                ZTID_VOID,         SETTER,       NPCDATARANDOM,        1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHaltrate",              ZTID_FLOAT,        GETTER,       NPCDATAHALT,          1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHaltrate",              ZTID_VOID,         SETTER,       NPCDATAHALT,          1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStep",                  ZTID_FLOAT,        GETTER,       NPCDATASTEP,          1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStep",                  ZTID_VOID,         SETTER,       NPCDATASTEP,          1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHoming",                ZTID_FLOAT,        GETTER,       NPCDATAHOMING,        1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHoming",                ZTID_VOID,         SETTER,       NPCDATAHOMING,        1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHunger",                ZTID_FLOAT,        GETTER,       NPCDATAHUNGER,        1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHunger",                ZTID_VOID,         SETTER,       NPCDATAHUNGER,        1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDropset",               ZTID_FLOAT,        GETTER,       NPCDATADROPSET,       1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDropset",               ZTID_VOID,         SETTER,       NPCDATADROPSET,       1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getBGSFX",                 ZTID_FLOAT,        GETTER,       NPCDATABGSFX,         1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setBGSFX",                 ZTID_VOID,         SETTER,       NPCDATABGSFX,         1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDeathSFX",              ZTID_FLOAT,        GETTER,       NPCDATADEATHSFX,      1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDeathSFX",              ZTID_VOID,         SETTER,       NPCDATADEATHSFX,      1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitSFX",                ZTID_FLOAT,        GETTER,       NPCDATAHITSFX,        1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitSFX",                ZTID_VOID,         SETTER,       NPCDATAHITSFX,        1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "getFireSFX",               ZTID_FLOAT,        GETTER,       SPRITEDATATYPE,       1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "setFireSFX",               ZTID_VOID,         SETTER,       SPRITEDATATYPE,       1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawXOffset",           ZTID_FLOAT,        GETTER,       NPCDATAXOFS,          1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawXOffset",           ZTID_VOID,         SETTER,       NPCDATAXOFS,          1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawYOffset",           ZTID_FLOAT,        GETTER,       NPCDATAYOFS,          1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawYOffset",           ZTID_VOID,         SETTER,       NPCDATAYOFS,          1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawZOffset",           ZTID_FLOAT,        GETTER,       NPCDATAZOFS,          1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawZOffset",           ZTID_VOID,         SETTER,       NPCDATAZOFS,          1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitXOffset",            ZTID_FLOAT,        GETTER,       NPCDATAHXOFS,         1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitXOffset",            ZTID_VOID,         SETTER,       NPCDATAHXOFS,         1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitYOffset",            ZTID_FLOAT,        GETTER,       NPCDATAHYOFS,         1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitYOffset",            ZTID_VOID,         SETTER,       NPCDATAHYOFS,         1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitWidth",              ZTID_FLOAT,        GETTER,       NPCDATAHITWIDTH,      1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitWidth",              ZTID_VOID,         SETTER,       NPCDATAHITWIDTH,      1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitHeight",             ZTID_FLOAT,        GETTER,       NPCDATAHITHEIGHT,     1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitHeight",             ZTID_VOID,         SETTER,       NPCDATAHITHEIGHT,     1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitZHeight",            ZTID_FLOAT,        GETTER,       NPCDATAHITZ,          1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitZHeight",            ZTID_VOID,         SETTER,       NPCDATAHITZ,          1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTileWidth",             ZTID_FLOAT,        GETTER,       NPCDATATILEWIDTH,     1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWidth",             ZTID_VOID,         SETTER,       NPCDATATILEWIDTH,     1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTileHeight",            ZTID_FLOAT,        GETTER,       NPCDATATILEHEIGHT,    1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileHeight",            ZTID_VOID,         SETTER,       NPCDATATILEHEIGHT,    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWeaponSprite",          ZTID_FLOAT,        GETTER,       NPCDATAWPNSPRITE,     1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWeaponSprite",          ZTID_VOID,         SETTER,       NPCDATAWPNSPRITE,     1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDefense[]",             ZTID_FLOAT,        GETTER,       NPCDATADEFENSE,       42,            0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDefense[]",             ZTID_VOID,         SETTER,       NPCDATADEFENSE,       42,            0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSizeFlag[]",            ZTID_BOOL,         GETTER,       NPCDATASIZEFLAG,      2,             0,                                    2,           { ZTID_FFC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSizeFlag[]",            ZTID_VOID,         SETTER,       NPCDATASIZEFLAG,      2,             0,                                    3,           { ZTID_FFC, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAttributes[]",          ZTID_FLOAT,        GETTER,       NPCDATAATTRIBUTE,     32,            0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAttributes[]",          ZTID_VOID,         SETTER,       NPCDATAATTRIBUTE,     32,            0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShield[]",              ZTID_BOOL,         GETTER,       NPCDATASHIELD,        5,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShield[]",              ZTID_VOID,         SETTER,       NPCDATASHIELD,        5,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getBFlags[]",              ZTID_BOOL,         GETTER,       NPCDATABEHAVIOUR,     16,            0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setBFlags[]",              ZTID_VOID,         SETTER,       NPCDATABEHAVIOUR,     16,            0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFrozenTile",            ZTID_FLOAT,        GETTER,       NPCDATAFROZENTILE,    1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFrozenTile",            ZTID_VOID,         SETTER,       NPCDATAFROZENTILE,    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFrozenCSet",            ZTID_FLOAT,        GETTER,       NPCDATAFROZENCSET,    1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFrozenCSet",            ZTID_VOID,         SETTER,       NPCDATAFROZENCSET,    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScript",                ZTID_FLOAT,        GETTER,       NPCDATASCRIPT,        1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScript",                ZTID_VOID,         SETTER,       NPCDATASCRIPT,        1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getInitD[]",               ZTID_FLOAT,        GETTER,       NPCDATAINITD,         8,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInitD[]",               ZTID_VOID,         SETTER,       NPCDATAINITD,         8,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWeaponScript",          ZTID_FLOAT,        GETTER,       NPCDATAWEAPONSCRIPT,  1,             0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWeaponScript",          ZTID_VOID,         SETTER,       NPCDATAWEAPONSCRIPT,  1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWeaponInitD[]",         ZTID_FLOAT,        GETTER,       NPCDATAWEAPONINITD,   8,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWeaponInitD[]",         ZTID_VOID,         SETTER,       NPCDATAWEAPONINITD,   8,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getShadowSprite",        ZTID_FLOAT,         GETTER,       NPCDSHADOWSPR,         1,           0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShadowSprite",        ZTID_VOID,          SETTER,       NPCDSHADOWSPR,         1,           0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSpawnSprite",         ZTID_FLOAT,         GETTER,       NPCDSPAWNSPR,          1,           0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSpawnSprite",         ZTID_VOID,          SETTER,       NPCDSPAWNSPR,          1,           0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDeathSprite",         ZTID_FLOAT,         GETTER,       NPCDDEATHSPR,          1,           0,                                    1,           { ZTID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDeathSprite",         ZTID_VOID,          SETTER,       NPCDDEATHSPR,          1,           0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "GetInitDLabel",            ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MatchInitDLabel",          ZTID_BOOL,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetName",                  ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
//{	Functions
//	one inout, no return
	{ "GetTile",                  ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetEHeight",               ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetFlags",                 ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetFlags2",                ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetWidth",                 ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetHeight",                ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetSTile",                 ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetSWidth",                ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetSHeight",               ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetETile",                 ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetEWidth",                ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetHP",                    ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetFamily",                ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetCSet",                  ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetAnim",                  ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetEAnim",                 ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetFramerate",             ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetEFramerate",            ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetTouchDamage",           ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetWeaponDamage",          ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetWeapon",                ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetRandom",                ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetHaltRate",              ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetStep",                  ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetHoming",                ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetHunger",                ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetDropset",               ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetBGSFX",                 ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetHitSFX",                ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetDeathSFX",              ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetDrawXOffset",           ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetDrawYOffset",           ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetDrawZOffset",           ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetHitXOffset",            ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetHitYOffset",            ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetHitWidth",              ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetHitHeight",             ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetHitZHeight",            ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetTileWidth",             ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetTileHeight",            ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetWeaponSprite",          ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZTID_NPCDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	two inouts, one return

	{ "GetScriptDefense",         ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetDefense",               ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetSizeFlag",              ZTID_BOOL,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetAttribute",             ZTID_FLOAT,        FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },

//	two inputs, no return
	{ "SetTile",                  ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetEHeight",               ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetFlags",                 ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetFlags2",                ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetWidth",                 ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetHeight",                ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetSTile",                 ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetSWidth",                ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetSHeight",               ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetETile",                 ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetEWidth",                ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetHP",                    ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetFamily",                ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetCSet",                  ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetAnim",                  ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetEAnim",                 ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetFramerate",             ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetEFramerate",            ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetTouchDamage",           ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetWeaponDamage",          ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetWeapon",                ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetRandom",                ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetHaltRate",              ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetStep",                  ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetHoming",                ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetHunger",                ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetDropset",               ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetBGSFX",                 ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetHitSFX",                ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetDeathSFX",              ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetDrawXOffset",           ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetDrawYOffset",           ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetDrawZOffset",           ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetHitXOffset",            ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetHitYOffset",            ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetHitWidth",              ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetHitHeight",             ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetHitZHeight",            ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetTileWidth",             ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetTileHeight",            ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetWeaponSprite",          ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	two inputs, no return

	{ "SetScriptDefense",         ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    4,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetDefense",               ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    4,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetSizeFlag",              ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    4,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetAttribute",             ZTID_VOID,         FUNCTION,     0,                    1,             0,                                    4,           { ZTID_NPCDATA, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
/*
	Use functions that accept two args (ID, value) per field for setters
	use functions that accept one arg (ID) per field as getters
	Plus One master function to set all attriutes using an array?
*/   
	
//}
	
	{ "",                         -1,                      -1,           -1,                   -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

NPCDataSymbols::NPCDataSymbols()
{
	table = NPCDataTable;
	refVar = REFNPCCLASS; // NUL; //
}

void NPCDataSymbols::generateCode()
{
	//GetTile(NPCData, int32_t)
	{
	
		Function* function = getFunction("GetTile", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONDataBaseTile(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	
	//GET_GUYDATA_MEMBER("Tile", ONDataBaseTile);
	}
	
	//void GetName(npcdata, int32_t)
	{
		Function* function = getFunction("GetName", 2);
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
	 //void GetDMapMusicFilename(game, int32_t, int32_t)
	{
		Function* function = getFunction("GetInitDLabel", 3);
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
		Function* function = getFunction("MatchInitDLabel", 3);
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
	
	}
	
	//GetEHeight(NPCData, int32_t)
	 {
	GET_GUYDATA_MEMBER("GetEHeight", ONDataEHeight);
	 }
	

	//int32_t GetScriptDefense((NPCData, int32_t, int32_t)
	{
		Function* function = getFunction("GetScriptDefense", 3);
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
		Function* function = getFunction("GetDefense", 3);
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
		Function* function = getFunction("GetSizeFlag", 3);
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
		Function* function = getFunction("GetAttribute", 3);
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
		Function* function = getFunction("SetScriptDefense", 4);
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
		Function* function = getFunction("SetDefense", 4);
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
		Function* function = getFunction("SetSizeFlag", 4);
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
		Function* function = getFunction("SetAttribute", 4);
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
	GET_GUYDATA_MEMBER("GetFlags", ONDataFlags);
	}
	//GetFlags2(NPCData, int32_t)
	{
		Function* function = getFunction("GetFlags2", 2);
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
	GET_GUYDATA_MEMBER("GetWidth", ONDataWidth);
	}
	//GetHeight(NPCData, int32_t)
	{
	GET_GUYDATA_MEMBER("GetHeight", ONDataHeight);
	}
	//GetSTile(NPCData, int32_t)
	{
	GET_GUYDATA_MEMBER("GetSTile", ONDataTile);
	}
	//GetSWidth(NPCData, int32_t)
	{
		Function* function = getFunction("GetSWidth", 2);
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
		Function* function = getFunction("GetSHeight", 2);
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
		Function* function = getFunction("GetETile", 2);
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
		Function* function = getFunction("GetEWidth", 2);
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
		Function* function = getFunction("GetHP", 2);
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
		Function* function = getFunction("GetFamily", 2);
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
		Function* function = getFunction("GetCSet", 2);
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
		Function* function = getFunction("GetAnim", 2);
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
		Function* function = getFunction("GetEAnim", 2);
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
		Function* function = getFunction("GetFramerate", 2);
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
		Function* function = getFunction("GetEFramerate", 2);
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
		Function* function = getFunction("GetTouchDamage", 2);
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
		Function* function = getFunction("GetWeaponDamage", 2);
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
		Function* function = getFunction("GetWeapon", 2);
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
		Function* function = getFunction("GetRandom", 2);
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
		Function* function = getFunction("GetHaltRate", 2);
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
		Function* function = getFunction("GetStep", 2);
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
		Function* function = getFunction("GetHoming", 2);
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
		Function* function = getFunction("GetHunger", 2);
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
		Function* function = getFunction("GetDropset", 2);
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
		Function* function = getFunction("GetBGSFX", 2);
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
		Function* function = getFunction("GetHitSFX", 2);
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
		Function* function = getFunction("GetDeathSFX", 2);
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
		Function* function = getFunction("GetDrawXOffset", 2);
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
		Function* function = getFunction("GetDrawYOffset", 2);
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
		Function* function = getFunction("GetDrawZOffset", 2);
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
		Function* function = getFunction("GetHitXOffset", 2);
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
		Function* function = getFunction("GetHitYOffset", 2);
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
		Function* function = getFunction("GetHitWidth", 2);
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
		Function* function = getFunction("GetHitHeight", 2);
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
		Function* function = getFunction("GetHitZHeight", 2);
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
		Function* function = getFunction("GetTileWidth", 2);
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
		Function* function = getFunction("GetTileHeight", 2);
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
		Function* function = getFunction("GetWeaponSprite", 2);
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
		Function* function = getFunction("SetFlags", 3);
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
		Function* function = getFunction("SetTile", 3);
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
		Function* function = getFunction("SetEHeight", 3);
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
		Function* function = getFunction("SetFlags2", 3);
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
		Function* function = getFunction("SetWidth", 3);
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
		Function* function = getFunction("SetHeight", 3);
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
		Function* function = getFunction("SetSTile", 3);
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
		Function* function = getFunction("SetSWidth", 3);
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
		Function* function = getFunction("SetSHeight", 3);
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
		Function* function = getFunction("SetETile", 3);
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
		Function* function = getFunction("SetEWidth", 3);
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
		Function* function = getFunction("SetHP", 3);
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
		Function* function = getFunction("SetFamily", 3);
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
		Function* function = getFunction("SetCSet", 3);
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
		Function* function = getFunction("SetAnim", 3);
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
}

