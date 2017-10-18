#include "button.h"
#include "common.h"
#include "renderer.h"
#include <allegro.h>

namespace GUI
{

int AllegroButton::proc(int msg, DIALOG* d, int c)
{
    AllegroButton* btn=static_cast<AllegroButton*>(d->dp);
    
    switch(msg)
    {
    case MSG_START:
        btn->underMouse=dialogUnderMouse(d);
        break;
    
    case MSG_WANTFOCUS:
        return D_WANTFOCUS;
    
    case MSG_GOTMOUSE:
        btn->onMouseEnter();
        break;
        
    case MSG_LOSTMOUSE:
        btn->onMouseExit();
        break;
        
    case MSG_LPRESS:
        btn->onMouseDown();
        break;
        
    case MSG_LRELEASE:
        btn->onMouseUp();
        break;
        
    case MSG_KEY:
        btn->onClick();
        break;
        
    case MSG_WHEEL:
        forwardMouseWheel(d, c);
        break;
    }
    
    return D_O_K;
}

AllegroButton::AllegroButton(AllegroWidget* i, boost::function<void(void)> f):
    image(i),
    onClick(f),
    underMouse(false),
    clicked(false)
{
}

void AllegroButton::onMouseEnter()
{
    underMouse=true;
    //if(clicked)
        //image->move(0, 2);
}

void AllegroButton::onMouseExit()
{
    underMouse=false;
    //if(clicked)
        //image->move(0, -2);
}

void AllegroButton::onMouseDown()
{
    clicked=true;
    //image->move(0, 2);
}

void AllegroButton::onMouseUp()
{
    clicked=false;
    if(underMouse)
    {
        //if(clicked)
            //image->move(0, 2);
        onClick();
    }
}

void AllegroButton::draw(AllegroGUIRenderer& renderer) const
{
    renderer.setOrigin(x, y);
    const GUIColor& fg=underMouse ? pal.buttonTextHL : pal.buttonText;
    const GUIColor& bg=underMouse ? pal.buttonHL : pal.button;
    int offset=(underMouse && clicked) ? 2 : 0;
    
    renderer.fillRoundedRect(0, offset, width, height, 8, bg);
    renderer.drawRoundedRect(0, offset, width, height, 8, pal.outline);
    if((dialog->flags&D_GOTFOCUS)!=0)
        renderer.drawRoundedRect(4, 4+offset, width-8, height-8, 5, pal.buttonHL);
    
    image->draw(renderer);
}

void AllegroButton::getPreferredSize(int& prefWidth, int& prefHeight)
{
    prefWidth=90;
    prefHeight=50;
}

void AllegroButton::setSizeAndPos(int newX, int newY,
  int newWidth, int newHeight)
{
    adjustSizeAndPos(newX, newWidth, 80);
    adjustSizeAndPos(newY, newHeight, 30);
    image->setSizeAndPos(newX+5, newY+5, newWidth-10, newHeight-10);
    StandardAllegroWidget::setSizeAndPos(newX, newY, newWidth, newHeight);
}

int AllegroButton::getDataSize() const
{
    return image->getDataSize()+1;
}

int AllegroButton::realize(DIALOG dlgArray[], int index)
{
    realizeAs<AllegroButton>(dlgArray, index, 0); //getKey(text));
    return 1+image->realize(dlgArray, index+1);
}

}
