#ifndef _ZC_TITLE_H_
#define _ZC_TITLE_H_

extern DIALOG gamemode_dlg[];

int32_t custom_game(int32_t file);
int32_t getsaveslot();
void titlescreen(int32_t lsave);
void game_over(int32_t type);
void save_game(bool savepoint);
bool save_game(bool savepoint, int32_t type);
#endif
