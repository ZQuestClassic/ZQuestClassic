#ifndef ZC_GUI_BUTTON_H
#define ZC_GUI_BUTTON_H

#include "widget.h"
#include "dialogRef.h"
#include <string>

namespace gui
{

class Button: public Widget
{
public:
    Button();
    void setText(std::string newText);

    template<typename T>
    void onClick(T m)
    {
        message=static_cast<int>(m);
    }

private:
    std::string text;
    DialogRef alDialog;
    int message;

    void realize(DialogRunner& runner) override;
    int onEvent(int event, MessageDispatcher sendMessage) override;
};

}

#endif
