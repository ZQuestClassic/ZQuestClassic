#pragma once

#include "core/combo.h"
#include "core/handles.h"
#include "core/weapon_data.h"
#include "core/zdefs.h"
#include "base/zfix.h"
#include "core/flags.h"
#include "../ffc.h"
#include "sprite.h"

#include <map>
#include <set>
#include <tuple>

class enemy;
extern byte boomframe[16];
extern byte bszboomflip[4];

#define WEAPON_CLOCKS 10

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
    
	optional<word> _handle_loadsprite(optional<word> spr, bool isDummy = false, bool force = false);
	optional<word> _ewpn_sprite(int parentid) const;
	
	bool _prism_dupe(zfix newx, zfix newy, rpos_t cpos, ffc_id_t ffcpos, int tdir);
	bool _mirror_refl(zfix newx, zfix newy, rpos_t cpos, ffc_id_t ffcpos, newcombo const& mirror_cmb);
public:
	void load_weap_data(weapon_data const& data, optional<word>* out_wpnspr = nullptr);
    void setAngle(double angletoset);
    void doAutoRotate(bool dodir = false, bool doboth = false);
    int32_t power = 0, dead = -1, clk2 = 0, misc2 = 0;
    // The level (or "sub-type") of the weapon.
    // Each weapon type (sprite::id) uses this differently (and some not at all).
    // For weapons created by an item, it's the item level.
    // For fireballs, the first bit is the "boss fireball" flag.
    int32_t level = 0;
	rpos_t ignorecombo = rpos_t::None;
	ffc_id_t ignoreffc = -1;
	bool fake_weapon = false;
    bool isLit = false; //if true, this weapon is providing light to the current screen
    int32_t parentid = 0, //Enemy who created it
        parentitem = -1; //Item which created it
    int32_t dragging = -1;
    zfix step;
    bool bounce = false, ignoreHero = false;
    word flash = 0, wid = 0, aframe = 0, csclk = 0;
    int32_t o_speed = 0, o_type = 0, frames = 0, o_flip = 0, ref_o_tile = 0;
	byte script_wrote_otile = 0;
    int32_t temp1 = 0;
    bool autorotate = false;
	byte linkedItem = 0;
	byte unblockable = 0;
	
    weapon_flags misc_wflags = WFLAG_NONE;
	word misc_wsprites[WPNSPR_MAX] = {};
	byte light_rads[WPNSPR_MAX] = {};
	int32_t light_offsets[WPNSPR_MAX] = {};
	byte last_burnstate = 0;
	byte get_burnstate() const;
	
	byte wscreengrid[22] = {};
	byte wscreengrid_layer[6][22] = {};
	// (layer, rpos_t)
	std::set<std::pair<int, rpos_t>> rposes_checked;
	std::set<ffcdata*> ffcs_checked;
	// The triggers this weapon ran this frame, by where they are, their combo and their index,
	// and whether each fired. Each point of the hitbox finds a combo, and the weapon can look
	// for combos again later in the frame, so without this a trigger fires several times a
	// frame.
	//
	// animate() clears these and looks with findcombotriggers(), then can look again
	// in the same animate(): through collision_check() when gravity lands it with
	// WFLAG_BREAK_WHEN_LANDING or WFLAG_STOP_WHEN_LANDING, and with findcombotriggers() when
	// its move hits something solid with WFLAG_BREAK_ON_SOLID or WFLAG_STOP_WHEN_HIT_SOLID.
	//
	// A trigger that doesn't react to the weapon (say, a ground-only trigger while it was in the
	// air) isn't here, so a later look in the same frame can still set it off. One that ran but
	// didn't fire (a failed condition or Chance roll, or a combo type effect that couldn't
	// happen) is here as false, so it isn't run again that frame: that would roll its Chance
	// again and repeat its failure effects. A trigger on Cooldown counts as fired.
	std::map<std::tuple<int, rpos_t, int, int>, bool> rpos_triggers_this_frame;
	std::map<std::tuple<ffcdata*, int, int>, bool> ffc_triggers_this_frame;
	// The 'Once Per Weapon' triggers this weapon has fired, by where they are, their combo and
	// their index.
	std::set<std::tuple<int, rpos_t, int, int>> rpos_triggers_once;
	std::set<std::tuple<ffcdata*, int, int>> ffc_triggers_once;
	
	int16_t death_spawnitem = -1;
	int16_t death_spawndropset = -1;
	int32_t death_item_pflags = 0;
	int16_t death_sprite = -1;
	word death_sfx = 0;
	
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
    byte usedefense = 0; //default defence type -Z
    byte usedefensedummy = 0; //defence type for things like bombs that pass it into a real weapon.
    word linked_parent = 0;
    byte quantity_iterator = 0;
    int32_t weap_pattern[ITEM_MOVEMENT_PATTERNS] = {}; //formation, arg1, arg2 -Z Note: totally unused.
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
    
    bool isHeroWeapon();
    bool isHeroMelee();
    
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
	int16_t pierce_count = -1;
	zfix bounce_mult, bounce_add;

	int shd_aclk = 0, shd_aframe = 0;
    word specialsfx = 0;
	
	bool disable_triggers = false;
	
	void do_death_fx();
    void convertType(bool toLW);
    void setIsLWeapon(bool toLW);
    weapon(weapon const &other);
    //weapon(zfix X,zfix Y,zfix Z,int32_t Id,int32_t Type,int32_t pow,int32_t Dir, int32_t Parentid, int32_t prntid, bool isDummy=false);
    weapon(zfix X,zfix Y,zfix Z,int32_t Id,int32_t Type,int32_t pow,int32_t Dir, int32_t Parentid, int32_t prntid, bool isDummy=false, byte script_gen=0, byte isLW=0, byte special = 0, int32_t Linked_Parent = 0, int32_t use_sprite = -1, bool autoRotate = false);
    weapon(zfix X,zfix Y,zfix Z,int32_t Id,int32_t usesprite, int32_t Dir, int32_t step, int32_t prntid, int32_t height, int32_t width, int32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f, int32_t g);
    virtual ~weapon();
	bool do_mirror(); // returns true if animate needs to early-break
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
	bool no_collision() const;
    virtual bool blocked(int32_t xOffset, int32_t yOffset);
	bool hardcoded_sideview_hitbox() const;
	newcombo const* check_conveyor();
    void limited_animate();
    virtual void on_freeze();
    virtual bool animate(int32_t index);
	std::set<rpos_t> getBombPositions();
	bool collide_solid() const;
	void collision_check();
	void hit_pierce(enemy* e = NULL, int32_t ehitType = -1, bool old_hit = true);
    virtual void onhit(bool clipped, enemy* e = NULL, int32_t ehitType = -1);
    virtual void onhit(bool clipped, int32_t special, int32_t herodir, enemy* e = NULL, int32_t ehitType = -1, bool was_pierced = false);
	void kill_weapon_special(); // kill a weapon, with SOME special handling; i.e. blow up bombs
    // override hit detection to check for invicibility, etc
    virtual bool hit();
    virtual bool hit(sprite *s);
    virtual bool hit(int32_t tx,int32_t ty,int32_t tz,int32_t txsz,int32_t tysz,int32_t tzsz);
	virtual bool hit(int32_t tx,int32_t ty,int32_t txsz,int32_t tysz);
	void animate_graphics();
	virtual bool can_drawshadow() const;
    virtual void draw(BITMAP *dest);
    virtual void drawshadow(BITMAP *dest, bool translucent);
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
const weapon_data* get_weapon_data_for_id(int id, int parentitem, int parentid, bool isLWeapon);
struct weapon_size_t
{
	int xofs = 0, yofs = 0;
	int txsz = 1, tysz = 1;
	int hxofs = 0, hyofs = 0;
	int hit_width = 16, hit_height = 16;
	optional<int> hzsz;

	std::pair<rect_t, rect_t> to_rects(int x, int y) const;
};

bool compute_do_size(weapon_data const& data, int weap_id, bool lw);
weapon_size_t calculate_weapon_size(weapon_data const& data, int weap_id, int dir, bool lw);
