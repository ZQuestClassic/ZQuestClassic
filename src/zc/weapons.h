#ifndef _WEAPONS_H_
#define _WEAPONS_H_

#include "base/zdefs.h"
#include "sprite.h"
#include "base/zfix.h"
#include <set>
#include <tuple>

/**************************************/
/***********  Weapon Class  ***********/
/**************************************/

class enemy;
extern byte boomframe[16];
extern byte bszboomflip[4];

#define WEAPON_CLOCKS 10

#define WPNUNB_NONE      0x00
#define WPNUNB_BLOCK     0x01
#define WPNUNB_IGNR      0x02
#define WPNUNB_SHLD      0x04
#define WPNUNB_REFL      0x08
#define WPNUNB_ALL       0x0F

#define WFLAG_PICKUP_ITEMS         0x0001
#define WFLAG_BREAK_WHEN_LANDING   0x0002
#define WFLAG_BREAK_ON_SOLID       0x0004
#define WFLAG_BURN_ANYFIRE         0x0008
#define WFLAG_BURN_STRONGFIRE      0x0010
#define WFLAG_BURN_MAGICFIRE       0x0020
#define WFLAG_BURN_DIVINEFIRE      0x0040
#define WFLAG_STOP_WHEN_LANDING    0x0080
#define WFLAG_STOP_WHEN_HIT_SOLID  0x0100
#define WFLAG_UPDATE_IGNITE_SPRITE 0x0200

#define WFLAG_MAX 10

#define WFLAG_BURNFLAGS            0x0078

enum
{
	WPNSPR_BASE,
	WPNSPR_IGNITE_ANY,
	WPNSPR_IGNITE_STRONG,
	WPNSPR_IGNITE_MAGIC,
	WPNSPR_IGNITE_DIVINE,
	WPNSPR_MAX
};

class weapon : public sprite
{
private:
    void seekHero();
    void seekEnemy(int32_t j);
    int32_t seekEnemy2(int32_t j);

// How close the weapon can get to the edge of the screen before
	// being deleted (or bouncing, in the case of boomerangs)
	int32_t minX = 0, maxX = 0, minY = 0, maxY = 0;
	friend void setScreenLimits(weapon&);
    
	optional<byte> _handle_loadsprite(optional<byte> spr, bool isDummy = false, bool force = false);
	optional<byte> _ewpn_sprite(int parentid) const;
public:
    void setAngle(double angletoset);
    void doAutoRotate(bool dodir = false, bool doboth = false);
    int32_t power = 0, type = 0, dead = -1, clk2 = 0, misc2 = 0, ignorecombo = -1;
    bool isLit = false; //if true, this weapon is providing light to the current screen
    int32_t parentid = 0, //Enemy who created it
        parentitem = -1; //Item which created it
    int32_t dragging = -1;
    zfix step;
    bool bounce = false, ignoreHero = false;
    word flash = 0, wid = 0, aframe = 0, csclk = 0;
    int32_t o_tile = 0, o_cset = 0, o_speed = 0, o_type = 0, frames = 0, o_flip = 0, ref_o_tile = 0;
	byte script_wrote_otile = 0;
    int32_t temp1 = 0;
    bool behind = false;
    bool autorotate = false;
	byte linkedItem = 0;
	byte unblockable = 0;
	
	word misc_wflags = 0;
	byte misc_wsprites[WPNSPR_MAX] = {};
	byte light_rads[WPNSPR_MAX] = {};
	byte last_burnstate = 0;
	byte get_burnstate() const;
	
	byte wscreengrid[22] = {};
	byte wscreengrid_layer[6][22] = {};
	byte wscreengrid_ffc[MAXFFCS/8] = {};
	// The combos this weapon set off this frame. Each point of the hitbox finds a combo, and
	// the weapon can look for combos again later in the frame, so without this a combo
	// triggers several times a frame.
	//
	// animate() clears these and looks with findcombotriggers(), then can look again
	// in the same animate(): through collision_check() when gravity lands it with
	// WFLAG_BREAK_WHEN_LANDING, and with findcombotriggers() when its move hits something
	// solid with WFLAG_BREAK_ON_SOLID or WFLAG_STOP_WHEN_HIT_SOLID.
	//
	// A combo the weapon didn't set off (say, a ground-only one while it was in the air)
	// isn't here, so a later look in the same frame can still set it off.
	// (layer, pos, combo)
	std::set<std::tuple<int, int, int>> combos_triggered_this_frame;
	// (ffc, combo)
	std::set<std::pair<int, int>> ffcs_triggered_this_frame;
	// The 'Once Per Weapon' combos this weapon has fired.
	// (layer, pos, combo)
	std::set<std::tuple<int, int, int>> combos_triggered_once;
	// (ffc, combo)
	std::set<std::pair<int, int>> ffcs_triggered_once;

	int16_t death_spawnitem = -1;
	int16_t death_spawndropset = -1;
	int32_t death_item_pflags = 0;
	int16_t death_sprite = -1;
	byte death_sfx = 0;
	
	bool has_shadow = true;
	
	byte lift_level = 0;
	byte lift_time = 16;
	zfix lift_height = 8;
	
	//!DIMI: More variables? That suuuuuure won't break anything. Nope.
    int32_t count1 = 0; 
    int32_t count2 = 0;
    int32_t count3 = 0;
    int32_t count4 = 0;
    int32_t count5 = 0;
    void bookfirecreate();

