//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  guys.cc
//
//  "Guys" code (and other related stuff) for zelda.cc
//
//  Still has some hardcoded stuff that should be moved
//  out into defdata.cc for customizing the enemies.
//
//--------------------------------------------------------

//#include "guys.h"
#ifndef _GUYS_H_
#define _GUYS_H_
#include <list>
#include "sprite.h"
#include "weapons.h"

#include "enemyAttack.h"
#include "sfxClass.h"
#include "scoped_ptr.h"

class item;

extern int repaircharge;
extern bool adjustmagic;
extern bool learnslash;
extern int itemindex;
extern int wallm_load_clk;
extern int sle_x,sle_y,sle_cnt,sle_clk;
extern int vhead;
extern int guycarryingitem;

bool tooclose(int x,int y,int d);
bool isflier(int id);
bool never_in_air(int id);
// Returns true iff a combo type or flag precludes enemy movement.
bool groundblocked(int dx, int dy);
// Returns true iff enemy is floating and blocked by a combo type or flag.
bool flyerblocked(int dx, int dy, int special);

// Start spinning tiles - called by load_default_enemies
void awaken_spinning_tile(mapscr *s, int pos);

// Used to remove/add fireball shooter enemies
void screen_combo_modify_preroutine(mapscr *s, int pos);
void screen_combo_modify_postroutine(mapscr *s, int pos);

// Find the IDs of enemies spawned by combos and flags. Called once on loading a quest.
void identifyCFEnemies();

// Let's make sure this check is consistent, shall we?
#define ON_SIDEPLATFORM (_walkflag(x+4,y+16,0) || (y>=160 && currscr>=0x70 && !(tmpscr->flags2&wfDOWN)))

/**********************************/
/*******  Enemy Base Class  *******/
/**********************************/

class enemy : public sprite
{
public:
    guydata *d;
    // Approximately all of these variables are accessed by either ffscript.cpp or inherited classes
    int o_tile, frate, hp, hclk, stunclk, fading, superman;
    bool mainguy, did_armos;
    byte item_set, grumble;
    bool itemguy, count_enemy, dying, ceiling, leader, scored, script_spawned;
    fix  step, floor_y;
    
    //d variables
    dword flags;
    dword flags2;
    
    short  family, dcset, anim;
    short  dp, wdp, wpn;
    
    short  rate, hrate, homing;
    fix dstep;
    long dmisc1, dmisc2, dmisc3, dmisc4, dmisc5, dmisc6, dmisc7, dmisc8, dmisc9, dmisc10, dmisc11, dmisc12, dmisc13, dmisc14, dmisc15;
    short bgsfx, bosspal;
    byte defense[edefLAST];
    
    int dummy_int1;
    
    enemy(fix X,fix Y,int Id,int Clk);                      // : sprite()
    virtual ~enemy();
    
    // auomatically kill off enemy (for rooms with ringleaders)
    virtual void kickbucket();
    // Take damage or ignore it
    virtual int takehit(weapon *w);
    // override hit detection to check for invicibility, stunned, etc
    virtual bool hit(sprite *s);
    virtual bool hit(int tx,int ty,int tz,int txsz,int tysz,int tzsz);
    virtual bool hit(weapon *w);
    virtual void break_shield() {}; // Overridden by types that can have shields
    virtual bool ignore_msg_freeze()
    {
        return false;
    }
    
protected:
    int  clk2,sclk;
    word  s_tile; //secondary (additional) tile(s)
    
    //virtual void move(fix s);
    void leave_item();
    
    // take damage or ignore it
    virtual bool hitshield(int wpnx, int wpny, int xdir);
    virtual int defend(int wpnId, int *power, int edef);
    bool candamage(int power, int edef);
    int defenditemclass(int wpnId, int *power);
    
    bool dont_draw();
    // base drawing function to be used by all derived classes instead of
    // sprite::draw()
    virtual void draw(BITMAP *dest);
    // similar to the overblock function - draws a 32x32 sprite
    void drawblock(BITMAP *dest);
    virtual void drawshadow(BITMAP *dest, bool translucent);
    void masked_draw(BITMAP *dest,int mx,int my,int mw,int mh);
    
    //                         --==**==--
    //   Movement routines that can be used by derived classes as needed
    //                         --==**==--
    
