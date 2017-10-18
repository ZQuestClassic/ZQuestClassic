#ifndef _ZC_GUI_ALLEGRO_FACTORY_H_
#define _ZC_GUI_ALLEGRO_FACTORY_H_

#include "../factory.h"

namespace GUI
{

class AllegroWidgetFactory: public WidgetFactory
{
public:
    DialogController* controller() const;
    
    // Regular widgets
    BitmapWidget* bitmap(BitmapBackend* backend, int scale) const;
    Button* button(const std::string& text, boost::function<void(void)> onClick) const;
    Checkbox* checkbox(const std::string& text) const;
    List* comboBox(const ListData* data) const;
    List* list(const ListData* data) const;
    Spinner* spinner(int minVal, int maxVal) const;
    Text* text(const std::string& text) const;
    TextBox* textBox() const;
    TextField* textField(int maxLength) const;
    
    // Toplevels
    Window* window(const std::string& title, Widget* contents) const;
    
    // Containers
    Widget* frame(const std::string& label, Widget* content) const;
    SerialContainer* column(const Contents& contents=Contents()) const;
    SerialContainer* spacedColumn(const Contents& contents=Contents()) const;
    SerialContainer* row(const Contents& contents=Contents()) const;
    SerialContainer* spacedRow(const Contents& contents=Contents()) const;
    SerialContainer* buttonRow(const Contents& contents=Contents()) const;
    TabPanel* tabPanel() const;
};

}

#endif


