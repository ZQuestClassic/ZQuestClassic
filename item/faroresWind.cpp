#include <precompiled.h> //always first
#include "faroresWind.h"

#include <link.h>
#include <particles.h>
#include <sound.h>
#include <zc_sys.h>
#include <zelda.h>

extern sprite_list particles;

FaroresWindAction::FaroresWindAction(int itemID, LinkClass& l):
    phase(0),
    timer(0),
    link(l),
    baseX(l.getX()),
    baseY(l.getY())
{
    magicitem=itemID;
}

FaroresWindAction::~FaroresWindAction()
{
    magicitem=-1;
}

void FaroresWindAction::update()
{
    if(timer>0)
    {
        timer--;
        if(phase!=1) // Whatever
            return;
    }
    
    int ltile=0;
    int lflip=0;
    
    switch(phase)
    {
    //if(timer==0)
    case 0:
        {
            linktile(&ltile, &lflip, ls_stab, down, zinit.linkanimationstyle);
            unpack_tile(newtilebuf, ltile, lflip, true);
            memcpy(linkTile, unpackbuf, 256);
            
            if(get_bit(quest_rules,qr_EXPANDEDLTM))
                ltile+=item_tile_mod();
            
            linktile(&link.tile, &link.flip, &link.extend, ls_pound, down, zinit.linkanimationstyle);
            
            if(get_bit(quest_rules,qr_EXPANDEDLTM))
            {
                link.tile+=item_tile_mod();
            }
        }
        phase=1;
        timer=64;
        // Fall though
        
    case 1:
        link.setX(fix(baseX+((rand()%3)-1)));
        link.setY(fix(baseY+((rand()%3)-1)));
        if(timer==0)
            phase=2;
        break;
        
    case 2:
        link.setX(fix(baseX));
        link.setY(fix(baseY));
        linktile(&link.tile, &link.flip, &link.extend, ls_stab, down, zinit.linkanimationstyle);
        
        if(get_bit(quest_rules,qr_EXPANDEDLTM))
            link.tile+=item_tile_mod();
        
        phase=3;
        timer=32;
        break;
    
    case 3:
        if(get_bit(quest_rules,qr_MORESOUNDS))
            sfx(itemsbuf[magicitem].usesound,pan(int(link.getX())));
            
        link.setDontDraw(true);
        
        for(int i=0; i<16; ++i)
        {
            for(int j=0; j<16; ++j)
            {
                if(linkTile[i*16+j])
                {
                    if(itemsbuf[magicitem].misc1==1)  // Twilight
                    {
                        particles.add(new pTwilight(link.getX()+j, link.getY()-link.getZ()+i, 5, 0, 0, (rand()%8)+i*4));
                        int k=particles.Count()-1;
                        particle *p = (particle*)(particles.spr(k));
                        p->step=3;
                    }
                    else if(itemsbuf[magicitem].misc1==2)  // Sands of Hours
                    {
                        particles.add(new pTwilight(link.getX()+j, link.getY()-link.getZ()+i, 5, 1, 2, (rand()%16)+i*2));
                        int k=particles.Count()-1;
                        particle *p = (particle*)(particles.spr(k));
                        p->step=4;
                        
                        if(rand()%10 < 2)
                        {
                            p->color=1;
                            p->cset=0;
                        }
                    }
                    else
                    {
                        particles.add(new pFaroresWindDust(link.getX()+j, link.getY()-link.getZ()+i, 5, 6, linkTile[i*16+j], rand()%96));
                        
                        int k=particles.Count()-1;
                        particle *p = (particle*)(particles.spr(k));
                        p->angular=true;
                        p->angle=rand();
                        p->step=(((double)j)/8);
                        p->yofs=link.getYOfs();
                    }
                }
            }
        }
        phase=4;
        timer=130;
        break;
    
    case 4:
        restart_level();
        magicitem=-1;
        link.setDontDraw(false);
        link.currentItemAction=this; // This won't do...
        phase=5;
        
        break;
    }
}

bool FaroresWindAction::isFinished() const
{
    return phase==5;
}

void FaroresWindAction::abort()
{
    link.setDontDraw(false);
}
