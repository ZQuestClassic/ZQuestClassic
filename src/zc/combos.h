#ifndef _COMBOS_H_
#define _COMBOS_H_

#include "base/zdefs.h"
#include "weapons.h"

extern newcombo *combobuf;
#define minSECRET_TYPE 0
#define maxSECRET_TYPE 43

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

extern cpos_info combo_posinfos[7][176];
extern std::vector<cpos_info> ffc_posinfos;

bool do_cswitch_combo(newcombo const& cmb, weapon* w = NULL);
void do_generic_combo_ffc2(const ffc_handle_t& ffc_handle, int32_t cid, int32_t ft);

void spawn_decoration(newcombo const& cmb, const pos_handle_t& pos_handle);
void spawn_decoration_xy(newcombo const& cmb, zfix x, zfix y);

bool can_locked_combo(newcombo const& cmb);
bool try_locked_combo(newcombo const& cmb);

void trigger_sign(newcombo const& cmb, int screen_index);
void trigger_sign(newcombo const& cmb);
void trigger_cuttable(const pos_handle_t& pos_handle);
void trigger_cuttable_ffc(const ffc_handle_t& ffc_handle);
bool trigger_step(const pos_handle_t& pos_handle);
bool trigger_step_ffc(const ffc_handle_t& ffc_handle);
bool trigger_chest(const pos_handle_t& pos_handle);
bool trigger_chest_ffc(const pos_handle_t& pos_handle);
bool trigger_lockblock(const pos_handle_t& pos_handle);
bool trigger_lockblock_ffc(const pos_handle_t& pos_handle);
bool trigger_armos_grave(const pos_handle_t& pos_handle, int32_t trigdir = -1);
bool trigger_armos_grave_ffc(const ffc_handle_t& ffc_handle, int32_t trigdir = -1);
bool trigger_damage_combo(mapscr* screen, int32_t cid, int32_t hdir = -1, bool force_solid = false);
bool trigger_stepfx(const pos_handle_t& pos_handle, bool stepped = false);
bool trigger_stepfx_ffc(const ffc_handle_t& ffc_handle, bool stepped = false);
void trigger_save(newcombo const& cmb);

void do_ex_trigger(const pos_handle_t& pos_handle);
void do_ex_trigger_ffc(const ffc_handle_t& ffc_handle);
bool force_ex_trigger(const pos_handle_t& pos_handle, char xstate = -1);
bool force_ex_trigger_ffc(const ffc_handle_t& ffc_handle, char xstate = -1);
bool do_trigger_combo(int layer, int pos, int32_t special = 0, weapon* w = NULL);
bool do_trigger_combo(const pos_handle_t& pos_handle, int32_t special = 0, weapon* w = NULL);
bool do_trigger_combo_ffc(const ffc_handle_t& ffc_handle, int32_t special = 0, weapon* w = NULL);

bool do_lift_combo(int32_t lyr, int32_t pos, int32_t gloveid);

void init_combo_timers();
void update_combo_timers();
bool on_cooldown(int32_t lyr, int32_t pos);

#endif

