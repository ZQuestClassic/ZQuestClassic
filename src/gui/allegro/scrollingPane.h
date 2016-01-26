#ifndef _ZC_GUI_ALLEGRO_SCROLLINGPANE_H_
#define _ZC_GUI_ALLEGRO_SCROLLINGPANE_H_

#include "standardWidget.h"
#include "scrollbar.h"

namespace GUI
{

class AllegroScrollingPane: public StandardAllegroWidget
{
public:
    AllegroScrollingPane(AllegroWidget* content);
    ~AllegroScrollingPane();
    void scroll(int amt);
    void setSizeAndPos(int x, int y, int width, int height);
    int getDataSize() const;
    int realize(DIALOG dlgArray[], int index);
    void unrealize();
    void setController(AllegroDialogController* c);
    void getPreferredSize(int& prefWidth, int& prefHeight);
    void setActive(bool val);
    
    void draw(AllegroGUIRenderer& renderer) const;
    
    int getContentWidth() { return contentWidth; }
    int getContentHeight() { return contentHeight; }
    
    static int proc(int msg, DIALOG* d, int c);
    
protected:
    AllegroWidget* content;
    
private:
    AllegroScrollbar verticalSB;
    int viewportWidth, viewportHeight;
    int contentWidth, contentHeight;
    int scrollPos;
};

}

#endif
