#include "SymbolDefs.h"

SpriteDataSymbols SpriteDataSymbols::singleton = SpriteDataSymbols();

static AccessorTable SpriteDataTable[] =
{
//	All of these return a function label error when used:
//	  name,                     rettype,                  setorget,     var,                  numindex,      params
/*	
	{ "GetTile",                ZTID_FLOAT,         FUNCTION,     0,                    1,             0,                                    2,           { ZTID_SPRITEDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetMisc",                ZTID_FLOAT,         FUNCTION,     0,                    1,             0,                                    2,           { ZTID_SPRITEDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetCSets",               ZTID_FLOAT,         FUNCTION,     0,                    1,             0,                                    2,           { ZTID_SPRITEDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetFrames",              ZTID_FLOAT,         FUNCTION,     0,                    1,             0,                                    2,           { ZTID_SPRITEDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetSpeed",               ZTID_FLOAT,         FUNCTION,     0,                    1,             0,                                    2,           { ZTID_SPRITEDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetType",                ZTID_FLOAT,         FUNCTION,     0,                    1,             0,                                    2,           { ZTID_SPRITEDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "SetTile",                ZTID_VOID,          FUNCTION,     0,                    1,             0,                                    3,           { ZTID_SPRITEDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetMisc",                ZTID_VOID,          FUNCTION,     0,                    1,             0,                                    3,           { ZTID_SPRITEDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetCSets",               ZTID_VOID,          FUNCTION,     0,                    1,             0,                                    3,           { ZTID_SPRITEDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetFrames",              ZTID_VOID,          FUNCTION,     0,                    1,             0,                                    3,           { ZTID_SPRITEDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetSpeed",               ZTID_VOID,          FUNCTION,     0,                    1,             0,                                    3,           { ZTID_SPRITEDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetType",                ZTID_VOID,          FUNCTION,     0,                    1,             0,                                    3,           { ZTID_SPRITEDATA, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
*/	
//	Datatype variables spritedata sd->Var;
	
	{ "getTile",                ZTID_FLOAT,         GETTER,       SPRITEDATATILE,       1,             0,                                    1,           { ZTID_SPRITEDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTile",                ZTID_VOID,          SETTER,       SPRITEDATATILE,       1,             0,                                    2,           { ZTID_SPRITEDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMisc",                ZTID_FLOAT,         GETTER,       SPRITEDATAMISC,       1,             0,                                    1,           { ZTID_SPRITEDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMisc",                ZTID_VOID,          SETTER,       SPRITEDATAMISC,       1,             0,                                    2,           { ZTID_SPRITEDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCSet",                ZTID_FLOAT,         GETTER,       SPRITEDATACSETS,      1,             0,                                    1,           { ZTID_SPRITEDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCSet",                ZTID_VOID,          SETTER,       SPRITEDATACSETS,      1,             0,                                    2,           { ZTID_SPRITEDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFrames",              ZTID_FLOAT,         GETTER,       SPRITEDATAFRAMES,     1,             0,                                    1,           { ZTID_SPRITEDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFrames",              ZTID_VOID,          SETTER,       SPRITEDATAFRAMES,     1,             0,                                    2,           { ZTID_SPRITEDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSpeed",               ZTID_FLOAT,         GETTER,       SPRITEDATASPEED,      1,             0,                                    1,           { ZTID_SPRITEDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSpeed",               ZTID_VOID,          SETTER,       SPRITEDATASPEED,      1,             0,                                    2,           { ZTID_SPRITEDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getType",                ZTID_FLOAT,         GETTER,       SPRITEDATATYPE,       1,             0,                                    1,           { ZTID_SPRITEDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setType",                ZTID_VOID,          SETTER,       SPRITEDATATYPE,       1,             0,                                    2,           { ZTID_SPRITEDATA, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "",                       -1,                       -1,           -1,                   -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

SpriteDataSymbols::SpriteDataSymbols()
{
	table = SpriteDataTable;
	refVar = REFSPRITEDATA; //NUL;// 
}

void SpriteDataSymbols::generateCode()
{
	//GetTile(SpriteData, int32_t)
	/*
	{
		Function* function = getFunction("GetTile", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSDataTile(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetMisc(SpriteData, int32_t)
	{
		Function* function = getFunction("GetMisc", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSDataMisc(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetCSets(SpriteData, int32_t)
	{
		Function* function = getFunction("GetCSets", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSDataCSets(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetFrames(SpriteData, int32_t)
	{
		Function* function = getFunction("GetFrames", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSDataFrames(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetSpeed(SpriteData, int32_t)
	{
		Function* function = getFunction("GetSpeed", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSDataSpeed(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//GetType(SpriteData, int32_t)
	{
		Function* function = getFunction("GetType", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSDataType(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//SetTile(SpriteData, int32_t, int32_t)
	{
		Function* function = getFunction("SetTile", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OSSetDataTile(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//SetMisc(SpriteData, int32_t, int32_t)
	{
		Function* function = getFunction("SetMisc", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OSSetDataMisc(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//SetCSets(SpriteData, int32_t, int32_t)
	{
		Function* function = getFunction("SetCSets", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OSSetDataCSets(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//SetFrames(SpriteData, int32_t, int32_t)
	{
		Function* function = getFunction("SetFrames", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OSSetDataFrames(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//SetSpeed(SpriteData, int32_t, int32_t)
	{
		Function* function = getFunction("SetSpeed", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OSSetDataSpeed(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//SetType(SpriteData, int32_t, int32_t)
	{
		Function* function = getFunction("SetType", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OSSetDataType(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	*/
}

