#ifndef _ZC_GUI_ALLEGRO_COLUMN_H_
#define _ZC_GUI_ALLEGRO_COLUMN_H_

#include "serialContainer.h"
#include <vector>

namespace GUI
{

class AllegroColumn: public AllegroSerialContainer
{
public:
    AllegroColumn();
    
    // Inherited
    void getPreferredSize(int& prefWidth, int& prefHeight);
    void setSizeAndPos(int newX, int newY, int newW, int newH);
    void add(Widget* w);
    
    bool heightIsFlexible() const { return numExpanders>0; }
    
private:
    int targetHeight, numExpanders, padding;
};

}

#endif

