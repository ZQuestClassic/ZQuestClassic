#include "../../precompiled.h"
#include "../Library.h"
#include "../LibraryHelper.h"

#include "../ByteCode.h"
#include "../Scope.h"

using namespace ZScript;
using namespace ZScript::Libraries;

LWeapon const& LWeapon::singleton()
{
	static LWeapon const instance;
	return instance;
}

void LWeapon::addTo(Scope& scope) const
{
	TypeStore& typeStore = scope.getTypeStore();
	DataType tVoid = typeStore.getVoid();
	DataType tBool = typeStore.getBool();
	DataType tFloat = typeStore.getFloat();
	DataType tGame = typeStore.getGame();
	DataType tDebug = typeStore.getDebug();
	DataType tScreen = typeStore.getScreen();
	DataType tAudio = typeStore.getAudio();
	DataType tLink = typeStore.getLink();
	DataType tItemClass = typeStore.getItemClass();
	DataType tItem = typeStore.getItem();
	DataType tNpcClass = typeStore.getNpcClass();
	DataType tNpc = typeStore.getNpc();
	DataType tFfc = typeStore.getFfc();
	DataType tLWpn = typeStore.getLWpn();
	DataType tEWpn = typeStore.getEWpn();
	DataType tEnd;

	const int refVar = REFLWPN;
	LibraryHelper lh(scope, refVar, tLWpn);

	addPair(lh, LWPNX, tFloat, "X");
	addPair(lh, LWPNY, tFloat, "Y");
	addPair(lh, LWPNZ, tFloat, "Z");
	addPair(lh, LWPNJUMP, tFloat, "Jump");
	addPair(lh, LWPNDIR, tFloat, "Dir");
	addPair(lh, LWPNANGLE, tFloat, "Angle");
	addPair(lh, LWPNSTEP, tFloat, "Step");
	addPair(lh, LWPNFRAMES, tFloat, "NumFrames");
	addPair(lh, LWPNFRAME, tFloat, "Frame");
	addPair(lh, LWPNDRAWTYPE, tFloat, "DrawStyle");
	addPair(lh, LWPNPOWER, tFloat, "Damage");
	addPair(lh, LWPNID, tFloat, "ID");
	addPair(lh, LWPNANGULAR, tBool, "Angular");
	addPair(lh, LWPNBEHIND, tBool, "Behind");
	addPair(lh, LWPNASPEED, tFloat, "ASpeed");
	addPair(lh, LWPNTILE, tFloat, "Tile");
	addPair(lh, LWPNFLASHCSET, tFloat, "FlashCSet");
	addPair(lh, LWPNDEAD, tFloat, "DeadState");
	addPair(lh, LWPNCSET, tFloat, "CSet");
	addPair(lh, LWPNFLASH, tBool, "Flash");
	addPair(lh, LWPNFLIP, tFloat, "Flip");
	addPair(lh, LWPNOTILE, tFloat, "OriginalTile");
	addPair(lh, LWPNOCSET, tFloat, "OriginalCSet");
	addPair(lh, LWPNEXTEND, tFloat, "Extend");
	addPair(lh, LWPNHXSZ, tFloat, "HitWidth");
	addPair(lh, LWPNHYSZ, tFloat, "HitHeight");
	addPair(lh, LWPNHZSZ, tFloat, "HitZHeight");
	addPair(lh, LWPNTXSZ, tFloat, "TileWidth");
	addPair(lh, LWPNTYSZ, tFloat, "TileHeight");
	addPair(lh, LWPNXOFS, tFloat, "DrawXOffset");
	addPair(lh, LWPNYOFS, tFloat, "DrawYOffset");
	addPair(lh, LWPNZOFS, tFloat, "DrawZOffset");
	addPair(lh, LWPNHXOFS, tFloat, "HitXOffset");
	addPair(lh, LWPNHYOFS, tFloat, "HitYOffset");
	addPair(lh, LWPNMISCD, tFloat, "Misc", 32);
	addPair(lh, LWPNCOLLDET, tBool, "CollDetection");
	addPair(lh, LWPNRANGE, tFloat, "Range");
	
    // bool lweapon->isValid()
    {
	    Function& function = lh.addFunction(tBool, "isValid", tEnd);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop off the pointer
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //Check validity
        code.push_back(new OIsValidLWpn(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
    
    // void lweapon->UseSprite(float spriteId)
    {
	    Function& function = lh.addFunction(
			    tVoid, "UseSprite", tFloat, tEnd);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop off the val
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop off the pointer
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OSetRegister(new VarArgument(refVar), new VarArgument(EXP2)));
        code.push_back(new OUseSpriteLWpn(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
}
