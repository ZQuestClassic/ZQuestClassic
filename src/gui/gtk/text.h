#ifndef _ZC_GUI_GTK_TEXT_H_
#define _ZC_GUI_GTK_TEXT_H_

#include <gui/text.h>
#include "widget.h"

namespace GUI
{

class ZCGtkText: public Text, public ZCGtkWidget
{
public:
    ZCGtkText(const std::string& text);
    void setText(const std::string& newText);
    GtkWidget* get();
    
private:
    GtkWidget *text;
};

}

#endif
