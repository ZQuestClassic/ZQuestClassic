#include "SymbolDefs.h"

BottleTypeSymbols BottleTypeSymbols::singleton = BottleTypeSymbols();

static AccessorTable BottleTypeTable[] =
{
//	  name,                    tag,            rettype,  var,    funcFlags,  params,optparams
	{ "GetName",                 0,          ZTID_VOID,   -1,            0,  { ZTID_BOTTLETYPE, ZTID_CHAR },{} },
	{ "SetName",                 0,          ZTID_VOID,   -1,            0,  { ZTID_BOTTLETYPE, ZTID_CHAR },{} },
	{ "getCounter[]",            0,         ZTID_FLOAT,   BOTTLECOUNTER, 0,  { ZTID_BOTTLETYPE, ZTID_FLOAT },{} },
	{ "setCounter[]",            0,          ZTID_VOID,   BOTTLECOUNTER, 0,  { ZTID_BOTTLETYPE, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getAmount[]",             0,         ZTID_FLOAT,   BOTTLEAMOUNT,  0,  { ZTID_BOTTLETYPE, ZTID_FLOAT },{} },
	{ "setAmount[]",             0,          ZTID_VOID,   BOTTLEAMOUNT,  0,  { ZTID_BOTTLETYPE, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getIsPercent[]",          0,          ZTID_BOOL,   BOTTLEPERCENT, 0,  { ZTID_BOTTLETYPE, ZTID_FLOAT },{} },
	{ "setIsPercent[]",          0,          ZTID_VOID,   BOTTLEPERCENT, 0,  { ZTID_BOTTLETYPE, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getFlags[]",              0,          ZTID_BOOL,   BOTTLEFLAGS,   0,  { ZTID_BOTTLETYPE, ZTID_FLOAT },{} },
	{ "setFlags[]",              0,          ZTID_VOID,   BOTTLEFLAGS,   0,  { ZTID_BOTTLETYPE, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getNextType",             0,         ZTID_FLOAT,   BOTTLENEXT,    0,  { ZTID_BOTTLETYPE },{} },
	{ "setNextType",             0,          ZTID_VOID,   BOTTLENEXT,    0,  { ZTID_BOTTLETYPE, ZTID_FLOAT },{} },
	
	{ "",                        0,          ZTID_VOID,   -1,          0,  {},{} }
};

BottleTypeSymbols::BottleTypeSymbols()
{
	table = BottleTypeTable;
	refVar = REFBOTTLETYPE;
}

void BottleTypeSymbols::generateCode()
{
	//void GetName(bottledata, int32_t)
	{
		Function* function = getFunction("GetName");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new OGetBottleName(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void SetName(bottledata, int32_t)
	{
		Function* function = getFunction("SetName");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new OSetBottleName(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
}

