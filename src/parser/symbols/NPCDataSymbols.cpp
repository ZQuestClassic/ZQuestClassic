#include "SymbolDefs.h"

NPCDataSymbols NPCDataSymbols::singleton = NPCDataSymbols();

static AccessorTable NPCDataTable[] =
{
//	  name,                       rettype,                 setorget,     var,                  numindex,      funcFlags,                            numParams,   params
//	datatype variables
	{ "getTile",                  ZVARTYPEID_FLOAT,        GETTER,       NPCDATATILE,          1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTile",                  ZVARTYPEID_VOID,         SETTER,       NPCDATATILE,          1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlags",                 ZVARTYPEID_FLOAT,        GETTER,       NPCDATAFLAGS,         1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlags",                 ZVARTYPEID_VOID,         SETTER,       NPCDATAFLAGS,         1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlags2",                ZVARTYPEID_FLOAT,        GETTER,       NPCDATAFLAGS2,        1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlags2",                ZVARTYPEID_VOID,         SETTER,       NPCDATAFLAGS2,        1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWidth",                 ZVARTYPEID_FLOAT,        GETTER,       NPCDATAWIDTH,         1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWidth",                 ZVARTYPEID_VOID,         SETTER,       NPCDATAWIDTH,         1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHeight",                ZVARTYPEID_FLOAT,        GETTER,       NPCDATAHEIGHT,        1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHeight",                ZVARTYPEID_VOID,         SETTER,       NPCDATAHEIGHT,        1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSTile",                 ZVARTYPEID_FLOAT,        GETTER,       NPCDATASTILE,         1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSTile",                 ZVARTYPEID_VOID,         SETTER,       NPCDATASTILE,         1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSWidth",                ZVARTYPEID_FLOAT,        GETTER,       NPCDATASWIDTH,        1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSWidth",                ZVARTYPEID_VOID,         SETTER,       NPCDATASWIDTH,        1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSHeight",               ZVARTYPEID_FLOAT,        GETTER,       NPCDATASHEIGHT,       1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSHeight",               ZVARTYPEID_VOID,         SETTER,       NPCDATASHEIGHT,       1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getExTile",                ZVARTYPEID_FLOAT,        GETTER,       NPCDATAETILE,         1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setExTile",                ZVARTYPEID_VOID,         SETTER,       NPCDATAETILE,         1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getExWidth",               ZVARTYPEID_FLOAT,        GETTER,       NPCDATAEWIDTH,        1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setExWidth",               ZVARTYPEID_VOID,         SETTER,       NPCDATAEWIDTH,        1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getExHeight",              ZVARTYPEID_FLOAT,        GETTER,       NPCDATAEHEIGHT,       1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setExHeight",              ZVARTYPEID_VOID,         SETTER,       NPCDATAEHEIGHT,       1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHP",                    ZVARTYPEID_FLOAT,        GETTER,       NPCDATAHP,            1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHP",                    ZVARTYPEID_VOID,         SETTER,       NPCDATAHP,            1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFamily",                ZVARTYPEID_FLOAT,        GETTER,       NPCDATAFAMILY,        1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFamily",                ZVARTYPEID_VOID,         SETTER,       NPCDATAFAMILY,        1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCSet",                  ZVARTYPEID_FLOAT,        GETTER,       NPCDATACSET,          1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCSet",                  ZVARTYPEID_VOID,         SETTER,       NPCDATACSET,          1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAnim",                  ZVARTYPEID_FLOAT,        GETTER,       NPCDATAANIM,          1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAnim",                  ZVARTYPEID_VOID,         SETTER,       NPCDATAANIM,          1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getExAnim",                ZVARTYPEID_FLOAT,        GETTER,       NPCDATAEANIM,         1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setExAnim",                ZVARTYPEID_VOID,         SETTER,       NPCDATAEANIM,         1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFramerate",             ZVARTYPEID_FLOAT,        GETTER,       NPCDATAFRAMERATE,     1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFramerate",             ZVARTYPEID_VOID,         SETTER,       NPCDATAFRAMERATE,     1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getExFramerate",           ZVARTYPEID_FLOAT,        GETTER,       NPCDATAEFRAMERATE,    1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setExFramerate",           ZVARTYPEID_VOID,         SETTER,       NPCDATAEFRAMERATE,    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTouchDamage",           ZVARTYPEID_FLOAT,        GETTER,       NPCDATATOUCHDAMAGE,   1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTouchDamage",           ZVARTYPEID_VOID,         SETTER,       NPCDATATOUCHDAMAGE,   1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWeaponDamage",          ZVARTYPEID_FLOAT,        GETTER,       NPCDATAWEAPONDAMAGE,  1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWeaponDamage",          ZVARTYPEID_VOID,         SETTER,       NPCDATAWEAPONDAMAGE,  1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWeapon",                ZVARTYPEID_FLOAT,        GETTER,       NPCDATAWEAPON,        1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWeapon",                ZVARTYPEID_VOID,         SETTER,       NPCDATAWEAPON,        1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRandom",                ZVARTYPEID_FLOAT,        GETTER,       NPCDATARANDOM,        1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRandom",                ZVARTYPEID_VOID,         SETTER,       NPCDATARANDOM,        1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHaltrate",              ZVARTYPEID_FLOAT,        GETTER,       NPCDATAHALT,          1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHaltrate",              ZVARTYPEID_VOID,         SETTER,       NPCDATAHALT,          1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStep",                  ZVARTYPEID_FLOAT,        GETTER,       NPCDATASTEP,          1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStep",                  ZVARTYPEID_VOID,         SETTER,       NPCDATASTEP,          1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHoming",                ZVARTYPEID_FLOAT,        GETTER,       NPCDATAHOMING,        1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHoming",                ZVARTYPEID_VOID,         SETTER,       NPCDATAHOMING,        1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHunger",                ZVARTYPEID_FLOAT,        GETTER,       NPCDATAHUNGER,        1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHunger",                ZVARTYPEID_VOID,         SETTER,       NPCDATAHUNGER,        1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDropset",               ZVARTYPEID_FLOAT,        GETTER,       NPCDATADROPSET,       1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDropset",               ZVARTYPEID_VOID,         SETTER,       NPCDATADROPSET,       1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getBGSFX",                 ZVARTYPEID_FLOAT,        GETTER,       NPCDATABGSFX,         1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setBGSFX",                 ZVARTYPEID_VOID,         SETTER,       NPCDATABGSFX,         1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDeathSFX",              ZVARTYPEID_FLOAT,        GETTER,       NPCDATADEATHSFX,      1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDeathSFX",              ZVARTYPEID_VOID,         SETTER,       NPCDATADEATHSFX,      1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitSFX",                ZVARTYPEID_FLOAT,        GETTER,       NPCDATAHITSFX,        1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitSFX",                ZVARTYPEID_VOID,         SETTER,       NPCDATAHITSFX,        1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "getFireSFX",               ZVARTYPEID_FLOAT,        GETTER,       SPRITEDATATYPE,       1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "setFireSFX",               ZVARTYPEID_VOID,         SETTER,       SPRITEDATATYPE,       1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawXOffset",           ZVARTYPEID_FLOAT,        GETTER,       NPCDATAXOFS,          1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawXOffset",           ZVARTYPEID_VOID,         SETTER,       NPCDATAXOFS,          1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawYOffset",           ZVARTYPEID_FLOAT,        GETTER,       NPCDATAYOFS,          1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawYOffset",           ZVARTYPEID_VOID,         SETTER,       NPCDATAYOFS,          1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawZOffset",           ZVARTYPEID_FLOAT,        GETTER,       NPCDATAZOFS,          1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawZOffset",           ZVARTYPEID_VOID,         SETTER,       NPCDATAZOFS,          1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitXOffset",            ZVARTYPEID_FLOAT,        GETTER,       NPCDATAHXOFS,         1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitXOffset",            ZVARTYPEID_VOID,         SETTER,       NPCDATAHXOFS,         1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitYOffset",            ZVARTYPEID_FLOAT,        GETTER,       NPCDATAHYOFS,         1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitYOffset",            ZVARTYPEID_VOID,         SETTER,       NPCDATAHYOFS,         1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitWidth",              ZVARTYPEID_FLOAT,        GETTER,       NPCDATAHITWIDTH,      1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitWidth",              ZVARTYPEID_VOID,         SETTER,       NPCDATAHITWIDTH,      1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitHeight",             ZVARTYPEID_FLOAT,        GETTER,       NPCDATAHITHEIGHT,     1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitHeight",             ZVARTYPEID_VOID,         SETTER,       NPCDATAHITHEIGHT,     1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitZHeight",            ZVARTYPEID_FLOAT,        GETTER,       NPCDATAHITZ,          1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitZHeight",            ZVARTYPEID_VOID,         SETTER,       NPCDATAHITZ,          1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTileWidth",             ZVARTYPEID_FLOAT,        GETTER,       NPCDATATILEWIDTH,     1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWidth",             ZVARTYPEID_VOID,         SETTER,       NPCDATATILEWIDTH,     1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTileHeight",            ZVARTYPEID_FLOAT,        GETTER,       NPCDATATILEHEIGHT,    1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileHeight",            ZVARTYPEID_VOID,         SETTER,       NPCDATATILEHEIGHT,    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWeaponSprite",          ZVARTYPEID_FLOAT,        GETTER,       NPCDATAWPNSPRITE,     1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWeaponSprite",          ZVARTYPEID_VOID,         SETTER,       NPCDATAWPNSPRITE,     1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDefense[]",             ZVARTYPEID_FLOAT,        GETTER,       NPCDATADEFENSE,       42,            0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDefense[]",             ZVARTYPEID_VOID,         SETTER,       NPCDATADEFENSE,       42,            0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSizeFlag[]",            ZVARTYPEID_BOOL,         GETTER,       NPCDATASIZEFLAG,      2,             0,                                    2,           { ZVARTYPEID_FFC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSizeFlag[]",            ZVARTYPEID_VOID,         SETTER,       NPCDATASIZEFLAG,      2,             0,                                    3,           { ZVARTYPEID_FFC, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAttributes[]",          ZVARTYPEID_FLOAT,        GETTER,       NPCDATAATTRIBUTE,     32,            0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAttributes[]",          ZVARTYPEID_VOID,         SETTER,       NPCDATAATTRIBUTE,     32,            0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShield[]",              ZVARTYPEID_BOOL,         GETTER,       NPCDATASHIELD,        5,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShield[]",              ZVARTYPEID_VOID,         SETTER,       NPCDATASHIELD,        5,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getBFlags[]",              ZVARTYPEID_BOOL,         GETTER,       NPCDATABEHAVIOUR,     16,            0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setBFlags[]",              ZVARTYPEID_VOID,         SETTER,       NPCDATABEHAVIOUR,     16,            0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFrozenTile",            ZVARTYPEID_FLOAT,        GETTER,       NPCDATAFROZENTILE,    1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFrozenTile",            ZVARTYPEID_VOID,         SETTER,       NPCDATAFROZENTILE,    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFrozenCSet",            ZVARTYPEID_FLOAT,        GETTER,       NPCDATAFROZENCSET,    1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFrozenCSet",            ZVARTYPEID_VOID,         SETTER,       NPCDATAFROZENCSET,    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScript",                ZVARTYPEID_FLOAT,        GETTER,       NPCDATASCRIPT,        1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScript",                ZVARTYPEID_VOID,         SETTER,       NPCDATASCRIPT,        1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getInitD[]",               ZVARTYPEID_FLOAT,        GETTER,       NPCDATAINITD,         8,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInitD[]",               ZVARTYPEID_VOID,         SETTER,       NPCDATAINITD,         8,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWeaponScript",          ZVARTYPEID_FLOAT,        GETTER,       NPCDATAWEAPONSCRIPT,  1,             0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWeaponScript",          ZVARTYPEID_VOID,         SETTER,       NPCDATAWEAPONSCRIPT,  1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWeaponInitD[]",         ZVARTYPEID_FLOAT,        GETTER,       NPCDATAWEAPONINITD,   8,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWeaponInitD[]",         ZVARTYPEID_VOID,         SETTER,       NPCDATAWEAPONINITD,   8,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getShadowSprite",        ZVARTYPEID_FLOAT,         GETTER,       NPCDSHADOWSPR,         1,           0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShadowSprite",        ZVARTYPEID_VOID,          SETTER,       NPCDSHADOWSPR,         1,           0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSpawnSprite",         ZVARTYPEID_FLOAT,         GETTER,       NPCDSPAWNSPR,          1,           0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSpawnSprite",         ZVARTYPEID_VOID,          SETTER,       NPCDSPAWNSPR,          1,           0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDeathSprite",         ZVARTYPEID_FLOAT,         GETTER,       NPCDDEATHSPR,          1,           0,                                    1,           { ZVARTYPEID_NPCDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDeathSprite",         ZVARTYPEID_VOID,          SETTER,       NPCDDEATHSPR,          1,           0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "GetInitDLabel",            ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MatchInitDLabel",          ZVARTYPEID_BOOL,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetName",                  ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
//{	Functions
//	one inout, no return
	{ "GetTile",                  ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetEHeight",               ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetFlags",                 ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetFlags2",                ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetWidth",                 ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetHeight",                ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetSTile",                 ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetSWidth",                ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetSHeight",               ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetETile",                 ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetEWidth",                ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetHP",                    ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetFamily",                ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetCSet",                  ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetAnim",                  ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetEAnim",                 ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetFramerate",             ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetEFramerate",            ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetTouchDamage",           ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetWeaponDamage",          ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetWeapon",                ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetRandom",                ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetHaltRate",              ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetStep",                  ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetHoming",                ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetHunger",                ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetDropset",               ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetBGSFX",                 ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetHitSFX",                ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetDeathSFX",              ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetDrawXOffset",           ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetDrawYOffset",           ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetDrawZOffset",           ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetHitXOffset",            ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetHitYOffset",            ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetHitWidth",              ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetHitHeight",             ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetHitZHeight",            ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetTileWidth",             ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetTileHeight",            ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetWeaponSprite",          ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	two inouts, one return

	{ "GetScriptDefense",         ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetDefense",               ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetSizeFlag",              ZVARTYPEID_BOOL,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetAttribute",             ZVARTYPEID_FLOAT,        FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },

//	two inputs, no return
	{ "SetTile",                  ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetEHeight",               ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetFlags",                 ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetFlags2",                ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetWidth",                 ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetHeight",                ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetSTile",                 ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetSWidth",                ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetSHeight",               ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetETile",                 ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetEWidth",                ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetHP",                    ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetFamily",                ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetCSet",                  ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetAnim",                  ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetEAnim",                 ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetFramerate",             ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetEFramerate",            ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetTouchDamage",           ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetWeaponDamage",          ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetWeapon",                ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetRandom",                ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetHaltRate",              ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetStep",                  ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetHoming",                ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetHunger",                ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetDropset",               ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetBGSFX",                 ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetHitSFX",                ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetDeathSFX",              ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetDrawXOffset",           ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetDrawYOffset",           ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetDrawZOffset",           ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetHitXOffset",            ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetHitYOffset",            ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetHitWidth",              ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetHitHeight",             ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetHitZHeight",            ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetTileWidth",             ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetTileHeight",            ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetWeaponSprite",          ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	two inputs, no return

	{ "SetScriptDefense",         ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    4,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetDefense",               ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    4,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetSizeFlag",              ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    4,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetAttribute",             ZVARTYPEID_VOID,         FUNCTION,     0,                    1,             0,                                    4,           { ZVARTYPEID_NPCDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
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

