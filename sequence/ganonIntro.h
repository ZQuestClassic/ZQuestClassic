#ifndef _ZC_SEQUENCE_GANONINTRO_H_
#define _ZC_SEQUENCE_GANONINTRO_H_

#include "sequence.h"
class LinkClass;

class GanonIntro: public Sequence
{
public:
    GanonIntro(LinkClass& link);
    void activate();
    void update();
    
private:
    int counter;
    LinkClass& link;
};

#endif
