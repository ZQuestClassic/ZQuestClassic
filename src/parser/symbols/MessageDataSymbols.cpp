#include "SymbolDefs.h"

MessageDataSymbols MessageDataSymbols::singleton = MessageDataSymbols();

static AccessorTable MessageDataTable[] =
{
//	All of these return a function label error when used:
//	  name,                     rettype,                  setorget,     var,                  numindex,     funcFlags,                            numParams,   params
	{ "Get",                    ZVARTYPEID_VOID,          FUNCTION,     0,                    1,            FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_ZMESSAGE, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Set",                    ZVARTYPEID_VOID,          FUNCTION,     0,                    1,            0,                                    2,           { ZVARTYPEID_ZMESSAGE, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getNext",                ZVARTYPEID_FLOAT,         GETTER,       MESSAGEDATANEXT,      1,            0,                                    1,           { ZVARTYPEID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNext",                ZVARTYPEID_VOID,          SETTER,       MESSAGEDATANEXT,      1,            0,                                    2,           { ZVARTYPEID_ZMESSAGE, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTile",                ZVARTYPEID_FLOAT,         GETTER,       MESSAGEDATATILE,      1,            0,                                    1,           { ZVARTYPEID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTile",                ZVARTYPEID_VOID,          SETTER,       MESSAGEDATATILE,      1,            0,                                    2,           { ZVARTYPEID_ZMESSAGE, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCSet",                ZVARTYPEID_FLOAT,         GETTER,       MESSAGEDATACSET,      1,            0,                                    1,           { ZVARTYPEID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCSet",                ZVARTYPEID_VOID,          SETTER,       MESSAGEDATACSET,      1,            0,                                    2,           { ZVARTYPEID_ZMESSAGE, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getTransparent",         ZVARTYPEID_BOOL,          GETTER,       MESSAGEDATATRANS,     1,            0,                                    1,           { ZVARTYPEID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setTransparent",         ZVARTYPEID_VOID,          SETTER,       MESSAGEDATATRANS,     1,            0,                                    2,           { ZVARTYPEID_ZMESSAGE, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFont",                ZVARTYPEID_FLOAT,         GETTER,       MESSAGEDATAFONT,      1,            0,                                    1,           { ZVARTYPEID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFont",                ZVARTYPEID_VOID,          SETTER,       MESSAGEDATAFONT,      1,            0,                                    2,           { ZVARTYPEID_ZMESSAGE, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getX",                   ZVARTYPEID_FLOAT,         GETTER,       MESSAGEDATAX,         1,            0,                                    1,           { ZVARTYPEID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setX",                   ZVARTYPEID_VOID,          SETTER,       MESSAGEDATAX,         1,            0,                                    2,           { ZVARTYPEID_ZMESSAGE, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getY",                   ZVARTYPEID_FLOAT,         GETTER,       MESSAGEDATAY,         1,            0,                                    1,           { ZVARTYPEID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setY",                   ZVARTYPEID_VOID,          SETTER,       MESSAGEDATAY,         1,            0,                                    2,           { ZVARTYPEID_ZMESSAGE, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWidth",               ZVARTYPEID_FLOAT,         GETTER,       MESSAGEDATAW,         1,            0,                                    1,           { ZVARTYPEID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWidth",               ZVARTYPEID_VOID,          SETTER,       MESSAGEDATAW,         1,            0,                                    2,           { ZVARTYPEID_ZMESSAGE, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHeight",              ZVARTYPEID_FLOAT,         GETTER,       MESSAGEDATAH,         1,            0,                                    1,           { ZVARTYPEID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHeight",              ZVARTYPEID_VOID,          SETTER,       MESSAGEDATAH,         1,            0,                                    2,           { ZVARTYPEID_ZMESSAGE, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSound",               ZVARTYPEID_FLOAT,         GETTER,       MESSAGEDATASFX,       1,            0,                                    1,           { ZVARTYPEID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSound",               ZVARTYPEID_VOID,          SETTER,       MESSAGEDATASFX,       1,            0,                                    2,           { ZVARTYPEID_ZMESSAGE, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getListPosition",        ZVARTYPEID_FLOAT,         GETTER,       MESSAGEDATALISTPOS,   1,            0,                                    1,           { ZVARTYPEID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setListPosition",        ZVARTYPEID_VOID,          SETTER,       MESSAGEDATALISTPOS,   1,            0,                                    2,           { ZVARTYPEID_ZMESSAGE, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getVSpace",              ZVARTYPEID_FLOAT,         GETTER,       MESSAGEDATAVSPACE,    1,            0,                                    1,           { ZVARTYPEID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setVSpace",              ZVARTYPEID_VOID,          SETTER,       MESSAGEDATAVSPACE,    1,            0,                                    2,           { ZVARTYPEID_ZMESSAGE, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHSpace",              ZVARTYPEID_FLOAT,         GETTER,       MESSAGEDATAHSPACE,    1,            0,                                    1,           { ZVARTYPEID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHSpace",              ZVARTYPEID_VOID,          SETTER,       MESSAGEDATAHSPACE,    1,            0,                                    2,           { ZVARTYPEID_ZMESSAGE, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getFlag",                ZVARTYPEID_FLOAT,         GETTER,       MESSAGEDATAFLAGS,     1,            0,                                    1,           { ZVARTYPEID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setFlag",                ZVARTYPEID_VOID,          SETTER,       MESSAGEDATAFLAGS,     1,            0,                                    2,           { ZVARTYPEID_ZMESSAGE, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlags[]",             ZVARTYPEID_BOOL,          GETTER,       MESSAGEDATAFLAGSARR,  7,            0,                                    2,           { ZVARTYPEID_ZMESSAGE, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlags[]",             ZVARTYPEID_VOID,          SETTER,       MESSAGEDATAFLAGSARR,  7,            0,                                    3,           { ZVARTYPEID_ZMESSAGE, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMargins[]",           ZVARTYPEID_FLOAT,         GETTER,       MESSAGEDATAMARGINS,   4,            0,                                    2,           { ZVARTYPEID_ZMESSAGE, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMargins[]",           ZVARTYPEID_VOID,          SETTER,       MESSAGEDATAMARGINS,   4,            0,                                    3,           { ZVARTYPEID_ZMESSAGE, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPortraitTile",        ZVARTYPEID_FLOAT,         GETTER,       MESSAGEDATAPORTTILE,  1,            0,                                    1,           { ZVARTYPEID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPortraitTile",        ZVARTYPEID_VOID,          SETTER,       MESSAGEDATAPORTTILE,  1,            0,                                    2,           { ZVARTYPEID_ZMESSAGE, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPortraitCSet",        ZVARTYPEID_FLOAT,         GETTER,       MESSAGEDATAPORTCSET,  1,            0,                                    1,           { ZVARTYPEID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPortraitCSet",        ZVARTYPEID_VOID,          SETTER,       MESSAGEDATAPORTCSET,  1,            0,                                    2,           { ZVARTYPEID_ZMESSAGE, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPortraitX",           ZVARTYPEID_FLOAT,         GETTER,       MESSAGEDATAPORTX,     1,            0,                                    1,           { ZVARTYPEID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPortraitX",           ZVARTYPEID_VOID,          SETTER,       MESSAGEDATAPORTX,     1,            0,                                    2,           { ZVARTYPEID_ZMESSAGE, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPortraitY",           ZVARTYPEID_FLOAT,         GETTER,       MESSAGEDATAPORTY,     1,            0,                                    1,           { ZVARTYPEID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPortraitY",           ZVARTYPEID_VOID,          SETTER,       MESSAGEDATAPORTY,     1,            0,                                    2,           { ZVARTYPEID_ZMESSAGE, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPortraitTileWidth",   ZVARTYPEID_FLOAT,         GETTER,       MESSAGEDATAPORTWID,   1,            0,                                    1,           { ZVARTYPEID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPortraitTileWidth",   ZVARTYPEID_VOID,          SETTER,       MESSAGEDATAPORTWID,   1,            0,                                    2,           { ZVARTYPEID_ZMESSAGE, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPortraitTileHeight",  ZVARTYPEID_FLOAT,         GETTER,       MESSAGEDATAPORTHEI,   1,            0,                                    1,           { ZVARTYPEID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPortraitTileHeight",  ZVARTYPEID_VOID,          SETTER,       MESSAGEDATAPORTHEI,   1,            0,                                    2,           { ZVARTYPEID_ZMESSAGE, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "TextHeight",             ZVARTYPEID_FLOAT,         GETTER,       MESSAGEDATATEXTHEI,   1,            0,                                    1,           { ZVARTYPEID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "TextWidth",              ZVARTYPEID_FLOAT,         GETTER,       MESSAGEDATATEXTWID,   1,            0,                                    1,           { ZVARTYPEID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLength",              ZVARTYPEID_FLOAT,         GETTER,       MESSAGEDATATEXTLEN,   1,            0,                                    1,           { ZVARTYPEID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLength",              ZVARTYPEID_VOID,          SETTER,       MESSAGEDATATEXTLEN,   1,            0,                                    2,           { ZVARTYPEID_ZMESSAGE, ZVARTYPEID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "",                       -1,                       -1,           -1,                   -1,           0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

MessageDataSymbols::MessageDataSymbols()
{
    table = MessageDataTable;
    refVar = REFMSGDATA;
}

void MessageDataSymbols::generateCode()
{
    // Get("dest_string[]")
    {
	    Function* function = getFunction("Get", 2); 
	    int32_t label = function->getLabel(); 
	    vector<shared_ptr<Opcode>> code;
	    addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
	    LABELBACK(label);
		POPREF();
	    addOpcode2 (code, new OMessageDataGetStringRegister(new VarArgument(EXP2))); 
	    RETURN(); 
        function->giveCode(code);
    }
    // Set("src_string[]")
    {
	    Function* function = getFunction("Set", 2); 
	    int32_t label = function->getLabel(); 
	    vector<shared_ptr<Opcode>> code; 
	    addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
	    LABELBACK(label); 
		POPREF();
	    addOpcode2 (code, new OMessageDataSetStringRegister(new VarArgument(EXP2))); 
	    RETURN(); 
        function->giveCode(code);
    }
    
    //void TriggerSecret(game, int32_t)
    /*
    {
	    Function* function = getFunction("TriggerSecret", 2);
	    int32_t label = function->getLabel();
	    vector<shared_ptr<Opcode>> code; 
	    addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
	    LABELBACK(label); 
	    addOpcode2 (code, new OMessageDataSetStringRegister(new VarArgument(EXP2))); 
	    RETURN(); 
	    function->giveCode(code); 
    }
    */
}

