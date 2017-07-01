#ifndef ITEMDEFINITIONTABLE_H
#define ITEMDEFINITIONTABLE_H

#include "../zdefs.h"
#include "../sfx.h"
#include <string>
#include <vector>

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
        byte wpn_, byte wpn2_, byte wpn3_, byte wpn4_, byte wpn5_, byte wpn6_, byte wpn7_, byte wpn8_, byte wpn9_, byte wpn10_, 
        byte pickup_hearts_, long misc1_, long misc2_, long misc3_, long misc4_, byte magic_, byte usesound_)
    {        
        clear();
        family = family_;
        fam_type = fam_type_;
        power = power_;
        flags = flags_;
        count = count_;
        amount = amount_;
        setmax = setmax_;
        max = max_;
        playsound = playsound_;
        wpn = wpn_;
        wpn2 = wpn2_;
        wpn3 = wpn3_;
        wpn4 = wpn4_;
        wpn5 = wpn5_;
        wpn6 = wpn6_;
        wpn7 = wpn7_;
        wpn8 = wpn8_;
        wpn9 = wpn9_;
        wpn10 = wpn10_;
        pickup_hearts = pickup_hearts_;
        misc1 = misc1_;
        misc2 = misc2_;
        misc3 = misc3_;
        misc4 = misc4_;
        magic = magic_;
        usesound = usesound_;
    }

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
        wpn = 0;
        wpn2 = 0;
        wpn3 = 0;
        wpn4 = 0;
        wpn5 = 0;
        wpn6 = 0;
        wpn7 = 0;
        wpn8 = 0;
        wpn9 = 0;
        wpn10 = 0;
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
    byte wpn;
    byte wpn2;
    byte wpn3;
    byte wpn4;
    byte wpn5;
    byte wpn6;
    byte wpn7;
    byte wpn8;
    byte wpn9;
    byte wpn10;
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
    int getNumItemDefinitions() const { return (int)itemData_.size(); }

    /* 
     * Retrieves the canonical item of a given item family: the item with 
     * lowest non-0 level. Returns the wooden sword for swords, blue candle
     * for candles, etc.
     * Returns -1 if no item of the given family exists.
     */
    int getCanonicalItemID(int family);

    /*
     * Retrieves the family of the item with given index in the definitions
     * table. The index must be valid.
     */
    int getItemFamily(int item);

    /* 
     * Searches the item table for an item with the given family and fam_type
     * (level) or power. Returns the first time in the table found satisfying
     * the search criteria, or -1 if no item exists.
     */
    int getItemID(int family, int fam_type);
    int getItemIDPower(int family, int power);

    /*
     * Adds items to the init data based on old-style bitmasks (levels) 
     * specifying item levels within families.
     */
    void addOldStyleFamily(zinitdata *dest, int family, char levels);

    /*
     * Searches the init data for the item of the given family with highest
     * level (fam_type). Returns -1 if no such item is found.
     */
    int getHighestLevelOfFamily(zinitdata *source, int family);

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
    * Removes all items from the game data whose family matches the given
    * family (does nothing if there weren't any such items in the game
    * data in the first place).
    */
    void removeItemsOfFamily(gamedata *g, int family);
    
    /*
    * Searches the game data for the item of the given family with highest
    * level (fam_type). Returns -1 if no such item is found.
    */
    int getHighestLevelOfFamily(gamedata *source, int family, bool checkenabled = false);

    /*
     * Searches through the items in the game data and removes all items
     * of the given family whose level (fam_type) is lower than the given
     * treshold. Used when upgrading items removes all lower-level items
     * of the same family, such as boomerangs, etc.
     */
    void removeLowerLevelItemsOfFamily(gamedata *g, int family, int level);

private:

    std::vector<itemdata> itemData_;
    std::vector<std::string> itemNames_;
};

#endif