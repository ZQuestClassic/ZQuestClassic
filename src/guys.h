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

extern int repaircharge;
extern bool adjustmagic;
extern bool learnslash;
extern int itemindex;
extern int wallm_load_clk;
extern int sle_x,sle_y,sle_cnt,sle_clk;
extern int vhead;
extern int guycarryingitem;

int random_layer_enemy();
int count_layer_enemies();
bool can_do_clock();
int link_on_wall();
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
    int o_tile, frate, hp, hclk, clk3, stunclk, timer, fading, superman, mainguy, did_armos;
    byte movestatus, item_set, grumble, posframe;
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
    byte defense[edefLAST255];
    byte hitsfx,deadsfx;
    
    long hitby[NUM_HIT_TYPES_USED];
    short firesfx;
    bool isCore;
    short parentCore; 
    int script_UID; 	//used to determine the UID of an enemy by the user (in scripts), plus
			//in-engine assignment of children to a parent and
			//for future use in npc scripts (as the ref for `this`)--the easy way to determine to
			//what npc a script on the stack is bound.
    
    int wpnsprite; //wpnsprite is new for 2.6 -Z
    int SIZEflags; //Flags for size panel offsets. The user must enable these to override defaults. 
    int frozentile, frozencset, frozenclock;
    short frozenmisc[10];
    
    long dmisc16, dmisc17, dmisc18, dmisc19, dmisc20, dmisc21, dmisc22, dmisc23, 
	dmisc24, dmisc25, dmisc26, dmisc27, dmisc28, dmisc29, dmisc30, dmisc31, dmisc32;
    long movement[32]; //Reserved for npc movement types and args. 
    long new_weapon[32]; //Reserved for weapon patterns and args.
    word npcscript; 
    long initD[8], initA[2];
    
    long dialogue_str; //WIll be set in spawn flags. 
    
    
    int getScriptUID();
    void setScriptUID(int new_id);
    
    fix  getX();
    fix  getY();
    int  getID();
    enemy(fix X,fix Y,int Id,int Clk);                      // : sprite()
    virtual ~enemy();
    
    // Supplemental animation code that all derived classes should call
    // as a return value for animate().
    // Handles the death animation and returns true when enemy is finished.
    virtual bool Dead(int index);
    // Basic animation code that all derived classes should call.
    // The one with an index is the one that is called by
    // the guys sprite list; index is the enemy's index in the list.
    virtual bool animate(int index);
    
    // auomatically kill off enemy (for rooms with ringleaders)
    virtual void kickbucket();
    // Stop BG SFX only if no other enemy is playing it
    void stop_bgsfx(int index);
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
    int  starting_hp;
    int  ox, oy;
    word  s_tile; //secondary (additional) tile(s)
    
    // to allow for different sfx on defeating enemy
    virtual void death_sfx();
    virtual void move(fix dx,fix dy);
    virtual void removearmos(int ax,int ay);
    virtual void move(fix s);
    void leave_item();
    
    // take damage or ignore it
    virtual bool hitshield(int wpnx, int wpny, int xdir);
    virtual int defend(int wpnId, int *power, int edef);
    //virtual int defend_wdmg(int wpnId, int dmg, int edef);
    bool candamage(int power, int edef);
    int defenditemclass(int wpnId, int *power);
    
    bool dont_draw();
    // base drawing function to be used by all derived classes instead of
    // sprite::draw()
    virtual void draw(BITMAP *dest);
    // similar to the overblock function--can do up to a 32x32 sprite
    void drawblock(BITMAP *dest,int mask);
    virtual void drawshadow(BITMAP *dest, bool translucent);
    void masked_draw(BITMAP *dest,int mx,int my,int mw,int mh);
    
    //                         --==**==--
    //   Movement routines that can be used by derived classes as needed
    //                         --==**==--
    void fix_coords(bool bound=false);
    // returns true if next step is ok, false if there is something there
    bool canmove(int ndir,fix s,int special,int dx1,int dy1,int dx2,int dy2);
    bool canmove(int ndir,fix s,int special);
    bool canmove(int ndir,int special);
    bool canmove(int ndir);
    // 8-directional
    void newdir_8(int rate,int homing, int special,int dx1,int dy1,int dx2,int dy2);
    void newdir_8(int rate,int homing, int special);
    // makes the enemy slide backwards when hit
    // sclk: first byte is clk, second byte is dir
    bool slide();
    bool can_slide();
    bool fslide();
    // changes enemy's direction, checking restrictions
    // rate:   0 = no random changes, 16 = always random change
    // homing: 0 = none, 256 = always
    // grumble 0 = none, 4 = strongest appetite
    void newdir(int rate,int homing,int special);
    void newdir();
    fix distance_left();
    // keeps walking around
    void constant_walk(int rate,int homing,int special);
    void constant_walk();
    // falls through the Z axis;
    void falldown();
    int pos(int x,int y);
    // for variable step rates
    void variable_walk(int rate,int homing,int special);
    // pauses for a while after it makes a complete move (to a new square)
    void halting_walk(int rate,int homing,int special,int hrate, int haltcnt);
    // 8-directional movement, aligns to 8 pixels
    void constant_walk_8(int rate,int homing,int special);
    // 8-directional movement, halting
    void halting_walk_8(int newrate,int newhoming, int newclk,int special,int newhrate, int haltcnt);
    // 8-directional movement, no alignment
    void variable_walk_8(int rate,int homing,int newclk,int special);
    // same as above but with variable enemy size
    void variable_walk_8(int rate,int homing,int newclk,int special,int dx1,int dy1,int dx2,int dy2);
    // the variable speed floater movement
    // ms is max speed
    // ss is step speed
    // s is step count
    // p is pause count
    // g is graduality :)
    void floater_walk(int rate,int newclk,fix ms,fix ss,int s,int p, int g);
    void floater_walk(int rate,int newclk,fix s);
    // Checks if enemy is lined up with Link. If so, returns direction Link is
    // at as compared to enemy. Returns -1 if not lined up. Range is inclusive.
    int lined_up(int range, bool dir8);
    // returns true if Link is within 'range' pixels of the enemy
    bool LinkInRange(int range);
    // Breathe fire
    void FireBreath(bool seeklink);
    // Shoot weapons
    void FireWeapon();
    // place the enemy in line with Link (red wizzrobes)
    void place_on_axis(bool floater, bool solid_ok);
    void update_enemy_frame();
    void n_frame_n_dir(int frames, int dir, int f4);
    void tiledir_small(int ndir, bool fourdir);
    void tiledir_three(int ndir);
    void tiledir(int ndir, bool fourdir);
    void tiledir_big(int ndir, bool fourdir);
    // Enemies that cannot ever be penetrated by weapons
    bool cannotpenetrate();
    
