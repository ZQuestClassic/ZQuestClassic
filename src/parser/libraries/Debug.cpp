#include "../../precompiled.h"
#include "../Library.h"
#include "../LibraryHelper.h"

#include "../ByteCode.h"
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
	DataTypeSimple const* tVoid = &DataType::ZVOID;
	DataTypeSimple const* tBool = &DataType::BOOL;
	DataTypeSimple const* tFloat = &DataType::FLOAT;
	DataTypeClass const* tGame = &DataType::GAME;
	DataTypeClass const* tDebug = &DataType::DEBUG;
	DataTypeClass const* tScreen = &DataType::SCREEN;
	DataTypeClass const* tAudio = &DataType::AUDIO;
	DataTypeClass const* tLink = &DataType::_LINK;
	DataTypeClass const* tItemClass = &DataType::ITEMCLASS;
	DataTypeClass const* tItem = &DataType::ITEM;
	DataTypeClass const* tNpcClass = &DataType::NPCDATA;
	DataTypeClass const* tNpc = &DataType::NPC;
	DataTypeClass const* tFfc = &DataType::FFC;
	DataTypeClass const* tLWpn = &DataType::LWPN;
	DataTypeClass const* tEWpn = &DataType::EWPN;
	DataType const* tEnd = NULL;
	
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
					tFloat, "GetItemdataPointer", tItemClass, tEnd);
	    
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
					tItemClass, "SetItemdataPointer", tFloat, tEnd);
	    
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
					tFloat, "GetItemPointer", tItem, tEnd);
	    
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
					tItem, "SetItemPointer", tFloat, tEnd);
	    
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
					tFloat, "GetFFCPointer", tFfc, tEnd);
	    
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
					tFfc, "SetFFCPointer", tFloat, tEnd);
	    
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
					tFloat, "GetEWeaponPointer", tEWpn, tEnd);
	    
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
					tEWpn, "SetEWeaponPointer", tFloat, tEnd);
	    
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
					tFloat, "GetLWeaponPointer", tLWpn, tEnd);
	    
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
					tLWpn, "SetLWeaponPointer", tFloat, tEnd);
	    
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
					tFloat, "GetNPCPointer", tNpc, tEnd);
	    
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
					tNpc, "SetNPCPointer", tFloat, tEnd);
	    
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
					tFloat, "GetBoolPointer", tBool, tEnd);
	    
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
					tBool, "SetBoolPointer", tFloat, tEnd);
	    
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
			    tVoid, "TriggerSecret", tFloat, tEnd);
	    
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
			    tVoid, "ChangeFFCScript", tFloat, tEnd);
	    
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
