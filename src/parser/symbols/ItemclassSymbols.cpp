#include "SymbolDefs.h"

ItemclassSymbols ItemclassSymbols::singleton = ItemclassSymbols();

static AccessorTable itemclassTable[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
	{ "getFamily",                  0,         ZTID_FLOAT,   IDATAFAMILY,           0,  { ZTID_ITEMCLASS },{} },
	{ "setFamily",                  0,          ZTID_VOID,   IDATAFAMILY,           0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getType",                    0,         ZTID_FLOAT,   IDATAFAMILY,           0,  { ZTID_ITEMCLASS },{} },
	{ "setType",                    0,          ZTID_VOID,   IDATAFAMILY,           0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getLevel",                   0,         ZTID_FLOAT,   IDATALEVEL,          0,  { ZTID_ITEMCLASS },{} },
	{ "setLevel",                   0,          ZTID_VOID,   IDATALEVEL,          0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getAmount",                  0,         ZTID_FLOAT,   IDATAAMOUNT,           0,  { ZTID_ITEMCLASS },{} },
	{ "setAmount",                  0,          ZTID_VOID,   IDATAAMOUNT,           0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getMax",                     0,         ZTID_FLOAT,   IDATAMAX,              0,  { ZTID_ITEMCLASS },{} },
	{ "setMax",                     0,          ZTID_VOID,   IDATAMAX,              0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getMaxIncrement",            0,         ZTID_FLOAT,   IDATASETMAX,           0,  { ZTID_ITEMCLASS },{} },
	{ "setMaxIncrement",            0,          ZTID_VOID,   IDATASETMAX,           0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getEquipmentItem",           0,          ZTID_BOOL,   IDATAKEEP,          0,  { ZTID_ITEMCLASS },{} },
	{ "setEquipmentItem",           0,          ZTID_VOID,   IDATAKEEP,          0,  { ZTID_ITEMCLASS, ZTID_BOOL },{} },
	{ "getCounter",                 0,         ZTID_FLOAT,   IDATACOUNTER,          0,  { ZTID_ITEMCLASS },{} },
	{ "setCounter",                 0,          ZTID_VOID,   IDATACOUNTER,          0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getPickupSound",             0,         ZTID_FLOAT,   IDATAPSOUND,               0,  { ZTID_ITEMCLASS },{} },
	{ "setPickupSound",             0,          ZTID_VOID,   IDATAPSOUND,               0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getUseSound",                0,         ZTID_FLOAT,   IDATAUSESOUND,         0,  { ZTID_ITEMCLASS },{} },
	{ "setUseSound",                0,          ZTID_VOID,   IDATAUSESOUND,         0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getUseSound2",               0,         ZTID_FLOAT,   IDATAUSESOUND2,            0,  { ZTID_ITEMCLASS },{} },
	{ "setUseSound2",               0,          ZTID_VOID,   IDATAUSESOUND2,            0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getID",                      0,         ZTID_FLOAT,   IDATAID,               0,  { ZTID_ITEMCLASS },{} },
	{ "setID",                      0,          ZTID_VOID,   IDATAID,       FL_RDONLY,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	
	{ "getPower",                   0,         ZTID_FLOAT,   IDATAPOWER,            0,  { ZTID_ITEMCLASS },{} },
	{ "setPower",                   0,          ZTID_VOID,   IDATAPOWER,            0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getDamage",                  0,         ZTID_FLOAT,   IDATAPOWER,            0,  { ZTID_ITEMCLASS },{} },
	{ "setDamage",                  0,          ZTID_VOID,   IDATAPOWER,            0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getInitD[]",                 0,       ZTID_UNTYPED,   IDATAINITDD,           0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "setInitD[]",                 0,          ZTID_VOID,   IDATAINITDD,           0,  { ZTID_ITEMCLASS, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "GetName",                    0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "GetDisplayName",             0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "SetDisplayName",             0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "GetShownName",               0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "RunScript",                  0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_ITEMCLASS, ZTID_FLOAT },{ 10000 } },
	
	{ "getTileMod",                 0,         ZTID_FLOAT,   IDATALTM,                  0,  { ZTID_ITEMCLASS },{} },
	{ "setTileMod",                 0,          ZTID_VOID,   IDATALTM,                  0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getScript",                  0,         ZTID_FLOAT,   IDATASCRIPT,               0,  { ZTID_ITEMCLASS },{} },
	{ "setScript",                  0,          ZTID_VOID,   IDATASCRIPT,               0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getPScript",                 0,         ZTID_FLOAT,   IDATAPSCRIPT,              0,  { ZTID_ITEMCLASS },{} },
	{ "setPScript",                 0,          ZTID_VOID,   IDATAPSCRIPT,              0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getSpriteScript",            0,         ZTID_FLOAT,   IDATASPRSCRIPT,            0,  { ZTID_ITEMCLASS },{} },
	{ "setSpriteScript",            0,          ZTID_VOID,   IDATASPRSCRIPT,            0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getPString",                 0,         ZTID_FLOAT,   IDATAPSTRING,              0,  { ZTID_ITEMCLASS },{} },
	{ "setPString",                 0,          ZTID_VOID,   IDATAPSTRING,              0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getPickupString",            0,         ZTID_FLOAT,   IDATAPSTRING,              0,  { ZTID_ITEMCLASS },{} },
	{ "setPickupString",            0,          ZTID_VOID,   IDATAPSTRING,              0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getPickupStringFlags",       0,         ZTID_FLOAT,   IDATAPFLAGS,               0,  { ZTID_ITEMCLASS },{} },
	{ "setPickupStringFlags",       0,          ZTID_VOID,   IDATAPFLAGS,               0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getCost",                    0,         ZTID_FLOAT,   IDATAMAGCOST,              0,  { ZTID_ITEMCLASS },{} },
	{ "setCost",                    0,          ZTID_VOID,   IDATAMAGCOST,              0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getCost2",                   0,         ZTID_FLOAT,   IDATACOST2,                0,  { ZTID_ITEMCLASS },{} },
	{ "setCost2",                   0,          ZTID_VOID,   IDATACOST2,                0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getCostCounter",             0,         ZTID_FLOAT,   IDATACOSTCOUNTER,          0,  { ZTID_ITEMCLASS },{} },
	{ "setCostCounter",             0,          ZTID_VOID,   IDATACOSTCOUNTER,          0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getCostCounter2",            0,         ZTID_FLOAT,   IDATACOSTCOUNTER2,         0,  { ZTID_ITEMCLASS },{} },
	{ "setCostCounter2",            0,          ZTID_VOID,   IDATACOSTCOUNTER2,         0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getMinHearts",               0,         ZTID_FLOAT,   IDATAMINHEARTS,            0,  { ZTID_ITEMCLASS },{} },
	{ "setMinHearts",               0,          ZTID_VOID,   IDATAMINHEARTS,            0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getTile",                    0,         ZTID_FLOAT,   IDATATILE,                 0,  { ZTID_ITEMCLASS },{} },
	{ "setTile",                    0,          ZTID_VOID,   IDATATILE,                 0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getMiscFlags",               0,         ZTID_FLOAT,   IDATAMISC,                 0,  { ZTID_ITEMCLASS },{} },
	{ "setMiscFlags",               0,          ZTID_VOID,   IDATAMISC,                 0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getCSet",                    0,         ZTID_FLOAT,   IDATACSET,                 0,  { ZTID_ITEMCLASS },{} },
	{ "setCSet",                    0,          ZTID_VOID,   IDATACSET,                 0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getAFrames",                 0,         ZTID_FLOAT,   IDATAFRAMES,               0,  { ZTID_ITEMCLASS },{} },
	{ "setAFrames",                 0,          ZTID_VOID,   IDATAFRAMES,               0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getASpeed",                  0,         ZTID_FLOAT,   IDATAASPEED,               0,  { ZTID_ITEMCLASS },{} },
	{ "setASpeed",                  0,          ZTID_VOID,   IDATAASPEED,               0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getDelay",                   0,         ZTID_FLOAT,   IDATADELAY,                0,  { ZTID_ITEMCLASS },{} },
	{ "setDelay",                   0,          ZTID_VOID,   IDATADELAY,                0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getCombine",                 0,          ZTID_BOOL,   IDATACOMBINE,              0,  { ZTID_ITEMCLASS },{} },
	{ "setCombine",                 0,          ZTID_VOID,   IDATACOMBINE,              0,  { ZTID_ITEMCLASS, ZTID_BOOL },{} },
	{ "getDowngrade",               0,          ZTID_BOOL,   IDATADOWNGRADE,            0,  { ZTID_ITEMCLASS },{} },
	{ "setDowngrade",               0,          ZTID_VOID,   IDATADOWNGRADE,            0,  { ZTID_ITEMCLASS, ZTID_BOOL },{} },
	{ "getKeepOld",                 0,          ZTID_BOOL,   IDATAKEEPOLD,              0,  { ZTID_ITEMCLASS },{} },
	{ "setKeepOld",                 0,          ZTID_VOID,   IDATAKEEPOLD,              0,  { ZTID_ITEMCLASS, ZTID_BOOL },{} },
	{ "getGradual",                 0,          ZTID_BOOL,   IDATAGRADUAL,              0,  { ZTID_ITEMCLASS },{} },
	{ "setGradual",                 0,          ZTID_VOID,   IDATAGRADUAL,              0,  { ZTID_ITEMCLASS, ZTID_BOOL },{} },
	{ "getEdible",                  0,          ZTID_BOOL,   IDATAEDIBLE,               0,  { ZTID_ITEMCLASS },{} },
	{ "setEdible",                  0,          ZTID_VOID,   IDATAEDIBLE,               0,  { ZTID_ITEMCLASS, ZTID_BOOL },{} },
	{ "getGainLower",               0,          ZTID_BOOL,   IDATAGAINLOWER,            0,  { ZTID_ITEMCLASS },{} },
	{ "setGainLower",               0,          ZTID_VOID,   IDATAGAINLOWER,            0,  { ZTID_ITEMCLASS, ZTID_BOOL },{} },
	{ "getAttributes[]",            0,       ZTID_UNTYPED,   IDATAATTRIB,               0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "setAttributes[]",            0,          ZTID_VOID,   IDATAATTRIB,               0,  { ZTID_ITEMCLASS, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getLAttributes[]",           0,          ZTID_LONG,   IDATAATTRIB_L,             0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "setLAttributes[]",           0,          ZTID_VOID,   IDATAATTRIB_L,             0,  { ZTID_ITEMCLASS, ZTID_FLOAT, ZTID_LONG },{} },
	{ "getFlags[]",                 0,          ZTID_BOOL,   IDATAFLAGS,                0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "setFlags[]",                 0,          ZTID_VOID,   IDATAFLAGS,                0,  { ZTID_ITEMCLASS, ZTID_FLOAT, ZTID_BOOL },{} },
	
	{ "getSprites[]",               0,         ZTID_FLOAT,   IDATASPRITE,               0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "setSprites[]",               0,          ZTID_VOID,   IDATASPRITE,               0,  { ZTID_ITEMCLASS, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getWeapon",                  0,         ZTID_FLOAT,   IDATAUSEWPN,               0,  { ZTID_ITEMCLASS },{} },
	{ "setWeapon",                  0,          ZTID_VOID,   IDATAUSEWPN,               0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getDefense",                 0,         ZTID_FLOAT,   IDATAUSEDEF,               0,  { ZTID_ITEMCLASS },{} },
	{ "setDefense",                 0,          ZTID_VOID,   IDATAUSEDEF,               0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	
	{ "getWeaponInitD[]",           0,       ZTID_UNTYPED,   IDATAWPNINITD,             0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "setWeaponInitD[]",           0,          ZTID_VOID,   IDATAWPNINITD,             0,  { ZTID_ITEMCLASS, ZTID_FLOAT, ZTID_UNTYPED },{} },
	
	{ "getWeaponScript",            0,         ZTID_FLOAT,   IDATAWEAPONSCRIPT,         0,  { ZTID_ITEMCLASS },{} },
	{ "setWeaponScript",            0,          ZTID_VOID,   IDATAWEAPONSCRIPT,         0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	
	{ "getWeaponHitXOffset",        0,         ZTID_FLOAT,   IDATAWEAPHXOFS,            0,  { ZTID_ITEMCLASS },{} },
	{ "setWeaponHitXOffset",        0,          ZTID_VOID,   IDATAWEAPHXOFS,            0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getWeaponHitYOffset",        0,         ZTID_FLOAT,   IDATAWEAPHYOFS,            0,  { ZTID_ITEMCLASS },{} },
	{ "setWeaponHitYOffset",        0,          ZTID_VOID,   IDATAWEAPHYOFS,            0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	
	{ "getWeaponHitWidth",          0,         ZTID_FLOAT,   IDATAWEAPHXSZ,             0,  { ZTID_ITEMCLASS },{} },
	{ "setWeaponHitWidth",          0,          ZTID_VOID,   IDATAWEAPHXSZ,             0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	
	{ "getWeaponHitHeight",         0,         ZTID_FLOAT,   IDATAWEAPHYSZ,             0,  { ZTID_ITEMCLASS },{} },
	{ "setWeaponHitHeight",         0,          ZTID_VOID,   IDATAWEAPHYSZ,             0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	
	{ "getWeaponHitZHeight",        0,         ZTID_FLOAT,   IDATAWEAPHZSZ,             0,  { ZTID_ITEMCLASS },{} },
	{ "setWeaponHitZHeight",        0,          ZTID_VOID,   IDATAWEAPHZSZ,             0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	
	{ "getWeaponDrawXOffset",       0,         ZTID_FLOAT,   IDATAWEAPXOFS,             0,  { ZTID_ITEMCLASS },{} },
	{ "setWeaponDrawXOffset",       0,          ZTID_VOID,   IDATAWEAPXOFS,             0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getWeaponDrawYOffset",       0,         ZTID_FLOAT,   IDATAWEAPYOFS,             0,  { ZTID_ITEMCLASS },{} },
	{ "setWeaponDrawYOffset",       0,          ZTID_VOID,   IDATAWEAPYOFS,             0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getWeaponDrawZOffset",       0,         ZTID_FLOAT,   IDATAWEAPZOFS,             0,  { ZTID_ITEMCLASS },{} },
	{ "setWeaponDrawZOffset",       0,          ZTID_VOID,   IDATAWEAPZOFS,             0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getCostTimer",               0,         ZTID_FLOAT,   IDATAMAGICTIMER,           0,  { ZTID_ITEMCLASS },{} },
	{ "setCostTimer",               0,          ZTID_VOID,   IDATAMAGICTIMER,           0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getCostTimer2",              0,         ZTID_FLOAT,   IDATAMAGICTIMER2,          0,  { ZTID_ITEMCLASS },{} },
	{ "setCostTimer2",              0,          ZTID_VOID,   IDATAMAGICTIMER2,          0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	
	
	
	{ "getHitXOffset",              0,         ZTID_FLOAT,   IDATAHXOFS,                0,  { ZTID_ITEMCLASS },{} },
	{ "setHitXOffset",              0,          ZTID_VOID,   IDATAHXOFS,                0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getHitYOffset",              0,         ZTID_FLOAT,   IDATAHYOFS,                0,  { ZTID_ITEMCLASS },{} },
	{ "setHitYOffset",              0,          ZTID_VOID,   IDATAHYOFS,                0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	
	{ "getDrawXOffset",             0,         ZTID_FLOAT,   IDATADXOFS,                0,  { ZTID_ITEMCLASS },{} },
	{ "setDrawXOffset",             0,          ZTID_VOID,   IDATADXOFS,                0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	
	{ "getDrawYOffset",             0,         ZTID_FLOAT,   IDATADYOFS,                0,  { ZTID_ITEMCLASS },{} },
	{ "setDrawYOffset",             0,          ZTID_VOID,   IDATADYOFS,                0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	
	{ "getHitWidth",                0,         ZTID_FLOAT,   IDATAHXSZ,                 0,  { ZTID_ITEMCLASS },{} },
	{ "setHitWidth",                0,          ZTID_VOID,   IDATAHXSZ,                 0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getHitHeight",               0,         ZTID_FLOAT,   IDATAHYSZ,                 0,  { ZTID_ITEMCLASS },{} },
	{ "setHitHeight",               0,          ZTID_VOID,   IDATAHYSZ,                 0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getHitZHeight",              0,         ZTID_FLOAT,   IDATAHZSZ,                 0,  { ZTID_ITEMCLASS },{} },
	{ "setHitZHeight",              0,          ZTID_VOID,   IDATAHZSZ,                 0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	
	{ "getTileWidth",               0,         ZTID_FLOAT,   IDATATILEW,                0,  { ZTID_ITEMCLASS },{} },
	{ "setTileWidth",               0,          ZTID_VOID,   IDATATILEW,                0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getTileHeight",              0,         ZTID_FLOAT,   IDATATILEH,                0,  { ZTID_ITEMCLASS },{} },
	{ "setTileHeight",              0,          ZTID_VOID,   IDATATILEH,                0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	
	{ "getPickup",                  0,         ZTID_FLOAT,   IDATAPICKUP,               0,  { ZTID_ITEMCLASS },{} },
	{ "setPickup",                  0,          ZTID_VOID,   IDATAPICKUP,               0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	
	{ "getSizeFlags",               0,         ZTID_FLOAT,   IDATAOVERRIDEFL,           0,  { ZTID_ITEMCLASS },{} },
	{ "setSizeFlags",               0,          ZTID_VOID,   IDATAOVERRIDEFL,           0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	
	{ "getWeaponTileWidth",         0,         ZTID_FLOAT,   IDATATILEWWEAP,            0,  { ZTID_ITEMCLASS },{} },
	{ "setWeaponTileWidth",         0,          ZTID_VOID,   IDATATILEWWEAP,            0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getWeaponTileHeight",        0,         ZTID_FLOAT,   IDATATILEHWEAP,            0,  { ZTID_ITEMCLASS },{} },
	{ "setWeaponTileHeight",        0,          ZTID_VOID,   IDATATILEHWEAP,            0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	
	{ "getWeaponSizeFlags",         0,         ZTID_FLOAT,   IDATAOVERRIDEFLWEAP,       0,  { ZTID_ITEMCLASS },{} },
	{ "setWeaponSizeFlags",         0,          ZTID_VOID,   IDATAOVERRIDEFLWEAP,       0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	
	{ "getValidate",                0,          ZTID_BOOL,   IDATAVALIDATE,             0,  { ZTID_ITEMCLASS },{} },
	{ "setValidate",                0,          ZTID_VOID,   IDATAVALIDATE,             0,  { ZTID_ITEMCLASS, ZTID_BOOL },{} },
	{ "getValidate2",               0,          ZTID_BOOL,   IDATAVALIDATE2,            0,  { ZTID_ITEMCLASS },{} },
	{ "setValidate2",               0,          ZTID_VOID,   IDATAVALIDATE2,            0,  { ZTID_ITEMCLASS, ZTID_BOOL },{} },
	
	{ "getConstantScript",          0,          ZTID_BOOL,   IDATACONSTSCRIPT,          0,  { ZTID_ITEMCLASS },{} },
	{ "setConstantScript",          0,          ZTID_VOID,   IDATACONSTSCRIPT,          0,  { ZTID_ITEMCLASS, ZTID_BOOL },{} },
	{ "getSideSwimDisabled",        0,          ZTID_BOOL,   IDATASSWIMDISABLED,        0,  { ZTID_ITEMCLASS },{} },
	{ "setSideSwimDisabled",        0,          ZTID_VOID,   IDATASSWIMDISABLED,        0,  { ZTID_ITEMCLASS, ZTID_BOOL },{} },
	{ "getBunnyUsable",             0,          ZTID_BOOL,   IDATABUNNYABLE,            0,  { ZTID_ITEMCLASS },{} },
	{ "setBunnyUsable",             0,          ZTID_VOID,   IDATABUNNYABLE,            0,  { ZTID_ITEMCLASS, ZTID_BOOL },{} },
	{ "getJinxImmune",              0,          ZTID_BOOL,   IDATAJINXIMMUNE,           0,  { ZTID_ITEMCLASS },{} },
	{ "setJinxImmune",              0,          ZTID_VOID,   IDATAJINXIMMUNE,           0,  { ZTID_ITEMCLASS, ZTID_BOOL },{} },
	{ "getJinxSwapped",             0,          ZTID_BOOL,   IDATAJINXSWAP,             0,  { ZTID_ITEMCLASS },{} },
	{ "setJinxSwapped",             0,          ZTID_VOID,   IDATAJINXSWAP,             0,  { ZTID_ITEMCLASS, ZTID_BOOL },{} },
	
	{ "getUseBurnSprites",          0,          ZTID_BOOL,   IDATAUSEBURNSPR,           0,  { ZTID_ITEMCLASS },{} },
	{ "setUseBurnSprites",          0,          ZTID_VOID,   IDATAUSEBURNSPR,           0,  { ZTID_ITEMCLASS, ZTID_BOOL },{} },
	{ "getBurnSprites[]",           0,         ZTID_FLOAT,   IDATABURNINGSPR,           0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "setBurnSprites[]",           0,          ZTID_VOID,   IDATABURNINGSPR,           0,  { ZTID_ITEMCLASS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getBurnLightRadius[]",       0,         ZTID_FLOAT,   IDATABURNINGLIGHTRAD,      0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "setBurnLightRadius[]",       0,          ZTID_VOID,   IDATABURNINGLIGHTRAD,      0,  { ZTID_ITEMCLASS, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	//Intentionally undocumented
	{ "getKeep",                    0,          ZTID_BOOL,   IDATAKEEP,          0,  { ZTID_ITEMCLASS },{} },
	{ "setKeep",                    0,          ZTID_VOID,   IDATAKEEP,          0,  { ZTID_ITEMCLASS, ZTID_BOOL },{} },
	{ "getModifier",                0,         ZTID_FLOAT,   IDATALTM,                  0,  { ZTID_ITEMCLASS },{} },
	{ "setModifier",                0,          ZTID_VOID,   IDATALTM,                  0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getMagicCost",               0,         ZTID_FLOAT,   IDATAMAGCOST,              0,  { ZTID_ITEMCLASS },{} },
	{ "setMagicCost",               0,          ZTID_VOID,   IDATAMAGCOST,              0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getMagicCostTimer",          0,         ZTID_FLOAT,   IDATAMAGICTIMER,           0,  { ZTID_ITEMCLASS },{} },
	{ "setMagicCostTimer",          0,          ZTID_VOID,   IDATAMAGICTIMER,           0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getMisc[]",                  0,       ZTID_UNTYPED,   IDATAATTRIB,               0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "setMisc[]",                  0,          ZTID_VOID,   IDATAATTRIB,               0,  { ZTID_ITEMCLASS, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getFlash",                   0,         ZTID_FLOAT,   IDATAMISC,                 0,  { ZTID_ITEMCLASS },{} },
	{ "setFlash",                   0,          ZTID_VOID,   IDATAMISC,                 0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getOverrideFlags",           0,         ZTID_FLOAT,   IDATAOVERRIDEFL,           0,  { ZTID_ITEMCLASS },{} },
	{ "setOverrideFlags",           0,          ZTID_VOID,   IDATAOVERRIDEFL,           0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getWeaponOverrideFlags",     0,         ZTID_FLOAT,   IDATAOVERRIDEFLWEAP,       0,  { ZTID_ITEMCLASS },{} },
	{ "setWeaponOverrideFlags",     0,          ZTID_VOID,   IDATAOVERRIDEFLWEAP,       0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	
	{ "getDefence",                 0,         ZTID_FLOAT,   IDATAUSEDEF,               0,  { ZTID_ITEMCLASS },{} },
	{ "setDefence",                 0,          ZTID_VOID,   IDATAUSEDEF,               0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getRange",                   0,         ZTID_FLOAT,   IDATAWRANGE,               0,  { ZTID_ITEMCLASS },{} },
	{ "setRange",                   0,          ZTID_VOID,   IDATAWRANGE,               0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getDuration",                0,         ZTID_FLOAT,   IDATADURATION,             0,  { ZTID_ITEMCLASS },{} },
	{ "setDuration",                0,          ZTID_VOID,   IDATADURATION,             0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getDuplicates",              0,         ZTID_FLOAT,   IDATADUPLICATES,           0,  { ZTID_ITEMCLASS },{} },
	{ "setDuplicates",              0,          ZTID_VOID,   IDATADUPLICATES,           0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getDrawLayer",               0,         ZTID_FLOAT,   IDATADRAWLAYER,            0,  { ZTID_ITEMCLASS },{} },
	{ "setDrawLayer",               0,          ZTID_VOID,   IDATADRAWLAYER,            0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },

	{ "getRupeeCost",               0,          ZTID_BOOL,   IDATARUPEECOST,            0,  { ZTID_ITEMCLASS },{} },
	{ "setRupeeCost",               0,          ZTID_VOID,   IDATARUPEECOST,            0,  { ZTID_ITEMCLASS, ZTID_BOOL },{} },
	//Unused?
	{ "getUnused",                  0,          ZTID_BOOL,   IDATAFLAGUNUSED,           0,  { ZTID_ITEMCLASS },{} },
	{ "setUnused",                  0,          ZTID_VOID,   IDATAFLAGUNUSED,           0,  { ZTID_ITEMCLASS, ZTID_BOOL },{} },
	{ "getCollectFlags",            0,         ZTID_FLOAT,   IDATACOLLECTFLAGS,         0,  { ZTID_ITEMCLASS },{} },
	{ "setCollectFlags",            0,          ZTID_VOID,   IDATACOLLECTFLAGS,         0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "getWeaponMisc[]",            0,       ZTID_UNTYPED,   IDATAMISCD,                0,  { ZTID_ITEMCLASS, ZTID_FLOAT },{} },
	{ "setWeaponMisc[]",            0,          ZTID_VOID,   IDATAMISCD,                0,  { ZTID_ITEMCLASS, ZTID_FLOAT, ZTID_UNTYPED },{} },
	
	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
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
		Function* function = getFunction("GetName");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new OGetItemName(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void GetDisplayName(itemclass, int32_t)
	{
		Function* function = getFunction("GetDisplayName");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new OItemGetDispName(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void SetDisplayName(itemclass, int32_t)
	{
		Function* function = getFunction("SetDisplayName");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new OItemSetDispName(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void GetShownName(itemclass, int32_t)
	{
		Function* function = getFunction("GetShownName");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new OItemGetShownName(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void RunScript(itemclass)
	{
		Function* function = getFunction("RunScript");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new ORunItemScript(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	
}

