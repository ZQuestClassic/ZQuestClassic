#ifndef _ZC_DIALOG_ZQUEST_PALETTEVIEWER_H_
#define _ZC_DIALOG_ZQUEST_PALETTEVIEWER_H_

#include "../../gui/dialog.h"

class PaletteViewer: public GUI::Dialog
{
private:
    GUI::Widget* createDialog(const GUI::WidgetFactory& f);
};

#endif
