#include "SymbolDefs.h"

MessageDataSymbols MessageDataSymbols::singleton = MessageDataSymbols();

static AccessorTable MessageDataTable[] =
{
//	All of these return a function label error when used:
//	  name,                     rettype,                  setorget,     var,                  numindex,     funcFlags,                            numParams,   params
	{ "Get",                    ZTID_VOID,          FUNCTION,     0,                    1,            FUNCFLAG_INLINE,                      2,           { ZTID_ZMESSAGE, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Set",                    ZTID_VOID,          FUNCTION,     0,                    1,            0,                                    2,           { ZTID_ZMESSAGE, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "getNext",                ZTID_FLOAT,         GETTER,       MESSAGEDATANEXT,      1,            0,                                    1,           { ZTID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setNext",                ZTID_VOID,          SETTER,       MESSAGEDATANEXT,      1,            0,                                    2,           { ZTID_ZMESSAGE, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getTile",                ZTID_FLOAT,         GETTER,       MESSAGEDATATILE,      1,            0,                                    1,           { ZTID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setTile",                ZTID_VOID,          SETTER,       MESSAGEDATATILE,      1,            0,                                    2,           { ZTID_ZMESSAGE, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getCSet",                ZTID_FLOAT,         GETTER,       MESSAGEDATACSET,      1,            0,                                    1,           { ZTID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setCSet",                ZTID_VOID,          SETTER,       MESSAGEDATACSET,      1,            0,                                    2,           { ZTID_ZMESSAGE, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getTransparent",         ZTID_BOOL,          GETTER,       MESSAGEDATATRANS,     1,            0,                                    1,           { ZTID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setTransparent",         ZTID_VOID,          SETTER,       MESSAGEDATATRANS,     1,            0,                                    2,           { ZTID_ZMESSAGE, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFont",                ZTID_FLOAT,         GETTER,       MESSAGEDATAFONT,      1,            0,                                    1,           { ZTID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFont",                ZTID_VOID,          SETTER,       MESSAGEDATAFONT,      1,            0,                                    2,           { ZTID_ZMESSAGE, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getX",                   ZTID_FLOAT,         GETTER,       MESSAGEDATAX,         1,            0,                                    1,           { ZTID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setX",                   ZTID_VOID,          SETTER,       MESSAGEDATAX,         1,            0,                                    2,           { ZTID_ZMESSAGE, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getY",                   ZTID_FLOAT,         GETTER,       MESSAGEDATAY,         1,            0,                                    1,           { ZTID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setY",                   ZTID_VOID,          SETTER,       MESSAGEDATAY,         1,            0,                                    2,           { ZTID_ZMESSAGE, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getWidth",               ZTID_FLOAT,         GETTER,       MESSAGEDATAW,         1,            0,                                    1,           { ZTID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setWidth",               ZTID_VOID,          SETTER,       MESSAGEDATAW,         1,            0,                                    2,           { ZTID_ZMESSAGE, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHeight",              ZTID_FLOAT,         GETTER,       MESSAGEDATAH,         1,            0,                                    1,           { ZTID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHeight",              ZTID_VOID,          SETTER,       MESSAGEDATAH,         1,            0,                                    2,           { ZTID_ZMESSAGE, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getSound",               ZTID_FLOAT,         GETTER,       MESSAGEDATASFX,       1,            0,                                    1,           { ZTID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setSound",               ZTID_VOID,          SETTER,       MESSAGEDATASFX,       1,            0,                                    2,           { ZTID_ZMESSAGE, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getListPosition",        ZTID_FLOAT,         GETTER,       MESSAGEDATALISTPOS,   1,            0,                                    1,           { ZTID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setListPosition",        ZTID_VOID,          SETTER,       MESSAGEDATALISTPOS,   1,            0,                                    2,           { ZTID_ZMESSAGE, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getVSpace",              ZTID_FLOAT,         GETTER,       MESSAGEDATAVSPACE,    1,            0,                                    1,           { ZTID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setVSpace",              ZTID_VOID,          SETTER,       MESSAGEDATAVSPACE,    1,            0,                                    2,           { ZTID_ZMESSAGE, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getHSpace",              ZTID_FLOAT,         GETTER,       MESSAGEDATAHSPACE,    1,            0,                                    1,           { ZTID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setHSpace",              ZTID_VOID,          SETTER,       MESSAGEDATAHSPACE,    1,            0,                                    2,           { ZTID_ZMESSAGE, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "getFlag",                ZTID_FLOAT,         GETTER,       MESSAGEDATAFLAGS,     1,            0,                                    1,           { ZTID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setFlag",                ZTID_VOID,          SETTER,       MESSAGEDATAFLAGS,     1,            0,                                    2,           { ZTID_ZMESSAGE, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getFlags[]",             ZTID_BOOL,          GETTER,       MESSAGEDATAFLAGSARR,  7,            0,                                    2,           { ZTID_ZMESSAGE, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setFlags[]",             ZTID_VOID,          SETTER,       MESSAGEDATAFLAGSARR,  7,            0,                                    3,           { ZTID_ZMESSAGE, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getMargins[]",           ZTID_FLOAT,         GETTER,       MESSAGEDATAMARGINS,   4,            0,                                    2,           { ZTID_ZMESSAGE, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setMargins[]",           ZTID_VOID,          SETTER,       MESSAGEDATAMARGINS,   4,            0,                                    3,           { ZTID_ZMESSAGE, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPortraitTile",        ZTID_FLOAT,         GETTER,       MESSAGEDATAPORTTILE,  1,            0,                                    1,           { ZTID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPortraitTile",        ZTID_VOID,          SETTER,       MESSAGEDATAPORTTILE,  1,            0,                                    2,           { ZTID_ZMESSAGE, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPortraitCSet",        ZTID_FLOAT,         GETTER,       MESSAGEDATAPORTCSET,  1,            0,                                    1,           { ZTID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPortraitCSet",        ZTID_VOID,          SETTER,       MESSAGEDATAPORTCSET,  1,            0,                                    2,           { ZTID_ZMESSAGE, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPortraitX",           ZTID_FLOAT,         GETTER,       MESSAGEDATAPORTX,     1,            0,                                    1,           { ZTID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPortraitX",           ZTID_VOID,          SETTER,       MESSAGEDATAPORTX,     1,            0,                                    2,           { ZTID_ZMESSAGE, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPortraitY",           ZTID_FLOAT,         GETTER,       MESSAGEDATAPORTY,     1,            0,                                    1,           { ZTID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPortraitY",           ZTID_VOID,          SETTER,       MESSAGEDATAPORTY,     1,            0,                                    2,           { ZTID_ZMESSAGE, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPortraitTileWidth",   ZTID_FLOAT,         GETTER,       MESSAGEDATAPORTWID,   1,            0,                                    1,           { ZTID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPortraitTileWidth",   ZTID_VOID,          SETTER,       MESSAGEDATAPORTWID,   1,            0,                                    2,           { ZTID_ZMESSAGE, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPortraitTileHeight",  ZTID_FLOAT,         GETTER,       MESSAGEDATAPORTHEI,   1,            0,                                    1,           { ZTID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPortraitTileHeight",  ZTID_VOID,          SETTER,       MESSAGEDATAPORTHEI,   1,            0,                                    2,           { ZTID_ZMESSAGE, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "TextHeight",             ZTID_FLOAT,         GETTER,       MESSAGEDATATEXTHEI,   1,            0,                                    1,           { ZTID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "TextWidth",              ZTID_FLOAT,         GETTER,       MESSAGEDATATEXTWID,   1,            0,                                    1,           { ZTID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getLength",              ZTID_FLOAT,         GETTER,       MESSAGEDATATEXTLEN,   1,            0,                                    1,           { ZTID_ZMESSAGE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setLength",              ZTID_VOID,          SETTER,       MESSAGEDATATEXTLEN,   1,            0,                                    2,           { ZTID_ZMESSAGE, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
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

