#ifndef _ZC_LINKHANDLER_H_
#define _ZC_LINKHANDLER_H_

#include "entityPtr.h"
#include "sfxClass.h"
class dHover;
class dRipples;
class dTallGrass;
class LinkClass;

/// This class will handle some things that Link does but shouldn't be LinkClass's responsibility.
class LinkHandler
{
public:
    LinkHandler();
    bool update();
    
private:
    LinkClass& link;
    SFX lowHealthSFX;
    EntityPtr<dTallGrass> tallGrassDeco;
    EntityPtr<dRipples> ripplesDeco;
    EntityPtr<dHover> hoverDeco;
    
    /// Add tall grass, shallow water, and hover decorations.
    void addDecorations();
    
    /// Play or stop the low health beep.
    void updateLowHealthWarning();
};

#endif
