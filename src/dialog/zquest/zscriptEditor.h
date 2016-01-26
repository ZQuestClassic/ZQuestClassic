#ifndef _ZC_DIALOG_ZQUEST_ZSCRIPTEDITOR_H_
#define _ZC_DIALOG_ZQUEST_ZSCRIPTEDITOR_H_

#include <gui/dialog.h>

namespace GUI
{
    class GUIManager;
    class TextBox;
}

class ZScriptEditor: public GUI::Dialog
{
public:
    ZScriptEditor(GUI::GUIManager& gui);
    
private:
    GUI::GUIManager& gui;
    GUI::TextBox* textBox;
    
    GUI::Widget* createDialog(const GUI::WidgetFactory& f);
    bool onKeyPressed(const KeyInput& key);
    bool onClose();
};

#endif
