#include "../../precompiled.h"
#include "../Library.h"
#include "../LibraryHelper.h"

#include "../Scope.h"

using namespace ZScript;
using namespace ZScript::Libraries;
using namespace ZAsm;

Item const& Item::singleton()
{
	static Item const instance;
	return instance;
}

void Item::addTo(Scope& scope) const
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
	
	LibraryHelper lh(scope, varREFITEM(), tItem);

	addPair(lh, varITEMX(), tFloat, "X");
	addPair(lh, varITEMY(), tFloat, "Y");
	addPair(lh, varITEMZ(), tFloat, "Z");
	addPair(lh, varITEMJUMP(), tFloat, "Jump");
	addPair(lh, varITEMDRAWTYPE(), tFloat, "DrawStyle");
	addPair(lh, varITEMID(), tFloat, "ID");
	addPair(lh, varITEMTILE(), tFloat, "Tile");
	addPair(lh, varITEMOTILE(), tFloat, "OriginalTile");
	addPair(lh, varITEMCSET(), tFloat, "CSet");
	addPair(lh, varITEMFLASHCSET(), tFloat, "FlashCSet");
	addPair(lh, varITEMFRAMES(), tFloat, "NumFrames");
	addPair(lh, varITEMFRAME(), tFloat, "Frame");
	addPair(lh, varITEMASPEED(), tFloat, "ASpeed");
	addPair(lh, varITEMDELAY(), tFloat, "Delay");
	addPair(lh, varITEMFLASH(), tBool, "Flash");
	addPair(lh, varITEMFLIP(), tFloat, "Flip");
	addPair(lh, varITEMEXTEND(), tFloat, "Extend");
	addPair(lh, varITEMHXSZ(), tFloat, "HitWidth");
	addPair(lh, varITEMHYSZ(), tFloat, "HitHeight");
	addPair(lh, varITEMHZSZ(), tFloat, "HitZHeight");
	addPair(lh, varITEMTXSZ(), tFloat, "TileWidth");
	addPair(lh, varITEMTYSZ(), tFloat, "TileHeight");
	addPair(lh, varITEMXOFS(), tFloat, "DrawXOffset");
	addPair(lh, varITEMYOFS(), tFloat, "DrawYOffset");
	addPair(lh, varITEMZOFS(), tFloat, "DrawZOffset");
	addPair(lh, varITEMHXOFS(), tFloat, "HitXOffset");
	addPair(lh, varITEMHYOFS(), tFloat, "HitYOffset");
	addPair(lh, varITEMPICKUP(), tFloat, "Pickup");
	addPair(lh, varITEMMISCD(), tFloat, "Misc", 32);

    // bool Item->isValid()
	defineFunction(
			lh, tBool, "isValid", P(), R() << varExp1(),
			opISVALIDITEM(varExp1()));
}

