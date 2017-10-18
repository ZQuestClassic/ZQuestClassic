#include "alert.h"
#include "factory.h"
#include <boost/bind.hpp>

using std::string;

namespace GUI
{

Alert::Alert(const string& t, const string& x, const string& b):
    numButtons(1),
    defaultValue(1),
    returnValue(1),
    title(t),
    text(x),
    btn1(b)
{
    fixText();
}

Alert::Alert(const string& t, const string& x, const string& b1,
  const string& b2, int def):
    numButtons(2),
    defaultValue(def),
    returnValue(def),
    title(t),
    text(x),
    btn1(b1),
    btn2(b2)
{
    fixText();
}

Alert::Alert(const string& t, const string& x, const string& b1,
  const string& b2, const string& b3, int def):
    numButtons(3),
    defaultValue(def),
    returnValue(def),
    title(t),
    text(x),
    btn1(b1),
    btn2(b2),
    btn3(b3)
{
    fixText();
}

int Alert::getResult() const
{
    return returnValue;
}

void Alert::fixText()
{
    size_t space=0;
    while(true)
    {
        space=text.find(' ', space+50);
        if(space==string::npos)
            return;
        text.replace(space, 1, "\n");
    }
}

#define SETVAL(v) boost::bind(&Alert::setValue, this, v)

Widget* Alert::createDialog(const GUI::WidgetFactory& f)
{
    GUI::SerialContainer* btns;
    
    GUI::Window* win=f.window(title,
      f.column(GUI::Contents(
        f.text(text),
        btns=f.buttonRow(GUI::Contents(
          f.button(btn1, SETVAL(1)))
        )
      ))
    );
    
    if(numButtons>1)
        btns->add(f.button(btn2, SETVAL(2)));
    if(numButtons==3)
        btns->add(f.button(btn3, SETVAL(3)));
    
    return win;
}

void Alert::setValue(int val)
{
    returnValue=val;
    shutDown();
}

}
