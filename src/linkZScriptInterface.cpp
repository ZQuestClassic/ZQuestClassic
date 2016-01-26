#include "linkZScriptInterface.h"
#include "link.h"
#include "zdefs.h"
#include "zelda.h"
extern LinkClass Link;
extern mapscr tmpscr[2];

LinkZScriptInterface::LinkZScriptInterface():
    link(Link),
    warpDMap(-1)
{
}

int LinkZScriptInterface::getX() const
{
    return link.getX();
}

int LinkZScriptInterface::getY() const
{
    return link.getY();
}

int LinkZScriptInterface::getZ() const
{
    return link.getZ();
}

void LinkZScriptInterface::setX(int newX)
{
    if(Lwpns.idFirst(wHookshot)>-1)
        Lwpns.spr(Lwpns.idFirst(wHookshot))->x+=(newX-link.x);
    
    if(Lwpns.idFirst(wHSHandle)>-1)
        Lwpns.spr(Lwpns.idFirst(wHSHandle))->x+=(newX-link.x);
    
    link.setX(fix(newX));
    
    // A kludge. Why? Who knows.
    if(!link.diagonalMovement && link.dir<=down)
        is_on_conveyor=true;
}

void LinkZScriptInterface::setY(int newY)
{
    if(Lwpns.idFirst(wHookshot)>-1)
        Lwpns.spr(Lwpns.idFirst(wHookshot))->y+=(newY-link.y);
    
    if(Lwpns.idFirst(wHSHandle)>-1)
        Lwpns.spr(Lwpns.idFirst(wHSHandle))->y+=(newY-link.y);
    
    link.setY(fix(newY));
    
    if(!link.diagonalMovement && link.dir>=left)
        is_on_conveyor=true;
}

void LinkZScriptInterface::setZ(int newZ)
{
    if((tmpscr->flags7&fSIDEVIEW)!=0)
        return;
    
    if(link.z==0 && newZ>0)
    {
        switch(link.action)
        {
        case swimming:
            link.diveclk=0;
            link.action=walking;
            break;
            
        case waterhold1:
            link.action=landhold1;
            break;
            
        case waterhold2:
            link.action=landhold2;
            break;
            
        default:
            if(link.charging)
            {
                link.reset_swordcharge();
                link.attackclk=0;
            }
            
            break;
        }
    }
    
    link.setZ(fix(newZ>0 ? newZ : 0));
}

int LinkZScriptInterface::getDir() const
{
    return Link.dir;
}

void LinkZScriptInterface::setDir(int newDir)
{
    link.dir=newDir;
    link.reset_hookshot();
}

void LinkZScriptInterface::setJump(fix newJump)
{
    link.fall=-newJump;
    link.jumping=-1;
}

void LinkZScriptInterface::setAction(int newAction)
{
    if(newAction==dying || newAction==won || newAction==scrolling ||
       newAction==inwind || newAction==rafting || newAction==ischarging)
        return; // Can't use these actions.
    link.setAction(static_cast<actiontype>(newAction));
}

int LinkZScriptInterface::getLadderX() const
{
    return link.ladderx;
}

int LinkZScriptInterface::getLadderY() const
{
    return link.laddery;
}

void LinkZScriptInterface::warp(int dmap, int screen, bool pit)
{
    warpDMap=dmap;
    warpScreen=screen;
    warpIsPit=pit;
}

void LinkZScriptInterface::activateWarp()
{
    if(warpIsPit)
    {
        didpit=true;
        pitx=link.getX();
        pity=link.getY();
    }
    
    tmpscr->sidewarpdmap[0]=warpDMap;
    tmpscr->sidewarpscr[0]=warpScreen;
    tmpscr->sidewarptype[0]=wtIWARP;
    warpDMap=-1; // So warpPending() returns false
    link.dowarp(1,0);
}
