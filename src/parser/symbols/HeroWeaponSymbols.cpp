#include "SymbolDefs.h"

HeroWeaponSymbols HeroWeaponSymbols::singleton = HeroWeaponSymbols();

static AccessorTable lwpnTable[] =
{
//	  name,                     rettype,                  setorget,     var,                  numindex,      funcFlags,                            numParams,   params
	{ "getX",                   ZTID_FLOAT,         GETTER,       LWPNX,                1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setX",                   ZTID_VOID,          SETTER,       LWPNX,                1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSpecial",                   ZTID_FLOAT,         GETTER,       LWPNSPECIAL,                1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSpecial",                   ZTID_VOID,          SETTER,       LWPNSPECIAL,                1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Max",                   ZTID_FLOAT,         GETTER,       SPRITEMAXLWPN,                1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Max",                   ZTID_VOID,          SETTER,       SPRITEMAXLWPN,                1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getY",                   ZTID_FLOAT,         GETTER,       LWPNY,                1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setY",                   ZTID_VOID,          SETTER,       LWPNY,                1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getZ",                   ZTID_FLOAT,         GETTER,       LWPNZ,                1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setZ",                   ZTID_VOID,          SETTER,       LWPNZ,                1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getJump",                ZTID_FLOAT,         GETTER,       LWPNJUMP,             1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setJump",                ZTID_VOID,          SETTER,       LWPNJUMP,             1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDir",                 ZTID_FLOAT,         GETTER,       LWPNDIR,              1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDir",                 ZTID_VOID,          SETTER,       LWPNDIR,              1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAngle",               ZTID_FLOAT,         GETTER,       LWPNANGLE,            1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAngle",               ZTID_VOID,          SETTER,       LWPNANGLE,            1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStep",                ZTID_FLOAT,         GETTER,       LWPNSTEP,             1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStep",                ZTID_VOID,          SETTER,       LWPNSTEP,             1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getNumFrames",           ZTID_FLOAT,         GETTER,       LWPNFRAMES,           1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNumFrames",           ZTID_VOID,          SETTER,       LWPNFRAMES,           1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFrame",               ZTID_FLOAT,         GETTER,       LWPNFRAME,            1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFrame",               ZTID_VOID,          SETTER,       LWPNFRAME,            1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawStyle",           ZTID_FLOAT,         GETTER,       LWPNDRAWTYPE,         1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawStyle",           ZTID_VOID,          SETTER,       LWPNDRAWTYPE,         1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPower",               ZTID_FLOAT,         GETTER,       LWPNPOWER,            1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPower",               ZTID_VOID,          SETTER,       LWPNPOWER,            1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDamage",              ZTID_FLOAT,         GETTER,       LWPNPOWER,            1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDamage",              ZTID_VOID,          SETTER,       LWPNPOWER,            1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getID",                  ZTID_FLOAT,         GETTER,       LWPNID,               1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setID",                  ZTID_VOID,          SETTER,       LWPNID,               1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getType",                ZTID_FLOAT,         GETTER,       LWPNID,               1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setType",                ZTID_VOID,          SETTER,       LWPNID,               1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFamily",              ZTID_FLOAT,         GETTER,       LWPNID,               1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFamily",              ZTID_VOID,          SETTER,       LWPNID,               1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAngular",             ZTID_BOOL,          GETTER,       LWPNANGULAR,          1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAngular",             ZTID_VOID,          SETTER,       LWPNANGULAR,          1,             0,                                    2,           { ZTID_LWPN, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getBehind",              ZTID_BOOL,          GETTER,       LWPNBEHIND,           1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setBehind",              ZTID_VOID,          SETTER,       LWPNBEHIND,           1,             0,                                    2,           { ZTID_LWPN, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getASpeed",              ZTID_FLOAT,         GETTER,       LWPNASPEED,           1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setASpeed",              ZTID_VOID,          SETTER,       LWPNASPEED,           1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTile",                ZTID_FLOAT,         GETTER,       LWPNTILE,             1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTile",                ZTID_VOID,          SETTER,       LWPNTILE,             1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScriptTile",          ZTID_FLOAT,         GETTER,       LWPNSCRIPTTILE,       1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScriptTile",          ZTID_VOID,          SETTER,       LWPNSCRIPTTILE,       1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScriptFlip",          ZTID_FLOAT,         GETTER,       LWPNSCRIPTFLIP,       1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScriptFlip",          ZTID_VOID,          SETTER,       LWPNSCRIPTFLIP,       1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlashCSet",           ZTID_FLOAT,         GETTER,       LWPNFLASHCSET,        1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlashCSet",           ZTID_VOID,          SETTER,       LWPNFLASHCSET,        1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDeadState",           ZTID_FLOAT,         GETTER,       LWPNDEAD,             1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDeadState",           ZTID_VOID,          SETTER,       LWPNDEAD,             1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCSet",                ZTID_FLOAT,         GETTER,       LWPNCSET,             1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCSet",                ZTID_VOID,          SETTER,       LWPNCSET,             1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlash",               ZTID_BOOL,          GETTER,       LWPNFLASH,            1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlash",               ZTID_VOID,          SETTER,       LWPNFLASH,            1,             0,                                    2,           { ZTID_LWPN, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlip",                ZTID_FLOAT,         GETTER,       LWPNFLIP,             1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlip",                ZTID_VOID,          SETTER,       LWPNFLIP,             1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getOriginalTile",        ZTID_FLOAT,         GETTER,       LWPNOTILE,            1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setOriginalTile",        ZTID_VOID,          SETTER,       LWPNOTILE,            1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getOriginalCSet",        ZTID_FLOAT,         GETTER,       LWPNOCSET,            1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setOriginalCSet",        ZTID_VOID,          SETTER,       LWPNOCSET,            1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getExtend",              ZTID_FLOAT,         GETTER,       LWPNEXTEND,           1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setExtend",              ZTID_VOID,          SETTER,       LWPNEXTEND,           1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitWidth",            ZTID_FLOAT,         GETTER,       LWPNHXSZ,             1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitWidth",            ZTID_VOID,          SETTER,       LWPNHXSZ,             1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitHeight",           ZTID_FLOAT,         GETTER,       LWPNHYSZ,             1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitHeight",           ZTID_VOID,          SETTER,       LWPNHYSZ,             1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitZHeight",          ZTID_FLOAT,         GETTER,       LWPNHZSZ,             1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitZHeight",          ZTID_VOID,          SETTER,       LWPNHZSZ,             1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTileWidth",           ZTID_FLOAT,         GETTER,       LWPNTXSZ,             1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWidth",           ZTID_VOID,          SETTER,       LWPNTXSZ,             1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTileHeight",          ZTID_FLOAT,         GETTER,       LWPNTYSZ,             1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileHeight",          ZTID_VOID,          SETTER,       LWPNTYSZ,             1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawXOffset",         ZTID_FLOAT,         GETTER,       LWPNXOFS,             1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawXOffset",         ZTID_VOID,          SETTER,       LWPNXOFS,             1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawYOffset",         ZTID_FLOAT,         GETTER,       LWPNYOFS,             1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawYOffset",         ZTID_VOID,          SETTER,       LWPNYOFS,             1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTotalDYOffset",       ZTID_FLOAT,         GETTER,       LWPNTOTALDYOFFS,      1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTotalDYOffset",       ZTID_VOID,          SETTER,       LWPNTOTALDYOFFS,      1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawZOffset",         ZTID_FLOAT,         GETTER,       LWPNZOFS,             1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawZOffset",         ZTID_VOID,          SETTER,       LWPNZOFS,             1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitXOffset",          ZTID_FLOAT,         GETTER,       LWPNHXOFS,            1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitXOffset",          ZTID_VOID,          SETTER,       LWPNHXOFS,            1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitYOffset",          ZTID_FLOAT,         GETTER,       LWPNHYOFS,            1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitYOffset",          ZTID_VOID,          SETTER,       LWPNHYOFS,            1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "isValid",                ZTID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "UseSprite",              ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMisc[]",              ZTID_UNTYPED,       GETTER,       LWPNMISCD,            32,            0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMisc[]",              ZTID_VOID,          SETTER,       LWPNMISCD,            32,            0,                                    3,           { ZTID_LWPN, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCollDetection",       ZTID_BOOL,          GETTER,       LWPNCOLLDET,          1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCollDetection",       ZTID_VOID,          SETTER,       LWPNCOLLDET,          1,             0,                                    2,           { ZTID_LWPN, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAnimation",           ZTID_BOOL,          GETTER,       LWPNENGINEANIMATE,    1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAnimation",           ZTID_VOID,          SETTER,       LWPNENGINEANIMATE,    1,             0,                                    2,           { ZTID_LWPN, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "getRange",               ZTID_FLOAT,         GETTER,       LWPNRANGE,            1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "setRange",               ZTID_VOID,          SETTER,       LWPNRANGE,            1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getParent",              ZTID_FLOAT,         GETTER,       LWPNPARENT,           1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setParent",              ZTID_VOID,          SETTER,       LWPNPARENT,           1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLevel",               ZTID_FLOAT,         GETTER,       LWPNLEVEL,            1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLevel",               ZTID_VOID,          SETTER,       LWPNLEVEL,            1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScript",              ZTID_FLOAT,         GETTER,       LWPNSCRIPT,           1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScript",              ZTID_VOID,          SETTER,       LWPNSCRIPT,           1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWeapon",              ZTID_FLOAT,         GETTER,       LWPNUSEWEAPON,        1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWeapon",              ZTID_VOID,          SETTER,       LWPNUSEWEAPON,        1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDefense",             ZTID_FLOAT,         GETTER,       LWPNUSEDEFENCE,       1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDefense",             ZTID_VOID,          SETTER,       LWPNUSEDEFENCE,       1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDefence",             ZTID_FLOAT,         GETTER,       LWPNUSEDEFENCE,       1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDefence",             ZTID_VOID,          SETTER,       LWPNUSEDEFENCE,       1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getInitD[]",             ZTID_UNTYPED,       GETTER,       LWPNINITD,            8,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInitD[]",             ZTID_VOID,          SETTER,       LWPNINITD,            8,             0,                                    3,           { ZTID_LWPN, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getUID",                 ZTID_FLOAT,         GETTER,       LWEAPONSCRIPTUID,     1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Explode",                ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRotation",            ZTID_FLOAT,         GETTER,       LWPNROTATION,         1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRotation",            ZTID_VOID,          SETTER,       LWPNROTATION,         1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScale",               ZTID_FLOAT,         GETTER,       LWPNSCALE,            1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScale",               ZTID_VOID,          SETTER,       LWPNSCALE,            1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getGravity",             ZTID_BOOL,          GETTER,       LWPNGRAVITY,          1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setGravity",             ZTID_VOID,          SETTER,       LWPNGRAVITY,          1,             0,                                    2,           { ZTID_LWPN, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFalling",             ZTID_FLOAT,         GETTER,       LWPNFALLCLK,          1,           0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFalling",             ZTID_VOID,          SETTER,       LWPNFALLCLK,          1,           0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFallCombo",           ZTID_FLOAT,         GETTER,       LWPNFALLCMB,          1,           0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFallCombo",           ZTID_VOID,          SETTER,       LWPNFALLCMB,          1,           0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMoveFlags[]",         ZTID_BOOL,          GETTER,       LWPNMOVEFLAGS,        2,           0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMoveFlags[]",         ZTID_VOID,          SETTER,       LWPNMOVEFLAGS,        2,           0,                                    3,           { ZTID_LWPN, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLightRadius",         ZTID_FLOAT,         GETTER,       LWPNGLOWRAD,          1,           0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLightRadius",         ZTID_VOID,          SETTER,       LWPNGLOWRAD,          1,           0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLightShape",          ZTID_FLOAT,         GETTER,       LWPNGLOWSHP,          1,           0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLightShape",          ZTID_VOID,          SETTER,       LWPNGLOWSHP,          1,           0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getUnblockable",         ZTID_FLOAT,         GETTER,       LWPNUNBL,             1,           0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setUnblockable",         ZTID_VOID,          SETTER,       LWPNUNBL,             1,           0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Remove",                 ZTID_VOID,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShadowSprite",        ZTID_FLOAT,         GETTER,       LWPNSHADOWSPR,        1,           0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShadowSprite",        ZTID_VOID,          SETTER,       LWPNSHADOWSPR,        1,           0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSwitchHooked",        ZTID_BOOL,          GETTER,       LWSWHOOKED,           1,           0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSwitchHooked",        ZTID_VOID,          SETTER,       LWSWHOOKED,           1,           0,                                    2,           { ZTID_LWPN, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Switch",                 ZTID_BOOL,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrowning",            ZTID_FLOAT,         GETTER,       LWPNDROWNCLK,         1,           0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrowning",            ZTID_VOID,          SETTER,       LWPNDROWNCLK,         1,           0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrownCombo",          ZTID_FLOAT,         GETTER,       LWPNDROWNCMB,         1,           0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrownCombo",          ZTID_VOID,          SETTER,       LWPNDROWNCMB,         1,           0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFakeZ",               ZTID_FLOAT,         GETTER,       LWPNFAKEZ,            1,           0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFakeZ",               ZTID_VOID,          SETTER,       LWPNFAKEZ,            1,           0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFakeJump",            ZTID_FLOAT,         GETTER,       LWPNFAKEJUMP,         1,           0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFakeJump",            ZTID_VOID,          SETTER,       LWPNFAKEJUMP,         1,           0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShadowXOffset",       ZTID_FLOAT,         GETTER,       LWPNSHADOWXOFS,       1,           0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShadowXOffset",       ZTID_VOID,          SETTER,       LWPNSHADOWXOFS,       1,           0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShadowYOffset",       ZTID_FLOAT,         GETTER,       LWPNSHADOWYOFS,       1,           0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShadowYOffset",       ZTID_VOID,          SETTER,       LWPNSHADOWYOFS,       1,           0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDegAngle",            ZTID_FLOAT,         GETTER,       LWPNDEGANGLE,         1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDegAngle",            ZTID_VOID,          SETTER,       LWPNDEGANGLE,         1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getVx",                  ZTID_FLOAT,         GETTER,       LWPNVX,               1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setVx",                  ZTID_VOID,          SETTER,       LWPNVX,               1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getVy",                  ZTID_FLOAT,         GETTER,       LWPNVY,               1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setVy",                  ZTID_VOID,          SETTER,       LWPNVY,               1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAutoRotate",          ZTID_BOOL,          GETTER,       LWPNAUTOROTATE,       1,             0,                                    1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAutoRotate",          ZTID_VOID,          SETTER,       LWPNAUTOROTATE,       1,             0,                                    2,           { ZTID_LWPN, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MakeAngular",            ZTID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MakeDirectional",        ZTID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlags[]",             ZTID_BOOL,          GETTER,       LWPNFLAGS,            1,             0,                                    2,           { ZTID_LWPN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlags[]",             ZTID_VOID,          SETTER,       LWPNFLAGS,            1,             0,                                    3,           { ZTID_LWPN, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "",                       -1,                       -1,           -1,                   -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

HeroWeaponSymbols::HeroWeaponSymbols()
{
    table = lwpnTable;
    refVar = REFLWPN;
}

void HeroWeaponSymbols::generateCode()
{
	//bool isValid(lweapon)
	{
		Function* function = getFunction("isValid", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Check validity
		addOpcode2 (code, new OIsValidLWpn(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void Explode(lweapon, int32_t)
	{
		Function* function = getFunction("Explode", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OLWeaponExplodeRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void UseSprite(lweapon, int32_t val)
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
		addOpcode2 (code, new OUseSpriteLWpn(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void Remove(lweapon)
	{
		Function* function = getFunction("Remove", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		POPREF();
		LABELBACK(label);
		//Break shield
		addOpcode2 (code, new OLWpnRemove());
		RETURN();
		function->giveCode(code);
	}
	//bool Switch(lweapon, int)
	{
		Function* function = getFunction("Switch", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSwitchLW(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void MakeAngular(lweapon)
	{
		Function* function = getFunction("MakeAngular", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Check validity
		addOpcode2 (code, new OMakeAngularLwpn(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void MakeDirectional(lweapon)
	{
		Function* function = getFunction("MakeDirectional", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Check validity
		addOpcode2 (code, new OMakeDirectionalLwpn(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
}

