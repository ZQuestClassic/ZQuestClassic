#ifndef ZC_GUI_LABEL_H
#define ZC_GUI_LABEL_H

#include "widget.h"
#include "dialogRef.h"
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
    int contX, contY, contW, contH;

    void arrange(int contX, int contY, int contW, int contH) override;
    void realize(DialogRunner& runner) override;
};

}

#endif
