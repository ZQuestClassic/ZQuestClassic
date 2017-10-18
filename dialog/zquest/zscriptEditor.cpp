#include "zscriptEditor.h"
#include <gui/factory.h>
#include <gui/key.h>
#include <gui/manager.h>
#include <gui/textBox.h>
#include <zquest.h>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#define CB(func) boost::bind(&ZScriptEditor::func, this)

extern std::string zScript;

ZScriptEditor::ZScriptEditor(GUI::GUIManager& g):
    gui(g)
{
}

GUI::Widget* ZScriptEditor::createDialog(const GUI::WidgetFactory& f)
{
    GUI::Window* win=f.window("ZScript editor", textBox=f.textBox());
    textBox->setText(zScript);
    win->setOnClose(CB(onClose));
    
    return win;
}

bool ZScriptEditor::onKeyPressed(const KeyInput& key)
{
    if(key.key==key_ESCAPE)
    {
        if(onClose())
            shutDown();
        return true;
    }
    return Dialog::onKeyPressed(key);
}

bool ZScriptEditor::onClose()
{
    int ret=gui.alert("ZScript buffer",
      "Save changes to buffer?",
      "&Yes", "&No", "&Cancel", 1);
    if(ret==3)
        return false;
    if(ret==1)
        zScript=textBox->getText();
    return true;
}
