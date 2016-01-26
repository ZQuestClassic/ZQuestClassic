//Makes sure the correct version of the ZScript interpreter is called based on
//the quest file's version, thus allowing for a re-write of the interpreter,
//which would otherwise break scripts from the LI build
//~Joe123

#ifndef _ZSCRIPTVER_H
#define _ZSCRIPTVER_H

#include "zdefs.h"
#include "ffscript.h"

class ZScriptVersion
{
public:
    //If checks are done at setVersion rather than during play-time
    static inline void setVersion(int newVersion)
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
    static inline int RunScript(const byte type, const word script, const byte i = -1)
    {
        return (*Interpreter)(type, script, i);
    }
    
    static inline void RunScrollingScript(int scrolldir, int cx, int sx, int sy, bool end_frames)
    {
        (*onScrolling)(scrolldir, cx, sx, sy, end_frames);
    }
    
private:
    static int CurrentVersion;
    static int (*Interpreter)(const byte, const word, const byte);
    static void (*onScrolling)(int, int, int, int, bool);
    
    //Couldn't do anything with old ZScript
    static inline void NullScrollingScript(int, int, int, int, bool) { }
    static void ScrollingScript(int scrolldir, int cx, int sx, int sy, bool end_frames);
};

#endif
