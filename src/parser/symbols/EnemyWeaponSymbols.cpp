#include "SymbolDefs.h"

EnemyWeaponSymbols EnemyWeaponSymbols::singleton = EnemyWeaponSymbols();

static AccessorTable ewpnTable[] =
{
//	  name,                     rettype,                  setorget,     var,                  numindex,      funcFlags,                            numParams,   params
	{ "getLevel",               ZTID_FLOAT,         GETTER,       EWPNLEVEL,            1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLevel",               ZTID_VOID,          SETTER,       EWPNLEVEL,            1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getX",                   ZTID_FLOAT,         GETTER,       EWPNX,                1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setX",                   ZTID_VOID,          SETTER,       EWPNX,                1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMax",                 ZTID_FLOAT,         GETTER,       SPRITEMAXEWPN,        1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMax",                 ZTID_VOID,          SETTER,       SPRITEMAXEWPN,        1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getY",                   ZTID_FLOAT,         GETTER,       EWPNY,                1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setY",                   ZTID_VOID,          SETTER,       EWPNY,                1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getZ",                   ZTID_FLOAT,         GETTER,       EWPNZ,                1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setZ",                   ZTID_VOID,          SETTER,       EWPNZ,                1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getJump",                ZTID_FLOAT,         GETTER,       EWPNJUMP,             1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setJump",                ZTID_VOID,          SETTER,       EWPNJUMP,             1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDir",                 ZTID_FLOAT,         GETTER,       EWPNDIR,              1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDir",                 ZTID_VOID,          SETTER,       EWPNDIR,              1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAngle",               ZTID_FLOAT,         GETTER,       EWPNANGLE,            1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAngle",               ZTID_VOID,          SETTER,       EWPNANGLE,            1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStep",    	        ZTID_FLOAT,         GETTER,       EWPNSTEP,             1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStep",    	        ZTID_VOID,          SETTER,       EWPNSTEP,             1,             0,                                    1,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getNumFrames",           ZTID_FLOAT,         GETTER,       EWPNFRAMES,           1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNumFrames",           ZTID_VOID,          SETTER,       EWPNFRAMES,           1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFrame",               ZTID_FLOAT,         GETTER,       EWPNFRAME,            1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFrame",               ZTID_VOID,          SETTER,       EWPNFRAME,            1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawStyle",           ZTID_FLOAT,         GETTER,       EWPNDRAWTYPE,         1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawStyle",           ZTID_VOID,          SETTER,       EWPNDRAWTYPE,         1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPower",               ZTID_FLOAT,         GETTER,       EWPNPOWER,            1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPower",               ZTID_VOID,          SETTER,       EWPNPOWER,            1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDamage",              ZTID_FLOAT,         GETTER,       EWPNPOWER,            1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDamage",              ZTID_VOID,          SETTER,       EWPNPOWER,            1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getID",                  ZTID_FLOAT,         GETTER,       EWPNID,               1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setID",                  ZTID_VOID,          SETTER,       EWPNID,               1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getType",                ZTID_FLOAT,         GETTER,       EWPNID,               1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setType",                ZTID_VOID,          SETTER,       EWPNID,               1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFamily",              ZTID_FLOAT,         GETTER,       EWPNID,               1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFamily",              ZTID_VOID,          SETTER,       EWPNID,               1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAngular",             ZTID_BOOL,          GETTER,       EWPNANGULAR,          1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAngular",             ZTID_VOID,          SETTER,       EWPNANGULAR,          1,             0,                                    2,           { ZTID_EWPN, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getBehind",              ZTID_BOOL,          GETTER,       EWPNBEHIND,           1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setBehind",              ZTID_VOID,          SETTER,       EWPNBEHIND,           1,             0,                                    2,           { ZTID_EWPN, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getASpeed",              ZTID_FLOAT,         GETTER,       EWPNASPEED,           1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setASpeed",              ZTID_VOID,          SETTER,       EWPNASPEED,           1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTile",                ZTID_FLOAT,         GETTER,       EWPNTILE,             1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTile",                ZTID_VOID,          SETTER,       EWPNTILE,             1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScriptTile",          ZTID_FLOAT,         GETTER,       EWPNSCRIPTTILE,       1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScriptTile",          ZTID_VOID,          SETTER,       EWPNSCRIPTTILE,       1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScriptFlip",          ZTID_FLOAT,         GETTER,       EWPNSCRIPTFLIP,       1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScriptFlip",          ZTID_VOID,          SETTER,       EWPNSCRIPTFLIP,       1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlashCSet",           ZTID_FLOAT,         GETTER,       EWPNFLASHCSET,        1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlashCSet",           ZTID_VOID,          SETTER,       EWPNFLASHCSET,        1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDeadState",           ZTID_FLOAT,         GETTER,       EWPNDEAD,             1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDeadState",           ZTID_VOID,          SETTER,       EWPNDEAD,             1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCSet",                ZTID_FLOAT,         GETTER,       EWPNCSET,             1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCSet",                ZTID_VOID,          SETTER,       EWPNCSET,             1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlash",               ZTID_BOOL,          GETTER,       EWPNFLASH,            1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlash",               ZTID_VOID,          SETTER,       EWPNFLASH,            1,             0,                                    2,           { ZTID_EWPN, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlip",                ZTID_FLOAT,         GETTER,       EWPNFLIP,             1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlip",                ZTID_VOID,          SETTER,       EWPNFLIP,             1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getOriginalTile",        ZTID_FLOAT,         GETTER,       EWPNOTILE,            1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setOriginalTile",        ZTID_VOID,          SETTER,       EWPNOTILE,            1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getOriginalCSet",        ZTID_FLOAT,         GETTER,       EWPNOCSET,            1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setOriginalCSet",        ZTID_VOID,          SETTER,       EWPNOCSET,            1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getExtend",              ZTID_FLOAT,         GETTER,       EWPNEXTEND,           1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setExtend",              ZTID_VOID,          SETTER,       EWPNEXTEND,           1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitWidth",            ZTID_FLOAT,         GETTER,       EWPNHXSZ,             1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitWidth",            ZTID_VOID,          SETTER,       EWPNHXSZ,             1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitHeight",           ZTID_FLOAT,         GETTER,       EWPNHYSZ,             1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitHeight",           ZTID_VOID,          SETTER,       EWPNHYSZ,             1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitZHeight",          ZTID_FLOAT,         GETTER,       EWPNHZSZ,             1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitZHeight",          ZTID_VOID,          SETTER,       EWPNHZSZ,             1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTileWidth",           ZTID_FLOAT,         GETTER,       EWPNTXSZ,             1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWidth",           ZTID_VOID,          SETTER,       EWPNTXSZ,             1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTileHeight",          ZTID_FLOAT,         GETTER,       EWPNTYSZ,             1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileHeight",          ZTID_VOID,          SETTER,       EWPNTYSZ,             1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawXOffset",         ZTID_FLOAT,         GETTER,       EWPNXOFS,             1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawXOffset",         ZTID_VOID,          SETTER,       EWPNXOFS,             1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawYOffset",         ZTID_FLOAT,         GETTER,       EWPNYOFS,             1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawYOffset",         ZTID_VOID,          SETTER,       EWPNYOFS,             1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTotalDYOffset",       ZTID_FLOAT,         GETTER,       EWPNTOTALDYOFFS,      1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTotalDYOffset",       ZTID_VOID,          SETTER,       EWPNTOTALDYOFFS,      1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawZOffset",         ZTID_FLOAT,         GETTER,       EWPNZOFS,             1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawZOffset",         ZTID_VOID,          SETTER,       EWPNZOFS,             1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitXOffset",          ZTID_FLOAT,         GETTER,       EWPNHXOFS,            1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitXOffset",          ZTID_VOID,          SETTER,       EWPNHXOFS,            1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitYOffset",          ZTID_FLOAT,         GETTER,       EWPNHYOFS,            1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitYOffset",          ZTID_VOID,          SETTER,       EWPNHYOFS,            1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getParent",              ZTID_NPC,           GETTER,       EWPNPARENT,           1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setParent",              ZTID_VOID,          SETTER,       EWPNPARENT,           1,             0,                                    2,           { ZTID_EWPN, ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "isValid",    	        ZTID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "UseSprite",              ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMisc[]",              ZTID_UNTYPED,       GETTER,       EWPNMISCD,            32,            0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMisc[]",              ZTID_VOID,          SETTER,       EWPNMISCD,            32,            0,                                    3,           { ZTID_EWPN, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCollDetection",       ZTID_FLOAT,         GETTER,       EWPNCOLLDET,          1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCollDetection",       ZTID_VOID,          SETTER,       EWPNCOLLDET,          1,             0,                                    2,           { ZTID_EWPN, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAnimation",           ZTID_FLOAT,         GETTER,       EWPNENGINEANIMATE,    1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAnimation",           ZTID_VOID,          SETTER,       EWPNENGINEANIMATE,    1,             0,                                    2,           { ZTID_EWPN, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getUID",                 ZTID_FLOAT,         GETTER,       EWEAPONSCRIPTUID,     1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getParentUID",           ZTID_FLOAT,         GETTER,       EWPNPARENTUID,        1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setParentUID",            ZTID_VOID,         SETTER,       EWPNPARENTUID,        1,             0,                                    1,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScript",              ZTID_FLOAT,         GETTER,       EWPNSCRIPT,           1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScript",              ZTID_VOID,          SETTER,       EWPNSCRIPT,           1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getInitD[]",             ZTID_UNTYPED,       GETTER,       EWPNINITD,            8,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInitD[]",             ZTID_VOID,          SETTER,       EWPNINITD,            8,             0,                                    3,           { ZTID_EWPN, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "Explode",                ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRotation",            ZTID_FLOAT,         GETTER,       EWPNROTATION,         1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRotation",            ZTID_VOID,          SETTER,       EWPNROTATION,         1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScale",               ZTID_FLOAT,         GETTER,       EWPNSCALE,            1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScale",               ZTID_VOID,          SETTER,       EWPNSCALE,            1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getGravity",             ZTID_BOOL,          GETTER,       EWPNGRAVITY,          1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setGravity",             ZTID_VOID,          SETTER,       EWPNGRAVITY,          1,             0,                                    2,           { ZTID_EWPN, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFalling",             ZTID_FLOAT,         GETTER,       EWPNFALLCLK,          1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFalling",             ZTID_VOID,          SETTER,       EWPNFALLCLK,          1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFallCombo",           ZTID_FLOAT,         GETTER,       EWPNFALLCMB,          1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFallCombo",           ZTID_VOID,          SETTER,       EWPNFALLCMB,          1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMoveFlags[]",         ZTID_BOOL,          GETTER,       EWPNMOVEFLAGS,        2,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMoveFlags[]",         ZTID_VOID,          SETTER,       EWPNMOVEFLAGS,        2,             0,                                    3,           { ZTID_EWPN, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLightRadius",         ZTID_FLOAT,         GETTER,       EWPNGLOWRAD,          1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLightRadius",         ZTID_VOID,          SETTER,       EWPNGLOWRAD,          1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLightShape",          ZTID_FLOAT,         GETTER,       EWPNGLOWSHP,          1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLightShape",          ZTID_VOID,          SETTER,       EWPNGLOWSHP,          1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getUnblockable",         ZTID_FLOAT,         GETTER,       EWPNUNBL,             1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setUnblockable",         ZTID_VOID,          SETTER,       EWPNUNBL,             1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Remove",                 ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShadowSprite",        ZTID_FLOAT,         GETTER,       EWPNSHADOWSPR,        1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShadowSprite",        ZTID_VOID,          SETTER,       EWPNSHADOWSPR,        1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSwitchHooked",        ZTID_BOOL,          GETTER,       EWSWHOOKED,           1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSwitchHooked",        ZTID_VOID,          SETTER,       EWSWHOOKED,           1,             0,                                    2,           { ZTID_EWPN, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Switch",                 ZTID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrowning",            ZTID_FLOAT,         GETTER,       EWPNDROWNCLK,         1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrowning",            ZTID_VOID,          SETTER,       EWPNDROWNCLK,         1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrownCombo",          ZTID_FLOAT,         GETTER,       EWPNDROWNCMB,         1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrownCombo",          ZTID_VOID,          SETTER,       EWPNDROWNCMB,         1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFakeZ",               ZTID_FLOAT,         GETTER,       EWPNFAKEZ,            1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFakeZ",               ZTID_VOID,          SETTER,       EWPNFAKEZ,            1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFakeJump",            ZTID_FLOAT,         GETTER,       EWPNFAKEJUMP,         1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFakeJump",            ZTID_VOID,          SETTER,       EWPNFAKEJUMP,         1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShadowXOffset",       ZTID_FLOAT,         GETTER,       EWPNSHADOWXOFS,       1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShadowXOffset",       ZTID_VOID,          SETTER,       EWPNSHADOWXOFS,       1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShadowYOffset",       ZTID_FLOAT,         GETTER,       EWPNSHADOWYOFS,       1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShadowYOffset",       ZTID_VOID,          SETTER,       EWPNSHADOWYOFS,       1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDegAngle",            ZTID_FLOAT,         GETTER,       EWPNDEGANGLE,         1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDegAngle",            ZTID_VOID,          SETTER,       EWPNDEGANGLE,         1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getVx",                  ZTID_FLOAT,         GETTER,       EWPNVX,               1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setVx",                  ZTID_VOID,          SETTER,       EWPNVX,               1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getVy",                  ZTID_FLOAT,         GETTER,       EWPNVY,               1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setVy",                  ZTID_VOID,          SETTER,       EWPNVY,               1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAutoRotate",          ZTID_BOOL,          GETTER,       EWPNAUTOROTATE,       1,             0,                                    1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAutoRotate",          ZTID_VOID,          SETTER,       EWPNAUTOROTATE,       1,             0,                                    2,           { ZTID_EWPN, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MakeAngular",    	    ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MakeDirectional",    	ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlags[]",             ZTID_BOOL,          GETTER,       EWPNFLAGS,            1,             0,                                    2,           { ZTID_EWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlags[]",             ZTID_VOID,          SETTER,       EWPNFLAGS,            1,             0,                                    3,           { ZTID_EWPN, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
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

