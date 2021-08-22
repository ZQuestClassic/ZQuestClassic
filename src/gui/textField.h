#ifndef ZC_GUI_TEXTFIELD_H
#define ZC_GUI_TEXTFIELD_H

#include "widget.h"
#include <memory>
#include <string_view>

namespace gui
{

class TextField: public Widget
{
public:
    TextField();

    /* Returns the current text.
     * The string is owned by the TextField, so don't hold on to it
     * after the dialog is closed.
     * (That should really be changed...)
     */
    std::string_view getText();

    void setText(std::string_view newText);

    /* Set the maximum length of the text, NOT including the null terminator.
     */
    void setMaxLength(size_t newMax);

    template<typename T>
    void onEnter(T m)
    {
        message=static_cast<int>(m);
    }

private:
    std::unique_ptr<char[]> buffer;
    size_t maxLength;
    int message;

    void realize(DialogRunner& runner) override;
    int getMessage() override;
};

}

#endif
