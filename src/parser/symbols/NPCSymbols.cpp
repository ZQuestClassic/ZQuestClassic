#include "SymbolDefs.h"

NPCSymbols NPCSymbols::singleton = NPCSymbols();

static AccessorTable npcTable[] =
{
//	  name,                     rettype,                  setorget,     var,                  numindex,      funcFlags,                            numParams,   params
	{ "getX",                   ZVARTYPEID_FLOAT,         GETTER,       NPCX,                 1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setX",                   ZVARTYPEID_VOID,          SETTER,       NPCX,                 1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFrame",               ZVARTYPEID_FLOAT,         GETTER,       NPCFRAME,                 1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFrame",               ZVARTYPEID_VOID,          SETTER,       NPCFRAME,                 1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Max",                    ZVARTYPEID_FLOAT,         GETTER,       SPRITEMAXNPC,                 1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Max",                    ZVARTYPEID_VOID,          SETTER,       SPRITEMAXNPC,                 1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getY",                   ZVARTYPEID_FLOAT,         GETTER,       NPCY,                 1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setY",                   ZVARTYPEID_VOID,          SETTER,       NPCY,                 1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getZ",                   ZVARTYPEID_FLOAT,         GETTER,       NPCZ,                 1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setZ",                   ZVARTYPEID_VOID,          SETTER,       NPCZ,                 1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getJump",                ZVARTYPEID_FLOAT,         GETTER,       NPCJUMP,              1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setJump",                ZVARTYPEID_VOID,          SETTER,       NPCJUMP,              1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDir",                 ZVARTYPEID_FLOAT,         GETTER,       NPCDIR,               1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDir",                 ZVARTYPEID_VOID,          SETTER,       NPCDIR,               1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSlideClock",                 ZVARTYPEID_FLOAT,         GETTER,       NPCSLIDECLK,               1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSlideClock",                 ZVARTYPEID_VOID,          SETTER,       NPCSLIDECLK,               1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFading",                 ZVARTYPEID_FLOAT,         GETTER,       NPCFADING,               1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFading",                 ZVARTYPEID_VOID,          SETTER,       NPCFADING,               1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRate",                ZVARTYPEID_FLOAT,         GETTER,       NPCRATE,              1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRate",                ZVARTYPEID_VOID,          SETTER,       NPCRATE,              1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHoming",              ZVARTYPEID_FLOAT,         GETTER,       NPCHOMING,            1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHoming",              ZVARTYPEID_VOID,          SETTER,       NPCHOMING,            1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStep",                ZVARTYPEID_FLOAT,         GETTER,       NPCSTEP,              1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStep",                ZVARTYPEID_VOID,          SETTER,       NPCSTEP,              1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getASpeed",              ZVARTYPEID_FLOAT,         GETTER,       NPCFRAMERATE,         1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setASpeed",              ZVARTYPEID_VOID,          SETTER,       NPCFRAMERATE,         1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHalt",            ZVARTYPEID_FLOAT,         GETTER,       NPCHALTCLK,          1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHalt",            ZVARTYPEID_VOID,          SETTER,       NPCHALTCLK,          1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMoveStatus",            ZVARTYPEID_FLOAT,         GETTER,       NPCMOVESTATUS,          1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMoveStatus",            ZVARTYPEID_VOID,          SETTER,       NPCMOVESTATUS,          1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHaltrate",            ZVARTYPEID_FLOAT,         GETTER,       NPCHALTRATE,          1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHaltrate",            ZVARTYPEID_VOID,          SETTER,       NPCHALTRATE,          1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRandom",            ZVARTYPEID_FLOAT,         GETTER,       NPCRANDOM,          1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRandom",            ZVARTYPEID_VOID,          SETTER,       NPCRANDOM,          1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawStyle",           ZVARTYPEID_FLOAT,         GETTER,       NPCDRAWTYPE,          1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawStyle",           ZVARTYPEID_VOID,          SETTER,       NPCDRAWTYPE,          1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHP",                  ZVARTYPEID_FLOAT,         GETTER,       NPCHP,                1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHP",                  ZVARTYPEID_VOID,          SETTER,       NPCHP,                1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getID",                  ZVARTYPEID_FLOAT,         GETTER,       NPCID,                1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setID",                  ZVARTYPEID_VOID,          SETTER,       NPCID,                1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getType",                ZVARTYPEID_FLOAT,         GETTER,       NPCTYPE,              1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setType",                ZVARTYPEID_VOID,          SETTER,       NPCTYPE,              1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFamily",              ZVARTYPEID_FLOAT,         GETTER,       NPCTYPE,              1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFamily",              ZVARTYPEID_VOID,          SETTER,       NPCTYPE,              1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDamage",              ZVARTYPEID_FLOAT,         GETTER,       NPCDP,                1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDamage",              ZVARTYPEID_VOID,          SETTER,       NPCDP,                1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWeaponDamage",        ZVARTYPEID_FLOAT,         GETTER,       NPCWDP,               1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWeaponDamage",        ZVARTYPEID_VOID,          SETTER,       NPCWDP,               1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTile",                ZVARTYPEID_FLOAT,         GETTER,       NPCTILE,              1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTile",                ZVARTYPEID_VOID,          SETTER,       NPCTILE,              1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScriptTile",          ZVARTYPEID_FLOAT,         GETTER,       NPCSCRIPTTILE,        1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScriptTile",          ZVARTYPEID_VOID,          SETTER,       NPCSCRIPTTILE,        1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScriptFlip",          ZVARTYPEID_FLOAT,         GETTER,       NPCSCRIPTFLIP,        1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScriptFlip",          ZVARTYPEID_VOID,          SETTER,       NPCSCRIPTFLIP,        1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getOriginalTile",        ZVARTYPEID_FLOAT,         GETTER,       NPCOTILE,             1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setOriginalTile",        ZVARTYPEID_VOID,          SETTER,       NPCOTILE,             1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWeapon",              ZVARTYPEID_FLOAT,         GETTER,       NPCWEAPON,            1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWeapon",              ZVARTYPEID_VOID,          SETTER,       NPCWEAPON,            1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getItemSet",             ZVARTYPEID_FLOAT,         GETTER,       NPCITEMSET,           1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItemSet",             ZVARTYPEID_VOID,          SETTER,       NPCITEMSET,           1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDropset",             ZVARTYPEID_FLOAT,         GETTER,       NPCITEMSET,           1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDropset",             ZVARTYPEID_VOID,          SETTER,       NPCITEMSET,           1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCSet",                ZVARTYPEID_FLOAT,         GETTER,       NPCCSET,              1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCSet",                ZVARTYPEID_VOID,          SETTER,       NPCCSET,              1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getBossPal",             ZVARTYPEID_FLOAT,         GETTER,       NPCBOSSPAL,           1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setBossPal",             ZVARTYPEID_VOID,          SETTER,       NPCBOSSPAL,           1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSFX",                 ZVARTYPEID_FLOAT,         GETTER,       NPCBGSFX,             1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSFX",                 ZVARTYPEID_VOID,          SETTER,       NPCBGSFX,             1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getExtend",              ZVARTYPEID_FLOAT,         GETTER,       NPCEXTEND,            1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setExtend",              ZVARTYPEID_VOID,          SETTER,       NPCEXTEND,            1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitWidth",            ZVARTYPEID_FLOAT,         GETTER,       NPCHXSZ,              1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitWidth",            ZVARTYPEID_VOID,          SETTER,       NPCHXSZ,              1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitHeight",           ZVARTYPEID_FLOAT,         GETTER,       NPCHYSZ,              1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitHeight",           ZVARTYPEID_VOID,          SETTER,       NPCHYSZ,              1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitZHeight",          ZVARTYPEID_FLOAT,         GETTER,       NPCHZSZ,              1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitZHeight",          ZVARTYPEID_VOID,          SETTER,       NPCHZSZ,              1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTileWidth",           ZVARTYPEID_FLOAT,         GETTER,       NPCTXSZ,              1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWidth",           ZVARTYPEID_VOID,          SETTER,       NPCTXSZ,              1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTileHeight",          ZVARTYPEID_FLOAT,         GETTER,       NPCTYSZ,              1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileHeight",          ZVARTYPEID_VOID,          SETTER,       NPCTYSZ,              1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawXOffset",         ZVARTYPEID_FLOAT,         GETTER,       NPCXOFS,              1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawXOffset",         ZVARTYPEID_VOID,          SETTER,       NPCXOFS,              1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawYOffset",         ZVARTYPEID_FLOAT,         GETTER,       NPCYOFS,              1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawYOffset",         ZVARTYPEID_VOID,          SETTER,       NPCYOFS,              1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTotalDYOffset",       ZVARTYPEID_FLOAT,         GETTER,       NPCTOTALDYOFFS,       1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTotalDYOffset",       ZVARTYPEID_VOID,          SETTER,       NPCTOTALDYOFFS,       1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawZOffset",         ZVARTYPEID_FLOAT,         GETTER,       NPCZOFS,              1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawZOffset",         ZVARTYPEID_VOID,          SETTER,       NPCZOFS,              1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitXOffset",          ZVARTYPEID_FLOAT,         GETTER,       NPCHXOFS,             1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitXOffset",          ZVARTYPEID_VOID,          SETTER,       NPCHXOFS,             1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitYOffset",          ZVARTYPEID_FLOAT,         GETTER,       NPCHYOFS,             1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitYOffset",          ZVARTYPEID_VOID,          SETTER,       NPCHYOFS,             1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "isValid",                ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Explode",                ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getMisc[]",              ZVARTYPEID_UNTYPED,       GETTER,       NPCMISCD,             32,            0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMisc[]",              ZVARTYPEID_VOID,          SETTER,       NPCMISCD,             32,            0,                                    3,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getInitD[]",             ZVARTYPEID_UNTYPED,       GETTER,       NPCINITD,             8,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInitD[]",             ZVARTYPEID_VOID,          SETTER,       NPCINITD,             8,             0,                                    3,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAttributes[]",        ZVARTYPEID_UNTYPED,       GETTER,       NPCDD,                32,            0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAttributes[]",        ZVARTYPEID_VOID,          SETTER,       NPCDD,                32,            0,                                    3,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMiscFlags",           ZVARTYPEID_FLOAT,         GETTER,       NPCMFLAGS,            1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMiscFlags",           ZVARTYPEID_VOID,          SETTER,       NPCMFLAGS,            1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCollDetection",       ZVARTYPEID_BOOL,          GETTER,       NPCCOLLDET,           1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCollDetection",       ZVARTYPEID_VOID,          SETTER,       NPCCOLLDET,           1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getGravity",       ZVARTYPEID_BOOL,          GETTER,       NPCGRAVITY,           1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setGravity",       ZVARTYPEID_VOID,          SETTER,       NPCGRAVITY,           1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAnimation",           ZVARTYPEID_BOOL,          GETTER,       NPCENGINEANIMATE,     1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAnimation",           ZVARTYPEID_VOID,          SETTER,       NPCENGINEANIMATE,     1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Submerged",                ZVARTYPEID_BOOL,          GETTER,       NPCSUBMERGED,              1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStun",                ZVARTYPEID_FLOAT,         GETTER,       NPCSTUN,              1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStun",                ZVARTYPEID_VOID,          SETTER,       NPCSTUN,              1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCore",                ZVARTYPEID_BOOL,          GETTER,       NPCISCORE,            1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCore",                ZVARTYPEID_VOID,          SETTER,       NPCISCORE,            1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDefense[]",           ZVARTYPEID_FLOAT,         GETTER,       NPCDEFENSED,          42,            0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDefense[]",           ZVARTYPEID_VOID,          SETTER,       NPCDEFENSED,          42,            0,                                    3,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitBy[]",             ZVARTYPEID_UNTYPED,       GETTER,       NPCHITBY,             16,            0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitBy[]",             ZVARTYPEID_VOID,          SETTER,       NPCHITBY,             16,            0,                                    3,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetName",                ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHunger",              ZVARTYPEID_FLOAT,         GETTER,       NPCHUNGER,            1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHunger",              ZVARTYPEID_VOID,          SETTER,       NPCHUNGER,            1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "BreakShield",            ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWeaponSprite",        ZVARTYPEID_FLOAT,         GETTER,       NPCWEAPSPRITE,        1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWeaponSprite",        ZVARTYPEID_VOID,          SETTER,       NPCWEAPSPRITE,        1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	2.future srit compatibility. -Z
	{ "getScriptDefense[]",     ZVARTYPEID_FLOAT,         GETTER,       NPCSCRDEFENSED,       10,            0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScriptDefense[]",     ZVARTYPEID_VOID,          SETTER,       NPCSCRDEFENSED,       10,            0,                                    3,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	
	{ "getInvFrames",           ZVARTYPEID_FLOAT,         GETTER,       NPCINVINC,            1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInvFrames",           ZVARTYPEID_VOID,          SETTER,       NPCINVINC,            1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getInvincible",          ZVARTYPEID_FLOAT,         GETTER,       NPCSUPERMAN,          1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInvincible",          ZVARTYPEID_VOID,          SETTER,       NPCSUPERMAN,          1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRingleader",          ZVARTYPEID_BOOL,          GETTER,       NPCRINGLEAD,          1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRingleader",          ZVARTYPEID_VOID,          SETTER,       NPCRINGLEAD,          1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHasItem",             ZVARTYPEID_BOOL,          GETTER,       NPCHASITEM,           1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHasItem",             ZVARTYPEID_VOID,          SETTER,       NPCHASITEM,           1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShield[]",            ZVARTYPEID_BOOL,          GETTER,       NPCSHIELD,            5,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShield[]",            ZVARTYPEID_VOID,          SETTER,       NPCSHIELD,            5,             0,                                    3,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getBFlags[]",            ZVARTYPEID_BOOL,          GETTER,       NPCBEHAVIOUR,         16,            0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setBFlags[]",            ZVARTYPEID_VOID,          SETTER,       NPCBEHAVIOUR,         16,            0,                                    3,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFrozen",              ZVARTYPEID_FLOAT,         GETTER,       NPCFROZEN,            1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFrozen",              ZVARTYPEID_VOID,          SETTER,       NPCFROZEN,            1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getUID",                 ZVARTYPEID_FLOAT,         GETTER,       NPCSCRIPTUID,         1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "setUID",                 ZVARTYPEID_VOID,          SETTER,       NPCSCRIPTUID,         1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFrozenTile",          ZVARTYPEID_FLOAT,         GETTER,       NPCFROZENTILE,        1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFrozenTile",          ZVARTYPEID_VOID,          SETTER,       NPCFROZENTILE,        1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getParentUID",          ZVARTYPEID_FLOAT,         GETTER,       NPCPARENTUID,        1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setParentUID",          ZVARTYPEID_VOID,          SETTER,       NPCPARENTUID,        1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFrozenCSet",          ZVARTYPEID_FLOAT,         GETTER,       NPCFROZENCSET,        1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFrozenCSet",          ZVARTYPEID_VOID,          SETTER,       NPCFROZENCSET,        1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitDir",              ZVARTYPEID_FLOAT,         GETTER,       NPCHITDIR,            1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitDir",          ZVARTYPEID_VOID,          SETTER,       NPCHITDIR,        1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScript",              ZVARTYPEID_FLOAT,         GETTER,       NPCSCRIPT,            1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScript",              ZVARTYPEID_VOID,          SETTER,       NPCSCRIPT,            1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "isDead",                 ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "CanSlide",               ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Slide",                  ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Remove",                 ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "StopBGSFX",              ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Attack",                 ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "NewDir",                 ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "ConstantWalk",           ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "ConstantWalk8",          ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "VariableWalk",           ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "VariableWalk8",          ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "HaltingWalk",            ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "HaltingWalk8",           ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "FloatingWalk",           ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "BreathAttack",           ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_NPC, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "NewDir8",                ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Collision",              ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LinedUp",                ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LinkInRange",            ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "CanMove",                ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SimulateHit",            ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Create",                 ZVARTYPEID_NPC,           FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRotation",            ZVARTYPEID_FLOAT,         GETTER,       NPCROTATION,          1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRotation",            ZVARTYPEID_VOID,          SETTER,       NPCROTATION,          1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScale",               ZVARTYPEID_FLOAT,         GETTER,       NPCSCALE,             1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScale",               ZVARTYPEID_VOID,          SETTER,       NPCSCALE,             1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getImmortal",            ZVARTYPEID_BOOL,          GETTER,       NPCIMMORTAL,          1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setImmortal",            ZVARTYPEID_VOID,          SETTER,       NPCIMMORTAL,          1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getNoSlide",             ZVARTYPEID_BOOL,          GETTER,       NPCNOSLIDE,           1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNoSlide",             ZVARTYPEID_VOID,          SETTER,       NPCNOSLIDE,           1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getNoScriptKnockback",   ZVARTYPEID_BOOL,          GETTER,       NPCNOSCRIPTKB,        1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNoScriptKnockback",   ZVARTYPEID_VOID,          SETTER,       NPCNOSCRIPTKB,        1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSlideSpeed",          ZVARTYPEID_FLOAT,         GETTER,       NPCKNOCKBACKSPEED,    1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSlideSpeed",          ZVARTYPEID_VOID,          SETTER,       NPCKNOCKBACKSPEED,    1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Knockback",              ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,             0,                                    4,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFalling",             ZVARTYPEID_FLOAT,         GETTER,       NPCFALLCLK,          1,           0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFalling",             ZVARTYPEID_VOID,          SETTER,       NPCFALLCLK,          1,           0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFallCombo",           ZVARTYPEID_FLOAT,         GETTER,       NPCFALLCMB,          1,           0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFallCombo",           ZVARTYPEID_VOID,          SETTER,       NPCFALLCMB,          1,           0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMoveFlags[]",         ZVARTYPEID_BOOL,          GETTER,       NPCMOVEFLAGS,        8,           0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMoveFlags[]",         ZVARTYPEID_VOID,          SETTER,       NPCMOVEFLAGS,        8,           0,                                    3,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLightRadius",         ZVARTYPEID_FLOAT,         GETTER,       NPCGLOWRAD,           1,           0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLightRadius",         ZVARTYPEID_VOID,          SETTER,       NPCGLOWRAD,           1,           0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLightShape",          ZVARTYPEID_FLOAT,         GETTER,       NPCGLOWSHP,           1,           0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLightShape",          ZVARTYPEID_VOID,          SETTER,       NPCGLOWSHP,           1,           0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShadowSprite",        ZVARTYPEID_FLOAT,         GETTER,       NPCSHADOWSPR,         1,           0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShadowSprite",        ZVARTYPEID_VOID,          SETTER,       NPCSHADOWSPR,         1,           0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSpawnSprite",         ZVARTYPEID_FLOAT,         GETTER,       NPCSPAWNSPR,          1,           0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSpawnSprite",         ZVARTYPEID_VOID,          SETTER,       NPCSPAWNSPR,          1,           0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDeathSprite",         ZVARTYPEID_FLOAT,         GETTER,       NPCDEATHSPR,          1,           0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDeathSprite",         ZVARTYPEID_VOID,          SETTER,       NPCDEATHSPR,          1,           0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSwitchHooked",        ZVARTYPEID_BOOL,          GETTER,       NPCSWHOOKED,          1,             0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSwitchHooked",        ZVARTYPEID_VOID,          SETTER,       NPCSWHOOKED,          1,             0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Switch",                 ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MovePaused",             ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Move",                   ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      4,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MoveAtAngle",            ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      4,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MoveXY",                 ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      4,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "CanMove",                ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      4,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "CanMoveAtAngle",         ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      4,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "CanMoveXY",              ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      4,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInvFlicker",          ZVARTYPEID_VOID,          SETTER,       NPCCANFLICKER,        1,           0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getInvFlicker",          ZVARTYPEID_BOOL,          GETTER,       NPCCANFLICKER,        1,           0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrowning",             ZVARTYPEID_FLOAT,         GETTER,       NPCDROWNCLK,          1,           0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrowning",             ZVARTYPEID_VOID,          SETTER,       NPCDROWNCLK,          1,           0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrownCombo",           ZVARTYPEID_FLOAT,         GETTER,       NPCDROWNCMB,          1,           0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrownCombo",           ZVARTYPEID_VOID,          SETTER,       NPCDROWNCMB,          1,           0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFakeZ",           ZVARTYPEID_FLOAT,         GETTER,       NPCFAKEZ,          1,           0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFakeZ",           ZVARTYPEID_VOID,          SETTER,       NPCFAKEZ,          1,           0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFakeJump",           ZVARTYPEID_FLOAT,         GETTER,       NPCFAKEJUMP,          1,           0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFakeJump",           ZVARTYPEID_VOID,          SETTER,       NPCFAKEJUMP,          1,           0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShadowXOffset",           ZVARTYPEID_FLOAT,         GETTER,       NPCSHADOWXOFS,          1,           0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShadowXOffset",           ZVARTYPEID_VOID,          SETTER,       NPCSHADOWXOFS,          1,           0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShadowYOffset",           ZVARTYPEID_FLOAT,         GETTER,       NPCSHADOWYOFS,          1,           0,                                    1,           { ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShadowYOffset",           ZVARTYPEID_VOID,          SETTER,       NPCSHADOWYOFS,          1,           0,                                    2,           { ZVARTYPEID_NPC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	
	{ "",                       -1,                       -1,           -1,                   -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

NPCSymbols::NPCSymbols()
{
    table = npcTable;
    refVar = REFNPC;
}

void NPCSymbols::generateCode()
{
	//bool isValid(npc)
	{
		Function* function = getFunction("isValid", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Check validity
		addOpcode2 (code, new OIsValidNPC(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void GetName(npc, int32_t)
	{
		Function* function = getFunction("GetName", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OGetNPCName(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void Explode(npc, int32_t)
	{
		    Function* function = getFunction("Explode", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCExplodeRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void BreakShield(npc)
	{
		Function* function = getFunction("BreakShield", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Break shield
		addOpcode2 (code, new OBreakShield(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//bool isDead(npc)
	{
		Function* function = getFunction("isDead", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Check validity
		addOpcode2 (code, new ONPCDead(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//bool CanSlide(npc)
	{
		Function* function = getFunction("CanSlide", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Check validity
		addOpcode2 (code, new ONPCCanSlide(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t Slide(npc)
	{
		Function* function = getFunction("Slide", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Check validity
		addOpcode2 (code, new ONPCSlide(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void Remove(npc)
	{
		Function* function = getFunction("Remove", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		POPREF();
		LABELBACK(label);
		//Break shield
		addOpcode2 (code, new ONPCRemove(new VarArgument(EXP1)));
		RETURN();
		 function->giveCode(code);
	}
	//void StopBGSFX(npc)
	{
		Function* function = getFunction("StopBGSFX", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Break shield
		addOpcode2 (code, new ONPCStopSFX(new VarArgument(EXP1)));
		RETURN();
		 function->giveCode(code);
	}
	//void Attack(npc)
	{
		Function* function = getFunction("Attack", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Break shield
		addOpcode2 (code, new ONPCAttack(new VarArgument(EXP1)));
		RETURN();
		 function->giveCode(code);
	}
	//void NewDir(int32_t arr[])
	{
		Function* function = getFunction("NewDir", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCNewDir(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void ConstantWalk(int32_t arr[])
	{
		Function* function = getFunction("ConstantWalk", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCConstWalk(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	
	//void ConstantWalk8(int32_t arr[])
	{
		Function* function = getFunction("ConstantWalk8", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCConstWalk8(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	
	//void VariableWalk(int32_t arr[])
	{
		Function* function = getFunction("VariableWalk", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCVarWalk(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void VariableWalk8(int32_t arr[])
	{
		Function* function = getFunction("VariableWalk8", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCVarWalk8(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void HaltingWalk(int32_t arr[])
	{
		Function* function = getFunction("HaltingWalk", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCHaltWalk(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void HaltingWalk8(int32_t arr[])
	{
		Function* function = getFunction("HaltingWalk8", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCHaltWalk8(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void FloatingWalk(int32_t arr[])
	{
		Function* function = getFunction("FloatingWalk", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCFloatWalk(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void BreathAttack(bool seeklink)
	{
		Function* function = getFunction("BreathAttack", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCBreatheFire(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void NewDir8(int32_t arr[])
	{
		Function* function = getFunction("NewDir8", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCNewDir8(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//bool Collision(int32_t obj_type, untyped obj_pointer)
	{
		Function* function = getFunction("Collision", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(NPCCOLLISION)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t LinedUp(int32_t range, bool dir8)
	{
		Function* function = getFunction("LinedUp", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(NPCLINEDUP)));
		RETURN();
		function->giveCode(code);
	}
	//bool HeroInRange(int32_t dist_in_pixels)
	{
		Function* function = getFunction("LinkInRange", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCHeroInRange(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
    
	//npc Create(int32_t array[])
	{
		Function* function = getFunction("Create", 2);
        
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCAdd(new VarArgument(EXP1)));
		REASSIGN_PTR(EXP2); //The value from ONPCAdd is placed in REFNPC, EXP1, and EXP2.
		RETURN();
		function->giveCode(code);
	}
	//bool CanMove(int32_t array[])
	{
		Function* function = getFunction("CanMove", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCCanMove(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//bool SimulateHit(int32_t array[])
	{
		Function* function = getFunction("SimulateHit", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCHitWith(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//bool Knockback(int32_t time, int32_t dir, int32_t spd)
	{
		Function* function = getFunction("Knockback", 4);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCKnockback(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
    //bool Switch(npc, int)
    {
        Function* function = getFunction("Switch", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer
        POPREF();
        addOpcode2 (code, new OSwitchNPC(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
	//bool MovePaused(npc)
	{
		Function* function = getFunction("MovePaused", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new ONPCMovePaused());
		RETURN();
		function->giveCode(code);
	}
	//bool Move(npc, int32_t, int32_t, int32_t)
	{
		Function* function = getFunction("Move", 4);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		POPREF();
		addOpcode2 (code, new ONPCMove());
		RETURN();
		function->giveCode(code);
	}
	//bool MoveAtAngle(npc, int32_t, int32_t, int32_t)
	{
		Function* function = getFunction("MoveAtAngle", 4);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		POPREF();
		addOpcode2 (code, new ONPCMoveAngle());
		RETURN();
		function->giveCode(code);
	}
	//bool MoveXY(npc, int32_t, int32_t, int32_t)
	{
		Function* function = getFunction("MoveXY", 4);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		POPREF();
		addOpcode2 (code, new ONPCMoveXY());
		RETURN();
		function->giveCode(code);
	}
	//bool CanMove(npc, int32_t, int32_t, int32_t)
	{
		Function* function = getFunction("CanMove", 4);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		POPREF();
		addOpcode2 (code, new ONPCCanMoveDir());
		RETURN();
		function->giveCode(code);
	}
	//bool CanMoveAtAngle(npc, int32_t, int32_t, int32_t)
	{
		Function* function = getFunction("CanMoveAtAngle", 4);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		POPREF();
		addOpcode2 (code, new ONPCCanMoveAngle());
		RETURN();
		function->giveCode(code);
	}
	//bool CanMoveXY(npc, int32_t, int32_t, int32_t)
	{
		Function* function = getFunction("CanMoveXY", 4);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		POPREF();
		addOpcode2 (code, new ONPCCanMoveXY());
		RETURN();
		function->giveCode(code);
	}
}

