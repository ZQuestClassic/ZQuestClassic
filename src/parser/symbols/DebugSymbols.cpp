#include "SymbolDefs.h"

DebugSymbols DebugSymbols::singleton = DebugSymbols();

static AccessorTable DebugTable[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
//	All of these return a function label error when used:
	{ "getRefFFC",                  0,         ZTID_FLOAT,   DEBUGREFFFC,               0,  { ZTID_DEBUG },{} },
	{ "getRefItem",                 0,         ZTID_FLOAT,   DEBUGREFITEM,              0,  { ZTID_DEBUG },{} },
	{ "getRefItemdata",             0,         ZTID_FLOAT,   DEBUGREFITEMDATA,          0,  { ZTID_DEBUG },{} },
	{ "getRefNPC",                  0,         ZTID_FLOAT,   DEBUGREFNPC,               0,  { ZTID_DEBUG },{} },
	{ "getRefLWeapon",              0,         ZTID_FLOAT,   DEBUGREFLWEAPON,           0,  { ZTID_DEBUG },{} },
	{ "getRefEWeapon",              0,         ZTID_FLOAT,   DEBUGREFEWEAPON,           0,  { ZTID_DEBUG },{} },
	{ "getSP",                      0,         ZTID_FLOAT,   SP,                        0,  { ZTID_DEBUG },{} },
	{ "getPC",                      0,         ZTID_FLOAT,   PC,                        0,  { ZTID_DEBUG },{} },
	{ "setRefFFC",                  0,         ZTID_FLOAT,   DEBUGREFFFC,               0,  { ZTID_DEBUG },{} },
	{ "setRefItem",                 0,         ZTID_FLOAT,   DEBUGREFITEM,              0,  { ZTID_DEBUG },{} },
	{ "setRefItemdata",             0,         ZTID_FLOAT,   DEBUGREFITEMDATA,          0,  { ZTID_DEBUG },{} },
	{ "setRefNPC",                  0,         ZTID_FLOAT,   DEBUGREFNPC,               0,  { ZTID_DEBUG },{} },
	{ "setRefLWeapon",              0,         ZTID_FLOAT,   DEBUGREFLWEAPON,           0,  { ZTID_DEBUG },{} },
	{ "setRefEWeapon",              0,         ZTID_FLOAT,   DEBUGREFEWEAPON,           0,  { ZTID_DEBUG },{} },
	{ "setPC",                      0,         ZTID_FLOAT,   PC,                        0,  { ZTID_DEBUG },{} },
	{ "setSP",                      0,         ZTID_FLOAT,   SP,                        0,  { ZTID_DEBUG },{} },
	{ "getGDR[]",                   0,         ZTID_FLOAT,   DEBUGGDR,                  0,  { ZTID_DEBUG, ZTID_FLOAT },{} },
	{ "setGDR[]",                   0,          ZTID_VOID,   DEBUGGDR,                  0,  { ZTID_DEBUG, ZTID_FLOAT, ZTID_FLOAT },{} },
	
//	These all work, but may not be permitted by the rest of the dev team. 
//	At least they are now in a pointer class that denotes that they can be dangerous. 
	{ "GetBoolPointer",             0,         ZTID_FLOAT,   -1,                        0,  { ZTID_DEBUG, ZTID_BOOL },{} },
	{ "SetBoolPointer",             0,          ZTID_BOOL,   -1,                        0,  { ZTID_DEBUG, ZTID_FLOAT },{} },
	{ "GetNPCPointer",              0,         ZTID_FLOAT,   -1,                        0,  { ZTID_DEBUG, ZTID_NPC },{} },
	{ "SetNPCPointer",              0,           ZTID_NPC,   -1,                        0,  { ZTID_DEBUG, ZTID_FLOAT },{} },
	{ "GetLWeaponPointer",          0,         ZTID_FLOAT,   -1,                        0,  { ZTID_DEBUG, ZTID_LWPN },{} },
	{ "SetLWeaponPointer",          0,          ZTID_LWPN,   -1,                        0,  { ZTID_DEBUG, ZTID_FLOAT },{} },
	{ "GetEWeaponPointer",          0,         ZTID_FLOAT,   -1,                        0,  { ZTID_DEBUG, ZTID_EWPN },{} },
	{ "SetEWeaponPointer",          0,          ZTID_EWPN,   -1,                        0,  { ZTID_DEBUG, ZTID_FLOAT },{} },
	{ "GetFFCPointer",              0,         ZTID_FLOAT,   -1,                        0,  { ZTID_DEBUG, ZTID_FFC },{} },
	{ "SetFFCPointer",              0,           ZTID_FFC,   -1,                        0,  { ZTID_DEBUG, ZTID_FLOAT },{} },
	{ "GetItemPointer",             0,         ZTID_FLOAT,   -1,                        0,  { ZTID_DEBUG, ZTID_ITEM },{} },
	{ "SetItemPointer",             0,          ZTID_ITEM,   -1,                        0,  { ZTID_DEBUG, ZTID_FLOAT },{} },
	{ "GetItemdataPointer",         0,         ZTID_FLOAT,   -1,                        0,  { ZTID_DEBUG, ZTID_ITEMCLASS },{} },
	{ "SetItemdataPointer",         0,     ZTID_ITEMCLASS,   -1,                        0,  { ZTID_DEBUG, ZTID_FLOAT },{} },
	
	{ "getPlayfieldOffset",         0,         ZTID_FLOAT,   GAMEPLAYFIELDOFS,          0,  { ZTID_DEBUG },{} },
	{ "setPlayfieldOffset",         0,          ZTID_VOID,   GAMEPLAYFIELDOFS,  FL_RDONLY,  { ZTID_DEBUG, ZTID_FLOAT },{} },
	{ "TriggerSecret",              0,          ZTID_VOID,   -1,                        0,  { ZTID_DEBUG, ZTID_FLOAT },{} },
	
	{ "getD[]",                     0,         ZTID_FLOAT,   DEBUGD,                    0,  { ZTID_DEBUG, ZTID_FLOAT },{} },
	{ "setD[]",                     0,         ZTID_FLOAT,   DEBUGD,                    0,  { ZTID_DEBUG, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "_getNULL",                   0,       ZTID_UNTYPED,   DONULL,                    0,  { ZTID_DEBUG },{} },
	{ "_getNull",                   0,       ZTID_UNTYPED,   DONULL,                    0,  { ZTID_DEBUG },{} },
	{ "getNULL",                    0,       ZTID_UNTYPED,   DONULL,                    0,  { ZTID_DEBUG },{} },
	{ "setNULL",                    0,          ZTID_VOID,   DONULL,            FL_RDONLY,  { ZTID_DEBUG, ZTID_UNTYPED },{} },
	{ "getNull", 0, "getNULL", 0 },
	{ "setNull", 0, "setNULL", 0 },
	{ "Breakpoint",                 0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_DEBUG, ZTID_CHAR },{ 0 } },
	{ "getTesting",                 0,          ZTID_BOOL,   DEBUGTESTING,              0,  { ZTID_DEBUG },{} },
	{ "setTesting",                 0,          ZTID_VOID,   DEBUGTESTING,      FL_RDONLY,  { ZTID_DEBUG, ZTID_BOOL },{} },
	
	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
};

