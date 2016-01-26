#include "room.h"

#include "gamedata.h"
#include "guys.h"
#include "items.h"
#include "link.h"
#include "sound.h"
#include "maps.h"
#include "messageManager.h"
#include "zc_sys.h"
#include "zelda.h"
#include <allegro.h>
#include <cmath>
#include <string>

extern LinkClass Link;
extern MessageManager messageMgr;
extern sprite_list items;
RoomItems roomItems;

item* createItem(int id)
{
    item* i=new item(id);
    items.add(i);
    return i;
}

RoomItems::RoomItems():
    //items{ 0, 0, 0 },
    //prices{ 0, 0, 0 },
    numItems(0),
    pricesVisible(true),
    showSignedPrices(false),
    rupeeMarker(0)
{
	items[0] = items[1] = items[2] = 0;
	prices[0] = prices[1] = prices[2] = 0;
}

void RoomItems::initialize()
{
    numItems=0;
    pricesVisible=true;
    showSignedPrices=false;
    if(rupeeMarker)
    {
        rupeeMarker->markForDeletion();
        rupeeMarker=0;
    }
}

void RoomItems::addItem(item* it, int price)
{
    items[numItems]=it;
    prices[numItems]=price;
    numItems++;
    
    it->y=get_bit(quest_rules, qr_NOITEMOFFSET) ? 88 : 89; // Is that right?
    
    // It's a little silly to redo the positions each time, but whatever...
    if(numItems==1)
        it->x=120;
    else if(numItems==2)
    {
        items[0]->x=88;
        items[1]->x=152;
    }
    else // Better be 3
    {
        items[0]->x=88;
        items[1]->x=120;
        items[2]->x=152;
    }
}

void RoomItems::drawPrices(BITMAP* target, int offset)
{
    if(!pricesVisible)
        return;
    
    for(int i=0; i<numItems; i++)
    {
        std::string priceStr;
        if(showSignedPrices)
        {
            if(prices[i]>0)
                priceStr='+';
            else if(prices[i]<0)
                priceStr='-';
        }
        
        char numBuf[6];
        sprintf(numBuf, "%d", static_cast<int>(std::abs(prices[i])));
        priceStr+=numBuf;
        
        int priceX=items[i]->x;
        switch(priceStr.length())
        {
        case 1:
            priceX+=8;
            break;
            
        case 3:
        case 4:
            priceX-=8;
            break;
            
        case 5:
        case 6:
            priceX-=16;
            break;
        }
        
        textout_ex(target, zfont, priceStr.c_str(), priceX, 112+offset, 1, -1);
        
        if(rupeeMarker)
            textout_ex(target, zfont, "X", 64, 112+offset, 1, -1);
    }
}

void RoomItems::clear()
{
    numItems=0;
    pricesVisible=false;
    if(rupeeMarker)
    {
        rupeeMarker->markForDeletion();
        rupeeMarker=0;
    }
}

void RoomItems::setPricesVisible(bool val, bool showRupee)
{
    pricesVisible=val;
    if(showRupee && !rupeeMarker)
    {
        rupeeMarker=createItem(iRupy);
        rupeeMarker->x=48;
        rupeeMarker->y=108; // Should this check the item offset rule?
        rupeeMarker->setPickupFlags(ipDUMMY);
    }
    else if(!showRupee && rupeeMarker)
    {
        rupeeMarker->markForDeletion();
        rupeeMarker=0;
    }
}

