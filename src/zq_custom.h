//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zq_custom.cc
//
//  Custom item, enemy, etc. for ZQuest.
//
//--------------------------------------------------------

#ifndef _ZQ_CUSTOM_H_
#define _ZQ_CUSTOM_H_

#include "quest/ItemDefinitionTable.h"

int d_cstile_proc(int msg,DIALOG *d,int c);
int jwin_as_droplist_proc(int msg,DIALOG *d,int c);

struct list_data_struct
{
    char *s;
    int i;
};

/*****************************/
/******  onCustomItems  ******/
/*****************************/

void test_item(itemdata test);
void edit_itemdata(ItemDefinitionRef index);
int onCustomItems();

/****************************/
/******  onCustomWpns  ******/
/****************************/

enum { ws_2_frame, ws_3_frame, ws_max };

void edit_weapondata(const SpriteDefinitionRef &index);
int onCustomWpns();
int onCustomEnemies();
void edit_enemydata(const EnemyDefinitionRef &index);
int onCustomGuys();
int d_ltile_proc(int msg,DIALOG *d,int c);
int onCustomLink();
void center_zq_custom_dialogs();
#endif