private:
    bool shieldCanBlock;
};

/********************************/
/*********  Guy Class  **********/
/********************************/

// good guys, fires, fairy, and other non-enemies
// based on enemy class b/c guys in dungeons act sort of like enemies
// also easier to manage all the guys this way
class guy : public enemy
{
public:
    guy(fix X,fix Y,int Id,int Clk,bool mg);                // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
    virtual bool ignore_msg_freeze()
    {
        return true;
    }
};

/*******************************/
/*********   Enemies   *********/
/*******************************/

class eFire : public enemy
{
public:
    int clk4;
    bool shield;
    eFire(fix X,fix Y,int Id,int Clk);                      // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
    virtual int takehit(weapon *w);
    virtual void break_shield();
};

class eOther : public enemy
{
public:
    int clk4;
    bool shield;
    eOther(fix X,fix Y,int Id,int Clk);                      // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
    virtual int takehit(weapon *w);
    virtual void break_shield();
};

void removearmos(int ax,int ay);

class eGhini : public enemy
{
public:
    int clk4, ox, oy, c;
    eGhini(fix X,fix Y,int Id,int Clk);                     // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
    virtual void kickbucket();
};

class eTektite : public enemy
{
    fix old_y; // a kludge used to check for ZScript modification of Y
public:
    int c;
    int clk2start;
    int cstart;
    
