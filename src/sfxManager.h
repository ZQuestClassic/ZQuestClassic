#ifndef _ZC_SFXMANAGER_H_
#define _ZC_SFXMANAGER_H_

#include "sfxAllegro.h"
#include "sfxClass.h"

class SFXManager
{
public:
    SFXManager();
    ~SFXManager();
    
    inline void playSFX(int id)
    {
        sfx[id]->play();
    }
    
    inline SFX getSFX(int id)
    {
        return SFX(sfx[id]);
    }
    
private:
    static const int numSFX=256;
    AllegroSFX* sfx[numSFX];
};

#endif
