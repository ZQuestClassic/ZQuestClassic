//--------------------------------------------------------
//  ZQuest Classic
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
#include "base/zc_alleg.h"
#include <string>

struct gamedata;
struct zinitdata;

#define MAXINITTABS 5

typedef struct item_class_struct
{
    char *s;
    int32_t i;
} item_class_struct;

extern const char *itype_names[itype_max];
extern const char *itype_new_names[itype_max];

int32_t d_line_proc(int32_t msg, DIALOG *d, int32_t c);
int32_t d_maxbombsedit_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t d_bombratio_proc(int32_t msg,DIALOG *d,int32_t c);
extern TABPANEL init_tabs[];
int32_t doInit(zinitdata *zinit, bool isZC);
void resetItems(gamedata *data, zinitdata *zinit, bool freshquest);

std::string serialize_init_data_delta(zinitdata *base, zinitdata *changed);
zinitdata *apply_init_data_delta(zinitdata *base, std::string delta);
#endif

