//--------------------------------------------------------
//  ZQuest Classic
//  by Jeremy Craner, 1999-2000
//
//  defdata.cc
//
//  Default item, weapon, and enemy data for ZQuest Classic.
//
//  This data was originally hard coded in the item class,
//  enemy classes, etc. I exctracted it to make generic
//  classes, thus allowing custom items and enemies.
//
//--------------------------------------------------------

#ifndef _DEFDATA_H_
#define _DEFDATA_H_

#include "base/zdefs.h"
#include "items.h"

extern comboclass default_combo_classes[cMAX+1];
extern itemdata default_items[iLast];
extern guydata default_guys[OLDMAXGUYS];
extern item_drop_object default_item_drop_sets[isMAX];
#endif
 
