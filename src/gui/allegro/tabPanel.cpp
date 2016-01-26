#include "tabPanel.h"
#include "common.h"
#include "renderer.h"
#include "scrollingPane.h"
#include <allegro.h>
#include <cstring>

extern FONT* lfont_l;

namespace GUI
{

int AllegroTabPanel::proc(int msg, DIALOG* d, int c)
{
    switch(msg)
    {
    case MSG_WHEEL:
        forwardMouseWheel(d, c);
        break;
    }
    
    return D_O_K;
}

AllegroTabPanel::AllegroTabPanel():
    tabBar(*this)
{
}

AllegroTabPanel::~AllegroTabPanel()
{
    for(int i=0; i<contents.size(); i++)
        delete contents[i];
}

void AllegroTabPanel::addTab(const std::string& name, Widget* c)
{
    AllegroWidget* alWidget=dynamic_cast<AllegroWidget*>(c);
    contents.push_back(new AllegroScrollingPane(alWidget));
    tabBar.addTab(name);
    if(contents.size()==1)
        activeTab=0;
}

void AllegroTabPanel::getPreferredSize(int& prefW, int& prefH)
{
    int tbW, tbH;
    tabBar.getPreferredSize(tbW, tbH);
    prefW=prefH=0;
    for(int i=0; i<contents.size(); i++)
    {
        int tempW, tempH;
        contents[i]->getPreferredSize(tempW, tempH);
        if(tempW>prefW)
            prefW=tempW;
        if(tempH>prefH)
            prefH=tempH;
    }
    prefW+=6;
    prefH+=tbH+6;
}

void AllegroTabPanel::activateTab(int newTab)
{
    contents[activeTab]->setActive(false);
    activeTab=newTab;
    contents[activeTab]->setActive(true);
}

void AllegroTabPanel::setController(AllegroDialogController* c)
{
    StandardAllegroWidget::setController(c);
}

int AllegroTabPanel::getDataSize() const
{
    contentDataSize=0;
    for(int i=0; i<contents.size(); i++)
        contentDataSize+=contents[i]->getDataSize();
    contentDataSize+=tabBar.getDataSize();
    return contentDataSize+1;
}

int AllegroTabPanel::realize(DIALOG dlgArray[], int index)
{
    int used=realizeAs<AllegroTabPanel>(dlgArray, index);
    
    used+=tabBar.realize(dlgArray, index+used);
    for(int i=0; i<contents.size(); i++)
    {
        used+=contents[i]->realize(dlgArray, index+used);
        if(i!=activeTab)
            contents[i]->setActive(false);
    }
    return used;
}

void AllegroTabPanel::setSizeAndPos(int newX, int newY, int newW, int newH)
{
    StandardAllegroWidget::setSizeAndPos(newX, newY, newW, newH);
    
    int tbW, tbH;
    tabBar.getPreferredSize(tbW, tbH);
    tabBar.setSizeAndPos(x, y, newW, tbH);
    for(int i=0; i<contents.size(); i++)
        contents[i]->setSizeAndPos(newX+3, newY+tbH+3, newW-6, newH-tbH-6);
}

void AllegroTabPanel::draw(AllegroGUIRenderer& renderer) const
{
    int tbW, tbH;
    const_cast<AllegroTabBar*>(&tabBar)->getPreferredSize(tbW, tbH);
    renderer.setOrigin(x, y);
    renderer.drawRoundedRect(0, tbH-1, width, height-tbH+1, 3, pal.outline);
    tabBar.draw(renderer);
    contents[activeTab]->draw(renderer);
}

}
