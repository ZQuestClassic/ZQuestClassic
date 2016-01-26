#ifndef _ZC_GUI_GTK_BUTTON_H_
#define _ZC_GUI_GTK_BUTTON_H_

#include <gui/button.h>
#include "widget.h"
#include <boost/function.hpp>
#include <string>

namespace GUI
{

class ZCGtkButton: public Button, public ZCGtkWidget
{
public:
    ZCGtkButton(const std::string& text, boost::function<void(void)> onClick);
    GtkWidget* get();
    
private:
    GtkWidget* button;
    boost::function<void(void)> onClickFunc;
    
    static void onClick(GtkButton*, ZCGtkButton* btn);
};

}

#endif
