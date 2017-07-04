//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  sprite.cc
//
//  Sprite classes:
//   - sprite:      base class for the guys and enemies in zelda.cc
//   - movingblock: the moving block class
//   - sprite_list: main container class for different groups of sprites
//   - item:        items class
//
//--------------------------------------------------------

/**********************************/
/**********  Item Class  **********/
/**********************************/

#include "precompiled.h" //always first

#include "items.h"
#include "maps.h"
#include "zelda.h"
#include "zdefs.h"
#include "mem_debug.h"
#include "backend/AllBackends.h"

#include <queue>

extern zinitdata zinit;

int fairy_cnt=0;

item::~item()
{
    if(curQuest->isValid(itemDefinition) && curQuest->getItemDefinition(itemDefinition).family==itype_fairy && curQuest->getItemDefinition(itemDefinition).misc3>0 && misc>0)
        killfairy(misc);
}

bool item::animate(int)
{
    if(!screenIsScrolling()) // Because subscreen items are items, too. :p
    {
        if(is_side_view())
        {
            if(can_drop(x,y) && !(pickup & ipDUMMY) && !(pickup & ipCHECK))
            {
                y+=fall/100;
                
                if((fall/100)==0 && fall>0)
                    fall*=(fall>0 ? 2 : 0.5); // That oughta do something about the floatiness.
                    
                if(fall <= (int)zinit.terminalv)
                {
                    fall += zinit.gravity;
                }
            }
            else if(!can_drop(x,y) && !(pickup & ipDUMMY) && !(pickup & ipCHECK))
            {
                fall = -fall/2; // LA key bounce.
            }
        }
        else
        {
            z-=fall/100;
            
            if(z<0)
            {
                z = 0;
                fall = -fall/2;
            }
            else if(z <= 1 && abs(fall) < (int)zinit.gravity)
            {
                z=0;
                fall=0;
            }
            else if(fall <= (int)zinit.terminalv)
            {
                fall += zinit.gravity;
            }
        }
    }
    
    // Maybe it fell off the bottom in sideview, or was moved by a script.
    if(y>352 || y<-176 || x<-256 || x > 512)
    {
        return true;
    }
    
    if((++clk)>=0x8000)
    {
        clk=0x7000;
    }
    
    if(flash)
    {
        cs = o_cset;
        
        if(frame&8)
        {
            cs >>= 4;
        }
        else
        {
            cs &= 15;
        }
    }
    
    if(anim)
    {
        int spd = o_speed;
        
        if(aframe==0)
        {
            spd *= o_delay+1;
        }
        
        if(++aclk >= spd)
        {
            aclk=0;
            
            if(++aframe >= frames)
            {
                aframe=0;
            }
        }
        
        //tile = o_tile + aframe;
        if(extend > 2)
        {
            if(o_tile/TILES_PER_ROW==(o_tile+txsz*aframe)/TILES_PER_ROW)
                tile=o_tile+txsz*aframe;
            else
                tile=o_tile+(txsz*aframe)+((tysz-1)*TILES_PER_ROW)*((o_tile+txsz*aframe)/TILES_PER_ROW)-(o_tile/TILES_PER_ROW);
        }
        else
            tile = o_tile + aframe;
    }
    
    if(curQuest->isValid(itemDefinition) && curQuest->getItemDefinition(itemDefinition).family ==itype_fairy && curQuest->getItemDefinition(itemDefinition).misc3)
    {
        movefairy(x,y,misc);
    }
    
    if(fadeclk==0 && !subscreenItem)
    {
        return true;
    }
    
    if(pickup&ipTIMER)
    {
        if(++clk2 == 512)
        {
            return true;
        }
    }
    
    return false;
}

void item::draw(BITMAP *dest)
{
    if(pickup&ipNODRAW || tile==0)
        return;
        
    if(!(pickup&ipFADE) || fadeclk<0 || fadeclk&1)
    {
        if(clk2>32 || (clk2&2)==0 || curQuest->isValid(itemDefinition) && curQuest->getItemDefinition(itemDefinition).family == itype_fairy)
        {
            sprite::draw(dest);
        }
    }
}

item::item(fix X,fix Y,fix Z,ItemDefinitionRef ref,int p,int c, bool isDummy) : sprite(*pool)
{
    x=X;
    y=Y;
    z=Z;
    itemDefinition = ref;
    pickup=p;
    clk=c;
    misc=clk2=0;
    aframe=aclk=0;
    anim=flash=twohand=subscreenItem=false;
    dummy_int[0]=PriceIndex=-1;
    
    if(!curQuest->isValid(itemDefinition))
        return;
        
    o_tile = curQuest->getItemDefinition(itemDefinition).tile;
    tile = curQuest->getItemDefinition(itemDefinition).tile;
    cs = curQuest->getItemDefinition(itemDefinition).csets&15;
    o_cset = curQuest->getItemDefinition(itemDefinition).csets;
    o_speed = curQuest->getItemDefinition(itemDefinition).speed;
    o_delay = curQuest->getItemDefinition(itemDefinition).delay;
    frames = curQuest->getItemDefinition(itemDefinition).frames;
    flip = curQuest->getItemDefinition(itemDefinition).misc>>2;
    
    if(curQuest->getItemDefinition(itemDefinition).misc&1)
        flash=true;
        
    if(curQuest->getItemDefinition(itemDefinition).misc&2)
        twohand=true;
        
    anim = curQuest->getItemDefinition(itemDefinition).frames>0;
    
    if(pickup&ipBIGRANGE)
    {
        hxofs=-8;
        hxsz=17;
        hyofs=-4;
        hysz=20;
    }
    else if(pickup&ipBIGTRI)
    {
        hxofs=-8;
        hxsz=28;
        hyofs=-4;
        hysz=20;
    }
    else
    {
        hxsz=1;
        hyofs=4;
        hysz=12;
    }
    
    if(!isDummy && curQuest->getItemDefinition(itemDefinition).family == itype_fairy && curQuest->getItemDefinition(itemDefinition).misc3)
    {
        misc = ++fairy_cnt;
        
        if(addfairy(x, y, curQuest->getItemDefinition(itemDefinition).misc3, misc))
            Backend::sfx->play(curQuest->getItemDefinition(itemDefinition).usesound,128);
    }
    
    /*for(int j=0;j<8;j++)
    {
      if(j<2) a[j]=itemsbuf[id].initiala[j]*10000;
      d[j]=itemsbuf[id].initiald[j];
    }*/
}

// easy way to draw an item

void putitem(BITMAP *dest,int x,int y, const ItemDefinitionRef &itemref)
{
    item temp((fix)x,(fix)y,(fix)0,itemref,0,0);
    temp.yofs=0;
    temp.animate(0);
    temp.draw(dest);
}

void putitem2(BITMAP *dest,int x,int y,const ItemDefinitionRef &itemref, int &aclk, int &aframe, int flash)
{
    item temp((fix)x,(fix)y,(fix)0,itemref,0,0,true);
    temp.yofs=0;
    temp.aclk=aclk;
    temp.aframe=aframe;
    
    if(flash)
    {
        temp.flash=(flash != 0);
    }
    
    temp.animate(0);
    temp.draw(dest);
    aclk=temp.aclk;
    aframe=temp.aframe;
}