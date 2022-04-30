//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zquest.cc
//
//  Main code for the quest editor.
//
//--------------------------------------------------------

#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include "precompiled.h" //always first

#include <stdio.h>

#include "gui.h"
#include "init.h"
#include "zc_alleg.h"
#include "zdefs.h"
#include "jwin.h"
#include "zq_init.h"
#include "zsys.h"
#include "zquest.h"

//#ifdef ALLEGRO_MACOSX
#define zc_min(a,b)  ((a)<(b)?(a):(b))
//#endif

extern char *item_string[];
extern const char *itype_names[];

extern int32_t doInit(zinitdata *zinit);

int32_t onInit()
{
    return doInit(&zinit, false);
}

void onInitOK()
{
    saved=false;
}

