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
#include "zc_alleg.h"

#ifdef ALLEGRO_DOS
#include <unistd.h>
#endif

#include "zdefs.h"
#include "zelda.h"
#include "zquest.h"
#include "tiles.h"
#include "colors.h"
#include "pal.h"
#include "zsys.h"
#include "qst.h"
#include "zc_sys.h"
#include "debug.h"
#include "jwin.h"
#include "jwinfsel.h"
#include "gui.h"
#include "mem_debug.h"
#include "backend/AllBackends.h"

/****************************/
/**********  GUI  ***********/
/****************************/

int miniscreenX() { return (Backend::graphics->virtualScreenW() - 320) / 2; }
int miniscreenY() { return (Backend::graphics->virtualScreenH() - 240) / 2; }

void saveMiniscreen()
{
	blit(screen, tmp_scr, miniscreenX(),miniscreenY(), 0, 0, 320, 240);
}

void restoreMiniscreen()
{
	blit(tmp_scr, screen, 0, 0, miniscreenX(), miniscreenY(), 320, 240);
}


// make it global so the joystick button routine can set joy_on=TRUE
DIALOG_PLAYER *player = NULL;

int zc_do_dialog(DIALOG *d, int f)
{
    int ret=do_zqdialog(d,f);
	Backend::mouse->setWheelPosition(0);
    return ret;
}

int zc_popup_dialog(DIALOG *d, int f)
{
    int ret=popup_zqdialog(d,f);
	Backend::mouse->setWheelPosition(0);
    return ret;
}

int do_dialog_through_bitmap(BITMAP *buffer, DIALOG *dialog, int focus_obj)
{
    BITMAP* orig_screen = screen;
    screen = buffer;
    
    int ret=do_dialog(dialog, focus_obj);
    
    screen = orig_screen;
    blit(buffer, screen, 0, 0, 0, 0, screen->w, screen->h);
	Backend::mouse->setWheelPosition(0);
    
    return ret;
}

int zc_popup_dialog_dbuf(DIALOG *dialog, int focus_obj)
{
    BITMAP* buffer = create_bitmap_ex(get_color_depth(),SCREEN_H,SCREEN_W);
    blit(screen, buffer, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
    
    gui_set_screen(buffer);
    int ret=popup_dialog(dialog, focus_obj);
    gui_set_screen(NULL);
    
    blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
	Backend::mouse->setWheelPosition(0);
    return ret;
}

int PopUp_dialog(DIALOG *d,int f)
{
    // uses the bitmap that's already allocated
	saveMiniscreen();
    player = init_dialog(d,f);
    
    while(update_dialog(player))
    {
		Backend::graphics->waitTick();
		Backend::graphics->showBackBuffer();
    }
    
    int ret = shutdown_dialog(player);
	restoreMiniscreen();
	Backend::mouse->setWheelPosition(0);
    return ret;
}

int popup_dialog_through_bitmap(BITMAP *buffer, DIALOG *dialog, int focus_obj)
{
    //these are here to bypass compiler warnings about unused arguments
    buffer=buffer;
    
    BITMAP *bmp;
    int ret;
    
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
    
	Backend::mouse->setWheelPosition(0);
    
    return ret;
}

int PopUp_dialog_through_bitmap(BITMAP *buffer,DIALOG *d,int f)
{
    // uses the bitmap that's already allocated
	saveMiniscreen();
    player = init_dialog(d,f);
    
    while(update_dialog_through_bitmap(buffer,player))
    {
		Backend::graphics->waitTick();
		Backend::graphics->showBackBuffer();
    }
    
    int ret = shutdown_dialog(player);
	restoreMiniscreen();
	Backend::mouse->setWheelPosition(0);
    return ret;
}

int update_dialog_through_bitmap(BITMAP* buffer, DIALOG_PLAYER *the_player)
{
    BITMAP* orig_screen = screen;
    int result;
    screen = buffer;
    result = update_dialog(the_player);
    screen = orig_screen;
    blit(buffer, screen, 0, 0, 0, 0, screen->w, screen->h);
	Backend::mouse->setWheelPosition(0);
    return result;
}

extern int zqwin_scale;

int do_zqdialog(DIALOG *dialog, int focus_obj)
{
    BITMAP *gui_bmp = screen;
    DIALOG_PLAYER *player2;
    ASSERT(dialog);
    
    player2 = init_dialog(dialog, focus_obj);
    
    while(update_dialog(player2))
    {
        /* If a menu is active, we yield here, since the dialog
        * engine is shut down so no user code can be running.
        */
		Backend::graphics->waitTick();
		Backend::graphics->showBackBuffer();
    }
    
    return shutdown_dialog(player2);
}



/* popup_dialog:
 *  Like do_dialog(), but it stores the data on the screen before drawing
 *  the dialog and restores it when the dialog is closed. The screen area
 *  to be stored is calculated from the dimensions of the first object in
 *  the dialog, so all the other objects should lie within this one.
 */
int popup_zqdialog(DIALOG *dialog, int focus_obj)
{
    BITMAP *bmp;
    BITMAP *gui_bmp;
    int ret;
    ASSERT(dialog);
    
    bmp = create_bitmap_ex(8, dialog->w, dialog->h);
    gui_bmp = screen;
    
    if(bmp)
    {
        blit(gui_bmp, bmp, dialog->x, dialog->y, 0, 0, dialog->w, dialog->h);
    }
    else
    {
        *allegro_errno = ENOMEM;
    }
    
    ret = do_zqdialog(dialog, focus_obj);
    
    if(bmp)
    {
        blit(bmp, gui_bmp, 0, 0, dialog->x, dialog->y, dialog->w, dialog->h);
        destroy_bitmap(bmp);
		Backend::graphics->showBackBuffer();
    }
    
    return ret;
}

/*** end of gui.cpp ***/