    //Weapon Editor -Z
    byte useweapon = 0; //lweapon id type -Z
    byte useweapondummy = 0; //weapon id type for things like bombs that pass it into a real weapon.
    byte usedefence = 0; //default defence type -Z
    byte usedefencedummy = 0; //defence type for things like bombs that pass it into a real weapon.
    word linked_parent = 0;
    byte quantity_iterator = 0;
    int32_t weap_pattern[ITEM_MOVEMENT_PATTERNS] = {}; //formation, arg1, arg2 -Z
    int32_t weaprange = 0; //default range -Z
    int32_t weapduration = 0; //default duration, 0 = infinite. 
    int32_t clocks[WEAPON_CLOCKS] = {};
    int32_t tilemod = 0;
    byte drawlayer = 0;
    word family_class = 0;												
    byte family_level = 0;
    word flags = 0;
    int32_t collectflags = 0; //items that this weapon can collect on contact.
    int32_t ffmisc[FFSCRIPT_MISC] = {};
    char weapname[128] = {}; //Weapon Name, pulled from editor. 
     int32_t duplicates = 0; //Number of duplicate weapons generated.
     int32_t wpn_misc_d[FFSCRIPT_MISC] = {};
    //! End weapon editor. -Z
    
    int32_t script_UID = 0;
    int32_t parent_script_UID = 0;
    int32_t getScriptUID();
    int32_t getParentScriptUID();
    void setScriptUID(int32_t new_id);
    void setParentScriptUID(int32_t new_id);
    bool isHeroWeapon();
    bool isHeroMelee();
    //2.6 ZScript -Z
    int32_t scriptrange = 0, blastsfx = 0;
    
    //2.6 enemy editor weapon sprite
    int32_t wpnsprite = 0;
    
    //Used only by ffscript! No not make readable by scripts!
    byte ScriptGenerated = 0; //Used to permit creating HeroClass weapons, or other weapon types that the engine does not control.
    byte isLWeapon = 0;
	bool weapon_dying_frame = false; //a last_hurrah for weapons -Em
	bool weapon_start_frame = true; //a ... first hurrah. Used for running scripts earlier on the first frame.
	bool rundeath = false; //run death effects
	int weap_timeout = 0;
    byte specialinfo = 0;
	
	int shd_aclk = 0, shd_aframe = 0;
	
	
	
	void do_death_fx();
    void convertType(bool toLW);
    void setIsLWeapon(bool toLW);
    weapon(weapon const &other);
    //weapon(zfix X,zfix Y,zfix Z,int32_t Id,int32_t Type,int32_t pow,int32_t Dir, int32_t Parentid, int32_t prntid, bool isDummy=false);
    weapon(zfix X,zfix Y,zfix Z,int32_t Id,int32_t Type,int32_t pow,int32_t Dir, int32_t Parentid, int32_t prntid, bool isDummy=false, byte script_gen=0, byte isLW=0, byte special = 0, int32_t Linked_Parent = 0, int32_t use_sprite = -1);
    weapon(zfix X,zfix Y,zfix Z,int32_t Id,int32_t usesprite, int32_t Dir, int32_t step, int32_t prntid, int32_t height, int32_t width, int32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f, int32_t g);
    virtual ~weapon();
	void cleanup_sfx();
	void reset_wgrids();
    void LOADGFX(int32_t wpn);
    void LOADGFX_CMB(int32_t cid, int32_t cset);
    void findcombotriggers();
    bool Dead();
    bool isScriptGenerated();
    bool clip();
    bool blocked();
	bool no_triggers() const;
    virtual bool blocked(int32_t xOffset, int32_t yOffset);
    void limited_animate();
    virtual bool animate(int32_t index);
	void getBombPoses(std::set<int>& poses);
	void collision_check();
    virtual void onhit(bool clipped, enemy* e = NULL, int32_t ehitType = -1);
    virtual void onhit(bool clipped, int32_t special, int32_t herodir, enemy* e = NULL, int32_t ehitType = -1);
	void kill_weapon_special(); // kill a weapon, with SOME special handling; i.e. blow up bombs
    // override hit detection to check for invicibility, etc
    virtual bool hit();
    virtual bool hit(sprite *s);
    virtual bool hit(int32_t tx,int32_t ty,int32_t tz,int32_t txsz,int32_t tysz,int32_t tzsz);
	virtual bool hit(int32_t tx,int32_t ty,int32_t txsz,int32_t tysz);
    virtual void draw(BITMAP *dest);
    virtual void update_weapon_frame(int32_t change, int32_t orig);
	virtual int32_t run_script(int32_t mode);
	virtual ALLEGRO_COLOR hitboxColor(byte opacity = 255) const;
	virtual void draw_hitbox();
	virtual optional<ScriptType> get_scrtype() const {return isLWeapon ? ScriptType::Lwpn : ScriptType::Ewpn;}
};

bool MatchComboTrigger(weapon *w, int32_t comboid);
void killgenwpn(weapon* w);
void do_generic_combo(weapon *w, int32_t bx, int32_t by, int32_t wid, 
	int32_t cid, int32_t flag, int32_t flag2, int32_t ft, int32_t scombo, bool single16, int32_t layer);
void do_generic_combo_ffc(weapon *w, int32_t ffcpos, int32_t cid, int32_t ft);
void putweapon(BITMAP *dest,int32_t x,int32_t y,int32_t weapon_id, int32_t type, int32_t dir, int32_t &aclk, int32_t &aframe,
               int32_t parentid);
	       
#endif
