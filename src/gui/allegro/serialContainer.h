#ifndef _ZC_GUI_ALLEGRO_SERIALCONTAINER_H_
#define _ZC_GUI_ALLEGRO_SERIALCONTAINER_H_

#include "../serialContainer.h"
#include "widget.h"
#include <utility>
#include <vector>

namespace GUI
{

class AllegroSerialContainer: public SerialContainer, public AllegroWidget
{
public:
    virtual ~AllegroSerialContainer();
    
    void add(Widget* w);
    int getDataSize() const;
    int realize(DIALOG dlgArray[], int index);
    void draw(AllegroGUIRenderer& renderer) const;
    void onScrollH(int amount, int left, int right);
    void onScrollV(int amount, int top, int bottom);
    void unrealize();
    void setController(AllegroDialogController* controller);
    void setActive(bool val);
    inline AllegroWidget* getWidget(int index) { return contents[index]; }
    
protected:
    std::vector<AllegroWidget*> contents;
    std::vector<std::pair<int, int> > contentSizes;
    
    int getGreatestContentWidth() const;
    int getGreatestContentHeight() const;
    int getTotalContentWidth() const;
    int getTotalContentHeight() const;
};

}

#endif
