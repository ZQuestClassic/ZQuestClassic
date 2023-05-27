
#include "precompiled.h" //always first

#include "zscriptversion.h"
#include "zc/zelda.h"
#include "zc/hero.h"

extern HeroClass Hero;

int32_t (*ZScriptVersion::Interpreter)(const byte, const word, const int32_t) = &run_script;
void (*ZScriptVersion::onScrolling)(int32_t, int32_t, int32_t, int32_t, bool, bool) = &ScrollingScript;
int32_t    ZScriptVersion::CurrentVersion = V_FFSCRIPT; //Set to current version by default

void ZScriptVersion::ScrollingScript(int32_t scrolldir, int32_t cx, int32_t sx, int32_t sy, bool end_frames, bool waitdraw)
{
    Hero.run_scrolling_script(scrolldir, cx, sx, sy, end_frames, waitdraw);
}
