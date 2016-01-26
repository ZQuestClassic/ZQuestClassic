#include "itemAction.h"

class LinkClass;

class HookshotAction: public ItemAction
{
public:
    HookshotAction(int itemID, LinkClass& link);
    ~HookshotAction();
    static bool canUse(const LinkClass& link);
    void update();
    bool isFinished() const;
    void abort();
    
private:
    LinkClass& link;
    
    void oldUpdate1(); // Formerly update_hookshot()
    void oldUpdate2(); // Formerly part of LinkClass::animate()
};
