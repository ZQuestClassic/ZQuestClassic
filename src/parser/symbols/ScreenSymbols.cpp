#include "SymbolDefs.h"

ScreenSymbols ScreenSymbols::singleton = ScreenSymbols();

static AccessorTable2 ScreenTable2[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
	{ "getD[]",                     0,       ZTID_UNTYPED,   SDD,                       0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setD[]",                     0,          ZTID_VOID,   SDD,                       0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getComboD[]",                0,         ZTID_FLOAT,   COMBODD,                   0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setComboD[]",                0,          ZTID_VOID,   COMBODD,                   0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getComboC[]",                0,         ZTID_FLOAT,   COMBOCD,                   0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setComboC[]",                0,          ZTID_VOID,   COMBOCD,                   0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getComboF[]",                0,         ZTID_FLOAT,   COMBOFD,                   0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setComboF[]",                0,          ZTID_VOID,   COMBOFD,                   0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getComboI[]",                0,         ZTID_FLOAT,   COMBOID,                   0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setComboI[]",                0,          ZTID_VOID,   COMBOID,                   0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getComboT[]",                0,         ZTID_FLOAT,   COMBOTD,                   0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setComboT[]",                0,          ZTID_VOID,   COMBOTD,                   0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getComboS[]",                0,         ZTID_FLOAT,   COMBOSD,                   0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setComboS[]",                0,          ZTID_VOID,   COMBOSD,                   0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getComboE[]",                0,         ZTID_FLOAT,   COMBOED,                   0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setComboE[]",                0,          ZTID_VOID,   COMBOED,                   0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getDoor[]",                  0,         ZTID_FLOAT,   SCRDOORD,                  0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setDoor[]",                  0,          ZTID_VOID,   SCRDOORD,                  0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getState[]",                 0,          ZTID_BOOL,   SCREENSTATED,              0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setState[]",                 0,          ZTID_VOID,   SCREENSTATED,              0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getExState[]",               0,          ZTID_BOOL,   SCREENEXSTATED,            0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setExState[]",               0,          ZTID_VOID,   SCREENEXSTATED,            0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getLit",                     0,          ZTID_BOOL,   LIT,                       0,  { ZTID_SCREEN },{} },
	{ "setLit",                     0,          ZTID_VOID,   LIT,                       0,  { ZTID_SCREEN, ZTID_BOOL },{} },
	{ "getWavy",                    0,         ZTID_FLOAT,   WAVY,                      0,  { ZTID_SCREEN },{} },
	{ "setWavy",                    0,          ZTID_VOID,   WAVY,                      0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "getQuake",                   0,         ZTID_FLOAT,   QUAKE,                     0,  { ZTID_SCREEN },{} },
	{ "setQuake",                   0,          ZTID_VOID,   QUAKE,                     0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "_getNumItems",               0,         ZTID_FLOAT,   ITEMCOUNT,                 0,  { ZTID_SCREEN },{} },
	{ "GetRenderTarget",            0,         ZTID_FLOAT,   GETRENDERTARGET,           0,  { ZTID_SCREEN },{} },
	{ "LoadItem",                   0,          ZTID_ITEM,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "CreateItem",                 0,          ZTID_ITEM,   -1,                   FL_INL,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "LoadFFC",                    0,           ZTID_FFC,   -1,                   FL_INL,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "_getNumNPCs",                0,         ZTID_FLOAT,   NPCCOUNT,                  0,  { ZTID_SCREEN },{} },
	{ "LoadNPC",                    0,           ZTID_NPC,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "CreateNPC",                  0,           ZTID_NPC,   -1,                   FL_INL,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "ClearSprites",               0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "Rectangle",                  0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL, ZTID_FLOAT },{} },
	{ "Circle",                     0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL, ZTID_FLOAT },{} },
	{ "Arc",                        0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL, ZTID_BOOL, ZTID_FLOAT },{} },
	{ "Ellipse",                    0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL, ZTID_FLOAT },{} },
	{ "Line",                       0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "Spline",                     0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "PutPixel",                   0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "PutPixels",                  0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "DrawTiles",                  0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "DrawCombos",                 0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "Lines",                      0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "DrawCharacter",              0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "DrawInteger",                0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "DrawTile",                   0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL, ZTID_FLOAT },{} },
	{ "DrawTileCloaked",            0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "DrawCombo",                  0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL, ZTID_FLOAT },{} },
	{ "DrawComboCloaked",           0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "Quad",                       0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "Triangle",                   0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "Quad3D",                     0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "Triangle3D",                 0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "FastTile",                   0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "FastCombo",                  0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "DrawString",                 0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "DrawString",                 1,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "DrawLayer",                  0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "DrawScreen",                 0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "DrawBitmap",                 0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "DrawBitmapEx",               0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "SetRenderTarget",            0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "Message",                    0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "getShowingMessage",          0,         ZTID_FLOAT,   SHOWNMSG,                  0,  { ZTID_SCREEN },{} },
	{ "setShowingMessage",          0,          ZTID_VOID,   SHOWNMSG,                  0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "_getNumLWeapons",            0,         ZTID_FLOAT,   LWPNCOUNT,                 0,  { ZTID_SCREEN },{} },
	{ "LoadLWeapon",                0,          ZTID_LWPN,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "CreateLWeapon",              0,          ZTID_LWPN,   -1,                   FL_INL,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "_getNumEWeapons",            0,         ZTID_FLOAT,   EWPNCOUNT,                 0,  { ZTID_SCREEN },{} },
	{ "LoadEWeapon",                0,          ZTID_EWPN,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "CreateEWeapon",              0,          ZTID_EWPN,   -1,                   FL_INL,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "isSolid",                    0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "isSolidLayer",               0,          ZTID_BOOL,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetSideWarp",                0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetTileWarp",                0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "LayerScreen",                0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "LayerMap",                   0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "getFlags[]",                 0,         ZTID_FLOAT,   SCREENFLAGSD,              0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setFlags[]",                 0,          ZTID_VOID,   SCREENFLAGSD,              0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
//	This is read-only, but it was not previously blocked! -Z
	{ "getEFlags[]",                0,         ZTID_FLOAT,   SCREENEFLAGSD,             0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
//	This is read-only, but it was not previously blocked! -Z
	{ "setEFlags[]",                0,          ZTID_VOID,   SCREENEFLAGSD,             0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "TriggerSecrets",             0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_SCREEN },{} },
	{ "getRoomType",                0,         ZTID_FLOAT,   ROOMTYPE,                  0,  { ZTID_SCREEN },{} },
	{ "setRoomType",                0,          ZTID_VOID,   ROOMTYPE,                  0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "getRoomData",                0,         ZTID_FLOAT,   ROOMDATA,                  0,  { ZTID_SCREEN },{} },
	{ "setRoomData",                0,          ZTID_VOID,   ROOMDATA,                  0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "getMovingBlockX",            0,         ZTID_FLOAT,   PUSHBLOCKX,                0,  { ZTID_SCREEN },{} },
	{ "setMovingBlockX",            0,          ZTID_VOID,   PUSHBLOCKX,                0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "getMovingBlockY",            0,         ZTID_FLOAT,   PUSHBLOCKY,                0,  { ZTID_SCREEN },{} },
	{ "setMovingBlockY",            0,          ZTID_VOID,   PUSHBLOCKY,                0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "getMovingBlockLayer",        0,         ZTID_FLOAT,   PUSHBLOCKLAYER,            0,  { ZTID_SCREEN },{} },
	{ "setMovingBlockLayer",        0,          ZTID_VOID,   PUSHBLOCKLAYER,            0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "getMovingBlockCombo",        0,         ZTID_FLOAT,   PUSHBLOCKCOMBO,            0,  { ZTID_SCREEN },{} },
	{ "setMovingBlockCombo",        0,          ZTID_VOID,   PUSHBLOCKCOMBO,            0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "getMovingBlockCSet",         0,         ZTID_FLOAT,   PUSHBLOCKCSET,             0,  { ZTID_SCREEN },{} },
	{ "setMovingBlockCSet",         0,          ZTID_VOID,   PUSHBLOCKCSET,             0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "getUnderCombo",              0,         ZTID_FLOAT,   UNDERCOMBO,                0,  { ZTID_SCREEN },{} },
	{ "setUnderCombo",              0,          ZTID_VOID,   UNDERCOMBO,                0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "getUnderCSet",               0,         ZTID_FLOAT,   UNDERCSET,                 0,  { ZTID_SCREEN },{} },
	{ "setUnderCSet",               0,          ZTID_VOID,   UNDERCSET,                 0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "GetSideWarpDMap",            0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "GetSideWarpScreen",          0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "GetSideWarpType",            0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "GetTileWarpDMap",            0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "GetTileWarpScreen",          0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "GetTileWarpType",            0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "ZapIn",                      0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_SCREEN },{} },
	{ "ZapOut",                     0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_SCREEN },{} },
	{ "WavyIn",                     0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_SCREEN },{} },
	{ "WavyOut",                    0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_SCREEN },{} },
	{ "OpeningWipe",                0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_SCREEN },{} },
	{ "ClosingWipe",                0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_SCREEN },{} },
	{ "OpeningWipe",                1,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "ClosingWipe",                1,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "CreateLWeaponDx",            0,          ZTID_LWPN,   -1,                   FL_INL,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "Polygon",                    0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "TriggerSecret",              0,          ZTID_VOID,   -1,                        0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
//	mapdata m-> class variables copied to Screen->
	{ "getValid",                   0,         ZTID_FLOAT,   SCREENDATAVALID,           0,  { ZTID_SCREEN },{} },
	{ "setValid",                   0,          ZTID_VOID,   SCREENDATAVALID,           0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getGuy",                     0,         ZTID_FLOAT,   SCREENDATAGUY,             0,  { ZTID_SCREEN },{} },
	{ "setGuy",                     0,          ZTID_VOID,   SCREENDATAGUY,             0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getString",                  0,         ZTID_FLOAT,   SCREENDATASTRING,          0,  { ZTID_SCREEN },{} },
	{ "setString",                  0,          ZTID_VOID,   SCREENDATASTRING,          0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
/*	
	{ "getRoomType",                1,         ZTID_FLOAT,   SCREENDATAROOM,            0,  { ZTID_SCREEN },{} },
	{ "setRoomType",                1,          ZTID_VOID,   SCREENDATAROOM,            0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
*/	
	
	{ "getItem",                    0,         ZTID_FLOAT,   SCREENDATAITEM,            0,  { ZTID_SCREEN },{} },
	{ "setItem",                    0,          ZTID_VOID,   SCREENDATAITEM,            0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getHasItem",                 0,         ZTID_FLOAT,   SCREENDATAHASITEM,         0,  { ZTID_SCREEN },{} },
	{ "setHasItem",                 0,          ZTID_VOID,   SCREENDATAHASITEM,         0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getTileWarpType[]",          0,         ZTID_FLOAT,   SCREENDATATILEWARPTYPE,    0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setTileWarpType[]",          0,          ZTID_VOID,   SCREENDATATILEWARPTYPE,    0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getTileWarpOverlay[]",       0,          ZTID_BOOL,   SCREENDATATILEWARPOVFLAGS, 0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setTileWarpOverlay[]",       0,          ZTID_VOID,   SCREENDATATILEWARPOVFLAGS, 0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_BOOL },{} },
	
	{ "getDoorComboSet",            0,         ZTID_FLOAT,   SCREENDATADOORCOMBOSET,    0,  { ZTID_SCREEN },{} },
	{ "setDoorComboSet",            0,          ZTID_VOID,   SCREENDATADOORCOMBOSET,    0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getWarpReturnX[]",           0,         ZTID_FLOAT,   SCREENDATAWARPRETX,        0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setWarpReturnX[]",           0,          ZTID_VOID,   SCREENDATAWARPRETX,        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getWarpReturnY[]",           0,         ZTID_FLOAT,   SCREENDATAWARPRETY,        0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setWarpReturnY[]",           0,          ZTID_VOID,   SCREENDATAWARPRETY,        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getWarpReturnC",             0,         ZTID_FLOAT,   SCREENDATAWARPRETURNC,     0,  { ZTID_SCREEN },{} },
	{ "setWarpReturnC",             0,          ZTID_VOID,   SCREENDATAWARPRETURNC,     0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getStairsX",                 0,         ZTID_FLOAT,   SCREENDATASTAIRX,          0,  { ZTID_SCREEN },{} },
	{ "setStairsX",                 0,          ZTID_VOID,   SCREENDATASTAIRX,          0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getStairsY",                 0,         ZTID_FLOAT,   SCREENDATASTAIRY,          0,  { ZTID_SCREEN },{} },
	{ "setStairsY",                 0,          ZTID_VOID,   SCREENDATASTAIRY,          0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getItemX",                   0,         ZTID_FLOAT,   SCREENDATAITEMX,           0,  { ZTID_SCREEN },{} },
	{ "setItemX",                   0,          ZTID_VOID,   SCREENDATAITEMX,           0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getItemY",                   0,         ZTID_FLOAT,   SCREENDATAITEMY,           0,  { ZTID_SCREEN },{} },
	{ "setItemY",                   0,          ZTID_VOID,   SCREENDATAITEMY,           0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getCSet",                    0,         ZTID_FLOAT,   SCREENDATACOLOUR,          0,  { ZTID_SCREEN },{} },
	{ "setCSet",                    0,          ZTID_VOID,   SCREENDATACOLOUR,          0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "getPalette",                 0,         ZTID_FLOAT,   SCREENDATACOLOUR,          0,  { ZTID_SCREEN },{} },
	{ "setPalette",                 0,          ZTID_VOID,   SCREENDATACOLOUR,          0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getEnemyFlags",              0,         ZTID_FLOAT,   SCREENDATAENEMYFLAGS,      0,  { ZTID_SCREEN },{} },
	{ "setEnemyFlags",              0,          ZTID_VOID,   SCREENDATAENEMYFLAGS,      0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getTileWarpDMap[]",          0,         ZTID_FLOAT,   SCREENDATATILEWARPDMAP,    0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setTileWarpDMap[]",          0,          ZTID_VOID,   SCREENDATATILEWARPDMAP,    0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getTileWarpScreen[]",        0,         ZTID_FLOAT,   SCREENDATATILEWARPSCREEN,  0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setTileWarpScreen[]",        0,          ZTID_VOID,   SCREENDATATILEWARPSCREEN,  0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getExitDir",                 0,         ZTID_FLOAT,   SCREENDATAEXITDIR,         0,  { ZTID_SCREEN },{} },
	{ "setExitDir",                 0,          ZTID_VOID,   SCREENDATAEXITDIR,         0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getEnemy[]",                 0,         ZTID_FLOAT,   SCREENDATAENEMY,           0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setEnemy[]",                 0,          ZTID_VOID,   SCREENDATAENEMY,           0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getPattern",                 0,         ZTID_FLOAT,   SCREENDATAPATTERN,         0,  { ZTID_SCREEN },{} },
	{ "setPattern",                 0,          ZTID_VOID,   SCREENDATAPATTERN,         0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getSideWarpType[]",          0,         ZTID_FLOAT,   SCREENDATASIDEWARPTYPE,    0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setSideWarpType[]",          0,          ZTID_VOID,   SCREENDATASIDEWARPTYPE,    0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getSideWarpOverlay[]",       0,          ZTID_BOOL,   SCREENDATASIDEWARPOVFLAGS, 0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setSideWarpOverlay[]",       0,          ZTID_VOID,   SCREENDATASIDEWARPOVFLAGS, 0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_BOOL },{} },
	
	{ "getWarpArrivalX",            0,         ZTID_FLOAT,   SCREENDATAWARPARRIVALX,    0,  { ZTID_SCREEN },{} },
	{ "setWarpArrivalX",            0,          ZTID_VOID,   SCREENDATAWARPARRIVALX,    0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getWarpArrivalY",            0,         ZTID_FLOAT,   SCREENDATAWARPARRIVALY,    0,  { ZTID_SCREEN },{} },
	{ "setWarpArrivalY",            0,          ZTID_VOID,   SCREENDATAWARPARRIVALY,    0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getMazePath[]",              0,         ZTID_FLOAT,   SCREENDATAPATH,            0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setMazePath[]",              0,          ZTID_VOID,   SCREENDATAPATH,            0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getSideWarpScreen[]",        0,         ZTID_FLOAT,   SCREENDATASIDEWARPSC,      0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setSideWarpScreen[]",        0,          ZTID_VOID,   SCREENDATASIDEWARPSC,      0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getSideWarpID[]",            0,         ZTID_FLOAT,   SCREENSIDEWARPID,          0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setSideWarpID[]",            0,          ZTID_VOID,   SCREENSIDEWARPID,          0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getTileWarpReturnSquare[]",  0,         ZTID_FLOAT,   SCREENDATATWARPRETSQR,     0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setTileWarpReturnSquare[]",  0,          ZTID_VOID,   SCREENDATATWARPRETSQR,     0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getSideWarpReturnSquare[]",  0,         ZTID_FLOAT,   SCREENDATASWARPRETSQR,     0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setSideWarpReturnSquare[]",  0,          ZTID_VOID,   SCREENDATASWARPRETSQR,     0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getSideWarpDMap[]",          0,         ZTID_FLOAT,   SCREENDATASIDEWARPDMAP,    0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setSideWarpDMap[]",          0,          ZTID_VOID,   SCREENDATASIDEWARPDMAP,    0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getSideWarpIndex",           0,         ZTID_FLOAT,   SCREENDATASIDEWARPINDEX,   0,  { ZTID_SCREEN },{} },
	{ "setSideWarpIndex",           0,          ZTID_VOID,   SCREENDATASIDEWARPINDEX,   0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
/*	
	{ "getUnderCombo",              1,         ZTID_FLOAT,   SCREENDATAUNDERCOMBO,      0,  { ZTID_SCREEN },{} },
	{ "setUnderCombo",              1,          ZTID_VOID,   SCREENDATAUNDERCOMBO,      0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
*/	
	
/*	
	{ "getUnderCSet",               1,         ZTID_FLOAT,   SCREENDATAUNDERCSET,       0,  { ZTID_SCREEN },{} },
	{ "setUnderCSet",               1,          ZTID_VOID,   SCREENDATAUNDERCSET,       0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
*/	
	
	{ "getCatchall",                0,         ZTID_FLOAT,   SCREENDATACATCHALL,        0,  { ZTID_SCREEN },{} },
	{ "setCatchall",                0,          ZTID_VOID,   SCREENDATACATCHALL,        0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getCSensitive",              0,         ZTID_FLOAT,   SCREENDATACSENSITIVE,      0,  { ZTID_SCREEN },{} },
	{ "setCSensitive",              0,          ZTID_VOID,   SCREENDATACSENSITIVE,      0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getNoReset",                 0,         ZTID_FLOAT,   SCREENDATANORESET,         0,  { ZTID_SCREEN },{} },
	{ "setNoReset",                 0,          ZTID_VOID,   SCREENDATANORESET,         0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getNoCarry",                 0,         ZTID_FLOAT,   SCREENDATANOCARRY,         0,  { ZTID_SCREEN },{} },
	{ "setNoCarry",                 0,          ZTID_VOID,   SCREENDATANOCARRY,         0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getLayerMap[]",              0,         ZTID_FLOAT,   SCREENDATALAYERMAP,        0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setLayerMap[]",              0,          ZTID_VOID,   SCREENDATALAYERMAP,        0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getLayerScreen[]",           0,         ZTID_FLOAT,   SCREENDATALAYERSCREEN,     0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setLayerScreen[]",           0,          ZTID_VOID,   SCREENDATALAYERSCREEN,     0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getLayerOpacity[]",          0,         ZTID_FLOAT,   SCREENDATALAYEROPACITY,    0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setLayerOpacity[]",          0,          ZTID_VOID,   SCREENDATALAYEROPACITY,    0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getLayerInvisible[]",        0,          ZTID_BOOL,   SCREENDATALAYERINVIS,      0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setLayerInvisible[]",        0,          ZTID_VOID,   SCREENDATALAYERINVIS,      0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_BOOL },{} },
	
	{ "getScriptDraws[]",           0,          ZTID_BOOL,   SCREENDATASCRIPTDRAWS,     0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setScriptDraws[]",           0,          ZTID_VOID,   SCREENDATASCRIPTDRAWS,     0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_BOOL },{} },
	
	{ "getTimedWarpTimer",          0,         ZTID_FLOAT,   SCREENDATATIMEDWARPTICS,   0,  { ZTID_SCREEN },{} },
	{ "setTimedWarpTimer",          0,          ZTID_VOID,   SCREENDATATIMEDWARPTICS,   0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getNextMap",                 0,         ZTID_FLOAT,   SCREENDATANEXTMAP,         0,  { ZTID_SCREEN },{} },
	{ "setNextMap",                 0,          ZTID_VOID,   SCREENDATANEXTMAP,         0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getNextScreen",              0,         ZTID_FLOAT,   SCREENDATANEXTSCREEN,      0,  { ZTID_SCREEN },{} },
	{ "setNextScreen",              0,          ZTID_VOID,   SCREENDATANEXTSCREEN,      0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	
	{ "getCarryoverMap",            0,         ZTID_FLOAT,   SCREENDATANEXTMAP,         0,  { ZTID_SCREEN },{} },
	{ "setCarryoverMap",            0,          ZTID_VOID,   SCREENDATANEXTMAP,         0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getCarryoverScreen",         0,         ZTID_FLOAT,   SCREENDATANEXTSCREEN,      0,  { ZTID_SCREEN },{} },
	{ "setCarryoverScreen",         0,          ZTID_VOID,   SCREENDATANEXTSCREEN,      0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getSecretCombo[]",           0,         ZTID_FLOAT,   SCREENDATASECRETCOMBO,     0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setSecretCombo[]",           0,          ZTID_VOID,   SCREENDATASECRETCOMBO,     0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getSecretCSet[]",            0,         ZTID_FLOAT,   SCREENDATASECRETCSET,      0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setSecretCSet[]",            0,          ZTID_VOID,   SCREENDATASECRETCSET,      0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getSecretFlags[]",           0,         ZTID_FLOAT,   SCREENDATASECRETFLAG,      0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setSecretFlags[]",           0,          ZTID_VOID,   SCREENDATASECRETFLAG,      0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getViewX",                   0,         ZTID_FLOAT,   SCREENDATAVIEWX,           0,  { ZTID_SCREEN },{} },
	{ "setViewX",                   0,          ZTID_VOID,   SCREENDATAVIEWX,           0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getViewY",                   0,         ZTID_FLOAT,   SCREENDATAVIEWY,           0,  { ZTID_SCREEN },{} },
	{ "setViewY",                   0,          ZTID_VOID,   SCREENDATAVIEWY,           0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getWidth",                   0,         ZTID_FLOAT,   SCREENDATASCREENWIDTH,     0,  { ZTID_SCREEN },{} },
	{ "setWidth",                   0,          ZTID_VOID,   SCREENDATASCREENWIDTH,     0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getHeight",                  0,         ZTID_FLOAT,   SCREENDATASCREENHEIGHT,    0,  { ZTID_SCREEN },{} },
	{ "setHeight",                  0,          ZTID_VOID,   SCREENDATASCREENHEIGHT,    0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getEntryX",                  0,         ZTID_FLOAT,   SCREENDATAENTRYX,          0,  { ZTID_SCREEN },{} },
	{ "setEntryX",                  0,          ZTID_VOID,   SCREENDATAENTRYX,          0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getEntryY",                  0,         ZTID_FLOAT,   SCREENDATAENTRYY,          0,  { ZTID_SCREEN },{} },
	{ "setEntryY",                  0,          ZTID_VOID,   SCREENDATAENTRYY,          0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getNumFFCs[]",               0,          ZTID_BOOL,   SCREENDATANUMFF,           0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	//read-only, for now?
	{ "setNumFFCs[]",               0,          ZTID_VOID,   SCREENDATANUMFF,           0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_BOOL },{} },
	
	{ "getScriptEntry",             0,         ZTID_FLOAT,   SCREENDATASCRIPTENTRY,     0,  { ZTID_SCREEN },{} },
	{ "setScriptEntry",             0,          ZTID_VOID,   SCREENDATASCRIPTENTRY,     0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getScriptOccupancy",         0,         ZTID_FLOAT,   SCREENDATASCRIPTOCCUPANCY, 0,  { ZTID_SCREEN },{} },
	{ "setScriptOccupancy",         0,          ZTID_VOID,   SCREENDATASCRIPTOCCUPANCY, 0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getExitScript",              0,         ZTID_FLOAT,   SCREENDATASCRIPTEXIT,      0,  { ZTID_SCREEN },{} },
	{ "setExitScript",              0,          ZTID_VOID,   SCREENDATASCRIPTEXIT,      0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getOceanSFX",                0,         ZTID_FLOAT,   SCREENDATAOCEANSFX,        0,  { ZTID_SCREEN },{} },
	{ "setOceanSFX",                0,          ZTID_VOID,   SCREENDATAOCEANSFX,        0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "getAmbientSFX",              0,         ZTID_FLOAT,   SCREENDATAOCEANSFX,        0,  { ZTID_SCREEN },{} },
	{ "setAmbientSFX",              0,          ZTID_VOID,   SCREENDATAOCEANSFX,        0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getBossSFX",                 0,         ZTID_FLOAT,   SCREENDATABOSSSFX,         0,  { ZTID_SCREEN },{} },
	{ "setBossSFX",                 0,          ZTID_VOID,   SCREENDATABOSSSFX,         0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getSecretSFX",               0,         ZTID_FLOAT,   SCREENDATASECRETSFX,       0,  { ZTID_SCREEN },{} },
	{ "setSecretSFX",               0,          ZTID_VOID,   SCREENDATASECRETSFX,       0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getItemSFX",                 0,         ZTID_FLOAT,   SCREENDATAHOLDUPSFX,       0,  { ZTID_SCREEN },{} },
	{ "setItemSFX",                 0,          ZTID_VOID,   SCREENDATAHOLDUPSFX,       0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getMIDI",                    0,         ZTID_FLOAT,   SCREENDATASCREENMIDI,      0,  { ZTID_SCREEN },{} },
	{ "setMIDI",                    0,          ZTID_VOID,   SCREENDATASCREENMIDI,      0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getLensLayer",               0,         ZTID_FLOAT,   SCREENDATALENSLAYER,       0,  { ZTID_SCREEN },{} },
	{ "setLensLayer",               0,          ZTID_VOID,   SCREENDATALENSLAYER,       0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getScreenFlags[]",           0,         ZTID_FLOAT,   SCREENDATAFLAGS,           0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setScreenFlags[]",           0,          ZTID_VOID,   SCREENDATAFLAGS,           0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getScript",                  0,         ZTID_FLOAT,   SCREENSCRIPT,              0,  { ZTID_SCREEN },{} },
	{ "setScript",                  0,          ZTID_VOID,   SCREENSCRIPT,              0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "getInitD[]",                 0,       ZTID_UNTYPED,   SCREENINITD,               0,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "setInitD[]",                 0,          ZTID_VOID,   SCREENINITD,               0,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_UNTYPED },{} },
	
	{ "LoadNPCByUID",               0,           ZTID_NPC,   -1,                   FL_INL,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "LoadLWeaponByUID",           0,          ZTID_LWPN,   -1,                   FL_INL,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	{ "LoadEWeaponByUID",           0,          ZTID_EWPN,   -1,                   FL_INL,  { ZTID_SCREEN, ZTID_FLOAT },{} },
	
	{ "DrawFrame",                  0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL, ZTID_FLOAT },{} },
	
	{ "_getSecretsTriggered",       0,          ZTID_BOOL,   SCREENSECRETSTRIGGERED,    0,  { ZTID_SCREEN },{} },
	
	{ "SpawnScreenEnemies",         0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_SCREEN },{} },
	{ "TriggerCombo",               0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
};

ScreenSymbols::ScreenSymbols()
{
	table2 = ScreenTable2;
	refVar = NUL;
}

void ScreenSymbols::generateCode()
{
	//item LoadItem(screen, int32_t)
	{
		Function* function = getFunction2("LoadItem");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		//convert from 1-index to 0-index
		addOpcode2 (code, new OSubImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
		addOpcode2 (code, new OLoadItemRegister(new VarArgument(EXP1)));
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(REFITEM)));
		RETURN();
		function->giveCode(code);
	}
	//item CreateItem(screen, int32_t)
	{
		Function* function = getFunction2("CreateItem");
		
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OCreateItemRegister(new VarArgument(EXP1)));
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(REFITEM)));
		RETURN();
		function->giveCode(code);
	}
	//ffc LoadFFC(screen, int32_t)
	{
		Function* function = getFunction2("LoadFFC");
		
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		//addOpcode2 (code, new OSetRegister(new VarArgument(REFFFC), new VarArgument(EXP1)));
		addOpcode2 (code, new OSubImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
		RETURN();
		function->giveCode(code);
	}
	//npc LoadNPC(screen, int32_t)
	{
		Function* function = getFunction2("LoadNPC");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		//convert from 1-index to 0-index
		addOpcode2 (code, new OSubImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
		addOpcode2 (code, new OLoadNPCRegister(new VarArgument(EXP1)));
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(REFNPC)));
		RETURN();
		function->giveCode(code);
	}
	//npc CreateNPC(screen, int32_t)
	{
		Function* function = getFunction2("CreateNPC");
		
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OCreateNPCRegister(new VarArgument(EXP1)));
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(REFNPC)));
		RETURN();
		function->giveCode(code);
	}
	//npc LoadLWeapon(screen, int32_t)
	{
		Function* function = getFunction2("LoadLWeapon");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		//convert from 1-index to 0-index
		addOpcode2 (code, new OSubImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
		addOpcode2 (code, new OLoadLWpnRegister(new VarArgument(EXP1)));
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(REFLWPN)));
		RETURN();
		function->giveCode(code);
	}
	//npc CreateLWeapon(screen, int32_t)
	{
		Function* function = getFunction2("CreateLWeapon");
		
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OCreateLWpnRegister(new VarArgument(EXP1)));
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(REFLWPN)));
		RETURN();
		function->giveCode(code);
	}
	
	//lweapon CreateLWeaponDX(screen, int32_t type, int32_t itemid)
	{
		Function* function = getFunction2("CreateLWeaponDx");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(CREATELWPNDX)));
		RETURN();
		function->giveCode(code);
	}
	 
	//ewpn LoadEWeapon(screen, int32_t)
	{
		Function* function = getFunction2("LoadEWeapon");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		//convert from 1-index to 0-index
		addOpcode2 (code, new OSubImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
		addOpcode2 (code, new OLoadEWpnRegister(new VarArgument(EXP1)));
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(REFEWPN)));
		RETURN();
		function->giveCode(code);
	}
	//npc LoadNPCByUID(screen, int32_t)
	{
		Function* function = getFunction2("LoadNPCByUID");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		
		addOpcode2 (code, new OLoadNPCBySUIDRegister(new VarArgument(EXP1)));
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(REFNPC)));
		RETURN();
		function->giveCode(code);
	}
	
	 //npc LoadLWeaponByUID(screen, int32_t)
	{
		Function* function = getFunction2("LoadLWeaponByUID");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OLoadLWeaponBySUIDRegister(new VarArgument(EXP1)));
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(REFLWPN)));
		RETURN();
		function->giveCode(code);
	}
	
	//ewpn LoadEWeaponByUID(screen, int32_t)
	{
		Function* function = getFunction2("LoadEWeaponByUID");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OLoadEWeaponBySUIDRegister(new VarArgument(EXP1)));
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(REFEWPN)));
		RETURN();
		function->giveCode(code);
	}
	//ewpn CreateEWeapon(screen, int32_t)
	{
		Function* function = getFunction2("CreateEWeapon");
		
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OCreateEWpnRegister(new VarArgument(EXP1)));
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(REFEWPN)));
		RETURN();
		function->giveCode(code);
	}
	//void ClearSprites(screen, int32_t)
	{
		Function* function = getFunction2("ClearSprites");
		
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OClearSpritesRegister(new VarArgument(EXP1)));
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(REFNPC)));
		RETURN();
		function->giveCode(code);
	}
	//void Rectangle(screen, float, float, float, float, float, float, float, float, float, float, bool, float)
	{
		Function* function = getFunction2("Rectangle");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new ORectangleRegister());
		LABELBACK(label);
		POP_ARGS(12, NUL);
		//pop pointer, and ignore it
		POPREF();
		RETURN();
		
		function->giveCode(code);
	}
	//void DrawFrame(screen, float, float, float, float, float, float, float, bool, float)
	{
		Function* function = getFunction2("DrawFrame");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OFrameRegister());
		LABELBACK(label);
		POP_ARGS(9, NUL);
		//pop pointer, and ignore it
		POPREF();
		RETURN();
		
		function->giveCode(code);
	}
	//void Circle(screen, float, float, float, float, float, float, float, float, float, bool, float)
	{
		Function* function = getFunction2("Circle");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OCircleRegister());
		LABELBACK(label);
		POP_ARGS(11, NUL);
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	//void Arc(screen, float, float, float, float, float, float, float, float, float, float, float, bool, bool, float)
	{
		Function* function = getFunction2("Arc");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OArcRegister());
		LABELBACK(label);
		POP_ARGS(14, NUL);
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	//void Ellipse(screen, float, float, float, float, float, bool, float, float, float)
	{
		Function* function = getFunction2("Ellipse");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OEllipseRegister());
		LABELBACK(label);
		POP_ARGS(12, NUL);
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	//void Line(screen, float, float, float, float, float, float, float, float, float, float, float)
	{
		Function* function = getFunction2("Line");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OLineRegister());
		LABELBACK(label);
		POP_ARGS(11, NUL);
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	//void Spline(screen, float, float, float, float, float, float, float, float, float, float, float)
	{
		Function* function = getFunction2("Spline");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OSplineRegister());
		LABELBACK(label);
		POP_ARGS(11, NUL);
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	//void PutPixel(screen, float, float, float, float, float, float, float, float)
	{
		Function* function = getFunction2("PutPixel");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPutPixelRegister());
		LABELBACK(label);
		POP_ARGS(8, NUL);
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	//void PutPixels(screen, float, float, float, float, float)
	{
		Function* function = getFunction2("PutPixels");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPutPixelArrayRegister());
		LABELBACK(label);
		POP_ARGS(5, NUL);
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	//void DrawTiles(screen, float, float)
	{
		Function* function = getFunction2("DrawTiles");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPutTileArrayRegister());
		LABELBACK(label);
		POP_ARGS(2, NUL);
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	//void DrawCombos(screen, float, float)
	{
		Function* function = getFunction2("DrawCombos");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OFastComboArrayRegister());
		LABELBACK(label);
		POP_ARGS(2, NUL);
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	//void Lines(screen, float, float)
	{
		Function* function = getFunction2("Lines");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPutLinesArrayRegister());
		LABELBACK(label);
		POP_ARGS(2, NUL);
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	//void DrawCharacter(screen, float, float, float, float, float, float, float, float, float, float)
	{
		Function* function = getFunction2("DrawCharacter");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new ODrawCharRegister());
		LABELBACK(label);
		POP_ARGS(10, NUL);
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	//void DrawInteger(screen, float, float, float, float, float, float, float, float, float, float, float)
	{
		Function* function = getFunction2("DrawInteger");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new ODrawIntRegister());
		LABELBACK(label);
		POP_ARGS(11, NUL);
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	//void DrawTile(screen, float, float, float, float, float, bool, float, float, float)
	{
		Function* function = getFunction2("DrawTile");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new ODrawTileRegister());
		LABELBACK(label);
		POP_ARGS(15, NUL);
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	//void DrawTileCloaked(screen, ...args)
	{
		Function* function = getFunction2("DrawTileCloaked");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new ODrawTileCloakedRegister());
		LABELBACK(label);
		POP_ARGS(7, NUL);
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	//void DrawCombo(screen, float, float, float, float, float, bool, float, float, float)
	{
		Function* function = getFunction2("DrawCombo");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new ODrawComboRegister());
		LABELBACK(label);
		POP_ARGS(16, NUL);
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	//void DrawComboCloaked(screen, ...args)
	{
		Function* function = getFunction2("DrawComboCloaked");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new ODrawComboCloakedRegister());
		LABELBACK(label);
		POP_ARGS(7, NUL);
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	//void Quad(screen, float, float, float, float, float, float, float, float, float)
	{
		Function* function = getFunction2("Quad");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OQuadRegister());
		LABELBACK(label);
		POP_ARGS(15, NUL);
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	//void Polygon(screen, float, float, float, float, float)
	
	{
		Function* function = getFunction2("Polygon");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPolygonRegister());
		LABELBACK(label);
		POP_ARGS(5, NUL);
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	
	//void Triangle(screen, float, float, float, float, float, float, float, float, float)
	{
	Function* function = getFunction2("Triangle");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OTriangleRegister());
		LABELBACK(label);
		POP_ARGS(13, NUL);
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	
	//void Quad3D(screen, float, float, float, float, float, float, float, float, float)
	{
		Function* function = getFunction2("Quad3D");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OQuad3DRegister());
		LABELBACK(label);
		POP_ARGS(8, NUL);
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	//void Triangle3D(screen, float, float, float, float, float, float, float, float, float)
	{
		Function* function = getFunction2("Triangle3D");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OTriangle3DRegister());
		LABELBACK(label);
		POP_ARGS(8, NUL);
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	
	//void FastTile(screen, float, float, float, float, float)
	{
		Function* function = getFunction2("FastTile");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OFastTileRegister());
		LABELBACK(label);
		POP_ARGS(6, NUL);
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	//void FastCombo(screen, float, float, float, float, float)
	{
		Function* function = getFunction2("FastCombo");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OFastComboRegister());
		LABELBACK(label);
		POP_ARGS(6, NUL);
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	//void DrawString(screen, float, float, float, float, float, float, float, int32_t *string)
	{
		Function* function = getFunction2("DrawString");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new ODrawStringRegister());
		LABELBACK(label);
		POP_ARGS(9, NUL);
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	//void DrawString(screen, float, float, float, float, float, float, float, int32_t *string)
	{
		Function* function = getFunction2("DrawString", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new ODrawString2Register());
		LABELBACK(label);
		POP_ARGS(11, NUL);
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	//void DrawLayer(screen, float, float, float, float, float, float, float, float)
	{
		Function* function = getFunction2("DrawLayer");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new ODrawLayerRegister());
		LABELBACK(label);
		POP_ARGS(8, NUL);
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	//void DrawScreen(screen, float, float, float, float, float, float)
	{
		Function* function = getFunction2("DrawScreen");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new ODrawScreenRegister());
		LABELBACK(label);
		POP_ARGS(6, NUL);
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	//void DrawBitmap(screen, float, float, float, float, float, float, float, float, float, bool)
	{
		Function* function = getFunction2("DrawBitmap");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new ODrawBitmapRegister());
		LABELBACK(label);
		POP_ARGS(12, NUL);
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	
	//void DrawBitmapEx(screen, float, float, float, float, float, float, float, float, float, float, bool)
	{
		Function* function = getFunction2("DrawBitmapEx");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new ODrawBitmapExRegister());
		LABELBACK(label);
		POP_ARGS(16, NUL);
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	
	//void SetRenderTarget(bitmap)
	{
		Function* function = getFunction2("SetRenderTarget");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OSetRenderTargetRegister());
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	//void Message(screen, float)
	{
		Function* function = getFunction2("Message");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OMessageRegister(new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//bool isSolid(screen, int32_t, int32_t)
	{
		Function* function = getFunction2("isSolid");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OIsSolid(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//bool isSolidLayer(screen, int32_t, int32_t, int32_t)
	{
		Function* function = getFunction2("isSolidLayer");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		//pop pointer
		POPREF();
		addOpcode2 (code, new OIsSolidLayer(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void SetSideWarp(screen, float, float, float, float)
	{
		Function* function = getFunction2("SetSideWarp");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OSetSideWarpRegister());
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	//void SetTileWarp(screen, float, float, float, float)
	{
		Function* function = getFunction2("SetTileWarp");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OSetTileWarpRegister());
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	//float LayerScreen(screen, float)
	{
		Function* function = getFunction2("LayerScreen");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OLayerScreenRegister(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//float LayerMap(screen, float)
	{
		Function* function = getFunction2("LayerMap");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OLayerMapRegister(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//void TriggerSecrets(screen)
	{
		Function* function = getFunction2("TriggerSecrets");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		ASSERT_NUL();
		addOpcode2 (code, new OTriggerSecrets());
		LABELBACK(label);
		RETURN();
		function->giveCode(code);
	}
	//void ZapIn(screen)
	{
		Function* function = getFunction2("ZapIn");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer, and ignore it
		ASSERT_NUL();
		addOpcode2 (code, new OZapIn());
		LABELBACK(label);
		RETURN();
		function->giveCode(code);
	}
	  

	//void ZapOut(screen)
	{
		Function* function = getFunction2("ZapOut");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer, and ignore it
		ASSERT_NUL();
		addOpcode2 (code, new OZapOut());
		LABELBACK(label);
		RETURN();
		function->giveCode(code);
	}
	
	 //void OpeningWipe(screen)
	{
		Function* function = getFunction2("OpeningWipe");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer, and ignore it
		ASSERT_NUL();
		addOpcode2 (code, new OOpenWipe());
		LABELBACK(label);
		RETURN();
		function->giveCode(code);
	}

	//void WavyIn(screen)
	{
		Function* function = getFunction2("WavyIn");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer, and ignore it
		ASSERT_NUL();
		addOpcode2 (code, new OWavyIn());
		LABELBACK(label);
		RETURN();
		function->giveCode(code);
	}
			
	//void WavyOut(screen)
	{
		Function* function = getFunction2("WavyOut");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer, and ignore it
		ASSERT_NUL();
		addOpcode2 (code, new OWavyOut());
		LABELBACK(label);
		RETURN();
		function->giveCode(code);
	}
	
	//int32_t GetSideWarpDMap(screen, int32_t)
	{
		Function* function = getFunction2("GetSideWarpDMap");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OGetSideWarpDMap(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t GetSideWarpScreen(screen, int32_t)
	{
		Function* function = getFunction2("GetSideWarpScreen");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OGetSideWarpScreen(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t GetSideWarpType(screen, int32_t)
	{
		Function* function = getFunction2("GetSideWarpType");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OGetSideWarpType(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t GetTileWarpDMap(screen, int32_t)
	{
		Function* function = getFunction2("GetTileWarpDMap");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OGetTileWarpDMap(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t GetTileWarpScreen(screen, int32_t)
	{
		Function* function = getFunction2("GetTileWarpScreen");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OGetTileWarpScreen(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t GetTileWarpType(screen, int32_t)
	{
		Function* function = getFunction2("GetTileWarpType");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OGetTileWarpType(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	
	//void ZapIn(screen)
	{
		Function* function = getFunction2("ZapIn");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer, and ignore it
		ASSERT_NUL();
		addOpcode2 (code, new OZapIn());
		LABELBACK(label);
		RETURN();
		function->giveCode(code);
	}
	
	 //void ClosingWipe(screen)
	{
		Function* function = getFunction2("ClosingWipe");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer, and ignore it
		ASSERT_NUL();
		addOpcode2 (code, new OCloseWipe());
		LABELBACK(label);
		RETURN();
		function->giveCode(code);
	}
	
	 //void OpeningWipe(screen, int32_t)
	{
		Function* function = getFunction2("OpeningWipe", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OOpenWipeShape(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	
	//void ClosingWipe(screen, int32_t)
	{
		Function* function = getFunction2("ClosingWipe", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OCloseWipeShape(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//bool SpawnScreenEnemies(screen)
	{
		Function* function = getFunction2("SpawnScreenEnemies");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer, and ignore it
		ASSERT_NUL();
		addOpcode2 (code, new OScreenDoSpawn());
		LABELBACK(label);
		RETURN();
		function->giveCode(code);
	}
	//bool TriggerCombo(screen,lyr,pos)
	{
		Function* function = getFunction2("TriggerCombo");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OScreenTriggerCombo(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
}

