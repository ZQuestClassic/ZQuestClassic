#include "SymbolDefs.h"

FFCSymbols FFCSymbols::singleton = FFCSymbols();

static AccessorTable FFCTable[] =
{
//	  name,                     rettype,                  setorget,     var,                  numindex,    funcFlags,                            numParams,   params
	{ "getData",                ZTID_FLOAT,         GETTER,       DATA,                 1,           0,                                    1,           { ZTID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setData",                ZTID_VOID,          SETTER,       DATA,                 1,           0,                                    2,           { ZTID_FFC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScript",              ZTID_FLOAT,         GETTER,       FFSCRIPT,             1,           0,                                    1,           { ZTID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScript",              ZTID_VOID,          SETTER,       FFSCRIPT,             1,           0,                                    2,           { ZTID_FFC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCSet",                ZTID_FLOAT,         GETTER,       FCSET,                1,           0,                                    1,           { ZTID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCSet",                ZTID_VOID,          SETTER,       FCSET,                1,           0,                                    2,           { ZTID_FFC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDelay",               ZTID_FLOAT,         GETTER,       DELAY,                1,           0,                                    1,           { ZTID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDelay",               ZTID_VOID,          SETTER,       DELAY,                1,           0,                                    2,           { ZTID_FFC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getX",                   ZTID_FLOAT,         GETTER,       FX,                   1,           0,                                    1,           { ZTID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setX",                   ZTID_VOID,          SETTER,       FX,                   1,           0,                                    2,           { ZTID_FFC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getY",                   ZTID_FLOAT,         GETTER,       FY,                   1,           0,                                    1,           { ZTID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setY",                   ZTID_VOID,          SETTER,       FY,                   1,           0,                                    2,           { ZTID_FFC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getVx",                  ZTID_FLOAT,         GETTER,       XD,                   1,           0,                                    1,           { ZTID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setVx",                  ZTID_VOID,          SETTER,       XD,                   1,           0,                                    2,           { ZTID_FFC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getVy",                  ZTID_FLOAT,         GETTER,       YD,                   1,           0,                                    1,           { ZTID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setVy",                  ZTID_VOID,          SETTER,       YD,                   1,           0,                                    2,           { ZTID_FFC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAx",                  ZTID_FLOAT,         GETTER,       XD2,                  1,           0,                                    1,           { ZTID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAx",                  ZTID_VOID,          SETTER,       XD2,                  1,           0,                                    2,           { ZTID_FFC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAy",                  ZTID_FLOAT,         GETTER,       YD2,                  1,           0,                                    1,           { ZTID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAy",                  ZTID_VOID,          SETTER,       YD2,                  1,           0,                                    2,           { ZTID_FFC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "WasTriggered",           ZTID_BOOL,          FUNCTION,     0,                    1,           0,                                    1,           { ZTID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlags[]",             ZTID_BOOL,          GETTER,       FFFLAGSD,             14,           0,                                    2,           { ZTID_FFC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlags[]",             ZTID_VOID,          SETTER,       FFFLAGSD,             14,           0,                                    3,           { ZTID_FFC, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTileWidth",           ZTID_FLOAT,         GETTER,       FFTWIDTH,             1,           0,                                    1,           { ZTID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWidth",           ZTID_VOID,          SETTER,       FFTWIDTH,             1,           0,                                    2,           { ZTID_FFC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTileHeight",          ZTID_FLOAT,         GETTER,       FFTHEIGHT,            1,           0,                                    1,           { ZTID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileHeight",          ZTID_VOID,          SETTER,       FFTHEIGHT,            1,           0,                                    2,           { ZTID_FFC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getEffectWidth",         ZTID_FLOAT,         GETTER,       FFCWIDTH,             1,           0,                                    1,           { ZTID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setEffectWidth",         ZTID_VOID,          SETTER,       FFCWIDTH,             1,           0,                                    2,           { ZTID_FFC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getEffectHeight",        ZTID_FLOAT,         GETTER,       FFCHEIGHT,            1,           0,                                    1,           { ZTID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setEffectHeight",        ZTID_VOID,          SETTER,       FFCHEIGHT,            1,           0,                                    2,           { ZTID_FFC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLink",                ZTID_FLOAT,         GETTER,       FFLINK,               1,           0,                                    1,           { ZTID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLink",                ZTID_VOID,          SETTER,       FFLINK,               1,           0,                                    2,           { ZTID_FFC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMisc[]",              ZTID_UNTYPED,       GETTER,       FFMISCD,              16,          0,                                    2,           { ZTID_FFC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMisc[]",              ZTID_VOID,          SETTER,       FFMISCD,              16,          0,                                    3,           { ZTID_FFC, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getInitD[]",             ZTID_UNTYPED,       GETTER,       FFINITDD,             8,           0,                                    2,           { ZTID_FFC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInitD[]",             ZTID_VOID,          SETTER,       FFINITDD,             8,           0,                                    3,           { ZTID_FFC, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "getD[]",                 ZTID_FLOAT,         GETTER,       FFDD,                 8,           0,                                    2,           { ZTID_FFC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "setD[]",                 ZTID_VOID,          SETTER,       FFDD,                 8,           0,                                    3,           { ZTID_FFC, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getID",                  ZTID_FLOAT,         GETTER,       FFCID,                1,           0,                                    1,           { ZTID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setID",                  ZTID_VOID,          SETTER,       FFCID,                1,           0,                                    2,           { ZTID_FFC, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "",                       -1,                       -1,           -1,                   -1,          0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

FFCSymbols::FFCSymbols()
{
    table = FFCTable;
    refVar = REFFFC;
}

void FFCSymbols::generateCode()
{}

