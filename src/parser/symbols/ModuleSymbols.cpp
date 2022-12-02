#include "SymbolDefs.h"

ModuleSymbols ModuleSymbols::singleton = ModuleSymbols();

static AccessorTable ModuleTable[] =
{
//	  name,                     rettype,                  setorget,     var,              numindex,      funcFlags,                            numParams,   params
	{ "GetInt",              ZTID_FLOAT,          FUNCTION,     0,                1,             0,                      3,           { ZTID_MODULE, ZTID_CHAR, ZTID_CHAR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetItemClass",              ZTID_VOID,          FUNCTION,     0,                1,             0,                      3,           { ZTID_MODULE, ZTID_CHAR, ZTID_CHAR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetString",             ZTID_VOID,          FUNCTION,     0,                1,             0,                      4,           { ZTID_MODULE, ZTID_CHAR, ZTID_CHAR, ZTID_CHAR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "",                       -1,                       -1,           -1,               -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

ModuleSymbols::ModuleSymbols()
{
	table = ModuleTable;
	refVar = NUL;
}

void ModuleSymbols::generateCode()
{
	//int32_t GetInt(file, char32* section, char32* entry)
	{
		Function* function = getFunction("GetInt", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(MODULEGETINT)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t GetString(file, char32* dest, char32* section, char32* entry)
	{
		Function* function = getFunction("GetString", 4);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSetRegister(new VarArgument(MODULEGETSTR), new VarArgument(SFTEMP)));
		RETURN();
		function->giveCode(code);
	}
	//GetItemClass(char32* dest, int32_t ic)
	{
		Function* function = getFunction("GetItemClass", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OModuleGetIC(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
}

