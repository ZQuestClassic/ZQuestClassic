#include "SymbolDefs.h"

ScreenSymbols ScreenSymbols::singleton = ScreenSymbols();

static AccessorTable ScreenTable[] =
{
//	  name,                           rettype,                  setorget,     var,                              numindex,     funcFlags,                            numParams,   params
	{ "getD[]",                       ZTID_UNTYPED,       GETTER,       SDD,                              8,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setD[]",                       ZTID_VOID,          SETTER,       SDD,                              8,            0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getComboD[]",                  ZTID_FLOAT,         GETTER,       COMBODD,                          176,          0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setComboD[]",                  ZTID_VOID,          SETTER,       COMBODD,                          176,          0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getComboC[]",                  ZTID_FLOAT,         GETTER,       COMBOCD,                          176,          0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setComboC[]",                  ZTID_VOID,          SETTER,       COMBOCD,                          176,          0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getComboF[]",                  ZTID_FLOAT,         GETTER,       COMBOFD,                          176,          0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setComboF[]",                  ZTID_VOID,          SETTER,       COMBOFD,                          176,          0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getComboI[]",                  ZTID_FLOAT,         GETTER,       COMBOID,                          176,          0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setComboI[]",                  ZTID_VOID,          SETTER,       COMBOID,                          176,          0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getComboT[]",                  ZTID_FLOAT,         GETTER,       COMBOTD,                          176,          0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setComboT[]",                  ZTID_VOID,          SETTER,       COMBOTD,                          176,          0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getComboS[]",                  ZTID_FLOAT,         GETTER,       COMBOSD,                          176,          0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setComboS[]",                  ZTID_VOID,          SETTER,       COMBOSD,                          176,          0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getComboE[]",                  ZTID_FLOAT,         GETTER,       COMBOED,                          176,          0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setComboE[]",                  ZTID_VOID,          SETTER,       COMBOED,                          176,          0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDoor[]",                    ZTID_FLOAT,         GETTER,       SCRDOORD,                         4,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDoor[]",                    ZTID_VOID,          SETTER,       SCRDOORD,                         4,            0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getState[]",                   ZTID_BOOL,          GETTER,       SCREENSTATED,                     32,           0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setState[]",                   ZTID_VOID,          SETTER,       SCREENSTATED,                     32,           0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getExState[]",                 ZTID_BOOL,          GETTER,       SCREENEXSTATED,                   32,           0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setExState[]",                 ZTID_VOID,          SETTER,       SCREENEXSTATED,                   32,           0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLit",                       ZTID_BOOL,          GETTER,       LIT,                              1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLit",                       ZTID_VOID,          SETTER,       LIT,                              1,            0,                                    2,           { ZTID_SCREEN, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWavy",                      ZTID_FLOAT,         GETTER,       WAVY,                             1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWavy",                      ZTID_VOID,          SETTER,       WAVY,                             1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getQuake",                     ZTID_FLOAT,         GETTER,       QUAKE,                            1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setQuake",                     ZTID_VOID,          SETTER,       QUAKE,                            1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "NumItems",                     ZTID_FLOAT,         GETTER,       ITEMCOUNT,                        1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "GetRenderTarget",              ZTID_FLOAT,         GETTER,       GETRENDERTARGET,                  1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadItem",                     ZTID_ITEM,          FUNCTION,     0,                                1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "CreateItem",                   ZTID_ITEM,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadFFC",                      ZTID_FFC,           FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "NumNPCs",                      ZTID_FLOAT,         GETTER,       NPCCOUNT,                         1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadNPC",                      ZTID_NPC,           FUNCTION,     0,                                1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "CreateNPC",                    ZTID_NPC,           FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "ClearSprites",                 ZTID_VOID,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Rectangle",                    ZTID_VOID,          FUNCTION,     0,                                1,            0,                                    13,          {  ZTID_SCREEN,         ZTID_FLOAT,         ZTID_FLOAT,         ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_BOOL,      ZTID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                      } },
	{ "Circle",                       ZTID_VOID,          FUNCTION,     0,                                1,            0,                                    12,          {  ZTID_SCREEN,         ZTID_FLOAT,         ZTID_FLOAT,         ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_BOOL,      ZTID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           } },
	{ "Arc",                          ZTID_VOID,          FUNCTION,     0,                                1,            0,                                    15,          {  ZTID_SCREEN,         ZTID_FLOAT,         ZTID_FLOAT,         ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_BOOL,      ZTID_BOOL,      ZTID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           } },
	{ "Ellipse",                      ZTID_VOID,          FUNCTION,     0,                                1,            0,                                    13,          {  ZTID_SCREEN,         ZTID_FLOAT,         ZTID_FLOAT,         ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_BOOL,      ZTID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           } },
	{ "Line",                         ZTID_VOID,          FUNCTION,     0,                                1,            0,                                    12,          {  ZTID_SCREEN,         ZTID_FLOAT,         ZTID_FLOAT,         ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           } },
	{ "Spline",                       ZTID_VOID,          FUNCTION,     0,                                1,            0,                                    12,          {  ZTID_SCREEN,         ZTID_FLOAT,         ZTID_FLOAT,         ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           } },
	{ "PutPixel",                     ZTID_VOID,          FUNCTION,     0,                                1,            0,                                    9,           {  ZTID_SCREEN,         ZTID_FLOAT,         ZTID_FLOAT,         ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "PutPixels",                    ZTID_VOID,          FUNCTION,     0,                                1,            0,                                    6,           {  ZTID_SCREEN,         ZTID_FLOAT,         ZTID_FLOAT,         ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     -1,     -1,     -1,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "DrawTiles",                    ZTID_VOID,          FUNCTION,     0,                                1,            0,                                    3,           {  ZTID_SCREEN,         ZTID_FLOAT,         ZTID_FLOAT,         -1,     -1,     -1,     -1,     -1,     -1,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "DrawCombos",                   ZTID_VOID,          FUNCTION,     0,                                1,            0,                                    3,           {  ZTID_SCREEN,         ZTID_FLOAT,         ZTID_FLOAT,         -1,     -1,     -1,     -1,     -1,     -1,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "Lines",                        ZTID_VOID,          FUNCTION,     0,                                1,            0,                                    3,           {  ZTID_SCREEN,         ZTID_FLOAT,         ZTID_FLOAT,         -1,     -1,     -1,     -1,     -1,     -1,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	
	{ "DrawCharacter",                ZTID_VOID,          FUNCTION,     0,                                1,            0,                                    11,          {  ZTID_SCREEN,         ZTID_FLOAT,         ZTID_FLOAT,         ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "DrawInteger",                  ZTID_VOID,          FUNCTION,     0,                                1,            0,                                    12,          {  ZTID_SCREEN,         ZTID_FLOAT,         ZTID_FLOAT,         ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "DrawTile",                     ZTID_VOID,          FUNCTION,     0,                                1,            0,                                    16,          {  ZTID_SCREEN,         ZTID_FLOAT,         ZTID_FLOAT,         ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,      ZTID_BOOL,    ZTID_FLOAT,                           -1,                           -1,                           -1,                           -1,                           } },
	{ "DrawTileCloaked",              ZTID_VOID,          FUNCTION,     0,                                1,            0,                                    8,           {  ZTID_SCREEN,         ZTID_FLOAT,         ZTID_FLOAT,         ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     -1,     -1,     -1,     -1,     -1,     -1,      -1,    -1,                           -1,                           -1,                           -1,                           -1,                           } },
	{ "DrawCombo",                    ZTID_VOID,          FUNCTION,     0,                                1,            0,                                    17,          {  ZTID_SCREEN,         ZTID_FLOAT,         ZTID_FLOAT,         ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,      ZTID_BOOL,    ZTID_FLOAT,                           -1,                           -1,                           -1,                           } },
	{ "DrawComboCloaked",             ZTID_VOID,          FUNCTION,     0,                                1,            0,                                    8,           {  ZTID_SCREEN,         ZTID_FLOAT,         ZTID_FLOAT,         ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     -1,     -1,     -1,     -1,     -1,     -1,     -1,      -1,    -1,                           -1,                           -1,                           -1,                           } },
	{ "Quad",                         ZTID_VOID,          FUNCTION,     0,                                1,            0,                                    16,          {  ZTID_SCREEN,         ZTID_FLOAT,         ZTID_FLOAT,         ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,  ZTID_FLOAT,                         -1,                           -1,                           -1,                           -1,                           } },
	{ "Triangle",                     ZTID_VOID,          FUNCTION,     0,                                1,            0,                                    14,          {  ZTID_SCREEN,         ZTID_FLOAT,         ZTID_FLOAT,         ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,                      -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           } },
	{ "Quad3D",                       ZTID_VOID,          FUNCTION,     0,                                1,            0,                                    9,           {  ZTID_SCREEN,         ZTID_FLOAT,         ZTID_FLOAT,         ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "Triangle3D",                   ZTID_VOID,          FUNCTION,     0,                                1,            0,                                    9,           {  ZTID_SCREEN,         ZTID_FLOAT,         ZTID_FLOAT,         ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "FastTile",                     ZTID_VOID,          FUNCTION,     0,                                1,            0,                                    7,           {  ZTID_SCREEN,         ZTID_FLOAT,         ZTID_FLOAT,         ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     -1,                           -1,                          -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "FastCombo",                    ZTID_VOID,          FUNCTION,     0,                                1,            0,                                    7,           {  ZTID_SCREEN,         ZTID_FLOAT,         ZTID_FLOAT,         ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     -1,                           -1,                          -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "DrawString",                   ZTID_VOID,          FUNCTION,     0,                                1,            0,                                    10,          {  ZTID_SCREEN,         ZTID_FLOAT,         ZTID_FLOAT,         ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "DrawString",                   ZTID_VOID,          FUNCTION,     0,                                1,            0,                                    12,          {  ZTID_SCREEN,         ZTID_FLOAT,         ZTID_FLOAT,         ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "DrawLayer",                    ZTID_VOID,                 FUNCTION,     0,                                1,            0,                                    9,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 } },
	{ "DrawScreen",                   ZTID_VOID,                 FUNCTION,     0,                                1,            0,                                    7,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 } },
	 { "DrawBitmap",                   ZTID_VOID,          FUNCTION,     0,                                1,            0,                                    13,          {  ZTID_SCREEN,         ZTID_FLOAT,         ZTID_FLOAT,         ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,       ZTID_FLOAT,   ZTID_FLOAT,        ZTID_FLOAT,    ZTID_BOOL,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	 { "DrawBitmapEx",                 ZTID_VOID,          FUNCTION,     0,                                1,            0,                                    17,          { ZTID_SCREEN, ZTID_FLOAT,ZTID_FLOAT,ZTID_FLOAT,ZTID_FLOAT,ZTID_FLOAT,ZTID_FLOAT,ZTID_FLOAT,ZTID_FLOAT,ZTID_FLOAT,ZTID_FLOAT,ZTID_FLOAT,ZTID_FLOAT,ZTID_FLOAT,ZTID_FLOAT,ZTID_FLOAT,ZTID_BOOL, -1,                           -1,                           -1,                              } },
	 { "SetRenderTarget",              ZTID_VOID,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Message",                      ZTID_VOID,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShowingMessage",            ZTID_FLOAT,         GETTER,       SHOWNMSG,                         1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShowingMessage",            ZTID_VOID,          SETTER,       SHOWNMSG,                         1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "NumLWeapons",                  ZTID_FLOAT,         GETTER,       LWPNCOUNT,                        1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadLWeapon",                  ZTID_LWPN,          FUNCTION,     0,                                1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "CreateLWeapon",                ZTID_LWPN,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "NumEWeapons",                  ZTID_FLOAT,         GETTER,       EWPNCOUNT,                        1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadEWeapon",                  ZTID_EWPN,          FUNCTION,     0,                                1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "CreateEWeapon",                ZTID_EWPN,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "isSolid",                      ZTID_BOOL,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "isSolidLayer",                 ZTID_BOOL,          FUNCTION,     0,                                1,            0,                                    4,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "SetSideWarp",                  ZTID_VOID,          FUNCTION,     0,                                1,            0,                                    5,           {  ZTID_SCREEN,         ZTID_FLOAT,         ZTID_FLOAT,         ZTID_FLOAT,     ZTID_FLOAT,    -1,     -1,     -1,                           -1,                          -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	 { "SetTileWarp",                  ZTID_VOID,          FUNCTION,     0,                                1,            0,                                    5,           {  ZTID_SCREEN,         ZTID_FLOAT,         ZTID_FLOAT,         ZTID_FLOAT,     ZTID_FLOAT,    -1,     -1,     -1,                           -1,                          -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	 { "LayerScreen",                  ZTID_FLOAT,         FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "LayerMap",                     ZTID_FLOAT,         FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlags[]",                   ZTID_FLOAT,         GETTER,       SCREENFLAGSD,                     10,           0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlags[]",                   ZTID_VOID,          SETTER,       SCREENFLAGSD,                     10,           0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	This is read-only, but it was not previously blocked! -Z
	{ "getEFlags[]",                  ZTID_FLOAT,         GETTER,       SCREENEFLAGSD,                    3,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	This is read-only, but it was not previously blocked! -Z
	{ "setEFlags[]",                  ZTID_VOID,          SETTER,       SCREENEFLAGSD,                    3,            0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "TriggerSecrets",               ZTID_VOID,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRoomType",                  ZTID_FLOAT,         GETTER,       ROOMTYPE,                         1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRoomType",                  ZTID_VOID,          SETTER,       ROOMTYPE,                         1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRoomData",                  ZTID_FLOAT,         GETTER,       ROOMDATA,                         1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRoomData",                  ZTID_VOID,          SETTER,       ROOMDATA,                         1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMovingBlockX",              ZTID_FLOAT,         GETTER,       PUSHBLOCKX,                       1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMovingBlockX",              ZTID_VOID,          SETTER,       PUSHBLOCKX,                       1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMovingBlockY",              ZTID_FLOAT,         GETTER,       PUSHBLOCKY,                       1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMovingBlockY",              ZTID_VOID,          SETTER,       PUSHBLOCKY,                       1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMovingBlockLayer",          ZTID_FLOAT,         GETTER,       PUSHBLOCKLAYER,                   1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMovingBlockLayer",          ZTID_VOID,          SETTER,       PUSHBLOCKLAYER,                   1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMovingBlockCombo",          ZTID_FLOAT,         GETTER,       PUSHBLOCKCOMBO,                   1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMovingBlockCombo",          ZTID_VOID,          SETTER,       PUSHBLOCKCOMBO,                   1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMovingBlockCSet",           ZTID_FLOAT,         GETTER,       PUSHBLOCKCSET,                    1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMovingBlockCSet",           ZTID_VOID,          SETTER,       PUSHBLOCKCSET,                    1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getUnderCombo",                ZTID_FLOAT,         GETTER,       UNDERCOMBO,                       1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setUnderCombo",                ZTID_VOID,          SETTER,       UNDERCOMBO,                       1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getUnderCSet",                 ZTID_FLOAT,         GETTER,       UNDERCSET,                        1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setUnderCSet",                 ZTID_VOID,          SETTER,       UNDERCSET,                        1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "GetSideWarpDMap",              ZTID_FLOAT,         FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "GetSideWarpScreen",            ZTID_FLOAT,         FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "GetSideWarpType",              ZTID_FLOAT,         FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "GetTileWarpDMap",              ZTID_FLOAT,         FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "GetTileWarpScreen",            ZTID_FLOAT,         FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "GetTileWarpType",              ZTID_FLOAT,         FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "ZapIn",                        ZTID_VOID,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "ZapOut",                       ZTID_VOID,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "WavyIn",                       ZTID_VOID,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "WavyOut",                      ZTID_VOID,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "OpeningWipe",                  ZTID_VOID,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "ClosingWipe",                  ZTID_VOID,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "OpeningWipe",                  ZTID_VOID,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "ClosingWipe",                  ZTID_VOID,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "CreateLWeaponDx",              ZTID_LWPN,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Polygon",                      ZTID_VOID,          FUNCTION,     0,                                1,            0,                                    6,           {  ZTID_SCREEN,         ZTID_FLOAT,         ZTID_FLOAT,         ZTID_FLOAT,     ZTID_FLOAT,     ZTID_FLOAT,     -1,     -1,     -1,     -1,         -1,     -1,     -1,     -1,     -1,  -1,                         -1,                           -1,                           -1,                           -1,                           } },
	{ "TriggerSecret",                ZTID_VOID,          FUNCTION,     0,                                1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
//	mapdata m-> class variables copied to Screen->
	{ "getValid",                     ZTID_FLOAT,         GETTER,       SCREENDATAVALID,                  1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setValid",                     ZTID_VOID,          SETTER,       SCREENDATAVALID,                  1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getGuy",                       ZTID_FLOAT,         GETTER,       SCREENDATAGUY,                    1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setGuy",                       ZTID_VOID,          SETTER,       SCREENDATAGUY,                    1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getString",                    ZTID_FLOAT,         GETTER,       SCREENDATASTRING,                 1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setString",                    ZTID_VOID,          SETTER,       SCREENDATASTRING,                 1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
/*	
	{ "getRoomType",                  ZTID_FLOAT,         GETTER,       SCREENDATAROOM,                   1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRoomType",                  ZTID_VOID,          SETTER,       SCREENDATAROOM,                   1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
*/	
	
	{ "getItem",                      ZTID_FLOAT,         GETTER,       SCREENDATAITEM,                   1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItem",                      ZTID_VOID,          SETTER,       SCREENDATAITEM,                   1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getHasItem",                   ZTID_FLOAT,         GETTER,       SCREENDATAHASITEM,                1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHasItem",                   ZTID_VOID,          SETTER,       SCREENDATAHASITEM,                1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getTileWarpType[]",            ZTID_FLOAT,         GETTER,       SCREENDATATILEWARPTYPE,           4,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWarpType[]",            ZTID_VOID,          SETTER,       SCREENDATATILEWARPTYPE,           4,            0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getTileWarpOverlay[]",         ZTID_BOOL,          GETTER,       SCREENDATATILEWARPOVFLAGS,        4,            0,                                    1,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWarpOverlay[]",         ZTID_VOID,          SETTER,       SCREENDATATILEWARPOVFLAGS,        4,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getDoorComboSet",              ZTID_FLOAT,         GETTER,       SCREENDATADOORCOMBOSET,           1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDoorComboSet",              ZTID_VOID,          SETTER,       SCREENDATADOORCOMBOSET,           1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWarpReturnX[]",             ZTID_FLOAT,         GETTER,       SCREENDATAWARPRETX,               4,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWarpReturnX[]",             ZTID_VOID,          SETTER,       SCREENDATAWARPRETX,               4,            0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWarpReturnY[]",             ZTID_FLOAT,         GETTER,       SCREENDATAWARPRETY,               4,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWarpReturnY[]",             ZTID_VOID,          SETTER,       SCREENDATAWARPRETY,               4,            0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getWarpReturnC",               ZTID_FLOAT,         GETTER,       SCREENDATAWARPRETURNC,            1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setWarpReturnC",               ZTID_VOID,          SETTER,       SCREENDATAWARPRETURNC,            1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getStairsX",                   ZTID_FLOAT,         GETTER,       SCREENDATASTAIRX,                 1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStairsX",                   ZTID_VOID,          SETTER,       SCREENDATASTAIRX,                 1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getStairsY",                   ZTID_FLOAT,         GETTER,       SCREENDATASTAIRY,                 1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStairsY",                   ZTID_VOID,          SETTER,       SCREENDATASTAIRY,                 1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getItemX",                     ZTID_FLOAT,         GETTER,       SCREENDATAITEMX,                  1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItemX",                     ZTID_VOID,          SETTER,       SCREENDATAITEMX,                  1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getItemY",                     ZTID_FLOAT,         GETTER,       SCREENDATAITEMY,                  1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItemY",                     ZTID_VOID,          SETTER,       SCREENDATAITEMY,                  1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getCSet",                      ZTID_FLOAT,         GETTER,       SCREENDATACOLOUR,                 1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setCSet",                      ZTID_VOID,          SETTER,       SCREENDATACOLOUR,                 1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPalette",                      ZTID_FLOAT,         GETTER,       SCREENDATACOLOUR,                 1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPalette",                      ZTID_VOID,          SETTER,       SCREENDATACOLOUR,                 1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getEnemyFlags",                ZTID_FLOAT,         GETTER,       SCREENDATAENEMYFLAGS,             1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setEnemyFlags",                ZTID_VOID,          SETTER,       SCREENDATAENEMYFLAGS,             1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getTileWarpDMap[]",            ZTID_FLOAT,         GETTER,       SCREENDATATILEWARPDMAP,           4,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWarpDMap[]",            ZTID_VOID,          SETTER,       SCREENDATATILEWARPDMAP,           4,            0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getTileWarpScreen[]",          ZTID_FLOAT,         GETTER,       SCREENDATATILEWARPSCREEN,         4,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWarpScreen[]",          ZTID_VOID,          SETTER,       SCREENDATATILEWARPSCREEN,         4,            0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getExitDir",                   ZTID_FLOAT,         GETTER,       SCREENDATAEXITDIR,                1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setExitDir",                   ZTID_VOID,          SETTER,       SCREENDATAEXITDIR,                1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getEnemy[]",                   ZTID_FLOAT,         GETTER,       SCREENDATAENEMY,                  10,           0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setEnemy[]",                   ZTID_VOID,          SETTER,       SCREENDATAENEMY,                  10,           0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getPattern",                   ZTID_FLOAT,         GETTER,       SCREENDATAPATTERN,                1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPattern",                   ZTID_VOID,          SETTER,       SCREENDATAPATTERN,                1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSideWarpType[]",            ZTID_FLOAT,         GETTER,       SCREENDATASIDEWARPTYPE,           4,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSideWarpType[]",            ZTID_VOID,          SETTER,       SCREENDATASIDEWARPTYPE,           4,            0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSideWarpOverlay[]",        ZTID_BOOL,          GETTER,       SCREENDATASIDEWARPOVFLAGS,        4,            0,                                    1,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSideWarpOverlay[]",        ZTID_VOID,          SETTER,       SCREENDATASIDEWARPOVFLAGS,        4,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getWarpArrivalX",              ZTID_FLOAT,         GETTER,       SCREENDATAWARPARRIVALX,           1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setWarpArrivalX",              ZTID_VOID,          SETTER,       SCREENDATAWARPARRIVALX,           1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getWarpArrivalY",              ZTID_FLOAT,         GETTER,       SCREENDATAWARPARRIVALY,           1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setWarpArrivalY",              ZTID_VOID,          SETTER,       SCREENDATAWARPARRIVALY,           1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getMazePath[]",                ZTID_FLOAT,         GETTER,       SCREENDATAPATH,                   4,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMazePath[]",                ZTID_VOID,          SETTER,       SCREENDATAPATH,                   4,            0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSideWarpScreen[]",          ZTID_FLOAT,         GETTER,       SCREENDATASIDEWARPSC,             4,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSideWarpScreen[]",          ZTID_VOID,          SETTER,       SCREENDATASIDEWARPSC,             4,            0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSideWarpID[]",              ZTID_FLOAT,         GETTER,       SCREENSIDEWARPID,             4,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSideWarpID[]",              ZTID_VOID,          SETTER,       SCREENSIDEWARPID,             4,            0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getTileWarpReturnSquare[]",          ZTID_FLOAT,         GETTER,       SCREENDATATWARPRETSQR,             4,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWarpReturnSquare[]",          ZTID_VOID,          SETTER,       SCREENDATATWARPRETSQR,             4,            0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSideWarpReturnSquare[]",          ZTID_FLOAT,         GETTER,       SCREENDATASWARPRETSQR,             4,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSideWarpReturnSquare[]",          ZTID_VOID,          SETTER,       SCREENDATASWARPRETSQR,             4,            0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSideWarpDMap[]",            ZTID_FLOAT,         GETTER,       SCREENDATASIDEWARPDMAP,           4,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSideWarpDMap[]",            ZTID_VOID,          SETTER,       SCREENDATASIDEWARPDMAP,           4,            0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getSideWarpIndex",             ZTID_FLOAT,         GETTER,       SCREENDATASIDEWARPINDEX,          1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setSideWarpIndex",             ZTID_VOID,          SETTER,       SCREENDATASIDEWARPINDEX,          1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
/*	
	{ "getUnderCombo",                ZTID_FLOAT,         GETTER,       SCREENDATAUNDERCOMBO,             1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setUnderCombo",                ZTID_VOID,          SETTER,       SCREENDATAUNDERCOMBO,             1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
*/	
	
/*	
	{ "getUnderCSet",                 ZTID_FLOAT,         GETTER,       SCREENDATAUNDERCSET,              1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setUnderCSet",                 ZTID_VOID,          SETTER,       SCREENDATAUNDERCSET,              1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
*/	
	
	{ "getCatchall",                  ZTID_FLOAT,         GETTER,       SCREENDATACATCHALL,               1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCatchall",                  ZTID_VOID,          SETTER,       SCREENDATACATCHALL,               1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getCSensitive",                ZTID_FLOAT,         GETTER,       SCREENDATACSENSITIVE,             1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCSensitive",                ZTID_VOID,          SETTER,       SCREENDATACSENSITIVE,             1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getNoReset",                   ZTID_FLOAT,         GETTER,       SCREENDATANORESET,                1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNoReset",                   ZTID_VOID,          SETTER,       SCREENDATANORESET,                1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getNoCarry",                   ZTID_FLOAT,         GETTER,       SCREENDATANOCARRY,                1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNoCarry",                   ZTID_VOID,          SETTER,       SCREENDATANOCARRY,                1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getLayerMap[]",                ZTID_FLOAT,         GETTER,       SCREENDATALAYERMAP,               7,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLayerMap[]",                ZTID_VOID,          SETTER,       SCREENDATALAYERMAP,               7,            0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getLayerScreen[]",             ZTID_FLOAT,         GETTER,       SCREENDATALAYERSCREEN,            7,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLayerScreen[]",             ZTID_VOID,          SETTER,       SCREENDATALAYERSCREEN,            7,            0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getLayerOpacity[]",            ZTID_FLOAT,         GETTER,       SCREENDATALAYEROPACITY,           7,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLayerOpacity[]",            ZTID_VOID,          SETTER,       SCREENDATALAYEROPACITY,           7,            0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getLayerInvisible[]",          ZTID_BOOL,          GETTER,       SCREENDATALAYERINVIS,             7,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLayerInvisible[]",          ZTID_VOID,          SETTER,       SCREENDATALAYERINVIS,             7,            0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getScriptDraws[]",             ZTID_BOOL,          GETTER,       SCREENDATASCRIPTDRAWS,            8,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScriptDraws[]",             ZTID_VOID,          SETTER,       SCREENDATASCRIPTDRAWS,            8,            0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getTimedWarpTimer",            ZTID_FLOAT,         GETTER,       SCREENDATATIMEDWARPTICS,          1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTimedWarpTimer",            ZTID_VOID,          SETTER,       SCREENDATATIMEDWARPTICS,          1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getNextMap",                   ZTID_FLOAT,         GETTER,       SCREENDATANEXTMAP,                1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNextMap",                   ZTID_VOID,          SETTER,       SCREENDATANEXTMAP,                1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getNextScreen",                ZTID_FLOAT,         GETTER,       SCREENDATANEXTSCREEN,             1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNextScreen",                ZTID_VOID,          SETTER,       SCREENDATANEXTSCREEN,             1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	
	{ "getCarryoverMap",              ZTID_FLOAT,         GETTER,       SCREENDATANEXTMAP,                1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCarryoverMap",               ZTID_VOID,          SETTER,       SCREENDATANEXTMAP,                1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getCarryoverScreen",           ZTID_FLOAT,         GETTER,       SCREENDATANEXTSCREEN,             1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCarryoverScreen",           ZTID_VOID,          SETTER,       SCREENDATANEXTSCREEN,             1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSecretCombo[]",             ZTID_FLOAT,         GETTER,       SCREENDATASECRETCOMBO,            128,          0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSecretCombo[]",             ZTID_VOID,          SETTER,       SCREENDATASECRETCOMBO,            128,          0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSecretCSet[]",              ZTID_FLOAT,         GETTER,       SCREENDATASECRETCSET,             128,          0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSecretCSet[]",              ZTID_VOID,          SETTER,       SCREENDATASECRETCSET,             128,          0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSecretFlags[]",             ZTID_FLOAT,         GETTER,       SCREENDATASECRETFLAG,             128,          0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSecretFlags[]",             ZTID_VOID,          SETTER,       SCREENDATASECRETFLAG,             128,          0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getViewX",                     ZTID_FLOAT,         GETTER,       SCREENDATAVIEWX,                  1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setViewX",                     ZTID_VOID,          SETTER,       SCREENDATAVIEWX,                  1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getViewY",                     ZTID_FLOAT,         GETTER,       SCREENDATAVIEWY,                  1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setViewY",                     ZTID_VOID,          SETTER,       SCREENDATAVIEWY,                  1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getWidth",                     ZTID_FLOAT,         GETTER,       SCREENDATASCREENWIDTH,            1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setWidth",                     ZTID_VOID,          SETTER,       SCREENDATASCREENWIDTH,            1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getHeight",                    ZTID_FLOAT,         GETTER,       SCREENDATASCREENHEIGHT,           1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setHeight",                    ZTID_VOID,          SETTER,       SCREENDATASCREENHEIGHT,           1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getEntryX",                    ZTID_FLOAT,         GETTER,       SCREENDATAENTRYX,                 1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setEntryX",                    ZTID_VOID,          SETTER,       SCREENDATAENTRYX,                 1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getEntryY",                    ZTID_FLOAT,         GETTER,       SCREENDATAENTRYY,                 1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setEntryY",                    ZTID_VOID,          SETTER,       SCREENDATAENTRYY,                 1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getNumFFCs[]",                 ZTID_BOOL,          GETTER,       SCREENDATANUMFF,                 33,           0,                                    1,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	//read-only, for now?
	 { "setNumFFCs[]",                 ZTID_VOID,          SETTER,       SCREENDATANUMFF,                 33,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getScriptEntry",               ZTID_FLOAT,         GETTER,       SCREENDATASCRIPTENTRY,            1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setScriptEntry",               ZTID_VOID,          SETTER,       SCREENDATASCRIPTENTRY,            1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getScriptOccupancy",           ZTID_FLOAT,         GETTER,       SCREENDATASCRIPTOCCUPANCY,        1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setScriptOccupancy",           ZTID_VOID,          SETTER,       SCREENDATASCRIPTOCCUPANCY,        1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getExitScript",                ZTID_FLOAT,         GETTER,       SCREENDATASCRIPTEXIT,             1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setExitScript",                ZTID_VOID,          SETTER,       SCREENDATASCRIPTEXIT,             1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getOceanSFX",                  ZTID_FLOAT,         GETTER,       SCREENDATAOCEANSFX,               1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setOceanSFX",                  ZTID_VOID,          SETTER,       SCREENDATAOCEANSFX,               1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAmbientSFX",                ZTID_FLOAT,         GETTER,       SCREENDATAOCEANSFX,               1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAmbientSFX",                ZTID_VOID,          SETTER,       SCREENDATAOCEANSFX,               1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getBossSFX",                   ZTID_FLOAT,         GETTER,       SCREENDATABOSSSFX,                1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setBossSFX",                   ZTID_VOID,          SETTER,       SCREENDATABOSSSFX,                1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSecretSFX",                 ZTID_FLOAT,         GETTER,       SCREENDATASECRETSFX,              1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSecretSFX",                 ZTID_VOID,          SETTER,       SCREENDATASECRETSFX,              1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getItemSFX",                   ZTID_FLOAT,         GETTER,       SCREENDATAHOLDUPSFX,              1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItemSFX",                   ZTID_VOID,          SETTER,       SCREENDATAHOLDUPSFX,              1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getMIDI",                      ZTID_FLOAT,         GETTER,       SCREENDATASCREENMIDI,             1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMIDI",                      ZTID_VOID,          SETTER,       SCREENDATASCREENMIDI,             1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getLensLayer",                 ZTID_FLOAT,         GETTER,       SCREENDATALENSLAYER,              1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLensLayer",                 ZTID_VOID,          SETTER,       SCREENDATALENSLAYER,              1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getScreenFlags[]",             ZTID_FLOAT,         GETTER,       SCREENDATAFLAGS,                  10,           0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setScreenFlags[]",             ZTID_VOID,          SETTER,       SCREENDATAFLAGS,                  10,           0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getScript",	                  ZTID_FLOAT,         GETTER,       SCREENSCRIPT, 	             1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScript",                    ZTID_VOID,          SETTER,       SCREENSCRIPT, 	             1,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getInitD[]",            	      ZTID_UNTYPED,       GETTER,       SCREENINITD,                   8,            0,                                    2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInitD[]",            	      ZTID_VOID,          SETTER,       SCREENINITD,                   8,            0,                                    3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "LoadNPCByUID",                 ZTID_NPC,           FUNCTION,     0,                             1,            FUNCFLAG_INLINE,                      2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadLWeaponByUID",             ZTID_LWPN,          FUNCTION,     0,                             1,            FUNCFLAG_INLINE,                      2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadEWeaponByUID",             ZTID_EWPN,          FUNCTION,     0,                             1,            FUNCFLAG_INLINE,                      2,           { ZTID_SCREEN, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "DrawFrame",                    ZTID_VOID,          FUNCTION,     0,                             1,            FUNCFLAG_INLINE,                     10,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "SecretsTriggered",             ZTID_BOOL,          GETTER,       SCREENSECRETSTRIGGERED,        1,            0,                                    1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "SpawnScreenEnemies",           ZTID_BOOL,          FUNCTION,     0,                             1,            FUNCFLAG_INLINE,                      1,           { ZTID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "TriggerCombo",                 ZTID_BOOL,          FUNCTION,     0,                             1,            FUNCFLAG_INLINE,                      3,           { ZTID_SCREEN, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "",                             -1,                       -1,           -1,                               -1,           0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

ScreenSymbols::ScreenSymbols()
{
	table = ScreenTable;
	refVar = NUL;
}

void ScreenSymbols::generateCode()
{
	//item LoadItem(screen, int32_t)
	{
		Function* function = getFunction("LoadItem", 2);
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
		Function* function = getFunction("CreateItem", 2);
		
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
		Function* function = getFunction("LoadFFC", 2);
		
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
		Function* function = getFunction("LoadNPC", 2);
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
		Function* function = getFunction("CreateNPC", 2);
		
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
		Function* function = getFunction("LoadLWeapon", 2);
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
		Function* function = getFunction("CreateLWeapon", 2);
		
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
		Function* function = getFunction("CreateLWeaponDx", 3);
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
		Function* function = getFunction("LoadEWeapon", 2);
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
		Function* function = getFunction("LoadNPCByUID", 2);
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
		Function* function = getFunction("LoadLWeaponByUID", 2);
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
		Function* function = getFunction("LoadEWeaponByUID", 2);
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
		Function* function = getFunction("CreateEWeapon", 2);
		
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
		Function* function = getFunction("ClearSprites", 2);
		
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
		Function* function = getFunction("Rectangle", 13);
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
		Function* function = getFunction("DrawFrame", 10);
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
		Function* function = getFunction("Circle", 12);
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
		Function* function = getFunction("Arc", 15);
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
		Function* function = getFunction("Ellipse", 13);
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
		Function* function = getFunction("Line", 12);
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
		Function* function = getFunction("Spline", 12);
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
		Function* function = getFunction("PutPixel", 9);
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
		Function* function = getFunction("PutPixels", 6);
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
		Function* function = getFunction("DrawTiles", 3);
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
		Function* function = getFunction("DrawCombos", 3);
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
		Function* function = getFunction("Lines", 3);
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
		Function* function = getFunction("DrawCharacter", 11);
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
		Function* function = getFunction("DrawInteger", 12);
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
		Function* function = getFunction("DrawTile", 16);
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
		Function* function = getFunction("DrawTileCloaked", 8);
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
		Function* function = getFunction("DrawCombo", 17);
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
		Function* function = getFunction("DrawComboCloaked", 8);
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
		Function* function = getFunction("Quad", 16);
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
		Function* function = getFunction("Polygon", 6);
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
	Function* function = getFunction("Triangle", 14);
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
		Function* function = getFunction("Quad3D", 9);
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
		Function* function = getFunction("Triangle3D", 9);
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
		Function* function = getFunction("FastTile", 7);
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
		Function* function = getFunction("FastCombo", 7);
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
		Function* function = getFunction("DrawString", 10);
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
		Function* function = getFunction("DrawString", 12);
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
		Function* function = getFunction("DrawLayer", 9);
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
		Function* function = getFunction("DrawScreen", 7);
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
		Function* function = getFunction("DrawBitmap", 13);
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
		Function* function = getFunction("DrawBitmapEx", 17);
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
		Function* function = getFunction("SetRenderTarget", 2);
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
		Function* function = getFunction("Message", 2);
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
		Function* function = getFunction("isSolid", 3);
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
		Function* function = getFunction("isSolidLayer", 4);
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
		Function* function = getFunction("SetSideWarp", 5);
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
		Function* function = getFunction("SetTileWarp", 5);
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
		Function* function = getFunction("LayerScreen", 2);
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
		Function* function = getFunction("LayerMap", 2);
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
		Function* function = getFunction("TriggerSecrets", 1);
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
		Function* function = getFunction("ZapIn", 1);
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
		Function* function = getFunction("ZapOut", 1);
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
		Function* function = getFunction("OpeningWipe", 1);
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
		Function* function = getFunction("WavyIn", 1);
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
		Function* function = getFunction("WavyOut", 1);
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
		Function* function = getFunction("GetSideWarpDMap", 2);
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
		Function* function = getFunction("GetSideWarpScreen", 2);
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
		Function* function = getFunction("GetSideWarpType", 2);
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
		Function* function = getFunction("GetTileWarpDMap", 2);
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
		Function* function = getFunction("GetTileWarpScreen", 2);
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
		Function* function = getFunction("GetTileWarpType", 2);
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
		Function* function = getFunction("ZapIn", 1);
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
		Function* function = getFunction("ClosingWipe", 1);
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
		Function* function = getFunction("OpeningWipe", 2);
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
		Function* function = getFunction("ClosingWipe", 2);
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
		Function* function = getFunction("SpawnScreenEnemies", 1);
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
		Function* function = getFunction("TriggerCombo", 3);
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

