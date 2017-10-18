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
    BitmapWidget* bitmap(BitmapBackend* backend, int scale) const;
    Button* button(const std::string& text, boost::function<void(void)> onClick) const;
    Checkbox* checkbox(const std::string& text) const;
    List* comboBox(const ListData* data) const { return 0; }
    List* list(const ListData* data) const { return 0; }
    Spinner* spinner(int minVal, int maxVal) const;
    Text* text(const std::string& text) const;
    TextBox* textBox() const;
    TextField* textField(int maxLength) const;
    
    // Toplevels
    Window* window(const std::string& title, Widget* content) const;
    
    // Containers
    Widget* frame(const std::string& label, Widget* content) const;
    SerialContainer* column(const Contents& contents) const;
    SerialContainer* spacedColumn(const Contents& contents) const;
    SerialContainer* row(const Contents& contents) const;
    SerialContainer* spacedRow(const Contents& contents) const;
    SerialContainer* buttonRow(const Contents& contents) const;
    TabPanel* tabPanel() const;
};

};

#endif

