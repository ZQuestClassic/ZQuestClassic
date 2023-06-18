#include "SymbolDefs.h"

FFCSymbols FFCSymbols::singleton = FFCSymbols();

static AccessorTable FFCTable[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
	{ "getData",                    0,         ZTID_FLOAT,   DATA,                      0,  { ZTID_FFC },{} },
	{ "setData",                    0,          ZTID_VOID,   DATA,                      0,  { ZTID_FFC, ZTID_FLOAT },{} },
	{ "getScript",                  0,         ZTID_FLOAT,   FFSCRIPT,                  0,  { ZTID_FFC },{} },
	{ "setScript",                  0,          ZTID_VOID,   FFSCRIPT,                  0,  { ZTID_FFC, ZTID_FLOAT },{} },
	{ "getCSet",                    0,         ZTID_FLOAT,   FCSET,                     0,  { ZTID_FFC },{} },
	{ "setCSet",                    0,          ZTID_VOID,   FCSET,                     0,  { ZTID_FFC, ZTID_FLOAT },{} },
	{ "getDelay",                   0,         ZTID_FLOAT,   DELAY,                     0,  { ZTID_FFC },{} },
	{ "setDelay",                   0,          ZTID_VOID,   DELAY,                     0,  { ZTID_FFC, ZTID_FLOAT },{} },
	{ "getX",                       0,         ZTID_FLOAT,   FX,                        0,  { ZTID_FFC },{} },
	{ "setX",                       0,          ZTID_VOID,   FX,                        0,  { ZTID_FFC, ZTID_FLOAT },{} },
	{ "getY",                       0,         ZTID_FLOAT,   FY,                        0,  { ZTID_FFC },{} },
	{ "setY",                       0,          ZTID_VOID,   FY,                        0,  { ZTID_FFC, ZTID_FLOAT },{} },
	{ "getVx",                      0,         ZTID_FLOAT,   XD,                        0,  { ZTID_FFC },{} },
	{ "setVx",                      0,          ZTID_VOID,   XD,                        0,  { ZTID_FFC, ZTID_FLOAT },{} },
	{ "getVy",                      0,         ZTID_FLOAT,   YD,                        0,  { ZTID_FFC },{} },
	{ "setVy",                      0,          ZTID_VOID,   YD,                        0,  { ZTID_FFC, ZTID_FLOAT },{} },
	{ "getAx",                      0,         ZTID_FLOAT,   XD2,                       0,  { ZTID_FFC },{} },
	{ "setAx",                      0,          ZTID_VOID,   XD2,                       0,  { ZTID_FFC, ZTID_FLOAT },{} },
	{ "getAy",                      0,         ZTID_FLOAT,   YD2,                       0,  { ZTID_FFC },{} },
	{ "setAy",                      0,          ZTID_VOID,   YD2,                       0,  { ZTID_FFC, ZTID_FLOAT },{} },
	{ "getFlags[]",                 0,          ZTID_BOOL,   FFFLAGSD,                  0,  { ZTID_FFC, ZTID_FLOAT },{} },
	{ "setFlags[]",                 0,          ZTID_VOID,   FFFLAGSD,                  0,  { ZTID_FFC, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getTileWidth",               0,         ZTID_FLOAT,   FFTWIDTH,                  0,  { ZTID_FFC },{} },
	{ "setTileWidth",               0,          ZTID_VOID,   FFTWIDTH,                  0,  { ZTID_FFC, ZTID_FLOAT },{} },
	{ "getTileHeight",              0,         ZTID_FLOAT,   FFTHEIGHT,                 0,  { ZTID_FFC },{} },
	{ "setTileHeight",              0,          ZTID_VOID,   FFTHEIGHT,                 0,  { ZTID_FFC, ZTID_FLOAT },{} },
	{ "getEffectWidth",             0,         ZTID_FLOAT,   FFCWIDTH,                  0,  { ZTID_FFC },{} },
	{ "setEffectWidth",             0,          ZTID_VOID,   FFCWIDTH,                  0,  { ZTID_FFC, ZTID_FLOAT },{} },
	{ "getEffectHeight",            0,         ZTID_FLOAT,   FFCHEIGHT,                 0,  { ZTID_FFC },{} },
	{ "setEffectHeight",            0,          ZTID_VOID,   FFCHEIGHT,                 0,  { ZTID_FFC, ZTID_FLOAT },{} },
	{ "getLink",                    0,         ZTID_FLOAT,   FFLINK,                    0,  { ZTID_FFC },{} },
	{ "setLink",                    0,          ZTID_VOID,   FFLINK,                    0,  { ZTID_FFC, ZTID_FLOAT },{} },
	{ "getMisc[]",                  0,       ZTID_UNTYPED,   FFMISCD,                   0,  { ZTID_FFC, ZTID_FLOAT },{} },
	{ "setMisc[]",                  0,          ZTID_VOID,   FFMISCD,                   0,  { ZTID_FFC, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getInitD[]",                 0,       ZTID_UNTYPED,   FFINITDD,                  0,  { ZTID_FFC, ZTID_FLOAT },{} },
	{ "setInitD[]",                 0,          ZTID_VOID,   FFINITDD,                  0,  { ZTID_FFC, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getScreenIndex",             0,         ZTID_FLOAT,   FF_SCREEN_INDEX,           0,  { ZTID_FFC },{} },
	{ "getID",                      0,         ZTID_FLOAT,   FFCID,                     0,  { ZTID_FFC },{} },
	{ "setID",                      0,          ZTID_VOID,   FFCID,                     0,  { ZTID_FFC, ZTID_FLOAT },{} },
	{ "getLastChangerX",            0,         ZTID_FLOAT,   FFCLASTCHANGERX,           0,  { ZTID_FFC },{} },
	{ "setLastChangerX",            0,          ZTID_VOID,   FFCLASTCHANGERX,           0,  { ZTID_FFC, ZTID_FLOAT },{} },
	{ "getLastChangerY",            0,         ZTID_FLOAT,   FFCLASTCHANGERY,           0,  { ZTID_FFC },{} },
	{ "setLastChangerY",            0,          ZTID_VOID,   FFCLASTCHANGERY,           0,  { ZTID_FFC, ZTID_FLOAT },{} },
	
	{ "Own",                        0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_FFC, ZTID_BITMAP },{} },
	{ "Own",                        1,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_FFC, ZTID_PALDATA },{} },
	{ "Own",                        2,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_FFC, ZTID_FILE },{} },
	{ "Own",                        3,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_FFC, ZTID_DIRECTORY },{} },
	{ "Own",                        4,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_FFC, ZTID_STACK },{} },
	{ "Own",                        5,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_FFC, ZTID_RNG },{} },
	{ "OwnArray",                   0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_FFC, ZTID_UNTYPED },{} },
	{ "OwnObject",                  0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_FFC, ZTID_UNTYPED },{} },
	
	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
};

