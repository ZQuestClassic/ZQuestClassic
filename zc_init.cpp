//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zquest.cc
//
//  Main code for the quest editor.
//
//--------------------------------------------------------
/*
  #include "zq_custom.h"
  */
#include <stdio.h>

#include "gui.h"
#include "init.h"
#include "zc_alleg.h"
#include "zdefs.h"
#include "jwin.h"
#include "zq_init.h"
#include "zsys.h"
#include "zquest.h"
#include "gamedata.h"
#include "link.h"
#include "zc_init.h"

int dmap_list_size=0;
bool dmap_list_zero=false;

int onCheatConsole()
{
  char init_title[80];
  sprintf(init_title, "Current Data");
  init_dlg[0].dp=init_title;
  zinitdata *zinit2 = copyIntoZinit(game);

  //modify some entries
  init_dlg[1658].dp = (void *)"Current HP (hearts):";
  init_dlg[1658].flags |= D_DISABLED;
  init_dlg[1659].flags |= D_DISABLED;
  init_dlg[1667].flags |= D_DISABLED;
  init_dlg[1664].flags |= D_DISABLED;
  init_dlg[1664].dp = (void *)"";

  int rval = doInit(zinit2);
  resetItems(game, zinit2);
  delete zinit2;
  return rval; 
}


void center_zq_init_dialog()
{
  jwin_center_dialog(init_dlg);
}

void onInitOK()
{
}

zinitdata *copyIntoZinit(gamedata *gdata)
{
	zinitdata *zinit2 = new zinitdata;
  //populate it
  zinit2->hc = get_gamedata_maxlife(gdata)/HP_PER_HEART;
  zinit2->bombs = get_gamedata_bombs(gdata);
  zinit2->keys = get_gamedata_keys(gdata);
  zinit2->max_bombs = get_gamedata_maxbombs(gdata);
  zinit2->super_bombs = get_gamedata_sbombs(gdata);
  zinit2->max_bombs = get_gamedata_maxcounter(gdata,6)<<2;
  zinit2->hcp = get_gamedata_HCpieces(gdata);
  zinit2->rupies = get_gamedata_rupies(gdata);
  for(int i=0; i<MAXLEVELS; i++)
  {
    set_bit(zinit2->map, i, (gdata->lvlitems[i] & liMAP) ? 1 : 0);
    set_bit(zinit2->compass, i, (gdata->lvlitems[i] & liCOMPASS) ? 1 : 0);
    set_bit(zinit2->boss_key, i, (gdata->lvlitems[i] & liBOSSKEY) ? 1 : 0);
    zinit2->level_keys[i] = gdata->lvlkeys[i];
  }
  for(int i=0; i<8; i++)
  {
    set_bit(&zinit2->triforce,i,(gdata->lvlitems[i+1]&liTRIFORCE) ? 1 : 0);
  }
  zinit2->max_magic = get_gamedata_maxmagic(gdata)/MAGICPERBLOCK;
  zinit2->magic = get_gamedata_magic(gdata)/MAGICPERBLOCK;
  set_bit(zinit2->misc, idM_DOUBLEMAGIC, (get_gamedata_magicdrainrate(gdata)==1) ? 1 : 0);
  set_bit(zinit2->misc, idM_CANSLASH, get_gamedata_canslash(gdata));
  
  zinit2->arrows = get_gamedata_arrows(gdata);
  zinit2->max_arrows = get_gamedata_maxarrows(gdata);

  zinit2->max_rupees = get_gamedata_maxcounter(gdata, 1);
  zinit2->max_keys = get_gamedata_maxcounter(gdata, 5);
  
  zinit2->start_heart = get_gamedata_life(gdata)/HP_PER_HEART;
     
  //now set up the items!
  for(int i=0; i<MAXITEMS; i++)
  {
    zinit2->items[i] = gdata->item[i];
  }
  return zinit2;
}

void resetItems(gamedata *gamed)
{
	zinitdata *z = copyIntoZinit(gamed);
	resetItems(gamed, z);
	delete z;
}

void resetItems(gamedata *game2, zinitdata *zinit2)
{
  //First set up the counters
    set_gamedata_maxlife(game2, min(zinit2->hc,get_bit(quest_rules,qr_24HC)?24:16)*HP_PER_HEART);
    set_gamedata_bombs(game2, zinit2->bombs);
    set_gamedata_keys(game2, zinit2->keys);
    set_gamedata_maxbombs(game2, zinit2->max_bombs);
    set_gamedata_sbombs(game2, zinit2->super_bombs);
    set_gamedata_maxcounter(game2, zinit2->max_bombs>>2, 6);
    set_gamedata_HCpieces(game2, zinit2->hcp);
    set_gamedata_rupies(game2, zinit2->rupies);

  for (int i=0; i<MAXITEMS; i++) game2->items_off[i]=0;

    for (int i=0; i<MAXLEVELS; i++)
    {
      game2->lvlitems[i]=0;
      game2->lvlitems[i]|=get_bit(zinit2->map,i)?liMAP:0;
      game2->lvlitems[i]|=get_bit(zinit2->compass,i)?liCOMPASS:0;
      game2->lvlitems[i]|=get_bit(zinit2->boss_key,i)?liBOSSKEY:0;
      game2->lvlkeys[i]=zinit2->level_keys[i];
    }

    for (int i=0; i<8; i++)
    {
      game2->lvlitems[i+1]|=get_bit(&zinit2->triforce,i)?liTRIFORCE:0;
    }
    set_gamedata_maxmagic(game2, min(zinit2->max_magic,8)*MAGICPERBLOCK);
    set_gamedata_magic(game2, min(zinit2->magic,zinit2->max_magic)*MAGICPERBLOCK);
    set_gamedata_magicdrainrate(game2, get_bit(zinit2->misc,idM_DOUBLEMAGIC)?1:2);
    set_gamedata_canslash(game2, get_bit(zinit2->misc,idM_CANSLASH)?1:0);

    set_gamedata_arrows(game2, zinit2->arrows);
    set_gamedata_maxarrows(game2, zinit2->max_arrows);
    
  set_gamedata_maxcounter(game2, zinit2->max_rupees, 1);
  set_gamedata_maxcounter(game2, zinit2->max_keys, 5);
    
  //now set up the items!
  for(int i=0; i<MAXITEMS; i++)
  {
    game2->item[i]=false;
    if(zinit2->items[i])
    {
	  getitem(i,true);
    }
  }
}
