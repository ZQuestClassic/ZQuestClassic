//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zquest.cc
//
//  Main code for the quest editor.
//
//--------------------------------------------------------

#include "precompiled.h" //always first

#include <stdio.h>

#include "base/gui.h"
#include "init.h"
#include "base/zc_alleg.h"
#include "base/zdefs.h"
#include "jwin.h"
#include "zq/zq_init.h"
#include "base/zsys.h"
#include "zq/zquest.h"

#define zc_min(a,b)  ((a)<(b)?(a):(b))

extern int32_t dmap_list_size;
extern bool dmap_list_zero;
extern char *item_string[];
extern const char *itype_names[];

extern int32_t doInit(zinitdata *zinit);

int32_t onInit()
{
    dmap_list_size=MAXDMAPS;
    dmap_list_zero=true;
    return doInit(&zinit, false);
}

void onInitOK()
{
    saved=false;
}

