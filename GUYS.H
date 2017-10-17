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
#include "sprite.h"
#include "weapons.h"

extern bool repaircharge;
extern bool adjustmagic;
extern bool learnslash;
extern int itemindex;
extern int wallm_load_clk;
extern int sle_x,sle_y,sle_cnt,sle_clk;
extern int vhead;
extern int guyindex;

bool can_do_clock();
bool m_walkflag(int x,int y,int special);
int link_on_wall();
bool tooclose(int x,int y,int d);
bool isflier(int id);
bool isfloater(int id);

/**********************************/
/*******  Enemy Base Class  *******/
/**********************************/

class enemy : public sprite
{
  public:
    guydata *d;
    int  clk2,clk3,stunclk,hclk,sclk,timer;
    int  hp,fading,superman,frate;
    byte movestatus, item_set, grumble, foobyte;
    bool mainguy, leader, itemguy, count_enemy, dying, scored;
    fix  step;

    fix  getX();
    fix  getY();
    int  getID();
    enemy(fix X,fix Y,int Id,int Clk);                      // : sprite()
    virtual ~enemy();
    // Supplemental animation code that all derived classes should call
    // as a return value for animate().
    // Handles the death animation and returns true when enemy is finished.
    virtual bool Dead(int index = -1);
    // Basic animation code that all derived classes should call.
    // The one with an index is the one that is called by
    // the guys sprite list; index is the enemy's index in the list.
    virtual bool animate(int index);
    // to allow for different sfx on defeating enemy
    virtual void death_sfx();
    virtual void move(fix dx,fix dy);
    virtual void move(fix s);
    void leave_item();
    // auomatically kill off enemy (for rooms with ringleaders)
    virtual void kickbucket();
    // take damage or ignore it
    virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir);
    bool dont_draw();
    // base drawing function to be used by all derived classes instead of
    // sprite::draw()
    virtual void draw(BITMAP *dest);
    // similar to the overblock function--can do up to a 32x32 sprite
    void drawblock(BITMAP *dest,int mask);
    virtual void drawshadow(BITMAP *dest, bool translucent);
    void masked_draw(BITMAP *dest,int mx,int my,int mw,int mh);
    // override hit detection to check for invicibility, stunned, etc
    virtual bool hit(sprite *s);
    virtual bool hit(int tx,int ty,int txsz,int tysz);
    virtual bool hit(weapon *w);
    //                         --==**==--
    //   Movement routines that can be used by derived classes as needed
    //                         --==**==--
    void fix_coords();
    // returns true if next step is ok, false if there is something there
    bool canmove(int ndir,fix s,int special,int dx1,int dy1,int dx2,int dy2);
    bool canmove(int ndir,fix s,int special);
    bool canmove(int ndir,int special);
    bool canmove(int ndir);
    // 8-directional
    void newdir_8(int rate,int special,int dx1,int dy1,int dx2,int dy2);
    void newdir_8(int rate,int special);
    // makes the enemy slide backwards when hit
    // sclk: first byte is clk, second byte is dir
    bool slide();
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
    int pos(int x,int y);
    // for variable step rates
    void variable_walk(int rate,int homing,int special);
    // pauses for a while after it makes a complete move (to a new square)
    void halting_walk(int rate,int homing,int special,int hrate, int haltcnt);
    // 8-directional movement, aligns to 8 pixels
    void constant_walk_8(int rate,int special);
    // 8-directional movement, no alignment
    void variable_walk_8(int rate,int newclk,int special);
    // same as above but with variable enemy size
    void variable_walk_8(int rate,int newclk,int special,int dx1,int dy1,int dx2,int dy2);
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
    int lined_up(int range);
    // returns true if Link is within 'range' pixels of the enemy
    bool LinkInRange(int range);
    // place the enemy in line with Link (red wizzrobes)
    void place_on_axis(bool floater);
    void update_enemy_frame();
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
};

/*******************************/
/*********   Enemies   *********/
/*******************************/

class eFire : public enemy
{
  public:
    eFire(fix X,fix Y,int Id,int Clk);                      // : enemy(X,Y,Id,Clk)
    virtual void draw(BITMAP *dest);
};

void removearmos(int ax,int ay);

class eArmos : public enemy
{
  public:
    bool fromstatue;
    eArmos(fix X,fix Y,int Id,int Clk);                     // : enemy(X,Y,Id,0)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
};

class eGhini : public enemy
{
  public:
    int ox, oy, c;
    eGhini(fix X,fix Y,int Id,int Clk);                     // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
    virtual void kickbucket();
};

class eTektite : public enemy
{
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
    int ox, oy, moving_superman, c;
    ePeahat(fix X,fix Y,int Id,int Clk);                    // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void drawshadow(BITMAP *dest, bool translucent);
    virtual void draw(BITMAP *dest);
    virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir);
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

