#ifndef _ZC_DIALOG_ZQUEST_TILESELECTORBACKEND_H_
#define _ZC_DIALOG_ZQUEST_TILESELECTORBACKEND_H_

class TileSelector;

class TileSelectorBackend
{
public:
    enum cursorMoveType
    {
        cm_UP, cm_DOWN, cm_LEFT, cm_RIGHT, cm_START, cm_END
    };
    
    static const int pageWidth=20;
    static const int pageHeight=13;
    static const int pageSize=20*13;
    static const int lastPage=251;
    static const int lastTile=65519;
    
    TileSelectorBackend(TileSelector& dialog, int tile, int cset);
    void moveCursor(cursorMoveType type, bool multiSelect);
    bool tileIsSelected(int x, int y) const;
    void selectTile(int tile, bool multiSelect);
    
    void setPage(int page, bool multiSelect);
    inline void modPage(int amount, bool multiSelect)
    {
        setPage(currentPage+amount, multiSelect);
    }
    
    void modCSet(int amount);
    
    inline int getCSet() const { return cset; }
    inline int getTile() const { return cursorPos; }
    inline int getPage() const { return currentPage; }
    inline int getFirstTileOnPage() const { return pageSize*currentPage; }
    
    inline int getTileAt(int x, int y) const
    {
        return pageSize*currentPage+pageWidth*y+x; 
    }
    
    inline void toggleSelectMode()
    {
        selectMode=(selectMode==sm_LINEAR) ? sm_RECTANGLE : sm_LINEAR;
    }
    
private:
    enum tileSelectMode { sm_LINEAR, sm_RECTANGLE };
    
    TileSelector& dialog;
    bool multiSelectEnabled;
    int currentPage;
    int cursorPos;
    int selectPos;
    int cset;
    tileSelectMode selectMode;
    
    //inline void cursorX() const { return cursorPos%pageWidth; }
    //inline void cursorY() const { return cursorPos/pageWidth; }
};

#endif
