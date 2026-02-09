#ifndef ZQ_CUSTOM_H_
#define ZQ_CUSTOM_H_

#include "items.h"

void large_dialog(DIALOG *d);
void large_dialog(DIALOG *d, float RESIZE_AMT);

struct list_data_struct
{
    char *s;
    int32_t i;
};

int32_t onCustomItems();

enum { ws_2_frame, ws_3_frame, ws_max };

int32_t onCustomWpns();
int32_t onCustomEnemies();
void edit_enemydata(int32_t index);
int32_t onCustomGuys();
int32_t d_ltile_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t onCustomHero();
void center_zq_custom_dialogs();

int32_t onMiscSprites();
int32_t onMiscSFX();

#endif
