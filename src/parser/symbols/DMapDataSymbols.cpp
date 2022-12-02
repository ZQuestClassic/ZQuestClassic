#include "SymbolDefs.h"

DMapDataSymbols DMapDataSymbols::singleton = DMapDataSymbols();

static AccessorTable DMapDataTable[] =
{
//	All of these return a function label error when used:
//	  name,                     rettype,                  setorget,     var,                        numindex,      funcFlags,                            numParams,   params
	{ "getTest",                ZTID_FLOAT,         GETTER,       DEBUGREFFFC,                1,             0,                                    1,           { ZTID_DMAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetName",                ZTID_VOID,          FUNCTION,     0,                          1,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetName",                ZTID_VOID,          FUNCTION,     0,                          1,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetTitle",               ZTID_VOID,          FUNCTION,     0,                          1,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetTitle",               ZTID_VOID,          FUNCTION,     0,                          1,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetIntro",               ZTID_VOID,          FUNCTION,     0,                          1,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetIntro",               ZTID_VOID,          FUNCTION,     0,                          1,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetMusic",               ZTID_VOID,          FUNCTION,     0,                          1,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetMusic",               ZTID_VOID,          FUNCTION,     0,                          1,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	//
	{ "getID",                  ZTID_FLOAT,         GETTER,       DMAPDATAID,                 1,             0,                                    1,           { ZTID_DMAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMap",                 ZTID_FLOAT,         GETTER,       DMAPDATAMAP,                1,             0,                                    1,           { ZTID_DMAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMap",                 ZTID_VOID,          SETTER,       DMAPDATAMAP,                1,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLevel",               ZTID_FLOAT,         GETTER,       DMAPDATALEVEL,              1,             0,                                    1,           { ZTID_DMAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLevel",               ZTID_VOID,          SETTER,       DMAPDATALEVEL,              1,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getOffset",              ZTID_FLOAT,         GETTER,       DMAPDATAOFFSET,             1,             0,                                    1,           { ZTID_DMAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setOffset",              ZTID_VOID,          SETTER,       DMAPDATAOFFSET,             1,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCompass",             ZTID_FLOAT,         GETTER,       DMAPDATACOMPASS,            1,             0,                                    1,           { ZTID_DMAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCompass",             ZTID_VOID,          SETTER,       DMAPDATACOMPASS,            1,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPalette",             ZTID_FLOAT,         GETTER,       DMAPDATAPALETTE,            1,             0,                                    1,           { ZTID_DMAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPalette",             ZTID_VOID,          SETTER,       DMAPDATAPALETTE,            1,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMIDI",                ZTID_FLOAT,         GETTER,       DMAPDATAMIDI,               1,             0,                                    1,           { ZTID_DMAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMIDI",                ZTID_VOID,          SETTER,       DMAPDATAMIDI,               1,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getContinue",            ZTID_FLOAT,         GETTER,       DMAPDATACONTINUE,           1,             0,                                    1,           { ZTID_DMAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setContinue",            ZTID_VOID,          SETTER,       DMAPDATACONTINUE,           1,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getType",                ZTID_FLOAT,         GETTER,       DMAPDATATYPE,               1,             0,                                    1,           { ZTID_DMAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setType",                ZTID_VOID,          SETTER,       DMAPDATATYPE,               1,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScript",              ZTID_FLOAT,         GETTER,       DMAPSCRIPT,                 1,             0,                                    1,           { ZTID_DMAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScript",              ZTID_VOID,          SETTER,       DMAPSCRIPT,                 1,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMusicTrack",          ZTID_FLOAT,         GETTER,       DMAPDATAMUISCTRACK,         1,             0,                                    1,           { ZTID_DMAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMusicTrack",          ZTID_VOID,          SETTER,       DMAPDATAMUISCTRACK,         1,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getActiveSubscreen",     ZTID_FLOAT,         GETTER,       DMAPDATASUBSCRA,            1,             0,                                    1,           { ZTID_DMAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setActiveSubscreen",     ZTID_VOID,          SETTER,       DMAPDATASUBSCRA,            1,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPassiveSubscreen",    ZTID_FLOAT,         GETTER,       DMAPDATASUBSCRP,            1,             0,                                    1,           { ZTID_DMAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPassiveSubscreen",    ZTID_VOID,          SETTER,       DMAPDATASUBSCRP,            1,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "getGravity[]",           ZTID_FLOAT,         GETTER,       DMAPDATASUBSCRP,            1,             0,                                    1,           { ZTID_DMAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "setGravity[]",           ZTID_VOID,          SETTER,       DMAPDATASUBSCRP,            1,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "getJumpThreshold",       ZTID_FLOAT,         GETTER,       DMAPDATASUBSCRP,            1,             0,                                    1,           { ZTID_DMAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "setJumpThreshold",       ZTID_VOID,          SETTER,       DMAPDATASUBSCRP,            1,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getGrid[]",              ZTID_FLOAT,         GETTER,       DMAPDATAGRID,               8,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setGrid[]",              ZTID_VOID,          SETTER,       DMAPDATAGRID,               8,             0,                                    3,           { ZTID_DMAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMiniMapTile[]",       ZTID_FLOAT,         GETTER,       DMAPDATAMINIMAPTILE,        2,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMiniMapTile[]",       ZTID_VOID,          SETTER,       DMAPDATAMINIMAPTILE,        2,             0,                                    3,           { ZTID_DMAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMiniMapCSet[]",       ZTID_FLOAT,         GETTER,       DMAPDATAMINIMAPCSET,        2,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMiniMapCSet[]",       ZTID_VOID,          SETTER,       DMAPDATAMINIMAPCSET,        2,             0,                                    3,           { ZTID_DMAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getInitD[]",             ZTID_FLOAT,         GETTER,       DMAPINITD,                  8,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInitD[]",             ZTID_VOID,          SETTER,       DMAPINITD,                  8,             0,                                    3,           { ZTID_DMAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMapTile[]",           ZTID_FLOAT,         GETTER,       DMAPDATALARGEMAPTILE,       2,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMapTile[]",           ZTID_VOID,          SETTER,       DMAPDATALARGEMAPTILE,       2,             0,                                    3,           { ZTID_DMAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMapCSet[]",           ZTID_FLOAT,         GETTER,       DMAPDATALARGEMAPCSET,       2,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMapCSet[]",           ZTID_VOID,          SETTER,       DMAPDATALARGEMAPCSET,       2,             0,                                    3,           { ZTID_DMAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDisabledItems[]",     ZTID_FLOAT,         GETTER,       DMAPDATADISABLEDITEMS,      256,           0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDisabledItems[]",     ZTID_VOID,          SETTER,       DMAPDATADISABLEDITEMS,      256,           0,                                    3,           { ZTID_DMAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlags",               ZTID_FLOAT,         GETTER,       DMAPDATAFLAGS,              1,             0,                                    1,           { ZTID_DMAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlags",               ZTID_VOID,          SETTER,       DMAPDATAFLAGS,              1,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlagset[]",     ZTID_FLOAT,         GETTER,       DMAPDATAFLAGARR,      32,           0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlagset[]",     ZTID_VOID,          SETTER,       DMAPDATAFLAGARR,      32,           0,                                    3,           { ZTID_DMAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSideview",            ZTID_BOOL,          GETTER,       DMAPDATASIDEVIEW,           1,             0,                                    1,           { ZTID_DMAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSideview",            ZTID_VOID,          SETTER,       DMAPDATASIDEVIEW,           1,             0,                                    2,           { ZTID_DMAPDATA, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getASubScript",          ZTID_FLOAT,         GETTER,       DMAPDATAASUBSCRIPT,         1,             0,                                    1,           { ZTID_DMAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setASubScript",          ZTID_VOID,          SETTER,       DMAPDATAASUBSCRIPT,         1,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPSubScript",          ZTID_FLOAT,         GETTER,       DMAPDATAPSUBSCRIPT,         1,             0,                                    1,           { ZTID_DMAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setPSubScript",          ZTID_VOID,          SETTER,       DMAPDATAPSUBSCRIPT,         1,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSubInitD[]",          ZTID_UNTYPED,       GETTER,       DMAPDATASUBINITD,           8,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSubInitD[]",          ZTID_VOID,          SETTER,       DMAPDATASUBINITD,           8,             0,                                    3,           { ZTID_DMAPDATA, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMapScript",           ZTID_FLOAT,         GETTER,       DMAPDATAMAPSCRIPT,          1,             0,                                    1,           { ZTID_DMAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setMapScript",           ZTID_VOID,          SETTER,       DMAPDATAMAPSCRIPT,          1,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMapInitD[]",          ZTID_UNTYPED,       GETTER,       DMAPDATAMAPINITD,           8,             0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMapInitD[]",          ZTID_VOID,          SETTER,       DMAPDATAMAPINITD,           8,             0,                                    3,           { ZTID_DMAPDATA, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCharted[]",           ZTID_FLOAT,         GETTER,       DMAPDATACHARTED,            128,           0,                                    2,           { ZTID_DMAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCharted[]",           ZTID_VOID,          SETTER,       DMAPDATACHARTED,            128,           0,                                    3,           { ZTID_DMAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "",                       -1,                       -1,           -1,                         -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

DMapDataSymbols::DMapDataSymbols()
{
	table = DMapDataTable;
	refVar = REFDMAPDATA;
}

void DMapDataSymbols::generateCode()
{
}

