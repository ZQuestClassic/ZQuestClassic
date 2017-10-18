#include "tilePreview.h"

TilePreviewBitmap::TilePreviewBitmap():
    tile(0),
    cset(0),
    orientation(0),
    needsRedrawn(true)
{
}

TilePreviewBitmap::TilePreviewBitmap(int t, int cs, Orientation o):
    tile(t),
    cset(cs),
    orientation(o),
    needsRedrawn(true)
{
}

void TilePreviewBitmap::initialize()
{
    renderer->initialize(16, 16);
}

void TilePreviewBitmap::redraw()
{
    if(needsRedrawn)
    {
        renderer->putTile(tile, cset, orientation, 0, 0);
        needsRedrawn=false;
    }
}

void TilePreviewBitmap::setCSet(int cs)
{
    cset=cs;
    needsRedrawn=true;
}

void TilePreviewBitmap::setTile(int t)
{
    tile=t;
    needsRedrawn=true;
}

void TilePreviewBitmap::flipHorizontal()
{
    orientation.flipHorizontal();
    needsRedrawn=true;
}

void TilePreviewBitmap::flipVertical()
{
    orientation.flipVertical();
    needsRedrawn=true;
}

void TilePreviewBitmap::rotate(Orientation::rotation rot)
{
    orientation.rotate(rot);
    needsRedrawn=true;
}
