#ifndef _COMBOS_H_
#define _COMBOS_H_

#include "zdefs.h"
#include "weapons.h"

extern newcombo *combobuf;
#define minSECRET_TYPE 0
#define maxSECRET_TYPE 43

int32_t isNextType(int32_t type);

void do_trigger_combo(int32_t layer, int32_t pos, int32_t special = 0);
bool do_cswitch_combo(newcombo const& cmb, int32_t layer, int32_t cpos, weapon* w = NULL);
void do_generic_combo2(int32_t bx, int32_t by, int32_t cid, int32_t flag, int32_t flag2, int32_t ft, int32_t scombo, bool single16, int32_t layer);

void runDecoration(newcombo const& cmb, int32_t pos);
void triggerCuttable(int32_t lyr, int32_t pos);

bool can_locked_combo(newcombo const& cmb);
bool try_locked_combo(newcombo const& cmb);

void trigger_sign(newcombo const& cmb);

#endif

