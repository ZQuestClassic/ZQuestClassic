#include "sfxManager.h"

SFXManager::SFXManager()
{
    for(int i=0; i<numSFX; i++)
        sfx[i]=new AllegroSFX(i);
}

SFXManager::~SFXManager()
{
    for(int i(0); i < numSFX; ++i )
        delete sfx[i];
}
