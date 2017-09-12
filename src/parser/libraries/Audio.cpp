#include "../../precompiled.h"
#include "../Library.h"
#include "../LibraryHelper.h"

#include "../ByteCode.h"
#include "../CompilerUtils.h"
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
	typedef VectorBuilder<int> R;
	typedef VectorBuilder<Opcode*> O;
	
	LibraryHelper lh(scope, NUL, tAudio);

	// void Audio->AdjustSound(float, float, bool)
	defineFunction(
			lh, tVoid, "AdjustSound",
			P() << tFloat << tFloat <<  tBool,
			R() <<  INDEX << INDEX2 << SFTEMP,
			new OSetRegister(new VarArgument(ADJUSTSFX),
			                 new VarArgument(SFTEMP)));

    // void Audio->PlaySound(float)
    defineFunction(
		    lh, tVoid, "PlaySound",
		    P() << tFloat, R() << EXP1,
		    new OPlaySoundRegister(new VarArgument(EXP1)));
    
    // void Audio->EndSound(float)
    defineFunction(
		    lh, tVoid, "EndSound",
		    P() << tFloat, R() << EXP1,
		    new OEndSoundRegister(new VarArgument(EXP1)));
    
    // void Audio->PauseSound(float)
    defineFunction(
		    lh, tVoid, "PauseSound",
		    P() << tFloat, R() << EXP1,
		    new OPauseSoundRegister(new VarArgument(EXP1)));
    
    // void Audio->ContinueSound(float)
    defineFunction(
		    lh, tVoid, "ContinueSound",
		    P() << tFloat, R() << EXP1,
		    new OContinueSFX(new VarArgument(EXP1)));
    
    // void Audio->ResumeSound(float)
    defineFunction(
		    lh, tVoid, "ResumeSound",
		    P() << tFloat, R() << EXP1,
		    new OResumeSoundRegister(new VarArgument(EXP1)));
    
    // void Audio->PauseMusic()
    defineFunction(lh, "PauseMusic", new OPauseMusic());
    
    // void Audio->ResumeMusic()
    defineFunction(lh, "ResumeMusic", new OResumeMusic());

    // void Audio->PlayMIDI(float)
    defineFunction(
		    lh, tVoid, "PlayMIDI",
		    P() << tFloat, R() << EXP1,
		    new OPlayMIDIRegister(new VarArgument(EXP1)));

    // void Audio->PlayEnhancedMusic(float, float)
    defineFunction(
		    lh, tVoid, "PlayEnhancedMusic",
		    P() << tFloat << tFloat,
		    R() <<   EXP2 <<   EXP1,
		    new OPlayEnhancedMusic(new VarArgument(EXP2),
		                           new VarArgument(EXP1)));
}