class eGel : public enemy
{
  public:
    eGel(fix X,fix Y,int Id,int Clk);                       // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
};

class eZol : public enemy
{
  public:
    eZol(fix X,fix Y,int Id,int Clk);                       // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
    virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir);
};

class eGelTrib : public enemy
{
  public:
    int clk4;
    eGelTrib(fix X,fix Y,int Id,int Clk);                   // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
};

class eZolTrib : public enemy
{
  public:
    eZolTrib(fix X,fix Y,int Id,int Clk);                   // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
    virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir);
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
    virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir);
};

class eTrap2 : public enemy                                 //trap that goes back and forth constantly
{
  public:
    eTrap2(fix X,fix Y,int Id,int Clk);                     // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    bool trapmove(int ndir);
    bool clip();
    virtual void draw(BITMAP *dest);
    virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir);
};

class eRock : public enemy
{
  public:
    eRock(fix X,fix Y,int Id,int Clk);                      // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void drawshadow(BITMAP *dest, bool translucent);
    virtual void draw(BITMAP *dest);
    virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir);
};

// Helper for launching fireballs from statues, etc.
// It's invisible and can't be hit.
// Pass the range value through the clk variable in the constuctor.
class eFBall : public enemy
{
  public:
    eFBall(fix X,fix Y,int Id,int Clk);                     // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
};

class eTrigger : public enemy
{
  public:
    eTrigger(fix X,fix Y,int Id,int Clk);                   // : enemy(X,Y,Id,Clk)
    virtual void draw(BITMAP *dest);
};

class eNPC : public enemy
{
  public:
    eNPC(fix X,fix Y,int Id,int Clk);                       // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
    virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir);
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
    bool doubleshot;
    eStalfos(fix X,fix Y,int Id,int Clk);                   // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
};

class eGibdo : public enemy
{
  public:
    eGibdo(fix X,fix Y,int Id,int Clk);                     // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
};

class eBubble : public enemy
{
  public:
    eBubble(fix X,fix Y,int Id,int Clk);                    // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
};

class eRope : public enemy
{
  public:
    bool charging;
    eRope(fix X,fix Y,int Id,int Clk);                      // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    void charge_attack();
    virtual void draw(BITMAP *dest);
};

class eKeese : public enemy
{
  public:
    int ox, oy, c;
    eKeese(fix X,fix Y,int Id,int Clk);                     // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void drawshadow(BITMAP *dest, bool translucent);
    virtual void draw(BITMAP *dest);
};

class eVire : public enemy
{
  public:
    eVire(fix X,fix Y,int Id,int Clk);                      // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void drawshadow(BITMAP *dest, bool translucent);
    virtual void draw(BITMAP *dest);
    void vire_hop();
};

class eKeeseTrib : public enemy
{
  public:
    int ox, oy, c;
    int clk4;
    eKeeseTrib(fix X,fix Y,int Id,int Clk);                 // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void drawshadow(BITMAP *dest, bool translucent);
    virtual void draw(BITMAP *dest);
};

class eVireTrib : public enemy
{
  public:
    eVireTrib(fix X,fix Y,int Id,int Clk);                  // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void drawshadow(BITMAP *dest, bool translucent);
    virtual void draw(BITMAP *dest);
    void vire_hop();

};

class ePolsVoice : public enemy
{
  public:
    fix fy, shadowdistance;
    ePolsVoice(fix X,fix Y,int Id,int Clk);                 // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void drawshadow(BITMAP *dest, bool translucent);
    virtual void draw(BITMAP *dest);
    virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir);
    void polsvoice_hop();
};

class eLikeLike : public enemy
{
  public:
    bool haslink;
    eLikeLike(fix X,fix Y,int Id,int Clk);                  // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
    void eatlink();
};

class eShooter : public enemy
{
  public:
    eShooter(fix X,fix Y,int Id,int Clk);                   // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
};

class eOctorok : public enemy
{
  public:
    eOctorok(fix X,fix Y,int Id,int Clk);                   // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
};

class eMoblin : public enemy
{
  public:
    eMoblin(fix X,fix Y,int Id,int Clk);                   // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
};

class eLynel : public enemy
{
  public:
    eLynel(fix X,fix Y,int Id,int Clk);                     // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
};

class eGoriya : public enemy
{
  public:
    eGoriya(fix X,fix Y,int Id,int Clk);                    // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    bool WeaponOut();
    void KillWeapon();
    virtual void draw(BITMAP *dest);
};

class eDarknut : public enemy
{
  private:
    bool noshield;
    bool fired;
  public:
    eDarknut(fix X,fix Y,int Id,int Clk);                   // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
    virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir);
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
    virtual void draw(BITMAP *dest);
    virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir);
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
    virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir);
    virtual void death_sfx();
};

