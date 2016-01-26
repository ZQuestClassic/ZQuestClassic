#include "controller.h"
#include "dialog.h"

namespace GUI
{

bool DialogController::onKeyPressed(const KeyInput& key)
{
    return owner->onKeyPressed(key);
}

}
