#ifndef _ZC_GUI_ALLEGRO_ROW_H_
#define _ZC_GUI_ALLEGRO_ROW_H_

#include "serialContainer.h"
#include <vector>

namespace GUI
{

class AllegroRow: public AllegroSerialContainer
{
public:
    AllegroRow();
    
    // Inherited
    void getPreferredSize(int& prefWidth, int& prefHeight);
    void setSizeAndPos(int newX, int newY, int newW, int newH);
    void add(Widget* w);
    
    bool widthIsFlexible() const { return numExpanders>0; }
    
private:
    //std::vector<AllegroWidget*> contents;
    int targetWidth, numExpanders, padding;
};

}

#endif
