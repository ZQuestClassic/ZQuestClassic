#ifndef WEAPONDEFINITIONTABLE_H
#define WEAPONDEFINITIONTABLE_H

#include "../zdefs.h"

struct wpndata
{
    const static int WF_BEHIND = 0x10;	//Weapon renders behind other sprites

    word tile;
    byte misc;                                                // 000bvhff (vh:flipping, f:flash (1:NES, 2:BSZ))
    byte csets;                                               // ffffcccc (f:flash cset, c:cset)
    byte frames;                                              // animation frame count
    byte speed;                                               // animation speed
    byte type;                                                // used by certain weapons
                                                              //  byte wpn_type;
    word script;
};

/*
* Stores all weapon definitions in the quest. These are the properties stored
* in the .qst file (*not* temporary instances of weapon sprites).
* Each weapon has a name and an itemdata structure associated to it, the latter
* of which contains all of the non-name data.
*/
class WeaponDefinitionTable
{
public:
    /*
    * Removes all item definitions.
    */
    void clear();

    /*
    * Computes the default item name for the weapon with given slot ID in the
    * weapon definition table.
    * These names are based on the order weapons were hard-coded in very old
    * versions of ZC. Weapons with slots that don't correspond to legacy 
    * weapons get a procedurally-generated name.
    */
    static std::string defaultWeaponName(int slot);

    /*
    * Retrives the weapon definition with given index from the table. The
    * given index must be valid.
    */
    wpndata &getWeaponDefinition(int idx) { return weaponData_[idx]; }

    /*
    * Retrives the name of the weapon with given index in the weapon
    * definition table. Crashes if the index is invalid.
    */
    const std::string &getWeaponName(int idx) { return weaponNames_[idx]; }

    /*
    * Adds a new weapon at the end of the item definition table, with given
    * settings and name.
    */
    void addWeaponDefinition(const wpndata &data, const std::string &name);

    /*
    * Overwrites the name of the weapon in the given slot in the definition
    * table with a new name. The index must be valid (crashes otherwise).
    */
    void setWeaponName(int idx, const std::string &name);

    /*
    * The number of weapons currently in the weapon definition table.
    */
    int getNumWeaponDefinitions() const { return (int)weaponData_.size(); }

private:

    std::vector<wpndata> weaponData_;
    std::vector<std::string> weaponNames_;

};


#endif