#ifndef _ZC_GUI_FACTORY_H_
#define _ZC_GUI_FACTORY_H_

#include "bitmap.h"
#include "button.h"
#include "checkbox.h"
#include "controller.h"
#include "contents.h"
#include "list.h"
#include "menu.h"
#include "menuItem.h"
#include "serialContainer.h"
#include "spinner.h"
#include "tabPanel.h"
#include "text.h"
#include "textBox.h"
#include "textField.h"
#include "widget.h"
#include "window.h"
#include <boost/function.hpp>
#include <string>

namespace GUI
{

class WidgetFactory
{
public:
    virtual ~WidgetFactory() {}
    
    virtual DialogController* controller() const=0;
    
    // Regular widgets
    virtual BitmapWidget* bitmap(BitmapBackend* backend, int scale=2) const=0;
    virtual Button* button(const std::string& text, boost::function<void(void)> onClick) const=0;
    virtual Checkbox* checkbox(const std::string& text) const=0;
    virtual List* comboBox(const ListData* data) const=0;
    virtual List* list(const ListData* data) const=0;
    virtual Spinner* spinner(int minVal, int maxVal) const=0;
    virtual Text* text(const std::string& text) const=0;
    virtual TextBox* textBox() const=0;
    virtual TextField* textField(int maxLength) const=0;
    
    // Toplevels
    virtual Window* window(const std::string& title, Widget* contents) const=0;
    
    // Containers
    virtual Widget* frame(const std::string& label, Widget* content) const=0;
    virtual SerialContainer* column(const Contents& contents=Contents()) const=0;
    virtual SerialContainer* spacedColumn(const Contents& contents=Contents()) const=0;
    virtual SerialContainer* row(const Contents& contents=Contents()) const=0;
    virtual SerialContainer* spacedRow(const Contents& contents=Contents()) const=0;
    virtual SerialContainer* buttonRow(const Contents& contents=Contents()) const=0;
    virtual TabPanel* tabPanel() const=0;
};

}

#endif


