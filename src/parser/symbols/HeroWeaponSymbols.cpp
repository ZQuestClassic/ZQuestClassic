#include "SymbolDefs.h"

HeroWeaponSymbols HeroWeaponSymbols::singleton = HeroWeaponSymbols();

static AccessorTable lwpnTable[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
	{ "getX",                       0,         ZTID_FLOAT,   LWPNX,                     0,  { ZTID_LWPN },{} },
	{ "setX",                       0,          ZTID_VOID,   LWPNX,                     0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getSpecial",                 0,         ZTID_FLOAT,   LWPNSPECIAL,               0,  { ZTID_LWPN },{} },
	{ "setSpecial",                 0,          ZTID_VOID,   LWPNSPECIAL,               0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "_getMax",                    0,         ZTID_FLOAT,   SPRITEMAXLWPN,             0,  { ZTID_LWPN },{} },
	{ "_setMax",                    0,          ZTID_VOID,   SPRITEMAXLWPN,             0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getY",                       0,         ZTID_FLOAT,   LWPNY,                     0,  { ZTID_LWPN },{} },
	{ "setY",                       0,          ZTID_VOID,   LWPNY,                     0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getZ",                       0,         ZTID_FLOAT,   LWPNZ,                     0,  { ZTID_LWPN },{} },
	{ "setZ",                       0,          ZTID_VOID,   LWPNZ,                     0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getJump",                    0,         ZTID_FLOAT,   LWPNJUMP,                  0,  { ZTID_LWPN },{} },
	{ "setJump",                    0,          ZTID_VOID,   LWPNJUMP,                  0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getDir",                     0,         ZTID_FLOAT,   LWPNDIR,                   0,  { ZTID_LWPN },{} },
	{ "setDir",                     0,          ZTID_VOID,   LWPNDIR,                   0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getAngle",                   0,         ZTID_FLOAT,   LWPNANGLE,                 0,  { ZTID_LWPN },{} },
	{ "setAngle",                   0,          ZTID_VOID,   LWPNANGLE,                 0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getStep",                    0,         ZTID_FLOAT,   LWPNSTEP,                  0,  { ZTID_LWPN },{} },
	{ "setStep",                    0,          ZTID_VOID,   LWPNSTEP,                  0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getNumFrames",               0,         ZTID_FLOAT,   LWPNFRAMES,                0,  { ZTID_LWPN },{} },
	{ "setNumFrames",               0,          ZTID_VOID,   LWPNFRAMES,                0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getFrame",                   0,         ZTID_FLOAT,   LWPNFRAME,                 0,  { ZTID_LWPN },{} },
	{ "setFrame",                   0,          ZTID_VOID,   LWPNFRAME,                 0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getDrawStyle",               0,         ZTID_FLOAT,   LWPNDRAWTYPE,              0,  { ZTID_LWPN },{} },
	{ "setDrawStyle",               0,          ZTID_VOID,   LWPNDRAWTYPE,              0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getPower",                   0,         ZTID_FLOAT,   LWPNPOWER,                 0,  { ZTID_LWPN },{} },
	{ "setPower",                   0,          ZTID_VOID,   LWPNPOWER,                 0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getDamage",                  0,         ZTID_FLOAT,   LWPNPOWER,                 0,  { ZTID_LWPN },{} },
	{ "setDamage",                  0,          ZTID_VOID,   LWPNPOWER,                 0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getID",                      0,         ZTID_FLOAT,   LWPNID,                    0,  { ZTID_LWPN },{} },
	{ "setID",                      0,          ZTID_VOID,   LWPNID,                    0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getType",                    0,         ZTID_FLOAT,   LWPNID,                    0,  { ZTID_LWPN },{} },
	{ "setType",                    0,          ZTID_VOID,   LWPNID,                    0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getFamily",                  0,         ZTID_FLOAT,   LWPNID,                    0,  { ZTID_LWPN },{} },
	{ "setFamily",                  0,          ZTID_VOID,   LWPNID,                    0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getAngular",                 0,          ZTID_BOOL,   LWPNANGULAR,               0,  { ZTID_LWPN },{} },
	{ "setAngular",                 0,          ZTID_VOID,   LWPNANGULAR,               0,  { ZTID_LWPN, ZTID_BOOL },{} },
	{ "getBehind",                  0,          ZTID_BOOL,   LWPNBEHIND,                0,  { ZTID_LWPN },{} },
	{ "setBehind",                  0,          ZTID_VOID,   LWPNBEHIND,                0,  { ZTID_LWPN, ZTID_BOOL },{} },
	{ "getASpeed",                  0,         ZTID_FLOAT,   LWPNASPEED,                0,  { ZTID_LWPN },{} },
	{ "setASpeed",                  0,          ZTID_VOID,   LWPNASPEED,                0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getTile",                    0,         ZTID_FLOAT,   LWPNTILE,                  0,  { ZTID_LWPN },{} },
	{ "setTile",                    0,          ZTID_VOID,   LWPNTILE,                  0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getScriptTile",              0,         ZTID_FLOAT,   LWPNSCRIPTTILE,            0,  { ZTID_LWPN },{} },
	{ "setScriptTile",              0,          ZTID_VOID,   LWPNSCRIPTTILE,            0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getScriptFlip",              0,         ZTID_FLOAT,   LWPNSCRIPTFLIP,            0,  { ZTID_LWPN },{} },
	{ "setScriptFlip",              0,          ZTID_VOID,   LWPNSCRIPTFLIP,            0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getFlashCSet",               0,         ZTID_FLOAT,   LWPNFLASHCSET,             0,  { ZTID_LWPN },{} },
	{ "setFlashCSet",               0,          ZTID_VOID,   LWPNFLASHCSET,             0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getDeadState",               0,         ZTID_FLOAT,   LWPNDEAD,                  0,  { ZTID_LWPN },{} },
	{ "setDeadState",               0,          ZTID_VOID,   LWPNDEAD,                  0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getCSet",                    0,         ZTID_FLOAT,   LWPNCSET,                  0,  { ZTID_LWPN },{} },
	{ "setCSet",                    0,          ZTID_VOID,   LWPNCSET,                  0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getFlash",                   0,          ZTID_BOOL,   LWPNFLASH,                 0,  { ZTID_LWPN },{} },
	{ "setFlash",                   0,          ZTID_VOID,   LWPNFLASH,                 0,  { ZTID_LWPN, ZTID_BOOL },{} },
	{ "getFlip",                    0,         ZTID_FLOAT,   LWPNFLIP,                  0,  { ZTID_LWPN },{} },
	{ "setFlip",                    0,          ZTID_VOID,   LWPNFLIP,                  0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getOriginalTile",            0,         ZTID_FLOAT,   LWPNOTILE,                 0,  { ZTID_LWPN },{} },
	{ "setOriginalTile",            0,          ZTID_VOID,   LWPNOTILE,                 0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getOriginalCSet",            0,         ZTID_FLOAT,   LWPNOCSET,                 0,  { ZTID_LWPN },{} },
	{ "setOriginalCSet",            0,          ZTID_VOID,   LWPNOCSET,                 0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getExtend",                  0,         ZTID_FLOAT,   LWPNEXTEND,                0,  { ZTID_LWPN },{} },
	{ "setExtend",                  0,          ZTID_VOID,   LWPNEXTEND,                0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getHitWidth",                0,         ZTID_FLOAT,   LWPNHXSZ,                  0,  { ZTID_LWPN },{} },
	{ "setHitWidth",                0,          ZTID_VOID,   LWPNHXSZ,                  0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getHitHeight",               0,         ZTID_FLOAT,   LWPNHYSZ,                  0,  { ZTID_LWPN },{} },
	{ "setHitHeight",               0,          ZTID_VOID,   LWPNHYSZ,                  0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getHitZHeight",              0,         ZTID_FLOAT,   LWPNHZSZ,                  0,  { ZTID_LWPN },{} },
	{ "setHitZHeight",              0,          ZTID_VOID,   LWPNHZSZ,                  0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getTileWidth",               0,         ZTID_FLOAT,   LWPNTXSZ,                  0,  { ZTID_LWPN },{} },
	{ "setTileWidth",               0,          ZTID_VOID,   LWPNTXSZ,                  0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getTileHeight",              0,         ZTID_FLOAT,   LWPNTYSZ,                  0,  { ZTID_LWPN },{} },
	{ "setTileHeight",              0,          ZTID_VOID,   LWPNTYSZ,                  0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getDrawXOffset",             0,         ZTID_FLOAT,   LWPNXOFS,                  0,  { ZTID_LWPN },{} },
	{ "setDrawXOffset",             0,          ZTID_VOID,   LWPNXOFS,                  0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getDrawYOffset",             0,         ZTID_FLOAT,   LWPNYOFS,                  0,  { ZTID_LWPN },{} },
	{ "setDrawYOffset",             0,          ZTID_VOID,   LWPNYOFS,                  0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getTotalDYOffset",           0,         ZTID_FLOAT,   LWPNTOTALDYOFFS,           0,  { ZTID_LWPN },{} },
	{ "setTotalDYOffset",           0,          ZTID_VOID,   LWPNTOTALDYOFFS,           0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getDrawZOffset",             0,         ZTID_FLOAT,   LWPNZOFS,                  0,  { ZTID_LWPN },{} },
	{ "setDrawZOffset",             0,          ZTID_VOID,   LWPNZOFS,                  0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getHitXOffset",              0,         ZTID_FLOAT,   LWPNHXOFS,                 0,  { ZTID_LWPN },{} },
	{ "setHitXOffset",              0,          ZTID_VOID,   LWPNHXOFS,                 0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getHitYOffset",              0,         ZTID_FLOAT,   LWPNHYOFS,                 0,  { ZTID_LWPN },{} },
	{ "setHitYOffset",              0,          ZTID_VOID,   LWPNHYOFS,                 0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "isValid",                    0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_LWPN },{} },
	{ "UseSprite",                  0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getMisc[]",                  0,       ZTID_UNTYPED,   LWPNMISCD,                 0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "setMisc[]",                  0,          ZTID_VOID,   LWPNMISCD,                 0,  { ZTID_LWPN, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getCollDetection",           0,          ZTID_BOOL,   LWPNCOLLDET,               0,  { ZTID_LWPN },{} },
	{ "setCollDetection",           0,          ZTID_VOID,   LWPNCOLLDET,               0,  { ZTID_LWPN, ZTID_BOOL },{} },
	{ "getAnimation",               0,          ZTID_BOOL,   LWPNENGINEANIMATE,         0,  { ZTID_LWPN },{} },
	{ "setAnimation",               0,          ZTID_VOID,   LWPNENGINEANIMATE,         0,  { ZTID_LWPN, ZTID_BOOL },{} },
//	{ "getRange",                   0,         ZTID_FLOAT,   LWPNRANGE,                 0,  { ZTID_LWPN },{} },
//	{ "setRange",                   0,          ZTID_VOID,   LWPNRANGE,                 0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getParent",                  0,         ZTID_FLOAT,   LWPNPARENT,                0,  { ZTID_LWPN },{} },
	{ "setParent",                  0,          ZTID_VOID,   LWPNPARENT,                0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getLevel",                   0,         ZTID_FLOAT,   LWPNLEVEL,                 0,  { ZTID_LWPN },{} },
	{ "setLevel",                   0,          ZTID_VOID,   LWPNLEVEL,                 0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getScript",                  0,         ZTID_FLOAT,   LWPNSCRIPT,                0,  { ZTID_LWPN },{} },
	{ "setScript",                  0,          ZTID_VOID,   LWPNSCRIPT,                0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getWeapon",                  0,         ZTID_FLOAT,   LWPNUSEWEAPON,             0,  { ZTID_LWPN },{} },
	{ "setWeapon",                  0,          ZTID_VOID,   LWPNUSEWEAPON,             0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getDefense",                 0,         ZTID_FLOAT,   LWPNUSEDEFENCE,            0,  { ZTID_LWPN },{} },
	{ "setDefense",                 0,          ZTID_VOID,   LWPNUSEDEFENCE,            0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getDefence",                 0,         ZTID_FLOAT,   LWPNUSEDEFENCE,            0,  { ZTID_LWPN },{} },
	{ "setDefence",                 0,          ZTID_VOID,   LWPNUSEDEFENCE,            0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getInitD[]",                 0,       ZTID_UNTYPED,   LWPNINITD,                 0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "setInitD[]",                 0,          ZTID_VOID,   LWPNINITD,                 0,  { ZTID_LWPN, ZTID_FLOAT, ZTID_UNTYPED },{} },
	
	{ "Explode",                    0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getRotation",                0,         ZTID_FLOAT,   LWPNROTATION,              0,  { ZTID_LWPN },{} },
	{ "setRotation",                0,          ZTID_VOID,   LWPNROTATION,              0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getScale",                   0,         ZTID_FLOAT,   LWPNSCALE,                 0,  { ZTID_LWPN },{} },
	{ "setScale",                   0,          ZTID_VOID,   LWPNSCALE,                 0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getGravity",                 0,          ZTID_BOOL,   LWPNGRAVITY,               0,  { ZTID_LWPN },{} },
	{ "setGravity",                 0,          ZTID_VOID,   LWPNGRAVITY,               0,  { ZTID_LWPN, ZTID_BOOL },{} },
	{ "getFalling",                 0,         ZTID_FLOAT,   LWPNFALLCLK,               0,  { ZTID_LWPN },{} },
	{ "setFalling",                 0,          ZTID_VOID,   LWPNFALLCLK,               0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getFallCombo",               0,         ZTID_FLOAT,   LWPNFALLCMB,               0,  { ZTID_LWPN },{} },
	{ "setFallCombo",               0,          ZTID_VOID,   LWPNFALLCMB,               0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getMoveFlags[]",             0,          ZTID_BOOL,   LWPNMOVEFLAGS,             0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "setMoveFlags[]",             0,          ZTID_VOID,   LWPNMOVEFLAGS,             0,  { ZTID_LWPN, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getLightRadius",             0,         ZTID_FLOAT,   LWPNGLOWRAD,               0,  { ZTID_LWPN },{} },
	{ "setLightRadius",             0,          ZTID_VOID,   LWPNGLOWRAD,               0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getLightShape",              0,         ZTID_FLOAT,   LWPNGLOWSHP,               0,  { ZTID_LWPN },{} },
	{ "setLightShape",              0,          ZTID_VOID,   LWPNGLOWSHP,               0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getUnblockable",             0,         ZTID_FLOAT,   LWPNUNBL,                  0,  { ZTID_LWPN },{} },
	{ "setUnblockable",             0,          ZTID_VOID,   LWPNUNBL,                  0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "Remove",                     0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_LWPN },{} },
	{ "getShadowSprite",            0,         ZTID_FLOAT,   LWPNSHADOWSPR,             0,  { ZTID_LWPN },{} },
	{ "setShadowSprite",            0,          ZTID_VOID,   LWPNSHADOWSPR,             0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getSwitchHooked",            0,          ZTID_BOOL,   LWSWHOOKED,                0,  { ZTID_LWPN },{} },
	{ "setSwitchHooked",            0,          ZTID_VOID,   LWSWHOOKED,                0,  { ZTID_LWPN, ZTID_BOOL },{} },
	{ "Switch",                     0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getDrowning",                0,         ZTID_FLOAT,   LWPNDROWNCLK,              0,  { ZTID_LWPN },{} },
	{ "setDrowning",                0,          ZTID_VOID,   LWPNDROWNCLK,              0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getDrownCombo",              0,         ZTID_FLOAT,   LWPNDROWNCMB,              0,  { ZTID_LWPN },{} },
	{ "setDrownCombo",              0,          ZTID_VOID,   LWPNDROWNCMB,              0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getFakeZ",                   0,         ZTID_FLOAT,   LWPNFAKEZ,                 0,  { ZTID_LWPN },{} },
	{ "setFakeZ",                   0,          ZTID_VOID,   LWPNFAKEZ,                 0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getFakeJump",                0,         ZTID_FLOAT,   LWPNFAKEJUMP,              0,  { ZTID_LWPN },{} },
	{ "setFakeJump",                0,          ZTID_VOID,   LWPNFAKEJUMP,              0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getShadowXOffset",           0,         ZTID_FLOAT,   LWPNSHADOWXOFS,            0,  { ZTID_LWPN },{} },
	{ "setShadowXOffset",           0,          ZTID_VOID,   LWPNSHADOWXOFS,            0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getShadowYOffset",           0,         ZTID_FLOAT,   LWPNSHADOWYOFS,            0,  { ZTID_LWPN },{} },
	{ "setShadowYOffset",           0,          ZTID_VOID,   LWPNSHADOWYOFS,            0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getDegAngle",                0,         ZTID_FLOAT,   LWPNDEGANGLE,              0,  { ZTID_LWPN },{} },
	{ "setDegAngle",                0,          ZTID_VOID,   LWPNDEGANGLE,              0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getVx",                      0,         ZTID_FLOAT,   LWPNVX,                    0,  { ZTID_LWPN },{} },
	{ "setVx",                      0,          ZTID_VOID,   LWPNVX,                    0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getVy",                      0,         ZTID_FLOAT,   LWPNVY,                    0,  { ZTID_LWPN },{} },
	{ "setVy",                      0,          ZTID_VOID,   LWPNVY,                    0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "getAutoRotate",              0,          ZTID_BOOL,   LWPNAUTOROTATE,            0,  { ZTID_LWPN },{} },
	{ "setAutoRotate",              0,          ZTID_VOID,   LWPNAUTOROTATE,            0,  { ZTID_LWPN, ZTID_BOOL },{} },
	{ "MakeAngular",                0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_LWPN },{} },
	{ "MakeDirectional",            0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_LWPN },{} },
	{ "getFlags[]",                 0,          ZTID_BOOL,   LWPNFLAGS,                 0,  { ZTID_LWPN, ZTID_FLOAT },{} },
	{ "setFlags[]",                 0,          ZTID_VOID,   LWPNFLAGS,                 0,  { ZTID_LWPN, ZTID_FLOAT, ZTID_BOOL },{} },
	
	//Intentionally undocumented
	{ "getUID",                     0,         ZTID_FLOAT,   LWEAPONSCRIPTUID,    FL_DEPR,  { ZTID_LWPN },{} },
	
	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
};

HeroWeaponSymbols::HeroWeaponSymbols()
{
	table = lwpnTable;
	refVar = REFLWPN;
}

void HeroWeaponSymbols::generateCode()
{
	//bool isValid(lweapon)
	{
		Function* function = getFunction("isValid");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Check validity
		addOpcode2 (code, new OIsValidLWpn(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void Explode(lweapon, int32_t)
	{
		Function* function = getFunction("Explode");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OLWeaponExplodeRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void UseSprite(lweapon, int32_t val)
	{
		Function* function = getFunction("UseSprite");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the val
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OSetRegister(new VarArgument(refVar), new VarArgument(EXP2)));
		addOpcode2 (code, new OUseSpriteLWpn(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void Remove(lweapon)
	{
		Function* function = getFunction("Remove");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		POPREF();
		LABELBACK(label);
		//Break shield
		addOpcode2 (code, new OLWpnRemove());
		RETURN();
		function->giveCode(code);
	}
	//bool Switch(lweapon, int)
	{
		Function* function = getFunction("Switch");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSwitchLW(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void MakeAngular(lweapon)
	{
		Function* function = getFunction("MakeAngular");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Check validity
		addOpcode2 (code, new OMakeAngularLwpn(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void MakeDirectional(lweapon)
	{
		Function* function = getFunction("MakeDirectional");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Check validity
		addOpcode2 (code, new OMakeDirectionalLwpn(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
}

