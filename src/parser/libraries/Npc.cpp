#include "../../precompiled.h"
#include "../Library.h"
#include "../LibraryHelper.h"

#include "../Scope.h"

using namespace ZScript;
using namespace ZScript::Libraries;
using namespace ZAsm;

Npc const& Npc::singleton()
{
	static Npc const instance;
	return instance;
}

void Npc::addTo(Scope& scope) const
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
	
	LibraryHelper lh(scope, varREFNPC(), tNpc);

	addPair(lh, varNPCX(), tFloat, "X");
	addPair(lh, varNPCY(), tFloat, "Y");
	addPair(lh, varNPCZ(), tFloat, "Z");
	addPair(lh, varNPCJUMP(), tFloat, "Jump");
	addPair(lh, varNPCDIR(), tFloat, "Dir");
	addPair(lh, varNPCRATE(), tFloat, "Rate");
	addPair(lh, varNPCHOMING(), tFloat, "Homing");
	addPair(lh, varNPCSTEP(), tFloat, "Step");
	addPair(lh, varNPCFRAMERATE(), tFloat, "ASpeed");
	addPair(lh, varNPCHALTRATE(), tFloat, "Haltrate");
	addPair(lh, varNPCDRAWTYPE(), tFloat, "DrawStyle");
	addPair(lh, varNPCHP(), tFloat, "HP");
	addPair(lh, varNPCID(), tFloat, "ID");
	addPair(lh, varNPCTYPE(), tFloat, "Type");
	addPair(lh, varNPCDP(), tFloat, "Damage");
	addPair(lh, varNPCWDP(), tFloat, "WeaponDamage");
	addPair(lh, varNPCTILE(), tFloat, "Tile");
	addPair(lh, varNPCOTILE(), tFloat, "OriginalTile");
	addPair(lh, varNPCWEAPON(), tFloat, "Weapon");
	addPair(lh, varNPCITEMSET(), tFloat, "ItemSet");
	addPair(lh, varNPCCSET(), tFloat, "CSet");
	addPair(lh, varNPCBOSSPAL(), tFloat, "BossPal");
	addPair(lh, varNPCBGSFX(), tFloat, "SFX");
	addPair(lh, varNPCEXTEND(), tFloat, "Extend");
	addPair(lh, varNPCHXSZ(), tFloat, "HitWidth");
	addPair(lh, varNPCHYSZ(), tFloat, "HitHeight");
	addPair(lh, varNPCHZSZ(), tFloat, "HitZHeight");
	addPair(lh, varNPCTXSZ(), tFloat, "TileWidth");
	addPair(lh, varNPCTYSZ(), tFloat, "TileHeight");
	addPair(lh, varNPCXOFS(), tFloat, "DrawXOffset");
	addPair(lh, varNPCYOFS(), tFloat, "DrawYOffset");
	addPair(lh, varNPCZOFS(), tFloat, "DrawZOffset");
	addPair(lh, varNPCHXOFS(), tFloat, "HitXOffset");
	addPair(lh, varNPCHYOFS(), tFloat, "HitYOffset");
	addPair(lh, varNPCMISCD(), tFloat, "Misc", 32);
	addPair(lh, varNPCDD(), tFloat, "Attributes", 15);
	addPair(lh, varNPCMFLAGS(), tFloat, "MiscFlags");
	addPair(lh, varNPCCOLLDET(), tBool, "CollDetection"); // XXX Getter was float?
	addPair(lh, varNPCSTUN(), tFloat, "Stun");
	addPair(lh, varNPCDEFENSED(), tFloat, "Defense", 40);
	addPair(lh, varNPCHUNGER(), tFloat, "Hunger");
	addPair(lh, varNPCWEAPSPRITE(), tFloat, "WeaponSprite");
	// 2.future srit compatibility. -Z
	addPair(lh, varNPCSCRDEFENSED(), tFloat, "ScriptDefense", 10);
	addPair(lh, varNPCINVINC(), tFloat, "InvFrames");
	addPair(lh, varNPCSUPERMAN(), tFloat, "Invincible");
	addPair(lh, varNPCRINGLEAD(), tBool, "Ringleader");
	addPair(lh, varNPCHASITEM(), tBool, "HasItem");
	
    // bool Npc->isValid()
	defineFunction(
			lh, tBool, "isValid", P(), R() << varExp1(),
			opISVALIDNPC(varExp1()));
    
    // void Npc->GetName(float bufferId)
	defineFunction(
			lh, tVoid, "GetName",
			P() << tFloat, R() << varExp1(),
			opNPCNAME(varExp1()));
    
    // void Npc->BreakShield()
	defineFunction(
			lh, tVoid, "BreakShield",
			P(), R() << varExp1(),
			opBREAKSHIELD(varExp1()));
}
