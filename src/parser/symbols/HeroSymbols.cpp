#include "SymbolDefs.h"

HeroSymbols HeroSymbols::singleton = HeroSymbols();

static AccessorTable HeroSTable[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
	{ "getX",                       0,         ZTID_FLOAT,   LINKX,                     0,  { ZTID_PLAYER },{} },
	{ "setX",                       0,          ZTID_VOID,   LINKX,                     0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getY",                       0,         ZTID_FLOAT,   LINKY,                     0,  { ZTID_PLAYER },{} },
	{ "setY",                       0,          ZTID_VOID,   LINKY,                     0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getZ",                       0,         ZTID_FLOAT,   LINKZ,                     0,  { ZTID_PLAYER },{} },
	{ "setZ",                       0,          ZTID_VOID,   LINKZ,                     0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getJump",                    0,         ZTID_FLOAT,   LINKJUMP,                  0,  { ZTID_PLAYER },{} },
	{ "setJump",                    0,          ZTID_VOID,   LINKJUMP,                  0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getDir",                     0,         ZTID_FLOAT,   LINKDIR,                   0,  { ZTID_PLAYER },{} },
	{ "setDir",                     0,          ZTID_VOID,   LINKDIR,                   0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getHitDir",                  0,         ZTID_FLOAT,   LINKHITDIR,                0,  { ZTID_PLAYER },{} },
	{ "setHitDir",                  0,          ZTID_VOID,   LINKHITDIR,                0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getSwordJinx",               0,         ZTID_FLOAT,   LINKSWORDJINX,             0,  { ZTID_PLAYER },{} },
	{ "setSwordJinx",               0,          ZTID_VOID,   LINKSWORDJINX,             0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getItemJinx",                0,         ZTID_FLOAT,   LINKITEMJINX,              0,  { ZTID_PLAYER },{} },
	{ "setItemJinx",                0,          ZTID_VOID,   LINKITEMJINX,              0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getHP",                      0,         ZTID_FLOAT,   LINKHP,                    0,  { ZTID_PLAYER },{} },
	{ "setHP",                      0,          ZTID_VOID,   LINKHP,                    0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getMP",                      0,         ZTID_FLOAT,   LINKMP,                    0,  { ZTID_PLAYER },{} },
	{ "setMP",                      0,          ZTID_VOID,   LINKMP,                    0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getMaxHP",                   0,         ZTID_FLOAT,   LINKMAXHP,                 0,  { ZTID_PLAYER },{} },
	{ "setMaxHP",                   0,          ZTID_VOID,   LINKMAXHP,                 0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getMaxMP",                   0,         ZTID_FLOAT,   LINKMAXMP,                 0,  { ZTID_PLAYER },{} },
	{ "setMaxMP",                   0,          ZTID_VOID,   LINKMAXMP,                 0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getAction",                  0,         ZTID_FLOAT,   LINKACTION,                0,  { ZTID_PLAYER },{} },
	{ "setAction",                  0,          ZTID_VOID,   LINKACTION,                0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getTileMod",                 0,         ZTID_FLOAT,   LINKTILEMOD,               0,  { ZTID_PLAYER },{} },
	{ "setTileMod",                 0,          ZTID_VOID,   LINKTILEMOD,               0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getHeldItem",                0,         ZTID_FLOAT,   LINKHELD,                  0,  { ZTID_PLAYER },{} },
	{ "setHeldItem",                0,          ZTID_VOID,   LINKHELD,                  0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "Warp",                       0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PLAYER, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "WarpEx",                     0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "Explode",                    0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "PitWarp",                    0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PLAYER, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getItem[]",                  0,          ZTID_BOOL,   LINKITEMD,                 0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "setItem[]",                  0,          ZTID_VOID,   LINKITEMD,                 0,  { ZTID_PLAYER, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getSteps[]",                 0,         ZTID_FLOAT,   HEROSTEPS,                 0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "setSteps[]",                 0,          ZTID_VOID,   HEROSTEPS,                 0,  { ZTID_PLAYER, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getStep",                    0,         ZTID_FLOAT,   HEROSTEPRATE,              0,  { ZTID_PLAYER },{} },
	{ "setStep",                    0,          ZTID_VOID,   HEROSTEPRATE,              0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getShoveOffset",             0,         ZTID_FLOAT,   HEROSHOVEOFFSET,           0,  { ZTID_PLAYER },{} },
	{ "setShoveOffset",             0,          ZTID_VOID,   HEROSHOVEOFFSET,           0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getDrunk",                   0,         ZTID_FLOAT,   LINKDRUNK,                 0,  { ZTID_PLAYER },{} },
	{ "setDrunk",                   0,          ZTID_VOID,   LINKDRUNK,                 0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getHealthBeep",              0,         ZTID_FLOAT,   HEROHEALTHBEEP,            0,  { ZTID_PLAYER },{} },
	{ "setHealthBeep",              0,          ZTID_VOID,   HEROHEALTHBEEP,            0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getInvisible",               0,          ZTID_BOOL,   LINKINVIS,                 0,  { ZTID_PLAYER },{} },
	{ "setInvisible",               0,          ZTID_VOID,   LINKINVIS,                 0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getNoStepForward",           0,          ZTID_BOOL,   HERONOSTEPFORWARD,         0,  { ZTID_PLAYER },{} },
	{ "setNoStepForward",           0,          ZTID_VOID,   HERONOSTEPFORWARD,         0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getAnimation",               0,         ZTID_FLOAT,   LINKENGINEANIMATE,         0,  { ZTID_PLAYER },{} },
	{ "setAnimation",               0,          ZTID_VOID,   LINKENGINEANIMATE,         0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getCollDetection",           0,          ZTID_BOOL,   LINKINVINC,                0,  { ZTID_PLAYER },{} },
	{ "setCollDetection",           0,          ZTID_VOID,   LINKINVINC,                0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getMisc[]",                  0,       ZTID_UNTYPED,   LINKMISCD,                 0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "setMisc[]",                  0,          ZTID_VOID,   LINKMISCD,                 0,  { ZTID_PLAYER, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getDefense[]",               0,         ZTID_FLOAT,   LINKDEFENCE,               0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "setDefense[]",               0,          ZTID_VOID,   LINKDEFENCE,               0,  { ZTID_PLAYER, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getHitBy[]",                 0,       ZTID_UNTYPED,   LINKHITBY,                 0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "setHitBy[]",                 0,          ZTID_VOID,   LINKHITBY,                 0,  { ZTID_PLAYER, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getLadderX",                 0,         ZTID_FLOAT,   LINKLADDERX,               0,  { ZTID_PLAYER },{} },
	{ "getLadderY",                 0,         ZTID_FLOAT,   LINKLADDERY,               0,  { ZTID_PLAYER },{} },
	{ "getTile",                    0,         ZTID_FLOAT,   LINKTILE,                  0,  { ZTID_PLAYER },{} },
	{ "setTile",                    0,          ZTID_VOID,   LINKTILE,                  0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getFlip",                    0,         ZTID_FLOAT,   LINKFLIP,                  0,  { ZTID_PLAYER },{} },
	{ "setFlip",                    0,          ZTID_VOID,   LINKFLIP,                  0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "SelectAWeapon",              0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "SelectBWeapon",              0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "SelectXWeapon",              0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "SelectYWeapon",              0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "setInvFrames",               0,          ZTID_VOID,   LINKINVFRAME,              0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getInvFrames",               0,         ZTID_FLOAT,   LINKINVFRAME,              0,  { ZTID_PLAYER },{} },
	{ "setInvFlicker",              0,          ZTID_VOID,   LINKCANFLICKER,            0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getInvFlicker",              0,          ZTID_BOOL,   LINKCANFLICKER,            0,  { ZTID_PLAYER },{} },
	{ "setHurtSound",               0,          ZTID_VOID,   LINKHURTSFX,               0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getHurtSound",               0,         ZTID_FLOAT,   LINKHURTSFX,               0,  { ZTID_PLAYER },{} },
	{ "setItemB",                   0,          ZTID_VOID,   LINKITEMB,                 0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "setItemA",                   0,          ZTID_VOID,   LINKITEMA,                 0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getItemB",                   0,         ZTID_FLOAT,   LINKITEMB,                 0,  { ZTID_PLAYER },{} },
	{ "getItemA",                   0,         ZTID_FLOAT,   LINKITEMA,                 0,  { ZTID_PLAYER },{} },
	{ "getItemX",                   0,         ZTID_FLOAT,   LINKITEMX,                 0,  { ZTID_PLAYER },{} },
	{ "setItemX",                   0,          ZTID_VOID,   LINKITEMX,                 0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getItemY",                   0,         ZTID_FLOAT,   LINKITEMY,                 0,  { ZTID_PLAYER },{} },
	{ "setItemY",                   0,          ZTID_VOID,   LINKITEMY,                 0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getEaten",                   0,         ZTID_FLOAT,   LINKEATEN,                 0,  { ZTID_PLAYER },{} },
	{ "setEaten",                   0,          ZTID_VOID,   LINKEATEN,                 0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getGrabbed",                 0,          ZTID_BOOL,   LINKGRABBED,               0,  { ZTID_PLAYER },{} },
	{ "setGrabbed",                 0,          ZTID_VOID,   LINKGRABBED,               0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getStun",                    0,         ZTID_FLOAT,   LINKSTUN,                  0,  { ZTID_PLAYER },{} },
	{ "setStun",                    0,          ZTID_VOID,   LINKSTUN,                  0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getBunnyClk",                0,         ZTID_FLOAT,   HEROBUNNY,                 0,  { ZTID_PLAYER },{} },
	{ "setBunnyClk",                0,          ZTID_VOID,   HEROBUNNY,                 0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getPushing",                 0,         ZTID_FLOAT,   LINKPUSH,                  0,  { ZTID_PLAYER },{} },
	{ "setPushing",                 0,          ZTID_VOID,   LINKPUSH,                  0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getScriptCSet",              0,         ZTID_FLOAT,   HEROSCRIPTCSET,            0,  { ZTID_PLAYER },{} },
	{ "setScriptCSet",              0,          ZTID_VOID,   HEROSCRIPTCSET,            0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getScriptTile",              0,         ZTID_FLOAT,   LINKSCRIPTTILE,            0,  { ZTID_PLAYER },{} },
	{ "setScriptTile",              0,          ZTID_VOID,   LINKSCRIPTTILE,            0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getScriptFlip",              0,         ZTID_FLOAT,   LINKSCRIPFLIP,             0,  { ZTID_PLAYER },{} },
	{ "setScriptFlip",              0,          ZTID_VOID,   LINKSCRIPFLIP,             0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getDiagonal",                0,          ZTID_BOOL,   LINKDIAG,                  0,  { ZTID_PLAYER },{} },
	{ "setDiagonal",                0,          ZTID_VOID,   LINKDIAG,                  0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getBigHitbox",               0,          ZTID_BOOL,   LINKBIGHITBOX,             0,  { ZTID_PLAYER },{} },
	{ "setBigHitbox",               0,          ZTID_VOID,   LINKBIGHITBOX,             0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getGravity",                 0,          ZTID_BOOL,   LINKGRAVITY,               0,  { ZTID_PLAYER },{} },
	{ "setGravity",                 0,          ZTID_VOID,   LINKGRAVITY,               0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getRotation",                0,         ZTID_FLOAT,   LINKROTATION,              0,  { ZTID_PLAYER },{} },
	{ "setRotation",                0,          ZTID_VOID,   LINKROTATION,              0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getScale",                   0,         ZTID_FLOAT,   LINKSCALE,                 0,  { ZTID_PLAYER },{} },
	{ "setScale",                   0,          ZTID_VOID,   LINKSCALE,                 0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getClimbing",                0,          ZTID_BOOL,   LINKCLIMBING,              0,  { ZTID_PLAYER },{} },
	{ "setClimbing",                0,          ZTID_VOID,   LINKCLIMBING,              0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getJumpCount",               0,         ZTID_FLOAT,   HEROJUMPCOUNT,             0,  { ZTID_PLAYER },{} },
	{ "setJumpCount",               0,          ZTID_VOID,   HEROJUMPCOUNT,             0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getPitPullDir",              0,         ZTID_FLOAT,   HEROPULLDIR,               0,  { ZTID_PLAYER },{} },
	{ "getPitPullTimer",            0,         ZTID_FLOAT,   HEROPULLCLK,               0,  { ZTID_PLAYER },{} },
	{ "setPitPullTimer",            0,          ZTID_VOID,   HEROPULLCLK,               0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getFalling",                 0,         ZTID_FLOAT,   HEROFALLCLK,               0,  { ZTID_PLAYER },{} },
	{ "setFalling",                 0,          ZTID_VOID,   HEROFALLCLK,               0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getFallCombo",               0,         ZTID_FLOAT,   HEROFALLCMB,               0,  { ZTID_PLAYER },{} },
	{ "setFallCombo",               0,          ZTID_VOID,   HEROFALLCMB,               0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getMoveFlags[]",             0,          ZTID_BOOL,   HEROMOVEFLAGS,             0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "setMoveFlags[]",             0,          ZTID_VOID,   HEROMOVEFLAGS,             0,  { ZTID_PLAYER, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getClockActive",             0,          ZTID_BOOL,   CLOCKACTIVE,               0,  { ZTID_PLAYER },{} },
	{ "setClockActive",             0,          ZTID_VOID,   CLOCKACTIVE,               0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getClockTimer",              0,         ZTID_FLOAT,   CLOCKCLK,                  0,  { ZTID_PLAYER },{} },
	{ "setClockTimer",              0,          ZTID_VOID,   CLOCKCLK,                  0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getCSet",                    0,         ZTID_FLOAT,   LINKCSET,                  0,  { ZTID_PLAYER },{} },
	{ "setCSet",                    0,          ZTID_VOID,   LINKCSET,                  0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getIsWarping",               0,          ZTID_BOOL,   HEROISWARPING,             0,  { ZTID_PLAYER },{} },
	
	{ "getRespawnX",                0,         ZTID_FLOAT,   HERORESPAWNX,              0,  { ZTID_PLAYER },{} },
	{ "setRespawnX",                0,          ZTID_VOID,   HERORESPAWNX,              0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getRespawnY",                0,         ZTID_FLOAT,   HERORESPAWNY,              0,  { ZTID_PLAYER },{} },
	{ "setRespawnY",                0,          ZTID_VOID,   HERORESPAWNY,              0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getRespawnDMap",             0,         ZTID_FLOAT,   HERORESPAWNDMAP,           0,  { ZTID_PLAYER },{} },
	{ "setRespawnDMap",             0,          ZTID_VOID,   HERORESPAWNDMAP,           0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getRespawnScreen",           0,         ZTID_FLOAT,   HERORESPAWNSCR,            0,  { ZTID_PLAYER },{} },
	{ "setRespawnScreen",           0,          ZTID_VOID,   HERORESPAWNSCR,            0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getSwitchTimer",             0,         ZTID_FLOAT,   HEROSWITCHTIMER,           0,  { ZTID_PLAYER },{} },
	{ "setSwitchTimer",             0,          ZTID_VOID,   HEROSWITCHTIMER,           0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getSwitchMaxTimer",          0,         ZTID_FLOAT,   HEROSWITCHMAXTIMER,        0,  { ZTID_PLAYER },{} },
	{ "setSwitchMaxTimer",          0,          ZTID_VOID,   HEROSWITCHMAXTIMER,        0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getImmortal",                0,         ZTID_FLOAT,   HEROIMMORTAL,              0,  { ZTID_PLAYER },{} },
	{ "setImmortal",                0,          ZTID_VOID,   HEROIMMORTAL,              0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "SwitchCombo",                0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_PLAYER, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "Kill",                       0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PLAYER, ZTID_BOOL },{ 0 } },
	{ "getDrowning",                0,         ZTID_FLOAT,   HERODROWNCLK,              0,  { ZTID_PLAYER },{} },
	{ "setDrowning",                0,          ZTID_VOID,   HERODROWNCLK,              0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getDrownCombo",              0,         ZTID_FLOAT,   HERODROWNCMB,              0,  { ZTID_PLAYER },{} },
	{ "setDrownCombo",              0,          ZTID_VOID,   HERODROWNCMB,              0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getFakeZ",                   0,         ZTID_FLOAT,   HEROFAKEZ,                 0,  { ZTID_PLAYER },{} },
	{ "setFakeZ",                   0,          ZTID_VOID,   HEROFAKEZ,                 0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getFakeJump",                0,         ZTID_FLOAT,   HEROFAKEJUMP,              0,  { ZTID_PLAYER },{} },
	{ "setFakeJump",                0,          ZTID_VOID,   HEROFAKEJUMP,              0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getShadowXOffset",           0,         ZTID_FLOAT,   HEROSHADOWXOFS,            0,  { ZTID_PLAYER },{} },
	{ "setShadowXOffset",           0,          ZTID_VOID,   HEROSHADOWXOFS,            0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getShadowYOffset",           0,         ZTID_FLOAT,   HEROSHADOWYOFS,            0,  { ZTID_PLAYER },{} },
	{ "setShadowYOffset",           0,          ZTID_VOID,   HEROSHADOWYOFS,            0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getStanding",                0,          ZTID_BOOL,   HEROSTANDING,              0,  { ZTID_PLAYER },{} },
	{ "getCoyoteTime",              0,         ZTID_FLOAT,   HEROCOYOTETIME,            0,  { ZTID_PLAYER },{} },
	{ "setCoyoteTime",              0,          ZTID_VOID,   HEROCOYOTETIME,            0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "MoveXY",                     0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_PLAYER, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL, ZTID_BOOL, ZTID_BOOL },{ 0, 0, 10000 } },
	{ "CanMoveXY",                  0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_PLAYER, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL, ZTID_BOOL, ZTID_BOOL },{ 0, 0, 10000 } },
	{ "MoveAtAngle",                0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_PLAYER, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL, ZTID_BOOL, ZTID_BOOL },{ 0, 0, 10000 } },
	{ "CanMoveAtAngle",             0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_PLAYER, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL, ZTID_BOOL, ZTID_BOOL },{ 0, 0, 10000 } },
	{ "Move",                       0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_PLAYER, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL, ZTID_BOOL, ZTID_BOOL },{ 0, 0, 10000 } },
	{ "CanMove",                    0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_PLAYER, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL, ZTID_BOOL, ZTID_BOOL },{ 0, 0, 10000 } },
	{ "ReleaseLiftWeapon",          0,          ZTID_LWPN,   -1,                   FL_INL,  { ZTID_PLAYER },{} },
	{ "LiftWeapon",                 0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PLAYER, ZTID_LWPN, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getLiftedWeapon",            0,          ZTID_LWPN,   HEROLIFTEDWPN,             0,  { ZTID_PLAYER },{} },
	{ "setLiftedWeapon",            0,          ZTID_VOID,   HEROLIFTEDWPN,             0,  { ZTID_PLAYER, ZTID_LWPN },{} },
	{ "getLiftTimer",               0,         ZTID_FLOAT,   HEROLIFTTIMER,             0,  { ZTID_PLAYER },{} },
	{ "setLiftTimer",               0,          ZTID_VOID,   HEROLIFTTIMER,             0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getLiftMaxTimer",            0,         ZTID_FLOAT,   HEROLIFTMAXTIMER,          0,  { ZTID_PLAYER },{} },
	{ "setLiftMaxTimer",            0,          ZTID_VOID,   HEROLIFTMAXTIMER,          0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getLiftHeight",              0,         ZTID_FLOAT,   HEROLIFTHEIGHT,            0,  { ZTID_PLAYER },{} },
	{ "setLiftHeight",              0,          ZTID_VOID,   HEROLIFTHEIGHT,            0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getHammerState",             0,         ZTID_FLOAT,   HEROHAMMERSTATE,           0,  { ZTID_PLAYER },{} },
	{ "setHammerState",             0,          ZTID_VOID,   HEROHAMMERSTATE,           0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getLiftFlags[]",             0,          ZTID_BOOL,   HEROLIFTFLAGS,             0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "setLiftFlags[]",             0,          ZTID_VOID,   HEROLIFTFLAGS,             0,  { ZTID_PLAYER, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getShieldJinx",              0,         ZTID_FLOAT,   HEROSHIELDJINX,            0,  { ZTID_PLAYER },{} },
	{ "setShieldJinx",              0,          ZTID_VOID,   HEROSHIELDJINX,            0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	
	
	//Intentionally undocumented
	{ "Warp",                       1,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	// Input
	{ "getInputStart",              0,          ZTID_BOOL,   INPUTSTART,                0,  { ZTID_PLAYER },{} },
	{ "setInputStart",              0,          ZTID_VOID,   INPUTSTART,                0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getInputMap",                0,          ZTID_BOOL,   INPUTMAP,                  0,  { ZTID_PLAYER },{} },
	{ "setInputMap",                0,          ZTID_VOID,   INPUTMAP,                  0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getInputUp",                 0,          ZTID_BOOL,   INPUTUP,                   0,  { ZTID_PLAYER },{} },
	{ "setInputUp",                 0,          ZTID_VOID,   INPUTUP,                   0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getInputDown",               0,          ZTID_BOOL,   INPUTDOWN,                 0,  { ZTID_PLAYER },{} },
	{ "setInputDown",               0,          ZTID_VOID,   INPUTDOWN,                 0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getInputLeft",               0,          ZTID_BOOL,   INPUTLEFT,                 0,  { ZTID_PLAYER },{} },
	{ "setInputLeft",               0,          ZTID_VOID,   INPUTLEFT,                 0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getInputRight",              0,          ZTID_BOOL,   INPUTRIGHT,                0,  { ZTID_PLAYER },{} },
	{ "setInputRight",              0,          ZTID_VOID,   INPUTRIGHT,                0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getInputA",                  0,          ZTID_BOOL,   INPUTA,                    0,  { ZTID_PLAYER },{} },
	{ "setInputA",                  0,          ZTID_VOID,   INPUTA,                    0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getInputB",                  0,          ZTID_BOOL,   INPUTB,                    0,  { ZTID_PLAYER },{} },
	{ "setInputB",                  0,          ZTID_VOID,   INPUTB,                    0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getInputL",                  0,          ZTID_BOOL,   INPUTL,                    0,  { ZTID_PLAYER },{} },
	{ "setInputL",                  0,          ZTID_VOID,   INPUTL,                    0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getInputR",                  0,          ZTID_BOOL,   INPUTR,                    0,  { ZTID_PLAYER },{} },
	{ "setInputR",                  0,          ZTID_VOID,   INPUTR,                    0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getInputEx1",                0,          ZTID_BOOL,   INPUTEX1,                  0,  { ZTID_PLAYER },{} },
	{ "setInputEx1",                0,          ZTID_VOID,   INPUTEX1,                  0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getInputEx2",                0,          ZTID_BOOL,   INPUTEX2,                  0,  { ZTID_PLAYER },{} },
	{ "setInputEx2",                0,          ZTID_VOID,   INPUTEX2,                  0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getInputEx3",                0,          ZTID_BOOL,   INPUTEX3,                  0,  { ZTID_PLAYER },{} },
	{ "setInputEx3",                0,          ZTID_VOID,   INPUTEX3,                  0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getInputEx4",                0,          ZTID_BOOL,   INPUTEX4,                  0,  { ZTID_PLAYER },{} },
	{ "setInputEx4",                0,          ZTID_VOID,   INPUTEX4,                  0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getPressStart",              0,          ZTID_BOOL,   INPUTPRESSSTART,           0,  { ZTID_PLAYER },{} },
	{ "setPressStart",              0,          ZTID_VOID,   INPUTPRESSSTART,           0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getPressUp",                 0,          ZTID_BOOL,   INPUTPRESSUP,              0,  { ZTID_PLAYER },{} },
	{ "setPressUp",                 0,          ZTID_VOID,   INPUTPRESSUP,              0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getPressDown",               0,          ZTID_BOOL,   INPUTPRESSDOWN,            0,  { ZTID_PLAYER },{} },
	{ "setPressDown",               0,          ZTID_VOID,   INPUTPRESSDOWN,            0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getPressLeft",               0,          ZTID_BOOL,   INPUTPRESSLEFT,            0,  { ZTID_PLAYER },{} },
	{ "setPressLeft",               0,          ZTID_VOID,   INPUTPRESSLEFT,            0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getPressRight",              0,          ZTID_BOOL,   INPUTPRESSRIGHT,           0,  { ZTID_PLAYER },{} },
	{ "setPressRight",              0,          ZTID_VOID,   INPUTPRESSRIGHT,           0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getPressA",                  0,          ZTID_BOOL,   INPUTPRESSA,               0,  { ZTID_PLAYER },{} },
	{ "setPressA",                  0,          ZTID_VOID,   INPUTPRESSA,               0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getPressB",                  0,          ZTID_BOOL,   INPUTPRESSB,               0,  { ZTID_PLAYER },{} },
	{ "setPressB",                  0,          ZTID_VOID,   INPUTPRESSB,               0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getPressL",                  0,          ZTID_BOOL,   INPUTPRESSL,               0,  { ZTID_PLAYER },{} },
	{ "setPressL",                  0,          ZTID_VOID,   INPUTPRESSL,               0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getPressR",                  0,          ZTID_BOOL,   INPUTPRESSR,               0,  { ZTID_PLAYER },{} },
	{ "setPressR",                  0,          ZTID_VOID,   INPUTPRESSR,               0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getPressEx1",                0,          ZTID_BOOL,   INPUTPRESSEX1,             0,  { ZTID_PLAYER },{} },
	{ "setPressEx1",                0,          ZTID_VOID,   INPUTPRESSEX1,             0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getPressEx2",                0,          ZTID_BOOL,   INPUTPRESSEX2,             0,  { ZTID_PLAYER },{} },
	{ "setPressEx2",                0,          ZTID_VOID,   INPUTPRESSEX2,             0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getPressEx3",                0,          ZTID_BOOL,   INPUTPRESSEX3,             0,  { ZTID_PLAYER },{} },
	{ "setPressEx3",                0,          ZTID_VOID,   INPUTPRESSEX3,             0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getPressEx4",                0,          ZTID_BOOL,   INPUTPRESSEX4,             0,  { ZTID_PLAYER },{} },
	{ "setPressEx4",                0,          ZTID_VOID,   INPUTPRESSEX4,             0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getInputMouseX",             0,         ZTID_FLOAT,   INPUTMOUSEX,               0,  { ZTID_PLAYER },{} },
	{ "setInputMouseX",             0,          ZTID_VOID,   INPUTMOUSEX,               0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getInputMouseY",             0,         ZTID_FLOAT,   INPUTMOUSEY,               0,  { ZTID_PLAYER },{} },
	{ "setInputMouseY",             0,          ZTID_VOID,   INPUTMOUSEY,               0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getInputMouseZ",             0,         ZTID_FLOAT,   INPUTMOUSEZ,               0,  { ZTID_PLAYER },{} },
	{ "setInputMouseZ",             0,          ZTID_VOID,   INPUTMOUSEZ,               0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getInputMouseB",             0,         ZTID_FLOAT,   INPUTMOUSEB,               0,  { ZTID_PLAYER },{} },
	{ "setInputMouseB",             0,          ZTID_VOID,   INPUTMOUSEB,               0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getInputAxisUp",             0,          ZTID_BOOL,   INPUTAXISUP,               0,  { ZTID_PLAYER },{} },
	{ "setInputAxisUp",             0,          ZTID_VOID,   INPUTAXISUP,               0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getInputAxisDown",           0,          ZTID_BOOL,   INPUTAXISDOWN,             0,  { ZTID_PLAYER },{} },
	{ "setInputAxisDown",           0,          ZTID_VOID,   INPUTAXISDOWN,             0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getInputAxisLeft",           0,          ZTID_BOOL,   INPUTAXISLEFT,             0,  { ZTID_PLAYER },{} },
	{ "setInputAxisLeft",           0,          ZTID_VOID,   INPUTAXISLEFT,             0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getInputAxisRight",          0,          ZTID_BOOL,   INPUTAXISRIGHT,            0,  { ZTID_PLAYER },{} },
	{ "setInputAxisRight",          0,          ZTID_VOID,   INPUTAXISRIGHT,            0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getPressAxisUp",             0,          ZTID_BOOL,   INPUTPRESSAXISUP,          0,  { ZTID_PLAYER },{} },
	{ "setPressAxisUp",             0,          ZTID_VOID,   INPUTPRESSAXISUP,          0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getPressAxisDown",           0,          ZTID_BOOL,   INPUTPRESSAXISDOWN,        0,  { ZTID_PLAYER },{} },
	{ "setPressAxisDown",           0,          ZTID_VOID,   INPUTPRESSAXISDOWN,        0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getPressAxisLeft",           0,          ZTID_BOOL,   INPUTPRESSAXISLEFT,        0,  { ZTID_PLAYER },{} },
	{ "setPressAxisLeft",           0,          ZTID_VOID,   INPUTPRESSAXISLEFT,        0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getPressAxisRight",          0,          ZTID_BOOL,   INPUTPRESSAXISRIGHT,       0,  { ZTID_PLAYER },{} },
	{ "setPressAxisRight",          0,          ZTID_VOID,   INPUTPRESSAXISRIGHT,       0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	{ "getPressMap",                0,          ZTID_BOOL,   INPUTPRESSMAP,             0,  { ZTID_PLAYER },{} },
	{ "setPressMap",                0,          ZTID_VOID,   INPUTPRESSMAP,             0,  { ZTID_PLAYER, ZTID_BOOL },{} },
	
	// Size stuff
	{ "getHitWidth",                0,         ZTID_FLOAT,   LINKHXSZ,                  0,  { ZTID_PLAYER },{} },
	{ "setHitWidth",                0,          ZTID_VOID,   LINKHXSZ,                  0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getHitHeight",               0,         ZTID_FLOAT,   LINKHYSZ,                  0,  { ZTID_PLAYER },{} },
	{ "setHitHeight",               0,          ZTID_VOID,   LINKHYSZ,                  0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getHitZHeight",              0,         ZTID_FLOAT,   LINKHZSZ,                  0,  { ZTID_PLAYER },{} },
	{ "setHitZHeight",              0,          ZTID_VOID,   LINKHZSZ,                  0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getTileWidth",               0,         ZTID_FLOAT,   LINKTXSZ,                  0,  { ZTID_PLAYER },{} },
	{ "setTileWidth",               0,          ZTID_VOID,   LINKTXSZ,                  0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getTileHeight",              0,         ZTID_FLOAT,   LINKTYSZ,                  0,  { ZTID_PLAYER },{} },
	{ "setTileHeight",              0,          ZTID_VOID,   LINKTYSZ,                  0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getDrawXOffset",             0,         ZTID_FLOAT,   LINKXOFS,                  0,  { ZTID_PLAYER },{} },
	{ "setDrawXOffset",             0,          ZTID_VOID,   LINKXOFS,                  0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getDrawYOffset",             0,         ZTID_FLOAT,   LINKYOFS,                  0,  { ZTID_PLAYER },{} },
	{ "setDrawYOffset",             0,          ZTID_VOID,   LINKYOFS,                  0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getTotalDYOffset",           0,         ZTID_FLOAT,   HEROTOTALDYOFFS,           0,  { ZTID_PLAYER },{} },
	{ "setTotalDYOffset",           0,          ZTID_VOID,   HEROTOTALDYOFFS,           0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getDrawZOffset",             0,         ZTID_FLOAT,   LINKZOFS,                  0,  { ZTID_PLAYER },{} },
	{ "setDrawZOffset",             0,          ZTID_VOID,   LINKZOFS,                  0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getHitXOffset",              0,         ZTID_FLOAT,   LINKHXOFS,                 0,  { ZTID_PLAYER },{} },
	{ "setHitXOffset",              0,          ZTID_VOID,   LINKHXOFS,                 0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "getHitYOffset",              0,         ZTID_FLOAT,   LINKHYOFS,                 0,  { ZTID_PLAYER },{} },
	{ "setHitYOffset",              0,          ZTID_VOID,   LINKHYOFS,                 0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	
	//
	{ "SetItemA",                   0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "SetItemB",                   0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	{ "SetItemSlot",                0,          ZTID_VOID,   -1,                        0,  { ZTID_PLAYER, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetOriginalTile",            0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_PLAYER, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetOriginalFlip",            0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_PLAYER, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getEquipment",               0,         ZTID_FLOAT,   LINKEQUIP,                 0,  { ZTID_PLAYER },{} },
	{ "setEquipment",               0,          ZTID_VOID,   LINKEQUIP,                 0,  { ZTID_PLAYER, ZTID_FLOAT },{} },
	
	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
};

HeroSymbols::HeroSymbols()
{
	table = HeroSTable;
	refVar = NUL;
}

void HeroSymbols::generateCode()
{
	//Warp(link, int32_t, int32_t)
	{
		Function* function = getFunction("Warp");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop ffc, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		//ffc must be this (link is not a user-accessible type)
		addOpcode2 (code, new OWarp(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
	Function* function = getFunction("WarpEx");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OHeroWarpExRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);    
	}
	{
	Function* function = getFunction("Warp", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OHeroWarpExRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);    
	}
	{
	Function* function = getFunction("Explode");
	int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OHeroExplodeRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);        
	}
	   //void SetItemSlot(link, int32_t item, int32_t slot, int32_t force)
	{
		Function* function = getFunction("SetItemSlot");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSetRegister(new VarArgument(SETITEMSLOT), new VarArgument(SFTEMP)));
		RETURN();
		function->giveCode(code);
	}
	
	//void SetItemA(link, int32_t)
	{
		Function* function = getFunction("SetItemA");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		addOpcode2 (code, new OSetRegister(new VarArgument(GAMESETA), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//void SetItemB(link, int32_t)
	{
		Function* function = getFunction("SetItemB");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		addOpcode2 (code, new OSetRegister(new VarArgument(GAMESETB), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	
	//PitWarp(link, int32_t, int32_t)
	{
		Function* function = getFunction("PitWarp");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop ffc, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		//ffc must be this (link is not a user-accessible type)
		addOpcode2 (code, new OPitWarp(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//SelectAWeapon(link, int32_t)
	{
		Function* function = getFunction("SelectAWeapon");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer and ignore it
		POPREF();
		addOpcode2 (code, new OSelectAWeaponRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//SelectBWeapon(link, int32_t)
	{
		Function* function = getFunction("SelectBWeapon");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer and ignore it
		POPREF();
		addOpcode2 (code, new OSelectBWeaponRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//SelectXWeapon(link, int32_t)
	{
		Function* function = getFunction("SelectXWeapon");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer and ignore it
		POPREF();
		addOpcode2 (code, new OSelectXWeaponRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//SelectYWeapon(link, int32_t)
	{
		Function* function = getFunction("SelectYWeapon");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer and ignore it
		POPREF();
		addOpcode2 (code, new OSelectYWeaponRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t GetOriginaTile(link, int32_t,int32_t)
	{
		Function* function = getFunction("GetOriginalTile");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(LINKOTILE)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t GetOriginalFlip(link, int32_t,int32_t)
	{
		Function* function = getFunction("GetOriginalFlip");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(LINKOFLIP)));
		RETURN();
		function->giveCode(code);
	}
	//bool SwitchCombo(link, int, int)
	{
		Function* function = getFunction("SwitchCombo");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSwitchCombo(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//bool Kill(player, bool)
	{
		Function* function = getFunction("Kill");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OKillPlayer(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void MoveXY(player, int, int, bool, bool, bool)
	{
		Function* function = getFunction("MoveXY");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		ASSERT_NUL();
		addOpcode2 (code, new OHeroMoveXY());
		LABELBACK(label);
		POP_ARGS(5, NUL);
		RETURN();
		function->giveCode(code);
	}
	//void CanMoveXY(player, int, int, bool, bool, bool)
	{
		Function* function = getFunction("CanMoveXY");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		ASSERT_NUL();
		addOpcode2 (code, new OHeroCanMoveXY());
		LABELBACK(label);
		POP_ARGS(5, NUL);
		RETURN();
		function->giveCode(code);
	}
	//void MoveAtAngle(player, int, int, bool, bool, bool)
	{
		Function* function = getFunction("MoveAtAngle");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		ASSERT_NUL();
		addOpcode2 (code, new OHeroMoveAtAngle());
		LABELBACK(label);
		POP_ARGS(5, NUL);
		RETURN();
		function->giveCode(code);
	}
	//void CanMoveAtAngle(player, int, int, bool, bool, bool)
	{
		Function* function = getFunction("CanMoveAtAngle");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		ASSERT_NUL();
		addOpcode2 (code, new OHeroCanMoveAtAngle());
		LABELBACK(label);
		POP_ARGS(5, NUL);
		RETURN();
		function->giveCode(code);
	}
	//void Move(player, int, int, bool, bool, bool)
	{
		Function* function = getFunction("Move");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		ASSERT_NUL();
		addOpcode2 (code, new OHeroMove());
		LABELBACK(label);
		POP_ARGS(5, NUL);
		RETURN();
		function->giveCode(code);
	}
	//void CanMove(player, int, int, bool, bool, bool)
	{
		Function* function = getFunction("CanMove");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		ASSERT_NUL();
		addOpcode2 (code, new OHeroCanMove());
		LABELBACK(label);
		POP_ARGS(5, NUL);
		RETURN();
		function->giveCode(code);
	}
	//lweapon ReleaseLiftWeapon(player)
	{
		Function* function = getFunction("ReleaseLiftWeapon");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		ASSERT_NUL();
		addOpcode2 (code, new OHeroLiftRelease());
		LABELBACK(label);
		RETURN();
		function->giveCode(code);
	}
	//void LiftWeapon(player, lweapon, int, int)
	{
		Function* function = getFunction("LiftWeapon");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		ASSERT_NUL();
		addOpcode2 (code, new OHeroLiftGrab());
		LABELBACK(label);
		POP_ARGS(3, NUL);
		RETURN();
		function->giveCode(code);
	}
}

