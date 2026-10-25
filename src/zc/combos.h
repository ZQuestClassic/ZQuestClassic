#ifndef COMBOS_H_
#define COMBOS_H_

#include "base/compiler.h"
#include "core/handles.h"
#include "core/zdefs.h"
#include "core/cpos_info.h"
#include "core/combo.h"

#define minSECRET_TYPE 0
#define maxSECRET_TYPE 43

class weapon;

void do_cutscene_flags(newcombo const& cmb);
class CutsceneState
{
	bool active = false;
	int32_t allowed_btns = 0;
	bool nof6 = false;
	int errsfx = 0;
public:
	void clear();
	bool can_button(int q);
	bool can_f6();
	void error();
	friend void do_cutscene_flags(newcombo const& cmb);
};
extern CutsceneState active_cutscene;

class TriggerData
{
public:
	int32_t special;
	weapon* w;
	std::function<bool(combo_trigger const&)> trig_cond_a;
	std::function<bool(combo_trigger const&, size_t)> trig_cond_b;
	std::function<void(combo_trigger const&, bool)> side_fx;
	
	TriggerData(std::function<bool(combo_trigger const&)> const& cond)
		: special(0), w(nullptr), trig_cond_a(cond), trig_cond_b(), side_fx()
	{}
	TriggerData(std::function<bool(combo_trigger const&, size_t)> const& cond)
		: special(0), w(nullptr), trig_cond_a(), trig_cond_b(cond), side_fx()
	{}
	TriggerData& set_special(int32_t new_special);
	TriggerData& set_weapon(weapon* new_weapon);
	TriggerData& set_side_fx(std::function<void(combo_trigger const&, bool)> const& new_side_fx);
	
	bool execute(const combined_handle_t& comb_handle) const;
};
extern const TriggerData ctype_causes_trigger;

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
bool force_ex_trigger(const combined_handle_t& handle, size_t idx, int8_t xstate = -1);
bool force_ex_trigger_any(const combined_handle_t& handle, int8_t xstate = -1);
bool force_ex_door_trigger(const combined_handle_t& handle, size_t idx, int dir = -1, uint ind = 0);
bool force_ex_door_trigger_any(const combined_handle_t& handle, int dir = -1, uint ind = 0);
bool force_combopos_trigger(const combined_handle_t& handle, size_t idx, int mi = -1);
bool force_combopos_trigger_any(const combined_handle_t& handle, int mi = -1);
bool do_trigger_combo(const combined_handle_t& comb_handle, size_t idx, int32_t special = 0, weapon* w = NULL);
bool check_trig_conditions(const combined_handle_t& comb_handle, size_t idx);

bool do_lift_combo(const combined_handle_t&, int32_t gloveid);

void trig_trigger_groups();


// CPOS stuff
cpos_info& cpos_get(const combined_handle_t& handle);
int cpos_trig_group_count(int ind);
int cpos_exists_spotlight();

void cpos_clear_all();
void cpos_force_update();
void cpos_update();
// The per-region cpos counts (trigger groups, spotlights, looping sfx) are built
// from each instance's `cpos_info::data`. Before a combo's data changes,
// screen_combo_modify_pre uncounts (add = -1) every instance of it, and after
// the change screen_combo_modify_post counts (add = +1) them again, so the
// counts follow the combo's old and new settings.
void cpos_count_instance(int cid, int add);
// Whether the moving pushblock currently holds the counts of an instance of `cid`.
bool cpos_moving_block_holds(int cid);

void combos_init_game_vars();

#endif
