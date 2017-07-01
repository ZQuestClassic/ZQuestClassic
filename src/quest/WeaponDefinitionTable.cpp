#include "WeaponDefinitionTable.h"
#include "../defdata.h"

using namespace std;

void WeaponDefinitionTable::clear()
{
    weaponData_.clear();
    weaponNames_.clear();
}

string WeaponDefinitionTable::defaultWeaponName(int slot)
{
    if (slot < wLast)
        return string(old_weapon_names[slot]);
    else
    {
        char buf[6];
        sprintf(buf, "zz%03d", slot);
        return string(buf);
    }
}

void WeaponDefinitionTable::addWeaponDefinition(const wpndata &data, const std::string &name)
{
    weaponData_.push_back(data);
    weaponNames_.push_back(name);
}

void WeaponDefinitionTable::setWeaponName(int idx, const std::string &name)
{
    weaponNames_[idx] = name;
}