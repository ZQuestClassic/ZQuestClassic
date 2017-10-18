#include "factory.h"

#include "bitmap.h"
#include "button.h"
#include "buttonRow.h"
#include "checkbox.h"
#include "controller.h"
#include "frame.h"
#include "serialContainer.h"
#include "spinner.h"
#include "tabPanel.h"
#include "text.h"
#include "textBox.h"
#include "textField.h"
#include "window.h"
#include <guiBitmapRenderer.h>
#include <cstdarg>

namespace GUI
{

DialogController* GtkWidgetFactory::controller() const
{
    return new ZCGtkController();
}

BitmapWidget* GtkWidgetFactory::bitmap(BitmapBackend* backend) const
{
    backend->setRenderer(new GUIBitmapRenderer(2)); // TODO: scaling
    return new ZCGtkBitmap(backend);
}

Button* GtkWidgetFactory::button(const std::string& text, boost::function<void(void)> onClick) const
{
    return new ZCGtkButton(text, onClick);
}

Checkbox* GtkWidgetFactory::checkbox(const std::string& text) const
{
    return new ZCGtkCheckbox(text);
}

Spinner* GtkWidgetFactory::spinner(int minVal, int maxVal) const
{
    return new ZCGtkSpinner(minVal, maxVal);
}

Text* GtkWidgetFactory::text(const std::string& text) const
{
    return new ZCGtkText(text);
}

TextBox* GtkWidgetFactory::textBox() const
{
    return new ZCGtkTextBox();
}

TextField* GtkWidgetFactory::textField(int maxLength) const
{
    return new ZCGtkTextField(maxLength);
}

Window* GtkWidgetFactory::window(const std::string& title, Widget* content) const
{
    return new ZCGtkWindow(title, dynamic_cast<ZCGtkWidget*>(content));
}

Widget* GtkWidgetFactory::frame(const std::string& label, Widget* content) const
{
    return new ZCGtkFrame(label, dynamic_cast<ZCGtkWidget*>(content));
}

SerialContainer* GtkWidgetFactory::column(int numWidgets, ...) const
{
    ZCGtkSerialContainer* col=new ZCGtkSerialContainer(true, false);
    va_list args;
    va_start(args, numWidgets);
    for(int i=0; i<numWidgets; i++)
    {
        Widget* w=va_arg(args, Widget*);
        col->add(w);
    }
    va_end(args);
    return col;
}

SerialContainer* GtkWidgetFactory::spacedColumn(int numWidgets, ...) const
{
    ZCGtkSerialContainer* col=new ZCGtkSerialContainer(true, true);
    va_list args;
    va_start(args, numWidgets);
    for(int i=0; i<numWidgets; i++)
    {
        Widget* w=va_arg(args, Widget*);
        col->add(w);
    }
    va_end(args);
    return col;
}

SerialContainer* GtkWidgetFactory::row(int numWidgets, ...) const
{
    ZCGtkSerialContainer* row=new ZCGtkSerialContainer(false, false);
    va_list args;
    va_start(args, numWidgets);
    for(int i=0; i<numWidgets; i++)
    {
        Widget* w=va_arg(args, Widget*);
        row->add(w);
    }
    va_end(args);
    return row;
}

SerialContainer* GtkWidgetFactory::spacedRow(int numWidgets, ...) const
{
    ZCGtkSerialContainer* row=new ZCGtkSerialContainer(false, true);
    va_list args;
    va_start(args, numWidgets);
    for(int i=0; i<numWidgets; i++)
    {
        Widget* w=va_arg(args, Widget*);
        row->add(w);
    }
    va_end(args);
    return row;
}

SerialContainer* GtkWidgetFactory::buttonRow(int numWidgets, ...) const
{
    ZCGtkButtonRow* row=new ZCGtkButtonRow();
    va_list args;
    va_start(args, numWidgets);
    for(int i=0; i<numWidgets; i++)
    {
        Widget* w=va_arg(args, Widget*);
        row->add(w);
    }
    va_end(args);
    return row;
}

TabPanel* GtkWidgetFactory::tabPanel() const
{
    return new ZCGtkTabPanel();
}

}
