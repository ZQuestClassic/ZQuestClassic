#include "tabPanel.h"
#include "util.h"

namespace GUI
{

ZCGtkTabPanel::ZCGtkTabPanel()
{
    tabPanel=gtk_notebook_new();
    g_signal_connect(tabPanel, "destroy", G_CALLBACK(gtk_widget_destroyed), &tabPanel);
}

ZCGtkTabPanel::~ZCGtkTabPanel()
{
    for(int i=0; i<contents.size(); i++)
        delete contents[i];
}

void ZCGtkTabPanel::addTab(const std::string& name, Widget* w)
{
    std::string mod=convertMnemonic(name);
    GtkWidget* label=gtk_label_new_with_mnemonic(mod.c_str());
    
    // Put the content in a scrolling pane, in case it's really big
    // TODO: Maybe do something better about the size
    GtkWidget* scrollPane=gtk_scrolled_window_new(0, 0);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrollPane), 480);
    g_signal_connect(scrollPane, "destroy", G_CALLBACK(gtk_widget_destroyed), &scrollPane);
    
    ZCGtkWidget* zw=dynamic_cast<ZCGtkWidget*>(w);
    contents.push_back(zw);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrollPane), zw->get());
    gtk_notebook_append_page(GTK_NOTEBOOK(tabPanel), scrollPane, label);
}

void ZCGtkTabPanel::setController(ZCGtkController* c)
{
    ZCGtkWidget::setController(c);
    for(int i=0; i<contents.size(); i++)
        contents[i]->setController(c);
}

GtkWidget* ZCGtkTabPanel::get()
{
    return tabPanel;
}

}
