#include "SymbolDefs.h"

BottleShopSymbols BottleShopSymbols::singleton = BottleShopSymbols();

static AccessorTable BottleShopTable[] =
{
//	  name,                    tag,            rettype,  var,  funcFlags,  params,optparams
	{ "GetName",                 0,          ZTID_VOID,   -1,          0,  { ZTID_BOTTLESHOP, ZTID_CHAR },{} },
	{ "SetName",                 0,          ZTID_VOID,   -1,          0,  { ZTID_BOTTLESHOP, ZTID_CHAR },{} },
	{ "getFill[]",               0,         ZTID_FLOAT,   BSHOPFILL,   0,  { ZTID_BOTTLESHOP, ZTID_FLOAT },{} },
	{ "setFill[]",               0,          ZTID_VOID,   BSHOPFILL,   0,  { ZTID_BOTTLESHOP, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getCombo[]",              0,         ZTID_FLOAT,   BSHOPCOMBO,  0,  { ZTID_BOTTLESHOP, ZTID_FLOAT },{} },
	{ "setCombo[]",              0,          ZTID_VOID,   BSHOPCOMBO,  0,  { ZTID_BOTTLESHOP, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getCSet[]",               0,         ZTID_FLOAT,   BSHOPCSET,   0,  { ZTID_BOTTLESHOP, ZTID_FLOAT },{} },
	{ "setCSet[]",               0,          ZTID_VOID,   BSHOPCSET,   0,  { ZTID_BOTTLESHOP, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getPrice[]",              0,         ZTID_FLOAT,   BSHOPPRICE,  0,  { ZTID_BOTTLESHOP, ZTID_FLOAT },{} },
	{ "setPrice[]",              0,          ZTID_VOID,   BSHOPPRICE,  0,  { ZTID_BOTTLESHOP, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getInfoString[]",         0,         ZTID_FLOAT,   BSHOPSTR,    0,  { ZTID_BOTTLESHOP, ZTID_FLOAT },{} },
	{ "setInfoString[]",         0,          ZTID_VOID,   BSHOPSTR,    0,  { ZTID_BOTTLESHOP, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "",                        0,          ZTID_VOID,   -1,          0,  {},{} }
};

BottleShopSymbols::BottleShopSymbols()
{
	table = BottleShopTable;
	refVar = REFBOTTLESHOP;
}

void BottleShopSymbols::generateCode()
{
	//void GetName(bottleshopdata, char)
	{
		Function* function = getFunction("GetName");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new OGetBottleShopName(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void SetName(bottleshopdata, char)
	{
		Function* function = getFunction("SetName");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new OSetBottleShopName(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
}

