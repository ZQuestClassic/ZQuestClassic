#include "WeaponDefinitionTable.h"
#include "ItemDefinitionTable.h"
#include "../defdata.h"
#include "Quest.h"

using namespace std;

bool SpecialItemIndex::checkConsistency(Quest &quest)
{
    if (!quest.isValid(dustPile))
        return false;
    if (!quest.isValid(clock))
        return false;
    if (!quest.isValid(key))
        return false;
    if (!quest.isValid(levelKey))
        return false;
    if (!quest.isValid(bossKey))
        return false;
    if (!quest.isValid(triforce))
        return false;
    if (!quest.isValid(magicContainer))
        return false;
    if (!quest.isValid(compass))
        return false;
    if (!quest.isValid(map))
        return false;
    if (!quest.isValid(redPotion))
        return false;
    if (!quest.isValid(heartContainer))
        return false;
    if (!quest.isValid(selectA))
        return false;
    if (!quest.isValid(selectB))
        return false;
    if (!quest.isValid(rupy))
        return false;
    if (!quest.isValid(heart))
        return false;
    if (!quest.isValid(heartContainerPiece))
        return false;
    if (!quest.isValid(bomb))
        return false;
    if (!quest.isValid(superBomb))
        return false;
    if (!quest.isValid(bigTriforce))
        return false;
    if (!quest.isValid(bait))
        return false;
    if (!quest.isValid(fairyMoving))
        return false;
    return true;
}

itemdata::itemdata(byte family_, byte fam_type_, byte power_, word flags_, char count_, word amount_, short setmax_, word max_, byte playsound_, 
    uint32_t wpn_, uint32_t wpn2_, uint32_t wpn3_, uint32_t wpn4_, uint32_t wpn5_, uint32_t wpn6_, uint32_t wpn7_, uint32_t wpn8_, uint32_t wpn9_, uint32_t wpn10_, 
    byte pickup_hearts_, long misc1_, long misc2_, long misc3_, long misc4_, byte magic_, byte usesound_)
{        
    clear();
    family = family_;
    fam_type = fam_type_;
    power = power_;
    flags = flags_;
    count = count_;
    amount = amount_;
    setmax = setmax_;
    max = max_;
    playsound = playsound_;
    wpns[0] = SpriteDefinitionRef("CORE", wpn_);
    wpns[1] = SpriteDefinitionRef("CORE", wpn2_);
    wpns[2] = SpriteDefinitionRef("CORE", wpn3_);
    wpns[3] = SpriteDefinitionRef("CORE", wpn4_);
    wpns[4] = SpriteDefinitionRef("CORE", wpn5_);
    wpns[5] = SpriteDefinitionRef("CORE", wpn6_);
    wpns[6] = SpriteDefinitionRef("CORE", wpn7_);
    wpns[7] = SpriteDefinitionRef("CORE", wpn8_);
    wpns[8] = SpriteDefinitionRef("CORE", wpn9_);
    wpns[9] = SpriteDefinitionRef("CORE", wpn10_);
    pickup_hearts = pickup_hearts_;
    misc1 = misc1_;
    misc2 = misc2_;
    misc3 = misc3_;
    misc4 = misc4_;
    magic = magic_;
    usesound = usesound_;
}

 string ItemDefinitionTable::defaultItemName(int slot)
{
     if (slot < iLast)
         return string(old_item_names[slot]);
     else
     {
         char buf[40];
         sprintf(buf, "zz%03d", slot);
         return string(buf);
     }
}

itemdata ItemDefinitionTable::defaultItemData(int slot)
{
    if (slot < iLast)
    {
        return getDefaultItems()[slot];
    }
    else
        return itemdata();
}

void ItemDefinitionTable::clear()
{
    itemData_.clear();
    itemNames_.clear();
}

void ItemDefinitionTable::addItemDefinition(const itemdata &data, const std::string &name)
{
    itemData_.push_back(data);
    itemNames_.push_back(name);
}

void ItemDefinitionTable::setItemName(int idx, const std::string &name)
{
    assert(idx >= 0 && idx < (int)itemNames_.size());
    itemNames_[idx] = name;
}