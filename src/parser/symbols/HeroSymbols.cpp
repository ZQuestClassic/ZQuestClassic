#include "SymbolDefs.h"

HeroSymbols HeroSymbols::singleton = HeroSymbols();

static AccessorTable HeroSTable[] =
{
//	  name,                     rettype,                  setorget,     var,                  numindex,    funcFlags,                            numParams,   params
	{ "getX",                   ZVARTYPEID_FLOAT,         GETTER,       LINKX,                1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setX",                   ZVARTYPEID_VOID,          SETTER,       LINKX,                1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getY",                   ZVARTYPEID_FLOAT,         GETTER,       LINKY,                1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setY",                   ZVARTYPEID_VOID,          SETTER,       LINKY,                1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getZ",                   ZVARTYPEID_FLOAT,         GETTER,       LINKZ,                1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setZ",                   ZVARTYPEID_VOID,          SETTER,       LINKZ,                1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getJump",                ZVARTYPEID_FLOAT,         GETTER,       LINKJUMP,             1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setJump",                ZVARTYPEID_VOID,          SETTER,       LINKJUMP,             1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDir",                 ZVARTYPEID_FLOAT,         GETTER,       LINKDIR,              1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDir",                 ZVARTYPEID_VOID,          SETTER,       LINKDIR,              1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitDir",              ZVARTYPEID_FLOAT,         GETTER,       LINKHITDIR,           1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitDir",              ZVARTYPEID_VOID,          SETTER,       LINKHITDIR,           1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSwordJinx",           ZVARTYPEID_FLOAT,         GETTER,       LINKSWORDJINX,        1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSwordJinx",           ZVARTYPEID_VOID,          SETTER,       LINKSWORDJINX,        1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getItemJinx",            ZVARTYPEID_FLOAT,         GETTER,       LINKITEMJINX,         1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItemJinx",            ZVARTYPEID_VOID,          SETTER,       LINKITEMJINX,         1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHP",                  ZVARTYPEID_FLOAT,         GETTER,       LINKHP,               1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHP",                  ZVARTYPEID_VOID,          SETTER,       LINKHP,               1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMP",                  ZVARTYPEID_FLOAT,         GETTER,       LINKMP,               1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMP",                  ZVARTYPEID_VOID,          SETTER,       LINKMP,               1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMaxHP",               ZVARTYPEID_FLOAT,         GETTER,       LINKMAXHP,            1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMaxHP",               ZVARTYPEID_VOID,          SETTER,       LINKMAXHP,            1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMaxMP",               ZVARTYPEID_FLOAT,         GETTER,       LINKMAXMP,            1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMaxMP",               ZVARTYPEID_VOID,          SETTER,       LINKMAXMP,            1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAction",              ZVARTYPEID_FLOAT,         GETTER,       LINKACTION,           1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAction",              ZVARTYPEID_VOID,          SETTER,       LINKACTION,           1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTileMod",             ZVARTYPEID_FLOAT,         GETTER,       LINKTILEMOD,          1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileMod",             ZVARTYPEID_VOID,          SETTER,       LINKTILEMOD,          1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHeldItem",            ZVARTYPEID_FLOAT,         GETTER,       LINKHELD,             1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHeldItem",            ZVARTYPEID_VOID,          SETTER,       LINKHELD,             1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Warp",                   ZVARTYPEID_VOID,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Warp",                   ZVARTYPEID_VOID,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "WarpEx",                 ZVARTYPEID_VOID,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Explode",                ZVARTYPEID_VOID,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "PitWarp",                ZVARTYPEID_VOID,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputStart",          ZVARTYPEID_BOOL,          GETTER,       INPUTSTART,           1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputStart",          ZVARTYPEID_VOID,          SETTER,       INPUTSTART,           1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputMap",            ZVARTYPEID_BOOL,          GETTER,       INPUTMAP,             1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputMap",            ZVARTYPEID_VOID,          SETTER,       INPUTMAP,             1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputUp",             ZVARTYPEID_BOOL,          GETTER,       INPUTUP,              1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputUp",             ZVARTYPEID_VOID,          SETTER,       INPUTUP,              1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputDown",           ZVARTYPEID_BOOL,          GETTER,       INPUTDOWN,            1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputDown",           ZVARTYPEID_VOID,          SETTER,       INPUTDOWN,            1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputLeft",           ZVARTYPEID_BOOL,          GETTER,       INPUTLEFT,            1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputLeft",           ZVARTYPEID_VOID,          SETTER,       INPUTLEFT,            1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputRight",          ZVARTYPEID_BOOL,          GETTER,       INPUTRIGHT,           1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputRight",          ZVARTYPEID_VOID,          SETTER,       INPUTRIGHT,           1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputA",              ZVARTYPEID_BOOL,          GETTER,       INPUTA,               1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputA",              ZVARTYPEID_VOID,          SETTER,       INPUTA,               1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputB",              ZVARTYPEID_BOOL,          GETTER,       INPUTB,               1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputB",              ZVARTYPEID_VOID,          SETTER,       INPUTB,               1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputL",              ZVARTYPEID_BOOL,          GETTER,       INPUTL,               1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputL",              ZVARTYPEID_VOID,          SETTER,       INPUTL,               1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputR",              ZVARTYPEID_BOOL,          GETTER,       INPUTR,               1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputR",              ZVARTYPEID_VOID,          SETTER,       INPUTR,               1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputEx1",            ZVARTYPEID_BOOL,          GETTER,       INPUTEX1,             1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputEx1",            ZVARTYPEID_VOID,          SETTER,       INPUTEX1,             1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputEx2",            ZVARTYPEID_BOOL,          GETTER,       INPUTEX2,             1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputEx2",            ZVARTYPEID_VOID,          SETTER,       INPUTEX2,             1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputEx3",            ZVARTYPEID_BOOL,          GETTER,       INPUTEX3,             1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputEx3",            ZVARTYPEID_VOID,          SETTER,       INPUTEX3,             1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputEx4",            ZVARTYPEID_BOOL,          GETTER,       INPUTEX4,             1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputEx4",            ZVARTYPEID_VOID,          SETTER,       INPUTEX4,             1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressStart",          ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSSTART,      1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressStart",          ZVARTYPEID_VOID,          SETTER,       INPUTPRESSSTART,      1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressUp",             ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSUP,         1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressUp",             ZVARTYPEID_VOID,          SETTER,       INPUTPRESSUP,         1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressDown",           ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSDOWN,       1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressDown",           ZVARTYPEID_VOID,          SETTER,       INPUTPRESSDOWN,       1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressLeft",           ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSLEFT,       1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressLeft",           ZVARTYPEID_VOID,          SETTER,       INPUTPRESSLEFT,       1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressRight",          ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSRIGHT,      1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressRight",          ZVARTYPEID_VOID,          SETTER,       INPUTPRESSRIGHT,      1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressA",              ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSA,          1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressA",              ZVARTYPEID_VOID,          SETTER,       INPUTPRESSA,          1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressB",              ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSB,          1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressB",              ZVARTYPEID_VOID,          SETTER,       INPUTPRESSB,          1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressL",              ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSL,          1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressL",              ZVARTYPEID_VOID,          SETTER,       INPUTPRESSL,          1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressR",              ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSR,          1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressR",              ZVARTYPEID_VOID,          SETTER,       INPUTPRESSR,          1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressEx1",            ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSEX1,        1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressEx1",            ZVARTYPEID_VOID,          SETTER,       INPUTPRESSEX1,        1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressEx2",            ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSEX2,        1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressEx2",            ZVARTYPEID_VOID,          SETTER,       INPUTPRESSEX2,        1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressEx3",            ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSEX3,        1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressEx3",            ZVARTYPEID_VOID,          SETTER,       INPUTPRESSEX3,        1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressEx4",            ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSEX4,        1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressEx4",            ZVARTYPEID_VOID,          SETTER,       INPUTPRESSEX4,        1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputMouseX",         ZVARTYPEID_FLOAT,         GETTER,       INPUTMOUSEX,          1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputMouseX",         ZVARTYPEID_VOID,          SETTER,       INPUTMOUSEX,          1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputMouseY",         ZVARTYPEID_FLOAT,         GETTER,       INPUTMOUSEY,          1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputMouseY",         ZVARTYPEID_VOID,          SETTER,       INPUTMOUSEY,          1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputMouseZ",         ZVARTYPEID_FLOAT,         GETTER,       INPUTMOUSEZ,          1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputMouseZ",         ZVARTYPEID_VOID,          SETTER,       INPUTMOUSEZ,          1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputMouseB",         ZVARTYPEID_FLOAT,         GETTER,       INPUTMOUSEB,          1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputMouseB",         ZVARTYPEID_VOID,          SETTER,       INPUTMOUSEB,          1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getItem[]",              ZVARTYPEID_BOOL,          GETTER,       LINKITEMD,            256,         0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItem[]",              ZVARTYPEID_VOID,          SETTER,       LINKITEMD,            256,         0,                                    3,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSteps[]",             ZVARTYPEID_FLOAT,         GETTER,       HEROSTEPS,            8,         0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSteps[]",             ZVARTYPEID_VOID,          SETTER,       HEROSTEPS,            8,         0,                                    3,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getHitWidth",            ZVARTYPEID_FLOAT,         GETTER,       LINKHXSZ,             1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setHitWidth",            ZVARTYPEID_VOID,          SETTER,       LINKHXSZ,             1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStep",                ZVARTYPEID_FLOAT,         GETTER,       HEROSTEPRATE,         1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStep",                ZVARTYPEID_VOID,          SETTER,       HEROSTEPRATE,         1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getHitHeight",           ZVARTYPEID_FLOAT,         GETTER,       LINKHYSZ,             1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setHitHeight",           ZVARTYPEID_VOID,          SETTER,       LINKHYSZ,             1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getHitZHeight",          ZVARTYPEID_FLOAT,         GETTER,       LINKHZSZ,             1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setHitZHeight",          ZVARTYPEID_VOID,          SETTER,       LINKHZSZ,             1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getTileWidth",           ZVARTYPEID_FLOAT,         GETTER,       LINKTXSZ,             1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setTileWidth",           ZVARTYPEID_VOID,          SETTER,       LINKTXSZ,             1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getTileHeight",          ZVARTYPEID_FLOAT,         GETTER,       LINKTYSZ,             1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setTileHeight",          ZVARTYPEID_VOID,          SETTER,       LINKTYSZ,             1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getDrawXOffset",         ZVARTYPEID_FLOAT,         GETTER,       LINKXOFS,             1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setDrawXOffset",         ZVARTYPEID_VOID,          SETTER,       LINKXOFS,             1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getDrawYOffset",         ZVARTYPEID_FLOAT,         GETTER,       LINKYOFS,             1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setDrawYOffset",         ZVARTYPEID_VOID,          SETTER,       LINKYOFS,             1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getTotalDYOffset",       ZVARTYPEID_FLOAT,         GETTER,       HEROTOTALDYOFFS,      1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setTotalDYOffset",       ZVARTYPEID_VOID,          SETTER,       HEROTOTALDYOFFS,      1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getDrawZOffset",         ZVARTYPEID_FLOAT,         GETTER,       LINKZOFS,             1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setDrawZOffset",         ZVARTYPEID_VOID,          SETTER,       LINKZOFS,             1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getHitXOffset",          ZVARTYPEID_FLOAT,         GETTER,       LINKHXOFS,            1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setHitXOffset",          ZVARTYPEID_VOID,          SETTER,       LINKHXOFS,            1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getHitYOffset",          ZVARTYPEID_FLOAT,         GETTER,       LINKHYOFS,            1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setHitYOffset",          ZVARTYPEID_VOID,          SETTER,       LINKHYOFS,            1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrunk",               ZVARTYPEID_FLOAT,         GETTER,       LINKDRUNK,            1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrunk",               ZVARTYPEID_VOID,          SETTER,       LINKDRUNK,            1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getEquipment",           ZVARTYPEID_FLOAT,         GETTER,       LINKEQUIP,            1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setEquipment",           ZVARTYPEID_VOID,          SETTER,       LINKEQUIP,            1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputAxisUp",         ZVARTYPEID_BOOL,          GETTER,       INPUTAXISUP,          1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputAxisUp",         ZVARTYPEID_VOID,          SETTER,       INPUTAXISUP,          1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputAxisDown",       ZVARTYPEID_BOOL,          GETTER,       INPUTAXISDOWN,        1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputAxisDown",       ZVARTYPEID_VOID,          SETTER,       INPUTAXISDOWN,        1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputAxisLeft",       ZVARTYPEID_BOOL,          GETTER,       INPUTAXISLEFT,        1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputAxisLeft",       ZVARTYPEID_VOID,          SETTER,       INPUTAXISLEFT,        1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getInputAxisRight",      ZVARTYPEID_BOOL,          GETTER,       INPUTAXISRIGHT,       1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setInputAxisRight",      ZVARTYPEID_VOID,          SETTER,       INPUTAXISRIGHT,       1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressAxisUp",         ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSAXISUP,     1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressAxisUp",         ZVARTYPEID_VOID,          SETTER,       INPUTPRESSAXISUP,     1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressAxisDown",       ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSAXISDOWN,   1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressAxisDown",       ZVARTYPEID_VOID,          SETTER,       INPUTPRESSAXISDOWN,   1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressAxisLeft",       ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSAXISLEFT,   1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressAxisLeft",       ZVARTYPEID_VOID,          SETTER,       INPUTPRESSAXISLEFT,   1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressAxisRight",      ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSAXISRIGHT,  1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressAxisRight",      ZVARTYPEID_VOID,          SETTER,       INPUTPRESSAXISRIGHT,  1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHealthBeep",          ZVARTYPEID_FLOAT,         GETTER,       HEROHEALTHBEEP,            1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHealthBeep",          ZVARTYPEID_VOID,          SETTER,       HEROHEALTHBEEP,            1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getInvisible",           ZVARTYPEID_BOOL,          GETTER,       LINKINVIS,            1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInvisible",           ZVARTYPEID_VOID,          SETTER,       LINKINVIS,            1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getNoStepForward",       ZVARTYPEID_BOOL,          GETTER,       HERONOSTEPFORWARD,    1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNoStepForward",       ZVARTYPEID_VOID,          SETTER,       HERONOSTEPFORWARD,    1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAnimation",           ZVARTYPEID_FLOAT,         GETTER,       LINKENGINEANIMATE,    1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAnimation",           ZVARTYPEID_VOID,          SETTER,       LINKENGINEANIMATE,    1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCollDetection",       ZVARTYPEID_BOOL,          GETTER,       LINKINVINC,           1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCollDetection",       ZVARTYPEID_VOID,          SETTER,       LINKINVINC,           1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMisc[]",              ZVARTYPEID_UNTYPED,       GETTER,       LINKMISCD,            32,          0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMisc[]",              ZVARTYPEID_VOID,          SETTER,       LINKMISCD,            32,          0,                                    3,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDefense[]",           ZVARTYPEID_FLOAT,         GETTER,       LINKDEFENCE,          256,         0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDefense[]",           ZVARTYPEID_VOID,          SETTER,       LINKDEFENCE,          256,         0,                                    3,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitBy[]",             ZVARTYPEID_UNTYPED,       GETTER,       LINKHITBY,            10,          0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitBy[]",             ZVARTYPEID_VOID,          SETTER,       LINKHITBY,            10,          0,                                    3,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLadderX",             ZVARTYPEID_FLOAT,         GETTER,       LINKLADDERX,          1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLadderY",             ZVARTYPEID_FLOAT,         GETTER,       LINKLADDERY,          1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTile",                ZVARTYPEID_FLOAT,         GETTER,       LINKTILE,             1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTile",                ZVARTYPEID_VOID,          SETTER,       LINKTILE,             1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlip",                ZVARTYPEID_FLOAT,         GETTER,       LINKFLIP,             1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlip",                ZVARTYPEID_VOID,          SETTER,       LINKFLIP,             1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getPressMap",            ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSMAP,        1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setPressMap",            ZVARTYPEID_VOID,          SETTER,       INPUTPRESSMAP,        1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SelectAWeapon",          ZVARTYPEID_VOID,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SelectBWeapon",          ZVARTYPEID_VOID,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SelectXWeapon",          ZVARTYPEID_VOID,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SelectYWeapon",          ZVARTYPEID_VOID,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInvFrames",           ZVARTYPEID_VOID,          SETTER,       LINKINVFRAME,         1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getInvFrames",           ZVARTYPEID_FLOAT,         GETTER,       LINKINVFRAME,         1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInvFlicker",          ZVARTYPEID_VOID,          SETTER,       LINKCANFLICKER,       1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getInvFlicker",          ZVARTYPEID_BOOL,          GETTER,       LINKCANFLICKER,       1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHurtSound",           ZVARTYPEID_VOID,          SETTER,       LINKHURTSFX,          1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHurtSound",           ZVARTYPEID_FLOAT,         GETTER,       LINKHURTSFX,          1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "getUsingItem",           ZVARTYPEID_FLOAT,         GETTER,       LINKUSINGITEM,        1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "setUsingItem",           ZVARTYPEID_VOID,          SETTER,       LINKUSINGITEM,        1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "getAttack",              ZVARTYPEID_FLOAT,         GETTER,       LINKUSINGITEMA,       1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "setAttack",              ZVARTYPEID_VOID,          SETTER,       LINKUSINGITEMA,       1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetItemA",               ZVARTYPEID_VOID,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetItemB",               ZVARTYPEID_VOID,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "SetItemSlot",            ZVARTYPEID_VOID,          FUNCTION,     0,                    1,           0,                                    4,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItemB",               ZVARTYPEID_VOID,          SETTER,       LINKITEMB,            1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItemA",               ZVARTYPEID_VOID,          SETTER,       LINKITEMA,            1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getItemB",               ZVARTYPEID_FLOAT,         GETTER,       LINKITEMB,            1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getItemA",               ZVARTYPEID_FLOAT,         GETTER,       LINKITEMA,            1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getItemX",               ZVARTYPEID_FLOAT,         GETTER,       LINKITEMX,            1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItemX",               ZVARTYPEID_VOID,          SETTER,       LINKITEMX,            1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getItemY",               ZVARTYPEID_FLOAT,         GETTER,       LINKITEMY,            1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItemY",               ZVARTYPEID_VOID,          SETTER,       LINKITEMY,            1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getEaten",               ZVARTYPEID_FLOAT,         GETTER,       LINKEATEN,            1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setEaten",               ZVARTYPEID_VOID,          SETTER,       LINKEATEN,            1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getGrabbed",             ZVARTYPEID_BOOL,          GETTER,       LINKGRABBED,          1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setGrabbed",             ZVARTYPEID_VOID,          SETTER,       LINKGRABBED,          1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStun",                ZVARTYPEID_FLOAT,         GETTER,       LINKSTUN,             1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStun",                ZVARTYPEID_VOID,          SETTER,       LINKSTUN,             1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getBunnyClk",            ZVARTYPEID_FLOAT,         GETTER,       HEROBUNNY,            1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setBunnyClk",            ZVARTYPEID_VOID,          SETTER,       HEROBUNNY,            1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPushing",             ZVARTYPEID_FLOAT,         GETTER,       LINKPUSH,             1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPushing",             ZVARTYPEID_VOID,          SETTER,       LINKPUSH,             1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScriptCSet",          ZVARTYPEID_FLOAT,         GETTER,       HEROSCRIPTCSET,       1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScriptCSet",          ZVARTYPEID_VOID,          SETTER,       HEROSCRIPTCSET,       1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScriptTile",          ZVARTYPEID_FLOAT,         GETTER,       LINKSCRIPTTILE,       1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScriptTile",          ZVARTYPEID_VOID,          SETTER,       LINKSCRIPTTILE,       1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScriptFlip",          ZVARTYPEID_FLOAT,         GETTER,       LINKSCRIPFLIP,        1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScriptFlip",          ZVARTYPEID_VOID,          SETTER,       LINKSCRIPFLIP,        1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDiagonal",            ZVARTYPEID_BOOL,          GETTER,       LINKDIAG,             1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDiagonal",            ZVARTYPEID_VOID,          SETTER,       LINKDIAG,             1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getBigHitbox",           ZVARTYPEID_BOOL,          GETTER,       LINKBIGHITBOX,        1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setBigHitbox",           ZVARTYPEID_VOID,          SETTER,       LINKBIGHITBOX,        1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getGravity",             ZVARTYPEID_BOOL,          GETTER,       LINKGRAVITY,          1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setGravity",             ZVARTYPEID_VOID,          SETTER,       LINKGRAVITY,          1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRotation",            ZVARTYPEID_FLOAT,         GETTER,       LINKROTATION,         1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRotation",            ZVARTYPEID_VOID,          SETTER,       LINKROTATION,         1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScale",               ZVARTYPEID_FLOAT,         GETTER,       LINKSCALE,            1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScale",               ZVARTYPEID_VOID,          SETTER,       LINKSCALE,            1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetOriginalTile",        ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetOriginalFlip",        ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getClimbing",            ZVARTYPEID_BOOL,          GETTER,       LINKCLIMBING,         1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setClimbing",            ZVARTYPEID_VOID,          SETTER,       LINKCLIMBING,         1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getJumpCount",           ZVARTYPEID_FLOAT,         GETTER,       HEROJUMPCOUNT,        1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setJumpCount",           ZVARTYPEID_VOID,          SETTER,       HEROJUMPCOUNT,        1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPitPullDir",          ZVARTYPEID_FLOAT,         GETTER,       HEROPULLDIR,          1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPitPullTimer",        ZVARTYPEID_FLOAT,         GETTER,       HEROPULLCLK,          1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPitPullTimer",        ZVARTYPEID_VOID,          SETTER,       HEROPULLCLK,          1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFalling",             ZVARTYPEID_FLOAT,         GETTER,       HEROFALLCLK,          1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFalling",             ZVARTYPEID_VOID,          SETTER,       HEROFALLCLK,          1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFallCombo",           ZVARTYPEID_FLOAT,         GETTER,       HEROFALLCMB,          1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFallCombo",           ZVARTYPEID_VOID,          SETTER,       HEROFALLCMB,          1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMoveFlags[]",         ZVARTYPEID_BOOL,          GETTER,       HEROMOVEFLAGS,        2,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMoveFlags[]",         ZVARTYPEID_VOID,          SETTER,       HEROMOVEFLAGS,        2,           0,                                    3,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getClockActive",         ZVARTYPEID_BOOL,          GETTER,       CLOCKACTIVE,          1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setClockActive",         ZVARTYPEID_VOID,          SETTER,       CLOCKACTIVE,          1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getClockTimer",          ZVARTYPEID_FLOAT,         GETTER,       CLOCKCLK,             1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setClockTimer",          ZVARTYPEID_VOID,          SETTER,       CLOCKCLK,             1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCSet",                ZVARTYPEID_FLOAT,         GETTER,       LINKCSET,             1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCSet",                ZVARTYPEID_VOID,          SETTER,       LINKCSET,             1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getIsWarping",           ZVARTYPEID_BOOL,          GETTER,       HEROISWARPING,        1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getRespawnX",            ZVARTYPEID_FLOAT,         GETTER,       HERORESPAWNX,         1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRespawnX",            ZVARTYPEID_VOID,          SETTER,       HERORESPAWNX,         1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRespawnY",            ZVARTYPEID_FLOAT,         GETTER,       HERORESPAWNY,         1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRespawnY",            ZVARTYPEID_VOID,          SETTER,       HERORESPAWNY,         1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRespawnDMap",         ZVARTYPEID_FLOAT,         GETTER,       HERORESPAWNDMAP,      1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRespawnDMap",         ZVARTYPEID_VOID,          SETTER,       HERORESPAWNDMAP,      1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRespawnScreen",       ZVARTYPEID_FLOAT,         GETTER,       HERORESPAWNSCR,       1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRespawnScreen",       ZVARTYPEID_VOID,          SETTER,       HERORESPAWNSCR,       1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSwitchTimer",         ZVARTYPEID_FLOAT,         GETTER,       HEROSWITCHTIMER,      1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSwitchTimer",         ZVARTYPEID_VOID,          SETTER,       HEROSWITCHTIMER,      1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSwitchMaxTimer",      ZVARTYPEID_FLOAT,         GETTER,       HEROSWITCHMAXTIMER,   1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSwitchMaxTimer",      ZVARTYPEID_VOID,          SETTER,       HEROSWITCHMAXTIMER,   1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getImmortal",            ZVARTYPEID_FLOAT,         GETTER,       HEROIMMORTAL,         1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setImmortal",            ZVARTYPEID_VOID,          SETTER,       HEROIMMORTAL,         1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SwitchCombo",            ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Kill",                   ZVARTYPEID_VOID,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrowning",             ZVARTYPEID_FLOAT,         GETTER,       HERODROWNCLK,          1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrowning",             ZVARTYPEID_VOID,          SETTER,       HERODROWNCLK,          1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrownCombo",           ZVARTYPEID_FLOAT,         GETTER,       HERODROWNCMB,          1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrownCombo",           ZVARTYPEID_VOID,          SETTER,       HERODROWNCMB,          1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFakeZ",           ZVARTYPEID_FLOAT,         GETTER,       HEROFAKEZ,          1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFakeZ",           ZVARTYPEID_VOID,          SETTER,       HEROFAKEZ,          1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFakeJump",           ZVARTYPEID_FLOAT,         GETTER,       HEROFAKEJUMP,          1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFakeJump",           ZVARTYPEID_VOID,          SETTER,       HEROFAKEJUMP,          1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShadowXOffset",           ZVARTYPEID_FLOAT,         GETTER,       HEROSHADOWXOFS,          1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShadowXOffset",           ZVARTYPEID_VOID,          SETTER,       HEROSHADOWXOFS,          1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShadowYOffset",           ZVARTYPEID_FLOAT,         GETTER,       HEROSHADOWYOFS,          1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShadowYOffset",           ZVARTYPEID_VOID,          SETTER,       HEROSHADOWYOFS,          1,           0,                                    2,           { ZVARTYPEID_PLAYER, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getStanding",                ZVARTYPEID_BOOL,          GETTER,       HEROSTANDING,            1,           0,                                    1,           { ZVARTYPEID_PLAYER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
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

