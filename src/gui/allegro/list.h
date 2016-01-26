#ifndef _ZC_GUI_ALLEGRO_LIST_H_
#define _ZC_GUI_ALLEGRO_LIST_H_

#include "../list.h"
#include "scrollingPane.h"

namespace GUI
{

class AllegroWidgetFactory;

class AllegroList: public List, public AllegroScrollingPane
{
public:
    AllegroList(const ListData* ld, const AllegroWidgetFactory& awf);
    ~AllegroList();
    void getPreferredSize(int& prefWidth, int& prefHeight);
    void draw(AllegroGUIRenderer& renderer) const;
    
    inline void setOnValueChanged(const boost::function<void(int)>& f) { onSelectionChanged=f; }
    inline void setOnDoubleClick(const boost::function<void(int)>& f) { onDoubleClick=f; }
    void setSelected(int num);
    inline int getSelected() const { return selected; }
    
    // Called when list items are clicked
    void changeSelection(int num);
    void doubleClick(int num);
    
private:
    const ListData* data;
    const AllegroWidgetFactory& widgetFactory;
    int selected;
    boost::function<void(int)> onSelectionChanged;
    boost::function<void(int)> onDoubleClick;
};

}

#endif
