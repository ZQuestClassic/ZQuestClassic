#include "../../precompiled.h"
#include "../Library.h"
#include "../LibraryHelper.h"

#include "../../scripting/ZAsmVariables.h"
#include "../CompilerUtils.h"
#include "../Opcode.h"
#include "../Scope.h"

using namespace ZScript;
using namespace ZScript::Libraries;
using namespace ZAsm;

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
	typedef VectorBuilder<ZAsm::Variable> R;
	typedef VectorBuilder<Opcode> O;

	ZAsm::Variable refVar = varREFEWPN();
	LibraryHelper lh(scope, refVar, tEWpn);	

	addPair(lh, varEWPNX(), tFloat, "X");
	addPair(lh, varEWPNY(), tFloat, "Y");
	addPair(lh, varEWPNZ(), tFloat, "Z");
	addPair(lh, varEWPNJUMP(), tFloat, "Jump");
	addPair(lh, varEWPNDIR(), tFloat, "Dir");
	addPair(lh, varEWPNANGLE(), tFloat, "Angle");
	addPair(lh, varEWPNSTEP(), tFloat, "Step");
	addPair(lh, varEWPNFRAMES(), tFloat, "NumFrames");
	addPair(lh, varEWPNFRAME(), tFloat, "Frame");
	addPair(lh, varEWPNDRAWTYPE(), tFloat, "DrawStyle");
	addPair(lh, varEWPNPOWER(), tFloat, "Damage");
	addPair(lh, varEWPNID(), tFloat, "ID");
	addPair(lh, varEWPNANGULAR(), tBool, "Angular");
	addPair(lh, varEWPNBEHIND(), tBool, "Behind");
	addPair(lh, varEWPNASPEED(), tFloat, "ASpeed");
	addPair(lh, varEWPNTILE(), tFloat, "Tile");
	addPair(lh, varEWPNFLASHCSET(), tFloat, "FlashCSet");
	addPair(lh, varEWPNDEAD(), tFloat, "DeadState");
	addPair(lh, varEWPNCSET(), tFloat, "CSet");
	addPair(lh, varEWPNFLASH(), tBool, "Flash");
	addPair(lh, varEWPNFLIP(), tFloat, "Flip");
	addPair(lh, varEWPNOTILE(), tFloat, "OriginalTile");
	addPair(lh, varEWPNOCSET(), tFloat, "OriginalCSet");
	addPair(lh, varEWPNEXTEND(), tFloat, "Extend");
	addPair(lh, varEWPNHXSZ(), tFloat, "HitWidth");
	addPair(lh, varEWPNHYSZ(), tFloat, "HitHeight");
	addPair(lh, varEWPNHZSZ(), tFloat, "HitZHeight");
	addPair(lh, varEWPNTXSZ(), tFloat, "TileWidth");
	addPair(lh, varEWPNTYSZ(), tFloat, "TileHeight");
	addPair(lh, varEWPNXOFS(), tFloat, "DrawXOffset");
	addPair(lh, varEWPNYOFS(), tFloat, "DrawYOffset");
	addPair(lh, varEWPNZOFS(), tFloat, "DrawZOffset");
	addPair(lh, varEWPNHXOFS(), tFloat, "HitXOffset");
	addPair(lh, varEWPNHYOFS(), tFloat, "HitYOffset");
	addPair(lh, varEWPNMISCD(), tFloat, "Misc", 32);
	addPair(lh, varEWPNCOLLDET(), tBool, "CollDetection");
	
    // bool eweapon->isValid()
	defineFunction(
			lh, tBool, "isValid", P(), R() << varExp1(),
			opISVALIDEWPN(varExp1()));
    
    // void eweapon->UseSprite(float spriteId)
	defineFunction(
			lh, tVoid, "UseSprite",
			P() /* this   */ << tFloat,
			R() << varExp2() << varExp1(),
			O() << opSETR(refVar, varExp2())
			    << opEWPNUSESPRITER(varExp1()));
}
