#ifndef CORE_COMBO_H_
#define CORE_COMBO_H_

#include "base/headers.h"
#include "base/general.h"
#include "base/containers.h"
#include "core/flags.h"
#include "core/weapon_data.h"
#include <functional>

namespace flags {
enum lift_flags : uint8_t
{
	LF_LIFTABLE             = F(0),
	LF_DROPSET              = F(1),
	LF_DROPONLIFT           = F(2),
	LF_SPECIALITEM          = F(3),
	LF_NOUCSET              = F(4),
	LF_NOWPNCMBCSET         = F(5),
	LF_BREAKONSOLID         = F(6),
	LF_LARGE_LIFTABLE       = F(7),
};
} // ends namespace

#define MAX_COMBO_TRIGGERS              255
///user flags
#define cflag1                          0x00001
#define cflag2                          0x00002
#define cflag3                          0x00004
#define cflag4                          0x00008
#define cflag5                          0x00010
#define cflag6                          0x00020
#define cflag7                          0x00040
#define cflag8                          0x00080
#define cflag9                          0x00100
#define cflag10                         0x00200
#define cflag11                         0x00400
#define cflag12                         0x00800
#define cflag13                         0x01000
#define cflag14                         0x02000
#define cflag15                         0x04000
#define cflag16                         0x08000

enum
{
	CMB_ADVP_TILE,
	CMB_ADVP_CSET2,
	CMB_ADVP_SOLIDITY,
	CMB_ADVP_ANIM,
	CMB_ADVP_TYPE,
	CMB_ADVP_INHFLAG,
	CMB_ADVP_ATTRIBUTE,
	CMB_ADVP_FLAGS,
	CMB_ADVP_GENFLAGS,
	CMB_ADVP_LABEL,
	CMB_ADVP_SCRIPT,
	CMB_ADVP_EFFECT,
	CMB_ADVP_TRIGGERS,
	CMB_ADVP_LIFTING,
	CMB_ADVP_GEN_MOVESPEED,
	CMB_ADVP_GEN_SFX,
	CMB_ADVP_GEN_SPRITES,
	CMB_ADVP_GEN_ZHEIGHT,
	CMB_ADVP_GEN_LARGECOMBO,
	CMB_ADVP_MISC_WEAP_DATA,
	CMB_ADVP_SZ
};

struct newcombo;
struct combo_trigger
{
	string label;
	bitstring trigger_flags;
	int32_t triggerlevel;
	byte triggerbtn;
	word triggeritem;
	word trigtimer;
	word trigsfx;
	int32_t trigchange;
	word trigprox;
	int16_t trigctr;
	int32_t trigctramnt;
	byte triglbeam;
	int8_t trigcschange;
	int32_t spawnitem;
	int16_t spawnenemy;
	int8_t exstate = -1, exdoor_dir = -1, exdoor_ind;
	byte combopos_state;
	int32_t spawnip;
	byte trigcopycat;
	byte trigcooldown;
	byte trig_lstate, trig_gstate;
	int32_t trig_statetime;
	word trig_genscr;
	byte trig_group;
	word trig_group_val;
	word trig_levelitems;
	dword req_level_state, unreq_level_state;
	bitstring req_global_state, unreq_global_state, req_screen_state,
		unreq_screen_state, req_screen_ex_state, unreq_screen_ex_state;
	std::array<bitstring, 4> req_screen_ex_door, unreq_screen_ex_door;
	int16_t trigdmlevel = -1;
	byte trigstatemap, trigstatescreen;
	std::array<int16_t, 3> trigtint{}; //r,g,b range [-255,255]
	int16_t triglvlpalette = -1, trigbosspalette = -1;
	int16_t trigquaketime = -1, trigwavytime = -1;
	int16_t trig_swjinxtime = -2, trig_itmjinxtime = -2, trig_shieldjinxtime = -2, trig_stuntime = -2, trig_bunnytime = -2;
	byte trig_pushtime = 8;
	word prompt_cid, fail_prompt_cid;
	byte prompt_cs, fail_prompt_cs;
	int16_t prompt_x = 12;
	int16_t prompt_y = -8;
	int32_t trig_msgstr, fail_msgstr;
	zfix req_player_x, req_player_y, req_player_z, req_player_jump;
	zfix dest_player_x, dest_player_y, dest_player_z, player_bounce;
	byte req_player_dir;
	int8_t dest_player_dir = -1;
	
