#include "ffc.h"
#include "tiles.h" // isonline() is in tiles for some reason...
#include "zdefs.h" // mapscr

FFC::FFC()
{
    clear();
}

bool FFC::isVisible(bool lensActive) const
{
    if(data==0 || (flags&ffCHANGER)!=0)
        return false;
    
    if((flags&ffLENSVIS)!=0)
        return lensActive;
    
    return true;
}

bool FFC::isUnderPoint(int px, int py) const
{
    int fx=x/10000;
    if(px<fx || px>fx+(width&63))
        return false;
    
    int fy=y/10000;
    if(py<fy || py>fy+(height&63))
        return false;
    
    if((flags&(ffCHANGER|ffETHEREAL))!=0)
        return false;
    
    return true;
}

void FFC::clear()
{
    data=0;
    cset=0;
    delay=0;
    x=0;
    y=0;
    xVel=0;
    yVel=0;
    xAccel=0;
    yAccel=0;
    width=15;
    height=15;
    flags=0;
    link=0;
    lastChangerX=-1000;
    lastChangerY=-1000;
    prevX=-10000000;
    prevY=-10000000;
    
    script=0;
    scriptData.Clear();
    for(int i=0; i<16; i++)
        misc[i]=0;
    for(int i=0; i<8; i++)
        initd[i]=0;
    inita[0]=10000; // Should these be 0? They get reset
    inita[1]=10000; // to 10000 everywhere else, so...
    
    initialized=false;
}

void FFC::copy(const FFC& other, copyType type)
{
    // This is wrong. Redo it.
    
    // This stuff is always copied...
    data=other.data;
    cset=other.cset;
    delay=other.delay;
    xVel=other.xVel;
    yVel=other.yVel;
    xAccel=other.xAccel;
    yAccel=other.yAccel;
    flags=other.flags;
    width=other.width;
    height=other.height;
    link=other.link;
    
    // loadscr() used to copy script both conditionally and unconditionally
    // on carry over. I think unconditional is correct, but maybe not...
    script=other.script;
    
    scriptData=other.scriptData; // I think?
    
    for(int i=0; i<16; i++)
        misc[i]=other.misc[i];
    for(int i=0; i<8; i++)
        initd[i]=other.initd[i];
    inita[0]=other.inita[0];
    inita[1]=other.inita[1];
    
    if(type==copy_pasteSingle)
        return;
    
    x=other.x;
    y=other.y;
    
    if(type==copy_full)
    {
        initialized=other.initialized;
        lastChangerX=other.lastChangerX;
        lastChangerY=other.lastChangerY;
        prevX=other.prevX;
        prevY=other.prevY;
    }
    else // Carry over
    {
        if((flags&ffSCRIPTRESET)==0)
            initialized=other.initialized;
        else // Script resets
        {
            initialized=false;
            scriptData.pc=0;
            scriptData.sp=0;
            scriptData.ffcref=0;
        }
    }
}

void FFC::applyChanger(FFC& changer)
{
    if((changer.flags&ffCHANGETHIS)!=0)
    {
        data=changer.data;
        cset=changer.cset;
    }
    
    if((changer.flags&ffCHANGENEXT)!=0)
        data++;
    
    if((changer.flags&ffCHANGEPREV)!=0)
        data--;
    
    delay=changer.delay;
    x=changer.x;
    y=changer.y;
    width=changer.width;
    height=changer.height;
    xVel=changer.xVel;
    yVel=changer.yVel;
    xAccel=changer.xAccel;
    yAccel=changer.yAccel;
    link=changer.link;
    
    dword newFlags=changer.flags;
    newFlags&=~ffCHANGER;
    if((flags&ffCARRYOVER)!=0) // Keep carry-over flag if it's set.
        newFlags|=ffCARRYOVER;    // (it can still be taken from the changer.
    flags=newFlags;               // It's always been that way.)
}

