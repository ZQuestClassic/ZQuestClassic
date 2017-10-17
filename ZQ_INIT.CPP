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

extern int dmap_list_size;
extern bool dmap_list_zero;
extern char *item_string[];
extern const char *itype_names[];

extern int doInit(zinitdata *zinit);

int onInit()
{
	char init_title[80];
	sprintf(init_title, "Initialization Data");
	init_dlg[0].dp=init_title;

	dmap_list_size=MAXDMAPS;
	dmap_list_zero=true;
	init_dlg[1656].d1 = zinit.start_dmap;
	return doInit(&zinit);
}

void center_zq_init_dialog()
{
  jwin_center_dialog(init_dlg);
}

void onInitOK()
{
	saved=false;
}
 
