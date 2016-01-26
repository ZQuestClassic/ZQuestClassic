#ifndef _ZC_SFXCLASS_H_
#define _ZC_SFXCLASS_H_

#include "sfxAllegro.h"

class SFX
{
public:
    SFX();
    SFX(AllegroSFX* alSFX);
    SFX(const SFX& other);
    ~SFX();
    
    SFX& operator=(const SFX& other);
    
    void startLooping();
    void stopLooping();
    
    inline void play(int x)
    {
        if(alSFX)
            alSFX->play(x);
    }
    
    inline void play()
    {
        if(alSFX)
            alSFX->play();
    }
    
    inline bool isLooping() const
    {
        return looping;
    }
    
    inline bool isPlaying() const
    {
        return looping || alSFX->isPlaying();
    }
    
private:
    AllegroSFX* alSFX;
    bool looping;
};

#endif
