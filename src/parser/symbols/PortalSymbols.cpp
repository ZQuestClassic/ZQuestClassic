#include "SymbolDefs.h"

PortalSymbols PortalSymbols::singleton = PortalSymbols();
SavedPortalSymbols SavedPortalSymbols::singleton = SavedPortalSymbols();

static AccessorTable PortalTable[] =
{
//	  name,                    tag,            rettype,   var,     funcFlags,  params,optparams
	{ "getX",                    0,         ZTID_FLOAT,   PORTALX,         0,  { ZTID_PORTAL },{} },
	{ "setX",                    0,          ZTID_VOID,   PORTALX,         0,  { ZTID_PORTAL, ZTID_FLOAT },{} },
	{ "getY",                    0,         ZTID_FLOAT,   PORTALY,         0,  { ZTID_PORTAL },{} },
	{ "setY",                    0,          ZTID_VOID,   PORTALY,         0,  { ZTID_PORTAL, ZTID_FLOAT },{} },
	{ "getDMap",                 0,         ZTID_FLOAT,   PORTALDMAP,      0,  { ZTID_PORTAL },{} },
	{ "setDMap",                 0,          ZTID_VOID,   PORTALDMAP,      0,  { ZTID_PORTAL, ZTID_FLOAT },{} },
	{ "getScreen",               0,         ZTID_FLOAT,   PORTALSCREEN,    0,  { ZTID_PORTAL },{} },
	{ "setScreen",               0,          ZTID_VOID,   PORTALSCREEN,    0,  { ZTID_PORTAL, ZTID_FLOAT },{} },
	{ "getAClk",                 0,         ZTID_FLOAT,   PORTALACLK,      0,  { ZTID_PORTAL },{} },
	{ "setAClk",                 0,          ZTID_VOID,   PORTALACLK,      0,  { ZTID_PORTAL, ZTID_FLOAT },{} },
	{ "getAFrame",               0,         ZTID_FLOAT,   PORTALAFRM,      0,  { ZTID_PORTAL },{} },
	{ "setAFrame",               0,          ZTID_VOID,   PORTALAFRM,      0,  { ZTID_PORTAL, ZTID_FLOAT },{} },
	{ "getOriginalTile",         0,         ZTID_FLOAT,   PORTALOTILE,     0,  { ZTID_PORTAL },{} },
	{ "setOriginalTile",         0,          ZTID_VOID,   PORTALOTILE,     0,  { ZTID_PORTAL, ZTID_FLOAT },{} },
	{ "getASpeed",               0,         ZTID_FLOAT,   PORTALASPD,      0,  { ZTID_PORTAL },{} },
	{ "setASpeed",               0,          ZTID_VOID,   PORTALASPD,      0,  { ZTID_PORTAL, ZTID_FLOAT },{} },
	{ "getFrames",               0,         ZTID_FLOAT,   PORTALFRAMES,    0,  { ZTID_PORTAL },{} },
	{ "setFrames",               0,          ZTID_VOID,   PORTALFRAMES,    0,  { ZTID_PORTAL, ZTID_FLOAT },{} },
	{ "getSavedPortal",          0,     ZTID_SAVPORTAL,   PORTALSAVED,     0,  { ZTID_PORTAL },{} },
	{ "setSavedPortal",          0,          ZTID_VOID,   PORTALSAVED,     0,  { ZTID_PORTAL, ZTID_SAVPORTAL },{} },
	{ "getCloseDisabled",        0,          ZTID_BOOL,   PORTALCLOSEDIS,  0,  { ZTID_PORTAL },{} },
	{ "setCloseDisabled",        0,          ZTID_VOID,   PORTALCLOSEDIS,  0,  { ZTID_PORTAL, ZTID_BOOL },{} },
	{ "getWarpSFX",              0,         ZTID_FLOAT,   PORTALWARPSFX,   0,  { ZTID_PORTAL },{} },
	{ "setWarpSFX",              0,          ZTID_VOID,   PORTALWARPSFX,   0,  { ZTID_PORTAL, ZTID_FLOAT },{} },
	{ "getWarpEffect",           0,         ZTID_FLOAT,   PORTALWARPVFX,   0,  { ZTID_PORTAL },{} },
	{ "setWarpEffect",           0,          ZTID_VOID,   PORTALWARPVFX,   0,  { ZTID_PORTAL, ZTID_FLOAT },{} },
	{ "Remove",                  0,          ZTID_VOID,   -1,              0,  { ZTID_PORTAL },{} },
	{ "UseSprite",               0,          ZTID_VOID,   -1,              0,  { ZTID_PORTAL, ZTID_FLOAT },{} },
	
	{ "",                        0,          ZTID_VOID,   -1,              0,  {},{} }
};
static AccessorTable SavedPortalTable[] =
{
//	  name,                    tag,            rettype,   var,          funcFlags,  params,optparams
	{ "getX",                    0,         ZTID_FLOAT,   SAVEDPORTALX,         0,  { ZTID_SAVPORTAL },{} },
	{ "setX",                    0,          ZTID_VOID,   SAVEDPORTALX,         0,  { ZTID_SAVPORTAL, ZTID_FLOAT },{} },
	{ "getY",                    0,         ZTID_FLOAT,   SAVEDPORTALY,         0,  { ZTID_SAVPORTAL },{} },
	{ "setY",                    0,          ZTID_VOID,   SAVEDPORTALY,         0,  { ZTID_SAVPORTAL, ZTID_FLOAT },{} },
	{ "getSrcDMap",              0,         ZTID_FLOAT,   SAVEDPORTALSRCDMAP,   0,  { ZTID_SAVPORTAL },{} },
	{ "setSrcDMap",              0,          ZTID_VOID,   SAVEDPORTALSRCDMAP,   0,  { ZTID_SAVPORTAL, ZTID_FLOAT },{} },
	{ "getDestDMap",             0,         ZTID_FLOAT,   SAVEDPORTALDESTDMAP,  0,  { ZTID_SAVPORTAL },{} },
	{ "setDestDMap",             0,          ZTID_VOID,   SAVEDPORTALDESTDMAP,  0,  { ZTID_SAVPORTAL, ZTID_FLOAT },{} },
	{ "getSrcScreen",            0,         ZTID_FLOAT,   SAVEDPORTALSRCSCREEN, 0,  { ZTID_SAVPORTAL },{} },
	{ "setSrcScreen",            0,          ZTID_VOID,   SAVEDPORTALSRCSCREEN, 0,  { ZTID_SAVPORTAL, ZTID_FLOAT },{} },
	{ "getDestScreen",           0,         ZTID_FLOAT,   SAVEDPORTALDSTSCREEN, 0,  { ZTID_SAVPORTAL },{} },
	{ "setDestScreen",           0,          ZTID_VOID,   SAVEDPORTALDSTSCREEN, 0,  { ZTID_SAVPORTAL, ZTID_FLOAT },{} },
	{ "getWarpSFX",              0,         ZTID_FLOAT,   SAVEDPORTALWARPSFX,   0,  { ZTID_SAVPORTAL },{} },
	{ "setWarpSFX",              0,          ZTID_VOID,   SAVEDPORTALWARPSFX,   0,  { ZTID_SAVPORTAL, ZTID_FLOAT },{} },
	{ "getWarpEffect",           0,         ZTID_FLOAT,   SAVEDPORTALWARPVFX,   0,  { ZTID_SAVPORTAL },{} },
	{ "setWarpEffect",           0,          ZTID_VOID,   SAVEDPORTALWARPVFX,   0,  { ZTID_SAVPORTAL, ZTID_FLOAT },{} },
	{ "getSprite",               0,         ZTID_FLOAT,   SAVEDPORTALSPRITE,    0,  { ZTID_SAVPORTAL },{} },
	{ "setSprite",               0,          ZTID_VOID,   SAVEDPORTALSPRITE,    0,  { ZTID_SAVPORTAL, ZTID_FLOAT },{} },
	{ "getPortal",               0,        ZTID_PORTAL,   SAVEDPORTALPORTAL,    0,  { ZTID_SAVPORTAL },{} },
	{ "setPortal",               0,          ZTID_VOID,   SAVEDPORTALPORTAL,    0,  { ZTID_SAVPORTAL, ZTID_PORTAL },{} },
	{ "Remove",                  0,          ZTID_VOID,   -1,                   0,  { ZTID_SAVPORTAL },{} },
	{ "Generate",                0,        ZTID_PORTAL,   -1,                   0,  { ZTID_SAVPORTAL },{} },
	
	{ "",                        0,          ZTID_VOID,   -1,                   0,  {},{} }
};
PortalSymbols::PortalSymbols()
{
	table = PortalTable;
	refVar = REFPORTAL;
}
SavedPortalSymbols::SavedPortalSymbols()
{
	table = SavedPortalTable;
	refVar = REFSAVPORTAL;
}

void PortalSymbols::generateCode()
{
	//void Remove(portal)
	{
		Function* function = getFunction("Remove");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		POPREF();
		ASSERT_NON_NUL();
		LABELBACK(label);
		addOpcode2 (code, new OPortalRemove());
		RETURN();
		function->giveCode(code);
	}
	//void UseSprite(portal, int)
	{
		Function* function = getFunction("UseSprite");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the val
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop off the pointer
		POPREF();
		addOpcode2 (code, new OUseSpritePortal(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
}
void SavedPortalSymbols::generateCode()
{
	//void Remove(savedportal)
	{
		Function* function = getFunction("Remove");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		POPREF();
		ASSERT_NON_NUL();
		LABELBACK(label);
		addOpcode2 (code, new OSavedPortalRemove());
		RETURN();
		function->giveCode(code);
	}
	//void Generate(savedportal)
	{
		Function* function = getFunction("Generate");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		POPREF();
		ASSERT_NON_NUL();
		LABELBACK(label);
		addOpcode2 (code, new OSavedPortalGenerate());
		RETURN();
		function->giveCode(code);
	}
}

