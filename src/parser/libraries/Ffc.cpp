#include "../../precompiled.h"
#include "../Library.h"
#include "../LibraryHelper.h"

#include "../../scripting/ZAsmVariables.h"
#include "../CompilerUtils.h"
#include "../Opcode.h"
#include "../Scope.h"

using namespace ZScript;
using namespace ZScript::Libraries;
using namespace ZAsm;

Ffc const& Ffc::singleton()
{
	static Ffc const instance;
	return instance;
}

void Ffc::addTo(Scope& scope) const
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
	
	LibraryHelper lh(scope, varREFFFC(), tFfc);

	addPair(lh, varDATA(), tFloat, "Data");
	addPair(lh, varFFSCRIPT(), tFloat, "Script");
	addPair(lh, varCSET(), tFloat, "CSet");
	addPair(lh, varDELAY(), tFloat, "Delay");
	addPair(lh, varX(), tFloat, "X");
	addPair(lh, varY(), tFloat, "Y");
	addPair(lh, varXD(), tFloat, "Vx");
	addPair(lh, varYD(), tFloat, "Vy");
	addPair(lh, varXD2(), tFloat, "Ax");
	addPair(lh, varYD2(), tFloat, "Ay");
	addPair(lh, varFFFLAGSD(), tBool, "Flags", 2); // XXX is this really 2?
	addPair(lh, varFFTWIDTH(), tFloat, "TileWidth");
	addPair(lh, varFFTHEIGHT(), tFloat, "TileHeight");
	addPair(lh, varFFCWIDTH(), tFloat, "EffectWidth");
	addPair(lh, varFFCHEIGHT(), tFloat, "EffectHeight");
	addPair(lh, varFFLINK(), tFloat, "Link");
	addPair(lh, varFFMISCD(), tFloat, "Misc", 16);
	addPair(lh, varFFINITDD(), tFloat, "InitD", 8);
	// addPair(lh, varFFDD(), tFloat, "D", 8);
	addPair(lh, varFFCID(), tFloat, "ID");

	// void ffc->ChangeFFCScript(float script)
	defineFunction(
			lh, tVoid, "ChangeFFCScript",
			P() << tFloat, R() << varExp1(),
			opCHANGEFFSCRIPTR(varExp1()));

	/*
    // bool ffc->WasTriggered()
    {
	    Function& function = lh.addFunction(tBool, "WasTriggered", P());
	    
	    int label = function.getLabel();
	    vector<Opcode *> code;
	    //pop ffc
	    Opcode *first = opPOP(varExp2());
	    first->setLabel(label);
	    code.push_back(first);
	    //if ffc = -1, it is "this"
	    int thislabel = ScriptParser::getUniqueLabelID();
	    code.push_back(new OCompareImmediate(varExp2(), -1));
	    code.push_back(new OGotoTrueImmediate(thislabel));
	    //if not this
	    //NOT POSSIBLE YET
	    //QUIT
	    code.push_back(new OQuit());
	    //if "this"
	    code.push_back(new OCheckTrig());
	    int truelabel = ScriptParser::getUniqueLabelID();
	    code.push_back(new OGotoTrueImmediate(truelabel));
	    code.push_back(new OSetImmediate(varExp1(), 0));
	    code.push_back(opPOP(varExp2()));
	    code.push_back(new OGotoRegister(varExp2()));
	    Opcode *next = new OSetImmediate(varExp1(), 1);
	    next->setLabel(truelabel);
	    code.push_back(next);
	    code.push_back(opPOP(varExp2()));
	    code.push_back(new OGotoRegister(varExp2()));
	    function.giveCode(code);
    }
	*/
}
