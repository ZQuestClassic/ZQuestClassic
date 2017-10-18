#include "window.h"
#include "common.h"
#include "renderer.h"
#include <allegro.h>

namespace GUI
{

int AllegroWindow::proc(int msg, DIALOG* d, int c)
{
    AllegroWindow* window=static_cast<AllegroWindow*>(d->dp);
    
    switch(msg)
    {
    case MSG_WHEEL:
        forwardMouseWheel(d, c);
        break;
    }
    
    return D_O_K;
}

AllegroWindow::AllegroWindow(const std::string& t, FONT* f,
  AllegroWidget* c):
    StandardAllegroWidget(),
    title(t),
    titleFont(f),
    contents(c)
{
}

AllegroWindow::~AllegroWindow()
{
    delete contents;
}

void AllegroWindow::draw(AllegroGUIRenderer& renderer) const
{
    renderer.setOrigin(x, y);
    
    // Border, title bar, and outline
    renderer.fillRoundedRect(0, 0, width, height, 9, pal.winBorder);
    renderer.drawRoundedRect(0, 0, width, height, 9, pal.outline);
    renderer.drawText(title, width/2, 4, titleFont, pal.winTitleText, AllegroGUIRenderer::ta_center);
    
    // Body and outline
    renderer.fillPartialRoundedRect(3, 22, width-6, height-25, 7, AllegroGUIRenderer::prr_bottom, pal.winBody);
    renderer.drawPartialRoundedRect(3, 22, width-6, height-25, 7, AllegroGUIRenderer::prr_bottom, pal.outline);
    renderer.drawLine(3, 21, width-4, 21, pal.outline);
    
    if(contents)
        contents->draw(renderer);
}

void AllegroWindow::getPreferredSize(int& prefWidth, int& prefHeight)
{
    if(contents)
    {
        contents->getPreferredSize(prefWidth, prefHeight);
        prefWidth+=48;
        prefHeight+=34;
    }
    else
    {
        prefWidth=48;
        prefHeight=34;
    }
    
    // Limit size to 95% of screen height. At the moment, this doesn't cause any problems...
    if(prefHeight>0.95f*SCREEN_H)
        prefHeight=0.95f*SCREEN_H;
}

int AllegroWindow::getDataSize() const
{
    if(contents)
        return contents->getDataSize()+1;
    else
        return 1;
}

int AllegroWindow::realize(DIALOG arr[], int index)
{
    realizeAs<AllegroWindow>(arr, index);
    
    dlgArray=arr;
    int contentSize=0;
    if(contents)
        contentSize=contents->realize(arr, index+1);
    contentStart=index+1;
    contentEnd=index+1+contentSize;
    
    return contentSize+1;
}

void AllegroWindow::unrealize()
{
    if(onClose())
        onClose();
    StandardAllegroWidget::unrealize();
    if(contents)
        contents->unrealize();
}

void AllegroWindow::setController(AllegroDialogController* c)
{
    StandardAllegroWidget::setController(c);
    if(contents)
        contents->setController(c);
}

void AllegroWindow::setSizeAndPos(int newX, int newY, int newW, int newH)
{
    StandardAllegroWidget::setSizeAndPos(newX, newY, newW, newH);
    if(contents)
        contents->setSizeAndPos(newX+24, newY+30, newW-48, newH-34);
}

} // Namespace
