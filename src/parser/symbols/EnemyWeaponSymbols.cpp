#include "SymbolDefs.h"

EnemyWeaponSymbols EnemyWeaponSymbols::singleton = EnemyWeaponSymbols();

static AccessorTable ewpnTable[] =
{
//	  name,                     rettype,                  setorget,     var,                  numindex,      funcFlags,                            numParams,   params
	{ "getLevel",               ZVARTYPEID_FLOAT,         GETTER,       EWPNLEVEL,            1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLevel",               ZVARTYPEID_VOID,          SETTER,       EWPNLEVEL,            1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getX",                   ZVARTYPEID_FLOAT,         GETTER,       EWPNX,                1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setX",                   ZVARTYPEID_VOID,          SETTER,       EWPNX,                1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMax",                 ZVARTYPEID_FLOAT,         GETTER,       SPRITEMAXEWPN,        1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMax",                 ZVARTYPEID_VOID,          SETTER,       SPRITEMAXEWPN,        1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getY",                   ZVARTYPEID_FLOAT,         GETTER,       EWPNY,                1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setY",                   ZVARTYPEID_VOID,          SETTER,       EWPNY,                1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getZ",                   ZVARTYPEID_FLOAT,         GETTER,       EWPNZ,                1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setZ",                   ZVARTYPEID_VOID,          SETTER,       EWPNZ,                1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getJump",                ZVARTYPEID_FLOAT,         GETTER,       EWPNJUMP,             1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setJump",                ZVARTYPEID_VOID,          SETTER,       EWPNJUMP,             1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDir",                 ZVARTYPEID_FLOAT,         GETTER,       EWPNDIR,              1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDir",                 ZVARTYPEID_VOID,          SETTER,       EWPNDIR,              1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAngle",               ZVARTYPEID_FLOAT,         GETTER,       EWPNANGLE,            1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAngle",               ZVARTYPEID_VOID,          SETTER,       EWPNANGLE,            1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStep",    	        ZVARTYPEID_FLOAT,         GETTER,       EWPNSTEP,             1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStep",    	        ZVARTYPEID_VOID,          SETTER,       EWPNSTEP,             1,             0,                                    1,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getNumFrames",           ZVARTYPEID_FLOAT,         GETTER,       EWPNFRAMES,           1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNumFrames",           ZVARTYPEID_VOID,          SETTER,       EWPNFRAMES,           1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFrame",               ZVARTYPEID_FLOAT,         GETTER,       EWPNFRAME,            1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFrame",               ZVARTYPEID_VOID,          SETTER,       EWPNFRAME,            1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawStyle",           ZVARTYPEID_FLOAT,         GETTER,       EWPNDRAWTYPE,         1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawStyle",           ZVARTYPEID_VOID,          SETTER,       EWPNDRAWTYPE,         1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPower",               ZVARTYPEID_FLOAT,         GETTER,       EWPNPOWER,            1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPower",               ZVARTYPEID_VOID,          SETTER,       EWPNPOWER,            1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDamage",              ZVARTYPEID_FLOAT,         GETTER,       EWPNPOWER,            1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDamage",              ZVARTYPEID_VOID,          SETTER,       EWPNPOWER,            1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getID",                  ZVARTYPEID_FLOAT,         GETTER,       EWPNID,               1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setID",                  ZVARTYPEID_VOID,          SETTER,       EWPNID,               1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getType",                ZVARTYPEID_FLOAT,         GETTER,       EWPNID,               1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setType",                ZVARTYPEID_VOID,          SETTER,       EWPNID,               1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFamily",              ZVARTYPEID_FLOAT,         GETTER,       EWPNID,               1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFamily",              ZVARTYPEID_VOID,          SETTER,       EWPNID,               1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAngular",             ZVARTYPEID_BOOL,          GETTER,       EWPNANGULAR,          1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAngular",             ZVARTYPEID_VOID,          SETTER,       EWPNANGULAR,          1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getBehind",              ZVARTYPEID_BOOL,          GETTER,       EWPNBEHIND,           1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setBehind",              ZVARTYPEID_VOID,          SETTER,       EWPNBEHIND,           1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getASpeed",              ZVARTYPEID_FLOAT,         GETTER,       EWPNASPEED,           1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setASpeed",              ZVARTYPEID_VOID,          SETTER,       EWPNASPEED,           1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTile",                ZVARTYPEID_FLOAT,         GETTER,       EWPNTILE,             1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTile",                ZVARTYPEID_VOID,          SETTER,       EWPNTILE,             1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScriptTile",          ZVARTYPEID_FLOAT,         GETTER,       EWPNSCRIPTTILE,       1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScriptTile",          ZVARTYPEID_VOID,          SETTER,       EWPNSCRIPTTILE,       1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScriptFlip",          ZVARTYPEID_FLOAT,         GETTER,       EWPNSCRIPTFLIP,       1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScriptFlip",          ZVARTYPEID_VOID,          SETTER,       EWPNSCRIPTFLIP,       1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlashCSet",           ZVARTYPEID_FLOAT,         GETTER,       EWPNFLASHCSET,        1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlashCSet",           ZVARTYPEID_VOID,          SETTER,       EWPNFLASHCSET,        1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDeadState",           ZVARTYPEID_FLOAT,         GETTER,       EWPNDEAD,             1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDeadState",           ZVARTYPEID_VOID,          SETTER,       EWPNDEAD,             1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCSet",                ZVARTYPEID_FLOAT,         GETTER,       EWPNCSET,             1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCSet",                ZVARTYPEID_VOID,          SETTER,       EWPNCSET,             1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlash",               ZVARTYPEID_BOOL,          GETTER,       EWPNFLASH,            1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlash",               ZVARTYPEID_VOID,          SETTER,       EWPNFLASH,            1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlip",                ZVARTYPEID_FLOAT,         GETTER,       EWPNFLIP,             1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlip",                ZVARTYPEID_VOID,          SETTER,       EWPNFLIP,             1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getOriginalTile",        ZVARTYPEID_FLOAT,         GETTER,       EWPNOTILE,            1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setOriginalTile",        ZVARTYPEID_VOID,          SETTER,       EWPNOTILE,            1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getOriginalCSet",        ZVARTYPEID_FLOAT,         GETTER,       EWPNOCSET,            1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setOriginalCSet",        ZVARTYPEID_VOID,          SETTER,       EWPNOCSET,            1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getExtend",              ZVARTYPEID_FLOAT,         GETTER,       EWPNEXTEND,           1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setExtend",              ZVARTYPEID_VOID,          SETTER,       EWPNEXTEND,           1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitWidth",            ZVARTYPEID_FLOAT,         GETTER,       EWPNHXSZ,             1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitWidth",            ZVARTYPEID_VOID,          SETTER,       EWPNHXSZ,             1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitHeight",           ZVARTYPEID_FLOAT,         GETTER,       EWPNHYSZ,             1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitHeight",           ZVARTYPEID_VOID,          SETTER,       EWPNHYSZ,             1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitZHeight",          ZVARTYPEID_FLOAT,         GETTER,       EWPNHZSZ,             1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitZHeight",          ZVARTYPEID_VOID,          SETTER,       EWPNHZSZ,             1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTileWidth",           ZVARTYPEID_FLOAT,         GETTER,       EWPNTXSZ,             1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWidth",           ZVARTYPEID_VOID,          SETTER,       EWPNTXSZ,             1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTileHeight",          ZVARTYPEID_FLOAT,         GETTER,       EWPNTYSZ,             1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileHeight",          ZVARTYPEID_VOID,          SETTER,       EWPNTYSZ,             1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawXOffset",         ZVARTYPEID_FLOAT,         GETTER,       EWPNXOFS,             1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawXOffset",         ZVARTYPEID_VOID,          SETTER,       EWPNXOFS,             1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawYOffset",         ZVARTYPEID_FLOAT,         GETTER,       EWPNYOFS,             1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawYOffset",         ZVARTYPEID_VOID,          SETTER,       EWPNYOFS,             1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTotalDYOffset",       ZVARTYPEID_FLOAT,         GETTER,       EWPNTOTALDYOFFS,      1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTotalDYOffset",       ZVARTYPEID_VOID,          SETTER,       EWPNTOTALDYOFFS,      1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawZOffset",         ZVARTYPEID_FLOAT,         GETTER,       EWPNZOFS,             1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawZOffset",         ZVARTYPEID_VOID,          SETTER,       EWPNZOFS,             1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitXOffset",          ZVARTYPEID_FLOAT,         GETTER,       EWPNHXOFS,            1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitXOffset",          ZVARTYPEID_VOID,          SETTER,       EWPNHXOFS,            1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitYOffset",          ZVARTYPEID_FLOAT,         GETTER,       EWPNHYOFS,            1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitYOffset",          ZVARTYPEID_VOID,          SETTER,       EWPNHYOFS,            1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getParent",              ZVARTYPEID_NPC,           GETTER,       EWPNPARENT,           1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setParent",              ZVARTYPEID_VOID,          SETTER,       EWPNPARENT,           1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "isValid",    	        ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "UseSprite",              ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMisc[]",              ZVARTYPEID_UNTYPED,       GETTER,       EWPNMISCD,            32,            0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMisc[]",              ZVARTYPEID_VOID,          SETTER,       EWPNMISCD,            32,            0,                                    3,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCollDetection",       ZVARTYPEID_FLOAT,         GETTER,       EWPNCOLLDET,          1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCollDetection",       ZVARTYPEID_VOID,          SETTER,       EWPNCOLLDET,          1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAnimation",           ZVARTYPEID_FLOAT,         GETTER,       EWPNENGINEANIMATE,    1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAnimation",           ZVARTYPEID_VOID,          SETTER,       EWPNENGINEANIMATE,    1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getUID",                 ZVARTYPEID_FLOAT,         GETTER,       EWEAPONSCRIPTUID,     1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getParentUID",           ZVARTYPEID_FLOAT,         GETTER,       EWPNPARENTUID,        1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setParentUID",            ZVARTYPEID_VOID,         SETTER,       EWPNPARENTUID,        1,             0,                                    1,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScript",              ZVARTYPEID_FLOAT,         GETTER,       EWPNSCRIPT,           1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScript",              ZVARTYPEID_VOID,          SETTER,       EWPNSCRIPT,           1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getInitD[]",             ZVARTYPEID_UNTYPED,       GETTER,       EWPNINITD,            8,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInitD[]",             ZVARTYPEID_VOID,          SETTER,       EWPNINITD,            8,             0,                                    3,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "Explode",                ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRotation",            ZVARTYPEID_FLOAT,         GETTER,       EWPNROTATION,         1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRotation",            ZVARTYPEID_VOID,          SETTER,       EWPNROTATION,         1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScale",               ZVARTYPEID_FLOAT,         GETTER,       EWPNSCALE,            1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScale",               ZVARTYPEID_VOID,          SETTER,       EWPNSCALE,            1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getGravity",             ZVARTYPEID_BOOL,          GETTER,       EWPNGRAVITY,          1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setGravity",             ZVARTYPEID_VOID,          SETTER,       EWPNGRAVITY,          1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFalling",             ZVARTYPEID_FLOAT,         GETTER,       EWPNFALLCLK,          1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFalling",             ZVARTYPEID_VOID,          SETTER,       EWPNFALLCLK,          1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFallCombo",           ZVARTYPEID_FLOAT,         GETTER,       EWPNFALLCMB,          1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFallCombo",           ZVARTYPEID_VOID,          SETTER,       EWPNFALLCMB,          1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMoveFlags[]",         ZVARTYPEID_BOOL,          GETTER,       EWPNMOVEFLAGS,        2,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMoveFlags[]",         ZVARTYPEID_VOID,          SETTER,       EWPNMOVEFLAGS,        2,             0,                                    3,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLightRadius",         ZVARTYPEID_FLOAT,         GETTER,       EWPNGLOWRAD,          1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLightRadius",         ZVARTYPEID_VOID,          SETTER,       EWPNGLOWRAD,          1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLightShape",          ZVARTYPEID_FLOAT,         GETTER,       EWPNGLOWSHP,          1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLightShape",          ZVARTYPEID_VOID,          SETTER,       EWPNGLOWSHP,          1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getUnblockable",         ZVARTYPEID_FLOAT,         GETTER,       EWPNUNBL,             1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setUnblockable",         ZVARTYPEID_VOID,          SETTER,       EWPNUNBL,             1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Remove",                 ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShadowSprite",        ZVARTYPEID_FLOAT,         GETTER,       EWPNSHADOWSPR,        1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShadowSprite",        ZVARTYPEID_VOID,          SETTER,       EWPNSHADOWSPR,        1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSwitchHooked",        ZVARTYPEID_BOOL,          GETTER,       EWSWHOOKED,           1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSwitchHooked",        ZVARTYPEID_VOID,          SETTER,       EWSWHOOKED,           1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Switch",                 ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrowning",            ZVARTYPEID_FLOAT,         GETTER,       EWPNDROWNCLK,         1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrowning",            ZVARTYPEID_VOID,          SETTER,       EWPNDROWNCLK,         1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrownCombo",          ZVARTYPEID_FLOAT,         GETTER,       EWPNDROWNCMB,         1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrownCombo",          ZVARTYPEID_VOID,          SETTER,       EWPNDROWNCMB,         1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFakeZ",               ZVARTYPEID_FLOAT,         GETTER,       EWPNFAKEZ,            1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFakeZ",               ZVARTYPEID_VOID,          SETTER,       EWPNFAKEZ,            1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFakeJump",            ZVARTYPEID_FLOAT,         GETTER,       EWPNFAKEJUMP,         1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFakeJump",            ZVARTYPEID_VOID,          SETTER,       EWPNFAKEJUMP,         1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShadowXOffset",       ZVARTYPEID_FLOAT,         GETTER,       EWPNSHADOWXOFS,       1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShadowXOffset",       ZVARTYPEID_VOID,          SETTER,       EWPNSHADOWXOFS,       1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShadowYOffset",       ZVARTYPEID_FLOAT,         GETTER,       EWPNSHADOWYOFS,       1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShadowYOffset",       ZVARTYPEID_VOID,          SETTER,       EWPNSHADOWYOFS,       1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDegAngle",            ZVARTYPEID_FLOAT,         GETTER,       EWPNDEGANGLE,         1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDegAngle",            ZVARTYPEID_VOID,          SETTER,       EWPNDEGANGLE,         1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getVx",                  ZVARTYPEID_FLOAT,         GETTER,       EWPNVX,               1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setVx",                  ZVARTYPEID_VOID,          SETTER,       EWPNVX,               1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getVy",                  ZVARTYPEID_FLOAT,         GETTER,       EWPNVY,               1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setVy",                  ZVARTYPEID_VOID,          SETTER,       EWPNVY,               1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAutoRotate",          ZVARTYPEID_BOOL,          GETTER,       EWPNAUTOROTATE,       1,             0,                                    1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAutoRotate",          ZVARTYPEID_VOID,          SETTER,       EWPNAUTOROTATE,       1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MakeAngular",    	    ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MakeDirectional",    	ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlags[]",             ZVARTYPEID_BOOL,          GETTER,       EWPNFLAGS,            1,             0,                                    2,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlags[]",             ZVARTYPEID_VOID,          SETTER,       EWPNFLAGS,            1,             0,                                    3,           { ZVARTYPEID_EWPN, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "",                       -1,                       -1,           -1,                   -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

EnemyWeaponSymbols::EnemyWeaponSymbols()
{
    table = ewpnTable;
    refVar = REFEWPN;
}

void EnemyWeaponSymbols::generateCode()
{
	//bool isValid(eweapon)
	{
		Function* function = getFunction("isValid", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Check validity
		addOpcode2 (code, new OIsValidEWpn(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void Explode(eweapon, int32_t)
	{
		Function* function = getFunction("Explode", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OEWeaponExplodeRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void UseSprite(eweapon, int32_t val)
	{
		Function* function = getFunction("UseSprite", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the val
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OSetRegister(new VarArgument(refVar), new VarArgument(EXP2)));
		addOpcode2 (code, new OUseSpriteEWpn(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void Remove(eweapon)
	{
		Function* function = getFunction("Remove", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		POPREF();
		LABELBACK(label);
		//Break shield
		addOpcode2 (code, new OEWpnRemove());
		RETURN();
		function->giveCode(code);
	}
	//bool Switch(eweapon, int)
	{
		Function* function = getFunction("Switch", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSwitchEW(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//bool MakeAngular(eweapon)
	{
		Function* function = getFunction("MakeAngular", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Check validity
		addOpcode2 (code, new OMakeAngularEwpn(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//bool MakeDirectional(eweapon)
	{
		Function* function = getFunction("MakeAngular", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Check validity
		addOpcode2 (code, new OMakeDirectionalEwpn(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
}

