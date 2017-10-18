#ifndef _ZC_ITEM_FARORESWIND_H_
#define _ZC_ITEM_FARORESWIND_H_

#include "itemAction.h"

class LinkClass;

class FaroresWindAction: public ItemAction
{
public:
    FaroresWindAction(int itemID, LinkClass& link);
    ~FaroresWindAction();
    void update();
    bool isFinished() const;
    void abort();
    
private:
    int phase;
    int timer;
    LinkClass& link;
    int baseX, baseY;
    unsigned char linkTile[256];
};

#endif

