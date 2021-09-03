#ifndef ZC_GUI_CHECKBOX_H
#define ZC_GUI_CHECKBOX_H

#include "widget.h"
#include "dialogRef.h"

namespace gui
{

class Checkbox: public Widget
{
public:
    enum class BoxPlacement { right, left };

    Checkbox();
    void setText(std::string newText);
    void setBoxPlacement(BoxPlacement bp);
    void setChecked(bool value);
    bool getChecked();

private:
    bool checked;
    std::string text;
    BoxPlacement boxPlacement;
    DialogRef alDialog;

    void realize(DialogRunner& runner) override;
};

}

#endif
