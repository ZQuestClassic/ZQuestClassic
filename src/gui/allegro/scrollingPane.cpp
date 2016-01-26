#include "scrollingPane.h"
#include "common.h"
#include "renderer.h"

namespace GUI
{

int AllegroScrollingPane::proc(int msg, DIALOG* d, int c)
{
    switch(msg)
    {
        // We have to tell Allegro the pane doesn't want mouse input;
        // otherwise, it'll be the only thing that gets it. Other dialog
        // procs call forwardMouseWheel().
    case MSG_WANTMOUSE:
        return D_DONTWANTMOUSE;
        
    case MSG_WHEEL:
        static_cast<AllegroScrollingPane*>(d->dp)->scroll(c*-10);
        break;
    }
    
    return D_O_K;
}

AllegroScrollingPane::AllegroScrollingPane(AllegroWidget* c):
    content(c),
    verticalSB(*this, 10),
    contentWidth(0),
    contentHeight(0),
    scrollPos(0)
{
}

AllegroScrollingPane::~AllegroScrollingPane()
{
    delete content;
}

void AllegroScrollingPane::scroll(int amt)
{
    const int maxScroll=contentHeight-viewportHeight;
    if(maxScroll<=0)
        return;
    
    if(scrollPos+amt<0)
        amt=-scrollPos;
    else if(scrollPos+amt>maxScroll)
        amt=maxScroll-scrollPos;
    
    if(amt==0)
        return;
    scrollPos+=amt;
    content->onScrollV(-amt, y, y+viewportHeight-1);
    verticalSB.updatePos(scrollPos);
}

void AllegroScrollingPane::setSizeAndPos(int x, int y, int w, int h)
{
    content->setSizeAndPos(x, y, contentWidth, contentHeight);
    StandardAllegroWidget::setSizeAndPos(x, y, w, h);
    viewportHeight=h;
    if(contentHeight>h)
    {
        viewportWidth=w-15;
        verticalSB.setActive(true);
    }
    else
    {
        viewportWidth=w;
        verticalSB.setActive(false);
    }
    
    verticalSB.setSizeAndPos(x+w-15, y, 15, h);
    verticalSB.updateSize(contentHeight, viewportHeight);
    scrollPos=0;
    verticalSB.updatePos(0);
}

int AllegroScrollingPane::getDataSize() const
{
    return content->getDataSize()+verticalSB.getDataSize()+1;
}

int AllegroScrollingPane::realize(DIALOG dlgArray[], int index)
{
    // forwardMouseWheel() needs the pane's DIALOG to be placed after
    // those of its contents.
    int used=content->realize(dlgArray, index);
    used+=verticalSB.realize(dlgArray, index+used);
    used+=realizeAs<AllegroScrollingPane>(dlgArray, index+used);
    dialog->flags|=D_WANTWHEEL;
    return used;
}

void AllegroScrollingPane::unrealize()
{
    StandardAllegroWidget::unrealize();
    verticalSB.unrealize();
    content->unrealize();
}

void AllegroScrollingPane::setController(AllegroDialogController* c)
{
    StandardAllegroWidget::setController(c);
    if(content)
        content->setController(c);
}

void AllegroScrollingPane::getPreferredSize(int& prefWidth, int& prefHeight)
{
    content->getPreferredSize(prefWidth, prefHeight);
    contentWidth=prefWidth;
    contentHeight=prefHeight;
    
    // Not gonna bother asking the scrollbars.
}

void AllegroScrollingPane::draw(GUI::AllegroGUIRenderer& renderer) const
{
    renderer.setOrigin(x, y);
    renderer.pushClipRect(0, 0, viewportWidth, viewportHeight);
    content->draw(renderer);
    renderer.popClipRect();
    if(contentHeight>viewportHeight)
        verticalSB.draw(renderer);
}

void AllegroScrollingPane::setActive(bool val)
{
    StandardAllegroWidget::setActive(val);
    content->setActive(val);
    verticalSB.setActive(val);
}

}
