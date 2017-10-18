#ifndef _ZC_GUI_GTK_FACTORY_H_
#define _ZC_GUI_GTK_FACTORY_H_

#include <gui/factory.h>

namespace GUI
{

class GtkWidgetFactory: public WidgetFactory
{
public:
    
    DialogController* controller() const;
    
    // Regular widgets
    BitmapWidget* bitmap(BitmapBackend* backend) const;
    Button* button(const std::string& text, boost::function<void(void)> onClick) const;
    Checkbox* checkbox(const std::string& text) const;
    Spinner* spinner(int minVal, int maxVal) const;
    Text* text(const std::string& text) const;
    TextBox* textBox() const;
    TextField* textField(int maxLength) const;
    
    // Toplevels
    Window* window(const std::string& title, Widget* content) const;
    
    // Containers
    Widget* frame(const std::string& label, Widget* content) const;
    SerialContainer* column(int numWidgets=0, ...) const;
    SerialContainer* spacedColumn(int numWidgets=0, ...) const;
    SerialContainer* row(int numWidgets=0, ...) const;
    SerialContainer* spacedRow(int numWidgets=0, ...) const;
    SerialContainer* buttonRow(int numWidgets=0, ...) const;
    TabPanel* tabPanel() const;
};

};

#endif

