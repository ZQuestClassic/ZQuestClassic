#include "SymbolDefs.h"

TextPtrSymbols TextPtrSymbols::singleton = TextPtrSymbols();

static AccessorTable TextTable[] =
{
//	All of these return a function label error when used:
//	  name,                     rettype,                  setorget,     var,                    numindex,      funcFlags,                            numParams,   params
//	{ "getTest",                ZVARTYPEID_FLOAT,         GETTER,       DEBUGREFFFC,            1,             0,                                    1,           { ZVARTYPEID_TEXT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "StringWidth",            ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_TEXT, ZVARTYPEID_CHAR, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "CharWidth",              ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_TEXT, ZVARTYPEID_CHAR, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "StringHeight",           ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_TEXT, ZVARTYPEID_CHAR, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "CharHeight",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_TEXT, ZVARTYPEID_CHAR, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "FontHeight",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_TEXT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MessageWidth",           ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      3,           { ZVARTYPEID_TEXT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "MessageHeight",          ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_TEXT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "",                       -1,                       -1,           -1,                     -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

TextPtrSymbols::TextPtrSymbols()
{
    table = TextTable;
    refVar = NUL;
}

void TextPtrSymbols::generateCode()
{
	//void StringWidth(char32 ptr, int32_t font)
	{
		Function* function = getFunction("StringWidth", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the font
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop off the string ptr
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop off the pointer
		POPREF();
		addOpcode2 (code, new OStringWidth(new VarArgument(EXP2),new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void CharWidth(char32 chr, int32_t font)
	{
		Function* function = getFunction("CharWidth", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the font
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop off the character
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop off the pointer
		POPREF();
		addOpcode2 (code, new OCharWidth(new VarArgument(EXP2),new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void StringHeight(char32 ptr, int32_t font)
	{
		Function* function = getFunction("StringHeight", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the font
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//ignore the string ptr; height is purely font-based
		addOpcode2 (code, new OPopRegister(new VarArgument(NUL)));
		//pop off the pointer
		POPREF();
		addOpcode2 (code, new OFontHeight(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void CharHeight(char32 chr, int32_t font)
	{
		Function* function = getFunction("CharHeight", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the font
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//ignore the character; height is purely font-based
		addOpcode2 (code, new OPopRegister(new VarArgument(NUL)));
		//pop off the pointer
		POPREF();
		addOpcode2 (code, new OFontHeight(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void FontHeight(int32_t font)
	{
		Function* function = getFunction("FontHeight", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the font
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop off the pointer
		POPREF();
		addOpcode2 (code, new OFontHeight(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void MessageWidth(int32_t message)
	{
		Function* function = getFunction("MessageWidth", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the message
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop off the pointer
		POPREF();
		addOpcode2 (code, new OMessageWidth(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void MessageHeight(int32_t message)
	{
		Function* function = getFunction("MessageHeight", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the message
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop off the pointer
		POPREF();
		addOpcode2 (code, new OMessageHeight(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
}

