#ifndef _ZC_GUI_ALLEGRO_CONTROLLER_H_
#define _ZC_GUI_ALLEGRO_CONTROLLER_H_

#include "../controller.h"
#include "renderer.h"
struct BITMAP;
struct DIALOG;
struct DIALOG_PLAYER;

namespace GUI
{

class AllegroWidget;

class AllegroDialogController: public DialogController
{
public:
    AllegroDialogController();
    ~AllegroDialogController();
    
    void setDialogRoot(Widget* root);
    void initialize();
    void update();
    void shutDown();
    inline bool isOpen() { return open; }
    
    /// Runs a widget as the root of a new dialog over the current one.
    void beginSubDialog(AllegroWidget* root);
    void closeSubDialog(bool safe=false);
    
private:
    struct BackupBitmap
    {
        BITMAP* bmp;
        int x, y;
    };
    
    AllegroWidget* root;
    AllegroWidget* subRoot;
    DIALOG* allegDlg;
    DIALOG* subAllegDlg;
    DIALOG_PLAYER* player;
    DIALOG_PLAYER* subPlayer;
    bool open;
    bool updating;
    BackupBitmap savedBG;
    AllegroGUIRenderer renderer;
    
    void deleteDialogData();
    void restoreBG();
};

}

#endif
