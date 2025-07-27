//Makes sure the correct version of the ZScript interpreter is called based on
//the quest file's version, thus allowing for a re-write of the interpreter,
//which would otherwise break scripts from the LI build
//~Joe123

#ifndef ZSCRIPTVER_H_
#define ZSCRIPTVER_H_

#include "base/zdefs.h"
#include "zc/ffscript.h"

extern FFScript FFCore;

class ZScriptVersion
{
public:
    static inline void setVersion(int32_t newVersion)
    {
        CurrentVersion = newVersion;
    }

    static inline bool gc()
    {
        return CurrentVersion >= 25;
    }

    static inline bool ffcRefIsSpriteId()
    {
        return CurrentVersion >= 26;
    }

    static inline bool singleZasmChunk()
    {
        return CurrentVersion >= 27;
    }

    // 1. User arrays are managed by the garbage collector (see script_arrays).
    // 2. Internal arrays can be passed around by reference, and support SizeOfArray, foreach.
    static inline bool gc_arrays()
    {
        return CurrentVersion >= 28;
    }
    
    static inline int32_t RunScript(ScriptType type, const word script, const int32_t i = 0)
    {
        return run_script(type, script, i);
    }
    
    static inline void RunScrollingScript(int32_t scrolldir, int32_t cx, int32_t sx, int32_t sy, bool end_frames, bool waitdraw)
    {
        if (CurrentVersion >= 6)
            ScrollingScript(scrolldir, cx, sx, sy, end_frames,waitdraw);
    }
    
private:
    static int32_t CurrentVersion;
    
    static void ScrollingScript(int32_t scrolldir, int32_t cx, int32_t sx, int32_t sy, bool end_frames, bool waitdraw);
};

#endif
