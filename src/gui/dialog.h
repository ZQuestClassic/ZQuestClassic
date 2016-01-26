#ifndef _ZC_GUI_DIALOG_H_
#define _ZC_GUI_DIALOG_H_

#include "controller.h"
#include "../scoped_ptr.h"
struct KeyInput;

namespace GUI
{

class Widget;
class WidgetFactory;

/// Base type for dialogs to be displayed by GUIManager::showDialog().
class Dialog
{
public:
    virtual ~Dialog() {}
    void initialize(const WidgetFactory& f);
    inline void update() { controller->update(); }
    inline bool isOpen() const { return controller->isOpen(); }
    inline void shutDown() { controller->shutDown(); }
    virtual bool onKeyPressed(const KeyInput& key);
    
private:
    scoped_ptr<DialogController> controller;
    
    virtual Widget* createDialog(const WidgetFactory& f)=0;
};

}

#endif
