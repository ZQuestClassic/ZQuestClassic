#include "tab.h"
#include "common.h"
#include "renderer.h"
#include "tabBar.h"
#include <allegro.h>

namespace GUI
{

int AllegroTab::proc(int msg, DIALOG* d, int c)
{
    switch(msg)
    {
    case MSG_WANTFOCUS:
        return D_WANTFOCUS;
    
    case MSG_LPRESS:
        static_cast<AllegroTab*>(d->dp)->onClick();
        break;
    
    case MSG_WHEEL:
        forwardMouseWheel(d, c);
        break;
        
    case MSG_CHAR:
        switch(c>>8)
        {
        case KEY_LEFT:
            if(static_cast<AllegroTab*>(d->dp)->owner->switchTab(-1))
                return D_USED_CHAR;
            break;
        case KEY_RIGHT:
            if(static_cast<AllegroTab*>(d->dp)->owner->switchTab(1))
                return D_USED_CHAR;
            break;
        }
        break;
    }
    
    return D_O_K;
}

AllegroTab::AllegroTab(AllegroTabBar* o, int i, const std::string& t, FONT* f):
    owner(o),
    id(i),
    text(t),
    font(f),
    selected(false)
{
}

void AllegroTab::onClick()
{
    if(selected)
        return;
    owner->onTabClicked(id);
}

int AllegroTab::realize(DIALOG dlgArray[], int index)
{
    return realizeAs<AllegroTab>(dlgArray, index);
}

void AllegroTab::draw(AllegroGUIRenderer& renderer) const
{
    renderer.setOrigin(x, y);
    int offset=selected ? 0 : 3;
    renderer.drawPartialRoundedRect(0, offset, width, height-offset, 3, AllegroGUIRenderer::prr_top, pal.outline);
    renderer.drawText(text, 5, 3+offset, font, pal.winBodyText);
    if(selected)
        // Cover up the corresponding part of the frame below
        renderer.drawLine(1, height-1, width-1, height-1, pal.winBody);
    
    if((dialog->flags&D_GOTFOCUS)!=0)
        renderer.drawRoundedRect(2, 2, width-4, height-4, 2, pal.buttonHL);
}

void AllegroTab::getPreferredSize(int& prefWidth, int& prefHeight)
{
    prefWidth=text_length(font, text.c_str())+10;
    prefHeight=text_height(font)+8;
}

void AllegroTab::setSelected(bool s)
{
    selected=s;
    if(dialog)
    {
        if(s)
            dialog->flags|=D_GOTFOCUS;
        else
            dialog->flags&=~D_GOTFOCUS;
    }
}

void AllegroTab::setX(int newX)
{
    x=newX;
    if(dialog)
        dialog->x=newX;
}

}
