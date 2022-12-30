#include "SymbolDefs.h"

TextPtrSymbols TextPtrSymbols::singleton = TextPtrSymbols();

static AccessorTable TextTable[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
	{ "StringWidth",                0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_TEXT, ZTID_CHAR, ZTID_FLOAT },{} },
	{ "CharWidth",                  0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_TEXT, ZTID_CHAR, ZTID_FLOAT },{} },
	{ "StringHeight",               0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_TEXT, ZTID_CHAR, ZTID_FLOAT },{} },
	{ "CharHeight",                 0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_TEXT, ZTID_CHAR, ZTID_FLOAT },{} },
	{ "FontHeight",                 0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_TEXT, ZTID_FLOAT },{} },
	{ "MessageWidth",               0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_TEXT, ZTID_FLOAT },{} },
	{ "MessageHeight",              0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_TEXT, ZTID_FLOAT },{} },
	
	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
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
		Function* function = getFunction("StringWidth");
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
		Function* function = getFunction("CharWidth");
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
		Function* function = getFunction("StringHeight");
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
		Function* function = getFunction("CharHeight");
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
		Function* function = getFunction("FontHeight");
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
		Function* function = getFunction("MessageWidth");
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
		Function* function = getFunction("MessageHeight");
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

