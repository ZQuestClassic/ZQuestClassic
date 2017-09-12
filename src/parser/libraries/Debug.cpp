#include "../../precompiled.h"
#include "../Library.h"
#include "../LibraryHelper.h"

#include "../ByteCode.h"
#include "../CompilerUtils.h"
#include "../Scope.h"

using namespace ZScript;
using namespace ZScript::Libraries;

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
	typedef VectorBuilder<int> R;
	typedef VectorBuilder<Opcode*> O;
	
	LibraryHelper lh(scope, NUL, tDebug);

	// All of these return a function label error when used
	addPair(lh, DEBUGREFFFC, tFloat, "RefFFC");
	addPair(lh, DEBUGREFITEM, tFloat, "RefItem");
	addPair(lh, DEBUGREFITEMDATA, tFloat, "RefItemdata");
	addPair(lh, DEBUGREFNPC, tFloat, "RefNPC");
	addPair(lh, DEBUGREFLWEAPON, tFloat, "RefLWeapon");
	addPair(lh, DEBUGREFEWEAPON, tFloat, "RefEWeapon");
	addPair(lh, DEBUGSP, tFloat, "SP");
	addPair(lh, DEBUGGDR, tFloat, "GDR", 256);

	// Changing the subscreen and screen offsets seems to do nothing.
	addPair(lh, PASSSUBOFS, tFloat, "SubscreenOffset");
	addPair(lh, GAMESUBSCHEIGHT, tFloat, "SubscreenHeight");
	addPair(lh, GAMEPLAYFIELDOFS, tFloat, "PlayfieldOffset");
	

	// These all work, but may not be permitted by the rest of the dev team.
	// At least they are now in a pointer class that denotes that they can
	// be dangerous.
	{
		// float Debug->GetItemdataPointer(itemclass)
		defineFunction(
				lh, tFloat, "GetItemdataPointer",
				P() << tItemClass, R() << EXP1,
				new OGetItemDataPointer(new VarArgument(EXP1)));
    
		// itemclass Debug->SetItemdataPointer(float)
		defineFunction(
				lh, tItemClass, "SetItemdataPointer",
				P() << tFloat, R() << EXP1,
				new OSetItemDataPointer(new VarArgument(EXP1)));

		// float Debug->GetItemPointer(item)
		defineFunction(
				lh, tFloat, "GetItemPointer",
				P() << tItem, R() << EXP1,
				new OGetItemPointer(new VarArgument(EXP1)));
    
		// item Debug->SetItemPointer(float)
		defineFunction(
				lh, tItem, "SetItemPointer",
				P() << tFloat, R() << EXP1,
				new OSetItemPointer(new VarArgument(EXP1)));
    
		// float Debug->GetPointer(ffc)
		defineFunction(
				lh, tFloat, "GetFFCPointer",
				P() << tFfc, R() << EXP1,
				new OGetFFCPointer(new VarArgument(EXP1)));
    
		// ffc Debug->SetFFCPointer(float)
		defineFunction(
				lh, tFfc, "SetFFCPointer",
				P() << tFloat, R() << EXP1,
				new OSetFFCPointer(new VarArgument(EXP1)));

		// float Debug->GetEWeaponPointer(eweapon)
		defineFunction(
				lh, tFloat, "GetEWeaponPointer",
				P() << tEWpn, R() << EXP1,
				new OGetEWeaponPointer(new VarArgument(EXP1)));
    
		// eweapon Debug->SetEWeaponPointer(float)
		defineFunction(
				lh, tEWpn, "SetEWeaponPointer",
				P() << tFloat, R() << EXP1,
				new OSetEWeaponPointer(new VarArgument(EXP1)));

		// float Debug->GetLWeaponPointer(lweapon)
		defineFunction(
				lh, tFloat, "GetLWeaponPointer",
				P() << tLWpn, R() << EXP1,
				new OGetLWeaponPointer(new VarArgument(EXP1)));
    
		// lweapon Debug->SetLWeaponPointer(float)
		defineFunction(
				lh, tLWpn, "SetLWeaponPointer",
				P() << tFloat, R() << EXP1,
				new OSetLWeaponPointer(new VarArgument(EXP1)));

		// float Debug->GetNPCPointer(npc)
		defineFunction(
				lh, tFloat, "GetNPCPointer",
				P() << tNpc, R() << EXP1,
				new OGetNPCPointer(new VarArgument(EXP1)));
    
		// npc Debug->SetNPCPointer(float)
		defineFunction(
				lh, tNpc, "SetNPCPointer",
				P() << tFloat, R() << EXP1,
				new OSetNPCPointer(new VarArgument(EXP1)));

		// float Debug->GetBoolPointer(bool)
		defineFunction(
				lh, tFloat, "GetBoolPointer",
				P() << tBool, R() << EXP1,
				new OGetBoolPointer(new VarArgument(EXP1)));
    
		// bool Debug->SetBoolPointer(float)
		defineFunction(
				lh, tBool, "SetBoolPointer",
				P() << tFloat, R() << EXP1,
				new OSetBoolPointer(new VarArgument(EXP1)));
	}

	// void Debug->TriggerSecret(float)
	defineFunction(
			lh, tVoid, "TriggerSecret",
			P() << tFloat, R() << EXP1,
			new OTriggerSecretRegister(new VarArgument(EXP1)));
    
    // void Debug->ChangeFFCScript(float)
	defineFunction(
			lh, tVoid, "ChangeFFCScript",
			P() << tFloat, R() << EXP1,
			new OChangeFFCScriptRegister(new VarArgument(EXP1)));
}
