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
#include "mem_debug.h"

extern int32_t zq_screen_w, zq_screen_h;

//Don't really need this for anything at the moment?
//!TODO trim -Em
static size_t sp_acquire_ctr = 0;
void sp_acquire_screen()
{
	++sp_acquire_ctr;
	acquire_screen();
}
void sp_release_screen()
{
	if(sp_acquire_ctr)
	{
		--sp_acquire_ctr;
		release_screen();
	}
}
void sp_release_screen_all()
{
	while(sp_acquire_ctr)
	{
		--sp_acquire_ctr;
		release_screen();
	}
}

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

int32_t zc_do_dialog(DIALOG *d, int32_t f)
{
	int32_t ret=do_zqdialog(d,f);
	position_mouse_z(0);
	return ret;
}

int32_t zc_popup_dialog(DIALOG *d, int32_t f)
{
	int32_t ret=popup_zqdialog(d,f);
	position_mouse_z(0);
	return ret;
}

int32_t do_dialog_through_bitmap(BITMAP *buffer, DIALOG *dialog, int32_t focus_obj)
{
	BITMAP* orig_screen = screen;
	screen = buffer;
	
	int32_t ret=do_dialog(dialog, focus_obj);
	
	screen = orig_screen;
	blit(buffer, screen, 0, 0, 0, 0, screen->w, screen->h);
	position_mouse_z(0);
	
	return ret;
}