namespace
{

void clearRoomItems()
{
    roomItems.clear();
}

void buyShopItem(item* it, int price)
{
    // We've already checked that Link has enough money
    
    if(!current_item_power(itype_wallet)) // Infinite money?
        game->change_drupy(-price);
    
    // Leave the rupee behind. It'll disappear when fadeclk hits 0.
    // Which is another thing that ought to be changed...
    roomItems.setPricesVisible(false, true);
    
    for(int i=0; i<3; i++)
    {
        item* it2=roomItems.getItem(i);
        if(!it2)
            break;
        else if(it2==it)
            continue;
        it2->setPickupFlags(ipDUMMY|ipFADE);
        //it2->onGetDummyMoney.clear();
    }
}

void buyInfo(int infoMsg, int price)
{
    if(game->get_spendable_rupies()<price)
        return;
    
    if(!current_item_power(itype_wallet)) // Infinite money?
        game->change_drupy(-price);
    
    roomItems.setPricesVisible(false);
    messageMgr.showMessage(infoMsg);
    
    for(int i=0; i<3; i++)
    {
        item* it=roomItems.getItem(i);
        if(!it)
            break;
        it->setPickupFlags(ipDUMMY);
        //it->onGetDummyMoney.clear();
    }
}

void getSecretMoney(item* it, int amount)
{
    it->pickup=ipDUMMY;
    roomItems.setPricesVisible(true, false);
    setmapflag();
    if(!current_item_power(itype_wallet))
        game->change_drupy(amount);
}

// There's probably a simpler way to handle this...
const int gambleResults[24][3]={
    { 20, -10, -10 }, { 20, -10, -10 }, { 20, -40, -10}, { 20, -10, -40},
    { 50, -10, -10 }, { 50, -10, -10 }, { 50, -40, -10}, { 50, -10, -40},
    { -10, 20, -10 }, { -10, 20, -10 }, { -40, 20, -10}, { -10, 20, -40},
    { -10, 50, -10 }, { -10, 50, -10 }, { -40, 50, -10}, { -10, 50, -40},
    { -10, -10, 20 }, { -10, -10, 20 }, { -10, -40, 20}, { -40, -10, 20},
    { -10, -10, 50 }, { -10, -10, 50 }, { -10, -40, 50}, { -40, -10, 50}
};

void gamble(int index)
{
    if(game->get_spendable_rupies()<10 &&
      !current_item_power(itype_wallet))
        return;
    
    const int* results=gambleResults[rand()%24];
    for(int i=0; i<3; i++)
        roomItems.setPrice(i, results[i]);
    
    game->change_drupy(results[index]);
    
    for(int i=0; i<3; i++)
    {
        item* it=roomItems.getItem(i);
        if(!it)
            break;
        it->setPickupFlags(ipDUMMY);
        //it->onGetDummyMoney.clear();
    }
}

void moreBombsOrArrows(item* it, int price, bool arrows)
{
    if(game->get_spendable_rupies()<price &&
      !current_item_power(itype_wallet))
        return;
        
    game->change_drupy(-price);
    setmapflag();
    
    if(arrows)
    {
        game->change_maxarrows(10);
        game->set_arrows(game->get_maxarrows());
    }
    else // Bombs
    {
        game->change_maxbombs(4);
        game->set_bombs(game->get_maxbombs());
        {
            int div=zinit.bomb_ratio;
            if(div>0)
                game->change_maxcounter(4/div, 6);
        }
        
        //also give Link an actual Bomb item
        for(int i=0; i<MAXITEMS; i++)
        {
            if(itemsbuf[i].family == itype_bomb && itemsbuf[i].fam_type == 1)
                getitem(i, true);
        }
    }
    
    it->pickup=ipDUMMY|ipFADE;
    fadeclk=66;
    messageMgr.clear(true);
    roomItems.setPricesVisible(false);
    verifyBothWeapons();
}

void leaveMoneyOrLife(item* it, int price)
{
    if(it->id==iRupy)
    {
        if(game->get_spendable_rupies()<price &&
          !current_item_power(itype_wallet))
            return;
        
        game->change_drupy(-price);
    }
    else
    {
        if(game->get_maxlife()<=HP_PER_HEART)
            return;
            
        game->set_life(zc_max(game->get_life()-HP_PER_HEART, 0));
        game->set_maxlife(zc_max(game->get_maxlife()-HP_PER_HEART, HP_PER_HEART));
    }
    
    for(int i=0; i<3; i++)
    {
        item* it2=roomItems.getItem(i);
        if(!it2)
            break;
        it2->setPickupFlags(ipDUMMY|ipFADE);
        //it2->onGetDummyMoney.clear();
    }
    
    roomItems.setPricesVisible(false);
    messageMgr.clear(true);
    fadeclk=66;
    setmapflag();
}

// Adjust for wealth medals. Price should be positive.
int adjustPrice(int price)
{
    if(price==0)
        return 0;
    
    int medalID=current_item_id(itype_wealthmedal);
    
    if(medalID>=0)
    {
        if((itemsbuf[medalID].flags&ITEM_FLAG1)!=0) // Percent
            price=((price*itemsbuf[medalID].misc1)/100);
        else
            price-=itemsbuf[medalID].misc1;
    }
    
    return price;
}

void learnSlash()
{
    game->set_canslash(1);
    sfx(WAV_SCALE);
}

void getHalfMagic()
{
    if(game->get_magicdrainrate()) // ???
        game->set_magicdrainrate(1);
    sfx(WAV_SCALE);
}

void dummyShopItems()
{
    for(int i=0; i<3; i++)
    {
        item* it=roomItems.getItem(i);
        if(it)
            it->pickup&=~ipDUMMY;
    }
}

void payForDoorRepair(int amount)
{
    game->change_drupy(amount);
}

} // End namespace