DebugSymbols::DebugSymbols()
{
	table = DebugTable;
	refVar = NUL;
}

void DebugSymbols::generateCode()
{
	//int32_t GetPointer(itemclass, itemclass)
	{
		Function* function = getFunction("GetItemdataPointer");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OGetItemDataPointer(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	
	//int32_t SetPointer(itemclass, float)
	{
		Function* function = getFunction("SetItemdataPointer");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSetItemDataPointer(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t GetPointer(item, item)
	{
		Function* function = getFunction("GetItemPointer");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OGetItemPointer(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	
	//int32_t SetPointer(item, float)
	{
		Function* function = getFunction("SetItemPointer");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSetItemPointer(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}    
	//int32_t GetPointer(ffc, ffc)
	{
		Function* function = getFunction("GetFFCPointer");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OGetFFCPointer(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	
	//int32_t SetPointer(ffc, float)
	{
		Function* function = getFunction("SetFFCPointer");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSetFFCPointer(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
		  //int32_t GetPointer(eweapon, eweapon)
	{
		Function* function = getFunction("GetEWeaponPointer");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OGetEWeaponPointer(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	
	//int32_t SetPointer(eweapon, float)
	{
		Function* function = getFunction("SetEWeaponPointer");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSetEWeaponPointer(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	   //int32_t GetPointer(lweapon, lweapon)
	{
		Function* function = getFunction("GetLWeaponPointer");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OGetLWeaponPointer(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	
	//int32_t SetPointer(lweapon, float)
	{
		Function* function = getFunction("SetLWeaponPointer");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSetLWeaponPointer(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	 //int32_t GetPointer(npc, ffc)
	{
		Function* function = getFunction("GetNPCPointer");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OGetNPCPointer(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	
	//int32_t SetPointer(npc, float)
	{
		Function* function = getFunction("SetNPCPointer");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSetNPCPointer(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t GetPointer(game, bool)
	{
		Function* function = getFunction("GetBoolPointer");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OGetBoolPointer(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	
	//int32_t SetPointer(game, float)
	{
		Function* function = getFunction("SetBoolPointer");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSetBoolPointer(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	
	
	//void TriggerSecret(game, int32_t)
	{
		Function* function = getFunction("TriggerSecret");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OTriggerSecretRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	
	//void Breakpoint(debug, char)
	{
		Function* function = getFunction("Breakpoint");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		POPREF();
		addOpcode2 (code, new OBreakpoint(new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
}

