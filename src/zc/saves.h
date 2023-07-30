#ifndef _ZC_SAVES_H_
#define _ZC_SAVES_H_

#include "gamedata.h"

extern gamedata *saves;

int32_t saves_init();
int32_t saves_load();
int32_t saves_write();
void saves_select(int32_t index);
int32_t saves_count();
int32_t saves_current_selection();
const gamedata* saves_get_data(int32_t index);
gamedata* saves_get_data_mutable(int32_t index);
const gamedata* saves_get_data();
void saves_update_icon(int index);
void save_game(bool savepoint);
bool save_game(bool savepoint, int32_t type);
void saves_do_first_time_stuff(int index);

#endif
