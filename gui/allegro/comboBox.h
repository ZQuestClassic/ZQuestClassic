#ifndef _ZC_GUI_ALLEGRO_COMBOBOX_H_
#define _ZC_GUI_ALLEGRO_COMBOBOX_H_

#include "../list.h"
#include "standardWidget.h"

namespace GUI
{

class AllegroList;
class AllegroText;
class AllegroWidgetFactory;

class AllegroComboBox: public List, public StandardAllegroWidget
{
public:
    AllegroComboBox(const ListData* ld, const AllegroWidgetFactory& awf);
    ~AllegroComboBox();
    void getPreferredSize(int& prefWidth, int& prefHeight);
    int getDataSize() const;
    int realize(DIALOG dlgArray[], int index);
    void setController(AllegroDialogController* c);
    void setSizeAndPos(int x, int y, int width, int height);
    void draw(AllegroGUIRenderer& renderer) const;
    
    void setSelected(int num);
    inline int getSelected() const { return selected; }
    inline void setOnValueChanged(const boost::function<void(int)>& f) { onValueChanged=f; }
    static int proc(int msg, DIALOG* d, int c);
    
private:
    AllegroList* list;
    AllegroText* text;
    const ListData* data;
    int selected;
    boost::function<void(int)> onValueChanged;
    
    void onClick();
    void selectionChanged(int newSel);
};

}

#endif
