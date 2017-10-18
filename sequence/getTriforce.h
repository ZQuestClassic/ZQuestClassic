#ifndef _ZC_SEQUENCE_GETTRIFORCE_H_
#define _ZC_SEQUENCE_GETTRIFORCE_H_

#include "sequence.h"
#include <zc_alleg.h>
class LinkClass;

class GetTriforce: public Sequence
{
public:
    GetTriforce(int id, LinkClass& link);
    void activate();
    void update();
    
private:
    LinkClass& link;
    int triforceID;
    int counter;
    int counter2;
    PALETTE flash_pal;
    int x2;
};

#endif
