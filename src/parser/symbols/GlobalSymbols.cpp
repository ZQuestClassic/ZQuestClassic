#include "SymbolDefs.h"

const int32_t radsperdeg = 572958;
GlobalSymbols GlobalSymbols::singleton;

static AccessorTable2 GlobalTable2[] =
{
//	  name,                    tag,            rettype,  var,  funcFlags,  params,optparams
	{ "Quit",                    0,          ZTID_VOID,   -1,          0,  {},{} },
	{ "Waitframe",               0,          ZTID_VOID,   -1,          0,  {},{} },
	{ "Waitdraw",                0,          ZTID_VOID,   -1,          0,  {},{} },
	{ "WaitTo",                  0,          ZTID_VOID,   -1,          0,  { ZTID_FLOAT, ZTID_BOOL },{} },
	{ "WaitEvent",               0,         ZTID_FLOAT,   -1,          0,  {},{} },
	{ "Trace",                   0,          ZTID_VOID,   -1,          0,  { ZTID_UNTYPED },{} },
	{ "TraceB",                  0,          ZTID_VOID,   -1,          0,  { ZTID_BOOL },{} },
	{ "TraceS",                  0,          ZTID_VOID,   -1,          0,  { ZTID_FLOAT },{} },
	{ "TraceNL",                 0,          ZTID_VOID,   -1,          0,  {},{} },
	{ "ClearTrace",              0,          ZTID_VOID,   -1,          0,  {},{} },
	{ "TraceToBase",             0,          ZTID_VOID,   -1,          0,  { ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "Sin",                     0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT },{} },
	{ "Cos",                     0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT },{} },
	{ "Tan",                     0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT },{} },
	{ "ArcTan",                  0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "ArcSin",                  0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT },{} },
	{ "ArcCos",                  0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT },{} },
	{ "RadianSin",               0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT },{} },
	{ "RadianCos",               0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT },{} },
	{ "RadianTan",               0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT },{} },
	{ "Pow",                     0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "LPow",                    0,          ZTID_LONG,   -1,          0,  { ZTID_LONG, ZTID_LONG },{} },
	{ "InvPow",                  0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "Factorial",               0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT },{} },
	{ "Abs",                     0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT },{} },
	{ "Log10",                   0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT },{} },
	{ "Ln",                      0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT },{} },
	{ "Sqrt",                    0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT },{} },
	{ "CopyTile",                0,          ZTID_VOID,   -1,          0,  { ZTID_FLOAT, ZTID_FLOAT },{} },

	{ "SwapTile",                0,          ZTID_VOID,   -1,          0,  { ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "ClearTile",               0,          ZTID_VOID,   -1,          0,  { ZTID_FLOAT },{} },
	{ "GetScriptRAM",            0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT },{} },
	{ "SetScriptRAM",            0,          ZTID_VOID,   -1,          0,  { ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetGlobalRAM",            0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT },{} },
	{ "SetGlobalRAM",            0,          ZTID_VOID,   -1,          0,  { ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetColorBuffer",          0,          ZTID_VOID,   -1,          0,  { ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SetDepthBuffer",          0,          ZTID_VOID,   -1,          0,  { ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetColorBuffer",          0,          ZTID_VOID,   -1,          0,  { ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetDepthBuffer",          0,          ZTID_VOID,   -1,          0,  { ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "SizeOfArray",             0,         ZTID_FLOAT,   -1,          0,  { ZTID_UNTYPED },{} },
	{ "ResizeArray",             0,          ZTID_VOID,   -1,          0,  { ZTID_UNTYPED, ZTID_FLOAT },{} },
	{ "OwnArray",                0,          ZTID_VOID,   -1,          0,  { ZTID_UNTYPED },{} },
	{ "DestroyArray",            0,          ZTID_VOID,   -1,          0,  { ZTID_UNTYPED },{} },
	{ "OwnObject",               0,          ZTID_VOID,   -1,          0,  { ZTID_UNTYPED },{} },
	{ "GlobalObject",            0,          ZTID_VOID,   -1,          0,  { ZTID_UNTYPED },{} },
	{ "OverlayTile",             0,          ZTID_VOID,   -1,          0,  { ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "Untype",                  0,       ZTID_UNTYPED,   -1,          0,  { ZTID_UNTYPED },{} },
	{ "Byte",                    0,       ZTID_UNTYPED,   -1,          0,  { ZTID_UNTYPED },{} },
	{ "Int8",                    0,       ZTID_UNTYPED,   -1,          0,  { ZTID_UNTYPED },{} },
	{ "SignedByte",              0,       ZTID_UNTYPED,   -1,          0,  { ZTID_UNTYPED },{} },
	{ "Word",                    0,       ZTID_UNTYPED,   -1,          0,  { ZTID_UNTYPED },{} },
	{ "Int16",                   0,       ZTID_UNTYPED,   -1,          0,  { ZTID_UNTYPED },{} },
	{ "Short",                   0,       ZTID_UNTYPED,   -1,          0,  { ZTID_UNTYPED },{} },
	{ "Integer",                 0,       ZTID_UNTYPED,   -1,          0,  { ZTID_UNTYPED },{} },
	{ "Floor",                   0,       ZTID_UNTYPED,   -1,          0,  { ZTID_UNTYPED },{} },
	{ "Ceiling",                 0,       ZTID_UNTYPED,   -1,          0,  { ZTID_UNTYPED },{} },
	{ "GetSystemTime",           0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT },{} },

	{ "Distance",                0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "Distance",                1,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "LongDistance",            0,          ZTID_LONG,   -1,          0,  { ZTID_LONG, ZTID_LONG, ZTID_LONG, ZTID_LONG },{} },
	{ "LongDistance",            1,          ZTID_LONG,   -1,          0,  { ZTID_LONG, ZTID_LONG, ZTID_LONG, ZTID_LONG, ZTID_LONG },{} },
	{ "strcmp",                  0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "strncmp",                 0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "stricmp",                 0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "strnicmp",                0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "strcpy",                  0,          ZTID_VOID,   -1,          0,  { ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "itoacat",                 0,          ZTID_VOID,   -1,          0,  { ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "ArrayCopy",               0,          ZTID_VOID,   -1,          0,  { ZTID_UNTYPED, ZTID_UNTYPED },{} },
	{ "strlen",                  0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT },{} },
	{ "atoi",                    0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT },{} },
	{ "ilen",                    0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT },{} },
	{ "utol",                    0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT },{} },
	{ "ltou",                    0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT },{} },
	{ "convcase",                0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT },{} },
	{ "itoa",                    0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "xtoa",                    0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "xtoi",                    0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT },{} },
	{ "SaveSRAM",                0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "LoadSRAM",                0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT, ZTID_FLOAT },{} },

	{ "strcat",                  0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "strchr",                  0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "strcspn",                 0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "strspn",                  0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "strstr",                  0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "strrchr",                 0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT, ZTID_FLOAT },{} },

	{ "SRand",                   0,          ZTID_VOID,   -1,          0,  { ZTID_LONG },{} },
	{ "SRand",                   1,          ZTID_LONG,   -1,          0,  {},{} },

	{ "IsValidArray",            0,          ZTID_BOOL,   -1,          0,  { ZTID_UNTYPED },{} },

	{ "DegtoRad",                0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT },{} },
	{ "RadtoDeg",                0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT },{} },

	{ "printf",                  0,          ZTID_VOID,   -1,    FL_VARG,  { ZTID_CHAR },{} },
	{ "sprintf",                 0,         ZTID_FLOAT,   -1,    FL_VARG,  { ZTID_CHAR, ZTID_CHAR },{} },
	
	{ "Max",                     0,       ZTID_UNTYPED,   -1,    FL_VARG,  { ZTID_UNTYPED, ZTID_UNTYPED },{} },
	{ "Min",                     0,       ZTID_UNTYPED,   -1,    FL_VARG,  { ZTID_UNTYPED, ZTID_UNTYPED },{} },
	{ "Choose",                  0,       ZTID_UNTYPED,   -1,    FL_VARG,  { ZTID_UNTYPED },{} },
	
	//Undocumented intentionally - compat only
	{ "Rand",                    0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT },{} },
	{ "EngineDegtoRad",          0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT },{} },
	{ "EngineRadtoDeg",          0,         ZTID_FLOAT,   -1,          0,  { ZTID_FLOAT },{} },
	{ "TraceLWeapon",            0,          ZTID_VOID,   -1,          0,  { ZTID_LWPN },{} },
	{ "TraceEWeapon",            0,          ZTID_VOID,   -1,          0,  { ZTID_EWPN },{} },
	{ "TraceNPC",                0,          ZTID_VOID,   -1,          0,  { ZTID_NPC },{} },
	{ "TraceFFC",                0,          ZTID_VOID,   -1,          0,  { ZTID_FFC },{} },
	{ "TraceItem",               0,          ZTID_VOID,   -1,          0,  { ZTID_ITEM },{} },
	{ "TraceItemData",           0,          ZTID_VOID,   -1,          0,  { ZTID_ITEMCLASS },{} },
	{ "SizeOfArrayBool",         0,         ZTID_FLOAT,   -1,          0,  { ZTID_BOOL },{} },
	{ "SizeOfArrayFFC",          0,         ZTID_FLOAT,   -1,          0,  { ZTID_FFC },{} },
	{ "SizeOfArrayItem",         0,         ZTID_FLOAT,   -1,          0,  { ZTID_ITEM },{} },
	{ "SizeOfArrayLWeapon",      0,         ZTID_FLOAT,   -1,          0,  { ZTID_LWPN },{} },
	{ "SizeOfArrayEWeapon",      0,         ZTID_FLOAT,   -1,          0,  { ZTID_EWPN },{} },
	{ "SizeOfArrayNPC",          0,         ZTID_FLOAT,   -1,          0,  { ZTID_NPC },{} },
	{ "SizeOfArrayItemdata",     0,         ZTID_FLOAT,   -1,          0,  { ZTID_ITEMCLASS },{} },
	
	{ "",                        0,          ZTID_VOID,   -1,          0,  {},{} }
	//Unused old junk
	//overload, 2 args
