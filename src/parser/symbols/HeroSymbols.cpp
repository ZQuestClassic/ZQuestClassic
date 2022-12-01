#include "SymbolDefs.h"

HeroSymbols HeroSymbols::singleton = HeroSymbols();

static AccessorTable HeroSTable[] =
{
//	  name,                     rettype,                  setorget,     var,                  numindex,    funcFlags,                            numParams,   params
	{ "getX",                   ZTID_FLOAT,         GETTER,       LINKX,                1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setX",                   ZTID_VOID,          SETTER,       LINKX,                1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getY",                   ZTID_FLOAT,         GETTER,       LINKY,                1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setY",                   ZTID_VOID,          SETTER,       LINKY,                1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getZ",                   ZTID_FLOAT,         GETTER,       LINKZ,                1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setZ",                   ZTID_VOID,          SETTER,       LINKZ,                1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getJump",                ZTID_FLOAT,         GETTER,       LINKJUMP,             1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setJump",                ZTID_VOID,          SETTER,       LINKJUMP,             1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDir",                 ZTID_FLOAT,         GETTER,       LINKDIR,              1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDir",                 ZTID_VOID,          SETTER,       LINKDIR,              1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitDir",              ZTID_FLOAT,         GETTER,       LINKHITDIR,           1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitDir",              ZTID_VOID,          SETTER,       LINKHITDIR,           1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSwordJinx",           ZTID_FLOAT,         GETTER,       LINKSWORDJINX,        1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSwordJinx",           ZTID_VOID,          SETTER,       LINKSWORDJINX,        1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getItemJinx",            ZTID_FLOAT,         GETTER,       LINKITEMJINX,         1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItemJinx",            ZTID_VOID,          SETTER,       LINKITEMJINX,         1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHP",                  ZTID_FLOAT,         GETTER,       LINKHP,               1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHP",                  ZTID_VOID,          SETTER,       LINKHP,               1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMP",                  ZTID_FLOAT,         GETTER,       LINKMP,               1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMP",                  ZTID_VOID,          SETTER,       LINKMP,               1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMaxHP",               ZTID_FLOAT,         GETTER,       LINKMAXHP,            1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMaxHP",               ZTID_VOID,          SETTER,       LINKMAXHP,            1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMaxMP",               ZTID_FLOAT,         GETTER,       LINKMAXMP,            1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMaxMP",               ZTID_VOID,          SETTER,       LINKMAXMP,            1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAction",              ZTID_FLOAT,         GETTER,       LINKACTION,           1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAction",              ZTID_VOID,          SETTER,       LINKACTION,           1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTileMod",             ZTID_FLOAT,         GETTER,       LINKTILEMOD,          1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileMod",             ZTID_VOID,          SETTER,       LINKTILEMOD,          1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHeldItem",            ZTID_FLOAT,         GETTER,       LINKHELD,             1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHeldItem",            ZTID_VOID,          SETTER,       LINKHELD,             1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Warp",                   ZTID_VOID,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      3,           { ZTID_PLAYER, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Warp",                   ZTID_VOID,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "WarpEx",                 ZTID_VOID,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Explode",                ZTID_VOID,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "PitWarp",                ZTID_VOID,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      3,           { ZTID_PLAYER, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputStart",          ZTID_BOOL,          GETTER,       INPUTSTART,           1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputStart",          ZTID_VOID,          SETTER,       INPUTSTART,           1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputMap",            ZTID_BOOL,          GETTER,       INPUTMAP,             1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputMap",            ZTID_VOID,          SETTER,       INPUTMAP,             1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputUp",             ZTID_BOOL,          GETTER,       INPUTUP,              1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputUp",             ZTID_VOID,          SETTER,       INPUTUP,              1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputDown",           ZTID_BOOL,          GETTER,       INPUTDOWN,            1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputDown",           ZTID_VOID,          SETTER,       INPUTDOWN,            1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputLeft",           ZTID_BOOL,          GETTER,       INPUTLEFT,            1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputLeft",           ZTID_VOID,          SETTER,       INPUTLEFT,            1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputRight",          ZTID_BOOL,          GETTER,       INPUTRIGHT,           1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputRight",          ZTID_VOID,          SETTER,       INPUTRIGHT,           1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputA",              ZTID_BOOL,          GETTER,       INPUTA,               1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputA",              ZTID_VOID,          SETTER,       INPUTA,               1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputB",              ZTID_BOOL,          GETTER,       INPUTB,               1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputB",              ZTID_VOID,          SETTER,       INPUTB,               1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputL",              ZTID_BOOL,          GETTER,       INPUTL,               1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputL",              ZTID_VOID,          SETTER,       INPUTL,               1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputR",              ZTID_BOOL,          GETTER,       INPUTR,               1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputR",              ZTID_VOID,          SETTER,       INPUTR,               1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputEx1",            ZTID_BOOL,          GETTER,       INPUTEX1,             1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputEx1",            ZTID_VOID,          SETTER,       INPUTEX1,             1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputEx2",            ZTID_BOOL,          GETTER,       INPUTEX2,             1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputEx2",            ZTID_VOID,          SETTER,       INPUTEX2,             1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputEx3",            ZTID_BOOL,          GETTER,       INPUTEX3,             1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputEx3",            ZTID_VOID,          SETTER,       INPUTEX3,             1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputEx4",            ZTID_BOOL,          GETTER,       INPUTEX4,             1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputEx4",            ZTID_VOID,          SETTER,       INPUTEX4,             1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressStart",          ZTID_BOOL,          GETTER,       INPUTPRESSSTART,      1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressStart",          ZTID_VOID,          SETTER,       INPUTPRESSSTART,      1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressUp",             ZTID_BOOL,          GETTER,       INPUTPRESSUP,         1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressUp",             ZTID_VOID,          SETTER,       INPUTPRESSUP,         1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressDown",           ZTID_BOOL,          GETTER,       INPUTPRESSDOWN,       1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressDown",           ZTID_VOID,          SETTER,       INPUTPRESSDOWN,       1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressLeft",           ZTID_BOOL,          GETTER,       INPUTPRESSLEFT,       1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressLeft",           ZTID_VOID,          SETTER,       INPUTPRESSLEFT,       1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressRight",          ZTID_BOOL,          GETTER,       INPUTPRESSRIGHT,      1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressRight",          ZTID_VOID,          SETTER,       INPUTPRESSRIGHT,      1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressA",              ZTID_BOOL,          GETTER,       INPUTPRESSA,          1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressA",              ZTID_VOID,          SETTER,       INPUTPRESSA,          1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressB",              ZTID_BOOL,          GETTER,       INPUTPRESSB,          1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressB",              ZTID_VOID,          SETTER,       INPUTPRESSB,          1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressL",              ZTID_BOOL,          GETTER,       INPUTPRESSL,          1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressL",              ZTID_VOID,          SETTER,       INPUTPRESSL,          1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressR",              ZTID_BOOL,          GETTER,       INPUTPRESSR,          1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressR",              ZTID_VOID,          SETTER,       INPUTPRESSR,          1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressEx1",            ZTID_BOOL,          GETTER,       INPUTPRESSEX1,        1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressEx1",            ZTID_VOID,          SETTER,       INPUTPRESSEX1,        1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressEx2",            ZTID_BOOL,          GETTER,       INPUTPRESSEX2,        1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressEx2",            ZTID_VOID,          SETTER,       INPUTPRESSEX2,        1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressEx3",            ZTID_BOOL,          GETTER,       INPUTPRESSEX3,        1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressEx3",            ZTID_VOID,          SETTER,       INPUTPRESSEX3,        1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressEx4",            ZTID_BOOL,          GETTER,       INPUTPRESSEX4,        1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressEx4",            ZTID_VOID,          SETTER,       INPUTPRESSEX4,        1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputMouseX",         ZTID_FLOAT,         GETTER,       INPUTMOUSEX,          1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputMouseX",         ZTID_VOID,          SETTER,       INPUTMOUSEX,          1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputMouseY",         ZTID_FLOAT,         GETTER,       INPUTMOUSEY,          1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputMouseY",         ZTID_VOID,          SETTER,       INPUTMOUSEY,          1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputMouseZ",         ZTID_FLOAT,         GETTER,       INPUTMOUSEZ,          1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputMouseZ",         ZTID_VOID,          SETTER,       INPUTMOUSEZ,          1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputMouseB",         ZTID_FLOAT,         GETTER,       INPUTMOUSEB,          1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputMouseB",         ZTID_VOID,          SETTER,       INPUTMOUSEB,          1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getItem[]",              ZTID_BOOL,          GETTER,       LINKITEMD,            256,         0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItem[]",              ZTID_VOID,          SETTER,       LINKITEMD,            256,         0,                                    3,           { ZTID_PLAYER, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSteps[]",             ZTID_FLOAT,         GETTER,       HEROSTEPS,            8,         0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSteps[]",             ZTID_VOID,          SETTER,       HEROSTEPS,            8,         0,                                    3,           { ZTID_PLAYER, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getHitWidth",            ZTID_FLOAT,         GETTER,       LINKHXSZ,             1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setHitWidth",            ZTID_VOID,          SETTER,       LINKHXSZ,             1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStep",                ZTID_FLOAT,         GETTER,       HEROSTEPRATE,         1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStep",                ZTID_VOID,          SETTER,       HEROSTEPRATE,         1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getHitHeight",           ZTID_FLOAT,         GETTER,       LINKHYSZ,             1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setHitHeight",           ZTID_VOID,          SETTER,       LINKHYSZ,             1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getHitZHeight",          ZTID_FLOAT,         GETTER,       LINKHZSZ,             1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setHitZHeight",          ZTID_VOID,          SETTER,       LINKHZSZ,             1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getTileWidth",           ZTID_FLOAT,         GETTER,       LINKTXSZ,             1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setTileWidth",           ZTID_VOID,          SETTER,       LINKTXSZ,             1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getTileHeight",          ZTID_FLOAT,         GETTER,       LINKTYSZ,             1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setTileHeight",          ZTID_VOID,          SETTER,       LINKTYSZ,             1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getDrawXOffset",         ZTID_FLOAT,         GETTER,       LINKXOFS,             1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setDrawXOffset",         ZTID_VOID,          SETTER,       LINKXOFS,             1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getDrawYOffset",         ZTID_FLOAT,         GETTER,       LINKYOFS,             1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setDrawYOffset",         ZTID_VOID,          SETTER,       LINKYOFS,             1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getTotalDYOffset",       ZTID_FLOAT,         GETTER,       HEROTOTALDYOFFS,      1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setTotalDYOffset",       ZTID_VOID,          SETTER,       HEROTOTALDYOFFS,      1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getDrawZOffset",         ZTID_FLOAT,         GETTER,       LINKZOFS,             1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setDrawZOffset",         ZTID_VOID,          SETTER,       LINKZOFS,             1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getHitXOffset",          ZTID_FLOAT,         GETTER,       LINKHXOFS,            1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setHitXOffset",          ZTID_VOID,          SETTER,       LINKHXOFS,            1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getHitYOffset",          ZTID_FLOAT,         GETTER,       LINKHYOFS,            1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setHitYOffset",          ZTID_VOID,          SETTER,       LINKHYOFS,            1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrunk",               ZTID_FLOAT,         GETTER,       LINKDRUNK,            1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrunk",               ZTID_VOID,          SETTER,       LINKDRUNK,            1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getEquipment",           ZTID_FLOAT,         GETTER,       LINKEQUIP,            1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setEquipment",           ZTID_VOID,          SETTER,       LINKEQUIP,            1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputAxisUp",         ZTID_BOOL,          GETTER,       INPUTAXISUP,          1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputAxisUp",         ZTID_VOID,          SETTER,       INPUTAXISUP,          1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputAxisDown",       ZTID_BOOL,          GETTER,       INPUTAXISDOWN,        1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputAxisDown",       ZTID_VOID,          SETTER,       INPUTAXISDOWN,        1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputAxisLeft",       ZTID_BOOL,          GETTER,       INPUTAXISLEFT,        1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputAxisLeft",       ZTID_VOID,          SETTER,       INPUTAXISLEFT,        1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputAxisRight",      ZTID_BOOL,          GETTER,       INPUTAXISRIGHT,       1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputAxisRight",      ZTID_VOID,          SETTER,       INPUTAXISRIGHT,       1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressAxisUp",         ZTID_BOOL,          GETTER,       INPUTPRESSAXISUP,     1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressAxisUp",         ZTID_VOID,          SETTER,       INPUTPRESSAXISUP,     1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressAxisDown",       ZTID_BOOL,          GETTER,       INPUTPRESSAXISDOWN,   1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressAxisDown",       ZTID_VOID,          SETTER,       INPUTPRESSAXISDOWN,   1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressAxisLeft",       ZTID_BOOL,          GETTER,       INPUTPRESSAXISLEFT,   1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressAxisLeft",       ZTID_VOID,          SETTER,       INPUTPRESSAXISLEFT,   1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressAxisRight",      ZTID_BOOL,          GETTER,       INPUTPRESSAXISRIGHT,  1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressAxisRight",      ZTID_VOID,          SETTER,       INPUTPRESSAXISRIGHT,  1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHealthBeep",          ZTID_FLOAT,         GETTER,       HEROHEALTHBEEP,            1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHealthBeep",          ZTID_VOID,          SETTER,       HEROHEALTHBEEP,            1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getInvisible",           ZTID_BOOL,          GETTER,       LINKINVIS,            1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInvisible",           ZTID_VOID,          SETTER,       LINKINVIS,            1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getNoStepForward",       ZTID_BOOL,          GETTER,       HERONOSTEPFORWARD,    1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNoStepForward",       ZTID_VOID,          SETTER,       HERONOSTEPFORWARD,    1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAnimation",           ZTID_FLOAT,         GETTER,       LINKENGINEANIMATE,    1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAnimation",           ZTID_VOID,          SETTER,       LINKENGINEANIMATE,    1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCollDetection",       ZTID_BOOL,          GETTER,       LINKINVINC,           1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCollDetection",       ZTID_VOID,          SETTER,       LINKINVINC,           1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMisc[]",              ZTID_UNTYPED,       GETTER,       LINKMISCD,            32,          0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMisc[]",              ZTID_VOID,          SETTER,       LINKMISCD,            32,          0,                                    3,           { ZTID_PLAYER, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDefense[]",           ZTID_FLOAT,         GETTER,       LINKDEFENCE,          256,         0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDefense[]",           ZTID_VOID,          SETTER,       LINKDEFENCE,          256,         0,                                    3,           { ZTID_PLAYER, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitBy[]",             ZTID_UNTYPED,       GETTER,       LINKHITBY,            10,          0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitBy[]",             ZTID_VOID,          SETTER,       LINKHITBY,            10,          0,                                    3,           { ZTID_PLAYER, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLadderX",             ZTID_FLOAT,         GETTER,       LINKLADDERX,          1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLadderY",             ZTID_FLOAT,         GETTER,       LINKLADDERY,          1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTile",                ZTID_FLOAT,         GETTER,       LINKTILE,             1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTile",                ZTID_VOID,          SETTER,       LINKTILE,             1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlip",                ZTID_FLOAT,         GETTER,       LINKFLIP,             1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlip",                ZTID_VOID,          SETTER,       LINKFLIP,             1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressMap",            ZTID_BOOL,          GETTER,       INPUTPRESSMAP,        1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressMap",            ZTID_VOID,          SETTER,       INPUTPRESSMAP,        1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SelectAWeapon",          ZTID_VOID,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SelectBWeapon",          ZTID_VOID,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SelectXWeapon",          ZTID_VOID,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SelectYWeapon",          ZTID_VOID,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInvFrames",           ZTID_VOID,          SETTER,       LINKINVFRAME,         1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getInvFrames",           ZTID_FLOAT,         GETTER,       LINKINVFRAME,         1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInvFlicker",          ZTID_VOID,          SETTER,       LINKCANFLICKER,       1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getInvFlicker",          ZTID_BOOL,          GETTER,       LINKCANFLICKER,       1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHurtSound",           ZTID_VOID,          SETTER,       LINKHURTSFX,          1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHurtSound",           ZTID_FLOAT,         GETTER,       LINKHURTSFX,          1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "getUsingItem",           ZTID_FLOAT,         GETTER,       LINKUSINGITEM,        1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "setUsingItem",           ZTID_VOID,          SETTER,       LINKUSINGITEM,        1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "getAttack",              ZTID_FLOAT,         GETTER,       LINKUSINGITEMA,       1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "setAttack",              ZTID_VOID,          SETTER,       LINKUSINGITEMA,       1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetItemA",               ZTID_VOID,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetItemB",               ZTID_VOID,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "SetItemSlot",            ZTID_VOID,          FUNCTION,     0,                    1,           0,                                    4,           { ZTID_PLAYER, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItemB",               ZTID_VOID,          SETTER,       LINKITEMB,            1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItemA",               ZTID_VOID,          SETTER,       LINKITEMA,            1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getItemB",               ZTID_FLOAT,         GETTER,       LINKITEMB,            1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getItemA",               ZTID_FLOAT,         GETTER,       LINKITEMA,            1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getItemX",               ZTID_FLOAT,         GETTER,       LINKITEMX,            1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItemX",               ZTID_VOID,          SETTER,       LINKITEMX,            1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getItemY",               ZTID_FLOAT,         GETTER,       LINKITEMY,            1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItemY",               ZTID_VOID,          SETTER,       LINKITEMY,            1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getEaten",               ZTID_FLOAT,         GETTER,       LINKEATEN,            1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setEaten",               ZTID_VOID,          SETTER,       LINKEATEN,            1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getGrabbed",             ZTID_BOOL,          GETTER,       LINKGRABBED,          1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setGrabbed",             ZTID_VOID,          SETTER,       LINKGRABBED,          1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStun",                ZTID_FLOAT,         GETTER,       LINKSTUN,             1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStun",                ZTID_VOID,          SETTER,       LINKSTUN,             1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getBunnyClk",            ZTID_FLOAT,         GETTER,       HEROBUNNY,            1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setBunnyClk",            ZTID_VOID,          SETTER,       HEROBUNNY,            1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPushing",             ZTID_FLOAT,         GETTER,       LINKPUSH,             1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPushing",             ZTID_VOID,          SETTER,       LINKPUSH,             1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScriptCSet",          ZTID_FLOAT,         GETTER,       HEROSCRIPTCSET,       1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScriptCSet",          ZTID_VOID,          SETTER,       HEROSCRIPTCSET,       1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScriptTile",          ZTID_FLOAT,         GETTER,       LINKSCRIPTTILE,       1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScriptTile",          ZTID_VOID,          SETTER,       LINKSCRIPTTILE,       1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScriptFlip",          ZTID_FLOAT,         GETTER,       LINKSCRIPFLIP,        1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScriptFlip",          ZTID_VOID,          SETTER,       LINKSCRIPFLIP,        1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDiagonal",            ZTID_BOOL,          GETTER,       LINKDIAG,             1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDiagonal",            ZTID_VOID,          SETTER,       LINKDIAG,             1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getBigHitbox",           ZTID_BOOL,          GETTER,       LINKBIGHITBOX,        1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setBigHitbox",           ZTID_VOID,          SETTER,       LINKBIGHITBOX,        1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getGravity",             ZTID_BOOL,          GETTER,       LINKGRAVITY,          1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setGravity",             ZTID_VOID,          SETTER,       LINKGRAVITY,          1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRotation",            ZTID_FLOAT,         GETTER,       LINKROTATION,         1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRotation",            ZTID_VOID,          SETTER,       LINKROTATION,         1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScale",               ZTID_FLOAT,         GETTER,       LINKSCALE,            1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScale",               ZTID_VOID,          SETTER,       LINKSCALE,            1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetOriginalTile",        ZTID_FLOAT,         FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      3,           { ZTID_PLAYER, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetOriginalFlip",        ZTID_FLOAT,         FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      3,           { ZTID_PLAYER, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getClimbing",            ZTID_BOOL,          GETTER,       LINKCLIMBING,         1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setClimbing",            ZTID_VOID,          SETTER,       LINKCLIMBING,         1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getJumpCount",           ZTID_FLOAT,         GETTER,       HEROJUMPCOUNT,        1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setJumpCount",           ZTID_VOID,          SETTER,       HEROJUMPCOUNT,        1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPitPullDir",          ZTID_FLOAT,         GETTER,       HEROPULLDIR,          1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPitPullTimer",        ZTID_FLOAT,         GETTER,       HEROPULLCLK,          1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPitPullTimer",        ZTID_VOID,          SETTER,       HEROPULLCLK,          1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFalling",             ZTID_FLOAT,         GETTER,       HEROFALLCLK,          1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFalling",             ZTID_VOID,          SETTER,       HEROFALLCLK,          1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFallCombo",           ZTID_FLOAT,         GETTER,       HEROFALLCMB,          1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFallCombo",           ZTID_VOID,          SETTER,       HEROFALLCMB,          1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMoveFlags[]",         ZTID_BOOL,          GETTER,       HEROMOVEFLAGS,        2,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMoveFlags[]",         ZTID_VOID,          SETTER,       HEROMOVEFLAGS,        2,           0,                                    3,           { ZTID_PLAYER, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getClockActive",         ZTID_BOOL,          GETTER,       CLOCKACTIVE,          1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setClockActive",         ZTID_VOID,          SETTER,       CLOCKACTIVE,          1,           0,                                    2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getClockTimer",          ZTID_FLOAT,         GETTER,       CLOCKCLK,             1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setClockTimer",          ZTID_VOID,          SETTER,       CLOCKCLK,             1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCSet",                ZTID_FLOAT,         GETTER,       LINKCSET,             1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCSet",                ZTID_VOID,          SETTER,       LINKCSET,             1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getIsWarping",           ZTID_BOOL,          GETTER,       HEROISWARPING,        1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getRespawnX",            ZTID_FLOAT,         GETTER,       HERORESPAWNX,         1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRespawnX",            ZTID_VOID,          SETTER,       HERORESPAWNX,         1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRespawnY",            ZTID_FLOAT,         GETTER,       HERORESPAWNY,         1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRespawnY",            ZTID_VOID,          SETTER,       HERORESPAWNY,         1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRespawnDMap",         ZTID_FLOAT,         GETTER,       HERORESPAWNDMAP,      1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRespawnDMap",         ZTID_VOID,          SETTER,       HERORESPAWNDMAP,      1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRespawnScreen",       ZTID_FLOAT,         GETTER,       HERORESPAWNSCR,       1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRespawnScreen",       ZTID_VOID,          SETTER,       HERORESPAWNSCR,       1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSwitchTimer",         ZTID_FLOAT,         GETTER,       HEROSWITCHTIMER,      1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSwitchTimer",         ZTID_VOID,          SETTER,       HEROSWITCHTIMER,      1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSwitchMaxTimer",      ZTID_FLOAT,         GETTER,       HEROSWITCHMAXTIMER,   1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSwitchMaxTimer",      ZTID_VOID,          SETTER,       HEROSWITCHMAXTIMER,   1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getImmortal",            ZTID_FLOAT,         GETTER,       HEROIMMORTAL,         1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setImmortal",            ZTID_VOID,          SETTER,       HEROIMMORTAL,         1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SwitchCombo",            ZTID_BOOL,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      3,           { ZTID_PLAYER, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Kill",                   ZTID_VOID,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      2,           { ZTID_PLAYER, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrowning",             ZTID_FLOAT,         GETTER,       HERODROWNCLK,          1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrowning",             ZTID_VOID,          SETTER,       HERODROWNCLK,          1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrownCombo",           ZTID_FLOAT,         GETTER,       HERODROWNCMB,          1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrownCombo",           ZTID_VOID,          SETTER,       HERODROWNCMB,          1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFakeZ",           ZTID_FLOAT,         GETTER,       HEROFAKEZ,          1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFakeZ",           ZTID_VOID,          SETTER,       HEROFAKEZ,          1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFakeJump",           ZTID_FLOAT,         GETTER,       HEROFAKEJUMP,          1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFakeJump",           ZTID_VOID,          SETTER,       HEROFAKEJUMP,          1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShadowXOffset",           ZTID_FLOAT,         GETTER,       HEROSHADOWXOFS,          1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShadowXOffset",           ZTID_VOID,          SETTER,       HEROSHADOWXOFS,          1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShadowYOffset",           ZTID_FLOAT,         GETTER,       HEROSHADOWYOFS,          1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShadowYOffset",           ZTID_VOID,          SETTER,       HEROSHADOWYOFS,          1,           0,                                    2,           { ZTID_PLAYER, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStanding",                ZTID_BOOL,          GETTER,       HEROSTANDING,            1,           0,                                    1,           { ZTID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "",                       -1,                       -1,           -1,                   -1,          0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
};

HeroSymbols::HeroSymbols()
{
    table = HeroSTable;
    refVar = NUL;
}

void HeroSymbols::generateCode()
{
    //Warp(link, int32_t, int32_t)
    {
	    Function* function = getFunction("Warp", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        //pop ffc, and ignore it
        addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
        //ffc must be this (link is not a user-accessible type)
        addOpcode2 (code, new OWarp(new VarArgument(EXP2), new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    {
	Function* function = getFunction("WarpEx", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OHeroWarpExRegister(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);    
    }
    {
	Function* function = getFunction("Warp", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OHeroWarpExRegister(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);    
    }
    {
	Function* function = getFunction("Explode", 2);
	int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OHeroExplodeRegister(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);        
    }
       //void SetItemSlot(link, int32_t item, int32_t slot, int32_t force)
    {
	    Function* function = getFunction("SetItemSlot", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(SETITEMSLOT), new VarArgument(SFTEMP)));
        RETURN();
        function->giveCode(code);
    }
    
    //void SetItemA(link, int32_t)
    {
	    Function* function = getFunction("SetItemA", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        //addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        addOpcode2 (code, new OSetRegister(new VarArgument(GAMESETA), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //void SetItemB(link, int32_t)
    {
	    Function* function = getFunction("SetItemB", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        //addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        addOpcode2 (code, new OSetRegister(new VarArgument(GAMESETB), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    
    //PitWarp(link, int32_t, int32_t)
    {
	    Function* function = getFunction("PitWarp", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        //pop ffc, and ignore it
        addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
        //ffc must be this (link is not a user-accessible type)
        addOpcode2 (code, new OPitWarp(new VarArgument(EXP2), new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //SelectAWeapon(link, int32_t)
    {
	    Function* function = getFunction("SelectAWeapon", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer and ignore it
        POPREF();
        addOpcode2 (code, new OSelectAWeaponRegister(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //SelectBWeapon(link, int32_t)
    {
	    Function* function = getFunction("SelectBWeapon", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer and ignore it
        POPREF();
        addOpcode2 (code, new OSelectBWeaponRegister(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //SelectXWeapon(link, int32_t)
    {
	    Function* function = getFunction("SelectXWeapon", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer and ignore it
        POPREF();
        addOpcode2 (code, new OSelectXWeaponRegister(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //SelectYWeapon(link, int32_t)
    {
	    Function* function = getFunction("SelectYWeapon", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer and ignore it
        POPREF();
        addOpcode2 (code, new OSelectYWeaponRegister(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetOriginaTile(link, int32_t,int32_t)
    {
        Function* function = getFunction("GetOriginalTile", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(LINKOTILE)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t GetOriginalFlip(link, int32_t,int32_t)
    {
        Function* function = getFunction("GetOriginalFlip", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(LINKOFLIP)));
        RETURN();
        function->giveCode(code);
    }
	//bool SwitchCombo(link, int, int)
    {
        Function* function = getFunction("SwitchCombo", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSwitchCombo(new VarArgument(EXP1), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //bool Kill(player, bool)
    {
        Function* function = getFunction("Kill", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OKillPlayer(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
}

