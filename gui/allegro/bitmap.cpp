#include "bitmap.h"
#include "common.h"
#include "renderer.h"
#include "../mouse.h"
#include <scoped_ptr.h>

namespace GUI
{

int AllegroBitmapWidget::proc(int msg, DIALOG* d, int c)
{
    AllegroBitmapWidget* bmp=static_cast<AllegroBitmapWidget*>(d->dp);
    
    switch(msg)
    {
    case MSG_START:
        bmp->underMouse=dialogUnderMouse(d);
        bmp->lastMouseX=gui_mouse_x();
        bmp->lastMouseY=gui_mouse_y();
        break;
        
    case MSG_WANTFOCUS:
        return D_WANTFOCUS;
        
    case MSG_GOTMOUSE:
        bmp->underMouse=true;
        break;
        
    case MSG_LOSTMOUSE:
        bmp->underMouse=true;
        break;
        
    case MSG_LPRESS:
    case MSG_LRELEASE:
    case MSG_RPRESS:
    case MSG_RRELEASE:
    case MSG_WHEEL:
    case MSG_IDLE: // There's no mouse motion event...
        if(bmp->handlingMouseInput)
            bmp->onMouseInput(msg, c);
        break;
    }
    
    return D_O_K;
}

AllegroBitmapWidget::AllegroBitmapWidget(BitmapBackend* be):
    backend(be),
    handlingMouseInput(false),
    underMouse(false),
    lastMouseX(gui_mouse_x()),
    lastMouseY(gui_mouse_y())
{
    backend->setWidget(this);
    backend->initialize();
}

void AllegroBitmapWidget::getPreferredSize(int& prefWidth, int& prefHeight)
{
    backend->getSize(prefWidth, prefHeight);
}

int AllegroBitmapWidget::realize(DIALOG dlgArray[], int index)
{
    return realizeAs<AllegroBitmapWidget>(dlgArray, index);
}

void AllegroBitmapWidget::draw(AllegroGUIRenderer& renderer) const
{
    renderer.setOrigin(x, y);
    backend->redraw();
    renderer.blitBitmap(backend->getBitmap(), 0, 0);
}

void AllegroBitmapWidget::onMouseInput(int msg, int c)
{
    if(msg==MSG_IDLE && lastMouseX==gui_mouse_x() && lastMouseY==gui_mouse_y())
        return;
    
    MouseInput mi;
    translateMouseInput(msg, c, x, y, backend->getScale(), mi);
    backend->onMouseInput(mi);
    lastMouseX=gui_mouse_x();
    lastMouseY=gui_mouse_y();
}

}
