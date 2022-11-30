#include "SymbolDefs.h"

ItemSymbols ItemSymbols::singleton = ItemSymbols();

static AccessorTable itemTable[] =
{
//	  name,                     rettype,                  setorget,     var,                  numindex,      funcFlags,                            numParams,   params
	{ "getUID",                 ZVARTYPEID_FLOAT,         GETTER,       ITEMSCRIPTUID,        1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Max",                   ZVARTYPEID_FLOAT,         GETTER,       SPRITEMAXITEM,                1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Max",                   ZVARTYPEID_VOID,          SETTER,       SPRITEMAXITEM,                1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getX",                   ZVARTYPEID_FLOAT,         GETTER,       ITEMX,                1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setX",                   ZVARTYPEID_VOID,          SETTER,       ITEMX,                1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getY",                   ZVARTYPEID_FLOAT,         GETTER,       ITEMY,                1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setY",                   ZVARTYPEID_VOID,          SETTER,       ITEMY,                1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getZ",                   ZVARTYPEID_FLOAT,         GETTER,       ITEMZ,                1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setZ",                   ZVARTYPEID_VOID,          SETTER,       ITEMZ,                1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getJump",                ZVARTYPEID_FLOAT,         GETTER,       ITEMJUMP,             1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setJump",                ZVARTYPEID_VOID,          SETTER,       ITEMJUMP,             1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawStyle",           ZVARTYPEID_FLOAT,         GETTER,       ITEMDRAWTYPE,         1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawStyle",           ZVARTYPEID_VOID,          SETTER,       ITEMDRAWTYPE,         1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getID",                  ZVARTYPEID_FLOAT,         GETTER,       ITEMID,               1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setID",                  ZVARTYPEID_VOID,          SETTER,       ITEMID,               1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getType",                ZVARTYPEID_FLOAT,         GETTER,       ITEMFAMILY,           1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setType",                ZVARTYPEID_VOID,          SETTER,       ITEMFAMILY,           1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFamily",              ZVARTYPEID_FLOAT,         GETTER,       ITEMFAMILY,           1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFamily",              ZVARTYPEID_VOID,          SETTER,       ITEMFAMILY,           1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLevel",               ZVARTYPEID_FLOAT,         GETTER,       ITEMLEVEL,            1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLevel",               ZVARTYPEID_VOID,          SETTER,       ITEMLEVEL,            1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTile",                ZVARTYPEID_FLOAT,         GETTER,       ITEMTILE,             1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTile",                ZVARTYPEID_VOID,          SETTER,       ITEMTILE,             1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScriptTile",          ZVARTYPEID_FLOAT,         GETTER,       ITEMSCRIPTTILE,       1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScriptTile",          ZVARTYPEID_VOID,          SETTER,       ITEMSCRIPTTILE,       1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScriptFlip",          ZVARTYPEID_FLOAT,         GETTER,       ITEMSCRIPTFLIP,        1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScriptFlip",          ZVARTYPEID_VOID,          SETTER,       ITEMSCRIPTFLIP,        1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getOriginalTile",        ZVARTYPEID_FLOAT,         GETTER,       ITEMOTILE,            1,             0,                                    1,           {  ZVARTYPEID_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                          } },
	{ "setOriginalTile",        ZVARTYPEID_VOID,          SETTER,       ITEMOTILE,            1,             0,                                    2,           {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                          } },
	{ "getCSet",                ZVARTYPEID_FLOAT,         GETTER,       ITEMCSET,             1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCSet",                ZVARTYPEID_VOID,          SETTER,       ITEMCSET,             1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlashCSet",           ZVARTYPEID_FLOAT,         GETTER,       ITEMFLASHCSET,        1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlashCSet",           ZVARTYPEID_VOID,          SETTER,       ITEMFLASHCSET,        1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getNumFrames",           ZVARTYPEID_FLOAT,         GETTER,       ITEMFRAMES,           1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNumFrames",           ZVARTYPEID_VOID,          SETTER,       ITEMFRAMES,           1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFrame",               ZVARTYPEID_FLOAT,         GETTER,       ITEMFRAME,            1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFrame",               ZVARTYPEID_VOID,          SETTER,       ITEMFRAME,            1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getASpeed",              ZVARTYPEID_FLOAT,         GETTER,       ITEMASPEED,           1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setASpeed",              ZVARTYPEID_VOID,          SETTER,       ITEMASPEED,           1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDelay",               ZVARTYPEID_FLOAT,         GETTER,       ITEMDELAY,            1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDelay",               ZVARTYPEID_VOID,          SETTER,       ITEMDELAY,            1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlash",               ZVARTYPEID_BOOL,          GETTER,       ITEMFLASH,            1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlash",               ZVARTYPEID_VOID,          SETTER,       ITEMFLASH,            1,             0,                                    1,           { ZVARTYPEID_ITEM, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlip",                ZVARTYPEID_FLOAT,         GETTER,       ITEMFLIP,             1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlip",                ZVARTYPEID_VOID,          SETTER,       ITEMFLIP,             1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getExtend",              ZVARTYPEID_FLOAT,         GETTER,       ITEMEXTEND,           1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setExtend",              ZVARTYPEID_VOID,          SETTER,       ITEMEXTEND,           1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitWidth",            ZVARTYPEID_FLOAT,         GETTER,       ITEMHXSZ,             1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitWidth",            ZVARTYPEID_VOID,          SETTER,       ITEMHXSZ,             1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitHeight",           ZVARTYPEID_FLOAT,         GETTER,       ITEMHYSZ,             1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitHeight",           ZVARTYPEID_VOID,          SETTER,       ITEMHYSZ,             1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitZHeight",          ZVARTYPEID_FLOAT,         GETTER,       ITEMHZSZ,             1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitZHeight",          ZVARTYPEID_VOID,          SETTER,       ITEMHZSZ,             1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTileWidth",           ZVARTYPEID_FLOAT,         GETTER,       ITEMTXSZ,             1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWidth",           ZVARTYPEID_VOID,          SETTER,       ITEMTXSZ,             1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTileHeight",          ZVARTYPEID_FLOAT,         GETTER,       ITEMTYSZ,             1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileHeight",          ZVARTYPEID_VOID,          SETTER,       ITEMTYSZ,             1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawXOffset",         ZVARTYPEID_FLOAT,         GETTER,       ITEMXOFS,             1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawXOffset",         ZVARTYPEID_VOID,          SETTER,       ITEMXOFS,             1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawYOffset",         ZVARTYPEID_FLOAT,         GETTER,       ITEMYOFS,             1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawYOffset",         ZVARTYPEID_VOID,          SETTER,       ITEMYOFS,             1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawZOffset",         ZVARTYPEID_FLOAT,         GETTER,       ITEMZOFS,             1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawZOffset",         ZVARTYPEID_VOID,          SETTER,       ITEMZOFS,             1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitXOffset",          ZVARTYPEID_FLOAT,         GETTER,       ITEMHXOFS,            1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitXOffset",          ZVARTYPEID_VOID,          SETTER,       ITEMHXOFS,            1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitYOffset",          ZVARTYPEID_FLOAT,         GETTER,       ITEMHYOFS,            1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitYOffset",          ZVARTYPEID_VOID,          SETTER,       ITEMHYOFS,            1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPickup",              ZVARTYPEID_FLOAT,         GETTER,       ITEMPICKUP,           1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPickup",              ZVARTYPEID_VOID,          SETTER,       ITEMPICKUP,           1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "isValid",                ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMisc[]",              ZVARTYPEID_UNTYPED,       GETTER,       ITEMMISCD,            32,            0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMisc[]",              ZVARTYPEID_VOID,          SETTER,       ITEMMISCD,            32,            0,                                    3,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getInitD[]",             ZVARTYPEID_UNTYPED,       GETTER,       ITEMSPRITEINITD,      8,            0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInitD[]",             ZVARTYPEID_VOID,          SETTER,       ITEMSPRITEINITD,      8,            0,                                    3,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAClock",              ZVARTYPEID_FLOAT,         GETTER,       ITEMACLK,             1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAClock",              ZVARTYPEID_VOID,          SETTER,       ITEMACLK,             1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getPickupString",        ZVARTYPEID_FLOAT,         GETTER,       ITEMPSTRING,          1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPickupString",        ZVARTYPEID_VOID,          SETTER,       ITEMPSTRING,          1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPickupStringFlags",   ZVARTYPEID_FLOAT,         GETTER,       ITEMPSTRINGFLAGS,     1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPickupStringFlags",   ZVARTYPEID_VOID,          SETTER,       ITEMPSTRINGFLAGS,     1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSizeFlags",           ZVARTYPEID_FLOAT,         GETTER,       ITEMOVERRIDEFLAGS,    1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSizeFlags",           ZVARTYPEID_VOID,          SETTER,       ITEMOVERRIDEFLAGS,    1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Explode",                ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRotation",            ZVARTYPEID_FLOAT,         GETTER,       ITEMROTATION,         1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRotation",            ZVARTYPEID_VOID,          SETTER,       ITEMROTATION,         1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScale",               ZVARTYPEID_FLOAT,         GETTER,       ITEMSCALE,            1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScale",               ZVARTYPEID_VOID,          SETTER,       ITEMSCALE,            1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScript",              ZVARTYPEID_FLOAT,         GETTER,       ITEMSPRITESCRIPT,     1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScript",              ZVARTYPEID_VOID,          SETTER,       ITEMSPRITESCRIPT,     1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getGravity",              ZVARTYPEID_BOOL,         GETTER,       ITEMGRAVITY,     1,             0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setGravity",              ZVARTYPEID_VOID,          SETTER,       ITEMGRAVITY,     1,             0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFalling",             ZVARTYPEID_FLOAT,         GETTER,       ITEMFALLCLK,          1,           0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFalling",             ZVARTYPEID_VOID,          SETTER,       ITEMFALLCLK,          1,           0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFallCombo",           ZVARTYPEID_FLOAT,         GETTER,       ITEMFALLCMB,          1,           0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFallCombo",           ZVARTYPEID_VOID,          SETTER,       ITEMFALLCMB,          1,           0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMoveFlags[]",         ZVARTYPEID_BOOL,          GETTER,       ITEMMOVEFLAGS,        2,           0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMoveFlags[]",         ZVARTYPEID_VOID,          SETTER,       ITEMMOVEFLAGS,        2,           0,                                    3,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLightRadius",         ZVARTYPEID_FLOAT,         GETTER,       ITEMGLOWRAD,          1,           0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLightRadius",         ZVARTYPEID_VOID,          SETTER,       ITEMGLOWRAD,          1,           0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLightShape",          ZVARTYPEID_FLOAT,         GETTER,       ITEMGLOWSHP,          1,           0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLightShape",          ZVARTYPEID_VOID,          SETTER,       ITEMGLOWSHP,          1,           0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDir",                 ZVARTYPEID_FLOAT,         GETTER,       ITEMDIR,              1,           0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDir",                 ZVARTYPEID_VOID,          SETTER,       ITEMDIR,              1,           0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAnimation",           ZVARTYPEID_FLOAT,         GETTER,       ITEMENGINEANIMATE,    1,           0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAnimation",           ZVARTYPEID_VOID,          SETTER,       ITEMENGINEANIMATE,    1,           0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Remove",                 ZVARTYPEID_VOID,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShadowSprite",        ZVARTYPEID_FLOAT,         GETTER,       ITEMSHADOWSPR,        1,           0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShadowSprite",        ZVARTYPEID_VOID,          SETTER,       ITEMSHADOWSPR,        1,           0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDroppedBy",           ZVARTYPEID_FLOAT,         GETTER,       ITEMDROPPEDBY,        1,           0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDroppedBy",           ZVARTYPEID_VOID,          SETTER,       ITEMDROPPEDBY,        1,           0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSwitchHooked",        ZVARTYPEID_BOOL,          GETTER,       ITMSWHOOKED,          1,           0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSwitchHooked",        ZVARTYPEID_VOID,          SETTER,       ITMSWHOOKED,          1,           0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Switch",                 ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrowning",            ZVARTYPEID_FLOAT,         GETTER,       ITEMDROWNCLK,         1,           0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrowning",            ZVARTYPEID_VOID,          SETTER,       ITEMDROWNCLK,         1,           0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrownCombo",          ZVARTYPEID_FLOAT,         GETTER,       ITEMDROWNCMB,         1,           0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrownCombo",          ZVARTYPEID_VOID,          SETTER,       ITEMDROWNCMB,         1,           0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFakeZ",               ZVARTYPEID_FLOAT,         GETTER,       ITEMFAKEZ,            1,           0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFakeZ",               ZVARTYPEID_VOID,          SETTER,       ITEMFAKEZ,            1,           0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFakeJump",            ZVARTYPEID_FLOAT,         GETTER,       ITEMFAKEJUMP,         1,           0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFakeJump",            ZVARTYPEID_VOID,          SETTER,       ITEMFAKEJUMP,         1,           0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShadowXOffset",       ZVARTYPEID_FLOAT,         GETTER,       ITEMSHADOWXOFS,       1,           0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShadowXOffset",       ZVARTYPEID_VOID,          SETTER,       ITEMSHADOWXOFS,       1,           0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShadowYOffset",       ZVARTYPEID_FLOAT,         GETTER,       ITEMSHADOWYOFS,       1,           0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShadowYOffset",       ZVARTYPEID_VOID,          SETTER,       ITEMSHADOWYOFS,       1,           0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getForceGrab",           ZVARTYPEID_BOOL,          GETTER,       ITEMFORCEGRAB,        1,           0,                                    1,           { ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setForceGrab",           ZVARTYPEID_VOID,          SETTER,       ITEMFORCEGRAB,        1,           0,                                    2,           { ZVARTYPEID_ITEM, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "",                       -1,                       -1,           -1,                   -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

ItemSymbols::ItemSymbols()
{
    table = itemTable;
    refVar = REFITEM;
}

void ItemSymbols::generateCode()
{
    //bool isValid(item)
    {
	    Function* function = getFunction("isValid", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the pointer
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //Check validity
        addOpcode2 (code, new OIsValidItem(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //void Explode(ITEM, int32_t)
    {
	    Function* function = getFunction("Explode", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OItemExplodeRegister(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
	//void Remove(itemsprite)
	{
		Function* function = getFunction("Remove", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		POPREF();
		LABELBACK(label);
		//Break shield
		addOpcode2 (code, new OItemRemove());
		RETURN();
		function->giveCode(code);
	}
	//bool Switch(itemsprite, int)
    {
        Function* function = getFunction("Switch", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSwitchItem(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
}

