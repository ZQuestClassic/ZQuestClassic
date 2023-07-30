//--------------------------------------------------------
//  ZQuest Classic
//  by Jeremy Craner, 1999-2000
//
//  title.h
//
//  Title screen and intro for zelda.cc
//  Also has game loading and select screen code.
//
//--------------------------------------------------------

#ifndef _ZC_TITLE_H_
#define _ZC_TITLE_H_

extern DIALOG gamemode_dlg[];

int32_t init_saves();
int32_t  load_savedgames();
int32_t  save_savedgames();
int32_t custom_game(int32_t file);
int32_t getsaveslot();
void load_game_icon(gamedata *g, bool forceDefault, int32_t index);
void load_game_icon_to_buffer(bool forceDefault, int32_t index);
void load_game_icon_to_buffer_manual(bool forceDefault, int32_t index, int32_t ring_value);
void reload_icon_buffer(int32_t index);
void titlescreen(int32_t lsave);
void game_over(int32_t type);
void save_game(bool savepoint);
bool save_game(bool savepoint, int32_t type);
#endif

