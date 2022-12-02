#include "SymbolDefs.h"

ItemSymbols ItemSymbols::singleton = ItemSymbols();

static AccessorTable itemTable[] =
{
//	  name,                     rettype,                  setorget,     var,                  numindex,      funcFlags,                            numParams,   params
	{ "getUID",                 ZTID_FLOAT,         GETTER,       ITEMSCRIPTUID,        1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Max",                   ZTID_FLOAT,         GETTER,       SPRITEMAXITEM,                1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Max",                   ZTID_VOID,          SETTER,       SPRITEMAXITEM,                1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getX",                   ZTID_FLOAT,         GETTER,       ITEMX,                1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setX",                   ZTID_VOID,          SETTER,       ITEMX,                1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getY",                   ZTID_FLOAT,         GETTER,       ITEMY,                1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setY",                   ZTID_VOID,          SETTER,       ITEMY,                1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getZ",                   ZTID_FLOAT,         GETTER,       ITEMZ,                1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setZ",                   ZTID_VOID,          SETTER,       ITEMZ,                1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getJump",                ZTID_FLOAT,         GETTER,       ITEMJUMP,             1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setJump",                ZTID_VOID,          SETTER,       ITEMJUMP,             1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawStyle",           ZTID_FLOAT,         GETTER,       ITEMDRAWTYPE,         1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawStyle",           ZTID_VOID,          SETTER,       ITEMDRAWTYPE,         1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getID",                  ZTID_FLOAT,         GETTER,       ITEMID,               1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setID",                  ZTID_VOID,          SETTER,       ITEMID,               1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getType",                ZTID_FLOAT,         GETTER,       ITEMFAMILY,           1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setType",                ZTID_VOID,          SETTER,       ITEMFAMILY,           1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFamily",              ZTID_FLOAT,         GETTER,       ITEMFAMILY,           1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFamily",              ZTID_VOID,          SETTER,       ITEMFAMILY,           1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLevel",               ZTID_FLOAT,         GETTER,       ITEMLEVEL,            1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLevel",               ZTID_VOID,          SETTER,       ITEMLEVEL,            1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTile",                ZTID_FLOAT,         GETTER,       ITEMTILE,             1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTile",                ZTID_VOID,          SETTER,       ITEMTILE,             1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScriptTile",          ZTID_FLOAT,         GETTER,       ITEMSCRIPTTILE,       1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScriptTile",          ZTID_VOID,          SETTER,       ITEMSCRIPTTILE,       1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScriptFlip",          ZTID_FLOAT,         GETTER,       ITEMSCRIPTFLIP,        1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScriptFlip",          ZTID_VOID,          SETTER,       ITEMSCRIPTFLIP,        1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getOriginalTile",        ZTID_FLOAT,         GETTER,       ITEMOTILE,            1,             0,                                    1,           {  ZTID_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                          } },
	{ "setOriginalTile",        ZTID_VOID,          SETTER,       ITEMOTILE,            1,             0,                                    2,           {  ZTID_ITEM,          ZTID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                          } },
	{ "getCSet",                ZTID_FLOAT,         GETTER,       ITEMCSET,             1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCSet",                ZTID_VOID,          SETTER,       ITEMCSET,             1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlashCSet",           ZTID_FLOAT,         GETTER,       ITEMFLASHCSET,        1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlashCSet",           ZTID_VOID,          SETTER,       ITEMFLASHCSET,        1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getNumFrames",           ZTID_FLOAT,         GETTER,       ITEMFRAMES,           1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNumFrames",           ZTID_VOID,          SETTER,       ITEMFRAMES,           1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFrame",               ZTID_FLOAT,         GETTER,       ITEMFRAME,            1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFrame",               ZTID_VOID,          SETTER,       ITEMFRAME,            1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getASpeed",              ZTID_FLOAT,         GETTER,       ITEMASPEED,           1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setASpeed",              ZTID_VOID,          SETTER,       ITEMASPEED,           1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDelay",               ZTID_FLOAT,         GETTER,       ITEMDELAY,            1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDelay",               ZTID_VOID,          SETTER,       ITEMDELAY,            1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlash",               ZTID_BOOL,          GETTER,       ITEMFLASH,            1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlash",               ZTID_VOID,          SETTER,       ITEMFLASH,            1,             0,                                    1,           { ZTID_ITEM, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlip",                ZTID_FLOAT,         GETTER,       ITEMFLIP,             1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlip",                ZTID_VOID,          SETTER,       ITEMFLIP,             1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getExtend",              ZTID_FLOAT,         GETTER,       ITEMEXTEND,           1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setExtend",              ZTID_VOID,          SETTER,       ITEMEXTEND,           1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitWidth",            ZTID_FLOAT,         GETTER,       ITEMHXSZ,             1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitWidth",            ZTID_VOID,          SETTER,       ITEMHXSZ,             1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitHeight",           ZTID_FLOAT,         GETTER,       ITEMHYSZ,             1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitHeight",           ZTID_VOID,          SETTER,       ITEMHYSZ,             1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitZHeight",          ZTID_FLOAT,         GETTER,       ITEMHZSZ,             1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitZHeight",          ZTID_VOID,          SETTER,       ITEMHZSZ,             1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTileWidth",           ZTID_FLOAT,         GETTER,       ITEMTXSZ,             1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWidth",           ZTID_VOID,          SETTER,       ITEMTXSZ,             1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTileHeight",          ZTID_FLOAT,         GETTER,       ITEMTYSZ,             1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileHeight",          ZTID_VOID,          SETTER,       ITEMTYSZ,             1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawXOffset",         ZTID_FLOAT,         GETTER,       ITEMXOFS,             1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawXOffset",         ZTID_VOID,          SETTER,       ITEMXOFS,             1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawYOffset",         ZTID_FLOAT,         GETTER,       ITEMYOFS,             1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawYOffset",         ZTID_VOID,          SETTER,       ITEMYOFS,             1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrawZOffset",         ZTID_FLOAT,         GETTER,       ITEMZOFS,             1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrawZOffset",         ZTID_VOID,          SETTER,       ITEMZOFS,             1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitXOffset",          ZTID_FLOAT,         GETTER,       ITEMHXOFS,            1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitXOffset",          ZTID_VOID,          SETTER,       ITEMHXOFS,            1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitYOffset",          ZTID_FLOAT,         GETTER,       ITEMHYOFS,            1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHitYOffset",          ZTID_VOID,          SETTER,       ITEMHYOFS,            1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPickup",              ZTID_FLOAT,         GETTER,       ITEMPICKUP,           1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPickup",              ZTID_VOID,          SETTER,       ITEMPICKUP,           1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "isValid",                ZTID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMisc[]",              ZTID_UNTYPED,       GETTER,       ITEMMISCD,            32,            0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMisc[]",              ZTID_VOID,          SETTER,       ITEMMISCD,            32,            0,                                    3,           { ZTID_ITEM, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getInitD[]",             ZTID_UNTYPED,       GETTER,       ITEMSPRITEINITD,      8,            0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInitD[]",             ZTID_VOID,          SETTER,       ITEMSPRITEINITD,      8,            0,                                    3,           { ZTID_ITEM, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAClock",              ZTID_FLOAT,         GETTER,       ITEMACLK,             1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAClock",              ZTID_VOID,          SETTER,       ITEMACLK,             1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getPickupString",        ZTID_FLOAT,         GETTER,       ITEMPSTRING,          1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPickupString",        ZTID_VOID,          SETTER,       ITEMPSTRING,          1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPickupStringFlags",   ZTID_FLOAT,         GETTER,       ITEMPSTRINGFLAGS,     1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPickupStringFlags",   ZTID_VOID,          SETTER,       ITEMPSTRINGFLAGS,     1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSizeFlags",           ZTID_FLOAT,         GETTER,       ITEMOVERRIDEFLAGS,    1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSizeFlags",           ZTID_VOID,          SETTER,       ITEMOVERRIDEFLAGS,    1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Explode",                ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRotation",            ZTID_FLOAT,         GETTER,       ITEMROTATION,         1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRotation",            ZTID_VOID,          SETTER,       ITEMROTATION,         1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScale",               ZTID_FLOAT,         GETTER,       ITEMSCALE,            1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScale",               ZTID_VOID,          SETTER,       ITEMSCALE,            1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScript",              ZTID_FLOAT,         GETTER,       ITEMSPRITESCRIPT,     1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScript",              ZTID_VOID,          SETTER,       ITEMSPRITESCRIPT,     1,             0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getGravity",              ZTID_BOOL,         GETTER,       ITEMGRAVITY,     1,             0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setGravity",              ZTID_VOID,          SETTER,       ITEMGRAVITY,     1,             0,                                    2,           { ZTID_ITEM, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFalling",             ZTID_FLOAT,         GETTER,       ITEMFALLCLK,          1,           0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFalling",             ZTID_VOID,          SETTER,       ITEMFALLCLK,          1,           0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFallCombo",           ZTID_FLOAT,         GETTER,       ITEMFALLCMB,          1,           0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFallCombo",           ZTID_VOID,          SETTER,       ITEMFALLCMB,          1,           0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMoveFlags[]",         ZTID_BOOL,          GETTER,       ITEMMOVEFLAGS,        2,           0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMoveFlags[]",         ZTID_VOID,          SETTER,       ITEMMOVEFLAGS,        2,           0,                                    3,           { ZTID_ITEM, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLightRadius",         ZTID_FLOAT,         GETTER,       ITEMGLOWRAD,          1,           0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLightRadius",         ZTID_VOID,          SETTER,       ITEMGLOWRAD,          1,           0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLightShape",          ZTID_FLOAT,         GETTER,       ITEMGLOWSHP,          1,           0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLightShape",          ZTID_VOID,          SETTER,       ITEMGLOWSHP,          1,           0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDir",                 ZTID_FLOAT,         GETTER,       ITEMDIR,              1,           0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDir",                 ZTID_VOID,          SETTER,       ITEMDIR,              1,           0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAnimation",           ZTID_FLOAT,         GETTER,       ITEMENGINEANIMATE,    1,           0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAnimation",           ZTID_VOID,          SETTER,       ITEMENGINEANIMATE,    1,           0,                                    2,           { ZTID_ITEM, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Remove",                 ZTID_VOID,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShadowSprite",        ZTID_FLOAT,         GETTER,       ITEMSHADOWSPR,        1,           0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShadowSprite",        ZTID_VOID,          SETTER,       ITEMSHADOWSPR,        1,           0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDroppedBy",           ZTID_FLOAT,         GETTER,       ITEMDROPPEDBY,        1,           0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDroppedBy",           ZTID_VOID,          SETTER,       ITEMDROPPEDBY,        1,           0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSwitchHooked",        ZTID_BOOL,          GETTER,       ITMSWHOOKED,          1,           0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSwitchHooked",        ZTID_VOID,          SETTER,       ITMSWHOOKED,          1,           0,                                    2,           { ZTID_ITEM, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Switch",                 ZTID_BOOL,          FUNCTION,     0,                    1,           FUNCFLAG_INLINE,                      2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrowning",            ZTID_FLOAT,         GETTER,       ITEMDROWNCLK,         1,           0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrowning",            ZTID_VOID,          SETTER,       ITEMDROWNCLK,         1,           0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDrownCombo",          ZTID_FLOAT,         GETTER,       ITEMDROWNCMB,         1,           0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDrownCombo",          ZTID_VOID,          SETTER,       ITEMDROWNCMB,         1,           0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFakeZ",               ZTID_FLOAT,         GETTER,       ITEMFAKEZ,            1,           0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFakeZ",               ZTID_VOID,          SETTER,       ITEMFAKEZ,            1,           0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFakeJump",            ZTID_FLOAT,         GETTER,       ITEMFAKEJUMP,         1,           0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFakeJump",            ZTID_VOID,          SETTER,       ITEMFAKEJUMP,         1,           0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShadowXOffset",       ZTID_FLOAT,         GETTER,       ITEMSHADOWXOFS,       1,           0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShadowXOffset",       ZTID_VOID,          SETTER,       ITEMSHADOWXOFS,       1,           0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShadowYOffset",       ZTID_FLOAT,         GETTER,       ITEMSHADOWYOFS,       1,           0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShadowYOffset",       ZTID_VOID,          SETTER,       ITEMSHADOWYOFS,       1,           0,                                    2,           { ZTID_ITEM, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getForceGrab",           ZTID_BOOL,          GETTER,       ITEMFORCEGRAB,        1,           0,                                    1,           { ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setForceGrab",           ZTID_VOID,          SETTER,       ITEMFORCEGRAB,        1,           0,                                    2,           { ZTID_ITEM, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
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

