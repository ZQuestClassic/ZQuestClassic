#include "SymbolDefs.h"

AudioSymbols AudioSymbols::singleton = AudioSymbols();

static AccessorTable AudioTable[] =
{
//	  name,                    tag,            rettype,  var,             funcFlags,  params,optparams
	{ "PlaySound",               0,          ZTID_VOID,   -1,                     0,  { ZTID_AUDIO, ZTID_FLOAT },{} },
	{ "EndSound",                0,          ZTID_VOID,   -1,                     0,  { ZTID_AUDIO, ZTID_FLOAT },{} },
	{ "PauseSound",              0,          ZTID_VOID,   -1,                     0,  { ZTID_AUDIO, ZTID_FLOAT },{} },
	{ "ResumeSound",             0,          ZTID_VOID,   -1,                     0,  { ZTID_AUDIO, ZTID_FLOAT },{} },
	{ "ContinueSound",           0,          ZTID_VOID,   -1,                     0,  { ZTID_AUDIO, ZTID_FLOAT },{} },
	{ "AdjustMusicVolume",       0,          ZTID_VOID,   -1,                     0,  { ZTID_AUDIO, ZTID_FLOAT },{} },
	{ "AdjustSFXVolume",         0,          ZTID_VOID,   -1,                     0,  { ZTID_AUDIO, ZTID_FLOAT },{} },
	{ "PauseCurMIDI",            0,          ZTID_VOID,   -1,                     0,  { ZTID_AUDIO },{} },
	{ "ResumeCurMIDI",           0,          ZTID_VOID,   -1,                     0,  { ZTID_AUDIO },{} },
	{ "PlayMIDI",                0,          ZTID_VOID,   -1,                     0,  { ZTID_AUDIO, ZTID_FLOAT },{} },
	{ "PlayEnhancedMusic",       0,          ZTID_BOOL,   -1,                     0,  { ZTID_AUDIO, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getPanStyle",             0,         ZTID_FLOAT,   AUDIOPAN,               0,  { ZTID_AUDIO },{} },
	{ "setPanStyle",             0,          ZTID_VOID,   AUDIOPAN,               0,  { ZTID_AUDIO, ZTID_FLOAT },{} },
	{ "getMIDI",                 0,         ZTID_FLOAT,   GETMIDI,                0,  { ZTID_AUDIO },{} },
	{ "setMIDI",                 0,          ZTID_VOID,   GETMIDI,        FL_RDONLY,  { ZTID_AUDIO, ZTID_FLOAT },{} },
	
	{ "AdjustSound",             0,          ZTID_VOID,   -1,               FL_INL,   { ZTID_AUDIO, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL },{ 0, -1, 0 } },
	{ "PlayOgg",                 0,          ZTID_BOOL,   -1,               FL_DEPR,  { ZTID_AUDIO, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetMusicPos",             0,         ZTID_FLOAT,   -1,                FL_INL,  { ZTID_AUDIO },{} },
	{ "SetMusicPos",             0,          ZTID_VOID,   -1,                FL_INL,  { ZTID_AUDIO, ZTID_FLOAT },{} },
	{ "SetMusicSpeed",           0,          ZTID_VOID,   -1,                FL_INL,  { ZTID_AUDIO, ZTID_FLOAT },{} },
	{ "getVolume[]",             0,         ZTID_FLOAT,   AUDIOVOLUME,      FL_DEPR,  { ZTID_AUDIO, ZTID_FLOAT },{} },
	{ "setVolume[]",             0,          ZTID_VOID,   AUDIOVOLUME,      FL_DEPR,  { ZTID_AUDIO, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "GetMusicLength",          0,         ZTID_FLOAT,   -1,                FL_INL,  { ZTID_AUDIO },{} },
	{ "SetMusicLoop",            0,          ZTID_VOID,   -1,                FL_INL,  { ZTID_AUDIO, ZTID_FLOAT, ZTID_FLOAT},{} },
	{ "PlaySoundEx",               0,          ZTID_VOID,   -1,                FL_INL,   { ZTID_AUDIO, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL },{ 0, -1, 0 } },
	{ "GetSoundCompletion",      0,         ZTID_FLOAT,   -1,                FL_INL,   { ZTID_AUDIO, ZTID_FLOAT },{ } },
	{ "CrossfadeEnhancedMusic",  0,          ZTID_BOOL,   -1,                FL_INL,   { ZTID_AUDIO, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{ 0, 0 } },
	{ "getMusicRefresh",         0,         ZTID_FLOAT,   MUSICUPDATECOND,        0,  { ZTID_AUDIO },{} },
	{ "setMusicRefresh",         0,          ZTID_VOID,   MUSICUPDATECOND,        0,  { ZTID_AUDIO, ZTID_FLOAT },{} },
	{ "getMusicRefreshFlags[]",  0,          ZTID_BOOL,   MUSICUPDATEFLAGS,       0,  { ZTID_AUDIO, ZTID_FLOAT },{} },
	{ "setMusicRefreshFlags[]",  0,          ZTID_VOID,   MUSICUPDATEFLAGS,       0,  { ZTID_AUDIO, ZTID_FLOAT, ZTID_BOOL },{} },

	{ "",                        0,          ZTID_VOID,   -1,          0,  {},{} }
};

AudioSymbols::AudioSymbols()
{
	table = AudioTable;
	refVar = NUL;
}

void AudioSymbols::generateCode()
{
	//void AdjustVolume(audio, int32_t)
	{
		Function* function = getFunction("AdjustMusicVolume");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OAdjustVolumeRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void AdjustSFXVolume(audio, int32_t)
	{
		Function* function = getFunction("AdjustSFXVolume");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OAdjustSFXVolumeRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	
	//void AdjustSound(game, int32_t,int32_t,int32_t,bool)
	{
		Function* function = getFunction("AdjustSound");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2(code, new OAdjustSound());
		LABELBACK(label);
		POP_ARGS(5, NUL);
		//pop pointer
		POPREF();
		RETURN();
		function->giveCode(code);
	}
	//void PlaySound(game, int32_t)
	{
		Function* function = getFunction("PlaySound", 0);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OPlaySoundRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	
	//void EndSound(game, int32_t)
	{
		Function* function = getFunction("EndSound");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OEndSoundRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	
	//void PauseSound(game, int32_t)
	{
		Function* function = getFunction("PauseSound");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OPauseSoundRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	
	//void ContinueSound(game, int32_t)
	{
		Function* function = getFunction("ContinueSound");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OContinueSFX(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	
	//void ResumeSound(game, int32_t)
	{
		Function* function = getFunction("ResumeSound");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OResumeSoundRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	
	//void PauseCurMIDI(game)
	{
		Function* function = getFunction("PauseCurMIDI");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer, and ignore it
		ASSERT_NUL();
		addOpcode2 (code, new OPauseMusic());
		LABELBACK(label);
		RETURN();
		function->giveCode(code);
	}
	
	//void ResumeCurMIDI(game)
	{
		Function* function = getFunction("ResumeCurMIDI");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer, and ignore it
		ASSERT_NUL();
		addOpcode2 (code, new OResumeMusic());
		LABELBACK(label);
		RETURN();
		function->giveCode(code);
	}
	//void PlayMIDI(game, int32_t)
	{
		Function* function = getFunction("PlayMIDI");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OPlayMIDIRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void PlayEnhancedMusic(game, int32_t, int32_t)
	{
		Function* function = getFunction("PlayEnhancedMusic");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OPlayEnhancedMusic(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void PlayEnhancedMusicEx(game, int32_t, int32_t)
	{
		Function* function = getFunction("PlayOgg");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OPlayEnhancedMusicEx(new VarArgument(EXP2), new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t GetEnhancedMusicPos(game)
	{
		Function* function = getFunction("GetMusicPos");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer, and ignore it
		ASSERT_NUL();
		addOpcode2 (code, new OGetEnhancedMusicPos(new VarArgument(EXP1)));
		LABELBACK(label);
		RETURN();
		function->giveCode(code);
	}
	 //void SetEnhancedMusicPos(game, int32_t)
	{
		Function* function = getFunction("SetMusicPos");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSetEnhancedMusicPos(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void SetEnhancedMusicSpeed(game, int32_t)
	{
		Function* function = getFunction("SetMusicSpeed");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSetEnhancedMusicSpeed(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t GetEnhancedMusicLength(game)
	{
		Function* function = getFunction("GetMusicLength");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer, and ignore it
		ASSERT_NUL();
		addOpcode2(code, new OGetEnhancedMusicLength(new VarArgument(EXP1)));
		LABELBACK(label);
		RETURN();
		function->giveCode(code);
	}
	//void SetEnhancedMusicLoop(paldata, int32_t, int32_t)
	{
		Function* function = getFunction("SetMusicLoop");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2(code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2(code, new OPopRegister(new VarArgument(EXP1)));
		//pop pointer
		POPREF();
		addOpcode2(code, new OSetEnhancedMusicLoop(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//void PlaySound(game, int32_t,int32_t,int32_t,int32_t,bool)
	{
		Function* function = getFunction("PlaySoundEx");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2(code, new OPlaySoundEX());
		LABELBACK(label);
		POP_ARGS(5, NUL);
		//pop pointer
		POPREF();
		RETURN();
		function->giveCode(code);
	}
	//void GetSoundCompletion(game, int32_t)
	{
		Function* function = getFunction("GetSoundCompletion");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2(code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer, and ignore it
		POPREF();
		addOpcode2(code, new OGetSoundCompletion(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void CrossfadeEnhancedMusic(game, int32_t,int32_t,int32_t,int32_t)
	{
		Function* function = getFunction("CrossfadeEnhancedMusic", 0);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2(code, new OCrossfadeEnhancedMusic());
		LABELBACK(label);
		POP_ARGS(6, NUL);
		//pop pointer
		POPREF();
		RETURN();
		function->giveCode(code);
	}
}

