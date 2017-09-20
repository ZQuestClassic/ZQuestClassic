#include "EnemyDefinitionTable.h"
#include "../defdata.h"
#include "Quest.h"

//using namespace std;

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
}

string EnemyDefinitionTable::defaultEnemyName(int slot)
{
    if (slot < OLDMAXGUYS)
        return getDefaultEnemies()[slot].name;
    else
    {
        char buf[40];
        sprintf(buf, "zz%03d", slot);
        return string(buf);
    }
}

void EnemyDefinitionTable::addEnemyDefinition(const guydata &data)
{
    guyData_.push_back(data);
}