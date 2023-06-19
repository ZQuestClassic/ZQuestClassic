#include "SymbolDefs.h"

RegionSymbols RegionSymbols::singleton = RegionSymbols();

static AccessorTable RegionTable[] =
{
	//name,                       tag,            rettype,   var,                       funcFlags,  params,optparams
	{ "getComboD[]",                0,         ZTID_FLOAT,   REGIONDD,                           0,  { ZTID_REGION, ZTID_FLOAT },{} },
	{ "setComboD[]",                0,          ZTID_VOID,   REGIONDD,                           0,  { ZTID_REGION, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getComboC[]",                0,         ZTID_FLOAT,   REGIONCD,                           0,  { ZTID_REGION, ZTID_FLOAT },{} },
	{ "setComboC[]",                0,          ZTID_VOID,   REGIONCD,                           0,  { ZTID_REGION, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getComboF[]",                0,         ZTID_FLOAT,   REGIONFD,                           0,  { ZTID_REGION, ZTID_FLOAT },{} },
	{ "setComboF[]",                0,          ZTID_VOID,   REGIONFD,                           0,  { ZTID_REGION, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getComboI[]",                0,         ZTID_FLOAT,   REGIONID,                           0,  { ZTID_REGION, ZTID_FLOAT },{} },
	{ "setComboI[]",                0,          ZTID_VOID,   REGIONID,                           0,  { ZTID_REGION, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getComboT[]",                0,         ZTID_FLOAT,   REGIONTD,                           0,  { ZTID_REGION, ZTID_FLOAT },{} },
	{ "setComboT[]",                0,          ZTID_VOID,   REGIONTD,                           0,  { ZTID_REGION, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getComboS[]",                0,         ZTID_FLOAT,   REGIONSD,                           0,  { ZTID_REGION, ZTID_FLOAT },{} },
	{ "setComboS[]",                0,          ZTID_VOID,   REGIONSD,                           0,  { ZTID_REGION, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getComboE[]",                0,         ZTID_FLOAT,   REGIONED,                           0,  { ZTID_REGION, ZTID_FLOAT },{} },
	{ "setComboE[]",                0,          ZTID_VOID,   REGIONED,                           0,  { ZTID_REGION, ZTID_FLOAT, ZTID_FLOAT },{} },

	{ "getWorldWidth",              0,          ZTID_LONG,   REGIONWORLDWIDTH,                   0,  { ZTID_REGION }, {} },
	{ "getWorldHeight",             0,          ZTID_LONG,   REGIONWORLDHEIGHT,                  0,  { ZTID_REGION }, {} },
	{ "getScreenWidth",             0,          ZTID_LONG,   REGIONSCREENWIDTH,                  0,  { ZTID_REGION }, {} },
	{ "getScreenHeight",            0,          ZTID_LONG,   REGIONSCREENHEIGHT,                 0,  { ZTID_REGION }, {} },
	{ "getMaxRpos",                 0,          ZTID_LONG,   REGION_MAX_RPOS,                    0,  { ZTID_REGION }, {} },
	{ "getNumRpos",                 0,          ZTID_LONG,   REGION_NUM_RPOS,                    0,  { ZTID_REGION }, {} },
	
	{ "GetScreenIndexForRpos",      0,          ZTID_FLOAT,  -1,                            FL_INL,  { ZTID_REGION, ZTID_FLOAT }, {} },

	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
};

RegionSymbols::RegionSymbols()
{
	table = RegionTable;
	refVar = NUL;
}

void RegionSymbols::generateCode()
{
	//int32_t GetScreenIndexForRpos(region, int32_t)
    {
	    Function* function = getFunction("GetScreenIndexForRpos");
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
        LABELBACK(label);
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OGetScreenIndexForRpos(new VarArgument(EXP1)));
        RETURN();
        function->giveCode(code);
    }
}
