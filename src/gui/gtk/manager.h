#ifndef _ZC_GUI_GTK_MANAGER_H_
#define _ZC_GUI_GTK_MANAGER_H_

#include <gui/manager.h>
#include "factory.h"

namespace GUI
{

class GtkGUIManager: public GUIManager
{
public:
    GtkGUIManager();
    
private:
    GtkWidgetFactory factory;
    
    inline const WidgetFactory& getFactory() const { return factory; }
};

};

#endif
