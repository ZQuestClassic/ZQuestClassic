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

enum OldSpriteID
{
    OS_SWORD, OS_WSWORD, OS_MSWORD, OS_XSWORD, OS_BRANG,
    OS_MBRANG, OS_FBRANG, OS_BOMB, OS_SBOMB, OS_BOOM,
    // 10
    OS_ARROW, OS_SARROW, OS_FIRE, OS_WIND, OS_BAIT,
    OS_WAND, OS_MAGIC, OS_ENEMY_FIREBALL, OS_ENEMY_ROCK, OS_ENEMY_ARROW,
    // 20
    OS_ENEMY_SWORD, OS_ENEMY_MAGIC, OS_SPAWN, OS_DEATH, OS_SWIM,
    OS_HAMMER, OS_HSHEAD, OS_HSCHAIN_H, OS_HSHANDLE, OS_SSPARKLE,
    // 30
    OS_GSPARKLE, OS_MSPARKLE, OS_FSPARKLE, OS_HAMMERSMACK, OS_GARROW,
    OS_ENEMY_FLAME, OS_ENEMY_WIND, OS_MMETER, OS_DINSFIRE1A, OS_DINSFIRE1B,
    // 40
    OS_DINSFIRES1A, OS_DINSFIRES1B, OS_HSCHAIN_V, OS_MORE, OS_BOSSMARKER,
    OS_LINKSLASH, OS_SWORDSLASH, OS_WSWORDSLASH, OS_MSWORDSLASH, OS_XSWORDSLASH,
    // 50
    OS_SHADOW, OS_LARGESHADOW, OS_BUSHLEAVES, OS_FLOWERCLIPPINGS, OS_GRASSCLIPPINGS,
    OS_TALLGRASS, OS_RIPPLES, OS_NPCS, OS_NAYRUSLOVE1A, OS_NAYRUSLOVE1B,
    // 60
    OS_NAYRUSLOVES1A, OS_NAYRUSLOVES1B, OS_NAYRUSLOVE2A, OS_NAYRUSLOVE2B, OS_NAYRUSLOVES2A,
    OS_NAYRUSLOVES2B, OS_NAYRUSLOVESHIELDFRONT, OS_NAYRUSLOVESHIELDBACK, OS_SUBSCREENVINE, OS_CBYRNA,
    // 70
    OS_CBYRNASLASH, OS_LSHEAD, OS_LSCHAIN_H, OS_LSHANDLE, OS_LSCHAIN_V,
    OS_SBOOM, OS_ENEMY_BOMB, OS_ENEMY_SBOMB, OS_ENEMY_BOOM, OS_ENEMY_SBOOM,
    // 80
    OS_ENEMY_FIRETRAIL, OS_ENEMY_FLAME2, OS_ENEMY_FLAME2TRAIL, OS_ENEMY_ICE, OS_HOVER,
    OS_FIREMAGIC, OS_QUARTERHEARTS, OS_CBYRNAORB, //iwSideLadder, iwSideRaft,

    OS_LAST
};
extern const char *old_weapon_names[];

extern guydata default_guys[OLDMAXGUYS];

const std::vector<item_drop_object> &getDefaultItemDropSets();
#endif
 
