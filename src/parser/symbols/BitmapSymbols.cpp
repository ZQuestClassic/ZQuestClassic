#include "SymbolDefs.h"

BitmapSymbols BitmapSymbols::singleton = BitmapSymbols();

static AccessorTable BitmapTable[] =
{
//	  name,                     rettype,                  setorget,     var,                  num,           funcFlags,                            numParams,   params
	{ "GetPixel",               ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_BITMAP, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "CountColor",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      6,           { ZVARTYPEID_BITMAP, ZVARTYPEID_BITMAP, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
//	{ "Create",                 ZVARTYPEID_BITMAP,        FUNCTION,     0,                    1,             0,                                    3,           { ZVARTYPEID_BITMAP, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Rectangle",              ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             0,                                    13,          {  ZVARTYPEID_BITMAP,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_BOOL,      ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                      } },
	{ "Circle",                 ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             0,                                    12,          {  ZVARTYPEID_BITMAP,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_BOOL,      ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           } },
	{ "Arc",                    ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             0,                                    15,          {  ZVARTYPEID_BITMAP,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_BOOL,      ZVARTYPEID_BOOL,      ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           } },
	{ "Ellipse",                ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             0,                                    13,          {  ZVARTYPEID_BITMAP,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_BOOL,      ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           } },
	{ "Line",                   ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             0,                                    12,          {  ZVARTYPEID_BITMAP,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           } },
	{ "Spline",                 ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             0,                                    12,          {  ZVARTYPEID_BITMAP,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           } },
	{ "PutPixel",               ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             0,                                    9,           {  ZVARTYPEID_BITMAP,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "DrawCharacter",          ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             0,                                    11,          {  ZVARTYPEID_BITMAP,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "DrawInteger",            ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             0,                                    12,          {  ZVARTYPEID_BITMAP,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "DrawTile",               ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             0,                                    16,          {  ZVARTYPEID_BITMAP,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,      ZVARTYPEID_BOOL,    ZVARTYPEID_FLOAT,                           -1,                           -1,                           -1,                           -1,                           } },
	{ "DrawTileCloaked",        ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             0,                                    8,           {  ZVARTYPEID_BITMAP,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     -1,     -1,     -1,     -1,     -1,     -1,      -1,    -1,                           -1,                           -1,                           -1,                           -1,                           } },
	{ "DrawCombo",              ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             0,                                    17,          {  ZVARTYPEID_BITMAP,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,      ZVARTYPEID_BOOL,    ZVARTYPEID_FLOAT,                           -1,                           -1,                           -1,                           } },
	{ "DrawComboCloaked",       ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             0,                                    8,           {  ZVARTYPEID_BITMAP,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     -1,     -1,     -1,     -1,     -1,     -1,     -1,      -1,    -1,                           -1,                           -1,                           -1,                           } },
	{ "Quad",                   ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             0,                                    17,          {  ZVARTYPEID_BITMAP,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,  ZVARTYPEID_FLOAT,                         ZVARTYPEID_BITMAP,                           -1,                           -1,                           -1,                           } },
	{ "Triangle",               ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             0,                                    15,          {  ZVARTYPEID_BITMAP,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,                      ZVARTYPEID_BITMAP,                           -1,                           -1,                           -1,                           -1,                           -1,                           } },
	{ "Quad3D",                 ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             0,                                    10,          {  ZVARTYPEID_BITMAP,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    ZVARTYPEID_BITMAP,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "Triangle3D",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             0,                                    10,          {  ZVARTYPEID_BITMAP,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    ZVARTYPEID_BITMAP,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "FastTile",               ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             0,                                    7,           {  ZVARTYPEID_BITMAP,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     -1,                           -1,                          -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "FastCombo",              ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             0,                                    7,           {  ZVARTYPEID_BITMAP,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     -1,                           -1,                          -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "DrawString",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             0,                                    10,          {  ZVARTYPEID_BITMAP,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "DrawString",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             0,                                    11,          {  ZVARTYPEID_BITMAP,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                              } },
	{ "DrawLayer",              typeVOID,                 FUNCTION,     0,                    1,             0,                                    9,           ARGS_8(ZVARTYPEID_BITMAP,F,F,F,F,F,F,F,F) },
	{ "DrawLayerSolid",         typeVOID,                 FUNCTION,     0,                    1,             0,                                    9,           ARGS_8(ZVARTYPEID_BITMAP,F,F,F,F,F,F,F,F) },
	{ "DrawLayerSolidity",      typeVOID,                 FUNCTION,     0,                    1,             0,                                    9,           ARGS_8(ZVARTYPEID_BITMAP,F,F,F,F,F,F,F,F) },
	{ "DrawLayerComboTypes",    typeVOID,                 FUNCTION,     0,                    1,             0,                                    9,           ARGS_8(ZVARTYPEID_BITMAP,F,F,F,F,F,F,F,F) },
	{ "DrawLayerComboFlags",    typeVOID,                 FUNCTION,     0,                    1,             0,                                    9,           ARGS_8(ZVARTYPEID_BITMAP,F,F,F,F,F,F,F,F) },
	{ "DrawLayerComboIFlags",   typeVOID,                 FUNCTION,     0,                    1,             0,                                    9,           ARGS_8(ZVARTYPEID_BITMAP,F,F,F,F,F,F,F,F) },
	{ "DrawScreen",             typeVOID,                 FUNCTION,     0,                    1,             0,                                    7,           ARGS_6(ZVARTYPEID_BITMAP,F,F,F,F,F,F) },
	{ "DrawScreenSolid",        typeVOID,                 FUNCTION,     0,                    1,             0,                                    7,           ARGS_6(ZVARTYPEID_BITMAP,F,F,F,F,F,F) },
	{ "DrawScreenSolidity",     typeVOID,                 FUNCTION,     0,                    1,             0,                                    7,           ARGS_6(ZVARTYPEID_BITMAP,F,F,F,F,F,F) },
	{ "DrawScreenComboTypes",   typeVOID,                 FUNCTION,     0,                    1,             0,                                    7,           ARGS_6(ZVARTYPEID_BITMAP,F,F,F,F,F,F) },
	{ "DrawScreenComboFlags",   typeVOID,                 FUNCTION,     0,                    1,             0,                                    7,           ARGS_6(ZVARTYPEID_BITMAP,F,F,F,F,F,F) },
	{ "DrawScreenComboIFlags",  typeVOID,                 FUNCTION,     0,                    1,             0,                                    7,           ARGS_6(ZVARTYPEID_BITMAP,F,F,F,F,F,F) },
	{ "Blit",                   ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             0,                                    17,          { ZVARTYPEID_BITMAP, ZVARTYPEID_FLOAT,ZVARTYPEID_UNTYPED,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_BOOL, -1,                           -1,                           -1,                              } },
	{ "BlitTo",                 ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             0,                                    17,          { ZVARTYPEID_BITMAP, ZVARTYPEID_FLOAT,ZVARTYPEID_UNTYPED,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_BOOL, -1,                           -1,                           -1,                              } },
	{ "RevBlit",                 ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             0,                                    17,          { ZVARTYPEID_BITMAP, ZVARTYPEID_FLOAT,ZVARTYPEID_UNTYPED,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_BOOL, -1,                           -1,                           -1,                              } },
//	{ "Mode7",                  ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             0,                                    17,          { ZVARTYPEID_BITMAP, ZVARTYPEID_FLOAT,ZVARTYPEID_UNTYPED,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_BOOL, -1,                           -1,                           -1,                              } },
	{ "DrawPlane",              ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             0,                                    14,          { ZVARTYPEID_BITMAP, ZVARTYPEID_FLOAT,ZVARTYPEID_UNTYPED,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_BOOL,-1,-1,-1, -1,                           -1,                           -1,                              } },
	{ "isValid",                ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_BITMAP, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "isAllocated",            ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_BITMAP, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWidth",               ZVARTYPEID_FLOAT,         GETTER,       BITMAPWIDTH,          1,             0,                                    1,           { ZVARTYPEID_BITMAP, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWidth",               ZVARTYPEID_VOID,          SETTER,       BITMAPWIDTH,          1,             0,                                    2,           { ZVARTYPEID_BITMAP, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHeight",              ZVARTYPEID_FLOAT,         GETTER,       BITMAPHEIGHT,         1,             0,                                    1,           { ZVARTYPEID_BITMAP, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHeight",              ZVARTYPEID_VOID,          SETTER,       BITMAPHEIGHT,         1,             0,                                    2,           { ZVARTYPEID_BITMAP, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Write",                  ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             0,                                    4,           { ZVARTYPEID_BITMAP, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Read",                   ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_BITMAP, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Create",                 ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             0,                                    4,           { ZVARTYPEID_BITMAP, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Polygon",                ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             0,                                    6,           {  ZVARTYPEID_BITMAP,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     -1,     -1,     -1,     -1,         -1,     -1,     -1,     -1,     -1,  -1,                         -1,                           -1,                           -1,                           -1,                           } },
	{ "ClearToColor",           ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_BITMAP, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Free",                   ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_BITMAP, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Own",                    ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_BITMAP, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "DrawFrame",              ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                     10,           { ZVARTYPEID_BITMAP, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "WriteTile",              ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      7,           { ZVARTYPEID_BITMAP, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "Dither",                 ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      6,           { ZVARTYPEID_BITMAP, ZVARTYPEID_FLOAT, ZVARTYPEID_UNTYPED, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "ReplaceColors",          ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      5,           { ZVARTYPEID_BITMAP, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "ShiftColors",            ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      5,           { ZVARTYPEID_BITMAP, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MaskedDraw",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      4,           { ZVARTYPEID_BITMAP, ZVARTYPEID_FLOAT, ZVARTYPEID_BITMAP, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MaskedDraw",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      5,           { ZVARTYPEID_BITMAP, ZVARTYPEID_FLOAT, ZVARTYPEID_BITMAP, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MaskedDraw",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      6,           { ZVARTYPEID_BITMAP, ZVARTYPEID_FLOAT, ZVARTYPEID_BITMAP, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MaskedBlit",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      5,           { ZVARTYPEID_BITMAP, ZVARTYPEID_FLOAT, ZVARTYPEID_BITMAP, ZVARTYPEID_BITMAP, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MaskedBlit",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      6,           { ZVARTYPEID_BITMAP, ZVARTYPEID_FLOAT, ZVARTYPEID_BITMAP, ZVARTYPEID_BITMAP, ZVARTYPEID_BOOL, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MaskedBlit",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      7,           { ZVARTYPEID_BITMAP, ZVARTYPEID_FLOAT, ZVARTYPEID_BITMAP, ZVARTYPEID_BITMAP, ZVARTYPEID_BOOL, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	

	{ "",                       -1,                       -1,           -1,                   -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

static AccessorTable2 BitmapTable2[] =
{
//	  name,          tag,                  rettype,  var,  funcFlags,  params,optparams
	{ "Clear",         0,          ZVARTYPEID_VOID,   -1,  0,          { ZVARTYPEID_BITMAP, ZVARTYPEID_FLOAT }, { 0 } },
	{ "",              0,          ZVARTYPEID_VOID,   -1,  0,          {}, {}}
};

BitmapSymbols::BitmapSymbols()
{
    table = BitmapTable;
    table2 = BitmapTable2;
	refVar = REFBITMAP;
}

void BitmapSymbols::generateCode()
{
	//void GetPixel(bitmap, x, y)
	{
		Function* function = getFunction("GetPixel", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		//pop pointer to EXP1
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		addOpcode2 (code, new OGraphicsGetpixel(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void GetPixel(bitmap, bitmap, int, int, int, int)
	{
		Function* function = getFunction("CountColor", 6);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OGraphicsCountColor(new VarArgument(EXP1)));
		LABELBACK(label);
		POP_ARGS(6, REFBITMAP);
		RETURN();
		function->giveCode(code);
	}
	/*
	//int32_t Create(bitmap, int32_t map,int32_t scr)
	{
		Function* function = getFunction("Create", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(CREATEBITMAP)));
		RETURN();
		function->giveCode(code);
	}
	*/
	 //void Rectangle(bitmap, float, float, float, float, float, float, float, float, float, float, bool, float)
	{
		Function* function = getFunction("Rectangle", 13);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPRectangleRegister());
		LABELBACK(label);
		POP_ARGS(12, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		RETURN();
        
		function->giveCode(code);
	}
	//void DrawFrame(bitmap, float, float, float, float, float, float, float, bool, float)
	{
		Function* function = getFunction("DrawFrame", 10);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPFrameRegister());
		LABELBACK(label);
		POP_ARGS(9, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		RETURN();
        
		function->giveCode(code);
	}
	//void Read(bitmap, layer, "filename")
	{
		Function* function = getFunction("Read", 3);
		
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OReadBitmap());
		REASSIGN_PTR(EXP2);
		LABELBACK(label);
		POP_ARGS(2, NUL);
		//pop pointer, and ignore it
		POPREF();
		RETURN();
		function->giveCode(code);

	}
	//void Clear(bitmap, layer)
	{
		Function* function = getFunction2("Clear");
		
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OClearBitmap());
		LABELBACK(label);
		POP_ARGS(1, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		
		RETURN();
		function->giveCode(code);

	}
	//void Create(bitmap, layer, int32_t h, int32_t w)
	{
		Function* function = getFunction("Create", 4);
		
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new ORegenerateBitmap());
		REASSIGN_PTR(EXP2);
		LABELBACK(label);
		POP_ARGS(3, NUL);
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);

	}
	//void Write(bitmap, layer, "filename")
	{
		Function* function = getFunction("Write", 4);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OWriteBitmap());
		LABELBACK(label);
		POP_ARGS(3, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		RETURN();
        
		function->giveCode(code);
	}
	//void Circle(bitmap, float, float, float, float, float, float, float, float, float, bool, float)
	{
		Function* function = getFunction("Circle", 12);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPCircleRegister());
		LABELBACK(label);
		POP_ARGS(11, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void Arc(bitmap, float, float, float, float, float, float, float, float, float, float, float, bool, bool, float)
	{
		Function* function = getFunction("Arc", 15);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPArcRegister());
		LABELBACK(label);
		POP_ARGS(14, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void Ellipse(bitmap, float, float, float, float, float, bool, float, float, float)
	{
		Function* function = getFunction("Ellipse", 13);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPEllipseRegister());
		LABELBACK(label);
		POP_ARGS(12, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void Line(bitmap, float, float, float, float, float, float, float, float, float, float, float)
	{
		Function* function = getFunction("Line", 12);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPLineRegister());
		LABELBACK(label);
		POP_ARGS(11, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void Spline(bitmap, float, float, float, float, float, float, float, float, float, float, float)
	{
		Function* function = getFunction("Spline", 12);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPSplineRegister());
		LABELBACK(label);
		POP_ARGS(11, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void PutPixel(bitmap, float, float, float, float, float, float, float, float)
	{
		Function* function = getFunction("PutPixel", 9);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPPutPixelRegister());
		LABELBACK(label);
		POP_ARGS(8, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawCharacter(bitmap, float, float, float, float, float, float, float, float, float, float)
	{
		Function* function = getFunction("DrawCharacter", 11);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawCharRegister());
		LABELBACK(label);
		POP_ARGS(10, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawInteger(bitmap, float, float, float, float, float, float, float, float, float, float, float)
	{
		Function* function = getFunction("DrawInteger", 12);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawIntRegister());
		LABELBACK(label);
		POP_ARGS(11, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawTile(bitmap, float, float, float, float, float, bool, float, float, float)
	{
		Function* function = getFunction("DrawTile", 16);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawTileRegister());
		LABELBACK(label);
		POP_ARGS(15, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawTileCloaked(bitmap, ...args)
	{
		Function* function = getFunction("DrawTileCloaked", 8);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawTileCloakedRegister());
		LABELBACK(label);
		POP_ARGS(7, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawCombo(bitmap, float, float, float, float, float, bool, float, float, float)
	{
		Function* function = getFunction("DrawCombo", 17);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawComboRegister());
		LABELBACK(label);
		POP_ARGS(16, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawComboCloaked(bitmap, ...args)
	{
		Function* function = getFunction("DrawComboCloaked", 8);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawComboCloakedRegister());
		LABELBACK(label);
		POP_ARGS(7, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void Quad(bitmap, float, float, float, float, float, float, float, float, float, bitmap)
	{
		Function* function = getFunction("Quad", 17);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPQuadRegister());
		LABELBACK(label);
		POP_ARGS(16, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void Polygon(bitmap, float, float, float, float, float)
	
	{
		Function* function = getFunction("Polygon", 6);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPPolygonRegister());
		LABELBACK(label);
		POP_ARGS(5, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
    
	//void Triangle(bitmap, float, float, float, float, float, float, float, float, float, bitmap)
	{
		Function* function = getFunction("Triangle", 15);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPTriangleRegister());
		LABELBACK(label);
		POP_ARGS(14, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
    
	//void Quad3D(bitmap, float, float, float, float, float, float, float, float, float, bitmap)
	{
		Function* function = getFunction("Quad3D", 10);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPQuad3DRegister());
		LABELBACK(label);
		POP_ARGS(9, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void Triangle3D(bitmap, float, float, float, float, float, float, float, float, float, bitmap)
	{
		Function* function = getFunction("Triangle3D", 10);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPTriangle3DRegister());
		LABELBACK(label);
		POP_ARGS(9, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
    
	//void FastTile(bitmap, float, float, float, float, float)
	{
		Function* function = getFunction("FastTile", 7);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPFastTileRegister());
		LABELBACK(label);
		POP_ARGS(6, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void FastCombo(bitmap, float, float, float, float, float)
	{
		Function* function = getFunction("FastCombo", 7);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPFastComboRegister());
		LABELBACK(label);
		POP_ARGS(6, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawString(bitmap, float, float, float, float, float, float, float, int32_t *string)
	{
		Function* function = getFunction("DrawString", 10);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawStringRegister());
		LABELBACK(label);
		POP_ARGS(9, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawString(bitmap, float, float, float, float, float, float, float, int32_t *string)
	{
		Function* function = getFunction("DrawString", 12);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawString2Register());
		LABELBACK(label);
		POP_ARGS(11, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawLayer(bitmap, float, float, float, float, float, float, float, float)
	{
		Function* function = getFunction("DrawLayer", 9);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawLayerRegister());
		LABELBACK(label);
		POP_ARGS(8, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawLayerComboIFlags(bitmap, float, float, float, float, float, float, float, float)
	{
		Function* function = getFunction("DrawLayerComboIFlags", 9);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawScreenCIFlagRegister());
		LABELBACK(label);
		POP_ARGS(8, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawLayerComboFlags(bitmap, float, float, float, float, float, float, float, float)
	{
		Function* function = getFunction("DrawLayerComboFlags", 9);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawScreenCFlagRegister());
		LABELBACK(label);
		POP_ARGS(8, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawLayerSolid(bitmap, float, float, float, float, float, float, float, float)
	{
		Function* function = getFunction("DrawLayerSolid", 9);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawScreenSolidMaskRegister());
		LABELBACK(label);
		POP_ARGS(8, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawLayerComboTypes(bitmap, float, float, float, float, float, float, float, float)
	{
		Function* function = getFunction("DrawLayerComboTypes", 9);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawScreenCTypeRegister());
		LABELBACK(label);
		POP_ARGS(8, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	
	//void DrawLayerSolidity(bitmap, float, float, float, float, float, float, float, float)
	{
		Function* function = getFunction("DrawLayerSolidity", 9);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawScreenSolidityRegister());
		LABELBACK(label);
		POP_ARGS(8, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawScreen(bitmap, float, float, float, float, float, float)
	{
		Function* function = getFunction("DrawScreen", 7);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawScreenRegister());
		LABELBACK(label);
		POP_ARGS(6, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	
	//void DrawScreenSolidity(bitmap, float, float, float, float, float, float)
	{
		Function* function = getFunction("DrawScreenSolidity", 7);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawScreenSolidRegister());
		LABELBACK(label);
		POP_ARGS(6, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	
	//void DrawScreenSolid(bitmap, float, float, float, float, float, float)
	{
		Function* function = getFunction("DrawScreenSolid", 7);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawScreenSolid2Register());
		LABELBACK(label);
		POP_ARGS(6, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawScreenComboTypes(bitmap, float, float, float, float, float, float)
	{
		Function* function = getFunction("DrawScreenComboTypes", 7);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawScreenComboTRegister());
		LABELBACK(label);
		POP_ARGS(6, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawScreenComboFlags(bitmap, float, float, float, float, float, float)
	{
		Function* function = getFunction("DrawScreenComboFlags", 7);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawScreenComboFRegister());
		LABELBACK(label);
		POP_ARGS(6, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawScreenComboFlags(bitmap, float, float, float, float, float, float)
	{
		Function* function = getFunction("DrawScreenComboIFlags", 7);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawScreenComboIRegister());
		LABELBACK(label);
		POP_ARGS(6, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	
    
	//void DrawBitmapEx(bitmap, float, float, float, float, float, float, float, float, float, float, bool)
	{
		Function* function = getFunction("Blit", 17);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawBitmapExRegister());
		LABELBACK(label);
		POP_ARGS(16, NUL);
		//pop pointer, and ignore it
		POPREF();
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawPlane(bitmap, float, float, float, float, float, float, float, float, float, float, bool)
	{
		Function* function = getFunction("DrawPlane", 14);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPMode7());
		LABELBACK(label);
		POP_ARGS(13, NUL);
		//pop pointer, and ignore it
		POPREF();
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawBitmapEx(bitmap, float, float, float, float, float, float, float, float, float, float, bool)
	{
		Function* function = getFunction("BlitTo", 17);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPBlitTO());
		LABELBACK(label);
		POP_ARGS(16, NUL);
		//pop pointer, and ignore it
		POPREF();
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawBitmapEx(bitmap, float, float, float, float, float, float, float, float, float, float, bool)
	{
		Function* function = getFunction("RevBlit", 17);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPBlitTO());
		LABELBACK(label);
		POP_ARGS(16, NUL);
		//pop pointer, and ignore it
		POPREF();
        
		RETURN();
		function->giveCode(code);
	}
	//bool isValid(bitmap)
	{
		Function* function = getFunction("isValid", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Check validity
		addOpcode2 (code, new OIsValidBitmap(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//bool isAllocated(bitmap)
	{
		Function* function = getFunction("isAllocated", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Check validity
		addOpcode2 (code, new OIsAllocatedBitmap(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void ClearToColor(bitmap, layer, color)
	{
		Function* function = getFunction("ClearToColor", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBitmapClearToColor());
		LABELBACK(label);
		POP_ARGS(2, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		
		RETURN();
		function->giveCode(code);

	}
	//void Free(bitmap)
	{
		Function* function = getFunction("Free", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new OBitmapFree());
		RETURN();
		function->giveCode(code);
	}
	//void Own(bitmap)
	{
		Function* function = getFunction("Own", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new OBitmapOwn());
		RETURN();
		function->giveCode(code);
	}
	
	//void WriteTile(bitmap, int32_t, int32_t, int32_t, int32_t, bool, bool)
	{
		Function* function = getFunction("WriteTile", 7);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBitmapWriteTile());
		LABELBACK(label);
		POP_ARGS(6, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	
	//void Dither(bitmap, int32_t, int32_t, int32_t, int32_t, int32_t)
	{
		Function* function = getFunction("Dither", 6);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBitmapDither());
		LABELBACK(label);
		POP_ARGS(5, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	
	//void ReplaceColors(bitmap, int32_t, int32_t, int32_t, int32_t)
	{
		Function* function = getFunction("ReplaceColors", 5);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBitmapReplColor());
		LABELBACK(label);
		POP_ARGS(4, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	
	//void ShiftColors(bitmap, int32_t, int32_t, int32_t, int32_t)
	{
		Function* function = getFunction("ShiftColors", 5);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBitmapShiftColor());
		LABELBACK(label);
		POP_ARGS(4, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	
	//void MaskedDraw(bitmap, int32_t, bitmap, int32_t)
	{
		Function* function = getFunction("MaskedDraw", 4);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBitmapMaskDraw());
		LABELBACK(label);
		POP_ARGS(3, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void MaskedDraw(bitmap, int32_t, bitmap, int32_t, int32_t)
	{
		Function* function = getFunction("MaskedDraw", 5);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBitmapMaskDraw2());
		LABELBACK(label);
		POP_ARGS(4, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void MaskedDraw(bitmap, int32_t, bitmap, int32_t, int32_t, int32_t)
	{
		Function* function = getFunction("MaskedDraw", 6);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBitmapMaskDraw3());
		LABELBACK(label);
		POP_ARGS(5, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void MaskedBlit(bitmap, int32_t, bitmap, bitmap)
	{
		Function* function = getFunction("MaskedBlit", 5);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBitmapMaskBlit());
		LABELBACK(label);
		POP_ARGS(4, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void MaskedBlit(bitmap, int32_t, bitmap, bitmap, int32_t)
	{
		Function* function = getFunction("MaskedBlit", 6);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBitmapMaskBlit2());
		LABELBACK(label);
		POP_ARGS(5, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void MaskedBlit(bitmap, int32_t, bitmap, bitmap, int32_t, int32_t)
	{
		Function* function = getFunction("MaskedBlit", 7);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBitmapMaskBlit3());
		LABELBACK(label);
		POP_ARGS(6, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
}

