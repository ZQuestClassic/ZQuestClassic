#ifndef _ZC_DIALOG_ZQUEST_ZSCRIPTMAIN_H_
#define _ZC_DIALOG_ZQUEST_ZSCRIPTMAIN_H_

#include "../../gui/dialog.h"

namespace GUI
{
    class GUIManager;
    class Text;
}

class ZScriptMainDialog: public GUI::Dialog
{
public:
    ZScriptMainDialog(GUI::GUIManager& gui);
    
private:
    GUI::GUIManager& gui;
    GUI::Text* sizeText;
    
    GUI::Widget* createDialog(const GUI::WidgetFactory& f);
    void onEdit();
    void onCompile();
    void onImport();
    void onExport();
    void setSizeText();
};

#endif
