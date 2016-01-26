#ifndef _ZC_GUI_ALLEGRO_TABPANEL_H_
#define _ZC_GUI_ALLEGRO_TABPANEL_H_

#include "../tabPanel.h"
#include "standardWidget.h"
#include "tabBar.h"
#include <vector>

namespace GUI
{

class AllegroScrollingPane;

class AllegroTabPanel: public TabPanel, public StandardAllegroWidget
{
public:
    AllegroTabPanel();
    ~AllegroTabPanel();
    // Inherited
    void addTab(const std::string& name, Widget* contents);
    
    void getPreferredSize(int& prefWidth, int& prefHeight);
    int getDataSize() const;
    int realize(DIALOG dlg[], int index);
    void setSizeAndPos(int newX, int newY, int newWidth, int newHeight);
    void setController(AllegroDialogController* controller);
    void draw(AllegroGUIRenderer& renderer) const;
    void activateTab(int newTab);
    
    bool widthIsFlexible() const { return true; }
    bool heightIsFlexible() const { return true; }
    
    static int proc(int msg, DIALOG* d, int c);
    
private:
    AllegroTabBar tabBar;
    std::vector<AllegroScrollingPane*> contents;
    int activeTab;
    mutable int contentDataSize;
    DIALOG* contentDlgStart;
};

}

#endif


