#ifndef _ZC_GUI_ALLEGRO_CHECKBOX_H_
#define _ZC_GUI_ALLEGRO_CHECKBOX_H_

#include "../checkbox.h"
#include "standardWidget.h"
#include <boost/function.hpp>
#include <string>

struct FONT;

namespace GUI
{

class AllegroCheckbox: public Checkbox, public StandardAllegroWidget
{
public:
    AllegroCheckbox(const std::string& text, FONT* font);
    
    // Inherited
    inline bool getValue() const { return checked; }
    inline void setValue(bool newValue) { checked=newValue; }
    inline void setCallback(boost::function<void(bool)> func) { onValueChanged=func; }
    void draw(AllegroGUIRenderer& renderer) const;
    void getPreferredSize(int& prefWidth, int& prefHeight);
    void setSizeAndPos(int newX, int newY, int newWidth, int newHeight);
    int realize(DIALOG dlgArray[], int index);
    
    static int proc(int msg, DIALOG* d, int c);
    
private:
    std::string text;
    FONT* font;
    bool checked;
    boost::function<void(bool)> onValueChanged;
    
    /// Toggles the checkbox's state.
    void toggle();
};

}

#endif

