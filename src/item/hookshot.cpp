#include <precompiled.h>
#include "hookshot.h"
#include <link.h>
#include <zelda.h>

HookshotAction::HookshotAction(int itemID, LinkClass& l):
    link(l)
{
    int hookitem = itemsbuf[itemID].fam_type;
    int hookpower = itemsbuf[itemID].power;
    
    if(Lwpns.Count()>=SLMAX)
        Lwpns.del(0);
    
    if(Lwpns.Count()>=SLMAX-1)
        Lwpns.del(0);
    
    int dir=link.getDir();
    fix wx=link.getX();
    fix wy=link.getY();
    fix wz=link.getZ();
    
    switch(dir)
    {
    case up:
        wy-=16;
        break;
        
    case down:
        wy+=16;
        break;
        
    case left:
        wx-=16;
        break;
        
    case right:
        wx+=16;
        break;
    }
    
    if(dir==up)
    {
        hookshot_used=true;
        Lwpns.add(new weapon((fix)wx,(fix)wy,(fix)wz,wHSHandle,hookitem,
                             hookpower*DAMAGE_MULTIPLIER,dir,itemID,link.getUID()));
        Lwpns.add(new weapon((fix)wx,(fix)wy-4,(fix)wz,wHookshot,hookitem,
                             hookpower*DAMAGE_MULTIPLIER,dir,itemID,link.getUID()));
        hs_startx=wx;
        hs_starty=wy-4;
    }
    
    if(dir==down)
    {
        int offset=get_bit(quest_rules,qr_HOOKSHOTDOWNBUG)?4:0;
        hookshot_used=true;
        Lwpns.add(new weapon((fix)wx,(fix)wy+offset,(fix)wz,wHSHandle,hookitem,
                             hookpower*DAMAGE_MULTIPLIER,dir,itemID,link.getUID()));
        Lwpns.add(new weapon((fix)wx,(fix)wy+offset,(fix)wz,wHookshot,hookitem,
                             hookpower*DAMAGE_MULTIPLIER,dir,itemID,link.getUID()));
        hs_startx=wx;
        hs_starty=wy;
    }
    
    if(dir==left)
    {
        hookshot_used=true;
        Lwpns.add(new weapon((fix)wx,(fix)wy,(fix)wz,wHSHandle,hookitem,
                             hookpower*DAMAGE_MULTIPLIER,dir,itemID,link.getUID()));
        Lwpns.add(new weapon((fix)(wx-4),(fix)wy,(fix)wz,wHookshot,hookitem,
                             hookpower*DAMAGE_MULTIPLIER,dir,itemID,link.getUID()));
        hs_startx=wx-4;
        hs_starty=wy;
    }
    
    if(dir==right)
    {
        hookshot_used=true;
        Lwpns.add(new weapon((fix)wx,(fix)wy,(fix)wz,wHSHandle,hookitem,
                             hookpower*DAMAGE_MULTIPLIER,dir,itemID,link.getUID()));
        Lwpns.add(new weapon((fix)(wx+4),(fix)wy,(fix)wz,wHookshot,hookitem,
                             hookpower*DAMAGE_MULTIPLIER,dir,itemID,link.getUID()));
        hs_startx=wx+4;
        hs_starty=wy;
    }
    
    hookshot_frozen=true;
}

bool HookshotAction::canUse(const LinkClass& link)
{
    int dir=link.getDir();
    fix x=link.getX();
    fix y=link.getY();
    
    for(int i=-1; i<2; i++)
    {
        if(dir==up)
        {
            if((combobuf[MAPCOMBO2(i,x,y-7)].type==cHSGRAB)||
              (_walkflag(x+2,y+4,1) && !ishookshottable(int(x),int(y+4))))
                return false;
        }
        else if(dir==down)
        {
            if((combobuf[MAPCOMBO2(i,x+12,y+23)].type==cHSGRAB))
                return false;
        }
        else if(dir==left)
        {
            if((combobuf[MAPCOMBO2(i,x-7,y+12)].type==cHSGRAB))
                return false;
        }
        else if(dir==right)
        {
            if((combobuf[MAPCOMBO2(i,x+23,y+12)].type==cHSGRAB))
                return false;
        }
    }
    
    return true;
}

HookshotAction::~HookshotAction()
{
    link.reset_hookshot();
}

void HookshotAction::update()
{
    oldUpdate1();
    oldUpdate2();
}

