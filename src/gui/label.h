#ifndef ZC_GUI_LABEL_H
#define ZC_GUI_LABEL_H

#include "widget.h"
#include "dialog.h"
#include <string>

namespace gui
{

class Label: public Widget
{
public:
    Label();
    void setText(std::string newText);

private:
    std::string text;
    DialogRef alDialog;

    void realize(DialogRunner& runner) override;
};

}

#endif
