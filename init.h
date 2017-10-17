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

#define MAXINITTABS 5

typedef struct item_class_struct
{
  char *s;
  int i;
} item_class_struct;

extern item_class_struct biic[itype_max];
extern const char *itype_names[itype_max];

extern int biic_cnt;
void build_biic_list();
void deallocate_biic_list();
const char *item_class_list(int index, int *list_size);

int jwin_initlist_proc(int msg,DIALOG *d,int c);
int d_line_proc(int msg, DIALOG *d, int c);
int d_maxbombsedit_proc(int msg,DIALOG *d,int c);
int d_bombratio_proc(int msg,DIALOG *d,int c);
extern DIALOG init_dlg[];
extern TABPANEL init_tabs[];
int doInit(zinitdata *zinit);
void resetItems(gamedata *data, zinitdata *zinit, bool lvlitems);
#endif
 
