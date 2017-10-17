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

int d_cstile_proc(int msg,DIALOG *d,int c);

/*****************************/
/******  onCustomItems  ******/
/*****************************/

void test_item(itemdata test);
void edit_itemdata(int index);
int onCustomItems();

/****************************/
/******  onCustomWpns  ******/
/****************************/

void edit_weapondata(int index);
int onCustomWpns();
int onCustomGuys();
int d_ltile_proc(int msg,DIALOG *d,int c);
int onCustomLink();
void center_zq_custom_dialogs();
#endif
