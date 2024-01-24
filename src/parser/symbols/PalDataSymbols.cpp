#include "SymbolDefs.h"

PalDataSymbols PalDataSymbols::singleton = PalDataSymbols();

static AccessorTable PalDataTable[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
//	{ "DirExists",                  0,          ZTID_BOOL,   -1,                        0,  { ZTID_FILESYSTEM, ZTID_CHAR },{} },
	{ "LoadLevelPalette",           0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PALDATA, ZTID_FLOAT },{} },
	{ "LoadSpritePalette",          0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PALDATA, ZTID_FLOAT },{} },
	{ "LoadMainPalette",            0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PALDATA },{} },
	{ "LoadCyclePalette",           0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PALDATA, ZTID_FLOAT },{} },
	{ "LoadBitmapPalette",          0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PALDATA, ZTID_FLOAT },{} },
	{ "WriteLevelPalette",          0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PALDATA, ZTID_FLOAT },{} },
	{ "WriteLevelCSet",             0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PALDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "WriteSpritePalette",         0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PALDATA, ZTID_FLOAT },{} },
	{ "WriteSpriteCSet",            0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PALDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "WriteMainPalette",           0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PALDATA },{} },
	{ "WriteMainCSet",              0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PALDATA, ZTID_FLOAT },{} },
	{ "WriteCyclePalette",          0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PALDATA, ZTID_FLOAT },{} },
	{ "WriteCycleCSet",             0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PALDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "ColorValid",                 0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_PALDATA, ZTID_FLOAT },{} },
	{ "ClearColor",                 0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PALDATA, ZTID_FLOAT },{} },
	{ "ClearCSet",                  0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PALDATA, ZTID_FLOAT },{} },
	{ "Mix",                        0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PALDATA, ZTID_PALDATA, ZTID_PALDATA, ZTID_FLOAT, ZTID_FLOAT },{ 5000, 0 } },
	{ "MixCSet",                    0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PALDATA, ZTID_PALDATA, ZTID_PALDATA, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{ 5000, 0 } },
	{ "Copy",                       0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PALDATA, ZTID_PALDATA },{} },
	{ "CopyCSet",                   0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PALDATA, ZTID_PALDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "Free",                       0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PALDATA },{} },
	{ "Own",                        0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_PALDATA },{} },
	{ "getColor[]",                 0,       ZTID_RGBDATA,   PALDATACOLOR,         FL_INL,  { ZTID_PALDATA, ZTID_FLOAT },{} },
	{ "setColor[]",                 0,          ZTID_VOID,   PALDATACOLOR,         FL_INL,  { ZTID_PALDATA, ZTID_FLOAT, ZTID_RGBDATA },{} },
	{ "getR[]",                     0,         ZTID_FLOAT,   PALDATAR,             FL_INL,  { ZTID_PALDATA, ZTID_FLOAT },{} },
	{ "setR[]",                     0,          ZTID_VOID,   PALDATAR,             FL_INL,  { ZTID_PALDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getG[]",                     0,         ZTID_FLOAT,   PALDATAG,             FL_INL,  { ZTID_PALDATA, ZTID_FLOAT },{} },
	{ "setG[]",                     0,          ZTID_VOID,   PALDATAG,             FL_INL,  { ZTID_PALDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getB[]",                     0,         ZTID_FLOAT,   PALDATAB,             FL_INL,  { ZTID_PALDATA, ZTID_FLOAT },{} },
	{ "setB[]",                     0,          ZTID_VOID,   PALDATAB,             FL_INL,  { ZTID_PALDATA, ZTID_FLOAT, ZTID_FLOAT },{} },

	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
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
		Function* function = getFunction("LoadLevelPalette");
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
		Function* function = getFunction("LoadSpritePalette");
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
		Function* function = getFunction("LoadMainPalette");
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
		Function* function = getFunction("LoadCyclePalette");
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
		Function* function = getFunction("LoadBitmapPalette");
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
		Function* function = getFunction("WriteLevelPalette");
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
		Function* function = getFunction("WriteLevelCSet");
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
		Function* function = getFunction("WriteSpritePalette");
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
		Function* function = getFunction("WriteSpriteCSet");
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
		Function* function = getFunction("WriteMainPalette");
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
		Function* function = getFunction("WriteMainCSet");
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
		Function* function = getFunction("WriteCyclePalette");
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
		Function* function = getFunction("WriteCycleCSet");
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
		Function* function = getFunction("ColorValid");
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
		Function* function = getFunction("ClearColor");
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
		Function* function = getFunction("ClearCSet");
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
		Function* function = getFunction("Mix");
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
		Function* function = getFunction("MixCSet");
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
		Function* function = getFunction("Copy");
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
		Function* function = getFunction("CopyCSet");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2(code, new OPalDataCopyCSet());
		LABELBACK(label);
		POP_ARGS(4, NUL);
		RETURN();
		function->giveCode(code);
	}
	//void Free()
	{
		Function* function = getFunction("Free");
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
		Function* function = getFunction("Own");
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

