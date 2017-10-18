#ifndef _ZC_GUI_MANAGER_H_
#define _ZC_GUI_MANAGER_H_

#include <string>

namespace GUI
{

class Dialog;
class WidgetFactory;

class GUIManager
{
public:
    virtual ~GUIManager() {}
    void showDialog(Dialog& d); // const? There may be a dialog stack at some point
    
    // Should alerts be const? Not sure how they'll work, exactly.
    
    // Has only an "OK" button and no return value
    void alert(const std::string& title, const std::string& text);

    // Has two buttons, returns 1 or 2
    int alert(const std::string& title, const std::string& text, const std::string& button1,
      const std::string& button2, int defaultValue);

    // Has three buttons, returns 1, 2, or 3
    int alert(const std::string& title, const std::string& text, const std::string& button1,
      const std::string& button2, const std::string& button3,  int defaultValue);
    
private:
    virtual const WidgetFactory& getFactory() const=0;
};

}

#endif