//	{ "atoi",                       0,         ZTID_FLOAT,   -1,                        0,  { ZTID_FLOAT, ZTID_FLOAT },{} },
//	{ "xtoi",                       0,         ZTID_FLOAT,   -1,                        0,  { ZTID_FLOAT },{} },
	//overload, 2 args
//	{ "xtoi",                       1,         ZTID_FLOAT,   -1,                        0,  { ZTID_FLOAT, ZTID_FLOAT },{} },
//	{ "xlen",                       0,         ZTID_FLOAT,   -1,                        0,  { ZTID_FLOAT },{} },
	//overload, 2 args
//	{ "xlen",                       1,         ZTID_FLOAT,   -1,                        0,  { ZTID_FLOAT, ZTID_FLOAT },{} },
//	{ "xtoa",                       0,         ZTID_FLOAT,   -1,                        0,  { ZTID_FLOAT, ZTID_FLOAT },{} },
	//overload, 2 args
//	{ "ilen",                       0,         ZTID_FLOAT,   -1,                        0,  { ZTID_FLOAT, ZTID_FLOAT },{} },
//	{ "CalculateSpline",            0,         ZTID_FLOAT,   -1,                        0,  { ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
//	{ "CollisionRect",              0,          ZTID_BOOL,   -1,                        0,  { ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
//	{ "CollisionBox",               0,          ZTID_BOOL,   -1,                        0,  { ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },

};