class eDodongo2 : public enemy
{
  public:
    int previous_dir;
    eDodongo2(fix X,fix Y,int Id,int Clk);                  // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
    virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir);
    virtual void death_sfx();
};

class eAquamentus : public enemy
{
  public:
    int fbx; //fireball x position
    eAquamentus(fix X,fix Y,int Id,int Clk);                // : enemy((fix)176,(fix)64,Id,Clk)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
    virtual bool hit(weapon *w);
    virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir);
    virtual void death_sfx();
};

class eGohma : public enemy
{
  public:
    eGohma(fix X,fix Y,int Id,int Clk);                     // : enemy((fix)128,(fix)48,Id,0)
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
    virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir);
    virtual void death_sfx();
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
    virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir);
};

class eGanon : public enemy
{
  public:
    int Stunclk;
    eGanon(fix X,fix Y,int Id,int Clk);                     // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir);
    virtual void draw(BITMAP *dest);
    void draw_guts(BITMAP *dest);
    void draw_flash(BITMAP *dest);
};

void getBigTri();

/**********************************/
/***  Multiple-Segment Enemies  ***/
/**********************************/

// segment manager
class eMoldorm : public enemy
{
  public:
    int segcnt;
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
    virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir);
    virtual void draw(BITMAP *dest);
    virtual void death_sfx();
};

// segment manager
class eLanmola : public enemy
{
  public:
    int segcnt;
    // tell it the segment count through the clk param
    eLanmola(fix X,fix Y,int Id,int Clk);                   // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
};

// segment class
class esLanmola : public enemy
{
  public:
    int crate;
    esLanmola(fix X,fix Y,int Id,int Clk);                  // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir);
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
    virtual void death_sfx();
    virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir);
    virtual void draw(BITMAP *dest);
};

class esManhandla : public enemy
{
  public:
    // use clk param to tell which arm it is
    esManhandla(fix X,fix Y,int Id,int Clk);                // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir);
    virtual void draw(BITMAP *dest);
};

class eGleeok : public enemy
{
  public:
    int flameclk, flamehead;
    int necktile;
    eGleeok(fix X,fix Y,int Id,int Clk);                    // : enemy((fix)120,(fix)48,Id,Clk)
    virtual bool animate(int index);
    virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir);
    virtual void death_sfx();
    virtual void draw(BITMAP *dest);
    virtual void draw2(BITMAP *dest);
};

// head class
class esGleeok : public enemy
{
  public:
    int nx[4],ny[4];                                        //neck x and y?
    int headtile;
    int flyingheadtile;
    int necktile;
    esGleeok(fix X,fix Y,int Id,int Clk);                   // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir);
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
    virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir);
    virtual void death_sfx();
};

// segment class
class esPatra : public enemy
{
  public:
    esPatra(fix X,fix Y,int Id,int Clk);                    // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir);
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
    virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir);
    virtual void death_sfx();
};

// segment class
class esPatraBS : public enemy
{
  public:
    esPatraBS(fix X,fix Y,int Id,int Clk);                  // : enemy(X,Y,Id,Clk)
    virtual bool animate(int index);
    virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir);
    virtual void draw(BITMAP *dest);
};

/**********************************/
/**********  Misc Code  ***********/
/**********************************/
void addEwpn(int x,int y,int id,int type,int power,int dir);
int enemy_dp(int index);
int ewpn_dp(int index);
int lwpn_dp(int index);
int hit_enemy(int index,int wpnId,int power,int wpnx,int wpny,int dir);
void enemy_scored(int index);
void addguy(int x,int y,int id,int clk,bool mainguy);
void additem(int x,int y,int id,int pickup);
void additem(int x,int y,int id,int pickup,int clk);
void kill_em_all();
// For Link's hit detection. Don't count them if they are stunned or are guys.
int GuyHit(int tx,int ty,int txsz,int tysz);
// For Link's hit detection. Count them if they are dying.
int GuyHit(int index,int tx,int ty,int txsz,int tysz);
bool hasMainGuy();
void EatLink(int index);
void GrabLink(int index);
bool CarryLink();
void movefairy(fix &x,fix &y,int misc);
void killfairy(int misc);
void addenemy(int x,int y,int id,int clk);
bool checkpos(int id);
bool isjumper(int id);
void addfires();
void loadguys();
void never_return(int index);
bool hasBoss();
bool slowguy(int id);
bool countguy(int id);
bool ok2add(int id);
void load_default_enemies();
void nsp();
int next_side_pos();
bool can_side_load(int id);
void side_load_enemies();
void loadenemies();
void moneysign();
void putprices(bool sign);
void setupscreen();
void putmsg();
void domoney();
void domagic();                                             //basically a copy of domoney()

/***  Collision detection & handling  ***/

void check_collisions();
void dragging_item();
void roaming_item();
#endif
/*** end of guys.cc ***/
