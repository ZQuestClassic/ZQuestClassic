//--------------------------------------------------------
//  Zelda Classic
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

int  load_savedgames();
int  save_savedgames();
int custom_game(int file);
int getsaveslot();
void load_game_icon(gamedata *g, bool forceDefault, int index);
void load_game_icon_to_buffer(bool forceDefault, int index);
void titlescreen(int lsave);
void game_over(int type);
void save_game(bool savepoint);
void save_game(bool savepoint, int type);
#endif

