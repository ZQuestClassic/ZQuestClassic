#ifndef _ZC_GUI_ALLEGRO_STANDARDWIDGET_H_
#define _ZC_GUI_ALLEGRO_STANDARDWIDGET_H_

#include "widget.h"
#include <allegro.h>

namespace GUI
{

/// Base class for most widgets.
/** This class provides a straightforward implementation of most AllegroWidget
 *  functions. Its realizeAs() function stores a pointer to this in DIALOG->dp.
 */
class StandardAllegroWidget: public AllegroWidget
{
public:
    typedef int(*dialogProc)(int, DIALOG*, int);
    
    StandardAllegroWidget();
    virtual ~StandardAllegroWidget() {}
    
    // Inherited
    virtual void setSizeAndPos(int newX, int newY, int newWidth, int newHeight);
    virtual inline int getDataSize() const { return 1; }
    inline void setController(AllegroDialogController* c) { controller=c; }
    virtual void unrealize();
    virtual void setActive(bool val);
    virtual bool isActive();
    
    void onScrollH(int amount, int left, int right);
    void onScrollV(int amount, int top, int bottom);
    
protected:
    DIALOG* dialog;
    AllegroDialogController* controller;
    int x, y;
    int width, height;
    
    template<typename T>
    inline int realizeAs(DIALOG dlgArray[], int index, int key=0)
    {
        realizeCommon(dlgArray, index, T::proc, key);
        dialog->dp=static_cast<T*>(this);
        return 1;
    }
    
private:
    bool hidden;
    int realizeCommon(DIALOG dlgArray[], int index, dialogProc proc, int key);
};

}

#endif
