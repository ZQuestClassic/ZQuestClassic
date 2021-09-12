#include "../precompiled.h" //2.53 Updated to 16th Jan, 2017
#include "../zsys.h"
#include "Compiler.h"

ZScript::ScriptsData * compile(const char *filename)
{
    box_out("Cannot compile ZScript: ZQuest was compiled without ZScript support!");
    box_eol();

    return NULL;
}
