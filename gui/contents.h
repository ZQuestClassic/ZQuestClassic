#ifndef _ZC_GUI_CONTENTS_H_
#define _ZC_GUI_CONTENTS_H_

#include <cstddef> // for size_t

// A class to be used in creating containers. Variadic functions are a pain
// to use correctly, and variadic templates and initializer_lists aren't
// available. This is an easy and not-too-ugly alternative.

namespace GUI
{

class Widget;

class Contents
{
public:
    Contents();
    Contents(Widget* w1);
    Contents(Widget* w1, Widget* w2);
    Contents(Widget* w1, Widget* w2, Widget* w3);
    Contents(Widget* w1, Widget* w2, Widget* w3, Widget* w4);
    Contents(Widget* w1, Widget* w2, Widget* w3, Widget* w4, Widget* w5);
    Contents(Widget* w1, Widget* w2, Widget* w3, Widget* w4, Widget* w5,
      Widget* w6);
    Contents(Widget* w1, Widget* w2, Widget* w3, Widget* w4, Widget* w5,
      Widget* w6, Widget* w7);
    
    inline Widget* operator [](int index) const { return widgets[index]; }
    inline size_t size() const { return numWidgets; }
    
private:
    const int numWidgets;
    GUI::Widget* widgets[7];
};

}

#endif
