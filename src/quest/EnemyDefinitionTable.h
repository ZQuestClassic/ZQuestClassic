#ifndef ENEMYDEFTABLE_H
#define ENEMYDEFTABLE_H

#include "../zdefs.h"

class Quest;

struct guydata
{
    guydata()
    {
        clear();
    }

    //only the fields needed to build the default enemy table
    guydata(const std::string &name_,
        dword flags_,
        dword flags2_,
        word tile_,
        byte width_,
        byte height_,
        word s_tile_,
        byte s_width_,
        byte s_height_,
        word etile_,
        byte e_width_,
        byte e_height_,
        short hp_,
        short family_,
        short cset_,
        short anim_,
        short e_anim_,
        short frate_,
        short e_frate_,
        short dp_,
        short wdp_,
        short weapon_,
        short rate_,
        short hrate_,
        short step_,
        short homing_,
        short grumble_,
        short item_set_,
        long misc1_,
        long misc2_,
        long misc3_,
        long misc4_,
        long m5_,
        long m6_,
        long m7_,
        long m8_,
        long m9_,
        long m10_,
        long m11_,
        long m12_,
        long m13_,
        long m14_,
        long m15_,
        short bgsfx_,
        short bosspal_,
        short extend_,
        byte defense1_,
        byte defense2_,
        byte defense3_,
        byte defense4_,
        byte defense5_,
        byte defense6_,
        byte defense7_,
        byte defense8_,
        byte defense9_,
        byte defense10_,
        byte defense11_,
        byte defense12_,
        byte defense13_,
        byte defense14_,
        byte defense15_,
        byte defense16_,
        byte defense17_,
        byte defense18_,
        byte defense19_,
        byte hitsfx_,
        byte deadsfx_)
    {
        clear();

        name = name_;
        flags = flags_;
        flags2 = flags2_;
        tile = tile_;
        width = width_;
        height = height_;
        s_tile = s_tile_;
        s_width = s_width_;
        s_height = s_height_;
        e_tile = etile_;
        e_width = e_width_;
        e_height = e_height_;
        hp = hp_;
        family = family_;
        cset = cset_;
        anim = anim_;
        e_anim = e_anim_;
        frate = frate_;
        e_frate = e_frate_;
        dp = dp_;
        wdp = wdp_;
        weapon = weapon_;
        rate = rate_;
        hrate = hrate_;
        step = step_;
        homing = homing_;
        grumble = grumble_;
        item_set = item_set_;
        miscs[0] = misc1_;
        miscs[1] = misc2_;
        miscs[2] = misc3_;
        miscs[3] = misc4_;
        miscs[4] = m5_;
        miscs[5] = m6_;
        miscs[6] = m7_;
        miscs[7] = m8_;
        miscs[8] = m9_;
        miscs[9] = m10_;
        miscs[10] = m11_;
        miscs[11] = m12_;
        miscs[12] = m13_;
        miscs[13] = m14_;
        miscs[14] = m15_;
        bgsfx = bgsfx_;
        bosspal = bosspal_;
        extend = extend_;
        defense[0] = defense1_;
        defense[1] = defense2_;
        defense[2] = defense3_;
        defense[3] = defense4_;
        defense[4] = defense5_;
        defense[5] = defense6_;
        defense[6] = defense7_;
        defense[7] = defense8_;
        defense[8] = defense9_;
        defense[9] = defense10_;
        defense[10] = defense11_;
        defense[11] = defense12_;
        defense[12] = defense13_;
        defense[13] = defense14_;
        defense[14] = defense15_;
        defense[15] = defense16_;
        defense[16] = defense17_;
        defense[17] = defense18_;
        defense[18] = defense19_;
        hitsfx = hitsfx_;
        deadsfx = deadsfx_;
    }


