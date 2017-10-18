#ifndef _ZC_GUI_ALLEGRO_WINDOW_H_
#define _ZC_GUI_ALLEGRO_WINDOW_H_

#include "standardWidget.h"
#include "../window.h"
#include <string>

struct DIALOG;
struct FONT;

namespace GUI
{

class AllegroWindow: public Window, public StandardAllegroWidget
{
public:
    AllegroWindow(const std::string& title, FONT* titleFont, AllegroWidget* contents);
    ~AllegroWindow();
    void getPreferredSize(int& prefWidth, int& prefHeight);
    int getDataSize() const;
    int realize(DIALOG dlgArray[], int index);
    void unrealize();
    void setController(AllegroDialogController* c);
    void setSizeAndPos(int newX, int newY, int newW, int newH);
    void draw(AllegroGUIRenderer& renderer) const;
    void setMenu(Menu* menu) {}
    inline void setOnClose(boost::function<bool()> func) { onClose=func; }
    
    static int proc(int msg, DIALOG* d, int c);
    
private:
    std::string title;
    FONT* titleFont;
    AllegroWidget* contents;
    boost::function<bool()> onClose;
    
    DIALOG* dlgArray;
    int contentStart, contentEnd;
};

}

#endif

