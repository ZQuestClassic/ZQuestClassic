#include "SymbolDefs.h"

StackSymbols StackSymbols::singleton = StackSymbols();

static AccessorTable StackTable[] =
{
//	  name,                     rettype,                  setorget,     var,              numindex,      funcFlags,                            numParams,   params
	{ "getSize",                ZTID_LONG,          GETTER,       STACKSIZE,        1,             0,                                    1,           { ZTID_STACK, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFull",                ZTID_BOOL,          GETTER,       STACKFULL,        1,             0,                                    1,           { ZTID_STACK, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Free",                   ZTID_VOID,          FUNCTION,     0,                1,             0,                                    1,           { ZTID_STACK, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Own",                    ZTID_VOID,          FUNCTION,     0,                1,             0,                                    1,           { ZTID_STACK, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Clear",                  ZTID_VOID,          FUNCTION,     0,                1,             0,                                    1,           { ZTID_STACK, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "PushBack",               ZTID_VOID,          FUNCTION,     0,                1,             0,                                    2,           { ZTID_STACK, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "PushFront",              ZTID_VOID,          FUNCTION,     0,                1,             0,                                    2,           { ZTID_STACK, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "PopBack",                ZTID_UNTYPED,       FUNCTION,     0,                1,             0,                                    1,           { ZTID_STACK, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "PopFront",               ZTID_UNTYPED,       FUNCTION,     0,                1,             0,                                    1,           { ZTID_STACK, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "PeekBack",               ZTID_UNTYPED,       FUNCTION,     0,                1,             0,                                    1,           { ZTID_STACK, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "PeekFront",              ZTID_UNTYPED,       FUNCTION,     0,                1,             0,                                    1,           { ZTID_STACK, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Get",                    ZTID_UNTYPED,       FUNCTION,     0,                1,             0,                                    2,           { ZTID_STACK, ZTID_LONG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Set",                    ZTID_VOID,          FUNCTION,     0,                1,             0,                                    3,           { ZTID_STACK, ZTID_LONG, ZTID_UNTYPED, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "",                       -1,                       -1,           -1,               -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

StackSymbols::StackSymbols()
{
	table = StackTable;
	refVar = REFSTACK;
}

void StackSymbols::generateCode()
{
	//void Free(stack)
	{
		Function* function = getFunction("Free", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new OStackFree());
		RETURN();
		function->giveCode(code);
	}
	//void Own(stack)
	{
		Function* function = getFunction("Own", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new OStackOwn());
		RETURN();
		function->giveCode(code);
	}
	//void Clear(stack)
	{
		Function* function = getFunction("Clear", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new OStackClear());
		RETURN();
		function->giveCode(code);
	}
	//untyped PopBack(stack)
	{
		Function* function = getFunction("PopBack", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new OStackPopBack(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//untyped PopFront(stack)
	{
		Function* function = getFunction("PopFront", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new OStackPopFront(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//untyped PeekBack(stack)
	{
		Function* function = getFunction("PeekBack", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new OStackPeekBack(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//untyped PeekFront(stack)
	{
		Function* function = getFunction("PeekFront", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new OStackPeekFront(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void PushBack(stack, untyped)
	{
		Function* function = getFunction("PushBack", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new OStackPushBack(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void PushFront(stack, untyped)
	{
		Function* function = getFunction("PushFront", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new OStackPushFront(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//untyped Get(stack, long)
	{
		Function* function = getFunction("Get", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new OStackGet(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void Set(stack, long, untyped)
	{
		Function* function = getFunction("Set", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		//pop pointer
		POPREF();
		addOpcode2 (code, new OStackSet(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
}

