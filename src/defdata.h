//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  defdata.cc
//
//  Default item, weapon, and enemy data for Zelda Classic.
//
//  This data was originally hard coded in the item class,
//  enemy classes, etc. I exctracted it to make generic
//  classes, thus allowing custom items and enemies.
//
//--------------------------------------------------------

#ifndef _DEFDATA_H_
#define _DEFDATA_H_

#include "quest/ItemDefinitionTable.h"
#include "guys.h"
extern comboclass default_combo_classes[cMAX];

const char *old_item_names[];
const std::vector<itemdata> &getDefaultItems();

extern const char *old_weapon_names[];

extern guydata default_guys[OLDMAXGUYS];
extern item_drop_object default_item_drop_sets[isMAX];
#endif
 
