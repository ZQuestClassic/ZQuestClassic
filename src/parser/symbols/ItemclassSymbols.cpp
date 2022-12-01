#include "SymbolDefs.h"

ItemclassSymbols ItemclassSymbols::singleton = ItemclassSymbols();

static AccessorTable itemclassTable[] =
{
//	name,                           rettype,                  setorget,     var,                  numindex,      funcFlags,                            numParams,   params
	{ "getFamily",                  ZTID_FLOAT,         GETTER,       ITEMCLASSFAMILY,      1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFamily",                  ZTID_VOID,          SETTER,       ITEMCLASSFAMILY,      1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getType",                    ZTID_FLOAT,         GETTER,       ITEMCLASSFAMILY,      1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setType",                    ZTID_VOID,          SETTER,       ITEMCLASSFAMILY,      1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLevel",                   ZTID_FLOAT,         GETTER,       ITEMCLASSFAMTYPE,     1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLevel",                   ZTID_VOID,          SETTER,       ITEMCLASSFAMTYPE,     1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAmount",                  ZTID_FLOAT,         GETTER,       ITEMCLASSAMOUNT,      1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAmount",                  ZTID_VOID,          SETTER,       ITEMCLASSAMOUNT,      1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMax",                     ZTID_FLOAT,         GETTER,       ITEMCLASSMAX,         1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMax",                     ZTID_VOID,          SETTER,       ITEMCLASSMAX,         1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMaxIncrement",            ZTID_FLOAT,         GETTER,       ITEMCLASSSETMAX,      1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMaxIncrement",            ZTID_VOID,          SETTER,       ITEMCLASSSETMAX,      1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getKeep",                    ZTID_BOOL,          GETTER,       ITEMCLASSSETGAME,     1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setKeep",                    ZTID_VOID,          SETTER,       ITEMCLASSSETGAME,     1,             0,                                    1,           { ZTID_ITEMCLASS, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getEquipmentItem",           ZTID_BOOL,          GETTER,       ITEMCLASSSETGAME,     1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setEquipmentItem",           ZTID_VOID,          SETTER,       ITEMCLASSSETGAME,     1,             0,                                    1,           { ZTID_ITEMCLASS, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCounter",                 ZTID_FLOAT,         GETTER,       ITEMCLASSCOUNTER,     1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCounter",                 ZTID_VOID,          SETTER,       ITEMCLASSCOUNTER,     1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getUseSound",                ZTID_FLOAT,         GETTER,       ITEMCLASSUSESOUND,    1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setUseSound",                ZTID_VOID,          SETTER,       ITEMCLASSUSESOUND,    1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getUseSound2",               ZTID_FLOAT,         GETTER,       IDATAUSESOUND2,       1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setUseSound2",               ZTID_VOID,          SETTER,       IDATAUSESOUND2,       1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getID",                      ZTID_FLOAT,         GETTER,       ITEMCLASSID,          1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getPower",                   ZTID_FLOAT,         GETTER,       ITEMCLASSPOWER,       1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setPower",                   ZTID_VOID,          SETTER,       ITEMCLASSPOWER,       1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDamage",                  ZTID_FLOAT,         GETTER,       ITEMCLASSPOWER,       1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setDamage",                  ZTID_VOID,          SETTER,       ITEMCLASSPOWER,       1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getInitD[]",                 ZTID_UNTYPED,       GETTER,       ITEMCLASSINITDD,      2,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInitD[]",                 ZTID_VOID,          SETTER,       ITEMCLASSINITDD,      2,             0,                                    3,           { ZTID_ITEMCLASS, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetName",                    ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "RunScript",                  ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getModifier",                ZTID_FLOAT,         GETTER,       IDATALTM,             1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setModifier",                ZTID_VOID,          SETTER,       IDATALTM,             1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTileMod",                 ZTID_FLOAT,         GETTER,       IDATALTM,             1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setTileMod",                 ZTID_VOID,          SETTER,       IDATALTM,             1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScript",                  ZTID_FLOAT,         GETTER,       IDATASCRIPT,          1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setScript",                  ZTID_VOID,          SETTER,       IDATASCRIPT,          1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPScript",                 ZTID_FLOAT,         GETTER,       IDATAPSCRIPT,         1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setPScript",                 ZTID_VOID,          SETTER,       IDATAPSCRIPT,         1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPString",                 ZTID_FLOAT,         GETTER,       IDATAPSTRING,         1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setPString",                 ZTID_VOID,          SETTER,       IDATAPSTRING,         1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPickupString",                 ZTID_FLOAT,         GETTER,       IDATAPSTRING,         1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setPickupString",                 ZTID_VOID,          SETTER,       IDATAPSTRING,         1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPickupStringFlags",                 ZTID_FLOAT,         GETTER,       IDATAPFLAGS,         1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setPickupStringFlags",                 ZTID_VOID,          SETTER,       IDATAPFLAGS,         1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMagicCost",               ZTID_FLOAT,         GETTER,       IDATAMAGCOST,         1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setMagicCost",               ZTID_VOID,          SETTER,       IDATAMAGCOST,         1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCost",                    ZTID_FLOAT,         GETTER,       IDATAMAGCOST,         1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setCost",                    ZTID_VOID,          SETTER,       IDATAMAGCOST,         1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCost2",                   ZTID_FLOAT,         GETTER,       IDATACOST2,           1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setCost2",                   ZTID_VOID,          SETTER,       IDATACOST2,           1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCostCounter",             ZTID_FLOAT,         GETTER,       IDATACOSTCOUNTER,     1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setCostCounter",             ZTID_VOID,          SETTER,       IDATACOSTCOUNTER,     1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCostCounter2",            ZTID_FLOAT,         GETTER,       IDATACOSTCOUNTER2,    1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setCostCounter2",            ZTID_VOID,          SETTER,       IDATACOSTCOUNTER2,    1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMinHearts",               ZTID_FLOAT,         GETTER,       IDATAMINHEARTS,       1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setMinHearts",               ZTID_VOID,          SETTER,       IDATAMINHEARTS,       1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTile",                    ZTID_FLOAT,         GETTER,       IDATATILE,            1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setTile",                    ZTID_VOID,          SETTER,       IDATATILE,            1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlash",                   ZTID_FLOAT,         GETTER,       IDATAMISC,            1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setFlash",                   ZTID_VOID,          SETTER,       IDATAMISC,            1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCSet",                    ZTID_FLOAT,         GETTER,       IDATACSET,            1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setCSet",                    ZTID_VOID,          SETTER,       IDATACSET,            1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "getFrame",                   ZTID_FLOAT,         GETTER,       IDATAFRAME,           1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
//	{ "setFrame",                   ZTID_VOID,          SETTER,       IDATAFRAME,           1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAFrames",                 ZTID_FLOAT,         GETTER,       IDATAFRAMES,          1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setAFrames",                 ZTID_VOID,          SETTER,       IDATAFRAMES,          1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getASpeed",                  ZTID_FLOAT,         GETTER,       IDATAASPEED,          1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setASpeed",                  ZTID_VOID,          SETTER,       IDATAASPEED,          1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDelay",                   ZTID_FLOAT,         GETTER,       IDATADELAY,           1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setDelay",                   ZTID_VOID,          SETTER,       IDATADELAY,           1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCombine",                 ZTID_BOOL,          GETTER,       IDATACOMBINE,         1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCombine",                 ZTID_VOID,          SETTER,       IDATACOMBINE,         1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDowngrade",               ZTID_BOOL,          GETTER,       IDATADOWNGRADE,       1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDowngrade",               ZTID_VOID,          SETTER,       IDATADOWNGRADE,       1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getKeepOld",                 ZTID_BOOL,          GETTER,       IDATAKEEPOLD,         1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setKeepOld",                 ZTID_VOID,          SETTER,       IDATAKEEPOLD,         1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRupeeCost",               ZTID_BOOL,          GETTER,       IDATARUPEECOST,       1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRupeeCost",               ZTID_VOID,          SETTER,       IDATARUPEECOST,       1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getEdible",                  ZTID_BOOL,          GETTER,       IDATAEDIBLE,          1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setEdible",                  ZTID_VOID,          SETTER,       IDATAEDIBLE,          1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getUnused",                  ZTID_BOOL,          GETTER,       IDATAFLAGUNUSED,      1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setUnused",                  ZTID_VOID,          SETTER,       IDATAFLAGUNUSED,      1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getGainLower",               ZTID_BOOL,          GETTER,       IDATAGAINLOWER,       1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setGainLower",               ZTID_VOID,          SETTER,       IDATAGAINLOWER,       1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAttributes[]",            ZTID_UNTYPED,       GETTER,       IDATAATTRIB,          15,            0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAttributes[]",            ZTID_VOID,          SETTER,       IDATAATTRIB,          15,            0,                                    3,           { ZTID_ITEMCLASS, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMisc[]",                  ZTID_UNTYPED,       GETTER,       IDATAATTRIB,          10,            0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMisc[]",                  ZTID_VOID,          SETTER,       IDATAATTRIB,          10,            0,                                    3,           { ZTID_ITEMCLASS, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlags[]",                 ZTID_BOOL,          GETTER,       IDATAFLAGS,           16,            0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlags[]",                 ZTID_VOID,          SETTER,       IDATAFLAGS,           16,            0,                                    3,           { ZTID_ITEMCLASS, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSprites[]",               ZTID_FLOAT,         GETTER,       IDATASPRITE,          10,            0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSprites[]",               ZTID_VOID,          SETTER,       IDATASPRITE,          10,            0,                                    3,           { ZTID_ITEMCLASS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWeapon",                  ZTID_FLOAT,         GETTER,       IDATAUSEWPN,          1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setWeapon",                  ZTID_VOID,          SETTER,       IDATAUSEWPN,          1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDefense",                 ZTID_FLOAT,         GETTER,       IDATAUSEDEF,          1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setDefense",                 ZTID_VOID,          SETTER,       IDATAUSEDEF,          1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDefence",                 ZTID_FLOAT,         GETTER,       IDATAUSEDEF,          1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setDefence",                 ZTID_VOID,          SETTER,       IDATAUSEDEF,          1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRange",                   ZTID_FLOAT,         GETTER,       IDATAWRANGE,          1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setRange",                   ZTID_VOID,          SETTER,       IDATAWRANGE,          1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getDuration",                ZTID_FLOAT,         GETTER,       IDATADURATION,        1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setDuration",                ZTID_VOID,          SETTER,       IDATADURATION,        1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	
	
//	{ "getMovement[]",              ZTID_FLOAT,         GETTER,       IDATAUSEMVT,          ITEM_MOVEMENT_PATTERNS,     { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "setMovement[]",              ZTID_VOID,          SETTER,       IDATAUSEMVT,          ITEM_MOVEMENT_PATTERNS,     { ZTID_ITEMCLASS, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWeaponInitD[]",           ZTID_UNTYPED,       GETTER,       IDATAWPNINITD,        8,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWeaponInitD[]",           ZTID_VOID,          SETTER,       IDATAWPNINITD,        8,             0,                                    3,           { ZTID_ITEMCLASS, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWeaponMisc[]",            ZTID_UNTYPED,       GETTER,       IDATAMISCD,           32,            0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWeaponMisc[]",            ZTID_VOID,          SETTER,       IDATAMISCD,           32,            0,                                    3,           { ZTID_ITEMCLASS, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getDuplicates",              ZTID_FLOAT,         GETTER,       IDATADUPLICATES,      1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setDuplicates",              ZTID_VOID,          SETTER,       IDATADUPLICATES,      1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getDrawLayer",               ZTID_FLOAT,         GETTER,       IDATADRAWLAYER,       1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setDrawLayer",               ZTID_VOID,          SETTER,       IDATADRAWLAYER,       1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getCollectFlags",            ZTID_FLOAT,         GETTER,       IDATACOLLECTFLAGS,    1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setCollectFlags",            ZTID_VOID,          SETTER,       IDATACOLLECTFLAGS,    1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWeaponScript",            ZTID_FLOAT,         GETTER,       IDATAWEAPONSCRIPT,    1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setWeaponScript",            ZTID_VOID,          SETTER,       IDATAWEAPONSCRIPT,    1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWeaponHitXOffset",        ZTID_FLOAT,         GETTER,       IDATAWEAPHXOFS,       1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setWeaponHitXOffset",        ZTID_VOID,          SETTER,       IDATAWEAPHXOFS,       1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWeaponHitYOffset",        ZTID_FLOAT,         GETTER,       IDATAWEAPHYOFS,       1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setWeaponHitYOffset",        ZTID_VOID,          SETTER,       IDATAWEAPHYOFS,       1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWeaponHitWidth",          ZTID_FLOAT,         GETTER,       IDATAWEAPHXSZ,        1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setWeaponHitWidth",          ZTID_VOID,          SETTER,       IDATAWEAPHXSZ,        1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWeaponHitHeight",         ZTID_FLOAT,         GETTER,       IDATAWEAPHYSZ,        1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setWeaponHitHeight",         ZTID_VOID,          SETTER,       IDATAWEAPHYSZ,        1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWeaponHitZHeight",        ZTID_FLOAT,         GETTER,       IDATAWEAPHZSZ,        1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setWeaponHitZHeight",        ZTID_VOID,          SETTER,       IDATAWEAPHZSZ,        1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWeaponDrawXOffset",       ZTID_FLOAT,         GETTER,       IDATAWEAPXOFS,        1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setWeaponDrawXOffset",       ZTID_VOID,          SETTER,       IDATAWEAPXOFS,        1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWeaponDrawYOffset",       ZTID_FLOAT,         GETTER,       IDATAWEAPYOFS,        1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setWeaponDrawYOffset",       ZTID_VOID,          SETTER,       IDATAWEAPYOFS,        1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWeaponDrawZOffset",       ZTID_FLOAT,         GETTER,       IDATAWEAPZOFS,        1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setWeaponDrawZOffset",       ZTID_VOID,          SETTER,       IDATAWEAPZOFS,        1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMagicCostTimer",          ZTID_FLOAT,         GETTER,       IDATAMAGICTIMER,      1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setMagicCostTimer",          ZTID_VOID,          SETTER,       IDATAMAGICTIMER,      1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCostTimer",               ZTID_FLOAT,         GETTER,       IDATAMAGICTIMER,      1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setCostTimer",               ZTID_VOID,          SETTER,       IDATAMAGICTIMER,      1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCostTimer2",              ZTID_FLOAT,         GETTER,       IDATAMAGICTIMER2,      1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setCostTimer2",              ZTID_VOID,          SETTER,       IDATAMAGICTIMER2,      1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	
	
	{ "getHitXOffset",              ZTID_FLOAT,         GETTER,       IDATAHXOFS,           1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setHitXOffset",              ZTID_VOID,          SETTER,       IDATAHXOFS,           1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitYOffset",              ZTID_FLOAT,         GETTER,       IDATAHYOFS,           1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setHitYOffset",              ZTID_VOID,          SETTER,       IDATAHYOFS,           1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getDrawXOffset",             ZTID_FLOAT,         GETTER,       IDATADXOFS,           1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setDrawXOffset",             ZTID_VOID,          SETTER,       IDATADXOFS,           1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getDrawYOffset",             ZTID_FLOAT,         GETTER,       IDATADYOFS,           1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setDrawYOffset",             ZTID_VOID,          SETTER,       IDATADYOFS,           1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getHitWidth",                ZTID_FLOAT,         GETTER,       IDATAHXSZ,            1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setHitWidth",                ZTID_VOID,          SETTER,       IDATAHXSZ,            1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitHeight",               ZTID_FLOAT,         GETTER,       IDATAHYSZ,            1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setHitHeight",               ZTID_VOID,          SETTER,       IDATAHYSZ,            1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitZHeight",              ZTID_FLOAT,         GETTER,       IDATAHZSZ,            1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setHitZHeight",              ZTID_VOID,          SETTER,       IDATAHZSZ,            1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getTileWidth",               ZTID_FLOAT,         GETTER,       IDATATILEW,           1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setTileWidth",               ZTID_VOID,          SETTER,       IDATATILEW,           1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTileHeight",              ZTID_FLOAT,         GETTER,       IDATATILEH,           1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setTileHeight",              ZTID_VOID,          SETTER,       IDATATILEH,           1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getPickup",                  ZTID_FLOAT,         GETTER,       IDATAPICKUP,          1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setPickup",                  ZTID_VOID,          SETTER,       IDATAPICKUP,          1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getOverrideFlags",           ZTID_FLOAT,         GETTER,       IDATAOVERRIDEFL,      1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setOverrideFlags",           ZTID_VOID,          SETTER,       IDATAOVERRIDEFL,      1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	Weapon-forwarded Variables
	
	{ "getWeaponTileWidth",         ZTID_FLOAT,         GETTER,       IDATATILEWWEAP,       1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setWeaponTileWidth",         ZTID_VOID,          SETTER,       IDATATILEWWEAP,       1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWeaponTileHeight",        ZTID_FLOAT,         GETTER,       IDATATILEHWEAP,       1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setWeaponTileHeight",        ZTID_VOID,          SETTER,       IDATATILEHWEAP,       1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWeaponOverrideFlags",     ZTID_FLOAT,         GETTER,       IDATAOVERRIDEFLWEAP,  1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setWeaponOverrideFlags",     ZTID_VOID,          SETTER,       IDATAOVERRIDEFLWEAP,  1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getValidate",                ZTID_BOOL,          GETTER,       IDATAVALIDATE,        1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setValidate",                ZTID_VOID,          SETTER,       IDATAVALIDATE,        1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getValidate2",               ZTID_BOOL,          GETTER,       IDATAVALIDATE2,       1,             0,                                    1,           { ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setValidate2",               ZTID_VOID,          SETTER,       IDATAVALIDATE2,       1,             0,                                    2,           { ZTID_ITEMCLASS, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "",                           -1,                       -1,           -1,                   -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

ItemclassSymbols::ItemclassSymbols()
{
    table = itemclassTable;
    refVar = REFITEMCLASS;
}

void ItemclassSymbols::generateCode()
{
    //void GetName(itemclass, int32_t)
    {
		Function* function = getFunction("GetName", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGetItemName(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //void RunScript(itemclass)
    {
	Function* function = getFunction("RunScript", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the param
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new ORunItemScript(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    
}

