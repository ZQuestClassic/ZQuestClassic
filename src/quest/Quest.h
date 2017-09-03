#ifndef QUEST_H
#define QUEST_H

#include "../zdefs.h"
#include "ItemDefinitionTable.h"
#include "SpriteDefinitionTable.h"
#include "EnemyDefinitionTable.h"

/*
 * Stores all quest data that can be modularly imported/exported from the quest.
 * Currently this data includes:
 * - enemies
 * - sprites/weapons
 * - items
 * 
 * Each module consists of a name, and tables defining the data above.
 */
struct QuestModule
{
public:
    /*
     * Replaces the entire table of item definitions in this module with the
     * new table idt. 
     */
    void setItemDefTable(ItemDefinitionTable &idt) { itemDefTable_ = idt; }

    /*
     * Retrives the list of item definitions in this module.
     */
    ItemDefinitionTable &itemDefTable() { return itemDefTable_; }
    
    /*
     * Replaces the entire table of sprite definitions in this module with
     * the new table sdt.
     */
    void setSpriteDefTable(SpriteDefinitionTable &sdt) { spriteDefTable_ = sdt; }

    /*
     * Retrieves the list of sprite definitions in this module.
     */
    SpriteDefinitionTable &spriteDefTable() { return spriteDefTable_; }

    /*
     * Replaces the entire table of enemy definitions in this module with
     * the new table edt.
     */
    void setEnemyDefTable(EnemyDefinitionTable &edt) { enemyDefTable_ = edt; }

    /* 
     * Retrieves the list of enemy definitions in this module.
     */
    EnemyDefinitionTable &enemyDefTable() { return enemyDefTable_; }

private:
    ItemDefinitionTable itemDefTable_;
    SpriteDefinitionTable spriteDefTable_;
    EnemyDefinitionTable enemyDefTable_;
};

/*
 * The Quest class contains all persistent data associated to a quest (i.e. all
 * data stored in the .qst file). Each quest contains one or more modules
 * containing item/enemy/sprite definitions, etc, and some single-copy data
 * like the quest rules. Generally speaking, if it makes sense for users to
 * import/export portions of a quest data structure independent of other
 * portions, that data structure should go in the modules; if it only makes
 * sense to have one copy of the structure in a quest, it should not.
 */
class Quest
{
public:
    /* 
     * Retrieves the module in the quest with the given module name. The
     * name must be valid.
     */
    QuestModule &getModule(const std::string &name);

    /*
     * Removes a module from a quest and deletes it (and all of its contents)
     * completely. The module name must be valid.
     */
    void deleteModule(const std::string &moduleName);

    /*
     * Retrieves the list of all modules contained in the quest (by name).
     */
    void getModules(std::vector<std::string> &moduleNames);

    /*
     * Convenience methods to retrieve data from the quest modules. For
     * instance
     * getItemDefinition(ref)
     * is equivalent to
     * getModule(ref.module).getItemDefinition(ref.slot)
     * and the reference must be valid!
     */
    itemdata &getItemDefinition(const ItemDefinitionRef &ref);    
    wpndata &getSpriteDefinition(const SpriteDefinitionRef &ref);
    guydata &getEnemyDefinition(const EnemyDefinitionRef &ref);

    /*
     * Returns whether a given reference is valid (refers to a module that is
     * contained in this quest, and to a slot in that module that exists).
     */
    bool isValid(const ItemDefinitionRef &ref);
    bool isValid(const SpriteDefinitionRef &ref);
    bool isValid(const EnemyDefinitionRef &ref);

    /*
     * Returns the table of special sprite references (those used in hard-coded
     * places during ZC gameplay).
     */
    SpecialSpriteIndex &specialSprites() { return specialSpriteIndex_; }

    /*
     * Replaces the table of special sprite references with ssi.
     */
    void setSpecialSpriteIndex(SpecialSpriteIndex &ssi) { specialSpriteIndex_ = ssi; }

    /*
     * Returns the table of special item references (those used in hard-coded
     * places during ZC gameplay).
     */
    SpecialItemIndex &specialItems() { return specialItemIndex_; }

    /*
     * Replaces the table of special item references with sii.
     */
    void setSpecialItemIndex(SpecialItemIndex &sii) { specialItemIndex_ = sii; }

    /*
     * Returns the table of special enemy references (those used in hard-coded
     * places during ZC gameplay).
     */
    SpecialEnemyIndex &specialEnemies() { return specialEnemyIndex_; }

    /*
     * Replaces the table of special enemy references with sei.
     */
    void setSpecialEnemyIndex(SpecialEnemyIndex &sei) { specialEnemyIndex_ = sei; }

    /* 
    * Retrieves the canonical item of a given item family: the item with 
    * lowest non-0 level. Returns the wooden sword for swords, blue candle
    * for candles, etc.
    * Returns an invalid reference if no item of the given family exists.
    */
    ItemDefinitionRef getCanonicalItemID(int family);

    /* 
    * Searches the item tables for an item with the given family and fam_type
    * (level) or power. Returns the first item in the tables found satisfying
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
    * threshold. Used when upgrading items removes all lower-level items
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
    SpecialEnemyIndex specialEnemyIndex_;
};

#endif