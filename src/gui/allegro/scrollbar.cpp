#include "scrollbar.h"
#include "common.h"
#include "renderer.h"
#include "scrollingPane.h"
#include <allegro.h>

namespace GUI
{

int AllegroScrollbar::proc(int msg, DIALOG* d, int c)
{
    switch(msg)
    {
    case MSG_IDLE:
        if(d->d1!=0)
        {
            d->d1=0;
        }
        break;
    case MSG_LPRESS:
        static_cast<AllegroScrollbar*>(d->dp)->onClick(gui_mouse_x(), gui_mouse_y());
        d->d1=1;
        break;
        
    case MSG_WHEEL:
        forwardMouseWheel(d, c);
    }
    return D_O_K;
}

AllegroScrollbar::AllegroScrollbar(AllegroScrollingPane& o, int btn):
    owner(o),
    buttonStep(btn),
    sliderPos(0),
    sliderSize(0),
    contentSize(0),
    viewportSize(0)
{
}

void AllegroScrollbar::updateSize(int cSize, int vpSize)
{
    contentSize=cSize;
    viewportSize=vpSize;
    recalculateSliderSize();
}

void AllegroScrollbar::updatePos(int scrollPos)
{
    const int maxOffset=height-30-sliderSize;
    if(maxOffset<=0)
    {
        sliderPos=15;
        return;
    }
    float relPos=static_cast<float>(scrollPos)/(contentSize-viewportSize);
    sliderPos=15+relPos*maxOffset;
}

void AllegroScrollbar::draw(GUI::AllegroGUIRenderer& renderer) const
{
    renderer.setOrigin(x, y);
    
    // Slider background
    renderer.fillRect(0, 15, width, height-30, pal.scrollbarFill);
    renderer.drawRect(0, 15, width, height-30, pal.outline);
    
    // Slider
    renderer.fillRoundedRect(0, sliderPos, width, sliderSize, 3, pal.scrollbarSlider);
    renderer.drawRoundedRect(0, sliderPos, width, sliderSize, 3, pal.outline);
    
    // Top button
    renderer.fillPartialRoundedRect(0, 0, width, 15, 7, AllegroGUIRenderer::prr_top, pal.button);
    renderer.drawPartialRoundedRect(0, 0, width, 15, 7, AllegroGUIRenderer::prr_top, pal.outline);
    renderer.fillTriangle(4, 9, 7, 6, 10, 9, pal.buttonText);
    
    // Bottom button
    renderer.fillPartialRoundedRect(0, height-15, width, 15, 7, AllegroGUIRenderer::prr_bottom, pal.button);
    renderer.drawPartialRoundedRect(0, height-15, width, 15, 7, AllegroGUIRenderer::prr_bottom, pal.outline);
    renderer.fillTriangle(4, height-10, 7, height-7, 10, height-10, pal.buttonText);
}

void AllegroScrollbar::getPreferredSize(int& prefWidth, int& prefHeight)
{
    // I don't think this will ever be called, but whatever.
    prefWidth=15;
    prefHeight=100;
}

void AllegroScrollbar::setSizeAndPos(int newX, int newY, int newW, int newH)
{
    StandardAllegroWidget::setSizeAndPos(newX, newY, newW, newH);
    recalculateSliderSize();
}

void AllegroScrollbar::recalculateSliderSize()
{
    if(viewportSize>=contentSize)
        return;
    
    float portion=static_cast<float>(viewportSize)/contentSize;
    sliderSize=portion*(height-30);
    if(sliderSize<15)
        sliderSize=15;
}

int AllegroScrollbar::realize(DIALOG dlgArray[], int index)
{
    return realizeAs<AllegroScrollbar>(dlgArray, index);
}

void AllegroScrollbar::onClick(int mouseX, int mouseY)
{
    if(mouseY<y+15)
        owner.scroll(-buttonStep);
    else if(mouseY>=y+height-15)
        owner.scroll(buttonStep);
    else if(mouseY<y+sliderPos)
        owner.scroll(-(viewportSize-buttonStep));
    else if(mouseY>=y+sliderPos+sliderSize)
        owner.scroll(viewportSize-buttonStep);
}

}
