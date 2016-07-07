#include "inventory.h"
#include "guys.h"
#include "link.h"
#include "messageManager.h" // Only for bait
#include "sequence.h"
#include "sound.h"
#include "weapons.h"
#include "zc_sys.h"
#include "zdefs.h"
#include "zelda.h"

extern MessageManager messageMgr; // Only for bait

static fix weaponX(LinkClass& link)
{
    switch(link.getDir())
    {
        case left: return link.getX()-16;
        case right: return link.getX()+16;
        default: return link.getX();
    }
}

static fix weaponY(LinkClass& link)
{
    switch(link.getDir())
    {
        case up: return link.getY()-16;
        case down: return link.getY()+16;
        default: return link.getY();
    }
}

// Common function for items that only require MP to use
static bool canUseWithMP(int id, LinkClass&)
{
    return checkmagiccost(id);
}

static bool dummyTrue(int, LinkClass&)
{
    return true;
}

static bool dummyFalse(int, LinkClass&)
{
    return false;
}

// Sword -----------------------------------------------------------------------

static bool canUseSword(int id, LinkClass& link)
{
    if(!checkmagiccost(id))
        return false;
        
    if((Lwpns.idCount(wBeam)>0 && !link.isCharged())|| Lwpns.idCount(wMagic)>0)
        return false;
    
    return true;
}

static bool activateSword(int id, LinkClass& link)
{
    paymagiccost(id);
    float power;
    
    if((itemsbuf[id].flags&ITEM_FLAG2)!=0)
    {
        power=DAMAGE_MULTIPLIER*itemsbuf[id].power;
        power*=itemsbuf[id].misc2;
        power/=100;
    }
    else
        power=DAMAGE_MULTIPLIER*itemsbuf[id].misc2;
    
    Lwpns.add(new weapon(weaponX(link), weaponY(link), link.getZ(), wBeam,
      itemsbuf[id].fam_type, power, link.getDir(), id, link.getUID()));
    sfx(WAV_BEAM, pan(weaponX(link)));
    return true;
}

// Boomerang -------------------------------------------------------------------

static bool canUseBoomerang(int id, LinkClass& link)
{
    if(Lwpns.idCount(wBrang)>itemsbuf[id].misc2)
        return false;
    
    if(!checkmagiccost(id))
        return false;
    
    return true;
}

static bool activateBoomerang(int id, LinkClass& link)
{
    paymagiccost(id);
    
    weapon* wpn=new weapon(weaponX(link), weaponY(link), link.getZ(), wBrang,
      itemsbuf[id].fam_type, itemsbuf[id].power*DAMAGE_MULTIPLIER,
      link.getDir(), id, link.getUID());
    if(itemsbuf[id].wpn2!=0 || itemsbuf[id].wpn3!=0)
        wpn->setSparkleFunc(standardSparkle);
    Lwpns.add(wpn);
    
    return true;
}

// Arrow -----------------------------------------------------------------------

static bool canUseArrow(int id, LinkClass& link)
{
    if(Lwpns.idCount(wArrow)>itemsbuf[id].misc2)
        return false;
        
    if(!checkmagiccost(id))
        return false;
    
    if(get_bit(quest_rules,qr_TRUEARROWS) && !current_item_power(itype_quiver))
    {
        if(game->get_arrows()<=0)
            return false;
    }
    else if(!current_item_power(itype_quiver) && !current_item_power(itype_wallet))
    {
        if(game->get_drupy()+game->get_rupies()<=0)
            return false;
    }
    
    return true;
}

static bool activateArrow(int id, LinkClass& link)
{
    paymagiccost(id);
    
    if(get_bit(quest_rules,qr_TRUEARROWS))
        game->change_arrows(-1);
    else
        game->change_drupy(-1);
    
    fix wx=weaponX(link);
    weapon* wpn=new weapon(wx, weaponY(link), link.getZ(), wArrow,
      itemsbuf[id].fam_type, DAMAGE_MULTIPLIER*itemsbuf[id].power,
      link.getDir(), id, link.getUID());
    if(itemsbuf[id].wpn2!=0 || itemsbuf[id].wpn3!=0)
        wpn->setSparkleFunc(standardSparkle);
    Lwpns.add(wpn);
    wpn->step*=(current_item_power(itype_bow)+1)/2;
    sfx(itemsbuf[id].usesound, pan(wx));
    return true;
}

// Candle ----------------------------------------------------------------------

