//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zquest.cc
//
//  Main code for the quest editor.
//
//--------------------------------------------------------

#ifndef _ZC_INIT_H_
#define _ZC_INIT_H_

int onCheatConsole();
void center_zq_init_dialog();
void resetItems(gamedata *data, zinitdata *zinit);
zinitdata *copyIntoZinit(gamedata *src);
void resetItems(gamedata *game);
#endif