GlobalSymbols::GlobalSymbols()
{
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
		Function* function = getFunction2("NULL");
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
		Function* function = getFunction2("Untype");
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
		Function* function = getFunction2("Distance",0);
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
		Function* function = getFunction2("Distance", 1);
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
		Function* function = getFunction2("LongDistance");
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
		Function* function = getFunction2("LongDistance", 1);
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
		Function* function = getFunction2("Rand");
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
		Function* function = getFunction2("SRand");
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
		Function* function = getFunction2("SRand", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OSRandRand(new VarArgument(EXP1)));
		LABELBACK(label);
		RETURN();
		function->giveCode(code);
	}

	//bool IsValidArray(untyped)
	{
		Function* function = getFunction2("IsValidArray");
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
		Function* function = getFunction2("GetSystemTime");
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
		Function* function = getFunction2("Quit");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OQuit());
		LABELBACK(label);
		function->giveCode(code);
	}
	//void Waitframe()
	{
		Function* function = getFunction2("Waitframe");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OWaitframe());
		LABELBACK(label);
		RETURN();
		function->giveCode(code);
	}
	//void Waitdraw()
	{
		Function* function = getFunction2("Waitdraw");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OWaitdraw());
		LABELBACK(label);
		RETURN();
		function->giveCode(code);
	}
	//void WaitTo(int32_t, bool)
	{
		Function* function = getFunction2("WaitTo");
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
		Function* function = getFunction2("WaitEvent");
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
		Function* function = getFunction2("TraceLWeapon");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new OTraceRegister(new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction2("TraceEWeapon");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new OTraceRegister(new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction2("TraceNPC");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new OTraceRegister(new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction2("TraceFFC");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new OTraceRegister(new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction2("TraceItem");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new OTraceRegister(new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction2("TraceItemData");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new OTraceRegister(new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//void TraceB(bool val)
	{
		Function* function = getFunction2("TraceB");
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
		Function* function = getFunction2("TraceS");
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
		Function* function = getFunction2("TraceNL");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OTrace3());
		LABELBACK(label);
		RETURN();
		function->giveCode(code);
	}
	//void ClearTrace()
	{
		Function* function = getFunction2("ClearTrace");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OTrace4());
		LABELBACK(label);
		RETURN();
		function->giveCode(code);
	}
	//void TraceToBase(float, float, float)
	{
		Function* function = getFunction2("TraceToBase");
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
		Function* function = getFunction2("Sin");
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
		Function* function = getFunction2("RadianSin");
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
		Function* function = getFunction2("ArcSin");
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
		Function* function = getFunction2("Cos");
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
		Function* function = getFunction2("RadianCos");
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
		Function* function = getFunction2("ArcCos");
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
		Function* function = getFunction2("Tan");
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
		Function* function = getFunction2("ArcTan");
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
		Function* function = getFunction2("RadianTan");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new OMultImmediate(new VarArgument(EXP2), new LiteralArgument(radsperdeg)));
		addOpcode2 (code, new OTanRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t Pow(int32_t first, int32_t second)
	{
		Function* function = getFunction2("Pow");
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
		Function* function = getFunction2("LPow");
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
		Function* function = getFunction2("InvPow");
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
		Function* function = getFunction2("Factorial");
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
		Function* function = getFunction2("Abs");
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
		Function* function = getFunction2("Log10");
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
		Function* function = getFunction2("Ln");
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
		Function* function = getFunction2("Sqrt");
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
		Function* function = getFunction2("CopyTile");
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
		Function* function = getFunction2("SwapTile");
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
		Function* function = getFunction2("OverlayTile");
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
		Function* function = getFunction2("ClearTile");
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
		Function* function = getFunction2("GetGlobalRAM");
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
		Function* function = getFunction2("SetGlobalRAM");
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
		Function* function = getFunction2("GetScriptRAM");
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
		Function* function = getFunction2("SetScriptRAM");
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
		Function* function = getFunction2("SetColorBuffer");
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
		Function* function = getFunction2("SetDepthBuffer");
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
		Function* function = getFunction2("GetColorBuffer");
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
		Function* function = getFunction2("GetDepthBuffer");
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
		Function* function = getFunction2("SizeOfArray");
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
		Function* function = getFunction2("ResizeArray");
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
		Function* function = getFunction2("OwnArray");
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
		Function* function = getFunction2("OwnObject");
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
		Function* function = getFunction2("GlobalObject");
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
		Function* function = getFunction2("DestroyArray");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new ODestroyArrayRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}

	{
		Function* function = getFunction2("Byte");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OByte(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction2("Int8");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OByte(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction2("SignedByte");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OSByte(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction2("Word");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OWord(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction2("Int16");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OWord(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction2("Short");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OShort(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction2("Integer");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OToInteger(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction2("Floor");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OFloor(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction2("Ceiling");
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
		Function* function = getFunction2("SizeOfArrayFFC");
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
		Function* function = getFunction2("SizeOfArrayNPC");
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
		Function* function = getFunction2("SizeOfArrayBool");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OArraySizeB(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	} //int32_t SizeOfArrayItem(item *ptr)
	{
		Function* function = getFunction2("SizeOfArrayItem");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OArraySizeI(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	} //int32_t SizeOfArrayItemdata(itemdata *ptr)
	{
		Function* function = getFunction2("SizeOfArrayItemdata");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OArraySizeID(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	} //int32_t SizeOfArrayLWeapon(lweapon *ptr)
	{
		Function* function = getFunction2("SizeOfArrayLWeapon");
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
		Function* function = getFunction2("SaveSRAM");
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
		Function* function = getFunction2("LoadSRAM");
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
		Function* function = getFunction2("SizeOfArrayEWeapon");
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
		Function* function = getFunction2("strlen");
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
		Function* function = getFunction2("strcpy");
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
		Function* function = getFunction2("itoacat");
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
		Function* function = getFunction2("strcmp");
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
		Function* function = getFunction2("strncmp");
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
		Function* function = getFunction2("stricmp");
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
		Function* function = getFunction2("strnicmp");
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
		Function* function = getFunction2("ArrayCopy");
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
		Function* function = getFunction2("atoi");
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
		Function* function = getFunction2("atoi");
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
		Function* function = getFunction2("ilen");
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
		Function* function = getFunction2("utol");
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
		Function* function = getFunction2("ltou");
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
		Function* function = getFunction2("convcase");
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
		Function* function = getFunction2("ilen");
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
		Function* function = getFunction2("itoa");
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
		Function* function = getFunction2("xtoa");
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
		Function* function = getFunction2("xtoi");
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
		Function* function = getFunction2("remchr");
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
		Function* function = getFunction2("strcat");
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
		Function* function = getFunction2("strchr");
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
		Function* function = getFunction2("strcspn");
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
		Function* function = getFunction2("strspn");
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
		Function* function = getFunction2("strstr");
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
		Function* function = getFunction2("strrchr");
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
		Function* function = getFunction2("xlen");
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
		Function* function = getFunction2("xlen");
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
		Function* function = getFunction2("xtoa");
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
		Function* function = getFunction2("xtoi");
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
		Function* function = getFunction2("xtoi");
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
	{
		Function* function = getFunction2("printf");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPrintfImmediate(new VargsArgument(-10000)));
		LABELBACK(label);
		POP_VARGS(1,NUL);
		RETURN();
		function->giveCode(code);
	}
	//void sprintf(str* buf, str* format, untyped args...)
	{
		Function* function = getFunction2("sprintf");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OSPrintfImmediate(new VargsArgument(-10000)));
		LABELBACK(label);
		POP_VARGS(2,NUL);
		RETURN();
		function->giveCode(code);
	}
	//int32_t EngineDegtoRad(int32_t val)
	{
		Function* function = getFunction2("EngineDegtoRad");
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
		Function* function = getFunction2("EngineRadtoDeg");
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
		Function* function = getFunction2("DegtoRad");
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
		Function* function = getFunction2("RadtoDeg");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new OEngineRadtoDeg(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}

	//untyped Max(untyped first, untyped second, untyped ..args)
	{
		Function* function = getFunction2("Max");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OMaxNew(new VargsArgument(-1,2)));
		POP_VARGS(2,NUL);
		LABELBACK(label);
		RETURN();
		function->giveCode(code);
	}
	//untyped Min(untyped first, untyped second, untyped ..args)
	{
		Function* function = getFunction2("Min");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OMinNew(new VargsArgument(-1,2)));
		POP_VARGS(2,NUL);
		LABELBACK(label);
		RETURN();
		function->giveCode(code);
	}
	//untyped Choose(untyped first, untyped ..args)
	{
		Function* function = getFunction2("Choose");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OChoose(new VargsArgument(-1,1)));
		POP_VARGS(1,NUL);
		LABELBACK(label);
		RETURN();
		function->giveCode(code);
	}
	
}

