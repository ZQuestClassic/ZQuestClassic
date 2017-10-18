#ifndef _ZC_GUI_CONTROLLER_H_
#define _ZC_GUI_CONTROLLER_H_

struct KeyInput;

namespace GUI
{

class Dialog;
class Widget;

class DialogController
{
public:
    virtual ~DialogController() {}
    virtual void setDialogRoot(Widget* root)=0;
    virtual void initialize()=0;
    virtual void update()=0;
    virtual void shutDown()=0;
    virtual bool isOpen()=0;
    
    inline void setOwner(Dialog* d) { owner=d; }
    bool onKeyPressed(const KeyInput& key);
    
private:
    Dialog* owner;
};

}

#endif
