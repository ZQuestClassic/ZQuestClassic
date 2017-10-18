#ifndef _ZC_GUI_ALLEGRO_SCROLLBAR_H_
#define _ZC_GUI_ALLEGRO_SCROLLBAR_H_

#include "standardWidget.h"

namespace GUI
{

class AllegroScrollingPane;

class AllegroScrollbar: public StandardAllegroWidget
{
public:
    AllegroScrollbar(AllegroScrollingPane& owner, int buttonStep);
    //void setSizes
    
    void draw(GUI::AllegroGUIRenderer& renderer) const;
    void getPreferredSize(int& prefWidth, int& prefHeight);
    void setSizeAndPos(int x, int y, int width, int height);
    void updateSize(int contentSize, int viewportSize);
    void updatePos(int scrollPos);
    int realize(DIALOG dlgArray[], int index);
    static int proc(int msg, DIALOG* d, int c);
    
private:
    AllegroScrollingPane& owner;
    int buttonStep;
    int sliderPos, sliderSize;
    int contentSize, viewportSize;
    
    void recalculateSliderSize();
    void onClick(int mouseX, int mouseY);
};

}

#endif
