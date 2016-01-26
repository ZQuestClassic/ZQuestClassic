#include <precompiled.h>
#include "sequence.h"
#include <guys.h>
#include <link.h>
#include <sfxManager.h>
#include <sound.h>
#include <zc_sys.h>
#include <zdefs.h>
#include <zelda.h>
#include <zeldadat.h>
//#include <tempStuff.h>
extern LinkClass Link;
extern SFXManager sfxMgr;
extern mapscr tmpscr[2];

extern int draw_screen_clip_rect_x1;
extern int draw_screen_clip_rect_x2;
extern int draw_screen_clip_rect_y1;
extern int draw_screen_clip_rect_y2;

#include "gameOver.h"
#include "ganonIntro.h"
#include "getBigTriforce.h"
#include "getTriforce.h"
#include "potion.h"
#include "whistle.h"

static Sequence* specialSequence=0;

Sequence::Sequence():
    finished(false)
{
}

Sequence::~Sequence()
{
}

void beginSpecialSequence(sequenceID id, int itemID)
{
    switch(id)
    {
    case seq_ganonIntro:
        specialSequence=new GanonIntro(Link);
        break;
        
    case seq_getTriforce:
        specialSequence=new GetTriforce(itemID, Link);
        break;
        
    case seq_getBigTriforce:
        specialSequence=new GetBigTriforce(itemID, Link);
        break;
        
    case seq_gameOver:
        specialSequence=new GameOver(Link);
        break;
        
    case seq_whistle:
        specialSequence=new WhistleSequence(itemID, Link,
          sfxMgr.getSFX(itemsbuf[itemID].usesound));
        break;
        
    case seq_potion:
        specialSequence=new PotionSequence(itemID, Link,
          sfxMgr.getSFX(WAV_REFILL), sfxMgr.getSFX(WAV_MSG));
        break;
        
    default:
        return; // Maybe throw something instead. This shouldn't happen.
    }
    
    if(specialSequence->isFinished())
    {
        // This can happen, e.g. Ganon's intro if he's already been beaten.
        // Maybe that should be checked beforehand, though...
        delete specialSequence;
        specialSequence=0;
    }
    else
        specialSequence->activate();
}

bool specialSequenceIsActive()
{
    return specialSequence;
}

void updateSpecialSequence()
{
    specialSequence->update();
    if(specialSequence->isFinished())
    {
        delete specialSequence;
        specialSequence=0;
    }
}

void clearSpecialSequence()
{
    if(specialSequence)
    {
        delete specialSequence;
        specialSequence=0;
    }
}


