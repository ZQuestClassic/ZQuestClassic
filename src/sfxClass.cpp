#include "sfxClass.h"

SFX::SFX():
    alSFX(0),
    looping(false)
{
}

SFX::SFX(AllegroSFX* a):
    alSFX(a),
    looping(false)
{
}

SFX::SFX(const SFX& other):
    alSFX(other.alSFX),
    looping(false)
{
}
    
SFX::~SFX()
{
    if(looping && alSFX)
        alSFX->removeLoop();
}

SFX& SFX::operator=(const SFX& other)
{
    if(looping && alSFX)
        alSFX->removeLoop();
    alSFX=other.alSFX;
    looping=false;
    return *this;
}

void SFX::startLooping()
{
    if(!looping && alSFX)
    {
        alSFX->addLoop();
        looping=true;
    }
}

void SFX::stopLooping()
{
    if(looping)
    {
        alSFX->removeLoop();
        looping=false;
    }
}
