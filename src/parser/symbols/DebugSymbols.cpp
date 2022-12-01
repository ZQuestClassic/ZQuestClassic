#include "SymbolDefs.h"

DebugSymbols DebugSymbols::singleton = DebugSymbols();

static AccessorTable DebugTable[] =
{
//	name,                        rettype,                  setorget,     var,                  numindex,      params
//	All of these return a function label error when used:
	{ "getRefFFC",               ZVARTYPEID_FLOAT,         GETTER,       DEBUGREFFFC,          1,             0,                                    1,           { ZVARTYPEID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRefItem",              ZVARTYPEID_FLOAT,         GETTER,       DEBUGREFITEM,         1,             0,                                    1,           { ZVARTYPEID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRefItemdata",          ZVARTYPEID_FLOAT,         GETTER,       DEBUGREFITEMDATA,     1,             0,                                    1,           { ZVARTYPEID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRefNPC",               ZVARTYPEID_FLOAT,         GETTER,       DEBUGREFNPC,          1,             0,                                    1,           { ZVARTYPEID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRefLWeapon",           ZVARTYPEID_FLOAT,         GETTER,       DEBUGREFLWEAPON,      1,             0,                                    1,           { ZVARTYPEID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getRefEWeapon",           ZVARTYPEID_FLOAT,         GETTER,       DEBUGREFEWEAPON,      1,             0,                                    1,           { ZVARTYPEID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSP",                   ZVARTYPEID_FLOAT,         GETTER,       DEBUGSP,              1,             0,                                    1,           { ZVARTYPEID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPC",                   ZVARTYPEID_FLOAT,         GETTER,       PC,                   1,             0,                                    1,           { ZVARTYPEID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRefFFC",               ZVARTYPEID_FLOAT,         SETTER,       DEBUGREFFFC,          1,             0,                                    1,           { ZVARTYPEID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRefItem",              ZVARTYPEID_FLOAT,         SETTER,       DEBUGREFITEM,         1,             0,                                    1,           { ZVARTYPEID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRefItemdata",          ZVARTYPEID_FLOAT,         SETTER,       DEBUGREFITEMDATA,     1,             0,                                    1,           { ZVARTYPEID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRefNPC",               ZVARTYPEID_FLOAT,         SETTER,       DEBUGREFNPC,          1,             0,                                    1,           { ZVARTYPEID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRefLWeapon",           ZVARTYPEID_FLOAT,         SETTER,       DEBUGREFLWEAPON,      1,             0,                                    1,           { ZVARTYPEID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setRefEWeapon",           ZVARTYPEID_FLOAT,         SETTER,       DEBUGREFEWEAPON,      1,             0,                                    1,           { ZVARTYPEID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPC",                   ZVARTYPEID_FLOAT,         SETTER,       PC,                   1,             0,                                    1,           { ZVARTYPEID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSP",                   ZVARTYPEID_FLOAT,         SETTER,       DEBUGSP,              1,             0,                                    1,           { ZVARTYPEID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getGDR[]",                ZVARTYPEID_FLOAT,         GETTER,       DEBUGGDR,             256,           0,                                    2,           { ZVARTYPEID_DEBUG, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setGDR[]",                ZVARTYPEID_VOID,          SETTER,       DEBUGGDR,             256,           0,                                    3,           { ZVARTYPEID_DEBUG, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
//	These all work, but may not be permitted by the rest of the dev team. 
//	At least they are now in a pointer class that denotes that they can be dangerous. 
	{ "GetBoolPointer",          ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_DEBUG, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetBoolPointer",          ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_DEBUG, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetNPCPointer",           ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_DEBUG, ZVARTYPEID_NPC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetNPCPointer",           ZVARTYPEID_NPC,           FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_DEBUG, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetLWeaponPointer",       ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_DEBUG, ZVARTYPEID_LWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetLWeaponPointer",       ZVARTYPEID_LWPN,          FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_DEBUG, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetEWeaponPointer",       ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_DEBUG, ZVARTYPEID_EWPN, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetEWeaponPointer",       ZVARTYPEID_EWPN,          FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_DEBUG, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetFFCPointer",           ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_DEBUG, ZVARTYPEID_FFC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetFFCPointer",           ZVARTYPEID_FFC,           FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_DEBUG, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetItemPointer",          ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_DEBUG, ZVARTYPEID_ITEM, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetItemPointer",          ZVARTYPEID_ITEM,          FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_DEBUG, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetItemdataPointer",      ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_DEBUG, ZVARTYPEID_ITEMCLASS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "SetItemdataPointer",      ZVARTYPEID_ITEMCLASS,     FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_DEBUG, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getPlayfieldOffset",      ZVARTYPEID_FLOAT,         GETTER,       GAMEPLAYFIELDOFS,     1,             0,                                    1,           { ZVARTYPEID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "TriggerSecret",           ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_DEBUG, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "ChangeFFCScript",         ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             0,                                    2,           { ZVARTYPEID_DEBUG, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getD[]",                     ZVARTYPEID_FLOAT,         GETTER,       DEBUGD,               256,           0,                                    2,           { ZVARTYPEID_DEBUG, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setD[]",                     ZVARTYPEID_FLOAT,         SETTER,       DEBUGD,               256,           0,                                    3,           { ZVARTYPEID_DEBUG, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "NULL",                    ZVARTYPEID_UNTYPED,       GETTER,       DONULL,               1,             0,                                    1,           { ZVARTYPEID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Null",                    ZVARTYPEID_UNTYPED,       GETTER,       DONULL,               1,             0,                                    1,           { ZVARTYPEID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getNULL",                 ZVARTYPEID_UNTYPED,       GETTER,       DONULL,               1,             0,                                    1,           { ZVARTYPEID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getNull",                 ZVARTYPEID_UNTYPED,       GETTER,       DONULL,               1,             0,                                    1,           { ZVARTYPEID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Breakpoint",              ZVARTYPEID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_DEBUG, ZVARTYPEID_CHAR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTesting",              ZVARTYPEID_BOOL,          GETTER,       DEBUGTESTING,         1,             0,                                    1,           { ZVARTYPEID_DEBUG, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
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

