//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  link.cc
//
//  Link's class: LinkClass
//  Handles a lot of game play stuff as well as Link's
//  movement, attacking, etc.
//
//--------------------------------------------------------

#ifndef _LINK_H_
#define _LINK_H_

#include "zc_alleg.h"
#include "zcmusic.h"
#include "zdefs.h"
#include "zelda.h"
#include "maps.h"
#include "tiles.h"
#include "colors.h"
#include "pal.h"
#include "aglogo.h"
#include "zsys.h"
#include "qst.h"
#include "matrix.h"
#include "jwin.h"
#include "jwinfsel.h"
#include "weapons.h"
//#include "save_gif.h"
#include "sprite.h"
#include "zc_custom.h"
#include "subscr.h"

extern movingblock mblock2;                                 //mblock[4]?
extern sprite_list  guys, items, Ewpns, Lwpns, Sitems, chainlinks, decorations;

extern const byte lsteps[8];

enum { did_fairy=1, did_candle=2, did_whistle=4, did_magic=8, did_glove=16, did_all=32 };

enum actiontype
{
    none, walking, attacking, freeze, landhold1, landhold2,
    rafting, gothit, inwind, scrolling, won, swimming, hopping,
    swimhit, waterhold1, waterhold2, casting, climbcovertop,
    climbcoverbottom, dying, drowning, 
	climbing, //not used -Z.
    // Fake actiontypes: used by ZScripts
    ischarging, isspinning, isdiving, gameover, hookshotout, stunned, ispushing
};

typedef struct tilesequence
{
    word tile;
    byte frames;                                              // animation frame count
    byte speed;                                               // animation speed
    byte exp;                                                 // not used
} tilesequence;

#define SWORDCHARGEFRAME 20
#define SWORDTAPFRAME SWORDCHARGEFRAME-8
#define HAMMERCHARGEFRAME 11
#define WANDCHARGEFRAME 12

// Values for specialcave
#define ITEMCELLAR 1
#define PASSAGEWAY 2
#define GUYCAVE 3
#define STAIRCAVE 4

class LinkClass : public sprite
{
    class WalkflagInfo
    {
    public:
        WalkflagInfo() : flags(0), newhopclk(0), newhopdir(0), newdir(0), newladderdir(0),
            newladderx(0), newladdery(0), newladderstart(0) {}
        static const int UNWALKABLE = 1;
        static const int SETILSWIM = 2;
        static const int CLEARILSWIM = 4;
        static const int SETHOPCLK = 8;
        static const int SETDIR = 16;
        static const int CLEARCHARGEATTACK = 32;
        static const int SETHOPDIR = 64;
        
        int getHopClk()
        {
            return newhopclk;
        }
        int getHopDir()
        {
            return newhopdir;
        }
        int getDir()
        {
            return newdir;
        }
        int getLadderdir()
        {
            return newladderdir;
        }
        int getLadderx()
        {
            return newladderx;
        }
        int getLaddery()
        {
            return newladdery;
        }
        int getLadderstart()
        {
            return newladderstart;
        }
        
        void setUnwalkable(bool val)
        {
            if(val) flags |= UNWALKABLE;
            else flags &= ~UNWALKABLE;
        }
        
        void setIlswim(bool val)
        {
            if(val) flags |= SETILSWIM;
            else flags |= CLEARILSWIM;
        }
        
        void setHopClk(int val)
        {
            flags |= SETHOPCLK;
            newhopclk = val;
        }
        
        void setHopDir(int val)
        {
            flags |= SETHOPDIR;
            newhopdir = val;
        }
        
        void setDir(int val)
        {
            flags |= SETDIR;
            newdir = val;
        }
        
        void setChargeAttack()
        {
            flags |= CLEARCHARGEATTACK;
        }
        
        int getFlags()
        {
            return flags;
        }
        
        bool isUnwalkable()
        {
            return (flags & UNWALKABLE) != 0;
        }
        
        WalkflagInfo operator||(WalkflagInfo other);
        WalkflagInfo operator&&(WalkflagInfo other);
        WalkflagInfo operator!();
        
    private:
        int flags;
        int newhopclk;
        int newhopdir;
        int newdir;
        int newladderdir;
        int newladderx;
        int newladdery;
        int newladderstart;
    };
    
    void execute(WalkflagInfo info);
    
