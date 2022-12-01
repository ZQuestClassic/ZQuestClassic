#include "SymbolDefs.h"

FFCSymbols FFCSymbols::singleton = FFCSymbols();

static AccessorTable FFCTable[] =
{
//	  name,                     rettype,                  setorget,     var,                  numindex,    funcFlags,                            numParams,   params
	{ "getData",                ZVARTYPEID_FLOAT,         GETTER,       DATA,                 1,           0,                                    1,           { ZVARTYPEID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setData",                ZVARTYPEID_VOID,          SETTER,       DATA,                 1,           0,                                    2,           { ZVARTYPEID_FFC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScript",              ZVARTYPEID_FLOAT,         GETTER,       FFSCRIPT,             1,           0,                                    1,           { ZVARTYPEID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScript",              ZVARTYPEID_VOID,          SETTER,       FFSCRIPT,             1,           0,                                    2,           { ZVARTYPEID_FFC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCSet",                ZVARTYPEID_FLOAT,         GETTER,       FCSET,                1,           0,                                    1,           { ZVARTYPEID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCSet",                ZVARTYPEID_VOID,          SETTER,       FCSET,                1,           0,                                    2,           { ZVARTYPEID_FFC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDelay",               ZVARTYPEID_FLOAT,         GETTER,       DELAY,                1,           0,                                    1,           { ZVARTYPEID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDelay",               ZVARTYPEID_VOID,          SETTER,       DELAY,                1,           0,                                    2,           { ZVARTYPEID_FFC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getX",                   ZVARTYPEID_FLOAT,         GETTER,       FX,                   1,           0,                                    1,           { ZVARTYPEID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setX",                   ZVARTYPEID_VOID,          SETTER,       FX,                   1,           0,                                    2,           { ZVARTYPEID_FFC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getY",                   ZVARTYPEID_FLOAT,         GETTER,       FY,                   1,           0,                                    1,           { ZVARTYPEID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setY",                   ZVARTYPEID_VOID,          SETTER,       FY,                   1,           0,                                    2,           { ZVARTYPEID_FFC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getVx",                  ZVARTYPEID_FLOAT,         GETTER,       XD,                   1,           0,                                    1,           { ZVARTYPEID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setVx",                  ZVARTYPEID_VOID,          SETTER,       XD,                   1,           0,                                    2,           { ZVARTYPEID_FFC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getVy",                  ZVARTYPEID_FLOAT,         GETTER,       YD,                   1,           0,                                    1,           { ZVARTYPEID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setVy",                  ZVARTYPEID_VOID,          SETTER,       YD,                   1,           0,                                    2,           { ZVARTYPEID_FFC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAx",                  ZVARTYPEID_FLOAT,         GETTER,       XD2,                  1,           0,                                    1,           { ZVARTYPEID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAx",                  ZVARTYPEID_VOID,          SETTER,       XD2,                  1,           0,                                    2,           { ZVARTYPEID_FFC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAy",                  ZVARTYPEID_FLOAT,         GETTER,       YD2,                  1,           0,                                    1,           { ZVARTYPEID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAy",                  ZVARTYPEID_VOID,          SETTER,       YD2,                  1,           0,                                    2,           { ZVARTYPEID_FFC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "WasTriggered",           ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,           0,                                    1,           { ZVARTYPEID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlags[]",             ZVARTYPEID_BOOL,          GETTER,       FFFLAGSD,             14,           0,                                    2,           { ZVARTYPEID_FFC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlags[]",             ZVARTYPEID_VOID,          SETTER,       FFFLAGSD,             14,           0,                                    3,           { ZVARTYPEID_FFC, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTileWidth",           ZVARTYPEID_FLOAT,         GETTER,       FFTWIDTH,             1,           0,                                    1,           { ZVARTYPEID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWidth",           ZVARTYPEID_VOID,          SETTER,       FFTWIDTH,             1,           0,                                    2,           { ZVARTYPEID_FFC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTileHeight",          ZVARTYPEID_FLOAT,         GETTER,       FFTHEIGHT,            1,           0,                                    1,           { ZVARTYPEID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileHeight",          ZVARTYPEID_VOID,          SETTER,       FFTHEIGHT,            1,           0,                                    2,           { ZVARTYPEID_FFC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getEffectWidth",         ZVARTYPEID_FLOAT,         GETTER,       FFCWIDTH,             1,           0,                                    1,           { ZVARTYPEID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setEffectWidth",         ZVARTYPEID_VOID,          SETTER,       FFCWIDTH,             1,           0,                                    2,           { ZVARTYPEID_FFC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getEffectHeight",        ZVARTYPEID_FLOAT,         GETTER,       FFCHEIGHT,            1,           0,                                    1,           { ZVARTYPEID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setEffectHeight",        ZVARTYPEID_VOID,          SETTER,       FFCHEIGHT,            1,           0,                                    2,           { ZVARTYPEID_FFC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLink",                ZVARTYPEID_FLOAT,         GETTER,       FFLINK,               1,           0,                                    1,           { ZVARTYPEID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLink",                ZVARTYPEID_VOID,          SETTER,       FFLINK,               1,           0,                                    2,           { ZVARTYPEID_FFC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMisc[]",              ZVARTYPEID_UNTYPED,       GETTER,       FFMISCD,              16,          0,                                    2,           { ZVARTYPEID_FFC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMisc[]",              ZVARTYPEID_VOID,          SETTER,       FFMISCD,              16,          0,                                    3,           { ZVARTYPEID_FFC, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getInitD[]",             ZVARTYPEID_UNTYPED,       GETTER,       FFINITDD,             8,           0,                                    2,           { ZVARTYPEID_FFC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInitD[]",             ZVARTYPEID_VOID,          SETTER,       FFINITDD,             8,           0,                                    3,           { ZVARTYPEID_FFC, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "getD[]",                 ZVARTYPEID_FLOAT,         GETTER,       FFDD,                 8,           0,                                    2,           { ZVARTYPEID_FFC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "setD[]",                 ZVARTYPEID_VOID,          SETTER,       FFDD,                 8,           0,                                    3,           { ZVARTYPEID_FFC, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getID",                  ZVARTYPEID_FLOAT,         GETTER,       FFCID,                1,           0,                                    1,           { ZVARTYPEID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setID",                  ZVARTYPEID_VOID,          SETTER,       FFCID,                1,           0,                                    2,           { ZVARTYPEID_FFC, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "",                       -1,                       -1,           -1,                   -1,          0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

FFCSymbols::FFCSymbols()
{
    table = FFCTable;
    refVar = REFFFC;
}

void FFCSymbols::generateCode()
{}