static bool canUseCandle(int id, LinkClass& link)
{
    if((itemsbuf[id].flags&ITEM_FLAG1)!=0 && link.didOnThisScreen(did_candle))
        return false;
    
    if(Lwpns.idCount(wFire)>=2)
        return false;
    
    if(!checkmagiccost(id))
        return false;
    
    return true;
}

static bool activateCandle(int id, LinkClass& link)
{
    paymagiccost(id);
    
    if(itemsbuf[id].flags&ITEM_FLAG1)
        link.doOncePerScreen(did_candle);
    
    fix wx=weaponX(link);
    Lwpns.add(new weapon(wx, weaponY(link), link.getZ(), wFire,
      (itemsbuf[id].fam_type>1), //To do with combo flags
      itemsbuf[id].power*DAMAGE_MULTIPLIER, link.getDir(), id, link.getUID()));
    sfx(itemsbuf[id].usesound, pan(wx));
    link.setAttack(wFire);
    return true;
}

// Whistle ---------------------------------------------------------------------

static bool activateWhistle(int id, LinkClass&)
{
    paymagiccost(id);
    beginSpecialSequence(seq_whistle, id);
    return false;
}

// Bait ------------------------------------------------------------------------

static bool canUseBait(int id, LinkClass& link)
{
    if(Lwpns.idCount(wBait))
        return false;
        
    if(!checkmagiccost(id))
        return false;
    
    return true;
}

static bool activateBait(int id, LinkClass& link)
{
    paymagiccost(id);
    sfx(itemsbuf[id].usesound, weaponX(link));
    
    fix wx=weaponX(link), wy=weaponY(link), wz=link.getZ();
    
    // This whole thing should be handled elsewhere, but we'll get to that.
    if(tmpscr->room==rGRUMBLE && !getmapflag())
    {
        items.add(new item(wx, wy, fix(0), iBait, ipDUMMY+ipFADE, 0));
        fadeclk=66;
        messageMgr.clear(true);
        setmapflag();
        removeItemsOfFamily(game,itemsbuf,itype_bait);
        verifyBothWeapons();
        sfx(tmpscr->secretsfx);
        
        // TODO: This doesn't match NES behavior. Link should walk in place
        // while the bait and goriya fade.
        return false;
    }
    
    Lwpns.add(new weapon(wx, wy, wz, wBait, 0, 0, link.getDir(), id, link.getUID()));
    return true;
}

// Letter ----------------------------------------------------------------------

static bool activateLetter(int, LinkClass& link)
{
    if(current_item(itype_letter)==i_letter &&
            tmpscr[currscr<128?0:1].room==rP_SHOP &&
            tmpscr[currscr<128?0:1].guy &&
            ((currscr<128&&!(DMaps[currdmap].flags&dmfGUYCAVES))||(currscr>=128&&DMaps[currdmap].flags&dmfGUYCAVES))
      )
    {
        int usedid = getItemID(itemsbuf, itype_letter,i_letter+1);
        
        if(usedid != -1)
            getitem(usedid, true);
            
        sfx(tmpscr[currscr<128?0:1].secretsfx);
        setupscreen();
        link.setAction(none);
    }
    
    return false;
}

// Potion ----------------------------------------------------------------------

static bool activatePotion(int id, LinkClass&)
{
    paymagiccost(id);
    beginSpecialSequence(seq_potion, id);
    return false;
}

// Wand ------------------------------------------------------------------------

// This thing's a bit of a mess. For now, it works the same as in 2.50.
// MP costs are paid for both the wand and the book, but as long as Link has
// enough MP to pay for each individually, it can still be used.
// The unused spin attack behavior has been removed.

static bool canUseWand(int id, LinkClass& link)
{
    if(Lwpns.idCount(wMagic)>0)
        return false;
    
    // Can the wand fire without the book?
    if((itemsbuf[id].flags&ITEM_FLAG1)==0)
    {
        int book=current_item_id(itype_book);
        if(book<0)
            return false;
        if(!checkmagiccost(book))
            return false;
    }
    
    if(!checkmagiccost(id))
        return false;
    
    return true;
}

