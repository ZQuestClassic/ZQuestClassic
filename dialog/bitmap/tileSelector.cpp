#include "tileSelector.h"
#include "../zquest/tileSelectorBackend.h"
#include <types.h>
#include <util.h>
#include <gui/mouse.h>
#include <algorithm>

TileSelectorBitmap::TileSelectorBitmap(TileSelectorBackend& be):
    backend(be),
    cursorTimer(0)
{
}

void TileSelectorBitmap::initialize()
{
    renderer->initialize(backend.pageWidth*16, backend.pageHeight*16);
    widget->setHandleMouseInput(true);
}

void TileSelectorBitmap::redraw()
{
    renderer->clear();
    cursorTimer=(cursorTimer+1)%20;
    
    int startTile=backend.getFirstTileOnPage();
    for(int x=0; x<backend.pageWidth; x++)
    {
        for(int y=0; y<backend.pageHeight; y++)
        {
            renderer->putTile(backend.getTileAt(x, y), backend.getCSet(),
              0, x*16, y*16);
            
            if(cursorTimer<10 &&
              backend.tileIsSelected(x, y))
                renderer->drawRect(x*16, y*16, 16, 16, 1);
        }
    }
}

void TileSelectorBitmap::onMouseInput(const MouseInput& mouse)
{
    switch(mouse.type)
    {
    case mouse_LEFTBUTTONDOWN:
        backend.selectTile(
          backend.getTileAt(mouse.x/16, mouse.y/16),
          mouse.shiftPressed);
        break;
        
    case mouse_WHEELUP:
        backend.modPage(-1, mouse.shiftPressed);
        break;
        
    case mouse_WHEELDOWN:
        backend.modPage(1, mouse.shiftPressed);
        break;
    }
}
