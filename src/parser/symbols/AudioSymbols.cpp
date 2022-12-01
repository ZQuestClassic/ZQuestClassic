#include "SymbolDefs.h"

AudioSymbols AudioSymbols::singleton = AudioSymbols();

static AccessorTable AudioTable[] =
{
//	name,                     rettype,                  setorget,     var,                  numindex,      params
	{ "PlaySound",            ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_AUDIO, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "EndSound",             ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_AUDIO, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "PauseSound",           ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_AUDIO, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "ResumeSound",          ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_AUDIO, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "ContinueSound",        ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_AUDIO, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "AdjustMusicVolume",    ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_AUDIO, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "AdjustSFXVolume",      ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_AUDIO, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "AdjustSound",          ZTID_VOID,          FUNCTION,     0,                    1,             0,                                    4,           { ZTID_AUDIO, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "PauseCurMIDI",         ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZTID_AUDIO, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "ResumeCurMIDI",        ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZTID_AUDIO, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "PlayMIDI",             ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_AUDIO, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "PlayEnhancedMusic",    ZTID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      3,           { ZTID_AUDIO, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "PlayOgg",              ZTID_BOOL,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      3,           { ZTID_AUDIO, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "GetOggPos",            ZTID_FLOAT,         FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      1,           { ZTID_AUDIO, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "SetOggPos",            ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_AUDIO, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "SetOggSpeed",          ZTID_VOID,          FUNCTION,     0,                    1,             FUNCFLAG_INLINE,                      2,           { ZTID_AUDIO, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },

	
	 { "getVolume[]",          ZTID_FLOAT,         GETTER,       AUDIOVOLUME,          4,             0,                                    2,           { ZTID_AUDIO, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	 { "setVolume[]",          ZTID_VOID,          SETTER,       AUDIOVOLUME,          4,             0,                                    3,           { ZTID_AUDIO, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPanStyle",          ZTID_FLOAT,         GETTER,       AUDIOPAN,             1,             0,                                    1,           { ZTID_AUDIO, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "setPanStyle",          ZTID_VOID,          SETTER,       AUDIOPAN,             1,             0,                                    2,           { ZTID_AUDIO, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "",                     -1,                       -1,           -1,                   -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
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
        Function* function = getFunction("AdjustMusicVolume", 2);
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
        Function* function = getFunction("AdjustSFXVolume", 2);
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
    
    //void AdjustSound(game, int32_t,int32_t,bool)
    {
        Function* function = getFunction("AdjustSound", 4);
        int32_t label = function->getLabel();
        vector<shared_ptr<Opcode>> code;
        //pop off the params
        addOpcode2 (code, new OPopRegister(new VarArgument(SFTEMP)));
        LABELBACK(label);
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
        addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        POPREF();
        addOpcode2 (code, new OSetRegister(new VarArgument(ADJUSTSFX), new VarArgument(SFTEMP)));
        RETURN();
        function->giveCode(code);
    }
    //void PlaySound(game, int32_t)
    {
        Function* function = getFunction("PlaySound", 2);
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
        Function* function = getFunction("EndSound", 2);
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
        Function* function = getFunction("PauseSound", 2);
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
        Function* function = getFunction("ContinueSound", 2);
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
        Function* function = getFunction("ResumeSound", 2);
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
        Function* function = getFunction("PauseCurMIDI", 1);
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
        Function* function = getFunction("ResumeCurMIDI", 1);
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
        Function* function = getFunction("PlayMIDI", 2);
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
        Function* function = getFunction("PlayEnhancedMusic", 3);
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
        Function* function = getFunction("PlayOgg", 3);
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
	    Function* function = getFunction("GetOggPos", 1);
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
	    Function* function = getFunction("SetOggPos", 2);
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
	    Function* function = getFunction("SetOggSpeed", 2);
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
}

