#include "SymbolDefs.h"

EnemyWeaponSymbols EnemyWeaponSymbols::singleton = EnemyWeaponSymbols();

static AccessorTable ewpnTable[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
	{ "getLevel",                   0,         ZTID_FLOAT,   EWPNLEVEL,                 0,  { ZTID_EWPN },{} },
	{ "setLevel",                   0,          ZTID_VOID,   EWPNLEVEL,                 0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getX",                       0,         ZTID_FLOAT,   EWPNX,                     0,  { ZTID_EWPN },{} },
	{ "setX",                       0,          ZTID_VOID,   EWPNX,                     0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "_getMax",                    0,         ZTID_FLOAT,   SPRITEMAXEWPN,             0,  { ZTID_EWPN },{} },
	{ "_setMax",                    0,          ZTID_VOID,   SPRITEMAXEWPN,             0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getY",                       0,         ZTID_FLOAT,   EWPNY,                     0,  { ZTID_EWPN },{} },
	{ "setY",                       0,          ZTID_VOID,   EWPNY,                     0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getZ",                       0,         ZTID_FLOAT,   EWPNZ,                     0,  { ZTID_EWPN },{} },
	{ "setZ",                       0,          ZTID_VOID,   EWPNZ,                     0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getJump",                    0,         ZTID_FLOAT,   EWPNJUMP,                  0,  { ZTID_EWPN },{} },
	{ "setJump",                    0,          ZTID_VOID,   EWPNJUMP,                  0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getDir",                     0,         ZTID_FLOAT,   EWPNDIR,                   0,  { ZTID_EWPN },{} },
	{ "setDir",                     0,          ZTID_VOID,   EWPNDIR,                   0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getAngle",                   0,         ZTID_FLOAT,   EWPNANGLE,                 0,  { ZTID_EWPN },{} },
	{ "setAngle",                   0,          ZTID_VOID,   EWPNANGLE,                 0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getStep",                    0,         ZTID_FLOAT,   EWPNSTEP,                  0,  { ZTID_EWPN },{} },
	{ "setStep",                    0,          ZTID_VOID,   EWPNSTEP,                  0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getNumFrames",               0,         ZTID_FLOAT,   EWPNFRAMES,                0,  { ZTID_EWPN },{} },
	{ "setNumFrames",               0,          ZTID_VOID,   EWPNFRAMES,                0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getFrame",                   0,         ZTID_FLOAT,   EWPNFRAME,                 0,  { ZTID_EWPN },{} },
	{ "setFrame",                   0,          ZTID_VOID,   EWPNFRAME,                 0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getDrawStyle",               0,         ZTID_FLOAT,   EWPNDRAWTYPE,              0,  { ZTID_EWPN },{} },
	{ "setDrawStyle",               0,          ZTID_VOID,   EWPNDRAWTYPE,              0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getPower",                   0,         ZTID_FLOAT,   EWPNPOWER,                 0,  { ZTID_EWPN },{} },
	{ "setPower",                   0,          ZTID_VOID,   EWPNPOWER,                 0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getDamage",                  0,         ZTID_FLOAT,   EWPNPOWER,                 0,  { ZTID_EWPN },{} },
	{ "setDamage",                  0,          ZTID_VOID,   EWPNPOWER,                 0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getID",                      0,         ZTID_FLOAT,   EWPNID,                    0,  { ZTID_EWPN },{} },
	{ "setID",                      0,          ZTID_VOID,   EWPNID,                    0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getType",                    0,         ZTID_FLOAT,   EWPNID,                    0,  { ZTID_EWPN },{} },
	{ "setType",                    0,          ZTID_VOID,   EWPNID,                    0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getFamily",                  0,         ZTID_FLOAT,   EWPNID,                    0,  { ZTID_EWPN },{} },
	{ "setFamily",                  0,          ZTID_VOID,   EWPNID,                    0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getAngular",                 0,          ZTID_BOOL,   EWPNANGULAR,               0,  { ZTID_EWPN },{} },
	{ "setAngular",                 0,          ZTID_VOID,   EWPNANGULAR,               0,  { ZTID_EWPN, ZTID_BOOL },{} },
	{ "getBehind",                  0,          ZTID_BOOL,   EWPNBEHIND,                0,  { ZTID_EWPN },{} },
	{ "setBehind",                  0,          ZTID_VOID,   EWPNBEHIND,                0,  { ZTID_EWPN, ZTID_BOOL },{} },
	{ "getASpeed",                  0,         ZTID_FLOAT,   EWPNASPEED,                0,  { ZTID_EWPN },{} },
	{ "setASpeed",                  0,          ZTID_VOID,   EWPNASPEED,                0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getTile",                    0,         ZTID_FLOAT,   EWPNTILE,                  0,  { ZTID_EWPN },{} },
	{ "setTile",                    0,          ZTID_VOID,   EWPNTILE,                  0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getScriptTile",              0,         ZTID_FLOAT,   EWPNSCRIPTTILE,            0,  { ZTID_EWPN },{} },
	{ "setScriptTile",              0,          ZTID_VOID,   EWPNSCRIPTTILE,            0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getScriptFlip",              0,         ZTID_FLOAT,   EWPNSCRIPTFLIP,            0,  { ZTID_EWPN },{} },
	{ "setScriptFlip",              0,          ZTID_VOID,   EWPNSCRIPTFLIP,            0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getFlashCSet",               0,         ZTID_FLOAT,   EWPNFLASHCSET,             0,  { ZTID_EWPN },{} },
	{ "setFlashCSet",               0,          ZTID_VOID,   EWPNFLASHCSET,             0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getDeadState",               0,         ZTID_FLOAT,   EWPNDEAD,                  0,  { ZTID_EWPN },{} },
	{ "setDeadState",               0,          ZTID_VOID,   EWPNDEAD,                  0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getCSet",                    0,         ZTID_FLOAT,   EWPNCSET,                  0,  { ZTID_EWPN },{} },
	{ "setCSet",                    0,          ZTID_VOID,   EWPNCSET,                  0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getFlash",                   0,          ZTID_BOOL,   EWPNFLASH,                 0,  { ZTID_EWPN },{} },
	{ "setFlash",                   0,          ZTID_VOID,   EWPNFLASH,                 0,  { ZTID_EWPN, ZTID_BOOL },{} },
	{ "getFlip",                    0,         ZTID_FLOAT,   EWPNFLIP,                  0,  { ZTID_EWPN },{} },
	{ "setFlip",                    0,          ZTID_VOID,   EWPNFLIP,                  0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getOriginalTile",            0,         ZTID_FLOAT,   EWPNOTILE,                 0,  { ZTID_EWPN },{} },
	{ "setOriginalTile",            0,          ZTID_VOID,   EWPNOTILE,                 0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getOriginalCSet",            0,         ZTID_FLOAT,   EWPNOCSET,                 0,  { ZTID_EWPN },{} },
	{ "setOriginalCSet",            0,          ZTID_VOID,   EWPNOCSET,                 0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getExtend",                  0,         ZTID_FLOAT,   EWPNEXTEND,                0,  { ZTID_EWPN },{} },
	{ "setExtend",                  0,          ZTID_VOID,   EWPNEXTEND,                0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getHitWidth",                0,         ZTID_FLOAT,   EWPNHXSZ,                  0,  { ZTID_EWPN },{} },
	{ "setHitWidth",                0,          ZTID_VOID,   EWPNHXSZ,                  0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getHitHeight",               0,         ZTID_FLOAT,   EWPNHYSZ,                  0,  { ZTID_EWPN },{} },
	{ "setHitHeight",               0,          ZTID_VOID,   EWPNHYSZ,                  0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getHitZHeight",              0,         ZTID_FLOAT,   EWPNHZSZ,                  0,  { ZTID_EWPN },{} },
	{ "setHitZHeight",              0,          ZTID_VOID,   EWPNHZSZ,                  0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getTileWidth",               0,         ZTID_FLOAT,   EWPNTXSZ,                  0,  { ZTID_EWPN },{} },
	{ "setTileWidth",               0,          ZTID_VOID,   EWPNTXSZ,                  0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getTileHeight",              0,         ZTID_FLOAT,   EWPNTYSZ,                  0,  { ZTID_EWPN },{} },
	{ "setTileHeight",              0,          ZTID_VOID,   EWPNTYSZ,                  0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getDrawXOffset",             0,         ZTID_FLOAT,   EWPNXOFS,                  0,  { ZTID_EWPN },{} },
	{ "setDrawXOffset",             0,          ZTID_VOID,   EWPNXOFS,                  0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getDrawYOffset",             0,         ZTID_FLOAT,   EWPNYOFS,                  0,  { ZTID_EWPN },{} },
	{ "setDrawYOffset",             0,          ZTID_VOID,   EWPNYOFS,                  0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getTotalDYOffset",           0,         ZTID_FLOAT,   EWPNTOTALDYOFFS,           0,  { ZTID_EWPN },{} },
	{ "setTotalDYOffset",           0,          ZTID_VOID,   EWPNTOTALDYOFFS,           0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getDrawZOffset",             0,         ZTID_FLOAT,   EWPNZOFS,                  0,  { ZTID_EWPN },{} },
	{ "setDrawZOffset",             0,          ZTID_VOID,   EWPNZOFS,                  0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getHitXOffset",              0,         ZTID_FLOAT,   EWPNHXOFS,                 0,  { ZTID_EWPN },{} },
	{ "setHitXOffset",              0,          ZTID_VOID,   EWPNHXOFS,                 0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getHitYOffset",              0,         ZTID_FLOAT,   EWPNHYOFS,                 0,  { ZTID_EWPN },{} },
	{ "setHitYOffset",              0,          ZTID_VOID,   EWPNHYOFS,                 0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getParent",                  0,           ZTID_NPC,   EWPNPARENT,                0,  { ZTID_EWPN },{} },
	{ "setParent",                  0,          ZTID_VOID,   EWPNPARENT,                0,  { ZTID_EWPN, ZTID_NPC },{} },
	{ "isValid",                    0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_EWPN },{} },
	{ "UseSprite",                  0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getMisc[]",                  0,       ZTID_UNTYPED,   EWPNMISCD,                 0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "setMisc[]",                  0,          ZTID_VOID,   EWPNMISCD,                 0,  { ZTID_EWPN, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getCollDetection",           0,         ZTID_FLOAT,   EWPNCOLLDET,               0,  { ZTID_EWPN },{} },
	{ "setCollDetection",           0,          ZTID_VOID,   EWPNCOLLDET,               0,  { ZTID_EWPN, ZTID_BOOL },{} },
	{ "getAnimation",               0,         ZTID_FLOAT,   EWPNENGINEANIMATE,         0,  { ZTID_EWPN },{} },
	{ "setAnimation",               0,          ZTID_VOID,   EWPNENGINEANIMATE,         0,  { ZTID_EWPN, ZTID_BOOL },{} },
	{ "getScript",                  0,         ZTID_FLOAT,   EWPNSCRIPT,                0,  { ZTID_EWPN },{} },
	{ "setScript",                  0,          ZTID_VOID,   EWPNSCRIPT,                0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getInitD[]",                 0,       ZTID_UNTYPED,   EWPNINITD,                 0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "setInitD[]",                 0,          ZTID_VOID,   EWPNINITD,                 0,  { ZTID_EWPN, ZTID_FLOAT, ZTID_UNTYPED },{} },
	
	{ "Explode",                    0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getRotation",                0,         ZTID_FLOAT,   EWPNROTATION,              0,  { ZTID_EWPN },{} },
	{ "setRotation",                0,          ZTID_VOID,   EWPNROTATION,              0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getScale",                   0,         ZTID_FLOAT,   EWPNSCALE,                 0,  { ZTID_EWPN },{} },
	{ "setScale",                   0,          ZTID_VOID,   EWPNSCALE,                 0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getGravity",                 0,          ZTID_BOOL,   EWPNGRAVITY,               0,  { ZTID_EWPN },{} },
	{ "setGravity",                 0,          ZTID_VOID,   EWPNGRAVITY,               0,  { ZTID_EWPN, ZTID_BOOL },{} },
	{ "getFalling",                 0,         ZTID_FLOAT,   EWPNFALLCLK,               0,  { ZTID_EWPN },{} },
	{ "setFalling",                 0,          ZTID_VOID,   EWPNFALLCLK,               0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getFallCombo",               0,         ZTID_FLOAT,   EWPNFALLCMB,               0,  { ZTID_EWPN },{} },
	{ "setFallCombo",               0,          ZTID_VOID,   EWPNFALLCMB,               0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getMoveFlags[]",             0,          ZTID_BOOL,   EWPNMOVEFLAGS,             0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "setMoveFlags[]",             0,          ZTID_VOID,   EWPNMOVEFLAGS,             0,  { ZTID_EWPN, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getLightRadius",             0,         ZTID_FLOAT,   EWPNGLOWRAD,               0,  { ZTID_EWPN },{} },
	{ "setLightRadius",             0,          ZTID_VOID,   EWPNGLOWRAD,               0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getLightShape",              0,         ZTID_FLOAT,   EWPNGLOWSHP,               0,  { ZTID_EWPN },{} },
	{ "setLightShape",              0,          ZTID_VOID,   EWPNGLOWSHP,               0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getUnblockable",             0,         ZTID_FLOAT,   EWPNUNBL,                  0,  { ZTID_EWPN },{} },
	{ "setUnblockable",             0,          ZTID_VOID,   EWPNUNBL,                  0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "Remove",                     0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_EWPN },{} },
	{ "getShadowSprite",            0,         ZTID_FLOAT,   EWPNSHADOWSPR,             0,  { ZTID_EWPN },{} },
	{ "setShadowSprite",            0,          ZTID_VOID,   EWPNSHADOWSPR,             0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getSwitchHooked",            0,          ZTID_BOOL,   EWSWHOOKED,                0,  { ZTID_EWPN },{} },
	{ "setSwitchHooked",            0,          ZTID_VOID,   EWSWHOOKED,                0,  { ZTID_EWPN, ZTID_BOOL },{} },
	{ "getTimeout",                 0,         ZTID_FLOAT,   EWPNTIMEOUT,               0,  { ZTID_EWPN },{} },
	{ "setTimeout",                 0,          ZTID_VOID,   EWPNTIMEOUT,               0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "Switch",                     0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getDrowning",                0,         ZTID_FLOAT,   EWPNDROWNCLK,              0,  { ZTID_EWPN },{} },
	{ "setDrowning",                0,          ZTID_VOID,   EWPNDROWNCLK,              0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getDrownCombo",              0,         ZTID_FLOAT,   EWPNDROWNCMB,              0,  { ZTID_EWPN },{} },
	{ "setDrownCombo",              0,          ZTID_VOID,   EWPNDROWNCMB,              0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getFakeZ",                   0,         ZTID_FLOAT,   EWPNFAKEZ,                 0,  { ZTID_EWPN },{} },
	{ "setFakeZ",                   0,          ZTID_VOID,   EWPNFAKEZ,                 0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getFakeJump",                0,         ZTID_FLOAT,   EWPNFAKEJUMP,              0,  { ZTID_EWPN },{} },
	{ "setFakeJump",                0,          ZTID_VOID,   EWPNFAKEJUMP,              0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getShadowXOffset",           0,         ZTID_FLOAT,   EWPNSHADOWXOFS,            0,  { ZTID_EWPN },{} },
	{ "setShadowXOffset",           0,          ZTID_VOID,   EWPNSHADOWXOFS,            0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getShadowYOffset",           0,         ZTID_FLOAT,   EWPNSHADOWYOFS,            0,  { ZTID_EWPN },{} },
	{ "setShadowYOffset",           0,          ZTID_VOID,   EWPNSHADOWYOFS,            0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getDegAngle",                0,         ZTID_FLOAT,   EWPNDEGANGLE,              0,  { ZTID_EWPN },{} },
	{ "setDegAngle",                0,          ZTID_VOID,   EWPNDEGANGLE,              0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getVx",                      0,         ZTID_FLOAT,   EWPNVX,                    0,  { ZTID_EWPN },{} },
	{ "setVx",                      0,          ZTID_VOID,   EWPNVX,                    0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getVy",                      0,         ZTID_FLOAT,   EWPNVY,                    0,  { ZTID_EWPN },{} },
	{ "setVy",                      0,          ZTID_VOID,   EWPNVY,                    0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "getAutoRotate",              0,          ZTID_BOOL,   EWPNAUTOROTATE,            0,  { ZTID_EWPN },{} },
	{ "setAutoRotate",              0,          ZTID_VOID,   EWPNAUTOROTATE,            0,  { ZTID_EWPN, ZTID_BOOL },{} },
	{ "MakeAngular",                0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_EWPN },{} },
	{ "MakeDirectional",            0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_EWPN },{} },
	{ "getFlags[]",                 0,          ZTID_BOOL,   EWPNFLAGS,                 0,  { ZTID_EWPN, ZTID_FLOAT },{} },
	{ "setFlags[]",                 0,          ZTID_VOID,   EWPNFLAGS,                 0,  { ZTID_EWPN, ZTID_FLOAT, ZTID_BOOL },{} },
	
	//Intentionally undocumented
	{ "getUID",                     0,         ZTID_FLOAT,   EWEAPONSCRIPTUID,    FL_DEPR,  { ZTID_EWPN },{} },
	{ "getParentUID",               0,         ZTID_FLOAT,   EWPNPARENTUID,       FL_DEPR,  { ZTID_EWPN },{} },
	{ "setParentUID",               0,          ZTID_VOID,   EWPNPARENTUID,       FL_DEPR,  { ZTID_EWPN, ZTID_FLOAT },{} },
	
	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
};

EnemyWeaponSymbols::EnemyWeaponSymbols()
{
	table = ewpnTable;
	refVar = REFEWPN;
}

void EnemyWeaponSymbols::generateCode()
{
	//bool isValid(eweapon)
	{
		Function* function = getFunction("isValid");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Check validity
		addOpcode2 (code, new OIsValidEWpn(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void Explode(eweapon, int32_t)
	{
		Function* function = getFunction("Explode");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OEWeaponExplodeRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void UseSprite(eweapon, int32_t val)
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
		addOpcode2 (code, new OUseSpriteEWpn(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void Remove(eweapon)
	{
		Function* function = getFunction("Remove");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		POPREF();
		LABELBACK(label);
		//Break shield
		addOpcode2 (code, new OEWpnRemove());
		RETURN();
		function->giveCode(code);
	}
	//bool Switch(eweapon, int)
	{
		Function* function = getFunction("Switch");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSwitchEW(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//bool MakeAngular(eweapon)
	{
		Function* function = getFunction("MakeAngular");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Check validity
		addOpcode2 (code, new OMakeAngularEwpn(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//bool MakeDirectional(eweapon)
	{
		Function* function = getFunction("MakeAngular");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Check validity
		addOpcode2 (code, new OMakeDirectionalEwpn(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
}

