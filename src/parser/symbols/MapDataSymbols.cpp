#include "SymbolDefs.h"

MapDataSymbols MapDataSymbols::singleton = MapDataSymbols();

static AccessorTable MapDataTable[] =
{
//	  name,                                     rettype,          setorget,     var,                 numindex,             funcFlags,                            numParams,   params
	{ "isSolid",                        ZVARTYPEID_BOOL,          FUNCTION,     0,                          1,             FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "isSolidLayer",                   ZVARTYPEID_BOOL,          FUNCTION,     0,                          1,             0,                                    4,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getValid",                       ZVARTYPEID_FLOAT,         GETTER,       MAPDATAVALID,               1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setValid",                       ZVARTYPEID_VOID,          SETTER,       MAPDATAVALID,               1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getGuy",                         ZVARTYPEID_FLOAT,         GETTER,       MAPDATAGUY,                 1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setGuy",                         ZVARTYPEID_VOID,          SETTER,       MAPDATAGUY,                 1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getString",                      ZVARTYPEID_FLOAT,         GETTER,       MAPDATASTRING,              1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setString",                      ZVARTYPEID_VOID,          SETTER,       MAPDATASTRING,              1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getRoomType",                    ZVARTYPEID_FLOAT,         GETTER,       MAPDATAROOM,                1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRoomType",                    ZVARTYPEID_VOID,          SETTER,       MAPDATAROOM,                1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getItem",                        ZVARTYPEID_FLOAT,         GETTER,       MAPDATAITEM,                1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItem",                        ZVARTYPEID_VOID,          SETTER,       MAPDATAITEM,                1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getHasItem",                     ZVARTYPEID_FLOAT,         GETTER,       MAPDATAHASITEM,             1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setHasItem",                     ZVARTYPEID_VOID,          SETTER,       MAPDATAHASITEM,             1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getTileWarpType[]",              ZVARTYPEID_FLOAT,         GETTER,       MAPDATATILEWARPTYPE,        4,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWarpType[]",              ZVARTYPEID_VOID,          SETTER,       MAPDATATILEWARPTYPE,        4,             0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getScript",                      ZVARTYPEID_FLOAT,         GETTER,       MAPDATASCRIPT,              1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScript",                      ZVARTYPEID_VOID,          SETTER,       MAPDATASCRIPT,              1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getInitD[]",                     ZVARTYPEID_UNTYPED,       GETTER,       MAPDATAINITDARRAY,          8,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInitD[]",                     ZVARTYPEID_VOID,          SETTER,       MAPDATAINITDARRAY,          8,             0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getTileWarpOverlay[]",           ZVARTYPEID_BOOL,          GETTER,       MAPDATATILEWARPOVFLAGS,     4,             0,                                    1,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWarpOverlay[]",           ZVARTYPEID_VOID,          SETTER,       MAPDATATILEWARPOVFLAGS,     4,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getDoorComboSet",                ZVARTYPEID_FLOAT,         GETTER,       MAPDATADOORCOMBOSET,        1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDoorComboSet",                ZVARTYPEID_VOID,          SETTER,       MAPDATADOORCOMBOSET,        1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWarpReturnX[]",               ZVARTYPEID_FLOAT,         GETTER,       MAPDATAWARPRETX,            4,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWarpReturnX[]",               ZVARTYPEID_VOID,          SETTER,       MAPDATAWARPRETX,            4,             0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWarpReturnY[]",               ZVARTYPEID_FLOAT,         GETTER,       MAPDATAWARPRETY,            4,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWarpReturnY[]",               ZVARTYPEID_VOID,          SETTER,       MAPDATAWARPRETY,            4,             0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getWarpReturnC",                 ZVARTYPEID_FLOAT,         GETTER,       MAPDATAWARPRETURNC,         1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setWarpReturnC",                 ZVARTYPEID_VOID,          SETTER,       MAPDATAWARPRETURNC,         1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getStairsX",                     ZVARTYPEID_FLOAT,         GETTER,       MAPDATASTAIRX,              1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStairsX",                     ZVARTYPEID_VOID,          SETTER,       MAPDATASTAIRX,              1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getStairsY",                     ZVARTYPEID_FLOAT,         GETTER,       MAPDATASTAIRY,              1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStairsY",                     ZVARTYPEID_VOID,          SETTER,       MAPDATASTAIRY,              1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getItemX",                       ZVARTYPEID_FLOAT,         GETTER,       MAPDATAITEMX,               1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItemX",                       ZVARTYPEID_VOID,          SETTER,       MAPDATAITEMX,               1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getItemY",                       ZVARTYPEID_FLOAT,         GETTER,       MAPDATAITEMY,               1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItemY",                       ZVARTYPEID_VOID,          SETTER,       MAPDATAITEMY,               1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getCSet",                        ZVARTYPEID_FLOAT,         GETTER,       MAPDATACOLOUR,              1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setCSet",                        ZVARTYPEID_VOID,          SETTER,       MAPDATACOLOUR,              1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getPalette",                     ZVARTYPEID_FLOAT,         GETTER,       MAPDATACOLOUR,              1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPalette",                     ZVARTYPEID_VOID,          SETTER,       MAPDATACOLOUR,              1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getEnemyFlags",                  ZVARTYPEID_FLOAT,         GETTER,       MAPDATAENEMYFLAGS,          1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setEnemyFlags",                  ZVARTYPEID_VOID,          SETTER,       MAPDATAENEMYFLAGS,          1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getDoor[]",                      ZVARTYPEID_FLOAT,         GETTER,       MAPDATADOOR,                4,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDoor[]",                      ZVARTYPEID_VOID,          SETTER,       MAPDATADOOR,                4,             0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getTileWarpDMap[]",              ZVARTYPEID_FLOAT,         GETTER,       MAPDATATILEWARPDMAP,        4,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWarpDMap[]",              ZVARTYPEID_VOID,          SETTER,       MAPDATATILEWARPDMAP,        4,             0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getTileWarpScreen[]",            ZVARTYPEID_FLOAT,         GETTER,       MAPDATATILEWARPSCREEN,      4,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWarpScreen[]",            ZVARTYPEID_VOID,          SETTER,       MAPDATATILEWARPSCREEN,      4,             0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getExitDir",                     ZVARTYPEID_FLOAT,         GETTER,       MAPDATAEXITDIR,             1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setExitDir",                     ZVARTYPEID_VOID,          SETTER,       MAPDATAEXITDIR,             1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getEnemy[]",                     ZVARTYPEID_FLOAT,         GETTER,       MAPDATAENEMY,               10,            0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setEnemy[]",                     ZVARTYPEID_VOID,          SETTER,       MAPDATAENEMY,               10,            0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getPattern",                     ZVARTYPEID_FLOAT,         GETTER,       MAPDATAPATTERN,             1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPattern",                     ZVARTYPEID_VOID,          SETTER,       MAPDATAPATTERN,             1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSideWarpType[]",              ZVARTYPEID_FLOAT,         GETTER,       MAPDATASIDEWARPTYPE,        4,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSideWarpType[]",              ZVARTYPEID_VOID,          SETTER,       MAPDATASIDEWARPTYPE,        4,             0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSideWarpOverlay[]",           ZVARTYPEID_FLOAT,         GETTER,       MAPDATASIDEWARPOVFLAGS,     4,             0,                                    1,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSideWarpOverlay[]",           ZVARTYPEID_VOID,          SETTER,       MAPDATASIDEWARPOVFLAGS,     4,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWarpArrivalX",                ZVARTYPEID_FLOAT,         GETTER,       MAPDATAWARPARRIVALX,        1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWarpArrivalX",                ZVARTYPEID_VOID,          SETTER,       MAPDATAWARPARRIVALX,        1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWarpArrivalY",                ZVARTYPEID_FLOAT,         GETTER,       MAPDATAWARPARRIVALY,        1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWarpArrivalY",                ZVARTYPEID_VOID,          SETTER,       MAPDATAWARPARRIVALY,        1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getMazePath[]",                  ZVARTYPEID_FLOAT,         GETTER,       MAPDATAPATH,                4,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMazePath[]",                  ZVARTYPEID_VOID,          SETTER,       MAPDATAPATH,                4,             0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSideWarpScreen[]",            ZVARTYPEID_FLOAT,         GETTER,       MAPDATASIDEWARPSC,          4,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSideWarpScreen[]",            ZVARTYPEID_VOID,          SETTER,       MAPDATASIDEWARPSC,          4,             0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSideWarpID[]",            ZVARTYPEID_FLOAT,         GETTER,       MAPDATASIDEWARPID,          4,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSideWarpID[]",            ZVARTYPEID_VOID,          SETTER,       MAPDATASIDEWARPID,          4,             0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getTileWarpReturnSquare[]",            ZVARTYPEID_FLOAT,         GETTER,       MAPDATATWARPRETSQR,          4,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWarpReturnSquare[]",            ZVARTYPEID_VOID,          SETTER,       MAPDATATWARPRETSQR,          4,             0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSideWarpReturnSquare[]",            ZVARTYPEID_FLOAT,         GETTER,       MAPDATASWARPRETSQR,          4,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSideWarpReturnSquare[]",            ZVARTYPEID_VOID,          SETTER,       MAPDATASWARPRETSQR,          4,             0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSideWarpDMap[]",              ZVARTYPEID_FLOAT,         GETTER,       MAPDATASIDEWARPDMAP,        4,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSideWarpDMap[]",              ZVARTYPEID_VOID,          SETTER,       MAPDATASIDEWARPDMAP,        4,             0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getSideWarpIndex",               ZVARTYPEID_FLOAT,         GETTER,       MAPDATASIDEWARPINDEX,       1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setSideWarpIndex",               ZVARTYPEID_VOID,          SETTER,       MAPDATASIDEWARPINDEX,       1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getUnderCombo",                  ZVARTYPEID_FLOAT,         GETTER,       MAPDATAUNDERCOMBO,          1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setUnderCombo",                  ZVARTYPEID_VOID,          SETTER,       MAPDATAUNDERCOMBO,          1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getUnderCSet",                   ZVARTYPEID_FLOAT,         GETTER,       MAPDATAUNDERCSET,           1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setUnderCSet",                   ZVARTYPEID_VOID,          SETTER,       MAPDATAUNDERCSET,           1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getCatchall",                    ZVARTYPEID_FLOAT,         GETTER,       MAPDATACATCHALL,            1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCatchall",                    ZVARTYPEID_VOID,          SETTER,       MAPDATACATCHALL,            1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getCSensitive",                  ZVARTYPEID_FLOAT,         GETTER,       MAPDATACSENSITIVE,          1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCSensitive",                  ZVARTYPEID_VOID,          SETTER,       MAPDATACSENSITIVE,          1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getNoReset",                     ZVARTYPEID_FLOAT,         GETTER,       MAPDATANORESET,             1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNoReset",                     ZVARTYPEID_VOID,          SETTER,       MAPDATANORESET,             1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getNoCarry",                     ZVARTYPEID_FLOAT,         GETTER,       MAPDATANOCARRY,             1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNoCarry",                     ZVARTYPEID_VOID,          SETTER,       MAPDATANOCARRY,             1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getLayerMap[]",                  ZVARTYPEID_FLOAT,         GETTER,       MAPDATALAYERMAP,            7,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLayerMap[]",                  ZVARTYPEID_VOID,          SETTER,       MAPDATALAYERMAP,            7,             0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getLayerScreen[]",               ZVARTYPEID_FLOAT,         GETTER,       MAPDATALAYERSCREEN,         7,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLayerScreen[]",               ZVARTYPEID_VOID,          SETTER,       MAPDATALAYERSCREEN,         7,             0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getLayerOpacity[]",              ZVARTYPEID_FLOAT,         GETTER,       MAPDATALAYEROPACITY,        7,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLayerOpacity[]",              ZVARTYPEID_VOID,          SETTER,       MAPDATALAYEROPACITY,        7,             0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getLayerInvisible[]",            ZVARTYPEID_BOOL,          GETTER,       MAPDATALAYERINVIS,          7,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLayerInvisible[]",            ZVARTYPEID_VOID,          SETTER,       MAPDATALAYERINVIS,          7,             0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getScriptDraws[]",               ZVARTYPEID_BOOL,          GETTER,       MAPDATASCRIPTDRAWS,         8,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScriptDraws[]",               ZVARTYPEID_VOID,          SETTER,       MAPDATASCRIPTDRAWS,         8,             0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	
	{ "getTimedWarpTimer",              ZVARTYPEID_FLOAT,         GETTER,       MAPDATATIMEDWARPTICS,       1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTimedWarpTimer",              ZVARTYPEID_VOID,          SETTER,       MAPDATATIMEDWARPTICS,       1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getNextMap",                     ZVARTYPEID_FLOAT,         GETTER,       MAPDATANEXTMAP,             1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNextMap",                     ZVARTYPEID_VOID,          SETTER,       MAPDATANEXTMAP,             1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getNextScreen",                  ZVARTYPEID_FLOAT,         GETTER,       MAPDATANEXTSCREEN,          1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNextScreen",                  ZVARTYPEID_VOID,          SETTER,       MAPDATANEXTSCREEN,          1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	
	{ "getCarryoverMap",                ZVARTYPEID_FLOAT,         GETTER,       MAPDATANEXTMAP,             1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCarryoverMap",                ZVARTYPEID_VOID,          SETTER,       MAPDATANEXTMAP,             1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getCarryoverScreen",             ZVARTYPEID_FLOAT,         GETTER,       MAPDATANEXTSCREEN,          1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCarryoverSreen",              ZVARTYPEID_VOID,          SETTER,       MAPDATANEXTSCREEN,          1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSecretCombo[]",               ZVARTYPEID_FLOAT,         GETTER,       MAPDATASECRETCOMBO,         128,           0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSecretCombo[]",               ZVARTYPEID_VOID,          SETTER,       MAPDATASECRETCOMBO,         128,           0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSecretCSet[]",                ZVARTYPEID_FLOAT,         GETTER,       MAPDATASECRETCSET,          128,           0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSecretCSet[]",                ZVARTYPEID_VOID,          SETTER,       MAPDATASECRETCSET,          128,           0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSecretFlags[]",               ZVARTYPEID_FLOAT,         GETTER,       MAPDATASECRETFLAG,          128,           0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSecretFlags[]",               ZVARTYPEID_VOID,          SETTER,       MAPDATASECRETFLAG,          128,           0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getViewX",                       ZVARTYPEID_FLOAT,         GETTER,       MAPDATAVIEWX,               1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setViewX",                       ZVARTYPEID_VOID,          SETTER,       MAPDATAVIEWX,               1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getViewY",                       ZVARTYPEID_FLOAT,         GETTER,       MAPDATAVIEWY,               1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setViewY",                       ZVARTYPEID_VOID,          SETTER,       MAPDATAVIEWY,               1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getWidth",                       ZVARTYPEID_FLOAT,         GETTER,       MAPDATASCREENWIDTH,         1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setWidth",                       ZVARTYPEID_VOID,          SETTER,       MAPDATASCREENWIDTH,         1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getHeight",                      ZVARTYPEID_FLOAT,         GETTER,       MAPDATASCREENHEIGHT,        1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setHeight",                      ZVARTYPEID_VOID,          SETTER,       MAPDATASCREENHEIGHT,        1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getEntryX",                      ZVARTYPEID_FLOAT,         GETTER,       MAPDATAENTRYX,              1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setEntryX",                      ZVARTYPEID_VOID,          SETTER,       MAPDATAENTRYX,              1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getEntryY",                      ZVARTYPEID_FLOAT,         GETTER,       MAPDATAENTRYY,              1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setEntryY",                      ZVARTYPEID_VOID,          SETTER,       MAPDATAENTRYY,              1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getNumFFCs[]",                   ZVARTYPEID_BOOL,          GETTER,       MAPDATANUMFF,              33,            0,                                    1,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	//read-only, for now?
	 { "setNumFFCs[]",                   ZVARTYPEID_VOID,          SETTER,       MAPDATANUMFF,              33,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCData[]",                   ZVARTYPEID_FLOAT,         GETTER,       MAPDATAFFDATA,              33,            0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCData[]",                   ZVARTYPEID_VOID,          SETTER,       MAPDATAFFDATA,              33,            0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCCSet[]",                   ZVARTYPEID_FLOAT,         GETTER,       MAPDATAFFCSET,              33,            0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCCSet[]",                   ZVARTYPEID_VOID,          SETTER,       MAPDATAFFCSET,              33,            0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCDelay[]",                  ZVARTYPEID_FLOAT,         GETTER,       MAPDATAFFDELAY,             33,            0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCDelay[]",                  ZVARTYPEID_VOID,          SETTER,       MAPDATAFFDELAY,             33,            0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCX[]",                      ZVARTYPEID_FLOAT,         GETTER,       MAPDATAFFX,                 33,            0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCX[]",                      ZVARTYPEID_VOID,          SETTER,       MAPDATAFFX,                 33,            0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCY[]",                      ZVARTYPEID_FLOAT,         GETTER,       MAPDATAFFY,                 33,            0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCY[]",                      ZVARTYPEID_VOID,          SETTER,       MAPDATAFFY,                 33,            0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCVx[]",                     ZVARTYPEID_FLOAT,         GETTER,       MAPDATAFFXDELTA,            33,            0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCVx[]",                     ZVARTYPEID_VOID,          SETTER,       MAPDATAFFXDELTA,            33,            0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCVy[]",                     ZVARTYPEID_FLOAT,         GETTER,       MAPDATAFFYDELTA,            33,            0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCVy[]",                     ZVARTYPEID_VOID,          SETTER,       MAPDATAFFYDELTA,            33,            0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCAx[]",                     ZVARTYPEID_FLOAT,         GETTER,       MAPDATAFFXDELTA2,           33,            0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCAx[]",                     ZVARTYPEID_VOID,          SETTER,       MAPDATAFFXDELTA2,           33,            0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCAy[]",                     ZVARTYPEID_FLOAT,         GETTER,       MAPDATAFFYDELTA2,           33,            0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCAy[]",                     ZVARTYPEID_VOID,          SETTER,       MAPDATAFFYDELTA2,           33,            0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCFlags[]",                  ZVARTYPEID_FLOAT,         GETTER,       MAPDATAFFFLAGS,             33,            0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCFlags[]",                  ZVARTYPEID_VOID,          SETTER,       MAPDATAFFFLAGS,             33,            0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCEffectWidth[]",            ZVARTYPEID_FLOAT,         GETTER,       MAPDATAFFEFFECTWIDTH,       33,            0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCEffectWidth[]",            ZVARTYPEID_VOID,          SETTER,       MAPDATAFFEFFECTWIDTH,       33,            0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCEffectHeight[]",           ZVARTYPEID_FLOAT,         GETTER,       MAPDATAFFEFFECTHEIGHT,      33,            0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCEffectHeight[]",           ZVARTYPEID_VOID,          SETTER,       MAPDATAFFEFFECTHEIGHT,      33,            0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCTileWidth[]",              ZVARTYPEID_FLOAT,         GETTER,       MAPDATAFFWIDTH,             33,            0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCTileWidth[]",              ZVARTYPEID_VOID,          SETTER,       MAPDATAFFWIDTH,             33,            0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCTileHeight[]",             ZVARTYPEID_FLOAT,         GETTER,       MAPDATAFFHEIGHT,            33,            0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCTileHeight[]",             ZVARTYPEID_VOID,          SETTER,       MAPDATAFFHEIGHT,            33,            0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCLink[]",                   ZVARTYPEID_FLOAT,         GETTER,       MAPDATAFFLINK,              33,            0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCLink[]",                   ZVARTYPEID_VOID,          SETTER,       MAPDATAFFLINK,              33,            0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFFCScript[]",                 ZVARTYPEID_FLOAT,         GETTER,       MAPDATAFFSCRIPT,            33,            0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFFCScript[]",                 ZVARTYPEID_VOID,          SETTER,       MAPDATAFFSCRIPT,            33,            0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getFFCRunning[]",                ZVARTYPEID_BOOL,          GETTER,       MAPDATAFFINITIALISED,       33,            0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setFFCRunning[]",                ZVARTYPEID_VOID,          SETTER,       MAPDATAFFINITIALISED,       33,            0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getScriptEntry",                 ZVARTYPEID_FLOAT,         GETTER,       MAPDATASCRIPTENTRY,         1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setScriptEntry",                 ZVARTYPEID_VOID,          SETTER,       MAPDATASCRIPTENTRY,         1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getScriptOccupancy",             ZVARTYPEID_FLOAT,         GETTER,       MAPDATASCRIPTOCCUPANCY,     1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setScriptOccupancy",             ZVARTYPEID_VOID,          SETTER,       MAPDATASCRIPTOCCUPANCY,     1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getExitScript",                  ZVARTYPEID_FLOAT,         GETTER,       MAPDATASCRIPTEXIT,          1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setExitScript",                  ZVARTYPEID_VOID,          SETTER,       MAPDATASCRIPTEXIT,          1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getOceanSFX",                    ZVARTYPEID_FLOAT,         GETTER,       MAPDATAOCEANSFX,            1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setOceanSFX",                    ZVARTYPEID_VOID,          SETTER,       MAPDATAOCEANSFX,            1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAmbientSFX",                  ZVARTYPEID_FLOAT,         GETTER,       MAPDATAOCEANSFX,            1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAmbientSFX",                  ZVARTYPEID_VOID,          SETTER,       MAPDATAOCEANSFX,            1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getBossSFX",                     ZVARTYPEID_FLOAT,         GETTER,       MAPDATABOSSSFX,             1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setBossSFX",                     ZVARTYPEID_VOID,          SETTER,       MAPDATABOSSSFX,             1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSecretSFX",                   ZVARTYPEID_FLOAT,         GETTER,       MAPDATASECRETSFX,           1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSecretSFX",                   ZVARTYPEID_VOID,          SETTER,       MAPDATASECRETSFX,           1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getItemSFX",                     ZVARTYPEID_FLOAT,         GETTER,       MAPDATAHOLDUPSFX,           1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItemSFX",                     ZVARTYPEID_VOID,          SETTER,       MAPDATAHOLDUPSFX,           1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getMIDI",                        ZVARTYPEID_FLOAT,         GETTER,       MAPDATASCREENMIDI,          1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMIDI",                        ZVARTYPEID_VOID,          SETTER,       MAPDATASCREENMIDI,          1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getLensLayer",                   ZVARTYPEID_FLOAT,         GETTER,       MAPDATALENSLAYER,           1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLensLayer",                   ZVARTYPEID_VOID,          SETTER,       MAPDATALENSLAYER,           1,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getFlags[]",                     ZVARTYPEID_FLOAT,         GETTER,       MAPDATAFLAGS,               10,            0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlags[]",                     ZVARTYPEID_VOID,          SETTER,       MAPDATAFLAGS,               10,            0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	array of 11
	{ "getD[]",                         ZVARTYPEID_UNTYPED,       GETTER,       MAPDATAMISCD,               8,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setD[]",                         ZVARTYPEID_VOID,          SETTER,       MAPDATAMISCD,               8,             0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	array of 10 MAPDATAMISCD
	{ "getComboD[]",                    ZVARTYPEID_FLOAT,         GETTER,       MAPDATACOMBODD,             176,           0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setComboD[]",                    ZVARTYPEID_VOID,          SETTER,       MAPDATACOMBODD,             176,           0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getComboC[]",                    ZVARTYPEID_FLOAT,         GETTER,       MAPDATACOMBOCD,             176,           0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setComboC[]",                    ZVARTYPEID_VOID,          SETTER,       MAPDATACOMBOCD,             176,           0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getComboF[]",                    ZVARTYPEID_FLOAT,         GETTER,       MAPDATACOMBOFD,             176,           0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setComboF[]",                    ZVARTYPEID_VOID,          SETTER,       MAPDATACOMBOFD,             176,           0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getComboI[]",                    ZVARTYPEID_FLOAT,         GETTER,       MAPDATACOMBOID,             176,           0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setComboI[]",                    ZVARTYPEID_VOID,          SETTER,       MAPDATACOMBOID,             176,           0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getComboT[]",                    ZVARTYPEID_FLOAT,         GETTER,       MAPDATACOMBOTD,             176,           0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setComboT[]",                    ZVARTYPEID_VOID,          SETTER,       MAPDATACOMBOTD,             176,           0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getComboS[]",                    ZVARTYPEID_FLOAT,         GETTER,       MAPDATACOMBOSD,             176,           0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setComboS[]",                    ZVARTYPEID_VOID,          SETTER,       MAPDATACOMBOSD,             176,           0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getComboE[]",                    ZVARTYPEID_FLOAT,         GETTER,       MAPDATACOMBOED,             176,           0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setComboE[]",                    ZVARTYPEID_VOID,          SETTER,       MAPDATACOMBOED,             176,           0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getState[]",                     ZVARTYPEID_BOOL,          GETTER,       MAPDATASCREENSTATED,        32,            0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setState[]",                     ZVARTYPEID_VOID,          SETTER,       MAPDATASCREENSTATED,        32,            0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getExState[]",                   ZVARTYPEID_BOOL,          GETTER,       MAPDATAEXSTATED,            32,            0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setExState[]",                   ZVARTYPEID_VOID,          SETTER,       MAPDATAEXSTATED,            32,            0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSFlags[]",                     ZVARTYPEID_FLOAT,         GETTER,       MAPDATASCREENFLAGSD,        10,            0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "setFlags[]",                     ZVARTYPEID_VOID,          SETTER,       MAPDATASCREENFLAGSD,        10,            0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	This is read-only, but it was not previously blocked! -Z
	{ "getEFlags[]",                    ZVARTYPEID_FLOAT,         GETTER,       MAPDATASCREENEFLAGSD,       3,             0,                                    2,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	This is read-only, but it was not previously blocked! -Z
//	{ "setEFlags[]",                    ZVARTYPEID_VOID,          SETTER,       MAPDATASCREENEFLAGSD,       3,             0,                                    3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
//	Functions

	{ "GetFFCInitD",                    ZVARTYPEID_UNTYPED,       FUNCTION,     0,                          1,             FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetFFCInitD",                    ZVARTYPEID_VOID,          FUNCTION,     0,                          1,             0,                                    4,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "GetFFCInitA",                    ZVARTYPEID_FLOAT,         FUNCTION,     0,                          1,             FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetFFCInitA",                    ZVARTYPEID_FLOAT,         FUNCTION,     0,                          1,             0,                                    4,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getMap",                         ZVARTYPEID_FLOAT,         GETTER,       MAPDATAMAP,                 1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMap",                         ZVARTYPEID_FLOAT,         SETTER,       MAPDATAMAP,                 1,             0,                                    1,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScreen",                      ZVARTYPEID_FLOAT,         GETTER,       MAPDATASCREEN,              1,             0,                                    1,           { ZVARTYPEID_MAPDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScreen",                      ZVARTYPEID_FLOAT,         SETTER,       MAPDATASCREEN,              1,             0,                                    1,           { ZVARTYPEID_MAPDATA, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
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

