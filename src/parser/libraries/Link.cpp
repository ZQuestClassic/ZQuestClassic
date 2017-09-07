#include "../../precompiled.h"
#include "../Library.h"
#include "../LibraryHelper.h"

#include "../ByteCode.h"
#include "../Scope.h"

using namespace ZScript;
using namespace ZScript::Libraries;

Link const& Link::singleton()
{
	static Link const instance;
	return instance;
}

void Link::addTo(Scope& scope) const
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
	
	LibraryHelper lh(scope, NUL, tLink);

	addPair(lh, LINKX, tFloat, "X");
	addPair(lh, LINKY, tFloat, "Y");
	addPair(lh, LINKZ, tFloat, "Z");
	addPair(lh, LINKJUMP, tFloat, "Jump");
	addPair(lh, LINKDIR, tFloat, "Dir");
	addPair(lh, LINKHITDIR, tFloat, "HitDir");
	addPair(lh, LINKSWORDJINX, tFloat, "SwordJinx");
	addPair(lh, LINKITEMJINX, tFloat, "ItemJinx");
	addPair(lh, LINKHP, tFloat, "HP");
	addPair(lh, LINKMP, tFloat, "MP");
	addPair(lh, LINKMAXHP, tFloat, "MaxHP");
	addPair(lh, LINKMAXMP, tFloat, "MaxMP");
	addPair(lh, LINKACTION, tFloat, "Action");
	addPair(lh, LINKHELD, tFloat, "HeldItem");
	addPair(lh, INPUTSTART, tBool, "InputStart");
	addPair(lh, INPUTMAP, tBool, "InputMap");
	addPair(lh, INPUTUP, tBool, "InputUp");
	addPair(lh, INPUTDOWN, tBool, "InputDown");
	addPair(lh, INPUTLEFT, tBool, "InputLeft");
	addPair(lh, INPUTRIGHT, tBool, "InputRight");
	addPair(lh, INPUTA, tBool, "InputA");
	addPair(lh, INPUTB, tBool, "InputB");
	addPair(lh, INPUTL, tBool, "InputL");
	addPair(lh, INPUTR, tBool, "InputR");
	addPair(lh, INPUTEX1, tBool, "InputEx1");
	addPair(lh, INPUTEX2, tBool, "InputEx2");
	addPair(lh, INPUTEX3, tBool, "InputEx3");
	addPair(lh, INPUTEX4, tBool, "InputEx4");
	addPair(lh, INPUTPRESSSTART, tBool, "PressStart");
	addPair(lh, INPUTPRESSMAP, tBool, "PressMap");
	addPair(lh, INPUTPRESSUP, tBool, "PressUp");
	addPair(lh, INPUTPRESSDOWN, tBool, "PressDown");
	addPair(lh, INPUTPRESSLEFT, tBool, "PressLeft");
	addPair(lh, INPUTPRESSRIGHT, tBool, "PressRight");
	addPair(lh, INPUTPRESSA, tBool, "PressA");
	addPair(lh, INPUTPRESSB, tBool, "PressB");
	addPair(lh, INPUTPRESSL, tBool, "PressL");
	addPair(lh, INPUTPRESSR, tBool, "PressR");
	addPair(lh, INPUTPRESSEX1, tBool, "PressEx1");
	addPair(lh, INPUTPRESSEX2, tBool, "PressEx2");
	addPair(lh, INPUTPRESSEX3, tBool, "PressEx3");
	addPair(lh, INPUTPRESSEX4, tBool, "PressEx4");
	addPair(lh, INPUTMOUSEX, tFloat, "InputMouseX");
	addPair(lh, INPUTMOUSEY, tFloat, "InputMouseY");
	addPair(lh, INPUTMOUSEZ, tFloat, "InputMouseZ");
	addPair(lh, INPUTMOUSEB, tFloat, "InputMouseB");
	addPair(lh, LINKITEMD, tBool, "Item", 256);
	addPair(lh, LINKHXSZ, tFloat, "HitWidth");
	addPair(lh, LINKHYSZ, tFloat, "HitHeight");
	addPair(lh, LINKHZSZ, tFloat, "HitZHeight");
	addPair(lh, LINKTXSZ, tFloat, "TileWidth");
	addPair(lh, LINKTYSZ, tFloat, "TileHeight");
	addPair(lh, LINKXOFS, tFloat, "DrawXOffset");
	addPair(lh, LINKYOFS, tFloat, "DrawYOffset");
	addPair(lh, LINKZOFS, tFloat, "DrawZOffset");
	addPair(lh, LINKHXOFS, tFloat, "HitXOffset");
	addPair(lh, LINKHYOFS, tFloat, "HitYOffset");
	addPair(lh, LINKDRUNK, tFloat, "Drunk");
	addPair(lh, LINKEQUIP, tFloat, "Equipment");
	addPair(lh, INPUTAXISUP, tBool, "InputAxisUp");
	addPair(lh, INPUTAXISDOWN, tBool, "InputAxisDown");
	addPair(lh, INPUTAXISLEFT, tBool, "InputAxisLeft");
	addPair(lh, INPUTAXISRIGHT, tBool, "InputAxisRight");
	addPair(lh, INPUTPRESSAXISUP, tBool, "PressAxisUp");
	addPair(lh, INPUTPRESSAXISDOWN, tBool, "PressAxisDown");
	addPair(lh, INPUTPRESSAXISLEFT, tBool, "PressAxisLeft");
	addPair(lh, INPUTPRESSAXISRIGHT, tBool, "PressAxisRight");
	// XXX Invisible getter was set as Float?
	addPair(lh, LINKINVIS, tBool, "Invisible");
	// XXX CollDetection getter was set as Float?
	addPair(lh, LINKINVINC, tBool, "CollDetection");
	addPair(lh, LINKMISCD, tFloat, "Misc", 32);
	lh.addGetter(LINKLADDERX, tFloat, "LadderX");
	lh.addGetter(LINKLADDERY, tFloat, "LadderY");
	addPair(lh, LINKTILE, tFloat, "Tile");
	addPair(lh, LINKFLIP, tFloat, "Flip");
	addPair(lh, LINKINVFRAME, tFloat, "InvFrames");
	addPair(lh, LINKCANFLICKER, tBool, "InvFlicker");
	addPair(lh, LINKHURTSFX, tFloat, "HurtSound");

	// What are these supposed to do? Is Get/SetItemA sufficient?
	// addPair(lh, LINKUSINGITEM, tFloat, "UsingItem");

	// Not sure what the 'attack' var is used for at present, but that is
	// what etItemA returned, so I'm renaming these to ->Attack
	// addPair(lh, LINKUSINGITEMA, tFloat, "Attack");

	// addPair(lh, LINKITEMA, tFloat, "ItemA");
	// addPair(lh, LINKITEMB, tFloat, "ItemB");

	addPair(lh, LINKEATEN, tFloat, "Eaten");
	addPair(lh, LINKDIAG, tBool, "Diagonal");
	addPair(lh, LINKBIGHITBOX, tBool, "BigHitbox");


	// void Link->Warp(float, float)
    {
	    Function& function = lh.addFunction(
			    tVoid, "Warp", tFloat, tFloat, tEnd);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        //pop ffc, and ignore it
        code.push_back(new OPopRegister(new VarArgument(SFTEMP)));
        //ffc must be this (link is not a user-accessible type)
        code.push_back(new OWarp(new VarArgument(EXP2), new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
    
    // void Link->PitWarp(float, float)
    {
	    Function& function = lh.addFunction(
			    tVoid, "PitWarp", tFloat, tFloat, tEnd);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        //pop ffc, and ignore it
        code.push_back(new OPopRegister(new VarArgument(SFTEMP)));
        //ffc must be this (link is not a user-accessible type)
        code.push_back(new OPitWarp(new VarArgument(EXP2), new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
    
    // void Link->SetItemSlot(float item, float slot, float force)
    {
	    Function& function = lh.addFunction(
			    tVoid, "SetItemSlot", tFloat, tFloat, tFloat, tEnd);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(SFTEMP));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(SETITEMSLOT), new VarArgument(SFTEMP)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
    
    // void Link->SetItemA(float itemId)
    {
	    Function& function = lh.addFunction(
			    tVoid, "SetItemA", tFloat, tEnd);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        //code.push_back(new OPopRegister(new VarArgument(INDEX)));
        code.push_back(new OSetRegister(new VarArgument(GAMESETA), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
    
    // void Link->SetItemB(float itemId)
    {
	    Function& function = lh.addFunction(
			    tVoid, "SetItemB", tFloat, tEnd);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        //code.push_back(new OPopRegister(new VarArgument(INDEX)));
        code.push_back(new OSetRegister(new VarArgument(GAMESETB), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
    
    // void Link->SelectAWeapon(float)
    {
	    Function& function = lh.addFunction(
			    tVoid, "SelectAWeapon", tFloat, tEnd);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSelectAWeaponRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }

    // void Link->SelectBWeapon(float)
    {
	    Function& function = lh.addFunction(
			    tVoid, "SelectBWeapon", tFloat, tEnd);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSelectBWeaponRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
}
