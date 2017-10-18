#ifndef _ZC_DIALOG_BITMAP_TILESELECTOR_H_
#define _ZC_DIALOG_BITMAP_TILESELECTOR_H_

#include <gui/bitmap.h>
class TileSelectorBackend;

class TileSelectorBitmap: public GUI::BitmapBackend
{
public:
    TileSelectorBitmap(TileSelectorBackend& backend);
    void initialize();
    void redraw();
    void onMouseInput(const MouseInput& mouse);
    
private:
    TileSelectorBackend& backend;
    int cursorTimer;
};

#endif