static bool activateWand(int id, LinkClass& link)
{
    if(Lwpns.idCount(wBeam)>0)
        Lwpns.del(Lwpns.idFirst(wBeam));
    
    int type, power;
    int book=current_item_id(itype_book);
    if(book>=0)
    {
        type=current_item(itype_book);
        power=current_item_power(itype_book)*DAMAGE_MULTIPLIER;
    }
    else
    {
        type=itemsbuf[id].fam_type;
        power=itemsbuf[id].power*DAMAGE_MULTIPLIER;
    }
    
    paymagiccost(id);
    if(book>=0)
        paymagiccost(book);
    
    Lwpns.add(new weapon(weaponX(link), weaponY(link), link.getZ(),
      wMagic, type, power, link.getDir(), id, link.getUID()));
    
    if(book>=0)
        sfx(itemsbuf[book].usesound, pan(weaponX(link)));
    else
        sfx(itemsbuf[id].usesound, pan(weaponX(link)));
    
    return true;
}

// Hookshot --------------------------------------------------------------------

static bool canUseHookshot(int id, LinkClass& link)
{
    if(Lwpns.idCount(wHookshot))
        return false;
        
    if(!checkmagiccost(id))
        return false;
    
    if(link.isBeingEaten())
        return false;
    
    int x=link.getX();
    int y=link.getY();
    
    for(int i=-1; i<2; i++)
    {
        switch(link.getDir())
        {
        case up:
            if(combobuf[MAPCOMBO2(i, x, y-7)].type==cHSGRAB)
                return false;
            if(_walkflag(x+2, y+4, 1) && !ishookshottable(x, y+4))
            break;
            
        case down:
            if(combobuf[MAPCOMBO2(i, x+12, y+23)].type==cHSGRAB)
                return false;
            break;
            
        case left:
            if(combobuf[MAPCOMBO2(i, x-7, y+12)].type==cHSGRAB)
                return false;
            break;
            
        case right:
            if(combobuf[MAPCOMBO2(i, x+23, y+12)].type==cHSGRAB)
                return false;
            break;
        }
    }
    
    return true;
}

static bool activateHookshot(int id, LinkClass& link)
{
    paymagiccost(id);
    
    int hookitem = itemsbuf[id].fam_type;
    int hookpower = itemsbuf[id].power;
    
    // Wouldn't it be better to consider this a prerequisite for using it?
    if(Lwpns.Count()>=SLMAX)
        Lwpns.del(0);
    if(Lwpns.Count()>=SLMAX-1)
        Lwpns.del(0);
    
    fix wx=weaponX(link), wy=weaponY(link);
    fix wx2=wx, wy2=wy;
    
    switch(link.getDir())
    {
    case up:
        wy2-=4;
        hs_startx=wx;
        hs_starty=wy2;
        break;
        
    case down:
        hs_startx=wx;
        hs_starty=wy;
        if(get_bit(quest_rules,qr_HOOKSHOTDOWNBUG)!=0)
        {
            wy+=4;
            wy2+=4;
        }
        break;
        
    case left:
        wx2-=4;
        hs_startx=wx2;
        hs_starty=wy;
        break;
        
    case right:
        wx2+=4;
        hs_startx=wx2;
        hs_starty=wy;
        break;
    }
    
    int offset=0;
    if(link.getDir()==down && get_bit(quest_rules,qr_HOOKSHOTDOWNBUG)!=0)
        offset=4;
    
    Lwpns.add(new weapon(wx, wy, link.getZ(), wHSHandle, hookitem,
      hookpower*DAMAGE_MULTIPLIER, link.getDir(), id, link.getUID()));
    Lwpns.add(new weapon(wx2, wy2, link.getZ(),wHookshot, hookitem,
      hookpower*DAMAGE_MULTIPLIER, link.getDir(), id, link.getUID()));
    
    hookshot_used=true;
    hookshot_frozen=true;
    return true;
}

// Magic (all three are identical) ---------------------------------------------

bool canUseMagic(int id, LinkClass& link)
{
    if(link.getZ()!=0 || ((tmpscr->flags7&fSIDEVIEW)!=0 && !link.isOnSideviewPlatform()))
        return false;
        
    if(!checkmagiccost(id))
        return false;
    
    return true;
}

bool activateMagic(int id, LinkClass& link)
{
    paymagiccost(id);
    link.setAction(casting);
    magicitem=id;
    return true;
}

// Bombs and super bombs -------------------------------------------------------

