#ifndef _ZC_GUI_ALLEGRO_TAB_H_
#define _ZC_GUI_ALLEGRO_TAB_H_

#include "standardWidget.h"
#include <string>

namespace GUI
{

class AllegroTabBar;

class AllegroTab: public StandardAllegroWidget
{
public:
    AllegroTab(AllegroTabBar* owner, int id, const std::string& text, FONT* font);
    int realize(DIALOG dlgArray[], int index);
    void draw(AllegroGUIRenderer& renderer) const;
    void getPreferredSize(int& prefWidth, int& prefHeight);
    void onClick();
    void setSelected(bool s);
    void setX(int newX);
    
    static int proc(int msg, DIALOG* d, int c);
    
private:
    AllegroTabBar* owner;
    int id;
    std::string text;
    FONT* font;
    bool selected;
};

}

#endif
