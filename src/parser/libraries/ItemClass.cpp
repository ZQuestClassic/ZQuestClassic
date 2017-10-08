#include "../../precompiled.h"
#include "../Library.h"
#include "../LibraryHelper.h"

#include "../../zdefs.h"
#include "../Scope.h"

using namespace ZScript;
using namespace ZScript::Libraries;
using namespace ZAsm;

ItemClass const& ItemClass::singleton()
{
	static ItemClass const instance;
	return instance;
}

void ItemClass::addTo(Scope& scope) const
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
	
	LibraryHelper lh(scope, varREFITEMCLASS(), tItemClass);

	addPair(lh, varIDATAFAMILY(), tFloat, "Family");
	addPair(lh, varIDATALEVEL(), tFloat, "Level");
	addPair(lh, varIDATAAMOUNT(), tFloat, "Amount");
	addPair(lh, varIDATAMAX(), tFloat, "Max");
	addPair(lh, varIDATASETMAX(), tFloat, "MaxIncrement");
	addPair(lh, varIDATAKEEP(), tBool, "Keep");
	addPair(lh, varIDATACOUNTER(), tFloat, "Counter");
	addPair(lh, varIDATAUSESOUND(), tFloat, "UseSound");
	lh.addGetter(varIDATAID(), tFloat, "ID");
	addPair(lh, varIDATAPOWER(), tFloat, "Power");
	addPair(lh, varIDATAINITDD(), tFloat, "InitD", 2); // XXX Is 2 correct?
	addPair(lh, varIDATALTM(), tFloat, "Modifier");
	addPair(lh, varIDATASCRIPT(), tFloat, "Script");
	addPair(lh, varIDATAPSCRIPT(), tFloat, "PScript");
	addPair(lh, varIDATAMAGCOST(), tFloat, "MagicCost");
	addPair(lh, varIDATAMINHEARTS(), tFloat, "MinHearts");
	addPair(lh, varIDATATILE(), tFloat, "Tile");
	addPair(lh, varIDATAMISC(), tFloat, "Flash");
	addPair(lh, varIDATACSET(), tFloat, "CSet");
	// addPair(lh, varIDATAFRAME(), tFloat, "Frame");
	addPair(lh, varIDATAFRAMES(), tFloat, "AFrames");
	addPair(lh, varIDATAASPEED(), tFloat, "ASpeed");
	addPair(lh, varIDATADELAY(), tFloat, "Delay");
	addPair(lh, varIDATACOMBINE(), tBool, "Combine");
	addPair(lh, varIDATADOWNGRADE(), tBool, "Downgrade");
	addPair(lh, varIDATAKEEPOLD(), tBool, "KeepOld");
	addPair(lh, varIDATARUPEECOST(), tBool, "RupeeCost");
	addPair(lh, varIDATAEDIBLE(), tBool, "Edible");
	addPair(lh, varIDATAFLAGUNUSED(), tBool, "Unused");
	addPair(lh, varIDATAGAINLOWER(), tBool, "GainLower");
	addPair(lh, varIDATAATTRIB(), tFloat, "Attributes", 15);
	addPair(lh, varIDATAATTRIB(), tFloat, "Misc", 10); // XXX Same thing?
	addPair(lh, varIDATAFLAGS(), tBool, "Flags", 5);
	addPair(lh, varIDATASPRITE(), tFloat, "Sprites", 10);
	addPair(lh, varIDATAUSEWPN(), tFloat, "Weapon");
	addPair(lh, varIDATAUSEDEF(), tFloat, "Defense");
	addPair(lh, varIDATAWRANGE(), tFloat, "Range");
	addPair(lh, varIDATADURATION(), tFloat, "Duration");
	addPair(lh, varIDATAUSEMVT(), tFloat, "Movement", ITEM_MOVEMENT_PATTERNS);
	addPair(lh, varIDATAWPNINITD(), tFloat, "WeaponD", 8);
	addPair(lh, varIDATAMISCD(), tFloat, "WeaponMisc", 32);
	addPair(lh, varIDATADUPLICATES(), tFloat, "Duplicates");
	addPair(lh, varIDATADRAWLAYER(), tFloat, "DrawLayer");
	addPair(lh, varIDATACOLLECTFLAGS(), tFloat, "CollectFlags");
	addPair(lh, varIDATAWEAPONSCRIPT(), tFloat, "WeaponScript");
	addPair(lh, varIDATAWEAPHXOFS(), tFloat, "WeaponHitXOffset");
	addPair(lh, varIDATAWEAPHYOFS(), tFloat, "WeaponHitYOffset");
	addPair(lh, varIDATAWEAPHXSZ(), tFloat, "WeaponHitWidth");
	addPair(lh, varIDATAWEAPHYSZ(), tFloat, "WeaponHitHeight");
	addPair(lh, varIDATAWEAPHZSZ(), tFloat, "WeaponHitZHeight");
	addPair(lh, varIDATAWEAPXOFS(), tFloat, "WeaponDrawXOffset");
	addPair(lh, varIDATAWEAPYOFS(), tFloat, "WeaponDrawYOffset");
	addPair(lh, varIDATAWEAPZOFS(), tFloat, "WeaponDrawZOffset");
	
    // void ItemClass->GetName(string buffer)
	defineFunction(
			lh, tVoid, "GetName",
			P() << tFloat, R() << varExp1(),
			opITEMNAME(varExp1()));
}