	int32_t force_ice_combo = -1;
	zfix force_ice_vx, force_ice_vy;
	
	zfix trig_gravity, trig_terminal_v;
	
	int32_t play_music = -2;
	int8_t set_music_refresh = -1;
	
	int16_t viewport_cond_range;
	int32_t chance_numerator = 1, chance_denominator = 1;
	
	int16_t sfx_pan = -1;
	byte sfx_volume = 100;
	int32_t sfx_frequency = -1;
	
	byte large_combo_copycat;
	
	std::string summarize(newcombo const& cmb) const;
	bool is_blank() const;
	void clear();
	bool operator==(combo_trigger const& other) const = default;
	
    friend void to_json(json& j, const combo_trigger& trig);
    friend void from_json(const json& j, combo_trigger& trig);
};

struct newcombo
{
	int32_t tile;
	byte flip;
	byte walk = 0xF0;
	byte type;
	byte csets;
	byte frames;
	byte speed;
	word nextcombo;
	byte nextcset;
	byte flag;
	byte skipanim;
	word nexttimer;
	byte skipanimy;
	byte animflags;
	zfix c_attributes[NUM_COMBO_ATTRIBUTES]; // combodata->Attributes[] and Screen->GetComboAttribute(pos, indx) / SetComboAttribute(pos, indx)
	uint32_t usrflags; // combodata->Flags
	uint16_t genflags; // general flags
	lift_flags liftflags;
	byte liftlvl;
	word liftsfx;
	byte liftitm;
	byte liftgfx;
	word liftcmb, liftundercmb;
	byte liftcs, liftundercs;
	word liftsprite;
	byte liftdmg;
	int16_t liftbreaksprite = -1;
	byte liftbreaksfx;
	byte lifthei = 8;
	byte lifttime = 16;
	word lift_parent_item;
	// TODO: reduce size.
	weapon_data lift_weap_data {weapon_data::def_lifting_data};
	weapon_data misc_weap_data {weapon_data::def_data};
	std::string label;
	script_config scrconfig;
	int32_t o_tile;
	byte cur_frame;
	byte aclk;
	byte speed_mult = 1;
	byte speed_div = 1;
	zfix speed_add;
	word sfx_appear, sfx_disappear, sfx_loop, sfx_walking, sfx_standing, sfx_tap, sfx_landing, sfx_falling, sfx_drowning, sfx_lava_drowning;
	word spr_appear, spr_disappear, spr_walking, spr_standing, spr_falling, spr_drowning, spr_lava_drowning;
	
	vector<combo_trigger> triggers;
	byte only_gentrig;
	
	zfix z_height, z_step_height;
	byte dive_under_level;
	
	byte large_combo_dirs;
	
	void set_tile(int32_t newtile);
	void clear();
	bool is_blank(bool ignoreEff = false) const;
	bool can_cycle() const;
	
	int each_tile(std::function<bool(int32_t)> proc) const;
	
	void advpaste(newcombo const& other, bitstring const& pasteflags);
};

// Define "view" structs to help access combo's c_attributes. Usage:
//     ComboView_CutsceneEffect_Camera cv{cmb};
//     zfix& speed = cv.speed()

// Macro helpers.

#define MAKE_VIEW_ATTR(idx, name) \
	zfix& name() \
	{ \
		return attr[idx]; \
	} \
	const zfix& name() const \
	{ \
		return attr[idx]; \
	}

#define DEFINE_VIEW(ViewName, AttrList) \
	struct ComboView_##ViewName \
	{ \
		zfix* attr; \
		ComboView_##ViewName(newcombo& cmb) : attr(cmb.c_attributes) \
		{ \
		} \
		ComboView_##ViewName(const newcombo& cmb) : attr(const_cast<zfix*>(cmb.c_attributes)) \
		{ \
		} \
		AttrList(MAKE_VIEW_ATTR) \
	};

// (index, name) macros.

#define CAMERA_ATTRS(X) \
	X(0, speed) \
	X(1, idle_time) \
	X(2, interpolation_mode) \
	X(3, sub_type) \
	X(4, dest_x) \
	X(5, dest_y) \
	X(6, causes_mode)

