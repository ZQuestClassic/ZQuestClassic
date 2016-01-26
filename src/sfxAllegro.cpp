#include "sfxAllegro.h"

#include "sound.h"
#include <allegro.h>

// This can all be rewritten once it's used exclusively...

AllegroSFX::AllegroSFX(int sfxID):
    id(sfxID),
    numLoopers(0)
{
}

AllegroSFX::~AllegroSFX()
{
    stop_sfx(id);
}

void AllegroSFX::play()
{
    sfx(id, 63, numLoopers>0, true);
}

void AllegroSFX::play(int x)
{
    sfx(id, pan(x), numLoopers>0, true);
}

void AllegroSFX::addLoop()
{
    numLoopers++;
    if(numLoopers==1)
        cont_sfx(id);
}

void AllegroSFX::removeLoop()
{
    numLoopers--;
    if(numLoopers==0)
        stop_sfx(id); // ??
}

bool AllegroSFX::isPlaying() const
{
    return sfx_allocated(id);
}
