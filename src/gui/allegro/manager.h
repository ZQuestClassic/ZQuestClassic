#ifndef _ZC_GUI_ALLEGRO_MANAGER_H_
#define _ZC_GUI_ALLEGRO_MANAGER_H_

#include "../manager.h"
#include "factory.h"

namespace GUI
{

class AllegroGUIManager: public GUIManager
{
private:
    AllegroWidgetFactory factory;
    
    inline const WidgetFactory& getFactory() const { return factory; }
};

}

#endif
