#include "../../precompiled.h"
#include "../Library.h"
#include "../LibraryHelper.h"

#include "../ByteCode.h"
#include "../Scope.h"

using namespace ZScript;
using namespace ZScript::Libraries;

Npc const& Npc::singleton()
{
	static Npc const instance;
	return instance;
}

void Npc::addTo(Scope& scope) const
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
	
	LibraryHelper lh(scope, REFNPC, tNpc);

	addPair(lh, NPCX, tFloat, "X");
	addPair(lh, NPCY, tFloat, "Y");
	addPair(lh, NPCZ, tFloat, "Z");
	addPair(lh, NPCJUMP, tFloat, "Jump");
	addPair(lh, NPCDIR, tFloat, "Dir");
	addPair(lh, NPCRATE, tFloat, "Rate");
	addPair(lh, NPCHOMING, tFloat, "Homing");
	addPair(lh, NPCSTEP, tFloat, "Step");
	addPair(lh, NPCFRAMERATE, tFloat, "ASpeed");
	addPair(lh, NPCHALTRATE, tFloat, "Haltrate");
	addPair(lh, NPCDRAWTYPE, tFloat, "DrawStyle");
	addPair(lh, NPCHP, tFloat, "HP");
	addPair(lh, NPCID, tFloat, "ID");
	addPair(lh, NPCTYPE, tFloat, "Type");
	addPair(lh, NPCDP, tFloat, "Damage");
	addPair(lh, NPCWDP, tFloat, "WeaponDamage");
	addPair(lh, NPCTILE, tFloat, "Tile");
	addPair(lh, NPCOTILE, tFloat, "OriginalTile");
	addPair(lh, NPCWEAPON, tFloat, "Weapon");
	addPair(lh, NPCITEMSET, tFloat, "ItemSet");
	addPair(lh, NPCCSET, tFloat, "CSet");
	addPair(lh, NPCBOSSPAL, tFloat, "BossPal");
	addPair(lh, NPCBGSFX, tFloat, "SFX");
	addPair(lh, NPCEXTEND, tFloat, "Extend");
	addPair(lh, NPCHXSZ, tFloat, "HitWidth");
	addPair(lh, NPCHYSZ, tFloat, "HitHeight");
	addPair(lh, NPCHZSZ, tFloat, "HitZHeight");
	addPair(lh, NPCTXSZ, tFloat, "TileWidth");
	addPair(lh, NPCTYSZ, tFloat, "TileHeight");
	addPair(lh, NPCXOFS, tFloat, "DrawXOffset");
	addPair(lh, NPCYOFS, tFloat, "DrawYOffset");
	addPair(lh, NPCZOFS, tFloat, "DrawZOffset");
	addPair(lh, NPCHXOFS, tFloat, "HitXOffset");
	addPair(lh, NPCHYOFS, tFloat, "HitYOffset");
	addPair(lh, NPCMISCD, tFloat, "Misc", 32);
	addPair(lh, NPCDD, tFloat, "Attributes", 15);
	addPair(lh, NPCMFLAGS, tFloat, "MiscFlags");
	addPair(lh, NPCCOLLDET, tBool, "CollDetection"); // XXX Getter was float?
	addPair(lh, NPCSTUN, tFloat, "Stun");
	addPair(lh, NPCDEFENSED, tFloat, "Defense", 40);
	addPair(lh, NPCHUNGER, tFloat, "Hunger");
	addPair(lh, NPCWEAPSPRITE, tFloat, "WeaponSprite");
	// 2.future srit compatibility. -Z
	addPair(lh, NPCSCRDEFENSED, tFloat, "ScriptDefense", 10);
	addPair(lh, NPCINVINC, tFloat, "InvFrames");
	addPair(lh, NPCSUPERMAN, tFloat, "Invincible");
	addPair(lh, NPCRINGLEAD, tBool, "Ringleader");
	addPair(lh, NPCHASITEM, tBool, "HasItem");
	
    // bool Npc->isValid()
    {
	    Function& function = lh.addFunction(tBool, "isValid", tEnd);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop off the pointer
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //Check validity
        code.push_back(new OIsValidNPC(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
    
    // void Npc->GetName(float bufferId)
    {
	    Function& function = lh.addFunction(tVoid, "GetName", tFloat, tEnd);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetNPCName(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
    
    // void Npc->BreakShield()
    {
	    Function& function = lh.addFunction(tVoid, "BreakShield", tEnd);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop off the pointer
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //Break shield
        code.push_back(new OBreakShield(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
}
