#ifndef QUEST_H
#define QUEST_H

#include "../zdefs.h"
#include "ItemDefinitionTable.h"
#include "WeaponDefinitionTable.h"

// Placeholder struct for now

struct QuestModule
{
public:
    void setItemDefTable(ItemDefinitionTable &idt) { itemDefTable_ = idt; }
    ItemDefinitionTable &itemDefTable() { return itemDefTable_; }
    
    void setWeaponDefTable(SpriteDefinitionTable &wdt) { weaponDefTable_ = wdt; }
    SpriteDefinitionTable &weaponDefTable() { return weaponDefTable_; }

private:
    ItemDefinitionTable itemDefTable_;

    SpriteDefinitionTable weaponDefTable_;
};

class Quest
{
public:
    QuestModule &getModule(const std::string &name);
    void getModules(std::vector<std::string> &moduleNames);

    // convenience methods
    itemdata &getItemDefinition(const ItemDefinitionRef &ref);
    const std::string &getItemName(const ItemDefinitionRef &ref);
    void setItemName(const ItemDefinitionRef &ref, const std::string &newname);

    wpndata &getSpriteDefinition(const SpriteDefinitionRef &ref);
    const std::string &getSpriteName(const SpriteDefinitionRef &ref);
    void setSpriteName(const SpriteDefinitionRef &ref, const std::string &newname);

    bool isValid(const ItemDefinitionRef &ref);
    bool isValid(const SpriteDefinitionRef &ref);

    SpecialSpriteIndex &specialSprites() { return specialSpriteIndex_; }
    void setSpecialSpriteIndex(SpecialSpriteIndex &ssi) { specialSpriteIndex_ = ssi; }
    SpecialItemIndex &specialItems() { return specialItemIndex_; }
    void setSpecialItemIndex(SpecialItemIndex &sii) { specialItemIndex_ = sii; }

    /* 
    * Retrieves the canonical item of a given item family: the item with 
    * lowest non-0 level. Returns the wooden sword for swords, blue candle
    * for candles, etc.
    * Returns an invalid reference if no item of the given family exists.
    */
    ItemDefinitionRef getCanonicalItemID(int family);

    /* 
    * Searches the item tables for an item with the given family and fam_type
    * (level) or power. Returns the first time in the table found satisfying
    * the search criteria, or an invalid reference if no item exists.
    */    
    ItemDefinitionRef getItemFromFamilyPower(int family, int power);
    ItemDefinitionRef getItemFromFamilyLevel(int family, int level);

    /*
    * Searches the init data for the item of the given family with highest
    * level (fam_type). Returns an invalid item reference if no such item is 
    * found.
    */
    ItemDefinitionRef getHighestLevelOfFamily(zinitdata *source, int family);

    /*
    * Adds items to the init data based on old-style bitmasks (levels) 
    * specifying item levels within families.
    */
    void addOldStyleFamily(zinitdata *dest, int family, char levels);

    /*
    * Removes all items from the init data whose family matches the given
    * family (does nothing if there weren't any such items in the init
    * data in the first place).
    */
    void removeItemsOfFamily(zinitdata *z, int family);

    /*
    * Computes an old-.qst-style bit field representing which items of a
    * given family are present in the init data.
    * Should *not* be used except for obscure compatibility purposes, since
    * there is no guarantee that modern, fully-editable item sets can be
    * correctly encoded in such a bit field.
    */
    int computeOldStyleBitfield(zinitdata *source, int family);

    /*
    * Searches through the items in the game data and removes all items
    * of the given family whose level (fam_type) is lower than the given
    * treshold. Used when upgrading items removes all lower-level items
    * of the same family, such as boomerangs, etc.
    */
    void removeLowerLevelItemsOfFamily(gamedata *g, int family, int level);

    /*
    * Searches the game data for the item of the given family with highest
    * level (fam_type). Returns -1 if no such item is found.
    */
    ItemDefinitionRef getHighestLevelOfFamily(gamedata *source, int family, bool checkenabled = false);

    /*
    * Removes all items from the game data whose family matches the given
    * family (does nothing if there weren't any such items in the game
    * data in the first place).
    */
    void removeItemsOfFamily(gamedata *g, int family);

private:
    std::map<std::string, QuestModule> questModules_;

    SpecialSpriteIndex specialSpriteIndex_;
    SpecialItemIndex specialItemIndex_;
};

#endif