    // returns true if next step is ok, false if there is something there
    bool canmove(int ndir,fix s,int special,int dx1,int dy1,int dx2,int dy2);
    bool canmove(int ndir,fix s,int special);
    bool canmove(int ndir,int special);
    bool canmove(int ndir);
    // falls through the Z axis;
    void falldown();
    int pos(int x,int y);
    // Enemies that cannot ever be penetrated by weapons
    bool cannotpenetrate();
    
private:
    bool shieldCanBlock;
    
protected:
    SFX bgSFX, hitSFX, deathSFX;
    scoped_ptr<EnemyAttack> attack;
    
public:
    void setBGSFX(SFX newBG);
    inline int getDir() const { return dir; }
    
    void setAttack(EnemyAttack* att);
    
    friend void registerEnemy();
};

class asIScriptFunction;

class ASEnemy: public enemy
{
public:
    bool haslink;
    
    ASEnemy(const char* scriptName, fix x, fix y, int id, int clk);
    bool animate(int index);
    void draw(BITMAP *dest);
    void draw2(BITMAP *dest);
    void drawshadow(BITMAP *dest, bool translucent);
    bool hit(weapon *w);
    int takehit(weapon *w);
    int defend(int wpnId, int* power, int edef);
    void break_shield();
    void eatlink();
    void setDeathAttack(EnemyAttack* att);
    
private:
    asIScriptFunction* asUpdate;
    asIScriptFunction* asDraw;
    asIScriptFunction* asDraw2;
    asIScriptFunction* asDrawShadow;
    asIScriptFunction* asTakeHit;
    asIScriptFunction* asHit;
    asIScriptFunction* asBreakShield;
    asIScriptFunction* asEatLink;
    
    int scriptFlags;
    int wpnPower;
    scoped_ptr<EnemyAttack> deathAttack;
    
    void fireWeapon();
    bool isFiring() const;
    void updateFiring();
    void setBreathTimer(int time);
    void setAttackOwner(int index);
    void activateDeathAttack();
    void scriptDraw();
    void scriptDrawBlock();
    void scriptDrawShadow();
    void scriptMaskedDraw(int mx, int my, int mw, int mh);
    void overTileCloaked16(int tile, int x, int y, int flip);
    void overTile16(int tile, int x, int y, int cset, int flip);
    void stopBGSFX();
    void playDeathSFX();
    void playHitSFX(int pan);
    bool defaultAnimate(int index);
    int defaultTakeHit(weapon* w);
    void kickbucket();
    int scriptDefendItemClass(int wpnId, int power);
    int getDefendedItemPower();
    
    friend void registerEnemy();
};

/**********************************/
/**********  Misc Code  ***********/
/**********************************/
// Used by Link's swords & stomp boots
int hit_enemy(int index,int wpnId,int power,int wpnx,int wpny,int dir, int enemyHitWeapon);
void enemy_scored(int index);
void addguy(int x,int y,int id,int clk,bool mainguy);
item* additem(int x,int y,int id,int pickup);
item* additem(int x,int y,int id,int pickup,int clk);
void kill_em_all();
// For Link's hit detection. Don't count them if they are stunned or are guys.
int GuyHit(int tx,int ty,int tz,int txsz,int tysz,int tzsz);
// For Link's hit detection. Count them if they are dying.
int GuyHit(int index,int tx,int ty,int tz,int txsz,int tysz,int tzsz);
bool hasMainGuy();
void EatLink(int index);
void GrabLink(int index);
bool CarryLink();
void movefairy(fix &x,fix &y,int misc);
void movefairy2(fix x,fix y,int misc);
void killfairy(int misc);
int addenemy(int x,int y,int id,int clk);
int addenemy(int x,int y,int z,int id,int clk);
bool isjumper(int id);
bool canfall(int id);
void addfires();
void loadguys();
void loaditem();
void never_return(int index);
bool hasBoss();
bool slowguy(int id);
bool ok2add(int id);
void load_default_enemies();
void nsp(bool random);
int next_side_pos(bool random);
bool can_side_load(int id);
void side_load_enemies();
void loadenemies();
void setupscreen();
int wpnsfx(int wpn);
bool m_walkflag(int dx,int dy,int special, int x=-1000, int y=-1000);

/***  Collision detection & handling  ***/

void check_collisions();
void dragging_item();
void roaming_item();
int more_carried_items();
#endif
/*** end of guys.cc ***/

