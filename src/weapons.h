//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  sprite.cc
//
//  Sprite classes:
//   - sprite:      base class for the guys and enemies in zelda.cc
//   - movingblock: the moving block class
//   - sprite_list: main container class for different groups of sprites
//   - item:        items class
//
//--------------------------------------------------------

#ifndef _WEAPONS_H_
#define _WEAPONS_H_

#include "zdefs.h"
#include "sprite.h"
#include "zfix.h"

/**************************************/
/***********  Weapon Class  ***********/
/**************************************/

extern byte boomframe[16];
extern byte bszboomflip[4];

#define WEAPON_CLOCKS 10

class weapon : public sprite
{
private:
    void seekLink();
    void seekEnemy(int j);
    int seekEnemy2(int j);

// How close the weapon can get to the edge of the screen before
	// being deleted (or bouncing, in the case of boomerangs)
	int minX, maxX, minY, maxY;
	friend void setScreenLimits(weapon&);
    
public:
    int power,type,dead,clk2,misc2,ignorecombo;
    bool isLit; //if true, this weapon is providing light to the current screen
    int parentid, //Enemy who created it
        parentitem; //Item which created it
    int dragging;
    zfix step;
    bool bounce, ignoreLink;
    word flash,wid,aframe,csclk;
    int o_tile, o_cset, o_speed, o_type, frames, o_flip;
	byte script_wrote_otile;
    int temp1;
    bool behind;
	byte linkedItem;
	byte wscreengrid[22];
	byte wscreengrid_layer[2][22];

	//!Dimentio: More variables? That suuuuuure won't break anything. Nope.
    int count1; 
    int count2;
    int count3;
    int count4;
    int count5;
    void bookfirecreate();

    //Weapon Editor -Z
    byte useweapon; //lweapon id type -Z
    byte usedefence; //default defence type -Z
    word linked_parent;
    byte quantity_iterator;
    int weap_pattern[ITEM_MOVEMENT_PATTERNS]; //formation, arg1, arg2 -Z
    int weaprange; //default range -Z
    int weapduration; //default duration, 0 = infinite. 
    long clocks[WEAPON_CLOCKS];
    long tilemod;
    byte drawlayer;
    byte family_class;												
    byte family_level;
    word flags;
    long collectflags; //items that this weapon can collect on contact.
    long ffmisc[FFSCRIPT_MISC];
    char weapname[128]; //Weapon Name, pulled from editor. 
     int duplicates; //Number of duplicate weapons generated.
     int wpn_misc_d[FFSCRIPT_MISC];
    //! End weapon editor. -Z
    
    int script_UID;
    int parent_script_UID;
    int getScriptUID();
    int getParentScriptUID();
    void setScriptUID(int new_id);
    void setParentScriptUID(int new_id);
    bool isLinkWeapon();
    bool isLinkMelee();
    //2.6 ZScript -Z
    int scriptrange,blastsfx;
    
    //2.6 enemy editor weapon sprite
    int wpnsprite;
    int magiccosttimer; ///Magic cost for byrna beam, boots. -Z
    
    //Used only by ffscript! No not make readable by scripts!
    byte ScriptGenerated; //Used to permit creating LinkClass weapons, or other weapon types that the engine does not control.
    byte isLWeapon;
	bool weapon_dying_frame; //a last_hurrah for weapons -V
    byte specialinfo;
    void convertType(bool toLW);
    weapon(weapon const &other);
    //weapon(zfix X,zfix Y,zfix Z,int Id,int Type,int pow,int Dir, int Parentid, int prntid, bool isDummy=false);
    weapon(zfix X,zfix Y,zfix Z,int Id,int Type,int pow,int Dir, int Parentid, int prntid, bool isDummy=false, byte script_gen=0, byte isLW=0, byte special = 0);
    weapon(zfix X,zfix Y,zfix Z,int Id,int usesprite, int Dir, int step, int prntid, int height, int width, int a, int b, int c, int d, int e, int f, int g);
    virtual ~weapon();
	void cleanup_sfx();
    void LOADGFX(int wpn);
    void findcombotriggers();
    bool Dead();
    bool isScriptGenerated();
    bool clip();
    bool blocked();
    void runscript(int index);
    virtual bool blocked(int xOffset, int yOffset);
    virtual bool animate(int index);
    virtual bool animateandrunscript(int index);
    virtual void onhit(bool clipped);
    virtual void onhit(bool clipped, int special, int linkdir);
    // override hit detection to check for invicibility, etc
    virtual bool hit(sprite *s);
    virtual bool hit(int tx,int ty,int tz,int txsz,int tysz,int tzsz);
    virtual void draw(BITMAP *dest);
    virtual void update_weapon_frame(int change, int orig);
	virtual int run_script(int mode);
};

void putweapon(BITMAP *dest,int x,int y,int weapon_id, int type, int dir, int &aclk, int &aframe,
               int parentid);
#endif
/*** end of sprite.cc ***/