int32_t zc_popup_dialog_dbuf(DIALOG *dialog, int32_t focus_obj)
{
	BITMAP* buffer = create_bitmap_ex(get_color_depth(),SCREEN_H,SCREEN_W);
	blit(screen, buffer, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
	
	gui_set_screen(buffer);
	int32_t ret=popup_dialog(dialog, focus_obj);
	gui_set_screen(NULL);
	
	blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
	position_mouse_z(0);
	return ret;
}

int32_t PopUp_dialog(DIALOG *d,int32_t f)
{
	// uses the bitmap that's already allocated
	go();
	player = init_dialog(d,f);
	
	while(update_dialog(player))
	{
		/* do nothing */
	}
	
	int32_t ret = shutdown_dialog(player);
	comeback();
	position_mouse_z(0);
	return ret;
}

int32_t popup_dialog_through_bitmap(BITMAP *buffer, DIALOG *dialog, int32_t focus_obj)
{
	//these are here to bypass compiler warnings about unused arguments
	buffer=buffer;
	
	BITMAP *bmp;
	int32_t ret;
	
	bmp = create_bitmap_ex(bitmap_color_depth(screen),dialog->w+1, dialog->h+1);
	
	if(bmp)
	{
		scare_mouse();
		blit(screen, bmp, dialog->x, dialog->y, 0, 0, dialog->w+1, dialog->h+1);
		unscare_mouse();
	}
	else
		*allegro_errno = ENOMEM;
		
	ret = do_zqdialog(dialog, focus_obj);
	
	if(bmp)
	{
		scare_mouse();
		blit(bmp, screen, 0, 0, dialog->x, dialog->y, dialog->w+1, dialog->h+1);
		unscare_mouse();
		destroy_bitmap(bmp);
	}
	
	position_mouse_z(0);
	
	return ret;
}

int32_t PopUp_dialog_through_bitmap(BITMAP *buffer,DIALOG *d,int32_t f)
{
	// uses the bitmap that's already allocated
	go();
	player = init_dialog(d,f);
	
	while(update_dialog_through_bitmap(buffer,player))
	{
		/* do nothing */
	}
	
	int32_t ret = shutdown_dialog(player);
	comeback();
	position_mouse_z(0);
	return ret;
}

int32_t update_dialog_through_bitmap(BITMAP* buffer, DIALOG_PLAYER *the_player)
{
	BITMAP* orig_screen = screen;
	int32_t result;
	screen = buffer;
	result = update_dialog(the_player);
	screen = orig_screen;
	blit(buffer, screen, 0, 0, 0, 0, screen->w, screen->h);
	position_mouse_z(0);
	return result;
}

extern int32_t zqwin_scale;

int32_t do_zqdialog(DIALOG *dialog, int32_t focus_obj)
{
	BITMAP *mouse_screen = _mouse_screen;
	BITMAP *gui_bmp = screen;
	int32_t screen_count = _gfx_mode_set_count;
	DIALOG_PLAYER *player2;
	ASSERT(dialog);
	
	if(!is_same_bitmap(_mouse_screen, gui_bmp) && !(gfx_capabilities&GFX_HW_CURSOR))
	{
		show_mouse(gui_bmp);
	}
	
	player2 = init_dialog(dialog, focus_obj);
	
	while(update_dialog(player2))
	{
		/* If a menu is active, we yield here, since the dialog
		* engine is shut down so no user code can be running.
		*/
		update_hw_screen();
		
		//if (active_menu_player2)
		//rest(1);
	}
	
	if(_gfx_mode_set_count == screen_count && !(gfx_capabilities&GFX_HW_CURSOR))
	{
		show_mouse(mouse_screen);
	}
	
	return shutdown_dialog(player2);
}



/* popup_dialog:
 *  Like do_dialog(), but it stores the data on the screen before drawing
 *  the dialog and restores it when the dialog is closed. The screen area
 *  to be stored is calculated from the dimensions of the first object in
 *  the dialog, so all the other objects should lie within this one.
 */
int32_t popup_zqdialog(DIALOG *dialog, int32_t focus_obj)
{
	BITMAP *bmp;
	BITMAP *gui_bmp;
	int32_t ret;
	ASSERT(dialog);
	
	bmp = create_bitmap_ex(8, dialog->w, dialog->h);
	gui_bmp = screen;
	
	if(bmp)
	{
		scare_mouse_area(dialog->x, dialog->y, dialog->w, dialog->h);
		blit(gui_bmp, bmp, dialog->x, dialog->y, 0, 0, dialog->w, dialog->h);
		unscare_mouse();
	}
	else
	{
		*allegro_errno = ENOMEM;
	}
	
	ret = do_zqdialog(dialog, focus_obj);
	
	if(bmp)
	{
		scare_mouse_area(dialog->x, dialog->y, dialog->w, dialog->h);
		blit(bmp, gui_bmp, 0, 0, dialog->x, dialog->y, dialog->w, dialog->h);
		unscare_mouse();
		destroy_bitmap(bmp);
	}
	
	return ret;
}

void new_gui_popup_dialog(DIALOG* dialog, int32_t focus_obj, bool& done, bool& running)
{
	running=true;
	int32_t ret=0;
	broadcast_dialog_message(dialog, MSG_START, 0);
	broadcast_dialog_message(dialog, MSG_DRAW, 0);
	while(!done && ret>=0)
		ret=do_zqdialog(dialog, focus_obj);
	running=false;
}

static std::vector<BITMAP*> zqdialog_tmp_bmps;
void popup_zqdialog_start()
{
	BITMAP* tmp_bmp = create_bitmap_ex(8, zq_screen_w, zq_screen_h);
	
	if(tmp_bmp)
	{
		scare_mouse();
		blit(screen, tmp_bmp, 0, 0, 0, 0, zq_screen_w, zq_screen_h);
		unscare_mouse();
		zqdialog_tmp_bmps.push_back(tmp_bmp);
	}
	else
	{
		*allegro_errno = ENOMEM;
	}
}

void popup_zqdialog_end()
{
	if (zqdialog_tmp_bmps.size())
	{
		BITMAP* tmp_bmp = zqdialog_tmp_bmps.back();
		scare_mouse();
		blit(tmp_bmp, screen, 0, 0, 0, 0, zq_screen_w, zq_screen_h);
		unscare_mouse();
		destroy_bitmap(tmp_bmp);
		zqdialog_tmp_bmps.pop_back();
	}
	position_mouse_z(0);
}

/*** end of gui.cpp ***/
