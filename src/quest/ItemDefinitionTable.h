#ifndef ITEMDEFINITIONTABLE_H
#define ITEMDEFINITIONTABLE_H

#include "../zdefs.h"
#include "../sfx.h"
#include "QuestRefs.h"
#include <string>
#include <vector>

class Quest;

struct SpecialItemIndex
{
public:
    SpecialItemIndex() {}

    /*
    * Checks that all entries in the index point to valid weapons in the given
    * weapons definition table.
    */
    bool checkConsistency(Quest &quest);

    ItemDefinitionRef dustPile;
    ItemDefinitionRef clock;    // items with special handling of the link tile modifier
    ItemDefinitionRef key;
    ItemDefinitionRef levelKey;
    ItemDefinitionRef bossKey;
    ItemDefinitionRef triforce;
    ItemDefinitionRef magicContainer;
    ItemDefinitionRef compass;
    ItemDefinitionRef map;
    ItemDefinitionRef redPotion;        // used in "take X or Y" rooms
    ItemDefinitionRef heartContainer;   // used in "take X or Y" rooms
    ItemDefinitionRef selectA;      // A button selector on subscreen
    ItemDefinitionRef selectB;      // B button selector on subscreen
    ItemDefinitionRef rupy;         // used for money special rooms and slash combos
    ItemDefinitionRef heart;        // used for slash combos
    ItemDefinitionRef heartContainerPiece;  // used in subscreen
    ItemDefinitionRef bomb;         // given to the player if bombs are positive in init data
    ItemDefinitionRef superBomb;    // given to the player if super bombs are positive in init data
    ItemDefinitionRef bigTriforce;  // used in ending animation
    ItemDefinitionRef bait;         // used in grumble room
    ItemDefinitionRef fairyMoving;  // special handling when picking up
};

struct itemdata
{
    const static int IF_GAMEDATA = 0x0001;  // Whether this item sets the corresponding gamedata value or not
    const static int IF_EDIBLE = 0x0002;  // can be eaten by Like Like
    const static int IF_COMBINE = 0x0004;  // blue potion + blue potion = red potion
    const static int IF_DOWNGRADE = 0x0008;
    const static int IF_FLAG1 = 0x0010;
    const static int IF_FLAG2 = 0x0020;
    const static int IF_KEEPOLD = 0x0040;
    const static int IF_RUPEE_MAGIC = 0x0080;
    const static int IF_UNUSED = 0x0100;
    const static int IF_GAINOLD = 0x0200;
    const static int IF_FLAG3 = 0x0400;
    const static int IF_FLAG4 = 0x0800;
    const static int IF_FLAG5 = 0x1000;

    itemdata() 
    {
        clear();
    }

    // just enough to initialize the default items
    itemdata(byte family_, byte fam_type_, byte power_, word flags_, char count_, word amount_, short setmax_, word max_, byte playsound_,
        uint32_t wpn_, uint32_t wpn2_, uint32_t wpn3_, uint32_t wpn4_, uint32_t wpn5_, uint32_t wpn6_, uint32_t wpn7_, uint32_t wpn8_, uint32_t wpn9_, uint32_t wpn10_,
        byte pickup_hearts_, long misc1_, long misc2_, long misc3_, long misc4_, byte magic_, byte usesound_);

    void clear()
    {
        tile = 0;
        misc = 0;
        csets = 0;
        frames = 0;
        speed = 0;
        delay = 0;
        ltm = 0;
        family = 0xFF;
        fam_type = 0;
        power = 0;
        flags = 0;
        script = 0;
        count = -1;
        amount = 0;
        setmax = 0;
        max = 0;
        playsound = WAV_SCALE;
        collect_script = 0;
        for (int i = 0; i < INITIAL_D; i++)
            initiald[i] = 0;
        for (int i = 0; i < INITIAL_A; i++)
            initiala[i] = 0;
        for (int i = 0; i < 9; i++)
            wpns[i] = SpriteDefinitionRef();
        pickup_hearts = 0;
        misc1 = 0;
        misc2 = 0;
        misc3 = 0;
        misc4 = 0;
        misc5 = 0;
        misc6 = 0;
        misc7 = 0;
        misc8 = 0;
        misc9 = 0;
        misc10 = 0;
        magic = 0;
        usesound = 0;
        useweapon = 0;
        usedefence = 0;
        for (int i = 0; i < ITEM_MOVEMENT_PATTERNS; i++)
            weap_pattern[i] = 0;
        weaprange = 0;
        weapduration = 0;
        duplicates = 0;
        for (int i = 0; i < FFSCRIPT_MISC; i++)
            wpn_misc_d[i] = 0;

        for (int i = 0; i < INITIAL_D; i++)
            weap_initiald[i] = 0;
        for (int i = 0; i < INITIAL_A; i++)
            weap_initiala[i] = 0;

        drawlayer=0;
        collectflags = 0;
        hxofs = 0;
        yxofs = 0;
        hxsz = 0;
        hysz = 0;
        hzsz = 0;
        xofs = 0;
        yofs = 0;
        weap_hxofs = 0;
        weap_yxofs = 0;
        weap_hxsz = 0;
        weap_hysz = 0;
        weap_hzsz = 0;
        weap_xofs = 0;
        weap_yofs = 0;

        weaponscript = 0;
        wpnsprite = 0;
    }

