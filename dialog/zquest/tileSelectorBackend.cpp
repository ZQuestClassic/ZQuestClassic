#include "tileSelectorBackend.h"
#include "tileSelector.h"
#include <util.h>
#include <algorithm>

using std::min;
using std::max;

TileSelectorBackend::TileSelectorBackend(TileSelector& dlg, int t, int cs):
    dialog(dlg),
    multiSelectEnabled(false),
    currentPage(t/pageSize),
    cursorPos(t),
    selectPos(t),
    cset(cs),
    selectMode(sm_RECTANGLE)
{
}

void TileSelectorBackend::moveCursor(cursorMoveType type, bool multiSelect)
{
    int oldPage=currentPage;
    
    switch(type)
    {
    case cm_UP:
        cursorPos-=pageWidth;
        break;
        
    case cm_DOWN:
        cursorPos+=pageWidth;
        break;
        
    case cm_LEFT:
        cursorPos-=1;
        break;
        
    case cm_RIGHT:
        cursorPos+=1;
        break;
        
    case cm_START:
        cursorPos=currentPage*pageSize;
        break;
        
    case cm_END:
        cursorPos=(currentPage+1)*pageSize-1;
        break;
    }
    
    cursorPos=vbound(cursorPos, 0, lastTile);
    int page=cursorPos/pageSize;
    if(page!=oldPage)
    {
        currentPage=page;
        dialog.onPageChanged();
    }
    else
        dialog.onTileChanged();
    
    if(!(multiSelect && multiSelectEnabled))
        selectPos=cursorPos;
}

bool TileSelectorBackend::tileIsSelected(int x, int y) const
{
    if(selectMode==sm_LINEAR)
    {
        int tile=getTileAt(x, y);
        int start=min(cursorPos, selectPos);
        int end=max(cursorPos, selectPos);
        return tile>=start && tile<=end;
    }
    else
    {
        y+=currentPage*pageHeight;
        int left=min(cursorPos%pageWidth, selectPos%pageWidth);
        int right=max(cursorPos%pageWidth, selectPos%pageWidth);
        int top=min(cursorPos/pageWidth, selectPos/pageWidth);
        int bottom=max(cursorPos/pageWidth, selectPos/pageWidth);
        return x>=left && x<=right && y>=top && y<=bottom;
    }
}

void TileSelectorBackend::selectTile(int tile, bool multiSelect)
{
    cursorPos=tile;
    if(!(multiSelect && multiSelectEnabled))
        selectPos=cursorPos;
    dialog.onTileChanged();
}

void TileSelectorBackend::setPage(int page, bool multiSelect)
{
    currentPage=clamp(page, 0, lastPage);
    cursorPos=currentPage*pageSize+(cursorPos%pageSize);
    if(!(multiSelect && multiSelectEnabled))
        selectPos=cursorPos;
    dialog.onPageChanged();
}

void TileSelectorBackend::modCSet(int amount)
{
    cset=wrap(cset+amount, 0, 11);
    dialog.onCSetChanged();
}