    eTektite(fix X,fix Y,int Id,int Clk);                   // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void drawshadow(BITMAP *dest,bool translucent);
    virtual void draw(BITMAP *dest);
};

class eItemFairy : public enemy
{
public:
    eItemFairy(fix X,fix Y,int Id,int Clk);                 // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
};

class ePeahat : public enemy
{
public:
    int ox, oy, c;
    ePeahat(fix X,fix Y,int Id,int Clk);                    // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void drawshadow(BITMAP *dest, bool translucent);
    virtual void draw(BITMAP *dest);
    virtual int takehit(weapon *w);
    virtual void kickbucket();
};

class eLeever : public enemy
{
public:
    bool temprule;
    eLeever(fix X,fix Y,int Id,int Clk);                    // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    bool canplace(int d);
    virtual void draw(BITMAP *dest);
};

class eWallM : public enemy
{
public:
    bool haslink;
    eWallM(fix X,fix Y,int Id,int Clk);                     // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    void wallm_crawl();
    void grablink();
    virtual void draw(BITMAP *dest);
};

class eTrap : public enemy
{
public:
    int  ox, oy;
    eTrap(fix X,fix Y,int Id,int Clk);                      // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    bool trapmove(int ndir);
    bool clip();
    virtual void draw(BITMAP *dest);
    virtual int takehit(weapon* w);
};

class eTrap2 : public enemy                                 //trap that goes back and forth constantly
{
public:
    eTrap2(fix X,fix Y,int Id,int Clk);                     // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    bool trapmove(int ndir);
    bool clip();
    virtual void draw(BITMAP *dest);
    virtual int takehit(weapon *w);
};

class eRock : public enemy
{
public:
    eRock(fix X,fix Y,int Id,int Clk);                      // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void drawshadow(BITMAP *dest, bool translucent);
    virtual void draw(BITMAP *dest);
    virtual int takehit(weapon *w);
};

class eBoulder : public enemy
{
public:
    eBoulder(fix X,fix Y,int Id,int Clk);                      // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void drawshadow(BITMAP *dest, bool translucent);
    virtual void draw(BITMAP *dest);
    virtual int takehit(weapon *w);
};

// Helper for launching fireballs from statues, etc.
// It's invisible and can't be hit.
// Pass the range value through the clk variable in the constuctor.
class eProjectile : public enemy
{
public:
    eProjectile(fix X,fix Y,int Id,int Clk);                     // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
    
private:
    const int minRange;
};

class eTrigger : public enemy
{
public:
    eTrigger(fix X,fix Y,int Id,int Clk);                   // : enemy(X,Y,Id,Clk)
    virtual void draw(BITMAP *dest);
    virtual void death_sfx();
};

class eNPC : public enemy
{
public:
    eNPC(fix X,fix Y,int Id,int Clk);                       // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
    virtual int takehit(weapon *w);
};

class eSpinTile : public enemy
{
public:
    eSpinTile(fix X,fix Y,int Id,int Clk);                       // : enemy(X,Y,Id,Clk)
    virtual void facelink();
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
    virtual void drawshadow(BITMAP *dest, bool translucent);
};

class eZora : public enemy
{
public:
    eZora(fix X,fix Y,int Id,int Clk);                      // : enemy(X,Y,Id,0)
    virtual void facelink();
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
};

class eStalfos : public enemy
{
public:
    int clk4, //Tribble clock
        clk5; // Like Like clock
    bool fired; // Darknut5, Bombchu
    bool shield; // Darknut
    bool dashing; // Rope
    bool haslink; // Like Like
    int multishot; // Shoot twice/thrice/n times in one halt
    fix fy, shadowdistance; // Pols Voice
    
