#ifndef _ZC_GUI_ALLEGRO_BITMAP_H_
#define _ZC_GUI_ALLEGRO_BITMAP_H_

#include "../bitmap.h"
#include "standardWidget.h"

namespace GUI
{

class AllegroBitmapWidget: public BitmapWidget, public StandardAllegroWidget
{
public:
    AllegroBitmapWidget(BitmapBackend* backend);
    void getPreferredSize(int& prefWidth, int& prefHeight);
    int realize(DIALOG dlgArray[], int index);
    void draw(AllegroGUIRenderer& renderer) const;
    inline void setHandleMouseInput(bool h) { handlingMouseInput=h; }
    
    static int proc(int msg, DIALOG* d, int c);
    
private:
    scoped_ptr<BitmapBackend> backend;
    bool handlingMouseInput;
    bool underMouse;
    int lastMouseX, lastMouseY;
    
    void onMouseInput(int msg, int c);
};

}

#endif
