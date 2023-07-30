//--------------------------------------------------------
//  ZQuest Classic
//  by Jeremy Craner, 1999-2000
//
//  gui.c
//
//  System functions, input handlers, GUI stuff, etc.
//  for ZQuest Classic.
//
//--------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "base/zc_alleg.h"
#include "base/zdefs.h"
#include "tiles.h"
#include "base/colors.h"
#include "pal.h"
#include "base/zsys.h"
#include "jwin.h"
#include "base/jwinfsel.h"
#include "base/gui.h"

extern int32_t zq_screen_w, zq_screen_h;
extern void comeback();
extern void go();

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
		blit(screen, bmp, dialog->x, dialog->y, 0, 0, dialog->w+1, dialog->h+1);
	}
	else
		*allegro_errno = ENOMEM;
		
	ret = do_zqdialog(dialog, focus_obj);
	
	if(bmp)
	{
		blit(bmp, screen, 0, 0, dialog->x, dialog->y, dialog->w+1, dialog->h+1);
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
	
	bool should_draw = true;
	int num_idle_frames = 0;
	while(update_dialog(player2))
	{
		if (player2->res & D_REDRAWME)
		{
			player2->res &= ~D_REDRAWME;
			should_draw = true;
		}
		if (should_draw)
		{
			should_draw = false;
			num_idle_frames = 0;
			update_hw_screen(true);
			al_rest(1. / 60);
			continue;
		}

		// Not perfect, but beats using 100% of CPU.
		// The above may miss things, so draw at least a few times a second.
		if (num_idle_frames++ == 15)
		{
			should_draw = 1;
		}
		al_rest(1. / 60);
	}
	
	int ret = shutdown_dialog(player2);

	popup_zqdialog_end();
	return ret;
}



/* popup_dialog:
 *  Like do_dialog(), but it stores the data on the screen before drawing
 *  the dialog and restores it when the dialog is closed. The screen area
 *  to be stored is calculated from the dimensions of the first object in
 *  the dialog, so all the other objects should lie within this one.
 */
int32_t popup_zqdialog(DIALOG *dialog, int32_t focus_obj)
{
	ASSERT(dialog);

	return do_zqdialog(dialog, focus_obj);
}

void new_gui_popup_dialog(DIALOG* dialog, int32_t focus_obj, bool& done, bool& running)
{
	running=true;
	int32_t ret=0;
	
	while(!done && ret>=0)
		ret=do_zqdialog(dialog, focus_obj);
	running=false;
}

