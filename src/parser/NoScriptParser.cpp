#include "../precompiled.h" //2.53 Updated to 16th Jan, 2017
#include "../zsys.h"
#include "Compiler.h"
#include <boost/move/unique_ptr.hpp>

namespace ZScript
{

boost::movelib::unique_ptr<ScriptsData> compile(std::string const& filename)
{
    box_out("Cannot compile ZScript: ZQuest was compiled without ZScript support!");
    box_eol();

    return nullptr;
}

// These still need to be defined somewhere to prevent linking errors.

//ZScript::ScriptType ZScript::ScriptType::
bool operator==(const ScriptType&, const ScriptType&)
{
    assert(false);
    return false;
}

ScriptType const ScriptType ::invalid;
ScriptType const ScriptType::global;
ScriptType const ScriptType::ffc;
ScriptType const ScriptType::item;
ScriptType const ScriptType::npc;
ScriptType const ScriptType::eweapon;
ScriptType const ScriptType::lweapon;
ScriptType const ScriptType::link;
ScriptType const ScriptType::dmapdata;
ScriptType const ScriptType::screendata;
ScriptType const ScriptType::itemsprite;
ScriptType const ScriptType::untyped;
ScriptType const ScriptType::subscreendata;
ScriptType const ScriptType::combodata;

}
