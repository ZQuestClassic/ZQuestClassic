#include "SymbolDefs.h"

NPCSymbols NPCSymbols::singleton = NPCSymbols();

static AccessorTable npcTable[] =
{
//	  name,                     rettype,                  setorget,     var,                  numindex,      funcFlags,                            numParams,   params
	{ "getX",                   ZTID_FLOAT,         GETTER,       NPCX,                 1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setX",                   ZTID_VOID,          SETTER,       NPCX,                 1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFrame",               ZTID_FLOAT,         GETTER,       NPCFRAME,                 1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFrame",               ZTID_VOID,          SETTER,       NPCFRAME,                 1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Max",                    ZTID_FLOAT,         GETTER,       SPRITEMAXNPC,                 1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Max",                    ZTID_VOID,          SETTER,       SPRITEMAXNPC,                 1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getY",                   ZTID_FLOAT,         GETTER,       NPCY,                 1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setY",                   ZTID_VOID,          SETTER,       NPCY,                 1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getZ",                   ZTID_FLOAT,         GETTER,       NPCZ,                 1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setZ",                   ZTID_VOID,          SETTER,       NPCZ,                 1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getJump",                ZTID_FLOAT,         GETTER,       NPCJUMP,              1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setJump",                ZTID_VOID,          SETTER,       NPCJUMP,              1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDir",                 ZTID_FLOAT,         GETTER,       NPCDIR,               1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDir",                 ZTID_VOID,          SETTER,       NPCDIR,               1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSlideClock",                 ZTID_FLOAT,         GETTER,       NPCSLIDECLK,               1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSlideClock",                 ZTID_VOID,          SETTER,       NPCSLIDECLK,               1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFading",                 ZTID_FLOAT,         GETTER,       NPCFADING,               1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFading",                 ZTID_VOID,          SETTER,       NPCFADING,               1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRate",                ZTID_FLOAT,         GETTER,       NPCRATE,              1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRate",                ZTID_VOID,          SETTER,       NPCRATE,              1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHoming",              ZTID_FLOAT,         GETTER,       NPCHOMING,            1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHoming",              ZTID_VOID,          SETTER,       NPCHOMING,            1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStep",                ZTID_FLOAT,         GETTER,       NPCSTEP,              1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStep",                ZTID_VOID,          SETTER,       NPCSTEP,              1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getASpeed",              ZTID_FLOAT,         GETTER,       NPCFRAMERATE,         1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setASpeed",              ZTID_VOID,          SETTER,       NPCFRAMERATE,         1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHalt",            ZTID_FLOAT,         GETTER,       NPCHALTCLK,          1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHalt",            ZTID_VOID,          SETTER,       NPCHALTCLK,          1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMoveStatus",            ZTID_FLOAT,         GETTER,       NPCMOVESTATUS,          1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMoveStatus",            ZTID_VOID,          SETTER,       NPCMOVESTATUS,          1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHaltrate",            ZTID_FLOAT,         GETTER,       NPCHALTRATE,          1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHaltrate",            ZTID_VOID,          SETTER,       NPCHALTRATE,          1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRandom",            ZTID_FLOAT,         GETTER,       NPCRANDOM,          1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRandom",            ZTID_VOID,          SETTER,       NPCRANDOM,          1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawStyle",           ZTID_FLOAT,         GETTER,       NPCDRAWTYPE,          1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawStyle",           ZTID_VOID,          SETTER,       NPCDRAWTYPE,          1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHP",                  ZTID_FLOAT,         GETTER,       NPCHP,                1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHP",                  ZTID_VOID,          SETTER,       NPCHP,                1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getID",                  ZTID_FLOAT,         GETTER,       NPCID,                1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setID",                  ZTID_VOID,          SETTER,       NPCID,                1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getType",                ZTID_FLOAT,         GETTER,       NPCTYPE,              1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setType",                ZTID_VOID,          SETTER,       NPCTYPE,              1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFamily",              ZTID_FLOAT,         GETTER,       NPCTYPE,              1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFamily",              ZTID_VOID,          SETTER,       NPCTYPE,              1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDamage",              ZTID_FLOAT,         GETTER,       NPCDP,                1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDamage",              ZTID_VOID,          SETTER,       NPCDP,                1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWeaponDamage",        ZTID_FLOAT,         GETTER,       NPCWDP,               1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWeaponDamage",        ZTID_VOID,          SETTER,       NPCWDP,               1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTile",                ZTID_FLOAT,         GETTER,       NPCTILE,              1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTile",                ZTID_VOID,          SETTER,       NPCTILE,              1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScriptTile",          ZTID_FLOAT,         GETTER,       NPCSCRIPTTILE,        1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScriptTile",          ZTID_VOID,          SETTER,       NPCSCRIPTTILE,        1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScriptFlip",          ZTID_FLOAT,         GETTER,       NPCSCRIPTFLIP,        1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScriptFlip",          ZTID_VOID,          SETTER,       NPCSCRIPTFLIP,        1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getOriginalTile",        ZTID_FLOAT,         GETTER,       NPCOTILE,             1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setOriginalTile",        ZTID_VOID,          SETTER,       NPCOTILE,             1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWeapon",              ZTID_FLOAT,         GETTER,       NPCWEAPON,            1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWeapon",              ZTID_VOID,          SETTER,       NPCWEAPON,            1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getItemSet",             ZTID_FLOAT,         GETTER,       NPCITEMSET,           1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItemSet",             ZTID_VOID,          SETTER,       NPCITEMSET,           1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDropset",             ZTID_FLOAT,         GETTER,       NPCITEMSET,           1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDropset",             ZTID_VOID,          SETTER,       NPCITEMSET,           1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCSet",                ZTID_FLOAT,         GETTER,       NPCCSET,              1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCSet",                ZTID_VOID,          SETTER,       NPCCSET,              1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getBossPal",             ZTID_FLOAT,         GETTER,       NPCBOSSPAL,           1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setBossPal",             ZTID_VOID,          SETTER,       NPCBOSSPAL,           1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSFX",                 ZTID_FLOAT,         GETTER,       NPCBGSFX,             1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSFX",                 ZTID_VOID,          SETTER,       NPCBGSFX,             1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getExtend",              ZTID_FLOAT,         GETTER,       NPCEXTEND,            1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setExtend",              ZTID_VOID,          SETTER,       NPCEXTEND,            1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitWidth",            ZTID_FLOAT,         GETTER,       NPCHXSZ,              1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitWidth",            ZTID_VOID,          SETTER,       NPCHXSZ,              1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitHeight",           ZTID_FLOAT,         GETTER,       NPCHYSZ,              1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitHeight",           ZTID_VOID,          SETTER,       NPCHYSZ,              1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitZHeight",          ZTID_FLOAT,         GETTER,       NPCHZSZ,              1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitZHeight",          ZTID_VOID,          SETTER,       NPCHZSZ,              1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTileWidth",           ZTID_FLOAT,         GETTER,       NPCTXSZ,              1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWidth",           ZTID_VOID,          SETTER,       NPCTXSZ,              1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTileHeight",          ZTID_FLOAT,         GETTER,       NPCTYSZ,              1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileHeight",          ZTID_VOID,          SETTER,       NPCTYSZ,              1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawXOffset",         ZTID_FLOAT,         GETTER,       NPCXOFS,              1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawXOffset",         ZTID_VOID,          SETTER,       NPCXOFS,              1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawYOffset",         ZTID_FLOAT,         GETTER,       NPCYOFS,              1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawYOffset",         ZTID_VOID,          SETTER,       NPCYOFS,              1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTotalDYOffset",       ZTID_FLOAT,         GETTER,       NPCTOTALDYOFFS,       1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTotalDYOffset",       ZTID_VOID,          SETTER,       NPCTOTALDYOFFS,       1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawZOffset",         ZTID_FLOAT,         GETTER,       NPCZOFS,              1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawZOffset",         ZTID_VOID,          SETTER,       NPCZOFS,              1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitXOffset",          ZTID_FLOAT,         GETTER,       NPCHXOFS,             1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitXOffset",          ZTID_VOID,          SETTER,       NPCHXOFS,             1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitYOffset",          ZTID_FLOAT,         GETTER,       NPCHYOFS,             1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitYOffset",          ZTID_VOID,          SETTER,       NPCHYOFS,             1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "isValid",                ZTID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Explode",                ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getMisc[]",              ZTID_UNTYPED,       GETTER,       NPCMISCD,             32,            0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMisc[]",              ZTID_VOID,          SETTER,       NPCMISCD,             32,            0,                                    3,           { ZTID_NPC, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getInitD[]",             ZTID_UNTYPED,       GETTER,       NPCINITD,             8,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInitD[]",             ZTID_VOID,          SETTER,       NPCINITD,             8,             0,                                    3,           { ZTID_NPC, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAttributes[]",        ZTID_UNTYPED,       GETTER,       NPCDD,                32,            0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAttributes[]",        ZTID_VOID,          SETTER,       NPCDD,                32,            0,                                    3,           { ZTID_NPC, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMiscFlags",           ZTID_FLOAT,         GETTER,       NPCMFLAGS,            1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMiscFlags",           ZTID_VOID,          SETTER,       NPCMFLAGS,            1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCollDetection",       ZTID_BOOL,          GETTER,       NPCCOLLDET,           1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCollDetection",       ZTID_VOID,          SETTER,       NPCCOLLDET,           1,             0,                                    2,           { ZTID_NPC, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getGravity",       ZTID_BOOL,          GETTER,       NPCGRAVITY,           1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setGravity",       ZTID_VOID,          SETTER,       NPCGRAVITY,           1,             0,                                    2,           { ZTID_NPC, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAnimation",           ZTID_BOOL,          GETTER,       NPCENGINEANIMATE,     1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAnimation",           ZTID_VOID,          SETTER,       NPCENGINEANIMATE,     1,             0,                                    2,           { ZTID_NPC, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Submerged",                ZTID_BOOL,          GETTER,       NPCSUBMERGED,              1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStun",                ZTID_FLOAT,         GETTER,       NPCSTUN,              1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStun",                ZTID_VOID,          SETTER,       NPCSTUN,              1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCore",                ZTID_BOOL,          GETTER,       NPCISCORE,            1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCore",                ZTID_VOID,          SETTER,       NPCISCORE,            1,             0,                                    2,           { ZTID_NPC, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDefense[]",           ZTID_FLOAT,         GETTER,       NPCDEFENSED,          42,            0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDefense[]",           ZTID_VOID,          SETTER,       NPCDEFENSED,          42,            0,                                    3,           { ZTID_NPC, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitBy[]",             ZTID_UNTYPED,       GETTER,       NPCHITBY,             16,            0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitBy[]",             ZTID_VOID,          SETTER,       NPCHITBY,             16,            0,                                    3,           { ZTID_NPC, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetName",                ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHunger",              ZTID_FLOAT,         GETTER,       NPCHUNGER,            1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHunger",              ZTID_VOID,          SETTER,       NPCHUNGER,            1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "BreakShield",            ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWeaponSprite",        ZTID_FLOAT,         GETTER,       NPCWEAPSPRITE,        1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWeaponSprite",        ZTID_VOID,          SETTER,       NPCWEAPSPRITE,        1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	2.future srit compatibility. -Z
	{ "getScriptDefense[]",     ZTID_FLOAT,         GETTER,       NPCSCRDEFENSED,       10,            0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScriptDefense[]",     ZTID_VOID,          SETTER,       NPCSCRDEFENSED,       10,            0,                                    3,           { ZTID_NPC, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	
	{ "getInvFrames",           ZTID_FLOAT,         GETTER,       NPCINVINC,            1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInvFrames",           ZTID_VOID,          SETTER,       NPCINVINC,            1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getInvincible",          ZTID_FLOAT,         GETTER,       NPCSUPERMAN,          1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInvincible",          ZTID_VOID,          SETTER,       NPCSUPERMAN,          1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRingleader",          ZTID_BOOL,          GETTER,       NPCRINGLEAD,          1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRingleader",          ZTID_VOID,          SETTER,       NPCRINGLEAD,          1,             0,                                    2,           { ZTID_NPC, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHasItem",             ZTID_BOOL,          GETTER,       NPCHASITEM,           1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHasItem",             ZTID_VOID,          SETTER,       NPCHASITEM,           1,             0,                                    2,           { ZTID_NPC, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShield[]",            ZTID_BOOL,          GETTER,       NPCSHIELD,            5,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShield[]",            ZTID_VOID,          SETTER,       NPCSHIELD,            5,             0,                                    3,           { ZTID_NPC, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getBFlags[]",            ZTID_BOOL,          GETTER,       NPCBEHAVIOUR,         16,            0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setBFlags[]",            ZTID_VOID,          SETTER,       NPCBEHAVIOUR,         16,            0,                                    3,           { ZTID_NPC, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFrozen",              ZTID_FLOAT,         GETTER,       NPCFROZEN,            1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFrozen",              ZTID_VOID,          SETTER,       NPCFROZEN,            1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getUID",                 ZTID_FLOAT,         GETTER,       NPCSCRIPTUID,         1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "setUID",                 ZTID_VOID,          SETTER,       NPCSCRIPTUID,         1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFrozenTile",          ZTID_FLOAT,         GETTER,       NPCFROZENTILE,        1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFrozenTile",          ZTID_VOID,          SETTER,       NPCFROZENTILE,        1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getParentUID",          ZTID_FLOAT,         GETTER,       NPCPARENTUID,        1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setParentUID",          ZTID_VOID,          SETTER,       NPCPARENTUID,        1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFrozenCSet",          ZTID_FLOAT,         GETTER,       NPCFROZENCSET,        1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFrozenCSet",          ZTID_VOID,          SETTER,       NPCFROZENCSET,        1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitDir",              ZTID_FLOAT,         GETTER,       NPCHITDIR,            1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitDir",          ZTID_VOID,          SETTER,       NPCHITDIR,        1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScript",              ZTID_FLOAT,         GETTER,       NPCSCRIPT,            1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScript",              ZTID_VOID,          SETTER,       NPCSCRIPT,            1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "isDead",                 ZTID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "CanSlide",               ZTID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Slide",                  ZTID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Remove",                 ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "StopBGSFX",              ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Attack",                 ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "NewDir",                 ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "ConstantWalk",           ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "ConstantWalk8",          ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "VariableWalk",           ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "VariableWalk8",          ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "HaltingWalk",            ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "HaltingWalk8",           ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "FloatingWalk",           ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "BreathAttack",           ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_NPC, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "NewDir8",                ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Collision",              ZTID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      3,           { ZTID_NPC, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LinedUp",                ZTID_FLOAT,         FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      3,           { ZTID_NPC, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LinkInRange",            ZTID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "CanMove",                ZTID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SimulateHit",            ZTID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Create",                 ZTID_NPC,           FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRotation",            ZTID_FLOAT,         GETTER,       NPCROTATION,          1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRotation",            ZTID_VOID,          SETTER,       NPCROTATION,          1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScale",               ZTID_FLOAT,         GETTER,       NPCSCALE,             1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScale",               ZTID_VOID,          SETTER,       NPCSCALE,             1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getImmortal",            ZTID_BOOL,          GETTER,       NPCIMMORTAL,          1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setImmortal",            ZTID_VOID,          SETTER,       NPCIMMORTAL,          1,             0,                                    2,           { ZTID_NPC, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getNoSlide",             ZTID_BOOL,          GETTER,       NPCNOSLIDE,           1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNoSlide",             ZTID_VOID,          SETTER,       NPCNOSLIDE,           1,             0,                                    2,           { ZTID_NPC, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getNoScriptKnockback",   ZTID_BOOL,          GETTER,       NPCNOSCRIPTKB,        1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNoScriptKnockback",   ZTID_VOID,          SETTER,       NPCNOSCRIPTKB,        1,             0,                                    2,           { ZTID_NPC, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSlideSpeed",          ZTID_FLOAT,         GETTER,       NPCKNOCKBACKSPEED,    1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSlideSpeed",          ZTID_VOID,          SETTER,       NPCKNOCKBACKSPEED,    1,             0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Knockback",              ZTID_BOOL,          FUNCTION,     0,                    1,             0,                                    4,           { ZTID_NPC, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFalling",             ZTID_FLOAT,         GETTER,       NPCFALLCLK,          1,           0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFalling",             ZTID_VOID,          SETTER,       NPCFALLCLK,          1,           0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFallCombo",           ZTID_FLOAT,         GETTER,       NPCFALLCMB,          1,           0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFallCombo",           ZTID_VOID,          SETTER,       NPCFALLCMB,          1,           0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMoveFlags[]",         ZTID_BOOL,          GETTER,       NPCMOVEFLAGS,        8,           0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMoveFlags[]",         ZTID_VOID,          SETTER,       NPCMOVEFLAGS,        8,           0,                                    3,           { ZTID_NPC, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLightRadius",         ZTID_FLOAT,         GETTER,       NPCGLOWRAD,           1,           0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLightRadius",         ZTID_VOID,          SETTER,       NPCGLOWRAD,           1,           0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLightShape",          ZTID_FLOAT,         GETTER,       NPCGLOWSHP,           1,           0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLightShape",          ZTID_VOID,          SETTER,       NPCGLOWSHP,           1,           0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShadowSprite",        ZTID_FLOAT,         GETTER,       NPCSHADOWSPR,         1,           0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShadowSprite",        ZTID_VOID,          SETTER,       NPCSHADOWSPR,         1,           0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSpawnSprite",         ZTID_FLOAT,         GETTER,       NPCSPAWNSPR,          1,           0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSpawnSprite",         ZTID_VOID,          SETTER,       NPCSPAWNSPR,          1,           0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDeathSprite",         ZTID_FLOAT,         GETTER,       NPCDEATHSPR,          1,           0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDeathSprite",         ZTID_VOID,          SETTER,       NPCDEATHSPR,          1,           0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSwitchHooked",        ZTID_BOOL,          GETTER,       NPCSWHOOKED,          1,             0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSwitchHooked",        ZTID_VOID,          SETTER,       NPCSWHOOKED,          1,             0,                                    2,           { ZTID_NPC, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Switch",                 ZTID_BOOL,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MovePaused",             ZTID_BOOL,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Move",                   ZTID_BOOL,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      4,           { ZTID_NPC, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MoveAtAngle",            ZTID_BOOL,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      4,           { ZTID_NPC, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MoveXY",                 ZTID_BOOL,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      4,           { ZTID_NPC, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "CanMove",                ZTID_BOOL,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      4,           { ZTID_NPC, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "CanMoveAtAngle",         ZTID_BOOL,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      4,           { ZTID_NPC, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "CanMoveXY",              ZTID_BOOL,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      4,           { ZTID_NPC, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInvFlicker",          ZTID_VOID,          SETTER,       NPCCANFLICKER,        1,           0,                                    2,           { ZTID_NPC, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getInvFlicker",          ZTID_BOOL,          GETTER,       NPCCANFLICKER,        1,           0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrowning",             ZTID_FLOAT,         GETTER,       NPCDROWNCLK,          1,           0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrowning",             ZTID_VOID,          SETTER,       NPCDROWNCLK,          1,           0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrownCombo",           ZTID_FLOAT,         GETTER,       NPCDROWNCMB,          1,           0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrownCombo",           ZTID_VOID,          SETTER,       NPCDROWNCMB,          1,           0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFakeZ",           ZTID_FLOAT,         GETTER,       NPCFAKEZ,          1,           0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFakeZ",           ZTID_VOID,          SETTER,       NPCFAKEZ,          1,           0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFakeJump",           ZTID_FLOAT,         GETTER,       NPCFAKEJUMP,          1,           0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFakeJump",           ZTID_VOID,          SETTER,       NPCFAKEJUMP,          1,           0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShadowXOffset",           ZTID_FLOAT,         GETTER,       NPCSHADOWXOFS,          1,           0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShadowXOffset",           ZTID_VOID,          SETTER,       NPCSHADOWXOFS,          1,           0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShadowYOffset",           ZTID_FLOAT,         GETTER,       NPCSHADOWYOFS,          1,           0,                                    1,           { ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShadowYOffset",           ZTID_VOID,          SETTER,       NPCSHADOWYOFS,          1,           0,                                    2,           { ZTID_NPC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	
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

