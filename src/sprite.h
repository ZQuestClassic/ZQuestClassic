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

// this code needs some patching for use in zquest.cc

extern itemdata *itemsbuf;
extern wpndata  *wpnsbuf;
extern bool     freeze_guys;
extern int fadeclk;
extern int frame;
extern bool BSZ;
extern int conveyclk;

/**********************************/
/******* Sprite Base Class ********/
/**********************************/

template<typename T> class EntityPtr;
class EntityRef;
class sprite_list;

class sprite
{
private:
    static long getNextUID();
    //unique sprite ID
    //given upon construction

protected:
	friend class sprite_list;
    long uid; //do not access this outside this file.
    
public:
    long getUID()
    {
        return uid;
    }
    fix x,y,z,fall;
    int tile,shadowtile,cs,flip,c_clk,clk,misc;
    
    fix xofs,yofs,zofs;
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
    int drawstyle;                                          //0=normal, 1=translucent, 2=cloaked
    int extend;
    // Scripting additions
    long miscellaneous[16];
    byte scriptcoldet;
    
    sprite();
    sprite(sprite const & other);
    sprite(fix X,fix Y,int T,int CS,int F,int Clk,int Yofs);
    virtual ~sprite();
    virtual void draw(BITMAP* dest);                        // main layer
    virtual void draw8(BITMAP* dest);                       // main layer
    virtual void drawcloaked(BITMAP* dest);                 // main layer
    virtual void drawshadow(BITMAP* dest, bool translucent);// main layer
    virtual void draw2(BITMAP* dest);                       // top layer for special needs
    virtual void drawcloaked2(BITMAP* dest);                // top layer for special needs
    virtual bool animate(int index);
    virtual void check_conveyor();
    int real_x(fix fx);
    int real_y(fix fy);
    int real_ground_y(fix fy);
    int real_z(fix fz);
    virtual bool hit(sprite *s);
    virtual bool hit(int tx,int ty,int tz,int txsz,int tysz,int tzsz);
    
    
    virtual int hitdir(int tx,int ty,int txsz,int tysz,int dir);
    virtual void move(fix dx,fix dy);
    virtual void move(fix s);
    
    inline fix getX() const { return x; }
    inline fix getY() const { return y; }
    inline fix getZ() const { return z; }
    
    inline void setX(fix newX) { x=newX; }
    inline void setY(fix newY) { y=newY; }
    inline void setZ(fix newZ) { z=newZ; }
    
    inline int getDir() const { return dir; }
    inline void setDir(int newDir) { dir=(newDir<0) ? 0 : (newDir>7) ? 7 : newDir; }
    
    inline void markForDeletion() { toBeDeleted=true; }
    inline bool isMarkedForDeletion() const { return toBeDeleted; }
    
private:
    bool toBeDeleted;
    EntityRef* ref;
    
    template<typename T> friend class EntityPtr;
    friend void registerSprite();
};


/***************************************************************************/

/**********************************/
/********** Sprite List ***********/
/**********************************/

#define SLMAX 255

class sprite_list
{
	int count;
    sprite *sprites[SLMAX];
	long uids[SLMAX];
    
public:
    sprite_list();
    
    sprite *getByUID(long uid) const;
	bool isValidUID(long uid) const;

    void clear();
    sprite *spr(int index);

	// TODO: 'add*()' is not entirely clear how it works.
	//   (this can be a cause of bugs; such as in the case of reflected weapons
	//    it can actually delete the sprite and cause undefined behavior...
	//      [note; this has been fixed in that case only. use with caution.])

    bool add(sprite *s);
    bool swap(int a,int b);
    bool addAtFront(sprite *s);

	// adds a sprite that was created in a past frame. (eg; reflected weapons)
	bool addExisting(sprite *s);

    // removes pointer from list but doesn't delete it
    bool remove(sprite *s);

    fix getX(int j);
    fix getY(int j);
    int getID(int j);
    int getMisc(int j);
    bool del(int j);
    void draw(BITMAP* dest,bool lowfirst);
    void drawshadow(BITMAP* dest,bool translucent, bool lowfirst);
    void draw2(BITMAP* dest,bool lowfirst);
    void drawcloaked2(BITMAP* dest,bool lowfirst);
    void animate();
    void check_conveyor();
    int Count();
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
    void push(fix bx,fix by,int d,int f);
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
};
#endif
/*** end of sprite.cc ***/

