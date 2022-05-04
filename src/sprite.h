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

#ifndef _SPRITE_H_
#define _SPRITE_H_

#include "zc_alleg.h"
#include "zdefs.h"
#include <set>
#include <map>
#include "zfix.h"

using std::map;
// this code needs some patching for use in zquest.cc

extern itemdata *itemsbuf;
extern wpndata  *wpnsbuf;
extern bool     freeze_guys;
extern int32_t fadeclk;
extern int32_t frame;
extern bool BSZ;
extern int32_t conveyclk;

struct scriptmem
{
    int32_t stack[MAX_SCRIPT_REGISTERS];
    refInfo scriptData;
};

/**********************************/
/******* Sprite Base Class ********/
/**********************************/

//Sprite Offscreen Bits
#define SPRITE_MOVESOFFSCREEN 0x01

// Forward reference
class refInfo;

class sprite
{
private:
    static int32_t getNextUID();
    //unique sprite ID
    //given upon construction
    int32_t uid;
    
public:
    int32_t getUID()
    {
        return uid;
    }
    
   
    
    zfix x,y,z,fall,fakefall,fakez;
    int32_t tile,shadowtile,cs,flip,c_clk,clk,misc;
    zfix xofs,yofs,zofs;
    // no hzofs - it's always equal to zofs.
    int32_t hxofs,hyofs,hxsz,hysz,hzsz;
    int32_t txsz,tysz;
    /*
     * Explanation of hzsz:
     * Hzsz is how "tall" a sprite is.
     * To jump over a sprite, you must be higher than its z+hzsz.
     * Enemies have hzsz of 0 so that Hero can jump over them (see LA.)
     */
    int32_t id,dir;
    bool angular,canfreeze;
    double angle;
    int32_t lasthit, lasthitclk;
    int32_t dummy_int[10];
    zfix dummy_fix[10];
    float dummy_float[10];
    bool dummy_bool[10];
    int32_t drawstyle;                                          //0=normal, 1=translucent, 2=cloaked
    int32_t extend;
    // Scripting additions
    int32_t miscellaneous[32];
    byte scriptcoldet;
    int32_t wpnsprite; //wpnsprite is new for 2.6 -Z
	scriptmem* scrmem;
    byte initialised;
    dword scriptflag;
    word doscript;
    byte waitdraw;
    word script;
    word weaponscript;
    int32_t initD[8];
    int32_t initA[2];
    int32_t weap_initd[8];
    int32_t weap_inita[2];
    int32_t scripttile;
    signed char scriptflip;
    byte do_animation;
    int32_t rotation;
    int32_t scale; 
    int32_t moveflags;
    byte drawflags;
	byte knockbackflags;
	byte screenedge;
	int32_t scriptshadowtile;
#define FLAG_NOSLIDE 0x01
#define FLAG_NOSCRIPTKNOCKBACK 0x02
	byte knockbackSpeed;
	int32_t script_knockback_clk;
	int32_t script_knockback_speed;
	int32_t pit_pulldir; // Pitfall pull direction
	int32_t pit_pullclk; // Pitfall pull clk
	int32_t fallclk; // Pitfall fall clk
	int32_t fallCombo; // Pitfall fallen combo
	int32_t old_cset; // Storage var for an old cset; used by pitfalls
	int32_t drownclk; // Pitfall fall clk
	int32_t drownCombo; // Pitfall fallen combo
	bool isspawning;
	bool can_flicker;
	
	byte spr_shadow, spr_death, spr_spawn;
	int16_t spr_death_anim_clk, spr_spawn_anim_clk;
	byte spr_death_anim_frm, spr_spawn_anim_frm;
	
	byte glowRad, glowShape;
	bool switch_hooked;
    
    sprite();
    sprite(sprite const & other);
    sprite(zfix X,zfix Y,int32_t T,int32_t CS,int32_t F,int32_t Clk,int32_t Yofs);
    virtual ~sprite();
	virtual void handle_sprlighting();
    virtual void draw(BITMAP* dest);                        // main layer
    virtual void drawzcboss(BITMAP* dest);                        // main layer
    virtual void draw8(BITMAP* dest);                       // main layer
    virtual void drawcloaked(BITMAP* dest);                 // main layer
    virtual void drawshadow(BITMAP* dest, bool translucent);// main layer
    virtual void draw2(BITMAP* dest);                       // top layer for special needs
    virtual void drawcloaked2(BITMAP* dest);                // top layer for special needs
    virtual bool animate(int32_t index);
    virtual void check_conveyor();
	virtual int32_t get_pit(); //Returns combo ID of pit that sprite WOULD fall into; no side-effects
	virtual int32_t check_pits(); //Returns combo ID of pit fallen into; 0 for not fallen.
	virtual int32_t get_water(); //Returns combo ID of pit that sprite WOULD fall into; no side-effects
	virtual int32_t check_water(); //Returns combo ID of pit fallen into; 0 for not fallen.
    int32_t real_x(zfix fx);
    int32_t real_y(zfix fy);
    int32_t real_ground_y(zfix fy);
    int32_t real_z(zfix fz);
    int32_t fake_z(zfix fz);
    virtual bool hit(sprite *s);
    virtual bool hit(int32_t tx,int32_t ty,int32_t tz,int32_t txsz,int32_t tysz,int32_t tzsz);
    
    
    virtual int32_t hitdir(int32_t tx,int32_t ty,int32_t txsz,int32_t tysz,int32_t dir);
    virtual void move(zfix dx,zfix dy);
    virtual void move(zfix s);
	virtual bool knockback(int32_t time, int32_t dir, int32_t speed);
	virtual bool runKnockback();
    void explode(int32_t mode);
	bool getCanFlicker();
	void setCanFlicker(bool v);
	
