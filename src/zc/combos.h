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

void spawn_decoration(newcombo const& cmb, const rpos_handle_t& rpos_handle);
void spawn_decoration_xy(newcombo const& cmb, zfix x, zfix y);

bool can_locked_combo(newcombo const& cmb);
bool try_locked_combo(newcombo const& cmb);

void trigger_sign(newcombo const& cmb, int screen);
bool play_combo_string(int str);
bool play_combo_string(int str, int scr);

void trigger_sign(newcombo const& cmb);
void trigger_cuttable(const rpos_handle_t& rpos_handle);
void trigger_cuttable_ffc(const ffc_handle_t& ffc_handle);
bool trigger_step(const rpos_handle_t& rpos_handle);
bool trigger_step_ffc(const ffc_handle_t& ffc_handle);
bool trigger_chest(const rpos_handle_t& rpos_handle);
bool trigger_chest_ffc(const rpos_handle_t& rpos_handle);
bool trigger_lockblock(const rpos_handle_t& rpos_handle);
bool trigger_lockblock_ffc(const rpos_handle_t& rpos_handle);
bool trigger_armos_grave(const rpos_handle_t& rpos_handle, int32_t trigdir = -1);
bool trigger_armos_grave_ffc(const ffc_handle_t& ffc_handle, int32_t trigdir = -1);
bool trigger_damage_combo(mapscr* scr, int32_t cid, int type = ZSD_NONE, int ptrval = 0, int32_t hdir = -1, bool force_solid = false);
bool trigger_damage_combo(const rpos_handle_t& rpos_handle, int type = ZSD_NONE, int ptrval = 0, int32_t hdir = -1, bool force_solid = false);
bool trigger_stepfx(const rpos_handle_t& rpos_handle, bool stepped = false);
bool trigger_stepfx_ffc(const ffc_handle_t& ffc_handle, bool stepped = false);
void trigger_save(newcombo const& cmb, mapscr* scr);

void trig_copycat(byte copyid);

void do_ex_trigger(const rpos_handle_t& rpos_handle);
void do_ex_trigger_ffc(const ffc_handle_t& ffc_handle);
bool force_ex_trigger(const rpos_handle_t& rpos_handle, char xstate = -1);
bool force_ex_trigger_ffc(const ffc_handle_t& ffc_handle, char xstate = -1);
bool force_ex_door_trigger(const rpos_handle_t& rpos_handle, int dir = -1, uint ind = 0);
bool force_ex_door_trigger_ffc(const ffc_handle_t& ffc_handle, int dir = -1, uint ind = 0);
bool do_trigger_combo(int layer, int pos, int32_t special = 0, weapon* w = NULL);
bool do_trigger_combo(const rpos_handle_t& rpos_handle, int32_t special = 0, weapon* w = NULL);
bool do_trigger_combo(const ffc_handle_t& ffc_handle, int32_t special = 0, weapon* w = NULL);

bool do_lift_combo(const rpos_handle_t&, int32_t gloveid);

void handle_cpos_type(byte combo_type, cpos_info& timer, int lyr, int pos);
void handle_ffcpos_type(newcombo const& cmb, cpos_info& timer, ffcdata& f);
void trig_trigger_groups();


// CPOS stuff
cpos_info& cpos_get(const rpos_handle_t& rpos_handle);
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
		byte trigtimer;

		minicombo_cpos_update() = default;
		minicombo_cpos_update(const newcombo& combo)
		{
			type = combo.type;
			sfx_loop = combo.sfx_loop;
			trigtimer = combo.trigtimer;
		}
	};
	extern combo_cache<minicombo_cpos_update> cpos_update;

	struct minicombo_trigger_group
	{
		byte flags;

		minicombo_trigger_group() = default;
		minicombo_trigger_group(const newcombo& combo)
		{
			bool less = combo.triggerflags[3] & combotriggerTGROUP_LESS;
			bool greater = combo.triggerflags[3] & combotriggerTGROUP_GREATER;
			flags = (less ? 1 : 0) + (greater ? 2 : 0);
		}

		bool less() const { return flags&1; }
		bool greater() const { return flags&2; }
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
			shutter = combo.triggerflags[0] & combotriggerSHUTTER;
		}
	};
	extern combo_cache<minicombo_shutter> shutter;

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

		minicombo_drawing() = default;
		minicombo_drawing(const newcombo& combo)
		{
			tile = combo.tile;
			csets = combo.csets;
			flip = combo.flip;
			type = combo.type;
			frames = combo.frames;
			skipanimy = combo.skipanimy;
		}
	};
	extern combo_cache<minicombo_drawing> drawing;

	struct minicombo_lens
	{
		byte flags;

		minicombo_lens() = default;
		minicombo_lens(const newcombo& combo)
		{
			bool on = combo.triggerflags[1] & combotriggerLENSON;
			bool off = combo.triggerflags[1] & combotriggerLENSOFF;
			flags = (on ? 1 : 0) + (off ? 2 : 0);
		}

		bool on() const { return flags&1; }
		bool off() const { return flags&2; }
	};
	extern combo_cache<minicombo_lens> lens;

	ZC_FORCE_INLINE void refresh()
	{
		type.init();
		flag.init();
		cpos_update.init();
		trigger_group.init();
		can_cycle.init();
		shutter.init();
		script.init();
		drawing.init();
		lens.init();

		for (int i = 0; i < combobuf.size(); i++)
		{
			type.refresh(i);
			flag.refresh(i);
			cpos_update.refresh(i);
			trigger_group.refresh(i);
			can_cycle.refresh(i);
			shutter.refresh(i);
			script.refresh(i);
			drawing.refresh(i);
			lens.refresh(i);
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
		script.refresh(cid);
		drawing.refresh(cid);
		lens.refresh(cid);
	}
}

#endif
