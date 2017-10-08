#include "../../precompiled.h"
#include "../Library.h"
#include "../LibraryHelper.h"

#include "../Scope.h"

using namespace ZScript;
using namespace ZScript::Libraries;

Audio const& Audio::singleton()
{
	static Audio const instance;
	return instance;
}

void Audio::addTo(Scope& scope) const
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
	
	LibraryHelper lh(scope, varNull(), tAudio);

	// void Audio->AdjustSound(float, float, bool)
	// ADJUSTSFX is a command, not a variable, so this was done up wrong.
	/*
	defineFunction(
			lh, tVoid, "AdjustSound",
			P() << tFloat   << tFloat    << tBool,
			R() << varIndex1() << varIndex2() << varSFTemp(),
			opSETR(ZAsm::varADJUSTSFX(), varSFTemp()));
	*/

    // void Audio->PlaySound(float)
    defineFunction(
		    lh, tVoid, "PlaySound",
		    P() << tFloat, R() << varExp1(),
		    opPLAYSOUNDR(varExp1()));
    
    // void Audio->EndSound(float)
    defineFunction(
		    lh, tVoid, "EndSound",
		    P() << tFloat, R() << varExp1(),
		    opENDSOUNDR(varExp1()));
    
    // void Audio->PauseSound(float)
    defineFunction(
		    lh, tVoid, "PauseSound",
		    P() << tFloat, R() << varExp1(),
		    opPAUSESOUNDR(varExp1()));
    
    // void Audio->ContinueSound(float)
    defineFunction(
		    lh, tVoid, "ContinueSound",
		    P() << tFloat, R() << varExp1(),
		    opCONTINUESFX(varExp1()));
    
    // void Audio->ResumeSound(float)
    defineFunction(
		    lh, tVoid, "ResumeSound",
		    P() << tFloat, R() << varExp1(),
		    opRESUMESOUNDR(varExp1()));
    
    // void Audio->PauseMusic()
    defineFunction(lh, "PauseMusic", opPAUSEMUSIC());
    
    // void Audio->ResumeMusic()
    defineFunction(lh, "ResumeMusic", opRESUMEMUSIC());

    // void Audio->PlayMIDI(float)
    defineFunction(
		    lh, tVoid, "PlayMIDI",
		    P() << tFloat, R() << varExp1(),
		    opPLAYMIDIR(varExp1()));

    // void Audio->PlayEnhancedMusic(float, float)
    defineFunction(
		    lh, tVoid, "PlayEnhancedMusic",
		    P() << tFloat  << tFloat,
		    R() << varExp2() << varExp1(),
		    opPLAYENHMUSIC(varExp2(), varExp1()));
}
