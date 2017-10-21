#include "../../precompiled.h"
#include "../Library.h"
#include "../LibraryHelper.h"

#include "../Scope.h"

using namespace ZScript;
using namespace ZScript::Libraries;
using namespace ZAsm;

Link const& Link::singleton()
{
	static Link const instance;
	return instance;
}

void Link::addTo(Scope& scope) const
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
	
	LibraryHelper lh(scope, varNull(), tLink);

	addPair(lh, varLINKX(), tFloat, "X");
	addPair(lh, varLINKY(), tFloat, "Y");
	addPair(lh, varLINKZ(), tFloat, "Z");
	addPair(lh, varLINKJUMP(), tFloat, "Jump");
	addPair(lh, varLINKDIR(), tFloat, "Dir");
	addPair(lh, varLINKHITDIR(), tFloat, "HitDir");
	addPair(lh, varLINKSWORDJINX(), tFloat, "SwordJinx");
	addPair(lh, varLINKITEMJINX(), tFloat, "ItemJinx");
	addPair(lh, varLINKHP(), tFloat, "HP");
	addPair(lh, varLINKMP(), tFloat, "MP");
	addPair(lh, varLINKMAXHP(), tFloat, "MaxHP");
	addPair(lh, varLINKMAXMP(), tFloat, "MaxMP");
	addPair(lh, varLINKACTION(), tFloat, "Action");
	addPair(lh, varLINKHELD(), tFloat, "HeldItem");
	addPair(lh, varINPUTSTART(), tBool, "InputStart");
	addPair(lh, varINPUTMAP(), tBool, "InputMap");
	addPair(lh, varINPUTUP(), tBool, "InputUp");
	addPair(lh, varINPUTDOWN(), tBool, "InputDown");
	addPair(lh, varINPUTLEFT(), tBool, "InputLeft");
	addPair(lh, varINPUTRIGHT(), tBool, "InputRight");
	addPair(lh, varINPUTA(), tBool, "InputA");
	addPair(lh, varINPUTB(), tBool, "InputB");
	addPair(lh, varINPUTL(), tBool, "InputL");
	addPair(lh, varINPUTR(), tBool, "InputR");
	addPair(lh, varINPUTEX1(), tBool, "InputEx1");
	addPair(lh, varINPUTEX2(), tBool, "InputEx2");
	addPair(lh, varINPUTEX3(), tBool, "InputEx3");
	addPair(lh, varINPUTEX4(), tBool, "InputEx4");
	addPair(lh, varINPUTPRESSSTART(), tBool, "PressStart");
	addPair(lh, varINPUTPRESSMAP(), tBool, "PressMap");
	addPair(lh, varINPUTPRESSUP(), tBool, "PressUp");
	addPair(lh, varINPUTPRESSDOWN(), tBool, "PressDown");
	addPair(lh, varINPUTPRESSLEFT(), tBool, "PressLeft");
	addPair(lh, varINPUTPRESSRIGHT(), tBool, "PressRight");
	addPair(lh, varINPUTPRESSA(), tBool, "PressA");
	addPair(lh, varINPUTPRESSB(), tBool, "PressB");
	addPair(lh, varINPUTPRESSL(), tBool, "PressL");
	addPair(lh, varINPUTPRESSR(), tBool, "PressR");
	addPair(lh, varINPUTPRESSEX1(), tBool, "PressEx1");
	addPair(lh, varINPUTPRESSEX2(), tBool, "PressEx2");
	addPair(lh, varINPUTPRESSEX3(), tBool, "PressEx3");
	addPair(lh, varINPUTPRESSEX4(), tBool, "PressEx4");
	addPair(lh, varINPUTMOUSEX(), tFloat, "InputMouseX");
	addPair(lh, varINPUTMOUSEY(), tFloat, "InputMouseY");
	addPair(lh, varINPUTMOUSEZ(), tFloat, "InputMouseZ");
	addPair(lh, varINPUTMOUSEB(), tFloat, "InputMouseB");
	addPair(lh, varLINKITEMD(), tBool, "Item", 256);
	addPair(lh, varLINKHXSZ(), tFloat, "HitWidth");
	addPair(lh, varLINKHYSZ(), tFloat, "HitHeight");
	addPair(lh, varLINKHZSZ(), tFloat, "HitZHeight");
	addPair(lh, varLINKTXSZ(), tFloat, "TileWidth");
	addPair(lh, varLINKTYSZ(), tFloat, "TileHeight");
	addPair(lh, varLINKXOFS(), tFloat, "DrawXOffset");
	addPair(lh, varLINKYOFS(), tFloat, "DrawYOffset");
	addPair(lh, varLINKZOFS(), tFloat, "DrawZOffset");
	addPair(lh, varLINKHXOFS(), tFloat, "HitXOffset");
	addPair(lh, varLINKHYOFS(), tFloat, "HitYOffset");
	addPair(lh, varLINKDRUNK(), tFloat, "Drunk");
	addPair(lh, varLINKEQUIP(), tFloat, "Equipment");
	addPair(lh, varINPUTAXISUP(), tBool, "InputAxisUp");
	addPair(lh, varINPUTAXISDOWN(), tBool, "InputAxisDown");
	addPair(lh, varINPUTAXISLEFT(), tBool, "InputAxisLeft");
	addPair(lh, varINPUTAXISRIGHT(), tBool, "InputAxisRight");
	addPair(lh, varPRESSAXISUP(), tBool, "PressAxisUp");
	addPair(lh, varPRESSAXISDOWN(), tBool, "PressAxisDown");
	addPair(lh, varPRESSAXISLEFT(), tBool, "PressAxisLeft");
	addPair(lh, varPRESSAXISRIGHT(), tBool, "PressAxisRight");
	// XXX Invisible getter was set as Float?
	addPair(lh, varLINKINVIS(), tBool, "Invisible");
	// XXX CollDetection getter was set as Float?
	addPair(lh, varLINKINVINC(), tBool, "CollDetection");
	addPair(lh, varLINKMISCD(), tFloat, "Misc", 32);
	lh.addGetter(varLINKLADDERX(), tFloat, "LadderX");
	lh.addGetter(varLINKLADDERY(), tFloat, "LadderY");
	addPair(lh, varLINKTILE(), tFloat, "Tile");
	addPair(lh, varLINKFLIP(), tFloat, "Flip");
	addPair(lh, varLINKINVFRAME(), tFloat, "InvFrames");
	addPair(lh, varLINKCANFLICKER(), tBool, "InvFlicker");
	addPair(lh, varLINKHURTSFX(), tFloat, "HurtSound");

	// What are these supposed to do? Is Get/SetItemA sufficient?
	// addPair(lh, varLINKUSINGITEM(), tFloat, "UsingItem");

	// Not sure what the 'attack' var is used for at present, but that is
	// what etItemA returned, so I'm renaming these to ->Attack
	// addPair(lh, varLINKUSINGITEMA(), tFloat, "Attack");

	// addPair(lh, varLINKITEMA(), tFloat, "ItemA");
	// addPair(lh, varLINKITEMB(), tFloat, "ItemB");

	addPair(lh, varLINKEATEN(), tFloat, "Eaten");
	addPair(lh, varLINKDIAG(), tBool, "Diagonal");
	addPair(lh, varLINKBIGHITBOX(), tBool, "BigHitbox");

	// void Link->Warp(float, float)
	defineFunction(
			lh, tVoid, "Warp",
			P() << tFloat  << tFloat,
			R() << varExp2() << varExp1(),
			opWARPR(varExp2(), varExp1()));
    
    // void Link->PitWarp(float, float)
	defineFunction(
			lh, tVoid, "PitWarp",
			P() << tFloat  << tFloat,
			R() << varExp2() << varExp1(),
			opPITWARPR(varExp2(), varExp1()));
    
    // void Link->SetItemSlot(float item, float slot, float force)
	defineFunction(
			lh, tVoid, "SetItemSlot",
			P() << tFloat   << tFloat    << tFloat,
			R() << varIndex1() << varIndex2() << varSFTemp(),
			opSETR(varSETITEMSLOT(), varSFTemp()));
    
    // void Link->SetItemA(float itemId)
	defineFunction(
			lh, tVoid, "SetItemA",
			P() << tFloat, R() << varExp2(),
			opSETR(varGAMESETA(), varExp2()));
    
    // void Link->SetItemB(float itemId)
	defineFunction(
			lh, tVoid, "SetItemB",
			P() << tFloat, R() << varExp2(),
			opSETR(varGAMESETB(), varExp2()));
    
    // void Link->SelectAWeapon(float)
	defineFunction(
			lh, tVoid, "SelectAWeapon",
			P() << tFloat, R() << varExp1(),
			opSELECTAWPNR(varExp1()));

    // void Link->SelectBWeapon(float)
	defineFunction(
			lh, tVoid, "SelectBWeapon",
			P() << tFloat, R() << varExp1(),
			opSELECTBWPNR(varExp1()));
}
