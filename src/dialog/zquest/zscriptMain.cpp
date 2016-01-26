#include "zscriptMain.h"
#include "zscriptEditor.h"
#include "../../gui/factory.h"
#include "../../gui/manager.h"
#include "../../gui/text.h"
#include "../../zquest.h"
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <cstdio>

#define CB(func) boost::bind(&ZScriptMainDialog::func, this)

extern std::string zScript;

ZScriptMainDialog::ZScriptMainDialog(GUI::GUIManager& g):
    gui(g)
{
}

GUI::Widget* ZScriptMainDialog::createDialog(const GUI::WidgetFactory& f)
{
    GUI::Window* win=
      f.window("Compile ZScript",
        f.column(GUI::Contents(
          sizeText=f.text(""),
          f.buttonRow(GUI::Contents(
            f.button("&Edit", CB(onEdit)),
            f.button("&Compile", CB(onCompile))
          )),
          f.buttonRow(GUI::Contents(
            f.button("&Import", CB(onImport)),
            f.button("E&xport", CB(onExport))
          )),
          f.button("C&lose", CB(shutDown))
        ))
      );
    
    setSizeText();
    
    return win;
}

void ZScriptMainDialog::onEdit()
{
    ZScriptEditor editor(gui);
    gui.showDialog(editor);
    setSizeText();
}

void ZScriptMainDialog::onCompile()
{
    shutDown();
    compileZScript();
}

void ZScriptMainDialog::onImport()
{
    int ret=gui.alert(
      "Confirm overwrite",
      "Loading will erase the current contents of the script buffer. Continue anyway?",
      "&Yes", "&No", 2);
    
    if(ret==2)
        return;
    
    if(!getname("Load ZScript (.z)","z",NULL,datapath,false))
        return;
    
    std::FILE *newScript = std::fopen(temppath,"r");
    if(!newScript)
    {
        gui.alert("Error", "An error occurred importing the script.");
        return;
    }
    
    zScript.clear();
    
    while(!std::feof(newScript))
    {
        char c = std::fgetc(newScript);
        zScript += c;
    }
    
    std::fclose(newScript);
    setSizeText();
    saved = false;
}

void ZScriptMainDialog::onExport()
{
    if(!getname("Save ZScript (.z)", "z", NULL,datapath,false))
        return;
        
    if(exists(temppath))
    {
        int ret=gui.alert(
          "Confirm overwrite",
          "File already exists. Overwrite?",
          "&Yes", "&No", 2);
        if(ret==2)
            return;
    }

    std::FILE* output = std::fopen(temppath,"w");

    if(!output)
    {
        gui.alert("Error", "Unable to open file");
        return;
    }

    size_t written=std::fwrite(zScript.c_str(), sizeof(char), zScript.size(), output);

    if(written!=zScript.size())
        gui.alert("Error", "An error occurred while writing the file.");
        
    std::fclose(output);
}

void ZScriptMainDialog::setSizeText()
{
    char buf[32];
    sprintf(buf, "%lu bytes in buffer", zScript.size());
    sizeText->setText(buf);
}
