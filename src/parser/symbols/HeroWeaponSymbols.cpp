#include "SymbolDefs.h"

HeroWeaponSymbols HeroWeaponSymbols::singleton = HeroWeaponSymbols();

static AccessorTable lwpnTable[] =
{
//	  name,                     rettype,                  setorget,     var,                  numindex,      funcFlags,                            numParams,   params
	{ "getX",                   ZVARTYPEID_FLOAT,         GETTER,       LWPNX,                1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setX",                   ZVARTYPEID_VOID,          SETTER,       LWPNX,                1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSpecial",                   ZVARTYPEID_FLOAT,         GETTER,       LWPNSPECIAL,                1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSpecial",                   ZVARTYPEID_VOID,          SETTER,       LWPNSPECIAL,                1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Max",                   ZVARTYPEID_FLOAT,         GETTER,       SPRITEMAXLWPN,                1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Max",                   ZVARTYPEID_VOID,          SETTER,       SPRITEMAXLWPN,                1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getY",                   ZVARTYPEID_FLOAT,         GETTER,       LWPNY,                1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setY",                   ZVARTYPEID_VOID,          SETTER,       LWPNY,                1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getZ",                   ZVARTYPEID_FLOAT,         GETTER,       LWPNZ,                1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setZ",                   ZVARTYPEID_VOID,          SETTER,       LWPNZ,                1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getJump",                ZVARTYPEID_FLOAT,         GETTER,       LWPNJUMP,             1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setJump",                ZVARTYPEID_VOID,          SETTER,       LWPNJUMP,             1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDir",                 ZVARTYPEID_FLOAT,         GETTER,       LWPNDIR,              1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDir",                 ZVARTYPEID_VOID,          SETTER,       LWPNDIR,              1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAngle",               ZVARTYPEID_FLOAT,         GETTER,       LWPNANGLE,            1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAngle",               ZVARTYPEID_VOID,          SETTER,       LWPNANGLE,            1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStep",                ZVARTYPEID_FLOAT,         GETTER,       LWPNSTEP,             1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStep",                ZVARTYPEID_VOID,          SETTER,       LWPNSTEP,             1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getNumFrames",           ZVARTYPEID_FLOAT,         GETTER,       LWPNFRAMES,           1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNumFrames",           ZVARTYPEID_VOID,          SETTER,       LWPNFRAMES,           1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFrame",               ZVARTYPEID_FLOAT,         GETTER,       LWPNFRAME,            1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFrame",               ZVARTYPEID_VOID,          SETTER,       LWPNFRAME,            1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawStyle",           ZVARTYPEID_FLOAT,         GETTER,       LWPNDRAWTYPE,         1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawStyle",           ZVARTYPEID_VOID,          SETTER,       LWPNDRAWTYPE,         1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPower",               ZVARTYPEID_FLOAT,         GETTER,       LWPNPOWER,            1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPower",               ZVARTYPEID_VOID,          SETTER,       LWPNPOWER,            1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDamage",              ZVARTYPEID_FLOAT,         GETTER,       LWPNPOWER,            1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDamage",              ZVARTYPEID_VOID,          SETTER,       LWPNPOWER,            1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getID",                  ZVARTYPEID_FLOAT,         GETTER,       LWPNID,               1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setID",                  ZVARTYPEID_VOID,          SETTER,       LWPNID,               1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getType",                ZVARTYPEID_FLOAT,         GETTER,       LWPNID,               1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setType",                ZVARTYPEID_VOID,          SETTER,       LWPNID,               1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFamily",              ZVARTYPEID_FLOAT,         GETTER,       LWPNID,               1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFamily",              ZVARTYPEID_VOID,          SETTER,       LWPNID,               1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAngular",             ZVARTYPEID_BOOL,          GETTER,       LWPNANGULAR,          1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAngular",             ZVARTYPEID_VOID,          SETTER,       LWPNANGULAR,          1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getBehind",              ZVARTYPEID_BOOL,          GETTER,       LWPNBEHIND,           1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setBehind",              ZVARTYPEID_VOID,          SETTER,       LWPNBEHIND,           1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getASpeed",              ZVARTYPEID_FLOAT,         GETTER,       LWPNASPEED,           1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setASpeed",              ZVARTYPEID_VOID,          SETTER,       LWPNASPEED,           1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTile",                ZVARTYPEID_FLOAT,         GETTER,       LWPNTILE,             1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTile",                ZVARTYPEID_VOID,          SETTER,       LWPNTILE,             1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScriptTile",          ZVARTYPEID_FLOAT,         GETTER,       LWPNSCRIPTTILE,       1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScriptTile",          ZVARTYPEID_VOID,          SETTER,       LWPNSCRIPTTILE,       1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScriptFlip",          ZVARTYPEID_FLOAT,         GETTER,       LWPNSCRIPTFLIP,       1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScriptFlip",          ZVARTYPEID_VOID,          SETTER,       LWPNSCRIPTFLIP,       1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlashCSet",           ZVARTYPEID_FLOAT,         GETTER,       LWPNFLASHCSET,        1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlashCSet",           ZVARTYPEID_VOID,          SETTER,       LWPNFLASHCSET,        1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDeadState",           ZVARTYPEID_FLOAT,         GETTER,       LWPNDEAD,             1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDeadState",           ZVARTYPEID_VOID,          SETTER,       LWPNDEAD,             1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCSet",                ZVARTYPEID_FLOAT,         GETTER,       LWPNCSET,             1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCSet",                ZVARTYPEID_VOID,          SETTER,       LWPNCSET,             1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlash",               ZVARTYPEID_BOOL,          GETTER,       LWPNFLASH,            1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlash",               ZVARTYPEID_VOID,          SETTER,       LWPNFLASH,            1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlip",                ZVARTYPEID_FLOAT,         GETTER,       LWPNFLIP,             1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlip",                ZVARTYPEID_VOID,          SETTER,       LWPNFLIP,             1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getOriginalTile",        ZVARTYPEID_FLOAT,         GETTER,       LWPNOTILE,            1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setOriginalTile",        ZVARTYPEID_VOID,          SETTER,       LWPNOTILE,            1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getOriginalCSet",        ZVARTYPEID_FLOAT,         GETTER,       LWPNOCSET,            1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setOriginalCSet",        ZVARTYPEID_VOID,          SETTER,       LWPNOCSET,            1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getExtend",              ZVARTYPEID_FLOAT,         GETTER,       LWPNEXTEND,           1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setExtend",              ZVARTYPEID_VOID,          SETTER,       LWPNEXTEND,           1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitWidth",            ZVARTYPEID_FLOAT,         GETTER,       LWPNHXSZ,             1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitWidth",            ZVARTYPEID_VOID,          SETTER,       LWPNHXSZ,             1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitHeight",           ZVARTYPEID_FLOAT,         GETTER,       LWPNHYSZ,             1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitHeight",           ZVARTYPEID_VOID,          SETTER,       LWPNHYSZ,             1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitZHeight",          ZVARTYPEID_FLOAT,         GETTER,       LWPNHZSZ,             1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitZHeight",          ZVARTYPEID_VOID,          SETTER,       LWPNHZSZ,             1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTileWidth",           ZVARTYPEID_FLOAT,         GETTER,       LWPNTXSZ,             1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWidth",           ZVARTYPEID_VOID,          SETTER,       LWPNTXSZ,             1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTileHeight",          ZVARTYPEID_FLOAT,         GETTER,       LWPNTYSZ,             1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileHeight",          ZVARTYPEID_VOID,          SETTER,       LWPNTYSZ,             1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawXOffset",         ZVARTYPEID_FLOAT,         GETTER,       LWPNXOFS,             1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawXOffset",         ZVARTYPEID_VOID,          SETTER,       LWPNXOFS,             1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawYOffset",         ZVARTYPEID_FLOAT,         GETTER,       LWPNYOFS,             1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawYOffset",         ZVARTYPEID_VOID,          SETTER,       LWPNYOFS,             1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTotalDYOffset",       ZVARTYPEID_FLOAT,         GETTER,       LWPNTOTALDYOFFS,      1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTotalDYOffset",       ZVARTYPEID_VOID,          SETTER,       LWPNTOTALDYOFFS,      1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawZOffset",         ZVARTYPEID_FLOAT,         GETTER,       LWPNZOFS,             1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawZOffset",         ZVARTYPEID_VOID,          SETTER,       LWPNZOFS,             1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitXOffset",          ZVARTYPEID_FLOAT,         GETTER,       LWPNHXOFS,            1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitXOffset",          ZVARTYPEID_VOID,          SETTER,       LWPNHXOFS,            1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitYOffset",          ZVARTYPEID_FLOAT,         GETTER,       LWPNHYOFS,            1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitYOffset",          ZVARTYPEID_VOID,          SETTER,       LWPNHYOFS,            1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "isValid",                ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "UseSprite",              ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMisc[]",              ZVARTYPEID_UNTYPED,       GETTER,       LWPNMISCD,            32,            0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMisc[]",              ZVARTYPEID_VOID,          SETTER,       LWPNMISCD,            32,            0,                                    3,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCollDetection",       ZVARTYPEID_BOOL,          GETTER,       LWPNCOLLDET,          1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCollDetection",       ZVARTYPEID_VOID,          SETTER,       LWPNCOLLDET,          1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAnimation",           ZVARTYPEID_BOOL,          GETTER,       LWPNENGINEANIMATE,    1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAnimation",           ZVARTYPEID_VOID,          SETTER,       LWPNENGINEANIMATE,    1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "getRange",               ZVARTYPEID_FLOAT,         GETTER,       LWPNRANGE,            1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "setRange",               ZVARTYPEID_VOID,          SETTER,       LWPNRANGE,            1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getParent",              ZVARTYPEID_FLOAT,         GETTER,       LWPNPARENT,           1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setParent",              ZVARTYPEID_VOID,          SETTER,       LWPNPARENT,           1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLevel",               ZVARTYPEID_FLOAT,         GETTER,       LWPNLEVEL,            1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLevel",               ZVARTYPEID_VOID,          SETTER,       LWPNLEVEL,            1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScript",              ZVARTYPEID_FLOAT,         GETTER,       LWPNSCRIPT,           1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScript",              ZVARTYPEID_VOID,          SETTER,       LWPNSCRIPT,           1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWeapon",              ZVARTYPEID_FLOAT,         GETTER,       LWPNUSEWEAPON,        1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWeapon",              ZVARTYPEID_VOID,          SETTER,       LWPNUSEWEAPON,        1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDefense",             ZVARTYPEID_FLOAT,         GETTER,       LWPNUSEDEFENCE,       1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDefense",             ZVARTYPEID_VOID,          SETTER,       LWPNUSEDEFENCE,       1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDefence",             ZVARTYPEID_FLOAT,         GETTER,       LWPNUSEDEFENCE,       1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDefence",             ZVARTYPEID_VOID,          SETTER,       LWPNUSEDEFENCE,       1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getInitD[]",             ZVARTYPEID_UNTYPED,       GETTER,       LWPNINITD,            8,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInitD[]",             ZVARTYPEID_VOID,          SETTER,       LWPNINITD,            8,             0,                                    3,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getUID",                 ZVARTYPEID_FLOAT,         GETTER,       LWEAPONSCRIPTUID,     1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Explode",                ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRotation",            ZVARTYPEID_FLOAT,         GETTER,       LWPNROTATION,         1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRotation",            ZVARTYPEID_VOID,          SETTER,       LWPNROTATION,         1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScale",               ZVARTYPEID_FLOAT,         GETTER,       LWPNSCALE,            1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScale",               ZVARTYPEID_VOID,          SETTER,       LWPNSCALE,            1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getGravity",             ZVARTYPEID_BOOL,          GETTER,       LWPNGRAVITY,          1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setGravity",             ZVARTYPEID_VOID,          SETTER,       LWPNGRAVITY,          1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFalling",             ZVARTYPEID_FLOAT,         GETTER,       LWPNFALLCLK,          1,           0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFalling",             ZVARTYPEID_VOID,          SETTER,       LWPNFALLCLK,          1,           0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFallCombo",           ZVARTYPEID_FLOAT,         GETTER,       LWPNFALLCMB,          1,           0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFallCombo",           ZVARTYPEID_VOID,          SETTER,       LWPNFALLCMB,          1,           0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMoveFlags[]",         ZVARTYPEID_BOOL,          GETTER,       LWPNMOVEFLAGS,        2,           0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMoveFlags[]",         ZVARTYPEID_VOID,          SETTER,       LWPNMOVEFLAGS,        2,           0,                                    3,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLightRadius",         ZVARTYPEID_FLOAT,         GETTER,       LWPNGLOWRAD,          1,           0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLightRadius",         ZVARTYPEID_VOID,          SETTER,       LWPNGLOWRAD,          1,           0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLightShape",          ZVARTYPEID_FLOAT,         GETTER,       LWPNGLOWSHP,          1,           0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLightShape",          ZVARTYPEID_VOID,          SETTER,       LWPNGLOWSHP,          1,           0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getUnblockable",         ZVARTYPEID_FLOAT,         GETTER,       LWPNUNBL,             1,           0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setUnblockable",         ZVARTYPEID_VOID,          SETTER,       LWPNUNBL,             1,           0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Remove",                 ZVARTYPEID_VOID,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShadowSprite",        ZVARTYPEID_FLOAT,         GETTER,       LWPNSHADOWSPR,        1,           0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShadowSprite",        ZVARTYPEID_VOID,          SETTER,       LWPNSHADOWSPR,        1,           0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSwitchHooked",        ZVARTYPEID_BOOL,          GETTER,       LWSWHOOKED,           1,           0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSwitchHooked",        ZVARTYPEID_VOID,          SETTER,       LWSWHOOKED,           1,           0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Switch",                 ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrowning",            ZVARTYPEID_FLOAT,         GETTER,       LWPNDROWNCLK,         1,           0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrowning",            ZVARTYPEID_VOID,          SETTER,       LWPNDROWNCLK,         1,           0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrownCombo",          ZVARTYPEID_FLOAT,         GETTER,       LWPNDROWNCMB,         1,           0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrownCombo",          ZVARTYPEID_VOID,          SETTER,       LWPNDROWNCMB,         1,           0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFakeZ",               ZVARTYPEID_FLOAT,         GETTER,       LWPNFAKEZ,            1,           0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFakeZ",               ZVARTYPEID_VOID,          SETTER,       LWPNFAKEZ,            1,           0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFakeJump",            ZVARTYPEID_FLOAT,         GETTER,       LWPNFAKEJUMP,         1,           0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFakeJump",            ZVARTYPEID_VOID,          SETTER,       LWPNFAKEJUMP,         1,           0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShadowXOffset",       ZVARTYPEID_FLOAT,         GETTER,       LWPNSHADOWXOFS,       1,           0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShadowXOffset",       ZVARTYPEID_VOID,          SETTER,       LWPNSHADOWXOFS,       1,           0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShadowYOffset",       ZVARTYPEID_FLOAT,         GETTER,       LWPNSHADOWYOFS,       1,           0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShadowYOffset",       ZVARTYPEID_VOID,          SETTER,       LWPNSHADOWYOFS,       1,           0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDegAngle",            ZVARTYPEID_FLOAT,         GETTER,       LWPNDEGANGLE,         1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDegAngle",            ZVARTYPEID_VOID,          SETTER,       LWPNDEGANGLE,         1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getVx",                  ZVARTYPEID_FLOAT,         GETTER,       LWPNVX,               1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setVx",                  ZVARTYPEID_VOID,          SETTER,       LWPNVX,               1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getVy",                  ZVARTYPEID_FLOAT,         GETTER,       LWPNVY,               1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setVy",                  ZVARTYPEID_VOID,          SETTER,       LWPNVY,               1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAutoRotate",          ZVARTYPEID_BOOL,          GETTER,       LWPNAUTOROTATE,       1,             0,                                    1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAutoRotate",          ZVARTYPEID_VOID,          SETTER,       LWPNAUTOROTATE,       1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MakeAngular",            ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MakeDirectional",        ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlags[]",             ZVARTYPEID_BOOL,          GETTER,       LWPNFLAGS,            1,             0,                                    2,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlags[]",             ZVARTYPEID_VOID,          SETTER,       LWPNFLAGS,            1,             0,                                    3,           { ZVARTYPEID_LWPN, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
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

