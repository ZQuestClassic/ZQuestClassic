//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  gui.c
//
//  System functions, input handlers, GUI stuff, etc.
//  for Zelda Classic.
//
//--------------------------------------------------------

#include "precompiled.h" //always first

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
//#include <dir.h>
#include <ctype.h>
#include "base/zc_alleg.h"

#ifdef ALLEGRO_DOS
#include <unistd.h>
#endif

#include "base/zdefs.h"
#include "zelda.h"
#include "tiles.h"
#include "base/colors.h"
#include "pal.h"
#include "base/zsys.h"
#include "qst.h"
#include "zc_sys.h"
#include "debug.h"
#include "jwin.h"
#include "base/jwinfsel.h"
#include "base/gui.h"

extern int32_t zq_screen_w, zq_screen_h;

void broadcast_dialog_message(DIALOG* dialog, int32_t msg, int32_t c)
{
	while(dialog->proc)
	{
		object_message(dialog++, msg, c);
	}
}

/****************************/
/**********  GUI  ***********/
/****************************/

// make it global so the joystick button routine can set joy_on=TRUE
DIALOG_PLAYER *player = NULL;

int new_popup_dlg(DIALOG* dialog, int32_t focus_obj)
{
	DIALOG_PLAYER *player2 = init_dialog(dialog, focus_obj);
	
	while(update_dialog(player2))
		update_hw_screen();
	
	return shutdown_dialog(player2);
}

int do_zqdialog(DIALOG *dialog, int focus_obj)
{
	DIALOG_PLAYER *player2;
	ASSERT(dialog);
	
	popup_zqdialog_start(0,0,-1,-1,0xFF);
	
	player2 = init_dialog(dialog, focus_obj);
	
	while(update_dialog(player2))
		update_hw_screen();
	
	int ret = shutdown_dialog(player2);

	popup_zqdialog_end();
	return ret;
}

void new_gui_popup_dialog(DIALOG* dialog, int32_t focus_obj, bool& done, bool& running)
{
	running=true;
	int32_t ret=0;
	
	ASSERT(dialog);
	while(!done && ret>=0)
	{
		DIALOG_PLAYER *player2 = init_dialog(dialog, focus_obj);
		
		while(update_dialog(player2))
			update_hw_screen();
		
		ret = shutdown_dialog(player2);
	}
	running=false;
}

int popup_menu(MENU *menu,int x,int y)
{
	while(gui_mouse_b())
		rest(1);
	
	popup_zqdialog_start_a5();
	auto ret = jwin_do_menu(menu,x,y);
	popup_zqdialog_end_a5();
	return ret;
}
int popup_menu_abs(MENU *menu,int x,int y)
{
	int ox,oy,ow,oh;
	get_zqdialog_offset(ox,oy,ow,oh);
	return popup_menu(menu,x+ox,y+oy);
}

/*** end of gui.cpp ***/
