#include "../../precompiled.h"
#include "../Library.h"
#include "../LibraryHelper.h"

#include "../../zdefs.h"
#include "../ByteCode.h"
#include "../Scope.h"

using namespace ZScript;
using namespace ZScript::Libraries;

ItemClass const& ItemClass::singleton()
{
	static ItemClass const instance;
	return instance;
}

void ItemClass::addTo(Scope& scope) const
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
	DataType tEnd;
	
	LibraryHelper lh(scope, REFITEMCLASS, tItemClass);

	addPair(lh, ITEMCLASSFAMILY, tFloat, "Family");
	addPair(lh, ITEMCLASSFAMTYPE, tFloat, "Level");
	addPair(lh, ITEMCLASSAMOUNT, tFloat, "Amount");
	addPair(lh, ITEMCLASSMAX, tFloat, "Max");
	addPair(lh, ITEMCLASSSETMAX, tFloat, "MaxIncrement");
	addPair(lh, ITEMCLASSSETGAME, tBool, "Keep");
	addPair(lh, ITEMCLASSCOUNTER, tFloat, "Counter");
	addPair(lh, ITEMCLASSUSESOUND, tFloat, "UseSound");
	lh.addGetter(ITEMCLASSID, tFloat, "ID");
	addPair(lh, ITEMCLASSPOWER, tFloat, "Power");
	addPair(lh, ITEMCLASSINITDD, tFloat, "InitD", 2); // XXX Is 2 correct?
	addPair(lh, IDATALTM, tFloat, "Modifier");
	addPair(lh, IDATASCRIPT, tFloat, "Script");
	addPair(lh, IDATAPSCRIPT, tFloat, "PScript");
	addPair(lh, IDATAMAGCOST, tFloat, "MagicCost");
	addPair(lh, IDATAMINHEARTS, tFloat, "MinHearts");
	addPair(lh, IDATATILE, tFloat, "Tile");
	addPair(lh, IDATAMISC, tFloat, "Flash");
	addPair(lh, IDATACSET, tFloat, "CSet");
	// addPair(lh, IDATAFRAME, tFloat, "Frame");
	addPair(lh, IDATAFRAMES, tFloat, "AFrames");
	addPair(lh, IDATAASPEED, tFloat, "ASpeed");
	addPair(lh, IDATADELAY, tFloat, "Delay");
	addPair(lh, IDATACOMBINE, tBool, "Combine");
	addPair(lh, IDATADOWNGRADE, tBool, "Downgrade");
	addPair(lh, IDATAKEEPOLD, tBool, "KeepOld");
	addPair(lh, IDATARUPEECOST, tBool, "RupeeCost");
	addPair(lh, IDATAEDIBLE, tBool, "Edible");
	addPair(lh, IDATAFLAGUNUSED, tBool, "Unused");
	addPair(lh, IDATAGAINLOWER, tBool, "GainLower");
	addPair(lh, IDATAATTRIB, tFloat, "Attributes", 15);
	addPair(lh, IDATAATTRIB, tFloat, "Misc", 10); // XXX Same thing?
	addPair(lh, IDATAFLAGS, tBool, "Flags", 5);
	addPair(lh, IDATASPRITE, tFloat, "Sprites", 10);
	addPair(lh, IDATAUSEWPN, tFloat, "Weapon");
	addPair(lh, IDATAUSEDEF, tFloat, "Defense");
	addPair(lh, IDATAWRANGE, tFloat, "Range");
	addPair(lh, IDATADURATION, tFloat, "Duration");
	addPair(lh, IDATAUSEMVT, tFloat, "Movement", ITEM_MOVEMENT_PATTERNS);
	addPair(lh, IDATAWPNINITD, tFloat, "WeaponD", 8);
	addPair(lh, IDATAMISCD, tFloat, "WeaponMisc", 32);
	addPair(lh, IDATADUPLICATES, tFloat, "Duplicates");
	addPair(lh, IDATADRAWLAYER, tFloat, "DrawLayer");
	addPair(lh, IDATACOLLECTFLAGS, tFloat, "CollectFlags");
	addPair(lh, IDATAWEAPONSCRIPT, tFloat, "WeaponScript");
	addPair(lh, IDATAWEAPHXOFS, tFloat, "WeaponHitXOffset");
	addPair(lh, IDATAWEAPHYOFS, tFloat, "WeaponHitYOffset");
	addPair(lh, IDATAWEAPHXSZ, tFloat, "WeaponHitWidth");
	addPair(lh, IDATAWEAPHYSZ, tFloat, "WeaponHitHeight");
	addPair(lh, IDATAWEAPHZSZ, tFloat, "WeaponHitZHeight");
	addPair(lh, IDATAWEAPXOFS, tFloat, "WeaponDrawXOffset");
	addPair(lh, IDATAWEAPYOFS, tFloat, "WeaponDrawYOffset");
	addPair(lh, IDATAWEAPZOFS, tFloat, "WeaponDrawZOffset");
	
    // void ItemClass->GetName(string buffer)
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
        code.push_back(new OGetItemName(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
}
