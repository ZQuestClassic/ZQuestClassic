#ifndef COMBOS_H_
#define COMBOS_H_

#include "base/zdefs.h"
#include "base/cpos_info.h"
#include "zc/weapons.h"
#include "base/combo.h"

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
bool do_trigger_combo_ffc(const ffc_handle_t& ffc_handle, int32_t special = 0, weapon* w = NULL);

bool do_lift_combo(const rpos_handle_t&, int32_t gloveid);

void handle_cpos_type(newcombo const& cmb, cpos_info& timer, int lyr, int pos);
void handle_ffcpos_type(newcombo const& cmb, cpos_info& timer, ffcdata& f);
void trig_trigger_groups();


// CPOS stuff
cpos_info& cpos_get(const rpos_handle_t& rpos_handle);
int cpos_trig_group_count(int ind);
int cpos_exists_spotlight();

void cpos_clear_all();
void cpos_force_update();
void cpos_update();


#endif