    bool autostep,superman,inwallm,tapping,stomping,last_hurrah;
    int refilling,
        ladderx,
        laddery,
        stepnext,  //location of step->next just triggered (don't recursively trigger it)
        stepsecret,  //location of step->secrets just triggered (don't recursively trigger it)
        warpx,warpy, //location of warp just arrived at (don't recursively enter it)
        pushing,  //incremental time spent pushing.
        jumping,  //incremental time spent airborne.
        charging, //incremental time spent charging weapon.
        spins,    //1 + number of sword spins left to do. 0 = not spinning.
        fairyclk, //fairy circle timeout.
        refillclk,//life refill timeout.
        drunkclk, //intoxication timeout.
        NayrusLoveShieldClk, // Nayru's Love timeout.
        hoverclk, //hover boots timeout.
        hclk,     //damage blinking timeout.
        holdclk,  //"hold up item" timeout.
        holditem, //item being held.
        attackclk,//attack timeout.
        attack,   //current attack wpnid.
        attackid, //current attack itemid.
        swordclk, //sword jinx timeout.
        itemclk,  //item jinx timeout.
        didstuff, //played the whistle? used the blue candle?
        blowcnt,  //number of times whistle blown on this screen.
        drownclk, //drowning timeout.
        stepoutindex, // where to step out when in a passageway
        stepoutwr, // which warp return to use for a passageway
        stepoutdmap, // which dmap the passageway exits to
        stepoutscr, // which screen the passageway exits to
        slashxofs, slashyofs; // used by positionSword() and draw()
	byte skipstep,lstep,
	hopclk, // hopping into water timeout.
	diveclk, // diving timeout.
	whirlwind, // is Link inside an arriving whirlwind? (yes = 255)
	specialcave, // is Link inside a special cave?
	hitdir, // direction from which damage was taken.
	ladderdir, // direction of ladder
	lastdir[4], // used in Maze Path screens
	ladderstart, // starting direction of ladder...?
	inlikelike, // 1 = Like Like. 2 = Taking damage while trapped
	link_is_stunned; //scripted stun from weapons; possibly for later eweapon effects in the future. 
    int shiftdir; // shift direction when walking into corners of solid combos
    int sdir; // scrolling direction
    int hopdir;  // direction to hop out of water (diagonal movement only)
    int holddir;
    int landswim; // incremental time spent swimming against land
    bool ilswim; // is land swimming?
    bool walkable;
    actiontype action, tempaction; // current action, cached action.
    int hshandle_id, hshead_id;
    byte conveyor_flags;
    fix climb_cover_x, climb_cover_y;
    fix entry_x, entry_y; // When drowning, re-create Link here
    fix falling_oldy; // Used by the Stomp Boots in sideview
    byte dontdraw;
    bool diagonalMovement;
    bool bigHitbox;
    byte defence[wMax];
    
    
     bool flickerorflash, preventsubscreenfalling; // Enable invincibility effects, disable dropping the subscreen.
    int hurtsfx; //Link's Hurt SOund
    int walkspeed; //Link's walking speed.
    int lastHitBy[NUM_HIT_TYPES_USED][2]; //[enemy, eweapon, combo, flag
	
	int last_lens_id;// The item ID of the last Lens of Truth type item used
    
    // Methods below here.
    void movelink();
    void move(int d);
    void hitlink(int hit);
    int  nextcombo(int cx,int cy,int cdir);
    int  nextflag(int cx,int cy,int cdir, bool comboflag);
    bool nextcombo_wf(int d);
    bool nextcombo_solid(int d);
    int  lookahead(int d);
    int  lookaheadflag(int d);
    void checkhit();
    bool checkdamagecombos(int dx, int dy);
    bool checkdamagecombos(int dx1, int dx2, int dy1, int dy2, int layer = -1, bool solid = false);
    void checkscroll();
    void checkspecial();
    void checkspecial2(int *ls);
    void checkspecial3();
    void checkpushblock();
    void checkbosslockblock();
    void checklockblock();
    void checkswordtap();
    void checkchest(int type);
    void checktouchblk();
    void checklocked();
    void deselectbombs(int super); // switch Link's weapon if his current weapon (bombs) was depleted.
    bool startwpn(int itemid);
    bool doattack();
    bool can_attack();
    void do_rafting();
    void do_hopping();
    WalkflagInfo walkflag(int wx,int wy,int cnt,byte d);
    WalkflagInfo walkflagMBlock(int wx,int wy);
    bool edge_of_dmap(int side);
    bool checkmaze(mapscr *scr, bool sound);
    bool maze_enabled_sizewarp(int scrolldir);
    
    int get_scroll_step(int scrolldir);
    int get_scroll_delay(int scrolldir);
public:
    void run_scrolling_script(int scrolldir, int cx, int sx, int sy, bool end_frames);
private:
    void scrollscr(int dir,int destscr = -1, int destdmap = -1);
    void walkdown(bool opening);
    void walkup(bool opening);
    void walkdown2(bool opening);
    void walkup2(bool opening);
    void exitcave();
    void stepout();
    void masked_draw(BITMAP *dest);
    void getTriforce(int id);
    int weaponattackpower();
    void positionSword(weapon* w,int itemid);
    void checkstab();
    void fairycircle(int type);
    void StartRefill(int refillWhat);
    int  EwpnHit();
    int  LwpnHit();
    void gameover();
    void ganon_intro();
    void saved_Zelda();
    void check_slash_block(int bx, int by);
    void check_wand_block(int bx, int by);
    void check_pound_block(int bx, int by);
    void check_conveyor();
    bool sideviewhammerpound();
    bool agonyflag(int flag);
    int ringpower(int dmg);
    void addsparkle(int wpn);
    void addsparkle2(int type1, int type2);
    
public:

