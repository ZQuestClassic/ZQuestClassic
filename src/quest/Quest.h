#ifndef QUEST_H
#define QUEST_H

#include "../zdefs.h"
#include "ItemDefinitionTable.h"

// Placeholder struct for now

struct Quest
{
public:
    ItemDefinitionTable &itemDefTable() { return itemDefTable_; }
    void setItemDefTable(ItemDefinitionTable &idt) { itemDefTable_ = idt; }

private:
    ItemDefinitionTable itemDefTable_;
};

#endif