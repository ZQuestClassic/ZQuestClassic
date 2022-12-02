#include "SymbolDefs.h"

StackSymbols StackSymbols::singleton = StackSymbols();

static AccessorTable StackTable[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
	{ "getSize",                    0,          ZTID_LONG,   STACKSIZE,                 0,  { ZTID_STACK },{} },
	{ "getFull",                    0,          ZTID_BOOL,   STACKFULL,                 0,  { ZTID_STACK },{} },
	{ "Free",                       0,          ZTID_VOID,   -1,                        0,  { ZTID_STACK },{} },
	{ "Own",                        0,          ZTID_VOID,   -1,                        0,  { ZTID_STACK },{} },
	{ "Clear",                      0,          ZTID_VOID,   -1,                        0,  { ZTID_STACK },{} },
	{ "PushBack",                   0,          ZTID_VOID,   -1,                        0,  { ZTID_STACK, ZTID_UNTYPED },{} },
	{ "PushFront",                  0,          ZTID_VOID,   -1,                        0,  { ZTID_STACK, ZTID_UNTYPED },{} },
	{ "PopBack",                    0,       ZTID_UNTYPED,   -1,                        0,  { ZTID_STACK },{} },
	{ "PopFront",                   0,       ZTID_UNTYPED,   -1,                        0,  { ZTID_STACK },{} },
	{ "PeekBack",                   0,       ZTID_UNTYPED,   -1,                        0,  { ZTID_STACK },{} },
	{ "PeekFront",                  0,       ZTID_UNTYPED,   -1,                        0,  { ZTID_STACK },{} },
	{ "Get",                        0,       ZTID_UNTYPED,   -1,                        0,  { ZTID_STACK, ZTID_LONG },{} },
	{ "Set",                        0,          ZTID_VOID,   -1,                        0,  { ZTID_STACK, ZTID_LONG, ZTID_UNTYPED },{} },
	
	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
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
		Function* function = getFunction2("Free");
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
		Function* function = getFunction2("Own");
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
		Function* function = getFunction2("Clear");
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
		Function* function = getFunction2("PopBack");
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
		Function* function = getFunction2("PopFront");
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
		Function* function = getFunction2("PeekBack");
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
		Function* function = getFunction2("PeekFront");
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
		Function* function = getFunction2("PushBack");
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
		Function* function = getFunction2("PushFront");
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
		Function* function = getFunction2("Get");
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
		Function* function = getFunction2("Set");
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

