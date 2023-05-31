//Makes sure the correct version of the ZScript interpreter is called based on
//the quest file's version, thus allowing for a re-write of the interpreter,
//which would otherwise break scripts from the LI build
//~Joe123

#ifndef _ZSCRIPTVER_H
#define _ZSCRIPTVER_H

#include "base/zdefs.h"
#include "zc/ffscript.h"
extern FFScript FFCore;

class ZScriptVersion
{
public:
    //If checks are done at setVersion rather than during play-time
    static inline void setVersion(int32_t newVersion)
    {
        CurrentVersion = newVersion;
        
        if(CurrentVersion < 6) //Old ZScript
        {
            onScrolling = &NullScrollingScript;
            Interpreter = &run_script;
        }
        else
        {
            onScrolling = &ScrollingScript;
            //Watch this space...
        }
    }
    
    //Only one if check at quest load, rather than each time we use the function
    static inline int32_t RunScript(const byte type, const word script, const int32_t i = -1)
    {
	/*
	switch(type)
	{
		case SCRIPT_PLAYER: 
		case SCRIPT_SCREEN: 
		case SCRIPT_LWPN: 
		case SCRIPT_SUBSCREEN: 
		case SCRIPT_NPC: 
		case SCRIPT_EWPN: 
		case SCRIPT_DMAP: 
		case SCRIPT_ITEMSPRITE: 
		{
			if ( FFCore.getQuestHeaderInfo(vZelda) < 0x255 ) 
			{
				if ( DEVLEVEL > 1 ) 
				{
					Z_scripterrlog("Invalid script type %d for ZC Quest Version: %x\n", type,  FFCore.getQuestHeaderInfo(vZelda));
				}
				return 0;
			}
			else return (*Interpreter)(type, script, i);
		}
		default:
			return (*Interpreter)(type, script, i);
	}
	*/
	return (*Interpreter)(type, script, i);
    }
    
    static inline void RunScrollingScript(int32_t scrolldir, int32_t cx, int32_t sx, int32_t sy, bool end_frames, bool waitdraw)
    {
        (*onScrolling)(scrolldir, cx, sx, sy, end_frames,waitdraw);
    }
    
private:
    static int32_t CurrentVersion;
    static int32_t (*Interpreter)(const byte, const word, const int32_t);
    static void (*onScrolling)(int32_t, int32_t, int32_t, int32_t, bool, bool);
    
    //Couldn't do anything with old ZScript
    static inline void NullScrollingScript(int32_t, int32_t, int32_t, int32_t, bool, bool) { }
    static void ScrollingScript(int32_t scrolldir, int32_t cx, int32_t sx, int32_t sy, bool end_frames, bool waitdraw);
};

#endif
