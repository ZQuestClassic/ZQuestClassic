#include "SymbolDefs.h"

ScreenSymbols ScreenSymbols::singleton = ScreenSymbols();

static AccessorTable ScreenTable[] =
{
//	  name,                           rettype,                  setorget,     var,                              numindex,     funcFlags,                            numParams,   params
	{ "getD[]",                       ZVARTYPEID_UNTYPED,       GETTER,       SDD,                              8,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setD[]",                       ZVARTYPEID_VOID,          SETTER,       SDD,                              8,            0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getComboD[]",                  ZVARTYPEID_FLOAT,         GETTER,       COMBODD,                          176,          0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setComboD[]",                  ZVARTYPEID_VOID,          SETTER,       COMBODD,                          176,          0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getComboC[]",                  ZVARTYPEID_FLOAT,         GETTER,       COMBOCD,                          176,          0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setComboC[]",                  ZVARTYPEID_VOID,          SETTER,       COMBOCD,                          176,          0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getComboF[]",                  ZVARTYPEID_FLOAT,         GETTER,       COMBOFD,                          176,          0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setComboF[]",                  ZVARTYPEID_VOID,          SETTER,       COMBOFD,                          176,          0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getComboI[]",                  ZVARTYPEID_FLOAT,         GETTER,       COMBOID,                          176,          0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setComboI[]",                  ZVARTYPEID_VOID,          SETTER,       COMBOID,                          176,          0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getComboT[]",                  ZVARTYPEID_FLOAT,         GETTER,       COMBOTD,                          176,          0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setComboT[]",                  ZVARTYPEID_VOID,          SETTER,       COMBOTD,                          176,          0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getComboS[]",                  ZVARTYPEID_FLOAT,         GETTER,       COMBOSD,                          176,          0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setComboS[]",                  ZVARTYPEID_VOID,          SETTER,       COMBOSD,                          176,          0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getComboE[]",                  ZVARTYPEID_FLOAT,         GETTER,       COMBOED,                          176,          0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setComboE[]",                  ZVARTYPEID_VOID,          SETTER,       COMBOED,                          176,          0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getDoor[]",                    ZVARTYPEID_FLOAT,         GETTER,       SCRDOORD,                         4,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDoor[]",                    ZVARTYPEID_VOID,          SETTER,       SCRDOORD,                         4,            0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getState[]",                   ZVARTYPEID_BOOL,          GETTER,       SCREENSTATED,                     32,           0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setState[]",                   ZVARTYPEID_VOID,          SETTER,       SCREENSTATED,                     32,           0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getExState[]",                 ZVARTYPEID_BOOL,          GETTER,       SCREENEXSTATED,                   32,           0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setExState[]",                 ZVARTYPEID_VOID,          SETTER,       SCREENEXSTATED,                   32,           0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLit",                       ZVARTYPEID_BOOL,          GETTER,       LIT,                              1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLit",                       ZVARTYPEID_VOID,          SETTER,       LIT,                              1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWavy",                      ZVARTYPEID_FLOAT,         GETTER,       WAVY,                             1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWavy",                      ZVARTYPEID_VOID,          SETTER,       WAVY,                             1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getQuake",                     ZVARTYPEID_FLOAT,         GETTER,       QUAKE,                            1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setQuake",                     ZVARTYPEID_VOID,          SETTER,       QUAKE,                            1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "NumItems",                     ZVARTYPEID_FLOAT,         GETTER,       ITEMCOUNT,                        1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "GetRenderTarget",              ZVARTYPEID_FLOAT,         GETTER,       GETRENDERTARGET,                  1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadItem",                     ZVARTYPEID_ITEM,          FUNCTION,     0,                                1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "CreateItem",                   ZVARTYPEID_ITEM,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadFFC",                      ZVARTYPEID_FFC,           FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "NumNPCs",                      ZVARTYPEID_FLOAT,         GETTER,       NPCCOUNT,                         1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadNPC",                      ZVARTYPEID_NPC,           FUNCTION,     0,                                1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "CreateNPC",                    ZVARTYPEID_NPC,           FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "ClearSprites",                 ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Rectangle",                    ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            0,                                    13,          {  ZVARTYPEID_SCREEN,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_BOOL,      ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                      } },
	{ "Circle",                       ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            0,                                    12,          {  ZVARTYPEID_SCREEN,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_BOOL,      ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           } },
	{ "Arc",                          ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            0,                                    15,          {  ZVARTYPEID_SCREEN,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_BOOL,      ZVARTYPEID_BOOL,      ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           } },
	{ "Ellipse",                      ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            0,                                    13,          {  ZVARTYPEID_SCREEN,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_BOOL,      ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           } },
	{ "Line",                         ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            0,                                    12,          {  ZVARTYPEID_SCREEN,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           } },
	{ "Spline",                       ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            0,                                    12,          {  ZVARTYPEID_SCREEN,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           } },
	{ "PutPixel",                     ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            0,                                    9,           {  ZVARTYPEID_SCREEN,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "PutPixels",                    ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            0,                                    6,           {  ZVARTYPEID_SCREEN,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     -1,     -1,     -1,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "DrawTiles",                    ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            0,                                    3,           {  ZVARTYPEID_SCREEN,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         -1,     -1,     -1,     -1,     -1,     -1,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "DrawCombos",                   ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            0,                                    3,           {  ZVARTYPEID_SCREEN,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         -1,     -1,     -1,     -1,     -1,     -1,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "Lines",                        ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            0,                                    3,           {  ZVARTYPEID_SCREEN,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         -1,     -1,     -1,     -1,     -1,     -1,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	
	{ "DrawCharacter",                ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            0,                                    11,          {  ZVARTYPEID_SCREEN,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "DrawInteger",                  ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            0,                                    12,          {  ZVARTYPEID_SCREEN,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "DrawTile",                     ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            0,                                    16,          {  ZVARTYPEID_SCREEN,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,      ZVARTYPEID_BOOL,    ZVARTYPEID_FLOAT,                           -1,                           -1,                           -1,                           -1,                           } },
	{ "DrawTileCloaked",              ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            0,                                    8,           {  ZVARTYPEID_SCREEN,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     -1,     -1,     -1,     -1,     -1,     -1,      -1,    -1,                           -1,                           -1,                           -1,                           -1,                           } },
	{ "DrawCombo",                    ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            0,                                    17,          {  ZVARTYPEID_SCREEN,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,      ZVARTYPEID_BOOL,    ZVARTYPEID_FLOAT,                           -1,                           -1,                           -1,                           } },
	{ "DrawComboCloaked",             ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            0,                                    8,           {  ZVARTYPEID_SCREEN,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     -1,     -1,     -1,     -1,     -1,     -1,     -1,      -1,    -1,                           -1,                           -1,                           -1,                           } },
	{ "Quad",                         ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            0,                                    16,          {  ZVARTYPEID_SCREEN,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,  ZVARTYPEID_FLOAT,                         -1,                           -1,                           -1,                           -1,                           } },
	{ "Triangle",                     ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            0,                                    14,          {  ZVARTYPEID_SCREEN,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,                      -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           } },
	{ "Quad3D",                       ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            0,                                    9,           {  ZVARTYPEID_SCREEN,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "Triangle3D",                   ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            0,                                    9,           {  ZVARTYPEID_SCREEN,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "FastTile",                     ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            0,                                    7,           {  ZVARTYPEID_SCREEN,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     -1,                           -1,                          -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "FastCombo",                    ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            0,                                    7,           {  ZVARTYPEID_SCREEN,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     -1,                           -1,                          -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "DrawString",                   ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            0,                                    10,          {  ZVARTYPEID_SCREEN,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "DrawString",                   ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            0,                                    11,          {  ZVARTYPEID_SCREEN,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "DrawLayer",                    typeVOID,                 FUNCTION,     0,                                1,            0,                                    9,           ARGS_8(S,F,F,F,F,F,F,F,F) },
	{ "DrawScreen",                   typeVOID,                 FUNCTION,     0,                                1,            0,                                    7,           ARGS_6(S,F,F,F,F,F,F) },
	 { "DrawBitmap",                   ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            0,                                    13,          {  ZVARTYPEID_SCREEN,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,       ZVARTYPEID_FLOAT,   ZVARTYPEID_FLOAT,        ZVARTYPEID_FLOAT,    ZVARTYPEID_BOOL,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	 { "DrawBitmapEx",                 ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            0,                                    17,          { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_BOOL, -1,                           -1,                           -1,                              } },
	 { "SetRenderTarget",              ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Message",                      ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getShowingMessage",            ZVARTYPEID_FLOAT,         GETTER,       SHOWNMSG,                         1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setShowingMessage",            ZVARTYPEID_VOID,          SETTER,       SHOWNMSG,                         1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "NumLWeapons",                  ZVARTYPEID_FLOAT,         GETTER,       LWPNCOUNT,                        1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadLWeapon",                  ZVARTYPEID_LWPN,          FUNCTION,     0,                                1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "CreateLWeapon",                ZVARTYPEID_LWPN,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "NumEWeapons",                  ZVARTYPEID_FLOAT,         GETTER,       EWPNCOUNT,                        1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadEWeapon",                  ZVARTYPEID_EWPN,          FUNCTION,     0,                                1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "CreateEWeapon",                ZVARTYPEID_EWPN,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "isSolid",                      ZVARTYPEID_BOOL,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "isSolidLayer",                 ZVARTYPEID_BOOL,          FUNCTION,     0,                                1,            0,                                    4,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "SetSideWarp",                  ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            0,                                    5,           {  ZVARTYPEID_SCREEN,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,     -1,     -1,                           -1,                          -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	 { "SetTileWarp",                  ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            0,                                    5,           {  ZVARTYPEID_SCREEN,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,     -1,     -1,                           -1,                          -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	 { "LayerScreen",                  ZVARTYPEID_FLOAT,         FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "LayerMap",                     ZVARTYPEID_FLOAT,         FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlags[]",                   ZVARTYPEID_FLOAT,         GETTER,       SCREENFLAGSD,                     10,           0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlags[]",                   ZVARTYPEID_VOID,          SETTER,       SCREENFLAGSD,                     10,           0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	This is read-only, but it was not previously blocked! -Z
	{ "getEFlags[]",                  ZVARTYPEID_FLOAT,         GETTER,       SCREENEFLAGSD,                    3,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	This is read-only, but it was not previously blocked! -Z
	{ "setEFlags[]",                  ZVARTYPEID_VOID,          SETTER,       SCREENEFLAGSD,                    3,            0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "TriggerSecrets",               ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRoomType",                  ZVARTYPEID_FLOAT,         GETTER,       ROOMTYPE,                         1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRoomType",                  ZVARTYPEID_VOID,          SETTER,       ROOMTYPE,                         1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRoomData",                  ZVARTYPEID_FLOAT,         GETTER,       ROOMDATA,                         1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRoomData",                  ZVARTYPEID_VOID,          SETTER,       ROOMDATA,                         1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMovingBlockX",              ZVARTYPEID_FLOAT,         GETTER,       PUSHBLOCKX,                       1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMovingBlockX",              ZVARTYPEID_VOID,          SETTER,       PUSHBLOCKX,                       1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMovingBlockY",              ZVARTYPEID_FLOAT,         GETTER,       PUSHBLOCKY,                       1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMovingBlockY",              ZVARTYPEID_VOID,          SETTER,       PUSHBLOCKY,                       1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMovingBlockLayer",          ZVARTYPEID_FLOAT,         GETTER,       PUSHBLOCKLAYER,                   1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMovingBlockLayer",          ZVARTYPEID_VOID,          SETTER,       PUSHBLOCKLAYER,                   1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMovingBlockCombo",          ZVARTYPEID_FLOAT,         GETTER,       PUSHBLOCKCOMBO,                   1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMovingBlockCombo",          ZVARTYPEID_VOID,          SETTER,       PUSHBLOCKCOMBO,                   1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMovingBlockCSet",           ZVARTYPEID_FLOAT,         GETTER,       PUSHBLOCKCSET,                    1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMovingBlockCSet",           ZVARTYPEID_VOID,          SETTER,       PUSHBLOCKCSET,                    1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getUnderCombo",                ZVARTYPEID_FLOAT,         GETTER,       UNDERCOMBO,                       1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setUnderCombo",                ZVARTYPEID_VOID,          SETTER,       UNDERCOMBO,                       1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getUnderCSet",                 ZVARTYPEID_FLOAT,         GETTER,       UNDERCSET,                        1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setUnderCSet",                 ZVARTYPEID_VOID,          SETTER,       UNDERCSET,                        1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "GetSideWarpDMap",              ZVARTYPEID_FLOAT,         FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "GetSideWarpScreen",            ZVARTYPEID_FLOAT,         FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "GetSideWarpType",              ZVARTYPEID_FLOAT,         FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "GetTileWarpDMap",              ZVARTYPEID_FLOAT,         FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "GetTileWarpScreen",            ZVARTYPEID_FLOAT,         FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "GetTileWarpType",              ZVARTYPEID_FLOAT,         FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "ZapIn",                        ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "ZapOut",                       ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "WavyIn",                       ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "WavyOut",                      ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "OpeningWipe",                  ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "ClosingWipe",                  ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "OpeningWipe",                  ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "ClosingWipe",                  ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "CreateLWeaponDx",              ZVARTYPEID_LWPN,          FUNCTION,     0,                                1,            FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Polygon",                      ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            0,                                    6,           {  ZVARTYPEID_SCREEN,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     -1,     -1,     -1,     -1,         -1,     -1,     -1,     -1,     -1,  -1,                         -1,                           -1,                           -1,                           -1,                           } },
	{ "TriggerSecret",                ZVARTYPEID_VOID,          FUNCTION,     0,                                1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
//	mapdata m-> class variables copied to Screen->
	{ "getValid",                     ZVARTYPEID_FLOAT,         GETTER,       SCREENDATAVALID,                  1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setValid",                     ZVARTYPEID_VOID,          SETTER,       SCREENDATAVALID,                  1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getGuy",                       ZVARTYPEID_FLOAT,         GETTER,       SCREENDATAGUY,                    1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setGuy",                       ZVARTYPEID_VOID,          SETTER,       SCREENDATAGUY,                    1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getString",                    ZVARTYPEID_FLOAT,         GETTER,       SCREENDATASTRING,                 1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setString",                    ZVARTYPEID_VOID,          SETTER,       SCREENDATASTRING,                 1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
/*	
	{ "getRoomType",                  ZVARTYPEID_FLOAT,         GETTER,       SCREENDATAROOM,                   1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRoomType",                  ZVARTYPEID_VOID,          SETTER,       SCREENDATAROOM,                   1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
*/	
	
	{ "getItem",                      ZVARTYPEID_FLOAT,         GETTER,       SCREENDATAITEM,                   1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItem",                      ZVARTYPEID_VOID,          SETTER,       SCREENDATAITEM,                   1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getHasItem",                   ZVARTYPEID_FLOAT,         GETTER,       SCREENDATAHASITEM,                1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHasItem",                   ZVARTYPEID_VOID,          SETTER,       SCREENDATAHASITEM,                1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getTileWarpType[]",            ZVARTYPEID_FLOAT,         GETTER,       SCREENDATATILEWARPTYPE,           4,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWarpType[]",            ZVARTYPEID_VOID,          SETTER,       SCREENDATATILEWARPTYPE,           4,            0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getTileWarpOverlay[]",         ZVARTYPEID_BOOL,          GETTER,       SCREENDATATILEWARPOVFLAGS,        4,            0,                                    1,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWarpOverlay[]",         ZVARTYPEID_VOID,          SETTER,       SCREENDATATILEWARPOVFLAGS,        4,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getDoorComboSet",              ZVARTYPEID_FLOAT,         GETTER,       SCREENDATADOORCOMBOSET,           1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setDoorComboSet",              ZVARTYPEID_VOID,          SETTER,       SCREENDATADOORCOMBOSET,           1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWarpReturnX[]",             ZVARTYPEID_FLOAT,         GETTER,       SCREENDATAWARPRETX,               4,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWarpReturnX[]",             ZVARTYPEID_VOID,          SETTER,       SCREENDATAWARPRETX,               4,            0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getWarpReturnY[]",             ZVARTYPEID_FLOAT,         GETTER,       SCREENDATAWARPRETY,               4,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWarpReturnY[]",             ZVARTYPEID_VOID,          SETTER,       SCREENDATAWARPRETY,               4,            0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getWarpReturnC",               ZVARTYPEID_FLOAT,         GETTER,       SCREENDATAWARPRETURNC,            1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setWarpReturnC",               ZVARTYPEID_VOID,          SETTER,       SCREENDATAWARPRETURNC,            1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getStairsX",                   ZVARTYPEID_FLOAT,         GETTER,       SCREENDATASTAIRX,                 1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStairsX",                   ZVARTYPEID_VOID,          SETTER,       SCREENDATASTAIRX,                 1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getStairsY",                   ZVARTYPEID_FLOAT,         GETTER,       SCREENDATASTAIRY,                 1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setStairsY",                   ZVARTYPEID_VOID,          SETTER,       SCREENDATASTAIRY,                 1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getItemX",                     ZVARTYPEID_FLOAT,         GETTER,       SCREENDATAITEMX,                  1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItemX",                     ZVARTYPEID_VOID,          SETTER,       SCREENDATAITEMX,                  1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getItemY",                     ZVARTYPEID_FLOAT,         GETTER,       SCREENDATAITEMY,                  1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItemY",                     ZVARTYPEID_VOID,          SETTER,       SCREENDATAITEMY,                  1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getCSet",                      ZVARTYPEID_FLOAT,         GETTER,       SCREENDATACOLOUR,                 1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setCSet",                      ZVARTYPEID_VOID,          SETTER,       SCREENDATACOLOUR,                 1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPalette",                      ZVARTYPEID_FLOAT,         GETTER,       SCREENDATACOLOUR,                 1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPalette",                      ZVARTYPEID_VOID,          SETTER,       SCREENDATACOLOUR,                 1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getEnemyFlags",                ZVARTYPEID_FLOAT,         GETTER,       SCREENDATAENEMYFLAGS,             1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setEnemyFlags",                ZVARTYPEID_VOID,          SETTER,       SCREENDATAENEMYFLAGS,             1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getTileWarpDMap[]",            ZVARTYPEID_FLOAT,         GETTER,       SCREENDATATILEWARPDMAP,           4,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWarpDMap[]",            ZVARTYPEID_VOID,          SETTER,       SCREENDATATILEWARPDMAP,           4,            0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getTileWarpScreen[]",          ZVARTYPEID_FLOAT,         GETTER,       SCREENDATATILEWARPSCREEN,         4,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWarpScreen[]",          ZVARTYPEID_VOID,          SETTER,       SCREENDATATILEWARPSCREEN,         4,            0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getExitDir",                   ZVARTYPEID_FLOAT,         GETTER,       SCREENDATAEXITDIR,                1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setExitDir",                   ZVARTYPEID_VOID,          SETTER,       SCREENDATAEXITDIR,                1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getEnemy[]",                   ZVARTYPEID_FLOAT,         GETTER,       SCREENDATAENEMY,                  10,           0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setEnemy[]",                   ZVARTYPEID_VOID,          SETTER,       SCREENDATAENEMY,                  10,           0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getPattern",                   ZVARTYPEID_FLOAT,         GETTER,       SCREENDATAPATTERN,                1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPattern",                   ZVARTYPEID_VOID,          SETTER,       SCREENDATAPATTERN,                1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSideWarpType[]",            ZVARTYPEID_FLOAT,         GETTER,       SCREENDATASIDEWARPTYPE,           4,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSideWarpType[]",            ZVARTYPEID_VOID,          SETTER,       SCREENDATASIDEWARPTYPE,           4,            0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSideWarpOverlay[]",        ZVARTYPEID_BOOL,          GETTER,       SCREENDATASIDEWARPOVFLAGS,        4,            0,                                    1,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSideWarpOverlay[]",        ZVARTYPEID_VOID,          SETTER,       SCREENDATASIDEWARPOVFLAGS,        4,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getWarpArrivalX",              ZVARTYPEID_FLOAT,         GETTER,       SCREENDATAWARPARRIVALX,           1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setWarpArrivalX",              ZVARTYPEID_VOID,          SETTER,       SCREENDATAWARPARRIVALX,           1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getWarpArrivalY",              ZVARTYPEID_FLOAT,         GETTER,       SCREENDATAWARPARRIVALY,           1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setWarpArrivalY",              ZVARTYPEID_VOID,          SETTER,       SCREENDATAWARPARRIVALY,           1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getMazePath[]",                ZVARTYPEID_FLOAT,         GETTER,       SCREENDATAPATH,                   4,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMazePath[]",                ZVARTYPEID_VOID,          SETTER,       SCREENDATAPATH,                   4,            0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSideWarpScreen[]",          ZVARTYPEID_FLOAT,         GETTER,       SCREENDATASIDEWARPSC,             4,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSideWarpScreen[]",          ZVARTYPEID_VOID,          SETTER,       SCREENDATASIDEWARPSC,             4,            0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSideWarpID[]",              ZVARTYPEID_FLOAT,         GETTER,       SCREENSIDEWARPID,             4,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSideWarpID[]",              ZVARTYPEID_VOID,          SETTER,       SCREENSIDEWARPID,             4,            0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getTileWarpReturnSquare[]",          ZVARTYPEID_FLOAT,         GETTER,       SCREENDATATWARPRETSQR,             4,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTileWarpReturnSquare[]",          ZVARTYPEID_VOID,          SETTER,       SCREENDATATWARPRETSQR,             4,            0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSideWarpReturnSquare[]",          ZVARTYPEID_FLOAT,         GETTER,       SCREENDATASWARPRETSQR,             4,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSideWarpReturnSquare[]",          ZVARTYPEID_VOID,          SETTER,       SCREENDATASWARPRETSQR,             4,            0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSideWarpDMap[]",            ZVARTYPEID_FLOAT,         GETTER,       SCREENDATASIDEWARPDMAP,           4,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSideWarpDMap[]",            ZVARTYPEID_VOID,          SETTER,       SCREENDATASIDEWARPDMAP,           4,            0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getSideWarpIndex",             ZVARTYPEID_FLOAT,         GETTER,       SCREENDATASIDEWARPINDEX,          1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setSideWarpIndex",             ZVARTYPEID_VOID,          SETTER,       SCREENDATASIDEWARPINDEX,          1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
/*	
	{ "getUnderCombo",                ZVARTYPEID_FLOAT,         GETTER,       SCREENDATAUNDERCOMBO,             1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setUnderCombo",                ZVARTYPEID_VOID,          SETTER,       SCREENDATAUNDERCOMBO,             1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
*/	
	
/*	
	{ "getUnderCSet",                 ZVARTYPEID_FLOAT,         GETTER,       SCREENDATAUNDERCSET,              1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setUnderCSet",                 ZVARTYPEID_VOID,          SETTER,       SCREENDATAUNDERCSET,              1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
*/	
	
	{ "getCatchall",                  ZVARTYPEID_FLOAT,         GETTER,       SCREENDATACATCHALL,               1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCatchall",                  ZVARTYPEID_VOID,          SETTER,       SCREENDATACATCHALL,               1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getCSensitive",                ZVARTYPEID_FLOAT,         GETTER,       SCREENDATACSENSITIVE,             1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCSensitive",                ZVARTYPEID_VOID,          SETTER,       SCREENDATACSENSITIVE,             1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getNoReset",                   ZVARTYPEID_FLOAT,         GETTER,       SCREENDATANORESET,                1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNoReset",                   ZVARTYPEID_VOID,          SETTER,       SCREENDATANORESET,                1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getNoCarry",                   ZVARTYPEID_FLOAT,         GETTER,       SCREENDATANOCARRY,                1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNoCarry",                   ZVARTYPEID_VOID,          SETTER,       SCREENDATANOCARRY,                1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getLayerMap[]",                ZVARTYPEID_FLOAT,         GETTER,       SCREENDATALAYERMAP,               7,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLayerMap[]",                ZVARTYPEID_VOID,          SETTER,       SCREENDATALAYERMAP,               7,            0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getLayerScreen[]",             ZVARTYPEID_FLOAT,         GETTER,       SCREENDATALAYERSCREEN,            7,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLayerScreen[]",             ZVARTYPEID_VOID,          SETTER,       SCREENDATALAYERSCREEN,            7,            0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getLayerOpacity[]",            ZVARTYPEID_FLOAT,         GETTER,       SCREENDATALAYEROPACITY,           7,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLayerOpacity[]",            ZVARTYPEID_VOID,          SETTER,       SCREENDATALAYEROPACITY,           7,            0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getLayerInvisible[]",          ZVARTYPEID_BOOL,          GETTER,       SCREENDATALAYERINVIS,             7,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLayerInvisible[]",          ZVARTYPEID_VOID,          SETTER,       SCREENDATALAYERINVIS,             7,            0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getScriptDraws[]",             ZVARTYPEID_BOOL,          GETTER,       SCREENDATASCRIPTDRAWS,            8,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScriptDraws[]",             ZVARTYPEID_VOID,          SETTER,       SCREENDATASCRIPTDRAWS,            8,            0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getTimedWarpTimer",            ZVARTYPEID_FLOAT,         GETTER,       SCREENDATATIMEDWARPTICS,          1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTimedWarpTimer",            ZVARTYPEID_VOID,          SETTER,       SCREENDATATIMEDWARPTICS,          1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getNextMap",                   ZVARTYPEID_FLOAT,         GETTER,       SCREENDATANEXTMAP,                1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNextMap",                   ZVARTYPEID_VOID,          SETTER,       SCREENDATANEXTMAP,                1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getNextScreen",                ZVARTYPEID_FLOAT,         GETTER,       SCREENDATANEXTSCREEN,             1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNextScreen",                ZVARTYPEID_VOID,          SETTER,       SCREENDATANEXTSCREEN,             1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	
	{ "getCarryoverMap",              ZVARTYPEID_FLOAT,         GETTER,       SCREENDATANEXTMAP,                1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCarryoverMap",               ZVARTYPEID_VOID,          SETTER,       SCREENDATANEXTMAP,                1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getCarryoverScreen",           ZVARTYPEID_FLOAT,         GETTER,       SCREENDATANEXTSCREEN,             1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCarryoverScreen",           ZVARTYPEID_VOID,          SETTER,       SCREENDATANEXTSCREEN,             1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSecretCombo[]",             ZVARTYPEID_FLOAT,         GETTER,       SCREENDATASECRETCOMBO,            128,          0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSecretCombo[]",             ZVARTYPEID_VOID,          SETTER,       SCREENDATASECRETCOMBO,            128,          0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSecretCSet[]",              ZVARTYPEID_FLOAT,         GETTER,       SCREENDATASECRETCSET,             128,          0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSecretCSet[]",              ZVARTYPEID_VOID,          SETTER,       SCREENDATASECRETCSET,             128,          0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSecretFlags[]",             ZVARTYPEID_FLOAT,         GETTER,       SCREENDATASECRETFLAG,             128,          0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSecretFlags[]",             ZVARTYPEID_VOID,          SETTER,       SCREENDATASECRETFLAG,             128,          0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getViewX",                     ZVARTYPEID_FLOAT,         GETTER,       SCREENDATAVIEWX,                  1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setViewX",                     ZVARTYPEID_VOID,          SETTER,       SCREENDATAVIEWX,                  1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getViewY",                     ZVARTYPEID_FLOAT,         GETTER,       SCREENDATAVIEWY,                  1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setViewY",                     ZVARTYPEID_VOID,          SETTER,       SCREENDATAVIEWY,                  1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getWidth",                     ZVARTYPEID_FLOAT,         GETTER,       SCREENDATASCREENWIDTH,            1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setWidth",                     ZVARTYPEID_VOID,          SETTER,       SCREENDATASCREENWIDTH,            1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getHeight",                    ZVARTYPEID_FLOAT,         GETTER,       SCREENDATASCREENHEIGHT,           1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setHeight",                    ZVARTYPEID_VOID,          SETTER,       SCREENDATASCREENHEIGHT,           1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getEntryX",                    ZVARTYPEID_FLOAT,         GETTER,       SCREENDATAENTRYX,                 1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setEntryX",                    ZVARTYPEID_VOID,          SETTER,       SCREENDATAENTRYX,                 1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getEntryY",                    ZVARTYPEID_FLOAT,         GETTER,       SCREENDATAENTRYY,                 1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setEntryY",                    ZVARTYPEID_VOID,          SETTER,       SCREENDATAENTRYY,                 1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getNumFFCs[]",                 ZVARTYPEID_BOOL,          GETTER,       SCREENDATANUMFF,                 33,           0,                                    1,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	//read-only, for now?
	 { "setNumFFCs[]",                 ZVARTYPEID_VOID,          SETTER,       SCREENDATANUMFF,                 33,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getScriptEntry",               ZVARTYPEID_FLOAT,         GETTER,       SCREENDATASCRIPTENTRY,            1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setScriptEntry",               ZVARTYPEID_VOID,          SETTER,       SCREENDATASCRIPTENTRY,            1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getScriptOccupancy",           ZVARTYPEID_FLOAT,         GETTER,       SCREENDATASCRIPTOCCUPANCY,        1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setScriptOccupancy",           ZVARTYPEID_VOID,          SETTER,       SCREENDATASCRIPTOCCUPANCY,        1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getExitScript",                ZVARTYPEID_FLOAT,         GETTER,       SCREENDATASCRIPTEXIT,             1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setExitScript",                ZVARTYPEID_VOID,          SETTER,       SCREENDATASCRIPTEXIT,             1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getOceanSFX",                  ZVARTYPEID_FLOAT,         GETTER,       SCREENDATAOCEANSFX,               1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setOceanSFX",                  ZVARTYPEID_VOID,          SETTER,       SCREENDATAOCEANSFX,               1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getAmbientSFX",                ZVARTYPEID_FLOAT,         GETTER,       SCREENDATAOCEANSFX,               1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setAmbientSFX",                ZVARTYPEID_VOID,          SETTER,       SCREENDATAOCEANSFX,               1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getBossSFX",                   ZVARTYPEID_FLOAT,         GETTER,       SCREENDATABOSSSFX,                1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setBossSFX",                   ZVARTYPEID_VOID,          SETTER,       SCREENDATABOSSSFX,                1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getSecretSFX",                 ZVARTYPEID_FLOAT,         GETTER,       SCREENDATASECRETSFX,              1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSecretSFX",                 ZVARTYPEID_VOID,          SETTER,       SCREENDATASECRETSFX,              1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getItemSFX",                   ZVARTYPEID_FLOAT,         GETTER,       SCREENDATAHOLDUPSFX,              1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setItemSFX",                   ZVARTYPEID_VOID,          SETTER,       SCREENDATAHOLDUPSFX,              1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getMIDI",                      ZVARTYPEID_FLOAT,         GETTER,       SCREENDATASCREENMIDI,             1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMIDI",                      ZVARTYPEID_VOID,          SETTER,       SCREENDATASCREENMIDI,             1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getLensLayer",                 ZVARTYPEID_FLOAT,         GETTER,       SCREENDATALENSLAYER,              1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLensLayer",                 ZVARTYPEID_VOID,          SETTER,       SCREENDATALENSLAYER,              1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	 { "getScreenFlags[]",             ZVARTYPEID_FLOAT,         GETTER,       SCREENDATAFLAGS,                  10,           0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setScreenFlags[]",             ZVARTYPEID_VOID,          SETTER,       SCREENDATAFLAGS,                  10,           0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getScript",	                  ZVARTYPEID_FLOAT,         GETTER,       SCREENSCRIPT, 	             1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setScript",                    ZVARTYPEID_VOID,          SETTER,       SCREENSCRIPT, 	             1,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getInitD[]",            	      ZVARTYPEID_UNTYPED,       GETTER,       SCREENINITD,                   8,            0,                                    2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setInitD[]",            	      ZVARTYPEID_VOID,          SETTER,       SCREENINITD,                   8,            0,                                    3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "LoadNPCByUID",                 ZVARTYPEID_NPC,           FUNCTION,     0,                             1,            FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadLWeaponByUID",             ZVARTYPEID_LWPN,          FUNCTION,     0,                             1,            FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadEWeaponByUID",             ZVARTYPEID_EWPN,          FUNCTION,     0,                             1,            FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "DrawFrame",                    ZVARTYPEID_VOID,          FUNCTION,     0,                             1,            FUNCFLAG_INLINE,                     10,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "SecretsTriggered",             ZVARTYPEID_BOOL,          GETTER,       SCREENSECRETSTRIGGERED,        1,            0,                                    1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "SpawnScreenEnemies",           ZVARTYPEID_BOOL,          FUNCTION,     0,                             1,            FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_SCREEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "TriggerCombo",                 ZVARTYPEID_BOOL,          FUNCTION,     0,                             1,            FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
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

