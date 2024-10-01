#ifndef ZC_TITLE_H_
#define ZC_TITLE_H_

#include "zc/saves.h"

extern DIALOG gamemode_dlg[];

bool prompt_for_quest_path(std::string current_qstpath);
save_t* get_unset_save_slot();
void titlescreen(int32_t lsave);
void game_over(int32_t type);
void save_game(bool savepoint);
bool save_game(bool savepoint, int32_t type);

#endif
