#include "../../precompiled.h"
#include "../Library.h"
#include "../LibraryHelper.h"

#include "../ByteCode.h"
#include "../CompilerUtils.h"
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
	    Function& function = lh.addFunction(tBool, "isValid", P());
	    
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
	    Function& function = lh.addFunction(tVoid, "GetName", P() << tFloat);
	    
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
	    Function& function = lh.addFunction(tVoid, "BreakShield", P());
	    
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
