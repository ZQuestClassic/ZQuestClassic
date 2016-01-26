#ifndef _ZC_GUI_ALLEGRO_DUMMY_H_
#define _ZC_GUI_ALLEGRO_DUMMY_H_

#include "standardWidget.h"
#include "../spinner.h"
#include "../textBox.h"

// A placeholder for any widgets that aren't implemented yet (and possibly won't be)

namespace GUI
{

class DummyAllegroWidget: public StandardAllegroWidget,
  public Spinner,
  public TextBox
{
public:
    DummyAllegroWidget(int w, int h);
    
    void getPreferredSize(int& prefWidth, int& prefHeight);
    int realize(DIALOG dlgArray[], int index);
    void draw(AllegroGUIRenderer& renderer) const;
    
    static int proc(int msg, DIALOG* d, int c);
    
    // Spinner
    void setValue(int val) {}
    int getValue() const { return 0; }
    void setOnValueChanged(boost::function<void(int)> func) {}
    
    // TextBox
    void setText(const std::string& text) {}
    std::string getText() const { return ""; }
    
private:
    int width, height;
};

}

#endif


