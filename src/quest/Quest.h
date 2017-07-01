#ifndef QUEST_H
#define QUEST_H

#include "../zdefs.h"
#include "ItemDefinitionTable.h"
#include "WeaponDefinitionTable.h"

// Placeholder struct for now

struct Quest
{
public:
    void setItemDefTable(ItemDefinitionTable &idt) { itemDefTable_ = idt; }
    ItemDefinitionTable &itemDefTable() { return itemDefTable_; }
    
    void setWeaponDefTable(SpriteDefinitionTable &wdt) { weaponDefTable_ = wdt; }
    SpriteDefinitionTable &weaponDefTable() { return weaponDefTable_; }

    SpecialSpriteIndex &specialSprites() { return specialSpriteIndex_; }
    void setSpecialSpriteIndex(SpecialSpriteIndex &ssi) { specialSpriteIndex_ = ssi; }
    
private:
    ItemDefinitionTable itemDefTable_;

    SpriteDefinitionTable weaponDefTable_;
    SpecialSpriteIndex specialSpriteIndex_;
};

#endif