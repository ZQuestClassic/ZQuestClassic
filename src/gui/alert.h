#ifndef _ZC_GUI_ALERT_H_
#define _ZC_GUI_ALERT_H_

#include "dialog.h"
#include <string>

namespace GUI
{

class Alert: public Dialog
{
public:
    Alert(const std::string& title, const std::string& text,
      const std::string& btn);
    Alert(const std::string& title, const std::string& text,
      const std::string& btn1, const std::string& btn2, int def);
    Alert(const std::string& title, const std::string& text,
      const std::string& btn1, const std::string& btn2, const std::string& btn3,
      int def);
    int getResult() const;
    
private:
    const int numButtons, defaultValue;
    int returnValue;
    std::string title, text, btn1, btn2, btn3;
    
    Widget* createDialog(const GUI::WidgetFactory& f);
    
    /// Sets the return value and closes the alert.
    void setValue(int val);
    
    /// Splits the text into reasonably-sized lines.
    void fixText();
};

}

#endif


