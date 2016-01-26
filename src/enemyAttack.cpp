#include "enemyAttack.h"

#include "zdefs.h"
#include "link.h"
#include "guys.h"
#include "zelda.h"

extern LinkClass Link;
extern mapscr tmpscr[2];
extern sprite_list  guys, Ewpns;


// These will have to do for now...
static int count_layer_enemies()
{
    int cnt=0;
    
    for(int i=0; i<6; i++)
    {
        if(tmpscr->layermap[i]!=0)
        {
            const mapscr* layerscreen=&TheMaps[(tmpscr->layermap[i]-1)*MAPSCRS]+
              tmpscr->layerscreen[i];
            
            for(int j=0; j<10; j++)
            {
                if(layerscreen->enemy[j]!=0)
                    cnt++;
            }
        }
    }
    
    return cnt;
}

static int random_layer_enemy()
{
    int cnt=count_layer_enemies();
    
    if(cnt==0)
        return eNONE;
    
    int ret=rand()%cnt;
    cnt=0;
    
    for(int i=0; i<6; ++i)
    {
        if(tmpscr->layermap[i]!=0)
        {
            const mapscr* layerscreen=&TheMaps[(tmpscr->layermap[i]-1)*MAPSCRS]+
              tmpscr->layerscreen[i];
            
            for(int j=0; j<10; ++j)
            {
                if(layerscreen->enemy[j]!=0)
                {
                    if(cnt==ret)
                        return layerscreen->enemy[j];
                    cnt++;
                }
            }
        }
    }
    
    return eNONE;
}


EnemyAttack::EnemyAttack(int wid, int wt, int pow, fireType ft, const SFX& s):
    owner(0),
    weaponID(wid),
    wpnType(wt),
    power(pow),
    fType(ft),
    breathTimer(0),
    sfx(s),
    xOffset(0),
    yOffset(0)
{
}

void EnemyAttack::fire(int dir, int type)
{
    weapon* wpn=new weapon(
      owner->getX()+xOffset,
      owner->getY()+yOffset,
      owner->getZ(),
      weaponID,
      type,
      power,
      dir,
      -1,
      owner->getUID());
    Ewpns.add(wpn);
}

weapon* EnemyAttack::fireFront(int dir, int type)
{
    weapon* wpn=new weapon(
      owner->getX()+xOffset,
      owner->getY()+yOffset,
      owner->getZ(),
      weaponID,
      type,
      power,
      dir,
      -1,
      owner->getUID());
    Ewpns.addAtFront(wpn);
    return wpn;
}

weapon* EnemyAttack::fireBreath()
{
    bool seeklink=false;
    
    float fire_angle=0.0;
    int wdir=owner->getDir();
    
    int oldXOffset=xOffset;
    int oldYOffset=yOffset;
    
    switch(wdir)
    {
    case -1:
    case up:
        fire_angle=PI*((rand()%20)+50)/40;
        yOffset-=8;
        break;
        
    case down:
        fire_angle=PI*((rand()%20)+10)/40;
        yOffset+=8;
        break;
        
    case left:
        fire_angle=PI*((rand()%20)+30)/40;
        xOffset-=8;
        break;
        
    case right:
        fire_angle=PI*((rand()%20)+70)/40;
        xOffset+=8;
        break;
    }
    
    if(weaponID==ewFlame || weaponID==ewFlame2)
    {
        if(fire_angle==-PI || fire_angle==PI) wdir=left;
        else if(fire_angle==-PI/2) wdir=up;
        else if(fire_angle==PI/2) wdir=down;
        else if(fire_angle==0) wdir=right;
        else if(fire_angle<-PI/2) wdir=l_up;
        else if(fire_angle<0) wdir=r_up;
        else if(fire_angle<(PI/2)) wdir=r_down;
        else if(fire_angle<PI) wdir=l_down;
    }
    
    weapon* ew=fireFront(wdir, 2);
    sfx.play(owner->getX());
    
    xOffset=oldXOffset;
    yOffset=oldYOffset;
    
    if((rand()&4)!=0)
    {
        ew->angular=true;
        ew->angle=fire_angle;
    }
    
    if(weaponID==ewFlame && wpnsbuf[ewFLAME].frames>1)
    {
        ew->aframe=rand()%wpnsbuf[ewFLAME].frames;
        ew->tile+=ew->aframe;
    }
    
    return 0;
}

weapon* EnemyAttack::fireAimedBreath()
{
    fireFront(0xFF, 2);
    sfx.play(owner->getX());
    
    weapon *ew = (weapon*)(Ewpns.spr(0));
    
    if(weaponID==ewFlame && wpnsbuf[ewFLAME].frames>1)
    {
        ew->aframe=rand()%wpnsbuf[ewFLAME].frames;
        ew->tile+=ew->aframe;
    }
    
    return 0;
}

