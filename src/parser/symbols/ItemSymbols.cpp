#include "SymbolDefs.h"

ItemSymbols ItemSymbols::singleton = ItemSymbols();

static AccessorTable itemTable[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
	{ "_getMax",                    0,         ZTID_FLOAT,   SPRITEMAXITEM,             0,  { ZTID_ITEM },{} },
	{ "_setMax",                    0,          ZTID_VOID,   SPRITEMAXITEM,             0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getX",                       0,         ZTID_FLOAT,   ITEMX,                     0,  { ZTID_ITEM },{} },
	{ "setX",                       0,          ZTID_VOID,   ITEMX,                     0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getY",                       0,         ZTID_FLOAT,   ITEMY,                     0,  { ZTID_ITEM },{} },
	{ "setY",                       0,          ZTID_VOID,   ITEMY,                     0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getZ",                       0,         ZTID_FLOAT,   ITEMZ,                     0,  { ZTID_ITEM },{} },
	{ "setZ",                       0,          ZTID_VOID,   ITEMZ,                     0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getJump",                    0,         ZTID_FLOAT,   ITEMJUMP,                  0,  { ZTID_ITEM },{} },
	{ "setJump",                    0,          ZTID_VOID,   ITEMJUMP,                  0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getDrawStyle",               0,         ZTID_FLOAT,   ITEMDRAWTYPE,              0,  { ZTID_ITEM },{} },
	{ "setDrawStyle",               0,          ZTID_VOID,   ITEMDRAWTYPE,              0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getID",                      0,         ZTID_FLOAT,   ITEMID,                    0,  { ZTID_ITEM },{} },
	{ "setID",                      0,          ZTID_VOID,   ITEMID,                    0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getType",                    0,         ZTID_FLOAT,   ITEMFAMILY,                0,  { ZTID_ITEM },{} },
	{ "setType",                    0,          ZTID_VOID,   ITEMFAMILY,                0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getFamily",                  0,         ZTID_FLOAT,   ITEMFAMILY,                0,  { ZTID_ITEM },{} },
	{ "setFamily",                  0,          ZTID_VOID,   ITEMFAMILY,                0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getLevel",                   0,         ZTID_FLOAT,   ITEMLEVEL,                 0,  { ZTID_ITEM },{} },
	{ "setLevel",                   0,          ZTID_VOID,   ITEMLEVEL,                 0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getTile",                    0,         ZTID_FLOAT,   ITEMTILE,                  0,  { ZTID_ITEM },{} },
	{ "setTile",                    0,          ZTID_VOID,   ITEMTILE,                  0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getScriptTile",              0,         ZTID_FLOAT,   ITEMSCRIPTTILE,            0,  { ZTID_ITEM },{} },
	{ "setScriptTile",              0,          ZTID_VOID,   ITEMSCRIPTTILE,            0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getScriptFlip",              0,         ZTID_FLOAT,   ITEMSCRIPTFLIP,            0,  { ZTID_ITEM },{} },
	{ "setScriptFlip",              0,          ZTID_VOID,   ITEMSCRIPTFLIP,            0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getOriginalTile",            0,         ZTID_FLOAT,   ITEMOTILE,                 0,  { ZTID_ITEM },{} },
	{ "setOriginalTile",            0,          ZTID_VOID,   ITEMOTILE,                 0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getCSet",                    0,         ZTID_FLOAT,   ITEMCSET,                  0,  { ZTID_ITEM },{} },
	{ "setCSet",                    0,          ZTID_VOID,   ITEMCSET,                  0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getFlashCSet",               0,         ZTID_FLOAT,   ITEMFLASHCSET,             0,  { ZTID_ITEM },{} },
	{ "setFlashCSet",               0,          ZTID_VOID,   ITEMFLASHCSET,             0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getNumFrames",               0,         ZTID_FLOAT,   ITEMFRAMES,                0,  { ZTID_ITEM },{} },
	{ "setNumFrames",               0,          ZTID_VOID,   ITEMFRAMES,                0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getFrame",                   0,         ZTID_FLOAT,   ITEMFRAME,                 0,  { ZTID_ITEM },{} },
	{ "setFrame",                   0,          ZTID_VOID,   ITEMFRAME,                 0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getASpeed",                  0,         ZTID_FLOAT,   ITEMASPEED,                0,  { ZTID_ITEM },{} },
	{ "setASpeed",                  0,          ZTID_VOID,   ITEMASPEED,                0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getDelay",                   0,         ZTID_FLOAT,   ITEMDELAY,                 0,  { ZTID_ITEM },{} },
	{ "setDelay",                   0,          ZTID_VOID,   ITEMDELAY,                 0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getFlash",                   0,          ZTID_BOOL,   ITEMFLASH,                 0,  { ZTID_ITEM },{} },
	{ "setFlash",                   0,          ZTID_VOID,   ITEMFLASH,                 0,  { ZTID_ITEM, ZTID_BOOL },{} },
	{ "getFlip",                    0,         ZTID_FLOAT,   ITEMFLIP,                  0,  { ZTID_ITEM },{} },
	{ "setFlip",                    0,          ZTID_VOID,   ITEMFLIP,                  0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getExtend",                  0,         ZTID_FLOAT,   ITEMEXTEND,                0,  { ZTID_ITEM },{} },
	{ "setExtend",                  0,          ZTID_VOID,   ITEMEXTEND,                0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getHitWidth",                0,         ZTID_FLOAT,   ITEMHXSZ,                  0,  { ZTID_ITEM },{} },
	{ "setHitWidth",                0,          ZTID_VOID,   ITEMHXSZ,                  0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getHitHeight",               0,         ZTID_FLOAT,   ITEMHYSZ,                  0,  { ZTID_ITEM },{} },
	{ "setHitHeight",               0,          ZTID_VOID,   ITEMHYSZ,                  0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getHitZHeight",              0,         ZTID_FLOAT,   ITEMHZSZ,                  0,  { ZTID_ITEM },{} },
	{ "setHitZHeight",              0,          ZTID_VOID,   ITEMHZSZ,                  0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getTileWidth",               0,         ZTID_FLOAT,   ITEMTXSZ,                  0,  { ZTID_ITEM },{} },
	{ "setTileWidth",               0,          ZTID_VOID,   ITEMTXSZ,                  0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getTileHeight",              0,         ZTID_FLOAT,   ITEMTYSZ,                  0,  { ZTID_ITEM },{} },
	{ "setTileHeight",              0,          ZTID_VOID,   ITEMTYSZ,                  0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getDrawXOffset",             0,         ZTID_FLOAT,   ITEMXOFS,                  0,  { ZTID_ITEM },{} },
	{ "setDrawXOffset",             0,          ZTID_VOID,   ITEMXOFS,                  0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getDrawYOffset",             0,         ZTID_FLOAT,   ITEMYOFS,                  0,  { ZTID_ITEM },{} },
	{ "setDrawYOffset",             0,          ZTID_VOID,   ITEMYOFS,                  0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getDrawZOffset",             0,         ZTID_FLOAT,   ITEMZOFS,                  0,  { ZTID_ITEM },{} },
	{ "setDrawZOffset",             0,          ZTID_VOID,   ITEMZOFS,                  0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getHitXOffset",              0,         ZTID_FLOAT,   ITEMHXOFS,                 0,  { ZTID_ITEM },{} },
	{ "setHitXOffset",              0,          ZTID_VOID,   ITEMHXOFS,                 0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getHitYOffset",              0,         ZTID_FLOAT,   ITEMHYOFS,                 0,  { ZTID_ITEM },{} },
	{ "setHitYOffset",              0,          ZTID_VOID,   ITEMHYOFS,                 0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getPickup",                  0,         ZTID_FLOAT,   ITEMPICKUP,                0,  { ZTID_ITEM },{} },
	{ "setPickup",                  0,          ZTID_VOID,   ITEMPICKUP,                0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "isValid",                    0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_ITEM },{} },
	{ "getMisc[]",                  0,       ZTID_UNTYPED,   ITEMMISCD,                 0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "setMisc[]",                  0,          ZTID_VOID,   ITEMMISCD,                 0,  { ZTID_ITEM, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getInitD[]",                 0,       ZTID_UNTYPED,   ITEMSPRITEINITD,           0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "setInitD[]",                 0,          ZTID_VOID,   ITEMSPRITEINITD,           0,  { ZTID_ITEM, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getAClock",                  0,         ZTID_FLOAT,   ITEMACLK,                  0,  { ZTID_ITEM },{} },
	{ "setAClock",                  0,          ZTID_VOID,   ITEMACLK,                  0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	
	{ "getPickupString",            0,         ZTID_FLOAT,   ITEMPSTRING,               0,  { ZTID_ITEM },{} },
	{ "setPickupString",            0,          ZTID_VOID,   ITEMPSTRING,               0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getPickupStringFlags",       0,         ZTID_FLOAT,   ITEMPSTRINGFLAGS,          0,  { ZTID_ITEM },{} },
	{ "setPickupStringFlags",       0,          ZTID_VOID,   ITEMPSTRINGFLAGS,          0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getSizeFlags",               0,         ZTID_FLOAT,   ITEMOVERRIDEFLAGS,         0,  { ZTID_ITEM },{} },
	{ "setSizeFlags",               0,          ZTID_VOID,   ITEMOVERRIDEFLAGS,         0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "Explode",                    0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getRotation",                0,         ZTID_FLOAT,   ITEMROTATION,              0,  { ZTID_ITEM },{} },
	{ "setRotation",                0,          ZTID_VOID,   ITEMROTATION,              0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getScale",                   0,         ZTID_FLOAT,   ITEMSCALE,                 0,  { ZTID_ITEM },{} },
	{ "setScale",                   0,          ZTID_VOID,   ITEMSCALE,                 0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getScript",                  0,         ZTID_FLOAT,   ITEMSPRITESCRIPT,          0,  { ZTID_ITEM },{} },
	{ "setScript",                  0,          ZTID_VOID,   ITEMSPRITESCRIPT,          0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getGravity",                 0,          ZTID_BOOL,   ITEMGRAVITY,               0,  { ZTID_ITEM },{} },
	{ "setGravity",                 0,          ZTID_VOID,   ITEMGRAVITY,               0,  { ZTID_ITEM, ZTID_BOOL },{} },
	{ "getFalling",                 0,         ZTID_FLOAT,   ITEMFALLCLK,               0,  { ZTID_ITEM },{} },
	{ "setFalling",                 0,          ZTID_VOID,   ITEMFALLCLK,               0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getFallCombo",               0,         ZTID_FLOAT,   ITEMFALLCMB,               0,  { ZTID_ITEM },{} },
	{ "setFallCombo",               0,          ZTID_VOID,   ITEMFALLCMB,               0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getMoveFlags[]",             0,          ZTID_BOOL,   ITEMMOVEFLAGS,             0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "setMoveFlags[]",             0,          ZTID_VOID,   ITEMMOVEFLAGS,             0,  { ZTID_ITEM, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getLightRadius",             0,         ZTID_FLOAT,   ITEMGLOWRAD,               0,  { ZTID_ITEM },{} },
	{ "setLightRadius",             0,          ZTID_VOID,   ITEMGLOWRAD,               0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getLightShape",              0,         ZTID_FLOAT,   ITEMGLOWSHP,               0,  { ZTID_ITEM },{} },
	{ "setLightShape",              0,          ZTID_VOID,   ITEMGLOWSHP,               0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getDir",                     0,         ZTID_FLOAT,   ITEMDIR,                   0,  { ZTID_ITEM },{} },
	{ "setDir",                     0,          ZTID_VOID,   ITEMDIR,                   0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getAnimation",               0,         ZTID_FLOAT,   ITEMENGINEANIMATE,         0,  { ZTID_ITEM },{} },
	{ "setAnimation",               0,          ZTID_VOID,   ITEMENGINEANIMATE,         0,  { ZTID_ITEM, ZTID_BOOL },{} },
	{ "Remove",                     0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_ITEM },{} },
	{ "getShadowSprite",            0,         ZTID_FLOAT,   ITEMSHADOWSPR,             0,  { ZTID_ITEM },{} },
	{ "setShadowSprite",            0,          ZTID_VOID,   ITEMSHADOWSPR,             0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getDroppedBy",               0,         ZTID_FLOAT,   ITEMDROPPEDBY,             0,  { ZTID_ITEM },{} },
	{ "setDroppedBy",               0,          ZTID_VOID,   ITEMDROPPEDBY,             0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getSwitchHooked",            0,          ZTID_BOOL,   ITMSWHOOKED,               0,  { ZTID_ITEM },{} },
	{ "setSwitchHooked",            0,          ZTID_VOID,   ITMSWHOOKED,               0,  { ZTID_ITEM, ZTID_BOOL },{} },
	{ "Switch",                     0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getDrowning",                0,         ZTID_FLOAT,   ITEMDROWNCLK,              0,  { ZTID_ITEM },{} },
	{ "setDrowning",                0,          ZTID_VOID,   ITEMDROWNCLK,              0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getDrownCombo",              0,         ZTID_FLOAT,   ITEMDROWNCMB,              0,  { ZTID_ITEM },{} },
	{ "setDrownCombo",              0,          ZTID_VOID,   ITEMDROWNCMB,              0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getFakeZ",                   0,         ZTID_FLOAT,   ITEMFAKEZ,                 0,  { ZTID_ITEM },{} },
	{ "setFakeZ",                   0,          ZTID_VOID,   ITEMFAKEZ,                 0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getFakeJump",                0,         ZTID_FLOAT,   ITEMFAKEJUMP,              0,  { ZTID_ITEM },{} },
	{ "setFakeJump",                0,          ZTID_VOID,   ITEMFAKEJUMP,              0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getShadowXOffset",           0,         ZTID_FLOAT,   ITEMSHADOWXOFS,            0,  { ZTID_ITEM },{} },
	{ "setShadowXOffset",           0,          ZTID_VOID,   ITEMSHADOWXOFS,            0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getShadowYOffset",           0,         ZTID_FLOAT,   ITEMSHADOWYOFS,            0,  { ZTID_ITEM },{} },
	{ "setShadowYOffset",           0,          ZTID_VOID,   ITEMSHADOWYOFS,            0,  { ZTID_ITEM, ZTID_FLOAT },{} },
	{ "getForceGrab",               0,          ZTID_BOOL,   ITEMFORCEGRAB,             0,  { ZTID_ITEM },{} },
	{ "setForceGrab",               0,          ZTID_VOID,   ITEMFORCEGRAB,             0,  { ZTID_ITEM, ZTID_BOOL },{} },
	
	//Intentionally undocumented
	{ "getUID",                     0,         ZTID_FLOAT,   ITEMSCRIPTUID,       FL_DEPR,  { ZTID_ITEM },{} },
	
	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
};

ItemSymbols::ItemSymbols()
{
	table = itemTable;
	refVar = REFITEM;
}

void ItemSymbols::generateCode()
{
	//bool isValid(item)
	{
		Function* function = getFunction("isValid");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Check validity
		addOpcode2 (code, new OIsValidItem(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void Explode(ITEM, int32_t)
	{
		Function* function = getFunction("Explode");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OItemExplodeRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void Remove(itemsprite)
	{
		Function* function = getFunction("Remove");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		POPREF();
		LABELBACK(label);
		//Break shield
		addOpcode2 (code, new OItemRemove());
		RETURN();
		function->giveCode(code);
	}
	//bool Switch(itemsprite, int)
	{
		Function* function = getFunction("Switch");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSwitchItem(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
}