static bool canUseBomb(int id, LinkClass&)
{
    bool super;
    int weaponType;
    if(itemsbuf[id].family==itype_bomb)
    {
        super=false;
        weaponType=wLitBomb;
    }
    else // Super bomb
    {
        super=true;
        weaponType=wLitSBomb;
    }
    
    // If we've reached the maximum number of bombs, the item can only be used
    // if it's to detonate them.
    if(Lwpns.idCount(weaponType)>=itemsbuf[id].misc2)
        return itemsbuf[id].misc1==0;
    
    if(!super)
    {
        if(game->get_bombs()==0 && current_item_power(itype_bombbag)==0)
            return false;
    }
    else if(game->get_sbombs()==0)
    {
        if(current_item_power(itype_bombbag)==0)
            return false;
        if((itemsbuf[current_item_id(itype_bombbag)].flags&ITEM_FLAG1)==0)
            return false;
    }
    
    if(!checkmagiccost(id))
        return false;
    
    return true;
}

static void detonateBombs(int type)
{
    weapon *bomb=static_cast<weapon*>(Lwpns.spr(Lwpns.idFirst(type)));
    
    while(Lwpns.idCount(type)>0 && bomb->misc==0)
    {
        bomb->activateRemoteBomb();
        bomb=(weapon*)(Lwpns.spr(Lwpns.idFirst(type)));
    }
}

static bool activateBomb(int id, LinkClass& link)
{
    bool super;
    int weaponType;
    if(itemsbuf[id].family==itype_bomb)
    {
        super=false;
        weaponType=wLitBomb;
    }
    else // Super bomb
    {
        super=true;
        weaponType=wLitSBomb;
    }
    
    if(itemsbuf[id].misc1==0 && Lwpns.idCount(weaponType)>0)
    {
        if(Lwpns.idCount(weaponType)>=itemsbuf[id].misc2) // Reached limit
        {
            detonateBombs(weaponType);
            link.deselectbombs(super);
            return false;
        }
        if(game->get_bombs()==0 && current_item_power(itype_bombbag)==0) // No more to set
        {
            detonateBombs(weaponType);
            link.deselectbombs(super);
            return false;
        }
        // Otherwise, just set another bomb
    }
    
    paymagiccost(id);
    Lwpns.add(new weapon(weaponX(link), weaponY(link), link.getZ(), weaponType,
      itemsbuf[id].fam_type, itemsbuf[id].power*DAMAGE_MULTIPLIER,
      link.getDir(), id, link.getUID()));
    sfx(WAV_PLACE, weaponX(link));
    
    bool magicBag=false;
    if(current_item_power(itype_bombbag)>0)
    {
        if(!super)
            magicBag=true;
        else if((itemsbuf[current_item_id(itype_bombbag)].flags&ITEM_FLAG1)!=0)
            magicBag=true;
    }
    
    if(magicBag)
        return true;
    
    int remainingBombs;
    if(!super)
    {
        game->change_bombs(-1);
        remainingBombs=game->get_bombs();
    }
    else
    {
        game->change_sbombs(-1);
        remainingBombs=game->get_sbombs();
    }
    
    if(remainingBombs==0 && itemsbuf[id].misc1>0)
        // No bombs left and not remotely detonated
        link.deselectbombs(super);
    
    return true;
}

// Cane of Byrna ---------------------------------------------------------------

static bool canUseCaneOfByrna(int id, LinkClass& link)
{
    if(Lwpns.idCount(wCByrna))
        // No MP needed to stop it
        return true;
    
    if(!checkmagiccost(id))
        return false;
    
    return true;
}

static bool activateCaneOfByrna(int id, LinkClass& link)
{
    if(Lwpns.idCount(wCByrna))
    {
        stopCaneOfByrna();
        return false;
    }
    
    paymagiccost(id);
    
    for(int i=0; i<itemsbuf[id].misc3; i++)
    {
        weapon* wpn=new weapon(weaponX(link), weaponY(link), link.getZ(), wCByrna,
          i, itemsbuf[id].power*DAMAGE_MULTIPLIER, link.getDir(), id, link.getUID());
        if(itemsbuf[id].wpn4!=0 || itemsbuf[id].wpn5!=0)
            wpn->setSparkleFunc(byrnaSparkle);
        Lwpns.add(wpn);
    }
    
    return true;
}

// Roc's feather ---------------------------------------------------------------

bool canUseRocsFeather(int id, LinkClass& link)
{
    if(link.getZ()>0 || link.getHoverClk()>0)
        return false;
    
    if(link.isCharging())
        return false;
    
    if(link.isBeingEaten())
        return false;
    
    if((tmpscr->flags7&fSIDEVIEW)!=0)
    {
        if(!link.isOnSideviewPlatform() && !link.ladderIsActive())
            return false;
    }
    
    if(!checkmagiccost(id))
        return false;
    
    return true;
}

