#include "SymbolDefs.h"

PalDataSymbols PalDataSymbols::singleton = PalDataSymbols();

static AccessorTable PalDataTable[] =
{
	//	  name,                     rettype,                     setorget,     var,                numindex,   funcFlags,                            numParams,   params
	//	{ "DirExists",              ZTID_BOOL,             FUNCTION,     0,                  1,          0,                                    2,           { ZTID_FILESYSTEM, ZTID_CHAR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
		{ "LoadLevelPalette",       ZTID_VOID,             FUNCTION,     0,                  1,          FUNCFLAG_INLINE,                      2,           { ZTID_PALDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
		{ "LoadSpritePalette",      ZTID_VOID,             FUNCTION,     0,                  1,          FUNCFLAG_INLINE,                      2,           { ZTID_PALDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
		{ "LoadMainPalette",        ZTID_VOID,             FUNCTION,     0,                  1,          FUNCFLAG_INLINE,                      1,           { ZTID_PALDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
		{ "LoadCyclePalette",       ZTID_VOID,             FUNCTION,     0,                  1,          FUNCFLAG_INLINE,                      2,           { ZTID_PALDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
		{ "LoadBitmapPalette",      ZTID_VOID,             FUNCTION,     0,                  1,          FUNCFLAG_INLINE,                      2,           { ZTID_PALDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
		{ "WriteLevelPalette",      ZTID_VOID,             FUNCTION,     0,                  1,          FUNCFLAG_INLINE,                      2,           { ZTID_PALDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
		{ "WriteLevelCSet",         ZTID_VOID,             FUNCTION,     0,                  1,          FUNCFLAG_INLINE,                      3,           { ZTID_PALDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
		{ "WriteSpritePalette",     ZTID_VOID,             FUNCTION,     0,                  1,          FUNCFLAG_INLINE,                      2,           { ZTID_PALDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
		{ "WriteSpriteCSet",        ZTID_VOID,             FUNCTION,     0,                  1,          FUNCFLAG_INLINE,                      3,           { ZTID_PALDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
		{ "WriteMainPalette",       ZTID_VOID,             FUNCTION,     0,                  1,          FUNCFLAG_INLINE,                      1,           { ZTID_PALDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
		{ "WriteMainCSet",          ZTID_VOID,             FUNCTION,     0,                  1,          FUNCFLAG_INLINE,                      2,           { ZTID_PALDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
		{ "WriteCyclePalette",      ZTID_VOID,             FUNCTION,     0,                  1,          FUNCFLAG_INLINE,                      2,           { ZTID_PALDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
		{ "WriteCycleCSet",         ZTID_VOID,             FUNCTION,     0,                  1,          FUNCFLAG_INLINE,                      3,           { ZTID_PALDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
		{ "ColorValid",             ZTID_BOOL,             FUNCTION,     0,                  1,          FUNCFLAG_INLINE,                      2,           { ZTID_PALDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
		{ "ClearColor",             ZTID_VOID,             FUNCTION,     0,                  1,          FUNCFLAG_INLINE,                      2,           { ZTID_PALDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
		{ "ClearCSet",              ZTID_VOID,             FUNCTION,     0,                  1,          FUNCFLAG_INLINE,                      2,           { ZTID_PALDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
		{ "Mix",                    ZTID_VOID,             FUNCTION,     0,                  1,          FUNCFLAG_INLINE,                      5,           { ZTID_PALDATA, ZTID_PALDATA, ZTID_PALDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
		{ "MixCSet",                ZTID_VOID,             FUNCTION,     0,                  1,          FUNCFLAG_INLINE,                      6,           { ZTID_PALDATA, ZTID_PALDATA, ZTID_PALDATA, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
		{ "Copy",                   ZTID_VOID,             FUNCTION,     0,                  1,          FUNCFLAG_INLINE,                      2,           { ZTID_PALDATA, ZTID_PALDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
		{ "CopyCSet",               ZTID_VOID,             FUNCTION,     0,                  1,          FUNCFLAG_INLINE,                      4,           { ZTID_PALDATA, ZTID_PALDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
		{ "Free",                   ZTID_VOID,             FUNCTION,     0,                  1,          FUNCFLAG_INLINE,                      1,           { ZTID_PALDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
		{ "Own",                    ZTID_VOID,             FUNCTION,     0,                  1,          FUNCFLAG_INLINE,                      1,           { ZTID_PALDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
		{ "getColor[]",             ZTID_RGBDATA,          GETTER,       PALDATACOLOR,     256,          FUNCFLAG_INLINE,                      2,           { ZTID_PALDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
		{ "setColor[]",             ZTID_VOID,             SETTER,       PALDATACOLOR,     256,          FUNCFLAG_INLINE,                      3,           { ZTID_PALDATA, ZTID_FLOAT, ZTID_RGBDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
		{ "getR[]",                 ZTID_FLOAT,            GETTER,       PALDATAR,         256,          FUNCFLAG_INLINE,                      2,           { ZTID_PALDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
		{ "setR[]",                 ZTID_VOID,             SETTER,       PALDATAR,         256,          FUNCFLAG_INLINE,                      3,           { ZTID_PALDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
		{ "getG[]",                 ZTID_FLOAT,            GETTER,       PALDATAG,         256,          FUNCFLAG_INLINE,                      2,           { ZTID_PALDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
		{ "setG[]",                 ZTID_VOID,             SETTER,       PALDATAG,         256,          FUNCFLAG_INLINE,                      3,           { ZTID_PALDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
		{ "getB[]",                 ZTID_FLOAT,            GETTER,       PALDATAB,         256,          FUNCFLAG_INLINE,                      2,           { ZTID_PALDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
		{ "setB[]",                 ZTID_VOID,             SETTER,       PALDATAB,         256,          FUNCFLAG_INLINE,                      3,           { ZTID_PALDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },

		{ "",                       -1,                       -1,           -1,               -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

PalDataSymbols::PalDataSymbols()
{
	table = PalDataTable;
	refVar = REFPALDATA;
}

void PalDataSymbols::generateCode()
{
	//void LoadLevelPalette(paldata, int32_t)
	{
		Function* function = getFunction("LoadLevelPalette", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new OLoadLevelPalette(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void LoadSpritePalette(paldata, int32_t)
	{
		Function* function = getFunction("LoadSpritePalette", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2(code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2(code, new OLoadSpritePalette(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void LoadMainPalette(paldata)
	{
		Function* function = getFunction("LoadMainPalette", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		POPREF();
		addOpcode2(code, new OLoadMainPalette());
		LABELBACK(label);
		RETURN();
		function->giveCode(code);
	}
	//void LoadCyclePalette(paldata, int32_t)
	{
		Function* function = getFunction("LoadCyclePalette", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2(code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2(code, new OLoadCyclePalette(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void LoadBitmapPalette(paldata, int32_t)
	{
		Function* function = getFunction("LoadBitmapPalette", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2(code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2(code, new OLoadBitmapPalette(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void WriteLevelPalette(paldata, int32_t)
	{
		Function* function = getFunction("WriteLevelPalette", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2(code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2(code, new OWriteLevelPalette(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void WriteLevelCSet(paldata, int32_t, int32_t)
	{
		Function* function = getFunction("WriteLevelCSet", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2(code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2(code, new OPopRegister(new VarArgument(EXP1)));
		//pop pointer
		POPREF();
		addOpcode2(code, new OWriteLevelCSet(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//void WriteSpritePalette(paldata, int32_t)
	{
		Function* function = getFunction("WriteSpritePalette", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2(code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2(code, new OWriteSpritePalette(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void WriteSpriteCSet(paldata, int32_t, int32_t)
	{
		Function* function = getFunction("WriteSpriteCSet", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2(code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2(code, new OPopRegister(new VarArgument(EXP1)));
		//pop pointer
		POPREF();
		addOpcode2(code, new OWriteSpriteCSet(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//void WriteMainPalette(paldata)
	{
		Function* function = getFunction("WriteMainPalette", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		POPREF();
		addOpcode2(code, new OWriteMainPalette());
		LABELBACK(label);
		RETURN();
		function->giveCode(code);
	}
	//void WriteMainCSet(paldata, int32_t)
	{
		Function* function = getFunction("WriteMainCSet", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2(code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2(code, new OWriteMainCSet(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void WriteCyclePalette(paldata, int32_t)
	{
		Function* function = getFunction("WriteCyclePalette", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2(code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2(code, new OWriteCyclePalette(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void WriteCycleCSet(paldata, int32_t, int32_t)
	{
		Function* function = getFunction("WriteCycleCSet", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2(code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2(code, new OPopRegister(new VarArgument(EXP1)));
		//pop pointer
		POPREF();
		addOpcode2(code, new OWriteCycleCSet(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//bool ColorValid(paldata, int32_t)
	{
		Function* function = getFunction("ColorValid", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2(code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2(code, new OPalDataColorValid(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void ClearColor(paldata, int32_t)
	{
		Function* function = getFunction("ClearColor", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2(code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2(code, new OPalDataClearColor(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void ClearCSet(paldata, int32_t)
	{
		Function* function = getFunction("ClearCSet", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2(code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2(code, new OPalDataClearCSet(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void Mix(paldata, paldata, paldata, int32_t, int32_t)
	{
		Function* function = getFunction("Mix", 5);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2(code, new OPalDataMix());
		LABELBACK(label);
		POP_ARGS(4, NUL);
		//pop pointer
		POPREF();
		RETURN();
		function->giveCode(code);
	}
	//void MixCSet(paldata, paldata, paldata, int32_t, int32_t, ints32_t)
	{
		Function* function = getFunction("MixCSet", 6);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2(code, new OPalDataMixCSet());
		LABELBACK(label);
		POP_ARGS(5, NUL);
		//pop pointer
		POPREF();
		RETURN();
		function->giveCode(code);
	}
	
	//void Copy(paldata, paldata)
	{
		Function* function = getFunction("Copy", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2(code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2(code, new OPalDataCopy(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void CopyCSet(paldata, paldata, int32_t, int32_t)
	{
		Function* function = getFunction("CopyCSet", 4);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2(code, new OPalDataCopyCSet());
		LABELBACK(label);
		POP_ARGS(3, NUL);
		//pop pointer
		POPREF();
		RETURN();
		function->giveCode(code);
	}
	//void Free()
	{
		Function* function = getFunction("Free", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2(code, new OPalDataFree());
		RETURN();
		function->giveCode(code);
	}
	//void Own()
	{
		Function* function = getFunction("Own", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2(code, new OPalDataOwn());
		RETURN();
		function->giveCode(code);
	}
}

