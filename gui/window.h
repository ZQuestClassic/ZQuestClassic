#ifndef _ZC_GUI_WINDOW_H_
#define _ZC_GUI_WINDOW_H_

#include "widget.h"
#include <boost/function.hpp>
#include <string>

namespace GUI
{

class Menu;

/// Base class for windows.
class Window: public Widget
{
public:
    virtual ~Window() {}
    
    /// Add a menu bar to the window.
    virtual void setMenu(Menu* menu)=0;
    
    /// Set a function to be called when the window's close button is clicked.
    /** If the function return false, the window will not close.
     */
    virtual void setOnClose(boost::function<bool()> func)=0;
};

}

#endif