#define PLAYER_WALK_ATTRS(X) \
	X(0, dest_x) \
	X(1, dest_y) \
	X(2, move_speed)

// Generate the structs.
DEFINE_VIEW(CutsceneEffect_Camera, CAMERA_ATTRS)
DEFINE_VIEW(CutsceneEffect_PlayerWalk, PLAYER_WALK_ATTRS)

// TODO: If we like the above approach, refactor existing code to use this.
// Though I like the "View" structs, I don't like the macros and may prefer to define
// these in a YAML file and then generate the C++ via Python. For example:
//
// combos:
//   CutsceneEffect_Camera:
//     attributes:
//       1: speed # default to creating zfix accessor
//       # Dictionary format can specify the type that the underlying zfix is interpreted as.
//       2:
//         name: freeze
//         type: bool
//       3:
//         name: camera_mode
//         type: CameraModeEnum

#define AF_FRESH                        0x01
#define AF_CYCLE                        0x02
#define AF_CYCLENOCSET                  0x04
#define AF_TRANSPARENT                  0x08
#define AF_CYCLEUNDERCOMBO              0x10
#define AF_EDITOR_ONLY                  0x20
#define AF_OVERHEAD                     0x40

enum ComboTriggerFlag
{
	TRIGFLAG_SWORD,
	TRIGFLAG_SWORDBEAM,
	TRIGFLAG_BRANG,
	TRIGFLAG_BOMB,
	TRIGFLAG_SBOMB,
	TRIGFLAG_LITBOMB,
	TRIGFLAG_LITSBOMB,
	TRIGFLAG_ARROW,
	TRIGFLAG_FIRE,
	TRIGFLAG_WHISTLE,
	TRIGFLAG_BAIT,
	TRIGFLAG_WAND,
	TRIGFLAG_MAGIC,
	TRIGFLAG_WIND,
	TRIGFLAG_REFMAGIC,
	TRIGFLAG_REFFIREBALL,
	TRIGFLAG_REFROCK,
	TRIGFLAG_HAMMER,
	TRIGFLAG_RESETANIM,
	TRIGFLAG_INVERTPROX,
	TRIGFLAG_BTN_TOP,
	TRIGFLAG_BTN_BOTTOM,
	TRIGFLAG_BTN_LEFT,
	TRIGFLAG_BTN_RIGHT,
	TRIGFLAG_INVERTMINMAX,
	TRIGFLAG_STEP,
	TRIGFLAG_STEPSENS,
	TRIGFLAG_SHUTTER,
	TRIGFLAG_CMBTYPEFX,
	TRIGFLAG_ONLYGENTRIG,
	TRIGFLAG_KILLWPN,
	TRIGFLAG_EWFIREBALL,
	TRIGFLAG_HOOKSHOT,
	TRIGFLAG_SPARKLE,
	TRIGFLAG_BYRNA,
	TRIGFLAG_REFBEAM,
	TRIGFLAG_STOMP,
	TRIGFLAG_SCRIPT01,
	TRIGFLAG_SCRIPT02,
	TRIGFLAG_SCRIPT03,
	TRIGFLAG_SCRIPT04,
	TRIGFLAG_SCRIPT05,
	TRIGFLAG_SCRIPT06,
	TRIGFLAG_SCRIPT07,
	TRIGFLAG_SCRIPT08,
	TRIGFLAG_SCRIPT09,
	TRIGFLAG_SCRIPT10,
	TRIGFLAG_AUTOMATIC,
	TRIGFLAG_SECRETS,
	TRIGFLAG_INVERTITEM,
	TRIGFLAG_CONSUMEITEM,
	TRIGFLAG_COUNTERGE,
	TRIGFLAG_COUNTERLT,
	TRIGFLAG_COUNTEREAT,
	TRIGFLAG_CTRNONLYTRIG,
	TRIGFLAG_LIGHTON,
	TRIGFLAG_LIGHTOFF,
	TRIGFLAG_PUSH,
	TRIGFLAG_LENSON,
	TRIGFLAG_LENSOFF,
	TRIGFLAG_EWARROW,
	TRIGFLAG_EWBRANG,
	TRIGFLAG_EWSWORD,
	TRIGFLAG_EWROCK,
	TRIGFLAG_EWSCRIPT01,
	TRIGFLAG_EWSCRIPT02,
	TRIGFLAG_EWSCRIPT03,
	TRIGFLAG_EWSCRIPT04,
	TRIGFLAG_EWSCRIPT05,
	TRIGFLAG_EWSCRIPT06,
	TRIGFLAG_EWSCRIPT07,
	TRIGFLAG_EWSCRIPT08,
	TRIGFLAG_EWSCRIPT09,
	TRIGFLAG_EWSCRIPT10,
	TRIGFLAG_EWMAGIC,
	TRIGFLAG_EWBBLAST,
	TRIGFLAG_EWSBBLAST,
	TRIGFLAG_EWLITBOMB,
	TRIGFLAG_EWLITSBOMB,
	TRIGFLAG_EWFIRETRAIL,
	TRIGFLAG_EWFLAME,
	TRIGFLAG_EWWIND,
	TRIGFLAG_EWFLAME2,
	TRIGFLAG_SPCITEM,
	TRIGFLAG_EXSTITEM,
	TRIGFLAG_EXSTENEMY,
	TRIGFLAG_AUTOGRABITEM,
	TRIGFLAG_ENEMIESKILLED,
	TRIGFLAG_SECRETSTR,
	TRIGFLAG_THROWN,
	TRIGFLAG_QUAKESTUN,
	TRIGFLAG_SQUAKESTUN,
	TRIGFLAG_ANYFIRE,
	TRIGFLAG_STRONGFIRE,
	TRIGFLAG_MAGICFIRE,
	TRIGFLAG_DIVINEFIRE,
	TRIGFLAG_TRIGLEVELSTATE,
	TRIGFLAG_LEVELSTATE,
	TRIGFLAG_TRIGGLOBALSTATE,
	TRIGFLAG_GLOBALSTATE,
	TRIGFLAG_KILLENEMIES,
	TRIGFLAG_CLEARENEMIES,
	TRIGFLAG_CLEARLWEAPONS,
	TRIGFLAG_CLEAREWEAPONS,
	TRIGFLAG_IGNITE_ANYFIRE,
	TRIGFLAG_IGNITE_STRONGFIRE,
	TRIGFLAG_IGNITE_MAGICFIRE,
	TRIGFLAG_IGNITE_DIVINEFIRE,
	TRIGFLAG_SEPARATEWEAPON,
	TRIGFLAG_TGROUP_CONTRIB,
	TRIGFLAG_TGROUP_LESS,
	TRIGFLAG_TGROUP_GREATER,
	TRIGFLAG_PUSHEDTRIG,
	TRIGFLAG_DIVETRIG,
	TRIGFLAG_DIVESENSTRIG,
	TRIGFLAG_LWREFARROW,
	TRIGFLAG_LWREFFIRE,
	TRIGFLAG_LWREFFIRE2,
	TRIGFLAG_TOGGLEDARK,
	TRIGFLAG_COND_DARK,
	TRIGFLAG_COND_NODARK,
	TRIGFLAG_LITEM_COND,
	TRIGFLAG_LITEM_REVCOND,
	TRIGFLAG_LITEM_SET,
	TRIGFLAG_LITEM_UNSET,
	TRIGFLAG_TINT_CLEAR,
	TRIGFLAG_ONLY_GROUND_WPN,
	TRIGFLAG_COUNTERDISCOUNT,
	TRIGFLAG_SCREENLOAD,
	TRIGFLAG_UNSETEXSTATE,
	TRIGFLAG_UNSETEXDOOR,
	TRIGFLAG_PLAYER_STANDING,
	TRIGFLAG_PLAYER_NOTSTANDING,
	TRIGFLAG_INVERT_PLAYER_Z,
	TRIGFLAG_COUNTER_GRADUAL,
	TRIGFLAG_COUNTER_PERCENT,
	TRIGFLAG_NO_COPYCAT_CAUSE,
	TRIGFLAG_PLAYERLANDHERE,
	TRIGFLAG_PLAYERLANDANYWHERE,
	TRIGFLAG_SETPLAYER_X_ABS,
	TRIGFLAG_SETPLAYER_X_REL_CMB,
	TRIGFLAG_SETPLAYER_Y_ABS,
	TRIGFLAG_SETPLAYER_Y_REL_CMB,
	TRIGFLAG_SETPLAYER_Z_ABS,
	TRIGFLAG_CMBTYPECAUSES,
	TRIGFLAG_REQ_JUMP_GE,
	TRIGFLAG_REQ_JUMP_LE,
	TRIGFLAG_REQ_X_GE,
	TRIGFLAG_REQ_X_LE,
	TRIGFLAG_REQ_X_REL,
	TRIGFLAG_REQ_Y_GE,
	TRIGFLAG_REQ_Y_LE,
	TRIGFLAG_REQ_Y_REL,
	TRIGFLAG_FORCE_ICE_VX,
	TRIGFLAG_FORCE_ICE_VY,
	TRIGFLAG_UNIGNITE_WEAPONS,
	TRIGFLAG_CANCEL_TRIGGER,
	TRIGFLAG_SET_GRAVITY,
	TRIGFLAG_REVERT_GRAVITY,
	TRIGFLAG_BLOCK_TRIGGER_SAME_LAYER,
	TRIGFLAG_BLOCK_TRIGGER_ANY_LAYER,
	TRIGFLAG_VIEWPORT_REQ_ONSCREEN,
	TRIGFLAG_VIEWPORT_REQ_OFFSCREEN,
	TRIGFLAG_SFX_LOOP,
	TRIGFLAG_SFX_NO_RESTART,
	TRIGFLAG_SFX_KILL,
	TRIGFLAG_CMB_CHANGE_ABSOLUTE,
	TRIGFLAG_CSET_CHANGE_ABSOLUTE,
	TRIGFLAG_SWIMTRIG,
	TRIGFLAG_SWIMSENSTRIG,
	TRIGFLAG_RESPAWN_HERE,
	TRIGFLAG_RESET_RESPAWN,
	TRIGFLAG_SCREENUNLOAD,
	TRIGFLAG_PROX_USE_SOL_HITBOX,
	TRIGFLAG_PROX_USE_SQUARE,
	TRIGFLAG_COUNTER_PARTIAL_CONSUME,
	// Trigger condition: only occurs during Map View (spacebar map).
	TRIGFLAG_MAPVIEW_ONLY,
	// Trigger condition: never occurs during Map View (spacebar map).
	TRIGFLAG_MAPVIEW_NEVER,
	// Trigger cause: combo position specific state
	TRIGFLAG_COMBOPOSSTATE_CAUSE,
	// Trigger condition: combo position specific state
	TRIGFLAG_COMBOPOSSTATE_REQ,
	// Trigger condition: not combo position specific state
	TRIGFLAG_COMBOPOSSTATE_UNREQ,
	// Trigger effect: set combo position specific state
	// Combined with [TRIGFLAG_COMBOPOSSTATE_UNSET] is also on, toggles state instead.
	TRIGFLAG_COMBOPOSSTATE_SET,
	// Trigger effect: unset combo position specific state
	// Combined with [TRIGFLAG_COMBOPOSSTATE_SET] is also on, toggles state instead.
	TRIGFLAG_COMBOPOSSTATE_UNSET,
	// Trigger effect: cause all combos in this combo's 'Large Combo' group
	// as determined by [combodata::LargeComboDirs] to trigger all triggers they
	// have with [TRIGFLAG_TRIGGERED_BY_LARGE_COMBO_COPYCAT] set, and the same
	// [combotrigger::LargeComboCopycatID].
	TRIGFLAG_TRIGGER_LARGE_COMBO_COPYCAT,
	// Trigger cause: caused when any combo in this combo's 'Large Combo' group
	// as determined by [combodata::LargeComboDirs] trigger a trigger they
	// have with [TRIGFLAG_TRIGGER_LARGE_COMBO_COPYCAT] set, and the same
	// [combotrigger::LargeComboCopycatID].
	TRIGFLAG_TRIGGERED_BY_LARGE_COMBO_COPYCAT,

	TRIGFLAG_MAX
};

#define ctrigNONE                       0x00
#define ctrigIGNORE_SIGN                0x01
#define ctrigSECRETS                    0x02
#define ctrigSWITCHSTATE                0x04

enum //cCRUMBLE types
{
	CMBTY_CRUMBLE_RESET,
	CMBTY_CRUMBLE_CUMULATIVE,
	CMBTY_CRUMBLE_INEVITABLE,
};

extern std::vector<newcombo> combobuf;

bool is_push_flag(int flag, optional<int> dir = nullopt);

#endif

