#ifndef _ZC_GUI_ALLEGRO_FRAME_H_
#define _ZC_GUI_ALLEGRO_FRAME_H_

#include "standardWidget.h"

namespace GUI
{

class AllegroText;

class AllegroFrame: public StandardAllegroWidget
{
public:
    AllegroFrame(AllegroText* label, AllegroWidget* contents);
    ~AllegroFrame();
    
    void getPreferredSize(int& prefWidth, int& prefHeight);
    void setSizeAndPos(int x, int y, int width, int height);
    int getDataSize() const;
    int realize(DIALOG dlgArray[], int index);
    void setController(AllegroDialogController* controller);
    bool widthIsFlexible() const;
    bool heightIsFlexible() const;
    void draw(AllegroGUIRenderer& renderer) const;
    
    static int proc(int msg, DIALOG* d, int c);
    
private:
    AllegroText* label;
    AllegroWidget* contents;
};

}

#endif