    void clear()
    {
        flags = 0;
        flags2 = 0;
        tile = 0;
        width = 0;
        height = 0; //0=striped, 1+=rectangular
        s_tile = 0; //secondary (additional) tile(s)
        s_width = 0;
        s_height = 0;  //0=striped, 1+=rectangular
        e_tile = 0;
        e_width = 0;
        e_height = 0;

        hp = 0;

        family = 0;
        cset = 0;
        anim = 0;
        e_anim = 0;
        frate = 0;
        e_frate = 0;
        dp = 0;
        wdp = 0;
        weapon = 0;

        rate = 0;
        hrate = 0;
        step = 0;
        homing = 0;
        grumble = 0;
        item_set = 0;
        for (int i = 0; i < 15; i++)
            miscs[i] = 0;
        bgsfx = 0;
        bosspal = 0;
        extend = 0;
        for (int i = 0; i < edefLAST255; i++)
            defense[i] = 0;
        hitsfx = 0;
        deadsfx = 0;
        
        xofs = 0;
        yofs = 0;
        zofs = 0; 
        hxofs = 0;
        hyofs = 0;
        hxsz = 0;
        hysz = 0;
        hzsz = 0;
        txsz = 0;
        tysz = 0;
        for(int i=0; i<scriptDEFLAST; i++)
            scriptdefense[i] = 0;
        wpnsprite = SpriteDefinitionRef();
        SIZEflags = 0;
    }

    std::string name;
    dword flags;
    dword flags2;
    word  tile;
    byte  width;
    byte  height; //0=striped, 1+=rectangular
    word  s_tile; //secondary (additional) tile(s)
    byte  s_width;
    byte  s_height;  //0=striped, 1+=rectangular
    word  e_tile;
    byte  e_width;
    byte  e_height;

    short hp;

    short  family, cset, anim, e_anim, frate, e_frate;
    short  dp, wdp, weapon;

    short  rate, hrate, step, homing, grumble, item_set;
    long   miscs[15];
    short  bgsfx, bosspal, extend;
    byte defense[edefLAST255];
    // byte scriptdefense[
    //  short  startx, starty;
    //  short  foo1,foo2,foo3,foo4,foo5,foo6;
    byte  hitsfx, deadsfx;
    //Add all new guydata variables after this point, if you do not want to edit defdata to fit.
    //Adding earlier will offset defdata arrays. -Z

    //2.6 enemy editor tile and hit sizes. -Z
    int xofs,yofs,zofs; //saved to the packfile, so I am using int. I can typecast to fix and back in the functions. 
                        // no hzofs - it's always equal to zofs.
    int hxofs,hyofs,hxsz,hysz,hzsz;
    int txsz,tysz;
    byte scriptdefense[scriptDEFLAST]; //old 2.future quest file crossover support. 
    SpriteDefinitionRef wpnsprite; //wpnsprite is new for 2.6 -Z
    int SIZEflags;; //Flags for size panel offsets. The user must enable these to override defaults. 
};

struct SpecialEnemyIndex
{
public:
    SpecialEnemyIndex() {}

    /*
    * Checks that all entries in the index point to valid weapons in the given
    * weapons definition table.
    */
    bool checkConsistency(Quest &quest);

    EnemyDefinitionRef zelda;       // these have special spawning behavior hard-coded
    EnemyDefinitionRef fire;
    EnemyDefinitionRef fairy;
    EnemyDefinitionRef fairyItem;   // the one that flies around, not the one in the room
    EnemyDefinitionRef fireballShooter; // summoned if you atack guys
    EnemyDefinitionRef spinTile1;       // spawned by the spin tile combo
    EnemyDefinitionRef keese;           // spawned in caves
};

/*
 * Stores all enemy definitions in the quest. These are the properties stored
 * in the .qst file (*not* temporary instances of enemies).
 * Each enemy has a guysdata structure associated to it which contains all of 
 * enemy data (including name).
 */
class EnemyDefinitionTable
{
public:
    /*
    * Removes all enemy definitions.
    */
    void clear();

    /*
     * Returns the name of the deafault hard-coded enemy in the given enemy
     * slot. Note that this name doesn't make much sense except for the CORE
     * module in an empty quest. Returns a procedurally-generated name if no
     * descriptive name exists for the given slot.
     */
    static std::string defaultEnemyName(int slot);

    /*
    * Retrives the enemy definition with given index from the table. The
    * given index must be valid.
    */
    guydata &getEnemyDefinition(int idx) { return guyData_[idx]; }

    /*
    * Adds a new enemy at the end of the enemy definition table, with given
    * settings.
    */
    void addEnemyDefinition(const guydata &data);

    /*
    * The number of enemies currently in the enemy definition table.
    */
    uint32_t getNumEnemyDefinitions() const { return (int)guyData_.size(); }

    /*
    * Returns whether the given enemy index is valid (in the table).
    */
    bool isValid(int slot) { return slot >= 0 && slot < (int)guyData_.size(); }

private:

    std::vector<guydata> guyData_;    

};

#endif
