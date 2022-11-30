#include "SymbolDefs.h"

const int32_t radsperdeg = 572958;
GlobalSymbols GlobalSymbols::singleton;

static AccessorTable GlobalTable[] =
{
//	  name,                     rettype,                     setorget,     var,   numindex,   funcFlags,                            numParams,   params
	{ "printf",                 ZVARTYPEID_VOID,             FUNCTION,     0,     1,          0,                                    1,           { ZVARTYPEID_CHAR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "printf",                 ZVARTYPEID_VOID,             FUNCTION,     0,     1,          0,                                    2,           { ZVARTYPEID_CHAR, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "printf",                 ZVARTYPEID_VOID,             FUNCTION,     0,     1,          0,                                    3,           { ZVARTYPEID_CHAR, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "printf",                 ZVARTYPEID_VOID,             FUNCTION,     0,     1,          0,                                    4,           { ZVARTYPEID_CHAR, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "printf",                 ZVARTYPEID_VOID,             FUNCTION,     0,     1,          0,                                    5,           { ZVARTYPEID_CHAR, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "printf",                 ZVARTYPEID_VOID,             FUNCTION,     0,     1,          0,                                    6,           { ZVARTYPEID_CHAR, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "printf",                 ZVARTYPEID_VOID,             FUNCTION,     0,     1,          0,                                    7,           { ZVARTYPEID_CHAR, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "printf",                 ZVARTYPEID_VOID,             FUNCTION,     0,     1,          0,                                    8,           { ZVARTYPEID_CHAR, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "printf",                 ZVARTYPEID_VOID,             FUNCTION,     0,     1,          0,                                    9,           { ZVARTYPEID_CHAR, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "printf",                 ZVARTYPEID_VOID,             FUNCTION,     0,     1,          0,                                   10,           { ZVARTYPEID_CHAR, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "printf",                 ZVARTYPEID_VOID,             FUNCTION,     0,     1,          0,                                   11,           { ZVARTYPEID_CHAR, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "printf",                 ZVARTYPEID_VOID,             FUNCTION,     0,     1,          0,                                   12,           { ZVARTYPEID_CHAR, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "printf",                 ZVARTYPEID_VOID,             FUNCTION,     0,     1,          0,                                   13,           { ZVARTYPEID_CHAR, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1, -1 } },
	{ "printf",                 ZVARTYPEID_VOID,             FUNCTION,     0,     1,          0,                                   14,           { ZVARTYPEID_CHAR, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1, -1 } },
	{ "printf",                 ZVARTYPEID_VOID,             FUNCTION,     0,     1,          0,                                   15,           { ZVARTYPEID_CHAR, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, -1, -1, -1, -1, -1 } },
	{ "printf",                 ZVARTYPEID_VOID,             FUNCTION,     0,     1,          0,                                   16,           { ZVARTYPEID_CHAR, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, -1, -1, -1, -1 } },
	{ "printf",                 ZVARTYPEID_VOID,             FUNCTION,     0,     1,          0,                                   17,           { ZVARTYPEID_CHAR, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, -1, -1, -1 } },
	
	{ "sprintf",                ZVARTYPEID_FLOAT,            FUNCTION,     0,     1,          0,                                    2,           { ZVARTYPEID_CHAR, ZVARTYPEID_CHAR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "sprintf",                ZVARTYPEID_FLOAT,            FUNCTION,     0,     1,          0,                                    3,           {  ZVARTYPEID_CHAR, ZVARTYPEID_CHAR,ZVARTYPEID_UNTYPED,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1 } },
	{ "sprintf",                ZVARTYPEID_FLOAT,            FUNCTION,     0,     1,          0,                                    4,           {  ZVARTYPEID_CHAR, ZVARTYPEID_CHAR,ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1 } },
	{ "sprintf",                ZVARTYPEID_FLOAT,            FUNCTION,     0,     1,          0,                                    5,           {  ZVARTYPEID_CHAR, ZVARTYPEID_CHAR,ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1 } },
	{ "sprintf",                ZVARTYPEID_FLOAT,            FUNCTION,     0,     1,          0,                                    6,           {  ZVARTYPEID_CHAR, ZVARTYPEID_CHAR,ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1 } },
	{ "sprintf",                ZVARTYPEID_FLOAT,            FUNCTION,     0,     1,          0,                                    7,           {  ZVARTYPEID_CHAR, ZVARTYPEID_CHAR,ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1 } },
	{ "sprintf",                ZVARTYPEID_FLOAT,            FUNCTION,     0,     1,          0,                                    8,           {  ZVARTYPEID_CHAR, ZVARTYPEID_CHAR,ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1 } },
	{ "sprintf",                ZVARTYPEID_FLOAT,            FUNCTION,     0,     1,          0,                                    9,           {  ZVARTYPEID_CHAR, ZVARTYPEID_CHAR,ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1 } },
	{ "sprintf",                ZVARTYPEID_FLOAT,            FUNCTION,     0,     1,          0,                                   10,           {  ZVARTYPEID_CHAR, ZVARTYPEID_CHAR,ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1 } },
	{ "sprintf",                ZVARTYPEID_FLOAT,            FUNCTION,     0,     1,          0,                                   11,           {  ZVARTYPEID_CHAR, ZVARTYPEID_CHAR,ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1 } },
	{ "sprintf",                ZVARTYPEID_FLOAT,            FUNCTION,     0,     1,          0,                                   12,           {  ZVARTYPEID_CHAR, ZVARTYPEID_CHAR,ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1 } },
	{ "sprintf",                ZVARTYPEID_FLOAT,            FUNCTION,     0,     1,          0,                                   13,           {  ZVARTYPEID_CHAR, ZVARTYPEID_CHAR,ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1 } },
	{ "sprintf",                ZVARTYPEID_FLOAT,            FUNCTION,     0,     1,          0,                                   14,           {  ZVARTYPEID_CHAR, ZVARTYPEID_CHAR,ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED,                 -1,                 -1,                 -1,                 -1,                 -1,                 -1 } },
	{ "sprintf",                ZVARTYPEID_FLOAT,            FUNCTION,     0,     1,          0,                                   15,           {  ZVARTYPEID_CHAR, ZVARTYPEID_CHAR,ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED,                 -1,                 -1,                 -1,                 -1,                 -1 } },
	{ "sprintf",                ZVARTYPEID_FLOAT,            FUNCTION,     0,     1,          0,                                   16,           {  ZVARTYPEID_CHAR, ZVARTYPEID_CHAR,ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED,                 -1,                 -1,                 -1,                 -1 } },
	{ "sprintf",                ZVARTYPEID_FLOAT,            FUNCTION,     0,     1,          0,                                   17,           {  ZVARTYPEID_CHAR, ZVARTYPEID_CHAR,ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED,                 -1,                 -1,                 -1 } },
	{ "sprintf",                ZVARTYPEID_FLOAT,            FUNCTION,     0,     1,          0,                                   18,           {  ZVARTYPEID_CHAR, ZVARTYPEID_CHAR,ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED,                 -1,                 -1 } },
//	TYPE_UNTYPED
	
	{ "",                       -1,                          -1,           -1,    -1,         0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

static AccessorTable2 GlobalTable2[] =
{
//	  name,                    tag,                  rettype,  var,  funcFlags,  params,optparams
	{ "Rand",                    0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT } },
	{ "Quit",                    0,          ZVARTYPEID_VOID,   -1,  0,          {},{} },
	{ "Waitframe",               0,          ZVARTYPEID_VOID,   -1,  0,          {},{} },
	{ "Waitdraw",                0,          ZVARTYPEID_VOID,   -1,  0,          {},{} },
	{ "WaitTo",                  0,          ZVARTYPEID_VOID,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL },{} },
	{ "WaitEvent",               0,         ZVARTYPEID_FLOAT,   -1,  0,          {},{} },
	{ "Trace",                   0,          ZVARTYPEID_VOID,   -1,  0,          { ZVARTYPEID_UNTYPED },{} },
	 { "TraceLWeapon",           0,          ZVARTYPEID_VOID,   -1,  0,          { ZVARTYPEID_LWPN },{} },
	 { "TraceEWeapon",           0,          ZVARTYPEID_VOID,   -1,  0,          { ZVARTYPEID_EWPN },{} },
	 { "TraceNPC",               0,          ZVARTYPEID_VOID,   -1,  0,          { ZVARTYPEID_NPC },{} },
	 { "TraceFFC",               0,          ZVARTYPEID_VOID,   -1,  0,          { ZVARTYPEID_FFC },{} },
	 { "TraceItem",              0,          ZVARTYPEID_VOID,   -1,  0,          { ZVARTYPEID_ITEM },{} },
	 { "TraceItemData",          0,          ZVARTYPEID_VOID,   -1,  0,          { ZVARTYPEID_ITEMCLASS },{} },
	{ "TraceB",                  0,          ZVARTYPEID_VOID,   -1,  0,          { ZVARTYPEID_BOOL },{} },
	{ "TraceS",                  0,          ZVARTYPEID_VOID,   -1,  0,          { ZVARTYPEID_FLOAT },{} },
	{ "TraceNL",                 0,          ZVARTYPEID_VOID,   -1,  0,          {},{} },
	{ "ClearTrace",              0,          ZVARTYPEID_VOID,   -1,  0,          {},{} },
	{ "TraceToBase",             0,          ZVARTYPEID_VOID,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	{ "Sin",                     0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT },{} },
	{ "Cos",                     0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT },{} },
	{ "Tan",                     0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT },{} },
	{ "ArcTan",                  0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	{ "ArcSin",                  0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT },{} },
	{ "ArcCos",                  0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT },{} },
	{ "RadianSin",               0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT },{} },
	{ "RadianCos",               0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT },{} },
	{ "RadianTan",               0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT },{} },
	{ "Max",                     0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	{ "Min",                     0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	{ "Pow",                     0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	{ "LPow",                    0,          ZVARTYPEID_LONG,   -1,  0,          { ZVARTYPEID_LONG, ZVARTYPEID_LONG },{} },
	{ "InvPow",                  0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	{ "Factorial",               0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT },{} },
	{ "Abs",                     0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT },{} },
	{ "Log10",                   0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT },{} },
	{ "Ln",                      0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT },{} },
	{ "Sqrt",                    0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT },{} },
	{ "CopyTile",                0,          ZVARTYPEID_VOID,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	
	{ "SwapTile",                0,          ZVARTYPEID_VOID,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	{ "ClearTile",               0,          ZVARTYPEID_VOID,   -1,  0,          { ZVARTYPEID_FLOAT },{} },
	{ "GetScriptRAM",            0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT },{} },
	{ "SetScriptRAM",            0,          ZVARTYPEID_VOID,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	{ "GetGlobalRAM",            0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT },{} },
	{ "SetGlobalRAM",            0,          ZVARTYPEID_VOID,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	{ "SetColorBuffer",          0,          ZVARTYPEID_VOID,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	{ "SetDepthBuffer",          0,          ZVARTYPEID_VOID,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	{ "GetColorBuffer",          0,          ZVARTYPEID_VOID,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	{ "GetDepthBuffer",          0,          ZVARTYPEID_VOID,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	{ "SizeOfArray",             0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_UNTYPED },{} },
	{ "ResizeArray",             0,          ZVARTYPEID_VOID,   -1,  0,          { ZVARTYPEID_UNTYPED, ZVARTYPEID_FLOAT },{} },
	{ "OwnArray",                0,          ZVARTYPEID_VOID,   -1,  0,          { ZVARTYPEID_UNTYPED },{} },
	{ "DestroyArray",            0,          ZVARTYPEID_VOID,   -1,  0,          { ZVARTYPEID_UNTYPED },{} },
	{ "OwnObject",               0,          ZVARTYPEID_VOID,   -1,  0,          { ZVARTYPEID_UNTYPED },{} },
	{ "GlobalObject",            0,          ZVARTYPEID_VOID,   -1,  0,          { ZVARTYPEID_UNTYPED },{} },
	 { "SizeOfArrayBool",        0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_BOOL },{} },
	 { "SizeOfArrayFFC",         0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FFC },{} },
	 { "SizeOfArrayItem",        0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_ITEM },{} },
	 { "SizeOfArrayLWeapon",     0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_LWPN },{} },
	 { "SizeOfArrayEWeapon",     0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_EWPN },{} },
	 { "SizeOfArrayNPC",         0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_NPC },{} },
	 { "SizeOfArrayItemdata",    0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_ITEMCLASS },{} },
	{ "OverlayTile",             0,          ZVARTYPEID_VOID,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	{ "Untype",                  0,       ZVARTYPEID_UNTYPED,   -1,  0,          { ZVARTYPEID_UNTYPED },{} },
	{ "Byte",                    0,       ZVARTYPEID_UNTYPED,   -1,  0,          { ZVARTYPEID_UNTYPED },{} },
	{ "Int8",                    0,       ZVARTYPEID_UNTYPED,   -1,  0,          { ZVARTYPEID_UNTYPED },{} },
	{ "SignedByte",              0,       ZVARTYPEID_UNTYPED,   -1,  0,          { ZVARTYPEID_UNTYPED },{} },
	{ "Word",                    0,       ZVARTYPEID_UNTYPED,   -1,  0,          { ZVARTYPEID_UNTYPED },{} },
	{ "Int16",                   0,       ZVARTYPEID_UNTYPED,   -1,  0,          { ZVARTYPEID_UNTYPED },{} },
	{ "Short",                   0,       ZVARTYPEID_UNTYPED,   -1,  0,          { ZVARTYPEID_UNTYPED },{} },
	{ "Integer",                 0,       ZVARTYPEID_UNTYPED,   -1,  0,          { ZVARTYPEID_UNTYPED },{} },
	{ "Floor",                   0,       ZVARTYPEID_UNTYPED,   -1,  0,          { ZVARTYPEID_UNTYPED },{} },
	{ "Ceiling",                 0,       ZVARTYPEID_UNTYPED,   -1,  0,          { ZVARTYPEID_UNTYPED },{} },
	{ "GetSystemTime",           0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT },{} },
	
	{ "Distance",                0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	{ "Distance",                0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	{ "LongDistance",            0,          ZVARTYPEID_LONG,   -1,  0,          { ZVARTYPEID_LONG, ZVARTYPEID_LONG, ZVARTYPEID_LONG, ZVARTYPEID_LONG },{} },
	{ "LongDistance",            0,          ZVARTYPEID_LONG,   -1,  0,          { ZVARTYPEID_LONG, ZVARTYPEID_LONG, ZVARTYPEID_LONG, ZVARTYPEID_LONG, ZVARTYPEID_LONG },{} },
	{ "strcmp",                  0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	{ "strncmp",                 0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	{ "stricmp",                 0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	{ "strnicmp",                0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	{ "strcpy",                  0,          ZVARTYPEID_VOID,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	{ "itoacat",                 0,          ZVARTYPEID_VOID,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	{ "ArrayCopy",               0,          ZVARTYPEID_VOID,   -1,  0,          { ZVARTYPEID_UNTYPED, ZVARTYPEID_UNTYPED },{} },
	{ "strlen",                  0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT },{} },
	{ "atoi",                    0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT },{} },
	{ "ilen",                    0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT },{} },
	{ "utol",                    0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT },{} },
	{ "ltou",                    0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT },{} },
	{ "convcase",                0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT },{} },
	{ "itoa",                    0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	{ "xtoa",                    0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	{ "xtoi",                    0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT },{} },
	{ "SaveSRAM",                0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	{ "LoadSRAM",                0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	
	{ "strcat",                  0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	{ "strchr",                  0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	{ "strcspn",                 0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	{ "strspn",                  0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	{ "strstr",                  0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	{ "strrchr",                 0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT },{} },
	
	{ "SRand",                   0,          ZVARTYPEID_VOID,   -1,  0,          { ZVARTYPEID_LONG },{} },
	{ "SRand",                   0,          ZVARTYPEID_LONG,   -1,  0,          {},{} },
	
	{ "IsValidArray",            0,          ZVARTYPEID_BOOL,   -1,  0,          { ZVARTYPEID_UNTYPED },{} },
	
	 { "EngineDegtoRad",         0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT },{} },
	 { "EngineRadtoDeg",         0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT },{} },
	{ "DegtoRad",                0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT },{} },
	{ "RadtoDeg",                0,         ZVARTYPEID_FLOAT,   -1,  0,          { ZVARTYPEID_FLOAT },{} },
	{ "",                        0,          ZVARTYPEID_VOID,   -1,  0,          {}, {}}
	//Unused old junk
	//overload, 2 args
	// { "atoi",                 ZVARTYPEID_FLOAT,            FUNCTION,     0,     1,          0,                                    2,           { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	// { "xtoi",                 ZVARTYPEID_FLOAT,            FUNCTION,     0,     1,          0,                                    1,           { ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	//overload, 2 args
	// { "xtoi",                 ZVARTYPEID_FLOAT,            FUNCTION,     0,     1,          0,                                    2,           { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	// { "xlen",                 ZVARTYPEID_FLOAT,            FUNCTION,     0,     1,          0,                                    1,           { ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	//overload, 2 args
	// { "xlen",                 ZVARTYPEID_FLOAT,            FUNCTION,     0,     1,          0,                                    2,           { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	// { "xtoa",                  ZVARTYPEID_FLOAT,            FUNCTION,     0,     1,          0,                                    2,           { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	//overload, 2 args
	// { "ilen",                 ZVARTYPEID_FLOAT,            FUNCTION,     0,     1,          0,                                    2,           { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	//  { "CalculateSpline",        0,          ZVARTYPEID_FLOAT,            FUNCTION,     0,     1,          0,                                    7,           { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	//  { "CollisionRect",          0,           ZVARTYPEID_BOOL,             FUNCTION,     0,     1,          0,                                    8,           { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	//  { "CollisionBox",           0,           ZVARTYPEID_BOOL,             FUNCTION,     0,     1,          0,                                    12,          { ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1 } },

};

GlobalSymbols::GlobalSymbols()
{
    table = GlobalTable;
    table2 = GlobalTable2;
    refVar = NUL;
	hasPrefixType = false;
}

void GlobalSymbols::generateCode()
{
	//no input, one return
	/*
    //untyped NULL()(global)
    {
	    Function* function = getFunction("NULL", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop pointer, and ignore it
		ASSERT_NUL();
        addOpcode2 (code, new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
        LABELBACK(label);
        RETURN();
        function->giveCode(code);
    }
	*/
    
    //one input, one return
    //untyped Untype(untype)
    {
	    Function* function = getFunction("Untype", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		/*
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);*/
        //addOpcode2 (code, new OSetImmediate(new VarArgument(EXP1), new VarArgument(EXP2)));
        RETURN(); //Just return it?
        function->giveCode(code);
    }
    //float Distance(game, int32_t, int32_t, int32_t, int32_t)
	{
		Function* function = getFunction("Distance", 4);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(DISTANCE)));
		RETURN();
		function->giveCode(code);
	}
	//float Distance(game, int32_t, int32_t, int32_t, int32_t, int32_t)
	{
		Function* function = getFunction("Distance", 5);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(WHAT_NO_7)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(DISTANCESCALE)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t LongDistance(game, int32_t, int32_t, int32_t, int32_t)
	{
		Function* function = getFunction("LongDistance", 4);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(LONGDISTANCE)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t LongDistance(game, int32_t, int32_t, int32_t, int32_t, int32_t)
	{
		Function* function = getFunction("LongDistance", 5);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(WHAT_NO_7)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(LONGDISTANCESCALE)));
		RETURN();
		function->giveCode(code);
	}
	
    //int32_t Rand(int32_t maxval)
    {
	    Function* function = getFunction("Rand", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop maxval
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new ORandRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
	
    //int_full SRand(int_full seed)
    {
	    Function* function = getFunction("SRand", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop seed
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OSRandRegister(new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
	
    //int_full SRand()
    {
	    Function* function = getFunction("SRand", 0);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OSRandRand(new VarArgument(EXP1)));
        LABELBACK(label);
        RETURN();
        function->giveCode(code);
    }
    
	//bool IsValidArray(untyped)
	{
		Function* function = getFunction("IsValidArray", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//Pop array ptr
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OIsValidArray(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
    
    //int32_t GetSystemTime(int32_t category)
    {
	    Function* function = getFunction("GetSystemTime", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OGetSystemRTCRegister(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    
    //void Quit()
    {
	    Function* function = getFunction("Quit", 0);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OQuit());
        LABELBACK(label);
        function->giveCode(code);
    }
    //void Waitframe()
    {
	    Function* function = getFunction("Waitframe", 0);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OWaitframe());
        LABELBACK(label);
        RETURN();
        function->giveCode(code);
    }
    //void Waitdraw()
    {
	    Function* function = getFunction("Waitdraw", 0);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OWaitdraw());
        LABELBACK(label);
        RETURN();
        function->giveCode(code);
    }
	//void WaitTo(int32_t, bool)
	{
	    Function* function = getFunction("WaitTo", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OWaitTo(new VarArgument(EXP1), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
	}
	//int WaitEvent()
	{
	    Function* function = getFunction("WaitEvent", 0);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OWaitEvent());
        LABELBACK(label);
        RETURN();
        function->giveCode(code);
	}
    //void Trace(int32_t val)
    {
	    Function* function = getFunction2("Trace");
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OTraceRegister(new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    {
	 TRACING_FUNCTION("TraceLWeapon", 1);   
    }
    {
	 TRACING_FUNCTION("TraceEWeapon", 1);   
    }
    {
	 TRACING_FUNCTION("TraceNPC", 1);   
    }
    {
	 TRACING_FUNCTION("TraceFFC", 1);   
    }
    {
	 TRACING_FUNCTION("TraceItem", 1);   
    }
    {
	 TRACING_FUNCTION("TraceItemData", 1);   
    }
    //void TraceB(bool val)
    {
	    Function* function = getFunction("TraceB", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OTrace2Register(new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //void TraceS(bool val)
    {
	    Function* function = getFunction("TraceS", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        LABELBACK(label);
        addOpcode2 (code, new OTrace6Register(new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //void TraceNL()
    {
	    Function* function = getFunction("TraceNL", 0);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OTrace3());
        LABELBACK(label);
        RETURN();
        function->giveCode(code);
    }
    //void ClearTrace()
    {
	    Function* function = getFunction("ClearTrace", 0);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OTrace4());
        LABELBACK(label);
        RETURN();
        function->giveCode(code);
    }
    //void TraceToBase(float, float, float)
    {
	    Function* function = getFunction("TraceToBase", 3);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OTrace5Register());
        LABELBACK(label);
		POP_ARGS(3, NUL);
        
        RETURN();
        function->giveCode(code);
    }
    //int32_t Sin(int32_t val)
    {
	    Function* function = getFunction("Sin", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OSinRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t RadianSin(int32_t val)
    {
	    Function* function = getFunction("RadianSin", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OMultImmediate(new VarArgument(EXP2), new LiteralArgument(radsperdeg)));
        addOpcode2 (code, new OSinRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t ArcSin(int32_t val)
    {
	    Function* function = getFunction("ArcSin", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OArcSinRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t Cos(int32_t val)
    {
	    Function* function = getFunction("Cos", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OCosRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t RadianCos(int32_t val)
    {
	    Function* function = getFunction("RadianCos", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OMultImmediate(new VarArgument(EXP2), new LiteralArgument(radsperdeg)));
        addOpcode2 (code, new OCosRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t ArcCos(int32_t val)
    {
	    Function* function = getFunction("ArcCos", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OArcCosRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t Tan(int32_t val)
    {
	    Function* function = getFunction("Tan", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OTanRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t ArcTan(int32_t X, int32_t Y)
    {
	    Function* function = getFunction("ArcTan", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        addOpcode2 (code, new OATanRegister(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    
    //int32_t RadianTan(int32_t val)
    {
	    Function* function = getFunction("RadianTan", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OMultImmediate(new VarArgument(EXP2), new LiteralArgument(radsperdeg)));
        addOpcode2 (code, new OTanRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t Max(int32_t first, int32_t second)
    {
	    Function* function = getFunction("Max", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OMaxRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t Min(int32_t first, int32_t second)
    {
	    Function* function = getFunction("Min", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OMinRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t Pow(int32_t first, int32_t second)
    {
	    Function* function = getFunction("Pow", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OPowRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t LPow(int32_t first, int32_t second)
    {
	    Function* function = getFunction("LPow", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OLPowRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t InvPow(int32_t first, int32_t second)
    {
	    Function* function = getFunction("InvPow", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OInvPowRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t Factorial(int32_t val)
    {
	    Function* function = getFunction("Factorial", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OFactorial(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t Abs(int32_t val)
    {
	    Function* function = getFunction("Abs", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OAbsRegister(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t Log10(int32_t val)
    {
	    Function* function = getFunction("Log10", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OLog10Register(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t Ln(int32_t val)
    {
	    Function* function = getFunction("Ln", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OLogERegister(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t Sqrt(int32_t val)
    {
	    Function* function = getFunction("Sqrt", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OSqrtRegister(new VarArgument(EXP1), new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    
    
    //int32_t CopyTile(int32_t source, int32_t dest)
    {
	    Function* function = getFunction("CopyTile", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OCopyTileRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    
    
    //int32_t SwapTile(int32_t first, int32_t second)
    {
	    Function* function = getFunction("SwapTile", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OSwapTileRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    
    //int32_t OverlayTile(int32_t first, int32_t second)
    {
	    Function* function = getFunction("OverlayTile", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OOverlayTileRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    
    //void ClearTile(int32_t tile)
    {
	    Function* function = getFunction("ClearTile", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OClearTileRegister(new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //void GetGlobalRAM(int32_t)
    {
	    Function* function = getFunction("GetGlobalRAM", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        LABELBACK(label);
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(GLOBALRAMD)));
        RETURN();
        function->giveCode(code);
    }
    //void SetGlobalRAM(int32_t, int32_t)
    {
	    Function* function = getFunction("SetGlobalRAM", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        addOpcode2 (code, new OSetRegister(new VarArgument(GLOBALRAMD), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //void GetScriptRAM(int32_t)
    {
	    Function* function = getFunction("GetScriptRAM", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        LABELBACK(label);
        //addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(SCRIPTRAMD)));
        RETURN();
        function->giveCode(code);
    }
    //void SetScriptRAM(int32_t, int32_t)
    {
	    Function* function = getFunction("SetScriptRAM", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        addOpcode2 (code, new OSetRegister(new VarArgument(SCRIPTRAMD), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //void SetColorBuffer(int32_t amount, int32_t offset, int32_t stride, int32_t *ptr)
    {
	    Function* function = getFunction("SetColorBuffer", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OSetColorBufferRegister());
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //void SetDepthBuffer(int32_t amount, int32_t offset, int32_t stride, int32_t *ptr)
    {
	    Function* function = getFunction("SetDepthBuffer", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OSetDepthBufferRegister());
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //void GetColorBuffer(int32_t amount, int32_t offset, int32_t stride, int32_t *ptr)
    {
	    Function* function = getFunction("GetColorBuffer", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OGetColorBufferRegister());
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //void GetDepthBuffer(int32_t amount, int32_t offset, int32_t stride, int32_t *ptr)
    {
	    Function* function = getFunction("GetDepthBuffer", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OGetDepthBufferRegister());
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t SizeOfArray(int32_t val)
    {
	    Function* function = getFunction("SizeOfArray", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OArraySize(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //void ResizeArray(untyped ptr, int sz)
    {
	    Function* function = getFunction("ResizeArray", 2);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OResizeArrayRegister(new VarArgument(EXP1),new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //void OwnArray(untyped ptr)
    {
	    Function* function = getFunction("OwnArray", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OOwnArrayRegister(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //void OwnObject(untyped ptr)
    {
	    Function* function = getFunction("OwnObject", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OOwnObject(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //void GlobalObject(untyped ptr)
    {
	    Function* function = getFunction("GlobalObject", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OGlobalObject(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //void DestroyArray(untyped ptr)
    {
	    Function* function = getFunction("DestroyArray", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new ODestroyArrayRegister(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    
    {
	    Function* function = getFunction("Byte", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OByte(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    {
	    Function* function = getFunction("Int8", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OByte(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    {
	    Function* function = getFunction("SignedByte", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OSByte(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    {
	    Function* function = getFunction("Word", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OWord(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    {
	    Function* function = getFunction("Int16", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OWord(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    {
	    Function* function = getFunction("Short", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OShort(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    {
	    Function* function = getFunction("Integer", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OToInteger(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    {
	    Function* function = getFunction("Floor", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OFloor(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    {
	    Function* function = getFunction("Ceiling", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OCeiling(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t SizeOfArrayFFC(ffc *ptr)
    {
	    Function* function = getFunction("SizeOfArrayFFC", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OArraySizeF(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    
     //int32_t SizeOfArrayNPC(npc *ptr)
    {
	    Function* function = getFunction("SizeOfArrayNPC", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OArraySizeN(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    } 
    
    //int32_t SizeOfArrayBool(bool *ptr)
    {
	    Function* function = getFunction("SizeOfArrayBool", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OArraySizeB(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    } //int32_t SizeOfArrayItem(item *ptr)
    {
	    Function* function = getFunction("SizeOfArrayItem", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OArraySizeI(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    } //int32_t SizeOfArrayItemdata(itemdata *ptr)
    {
	    Function* function = getFunction("SizeOfArrayItemdata", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OArraySizeID(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    } //int32_t SizeOfArrayLWeapon(lweapon *ptr)
    {
	    Function* function = getFunction("SizeOfArrayLWeapon", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OArraySizeL(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    } 
    //int32_t SaveSRAM(eweapon *ptr)
    {
	    Function* function = getFunction("SaveSRAM", 2);
	    int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OSaveGameStructs(new VarArgument(EXP1), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t LoadSRAM(eweapon *ptr)
    {
	    Function* function = getFunction("LoadSRAM", 2);
	    int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        addOpcode2 (code, new OReadGameStructs(new VarArgument(EXP1), new VarArgument(EXP2)));
        RETURN();
        function->giveCode(code);
    }
    //int32_t SizeOfArrayEWeapon(eweapon *ptr)
    {
	    Function* function = getFunction("SizeOfArrayEWeapon", 1);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        addOpcode2 (code, new OArraySizeE(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
    //String and Array Functions (String.h, Array.h)
    	//int32_t strlen(*p)
	{
		Function* function = getFunction("strlen", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new Ostrlen(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//void strcpy(str* dest, str* src)
	{
		Function* function = getFunction("strcpy", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		addOpcode2 (code, new Ostrcpy(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//void itoacat(str* dest, int32_t)
	{
		Function* function = getFunction("itoacat", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		addOpcode2 (code, new Oitoacat(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t strcmp(*a, *b)
	{
		Function* function = getFunction("strcmp",2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		addOpcode2 (code, new OStrCmp(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t strncmp(*a, *b, int32_t len)
	{
		Function* function = getFunction("strncmp", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OStrNCmp(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
    //int32_t stricmp(*a, *b)
	{
		Function* function = getFunction("stricmp",2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		addOpcode2 (code, new OStrICmp(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t strnicmp(*a, *b, int32_t len)
	{
		Function* function = getFunction("strnicmp", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OStrNICmp(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
    
	//int32_t ArrayCopy(int32_t source, int32_t dest)
	{
		Function* function = getFunction("ArrayCopy", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		addOpcode2 (code, new oARRAYCOPY(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t atoi(*p)
	{
		Function* function = getFunction("atoi", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new Oatoi(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	/*int32_t atoi2(*a, *b)
	{
		Function* function = getFunction("atoi",2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		addOpcode2 (code, new Oatoi2(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}*/
	
	//int32_t ilen(*p)
	{
		Function* function = getFunction("ilen", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new Oilen(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t utol(*p)
	{
		Function* function = getFunction("utol", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new Ouppertolower(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t ltou(*p)
	{
		Function* function = getFunction("ltou", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new Olowertoupper(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t convcase(*p)
	{
		Function* function = getFunction("convcase", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new Oconvertcase(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
        /*
	//int32_t ilen2(*a, *b)
	{
		Function* function = getFunction("ilen",2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		addOpcode2 (code, new Oilen2(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	*/
    
	
	//void itoa_c(str* dest, int32_t)
	{
		Function* function = getFunction("itoa", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		addOpcode2 (code, new Oitoa(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//void xtoa_c(str* dest, int32_t)
	{
		Function* function = getFunction("xtoa", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		addOpcode2 (code, new Oxtoa(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t xtoi(*p)
	{
		Function* function = getFunction("xtoi", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new Oxtoi(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	/*int32_t remchr(*a, *b)
	{
		Function* function = getFunction("remchr",2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		addOpcode2 (code, new Oremchr2(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}*/
	//int32_t strcat(*a, *b)
	{
		Function* function = getFunction("strcat",2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		addOpcode2 (code, new Ostrcat(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t strchr(*a, *b)
	{
		Function* function = getFunction("strchr",2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		addOpcode2 (code, new Ostrchr(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t strcspn(*a, *b)
	{
		Function* function = getFunction("strcspn",2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		addOpcode2 (code, new Ostrcspn(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t strspn(*a, *b)
	{
		Function* function = getFunction("strspn",2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		addOpcode2 (code, new Ostrspn(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t strstr(*a, *b)
	{
		Function* function = getFunction("strstr",2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		addOpcode2 (code, new Ostrstr(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t strrchr(*a, *b)
	{
		Function* function = getFunction("strrchr",2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		addOpcode2 (code, new Ostrrchr(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	/*
	//int32_t xlen(*p)
	{
		Function* function = getFunction("xlen", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new Oxlen(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t xlen(*a, *b)
	{
		Function* function = getFunction("xlen",2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		addOpcode2 (code, new Oxlen2(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t xtoa(*a, *b)
	{
		Function* function = getFunction("xtoa",2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		addOpcode2 (code, new Oxtoa(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t xtoi(*p)
	{
		Function* function = getFunction("xtoi", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new Oxtoi(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t xtoi2(*a, *b)
	{
		Function* function = getFunction("xtoi",2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		addOpcode2 (code, new Oxtoi2(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	*/
	
	//void printf(str* format, untyped args...)
	for(int32_t q = 0; q <= 16; ++q) //16 args max
	{
		Function* function = getFunction("printf", q+1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPrintfImmediate(new LiteralArgument(q * 10000)));
		LABELBACK(label);
		POP_ARGS(q+1, NUL);
		RETURN();
		function->giveCode(code);
	}
	//void sprintf(str* buf, str* format, untyped args...)
	for(int32_t q = 0; q <= 16; ++q) //16 args max
	{
		Function* function = getFunction("sprintf", q+2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OSPrintfImmediate(new LiteralArgument(q * 10000)));
		LABELBACK(label);
		POP_ARGS(q+2, NUL);
		RETURN();
		function->giveCode(code);
	}
	//int32_t EngineDegtoRad(int32_t val)
	{
		Function* function = getFunction("EngineDegtoRad", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new OEngineDegtoRad(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t EngineRadtoDeg(int32_t val)
	{
		Function* function = getFunction("EngineRadtoDeg", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new OEngineRadtoDeg(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t DegtoRad(int32_t val)
	{
		Function* function = getFunction("DegtoRad", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new OEngineDegtoRad(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t RadtoDeg(int32_t val)
	{
		Function* function = getFunction("RadtoDeg", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new OEngineRadtoDeg(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
}

