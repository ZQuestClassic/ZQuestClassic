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
		{
			Function& function = lh.addFunction(
					tFloat, "GetItemdataPointer", P() << tItemClass);
	    
			int label = function.getLabel();
			vector<Opcode *> code;
			Opcode *first = new OPopRegister(new VarArgument(EXP1));
			first->setLabel(label);
			code.push_back(first);
			//pop pointer, and ignore it
			code.push_back(new OPopRegister(new VarArgument(NUL)));
			code.push_back(new OGetItemDataPointer(new VarArgument(EXP1)));
			code.push_back(new OPopRegister(new VarArgument(EXP2)));
			code.push_back(new OGotoRegister(new VarArgument(EXP2)));
			function.giveCode(code);
		}
    
		// itemclass Debug->SetItemdataPointer(float)
		{
			Function& function = lh.addFunction(
					tItemClass, "SetItemdataPointer", P() << tFloat);
	    
			int label = function.getLabel();
			vector<Opcode *> code;
			Opcode *first = new OPopRegister(new VarArgument(EXP1));
			first->setLabel(label);
			code.push_back(first);
			//pop pointer, and ignore it
			code.push_back(new OPopRegister(new VarArgument(NUL)));
			code.push_back(new OSetItemDataPointer(new VarArgument(EXP1)));
			code.push_back(new OPopRegister(new VarArgument(EXP2)));
			code.push_back(new OGotoRegister(new VarArgument(EXP2)));
			function.giveCode(code);
		}

		// float Debug->GetItemPointer(item)
		{
			Function& function = lh.addFunction(
					tFloat, "GetItemPointer", P() << tItem);
	    
			int label = function.getLabel();
			vector<Opcode *> code;
			Opcode *first = new OPopRegister(new VarArgument(EXP1));
			first->setLabel(label);
			code.push_back(first);
			//pop pointer, and ignore it
			code.push_back(new OPopRegister(new VarArgument(NUL)));
			code.push_back(new OGetItemPointer(new VarArgument(EXP1)));
			code.push_back(new OPopRegister(new VarArgument(EXP2)));
			code.push_back(new OGotoRegister(new VarArgument(EXP2)));
			function.giveCode(code);
		}
    
		// item Debug->SetItemPointer(float)
		{
			Function& function = lh.addFunction(
					tItem, "SetItemPointer", P() << tFloat);
	    
			int label = function.getLabel();
			vector<Opcode *> code;
			Opcode *first = new OPopRegister(new VarArgument(EXP1));
			first->setLabel(label);
			code.push_back(first);
			//pop pointer, and ignore it
			code.push_back(new OPopRegister(new VarArgument(NUL)));
			code.push_back(new OSetItemPointer(new VarArgument(EXP1)));
			code.push_back(new OPopRegister(new VarArgument(EXP2)));
			code.push_back(new OGotoRegister(new VarArgument(EXP2)));
			function.giveCode(code);
		}
    
		// float Debug->GetPointer(ffc)
		{
			Function& function = lh.addFunction(
					tFloat, "GetFFCPointer", P() << tFfc);
	    
			int label = function.getLabel();
			vector<Opcode *> code;
			Opcode *first = new OPopRegister(new VarArgument(EXP1));
			first->setLabel(label);
			code.push_back(first);
			//pop pointer, and ignore it
			code.push_back(new OPopRegister(new VarArgument(NUL)));
			code.push_back(new OGetFFCPointer(new VarArgument(EXP1)));
			code.push_back(new OPopRegister(new VarArgument(EXP2)));
			code.push_back(new OGotoRegister(new VarArgument(EXP2)));
			function.giveCode(code);
		}
    
		// ffc Debug->SetFFCPointer(float)
		{
			Function& function = lh.addFunction(
					tFfc, "SetFFCPointer", P() << tFloat);
	    
			int label = function.getLabel();
			vector<Opcode *> code;
			Opcode *first = new OPopRegister(new VarArgument(EXP1));
			first->setLabel(label);
			code.push_back(first);
			//pop pointer, and ignore it
			code.push_back(new OPopRegister(new VarArgument(NUL)));
			code.push_back(new OSetFFCPointer(new VarArgument(EXP1)));
			code.push_back(new OPopRegister(new VarArgument(EXP2)));
			code.push_back(new OGotoRegister(new VarArgument(EXP2)));
			function.giveCode(code);
		}

		// float Debug->GetEWeaponPointer(eweapon)
		{
			Function& function = lh.addFunction(
					tFloat, "GetEWeaponPointer", P() << tEWpn);
	    
			int label = function.getLabel();
			vector<Opcode *> code;
			Opcode *first = new OPopRegister(new VarArgument(EXP1));
			first->setLabel(label);
			code.push_back(first);
			//pop pointer, and ignore it
			code.push_back(new OPopRegister(new VarArgument(NUL)));
			code.push_back(new OGetEWeaponPointer(new VarArgument(EXP1)));
			code.push_back(new OPopRegister(new VarArgument(EXP2)));
			code.push_back(new OGotoRegister(new VarArgument(EXP2)));
			function.giveCode(code);
		}
    
		// eweapon Debug->SetEWeaponPointer(float)
		{
			Function& function = lh.addFunction(
					tEWpn, "SetEWeaponPointer", P() << tFloat);
	    
			int label = function.getLabel();
			vector<Opcode *> code;
			Opcode *first = new OPopRegister(new VarArgument(EXP1));
			first->setLabel(label);
			code.push_back(first);
			//pop pointer, and ignore it
			code.push_back(new OPopRegister(new VarArgument(NUL)));
			code.push_back(new OSetEWeaponPointer(new VarArgument(EXP1)));
			code.push_back(new OPopRegister(new VarArgument(EXP2)));
			code.push_back(new OGotoRegister(new VarArgument(EXP2)));
			function.giveCode(code);
		}

		// float Debug->GetLWeaponPointer(lweapon)
		{
			Function& function = lh.addFunction(
					tFloat, "GetLWeaponPointer", P() << tLWpn);
	    
			int label = function.getLabel();
			vector<Opcode *> code;
			Opcode *first = new OPopRegister(new VarArgument(EXP1));
			first->setLabel(label);
			code.push_back(first);
			//pop pointer, and ignore it
			code.push_back(new OPopRegister(new VarArgument(NUL)));
			code.push_back(new OGetLWeaponPointer(new VarArgument(EXP1)));
			code.push_back(new OPopRegister(new VarArgument(EXP2)));
			code.push_back(new OGotoRegister(new VarArgument(EXP2)));
			function.giveCode(code);
		}
    
		// lweapon Debug->SetLWeaponPointer(float)
		{
			Function& function = lh.addFunction(
					tLWpn, "SetLWeaponPointer", P() << tFloat);
	    
			int label = function.getLabel();
			vector<Opcode *> code;
			Opcode *first = new OPopRegister(new VarArgument(EXP1));
			first->setLabel(label);
			code.push_back(first);
			//pop pointer, and ignore it
			code.push_back(new OPopRegister(new VarArgument(NUL)));
			code.push_back(new OSetLWeaponPointer(new VarArgument(EXP1)));
			code.push_back(new OPopRegister(new VarArgument(EXP2)));
			code.push_back(new OGotoRegister(new VarArgument(EXP2)));
			function.giveCode(code);
		}

		// float Debug->GetNPCPointer(npc)
		{
			Function& function = lh.addFunction(
					tFloat, "GetNPCPointer", P() << tNpc);
	    
			int label = function.getLabel();
			vector<Opcode *> code;
			Opcode *first = new OPopRegister(new VarArgument(EXP1));
			first->setLabel(label);
			code.push_back(first);
			//pop pointer, and ignore it
			code.push_back(new OPopRegister(new VarArgument(NUL)));
			code.push_back(new OGetNPCPointer(new VarArgument(EXP1)));
			code.push_back(new OPopRegister(new VarArgument(EXP2)));
			code.push_back(new OGotoRegister(new VarArgument(EXP2)));
			function.giveCode(code);
		}
    
		// npc Debug->SetNPCPointer(float)
		{
			Function& function = lh.addFunction(
					tNpc, "SetNPCPointer", P() << tFloat);
	    
			int label = function.getLabel();
			vector<Opcode *> code;
			Opcode *first = new OPopRegister(new VarArgument(EXP1));
			first->setLabel(label);
			code.push_back(first);
			//pop pointer, and ignore it
			code.push_back(new OPopRegister(new VarArgument(NUL)));
			code.push_back(new OSetNPCPointer(new VarArgument(EXP1)));
			code.push_back(new OPopRegister(new VarArgument(EXP2)));
			code.push_back(new OGotoRegister(new VarArgument(EXP2)));
			function.giveCode(code);
		}

		// float Debug->GetBoolPointer(bool)
		{
			Function& function = lh.addFunction(
					tFloat, "GetBoolPointer", P() << tBool);
	    
			int label = function.getLabel();
			vector<Opcode *> code;
			Opcode *first = new OPopRegister(new VarArgument(EXP1));
			first->setLabel(label);
			code.push_back(first);
			//pop pointer, and ignore it
			code.push_back(new OPopRegister(new VarArgument(NUL)));
			code.push_back(new OGetBoolPointer(new VarArgument(EXP1)));
			code.push_back(new OPopRegister(new VarArgument(EXP2)));
			code.push_back(new OGotoRegister(new VarArgument(EXP2)));
			function.giveCode(code);
		}
    
		// bool Debug->SetBoolPointer(float)
		{
			Function& function = lh.addFunction(
					tBool, "SetBoolPointer", P() << tFloat);
	    
			int label = function.getLabel();
			vector<Opcode *> code;
			Opcode *first = new OPopRegister(new VarArgument(EXP1));
			first->setLabel(label);
			code.push_back(first);
			//pop pointer, and ignore it
			code.push_back(new OPopRegister(new VarArgument(NUL)));
			code.push_back(new OSetBoolPointer(new VarArgument(EXP1)));
			code.push_back(new OPopRegister(new VarArgument(EXP2)));
			code.push_back(new OGotoRegister(new VarArgument(EXP2)));
			function.giveCode(code);
		}
	}

	// void Debug->TriggerSecret(float)
    {
	    Function& function = lh.addFunction(
			    tVoid, "TriggerSecret", P() << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OTriggerSecretRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
    
    // void Debug->ChangeFFCScript(float)
    {
	    Function& function = lh.addFunction(
			    tVoid, "ChangeFFCScript", P() << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OChangeFFCScriptRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
}
