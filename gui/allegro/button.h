#ifndef _ZC_GUI_ALLEGRO_BUTTON_H_
#define _ZC_GUI_ALLEGRO_BUTTON_H_

#include "../button.h"
#include "standardWidget.h"
#include <boost/function.hpp>
#include <string>

struct FONT;

namespace GUI
{

class AllegroButton: public Button, public StandardAllegroWidget
{
public:
    AllegroButton(AllegroWidget* image, boost::function<void(void)> onClick);
    
    // Inherited
    void draw(AllegroGUIRenderer& renderer) const;
    void getPreferredSize(int& prefWidth, int& prefHeight);
    void setSizeAndPos(int newX, int newY, int newWidth, int newHeight);
    int getDataSize() const;
    int realize(DIALOG dlgArray[], int index);
    
    static int proc(int msg, DIALOG* d, int c);
    
private:
    AllegroWidget* image;
    boost::function<void(void)> onClick;
    bool underMouse, clicked;
    
    void onMouseEnter();
    void onMouseExit();
    void onMouseDown();
    void onMouseUp();
};

}

#endif

