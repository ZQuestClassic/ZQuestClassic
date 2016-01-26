#include "manager.h"
#include "alert.h"
#include "dialog.h"

using std::string;

namespace GUI
{

void GUIManager::showDialog(Dialog& d)
{
    d.initialize(getFactory());
    while(d.isOpen())
        d.update();
}

void GUIManager::alert(const string& title, const string& text)
{
    Alert a(title, text, "O&K");
    showDialog(a);
}

int GUIManager::alert(const string& title, const string& text, const string& button1,
  const string& button2, int defaultValue)
{
    Alert a(title, text, button1, button2, defaultValue);
    showDialog(a);
    return a.getResult();
}

int GUIManager::alert(const string& title, const string& text, const string& button1,
  const string& button2, const string& button3, int defaultValue)
{
    Alert a(title, text, button1, button2, button3, defaultValue);
    showDialog(a);
    return a.getResult();
}

}
