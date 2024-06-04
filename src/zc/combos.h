#ifndef _COMBOS_H_
#define _COMBOS_H_

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
void do_generic_combo2(int32_t bx, int32_t by, int32_t cid, int32_t flag, int32_t flag2, int32_t ft, int32_t scombo, bool single16, int32_t layer);
void do_generic_combo_ffc2(int32_t pos, int32_t cid, int32_t ft);

void spawn_decoration(newcombo const& cmb, int32_t pos);
void spawn_decoration_xy(newcombo const& cmb, zfix x, zfix y);

bool can_locked_combo(newcombo const& cmb);
bool try_locked_combo(newcombo const& cmb);

bool play_combo_string(int str);

void trigger_sign(newcombo const& cmb);
void trigger_cuttable(int32_t lyr, int32_t pos);
void trigger_cuttable_ffc(int32_t pos);
bool trigger_step(int32_t lyr, int32_t pos);
bool trigger_step_ffc(int32_t pos);
bool trigger_chest(int32_t lyr, int32_t pos);
bool trigger_chest_ffc(int32_t pos);
bool trigger_lockblock(int32_t lyr, int32_t pos);
bool trigger_lockblock_ffc(int32_t pos);
bool trigger_armos_grave(int32_t lyr, int32_t pos, int32_t trigdir = -1);
bool trigger_armos_grave_ffc(int32_t pos, int32_t trigdir = -1);
bool trigger_damage_combo(int32_t cid, int type = ZSD_NONE, int ptrval = 0, int32_t hdir = -1, bool force_solid = false);
bool trigger_stepfx(int32_t lyr, int32_t pos, bool stepped = false);
bool trigger_stepfx_ffc(int32_t pos, bool stepped = false);
void trigger_save(newcombo const& cmb);

void do_ex_trigger(int32_t lyr, int32_t pos);
void do_ex_trigger_ffc(int32_t pos);
bool force_ex_trigger(int32_t lyr, int32_t pos, char xstate = -1);
bool force_ex_trigger_ffc(int32_t pos, char xstate = -1);
bool force_ex_door_trigger(uint lyr, uint pos, int dir = -1, uint ind = 0);
bool force_ex_door_trigger_ffc(uint pos, int dir = -1, uint ind = 0);
bool do_trigger_combo(int32_t layer, int32_t pos, int32_t special = 0, weapon* w = NULL);
bool do_trigger_combo_ffc(int32_t pos, int32_t special = 0, weapon* w = NULL);

bool do_lift_combo(int32_t lyr, int32_t pos, int32_t gloveid);

void handle_cpos_type(newcombo const& cmb, cpos_info& timer, int lyr, int pos);
void handle_ffcpos_type(newcombo const& cmb, cpos_info& timer, ffcdata& f);
void trig_trigger_groups();


// CPOS stuff
cpos_info& cpos_get(int32_t layer, int32_t pos);
int cpos_trig_group_count(int ind);
int cpos_exists_spotlight();

void cpos_clear_all();
void cpos_force_update();
void cpos_update();


#endif

