#include "../../precompiled.h"
#include "../Library.h"
#include "../LibraryHelper.h"

#include "../ByteCode.h"
#include "../Scope.h"

using namespace ZScript;
using namespace ZScript::Libraries;

Item const& Item::singleton()
{
	static Item const instance;
	return instance;
}

void Item::addTo(Scope& scope) const
{
	DataTypeSimple const* tVoid = &DataType::ZVOID;
	DataTypeSimple const* tBool = &DataType::BOOL;
	DataTypeSimple const* tFloat = &DataType::FLOAT;
	DataTypeClass const* tGame = &DataType::GAME;
	DataTypeClass const* tDebug = &DataType::DEBUG;
	DataTypeClass const* tScreen = &DataType::SCREEN;
	DataTypeClass const* tAudio = &DataType::AUDIO;
	DataTypeClass const* tLink = &DataType::_LINK;
	DataTypeClass const* tItemClass = &DataType::ITEMCLASS;
	DataTypeClass const* tItem = &DataType::ITEM;
	DataTypeClass const* tNpcClass = &DataType::NPCDATA;
	DataTypeClass const* tNpc = &DataType::NPC;
	DataTypeClass const* tFfc = &DataType::FFC;
	DataTypeClass const* tLWpn = &DataType::LWPN;
	DataTypeClass const* tEWpn = &DataType::EWPN;
	DataType const* tEnd = NULL;
	
	LibraryHelper lh(scope, REFITEM, tItem);

	addPair(lh, ITEMX, tFloat, "X");
	addPair(lh, ITEMY, tFloat, "Y");
	addPair(lh, ITEMZ, tFloat, "Z");
	addPair(lh, ITEMJUMP, tFloat, "Jump");
	addPair(lh, ITEMDRAWTYPE, tFloat, "DrawStyle");
	addPair(lh, ITEMID, tFloat, "ID");
	addPair(lh, ITEMTILE, tFloat, "Tile");
	addPair(lh, ITEMOTILE, tFloat, "OriginalTile");
	addPair(lh, ITEMCSET, tFloat, "CSet");
	addPair(lh, ITEMFLASHCSET, tFloat, "FlashCSet");
	addPair(lh, ITEMFRAMES, tFloat, "NumFrames");
	addPair(lh, ITEMFRAME, tFloat, "Frame");
	addPair(lh, ITEMASPEED, tFloat, "ASpeed");
	addPair(lh, ITEMDELAY, tFloat, "Delay");
	addPair(lh, ITEMFLASH, tBool, "Flash");
	addPair(lh, ITEMFLIP, tFloat, "Flip");
	addPair(lh, ITEMEXTEND, tFloat, "Extend");
	addPair(lh, ITEMHXSZ, tFloat, "HitWidth");
	addPair(lh, ITEMHYSZ, tFloat, "HitHeight");
	addPair(lh, ITEMHZSZ, tFloat, "HitZHeight");
	addPair(lh, ITEMTXSZ, tFloat, "TileWidth");
	addPair(lh, ITEMTYSZ, tFloat, "TileHeight");
	addPair(lh, ITEMXOFS, tFloat, "DrawXOffset");
	addPair(lh, ITEMYOFS, tFloat, "DrawYOffset");
	addPair(lh, ITEMZOFS, tFloat, "DrawZOffset");
	addPair(lh, ITEMHXOFS, tFloat, "HitXOffset");
	addPair(lh, ITEMHYOFS, tFloat, "HitYOffset");
	addPair(lh, ITEMPICKUP, tFloat, "Pickup");
	addPair(lh, ITEMMISCD, tFloat, "Misc", 32);

    // bool Item->isValid()
    {
	    Function& function = lh.addFunction(tBool, "isValid", tEnd);
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop off the pointer
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //Check validity
        code.push_back(new OIsValidItem(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
}

