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

void large_dialog(DIALOG *d);
void large_dialog(DIALOG *d, float RESIZE_AMT);
int d_cstile_proc(int msg,DIALOG *d,int c);
int jwin_as_droplist_proc(int msg,DIALOG *d,int c);
void build_biwt_list();
void build_bief_list();
void build_biic_list();
void build_biea_list();
void build_biew_list();

struct list_data_struct
{
    char *s;
    int i;
};

/*****************************/
/******  onCustomItems  ******/
/*****************************/

void test_item(itemdata test);
void edit_itemdata(int index);
int onCustomItems();

/****************************/
/******  onCustomWpns  ******/
/****************************/

enum { ws_2_frame, ws_3_frame, ws_max };

void edit_weapondata(int index);
int onCustomWpns();
int onCustomEnemies();
void edit_enemydata(int index);
int onCustomGuys();
int d_ltile_proc(int msg,DIALOG *d,int c);
int onCustomLink();
void center_zq_custom_dialogs();

/****************************/
/******  onMiscSprites ******/
/****************************/

int onMiscSprites();

#endif

