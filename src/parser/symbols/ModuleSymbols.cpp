#include "SymbolDefs.h"

ModuleSymbols ModuleSymbols::singleton = ModuleSymbols();

static AccessorTable ModuleTable[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
	{ "GetInt",                     0,         ZTID_FLOAT,   -1,                        0,  { ZTID_MODULE, ZTID_CHAR, ZTID_CHAR },{} },
	{ "GetItemClass",               0,          ZTID_VOID,   -1,                        0,  { ZTID_MODULE, ZTID_CHAR, ZTID_CHAR },{} },
	{ "GetString",                  0,          ZTID_VOID,   -1,                        0,  { ZTID_MODULE, ZTID_CHAR, ZTID_CHAR, ZTID_CHAR },{} },
	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
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
		Function* function = getFunction("GetInt");
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
		Function* function = getFunction("GetString");
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
		Function* function = getFunction("GetItemClass");
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

