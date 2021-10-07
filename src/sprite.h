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
extern int fadeclk;
extern int frame;
extern bool BSZ;
extern int conveyclk;

struct scriptmem
{
    long stack[MAX_SCRIPT_REGISTERS];
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
    static long getNextUID();
    //unique sprite ID
    //given upon construction
    long uid;
    
public:
    long getUID()
    {
        return uid;
    }
    
   
    
    zfix x,y,z,fall;
    int tile,shadowtile,cs,flip,c_clk,clk,misc;
    zfix xofs,yofs,zofs;
    // no hzofs - it's always equal to zofs.
    int hxofs,hyofs,hxsz,hysz,hzsz;
    int txsz,tysz;
    /*
     * Explanation of hzsz:
     * Hzsz is how "tall" a sprite is.
     * To jump over a sprite, you must be higher than its z+hzsz.
     * Enemies have hzsz of 0 so that Link can jump over them (see LA.)
     */
    int id,dir;
    bool angular,canfreeze;
    double angle;
    int lasthit, lasthitclk;
    int dummy_int[10];
    zfix dummy_fix[10];
    float dummy_float[10];
    bool dummy_bool[10];
    int drawstyle;                                          //0=normal, 1=translucent, 2=cloaked
    int extend;
    // Scripting additions
    long miscellaneous[32];
    byte scriptcoldet;
    int wpnsprite; //wpnsprite is new for 2.6 -Z
	scriptmem* scrmem;
    byte initialised;
    dword scriptflag;
    word doscript;
    byte waitdraw;
    word script;
    word weaponscript;
    long initD[8];
    long initA[2];
    long weap_initd[8];
    long weap_inita[2];
    int scripttile;
    signed char scriptflip;
    byte do_animation;
    int rotation;
    int scale; 
    byte moveflags;
    byte drawflags;
	byte knockbackflags;
	byte screenedge;
	int scriptshadowtile;
#define FLAG_NOSLIDE 0x01
#define FLAG_NOSCRIPTKNOCKBACK 0x02
	byte knockbackSpeed;
	int script_knockback_clk;
	int script_knockback_speed;
	int pit_pulldir; // Pitfall pull direction
	int pit_pullclk; // Pitfall pull clk
	int fallclk; // Pitfall fall clk
	int fallCombo; // Pitfall fallen combo
	int old_cset; // Storage var for an old cset; used by pitfalls
	int drownclk; // Pitfall fall clk
	int drownCombo; // Pitfall fallen combo
	bool isspawning;
	
	byte spr_shadow, spr_death, spr_spawn;
	short spr_death_anim_clk, spr_spawn_anim_clk;
	byte spr_death_anim_frm, spr_spawn_anim_frm;
	
	byte glowRad, glowShape;
    
    sprite();
    sprite(sprite const & other);
    sprite(zfix X,zfix Y,int T,int CS,int F,int Clk,int Yofs);
    virtual ~sprite();
	virtual void handle_sprlighting();
    virtual void draw(BITMAP* dest);                        // main layer
    virtual void drawzcboss(BITMAP* dest);                        // main layer
    virtual void old_draw(BITMAP* dest);                        // main layer
    virtual void draw8(BITMAP* dest);                       // main layer
    virtual void drawcloaked(BITMAP* dest);                 // main layer
    virtual void drawshadow(BITMAP* dest, bool translucent);// main layer
    virtual void draw2(BITMAP* dest);                       // top layer for special needs
    virtual void drawcloaked2(BITMAP* dest);                // top layer for special needs
    virtual bool animate(int index);
    virtual void check_conveyor();
	virtual int get_pit(); //Returns combo ID of pit that sprite WOULD fall into; no side-effects
	virtual int check_pits(); //Returns combo ID of pit fallen into; 0 for not fallen.
	virtual int get_water(); //Returns combo ID of pit that sprite WOULD fall into; no side-effects
	virtual int check_water(); //Returns combo ID of pit fallen into; 0 for not fallen.
    int real_x(zfix fx);
    int real_y(zfix fy);
    int real_ground_y(zfix fy);
    int real_z(zfix fz);
    virtual bool hit(sprite *s);
    virtual bool hit(int tx,int ty,int tz,int txsz,int tysz,int tzsz);
    
    
    virtual int hitdir(int tx,int ty,int txsz,int tysz,int dir);
    virtual void move(zfix dx,zfix dy);
    virtual void move(zfix s);
	virtual bool knockback(int time, int dir, int speed);
	virtual bool runKnockback();
    void explode(int mode);
	
	void alloc_scriptmem();
	
	virtual int run_script(int mode);
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
    int count;
	int active_iterator;
	long max_sprites;
    map<long, int> containedUIDs;
    // Cache requests from scripts
    mutable long lastUIDRequested;
    mutable sprite* lastSpriteRequested;
    
public:
    sprite_list();
    
    sprite *getByUID(long uid);
    void clear();
    sprite *spr(int index);
    bool swap(int a,int b);
    bool add(sprite *s);
    // removes pointer from list but doesn't delete it
    bool remove(sprite *s);
    zfix getX(int j);
    zfix getY(int j);
    int getID(int j);
    int getMisc(int j);
	long getMax() {return max_sprites;}
	void setMax(long max) {max_sprites = (max < SLMAX ? max : SLMAX);}
    bool del(int j);
    void draw(BITMAP* dest,bool lowfirst);
    void drawshadow(BITMAP* dest,bool translucent, bool lowfirst);
    void draw2(BITMAP* dest,bool lowfirst);
    void drawcloaked2(BITMAP* dest,bool lowfirst);
    void animate();
    void check_conveyor();
	void run_script(int mode);
    int Count();
	bool has_space(int space = 1);
    int hit(sprite *s);
    int hit(int x,int y,int z,int xsize, int ysize, int zsize);
    // returns the number of sprites with matching id
    int idCount(int id, int mask);
    // returns index of first sprite with matching id, -1 if none found
    int idFirst(int id, int mask);
    // returns index of last sprite with matching id, -1 if none found
    int idLast(int id, int mask);
    // returns the number of sprites with matching id
    int idCount(int id);
    // returns index of first sprite with matching id, -1 if none found
    int idFirst(int id);
    // returns index of last sprite with matching id, -1 if none found
    int idLast(int id);
    
private:

    void checkConsistency(); //for debugging
};

/**********************************/
/********** Moving Block **********/
/**********************************/

class movingblock : public sprite
{
public:
    int bcombo;
    int oldflag;
    int oldcset;
    int endx, endy;
    bool trigger;
    byte undercset;
    
    movingblock();
    void push(zfix bx,zfix by,int d,int f);
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
};
#endif
/*** end of sprite.cc ***/