    eStalfos(fix X,fix Y,int Id,int Clk);                   // : enemy(X,Y,Id,Clk)
    bool WeaponOut();
    void KillWeapon();
    void charge_attack();
    void eatlink();
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
    virtual int takehit(weapon *w);
    void vire_hop();
    virtual void drawshadow(BITMAP *dest, bool translucent);
    virtual void break_shield();
};

class eKeese : public enemy
{
public:
    int ox, oy, c;
    int clk4;
    eKeese(fix X,fix Y,int Id,int Clk);                     // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void drawshadow(BITMAP *dest, bool translucent);
    virtual void draw(BITMAP *dest);
};

class eWizzrobe : public enemy
{
public:
    bool charging;
    bool firing;
    int fclk;
    eWizzrobe(fix X,fix Y,int Id,int Clk);                  // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    void wizzrobe_attack();
    void wizzrobe_attack_for_real();
    void wizzrobe_newdir(int homing);
    virtual void draw(BITMAP *dest);
};

/*********************************/
/**********   Bosses   ***********/
/*********************************/

class eDodongo : public enemy
{
public:
    eDodongo(fix X,fix Y,int Id,int Clk);                   // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
    virtual int takehit(weapon *w);
};

class eDodongo2 : public enemy
{
public:
    int previous_dir;
    eDodongo2(fix X,fix Y,int Id,int Clk);                  // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
    virtual int takehit(weapon *w);
};

class eAquamentus : public enemy
{
public:
    int clk4, fbx; //fireball x position
    eAquamentus(fix X,fix Y,int Id,int Clk); // : enemy((fix)176,(fix)64,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
    virtual bool hit(weapon *w);
};

class eGohma : public enemy
{
public:
    int clk4; // Movement clk must be separate from animation clk because of the Clock item
    eGohma(fix X,fix Y,int Id,int Clk);                     // : enemy((fix)128,(fix)48,Id,0)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
    virtual int takehit(weapon *w);
};

class eLilDig : public enemy
{
public:
    eLilDig(fix X,fix Y,int Id,int Clk);                    // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
};

class eBigDig : public enemy
{
public:
    eBigDig(fix X,fix Y,int Id,int Clk);                    // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
    virtual int takehit(weapon *w);
};

class eGanon : public enemy
{
public:
    int Stunclk;
    eGanon(fix X,fix Y,int Id,int Clk);                     // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual int takehit(weapon *w);
    virtual void draw(BITMAP *dest);
    void draw_guts(BITMAP *dest);
    void draw_flash(BITMAP *dest);
};

void getBigTri(int id2);

/**********************************/
/***  Multiple-Segment Enemies  ***/
/**********************************/

// segment manager
class eMoldorm : public enemy
{
public:
    int segcnt;
    int segid;
    // tell it the segment count through the clk param
    eMoldorm(fix X,fix Y,int Id,int Clk);                   // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
};

// segment class
class esMoldorm : public enemy
{
public:
    esMoldorm(fix X,fix Y,int Id,int Clk);                  // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual int takehit(weapon *w);
    virtual void draw(BITMAP *dest);
private:
    int parentclk; //because of custom step speed, clk is not sufficient to keep track
    //of when to check the parent Moldorm's direction, since the frequency
    //of the parent's checking might not divide the Moldorm segment's
    //number of animation frames. -DD
};

class eBaseLanmola : public enemy
{
public:
    eBaseLanmola(fix X, fix Y, int Id, int Clk) : enemy(X, Y, Id, Clk) {}
    std::list<std::pair< std::pair<fix, fix>, int> > prevState;
};

// segment manager
class eLanmola : public eBaseLanmola
{
public:
    int segcnt;
    // tell it the segment count through the clk param
    eLanmola(fix X,fix Y,int Id,int Clk);                   // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
};

