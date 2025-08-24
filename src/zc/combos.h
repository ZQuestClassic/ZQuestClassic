#ifndef COMBOS_H_
#define COMBOS_H_

#include "base/compiler.h"
#include "base/handles.h"
#include "base/zdefs.h"
#include "base/cpos_info.h"
#include "base/combo.h"

#define minSECRET_TYPE 0
#define maxSECRET_TYPE 43

class weapon;

void do_cutscene_flags(newcombo const& cmb);
class CutsceneState
{
	bool active = false;
	int32_t allowed_btns = 0;
	bool nof6 = false;
	byte errsfx = 0;
public:
	void clear();
	bool can_button(int q);
	bool can_f6();
	void error();
	friend void do_cutscene_flags(newcombo const& cmb);
};
extern CutsceneState active_cutscene;

bool do_cswitch_combo(newcombo const& cmb, weapon* w = NULL);
void do_generic_combo_ffc2(const ffc_handle_t& ffc_handle, int32_t cid, int32_t ft);

void spawn_decoration(newcombo const& cmb, const combined_handle_t& handle);
void spawn_decoration_xy(newcombo const& cmb, zfix x, zfix y, zfix cx, zfix cy);

bool can_locked_combo(newcombo const& cmb);
bool try_locked_combo(newcombo const& cmb);

void trigger_sign(newcombo const& cmb, optional<int> screen = nullopt);
bool play_combo_string(int str, optional<int> screen = nullopt);

void trigger_cuttable(const combined_handle_t& handle);
bool trigger_step(const combined_handle_t& handle);
bool trigger_chest(const combined_handle_t& handle);
bool trigger_lockblock(const combined_handle_t& handle);
bool trigger_armos_grave(const combined_handle_t& ffc_handle, int32_t trigdir = -1);
bool trigger_damage_combo(mapscr* scr, int32_t cid, int type = ZSD_NONE, int ptrval = 0, int32_t hdir = -1, bool force_solid = false);
bool trigger_damage_combo(const combined_handle_t& handle, int type = ZSD_NONE, int ptrval = 0, int32_t hdir = -1, bool force_solid = false);
bool trigger_stepfx(const combined_handle_t& handle, bool stepped = false);
void trigger_save(newcombo const& cmb, mapscr* scr);

void trig_copycat(byte copyid);

void do_ex_trigger(const combined_handle_t& handle, size_t idx);
bool force_ex_trigger(const combined_handle_t& handle, size_t idx, char xstate = -1);
bool force_ex_trigger_any(const combined_handle_t& handle, char xstate = -1);
bool force_ex_door_trigger(const combined_handle_t& handle, size_t idx, int dir = -1, uint ind = 0);
bool force_ex_door_trigger_any(const combined_handle_t& handle, int dir = -1, uint ind = 0);
bool trig_each_combo_trigger(const combined_handle_t& comb_handle, std::function<bool(combo_trigger const&)> trig_cond, int32_t special = 0, weapon* w = NULL);
bool trig_each_combo_trigger(const combined_handle_t& comb_handle, std::function<bool(combo_trigger const&, size_t)> trig_cond, int32_t special = 0, weapon* w = NULL);
bool do_trigger_combo(const combined_handle_t& comb_handle, size_t idx, int32_t special = 0, weapon* w = NULL);
bool do_trigger_ctype_causes(const combined_handle_t& comb_handle);
bool check_trig_conditions(const combined_handle_t& comb_handle, size_t idx);

bool do_lift_combo(const rpos_handle_t&, int32_t gloveid);

void trig_trigger_groups();


// CPOS stuff
cpos_info& cpos_get(const combined_handle_t& handle);
int cpos_trig_group_count(int ind);
int cpos_exists_spotlight();

void cpos_clear_all();
void cpos_force_update();
void cpos_update();

// These combo caches improve cache locality for hot functions.
// Verify that any properties used here call `screen_combo_modify_post` when modified in
// ffscript.cpp.

template <typename T>
struct combo_cache
{
	using value_type = T;

	std::vector<value_type> minis;

	void init()
	{
		minis.resize(combobuf.size());
	}

	void refresh(int cid)
	{
		if (minis.size() > cid)
			minis[cid] = combobuf[cid];
	}
};

namespace combo_caches
{
	struct minicombo_type
	{
		byte type;

		minicombo_type() = default;
		minicombo_type(const newcombo& combo)
		{
			type = combo.type;
		}
	};
	extern combo_cache<minicombo_type> type;

	struct minicombo_flag
	{
		byte flag;

		minicombo_flag() = default;
		minicombo_flag(const newcombo& combo)
		{
			flag = combo.flag;
		}
	};
	extern combo_cache<minicombo_flag> flag;

	struct minicombo_cpos_update
	{
		byte type;
		byte sfx_loop;
		bool trigtimer;

		minicombo_cpos_update() = default;
		minicombo_cpos_update(const newcombo& combo)
		{
			type = combo.type;
			sfx_loop = combo.sfx_loop;
			trigtimer = false;
			for(auto& trig : combo.triggers)
			{
				if(trig.trigtimer)
				{
					trigtimer = true;
					break;
				}
			}
		}
	};
	extern combo_cache<minicombo_cpos_update> cpos_update;

