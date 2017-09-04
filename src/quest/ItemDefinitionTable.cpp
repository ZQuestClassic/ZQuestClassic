#include "SpriteDefinitionTable.h"
#include "ItemDefinitionTable.h"
#include "../defdata.h"
#include "Quest.h"

//using namespace std; STOP USING NAMESPACE STD!!! (It breaks compiles)
using std::string;


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

itemdata::itemdata(const std::string &name_, byte family_, byte fam_type_, byte power_, word flags_, char count_, word amount_, short setmax_, word max_, byte playsound_, 
    SpriteDefinitionRef wpn_, SpriteDefinitionRef wpn2_, SpriteDefinitionRef wpn3_, SpriteDefinitionRef wpn4_, SpriteDefinitionRef wpn5_, SpriteDefinitionRef wpn6_, SpriteDefinitionRef wpn7_, SpriteDefinitionRef wpn8_, SpriteDefinitionRef wpn9_, SpriteDefinitionRef wpn10_, 
    byte pickup_hearts_, long misc1_, long misc2_, long misc3_, long misc4_, byte magic_, byte usesound_)
{        
    clear();
    name = name_;
    family = family_;
    fam_type = fam_type_;
    power = power_;
    flags = flags_;
    count = count_;
    amount = amount_;
    setmax = setmax_;
    max = max_;
    playsound = playsound_;
    wpns[0] = wpn_;
    wpns[1] = wpn2_;
    wpns[2] = wpn3_;
    wpns[3] = wpn4_;
    wpns[4] = wpn5_;
    wpns[5] = wpn6_;
    wpns[6] = wpn7_;
    wpns[7] = wpn8_;
    wpns[8] = wpn9_;
    wpns[9] = wpn10_;
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
         return getDefaultItems()[slot].name;
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
}

void ItemDefinitionTable::addItemDefinition(const itemdata &data)
{
    itemData_.push_back(data);
}
