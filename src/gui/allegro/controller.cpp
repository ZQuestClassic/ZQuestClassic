#include "controller.h"
#include "common.h"
#include "widget.h"
#include "../key.h"
#include <allegro.h>
#include <cstring>

extern BITMAP *hw_screen; // Because ZQuest replaces screen...

namespace GUI
{

// This catches any keyboard input not handled by widgets.
static int handleKeyProc(int msg, DIALOG* d, int c)
{
    if(msg==MSG_XCHAR)
    {
        KeyInput ki;
        if(!translateKeyInput(c, ki))
            return D_O_K;
        AllegroDialogController* c=static_cast<AllegroDialogController*>(d->dp);
        if(c->onKeyPressed(ki))
            return D_USED_CHAR;
    }
    
    return D_O_K;
}

AllegroDialogController::AllegroDialogController():
    root(0),
    subRoot(0),
    allegDlg(0),
    subAllegDlg(0),
    player(0),
    subPlayer(0),
    open(false)
{
    savedBG.bmp=0;
    savedBG.x=0;
    savedBG.y=0;
}

AllegroDialogController::~AllegroDialogController()
{
    if(open)
    {
        if(subPlayer)
        {
            shutdown_dialog(subPlayer);
            delete[] subAllegDlg;
        }
        
        shutdown_dialog(player);
        delete[] allegDlg;
    }
    
    if(savedBG.bmp)
        destroy_bitmap(savedBG.bmp);
    
    delete root;
}

void AllegroDialogController::setDialogRoot(Widget* r)
{
    root=dynamic_cast<AllegroWidget*>(r);
    root->setController(this);
}

void AllegroDialogController::initialize()
{
    // Allocate an array big enough for all widgets, the keyboard handler,
    // and the terminator
    int dialogSize=root->getDataSize()+2;
    allegDlg=new DIALOG[dialogSize];
    std::memset(allegDlg, 0, dialogSize*sizeof(DIALOG));
    
    // Load up widgets
    int x, y, w, h;
    root->getPreferredSize(w, h);
    x=(SCREEN_W-w)/2;
    y=(SCREEN_H-h)/2;
    root->setSizeAndPos(x, y, w, h);
    root->realize(allegDlg, 0);
    
    // Set up the keyboard handler
    allegDlg[dialogSize-2].proc=handleKeyProc;
    allegDlg[dialogSize-2].dp=this;
    
    // Save the background to redraw when the dialog is closed
    // (... Is this actually useful?)
    savedBG.bmp=create_bitmap_ex(8, w, h);
    savedBG.x=x;
    savedBG.y=y;
    blit(screen, savedBG.bmp, x, y, 0, 0, w, h);
    
    // And start the dialog
    player=init_dialog(allegDlg, -1);
    open=true;
    updating=false;
}

void AllegroDialogController::update()
{
    if(open)
    {
        updating=true;
        bool ret;
        if(subPlayer)
            ret=update_dialog(subPlayer);
        else
            ret=update_dialog(player);
        
        // Undo any clipping before drawing or quitting
        screen->ct=0;
        screen->cb=screen->h-1;
        screen->cl=0;
        screen->cr=screen->w-1;
        
        if(ret)
        {
            if(subPlayer && !subRoot) // Sub-dialog was closed
                closeSubDialog(true);
            blit(screen, hw_screen, 0, 0, 0, 0, screen->w, screen->h);
            rest(1); // This should probably be moved...
        }
        else
        {
            // User pressed escape
            if(subPlayer)
                closeSubDialog(true);
            else
                open=false;
        }
        
        if(open)
        {
            if(subRoot)
                subRoot->draw(renderer);
            else
                root->draw(renderer);
        }
        else
        {
            // Escape was pressed or the dialog was closed mid-update
            deleteDialogData();
            restoreBG();
        }
        
        updating=false;
    }
}

void AllegroDialogController::shutDown()
{
    if(open)
    {
        open=false;
        
        // If updating is true, the dialog was shut down by a dialog element.
        // Deleting everything in the middle of an update would be a bad idea.
        if(!updating)
        {
            deleteDialogData();
            restoreBG();
        }
    }
    
    position_mouse_z(0);
}

void AllegroDialogController::deleteDialogData()
{
    if(subPlayer)
    {
        shutdown_dialog(subPlayer);
        subPlayer=0;
        delete[] subAllegDlg;
    }
    
    shutdown_dialog(player);
    player=0;
    delete[] allegDlg;
    allegDlg=0;
    open=false;
}

void AllegroDialogController::restoreBG()
{
    blit(savedBG.bmp, screen, 0, 0, savedBG.x, savedBG.y, savedBG.bmp->w, savedBG.bmp->h);
    destroy_bitmap(savedBG.bmp);
    savedBG.bmp=0;
}

void AllegroDialogController::beginSubDialog(AllegroWidget* r)
{
    subRoot=r;
    subRoot->setController(this); // Shouldn't be necessary...
    
    int size=subRoot->getDataSize()+1;
    subAllegDlg=new DIALOG[size]; // Still needs one more to catch clicks outside the dialog
    std::memset(subAllegDlg, 0, size*sizeof(DIALOG));
    subRoot->realize(subAllegDlg, 0);
    subPlayer=init_dialog(subAllegDlg, -1);
}

void AllegroDialogController::closeSubDialog(bool safe)
{
    if(subRoot)
        subRoot->unrealize();
    subRoot=0;
    
    if(updating && !safe)
        // Still in mid-update; don't shut down yet
        return;
    
    shutdown_dialog(subPlayer);
    subPlayer=0;
    delete[] subAllegDlg;
    subAllegDlg=0;
}

}
