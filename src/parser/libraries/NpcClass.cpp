#include "../../precompiled.h"
#include "../Library.h"
#include "../LibraryHelper.h"

#include "../ByteCode.h"
#include "../Scope.h"

using namespace ZScript;
using namespace ZScript::Libraries;

NpcClass const& NpcClass::singleton()
{
	static NpcClass const instance;
	return instance;
}

void NpcClass::addTo(Scope& scope) const
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
	
	LibraryHelper lh(scope, NUL, tNpcClass);

	lh.addGetter(FCSET, tFloat, "CSet");

	/*
	  Use functions that accept two args (ID, value) per field for setters
	  use functions that accept one arg (ID) per field as getters
	  Plus One master function to set all attriutes using an array?
	*/   

	/*
	// bool NpcClass->isValid(npcclass)
    {
	    Function& function = lh.addFunction(
			    tBool, "isValid", tNpcClass, tEnd);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop off the pointer
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //Check validity
        code.push_back(new OIsValidEWpn(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
    */
}
