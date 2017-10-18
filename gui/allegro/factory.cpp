#include "factory.h"
#include "bitmap.h"
#include "button.h"
#include "checkbox.h"
#include "column.h"
#include "comboBox.h"
#include "common.h"
#include "controller.h"
#include "frame.h"
#include "list.h"
#include "row.h"
#include "tabPanel.h"
#include "text.h"
#include "textField.h"
#include "window.h"
#include <guiBitmapRenderer.h>

#include "dummy.h"

extern FONT* lfont;
extern FONT* lfont_l;
extern FONT* dsphantompfont;

namespace GUI
{

DialogController* AllegroWidgetFactory::controller() const
{
    return new AllegroDialogController();
}

BitmapWidget* AllegroWidgetFactory::bitmap(BitmapBackend* backend, int scale) const
{
    backend->setRenderer(new GUIBitmapRenderer(scale));
    return new AllegroBitmapWidget(backend);
}

Window* AllegroWidgetFactory::window(const std::string& title,
  Widget* contents) const
{
    return new AllegroWindow(title, lfont,
      dynamic_cast<AllegroWidget*>(contents));
}

Button* AllegroWidgetFactory::button(const std::string& text,
  boost::function<void(void)> onClick) const
{
    AllegroText* allText=new AllegroText(text, lfont, pal.buttonText);
    allText->setCentered(true);
    return new AllegroButton(allText, onClick);
}

Checkbox* AllegroWidgetFactory::checkbox(const std::string& text) const
{
    return new AllegroCheckbox(text, lfont_l);
}

List* AllegroWidgetFactory::comboBox(const ListData* data) const
{
    return new AllegroComboBox(data, *this);
}

List* AllegroWidgetFactory::list(const ListData* data) const
{
    return new AllegroList(data, *this);
}

Spinner* AllegroWidgetFactory::spinner(int minVal, int maxVal) const
{
    return new DummyAllegroWidget(64, 16);
}

Text* AllegroWidgetFactory::text(const std::string& text) const
{
    return new AllegroText(text, lfont_l, pal.winBodyText);
}

TextBox* AllegroWidgetFactory::textBox() const
{
    return new DummyAllegroWidget(256, 128);
}

TextField* AllegroWidgetFactory::textField(int maxLength) const
{
    return new AllegroTextField(maxLength);
}

// Containers

Widget* AllegroWidgetFactory::frame(const std::string& label, Widget* content) const
{
    AllegroText* text=new AllegroText(label, lfont_l, pal.winBodyText);
    return new AllegroFrame(text, dynamic_cast<AllegroWidget*>(content));
}

SerialContainer* AllegroWidgetFactory::column(const Contents& contents) const
{
    AllegroColumn* col=new AllegroColumn();
    for(int i=0; i<contents.size(); i++)
        col->add(contents[i]);
    return col;
}

SerialContainer* AllegroWidgetFactory::spacedColumn(const Contents& contents) const
{
    AllegroColumn* col=new AllegroColumn();
    for(int i=0; i<contents.size(); i++)
        col->add(contents[i]);
    return col;
}

SerialContainer* AllegroWidgetFactory::row(const Contents& contents) const
{
    AllegroRow* row=new AllegroRow();
    for(int i=0; i<contents.size(); i++)
        row->add(contents[i]);
    return row;
}

SerialContainer* AllegroWidgetFactory::spacedRow(const Contents& contents) const
{
    AllegroRow* row=new AllegroRow();
    for(int i=0; i<contents.size(); i++)
        row->add(contents[i]);
    return row;
}

SerialContainer* AllegroWidgetFactory::buttonRow(const Contents& contents) const
{
    AllegroRow* row=new AllegroRow();
    for(int i=0; i<contents.size(); i++)
        row->add(contents[i]);
    return row;
}

TabPanel* AllegroWidgetFactory::tabPanel() const
{
    return new AllegroTabPanel();
}

}
