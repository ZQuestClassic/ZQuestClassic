//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zquest.cc
//
//  Main code for the quest editor.
//
//--------------------------------------------------------
#ifndef _INIT_H_
#define _INIT_H_

#include "tab_ctl.h"
#include "zc_alleg.h"

int d_line_proc(int msg, DIALOG *d, int c);
int d_maxbombsedit_proc(int msg,DIALOG *d,int c);
extern DIALOG init_dlg[];
extern TABPANEL init_tabs[];
int doInit(zinitdata *zinit);
void resetItems(gamedata *data, zinitdata *zinit);
#endif
