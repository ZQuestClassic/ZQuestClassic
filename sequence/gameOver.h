#ifndef _ZC_SEQUENCE_GAMEOVER_H_
#define _ZC_SEQUENCE_GAMEOVER_H_

#include "sequence.h"
struct BITMAP;
class LinkClass;

class GameOver: public Sequence
{
public:
    GameOver(LinkClass& link);
    void activate();
    void update();
    
private:
    int counter;
    LinkClass& link;
    BITMAP* subscrbmp;
};

#endif
