#include "SymbolDefs.h"

DebugSymbols DebugSymbols::singleton = DebugSymbols();

static AccessorTable DebugTable[] =
{
//	name,                        rettype,                  setorget,     var,                  numindex,      params
//	All of these return a function label error when used:
	{ "getRefFFC",               ZTID_FLOAT,         GETTER,       DEBUGREFFFC,          1,             0,                                    1,           { ZTID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRefItem",              ZTID_FLOAT,         GETTER,       DEBUGREFITEM,         1,             0,                                    1,           { ZTID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRefItemdata",          ZTID_FLOAT,         GETTER,       DEBUGREFITEMDATA,     1,             0,                                    1,           { ZTID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRefNPC",               ZTID_FLOAT,         GETTER,       DEBUGREFNPC,          1,             0,                                    1,           { ZTID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRefLWeapon",           ZTID_FLOAT,         GETTER,       DEBUGREFLWEAPON,      1,             0,                                    1,           { ZTID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRefEWeapon",           ZTID_FLOAT,         GETTER,       DEBUGREFEWEAPON,      1,             0,                                    1,           { ZTID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSP",                   ZTID_FLOAT,         GETTER,       DEBUGSP,              1,             0,                                    1,           { ZTID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPC",                   ZTID_FLOAT,         GETTER,       PC,                   1,             0,                                    1,           { ZTID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRefFFC",               ZTID_FLOAT,         SETTER,       DEBUGREFFFC,          1,             0,                                    1,           { ZTID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRefItem",              ZTID_FLOAT,         SETTER,       DEBUGREFITEM,         1,             0,                                    1,           { ZTID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRefItemdata",          ZTID_FLOAT,         SETTER,       DEBUGREFITEMDATA,     1,             0,                                    1,           { ZTID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRefNPC",               ZTID_FLOAT,         SETTER,       DEBUGREFNPC,          1,             0,                                    1,           { ZTID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRefLWeapon",           ZTID_FLOAT,         SETTER,       DEBUGREFLWEAPON,      1,             0,                                    1,           { ZTID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRefEWeapon",           ZTID_FLOAT,         SETTER,       DEBUGREFEWEAPON,      1,             0,                                    1,           { ZTID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPC",                   ZTID_FLOAT,         SETTER,       PC,                   1,             0,                                    1,           { ZTID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSP",                   ZTID_FLOAT,         SETTER,       DEBUGSP,              1,             0,                                    1,           { ZTID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getGDR[]",                ZTID_FLOAT,         GETTER,       DEBUGGDR,             256,           0,                                    2,           { ZTID_DEBUG, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setGDR[]",                ZTID_VOID,          SETTER,       DEBUGGDR,             256,           0,                                    3,           { ZTID_DEBUG, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
//	These all work, but may not be permitted by the rest of the dev team. 
//	At least they are now in a pointer class that denotes that they can be dangerous. 
	{ "GetBoolPointer",          ZTID_FLOAT,         FUNCTION,     0,                    1,             0,                                    2,           { ZTID_DEBUG, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetBoolPointer",          ZTID_BOOL,          FUNCTION,     0,                    1,             0,                                    2,           { ZTID_DEBUG, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetNPCPointer",           ZTID_FLOAT,         FUNCTION,     0,                    1,             0,                                    2,           { ZTID_DEBUG, ZTID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetNPCPointer",           ZTID_NPC,           FUNCTION,     0,                    1,             0,                                    2,           { ZTID_DEBUG, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetLWeaponPointer",       ZTID_FLOAT,         FUNCTION,     0,                    1,             0,                                    2,           { ZTID_DEBUG, ZTID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetLWeaponPointer",       ZTID_LWPN,          FUNCTION,     0,                    1,             0,                                    2,           { ZTID_DEBUG, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetEWeaponPointer",       ZTID_FLOAT,         FUNCTION,     0,                    1,             0,                                    2,           { ZTID_DEBUG, ZTID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetEWeaponPointer",       ZTID_EWPN,          FUNCTION,     0,                    1,             0,                                    2,           { ZTID_DEBUG, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetFFCPointer",           ZTID_FLOAT,         FUNCTION,     0,                    1,             0,                                    2,           { ZTID_DEBUG, ZTID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetFFCPointer",           ZTID_FFC,           FUNCTION,     0,                    1,             0,                                    2,           { ZTID_DEBUG, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetItemPointer",          ZTID_FLOAT,         FUNCTION,     0,                    1,             0,                                    2,           { ZTID_DEBUG, ZTID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetItemPointer",          ZTID_ITEM,          FUNCTION,     0,                    1,             0,                                    2,           { ZTID_DEBUG, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetItemdataPointer",      ZTID_FLOAT,         FUNCTION,     0,                    1,             0,                                    2,           { ZTID_DEBUG, ZTID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetItemdataPointer",      ZTID_ITEMCLASS,     FUNCTION,     0,                    1,             0,                                    2,           { ZTID_DEBUG, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getPlayfieldOffset",      ZTID_FLOAT,         GETTER,       GAMEPLAYFIELDOFS,     1,             0,                                    1,           { ZTID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "TriggerSecret",           ZTID_VOID,          FUNCTION,     0,                    1,             0,                                    2,           { ZTID_DEBUG, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "ChangeFFCScript",         ZTID_VOID,          FUNCTION,     0,                    1,             0,                                    2,           { ZTID_DEBUG, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getD[]",                     ZTID_FLOAT,         GETTER,       DEBUGD,               256,           0,                                    2,           { ZTID_DEBUG, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setD[]",                     ZTID_FLOAT,         SETTER,       DEBUGD,               256,           0,                                    3,           { ZTID_DEBUG, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "NULL",                    ZTID_UNTYPED,       GETTER,       DONULL,               1,             0,                                    1,           { ZTID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Null",                    ZTID_UNTYPED,       GETTER,       DONULL,               1,             0,                                    1,           { ZTID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getNULL",                 ZTID_UNTYPED,       GETTER,       DONULL,               1,             0,                                    1,           { ZTID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getNull",                 ZTID_UNTYPED,       GETTER,       DONULL,               1,             0,                                    1,           { ZTID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Breakpoint",              ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_DEBUG, ZTID_CHAR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTesting",              ZTID_BOOL,          GETTER,       DEBUGTESTING,         1,             0,                                    1,           { ZTID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "",                        -1,                       -1,           -1,                   -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
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
        Function* function = getFunction("GetItemdataPointer", 2);
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
        Function* function = getFunction("SetItemdataPointer", 2);
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
        Function* function = getFunction("GetItemPointer", 2);
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
        Function* function = getFunction("SetItemPointer", 2);
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
        Function* function = getFunction("GetFFCPointer", 2);
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
        Function* function = getFunction("SetFFCPointer", 2);
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
        Function* function = getFunction("GetEWeaponPointer", 2);
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
        Function* function = getFunction("SetEWeaponPointer", 2);
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
        Function* function = getFunction("GetLWeaponPointer", 2);
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
        Function* function = getFunction("SetLWeaponPointer", 2);
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
        Function* function = getFunction("GetNPCPointer", 2);
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
        Function* function = getFunction("SetNPCPointer", 2);
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
        Function* function = getFunction("GetBoolPointer", 2);
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
        Function* function = getFunction("SetBoolPointer", 2);
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
        Function* function = getFunction("TriggerSecret", 2);
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
	    Function* function = getFunction("Breakpoint", 2);
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

