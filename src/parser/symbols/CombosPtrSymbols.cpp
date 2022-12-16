#include "SymbolDefs.h"

CombosPtrSymbols CombosPtrSymbols::singleton = CombosPtrSymbols();

static AccessorTable CombosTable[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
	//Only as 'this->' in combo scripts
	{ "getX",                       0,         ZTID_FLOAT,   COMBOXR,                   0,  { ZTID_COMBOS },{} },
	{ "getY",                       0,         ZTID_FLOAT,   COMBOYR,                   0,  { ZTID_COMBOS },{} },
	{ "getPos",                     0,         ZTID_FLOAT,   COMBOPOSR,                 0,  { ZTID_COMBOS },{} },
	{ "getLayer",                   0,         ZTID_FLOAT,   COMBOLAYERR,               0,  { ZTID_COMBOS },{} },
	//
	{ "getInitD[]",                 0,       ZTID_UNTYPED,   COMBODATAINITD,            0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "setInitD[]",                 0,          ZTID_VOID,   COMBODATAINITD,            0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getID",                      0,         ZTID_FLOAT,   COMBODATAID,               0,  { ZTID_COMBOS },{} },
	{ "getOriginalTile",            0,         ZTID_FLOAT,   COMBODOTILE,               0,  { ZTID_COMBOS },{} },
	{ "setOriginalTile",            0,          ZTID_VOID,   COMBODOTILE,               0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getFrame",                   0,         ZTID_FLOAT,   COMBODFRAME,               0,  { ZTID_COMBOS },{} },
	{ "setFrame",                   0,          ZTID_VOID,   COMBODFRAME,               0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getAClk",                    0,         ZTID_FLOAT,   COMBODACLK,                0,  { ZTID_COMBOS },{} },
	{ "setAClk",                    0,          ZTID_VOID,   COMBODACLK,                0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTile",                    0,         ZTID_FLOAT,   COMBODTILE,                0,  { ZTID_COMBOS },{} },
	{ "setTile",                    0,          ZTID_VOID,   COMBODTILE,                0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getScript",                  0,         ZTID_FLOAT,   COMBODATASCRIPT,           0,  { ZTID_COMBOS },{} },
	{ "setScript",                  0,          ZTID_VOID,   COMBODATASCRIPT,           0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getASpeed",                  0,         ZTID_FLOAT,   COMBODASPEED,              0,  { ZTID_COMBOS },{} },
	{ "setASpeed",                  0,          ZTID_VOID,   COMBODASPEED,              0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getFlip",                    0,         ZTID_FLOAT,   COMBODFLIP,                0,  { ZTID_COMBOS },{} },
	{ "setFlip",                    0,          ZTID_VOID,   COMBODFLIP,                0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getWalk",                    0,         ZTID_FLOAT,   COMBODWALK,                0,  { ZTID_COMBOS },{} },
	{ "setWalk",                    0,          ZTID_VOID,   COMBODWALK,                0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getEffect",                  0,         ZTID_FLOAT,   COMBODEFFECT,              0,  { ZTID_COMBOS },{} },
	{ "setEffect",                  0,          ZTID_VOID,   COMBODEFFECT,              0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getType",                    0,         ZTID_FLOAT,   COMBODTYPE,                0,  { ZTID_COMBOS },{} },
	{ "setType",                    0,          ZTID_VOID,   COMBODTYPE,                0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getCSet",                    0,         ZTID_FLOAT,   COMBODCSET,                0,  { ZTID_COMBOS },{} },
	{ "setCSet",                    0,          ZTID_VOID,   COMBODCSET,                0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getCSet2",                   0,         ZTID_FLOAT,   COMBODCSET,                0,  { ZTID_COMBOS },{} },
	{ "setCSet2",                   0,          ZTID_VOID,   COMBODCSET,                0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getCSet2Flags",              0,         ZTID_FLOAT,   COMBODCSET2FLAGS,          0,  { ZTID_COMBOS },{} },
	{ "setCSet2Flags",              0,          ZTID_VOID,   COMBODCSET2FLAGS,          0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getFoo",                     0,         ZTID_FLOAT,   COMBODFOO,                 0,  { ZTID_COMBOS },{} },
	{ "setFoo",                     0,          ZTID_VOID,   COMBODFOO,                 0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getFrames",                  0,         ZTID_FLOAT,   COMBODFRAMES,              0,  { ZTID_COMBOS },{} },
	{ "setFrames",                  0,          ZTID_VOID,   COMBODFRAMES,              0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getNextData",                0,         ZTID_FLOAT,   COMBODNEXTD,               0,  { ZTID_COMBOS },{} },
	{ "setNextData",                0,          ZTID_VOID,   COMBODNEXTD,               0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getNextCSet",                0,         ZTID_FLOAT,   COMBODNEXTC,               0,  { ZTID_COMBOS },{} },
	{ "setNextCSet",                0,          ZTID_VOID,   COMBODNEXTC,               0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getFlag",                    0,         ZTID_FLOAT,   COMBODFLAG,                0,  { ZTID_COMBOS },{} },
	{ "setFlag",                    0,          ZTID_VOID,   COMBODFLAG,                0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getSkipAnim",                0,         ZTID_FLOAT,   COMBODSKIPANIM,            0,  { ZTID_COMBOS },{} },
	{ "setSkipAnim",                0,          ZTID_VOID,   COMBODSKIPANIM,            0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getNextTimer",               0,         ZTID_FLOAT,   COMBODNEXTTIMER,           0,  { ZTID_COMBOS },{} },
	{ "setNextTimer",               0,          ZTID_VOID,   COMBODNEXTTIMER,           0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getSkipAnimY",               0,         ZTID_FLOAT,   COMBODAKIMANIMY,           0,  { ZTID_COMBOS },{} },
	{ "setSkipAnimY",               0,          ZTID_VOID,   COMBODAKIMANIMY,           0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getAnimFlags",               0,         ZTID_FLOAT,   COMBODANIMFLAGS,           0,  { ZTID_COMBOS },{} },
	{ "setAnimFlags",               0,          ZTID_VOID,   COMBODANIMFLAGS,           0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getExpansion[]",             0,         ZTID_FLOAT,   COMBODEXPANSION,           0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "setExpansion[]",             0,          ZTID_VOID,   COMBODEXPANSION,           0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getAttributes[]",            0,         ZTID_FLOAT,   COMBODATTRIBUTES,          0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "setAttributes[]",            0,          ZTID_VOID,   COMBODATTRIBUTES,          0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getAttribytes[]",            0,         ZTID_FLOAT,   COMBODATTRIBYTES,          0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "setAttribytes[]",            0,          ZTID_VOID,   COMBODATTRIBYTES,          0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getAttrishorts[]",           0,         ZTID_FLOAT,   COMBODATTRISHORTS,         0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "setAttrishorts[]",           0,          ZTID_VOID,   COMBODATTRISHORTS,         0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getTriggerFlags[]",          0,         ZTID_FLOAT,   COMBODTRIGGERFLAGS,        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "setTriggerFlags[]",          0,          ZTID_VOID,   COMBODTRIGGERFLAGS,        0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getTrigFlags[]",             0,          ZTID_BOOL,   COMBODTRIGGERFLAGS2,       0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "setTrigFlags[]",             0,          ZTID_VOID,   COMBODTRIGGERFLAGS2,       0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getTriggerButton[]",         0,          ZTID_BOOL,   COMBODTRIGGERBUTTON,       0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "setTriggerButton[]",         0,          ZTID_VOID,   COMBODTRIGGERBUTTON,       0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getUserFlags",               0,         ZTID_FLOAT,   COMBODUSRFLAGS,            0,  { ZTID_COMBOS },{} },
	{ "setUserFlags",               0,          ZTID_VOID,   COMBODUSRFLAGS,            0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getFlags[]",                 0,          ZTID_BOOL,   COMBODUSRFLAGARR,          0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "setFlags[]",                 0,          ZTID_VOID,   COMBODUSRFLAGARR,          0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getGenFlags[]",              0,          ZTID_BOOL,   COMBODGENFLAGARR,          0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "setGenFlags[]",              0,          ZTID_VOID,   COMBODGENFLAGARR,          0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getTriggerLevel",            0,         ZTID_FLOAT,   COMBODTRIGGERLEVEL,        0,  { ZTID_COMBOS },{} },
	{ "setTriggerLevel",            0,          ZTID_VOID,   COMBODTRIGGERLEVEL,        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTriggerItem",             0,         ZTID_FLOAT,   COMBODTRIGGERITEM,         0,  { ZTID_COMBOS },{} },
	{ "setTriggerItem",             0,          ZTID_VOID,   COMBODTRIGGERITEM,         0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTriggerTimer",            0,         ZTID_FLOAT,   COMBODTRIGGERTIMER,        0,  { ZTID_COMBOS },{} },
	{ "setTriggerTimer",            0,          ZTID_VOID,   COMBODTRIGGERTIMER,        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTriggerSFX",              0,         ZTID_FLOAT,   COMBODTRIGGERSFX,          0,  { ZTID_COMBOS },{} },
	{ "setTriggerSFX",              0,          ZTID_VOID,   COMBODTRIGGERSFX,          0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTriggerChange",           0,         ZTID_FLOAT,   COMBODTRIGGERCHANGECMB,    0,  { ZTID_COMBOS },{} },
	{ "setTriggerChange",           0,          ZTID_VOID,   COMBODTRIGGERCHANGECMB,    0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTriggerProximity",        0,         ZTID_FLOAT,   COMBODTRIGGERPROX,         0,  { ZTID_COMBOS },{} },
	{ "setTriggerProximity",        0,          ZTID_VOID,   COMBODTRIGGERPROX,         0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTriggerLightBeam",        0,         ZTID_FLOAT,   COMBODTRIGGERLIGHTBEAM,    0,  { ZTID_COMBOS },{} },
	{ "setTriggerLightBeam",        0,          ZTID_VOID,   COMBODTRIGGERLIGHTBEAM,    0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTriggerCounter",          0,         ZTID_FLOAT,   COMBODTRIGGERCTR,          0,  { ZTID_COMBOS },{} },
	{ "setTriggerCounter",          0,          ZTID_VOID,   COMBODTRIGGERCTR,          0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTriggerCtrAmount",        0,         ZTID_FLOAT,   COMBODTRIGGERCTRAMNT,      0,  { ZTID_COMBOS },{} },
	{ "setTriggerCtrAmount",        0,          ZTID_VOID,   COMBODTRIGGERCTRAMNT,      0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTrigCooldown",            0,         ZTID_FLOAT,   COMBODTRIGGERCOOLDOWN,     0,  { ZTID_COMBOS },{} },
	{ "setTrigCooldown",            0,          ZTID_VOID,   COMBODTRIGGERCOOLDOWN,     0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTrigCopycat",             0,         ZTID_FLOAT,   COMBODTRIGGERCOPYCAT,      0,  { ZTID_COMBOS },{} },
	{ "setTrigCopycat",             0,          ZTID_VOID,   COMBODTRIGGERCOPYCAT,      0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTrigSpawnItemPickup",     0,         ZTID_FLOAT,   COMBODTRIGITEMPICKUP,      0,  { ZTID_COMBOS },{} },
	{ "setTrigSpawnItemPickup",     0,          ZTID_VOID,   COMBODTRIGITEMPICKUP,      0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTrigExState",             0,         ZTID_FLOAT,   COMBODTRIGEXSTATE,         0,  { ZTID_COMBOS },{} },
	{ "setTrigExState",             0,          ZTID_VOID,   COMBODTRIGEXSTATE,         0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTrigSpawnEnemy",          0,         ZTID_FLOAT,   COMBODTRIGSPAWNENEMY,      0,  { ZTID_COMBOS },{} },
	{ "setTrigSpawnEnemy",          0,          ZTID_VOID,   COMBODTRIGSPAWNENEMY,      0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTrigSpawnItem",           0,         ZTID_FLOAT,   COMBODTRIGSPAWNITEM,       0,  { ZTID_COMBOS },{} },
	{ "setTrigSpawnItem",           0,          ZTID_VOID,   COMBODTRIGSPAWNITEM,       0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getTrigCSetChange",          0,         ZTID_FLOAT,   COMBODTRIGCSETCHANGE,      0,  { ZTID_COMBOS },{} },
	{ "setTrigCSetChange",          0,          ZTID_VOID,   COMBODTRIGCSETCHANGE,      0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getLiftGFXCombo",            0,         ZTID_FLOAT,   COMBODLIFTGFXCOMBO,        0,  { ZTID_COMBOS },{} },
	{ "setLiftGFXCombo",            0,          ZTID_VOID,   COMBODLIFTGFXCOMBO,        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getLiftGFXCSet",             0,         ZTID_FLOAT,   COMBODLIFTGFXCCSET,        0,  { ZTID_COMBOS },{} },
	{ "setLiftGFXCSet",             0,          ZTID_VOID,   COMBODLIFTGFXCCSET,        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getLiftUnderCombo",          0,         ZTID_FLOAT,   COMBODLIFTUNDERCMB,        0,  { ZTID_COMBOS },{} },
	{ "setLiftUnderCombo",          0,          ZTID_VOID,   COMBODLIFTUNDERCMB,        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getLiftUnderCSet",           0,         ZTID_FLOAT,   COMBODLIFTUNDERCS,         0,  { ZTID_COMBOS },{} },
	{ "setLiftUnderCSet",           0,          ZTID_VOID,   COMBODLIFTUNDERCS,         0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getLiftDamage",              0,         ZTID_FLOAT,   COMBODLIFTDAMAGE,          0,  { ZTID_COMBOS },{} },
	{ "setLiftDamage",              0,          ZTID_VOID,   COMBODLIFTDAMAGE,          0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getLiftLevel",               0,         ZTID_FLOAT,   COMBODLIFTLEVEL,           0,  { ZTID_COMBOS },{} },
	{ "setLiftLevel",               0,          ZTID_VOID,   COMBODLIFTLEVEL,           0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getLiftItem",                0,         ZTID_FLOAT,   COMBODLIFTITEM,            0,  { ZTID_COMBOS },{} },
	{ "setLiftItem",                0,          ZTID_VOID,   COMBODLIFTITEM,            0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getLiftFlags[]",             0,          ZTID_BOOL,   COMBODLIFTFLAGS,           0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "setLiftFlags[]",             0,          ZTID_VOID,   COMBODLIFTFLAGS,           0,  { ZTID_COMBOS, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getLiftGFXType",             0,         ZTID_FLOAT,   COMBODLIFTGFXTYPE,         0,  { ZTID_COMBOS },{} },
	{ "setLiftGFXType",             0,          ZTID_VOID,   COMBODLIFTGFXTYPE,         0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getLiftGFXSprite",           0,         ZTID_FLOAT,   COMBODLIFTGFXSPRITE,       0,  { ZTID_COMBOS },{} },
	{ "setLiftGFXSprite",           0,          ZTID_VOID,   COMBODLIFTGFXSPRITE,       0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getLiftSFX",                 0,         ZTID_FLOAT,   COMBODLIFTSFX,             0,  { ZTID_COMBOS },{} },
	{ "setLiftSFX",                 0,          ZTID_VOID,   COMBODLIFTSFX,             0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getLiftBreakSprite",         0,         ZTID_FLOAT,   COMBODLIFTBREAKSPRITE,     0,  { ZTID_COMBOS },{} },
	{ "setLiftBreakSprite",         0,          ZTID_VOID,   COMBODLIFTBREAKSPRITE,     0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getLiftBreakSFX",            0,         ZTID_FLOAT,   COMBODLIFTBREAKSFX,        0,  { ZTID_COMBOS },{} },
	{ "setLiftBreakSFX",            0,          ZTID_VOID,   COMBODLIFTBREAKSFX,        0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getLiftHeight",              0,         ZTID_FLOAT,   COMBODLIFTHEIGHT,          0,  { ZTID_COMBOS },{} },
	{ "setLiftHeight",              0,          ZTID_VOID,   COMBODLIFTHEIGHT,          0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	{ "getLiftTime",                0,         ZTID_FLOAT,   COMBODLIFTTIME,            0,  { ZTID_COMBOS },{} },
	{ "setLiftTime",                0,          ZTID_VOID,   COMBODLIFTTIME,            0,  { ZTID_COMBOS, ZTID_FLOAT },{} },
	
	//Undocumented intentionally
	{ "_getPosX",                   0,         ZTID_FLOAT,   COMBOXR,                   0,  { ZTID_COMBOS },{} },
	{ "_getPosY",                   0,         ZTID_FLOAT,   COMBOYR,                   0,  { ZTID_COMBOS },{} },
	{ "_getPos",                    0,         ZTID_FLOAT,   COMBOPOSR,                 0,  { ZTID_COMBOS },{} },
	{ "_getLayer",                  0,         ZTID_FLOAT,   COMBOLAYERR,               0,  { ZTID_COMBOS },{} },
	
	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
};

CombosPtrSymbols::CombosPtrSymbols()
{
	table = CombosTable;
	refVar = REFCOMBODATA; //NUL;
}

void CombosPtrSymbols::generateCode()
{
	{
		Function* function = getFunction("GetBlockEnemies");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataBlockEnemy(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetBlockHole");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataBlockHole(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetBlockTrigger");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataBlockTrig(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetConveyorSpeedX");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataConveyX(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetConveyorSpeedY");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataConveyY(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetCreateEnemy");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataCreateNPC(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetCreateEnemyWhen");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataCreateEnemW(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetCreateEnemyChnge");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataCreateEnemC(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetDirChangeType");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataDirch(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetDistanceChangeTiles");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataDistTiles(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetDiveItem");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataDiveItem(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetDock");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataDock(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetFairy");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataFairy(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetFFComboChangeAttrib");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataAttrib(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetFootDecorationsTile");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataDecoTile(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetFootDecorationsType");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataDecoType(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetHookshotGrab");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataHookshotGrab(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetLadderPass");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataLadderPass(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetLockBlockType");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataLockBlock(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetLockBlockChange");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataLockBlockChange(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetMagicMirror");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataMagicMirror(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetModifyHPAmount");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataModHP(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetModifyHPDelay");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataModHPDelay(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetModifyHPType");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataModHpType(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetModifyMPAmount");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataModMP(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetModifyMPDelay");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataMpdMPDelay(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetModifyMPType");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataModMPType(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetNoPushBlocks");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataNoPush(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetOverhead");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataOverhead(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetPlaceEnemy");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataEnemyLoc(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetPushDirection");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataPushDir(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetPushWeight");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataPushWeight(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetPushWait");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataPushWait(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetPushed");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataPushed(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetRaft");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataRaft(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetResetRoom");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataResetRoom(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetSavePoint");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataSavePoint(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetScreenFreeze");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataFreeezeScreen(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetSecretCombo");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataSecretCombo(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetSingular");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataSingular(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetSlowMove");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataSlowMove(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetStatue");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataStatue(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetStepType");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataStepType(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetStepChangeTo");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataSteoChange(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetStrikeRemnants");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataStrikeRem(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetStrikeRemnantsType");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataStrikeRemType(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetStrikeChange");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataStrikeChange(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetStrikeItem");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataStrikeChangeItem(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetTouchItem");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataTouchItem(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetTouchStairs");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataTouchStairs(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetTriggerType");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataTriggerType(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetTriggerSens");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataTriggerSens(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetWarpType");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataWarpType(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetWarpSens");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataWarpSens(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetWarpDirect");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataWarpDirect(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetWarpLocation");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataWarpLoc(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetWater");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataWater(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetWhistle");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataWhistle(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetWinGame");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataWinGame(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetBlockWeaponLevel");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataWeapBlockLevel(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetTile");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataTile(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetFlip");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataFlip(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetWalkability");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataWalkability(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetType");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataType(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetCSets");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataCSets(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetFoo");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataFoo(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetFrames");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataFrames(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetSpeed");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataSpeed(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetNextCombo");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataNext(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetNextCSet");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataNextCSet(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetFlag");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataFlag(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetSkipAnim");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCSetDataSkipAnim(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetNextTimer");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataTimer(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetSkipAnimY");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataAnimY(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("GetAnimFlags");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		POPREF();
		addOpcode2 (code, new OCDataAnimFlags(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);;
	}
	{
		Function* function = getFunction("SetBlockEnemies");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataBlockEnemy(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetBlockHole");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataBlockHole(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetBlockTrigger");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataBlockTrig(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetConveyorSpeedX");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataConveyX(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetConveyorSpeedY");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataConveyY(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetCreateEnemy");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataCreateNPC(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetCreateEnemyWhen");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataCreateEnemW(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetCreateEnemyChnge");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataCreateEnemC(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetDirChangeType");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataDirch(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetDistanceChangeTiles");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataDistTiles(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetDiveItem");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataDiveItem(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetDock");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataDock(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetFairy");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataFairy(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetFFComboChangeAttrib");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataAttrib(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetFootDecorationsTile");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataDecoTile(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetFootDecorationsType");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataDecoType(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetHookshotGrab");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataHookshotGrab(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetLadderPass");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataLadderPass(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetLockBlockType");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataLockBlock(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetLockBlockChange");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataLockBlockChange(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetMagicMirror");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataMagicMirror(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetModifyHPAmount");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataModHP(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetModifyHPDelay");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataModHPDelay(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetModifyHPType");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataModHpType(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetModifyMPAmount");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataModMP(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetModifyMPDelay");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataMpdMPDelay(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetModifyMPType");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataModMPType(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetNoPushBlocks");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataNoPush(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetOverhead");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataOverhead(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetPlaceEnemy");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataEnemyLoc(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetPushDirection");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataPushDir(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetPushWeight");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataPushWeight(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetPushWait");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataPushWait(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetPushed");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataPushed(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetRaft");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataRaft(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetResetRoom");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataResetRoom(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetSavePoint");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataSavePoint(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetScreenFreeze");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataFreeezeScreen(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetSecretCombo");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataSecretCombo(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetSingular");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataSingular(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetSlowMove");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataSlowMove(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetStatue");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataStatue(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetStepType");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataStepType(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetStepChangeTo");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataSteoChange(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetStrikeRemnants");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataStrikeRem(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetStrikeRemnantsType");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataStrikeRemType(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetStrikeChange");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataStrikeChange(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetStrikeItem");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataStrikeChangeItem(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetTouchItem");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataTouchItem(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetTouchStairs");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataTouchStairs(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetTriggerType");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataTriggerType(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetTriggerSens");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataTriggerSens(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetWarpType");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataWarpType(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetWarpSens");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataWarpSens(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetWarpDirect");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataWarpDirect(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetWarpLocation");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataWarpLoc(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetWater");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataWater(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetWhistle");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataWhistle(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetWinGame");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataWinGame(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetBlockWeaponLevel");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataWeapBlockLevel(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetTile");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataTile(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetFlip");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataFlip(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetWalkability");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataWalkability(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetType");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataType(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetCSets");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataCSets(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetFoo");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataFoo(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetFrames");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataFrames(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetSpeed");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataSpeed(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetNextCombo");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataNext(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetNextCSet");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataNextCSet(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetFlag");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataFlag(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetSkipAnim");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataSkipAnim(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetNextTimer");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataTimer(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetSkipAnimY");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataAnimY(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetAnimFlags");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		addOpcode2 (code, new OCSetDataAnimFlags(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("GetBlockWeapon");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		POPREF();
		addOpcode2 (code, new OCDataBlockWeapon(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("GetExpansion");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		POPREF();
		addOpcode2 (code, new OCDataExpansion(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("GetStrikeWeapons");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		POPREF();
		addOpcode2 (code, new OCDataStrikeWeapon(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetBlockWeapon");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		POPREF();
		addOpcode2 (code, new OSetRegister(new VarArgument(SCDBLOCKWEAPON), new VarArgument(SFTEMP)));
		RETURN();
		function->giveCode(code);
	}
	{
		Function* function = getFunction("SetStrikeWeapons");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		POPREF();
		addOpcode2 (code, new OSetRegister(new VarArgument(SCDSTRIKEWEAPONS), new VarArgument(SFTEMP)));
		RETURN();
		function->giveCode(code);
	}
}

