#ifndef WEAPONS_H_
#define WEAPONS_H_

#include "base/combo.h"
#include "base/handles.h"
#include "base/zdefs.h"
#include "base/zfix.h"
#include "base/flags.h"
#include "sprite.h"

#include <set>

class enemy;
extern byte boomframe[16];
extern byte bszboomflip[4];

#define WEAPON_CLOCKS 10
#define WFLAG_MAX 10
#define WFLAG_BURNFLAGS            0x0078

class weapon : public sprite
{
private:
    void seekHero();
    void seekEnemy(int32_t j);
    int32_t seekEnemy2(int32_t j);

// How close the weapon can get to the edge of the screen before
	// being deleted (or bouncing, in the case of boomerangs)
	int32_t minX, maxX, minY, maxY;
	friend void setScreenLimits(weapon&);
    
	optional<byte> _handle_loadsprite(optional<byte> spr, bool isDummy = false, bool force = false);
	optional<byte> _ewpn_sprite(int parentid) const;
public:
    void setAngle(double angletoset);
    void doAutoRotate(bool dodir = false, bool doboth = false);
    int32_t power,type,dead,clk2,misc2;
	rpos_t ignorecombo;
    bool isLit; //if true, this weapon is providing light to the current screen
    int32_t parentid, //Enemy who created it
        parentitem; //Item which created it
    int32_t dragging;
    zfix step;
    bool bounce, ignoreHero;
    word flash,wid,aframe,csclk;
    int32_t o_speed, o_type, frames, o_flip, ref_o_tile;
	byte script_wrote_otile;
    int32_t temp1;
    bool behind;
    bool autorotate;
	byte linkedItem;
	byte unblockable;
	
    weapon_flags misc_wflags;
	byte misc_wsprites[WPNSPR_MAX];
	byte light_rads[WPNSPR_MAX];
	byte last_burnstate;
	byte get_burnstate() const;
	
	byte wscreengrid[22];
	byte wscreengrid_layer[6][22];
	// (layer, rpos_t)
	std::set<std::pair<int, rpos_t>> rposes_checked;
	std::set<ffcdata*> ffcs_checked;
	
	int16_t death_spawnitem;
	int16_t death_spawndropset;
	int32_t death_item_pflags;
	int16_t death_sprite;
	byte death_sfx;
	
	bool has_shadow;
	
	byte lift_level;
	byte lift_time = 16;
	zfix lift_height = 8;
	
	//!DIMI: More variables? That suuuuuure won't break anything. Nope.
    int32_t count1; 
    int32_t count2;
    int32_t count3;
    int32_t count4;
    int32_t count5;
    void bookfirecreate();

    //Weapon Editor -Z
    byte useweapon; //lweapon id type -Z
    byte useweapondummy; //weapon id type for things like bombs that pass it into a real weapon.
    byte usedefense; //default defence type -Z
    byte usedefensedummy; //defence type for things like bombs that pass it into a real weapon.
    word linked_parent;
    byte quantity_iterator;
    int32_t weap_pattern[ITEM_MOVEMENT_PATTERNS]; //formation, arg1, arg2 -Z
    int32_t weaprange; //default range -Z
    int32_t weapduration; //default duration, 0 = infinite. 
    int32_t clocks[WEAPON_CLOCKS];
    int32_t tilemod;
    byte drawlayer;
    word family_class;												
    byte family_level;
    word flags;
    int32_t collectflags; //items that this weapon can collect on contact.
    int32_t ffmisc[FFSCRIPT_MISC];
    char weapname[128]; //Weapon Name, pulled from editor. 
     int32_t duplicates; //Number of duplicate weapons generated.
     int32_t wpn_misc_d[FFSCRIPT_MISC];
    //! End weapon editor. -Z
    
    bool isHeroWeapon();
    bool isHeroMelee();
    //2.6 ZScript -Z
    int32_t scriptrange,blastsfx;
    
    //2.6 enemy editor weapon sprite
    int32_t wpnsprite;
    
    //Used only by ffscript! No not make readable by scripts!
    byte ScriptGenerated; //Used to permit creating HeroClass weapons, or other weapon types that the engine does not control.
    byte isLWeapon;
	bool weapon_dying_frame; //a last_hurrah for weapons -Em
	bool weapon_start_frame = true; //a ... first hurrah. Used for running scripts earlier on the first frame.
	bool rundeath; //run death effects
	int weap_timeout;
    byte specialinfo;
	
	int shd_aclk, shd_aframe;
    byte specialsfx;
	
	
	void do_death_fx();
    void convertType(bool toLW);
    weapon(weapon const &other);
    //weapon(zfix X,zfix Y,zfix Z,int32_t Id,int32_t Type,int32_t pow,int32_t Dir, int32_t Parentid, int32_t prntid, bool isDummy=false);
    weapon(zfix X,zfix Y,zfix Z,int32_t Id,int32_t Type,int32_t pow,int32_t Dir, int32_t Parentid, int32_t prntid, bool isDummy=false, byte script_gen=0, byte isLW=0, byte special = 0, int32_t Linked_Parent = 0, int32_t use_sprite = -1);
    weapon(zfix X,zfix Y,zfix Z,int32_t Id,int32_t usesprite, int32_t Dir, int32_t step, int32_t prntid, int32_t height, int32_t width, int32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f, int32_t g);
    virtual ~weapon();
    void eweapon_overrides();
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
	std::set<rpos_t> getBombPositions();
	void collision_check();
    virtual void onhit(bool clipped, enemy* e = NULL, int32_t ehitType = -1);
    virtual void onhit(bool clipped, int32_t special, int32_t herodir, enemy* e = NULL, int32_t ehitType = -1);
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

int FindComboTriggerMatch(weapon *w, int combo_id, int start_idx = 0);
void killgenwpn(weapon* w);
void do_generic_combo(const rpos_handle_t& rpos_handle, weapon *w, int32_t wid, 
	int32_t cid, int32_t flag, int32_t flag2, int32_t ft, bool single16);
void do_generic_combo_ffc(weapon *w, const ffc_handle_t& ffc_handle, int32_t cid, int32_t ft);
void putweapon(BITMAP *dest,int32_t x,int32_t y,int32_t weapon_id, int32_t type, int32_t dir, int32_t &aclk, int32_t &aframe,
               int32_t parentid);
	       
#endif
