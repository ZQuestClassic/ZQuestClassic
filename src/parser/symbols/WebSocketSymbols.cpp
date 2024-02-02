#include "SymbolDefs.h"

WebSocketSymbols WebSocketSymbols::singleton = WebSocketSymbols();

static AccessorTable WebSocketTable[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
	{ "Free",                       0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_WEBSOCKET },{} },
	{ "Own",                        0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_WEBSOCKET },{} },
	{ "GetError",                   0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_WEBSOCKET, ZTID_CHAR },{} },
	{ "getState",                   0,         ZTID_FLOAT,   WEBSOCKET_STATE,           0,  { ZTID_WEBSOCKET },{} },
	{ "Send",                       0,         ZTID_VOID,    -1,                   FL_INL,  { ZTID_WEBSOCKET, ZTID_CHAR, ZTID_LONG },{1}, {} },
	{ "getHasMessage",              0,         ZTID_BOOL,    WEBSOCKET_HAS_MESSAGE,     0,  { ZTID_WEBSOCKET },{} },
	{ "getMessageType",             0,        ZTID_FLOAT,    WEBSOCKET_MESSAGE_TYPE,    0,  { ZTID_WEBSOCKET },{} },
	{ "Receive",                    0,         ZTID_LONG,    -1,                   FL_INL,  { ZTID_WEBSOCKET },{} },
	{ "getURL",                     0,         ZTID_CHAR,    WEBSOCKET_URL,             0,  { ZTID_WEBSOCKET },{} },
	
	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
};

WebSocketSymbols::WebSocketSymbols()
{
	table = WebSocketTable;
	refVar = REFWEBSOCKET;
}

void WebSocketSymbols::generateCode()
{
	//void Free(websocket)
	{
		Function* function = getFunction("Free");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new OWebSocketFree());
		RETURN();
		function->giveCode(code);
	}
	//void Own(websocket)
	{
		Function* function = getFunction("Own");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new OWebSocketOwn());
		RETURN();
		function->giveCode(code);
	}
	//void GetError(websocket, char32*)
	{
		Function* function = getFunction("GetError");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new OWebSocketGetError(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void Send(websocket, char32*, float)
	{
		Function* function = getFunction("Send");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer
		POPREF();
		addOpcode2 (code, new OWebSocketSend(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//char32* Receive(websocket)
	{
		Function* function = getFunction("Receive");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new OWebSocketReceive(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
}
