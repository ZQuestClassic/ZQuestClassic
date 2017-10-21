#include "../../precompiled.h"
#include "../Library.h"
#include "../LibraryHelper.h"

#include "../ByteCode.h"
#include "../CompilerUtils.h"
#include "../Scope.h"

using namespace ZScript;
using namespace ZScript::Libraries;
using namespace ZAsm;

Debug const& Debug::singleton()
{
	static Debug const instance;
	return instance;
}

void Debug::addTo(Scope& scope) const
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
	
	LibraryHelper lh(scope, varNull(), tDebug);

	// All of these return a function label error when used
	addPair(lh, varDEBUGREFFFC(), tFloat, "RefFFC");
	addPair(lh, varDEBUGREFITEM(), tFloat, "RefItem");
	addPair(lh, varDEBUGREFITEMDATA(), tFloat, "RefItemdata");
	addPair(lh, varDEBUGREFNPC(), tFloat, "RefNPC");
	addPair(lh, varDEBUGREFLWEAPON(), tFloat, "RefLWeapon");
	addPair(lh, varDEBUGREFEWEAPON(), tFloat, "RefEWeapon");
	addPair(lh, varDEBUGSP(), tFloat, "SP");
	addPair(lh, varDEBUGGDR(), tFloat, "GDR", 256);

	// Changing the subscreen and screen offsets seems to do nothing.
	addPair(lh, varPASSSUBOFS(), tFloat, "SubscreenOffset");
	addPair(lh, varGAMESUBSCHEIGHT(), tFloat, "SubscreenHeight");
	addPair(lh, varGAMEPLAYFIELDOFS(), tFloat, "PlayfieldOffset");

	// These all work, but may not be permitted by the rest of the dev team.
	// At least they are now in a pointer class that denotes that they can
	// be dangerous.
	{
		// float Debug->GetItemdataPointer(itemclass)
		defineFunction(
				lh, tFloat, "GetItemdataPointer",
				P() << tItemClass, R() << varExp1(),
				opIDATAARRPTR(varExp1()));
    
		// itemclass Debug->SetItemdataPointer(float)
		defineFunction(
				lh, tItemClass, "SetItemdataPointer",
				P() << tFloat, R() << varExp1(),
				opIDATAARRPTR2(varExp1()));

		// float Debug->GetItemPointer(item)
		defineFunction(
				lh, tFloat, "GetItemPointer",
				P() << tItem, R() << varExp1(),
				opITEMARRPTR(varExp1()));
    
		// item Debug->SetItemPointer(float)
		defineFunction(
				lh, tItem, "SetItemPointer",
				P() << tFloat, R() << varExp1(),
				opITEMARRPTR2(varExp1()));
    
		// float Debug->GetPointer(ffc)
		defineFunction(
				lh, tFloat, "GetFFCPointer",
				P() << tFfc, R() << varExp1(),
				opFFCARRPTR(varExp1()));
    
		// ffc Debug->SetFFCPointer(float)
		defineFunction(
				lh, tFfc, "SetFFCPointer",
				P() << tFloat, R() << varExp1(),
				opFFCARRPTR2(varExp1()));

		// float Debug->GetEWeaponPointer(eweapon)
		defineFunction(
				lh, tFloat, "GetEWeaponPointer",
				P() << tEWpn, R() << varExp1(),
				opEWPNARRPTR(varExp1()));
    
		// eweapon Debug->SetEWeaponPointer(float)
		defineFunction(
				lh, tEWpn, "SetEWeaponPointer",
				P() << tFloat, R() << varExp1(),
				opEWPNARRPTR2(varExp1()));

		// float Debug->GetLWeaponPointer(lweapon)
		defineFunction(
				lh, tFloat, "GetLWeaponPointer",
				P() << tLWpn, R() << varExp1(),
				opLWPNARRPTR(varExp1()));
    
		// lweapon Debug->SetLWeaponPointer(float)
		defineFunction(
				lh, tLWpn, "SetLWeaponPointer",
				P() << tFloat, R() << varExp1(),
				opLWPNARRPTR2(varExp1()));

		// float Debug->GetNPCPointer(npc)
		defineFunction(
				lh, tFloat, "GetNPCPointer",
				P() << tNpc, R() << varExp1(),
				opNPCARRPTR(varExp1()));
    
		// npc Debug->SetNPCPointer(float)
		defineFunction(
				lh, tNpc, "SetNPCPointer",
				P() << tFloat, R() << varExp1(),
				opNPCARRPTR2(varExp1()));

		// float Debug->GetBoolPointer(bool)
		defineFunction(
				lh, tFloat, "GetBoolPointer",
				P() << tBool, R() << varExp1(),
				opBOOLARRPTR(varExp1()));
    
		// bool Debug->SetBoolPointer(float)
		defineFunction(
				lh, tBool, "SetBoolPointer",
				P() << tFloat, R() << varExp1(),
				opBOOLARRPTR2(varExp1()));
	}

	// void Debug->TriggerSecret(float)
	defineFunction(
			lh, tVoid, "TriggerSecret",
			P() << tFloat, R() << varExp1(),
			opTRIGGERSECRETR(varExp1()));
    
    // void Debug->ChangeFFCScript(float)
	defineFunction(
			lh, tVoid, "ChangeFFCScript",
			P() << tFloat, R() << varExp1(),
			opCHANGEFFSCRIPTR(varExp1()));
}
