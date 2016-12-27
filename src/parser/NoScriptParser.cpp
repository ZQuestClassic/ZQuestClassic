#include "../precompiled.h"
#include "../zsys.h"
#include "Compiler.h"

ScriptsData * compile(const char *filename)
{
#ifndef SCRIPTPARSER_COMPILE
    box_out("Cannot compile ZScript: ZQuest was compiled without ZScript support!");
    box_eol();
#endif
    return NULL;
}
