#include "dialog.h"
#include "factory.h"
#include "key.h"

namespace GUI
{

void Dialog::initialize(const WidgetFactory& f)
{
    controller.reset(f.controller());
    controller->setOwner(this);
    controller->setDialogRoot(createDialog(f));
    controller->initialize();
}

bool Dialog::onKeyPressed(const KeyInput& key)
{
    if(key.key==key_ESCAPE)
    {
        shutDown();
        return true;
    }
    
    return false;
}

}
