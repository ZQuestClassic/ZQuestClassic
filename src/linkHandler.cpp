#include "linkHandler.h"
#include "decorations.h"
#include "guys.h"
#include "link.h"
#include "linkZScriptInterface.h"
#include "sequence.h"
#include "sfxManager.h"
#include "sound.h"
#include "zc_sys.h"
#include "zelda.h"

extern LinkClass Link;
extern SFXManager sfxMgr;
extern LinkZScriptInterface linkIF;

LinkHandler::LinkHandler():
    link(Link),
    lowHealthSFX(sfxMgr.getSFX(WAV_ER))
{
}

void LinkHandler::addDecorations()
{
    fix x=Link.getX();
    fix y=Link.getY();
    fix z=Link.getZ();
    if(isGrassType(COMBOTYPE(x,y+15)) && isGrassType(COMBOTYPE(x+15,y+15))&& z<=8)
    {
        if(!tallGrassDeco)
        {
            tallGrassDeco.reset(new dTallGrass(x, y, dTALLGRASS, 0));
            decorations.add(tallGrassDeco.get());
        }
    }
    else if(tallGrassDeco)
        tallGrassDeco.del();
    
    if((COMBOTYPE(x,y+15)==cSHALLOWWATER)&&(COMBOTYPE(x+15,y+15)==cSHALLOWWATER) && z==0)
    {
        if(!ripplesDeco)
        {
            ripplesDeco.reset(new dRipples(x, y, dRIPPLES, 0));
            decorations.add(ripplesDeco.get());
        }
    }
    else if(ripplesDeco)
        ripplesDeco.del();
    
    if(Link.hoverclk>0)
    {
        if(!hoverDeco)
        {
            hoverDeco.reset(new dHover(x, y, dHOVER, 0));
            decorations.add(hoverDeco.get());
            sfx(itemsbuf[current_item_id(itype_hoverboots)].usesound,pan(int(x)));
        }
    }
    else if(hoverDeco)
        hoverDeco.del();
}

void LinkHandler::updateLowHealthWarning()
{
    // This is pretty incomplete. There are a lot of times when the sound
    // should be suspended.
    if(game->get_life()<=(HP_PER_HEART) && !(game->get_maxlife()<=(HP_PER_HEART)))
    {
        if(heart_beep)
        {
            if(!lowHealthSFX.isLooping())
                lowHealthSFX.startLooping();
        }
        else
        {
            if(heart_beep_timer==-1)
                heart_beep_timer=70;
            
            if(heart_beep_timer>0)
                --heart_beep_timer;
            else
                lowHealthSFX.stopLooping();
        }
    }
    else
    {
        heart_beep_timer=-1;
        lowHealthSFX.stopLooping();
    }
}

bool LinkHandler::update()
{
    if(linkIF.warpPending())
    {
        linkIF.activateWarp();
        return false;
    }
    
    if(DrunkrLbtn() && !get_bit(quest_rules,qr_SELECTAWPN))
        selectNextBWpn(SEL_LEFT);
    else if(DrunkrRbtn() && !get_bit(quest_rules,qr_SELECTAWPN))
        selectNextBWpn(SEL_RIGHT);
    
    if(rPbtn())
        onViewMap(); // return?
    
    updateLowHealthWarning();
    
    if(link.animate(0))
        return true;
    addDecorations();
    
    if((!loaded_guys) && (frame - newscr_clk >= 1))
    {
        if(tmpscr->room==rGANON)
        {
            beginSpecialSequence(seq_ganonIntro);
            return false; // May be unnecessary
        }
        else
            loadguys();
    }
    
    if((!loaded_enemies) && (frame - newscr_clk >= 2))
        loadenemies();
    
    // check lots of other things
    link.checkscroll();
    
    return false;
}
