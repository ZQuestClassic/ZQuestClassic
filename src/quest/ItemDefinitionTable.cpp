#include "ItemDefinitionTable.h"
#include "../defdata.h"

//using namespace std; STOP USING NAMESPACE STD!!! (It breaks compiles)
using std::string;


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

int ItemDefinitionTable::getCanonicalItemID(int family)
{
    int lowestid = -1;
    int lowestlevel = -1;

    for(int i=0; i<getNumItemDefinitions(); i++)
    {
        if(itemData_[i].family == family && (itemData_[i].fam_type < lowestlevel || lowestlevel == -1))
        {
            lowestlevel = itemData_[i].fam_type;
            lowestid = i;
        }
    }

    return lowestid;
}

int ItemDefinitionTable::getItemID(int family, int level)
{
    if(level<0) return getCanonicalItemID(family);

    for(int i=0; i<getNumItemDefinitions(); i++)
    {
        if(itemData_[i].family == family && itemData_[i].fam_type == level)
            return i;
    }

    return -1;
}

void ItemDefinitionTable::addOldStyleFamily(zinitdata *dest, int family, char levels)
{
    for(int i=0; i<8; i++)
    {
        if(levels & (1<<i))
        {
            int id = getItemID(family, i+1);

            if (id != -1)
            {
                dest->inventoryItems.insert(id);
            }
        }
    }
}

void ItemDefinitionTable::removeItemsOfFamily(gamedata *g, int family)
{
    for(int i=0; i<getNumItemDefinitions(); i++)
    {
        if(itemData_[i].family == family)
            g->set_item(i,false);
    }
}

int ItemDefinitionTable::getHighestLevelOfFamily(zinitdata *source, int family)
{
    int result = -1;
    int highestlevel = -1;

    for(int i=0; i<getNumItemDefinitions(); i++)
    {
        if(itemData_[i].family == family && source->inventoryItems.count(i)>0)
        {
            if(itemData_[i].fam_type >= highestlevel)
            {
                highestlevel = itemData_[i].fam_type;
                result=i;
            }
        }
    }

    return result;
}

int ItemDefinitionTable::getHighestLevelOfFamily(gamedata *source, int family, bool checkenabled)
{
    int result = -1;
    int highestlevel = -1;

    for(int i=0; i<getNumItemDefinitions(); i++)
    {
        if(itemData_[i].family == family && source->get_item(i) && (checkenabled?(!(source->get_disabled_item(i))):1))
        {
            if(itemData_[i].fam_type >= highestlevel)
            {
                highestlevel = itemData_[i].fam_type;
                result=i;
            }
        }
    }

    return result;
}

int ItemDefinitionTable::getItemIDPower(int family, int power)
{
    for(int i=0; i<getNumItemDefinitions(); i++)
    {
        if(itemData_[i].family == family && itemData_[i].power == power)
            return i;
    }

    return -1;
}

int ItemDefinitionTable::getItemFamily(int item)
{
    return itemData_[item].family;
}

void ItemDefinitionTable::removeLowerLevelItemsOfFamily(gamedata *g, int family, int level)
{
    for(int i=0; i<getNumItemDefinitions(); i++)
    {
        if(itemData_[i].family == family && itemData_[i].fam_type < level)
            g->set_item(i, false);
    }
}

void ItemDefinitionTable::removeItemsOfFamily(zinitdata *z, int family)
{
    for(int i=0; i<getNumItemDefinitions(); i++)
    {
        if (itemData_[i].family == family)
        {
            std::set<uint32_t>::iterator it = z->inventoryItems.find(i);
            if (it != z->inventoryItems.end())
                z->inventoryItems.erase(*it);
        }
    }
}

int ItemDefinitionTable::computeOldStyleBitfield(zinitdata *source, int family)
{
    int rval=0;

    for(int i=0; i<getNumItemDefinitions(); i++)
    {
        if(itemData_[i].family == family && source->inventoryItems.count(i)>0)
        {
            if(itemData_[i].fam_type > 0)
                rval |= 1<<(itemData_[i].fam_type-1);
        }
    }

    return rval;
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