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
void load_game_icon(gamedata *g);
void titlescreen();
void game_over();
#endif
