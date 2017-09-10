#include "../../precompiled.h"
#include "../Library.h"
#include "../LibraryHelper.h"

#include "../ByteCode.h"
#include "../CompilerUtils.h"
#include "../Scope.h"

using namespace ZScript;
using namespace ZScript::Libraries;

EWeapon const& EWeapon::singleton()
{
	static EWeapon const instance;
	return instance;
}

void EWeapon::addTo(Scope& scope) const
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
	typedef VectorBuilder<DataType> P;

	int const refVar = REFEWPN;
	LibraryHelper lh(scope, refVar, tEWpn);	

	addPair(lh, EWPNX, tFloat, "X");
	addPair(lh, EWPNY, tFloat, "Y");
	addPair(lh, EWPNZ, tFloat, "Z");
	addPair(lh, EWPNJUMP, tFloat, "Jump");
	addPair(lh, EWPNDIR, tFloat, "Dir");
	addPair(lh, EWPNANGLE, tFloat, "Angle");
	addPair(lh, EWPNSTEP, tFloat, "Step");
	addPair(lh, EWPNFRAMES, tFloat, "NumFrames");
	addPair(lh, EWPNFRAME, tFloat, "Frame");
	addPair(lh, EWPNDRAWTYPE, tFloat, "DrawStyle");
	addPair(lh, EWPNPOWER, tFloat, "Damage");
	addPair(lh, EWPNID, tFloat, "ID");
	addPair(lh, EWPNANGULAR, tBool, "Angular");
	addPair(lh, EWPNBEHIND, tBool, "Behind");
	addPair(lh, EWPNASPEED, tFloat, "ASpeed");
	addPair(lh, EWPNTILE, tFloat, "Tile");
	addPair(lh, EWPNFLASHCSET, tFloat, "FlashCSet");
	addPair(lh, EWPNDEAD, tFloat, "DeadState");
	addPair(lh, EWPNCSET, tFloat, "CSet");
	addPair(lh, EWPNFLASH, tBool, "Flash");
	addPair(lh, EWPNFLIP, tFloat, "Flip");
	addPair(lh, EWPNOTILE, tFloat, "OriginalTile");
	addPair(lh, EWPNOCSET, tFloat, "OriginalCSet");
	addPair(lh, EWPNEXTEND, tFloat, "Extend");
	addPair(lh, EWPNHXSZ, tFloat, "HitWidth");
	addPair(lh, EWPNHYSZ, tFloat, "HitHeight");
	addPair(lh, EWPNHZSZ, tFloat, "HitZHeight");
	addPair(lh, EWPNTXSZ, tFloat, "TileWidth");
	addPair(lh, EWPNTYSZ, tFloat, "TileHeight");
	addPair(lh, EWPNXOFS, tFloat, "DrawXOffset");
	addPair(lh, EWPNYOFS, tFloat, "DrawYOffset");
	addPair(lh, EWPNZOFS, tFloat, "DrawZOffset");
	addPair(lh, EWPNHXOFS, tFloat, "HitXOffset");
	addPair(lh, EWPNHYOFS, tFloat, "HitYOffset");
	addPair(lh, EWPNMISCD, tFloat, "Misc", 32);
	addPair(lh, EWPNCOLLDET, tBool, "CollDetection");
	
    // bool eweapon->isValid()
    {
	    Function& function = lh.addFunction(tBool, "isValid", P());
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop off the pointer
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //Check validity
        code.push_back(new OIsValidEWpn(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
    
    // void eweapon->UseSprite(float spriteId)
    {
	    Function& function = lh.addFunction(
			    tVoid, "UseSprite", P() << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop off the val
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop off the pointer
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OSetRegister(new VarArgument(refVar), new VarArgument(EXP2)));
        code.push_back(new OUseSpriteEWpn(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
}
