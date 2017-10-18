#include "tabBar.h"
#include "common.h"
#include "renderer.h"
#include "tabPanel.h"

extern FONT* lfont_l;

namespace GUI
{

int AllegroTabBar::proc(int msg, DIALOG* d, int c)
{
    if(msg==MSG_CLICK)
        static_cast<AllegroTabBar*>(d->dp)->onClick(gui_mouse_x());
    return D_O_K;
}

AllegroTabBar::AllegroTabBar(AllegroTabPanel& o):
    owner(o),
    tooBig(false),
    firstVisible(0),
    lastVisible(0)
{
    tabs.reserve(10);
    tabX.reserve(10);
    // tabX is always one larger than tabs.
    tabX.push_back(0);
}

void AllegroTabBar::getPreferredSize(int& prefWidth, int& prefHeight)
{
    if(tabs.size()==0)
    {
        prefWidth=0;
        prefHeight=0;
        return;
    }
    
    tabs[0].getPreferredSize(prefWidth, prefHeight); // Just to get the height
    prefWidth=tabX.back()+32;
}

void AllegroTabBar::onTabClicked(int tabNum)
{
    tabs[selectedTab].setSelected(false);
    selectedTab=tabNum;
    tabs[selectedTab].setSelected(true);
    owner.activateTab(tabNum);
}

void AllegroTabBar::addTab(const std::string& text)
{
    tabs.push_back(AllegroTab(this, tabs.size(), text, lfont_l));
    if(tabs.size()==1)
    {
        selectedTab=0;
        tabs[0].setSelected(true);
    }
    
    // Get the size, but subtract 1 from the width because we want them to
    // overlap by a pixel. That's to stop their outlines from doubling up.
    int tw, th;
    tabs[tabs.size()-1].getPreferredSize(tw, th);
    tabX.push_back(tabX.back()+tw-1);
}

void AllegroTabBar::draw(AllegroGUIRenderer& renderer) const
{
    for(int i=firstVisible; i<=lastVisible; i++)
        tabs[i].draw(renderer);
    
    // Draw the tab shift arrows
    renderer.setOrigin(x, y);
    if(tooBig)
    {
        if(firstVisible>0)
            renderer.fillTriangle(9, 5, 4, 10, 9, 15, pal.winBodyText);
        if(lastVisible<tabs.size()-1)
        {
            int tx=(firstVisible==0 ? 6 : 22)+visibleTabWidth();
            renderer.fillTriangle(tx, 5, tx+5, 10, tx, 15, pal.winBodyText);
        }
    }
}

void AllegroTabBar::setSizeAndPos(int newX, int newY, int newWidth, int newHeight)
{
    StandardAllegroWidget::setSizeAndPos(newX, newY, newWidth, newHeight);
    
    for(int i=0; i<tabs.size(); i++)
    {
        tabs[i].setSizeAndPos(newX+3+tabX[i], newY, tabWidth(i)+1, newHeight);
        if(newX+3+tabX[i+1]>=newX+newWidth-3)
            tabs[i].setActive(false);
        else
            lastVisible=i;
    }
    
    tooBig=lastVisible!=tabs.size()-1;
}

int AllegroTabBar::getDataSize() const
{
    int size=0;
    for(int i=0; i<tabs.size(); i++)
        size+=tabs[i].getDataSize();
    return size+1;
}

int AllegroTabBar::realize(DIALOG dlgArray[], int index)
{
    int used=realizeAs<AllegroTabBar>(dlgArray, index);
    for(int i=0; i<tabs.size(); i++)
        used+=tabs[i].realize(dlgArray, index+used);
    return used;
}

void AllegroTabBar::unrealize()
{
    StandardAllegroWidget::unrealize();
    for(int i=0; i<tabs.size(); i++)
        tabs[i].unrealize();
}

bool AllegroTabBar::switchTab(int val)
{
    int newTab=selectedTab+val;
    if(newTab>=0 && newTab<=tabs.size()-1)
    {
        if(newTab<firstVisible)
        {
            firstVisible=newTab;
            updateVisibleTabs(true);
        }
        else if(newTab>lastVisible)
        {
            lastVisible=newTab;
            updateVisibleTabs(false);
        }
        
        onTabClicked(newTab);
        return true;
    }
    else
        return false;
}

void AllegroTabBar::onClick(int mouseX)
{
    if(!tooBig)
        return;
    if(mouseX<x+16 && firstVisible>0)
    {
        firstVisible--;
        updateVisibleTabs(true);
    }
    else if(mouseX>=x+visibleTabWidth() && lastVisible<tabs.size()-1)
    {
        lastVisible++;
        updateVisibleTabs(false);
    }
}

void AllegroTabBar::updateVisibleTabs(bool adjustLast)
{
    if(adjustLast)
    {
        while(visibleTabWidth()>width-32)
            lastVisible--;
    }
    else
    {
        while(visibleTabWidth()>width-32)
            firstVisible++;
    }
    
    int tx=x+(firstVisible==0 ? 3 : 16);
    for(int i=0; i<tabs.size(); i++)
    {
        if(i<firstVisible || i>lastVisible)
            tabs[i].setActive(false);
        else
        {
            tabs[i].setActive(true);
            tabs[i].setX(tx+tabX[i]-tabX[firstVisible]);
        }
    }
}

}
