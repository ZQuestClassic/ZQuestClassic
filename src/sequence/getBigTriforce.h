#ifndef _ZC_SEQUENCE_GETBIGTRIFORCE_H_
#define _ZC_SEQUENCE_GETBIGTRIFORCE_H_

#include "sequence.h"
class LinkClass;

class GetBigTriforce: public Sequence
{
public:
    GetBigTriforce(int id, LinkClass& link);
    void activate();
    void update();
    
private:
    LinkClass& link;
    int triforceID;
    int counter;
};

#endif
