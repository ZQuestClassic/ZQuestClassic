#ifndef _ZC_DIALOG_BITMAP_TILEPREVIEW_H_
#define _ZC_DIALOG_BITMAP_TILEPREVIEW_H_

#include "../../gui/bitmap.h"

#include <types.h>

class TilePreviewBitmap: public GUI::BitmapBackend
{
public:
    TilePreviewBitmap();
    TilePreviewBitmap(int tile, int cset, Orientation orientation);
    void initialize();
    void redraw();
    void setCSet(int cset);
    void setTile(int tile);
    void flipHorizontal();
    void flipVertical();
    void rotate(Orientation::rotation rot);
    
    inline int getTile() const { return tile; }
    inline int getCSet() const { return cset; }
    inline Orientation getOrientation() const { return orientation; }
    
private:
    int tile;
    int cset;
    Orientation orientation;
    bool needsRedrawn;
};

#endif
