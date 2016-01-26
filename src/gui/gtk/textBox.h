#ifndef _ZC_GUI_GTK_TEXTBOX_H_
#define _ZC_GUI_GTK_TEXTBOX_H_

#include <gui/textBox.h>
#include "widget.h"

namespace GUI
{
    
class ZCGtkTextBox: public TextBox, public ZCGtkWidget
{
public:
    ZCGtkTextBox(float width=1.0f, float height=1.0f); // TODO
    void setText(const std::string& text);
    std::string getText() const;
    bool fillsWidth() const;
    bool fillsHeight() const;
    GtkWidget* get();
    
private:
    GtkWidget* textBox;
    GtkWidget* scrollPane;
};

}

#endif
