#include <stdio.h>

#include "base/gui.h"
#include "init.h"
#include "base/zc_alleg.h"
#include "base/zdefs.h"
#include "base/initdata.h"
#include "jwin.h"
#include "zq/zq_init.h"
#include "base/zsys.h"
#include "zq/zquest.h"

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
