#include "SymbolDefs.h"

GraphicsSymbols GraphicsSymbols::singleton = GraphicsSymbols();

static AccessorTable GraphicsTable[] =
{
//	  name,                     rettype,            setorget,    var,        num,           funcFlags,                            numParams,   params
	{ "Wavy",                   ZVARTYPEID_VOID,    FUNCTION,    0,          1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GRAPHICS, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Zap",                    ZVARTYPEID_VOID,    FUNCTION,    0,          1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GRAPHICS, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Greyscale",              ZVARTYPEID_VOID,    FUNCTION,    0,          1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GRAPHICS, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Monochrome",             ZVARTYPEID_VOID,    FUNCTION,    0,          1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GRAPHICS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Tint",                   ZVARTYPEID_VOID,    FUNCTION,    0,          1,             0,                                    4,           { ZVARTYPEID_GRAPHICS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MonochromeHue",          ZVARTYPEID_VOID,    FUNCTION,    0,          1,             0,                                    5,           { ZVARTYPEID_GRAPHICS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "ClearTint",              ZVARTYPEID_VOID,    FUNCTION,    0,          1,             FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_GRAPHICS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getIsBlankTile[]",       ZVARTYPEID_FLOAT,   GETTER,      ISBLANKTILE,214500,        0,                                    2,           { ZVARTYPEID_GRAPHICS, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "NumDraws",               ZVARTYPEID_FLOAT,   GETTER,      NUMDRAWS,   1,             0,                                    1,           { ZVARTYPEID_GRAPHICS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MaxDraws",               ZVARTYPEID_FLOAT,   GETTER,      MAXDRAWS,   1,             0,                                    1,           { ZVARTYPEID_GRAPHICS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetPixel",               ZVARTYPEID_FLOAT,   FUNCTION,    0,          1,             0,                                    4,           { ZVARTYPEID_GRAPHICS, ZVARTYPEID_UNTYPED, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "CreatePalData",          ZVARTYPEID_PALDATA, FUNCTION,    0,          1,             FUNCFLAG_INLINE,                      1,           { ZVARTYPEID_GRAPHICS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "CreatePalData",          ZVARTYPEID_PALDATA, FUNCTION,    0,          1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GRAPHICS, ZVARTYPEID_RGBDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MixColor",               ZVARTYPEID_RGBDATA, FUNCTION,    0,          1,             FUNCFLAG_INLINE,                      5,           { ZVARTYPEID_GRAPHICS, ZVARTYPEID_RGBDATA, ZVARTYPEID_RGBDATA, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "CreateRGB",              ZVARTYPEID_RGBDATA, FUNCTION,    0,          1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_GRAPHICS, ZVARTYPEID_LONG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "CreateRGB",              ZVARTYPEID_RGBDATA, FUNCTION,    0,          1,             FUNCFLAG_INLINE,                      4,           { ZVARTYPEID_GRAPHICS, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },

	{ "",                -1,                 -1,          -1,         -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

GraphicsSymbols::GraphicsSymbols()
{
    table = GraphicsTable;
    refVar = NUL;
}

void GraphicsSymbols::generateCode()
{
	{
		Function* function = getFunction("Wavy", 2);
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
		Function* function = getFunction("GetPixel", 4);
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
		Function* function = getFunction("Zap", 2);
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
		Function* function = getFunction("Greyscale", 2);
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
		Function* function = getFunction("Monochrome", 2);
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
		    Function* function = getFunction("Tint", 4);
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
		    Function* function = getFunction("MonochromeHue", 5);
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
		 Function* function = getFunction("ClearTint", 1);
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
		Function* function = getFunction("CreatePalData", 1);
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
		Function* function = getFunction("CreatePalData", 2);
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
		Function* function = getFunction("MixColor", 5);
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
		Function* function = getFunction("CreateRGB", 2);
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
		Function* function = getFunction("CreateRGB", 4);
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
}

