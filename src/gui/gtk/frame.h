#ifndef _ZC_GUI_GTK_FRAME_H_
#define _ZC_GUI_GTK_FRAME_H_

#include <gui/widget.h>
#include "widget.h"
#include <string>

namespace GUI
{

class ZCGtkFrame: public Widget, public ZCGtkWidget
{
public:
    ZCGtkFrame(const std::string& label, ZCGtkWidget* content);
    GtkWidget* get();
    
private:
    GtkWidget* frame;
};

}

#endif
