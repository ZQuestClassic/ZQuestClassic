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

/*
  extern FONT *lfont;
  extern int jwin_pal[jcMAX];
  extern bool saved;
  extern int startdmapxy[6];
  */
int dmap_list_size=0;
bool dmap_list_zero=false;

extern gamedata *game;
int onCheatConsole()
{
  init_dlg[0].dp2=lfont;
  char init_title[80];
  sprintf(init_title, "Initialization Data");
  init_dlg[0].dp=init_title;
  init_dlg[1655].proc=d_dummy_proc;
  init_dlg[1656].proc=d_dummy_proc;
  char *hptitle="HP (hearts):";
  init_dlg[1658].dp=hptitle;
  init_dlg[1659].proc=d_dummy_proc;
  init_dlg[1663].x=init_dlg[0].x+72;
  init_dlg[1664].proc=d_dummy_proc;
  init_dlg[1667].proc=d_dummy_proc;

  // equipment
  for(int i=0; i<4; i++)
  {
    init_dlg[i+7].flags = game->items[itype_sword]&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<3; i++)
  {
    init_dlg[i+13].flags = game->items[itype_shield]&(1<<i) ? D_SELECTED : 0;
  }
  init_dlg[13].flags |= D_DISABLED;
  for(int i=0; i<3; i++)
  {
    init_dlg[i+18].flags = game->items[itype_ring]&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<2; i++)
  {
    init_dlg[i+23].flags = game->items[itype_bracelet]&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<2; i++)
  {
    init_dlg[i+27].flags = game->items[itype_amulet]&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<2; i++)
  {
    init_dlg[i+31].flags = game->items[itype_wallet]&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<2; i++)
  {
    init_dlg[i+35].flags = game->items[itype_bow]&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+39].flags = game->items[itype_raft]&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+40].flags = game->items[itype_ladder]&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+41].flags = game->items[itype_book]&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+42].flags = game->items[itype_magickey]&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+43].flags = game->items[itype_flippers]&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+44].flags = game->items[itype_boots]&(1<<i) ? D_SELECTED : 0;
  }

  // items
  char bombstring[5];
  char maxbombstring[5];
  char sbombstring[5];
  char maxsbombstring[5];
  char arrowstring[5];
  char maxarrowstring[5];
  sprintf(bombstring, "%d", game->items[itype_bomb]);
  sprintf(maxbombstring, "%d", get_gamedata_maxbombs(game));
  sprintf(sbombstring, "%d", get_gamedata_sbombs(game));
  sprintf(maxsbombstring, "%d", get_gamedata_maxbombs(game)/4);
  sprintf(arrowstring, "%d", get_gamedata_arrows(game));
  sprintf(maxarrowstring, "%d", get_gamedata_maxarrows(game));
  for(int i=0; i<3; i++)
  {
    init_dlg[i+67].flags = game->items[itype_brang]&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<3; i++)
  {
    init_dlg[i+72].flags = game->items[itype_arrow]&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<3; i++)
  {
    init_dlg[i+77].flags = (game->items[itype_potion]==i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+82].flags = game->items[itype_whistle]&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+85].flags = game->items[itype_hammer]&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<2; i++)
  {
    init_dlg[i+88].flags = game->items[itype_hookshot]&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<2; i++)
  {
    init_dlg[i+92].flags = game->items[itype_candle]&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+96].flags = game->items[itype_bait]&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+97].flags = game->items[itype_letter]&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+98].flags = game->items[itype_wand]&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+99].flags = game->items[itype_lens]&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+100].flags = game->items[itype_dinsfire]&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+111].flags = game->items[itype_faroreswind]&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+112].flags = game->items[itype_nayruslove]&(1<<i) ? D_SELECTED : 0;
  }
    for(int i=0; i<1; i++)
  {
    init_dlg[i+1693].flags = game->items[itype_cbyrna]&(1<<i) ? D_SELECTED : 0;
  }
  init_dlg[109].dp=bombstring;
  init_dlg[110].dp=maxbombstring;
  init_dlg[115].dp=sbombstring;
  init_dlg[116].dp=maxsbombstring;
  init_dlg[119].dp=arrowstring;
  init_dlg[120].dp=maxarrowstring;

  // dmap items

  char key_list[256][4];
  for(int i=0; i<256; i++)
  {
    init_dlg[i+631].flags  = game->lvlitems[i]&liMAP ? D_SELECTED : 0;
    init_dlg[i+887].flags  = game->lvlitems[i]&liCOMPASS ? D_SELECTED : 0;
    init_dlg[i+1143].flags = game->lvlitems[i]&liBOSSKEY ? D_SELECTED : 0;
    sprintf(key_list[i], "%d", game->lvlkeys[i]);
    //sprintf(key_list[i], "%d", 0);
    init_dlg[i+1399].dp = key_list[i];
  }

  // misc
  //  dmap_list_size=MAXDMAPS;
  //  dmap_list_zero=true;
  //  init_dlg[1656].d1 = zinit.start_dmap;
  char tempbuf[5];
  char hcstring[5];
  char sheartstring[5];
  char cheartstring[5];
  char keystring[5];
  char rupiestring[5];
  char magicstring[5];
  char maxmagicstring[5];

  sprintf(tempbuf, "0");
  sprintf(hcstring, "%d", get_gamedata_maxlife(game)/HP_PER_HEART);
  sprintf(sheartstring, "%d", (get_gamedata_life(game)+(HP_PER_HEART/2)-1)/HP_PER_HEART);
  //  sprintf(cheartstring, "%d", zinit.cont_heart);
  sprintf(keystring, "%d", get_gamedata_keys(game));
  sprintf(rupiestring, "%d", get_gamedata_rupies(game));
  sprintf(magicstring, "%d", get_gamedata_magic(game)/MAGICPERBLOCK);
  sprintf(maxmagicstring, "%d", get_gamedata_maxmagic(game)/MAGICPERBLOCK);

  init_dlg[1662].dp=hcstring;
  init_dlg[1663].dp=sheartstring;
  init_dlg[1664].dp=cheartstring;
  init_dlg[1665].dp=keystring;
  init_dlg[1666].dp=rupiestring;
  //  init_dlg[1667].flags = get_bit(zinit.misc,idM_CONTPERCENT) ? D_SELECTED : 0;

  for (int i=0; i<4; i++)
  {
    init_dlg[i+1670].flags=0;
  }
  init_dlg[get_gamedata_HCpieces(game)+1670].flags=D_SELECTED;

  for(int i=0; i<8; i++)
  {
    init_dlg[1676+i].flags = game->lvlitems[i+1]&liTRIFORCE ? D_SELECTED : 0;
  }


  init_dlg[1684].flags = get_gamedata_canslash(game) ? D_SELECTED : 0;
  init_dlg[1689].dp=magicstring;
  init_dlg[1690].dp=maxmagicstring;
  init_dlg[1691].flags = get_gamedata_magicdrainrate(game) ? D_SELECTED : 0;

  int ret = zc_popup_dialog(init_dlg,1);
  if (ret==2)
  {

    /*
      zinit.=0;
      for(int i=0; i<; i++)
      {
      if(init_dlg[i+].flags & D_SELECTED)
      {
      zinit.|=(1<<i);
      }
      }
      */

    game->items[itype_sword]=0;
    for(int i=0; i<4; i++)
    {
      if(init_dlg[i+7].flags & D_SELECTED)
      {
        game->items[itype_sword]|=(1<<i);
      }
    }
    game->items[itype_shield]=0;
    for(int i=0; i<3; i++)
    {
      if(init_dlg[i+13].flags & D_SELECTED)
      {
        game->items[itype_shield]|=(1<<i);
      }
    }
    game->items[itype_ring]=0;
    for(int i=0; i<3; i++)
    {
      if(init_dlg[i+18].flags & D_SELECTED)
      {
        game->items[itype_ring]|=(1<<i);
      }
    }
    game->items[itype_bracelet]=0;
    for(int i=0; i<2; i++)
    {
      if(init_dlg[i+23].flags & D_SELECTED)
      {
        game->items[itype_bracelet]|=(1<<i);
      }
    }
    game->items[itype_amulet]=0;
    for(int i=0; i<2; i++)
    {
      if(init_dlg[i+27].flags & D_SELECTED)
      {
        game->items[itype_amulet]|=(1<<i);
      }
    }
    game->items[itype_wallet]=0;
    for(int i=0; i<2; i++)
    {
      if(init_dlg[i+31].flags & D_SELECTED)
      {
        game->items[itype_wallet]|=(1<<i);
      }
    }
    game->items[itype_bow]=0;
    for(int i=0; i<2; i++)
    {
      if(init_dlg[i+35].flags & D_SELECTED)
      {
        game->items[itype_bow]|=(1<<i);
      }
    }
    game->items[itype_raft]=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+39].flags & D_SELECTED)
      {
        game->items[itype_raft]|=(1<<i);
      }
    }
    game->items[itype_ladder]=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+40].flags & D_SELECTED)
      {
        game->items[itype_ladder]|=(1<<i);
      }
    }
    game->items[itype_book]=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+41].flags & D_SELECTED)
      {
        game->items[itype_book]|=(1<<i);
      }
    }
    game->items[itype_magickey]=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+42].flags & D_SELECTED)
      {
        game->items[itype_magickey]|=(1<<i);
      }
    }
    game->items[itype_flippers]=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+43].flags & D_SELECTED)
      {
        game->items[itype_flippers]|=(1<<i);
      }
    }
    game->items[itype_boots]=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+44].flags & D_SELECTED)
      {
        game->items[itype_boots]|=(1<<i);
      }
    }


    // items
    game->items[itype_brang]=0;
    for(int i=0; i<3; i++)
    {
      if(init_dlg[i+67].flags & D_SELECTED)
      {
        game->items[itype_brang]|=(1<<i);
      }
    }
    game->items[itype_arrow]=0;
    for(int i=0; i<3; i++)
    {
      if(init_dlg[i+72].flags & D_SELECTED)
      {
        game->items[itype_arrow]|=(1<<i);
      }
    }
    game->items[itype_potion]=0;
    for(int i=0; i<3; i++)
    {
      if(init_dlg[i+77].flags & D_SELECTED)
      {
        game->items[itype_potion]=i;
      }
    }
    game->items[itype_whistle]=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+82].flags & D_SELECTED)
      {
        game->items[itype_whistle]|=(1<<i);
      }
    }
    game->items[itype_hammer]=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+85].flags & D_SELECTED)
      {
        game->items[itype_hammer]|=(1<<i);
      }
    }
    game->items[itype_hookshot]=0;
    for(int i=0; i<2; i++)
    {
      if(init_dlg[i+88].flags & D_SELECTED)
      {
        game->items[itype_hookshot]|=(1<<i);
      }
    }
    game->items[itype_candle]=0;
    for(int i=0; i<2; i++)
    {
      if(init_dlg[i+92].flags & D_SELECTED)
      {
        game->items[itype_candle]|=(1<<i);
      }
    }
    game->items[itype_bait]=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+96].flags & D_SELECTED)
      {
        game->items[itype_bait]|=(1<<i);
      }
    }
    game->items[itype_letter]=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+97].flags & D_SELECTED)
      {
        game->items[itype_letter]|=(1<<i);
      }
    }
    game->items[itype_wand]=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+98].flags & D_SELECTED)
      {
        game->items[itype_wand]|=(1<<i);
      }
    }
    game->items[itype_lens]=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+99].flags & D_SELECTED)
      {
        game->items[itype_lens]|=(1<<i);
      }
    }
    game->items[itype_dinsfire]=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+100].flags & D_SELECTED)
      {
        game->items[itype_dinsfire]|=(1<<i);
      }
    }
    game->items[itype_faroreswind]=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+101].flags & D_SELECTED)
      {
        game->items[itype_faroreswind]|=(1<<i);
      }
    }
    game->items[itype_nayruslove]=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+102].flags & D_SELECTED)
      {
        game->items[itype_nayruslove]|=(1<<i);
      }
    }
	   for(int i=0; i<1; i++)
    {
      if(init_dlg[i+1693].flags & D_SELECTED)
      {
        game->items[itype_cbyrna]|=(1<<i);
      }
    }
    set_gamedata_bombs(game,atoi(bombstring));
    set_gamedata_maxbombs(game,atoi(maxbombstring));
    set_gamedata_sbombs(game,atoi(sbombstring));
    set_gamedata_arrows(game,atoi(arrowstring));
    set_gamedata_maxarrows(game,atoi(maxarrowstring));

    // dmap items
    for (int i=0; i<MAXLEVELS; ++i)
    {
      game->lvlitems[i]&=~liMAP;
      game->lvlitems[i]&=~liCOMPASS;
      game->lvlitems[i]&=~liBOSSKEY;
      game->lvlitems[i]|=init_dlg[i+631].flags&D_SELECTED?liMAP:0;
      game->lvlitems[i]|=init_dlg[i+887].flags&D_SELECTED?liCOMPASS:0;
      game->lvlitems[i]|=init_dlg[i+1143].flags&D_SELECTED?liBOSSKEY:0;
	  int numkeys=atoi(key_list[i]);
	  if(numkeys>255) numkeys=255;
	  if(numkeys<0) numkeys=0;
      game->lvlkeys[i]=numkeys;
    }


    // misc
    //zinit.start_dmap = init_dlg[1656].d1;
    set_gamedata_maxlife(game,min(atoi(hcstring),get_bit(quest_rules,qr_24HC)?24:16)*HP_PER_HEART);
    set_gamedata_life(game, min(atoi(sheartstring)*HP_PER_HEART,get_gamedata_maxlife(game)));
    /*
      set_bit(zinit.misc,idM_CONTPERCENT,init_dlg[1667].flags);
      if (get_bit(zinit.misc,idM_CONTPERCENT))
      {
      zinit.cont_heart = min(atoi(cheartstring),100);
      }
      else
      {
      zinit.cont_heart = min(atoi(cheartstring),zinit.hc);
      }
      */
    set_gamedata_keys(game,atoi(keystring));
    set_gamedata_rupies(game,atoi(rupiestring));

    set_gamedata_HCpieces(game,0);
    for(int i=0; i<4; i++)
    {
      if(init_dlg[i+1670].flags & D_SELECTED)
      {
        set_gamedata_HCpieces(game,i);
      }
    }

    // triforce
    for(int i=0; i<8; i++)
    {
      game->lvlitems[i+1]&=~liTRIFORCE;
      game->lvlitems[i+1]|=init_dlg[1676+i].flags&D_SELECTED?liTRIFORCE:0;
    }


    set_gamedata_canslash(game,init_dlg[1684].flags&D_SELECTED?1:0);
    set_gamedata_maxmagic(game,min(atoi(maxmagicstring),8)*MAGICPERBLOCK);
    set_gamedata_magic(game,min(atoi(magicstring)*MAGICPERBLOCK,get_gamedata_maxmagic(game)));
    set_gamedata_magicdrainrate(game, init_dlg[1691].flags&D_SELECTED?1:0);
  }
  return D_O_K;
}


void center_zq_init_dialog()
{
  jwin_center_dialog(init_dlg);
}
