#include "SymbolDefs.h"

MapDataSymbols MapDataSymbols::singleton = MapDataSymbols();

static AccessorTable MapDataTable[] =
{
//	  name,                                     rettype,          setorget,     var,                 numindex,             funcFlags,                            numParams,   params
	{ "isSolid",                        ZTID_BOOL,          FUNCTION,     0,                          1,             FUNCFLAG_INLINE,                      3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "isSolidLayer",                   ZTID_BOOL,          FUNCTION,     0,                          1,             0,                                    4,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getValid",                       ZTID_FLOAT,         GETTER,       MAPDATAVALID,               1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setValid",                       ZTID_VOID,          SETTER,       MAPDATAVALID,               1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getGuy",                         ZTID_FLOAT,         GETTER,       MAPDATAGUY,                 1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setGuy",                         ZTID_VOID,          SETTER,       MAPDATAGUY,                 1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getString",                      ZTID_FLOAT,         GETTER,       MAPDATASTRING,              1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setString",                      ZTID_VOID,          SETTER,       MAPDATASTRING,              1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getRoomType",                    ZTID_FLOAT,         GETTER,       MAPDATAROOM,                1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRoomType",                    ZTID_VOID,          SETTER,       MAPDATAROOM,                1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getItem",                        ZTID_FLOAT,         GETTER,       MAPDATAITEM,                1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItem",                        ZTID_VOID,          SETTER,       MAPDATAITEM,                1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getHasItem",                     ZTID_FLOAT,         GETTER,       MAPDATAHASITEM,             1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setHasItem",                     ZTID_VOID,          SETTER,       MAPDATAHASITEM,             1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getTileWarpType[]",              ZTID_FLOAT,         GETTER,       MAPDATATILEWARPTYPE,        4,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWarpType[]",              ZTID_VOID,          SETTER,       MAPDATATILEWARPTYPE,        4,             0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getScript",                      ZTID_FLOAT,         GETTER,       MAPDATASCRIPT,              1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScript",                      ZTID_VOID,          SETTER,       MAPDATASCRIPT,              1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getInitD[]",                     ZTID_UNTYPED,       GETTER,       MAPDATAINITDARRAY,          8,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInitD[]",                     ZTID_VOID,          SETTER,       MAPDATAINITDARRAY,          8,             0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getTileWarpOverlay[]",           ZTID_BOOL,          GETTER,       MAPDATATILEWARPOVFLAGS,     4,             0,                                    1,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWarpOverlay[]",           ZTID_VOID,          SETTER,       MAPDATATILEWARPOVFLAGS,     4,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getDoorComboSet",                ZTID_FLOAT,         GETTER,       MAPDATADOORCOMBOSET,        1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDoorComboSet",                ZTID_VOID,          SETTER,       MAPDATADOORCOMBOSET,        1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWarpReturnX[]",               ZTID_FLOAT,         GETTER,       MAPDATAWARPRETX,            4,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWarpReturnX[]",               ZTID_VOID,          SETTER,       MAPDATAWARPRETX,            4,             0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWarpReturnY[]",               ZTID_FLOAT,         GETTER,       MAPDATAWARPRETY,            4,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWarpReturnY[]",               ZTID_VOID,          SETTER,       MAPDATAWARPRETY,            4,             0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getWarpReturnC",                 ZTID_FLOAT,         GETTER,       MAPDATAWARPRETURNC,         1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setWarpReturnC",                 ZTID_VOID,          SETTER,       MAPDATAWARPRETURNC,         1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getStairsX",                     ZTID_FLOAT,         GETTER,       MAPDATASTAIRX,              1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStairsX",                     ZTID_VOID,          SETTER,       MAPDATASTAIRX,              1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getStairsY",                     ZTID_FLOAT,         GETTER,       MAPDATASTAIRY,              1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStairsY",                     ZTID_VOID,          SETTER,       MAPDATASTAIRY,              1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getItemX",                       ZTID_FLOAT,         GETTER,       MAPDATAITEMX,               1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItemX",                       ZTID_VOID,          SETTER,       MAPDATAITEMX,               1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getItemY",                       ZTID_FLOAT,         GETTER,       MAPDATAITEMY,               1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItemY",                       ZTID_VOID,          SETTER,       MAPDATAITEMY,               1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getCSet",                        ZTID_FLOAT,         GETTER,       MAPDATACOLOUR,              1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setCSet",                        ZTID_VOID,          SETTER,       MAPDATACOLOUR,              1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getPalette",                     ZTID_FLOAT,         GETTER,       MAPDATACOLOUR,              1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPalette",                     ZTID_VOID,          SETTER,       MAPDATACOLOUR,              1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getEnemyFlags",                  ZTID_FLOAT,         GETTER,       MAPDATAENEMYFLAGS,          1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setEnemyFlags",                  ZTID_VOID,          SETTER,       MAPDATAENEMYFLAGS,          1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getDoor[]",                      ZTID_FLOAT,         GETTER,       MAPDATADOOR,                4,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDoor[]",                      ZTID_VOID,          SETTER,       MAPDATADOOR,                4,             0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getTileWarpDMap[]",              ZTID_FLOAT,         GETTER,       MAPDATATILEWARPDMAP,        4,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWarpDMap[]",              ZTID_VOID,          SETTER,       MAPDATATILEWARPDMAP,        4,             0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getTileWarpScreen[]",            ZTID_FLOAT,         GETTER,       MAPDATATILEWARPSCREEN,      4,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWarpScreen[]",            ZTID_VOID,          SETTER,       MAPDATATILEWARPSCREEN,      4,             0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getExitDir",                     ZTID_FLOAT,         GETTER,       MAPDATAEXITDIR,             1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setExitDir",                     ZTID_VOID,          SETTER,       MAPDATAEXITDIR,             1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getEnemy[]",                     ZTID_FLOAT,         GETTER,       MAPDATAENEMY,               10,            0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setEnemy[]",                     ZTID_VOID,          SETTER,       MAPDATAENEMY,               10,            0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getPattern",                     ZTID_FLOAT,         GETTER,       MAPDATAPATTERN,             1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPattern",                     ZTID_VOID,          SETTER,       MAPDATAPATTERN,             1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSideWarpType[]",              ZTID_FLOAT,         GETTER,       MAPDATASIDEWARPTYPE,        4,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSideWarpType[]",              ZTID_VOID,          SETTER,       MAPDATASIDEWARPTYPE,        4,             0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSideWarpOverlay[]",           ZTID_FLOAT,         GETTER,       MAPDATASIDEWARPOVFLAGS,     4,             0,                                    1,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSideWarpOverlay[]",           ZTID_VOID,          SETTER,       MAPDATASIDEWARPOVFLAGS,     4,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWarpArrivalX",                ZTID_FLOAT,         GETTER,       MAPDATAWARPARRIVALX,        1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWarpArrivalX",                ZTID_VOID,          SETTER,       MAPDATAWARPARRIVALX,        1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWarpArrivalY",                ZTID_FLOAT,         GETTER,       MAPDATAWARPARRIVALY,        1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWarpArrivalY",                ZTID_VOID,          SETTER,       MAPDATAWARPARRIVALY,        1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getMazePath[]",                  ZTID_FLOAT,         GETTER,       MAPDATAPATH,                4,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMazePath[]",                  ZTID_VOID,          SETTER,       MAPDATAPATH,                4,             0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSideWarpScreen[]",            ZTID_FLOAT,         GETTER,       MAPDATASIDEWARPSC,          4,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSideWarpScreen[]",            ZTID_VOID,          SETTER,       MAPDATASIDEWARPSC,          4,             0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSideWarpID[]",            ZTID_FLOAT,         GETTER,       MAPDATASIDEWARPID,          4,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSideWarpID[]",            ZTID_VOID,          SETTER,       MAPDATASIDEWARPID,          4,             0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getTileWarpReturnSquare[]",            ZTID_FLOAT,         GETTER,       MAPDATATWARPRETSQR,          4,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWarpReturnSquare[]",            ZTID_VOID,          SETTER,       MAPDATATWARPRETSQR,          4,             0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSideWarpReturnSquare[]",            ZTID_FLOAT,         GETTER,       MAPDATASWARPRETSQR,          4,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSideWarpReturnSquare[]",            ZTID_VOID,          SETTER,       MAPDATASWARPRETSQR,          4,             0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSideWarpDMap[]",              ZTID_FLOAT,         GETTER,       MAPDATASIDEWARPDMAP,        4,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSideWarpDMap[]",              ZTID_VOID,          SETTER,       MAPDATASIDEWARPDMAP,        4,             0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getSideWarpIndex",               ZTID_FLOAT,         GETTER,       MAPDATASIDEWARPINDEX,       1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setSideWarpIndex",               ZTID_VOID,          SETTER,       MAPDATASIDEWARPINDEX,       1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getUnderCombo",                  ZTID_FLOAT,         GETTER,       MAPDATAUNDERCOMBO,          1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setUnderCombo",                  ZTID_VOID,          SETTER,       MAPDATAUNDERCOMBO,          1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getUnderCSet",                   ZTID_FLOAT,         GETTER,       MAPDATAUNDERCSET,           1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setUnderCSet",                   ZTID_VOID,          SETTER,       MAPDATAUNDERCSET,           1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getCatchall",                    ZTID_FLOAT,         GETTER,       MAPDATACATCHALL,            1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCatchall",                    ZTID_VOID,          SETTER,       MAPDATACATCHALL,            1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getCSensitive",                  ZTID_FLOAT,         GETTER,       MAPDATACSENSITIVE,          1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCSensitive",                  ZTID_VOID,          SETTER,       MAPDATACSENSITIVE,          1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getNoReset",                     ZTID_FLOAT,         GETTER,       MAPDATANORESET,             1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNoReset",                     ZTID_VOID,          SETTER,       MAPDATANORESET,             1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getNoCarry",                     ZTID_FLOAT,         GETTER,       MAPDATANOCARRY,             1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNoCarry",                     ZTID_VOID,          SETTER,       MAPDATANOCARRY,             1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getLayerMap[]",                  ZTID_FLOAT,         GETTER,       MAPDATALAYERMAP,            7,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLayerMap[]",                  ZTID_VOID,          SETTER,       MAPDATALAYERMAP,            7,             0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getLayerScreen[]",               ZTID_FLOAT,         GETTER,       MAPDATALAYERSCREEN,         7,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLayerScreen[]",               ZTID_VOID,          SETTER,       MAPDATALAYERSCREEN,         7,             0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getLayerOpacity[]",              ZTID_FLOAT,         GETTER,       MAPDATALAYEROPACITY,        7,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLayerOpacity[]",              ZTID_VOID,          SETTER,       MAPDATALAYEROPACITY,        7,             0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getLayerInvisible[]",            ZTID_BOOL,          GETTER,       MAPDATALAYERINVIS,          7,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLayerInvisible[]",            ZTID_VOID,          SETTER,       MAPDATALAYERINVIS,          7,             0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getScriptDraws[]",               ZTID_BOOL,          GETTER,       MAPDATASCRIPTDRAWS,         8,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScriptDraws[]",               ZTID_VOID,          SETTER,       MAPDATASCRIPTDRAWS,         8,             0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	
	{ "getTimedWarpTimer",              ZTID_FLOAT,         GETTER,       MAPDATATIMEDWARPTICS,       1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTimedWarpTimer",              ZTID_VOID,          SETTER,       MAPDATATIMEDWARPTICS,       1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getNextMap",                     ZTID_FLOAT,         GETTER,       MAPDATANEXTMAP,             1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNextMap",                     ZTID_VOID,          SETTER,       MAPDATANEXTMAP,             1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getNextScreen",                  ZTID_FLOAT,         GETTER,       MAPDATANEXTSCREEN,          1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNextScreen",                  ZTID_VOID,          SETTER,       MAPDATANEXTSCREEN,          1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	
	{ "getCarryoverMap",                ZTID_FLOAT,         GETTER,       MAPDATANEXTMAP,             1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCarryoverMap",                ZTID_VOID,          SETTER,       MAPDATANEXTMAP,             1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getCarryoverScreen",             ZTID_FLOAT,         GETTER,       MAPDATANEXTSCREEN,          1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCarryoverSreen",              ZTID_VOID,          SETTER,       MAPDATANEXTSCREEN,          1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSecretCombo[]",               ZTID_FLOAT,         GETTER,       MAPDATASECRETCOMBO,         128,           0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSecretCombo[]",               ZTID_VOID,          SETTER,       MAPDATASECRETCOMBO,         128,           0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSecretCSet[]",                ZTID_FLOAT,         GETTER,       MAPDATASECRETCSET,          128,           0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSecretCSet[]",                ZTID_VOID,          SETTER,       MAPDATASECRETCSET,          128,           0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSecretFlags[]",               ZTID_FLOAT,         GETTER,       MAPDATASECRETFLAG,          128,           0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSecretFlags[]",               ZTID_VOID,          SETTER,       MAPDATASECRETFLAG,          128,           0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getViewX",                       ZTID_FLOAT,         GETTER,       MAPDATAVIEWX,               1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setViewX",                       ZTID_VOID,          SETTER,       MAPDATAVIEWX,               1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getViewY",                       ZTID_FLOAT,         GETTER,       MAPDATAVIEWY,               1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setViewY",                       ZTID_VOID,          SETTER,       MAPDATAVIEWY,               1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getWidth",                       ZTID_FLOAT,         GETTER,       MAPDATASCREENWIDTH,         1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setWidth",                       ZTID_VOID,          SETTER,       MAPDATASCREENWIDTH,         1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getHeight",                      ZTID_FLOAT,         GETTER,       MAPDATASCREENHEIGHT,        1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setHeight",                      ZTID_VOID,          SETTER,       MAPDATASCREENHEIGHT,        1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getEntryX",                      ZTID_FLOAT,         GETTER,       MAPDATAENTRYX,              1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setEntryX",                      ZTID_VOID,          SETTER,       MAPDATAENTRYX,              1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getEntryY",                      ZTID_FLOAT,         GETTER,       MAPDATAENTRYY,              1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setEntryY",                      ZTID_VOID,          SETTER,       MAPDATAENTRYY,              1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getNumFFCs[]",                   ZTID_BOOL,          GETTER,       MAPDATANUMFF,              33,            0,                                    1,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	//read-only, for now?
	 { "setNumFFCs[]",                   ZTID_VOID,          SETTER,       MAPDATANUMFF,              33,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCData[]",                   ZTID_FLOAT,         GETTER,       MAPDATAFFDATA,              33,            0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCData[]",                   ZTID_VOID,          SETTER,       MAPDATAFFDATA,              33,            0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCCSet[]",                   ZTID_FLOAT,         GETTER,       MAPDATAFFCSET,              33,            0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCCSet[]",                   ZTID_VOID,          SETTER,       MAPDATAFFCSET,              33,            0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCDelay[]",                  ZTID_FLOAT,         GETTER,       MAPDATAFFDELAY,             33,            0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCDelay[]",                  ZTID_VOID,          SETTER,       MAPDATAFFDELAY,             33,            0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCX[]",                      ZTID_FLOAT,         GETTER,       MAPDATAFFX,                 33,            0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCX[]",                      ZTID_VOID,          SETTER,       MAPDATAFFX,                 33,            0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCY[]",                      ZTID_FLOAT,         GETTER,       MAPDATAFFY,                 33,            0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCY[]",                      ZTID_VOID,          SETTER,       MAPDATAFFY,                 33,            0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCVx[]",                     ZTID_FLOAT,         GETTER,       MAPDATAFFXDELTA,            33,            0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCVx[]",                     ZTID_VOID,          SETTER,       MAPDATAFFXDELTA,            33,            0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCVy[]",                     ZTID_FLOAT,         GETTER,       MAPDATAFFYDELTA,            33,            0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCVy[]",                     ZTID_VOID,          SETTER,       MAPDATAFFYDELTA,            33,            0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCAx[]",                     ZTID_FLOAT,         GETTER,       MAPDATAFFXDELTA2,           33,            0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCAx[]",                     ZTID_VOID,          SETTER,       MAPDATAFFXDELTA2,           33,            0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCAy[]",                     ZTID_FLOAT,         GETTER,       MAPDATAFFYDELTA2,           33,            0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCAy[]",                     ZTID_VOID,          SETTER,       MAPDATAFFYDELTA2,           33,            0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCFlags[]",                  ZTID_FLOAT,         GETTER,       MAPDATAFFFLAGS,             33,            0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCFlags[]",                  ZTID_VOID,          SETTER,       MAPDATAFFFLAGS,             33,            0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCEffectWidth[]",            ZTID_FLOAT,         GETTER,       MAPDATAFFEFFECTWIDTH,       33,            0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCEffectWidth[]",            ZTID_VOID,          SETTER,       MAPDATAFFEFFECTWIDTH,       33,            0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCEffectHeight[]",           ZTID_FLOAT,         GETTER,       MAPDATAFFEFFECTHEIGHT,      33,            0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCEffectHeight[]",           ZTID_VOID,          SETTER,       MAPDATAFFEFFECTHEIGHT,      33,            0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCTileWidth[]",              ZTID_FLOAT,         GETTER,       MAPDATAFFWIDTH,             33,            0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCTileWidth[]",              ZTID_VOID,          SETTER,       MAPDATAFFWIDTH,             33,            0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCTileHeight[]",             ZTID_FLOAT,         GETTER,       MAPDATAFFHEIGHT,            33,            0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCTileHeight[]",             ZTID_VOID,          SETTER,       MAPDATAFFHEIGHT,            33,            0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCLink[]",                   ZTID_FLOAT,         GETTER,       MAPDATAFFLINK,              33,            0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCLink[]",                   ZTID_VOID,          SETTER,       MAPDATAFFLINK,              33,            0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCScript[]",                 ZTID_FLOAT,         GETTER,       MAPDATAFFSCRIPT,            33,            0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCScript[]",                 ZTID_VOID,          SETTER,       MAPDATAFFSCRIPT,            33,            0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getFFCRunning[]",                ZTID_BOOL,          GETTER,       MAPDATAFFINITIALISED,       33,            0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setFFCRunning[]",                ZTID_VOID,          SETTER,       MAPDATAFFINITIALISED,       33,            0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getScriptEntry",                 ZTID_FLOAT,         GETTER,       MAPDATASCRIPTENTRY,         1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setScriptEntry",                 ZTID_VOID,          SETTER,       MAPDATASCRIPTENTRY,         1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getScriptOccupancy",             ZTID_FLOAT,         GETTER,       MAPDATASCRIPTOCCUPANCY,     1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setScriptOccupancy",             ZTID_VOID,          SETTER,       MAPDATASCRIPTOCCUPANCY,     1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getExitScript",                  ZTID_FLOAT,         GETTER,       MAPDATASCRIPTEXIT,          1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setExitScript",                  ZTID_VOID,          SETTER,       MAPDATASCRIPTEXIT,          1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getOceanSFX",                    ZTID_FLOAT,         GETTER,       MAPDATAOCEANSFX,            1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setOceanSFX",                    ZTID_VOID,          SETTER,       MAPDATAOCEANSFX,            1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAmbientSFX",                  ZTID_FLOAT,         GETTER,       MAPDATAOCEANSFX,            1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAmbientSFX",                  ZTID_VOID,          SETTER,       MAPDATAOCEANSFX,            1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getBossSFX",                     ZTID_FLOAT,         GETTER,       MAPDATABOSSSFX,             1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setBossSFX",                     ZTID_VOID,          SETTER,       MAPDATABOSSSFX,             1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSecretSFX",                   ZTID_FLOAT,         GETTER,       MAPDATASECRETSFX,           1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSecretSFX",                   ZTID_VOID,          SETTER,       MAPDATASECRETSFX,           1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getItemSFX",                     ZTID_FLOAT,         GETTER,       MAPDATAHOLDUPSFX,           1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItemSFX",                     ZTID_VOID,          SETTER,       MAPDATAHOLDUPSFX,           1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getMIDI",                        ZTID_FLOAT,         GETTER,       MAPDATASCREENMIDI,          1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMIDI",                        ZTID_VOID,          SETTER,       MAPDATASCREENMIDI,          1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getLensLayer",                   ZTID_FLOAT,         GETTER,       MAPDATALENSLAYER,           1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLensLayer",                   ZTID_VOID,          SETTER,       MAPDATALENSLAYER,           1,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFlags[]",                     ZTID_FLOAT,         GETTER,       MAPDATAFLAGS,               10,            0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlags[]",                     ZTID_VOID,          SETTER,       MAPDATAFLAGS,               10,            0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	array of 11
	{ "getD[]",                         ZTID_UNTYPED,       GETTER,       MAPDATAMISCD,               8,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setD[]",                         ZTID_VOID,          SETTER,       MAPDATAMISCD,               8,             0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	array of 10 MAPDATAMISCD
	{ "getComboD[]",                    ZTID_FLOAT,         GETTER,       MAPDATACOMBODD,             176,           0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setComboD[]",                    ZTID_VOID,          SETTER,       MAPDATACOMBODD,             176,           0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getComboC[]",                    ZTID_FLOAT,         GETTER,       MAPDATACOMBOCD,             176,           0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setComboC[]",                    ZTID_VOID,          SETTER,       MAPDATACOMBOCD,             176,           0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getComboF[]",                    ZTID_FLOAT,         GETTER,       MAPDATACOMBOFD,             176,           0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setComboF[]",                    ZTID_VOID,          SETTER,       MAPDATACOMBOFD,             176,           0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getComboI[]",                    ZTID_FLOAT,         GETTER,       MAPDATACOMBOID,             176,           0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setComboI[]",                    ZTID_VOID,          SETTER,       MAPDATACOMBOID,             176,           0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getComboT[]",                    ZTID_FLOAT,         GETTER,       MAPDATACOMBOTD,             176,           0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setComboT[]",                    ZTID_VOID,          SETTER,       MAPDATACOMBOTD,             176,           0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getComboS[]",                    ZTID_FLOAT,         GETTER,       MAPDATACOMBOSD,             176,           0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setComboS[]",                    ZTID_VOID,          SETTER,       MAPDATACOMBOSD,             176,           0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getComboE[]",                    ZTID_FLOAT,         GETTER,       MAPDATACOMBOED,             176,           0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setComboE[]",                    ZTID_VOID,          SETTER,       MAPDATACOMBOED,             176,           0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getState[]",                     ZTID_BOOL,          GETTER,       MAPDATASCREENSTATED,        32,            0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setState[]",                     ZTID_VOID,          SETTER,       MAPDATASCREENSTATED,        32,            0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getExState[]",                   ZTID_BOOL,          GETTER,       MAPDATAEXSTATED,            32,            0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setExState[]",                   ZTID_VOID,          SETTER,       MAPDATAEXSTATED,            32,            0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSFlags[]",                     ZTID_FLOAT,         GETTER,       MAPDATASCREENFLAGSD,        10,            0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "setFlags[]",                     ZTID_VOID,          SETTER,       MAPDATASCREENFLAGSD,        10,            0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	This is read-only, but it was not previously blocked! -Z
	{ "getEFlags[]",                    ZTID_FLOAT,         GETTER,       MAPDATASCREENEFLAGSD,       3,             0,                                    2,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	This is read-only, but it was not previously blocked! -Z
//	{ "setEFlags[]",                    ZTID_VOID,          SETTER,       MAPDATASCREENEFLAGSD,       3,             0,                                    3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
//	Functions

	{ "GetFFCInitD",                    ZTID_UNTYPED,       FUNCTION,     0,                          1,             FUNCFLAG_INLINE,                      3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetFFCInitD",                    ZTID_VOID,          FUNCTION,     0,                          1,             0,                                    4,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "GetFFCInitA",                    ZTID_FLOAT,         FUNCTION,     0,                          1,             FUNCFLAG_INLINE,                      3,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetFFCInitA",                    ZTID_FLOAT,         FUNCTION,     0,                          1,             0,                                    4,           { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getMap",                         ZTID_FLOAT,         GETTER,       MAPDATAMAP,                 1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMap",                         ZTID_FLOAT,         SETTER,       MAPDATAMAP,                 1,             0,                                    1,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScreen",                      ZTID_FLOAT,         GETTER,       MAPDATASCREEN,              1,             0,                                    1,           { ZTID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScreen",                      ZTID_FLOAT,         SETTER,       MAPDATASCREEN,              1,             0,                                    1,           { ZTID_MAPDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	Get/SetFFCMiscD
	
	{ "",                               -1,                       -1,           -1,                         -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

MapDataSymbols::MapDataSymbols()
{
	table = MapDataTable;
	refVar = REFMAPDATA; //NUL; //
}

void MapDataSymbols::generateCode()
{
	//bool isSolid(screen, int32_t, int32_t)
	{
		Function* function = getFunction("isSolid", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		//pop pointer
		POPREF();
		addOpcode2 (code, new OIsSolidMapdata(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	
	//bool isSolidLayer(screen, int32_t, int32_t, int32_t)
	{
		Function* function = getFunction("isSolidLayer", 4);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		//pop pointer
		POPREF();
		addOpcode2 (code, new OIsSolidMapdataLayer(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}

	//int32_t GetFFCInitD(mapscr, int32_t,int32_t,int32_t)
	{
		Function* function = getFunction("GetFFCInitD", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		//pop pointer
		POPREF();
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(MAPDATAINTID)));
		RETURN();
		function->giveCode(code);
	
	}
	
	//void SetFFCInitD(mapsc, int32_t,int32_t,int32_t,int32_t)
	{
		Function* function = getFunction("SetFFCInitD", 4);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		//pop pointer
		POPREF();
		addOpcode2 (code, new OSetRegister(new VarArgument(MAPDATAINTID), new VarArgument(SFTEMP)));
		RETURN();
		function->giveCode(code);
	}
	
	
	//int32_t GetFFCInitA(mapscr, int32_t,int32_t,int32_t)
	{
		Function* function = getFunction("GetFFCInitA", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		//pop pointer
		POPREF();
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(MAPDATAINITA)));
		RETURN();
		function->giveCode(code);
	
	}
	
	//void SetFFCInitA(mapsc, int32_t,int32_t,int32_t,int32_t)
	{
		Function* function = getFunction("SetFFCInitA", 4);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		//pop pointer
		POPREF();
		addOpcode2 (code, new OSetRegister(new VarArgument(MAPDATAINITA), new VarArgument(SFTEMP)));
		RETURN();
		function->giveCode(code);
	}
}

