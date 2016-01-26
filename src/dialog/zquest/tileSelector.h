#ifndef _ZC_DIALOG_ZQUEST_TILESELECTOR_H_
#define _ZC_DIALOG_ZQUEST_TILESELECTOR_H_

#include <gui/dialog.h>
#include "tileSelectorBackend.h"

#include "../bitmap/tilePreview.h"
#include <types.h>
class TileSelectorBitmap;
//class TilePreviewBitmap;
namespace GUI
{
    class Spinner;
    class Text;
}

class TileSelector: public GUI::Dialog
{
public:
    TileSelector(int tile, int cset, Orientation orientation);
    GUI::Widget* createDialog(const GUI::WidgetFactory& f);
    bool onKeyPressed(const KeyInput& key);
    void onOK();
    
    // Used to retrieve the user's selection after the dialog is closed.
    inline bool userCanceled() const { return cancel; }
    inline int getTile() const { return backend.getTile(); }
    inline int getCSet() const { return backend.getCSet(); }
    inline int getOrientation() const { return currTileBmp->getOrientation(); }
    
private:
    TileSelectorBackend backend;
    TileSelectorBitmap* tsBmp;
    TilePreviewBitmap* currTileBmp;
    TilePreviewBitmap* tileBmp2; // Old or copied tile
    GUI::Spinner* pageSelector;
    GUI::Text* currTileText;
    GUI::Text* currCSetText;
    bool cancel;
    
    void onTileChanged();
    void onCSetChanged();
    void onPageChanged();
    
    friend class TileSelectorBackend;
};

#endif