bool activateRocsFeather(int id, LinkClass& link)
{
    paymagiccost(id);
    int jump=FEATHERJUMP*(itemsbuf[id].power+2);
    link.jump(fix(jump));
    sfx(itemsbuf[id].usesound, pan(link.getX()));
    return false;
}

// Indices correspond to itype values.
// Hopefully, this can be cleaned up later on.
const ItemUseFuncs itemFuncs[66]={ // 66 is itype_misc
    { canUseSword, activateSword },                 // Sword
    { canUseBoomerang, activateBoomerang },         // Boomerang
    { canUseArrow, activateArrow },                 // Arrow
    { canUseCandle, activateCandle },               // Candle
    { canUseWithMP, activateWhistle },              // Whistle
    { canUseBait, activateBait },                   // Bait
    { dummyTrue, activateLetter },                  // Letter
    { canUseWithMP, activatePotion },               // Potion
    { canUseWand, activateWand },                   // Wand
    { dummyTrue, dummyFalse },                      // Ring
    { dummyTrue, dummyFalse },                      // Wallet
    { dummyTrue, dummyFalse },                      // Amulet
    { dummyTrue, dummyFalse },                      // Shield
    { dummyTrue, dummyFalse },                      // Bow
    { dummyTrue, dummyFalse },                      // Raft
    { dummyTrue, dummyFalse },                      // Ladder
    { dummyTrue, dummyFalse },                      // Book
    { dummyTrue, dummyFalse },                      // Magic key
    { dummyTrue, dummyFalse },                      // Bracelet
    { dummyTrue, dummyFalse },                      // Flippers
    { dummyTrue, dummyFalse },                      // Boots
    { canUseHookshot, activateHookshot },           // Hookshot
    { dummyTrue, dummyFalse },                      // Lens of truth
    { dummyTrue, dummyFalse },                      // Hammer
    { canUseMagic, activateMagic },                 // Din's fire
    { canUseMagic, activateMagic },                 // Farore's wind
    { canUseMagic, activateMagic },                 // Nayru's love
    { canUseBomb, activateBomb },                   // Bomb
    { canUseBomb, activateBomb },                   // Super bomb
    { dummyTrue, dummyFalse },                      // Clock
    { dummyTrue, dummyFalse },                      // Key
    { dummyTrue, dummyFalse },                      // Magic container
    { dummyTrue, dummyFalse },                      // Triforce piece
    { dummyTrue, dummyFalse },                      // Map
    { dummyTrue, dummyFalse },                      // Compass
    { dummyTrue, dummyFalse },                      // Boss key
    { dummyTrue, dummyFalse },                      // Quiver
    { dummyTrue, dummyFalse },                      // Level-specific key
    { canUseCaneOfByrna, activateCaneOfByrna },     // Cane of Byrna
    { dummyTrue, dummyFalse },                      // Rupee
    { dummyTrue, dummyFalse },                      // Arrow ammo
    { dummyTrue, dummyFalse },                      // Fairy
    { dummyTrue, dummyFalse },                      // Magic refill
    { dummyTrue, dummyFalse },                      // Heart
    { dummyTrue, dummyFalse },                      // Heart container
    { dummyTrue, dummyFalse },                      // Heart container piece
    { dummyTrue, dummyFalse },                      // Kill all enemies
    { dummyTrue, dummyFalse },                      // Bomb ammo
    { dummyTrue, dummyFalse },                      // Bomb bag
    { canUseRocsFeather, activateRocsFeather },     // Roc's feather
    { dummyTrue, dummyFalse },                      // Hover boots
    { dummyTrue, dummyFalse },                      // Spin attack scroll
    { dummyTrue, dummyFalse },                      // Cross beams scroll
    { dummyTrue, dummyFalse },                      // Quake hammer scroll
    { dummyTrue, dummyFalse },                      // Whisp ring
    { dummyTrue, dummyFalse },                      // Charge ring
    { dummyTrue, dummyFalse },                      // Peril beam scroll
    { dummyTrue, dummyFalse },                      // Wealth medal
    { dummyTrue, dummyFalse },                      // Heart ring
    { dummyTrue, dummyFalse },                      // Magic ring
    { dummyTrue, dummyFalse },                      // Hurricane spin scroll
    { dummyTrue, dummyFalse },                      // Super quake scroll
    { dummyTrue, dummyFalse },                      // Stone of agony
    { dummyTrue, dummyFalse },                      // Stomp boots
    { dummyTrue, dummyFalse },                      // Whimsical ring
    { dummyTrue, dummyFalse }                       // Peril ring
};
