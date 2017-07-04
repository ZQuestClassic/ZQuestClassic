#include "WeaponDefinitionTable.h"
#include "../defdata.h"
#include "Quest.h"

using namespace std;


SpecialSpriteIndex::SpecialSpriteIndex()
{

}

bool SpecialSpriteIndex::checkConsistency(Quest &quest)
{
    if (!quest.isValid(messageMoreIndicator))
        return false;
    if (!quest.isValid(bushLeavesDecoration))
        return false;
    if (!quest.isValid(flowerClippingsDecoration))
        return false;
    if (!quest.isValid(grassClippingsDecoration))
        return false;
    if (!quest.isValid(tallGrassDecoration))
        return false;
    if (!quest.isValid(ripplesDecoration))
        return false;
    if (!quest.isValid(nayruShieldFront))
        return false;
    if (!quest.isValid(nayruShieldBack))
        return false;
    if (!quest.isValid(lifeMeterHearts))
        return false;
    if (!quest.isValid(enemySpawnCloud))
        return false;
    if (!quest.isValid(enemyDeathCloud))
        return false;
    if (!quest.isValid(smallShadow))
        return false;
    if (!quest.isValid(largeShadow))
        return false;
    if (!quest.isValid(linkSwim))
        return false;
    if (!quest.isValid(linkSlash))
        return false;
    if (!quest.isValid(magicMeter))
        return false;
    if (!quest.isValid(flickeringFlame))
        return false;
    if (!quest.isValid(flickeringFlame2))
        return false;
    if (!quest.isValid(flickeringFireTrail))
        return false;
    if (!quest.isValid(subscreenVine))
        return false;
    if (!quest.isValid(npcTemplate))
        return false;
    if (!quest.isValid(defaultLinkWeaponSprite))
        return false;
    if (!quest.isValid(defaultLinkWeaponSprite))
        return false;
    if (!quest.isValid(defaultEnemySwordBeamSprite))
        return false;
    if (!quest.isValid(silverSparkle))
        return false;
    if (!quest.isValid(fireSparkle))
        return false;
    if (!quest.isValid(dinsRocketTrail))
        return false;
    if (!quest.isValid(dinsRocketTrailReturn))
        return false;
    if (!quest.isValid(nayruRocketTrail))
        return false;
    if (!quest.isValid(nayruRocketTrailReturn))
        return false;
    if (!quest.isValid(nayruRocketTrail2))
        return false;
    if (!quest.isValid(nayruRocketTrailReturn2))
        return false;
    if (!quest.isValid(defaultBombExplosion))
        return false;
    if (!quest.isValid(defaultSuperBombExplosion))
        return false;
    if (!quest.isValid(defaultEnemyBomb))
        return false;
    if (!quest.isValid(defaultEnemySuperBomb))
        return false;
    if (!quest.isValid(defaultEnemyFireball))
        return false;
    if (!quest.isValid(defaultEnemyRock))
        return false;
    if (!quest.isValid(defaultEnemyArrow))
        return false;
    if (!quest.isValid(defaultEnemyMagic))
        return false;
    if (!quest.isValid(defaultEnemyWind))
        return false;
    if (!quest.isValid(defaultEnemyBombExplosion))
        return false;
    if (!quest.isValid(defaultEnemySuperBombExplosion))
        return false;

    for (int i = 0; i < 3; i++)
    {
        if (!quest.isValid(defaultBoomerangSprites[i]))
            return false;
    }
    return true;
}


void SpriteDefinitionTable::clear()
{
    spriteData_.clear();
    spriteNames_.clear();
}

string SpriteDefinitionTable::defaultSpriteName(int slot)
{
    if (slot < OS_LAST)
        return string(old_weapon_names[slot]);
    else
    {
        char buf[40];
        sprintf(buf, "zz%03d", slot);
        return string(buf);
    }
}

void SpriteDefinitionTable::addSpriteDefinition(const wpndata &data, const std::string &name)
{
    spriteData_.push_back(data);
    spriteNames_.push_back(name);
}

void SpriteDefinitionTable::setSpriteName(int idx, const std::string &name)
{
    spriteNames_[idx] = name;
}