	struct minicombo_trigger_group
	{
		bool tgroup : 1;

		minicombo_trigger_group() = default;
		minicombo_trigger_group(const newcombo& combo)
		{
			tgroup = false;
			for(auto& trig : combo.triggers)
			{
				if(trig.trigger_flags.any({TRIGFLAG_TGROUP_LESS,TRIGFLAG_TGROUP_GREATER}))
				{
					tgroup = true;
					break;
				}
			}
		}
	};
	extern combo_cache<minicombo_trigger_group> trigger_group;

	struct minicombo_can_cycle
	{
		bool can_cycle;

		minicombo_can_cycle() = default;
		minicombo_can_cycle(const newcombo& combo)
		{
			can_cycle = combo.can_cycle();
		}
	};
	extern combo_cache<minicombo_can_cycle> can_cycle;

	struct minicombo_shutter
	{
		bool shutter;

		minicombo_shutter() = default;
		minicombo_shutter(const newcombo& combo)
		{
			shutter = false;
			for(auto& trig : combo.triggers)
			{
				if(trig.trigger_flags.get(TRIGFLAG_SHUTTER))
				{
					shutter = true;
					break;
				}
			}
		}
	};
	extern combo_cache<minicombo_shutter> shutter;

	struct minicombo_gswitch
	{
		bool trigger_global_state : 1;
		bool has_global_state : 1;

		minicombo_gswitch() = default;
		minicombo_gswitch(const newcombo& combo)
		{
			has_global_state = combo.usrflags & cflag11;
			trigger_global_state = false;
			for(auto& trig : combo.triggers)
			{
				if(trig.trigger_flags.get(TRIGFLAG_TRIGGLOBALSTATE))
				{
					trigger_global_state = true;
					break;
				}
			}
		}
	};
	extern combo_cache<minicombo_gswitch> gswitch;

	struct minicombo_script
	{
		word script;

		minicombo_script() = default;
		minicombo_script(const newcombo& combo)
		{
			script = combo.script;
		}
	};
	extern combo_cache<minicombo_script> script;

	struct minicombo_drawing
	{
		int32_t tile;
		byte csets;
		byte flip;
		byte type;
		byte frames;
		byte skipanimy;
		byte animflags;

		minicombo_drawing() = default;
		minicombo_drawing(const newcombo& combo)
		{
			tile = combo.tile;
			csets = combo.csets;
			flip = combo.flip;
			type = combo.type;
			frames = combo.frames;
			skipanimy = combo.skipanimy;
			animflags = combo.animflags;
		}
	};
	extern combo_cache<minicombo_drawing> drawing;

	struct minicombo_lens
	{
		bool on : 1;
		bool off : 1;

		minicombo_lens() = default;
		minicombo_lens(const newcombo& combo)
		{
			on = false;
			off = false;
			for(auto& trig : combo.triggers)
			{
				if(trig.trigger_flags.get(TRIGFLAG_LENSON))
				{
					on = true;
					if(off) break;
				}
				if(trig.trigger_flags.get(TRIGFLAG_LENSOFF))
				{
					off = true;
					if(on) break;
				}
			}
		}
	};
	extern combo_cache<minicombo_lens> lens;

	struct minicombo_spotlight
	{
		bool trigger : 1;
		bool target : 1;

		minicombo_spotlight() = default;
		minicombo_spotlight(const newcombo& combo)
		{
			target = combo.type == cLIGHTTARGET;
			trigger = false;
			for(auto& trig : combo.triggers)
			{
				if(trig.trigger_flags.any({TRIGFLAG_LIGHTON,TRIGFLAG_LIGHTOFF}))
				{
					trigger = true;
					break;
				}
			}
		}
	};
	extern combo_cache<minicombo_spotlight> spotlight;

	ZC_FORCE_INLINE void refresh()
	{
		type.init();
		flag.init();
		cpos_update.init();
		trigger_group.init();
		can_cycle.init();
		shutter.init();
		gswitch.init();
		script.init();
		drawing.init();
		lens.init();
		spotlight.init();

		for (int i = 0; i < combobuf.size(); i++)
		{
			type.refresh(i);
			flag.refresh(i);
			cpos_update.refresh(i);
			trigger_group.refresh(i);
			can_cycle.refresh(i);
			shutter.refresh(i);
			gswitch.refresh(i);
			script.refresh(i);
			drawing.refresh(i);
			lens.refresh(i);
			spotlight.refresh(i);
		}
	}

	ZC_FORCE_INLINE void refresh(int cid)
	{
		type.refresh(cid);
		flag.refresh(cid);
		cpos_update.refresh(cid);
		trigger_group.refresh(cid);
		can_cycle.refresh(cid);
		shutter.refresh(cid);
		gswitch.refresh(cid);
		script.refresh(cid);
		drawing.refresh(cid);
		lens.refresh(cid);
		spotlight.refresh(cid);
	}
}

#endif