	void alloc_scriptmem();
	
	virtual int32_t run_script(int32_t mode);
};

enum //run_script modes
{
	MODE_NORMAL,
	MODE_WAITDRAW
};

/***************************************************************************/

/**********************************/
/********** Sprite List ***********/
/**********************************/

#define SLMAX 255*(511*4)+1

class sprite_list
{
    sprite *sprites[SLMAX];
    int32_t count;
	int32_t active_iterator;
	int32_t max_sprites;
    map<int32_t, int32_t> containedUIDs;
    // Cache requests from scripts
    mutable int32_t lastUIDRequested;
    mutable sprite* lastSpriteRequested;
    
public:
    sprite_list();
    
    sprite *getByUID(int32_t uid);
    void clear();
    sprite *spr(int32_t index);
    bool swap(int32_t a,int32_t b);
    bool add(sprite *s);
    // removes pointer from list but doesn't delete it
    bool remove(sprite *s);
    zfix getX(int32_t j);
    zfix getY(int32_t j);
    int32_t getID(int32_t j);
    int32_t getMisc(int32_t j);
	int32_t getMax() {return max_sprites;}
	void setMax(int32_t max) {max_sprites = (max < SLMAX ? max : SLMAX);}
    bool del(int32_t j);
    void draw(BITMAP* dest,bool lowfirst);
    void drawshadow(BITMAP* dest,bool translucent, bool lowfirst);
    void draw2(BITMAP* dest,bool lowfirst);
    void drawcloaked2(BITMAP* dest,bool lowfirst);
    void animate();
    void check_conveyor();
	void run_script(int32_t mode);
    int32_t Count();
	bool has_space(int32_t space = 1);
    int32_t hit(sprite *s);
    int32_t hit(int32_t x,int32_t y,int32_t z,int32_t xsize, int32_t ysize, int32_t zsize);
    // returns the number of sprites with matching id
    int32_t idCount(int32_t id, int32_t mask);
    // returns index of first sprite with matching id, -1 if none found
    int32_t idFirst(int32_t id, int32_t mask);
    // returns index of nth sprite with matching id, -1 if none found
    int32_t idNth(int32_t id, int32_t n, int32_t mask);
    // returns index of last sprite with matching id, -1 if none found
    int32_t idLast(int32_t id, int32_t mask);
    // returns the number of sprites with matching id
    int32_t idCount(int32_t id);
    // returns index of first sprite with matching id, -1 if none found
    int32_t idFirst(int32_t id);
    // returns index of nth sprite with matching id, -1 if none found
    int32_t idNth(int32_t id, int32_t n);
    // returns index of last sprite with matching id, -1 if none found
    int32_t idLast(int32_t id);
    
private:

    void checkConsistency(); //for debugging
};

/**********************************/
/********** Moving Block **********/
/**********************************/

class movingblock : public sprite
{
public:
    int32_t bcombo;
    int32_t oldflag;
    int32_t oldcset;
    int32_t endx, endy;
    bool trigger, bhole;
    byte undercset;
	byte blockLayer;
	zfix step;
	bool force_many;
    
    movingblock();
	void clear();
	void set(int32_t X, int32_t Y, int32_t combo, int32_t cset, int32_t layer, int32_t placedfl);
    void push(zfix bx,zfix by,int32_t d,int32_t f);
    virtual bool animate(int32_t index);
    virtual void draw(BITMAP *dest);
};

class portal : public sprite
{
public:
	int32_t destdmap, destscr, weffect, wsfx;
    int32_t aclk, aframe, o_tile, aspd, frames;
	portal(int32_t dm, int32_t scr, int32_t gfx, int32_t sfx, int32_t spr);
	virtual bool animate(int32_t);
};

class breakable : public sprite
{
public:
    int32_t aclk, aframe, o_tile, aspd, frames;
	newcombo const& cmb;
	int32_t dropitem, breaktimer;
	int8_t breaksprtype;
	byte breaksfx,breakspr;
	
	breakable(zfix X, zfix Y, zfix Z, newcombo const& cmb, int32_t cset);
	breakable(zfix X, zfix Y, zfix Z, newcombo const& cmb, int32_t cset, int32_t dropitem,
		byte breaksfx, int8_t breaksprtype, byte breakspr, int32_t breaktimer);
	
	virtual bool animate(int32_t);
};
#endif
/*** end of sprite.cc ***/