void FFC::checkScreenEdges(bool wrap)
{
    if(wrap)
    {
        bool wrappedX=false, wrappedY=false;
        if(x<-320000)
        {
            x=2880000+(x+320000);
            wrappedX=true;
        }
        else if(x>=2880000)
        {
            x = x-2880000-320000;
            wrappedX=true;
        }
        
        if(y<-320000)
        {
            y=2080000+(y+320000);
            wrappedY=true;
        }
        else if(y>=2080000)
        {
            y=y-2080000-320000;
            wrappedY=true;
        }
        
        if(wrappedX || wrappedY)
        {
            if(wrappedX)
                prevY=y;
            if(wrappedY)
                prevX=x;
            lastChangerX=-1000;
            lastChangerY=-1000;
        }
    }
    else if(x<-640000 || x>=2880000 || y<-640000 || y>=2080000)
    {
        data=0;
        flags&=~ffCARRYOVER;
    }
}

void FFC::updateMovement(FFC* linked)
{
    if((linked ? linked->delay : delay)==0)
    {
        if(linked && linked!=this)
        {
            prevX=x;
            prevY=y;
            x+=linked->xVel;
            y+=linked->yVel;
        }
        else
        {
            prevX=x;
            prevY=y;
            x+=xVel;
            y+=yVel;
            xVel+=xAccel;
            yVel+=yAccel;
            
            if(xVel>1280000)
                xVel=1280000;
            else if(xVel<-1280000)
                xVel=-1280000;
            
            if(yVel>1280000)
                yVel=1280000;
            else if(yVel<-1280000)
                yVel=-1280000;
        }
    }
    else
    {
        if(!linked || linked==this)
            delay--;
    }
}

bool FFC::canMove()
{
    // Not stationary or a changer, has a combo set
    return data!=0 && (flags&(ffCHANGER|ffSTATIONARY))==0;
}




FFCSet::FFCSet()
{
    clear();
}

void FFCSet::updateMovement(mapscr* tmpscr, bool holdingItem)
{
    for(int i=0; i<32; i++)
    {
        FFC& curr=ffcs[i];
        
        if(!curr.canMove())
            continue;
            
        // Frozen because Link's holding up an item?
        if(holdingItem && (curr.flags&ffIGNOREHOLDUP)==0)
            continue;
            
        // Check for changers
        if(curr.link==0)
        {
            for(int j=0; j<32; j++)
            {
                FFC& changer=ffcs[j];
                
                // Not a functioning changer?
                if((changer.flags&ffCHANGER)==0 || changer.data==0)
                    continue;
                
                // Ignore this changer if it's in the same place as the last one
                if(changer.x/10000==curr.lastChangerX &&
                  changer.y/10000==curr.lastChangerY)
                    continue;
                    
                if((isonline(curr.x, curr.y, curr.prevX, curr.prevY, changer.x, changer.y) || // Along the line, or...
                  (curr.x==changer.x && curr.y==changer.y)) && // At exactly the same position, and...
                  (curr.prevX>-10000000 && curr.prevY>-10000000)) // Whatever this means
                {
                    curr.applyChanger(changer);
                    
                    curr.lastChangerX=(short)(changer.x/10000);
                    curr.lastChangerY=(short)(changer.y/10000);
                    
                    // This switches to a secret combo, I guess? Sure is ugly.
                    if(combobuf[changer.data].flag>15 && combobuf[changer.data].flag<32)
                        changer.data=tmpscr->secretcombo[combobuf[changer.data].flag-16+4];
                    
                    if((changer.flags&ffSWAPPREV)!=0)
                        swapData(changer, ffcs[(j>0) ? j-1 : 31]);
                    else if((changer.flags&ffSWAPPREV)!=0)
                        swapData(changer, ffcs[(j<31) ? j+1 : 0]);
                    
                    break;
                }
            }
        }
        
        curr.updateMovement((curr.link!=0) ? &ffcs[curr.link-1] : 0);
        curr.checkScreenEdges((tmpscr->flags6&fWRAPAROUNDFF)!=0);
    }
}

void FFCSet::swapData(FFC& f1, FFC& f2)
{
    // The flag checkboxes just say "swap speed"...
    // Should all this stuff really be swapped
    zc_swap(f1.data, f2.data);
    zc_swap(f1.cset, f2.cset);
    zc_swap(f1.delay, f2.delay);
    zc_swap(f1.xVel, f2.xVel);
    zc_swap(f1.yVel, f2.yVel);
    zc_swap(f1.xAccel, f2.xAccel);
    zc_swap(f1.yAccel, f2.yAccel);
    zc_swap(f1.link, f2.link);
    zc_swap(f1.width, f2.width);
    zc_swap(f1.height, f2.height);
    zc_swap(f1.flags, f2.flags);
}