    void checkitems(int index = -1);
    int DrunkClock();
    void setDrunkClock(int newdrunkclk);
    int StunClock();
    void setStunClock(int v);
    void setEntryPoints(int x, int y);
    LinkClass();
    void init();
    virtual void drawshadow(BITMAP* dest, bool translucent);
    virtual void draw(BITMAP* dest);
    virtual bool animate(int index);
    bool dowarp(int type, int index);
    
    void linkstep();
    void stepforward(int steps, bool adjust);
    void draw_under(BITMAP* dest);
    
    // called by ALLOFF()
    void resetflags(bool all);
    void Freeze();
    void unfreeze();
    void finishedmsg();
    void Drown();
    int getEaten();
    void setEaten(int i);
    fix  getX();
    fix  getY();
    fix  getZ();
    fix  getFall();
    fix  getXOfs();
    fix  getYOfs();
    void setXOfs(int newxofs);
    void setYOfs(int newyofs);
    int  getHXOfs();
    int  getHYOfs();
    int  getHXSz();
    int  getHYSz();
    fix  getClimbCoverX();
    fix  getClimbCoverY();
    int  getLadderX();
    int  getLadderY();
    void setX(int new_x);
    void setY(int new_y);
    void setZ(int new_Z);
    void setFall(fix new_fall);
    void setClimbCoverX(int new_x);
    void setClimbCoverY(int new_y);
    int  getLStep();
    int  getCharging();
    void setCharging(int new_charging);
    bool isCharged();
    int  getAttackClk();
    void  setAttackClk(int new_clk);
    int  getSwordClk();
    int  getItemClk();
    void  setSwordClk(int newclk);
    void  setItemClk(int newclk);
    fix  getModifiedX();
    fix  getModifiedY();
    int  getDir();
    void setDir(int new_dir);
    int  getHitDir();
    void setHitDir(int new_dir);
    int  getClk();
    int  getPushing();
    void reset_swordcharge();
    void reset_hookshot();
    bool can_deploy_ladder();
    void reset_ladder();
    bool refill();
    void Catch();
    bool getClock();
    void setClock(bool state);
    int  getAction();
    int  getAction2(); //used by the new ffcore.actions
    void setAction(actiontype new_action);
    int getHeldItem();
    void setHeldItem(int newitem);
    bool isDiving();
    bool isSwimming();
    void setDontDraw(byte new_dontdraw);
    byte getDontDraw();
    void setHClk(int newhclk);
    int getHClk();
    void setNayrusLoveShieldClk(int newclk);
    int getNayrusLoveShieldClk();
    int getHoverClk();
    int getHoldClk();
    int getSpecialCave(); // used only by maps.cpp
    bool ffwarp;
    bool ffpit;
    void setscriptnohit(bool);
    bool getscriptnohit();
    
     bool getCanLinkFlicker(); //enable or disable flicker or flash
    void setCanLinkFlicker(bool v);
    
    void sethitLinkUID(int type, int screen_index);
    void ClearhitLinkUIDs();
    void set_defence(int def, int v);
    int get_defence(int def);
    int gethitLinkUID(int type);
    
    void setHurtSFX(int sfx); //Set Link;s hurt sfx
    int getHurtSFX();
    
      //Prevent the subscreen from falling by script.
    bool stopSubscreenFalling();
    void stopSubscreenFalling(bool v);
    
    //Set the button items by brute force
    void setAButtonItem(int subscreenslot);
    void setBButtonItem(int subscreenslot);
    
    bool getDiagMove(); //Diagonal movement.
    void setDiagMove(bool newdiag);
    bool getBigHitbox(); //Large H-itbox
    void setBigHitbox(bool newbighitbox);
	
	
	int getLastLensID();	
	void setLastLensID(int p_item);
};

bool isRaftFlag(int flag);
void do_lens();
void do_210_lens();
int touchcombo(int x,int y);
extern bool did_secret;
int selectWlevel(int d);
void computeMaxArrows();

/************************************/
/********  More Items Code  *********/
/************************************/

int enemy_dp(int index);
int ewpn_dp(int index);
int lwpn_dp(int index);
bool checkmagiccost(int itemid);
void paymagiccost(int itemid);
int Bweapon(int pos);
void stopCaneOfByrna();
//void selectWpn(int xstep, int ystep, bool b);
const int SEL_UP = 0;
const int SEL_LEFT = 1;
const int SEL_DOWN = 2;
const int SEL_RIGHT = 3;
const int SEL_VERIFY_LEFT = 4;
const int SEL_VERIFY_RIGHT = 5;
int selectWpn_new(int type, int startpos, int forbiddenpos = -1);
bool isWpnPressed(int wpn);
int getWpnPressed(int wpn);
int selectSword();
void selectNextAWpn(int type);
void selectNextBWpn(int type);
void verifyBothWeapons();
void verifyAWpn();
void verifyBWpn();
bool canget(int id);
void dospecialmoney(int index);
void getitem(int id, bool nosound=false);
void takeitem(int id);
void red_shift();
void slide_in_color(int color);
#endif
/*** end of link.cc ***/
