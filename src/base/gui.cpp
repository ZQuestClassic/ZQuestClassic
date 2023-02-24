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

static BITMAP* saved_gui_bmp = nullptr;
void zc_set_gui_bmp(BITMAP* bmp)
{
	saved_gui_bmp = bmp;
}

BITMAP* zc_get_gui_bmp()
{
	return saved_gui_bmp;
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
	auto oz = gui_mouse_z();
	int32_t ret=do_zqdialog(d,f);
	position_mouse_z(oz);
	return ret;
}

int32_t zc_popup_dialog(DIALOG *d, int32_t f)
{
	auto oz = gui_mouse_z();
	int32_t ret=popup_zqdialog(d,f);
	position_mouse_z(oz);
	return ret;
}

int32_t do_dialog_through_bitmap(BITMAP *buffer, DIALOG *dialog, int32_t focus_obj)
{
	auto oz = gui_mouse_z();
	BITMAP* orig_screen = screen;
	screen = buffer;
	
	int32_t ret=do_dialog(dialog, focus_obj);
	
	screen = orig_screen;
	blit(buffer, screen, 0, 0, 0, 0, screen->w, screen->h);
	position_mouse_z(oz);
	
	return ret;
}

int32_t zc_popup_dialog_dbuf(DIALOG *dialog, int32_t focus_obj)
{
	auto oz = gui_mouse_z();
	BITMAP* buffer = create_bitmap_ex(get_color_depth(),screen->w, screen->h);
	blit(screen, buffer, 0, 0, 0, 0, screen->w, screen->h);
	
	gui_set_screen(buffer);
	int32_t ret=popup_dialog(dialog, focus_obj);
	gui_set_screen(NULL);
	
	blit(buffer, screen, 0, 0, 0, 0, screen->w, screen->h);
	position_mouse_z(oz);
	return ret;
}

int32_t PopUp_dialog(DIALOG *d,int32_t f)
{
	auto oz = gui_mouse_z();
	// uses the bitmap that's already allocated
	go();
	player = init_dialog(d,f);
	
	while(update_dialog(player))
	{
		/* do nothing */
		rest(1);
	}
	
	int32_t ret = shutdown_dialog(player);
	comeback();
	position_mouse_z(oz);
	return ret;
}

int32_t popup_dialog_through_bitmap(BITMAP *, DIALOG *dialog, int32_t focus_obj)
{
	auto oz = gui_mouse_z();
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
	
	position_mouse_z(oz);
	
	return ret;
}

int32_t PopUp_dialog_through_bitmap(BITMAP *buffer,DIALOG *d,int32_t f)
{
	auto oz = gui_mouse_z();
	// uses the bitmap that's already allocated
	go();
	player = init_dialog(d,f);
	
	while(update_dialog_through_bitmap(buffer,player))
	{
		/* do nothing */
		rest(1);
	}
	
	int32_t ret = shutdown_dialog(player);
	comeback();
	position_mouse_z(oz);
	return ret;
}

int32_t update_dialog_through_bitmap(BITMAP* buffer, DIALOG_PLAYER *the_player)
{
	auto oz = gui_mouse_z();
	BITMAP* orig_screen = screen;
	int32_t result;
	screen = buffer;
	result = update_dialog(the_player);
	screen = orig_screen;
	blit(buffer, screen, 0, 0, 0, 0, screen->w, screen->h);
	position_mouse_z(oz);
	return result;
}

int32_t do_zqdialog(DIALOG *dialog, int32_t focus_obj)
{
	DIALOG_PLAYER *player2;
	ASSERT(dialog);
	
	popup_zqdialog_start();
	
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
	
	popup_zqdialog_end();
	
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
	scare_mouse();
	acquire_screen();
	broadcast_dialog_message(dialog, MSG_START, 0);
	broadcast_dialog_message(dialog, MSG_DRAW, 0);
	release_screen();
	unscare_mouse();
	while(!done && ret>=0)
		ret=do_zqdialog(dialog, focus_obj);
	running=false;
}

BITMAP* zqdialog_bg_bmp = nullptr;
std::vector<BITMAP*> zqdialog_tmp_bmps;
std::vector<bool> zqdialog_bmp_trans;
void popup_zqdialog_start(bool transp)
{
	if(!zqdialog_bg_bmp)
		zqdialog_bg_bmp = screen;
	BITMAP* tmp_bmp = create_bitmap_ex(8, zq_screen_w, zq_screen_h);
	
	if(tmp_bmp)
	{
		clear_bitmap(tmp_bmp);
		show_mouse(tmp_bmp);
		screen = tmp_bmp;
		zqdialog_tmp_bmps.push_back(tmp_bmp);
		zqdialog_bmp_trans.push_back(transp);
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
		show_mouse(NULL);
		destroy_bitmap(zqdialog_tmp_bmps.back());
		zqdialog_tmp_bmps.pop_back();
		zqdialog_bmp_trans.pop_back();
		if(zqdialog_tmp_bmps.size())
			screen = zqdialog_tmp_bmps.back();
		else
		{
			screen = zqdialog_bg_bmp;
			zqdialog_bg_bmp = nullptr;
		}
		show_mouse(screen);
	}
	position_mouse_z(0);
}

void zqdialog_render(BITMAP* dest)
{
	clear_bitmap(dest);
	for(size_t q = 0; q < zqdialog_tmp_bmps.size(); ++q)
	{
		BITMAP* bmp = zqdialog_tmp_bmps[q];
		if(zqdialog_bmp_trans[q])
			masked_blit(bmp, dest, 0, 0, 0, 0, bmp->w, bmp->h);
		else blit(bmp, dest, 0, 0, 0, 0, bmp->w, bmp->h);
	}
}

/*** end of gui.cpp ***/
