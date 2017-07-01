#ifndef QUEST_H
#define QUEST_H

#include "../zdefs.h"
#include "ItemDefinitionTable.h"
#include "WeaponDefinitionTable.h"

// Placeholder struct for now

struct Quest
{
public:
    ItemDefinitionTable &itemDefTable() { return itemDefTable_; }
    WeaponDefinitionTable &weaponDefTable() { return weaponDefTable_; }
    void setItemDefTable(ItemDefinitionTable &idt) { itemDefTable_ = idt; }
    void setWeaponDefTable(WeaponDefinitionTable &wdt) { weaponDefTable_ = wdt; }

private:
    ItemDefinitionTable itemDefTable_;
    WeaponDefinitionTable weaponDefTable_;
};

#endif