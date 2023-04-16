#include "SymbolDefs.h"

NPCSymbols NPCSymbols::singleton = NPCSymbols();

static AccessorTable npcTable[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
	{ "getX",                       0,         ZTID_FLOAT,   NPCX,                      0,  { ZTID_NPC },{} },
	{ "setX",                       0,          ZTID_VOID,   NPCX,                      0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getFrame",                   0,         ZTID_FLOAT,   NPCFRAME,                  0,  { ZTID_NPC },{} },
	{ "setFrame",                   0,          ZTID_VOID,   NPCFRAME,                  0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "_getMax",                    0,         ZTID_FLOAT,   SPRITEMAXNPC,              0,  { ZTID_NPC },{} },
	{ "_setMax",                    0,          ZTID_VOID,   SPRITEMAXNPC,              0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getY",                       0,         ZTID_FLOAT,   NPCY,                      0,  { ZTID_NPC },{} },
	{ "setY",                       0,          ZTID_VOID,   NPCY,                      0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getZ",                       0,         ZTID_FLOAT,   NPCZ,                      0,  { ZTID_NPC },{} },
	{ "setZ",                       0,          ZTID_VOID,   NPCZ,                      0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getJump",                    0,         ZTID_FLOAT,   NPCJUMP,                   0,  { ZTID_NPC },{} },
	{ "setJump",                    0,          ZTID_VOID,   NPCJUMP,                   0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getDir",                     0,         ZTID_FLOAT,   NPCDIR,                    0,  { ZTID_NPC },{} },
	{ "setDir",                     0,          ZTID_VOID,   NPCDIR,                    0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getSlideClock",              0,         ZTID_FLOAT,   NPCSLIDECLK,               0,  { ZTID_NPC },{} },
	{ "setSlideClock",              0,          ZTID_VOID,   NPCSLIDECLK,               0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getFading",                  0,         ZTID_FLOAT,   NPCFADING,                 0,  { ZTID_NPC },{} },
	{ "setFading",                  0,          ZTID_VOID,   NPCFADING,                 0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getRate",                    0,         ZTID_FLOAT,   NPCRATE,                   0,  { ZTID_NPC },{} },
	{ "setRate",                    0,          ZTID_VOID,   NPCRATE,                   0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getHoming",                  0,         ZTID_FLOAT,   NPCHOMING,                 0,  { ZTID_NPC },{} },
	{ "setHoming",                  0,          ZTID_VOID,   NPCHOMING,                 0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getStep",                    0,         ZTID_FLOAT,   NPCSTEP,                   0,  { ZTID_NPC },{} },
	{ "setStep",                    0,          ZTID_VOID,   NPCSTEP,                   0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getASpeed",                  0,         ZTID_FLOAT,   NPCFRAMERATE,              0,  { ZTID_NPC },{} },
	{ "setASpeed",                  0,          ZTID_VOID,   NPCFRAMERATE,              0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getHalt",                    0,         ZTID_FLOAT,   NPCHALTCLK,                0,  { ZTID_NPC },{} },
	{ "setHalt",                    0,          ZTID_VOID,   NPCHALTCLK,                0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getMoveStatus",              0,         ZTID_FLOAT,   NPCMOVESTATUS,             0,  { ZTID_NPC },{} },
	{ "setMoveStatus",              0,          ZTID_VOID,   NPCMOVESTATUS,             0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getHaltrate",                0,         ZTID_FLOAT,   NPCHALTRATE,               0,  { ZTID_NPC },{} },
	{ "setHaltrate",                0,          ZTID_VOID,   NPCHALTRATE,               0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getRandom",                  0,         ZTID_FLOAT,   NPCRANDOM,                 0,  { ZTID_NPC },{} },
	{ "setRandom",                  0,          ZTID_VOID,   NPCRANDOM,                 0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getDrawStyle",               0,         ZTID_FLOAT,   NPCDRAWTYPE,               0,  { ZTID_NPC },{} },
	{ "setDrawStyle",               0,          ZTID_VOID,   NPCDRAWTYPE,               0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getHP",                      0,         ZTID_FLOAT,   NPCHP,                     0,  { ZTID_NPC },{} },
	{ "setHP",                      0,          ZTID_VOID,   NPCHP,                     0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getID",                      0,         ZTID_FLOAT,   NPCID,                     0,  { ZTID_NPC },{} },
	{ "setID",                      0,          ZTID_VOID,   NPCID,                     0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getType",                    0,         ZTID_FLOAT,   NPCTYPE,                   0,  { ZTID_NPC },{} },
	{ "setType",                    0,          ZTID_VOID,   NPCTYPE,                   0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getFamily",                  0,         ZTID_FLOAT,   NPCTYPE,                   0,  { ZTID_NPC },{} },
	{ "setFamily",                  0,          ZTID_VOID,   NPCTYPE,                   0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getDamage",                  0,         ZTID_FLOAT,   NPCDP,                     0,  { ZTID_NPC },{} },
	{ "setDamage",                  0,          ZTID_VOID,   NPCDP,                     0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getWeaponDamage",            0,         ZTID_FLOAT,   NPCWDP,                    0,  { ZTID_NPC },{} },
	{ "setWeaponDamage",            0,          ZTID_VOID,   NPCWDP,                    0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getTile",                    0,         ZTID_FLOAT,   NPCTILE,                   0,  { ZTID_NPC },{} },
	{ "setTile",                    0,          ZTID_VOID,   NPCTILE,                   0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getScriptTile",              0,         ZTID_FLOAT,   NPCSCRIPTTILE,             0,  { ZTID_NPC },{} },
	{ "setScriptTile",              0,          ZTID_VOID,   NPCSCRIPTTILE,             0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getScriptFlip",              0,         ZTID_FLOAT,   NPCSCRIPTFLIP,             0,  { ZTID_NPC },{} },
	{ "setScriptFlip",              0,          ZTID_VOID,   NPCSCRIPTFLIP,             0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getOriginalTile",            0,         ZTID_FLOAT,   NPCOTILE,                  0,  { ZTID_NPC },{} },
	{ "setOriginalTile",            0,          ZTID_VOID,   NPCOTILE,                  0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getWeapon",                  0,         ZTID_FLOAT,   NPCWEAPON,                 0,  { ZTID_NPC },{} },
	{ "setWeapon",                  0,          ZTID_VOID,   NPCWEAPON,                 0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getItemSet",                 0,         ZTID_FLOAT,   NPCITEMSET,                0,  { ZTID_NPC },{} },
	{ "setItemSet",                 0,          ZTID_VOID,   NPCITEMSET,                0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getDropset",                 0,         ZTID_FLOAT,   NPCITEMSET,                0,  { ZTID_NPC },{} },
	{ "setDropset",                 0,          ZTID_VOID,   NPCITEMSET,                0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getCSet",                    0,         ZTID_FLOAT,   NPCCSET,                   0,  { ZTID_NPC },{} },
	{ "setCSet",                    0,          ZTID_VOID,   NPCCSET,                   0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getBossPal",                 0,         ZTID_FLOAT,   NPCBOSSPAL,                0,  { ZTID_NPC },{} },
	{ "setBossPal",                 0,          ZTID_VOID,   NPCBOSSPAL,                0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getSFX",                     0,         ZTID_FLOAT,   NPCBGSFX,                  0,  { ZTID_NPC },{} },
	{ "setSFX",                     0,          ZTID_VOID,   NPCBGSFX,                  0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getExtend",                  0,         ZTID_FLOAT,   NPCEXTEND,                 0,  { ZTID_NPC },{} },
	{ "setExtend",                  0,          ZTID_VOID,   NPCEXTEND,                 0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getHitWidth",                0,         ZTID_FLOAT,   NPCHXSZ,                   0,  { ZTID_NPC },{} },
	{ "setHitWidth",                0,          ZTID_VOID,   NPCHXSZ,                   0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getHitHeight",               0,         ZTID_FLOAT,   NPCHYSZ,                   0,  { ZTID_NPC },{} },
	{ "setHitHeight",               0,          ZTID_VOID,   NPCHYSZ,                   0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getHitZHeight",              0,         ZTID_FLOAT,   NPCHZSZ,                   0,  { ZTID_NPC },{} },
	{ "setHitZHeight",              0,          ZTID_VOID,   NPCHZSZ,                   0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getTileWidth",               0,         ZTID_FLOAT,   NPCTXSZ,                   0,  { ZTID_NPC },{} },
	{ "setTileWidth",               0,          ZTID_VOID,   NPCTXSZ,                   0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getTileHeight",              0,         ZTID_FLOAT,   NPCTYSZ,                   0,  { ZTID_NPC },{} },
	{ "setTileHeight",              0,          ZTID_VOID,   NPCTYSZ,                   0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getDrawXOffset",             0,         ZTID_FLOAT,   NPCXOFS,                   0,  { ZTID_NPC },{} },
	{ "setDrawXOffset",             0,          ZTID_VOID,   NPCXOFS,                   0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getDrawYOffset",             0,         ZTID_FLOAT,   NPCYOFS,                   0,  { ZTID_NPC },{} },
	{ "setDrawYOffset",             0,          ZTID_VOID,   NPCYOFS,                   0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getTotalDYOffset",           0,         ZTID_FLOAT,   NPCTOTALDYOFFS,            0,  { ZTID_NPC },{} },
	{ "setTotalDYOffset",           0,          ZTID_VOID,   NPCTOTALDYOFFS,            0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getDrawZOffset",             0,         ZTID_FLOAT,   NPCZOFS,                   0,  { ZTID_NPC },{} },
	{ "setDrawZOffset",             0,          ZTID_VOID,   NPCZOFS,                   0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getHitXOffset",              0,         ZTID_FLOAT,   NPCHXOFS,                  0,  { ZTID_NPC },{} },
	{ "setHitXOffset",              0,          ZTID_VOID,   NPCHXOFS,                  0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getHitYOffset",              0,         ZTID_FLOAT,   NPCHYOFS,                  0,  { ZTID_NPC },{} },
	{ "setHitYOffset",              0,          ZTID_VOID,   NPCHYOFS,                  0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "isValid",                    0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_NPC },{} },
	{ "Explode",                    0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_NPC, ZTID_FLOAT },{} },
	
	{ "getMisc[]",                  0,       ZTID_UNTYPED,   NPCMISCD,                  0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "setMisc[]",                  0,          ZTID_VOID,   NPCMISCD,                  0,  { ZTID_NPC, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getInitD[]",                 0,       ZTID_UNTYPED,   NPCINITD,                  0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "setInitD[]",                 0,          ZTID_VOID,   NPCINITD,                  0,  { ZTID_NPC, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getAttributes[]",            0,       ZTID_UNTYPED,   NPCDD,                     0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "setAttributes[]",            0,          ZTID_VOID,   NPCDD,                     0,  { ZTID_NPC, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getMiscFlags",               0,         ZTID_FLOAT,   NPCMFLAGS,                 0,  { ZTID_NPC },{} },
	{ "setMiscFlags",               0,          ZTID_VOID,   NPCMFLAGS,                 0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getCollDetection",           0,          ZTID_BOOL,   NPCCOLLDET,                0,  { ZTID_NPC },{} },
	{ "setCollDetection",           0,          ZTID_VOID,   NPCCOLLDET,                0,  { ZTID_NPC, ZTID_BOOL },{} },
	{ "getGravity",                 0,          ZTID_BOOL,   NPCGRAVITY,                0,  { ZTID_NPC },{} },
	{ "setGravity",                 0,          ZTID_VOID,   NPCGRAVITY,                0,  { ZTID_NPC, ZTID_BOOL },{} },
	{ "getAnimation",               0,          ZTID_BOOL,   NPCENGINEANIMATE,          0,  { ZTID_NPC },{} },
	{ "setAnimation",               0,          ZTID_VOID,   NPCENGINEANIMATE,          0,  { ZTID_NPC, ZTID_BOOL },{} },
	{ "_getSubmerged",              0,          ZTID_BOOL,   NPCSUBMERGED,              0,  { ZTID_NPC },{} },
	{ "getStun",                    0,         ZTID_FLOAT,   NPCSTUN,                   0,  { ZTID_NPC },{} },
	{ "setStun",                    0,          ZTID_VOID,   NPCSTUN,                   0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getCore",                    0,          ZTID_BOOL,   NPCISCORE,                 0,  { ZTID_NPC },{} },
	{ "setCore",                    0,          ZTID_VOID,   NPCISCORE,                 0,  { ZTID_NPC, ZTID_BOOL },{} },
	{ "getDefense[]",               0,         ZTID_FLOAT,   NPCDEFENSED,               0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "setDefense[]",               0,          ZTID_VOID,   NPCDEFENSED,               0,  { ZTID_NPC, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getHitBy[]",                 0,       ZTID_UNTYPED,   NPCHITBY,                  0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "setHitBy[]",                 0,          ZTID_VOID,   NPCHITBY,                  0,  { ZTID_NPC, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "GetName",                    0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getHunger",                  0,         ZTID_FLOAT,   NPCHUNGER,                 0,  { ZTID_NPC },{} },
	{ "setHunger",                  0,          ZTID_VOID,   NPCHUNGER,                 0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "BreakShield",                0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_NPC },{} },
	{ "getWeaponSprite",            0,         ZTID_FLOAT,   NPCWEAPSPRITE,             0,  { ZTID_NPC },{} },
	{ "setWeaponSprite",            0,          ZTID_VOID,   NPCWEAPSPRITE,             0,  { ZTID_NPC, ZTID_FLOAT },{} },
//	2.future srit compatibility. -Z
	{ "getScriptDefense[]",         0,         ZTID_FLOAT,   NPCSCRDEFENSED,            0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "setScriptDefense[]",         0,          ZTID_VOID,   NPCSCRDEFENSED,            0,  { ZTID_NPC, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	
	{ "getInvFrames",               0,         ZTID_FLOAT,   NPCINVINC,                 0,  { ZTID_NPC },{} },
	{ "setInvFrames",               0,          ZTID_VOID,   NPCINVINC,                 0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getInvincible",              0,         ZTID_FLOAT,   NPCSUPERMAN,               0,  { ZTID_NPC },{} },
	{ "setInvincible",              0,          ZTID_VOID,   NPCSUPERMAN,               0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getRingleader",              0,          ZTID_BOOL,   NPCRINGLEAD,               0,  { ZTID_NPC },{} },
	{ "setRingleader",              0,          ZTID_VOID,   NPCRINGLEAD,               0,  { ZTID_NPC, ZTID_BOOL },{} },
	{ "getHasItem",                 0,          ZTID_BOOL,   NPCHASITEM,                0,  { ZTID_NPC },{} },
	{ "setHasItem",                 0,          ZTID_VOID,   NPCHASITEM,                0,  { ZTID_NPC, ZTID_BOOL },{} },
	{ "getShield[]",                0,          ZTID_BOOL,   NPCSHIELD,                 0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "setShield[]",                0,          ZTID_VOID,   NPCSHIELD,                 0,  { ZTID_NPC, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getBFlags[]",                0,          ZTID_BOOL,   NPCBEHAVIOUR,              0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "setBFlags[]",                0,          ZTID_VOID,   NPCBEHAVIOUR,              0,  { ZTID_NPC, ZTID_FLOAT, ZTID_BOOL },{} },
	
	{ "getHitDir",                  0,         ZTID_FLOAT,   NPCHITDIR,                 0,  { ZTID_NPC },{} },
	{ "setHitDir",                  0,          ZTID_VOID,   NPCHITDIR,                 0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getScript",                  0,         ZTID_FLOAT,   NPCSCRIPT,                 0,  { ZTID_NPC },{} },
	{ "setScript",                  0,          ZTID_VOID,   NPCSCRIPT,                 0,  { ZTID_NPC, ZTID_FLOAT },{} },
	
	{ "isDead",                     0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_NPC },{} },
	{ "CanSlide",                   0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_NPC },{} },
	{ "Slide",                      0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_NPC },{} },
	{ "Remove",                     0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_NPC },{} },
	{ "StopBGSFX",                  0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_NPC },{} },
	{ "Attack",                     0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_NPC },{} },
	{ "NewDir",                     0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "ConstantWalk",               0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "ConstantWalk8",              0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "VariableWalk",               0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "VariableWalk8",              0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "HaltingWalk",                0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "HaltingWalk8",               0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "FloatingWalk",               0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "BreathAttack",               0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_NPC, ZTID_BOOL },{ 0 } },
	{ "NewDir8",                    0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "Collision",                  0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_NPC, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "LinedUp",                    0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_NPC, ZTID_FLOAT, ZTID_BOOL },{ 0 } },
	{ "LinkInRange",                0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "CanMove",                    0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "SimulateHit",                0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "Create",                     0,           ZTID_NPC,   -1,                   FL_INL,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getRotation",                0,         ZTID_FLOAT,   NPCROTATION,               0,  { ZTID_NPC },{} },
	{ "setRotation",                0,          ZTID_VOID,   NPCROTATION,               0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getScale",                   0,         ZTID_FLOAT,   NPCSCALE,                  0,  { ZTID_NPC },{} },
	{ "setScale",                   0,          ZTID_VOID,   NPCSCALE,                  0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getImmortal",                0,          ZTID_BOOL,   NPCIMMORTAL,               0,  { ZTID_NPC },{} },
	{ "setImmortal",                0,          ZTID_VOID,   NPCIMMORTAL,               0,  { ZTID_NPC, ZTID_BOOL },{} },
	{ "getNoSlide",                 0,          ZTID_BOOL,   NPCNOSLIDE,                0,  { ZTID_NPC },{} },
	{ "setNoSlide",                 0,          ZTID_VOID,   NPCNOSLIDE,                0,  { ZTID_NPC, ZTID_BOOL },{} },
	{ "getNoScriptKnockback",       0,          ZTID_BOOL,   NPCNOSCRIPTKB,             0,  { ZTID_NPC },{} },
	{ "setNoScriptKnockback",       0,          ZTID_VOID,   NPCNOSCRIPTKB,             0,  { ZTID_NPC, ZTID_BOOL },{} },
	{ "getSlideSpeed",              0,         ZTID_FLOAT,   NPCKNOCKBACKSPEED,         0,  { ZTID_NPC },{} },
	{ "setSlideSpeed",              0,          ZTID_VOID,   NPCKNOCKBACKSPEED,         0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "Knockback",                  0,          ZTID_BOOL,   -1,                        0,  { ZTID_NPC, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{ 40000 } },
	{ "getFalling",                 0,         ZTID_FLOAT,   NPCFALLCLK,                0,  { ZTID_NPC },{} },
	{ "setFalling",                 0,          ZTID_VOID,   NPCFALLCLK,                0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getFallCombo",               0,         ZTID_FLOAT,   NPCFALLCMB,                0,  { ZTID_NPC },{} },
	{ "setFallCombo",               0,          ZTID_VOID,   NPCFALLCMB,                0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getMoveFlags[]",             0,          ZTID_BOOL,   NPCMOVEFLAGS,              0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "setMoveFlags[]",             0,          ZTID_VOID,   NPCMOVEFLAGS,              0,  { ZTID_NPC, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getLightRadius",             0,         ZTID_FLOAT,   NPCGLOWRAD,                0,  { ZTID_NPC },{} },
	{ "setLightRadius",             0,          ZTID_VOID,   NPCGLOWRAD,                0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getLightShape",              0,         ZTID_FLOAT,   NPCGLOWSHP,                0,  { ZTID_NPC },{} },
	{ "setLightShape",              0,          ZTID_VOID,   NPCGLOWSHP,                0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getShadowSprite",            0,         ZTID_FLOAT,   NPCSHADOWSPR,              0,  { ZTID_NPC },{} },
	{ "setShadowSprite",            0,          ZTID_VOID,   NPCSHADOWSPR,              0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getSpawnSprite",             0,         ZTID_FLOAT,   NPCSPAWNSPR,               0,  { ZTID_NPC },{} },
	{ "setSpawnSprite",             0,          ZTID_VOID,   NPCSPAWNSPR,               0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getDeathSprite",             0,         ZTID_FLOAT,   NPCDEATHSPR,               0,  { ZTID_NPC },{} },
	{ "setDeathSprite",             0,          ZTID_VOID,   NPCDEATHSPR,               0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getSwitchHooked",            0,          ZTID_BOOL,   NPCSWHOOKED,               0,  { ZTID_NPC },{} },
	{ "setSwitchHooked",            0,          ZTID_VOID,   NPCSWHOOKED,               0,  { ZTID_NPC, ZTID_BOOL },{} },
	{ "Switch",                     0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "MovePaused",                 0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_NPC },{} },
	{ "Move",                       0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_NPC, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{ 0 } },
	{ "MoveAtAngle",                0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_NPC, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{ 0 } },
	{ "MoveXY",                     0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_NPC, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{ 0 } },
	{ "CanMove",                    1,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_NPC, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{ 0 } },
	{ "CanMoveAtAngle",             0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_NPC, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{ 0 } },
	{ "CanMoveXY",                  0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_NPC, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{ 0 } },
	{ "CanPlace",                   0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_NPC, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL, ZTID_FLOAT, ZTID_FLOAT },{ 0, 0, -10000, -10000 } },
	{ "getInvFlicker",              0,          ZTID_BOOL,   NPCCANFLICKER,             0,  { ZTID_NPC },{} },
	{ "setInvFlicker",              0,          ZTID_VOID,   NPCCANFLICKER,             0,  { ZTID_NPC, ZTID_BOOL },{} },
	{ "getDrowning",                0,         ZTID_FLOAT,   NPCDROWNCLK,               0,  { ZTID_NPC },{} },
	{ "setDrowning",                0,          ZTID_VOID,   NPCDROWNCLK,               0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getDrownCombo",              0,         ZTID_FLOAT,   NPCDROWNCMB,               0,  { ZTID_NPC },{} },
	{ "setDrownCombo",              0,          ZTID_VOID,   NPCDROWNCMB,               0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getFakeZ",                   0,         ZTID_FLOAT,   NPCFAKEZ,                  0,  { ZTID_NPC },{} },
	{ "setFakeZ",                   0,          ZTID_VOID,   NPCFAKEZ,                  0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getFakeJump",                0,         ZTID_FLOAT,   NPCFAKEJUMP,               0,  { ZTID_NPC },{} },
	{ "setFakeJump",                0,          ZTID_VOID,   NPCFAKEJUMP,               0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getShadowXOffset",           0,         ZTID_FLOAT,   NPCSHADOWXOFS,             0,  { ZTID_NPC },{} },
	{ "setShadowXOffset",           0,          ZTID_VOID,   NPCSHADOWXOFS,             0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getShadowYOffset",           0,         ZTID_FLOAT,   NPCSHADOWYOFS,             0,  { ZTID_NPC },{} },
	{ "setShadowYOffset",           0,          ZTID_VOID,   NPCSHADOWYOFS,             0,  { ZTID_NPC, ZTID_FLOAT },{} },
	
	//Not yet implemented
	{ "getFrozen",                  0,         ZTID_FLOAT,   NPCFROZEN,                 0,  { ZTID_NPC },{} },
	{ "setFrozen",                  0,          ZTID_VOID,   NPCFROZEN,                 0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getFrozenTile",              0,         ZTID_FLOAT,   NPCFROZENTILE,             0,  { ZTID_NPC },{} },
	{ "setFrozenTile",              0,          ZTID_VOID,   NPCFROZENTILE,             0,  { ZTID_NPC, ZTID_FLOAT },{} },
	{ "getFrozenCSet",              0,         ZTID_FLOAT,   NPCFROZENCSET,             0,  { ZTID_NPC },{} },
	{ "setFrozenCSet",              0,          ZTID_VOID,   NPCFROZENCSET,             0,  { ZTID_NPC, ZTID_FLOAT },{} },
	
	//Intentionally undocumented
	{ "getUID",                     0,         ZTID_FLOAT,   NPCSCRIPTUID,        FL_DEPR,  { ZTID_NPC },{} },
	{ "getParentUID",               0,         ZTID_FLOAT,   NPCPARENTUID,        FL_DEPR,  { ZTID_NPC },{} },
	{ "setParentUID",               0,          ZTID_VOID,   NPCPARENTUID,        FL_DEPR,  { ZTID_NPC, ZTID_FLOAT },{} },
	
	{ "Own",                        0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_NPC, ZTID_BITMAP },{} },
	{ "Own",                        1,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_NPC, ZTID_PALDATA },{} },
	{ "Own",                        2,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_NPC, ZTID_FILE },{} },
	{ "Own",                        3,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_NPC, ZTID_DIRECTORY },{} },
	{ "Own",                        4,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_NPC, ZTID_STACK },{} },
	{ "Own",                        5,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_NPC, ZTID_RNG },{} },
	{ "OwnArray",                   0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_NPC, ZTID_UNTYPED },{} },
	{ "OwnObject",                  0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_NPC, ZTID_UNTYPED },{} },
	
	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
};

NPCSymbols::NPCSymbols()
{
	table = npcTable;
	refVar = REFNPC;
}

void NPCSymbols::generateCode()
{
	//bool isValid(npc)
	{
		Function* function = getFunction("isValid");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Check validity
		addOpcode2 (code, new OIsValidNPC(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void GetName(npc, int32_t)
	{
		Function* function = getFunction("GetName");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OGetNPCName(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void Explode(npc, int32_t)
	{
			Function* function = getFunction("Explode");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCExplodeRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void BreakShield(npc)
	{
		Function* function = getFunction("BreakShield");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Break shield
		addOpcode2 (code, new OBreakShield(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//bool isDead(npc)
	{
		Function* function = getFunction("isDead");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Check validity
		addOpcode2 (code, new ONPCDead(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//bool CanSlide(npc)
	{
		Function* function = getFunction("CanSlide");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Check validity
		addOpcode2 (code, new ONPCCanSlide(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t Slide(npc)
	{
		Function* function = getFunction("Slide");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Check validity
		addOpcode2 (code, new ONPCSlide(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void Remove(npc)
	{
		Function* function = getFunction("Remove");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		POPREF();
		LABELBACK(label);
		//Break shield
		addOpcode2 (code, new ONPCRemove(new VarArgument(EXP1)));
		RETURN();
		 function->giveCode(code);
	}
	//void StopBGSFX(npc)
	{
		Function* function = getFunction("StopBGSFX");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Break shield
		addOpcode2 (code, new ONPCStopSFX(new VarArgument(EXP1)));
		RETURN();
		 function->giveCode(code);
	}
	//void Attack(npc)
	{
		Function* function = getFunction("Attack");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Break shield
		addOpcode2 (code, new ONPCAttack(new VarArgument(EXP1)));
		RETURN();
		 function->giveCode(code);
	}
	//void NewDir(int32_t arr[])
	{
		Function* function = getFunction("NewDir");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCNewDir(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void ConstantWalk(int32_t arr[])
	{
		Function* function = getFunction("ConstantWalk");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCConstWalk(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	
	//void ConstantWalk8(int32_t arr[])
	{
		Function* function = getFunction("ConstantWalk8");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCConstWalk8(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	
	//void VariableWalk(int32_t arr[])
	{
		Function* function = getFunction("VariableWalk");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCVarWalk(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void VariableWalk8(int32_t arr[])
	{
		Function* function = getFunction("VariableWalk8");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCVarWalk8(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void HaltingWalk(int32_t arr[])
	{
		Function* function = getFunction("HaltingWalk");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCHaltWalk(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void HaltingWalk8(int32_t arr[])
	{
		Function* function = getFunction("HaltingWalk8");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCHaltWalk8(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void FloatingWalk(int32_t arr[])
	{
		Function* function = getFunction("FloatingWalk");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCFloatWalk(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void BreathAttack(bool seeklink)
	{
		Function* function = getFunction("BreathAttack");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCBreatheFire(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void NewDir8(int32_t arr[])
	{
		Function* function = getFunction("NewDir8");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCNewDir8(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//bool Collision(int32_t obj_type, untyped obj_pointer)
	{
		Function* function = getFunction("Collision");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(NPCCOLLISION)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t LinedUp(int32_t range, bool dir8)
	{
		Function* function = getFunction("LinedUp");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(NPCLINEDUP)));
		RETURN();
		function->giveCode(code);
	}
	//bool HeroInRange(int32_t dist_in_pixels)
	{
		Function* function = getFunction("LinkInRange");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCHeroInRange(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	
	//npc Create(int32_t array[])
	{
		Function* function = getFunction("Create");
		
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCAdd(new VarArgument(EXP1)));
		REASSIGN_PTR(EXP2); //The value from ONPCAdd is placed in REFNPC, EXP1, and EXP2.
		RETURN();
		function->giveCode(code);
	}
	//bool CanMove(int32_t array[])
	{
		Function* function = getFunction("CanMove");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCCanMove(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//bool SimulateHit(int32_t array[])
	{
		Function* function = getFunction("SimulateHit");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCHitWith(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//bool Knockback(int32_t time, int32_t dir, int32_t spd)
	{
		Function* function = getFunction("Knockback");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new ONPCKnockback(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//bool Switch(npc, int)
	{
		Function* function = getFunction("Switch");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new OSwitchNPC(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//bool MovePaused(npc)
	{
		Function* function = getFunction("MovePaused");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new ONPCMovePaused());
		RETURN();
		function->giveCode(code);
	}
	//bool Move(npc, int32_t, int32_t, int32_t)
	{
		Function* function = getFunction("Move");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		POPREF();
		addOpcode2 (code, new ONPCMove());
		RETURN();
		function->giveCode(code);
	}
	//bool MoveAtAngle(npc, int32_t, int32_t, int32_t)
	{
		Function* function = getFunction("MoveAtAngle");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		POPREF();
		addOpcode2 (code, new ONPCMoveAngle());
		RETURN();
		function->giveCode(code);
	}
	//bool MoveXY(npc, int32_t, int32_t, int32_t)
	{
		Function* function = getFunction("MoveXY");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		POPREF();
		addOpcode2 (code, new ONPCMoveXY());
		RETURN();
		function->giveCode(code);
	}
	//bool CanMove(npc, int32_t, int32_t, int32_t)
	{
		Function* function = getFunction("CanMove", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		POPREF();
		addOpcode2 (code, new ONPCCanMoveDir());
		RETURN();
		function->giveCode(code);
	}
	//bool CanMoveAtAngle(npc, int32_t, int32_t, int32_t)
	{
		Function* function = getFunction("CanMoveAtAngle");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		POPREF();
		addOpcode2 (code, new ONPCCanMoveAngle());
		RETURN();
		function->giveCode(code);
	}
	//bool CanMoveXY(npc, int32_t, int32_t, int32_t)
	{
		Function* function = getFunction("CanMoveXY");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		POPREF();
		addOpcode2 (code, new ONPCCanMoveXY());
		RETURN();
		function->giveCode(code);
	}
	//bool CanPlace(npc, int x, int y, int special = 0, bool kb = 0, int w = -1, int h = -1)
	{
		Function* function = getFunction("CanPlace");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new ONPCCanPlace());
		LABELBACK(label);
		POP_ARGS(7, NUL);
		RETURN();
		function->giveCode(code);
	}
	//void Own(npc,bitmap)
	{
		Function* function = getFunction("Own",0);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//Target object
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Owner object
		POPREF();
		
		addOpcode2(code, new OObjOwnBitmap(new VarArgument(EXP1), new LiteralArgument(SCRIPT_NPC)));
		RETURN();
		function->giveCode(code);
	}
	//void Own(npc,paldata)
	{
		Function* function = getFunction("Own",1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//Target object
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Owner object
		POPREF();
		
		addOpcode2(code, new OObjOwnPaldata(new VarArgument(EXP1), new LiteralArgument(SCRIPT_NPC)));
		RETURN();
		function->giveCode(code);
	}
	//void Own(npc,file)
	{
		Function* function = getFunction("Own",2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//Target object
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Owner object
		POPREF();
		
		addOpcode2(code, new OObjOwnFile(new VarArgument(EXP1), new LiteralArgument(SCRIPT_NPC)));
		RETURN();
		function->giveCode(code);
	}
	//void Own(npc,directory)
	{
		Function* function = getFunction("Own",3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//Target object
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Owner object
		POPREF();
		
		addOpcode2(code, new OObjOwnDir(new VarArgument(EXP1), new LiteralArgument(SCRIPT_NPC)));
		RETURN();
		function->giveCode(code);
	}
	//void Own(npc,stack)
	{
		Function* function = getFunction("Own",4);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//Target object
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Owner object
		POPREF();
		
		addOpcode2(code, new OObjOwnStack(new VarArgument(EXP1), new LiteralArgument(SCRIPT_NPC)));
		RETURN();
		function->giveCode(code);
	}
	//void Own(npc,rng)
	{
		Function* function = getFunction("Own",5);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//Target object
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Owner object
		POPREF();
		
		addOpcode2(code, new OObjOwnRNG(new VarArgument(EXP1), new LiteralArgument(SCRIPT_NPC)));
		RETURN();
		function->giveCode(code);
	}
	//void Own(npc,untyped)
	{
		Function* function = getFunction("OwnArray");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//Target object
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Owner object
		POPREF();
		
		addOpcode2(code, new OObjOwnArray(new VarArgument(EXP1), new LiteralArgument(SCRIPT_NPC)));
		RETURN();
		function->giveCode(code);
	}
	//void Own(npc,untyped)
	{
		Function* function = getFunction("OwnObject");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//Target object
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Owner object
		POPREF();
		
		addOpcode2(code, new OObjOwnClass(new VarArgument(EXP1), new LiteralArgument(SCRIPT_NPC)));
		RETURN();
		function->giveCode(code);
	}
}