    word tile;
    byte misc;                                                // 0000vhtf (vh:flipping, t:two hands, f:flash)
    byte csets;                                               // ffffcccc (f:flash cset, c:cset)
    byte frames;                                              // animation frame count
    byte speed;                                               // animation speed
    byte delay;                                               // extra delay factor (-1) for first frame
    long ltm;                                                 // Link Tile Modifier
    byte family;												// What family the item is in
    byte fam_type;	//level										// What type in this family the item is
    byte power;	// Damage, height, etc.
    word flags;
    word script;												// Which script the item is using
    char count;
    word amount;
    short setmax;
    word max;
    byte playsound;
    word collect_script;
    //  byte exp[10];                                             // not used
    long initiald[INITIAL_D];
    byte initiala[INITIAL_A];
    SpriteDefinitionRef wpns[10];
    byte pickup_hearts;
    long misc1;
    long misc2;
    long misc3;
    long misc4;
    long misc5;
    long misc6;
    long misc7;
    long misc8;
    long misc9;
    long misc10;
    byte magic; // Magic usage!
    byte usesound;
    byte useweapon; //lweapon id type -Z
    byte usedefence; //default defence type -Z
    int weap_pattern[ITEM_MOVEMENT_PATTERNS]; //formation, arg1, arg2 -Z
    int weaprange; //default range -Z
    int weapduration; //default duration, 0 = infinite. 


                      //To implement next;
    int duplicates; //Number of duplicate weapons generated.
    int wpn_misc_d[FFSCRIPT_MISC]; //THe initial Misc[d] that will be assiged to the weapon, 

    long weap_initiald[INITIAL_D];
    byte weap_initiala[INITIAL_A];

    byte drawlayer;
    long collectflags;
    int hxofs, yxofs, hxsz, hysz, hzsz, xofs, yofs; //item
    int weap_hxofs, weap_yxofs, weap_hxsz, weap_hysz, weap_hzsz, weap_xofs, weap_yofs; //weapon

    word weaponscript; //If only. -Z This would link an item to a weapon script in the item editor.
    int wpnsprite; //enemy weapon sprite. 
};

/*
 * Stores all item definitions in the quest. These are the properties specified
 * in the item editor and stored in the .qst file (*not* temporary instances of 
 * item sprites.
 * Each item has a name and an itemdata structure associated to it, the latter
 * of which contains all of the non-name data.
 */
class ItemDefinitionTable
{
public:
    /*
     * Removes all item definitions.
     */
    void clear();

    /*
     * Computes the default item name for the item with given slot ID in the
     * item definition table.
     * These names are based on the order items were hard-coded in very old
     * versions of ZC. Items with slots that don't correspond to legacy items
     * get a procedurally-generated name.
     */
    static std::string defaultItemName(int slot);

    /*
     * Returns the default item definition for the item with given slot ID,
     * based on the hard-coded items in very old versions of ZC. Items
     * with slots that don't correspond to legacy item get blank settings.
     */
    static itemdata defaultItemData(int slot);

    /*
     * Adds a new item at the end of the item definition table, with given
     * settings and name.
     */
    void addItemDefinition(const itemdata &data, const std::string &name);

    /*
     * Retrives the item definition with given index from the table. Given
     * how often this method is called all over the place in the ZC and
     * ZQ code, with no checking of whether or not the item index is valid,
     * this is bound to be a source of many bugs until all instances of
     * unsafe access are flushed out...
     */
    itemdata &getItemDefinition(int idx) { return itemData_[idx]; }
    const itemdata &getItemDefinition(int idx) const { return itemData_[idx]; }

    /*
     * Retrives the name of the item with given index in the item
     * definition table. Crashes if the index is invalid.
     */
    const std::string &getItemName(int idx) { return itemNames_[idx]; }

    /*
     * Overwrites the name of hte item in the given slot in the definition
     * table with a new name. The index must be valid (crashes otherwise).
     */
    void setItemName(int idx, const std::string &name);

    /*
     * The number of items currently in the items definition table.
     */
    uint32_t getNumItemDefinitions() const { return itemData_.size(); }

    /*
    * Returns whether the given item index is valid (in the table).
    */
    bool isValid(int slot) { return slot >= 0 && slot < (int)itemData_.size(); }
private:

    std::vector<itemdata> itemData_;
    std::vector<std::string> itemNames_;
};

#endif