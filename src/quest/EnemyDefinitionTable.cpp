#include "EnemyDefinitionTable.h"
#include "../defdata.h"
#include "Quest.h"

using namespace std;

bool SpecialEnemyIndex::checkConsistency(Quest &quest)
{
    if (!quest.isValid(zelda))
        return false;
    if (!quest.isValid(fire))
        return false;
    if (!quest.isValid(fairy))
        return false;
    if (!quest.isValid(fairyItem))
        return false;
    if (!quest.isValid(fireballShooter))
        return false;
    if (!quest.isValid(spinTile1))
        return false;
    if (!quest.isValid(keese))
        return false;
    return true;    
}

void EnemyDefinitionTable::clear()
{
    guyData_.clear();
    guyNames_.clear();
}

string EnemyDefinitionTable::defaultEnemyName(int slot)
{
    if (slot < OLDMAXGUYS)
        return string(old_enemy_names[slot]);
    else
    {
        char buf[40];
        sprintf(buf, "zz%03d", slot);
        return string(buf);
    }
}

void EnemyDefinitionTable::addEnemyDefinition(const guydata &data, const std::string &name)
{
    guyData_.push_back(data);
    guyNames_.push_back(name);
}

void EnemyDefinitionTable::setEnemyName(int idx, const std::string &name)
{
    guyNames_[idx] = name;
}