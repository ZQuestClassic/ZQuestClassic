#include "text.h"
#include "common.h"
#include "list.h"
#include "renderer.h"
#include <allegro.h>

namespace GUI
{

int AllegroText::proc(int msg, DIALOG* d, int c)
{
    AllegroText* text=static_cast<AllegroText*>(d->dp);
    
    switch(msg)
    {
    case MSG_WHEEL:
        forwardMouseWheel(d, c);
        break;
        
    case MSG_WANTMOUSE:
        if(!text->list)
            return D_DONTWANTMOUSE;
        break;
        
    case MSG_CLICK:
        if(!text->list)
            return D_O_K;
        text->list->changeSelection(text->listNum);
        break;
        
    case MSG_DCLICK:
        if(!text->list)
            return D_O_K;
        text->list->doubleClick(text->listNum);
        break;
    }
    
    return D_O_K;
}

AllegroText::AllegroText(const std::string& t, FONT* f, const GUIColor& c):
    StandardAllegroWidget(),
    text(t),
    font(f),
    color(c),
    centered(false),
    list(0),
    listNum(0),
    selected(false)
{
}

void AllegroText::draw(AllegroGUIRenderer& renderer) const
{
    renderer.setOrigin(x, y);
    GUIColor col=color;
    if(selected)
    {
        col=pal.editTextHL;
        renderer.fillRect(0, 0, width, height, pal.editHL);
    }
    if(centered)
        renderer.drawText(text, width/2, 0, font, col, AllegroGUIRenderer::ta_center);
    else
        renderer.drawText(text, 4, 0, font, col, AllegroGUIRenderer::ta_left);
}

void AllegroText::getPreferredSize(int& prefWidth, int& prefHeight)
{
    prefWidth=text_length(font, text.c_str())+8;
    prefHeight=text_height(font);
}

void AllegroText::setSizeAndPos(int newX, int newY,
  int newWidth, int newHeight)
{
    adjustSizeAndPos(newY, newHeight, text_height(font));
    StandardAllegroWidget::setSizeAndPos(newX, newY, newWidth, newHeight);
}

int AllegroText::realize(DIALOG dlgArray[], int index)
{
    return realizeAs<AllegroText>(dlgArray, index);
}

void AllegroText::setText(const std::string& newText)
{
    // TODO: Handle size change
    text=newText;
    if(dialog)
        object_message(dialog, MSG_DRAW, 0);
}

} // Namespace
