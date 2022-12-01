#include "SymbolDefs.h"

ItemclassSymbols ItemclassSymbols::singleton = ItemclassSymbols();

static AccessorTable itemclassTable[] =
{
//	name,                           rettype,                  setorget,     var,                  numindex,      funcFlags,                            numParams,   params
	{ "getFamily",                  ZVARTYPEID_FLOAT,         GETTER,       ITEMCLASSFAMILY,      1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFamily",                  ZVARTYPEID_VOID,          SETTER,       ITEMCLASSFAMILY,      1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getType",                    ZVARTYPEID_FLOAT,         GETTER,       ITEMCLASSFAMILY,      1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setType",                    ZVARTYPEID_VOID,          SETTER,       ITEMCLASSFAMILY,      1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLevel",                   ZVARTYPEID_FLOAT,         GETTER,       ITEMCLASSFAMTYPE,     1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLevel",                   ZVARTYPEID_VOID,          SETTER,       ITEMCLASSFAMTYPE,     1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAmount",                  ZVARTYPEID_FLOAT,         GETTER,       ITEMCLASSAMOUNT,      1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAmount",                  ZVARTYPEID_VOID,          SETTER,       ITEMCLASSAMOUNT,      1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMax",                     ZVARTYPEID_FLOAT,         GETTER,       ITEMCLASSMAX,         1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMax",                     ZVARTYPEID_VOID,          SETTER,       ITEMCLASSMAX,         1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMaxIncrement",            ZVARTYPEID_FLOAT,         GETTER,       ITEMCLASSSETMAX,      1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMaxIncrement",            ZVARTYPEID_VOID,          SETTER,       ITEMCLASSSETMAX,      1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getKeep",                    ZVARTYPEID_BOOL,          GETTER,       ITEMCLASSSETGAME,     1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setKeep",                    ZVARTYPEID_VOID,          SETTER,       ITEMCLASSSETGAME,     1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getEquipmentItem",           ZVARTYPEID_BOOL,          GETTER,       ITEMCLASSSETGAME,     1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setEquipmentItem",           ZVARTYPEID_VOID,          SETTER,       ITEMCLASSSETGAME,     1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCounter",                 ZVARTYPEID_FLOAT,         GETTER,       ITEMCLASSCOUNTER,     1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCounter",                 ZVARTYPEID_VOID,          SETTER,       ITEMCLASSCOUNTER,     1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getUseSound",                ZVARTYPEID_FLOAT,         GETTER,       ITEMCLASSUSESOUND,    1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setUseSound",                ZVARTYPEID_VOID,          SETTER,       ITEMCLASSUSESOUND,    1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getUseSound2",               ZVARTYPEID_FLOAT,         GETTER,       IDATAUSESOUND2,       1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setUseSound2",               ZVARTYPEID_VOID,          SETTER,       IDATAUSESOUND2,       1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getID",                      ZVARTYPEID_FLOAT,         GETTER,       ITEMCLASSID,          1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getPower",                   ZVARTYPEID_FLOAT,         GETTER,       ITEMCLASSPOWER,       1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setPower",                   ZVARTYPEID_VOID,          SETTER,       ITEMCLASSPOWER,       1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDamage",                  ZVARTYPEID_FLOAT,         GETTER,       ITEMCLASSPOWER,       1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setDamage",                  ZVARTYPEID_VOID,          SETTER,       ITEMCLASSPOWER,       1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getInitD[]",                 ZVARTYPEID_UNTYPED,       GETTER,       ITEMCLASSINITDD,      2,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInitD[]",                 ZVARTYPEID_VOID,          SETTER,       ITEMCLASSINITDD,      2,             0,                                    3,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetName",                    ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "RunScript",                  ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getModifier",                ZVARTYPEID_FLOAT,         GETTER,       IDATALTM,             1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setModifier",                ZVARTYPEID_VOID,          SETTER,       IDATALTM,             1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTileMod",                 ZVARTYPEID_FLOAT,         GETTER,       IDATALTM,             1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setTileMod",                 ZVARTYPEID_VOID,          SETTER,       IDATALTM,             1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getScript",                  ZVARTYPEID_FLOAT,         GETTER,       IDATASCRIPT,          1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setScript",                  ZVARTYPEID_VOID,          SETTER,       IDATASCRIPT,          1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPScript",                 ZVARTYPEID_FLOAT,         GETTER,       IDATAPSCRIPT,         1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setPScript",                 ZVARTYPEID_VOID,          SETTER,       IDATAPSCRIPT,         1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPString",                 ZVARTYPEID_FLOAT,         GETTER,       IDATAPSTRING,         1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setPString",                 ZVARTYPEID_VOID,          SETTER,       IDATAPSTRING,         1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPickupString",                 ZVARTYPEID_FLOAT,         GETTER,       IDATAPSTRING,         1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setPickupString",                 ZVARTYPEID_VOID,          SETTER,       IDATAPSTRING,         1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPickupStringFlags",                 ZVARTYPEID_FLOAT,         GETTER,       IDATAPFLAGS,         1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setPickupStringFlags",                 ZVARTYPEID_VOID,          SETTER,       IDATAPFLAGS,         1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMagicCost",               ZVARTYPEID_FLOAT,         GETTER,       IDATAMAGCOST,         1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setMagicCost",               ZVARTYPEID_VOID,          SETTER,       IDATAMAGCOST,         1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCost",                    ZVARTYPEID_FLOAT,         GETTER,       IDATAMAGCOST,         1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setCost",                    ZVARTYPEID_VOID,          SETTER,       IDATAMAGCOST,         1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCost2",                   ZVARTYPEID_FLOAT,         GETTER,       IDATACOST2,           1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setCost2",                   ZVARTYPEID_VOID,          SETTER,       IDATACOST2,           1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCostCounter",             ZVARTYPEID_FLOAT,         GETTER,       IDATACOSTCOUNTER,     1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setCostCounter",             ZVARTYPEID_VOID,          SETTER,       IDATACOSTCOUNTER,     1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCostCounter2",            ZVARTYPEID_FLOAT,         GETTER,       IDATACOSTCOUNTER2,    1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setCostCounter2",            ZVARTYPEID_VOID,          SETTER,       IDATACOSTCOUNTER2,    1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMinHearts",               ZVARTYPEID_FLOAT,         GETTER,       IDATAMINHEARTS,       1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setMinHearts",               ZVARTYPEID_VOID,          SETTER,       IDATAMINHEARTS,       1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTile",                    ZVARTYPEID_FLOAT,         GETTER,       IDATATILE,            1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setTile",                    ZVARTYPEID_VOID,          SETTER,       IDATATILE,            1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlash",                   ZVARTYPEID_FLOAT,         GETTER,       IDATAMISC,            1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setFlash",                   ZVARTYPEID_VOID,          SETTER,       IDATAMISC,            1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCSet",                    ZVARTYPEID_FLOAT,         GETTER,       IDATACSET,            1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setCSet",                    ZVARTYPEID_VOID,          SETTER,       IDATACSET,            1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "getFrame",                   ZVARTYPEID_FLOAT,         GETTER,       IDATAFRAME,           1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
//	{ "setFrame",                   ZVARTYPEID_VOID,          SETTER,       IDATAFRAME,           1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAFrames",                 ZVARTYPEID_FLOAT,         GETTER,       IDATAFRAMES,          1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setAFrames",                 ZVARTYPEID_VOID,          SETTER,       IDATAFRAMES,          1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getASpeed",                  ZVARTYPEID_FLOAT,         GETTER,       IDATAASPEED,          1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setASpeed",                  ZVARTYPEID_VOID,          SETTER,       IDATAASPEED,          1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDelay",                   ZVARTYPEID_FLOAT,         GETTER,       IDATADELAY,           1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setDelay",                   ZVARTYPEID_VOID,          SETTER,       IDATADELAY,           1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCombine",                 ZVARTYPEID_BOOL,          GETTER,       IDATACOMBINE,         1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCombine",                 ZVARTYPEID_VOID,          SETTER,       IDATACOMBINE,         1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDowngrade",               ZVARTYPEID_BOOL,          GETTER,       IDATADOWNGRADE,       1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDowngrade",               ZVARTYPEID_VOID,          SETTER,       IDATADOWNGRADE,       1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getKeepOld",                 ZVARTYPEID_BOOL,          GETTER,       IDATAKEEPOLD,         1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setKeepOld",                 ZVARTYPEID_VOID,          SETTER,       IDATAKEEPOLD,         1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRupeeCost",               ZVARTYPEID_BOOL,          GETTER,       IDATARUPEECOST,       1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRupeeCost",               ZVARTYPEID_VOID,          SETTER,       IDATARUPEECOST,       1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getEdible",                  ZVARTYPEID_BOOL,          GETTER,       IDATAEDIBLE,          1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setEdible",                  ZVARTYPEID_VOID,          SETTER,       IDATAEDIBLE,          1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getUnused",                  ZVARTYPEID_BOOL,          GETTER,       IDATAFLAGUNUSED,      1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setUnused",                  ZVARTYPEID_VOID,          SETTER,       IDATAFLAGUNUSED,      1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getGainLower",               ZVARTYPEID_BOOL,          GETTER,       IDATAGAINLOWER,       1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setGainLower",               ZVARTYPEID_VOID,          SETTER,       IDATAGAINLOWER,       1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAttributes[]",            ZVARTYPEID_UNTYPED,       GETTER,       IDATAATTRIB,          15,            0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAttributes[]",            ZVARTYPEID_VOID,          SETTER,       IDATAATTRIB,          15,            0,                                    3,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMisc[]",                  ZVARTYPEID_UNTYPED,       GETTER,       IDATAATTRIB,          10,            0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMisc[]",                  ZVARTYPEID_VOID,          SETTER,       IDATAATTRIB,          10,            0,                                    3,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlags[]",                 ZVARTYPEID_BOOL,          GETTER,       IDATAFLAGS,           16,            0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlags[]",                 ZVARTYPEID_VOID,          SETTER,       IDATAFLAGS,           16,            0,                                    3,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSprites[]",               ZVARTYPEID_FLOAT,         GETTER,       IDATASPRITE,          10,            0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSprites[]",               ZVARTYPEID_VOID,          SETTER,       IDATASPRITE,          10,            0,                                    3,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWeapon",                  ZVARTYPEID_FLOAT,         GETTER,       IDATAUSEWPN,          1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setWeapon",                  ZVARTYPEID_VOID,          SETTER,       IDATAUSEWPN,          1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDefense",                 ZVARTYPEID_FLOAT,         GETTER,       IDATAUSEDEF,          1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setDefense",                 ZVARTYPEID_VOID,          SETTER,       IDATAUSEDEF,          1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDefence",                 ZVARTYPEID_FLOAT,         GETTER,       IDATAUSEDEF,          1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setDefence",                 ZVARTYPEID_VOID,          SETTER,       IDATAUSEDEF,          1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRange",                   ZVARTYPEID_FLOAT,         GETTER,       IDATAWRANGE,          1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setRange",                   ZVARTYPEID_VOID,          SETTER,       IDATAWRANGE,          1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getDuration",                ZVARTYPEID_FLOAT,         GETTER,       IDATADURATION,        1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setDuration",                ZVARTYPEID_VOID,          SETTER,       IDATADURATION,        1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	
	
//	{ "getMovement[]",              ZVARTYPEID_FLOAT,         GETTER,       IDATAUSEMVT,          ITEM_MOVEMENT_PATTERNS,     { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "setMovement[]",              ZVARTYPEID_VOID,          SETTER,       IDATAUSEMVT,          ITEM_MOVEMENT_PATTERNS,     { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWeaponInitD[]",           ZVARTYPEID_UNTYPED,       GETTER,       IDATAWPNINITD,        8,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWeaponInitD[]",           ZVARTYPEID_VOID,          SETTER,       IDATAWPNINITD,        8,             0,                                    3,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWeaponMisc[]",            ZVARTYPEID_UNTYPED,       GETTER,       IDATAMISCD,           32,            0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWeaponMisc[]",            ZVARTYPEID_VOID,          SETTER,       IDATAMISCD,           32,            0,                                    3,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getDuplicates",              ZVARTYPEID_FLOAT,         GETTER,       IDATADUPLICATES,      1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setDuplicates",              ZVARTYPEID_VOID,          SETTER,       IDATADUPLICATES,      1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getDrawLayer",               ZVARTYPEID_FLOAT,         GETTER,       IDATADRAWLAYER,       1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setDrawLayer",               ZVARTYPEID_VOID,          SETTER,       IDATADRAWLAYER,       1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getCollectFlags",            ZVARTYPEID_FLOAT,         GETTER,       IDATACOLLECTFLAGS,    1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setCollectFlags",            ZVARTYPEID_VOID,          SETTER,       IDATACOLLECTFLAGS,    1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWeaponScript",            ZVARTYPEID_FLOAT,         GETTER,       IDATAWEAPONSCRIPT,    1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setWeaponScript",            ZVARTYPEID_VOID,          SETTER,       IDATAWEAPONSCRIPT,    1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWeaponHitXOffset",        ZVARTYPEID_FLOAT,         GETTER,       IDATAWEAPHXOFS,       1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setWeaponHitXOffset",        ZVARTYPEID_VOID,          SETTER,       IDATAWEAPHXOFS,       1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWeaponHitYOffset",        ZVARTYPEID_FLOAT,         GETTER,       IDATAWEAPHYOFS,       1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setWeaponHitYOffset",        ZVARTYPEID_VOID,          SETTER,       IDATAWEAPHYOFS,       1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWeaponHitWidth",          ZVARTYPEID_FLOAT,         GETTER,       IDATAWEAPHXSZ,        1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setWeaponHitWidth",          ZVARTYPEID_VOID,          SETTER,       IDATAWEAPHXSZ,        1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWeaponHitHeight",         ZVARTYPEID_FLOAT,         GETTER,       IDATAWEAPHYSZ,        1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setWeaponHitHeight",         ZVARTYPEID_VOID,          SETTER,       IDATAWEAPHYSZ,        1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWeaponHitZHeight",        ZVARTYPEID_FLOAT,         GETTER,       IDATAWEAPHZSZ,        1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setWeaponHitZHeight",        ZVARTYPEID_VOID,          SETTER,       IDATAWEAPHZSZ,        1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWeaponDrawXOffset",       ZVARTYPEID_FLOAT,         GETTER,       IDATAWEAPXOFS,        1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setWeaponDrawXOffset",       ZVARTYPEID_VOID,          SETTER,       IDATAWEAPXOFS,        1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWeaponDrawYOffset",       ZVARTYPEID_FLOAT,         GETTER,       IDATAWEAPYOFS,        1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setWeaponDrawYOffset",       ZVARTYPEID_VOID,          SETTER,       IDATAWEAPYOFS,        1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWeaponDrawZOffset",       ZVARTYPEID_FLOAT,         GETTER,       IDATAWEAPZOFS,        1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setWeaponDrawZOffset",       ZVARTYPEID_VOID,          SETTER,       IDATAWEAPZOFS,        1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMagicCostTimer",          ZVARTYPEID_FLOAT,         GETTER,       IDATAMAGICTIMER,      1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setMagicCostTimer",          ZVARTYPEID_VOID,          SETTER,       IDATAMAGICTIMER,      1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCostTimer",               ZVARTYPEID_FLOAT,         GETTER,       IDATAMAGICTIMER,      1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setCostTimer",               ZVARTYPEID_VOID,          SETTER,       IDATAMAGICTIMER,      1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCostTimer2",              ZVARTYPEID_FLOAT,         GETTER,       IDATAMAGICTIMER2,      1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setCostTimer2",              ZVARTYPEID_VOID,          SETTER,       IDATAMAGICTIMER2,      1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	
	
	{ "getHitXOffset",              ZVARTYPEID_FLOAT,         GETTER,       IDATAHXOFS,           1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setHitXOffset",              ZVARTYPEID_VOID,          SETTER,       IDATAHXOFS,           1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitYOffset",              ZVARTYPEID_FLOAT,         GETTER,       IDATAHYOFS,           1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setHitYOffset",              ZVARTYPEID_VOID,          SETTER,       IDATAHYOFS,           1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getDrawXOffset",             ZVARTYPEID_FLOAT,         GETTER,       IDATADXOFS,           1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setDrawXOffset",             ZVARTYPEID_VOID,          SETTER,       IDATADXOFS,           1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getDrawYOffset",             ZVARTYPEID_FLOAT,         GETTER,       IDATADYOFS,           1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setDrawYOffset",             ZVARTYPEID_VOID,          SETTER,       IDATADYOFS,           1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getHitWidth",                ZVARTYPEID_FLOAT,         GETTER,       IDATAHXSZ,            1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setHitWidth",                ZVARTYPEID_VOID,          SETTER,       IDATAHXSZ,            1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitHeight",               ZVARTYPEID_FLOAT,         GETTER,       IDATAHYSZ,            1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setHitHeight",               ZVARTYPEID_VOID,          SETTER,       IDATAHYSZ,            1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHitZHeight",              ZVARTYPEID_FLOAT,         GETTER,       IDATAHZSZ,            1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setHitZHeight",              ZVARTYPEID_VOID,          SETTER,       IDATAHZSZ,            1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getTileWidth",               ZVARTYPEID_FLOAT,         GETTER,       IDATATILEW,           1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setTileWidth",               ZVARTYPEID_VOID,          SETTER,       IDATATILEW,           1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTileHeight",              ZVARTYPEID_FLOAT,         GETTER,       IDATATILEH,           1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setTileHeight",              ZVARTYPEID_VOID,          SETTER,       IDATATILEH,           1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getPickup",                  ZVARTYPEID_FLOAT,         GETTER,       IDATAPICKUP,          1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setPickup",                  ZVARTYPEID_VOID,          SETTER,       IDATAPICKUP,          1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getOverrideFlags",           ZVARTYPEID_FLOAT,         GETTER,       IDATAOVERRIDEFL,      1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setOverrideFlags",           ZVARTYPEID_VOID,          SETTER,       IDATAOVERRIDEFL,      1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	Weapon-forwarded Variables
	
	{ "getWeaponTileWidth",         ZVARTYPEID_FLOAT,         GETTER,       IDATATILEWWEAP,       1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setWeaponTileWidth",         ZVARTYPEID_VOID,          SETTER,       IDATATILEWWEAP,       1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWeaponTileHeight",        ZVARTYPEID_FLOAT,         GETTER,       IDATATILEHWEAP,       1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setWeaponTileHeight",        ZVARTYPEID_VOID,          SETTER,       IDATATILEHWEAP,       1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWeaponOverrideFlags",     ZVARTYPEID_FLOAT,         GETTER,       IDATAOVERRIDEFLWEAP,  1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },      
	{ "setWeaponOverrideFlags",     ZVARTYPEID_VOID,          SETTER,       IDATAOVERRIDEFLWEAP,  1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getValidate",                ZVARTYPEID_BOOL,          GETTER,       IDATAVALIDATE,        1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setValidate",                ZVARTYPEID_VOID,          SETTER,       IDATAVALIDATE,        1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getValidate2",               ZVARTYPEID_BOOL,          GETTER,       IDATAVALIDATE2,       1,             0,                                    1,           { ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setValidate2",               ZVARTYPEID_VOID,          SETTER,       IDATAVALIDATE2,       1,             0,                                    2,           { ZVARTYPEID_ITEMCLASS, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
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

