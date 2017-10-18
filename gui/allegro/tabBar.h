#ifndef _ZC_GUI_ALLEGRO_TABBAR_H_
#define _ZC_GUI_ALLEGRO_TABBAR_H_

#include "standardWidget.h"
#include "tab.h"
#include <vector>

namespace GUI
{

class AllegroTabPanel;

class AllegroTabBar: public StandardAllegroWidget
{
public:
    AllegroTabBar(AllegroTabPanel& owner);
    void getPreferredSize(int& prefWidth, int& prefHeight);
    void onTabClicked(int tabNum);
    void addTab(const std::string& text);
    void draw(AllegroGUIRenderer& renderer) const;
    void setSizeAndPos(int newX, int newY, int newWidth, int newHeight);
    int getDataSize() const;
    int realize(DIALOG dlgArray[], int index);
    void unrealize();
    bool switchTab(int val);
    
    static int proc(int msg, DIALOG* d, int c);
    
private:
    AllegroTabPanel& owner;
    std::vector<AllegroTab> tabs;
    std::vector<int> tabX;
    int selectedTab;
    bool tooBig;
    int firstVisible, lastVisible;
    
    void onClick(int mouseX);
    
    /** Called after the first or last visible tab is changed to correct
     *  the other end of the range.
     *  \param adjustLast If true, firstVisible is correct and lastVisible
     *                    needs updated. If false, it's the other way around.
     */
    void updateVisibleTabs(bool adjustLast);
    
    /// Returns the width of tab i.
    inline int tabWidth(int i) const
    {
        return tabX[i+1]-tabX[i];
    }
    
    /// Returns the total width of the currently visible tabs.
    inline int visibleTabWidth() const
    {
        return tabX[lastVisible+1]-tabX[firstVisible];
    }
};

}

#endif