void setUpRoom(const mapscr& screen)
{
    int repairCharge;
    int message=screen.str;
    item* theItem;
    
    switch(screen.room)
    {
    case rSP_ITEM:
        roomItems.initialize();
        roomItems.setPricesVisible(false);
        
        theItem=createItem(screen.catchall);
        theItem->setPickupFlags(ipONETIME2|ipHOLDUP|ipCHECK|ipSPECIAL);
        theItem->onPickUp=clearRoomItems;
        roomItems.addItem(theItem);
        
        break;
        
    case rINFO:
        {
            const infotype& info=QMisc.info[screen.catchall];
            roomItems.initialize();
            roomItems.setPricesVisible(true);
            roomItems.setSignedPrices(true);
            
            for(int i=0; i<3; i++)
            {
                // The number of items isn't stored anywhere;
                // we just check if the message is non-zero.
                if(info.str[i]==0)
                    break; // continue?
                
                int msg=info.str[i];
                int price=adjustPrice(info.price[i]);
                
                theItem=createItem(iRupy);
                theItem->setPickupFlags(ipMONEY|ipDUMMY);
                //theItem->onGetDummyMoney=fastdelegate::bind(buyInfo, msg, price);
				theItem->onGetDummyMoney= (void*)buyInfo;
               roomItems.addItem(theItem, -price);
            }
            
            break;
        }
    
    case rMONEY:
        {
            roomItems.initialize();
            roomItems.setPricesVisible(false);
            
            int amount=screen.catchall;
            theItem=createItem(iRupy);
            theItem->setPickupFlags(ipONETIME|ipDUMMY|ipMONEY);
            //theItem->onGetDummyMoney=fastdelegate::bind(getSecretMoney, theItem, amount);
			theItem->onGetDummyMoney=(void*)getSecretMoney;
            roomItems.addItem(theItem, amount);
            
            break;
        }
        
    case rGAMBLE:
        roomItems.initialize();
        roomItems.setPricesVisible(true);
        roomItems.setSignedPrices(true);
        
        for(int i=0; i<3; i++)
        {
            theItem=createItem(iRupy);
            theItem->setPickupFlags(ipMONEY|ipDUMMY);
			//theItem->onGetDummyMoney=fastdelegate::bind(gamble, i);
            theItem->onGetDummyMoney=(void*)(gamble);
            roomItems.addItem(theItem, -10);
        }
        
        break;
        
    case rRP_HC:
        roomItems.initialize();
        roomItems.setPricesVisible(false);
        theItem=createItem(iRPotion);
        theItem->setPickupFlags(ipONETIME2|ipHOLDUP|ipFADE);
        roomItems.addItem(theItem);
        theItem=createItem(iHeartC);
        theItem->setPickupFlags(ipONETIME2|ipHOLDUP|ipFADE);
        roomItems.addItem(theItem);
        break;
        
    case rP_SHOP:
        if(current_item(itype_letter)<i_letter_used)
        {
            message=0;
            break;
        }
        
        // Fall through
        
    case rTAKEONE:
    case rSHOP:
    {
        const shoptype& shop=QMisc.shop[screen.catchall];
        roomItems.initialize();
        roomItems.setPricesVisible(screen.room!=rTAKEONE);
        
        for(int i=0; i<3; i++)
        {
            if(shop.hasitem[i]==0)
                break; // continue?
            
            int price;
            if(screen.room!=rTAKEONE)
                price=adjustPrice(shop.price[i]);
            else
                price=0;
            
            int flags=ipHOLDUP|ipFADE;
            if(screen.room==rSHOP)
                flags|=ipCHECK;
            else if(screen.room==rTAKEONE)
                flags|=ipONETIME2;
            else
                // Potion shop items can't be picked up until  the message
                // is finished; this flag will be removed in a callback.
                flags|=ipDUMMY;
            
            theItem=createItem(shop.item[i]);
            theItem->setPickupFlags(flags);
            theItem->setPrice(price); // There's got to be a better way...
           // theItem->onPickUp=fastdelegate::bind(buyShopItem, theItem, price);
			theItem->onPickUp=(void*)(buyShopItem);
            roomItems.addItem(theItem, price);
        }
        
        break;
    }
    
    case rBOMBS:
    case rARROWS:
        {
            roomItems.initialize();
            roomItems.setPricesVisible(true, false);
            roomItems.setSignedPrices(true);
            
            int price=adjustPrice(screen.catchall);
            bool arrows=(screen.room==rARROWS); // Select arrows or bombs
            theItem=createItem(iRupy);
            theItem->setPickupFlags(ipDUMMY|ipMONEY);
            //theItem->onGetDummyMoney=fastdelegate::bind(moreBombsOrArrows, theItem, price, arrows);
			theItem->onGetDummyMoney=(void*)(moreBombsOrArrows);
            roomItems.addItem(theItem, -price);
            
            break;
        }
        
    case rSWINDLE:
        {
            roomItems.initialize();
            roomItems.setPricesVisible(true, false);
            roomItems.setSignedPrices(true);
            
            int price=screen.catchall;
            theItem=createItem(iHeartC);
            theItem->setPickupFlags(ipDUMMY|ipMONEY);
           // theItem->onGetDummyMoney=fastdelegate::bind(leaveMoneyOrLife, theItem, price);
			theItem->onGetDummyMoney=(void*)(leaveMoneyOrLife);
            roomItems.addItem(theItem, -1);
            theItem=createItem(iRupy);
            theItem->setPickupFlags(ipDUMMY|ipMONEY);
           // theItem->onGetDummyMoney=fastdelegate::bind(leaveMoneyOrLife, theItem, price);
			theItem->onGetDummyMoney=(void*)(leaveMoneyOrLife);
            roomItems.addItem(theItem, -price);
            
            break;
        }
        
    case rREPAIR:
        repairCharge=-screen.catchall;
        setmapflag();
        break;
        
    // These are mostly handled later
    case rMUPGRADE:
    case rLEARNSLASH:
        setmapflag(); 
        break;
    }
    
    if(message!=0)
    {
        messageMgr.showMessage(message);
        
        // These should activate even if the player quits mid-message.
        // Currently, they work when F6->Continue is used, but not F6->Save.
        // None of them work with blank messages. That should probably change,
        // but it was the same in 2.50...
        switch(screen.room)
        {
        case rLEARNSLASH:
            messageMgr.setMessageEndCallback(learnSlash);
            break;
            
        case rMUPGRADE:
            messageMgr.setMessageEndCallback(getHalfMagic);
            break;
            
        case rREPAIR:
			__debugbreak();
            //messageMgr.setMessageEndCallback(fastdelegate::bind(payForDoorRepair, repairCharge));
            break;
            
        case rP_SHOP:
            messageMgr.setMessageEndCallback(dummyShopItems);
            break;
        }
        
    }
    else // No message
        Link.unfreeze();
}