FFCSymbols::FFCSymbols()
{
	table = FFCTable;
	refVar = REFFFC;
}

void FFCSymbols::generateCode()
{
	//void Own(ffc,bitmap)
	{
		Function* function = getFunction("Own",0);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//Target object
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Owner object
		POPREF();
		
		addOpcode2(code, new OObjOwnBitmap(new VarArgument(EXP1), new LiteralArgument(SCRIPT_FFC)));
		RETURN();
		function->giveCode(code);
	}
	//void Own(ffc,paldata)
	{
		Function* function = getFunction("Own",1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//Target object
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Owner object
		POPREF();
		
		addOpcode2(code, new OObjOwnPaldata(new VarArgument(EXP1), new LiteralArgument(SCRIPT_FFC)));
		RETURN();
		function->giveCode(code);
	}
	//void Own(ffc,file)
	{
		Function* function = getFunction("Own",2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//Target object
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Owner object
		POPREF();
		
		addOpcode2(code, new OObjOwnFile(new VarArgument(EXP1), new LiteralArgument(SCRIPT_FFC)));
		RETURN();
		function->giveCode(code);
	}
	//void Own(ffc,directory)
	{
		Function* function = getFunction("Own",3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//Target object
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Owner object
		POPREF();
		
		addOpcode2(code, new OObjOwnDir(new VarArgument(EXP1), new LiteralArgument(SCRIPT_FFC)));
		RETURN();
		function->giveCode(code);
	}
	//void Own(ffc,stack)
	{
		Function* function = getFunction("Own",4);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//Target object
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Owner object
		POPREF();
		
		addOpcode2(code, new OObjOwnStack(new VarArgument(EXP1), new LiteralArgument(SCRIPT_FFC)));
		RETURN();
		function->giveCode(code);
	}
	//void Own(ffc,rng)
	{
		Function* function = getFunction("Own",5);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//Target object
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Owner object
		POPREF();
		
		addOpcode2(code, new OObjOwnRNG(new VarArgument(EXP1), new LiteralArgument(SCRIPT_FFC)));
		RETURN();
		function->giveCode(code);
	}
	//void Own(ffc,untyped)
	{
		Function* function = getFunction("OwnArray");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//Target object
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Owner object
		POPREF();
		
		addOpcode2(code, new OObjOwnArray(new VarArgument(EXP1), new LiteralArgument(SCRIPT_FFC)));
		RETURN();
		function->giveCode(code);
	}
	//void Own(ffc,untyped)
	{
		Function* function = getFunction("OwnObject");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//Target object
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Owner object
		POPREF();
		
		addOpcode2(code, new OObjOwnClass(new VarArgument(EXP1), new LiteralArgument(SCRIPT_FFC)));
		RETURN();
		function->giveCode(code);
	}
}

