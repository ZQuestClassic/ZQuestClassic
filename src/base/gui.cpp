#include <stdio.h>
#include <stdlib.h>
#include <cstring>
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

int32_t do_zqdialog(DIALOG *dialog, int32_t focus_obj, bool checkexit)
{
	DIALOG_PLAYER *player2;
	ASSERT(dialog);
	
	popup_zqdialog_start();
	
	player2 = init_dialog(dialog, focus_obj);
	
	bool should_draw = true;
	int num_idle_frames = 0;
	while(update_dialog(player2))
	{
		if(checkexit)
		{
			HANDLE_CLOSE_ZQDLG();
			if(exiting_program)
			{
				popup_zqdialog_end();
				return -1;
			}
		}
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
int32_t do_zqdialog_custom(DIALOG *dialog, int32_t focus_obj, bool checkexit, std::function<bool(int)> proc)
{
	DIALOG_PLAYER *player2;
	ASSERT(dialog);
	int ret;
	
	popup_zqdialog_start();
	
	while(true)
	{
		player2 = init_dialog(dialog, focus_obj);
		
		bool should_draw = true;
		int num_idle_frames = 0;
		while(update_dialog(player2))
		{
			if(checkexit)
			{
				HANDLE_CLOSE_ZQDLG();
				if(exiting_program)
				{
					proc(-1);
					popup_zqdialog_end();
					return -1;
				}
			}
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
		
		ret = shutdown_dialog(player2);
		
		if(proc(ret))
			break;
	}
	
	popup_zqdialog_end();
	return ret;
}

void new_gui_popup_dialog(DIALOG* dialog, int32_t focus_obj, bool& done, bool& running)
{
	running=true;
	int32_t ret=0;
	
	while(!done && ret>=0)
		ret=do_zqdialog(dialog, focus_obj);
	running=false;
}
