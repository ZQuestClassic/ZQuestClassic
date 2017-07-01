#include "WeaponDefinitionTable.h"
#include "../defdata.h"

using namespace std;


SpecialSpriteIndex::SpecialSpriteIndex() :
    messageMoreIndicator(0),
    bushLeavesDecoration(0),
    flowerClippingsDecoration(0),
    grassClippingsDecoration(0),
    tallGrassDecoration(0),
    ripplesDecoration(0),
    nayruShieldFront(0),
    nayruShieldBack(0),
    lifeMeterHearts(0),
    enemySpawnCloud(0),
    enemyDeathCloud(0),
    smallShadow(0),
    largeShadow(0),
    linkSwim(0),
    linkSlash(0),
    magicMeter(0),
    flickeringFlame(0),
    flickeringFlame2(0),
    flickeringFireTrail(0),
    subscreenVine(0),
    npcTemplate(0),
    defaultLinkWeaponSprite(0),
    defaultFireSprite(0),
    defaultBombExplosion(0),
    defaultSuperBombExplosion(0),
    silverSparkle(0),
    fireSparkle(0),
    dinsRocketTrail(0),
    dinsRocketTrailReturn(0),
    nayruRocketTrail(0),
    nayruRocketTrailReturn(0),
    nayruRocketTrail2(0),
    nayruRocketTrailReturn2(0),
    defaultEnemySwordBeamSprite(0),
    defaultEnemyBomb(0),
    defaultEnemySuperBomb(0),
    defaultEnemyFireball(0),
    defaultEnemyRock(0),
    defaultEnemyArrow(0),
    defaultEnemyMagic(0),
    defaultEnemyWind(0),
    defaultEnemyBombExplosion(0),
    defaultEnemySuperBombExplosion(0)

{
    for (int i = 0; i < 3; i++)
        defaultBoomerangSprites[i] = 0;
}

bool SpecialSpriteIndex::checkConsistency(SpriteDefinitionTable &table)
{
    if (!table.isValid(messageMoreIndicator))
        return false;
    if (!table.isValid(bushLeavesDecoration))
        return false;
    if (!table.isValid(flowerClippingsDecoration))
        return false;
    if (!table.isValid(grassClippingsDecoration))
        return false;
    if (!table.isValid(tallGrassDecoration))
        return false;
    if (!table.isValid(ripplesDecoration))
        return false;
    if (!table.isValid(nayruShieldFront))
        return false;
    if (!table.isValid(nayruShieldBack))
        return false;
    if (!table.isValid(lifeMeterHearts))
        return false;
    if (!table.isValid(enemySpawnCloud))
        return false;
    if (!table.isValid(enemyDeathCloud))
        return false;
    if (!table.isValid(smallShadow))
        return false;
    if (!table.isValid(largeShadow))
        return false;
    if (!table.isValid(linkSwim))
        return false;
    if (!table.isValid(linkSlash))
        return false;
    if (!table.isValid(magicMeter))
        return false;
    if (!table.isValid(flickeringFlame))
        return false;
    if (!table.isValid(flickeringFlame2))
        return false;
    if (!table.isValid(flickeringFireTrail))
        return false;
    if (!table.isValid(subscreenVine))
        return false;
    if (!table.isValid(npcTemplate))
        return false;
    if (!table.isValid(defaultLinkWeaponSprite))
        return false;
    if (!table.isValid(defaultLinkWeaponSprite))
        return false;
    if (!table.isValid(defaultEnemySwordBeamSprite))
        return false;
    if (!table.isValid(silverSparkle))
        return false;
    if (!table.isValid(fireSparkle))
        return false;
    if (!table.isValid(dinsRocketTrail))
        return false;
    if (!table.isValid(dinsRocketTrailReturn))
        return false;
    if (!table.isValid(nayruRocketTrail))
        return false;
    if (!table.isValid(nayruRocketTrailReturn))
        return false;
    if (!table.isValid(nayruRocketTrail2))
        return false;
    if (!table.isValid(nayruRocketTrailReturn2))
        return false;
    if (!table.isValid(defaultBombExplosion))
        return false;
    if (!table.isValid(defaultSuperBombExplosion))
        return false;
    if (!table.isValid(defaultEnemyBomb))
        return false;
    if (!table.isValid(defaultEnemySuperBomb))
        return false;
    if (!table.isValid(defaultEnemyFireball))
        return false;
    if (!table.isValid(defaultEnemyRock))
        return false;
    if (!table.isValid(defaultEnemyArrow))
        return false;
    if (!table.isValid(defaultEnemyMagic))
        return false;
    if (!table.isValid(defaultEnemyWind))
        return false;
    if (!table.isValid(defaultEnemyBombExplosion))
        return false;
    if (!table.isValid(defaultEnemySuperBombExplosion))
        return false;

    for (int i = 0; i < 3; i++)
    {
        if (!table.isValid(defaultBoomerangSprites[i]))
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