void HookshotAction::oldUpdate1()
{
    int hs_x, hs_y, hs_z,hs_dx, hs_dy;
    bool check_hs=false;
    int dist_bx, dist_by, hs_w;
    chainlinks.animate();
    
    //find out where the head is and make it
    //easy to reference
    if(Lwpns.idFirst(wHookshot)>-1)
    {
        check_hs=true;
    }
    
    if(check_hs)
    {
        int parentitem = ((weapon*)Lwpns.spr(Lwpns.idFirst(wHookshot)))->parentitem;
        hs_x=Lwpns.spr(Lwpns.idFirst(wHookshot))->x;
        hs_y=Lwpns.spr(Lwpns.idFirst(wHookshot))->y;
        hs_z=Lwpns.spr(Lwpns.idFirst(wHookshot))->z;
        hs_dx=hs_x-hs_startx;
        hs_dy=hs_y-hs_starty;
        
        //extending
        if(((weapon*)Lwpns.spr(Lwpns.idFirst(wHookshot)))->misc==0)
        {
            int maxchainlinks=itemsbuf[parentitem].misc2;
            
            if(chainlinks.Count()<maxchainlinks)           //extending chain
            {
                if(abs(hs_dx)>=hs_xdist+8)
                {
                    hs_xdist=abs(hs_x-hs_startx);
                    chainlinks.add(new weapon((fix)hs_x, (fix)hs_y, (fix)hs_z,wHSChain, 0,0,link.getDir(), parentitem,link.getUID()));
                }
                else if(abs(hs_dy)>=hs_ydist+8)
                {
                    hs_ydist=abs(hs_y-hs_starty);
                    chainlinks.add(new weapon((fix)hs_x, (fix)hs_y, (fix)hs_z,wHSChain, 0,0,link.getDir(), parentitem,link.getUID()));
                }
            }                                                     //stretching chain
            else
            {
                dist_bx=(abs(hs_dx)-(8*chainlinks.Count()))/(chainlinks.Count()+1);
                dist_by=(abs(hs_dy)-(8*chainlinks.Count()))/(chainlinks.Count()+1);
                hs_w=8;
                
                if(hs_dx<0)
                {
                    dist_bx=0-dist_bx;
                    hs_w=-8;
                }
                
                if(hs_dy<0)
                {
                    dist_by=0-dist_by;
                    hs_w=-8;
                }
                
                for(int counter=0; counter<chainlinks.Count(); counter++)
                {
                    if(link.getDir()>down)                            //chain is moving horizontally
                    {
                        chainlinks.spr(counter)->x=hs_startx+hs_w+dist_bx+(counter*(hs_w+dist_bx));
                    }
                    else
                    {
                        chainlinks.spr(counter)->y=hs_starty+hs_w+dist_by+(counter*(hs_w+dist_by));
                    }
                }
            }
        }                                                       //retracting
        else if(((weapon*)Lwpns.spr(Lwpns.idFirst(wHookshot)))->misc==1)
        {
            dist_bx=(abs(hs_dx)-(8*chainlinks.Count()))/(chainlinks.Count()+1);
            dist_by=(abs(hs_dy)-(8*chainlinks.Count()))/(chainlinks.Count()+1);
            hs_w=8;
            
            if(hs_dx<0)
            {
                dist_bx=0-dist_bx;
                hs_w=-8;
            }
            
            if(hs_dy<0)
            {
                dist_by=0-dist_by;
                hs_w=-8;
            }
            
            /* With ZScript modification, chains can conceivably move diagonally.*/
            //if (Link.getDir()>down)                               //chain is moving horizontally
            {
                if(abs(hs_dx)-(8*chainlinks.Count())>0)             //chain is stretched
                {
                    for(int counter=0; counter<chainlinks.Count(); counter++)
                    {
                        chainlinks.spr(counter)->x=hs_startx+hs_w+dist_bx+(counter*(hs_w+dist_bx));
                    }
                }
                else
                {
                    if(abs(hs_x-hs_startx)<=hs_xdist-8)
                    {
                        hs_xdist=abs(hs_x-hs_startx);
                        
                        if(pull_link==false)
                        {
                            chainlinks.del(chainlinks.idLast(wHSChain));
                        }
                        else
                        {
                            chainlinks.del(chainlinks.idFirst(wHSChain));
                        }
                    }
                }
            }                                                     //chain is moving vertically
            //else
            {
                if(abs(hs_dy)-(8*chainlinks.Count())>0)             //chain is stretched
                {
                    for(int counter=0; counter<chainlinks.Count(); counter++)
                        chainlinks.spr(counter)->y=hs_starty+hs_w+dist_by+(counter*(hs_w+dist_by));
                }
                else
                {
                    if(abs(hs_y-hs_starty)<=hs_ydist-8)
                    {
                        hs_ydist=abs(hs_y-hs_starty);
                        
                        if(pull_link==false)
                            chainlinks.del(chainlinks.idLast(wHSChain));
                        else
                            chainlinks.del(chainlinks.idFirst(wHSChain));
                    }
                }
            }
        }
    }
}

void HookshotAction::oldUpdate2()
{
    fix x=link.getX();
    fix y=link.getY();
    
    if(hookshot_frozen)
    {
        if(hookshot_used)
        {
            link.Freeze();
            
            if(pull_link)
            {
                sprite *t;
                int i;
                
                for(i=0; i<Lwpns.Count() && (Lwpns.spr(i)->id!=wHSHandle); i++)
                {
                    /* do nothing */
                }
                
                t = Lwpns.spr(i);
                
                for(i=0; i<Lwpns.Count(); i++)
                {
                    sprite *s = Lwpns.spr(i);
                    
                    if(s->id==wHookshot)
                    {
                        int dx=0, dy=0;
                        if((s->y)>y)
                            dy=4;
                        else if((s->y)<y)
                            dy=-4;
                        else if((s->x)>x)
                            dx=4;
                        else if((s->x)<x)
                            dx=-4;
                        
                        link.setX(x+dx);
                        link.setY(y+dy);
                        hs_startx+=dx;
                        hs_starty+=dy;
                        if(Lwpns.idFirst(wHSHandle)!=-1)
                        {
                            t->x+=dx;
                            t->y+=dy;
                        }
                    }
                }
            }
        }
        else
        {
            Lwpns.del(Lwpns.idFirst(wHSHandle));
            link.reset_hookshot();
        }
        
        if(hs_fix)
        {
            if(link.getDir()==up || link.getDir()==down)
                y=int(y+7)&0xF0;
            else
                x=int(x+7)&0xF0;
            
            hs_fix=false;
        }
    }
}

bool HookshotAction::isFinished() const
{
    return Lwpns.idFirst(wHookshot)<0;
}

void HookshotAction::abort()
{
    link.reset_hookshot();
}

