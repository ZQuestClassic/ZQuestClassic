#include "frame.h"
#include "common.h"
#include "renderer.h"
#include "text.h"

namespace GUI
{

int AllegroFrame::proc(int msg, DIALOG* d, int c)
{
    return dummyProc(msg, d, c);
}

AllegroFrame::AllegroFrame(AllegroText* l, AllegroWidget* c):
    label(l),
    contents(c)
{
}

AllegroFrame::~AllegroFrame()
{
    delete contents;
    delete label;
}
    
void AllegroFrame::getPreferredSize(int& prefWidth, int& prefHeight)
{
    contents->getPreferredSize(prefWidth, prefHeight);
    prefWidth+=16;
    prefHeight+=16;
}

void AllegroFrame::setSizeAndPos(int x, int y, int width, int height)
{
    StandardAllegroWidget::setSizeAndPos(x, y, width, height);
    label->setSizeAndPos(x+8, y, width, height);
    contents->setSizeAndPos(x+8, y+8, width-16, height-16);
}

int AllegroFrame::getDataSize() const
{
    return contents->getDataSize()+label->getDataSize()+1;
}

int AllegroFrame::realize(DIALOG dlgArray[], int index)
{
    int used=realizeAs<AllegroFrame>(dlgArray, index);
    used+=label->realize(dlgArray, index+used);
    used+=contents->realize(dlgArray, index+used);
    return used;
}

void AllegroFrame::setController(AllegroDialogController* c)
{
    StandardAllegroWidget::setController(c);
    label->setController(c);
    contents->setController(c);
}

bool AllegroFrame::widthIsFlexible() const
{
    return contents->widthIsFlexible();
}

bool AllegroFrame::heightIsFlexible() const
{
    return contents->heightIsFlexible();
}

void AllegroFrame::draw(AllegroGUIRenderer& renderer) const
{
    renderer.setOrigin(x, y);
    renderer.drawRect(2, 2, width-4, height-4, pal.winBorder);
    label->draw(renderer);
    contents->draw(renderer);
}

}
