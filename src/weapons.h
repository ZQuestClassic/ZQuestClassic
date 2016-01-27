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
#include "sfxClass.h"

/**************************************/
/***********  Weapon Class  ***********/
/**************************************/

extern byte boomframe[16];
extern byte bszboomflip[4];

class weapon : public sprite
{
private:
    void seekLink();
    void seekEnemy(int j);
    
public:
    int power,type,dead,clk2,misc2,ignorecombo;
    bool isLit; //if true, this weapon is providing light to the current screen
    int parentid, //Enemy who created it
        parentitem; //Item which created it
    int dragging;
    fix step;
    bool bounce, ignoreLink;
    word flash,wid,aframe,csclk;
    int o_tile, o_cset, o_speed, o_type, frames, o_flip;
    int temp1;
    bool behind;
    bool aimedBrang;
    
    weapon(weapon const & other);
    weapon(fix X,fix Y,fix Z,int Id,int Type,int pow,int Dir, int Parentid, int prntid, bool isDummy=false);
    virtual ~weapon();
    void LOADGFX(int wpn);
    bool Dead();
    bool clip();
    bool blocked();
    virtual bool blocked(int xOffset, int yOffset);
    virtual bool animate(int index);
    virtual void onhit(bool clipped);
    virtual void onhit(bool clipped, int special, int linkdir);
    // override hit detection to check for invicibility, etc
    virtual bool hit(sprite *s);
    virtual bool hit(int tx,int ty,int tz,int txsz,int tysz,int tzsz);
    virtual void draw(BITMAP *dest);
    virtual void update_weapon_frame(int change, int orig);
    void setAngle(double angle);
    
    // Bombs only
    void activateRemoteBomb();
    
    // Boomerangs and hookshots only
    // Shift dragged item index after an item with a lower index is collected
    void adjustDraggedItem(int removed);
    
    // Wind only
    inline void pickUpLink() { misc=999; }
    inline bool hasLink() { return misc==999; }
    
    inline void setSparkleFunc(void (*sf)(const weapon&)) { sparkleFunc=sf; }
    
private:
    SFX sfxLoop;
    
    // How close the weapon can get to the edge of the screen before
    // being deleted (or bouncing, in the case of boomerangs)
    int minX, maxX, minY, maxY;
    
    // If set, this function will be called every frame to add sparkles.
    void (*sparkleFunc)(const weapon&);
    
    // Set dir to the appropriate value for angle.
    void matchDirToAngle();
    
    friend void setScreenLimits(weapon&);
};

void putMiscSprite(BITMAP *dest, int spr, int x, int y);
void putweapon(BITMAP *dest,int x,int y,int weapon_id, int type, int dir, int &aclk, int &aframe,
               int parentid);
#endif
/*** end of sprite.cc ***/

