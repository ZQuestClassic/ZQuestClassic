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
    if(curQuest->itemDefTable().getItemDefinition(id).family==itype_fairy && curQuest->itemDefTable().getItemDefinition(id).misc3>0 && misc>0)
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
    
    if(curQuest->itemDefTable().getItemDefinition(id).family ==itype_fairy && curQuest->itemDefTable().getItemDefinition(id).misc3)
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
        if(clk2>32 || (clk2&2)==0 || curQuest->itemDefTable().getItemDefinition(id).family == itype_fairy)
        {
            sprite::draw(dest);
        }
    }
}

item::item(fix X,fix Y,fix Z,int i,int p,int c, bool isDummy) : sprite(*pool)
{
    x=X;
    y=Y;
    z=Z;
    id=i;
    pickup=p;
    clk=c;
    misc=clk2=0;
    aframe=aclk=0;
    anim=flash=twohand=subscreenItem=false;
    dummy_int[0]=PriceIndex=-1;
    
    if(id<0 || id >= curQuest->itemDefTable().getNumItemDefinitions())
        return;
        
    o_tile = curQuest->itemDefTable().getItemDefinition(id).tile;
    tile = curQuest->itemDefTable().getItemDefinition(id).tile;
    cs = curQuest->itemDefTable().getItemDefinition(id).csets&15;
    o_cset = curQuest->itemDefTable().getItemDefinition(id).csets;
    o_speed = curQuest->itemDefTable().getItemDefinition(id).speed;
    o_delay = curQuest->itemDefTable().getItemDefinition(id).delay;
    frames = curQuest->itemDefTable().getItemDefinition(id).frames;
    flip = curQuest->itemDefTable().getItemDefinition(id).misc>>2;
    
    if(curQuest->itemDefTable().getItemDefinition(id).misc&1)
        flash=true;
        
    if(curQuest->itemDefTable().getItemDefinition(id).misc&2)
        twohand=true;
        
    anim = curQuest->itemDefTable().getItemDefinition(id).frames>0;
    
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
    
    if(!isDummy && curQuest->itemDefTable().getItemDefinition(id).family == itype_fairy && curQuest->itemDefTable().getItemDefinition(id).misc3)
    {
        misc = ++fairy_cnt;
        
        if(addfairy(x, y, curQuest->itemDefTable().getItemDefinition(id).misc3, misc))
            Backend::sfx->play(curQuest->itemDefTable().getItemDefinition(id).usesound,128);
    }
    
    /*for(int j=0;j<8;j++)
    {
      if(j<2) a[j]=itemsbuf[id].initiala[j]*10000;
      d[j]=itemsbuf[id].initiald[j];
    }*/
}

// easy way to draw an item

void putitem(BITMAP *dest,int x,int y,int item_id)
{
    item temp((fix)x,(fix)y,(fix)0,item_id,0,0);
    temp.yofs=0;
    temp.animate(0);
    temp.draw(dest);
}

void putitem2(BITMAP *dest,int x,int y,int item_id, int &aclk, int &aframe, int flash)
{
    item temp((fix)x,(fix)y,(fix)0,item_id,0,0,true);
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

const char *old_weapon_string[wLast] =
{
    "Sword 1 (Wooden)", "Sword 2 (White)", "Sword 3 (Magic)", "Sword 4 (Master)",
    "Boomerang 1 (Wooden)", "Boomerang 2 (Magic)", "Boomerang 3 (Fire)", "Bomb (Normal)",
    "Bomb (Super)", "Explosion (Normal)", "Arrow 1 (Wooden)", "Arrow 2 (Silver)", "Flame", "Whirlwind",
    "Bait", "Wand", "Magic (Normal)", "Fireball", "Rock", "Arrow (Enemy)", "Sword (Enemy)",
    "Magic (Enemy)", "MISC: Spawn", "MISC: Death", "MISC: <unused>", "Hammer",
    "Hookshot 1 (Short) Head", "Hookshot 1 (Short) Chain (Horizontal)", "Hookshot 1 (Short) Handle", "MISC: Arrow 2 (Silver) Sparkle",
    "MISC: Arrow 3 (Golden) Sparkle", "MISC: Boomerang 2 (Magic) Sparkle", "MISC: Boomerang 3 (Fire) Sparkle",
    "MISC: Hammer Impact", "Arrow 3 (Golden)", "Fire (Enemy)", "Whirlwind (Enemy)", "MISC: Magic Meter",
    "Din's Fire (Rising)", "Din's Fire (Falling)","Din's Fire Trail (Rising)", "Din's Fire Trail (Falling)", "Hookshot 1 (Short) Chain (Vertical)", "MISC: More...",
    "MISC: <unused>", "MISC: <unused>", "Sword 1 (Wooden) Slash",
    "Sword 2 (White) Slash", "Sword 3 (Magic) Slash", "Sword 4 (Master) Slash", "MISC: Shadow (Small)",
    "MISC: Shadow (Large)", "MISC: Bush Leaves", "MISC: Flower Clippings",
    "MISC: Grass Clippings", "MISC: Tall Grass", "MISC: Ripples", "MISC: <unused>",
    "Nayru's Love (Left)", "Nayru's Love (Left, Returning)","Nayru's Love Trail (Left)", "Nayru's Love Trail (Left, Returning)",
    "Nayru's Love (Right)", "Nayru's Love (Right, Returning)","Nayru's Love Trail (Right)", "Nayru's Love Trail (Right, Returning)",
    "Nayru's Love Shield (Front)", "Nayru's Love Shield (Back)", "MISC: Subscreen Vine", "Cane of Byrna", "Cane of Byrna (Slash)",
    "Hookshot 2 (Long) Head", "Hookshot 2 (Long) Chain (Horizontal)", "Hookshot 2 (Long) Handle", "Hookshot 2 (Long) Chain (Vertical)",
    "Explosion (Super)", "Bomb (Enemy, Normal)", "Bomb (Enemy, Super)", "Explosion (Enemy, Normal)",  "Explosion (Enemy, Super)",
    "Fire Trail (Enemy)", "Fire 2 (Enemy)", "Fire 2 Trail (Enemy) <Unused>", "Ice Magic (Enemy) <Unused>", "MISC: Hover Boots Glow", "Magic (Fire)", "MISC: Quarter Hearts", "Cane of Byrna (Beam)" /*, "MISC: Sideview Ladder", "MISC: Sideview Raft"*/
};

char *weapon_string[WPNCNT];

