#include "SymbolDefs.h"

MessageDataSymbols MessageDataSymbols::singleton = MessageDataSymbols();

static AccessorTable MessageDataTable[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
	{ "Get",                        0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_ZMESSAGE, ZTID_FLOAT },{} },
	{ "Set",                        0,          ZTID_VOID,   -1,                        0,  { ZTID_ZMESSAGE, ZTID_FLOAT },{} },
	
	{ "getNext",                    0,         ZTID_FLOAT,   MESSAGEDATANEXT,           0,  { ZTID_ZMESSAGE },{} },
	{ "setNext",                    0,          ZTID_VOID,   MESSAGEDATANEXT,           0,  { ZTID_ZMESSAGE, ZTID_FLOAT },{} },
	{ "getTile",                    0,         ZTID_FLOAT,   MESSAGEDATATILE,           0,  { ZTID_ZMESSAGE },{} },
	{ "setTile",                    0,          ZTID_VOID,   MESSAGEDATATILE,           0,  { ZTID_ZMESSAGE, ZTID_FLOAT },{} },
	{ "getCSet",                    0,         ZTID_FLOAT,   MESSAGEDATACSET,           0,  { ZTID_ZMESSAGE },{} },
	{ "setCSet",                    0,          ZTID_VOID,   MESSAGEDATACSET,           0,  { ZTID_ZMESSAGE, ZTID_FLOAT },{} },
	{ "getFont",                    0,         ZTID_FLOAT,   MESSAGEDATAFONT,           0,  { ZTID_ZMESSAGE },{} },
	{ "setFont",                    0,          ZTID_VOID,   MESSAGEDATAFONT,           0,  { ZTID_ZMESSAGE, ZTID_FLOAT },{} },
	{ "getX",                       0,         ZTID_FLOAT,   MESSAGEDATAX,              0,  { ZTID_ZMESSAGE },{} },
	{ "setX",                       0,          ZTID_VOID,   MESSAGEDATAX,              0,  { ZTID_ZMESSAGE, ZTID_FLOAT },{} },
	{ "getY",                       0,         ZTID_FLOAT,   MESSAGEDATAY,              0,  { ZTID_ZMESSAGE },{} },
	{ "setY",                       0,          ZTID_VOID,   MESSAGEDATAY,              0,  { ZTID_ZMESSAGE, ZTID_FLOAT },{} },
	{ "getWidth",                   0,         ZTID_FLOAT,   MESSAGEDATAW,              0,  { ZTID_ZMESSAGE },{} },
	{ "setWidth",                   0,          ZTID_VOID,   MESSAGEDATAW,              0,  { ZTID_ZMESSAGE, ZTID_FLOAT },{} },
	{ "getHeight",                  0,         ZTID_FLOAT,   MESSAGEDATAH,              0,  { ZTID_ZMESSAGE },{} },
	{ "setHeight",                  0,          ZTID_VOID,   MESSAGEDATAH,              0,  { ZTID_ZMESSAGE, ZTID_FLOAT },{} },
	{ "getSound",                   0,         ZTID_FLOAT,   MESSAGEDATASFX,            0,  { ZTID_ZMESSAGE },{} },
	{ "setSound",                   0,          ZTID_VOID,   MESSAGEDATASFX,            0,  { ZTID_ZMESSAGE, ZTID_FLOAT },{} },
	{ "getListPosition",            0,         ZTID_FLOAT,   MESSAGEDATALISTPOS,        0,  { ZTID_ZMESSAGE },{} },
	{ "setListPosition",            0,          ZTID_VOID,   MESSAGEDATALISTPOS,        0,  { ZTID_ZMESSAGE, ZTID_FLOAT },{} },
	{ "getVSpace",                  0,         ZTID_FLOAT,   MESSAGEDATAVSPACE,         0,  { ZTID_ZMESSAGE },{} },
	{ "setVSpace",                  0,          ZTID_VOID,   MESSAGEDATAVSPACE,         0,  { ZTID_ZMESSAGE, ZTID_FLOAT },{} },
	{ "getHSpace",                  0,         ZTID_FLOAT,   MESSAGEDATAHSPACE,         0,  { ZTID_ZMESSAGE },{} },
	{ "setHSpace",                  0,          ZTID_VOID,   MESSAGEDATAHSPACE,         0,  { ZTID_ZMESSAGE, ZTID_FLOAT },{} },
	{ "getFlags[]",                 0,          ZTID_BOOL,   MESSAGEDATAFLAGSARR,       0,  { ZTID_ZMESSAGE, ZTID_FLOAT },{} },
	{ "setFlags[]",                 0,          ZTID_VOID,   MESSAGEDATAFLAGSARR,       0,  { ZTID_ZMESSAGE, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getMargins[]",               0,         ZTID_FLOAT,   MESSAGEDATAMARGINS,        0,  { ZTID_ZMESSAGE, ZTID_FLOAT },{} },
	{ "setMargins[]",               0,          ZTID_VOID,   MESSAGEDATAMARGINS,        0,  { ZTID_ZMESSAGE, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getPortraitTile",            0,         ZTID_FLOAT,   MESSAGEDATAPORTTILE,       0,  { ZTID_ZMESSAGE },{} },
	{ "setPortraitTile",            0,          ZTID_VOID,   MESSAGEDATAPORTTILE,       0,  { ZTID_ZMESSAGE, ZTID_FLOAT },{} },
	{ "getPortraitCSet",            0,         ZTID_FLOAT,   MESSAGEDATAPORTCSET,       0,  { ZTID_ZMESSAGE },{} },
	{ "setPortraitCSet",            0,          ZTID_VOID,   MESSAGEDATAPORTCSET,       0,  { ZTID_ZMESSAGE, ZTID_FLOAT },{} },
	{ "getPortraitX",               0,         ZTID_FLOAT,   MESSAGEDATAPORTX,          0,  { ZTID_ZMESSAGE },{} },
	{ "setPortraitX",               0,          ZTID_VOID,   MESSAGEDATAPORTX,          0,  { ZTID_ZMESSAGE, ZTID_FLOAT },{} },
	{ "getPortraitY",               0,         ZTID_FLOAT,   MESSAGEDATAPORTY,          0,  { ZTID_ZMESSAGE },{} },
	{ "setPortraitY",               0,          ZTID_VOID,   MESSAGEDATAPORTY,          0,  { ZTID_ZMESSAGE, ZTID_FLOAT },{} },
	{ "getPortraitTileWidth",       0,         ZTID_FLOAT,   MESSAGEDATAPORTWID,        0,  { ZTID_ZMESSAGE },{} },
	{ "setPortraitTileWidth",       0,          ZTID_VOID,   MESSAGEDATAPORTWID,        0,  { ZTID_ZMESSAGE, ZTID_FLOAT },{} },
	{ "getPortraitTileHeight",      0,         ZTID_FLOAT,   MESSAGEDATAPORTHEI,        0,  { ZTID_ZMESSAGE },{} },
	{ "setPortraitTileHeight",      0,          ZTID_VOID,   MESSAGEDATAPORTHEI,        0,  { ZTID_ZMESSAGE, ZTID_FLOAT },{} },
	{ "_getTextHeight",             0,         ZTID_FLOAT,   MESSAGEDATATEXTHEI,        0,  { ZTID_ZMESSAGE },{} },
	{ "_getTextWidth",              0,         ZTID_FLOAT,   MESSAGEDATATEXTWID,        0,  { ZTID_ZMESSAGE },{} },
	{ "getLength",                  0,         ZTID_FLOAT,   MESSAGEDATATEXTLEN,        0,  { ZTID_ZMESSAGE },{} },
	{ "setLength",                  0,          ZTID_VOID,   MESSAGEDATATEXTLEN,        0,  { ZTID_ZMESSAGE, ZTID_FLOAT },{} },
	
	{ "getTransparent",             0,          ZTID_BOOL,   MESSAGEDATATRANS,    FL_DEPR,  { ZTID_ZMESSAGE },{},0,"Unused! Does nothing!" },
	{ "setTransparent",             0,          ZTID_VOID,   MESSAGEDATATRANS,    FL_DEPR,  { ZTID_ZMESSAGE, ZTID_BOOL },{},0,"Unused! Does nothing!" },
	{ "getFlag",                    0,         ZTID_FLOAT,   MESSAGEDATAFLAGS,    FL_DEPR,  { ZTID_ZMESSAGE },{},0,"Use '->Flags[]' instead!" },
	{ "setFlag",                    0,          ZTID_VOID,   MESSAGEDATAFLAGS,    FL_DEPR,  { ZTID_ZMESSAGE, ZTID_FLOAT },{},0,"Use '->Flags[]' instead!" },
	
	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
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
	    Function* function = getFunction("Get"); 
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
	    Function* function = getFunction("Set"); 
	    int32_t label = function->getLabel(); 
	    vector<shared_ptr<Opcode>> code; 
	    addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
	    LABELBACK(label); 
		POPREF();
	    addOpcode2 (code, new OMessageDataSetStringRegister(new VarArgument(EXP2))); 
	    RETURN(); 
        function->giveCode(code);
    }
}

