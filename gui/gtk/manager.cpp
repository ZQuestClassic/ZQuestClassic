#include "manager.h"
#include <gtk/gtk.h>

namespace GUI
{

GtkGUIManager::GtkGUIManager()
{
    gtk_init(0, 0);
}

}
