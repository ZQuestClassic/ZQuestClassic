#ifndef _ZC_ITEM_DINSFIRE_H_
#define _ZC_ITEM_DINSFIRE_H_

#include "itemAction.h"

#include "../entityPtr.h"
class LinkClass;
class weapon;

class DinsFireAction: public ItemAction
{
public:
    DinsFireAction(int itemID, LinkClass& link);
    ~DinsFireAction();
    void update();
    bool isFinished() const;
    void abort();
    
private:
    int phase;
    int timer;
    LinkClass& link;
    EntityPtr<weapon> rocket;
};

#endif