// segment class
class esLanmola : public eBaseLanmola
{
public:
    esLanmola(fix X,fix Y,int Id,int Clk);                  // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual int takehit(weapon *w);
    virtual void draw(BITMAP *dest);
};

class eManhandla : public enemy
{
public:
    char arm[8];                                            // side the arm is on
    int armcnt;
    bool adjusted;
    eManhandla(fix X,fix Y,int Id,int Clk);                 // : enemy(X,Y,Id,0)
    virtual bool animate(int index);
    virtual int takehit(weapon *w);
    virtual void draw(BITMAP *dest);
};

class esManhandla : public enemy
{
public:
    // use clk param to tell which arm it is
    esManhandla(fix X,fix Y,int Id,int Clk);                // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
};

class eGleeok : public enemy
{
public:
    int flameclk, flamehead;
    int necktile;
    eGleeok(fix X,fix Y,int Id,int Clk);                    // : enemy((fix)120,(fix)48,Id,Clk)
    virtual bool animate(int index);
    virtual int takehit(weapon *w);
    virtual void draw(BITMAP *dest);
    virtual void draw2(BITMAP *dest);
};

// head class
class esGleeok : public enemy
{
public:
    int nx[255],ny[255];                                        //neck x and y?
    int headtile;
    int flyingheadtile;
    int necktile;
    sprite * parent; //parent Gleeok to whose neck this segment is attached
    fix xoffset, yoffset; //coords relative to the parent Gleeok
    int nxoffset[255], nyoffset[255]; //random "jiggling" of the next tiles
    esGleeok(fix X,fix Y,int Id,int Clk, sprite * prnt);                   // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual int takehit(weapon *w);
    virtual void draw(BITMAP *dest);
    virtual void draw2(BITMAP *dest);
};

class ePatra : public enemy
{
public:
    int flycnt,flycnt2, loopcnt, lookat;
    double circle_x, circle_y;
    double temp_x, temp_y;
    bool adjusted;
    ePatra(fix X,fix Y,int Id,int Clk);                     // : enemy((fix)128,(fix)48,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
    virtual int defend(int wpnId, int *power, int edef);
};

// segment class
class esPatra : public enemy
{
public:
    esPatra(fix X,fix Y,int Id,int Clk);                    // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
};

class ePatraBS : public enemy
{
public:
    int flycnt,flycnt2, loopcnt, lookat;
    double temp_x, temp_y;
    bool adjusted;
    ePatraBS(fix X,fix Y,int Id,int Clk);                   // : enemy((fix)128,(fix)48,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
    virtual int defend(int wpnId, int *power, int edef);
};

// segment class
class esPatraBS : public enemy
{
public:
    esPatraBS(fix X,fix Y,int Id,int Clk);                  // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
};

/**********************************/
/**********  Misc Code  ***********/
/**********************************/
void addEwpn(int x,int y,int z,int id,int type,int power,int dir, int parentid, byte script_gen = 0);
// Used by Link's swords & stomp boots
int hit_enemy(int index,int wpnId,int power,int wpnx,int wpny,int dir, int enemyHitWeapon);
void enemy_scored(int index);
void addguy(int x,int y,int id,int clk,bool mainguy);
void additem(int x,int y,int id,int pickup);
void additem(int x,int y,int id,int pickup,int clk);
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
bool countguy(int id);
bool ok2add(int id);
void load_default_enemies();
void nsp(bool random);
int next_side_pos(bool random);
bool can_side_load(int id);
void side_load_enemies();
void loadenemies();
void moneysign();
void putprices(bool sign);
void setupscreen();
FONT *setmsgfont();
bool parsemsgcode();
void putmsg();
int message_more_y();
int wpnsfx(int wpn); 

/***  Collision detection & handling  ***/

void check_collisions();
void clear_script_one_frame_conditions();
void dragging_item();
void roaming_item();
int more_carried_items();
#endif
/*** end of guys.cc ***/