void EnemyAttack::activate()
{
    int dir=owner->getDir();
    fix x=owner->getX();
    fix y=owner->getY();
    fix z=owner->getZ();
    int wpn=owner->wpn;
    short wdp=owner->wdp;
    
    switch(fType)
    {
    case ft_5Shots:
        fire(dir, wpnType+(((dir^left)+5)<<3));
        fire(dir, wpnType+(((dir^right)+5)<<3));
        // Fall through
        
    case ft_3Shots:
        fire(dir, wpnType+(((dir^left)+1)<<3));
        fire(dir, wpnType+(((dir^right)+1)<<3));
        // Fall through
        
    case ft_1Shot:
        fire(dir, wpnType);
        sfx.play(owner->getX());
        break;
        
    case ft_1ShotSlanted:
    {
        int dx=Link.getX()-owner->getY();
        int dy=Link.getY()-owner->getY();
        int slant=0;
        switch(dir)
        {
        case up:
            if(dx<-8) slant=left;
            else if(dx>8) slant=right;
            break;
        case down:
            if(dx>8) slant=left;
            else if(dx<-8) slant=right;
            break;
        case left:
            if(dy<-8) slant=left;
            else if(dy>8) slant=right;
            break;
        case right:
            if(dy>8) slant=left;
            else if(dy<-8) slant=right;
            break;
        }
        
        fire((weaponID==ewFireball2 || weaponID==ewFireball) ? 0 : dir, // Is this necessary? Won't they set their direction automatically?
          wpnType+(((dir^slant)+1)<<3));
        sfx.play(owner->getX());
        break;
    }
    
    case ft_8Shots:
        fire(l_up, wpnType);
        fire(l_down, wpnType);
        fire(r_up, wpnType);
        fire(r_down, wpnType);
        // Fall through
        
    case ft_4Shots:
        fire(up, wpnType);
        fire(down, wpnType);
        fire(left, wpnType);
        fire(right, wpnType);
        sfx.play(owner->getX());
        break;
        
    case ft_aquamentus:
        fire(up, wpnType);
        fire(8, wpnType); // Sure, why not.
        fire(down, wpnType);
        sfx.play(owner->getX());
        break;
        
    case ft_gohma3Shots:
        fire(left, wpnType);
        fire(right, wpnType);
        // Fall through
        
    case ft_gohma1Shot:
        fire(8, wpnType);
        sfx.play(owner->getX());
        break;
        
    case ft_breath:
    case ft_breathAimed:
        breathTimer=rand()%50+50;
        break;
        
    case ft_summon:
        {
            int currentNum=0;
            
            for(int i=0; i<guys.Count(); i++)
            {
                if((((enemy*)guys.spr(i))->id)==weaponID)
                    currentNum++;
            }
            
            if(currentNum<=40)
            {
                int guyCount=guys.Count();
                int numToSummon=(rand()%power)+1;
                
                for(int i=0; i<numToSummon; i++)
                {
                    if(addenemy(x, y, weaponID, -10))
                        ((enemy*)guys.spr(guyCount+i))->count_enemy = false;
                }
                
                sfx.play(x);
            }
        }
        break;
        
    case ft_summonLayer:
        {
            // This should probably be checked elsewhere...
            if(count_layer_enemies()==0)
                return;
            
            int kids = guys.Count();
            
            if(kids<200)
            {
                int numToSummon=(rand()%power)+1;
                bool summoned=false;
                
                for(int i=0; i<numToSummon; i++)
                {
                    int id2=vbound(random_layer_enemy(),eSTART,eMAXGUYS-1);
                    int x2=0;
                    int y2=0;
                    
                    for(int k=0; k<20; ++k)
                    {
                        x2=16*((rand()%12)+2);
                        y2=16*((rand()%7)+2);
                        
                        if(!m_walkflag(x2,y2,0) && (abs(x2-Link.getX())>=32 || abs(y2-Link.getY())>=32))
                        {
                            if(addenemy(x2,y2,get_bit(quest_rules,qr_ENEMIESZAXIS) ? 64 : 0,id2,-10))
                                ((enemy*)guys.spr(kids+i))->count_enemy = false;
                                
                            summoned=true;
                            break;
                        }
                    }
                }
                
                if(summoned)
                    sfx.play(x);
            }
        }
        break;
    }
}

void EnemyAttack::update()
{
    if(fType==ft_breath)
    {
        if((breathTimer&3)==0) // Fires every 4 frames
            fireBreath();
    }
    else // Aimed
    {
        if((breathTimer&7)==0) // Fires every 8 frames
            fireAimedBreath();
    }
    
    breathTimer--;
}

bool EnemyAttack::isActive()
{
    if(!(fType==ft_breath || fType==ft_breathAimed))
       return false;
    return breathTimer>0;
}
