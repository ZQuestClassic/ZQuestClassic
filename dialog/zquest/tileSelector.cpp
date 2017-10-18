#include "tileSelector.h"
#include "../bitmap/tileSelector.h"
#include "../bitmap/tilePreview.h"
#include <gui/factory.h>
#include <gui/key.h>
#include <gui/spinner.h>
#include <gui/text.h>
#include <util.h>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <cstdio>

TileSelector::TileSelector(int tile, int cset, Orientation orientation):
    backend(*this, tile, cset),
    tsBmp(new TileSelectorBitmap(backend)),
    currTileBmp(new TilePreviewBitmap(tile, cset, orientation)),
    tileBmp2(new TilePreviewBitmap(tile, cset, orientation)),
    pageSelector(0),
    currTileText(0),
    currCSetText(0),
    cancel(true)
{
    
}

#define CB(func) boost::bind(&TileSelector::func, this)

GUI::Widget* TileSelector::createDialog(const GUI::WidgetFactory& f)
{
    char tileBuf[20];
    std::sprintf(tileBuf, "Old tile: %d", tileBmp2->getTile());
    
    GUI::SerialContainer* rightColumn=f.column(GUI::Contents(
      f.text("Page:"),
      pageSelector=f.spinner(0, backend.lastPage),
      f.bitmap(currTileBmp),
      currTileText=f.text(""),
      currCSetText=f.text(""),
      f.bitmap(tileBmp2),
      f.text(tileBuf)
    ));
    
    GUI::Window* win=f.window("Select tile",
      f.column(GUI::Contents(
        f.row(GUI::Contents(
          f.bitmap(tsBmp),
          rightColumn
        )),
        f.buttonRow(GUI::Contents(
          f.button("O&K", CB(onOK)),
          f.button("&Cancel", CB(shutDown))
        ))
      ))
    );
    
    pageSelector->setOnValueChanged(
      boost::bind(&TileSelectorBackend::setPage, &backend, _1, false));
    
    onPageChanged();
    onCSetChanged();
    
    return win;
}

bool TileSelector::onKeyPressed(const KeyInput& key)
{
    bool handled=true;
    
    switch(key.key)
    {
    case key_PLUS:
        backend.modCSet(1);
        break;
        
    case key_MINUS:
        backend.modCSet(-1);
        break;
        
    case key_PAGEUP:
        backend.modPage(-1, key.shiftPressed);
        break;
        
    case key_PAGEDOWN:
        backend.modPage(1, key.shiftPressed);
        break;
        
    case key_UP:
        backend.moveCursor(TileSelectorBackend::cm_UP, key.shiftPressed);
        break;
        
    case key_DOWN:
        backend.moveCursor(TileSelectorBackend::cm_DOWN, key.shiftPressed);
        break;
        
    case key_LEFT:
        backend.moveCursor(TileSelectorBackend::cm_LEFT, key.shiftPressed);
        break;
        
    case key_RIGHT:
        backend.moveCursor(TileSelectorBackend::cm_RIGHT, key.shiftPressed);
        break;
        
    case key_HOME:
        if(key.ctrlPressed)
            backend.setPage(0, key.shiftPressed);
        backend.moveCursor(TileSelectorBackend::cm_START, key.shiftPressed);
        break;
        
    case key_END:
        if(key.ctrlPressed)
            backend.setPage(backend.lastPage, key.shiftPressed);
        backend.moveCursor(TileSelectorBackend::cm_END, key.shiftPressed);
        break;
        
    case key_SPACE:
        backend.toggleSelectMode();
        break;
        
    case key_H:
        currTileBmp->flipHorizontal();
        break;
        
    case key_V:
        currTileBmp->flipVertical();
        break;
        
    case key_R:
        if(key.shiftPressed)
            currTileBmp->rotate(Orientation::rot_90CCW);
        else
            currTileBmp->rotate(Orientation::rot_90CW);
        break;
        
    default:
        handled=false;
        break;
    }
    
    if(handled)
        return true;
    else
        return Dialog::onKeyPressed(key);
}

void TileSelector::onTileChanged()
{
    int tile=backend.getTile();
    char buf[20];
    std::sprintf(buf, "Tile: %d", tile);
    currTileText->setText(buf);
    currTileBmp->setTile(tile);
}

void TileSelector::onCSetChanged()
{
    int cset=backend.getCSet();
    char buf[20];
    std::sprintf(buf, "CSet: %d", cset);
    currCSetText->setText(buf);
    currTileBmp->setCSet(cset);
}

void TileSelector::onPageChanged()
{
    int page=backend.getPage();
    pageSelector->setValue(page);
    onTileChanged();
}

void TileSelector::onOK()
{
    cancel=false;
    shutDown();
}
