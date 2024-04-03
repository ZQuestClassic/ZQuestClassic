#include "SymbolDefs.h"

GraphicsSymbols GraphicsSymbols::singleton = GraphicsSymbols();

static AccessorTable GraphicsTable[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
	{ "Wavy",                       0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GRAPHICS, ZTID_BOOL },{} },
	{ "Zap",                        0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GRAPHICS, ZTID_BOOL },{} },
	{ "Greyscale",                  0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GRAPHICS, ZTID_BOOL },{} },
	{ "Monochrome",                 0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GRAPHICS, ZTID_FLOAT },{} },
	{ "Tint",                       0,          ZTID_VOID,   -1,                        0,  { ZTID_GRAPHICS, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "MonochromeHue",              0,          ZTID_VOID,   -1,                        0,  { ZTID_GRAPHICS, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL },{ 10000 } },
	{ "ClearTint",                  0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GRAPHICS },{} },
	{ "getIsBlankTile[]",           0,         ZTID_FLOAT,   ISBLANKTILE,               0,  { ZTID_GRAPHICS, ZTID_FLOAT },{} },
	{ "setIsBlankTile[]",           0,          ZTID_VOID,   ISBLANKTILE,       FL_RDONLY,  { ZTID_GRAPHICS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "_getNumDraws",               0,         ZTID_FLOAT,   NUMDRAWS,                  0,  { ZTID_GRAPHICS },{} },
	{ "_getMaxDraws",               0,         ZTID_FLOAT,   MAXDRAWS,                  0,  { ZTID_GRAPHICS },{} },
	{ "GetPixel",                   0,         ZTID_FLOAT,   -1,                        0,  { ZTID_GRAPHICS, ZTID_UNTYPED, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "CreatePalData",              0,       ZTID_PALDATA,   -1,                   FL_INL,  { ZTID_GRAPHICS },{} },
	{ "CreatePalData",              1,       ZTID_PALDATA,   -1,                   FL_INL,  { ZTID_GRAPHICS, ZTID_RGBDATA },{} },
	{ "MixColor",                   0,       ZTID_RGBDATA,   -1,                   FL_INL,  { ZTID_GRAPHICS, ZTID_RGBDATA, ZTID_RGBDATA, ZTID_FLOAT, ZTID_FLOAT },{ 0 } },
	{ "CreateRGB",                  0,       ZTID_RGBDATA,   -1,                   FL_INL,  { ZTID_GRAPHICS, ZTID_LONG },{} },
	{ "CreateRGB",                  1,       ZTID_RGBDATA,   -1,                   FL_INL,  { ZTID_GRAPHICS, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "ConvertFromRGB",             0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GRAPHICS, ZTID_FLOAT, ZTID_RGBDATA, ZTID_FLOAT },{} },
	{ "ConvertToRGB",               0,       ZTID_RGBDATA,   -1,                   FL_INL,  { ZTID_GRAPHICS, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetTilePixel",               0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_GRAPHICS, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{ 0 } },
	{ "SetTilePixel",               0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_GRAPHICS, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getIs8BitTile[]",           0,          ZTID_BOOL,   IS8BITTILE,               0,  { ZTID_GRAPHICS, ZTID_FLOAT },{} },
	{ "setIs8BitTile[]",           0,          ZTID_VOID,   IS8BITTILE,       FL_RDONLY,  { ZTID_GRAPHICS, ZTID_FLOAT, ZTID_BOOL },{} },

	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
};

GraphicsSymbols::GraphicsSymbols()
{
	table = GraphicsTable;
	refVar = NUL;
}

void GraphicsSymbols::generateCode()
{
	{
		Function* function = getFunction("Wavy");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		// Pop argument.
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		// Pop pointer.
		POPREF();
		addOpcode2 (code, new OWavyR(new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	
	//int32_t GetPixel(graphics,bitmap,int32_t,int32_t)
	{
		Function* function = getFunction("GetPixel");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OGraphicsGetpixel(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("Zap");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		// Pop argument.
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		// Pop pointer.
		POPREF();
		addOpcode2 (code, new OZapR(new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("Greyscale");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		// Pop argument.
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		// Pop pointer.
		POPREF();
		addOpcode2 (code, new OGreyscaleR(new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("Monochrome");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		// Pop argument.
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		// Pop pointer.
		POPREF();
		addOpcode2 (code, new OMonochromeR(new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}

	//void Tint(graphics, float, float, float)
	{
			Function* function = getFunction("Tint");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OTintR());
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);
	}
	//void MonochromeHue(graphics, float, float, float, bool)
	{
			Function* function = getFunction("MonochromeHue");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OMonoHueR());
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
	
	//void ClearTint()
	{
		 Function* function = getFunction("ClearTint");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OClearTint());
		LABELBACK(label);
		POPREF(); //pop the 'this'
		RETURN();
		function->giveCode(code);
	}

	//paldata CreatePalData(graphics)
	{
		Function* function = getFunction("CreatePalData");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		POPREF();
		addOpcode2(code, new OCreatePalData());
		LABELBACK(label);
		RETURN();
		function->giveCode(code);
	}
	//paldata CreatePalData(graphics, rgb)
	{
		Function* function = getFunction("CreatePalData", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2(code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2(code, new OCreatePalDataClr(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//rgb MixColor(graphics, rgb, rgb, float, float)
	{
		Function* function = getFunction("MixColor");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2(code, new OMixColorArray());
		LABELBACK(label);
		POP_ARGS(4, NUL);
		//pop pointer, and ignore it
		POPREF();
		RETURN();
		function->giveCode(code);
	}
	//rgb CreateRGB(graphics, float)
	{
		Function* function = getFunction("CreateRGB");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2(code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2(code, new OCreateRGBHex(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//rgb CreateRGB(graphics, float, float, float)
	{
		Function* function = getFunction("CreateRGB", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2(code, new OCreateRGB());
		LABELBACK(label);
		POP_ARGS(3, NUL);
		//pop pointer, and ignore it
		POPREF();
		RETURN();
		function->giveCode(code);
	}
	//void ConvertFromRGB(graphics, float[], rgb, float)
	{
		Function* function = getFunction("ConvertFromRGB", 0);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2(code, new OConvertFromRGB());
		LABELBACK(label);
		POP_ARGS(3, NUL);
		//pop pointer, and ignore it
		POPREF();
		RETURN();
		function->giveCode(code);
	}
	//void ConvertToRGB(graphics, float[], rgb, float)
	{
		Function* function = getFunction("ConvertToRGB", 0);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2(code, new OConvertToRGB());
		LABELBACK(label);
		POP_ARGS(2, NUL);
		//pop pointer, and ignore it
		POPREF();
		RETURN();
		function->giveCode(code);
	}
	//void GetTilePixel(graphics, float, float, float, float)
	{
		Function* function = getFunction("GetTilePixel");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2(code, new OGetTilePixel());
		LABELBACK(label);
		POP_ARGS(4, NUL);
		//pop pointer, and ignore it
		POPREF();
		RETURN();
		function->giveCode(code);
	}
	//void SetTilePixel(graphics, float, float, float, float)
	{
		Function* function = getFunction("SetTilePixel");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2(code, new OSetTilePixel());
		LABELBACK(label);
		POP_ARGS(4, NUL);
		//pop pointer, and ignore it
		POPREF();
		RETURN();
		function->giveCode(code);
	}
}

