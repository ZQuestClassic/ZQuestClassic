#include "precompiled.h"
#include "sequence.h"
#include "link.h"
#include "sfx.h"
#include "sfxManager.h"
#include "sequence/sequence.h"
#include "sequence/gameOver.h"
#include "sequence/ganonIntro.h"
#include "sequence/getBigTriforce.h"
#include "sequence/getTriforce.h"
#include "sequence/potion.h"
#include "sequence/whistle.h"
extern LinkClass Link;
extern SFXManager sfxMgr;

static Sequence* specialSequence=0;

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
