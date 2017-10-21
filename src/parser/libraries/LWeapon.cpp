#include "../../precompiled.h"
#include "../Library.h"
#include "../LibraryHelper.h"

#include "../Scope.h"

using namespace ZScript;
using namespace ZScript::Libraries;
using namespace ZAsm;

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

	typedef VectorBuilder<DataType> P;
	typedef VectorBuilder<ZAsm::Variable> R;
	typedef VectorBuilder<Opcode> O;

	ZAsm::Variable const refVar = varREFLWPN();
	LibraryHelper lh(scope, refVar, tLWpn);

	addPair(lh, varLWPNX(), tFloat, "X");
	addPair(lh, varLWPNY(), tFloat, "Y");
	addPair(lh, varLWPNZ(), tFloat, "Z");
	addPair(lh, varLWPNJUMP(), tFloat, "Jump");
	addPair(lh, varLWPNDIR(), tFloat, "Dir");
	addPair(lh, varLWPNANGLE(), tFloat, "Angle");
	addPair(lh, varLWPNSTEP(), tFloat, "Step");
	addPair(lh, varLWPNFRAMES(), tFloat, "NumFrames");
	addPair(lh, varLWPNFRAME(), tFloat, "Frame");
	addPair(lh, varLWPNDRAWTYPE(), tFloat, "DrawStyle");
	addPair(lh, varLWPNPOWER(), tFloat, "Damage");
	addPair(lh, varLWPNID(), tFloat, "ID");
	addPair(lh, varLWPNANGULAR(), tBool, "Angular");
	addPair(lh, varLWPNBEHIND(), tBool, "Behind");
	addPair(lh, varLWPNASPEED(), tFloat, "ASpeed");
	addPair(lh, varLWPNTILE(), tFloat, "Tile");
	addPair(lh, varLWPNFLASHCSET(), tFloat, "FlashCSet");
	addPair(lh, varLWPNDEAD(), tFloat, "DeadState");
	addPair(lh, varLWPNCSET(), tFloat, "CSet");
	addPair(lh, varLWPNFLASH(), tBool, "Flash");
	addPair(lh, varLWPNFLIP(), tFloat, "Flip");
	addPair(lh, varLWPNOTILE(), tFloat, "OriginalTile");
	addPair(lh, varLWPNOCSET(), tFloat, "OriginalCSet");
	addPair(lh, varLWPNEXTEND(), tFloat, "Extend");
	addPair(lh, varLWPNHXSZ(), tFloat, "HitWidth");
	addPair(lh, varLWPNHYSZ(), tFloat, "HitHeight");
	addPair(lh, varLWPNHZSZ(), tFloat, "HitZHeight");
	addPair(lh, varLWPNTXSZ(), tFloat, "TileWidth");
	addPair(lh, varLWPNTYSZ(), tFloat, "TileHeight");
	addPair(lh, varLWPNXOFS(), tFloat, "DrawXOffset");
	addPair(lh, varLWPNYOFS(), tFloat, "DrawYOffset");
	addPair(lh, varLWPNZOFS(), tFloat, "DrawZOffset");
	addPair(lh, varLWPNHXOFS(), tFloat, "HitXOffset");
	addPair(lh, varLWPNHYOFS(), tFloat, "HitYOffset");
	addPair(lh, varLWPNMISCD(), tFloat, "Misc", 32);
	addPair(lh, varLWPNCOLLDET(), tBool, "CollDetection");
	addPair(lh, varLWPNRANGE(), tFloat, "Range");

    // bool lweapon->isValid()
	defineFunction(
			lh, tBool, "isValid", P(), R() << varExp1(),
			opISVALIDLWPN(varExp1()));
    
    // void lweapon->UseSprite(float spriteId)
	defineFunction(
			lh, tVoid, "UseSprite",
			P() /* this   */ << tFloat,
			R() << varExp2() << varExp1(),
			O() << opSETR(refVar, varExp2())
			    << opLWPNUSESPRITER(varExp1()));
}
