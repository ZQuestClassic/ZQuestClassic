#include "SymbolDefs.h"

MapDataSymbols MapDataSymbols::singleton = MapDataSymbols();

static AccessorTable MapDataTable[] =
{
	//name,                       tag,            rettype,   var,                      funcFlags,  params,optparams
	{ "isSolid",                    0,          ZTID_BOOL,   -1,                          FL_INL,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "isSolidLayer",               0,          ZTID_BOOL,   -1,                               0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getValid",                   0,         ZTID_FLOAT,   MAPDATAVALID,                     0,  { ZTID_MAPDATA },{} },
	{ "setValid",                   0,          ZTID_VOID,   MAPDATAVALID,                     0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "getGuy",                     0,         ZTID_FLOAT,   MAPDATAGUY,                       0,  { ZTID_MAPDATA },{} },
	{ "setGuy",                     0,          ZTID_VOID,   MAPDATAGUY,                       0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "getString",                  0,         ZTID_FLOAT,   MAPDATASTRING,                    0,  { ZTID_MAPDATA },{} },
	{ "setString",                  0,          ZTID_VOID,   MAPDATASTRING,                    0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "getRoomType",                0,         ZTID_FLOAT,   MAPDATAROOM,                      0,  { ZTID_MAPDATA },{} },
	{ "setRoomType",                0,          ZTID_VOID,   MAPDATAROOM,                      0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "getItem",                    0,         ZTID_FLOAT,   MAPDATAITEM,                      0,  { ZTID_MAPDATA },{} },
	{ "setItem",                    0,          ZTID_VOID,   MAPDATAITEM,                      0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "getTileWarpType[]",          0,         ZTID_FLOAT,   MAPDATATILEWARPTYPE,              0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setTileWarpType[]",          0,          ZTID_VOID,   MAPDATATILEWARPTYPE,              0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getScript",                  0,         ZTID_FLOAT,   MAPDATASCRIPT,                    0,  { ZTID_MAPDATA },{} },
	{ "setScript",                  0,          ZTID_VOID,   MAPDATASCRIPT,                    0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "getInitD[]",                 0,       ZTID_UNTYPED,   MAPDATAINITDARRAY,                0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setInitD[]",                 0,          ZTID_VOID,   MAPDATAINITDARRAY,                0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_UNTYPED },{} },
	
	{ "getTileWarpOverlay[]",       0,          ZTID_BOOL,   MAPDATATILEWARPOVFLAGS,           0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setTileWarpOverlay[]",       0,          ZTID_VOID,   MAPDATATILEWARPOVFLAGS,           0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_BOOL },{} },
	
	{ "getDoorComboSet",            0,         ZTID_FLOAT,   MAPDATADOORCOMBOSET,              0,  { ZTID_MAPDATA },{} },
	{ "setDoorComboSet",            0,          ZTID_VOID,   MAPDATADOORCOMBOSET,              0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "getWarpReturnX[]",           0,         ZTID_FLOAT,   MAPDATAWARPRETX,                  0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setWarpReturnX[]",           0,          ZTID_VOID,   MAPDATAWARPRETX,                  0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getWarpReturnY[]",           0,         ZTID_FLOAT,   MAPDATAWARPRETY,                  0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setWarpReturnY[]",           0,          ZTID_VOID,   MAPDATAWARPRETY,                  0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getStairsX",                 0,         ZTID_FLOAT,   MAPDATASTAIRX,                    0,  { ZTID_MAPDATA },{} },
	{ "setStairsX",                 0,          ZTID_VOID,   MAPDATASTAIRX,                    0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "getStairsY",                 0,         ZTID_FLOAT,   MAPDATASTAIRY,                    0,  { ZTID_MAPDATA },{} },
	{ "setStairsY",                 0,          ZTID_VOID,   MAPDATASTAIRY,                    0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "getItemX",                   0,         ZTID_FLOAT,   MAPDATAITEMX,                     0,  { ZTID_MAPDATA },{} },
	{ "setItemX",                   0,          ZTID_VOID,   MAPDATAITEMX,                     0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "getItemY",                   0,         ZTID_FLOAT,   MAPDATAITEMY,                     0,  { ZTID_MAPDATA },{} },
	{ "setItemY",                   0,          ZTID_VOID,   MAPDATAITEMY,                     0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "getPalette",                 0,         ZTID_FLOAT,   MAPDATACOLOUR,                    0,  { ZTID_MAPDATA },{} },
	{ "setPalette",                 0,          ZTID_VOID,   MAPDATACOLOUR,                    0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "getDoor[]",                  0,         ZTID_FLOAT,   MAPDATADOOR,                      0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setDoor[]",                  0,          ZTID_VOID,   MAPDATADOOR,                      0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getTileWarpDMap[]",          0,         ZTID_FLOAT,   MAPDATATILEWARPDMAP,              0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setTileWarpDMap[]",          0,          ZTID_VOID,   MAPDATATILEWARPDMAP,              0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getTileWarpScreen[]",        0,         ZTID_FLOAT,   MAPDATATILEWARPSCREEN,            0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setTileWarpScreen[]",        0,          ZTID_VOID,   MAPDATATILEWARPSCREEN,            0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getExitDir",                 0,         ZTID_FLOAT,   MAPDATAEXITDIR,                   0,  { ZTID_MAPDATA },{} },
	{ "setExitDir",                 0,          ZTID_VOID,   MAPDATAEXITDIR,                   0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "getEnemy[]",                 0,         ZTID_FLOAT,   MAPDATAENEMY,                     0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setEnemy[]",                 0,          ZTID_VOID,   MAPDATAENEMY,                     0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getPattern",                 0,         ZTID_FLOAT,   MAPDATAPATTERN,                   0,  { ZTID_MAPDATA },{} },
	{ "setPattern",                 0,          ZTID_VOID,   MAPDATAPATTERN,                   0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "getSideWarpType[]",          0,         ZTID_FLOAT,   MAPDATASIDEWARPTYPE,              0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setSideWarpType[]",          0,          ZTID_VOID,   MAPDATASIDEWARPTYPE,              0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getSideWarpOverlay[]",       0,         ZTID_FLOAT,   MAPDATASIDEWARPOVFLAGS,           0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setSideWarpOverlay[]",       0,          ZTID_VOID,   MAPDATASIDEWARPOVFLAGS,           0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_BOOL },{} },
	
	{ "getWarpArrivalX",            0,         ZTID_FLOAT,   MAPDATAWARPARRIVALX,              0,  { ZTID_MAPDATA },{} },
	{ "setWarpArrivalX",            0,          ZTID_VOID,   MAPDATAWARPARRIVALX,              0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "getWarpArrivalY",            0,         ZTID_FLOAT,   MAPDATAWARPARRIVALY,              0,  { ZTID_MAPDATA },{} },
	{ "setWarpArrivalY",            0,          ZTID_VOID,   MAPDATAWARPARRIVALY,              0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "getMazePath[]",              0,         ZTID_FLOAT,   MAPDATAPATH,                      0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setMazePath[]",              0,          ZTID_VOID,   MAPDATAPATH,                      0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getSideWarpScreen[]",        0,         ZTID_FLOAT,   MAPDATASIDEWARPSC,                0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setSideWarpScreen[]",        0,          ZTID_VOID,   MAPDATASIDEWARPSC,                0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getSideWarpID[]",            0,         ZTID_FLOAT,   MAPDATASIDEWARPID,                0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setSideWarpID[]",            0,          ZTID_VOID,   MAPDATASIDEWARPID,                0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getTileWarpReturnSquare[]",  0,         ZTID_FLOAT,   MAPDATATWARPRETSQR,               0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setTileWarpReturnSquare[]",  0,          ZTID_VOID,   MAPDATATWARPRETSQR,               0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getSideWarpReturnSquare[]",  0,         ZTID_FLOAT,   MAPDATASWARPRETSQR,               0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setSideWarpReturnSquare[]",  0,          ZTID_VOID,   MAPDATASWARPRETSQR,               0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getSideWarpDMap[]",          0,         ZTID_FLOAT,   MAPDATASIDEWARPDMAP,              0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setSideWarpDMap[]",          0,          ZTID_VOID,   MAPDATASIDEWARPDMAP,              0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getUnderCombo",              0,         ZTID_FLOAT,   MAPDATAUNDERCOMBO,                0,  { ZTID_MAPDATA },{} },
	{ "setUnderCombo",              0,          ZTID_VOID,   MAPDATAUNDERCOMBO,                0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "getUnderCSet",               0,         ZTID_FLOAT,   MAPDATAUNDERCSET,                 0,  { ZTID_MAPDATA },{} },
	{ "setUnderCSet",               0,          ZTID_VOID,   MAPDATAUNDERCSET,                 0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "getCatchall",                0,         ZTID_FLOAT,   MAPDATACATCHALL,                  0,  { ZTID_MAPDATA },{} },
	{ "setCatchall",                0,          ZTID_VOID,   MAPDATACATCHALL,                  0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "getCSensitive",              0,         ZTID_FLOAT,   MAPDATACSENSITIVE,                0,  { ZTID_MAPDATA },{} },
	{ "setCSensitive",              0,          ZTID_VOID,   MAPDATACSENSITIVE,                0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "getNoReset",                 0,         ZTID_FLOAT,   MAPDATANORESET,                   0,  { ZTID_MAPDATA },{} },
	{ "setNoReset",                 0,          ZTID_VOID,   MAPDATANORESET,                   0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "getNoCarry",                 0,         ZTID_FLOAT,   MAPDATANOCARRY,                   0,  { ZTID_MAPDATA },{} },
	{ "setNoCarry",                 0,          ZTID_VOID,   MAPDATANOCARRY,                   0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "getLayerMap[]",              0,         ZTID_FLOAT,   MAPDATALAYERMAP,                  0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setLayerMap[]",              0,          ZTID_VOID,   MAPDATALAYERMAP,                  0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getLayerScreen[]",           0,         ZTID_FLOAT,   MAPDATALAYERSCREEN,               0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setLayerScreen[]",           0,          ZTID_VOID,   MAPDATALAYERSCREEN,               0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getLayerOpacity[]",          0,         ZTID_FLOAT,   MAPDATALAYEROPACITY,              0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setLayerOpacity[]",          0,          ZTID_VOID,   MAPDATALAYEROPACITY,              0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getLayerInvisible[]",        0,          ZTID_BOOL,   MAPDATALAYERINVIS,                0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setLayerInvisible[]",        0,          ZTID_VOID,   MAPDATALAYERINVIS,                0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_BOOL },{} },
	
	{ "getScriptDraws[]",           0,          ZTID_BOOL,   MAPDATASCRIPTDRAWS,               0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setScriptDraws[]",           0,          ZTID_VOID,   MAPDATASCRIPTDRAWS,               0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_BOOL },{} },
	
	
	{ "getTimedWarpTimer",          0,         ZTID_FLOAT,   MAPDATATIMEDWARPTICS,             0,  { ZTID_MAPDATA },{} },
	{ "setTimedWarpTimer",          0,          ZTID_VOID,   MAPDATATIMEDWARPTICS,             0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "getNextMap",                 0,         ZTID_FLOAT,   MAPDATANEXTMAP,                   0,  { ZTID_MAPDATA },{} },
	{ "setNextMap",                 0,          ZTID_VOID,   MAPDATANEXTMAP,                   0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "getNextScreen",              0,         ZTID_FLOAT,   MAPDATANEXTSCREEN,                0,  { ZTID_MAPDATA },{} },
	{ "setNextScreen",              0,          ZTID_VOID,   MAPDATANEXTSCREEN,                0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "getSecretCombo[]",           0,         ZTID_FLOAT,   MAPDATASECRETCOMBO,               0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setSecretCombo[]",           0,          ZTID_VOID,   MAPDATASECRETCOMBO,               0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getSecretCSet[]",            0,         ZTID_FLOAT,   MAPDATASECRETCSET,                0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setSecretCSet[]",            0,          ZTID_VOID,   MAPDATASECRETCSET,                0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getSecretFlags[]",           0,         ZTID_FLOAT,   MAPDATASECRETFLAG,                0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setSecretFlags[]",           0,          ZTID_VOID,   MAPDATASECRETFLAG,                0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getFFCData[]",               0,         ZTID_FLOAT,   MAPDATAFFDATA,                    0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setFFCData[]",               0,          ZTID_VOID,   MAPDATAFFDATA,                    0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getFFCCSet[]",               0,         ZTID_FLOAT,   MAPDATAFFCSET,                    0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setFFCCSet[]",               0,          ZTID_VOID,   MAPDATAFFCSET,                    0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getFFCDelay[]",              0,         ZTID_FLOAT,   MAPDATAFFDELAY,                   0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setFFCDelay[]",              0,          ZTID_VOID,   MAPDATAFFDELAY,                   0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getFFCX[]",                  0,         ZTID_FLOAT,   MAPDATAFFX,                       0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setFFCX[]",                  0,          ZTID_VOID,   MAPDATAFFX,                       0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getFFCY[]",                  0,         ZTID_FLOAT,   MAPDATAFFY,                       0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setFFCY[]",                  0,          ZTID_VOID,   MAPDATAFFY,                       0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getFFCVx[]",                 0,         ZTID_FLOAT,   MAPDATAFFXDELTA,                  0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setFFCVx[]",                 0,          ZTID_VOID,   MAPDATAFFXDELTA,                  0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getFFCVy[]",                 0,         ZTID_FLOAT,   MAPDATAFFYDELTA,                  0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setFFCVy[]",                 0,          ZTID_VOID,   MAPDATAFFYDELTA,                  0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getFFCAx[]",                 0,         ZTID_FLOAT,   MAPDATAFFXDELTA2,                 0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setFFCAx[]",                 0,          ZTID_VOID,   MAPDATAFFXDELTA2,                 0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getFFCAy[]",                 0,         ZTID_FLOAT,   MAPDATAFFYDELTA2,                 0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setFFCAy[]",                 0,          ZTID_VOID,   MAPDATAFFYDELTA2,                 0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getFFCFlags[]",              0,         ZTID_FLOAT,   MAPDATAFFFLAGS,                   0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setFFCFlags[]",              0,          ZTID_VOID,   MAPDATAFFFLAGS,                   0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getFFCEffectWidth[]",        0,         ZTID_FLOAT,   MAPDATAFFEFFECTWIDTH,             0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setFFCEffectWidth[]",        0,          ZTID_VOID,   MAPDATAFFEFFECTWIDTH,             0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getFFCEffectHeight[]",       0,         ZTID_FLOAT,   MAPDATAFFEFFECTHEIGHT,            0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setFFCEffectHeight[]",       0,          ZTID_VOID,   MAPDATAFFEFFECTHEIGHT,            0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getFFCTileWidth[]",          0,         ZTID_FLOAT,   MAPDATAFFWIDTH,                   0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setFFCTileWidth[]",          0,          ZTID_VOID,   MAPDATAFFWIDTH,                   0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getFFCTileHeight[]",         0,         ZTID_FLOAT,   MAPDATAFFHEIGHT,                  0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setFFCTileHeight[]",         0,          ZTID_VOID,   MAPDATAFFHEIGHT,                  0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getFFCLink[]",               0,         ZTID_FLOAT,   MAPDATAFFLINK,                    0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setFFCLink[]",               0,          ZTID_VOID,   MAPDATAFFLINK,                    0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getFFCScript[]",             0,         ZTID_FLOAT,   MAPDATAFFSCRIPT,                  0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setFFCScript[]",             0,          ZTID_VOID,   MAPDATAFFSCRIPT,                  0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getAmbientSFX",              0,         ZTID_FLOAT,   MAPDATAOCEANSFX,                  0,  { ZTID_MAPDATA },{} },
	{ "setAmbientSFX",              0,          ZTID_VOID,   MAPDATAOCEANSFX,                  0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "getBossSFX",                 0,         ZTID_FLOAT,   MAPDATABOSSSFX,                   0,  { ZTID_MAPDATA },{} },
	{ "setBossSFX",                 0,          ZTID_VOID,   MAPDATABOSSSFX,                   0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "getSecretSFX",               0,         ZTID_FLOAT,   MAPDATASECRETSFX,                 0,  { ZTID_MAPDATA },{} },
	{ "setSecretSFX",               0,          ZTID_VOID,   MAPDATASECRETSFX,                 0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "getItemSFX",                 0,         ZTID_FLOAT,   MAPDATAHOLDUPSFX,                 0,  { ZTID_MAPDATA },{} },
	{ "setItemSFX",                 0,          ZTID_VOID,   MAPDATAHOLDUPSFX,                 0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "getMIDI",                    0,         ZTID_FLOAT,   MAPDATASCREENMIDI,                0,  { ZTID_MAPDATA },{} },
	{ "setMIDI",                    0,          ZTID_VOID,   MAPDATASCREENMIDI,                0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "getFlags[]",                 0,         ZTID_FLOAT,   MAPDATAFLAGS,                     0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setFlags[]",                 0,          ZTID_VOID,   MAPDATAFLAGS,                     0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getComboD[]",                0,         ZTID_FLOAT,   MAPDATACOMBODD,                   0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setComboD[]",                0,          ZTID_VOID,   MAPDATACOMBODD,                   0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getComboC[]",                0,         ZTID_FLOAT,   MAPDATACOMBOCD,                   0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setComboC[]",                0,          ZTID_VOID,   MAPDATACOMBOCD,                   0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getComboF[]",                0,         ZTID_FLOAT,   MAPDATACOMBOFD,                   0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setComboF[]",                0,          ZTID_VOID,   MAPDATACOMBOFD,                   0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getComboI[]",                0,         ZTID_FLOAT,   MAPDATACOMBOID,                   0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setComboI[]",                0,          ZTID_VOID,   MAPDATACOMBOID,                   0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getComboT[]",                0,         ZTID_FLOAT,   MAPDATACOMBOTD,                   0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setComboT[]",                0,          ZTID_VOID,   MAPDATACOMBOTD,                   0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getComboS[]",                0,         ZTID_FLOAT,   MAPDATACOMBOSD,                   0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setComboS[]",                0,          ZTID_VOID,   MAPDATACOMBOSD,                   0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getComboE[]",                0,         ZTID_FLOAT,   MAPDATACOMBOED,                   0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setComboE[]",                0,          ZTID_VOID,   MAPDATACOMBOED,                   0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getState[]",                 0,          ZTID_BOOL,   MAPDATASCREENSTATED,              0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setState[]",                 0,          ZTID_VOID,   MAPDATASCREENSTATED,              0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getExState[]",               0,          ZTID_BOOL,   MAPDATAEXSTATED,                  0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setExState[]",               0,          ZTID_VOID,   MAPDATAEXSTATED,                  0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getEFlags[]",                0,         ZTID_FLOAT,   MAPDATASCREENEFLAGSD,             0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
//	{ "setEFlags[]",                0,          ZTID_VOID,   MAPDATASCREENEFLAGSD,             0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
//	Functions

	{ "GetFFCInitD",                0,       ZTID_UNTYPED,   -1,                          FL_INL,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetFFCInitD",                0,          ZTID_VOID,   -1,                               0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, ZTID_UNTYPED },{} },
	
	{ "getMap",                     0,         ZTID_FLOAT,   MAPDATAMAP,                       0,  { ZTID_MAPDATA },{} },
	{ "setMap",                     0,         ZTID_FLOAT,   MAPDATAMAP,                       0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "getScreen",                  0,         ZTID_FLOAT,   MAPDATASCREEN,                    0,  { ZTID_MAPDATA },{} },
	{ "setScreen",                  0,         ZTID_FLOAT,   MAPDATASCREEN,                    0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "getLensShows[]",             0,          ZTID_BOOL,   MAPDATALENSSHOWS,                 0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setLensShows[]",             0,          ZTID_VOID,   MAPDATALENSSHOWS,                 0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getLensHides[]",             0,          ZTID_BOOL,   MAPDATALENSHIDES,                 0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setLensHides[]",             0,          ZTID_VOID,   MAPDATALENSHIDES,                 0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_BOOL },{} },
	
	//Undocumented intentionally
	{ "GetFFCInitA",                0,         ZTID_FLOAT,   -1,                          FL_INL,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetFFCInitA",                0,         ZTID_FLOAT,   -1,                               0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getFFCRunning[]",            0,          ZTID_BOOL,   MAPDATAFFINITIALISED,             0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "setFFCRunning[]",            0,          ZTID_VOID,   MAPDATAFFINITIALISED,             0,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_BOOL },{} },
	
	//Renamed to AmbientSFX
	{ "getOceanSFX",                0,         ZTID_FLOAT,   MAPDATAOCEANSFX,                  0,  { ZTID_MAPDATA },{} },
	{ "setOceanSFX",                0,          ZTID_VOID,   MAPDATAOCEANSFX,                  0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	//Renamed to Palette
	{ "getCSet",                    0,         ZTID_FLOAT,   MAPDATACOLOUR,                    0,  { ZTID_MAPDATA },{} },
	{ "setCSet",                    0,          ZTID_VOID,   MAPDATACOLOUR,                    0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	//NextMap
	{ "getCarryoverMap",            0,         ZTID_FLOAT,   MAPDATANEXTMAP,                   0,  { ZTID_MAPDATA },{} },
	{ "setCarryoverMap",            0,          ZTID_VOID,   MAPDATANEXTMAP,                   0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	//NextScreen
	{ "getCarryoverScreen",         0,         ZTID_FLOAT,   MAPDATANEXTSCREEN,                0,  { ZTID_MAPDATA },{} },
	{ "setCarryoverSreen",          0,          ZTID_VOID,   MAPDATANEXTSCREEN,                0,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	//Deprecated
	{ "getD[]",                     0,       ZTID_UNTYPED,   MAPDATAMISCD,               FL_DEPR,  { ZTID_MAPDATA, ZTID_FLOAT },{},0,"mapdata->D[] has never worked!" },
	{ "setD[]",                     0,          ZTID_VOID,   MAPDATAMISCD,               FL_DEPR,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_UNTYPED },{},0,"mapdata->D[] has never worked!" },
	{ "getHasItem",                 0,         ZTID_FLOAT,   MAPDATAHASITEM,             FL_DEPR,  { ZTID_MAPDATA },{},0,"Check '->Item > -1' instead!" },
	{ "setHasItem",                 0,          ZTID_VOID,   MAPDATAHASITEM,             FL_DEPR,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "getSFlags[]",                0,         ZTID_FLOAT,   MAPDATASCREENFLAGSD,        FL_DEPR,  { ZTID_MAPDATA, ZTID_FLOAT },{},0,"Use '->Flags[]' instead!" },
//	{ "setSFlags[]",                0,          ZTID_VOID,   MAPDATASCREENFLAGSD,        FL_DEPR,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getWarpReturnC",             0,         ZTID_FLOAT,   MAPDATAWARPRETURNC,         FL_DEPR,  { ZTID_MAPDATA },{} },
	{ "setWarpReturnC",             0,          ZTID_VOID,   MAPDATAWARPRETURNC,         FL_DEPR,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "getEnemyFlags",              0,         ZTID_FLOAT,   MAPDATAENEMYFLAGS,          FL_DEPR,  { ZTID_MAPDATA },{} },
	{ "setEnemyFlags",              0,          ZTID_VOID,   MAPDATAENEMYFLAGS,          FL_DEPR,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "getSideWarpIndex",           0,         ZTID_FLOAT,   MAPDATASIDEWARPINDEX,       FL_DEPR,  { ZTID_MAPDATA },{} },
	{ "setSideWarpIndex",           0,          ZTID_VOID,   MAPDATASIDEWARPINDEX,       FL_DEPR,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "getViewX",                   0,         ZTID_FLOAT,   MAPDATAVIEWX,               FL_DEPR,  { ZTID_MAPDATA },{} },
	{ "setViewX",                   0,          ZTID_VOID,   MAPDATAVIEWX,               FL_DEPR,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "getViewY",                   0,         ZTID_FLOAT,   MAPDATAVIEWY,               FL_DEPR,  { ZTID_MAPDATA },{} },
	{ "setViewY",                   0,          ZTID_VOID,   MAPDATAVIEWY,               FL_DEPR,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "getWidth",                   0,         ZTID_FLOAT,   MAPDATASCREENWIDTH,         FL_DEPR,  { ZTID_MAPDATA },{} },
	{ "setWidth",                   0,          ZTID_VOID,   MAPDATASCREENWIDTH,         FL_DEPR,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "getHeight",                  0,         ZTID_FLOAT,   MAPDATASCREENHEIGHT,        FL_DEPR,  { ZTID_MAPDATA },{} },
	{ "setHeight",                  0,          ZTID_VOID,   MAPDATASCREENHEIGHT,        FL_DEPR,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "getEntryX",                  0,         ZTID_FLOAT,   MAPDATAENTRYX,              FL_DEPR,  { ZTID_MAPDATA },{} },
	{ "setEntryX",                  0,          ZTID_VOID,   MAPDATAENTRYX,              FL_DEPR,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "getEntryY",                  0,         ZTID_FLOAT,   MAPDATAENTRYY,              FL_DEPR,  { ZTID_MAPDATA },{} },
	{ "setEntryY",                  0,          ZTID_VOID,   MAPDATAENTRYY,              FL_DEPR,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "getNumFFCs[]",               0,          ZTID_BOOL,   MAPDATANUMFF,               FL_DEPR,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	//read-only, for now?
	{ "setNumFFCs[]",               0,          ZTID_VOID,   MAPDATANUMFF,               FL_DEPR,  { ZTID_MAPDATA, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getScriptEntry",             0,         ZTID_FLOAT,   MAPDATASCRIPTENTRY,         FL_DEPR,  { ZTID_MAPDATA },{} },
	{ "setScriptEntry",             0,          ZTID_VOID,   MAPDATASCRIPTENTRY,         FL_DEPR,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "getScriptOccupancy",         0,         ZTID_FLOAT,   MAPDATASCRIPTOCCUPANCY,     FL_DEPR,  { ZTID_MAPDATA },{} },
	{ "setScriptOccupancy",         0,          ZTID_VOID,   MAPDATASCRIPTOCCUPANCY,     FL_DEPR,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "getExitScript",              0,         ZTID_FLOAT,   MAPDATASCRIPTEXIT,          FL_DEPR,  { ZTID_MAPDATA },{} },
	{ "setExitScript",              0,          ZTID_VOID,   MAPDATASCRIPTEXIT,          FL_DEPR,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	{ "getLensLayer",               0,         ZTID_FLOAT,   MAPDATALENSLAYER,           FL_DEPR,  { ZTID_MAPDATA },{} },
	{ "setLensLayer",               0,          ZTID_VOID,   MAPDATALENSLAYER,           FL_DEPR,  { ZTID_MAPDATA, ZTID_FLOAT },{} },
	
	{ "",                           0,          ZTID_VOID,   -1,                               0,  {},{} }
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
		Function* function = getFunction("isSolid");
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
		Function* function = getFunction("isSolidLayer");
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
		Function* function = getFunction("GetFFCInitD");
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
		Function* function = getFunction("SetFFCInitD");
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
		Function* function = getFunction("GetFFCInitA");
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
		Function* function = getFunction("SetFFCInitA");
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

