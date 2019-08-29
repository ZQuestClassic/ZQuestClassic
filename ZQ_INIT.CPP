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

/*
  extern FONT *lfont;
  extern int jwin_pal[jcMAX];
  extern bool saved;
  zinitdata zinit;
  extern int startdmapxy[6];
  */
extern int dmap_list_size;
extern bool dmap_list_zero;

int onInit()
{
  init_dlg[0].dp2=lfont;
  char init_title[80];
  sprintf(init_title, "Initialization Data");
  init_dlg[0].dp=init_title;

  // equipment
  for(int i=0; i<4; i++)
  {
    init_dlg[i+7].flags = zinit.sword&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<3; i++)
  {
    init_dlg[i+13].flags = zinit.shield&(1<<i) ? D_SELECTED : 0;
  }
  init_dlg[13].flags |= D_DISABLED;
  for(int i=0; i<3; i++)
  {
    init_dlg[i+18].flags = zinit.ring&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<2; i++)
  {
    init_dlg[i+23].flags = zinit.bracelet&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<2; i++)
  {
    init_dlg[i+27].flags = zinit.amulet&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<2; i++)
  {
    init_dlg[i+31].flags = zinit.wallet&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<2; i++)
  {
    init_dlg[i+35].flags = zinit.bow&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+39].flags = zinit.raft&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+40].flags = zinit.ladder&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+41].flags = zinit.book&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+42].flags = zinit.key&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+43].flags = zinit.flippers&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+44].flags = zinit.boots&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<3;i++)
  {
	  init_dlg[i+47].flags = zinit.quiver&(1<<i) ? D_SELECTED : 0;
  }

  // items
  char bombstring[5];
  char maxbombstring[5];
  char sbombstring[5];
  char maxsbombstring[5];
  char arrowstring[5];
  char maxarrowstring[5];
  sprintf(bombstring, "%d", zinit.bombs);
  sprintf(maxbombstring, "%d", zinit.max_bombs);
  sprintf(sbombstring, "%d", zinit.super_bombs);
  sprintf(maxsbombstring, "%d", zinit.max_bombs/4);
  sprintf(arrowstring, "%d", zinit.arrows);
  sprintf(maxarrowstring, "%d", zinit.max_arrows);
  for(int i=0; i<3; i++)
  {
    init_dlg[i+67].flags = zinit.boomerang&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<3; i++)
  {
    init_dlg[i+72].flags = zinit.arrow&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<3; i++)
  {
    init_dlg[i+77].flags = (zinit.potion==i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+82].flags = zinit.whistle&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+85].flags = zinit.hammer&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<2; i++)
  {
    init_dlg[i+88].flags = zinit.hookshot&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<2; i++)
  {
    init_dlg[i+92].flags = zinit.candle&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+96].flags = zinit.bait&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+97].flags = zinit.letter&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+98].flags = zinit.wand&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+99].flags = zinit.lens&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+100].flags = zinit.dins_fire&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+101].flags = zinit.farores_wind&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+102].flags = zinit.nayrus_love&(1<<i) ? D_SELECTED : 0;
  }
    for(int i=0; i<1; i++)
  {
    init_dlg[i+1693].flags = zinit.cbyrna&(1<<i) ? D_SELECTED : 0;
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
    init_dlg[i+631].flags  = get_bit(zinit.map,i) ? D_SELECTED : 0;
    init_dlg[i+887].flags  = get_bit(zinit.compass,i) ? D_SELECTED : 0;
    init_dlg[i+1143].flags = get_bit(zinit.boss_key,i) ? D_SELECTED : 0;
    sprintf(key_list[i], "%d", zinit.level_keys[i]);
    //sprintf(key_list[i], "%d", 0);
    init_dlg[i+1399].dp = key_list[i];
  }

  // misc
  dmap_list_size=MAXDMAPS;
  dmap_list_zero=true;
  init_dlg[1656].d1 = zinit.start_dmap;
  char tempbuf[5];
  char hcstring[5];
  char sheartstring[5];
  char cheartstring[5];
  char keystring[5];
  char rupiestring[5];
  char magicstring[5];
  char maxmagicstring[5];

  sprintf(tempbuf, "0");
  sprintf(hcstring, "%d", zinit.hc);
  sprintf(sheartstring, "%d", zinit.start_heart);
  sprintf(cheartstring, "%d", zinit.cont_heart);
  sprintf(keystring, "%d", zinit.keys);
  sprintf(rupiestring, "%d", zinit.rupies);
  sprintf(magicstring, "%d", zinit.magic);
  sprintf(maxmagicstring, "%d", zinit.max_magic);

  init_dlg[1662].dp=hcstring;
  init_dlg[1663].dp=sheartstring;
  init_dlg[1664].dp=cheartstring;
  init_dlg[1665].dp=keystring;
  init_dlg[1666].dp=rupiestring;
  init_dlg[1667].flags = get_bit(zinit.misc,idM_CONTPERCENT) ? D_SELECTED : 0;

  for (int i=0; i<4; i++)
  {
    init_dlg[i+1670].flags=0;
  }
  init_dlg[zinit.hcp+1670].flags=D_SELECTED;

  for(int i=0; i<8; i++)
  {
    init_dlg[1676+i].flags = get_bit(&zinit.triforce,i) ? D_SELECTED : 0;
  }


  init_dlg[1684].flags = get_bit(zinit.misc,idM_CANSLASH) ? D_SELECTED : 0;
  init_dlg[1689].dp=magicstring;
  init_dlg[1690].dp=maxmagicstring;
  init_dlg[1691].flags = get_bit(zinit.misc,idM_DOUBLEMAGIC) ? D_SELECTED : 0;

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
    saved=false;

    zinit.sword=0;
    for(int i=0; i<4; i++)
    {
      if(init_dlg[i+7].flags & D_SELECTED)
      {
        zinit.sword|=(1<<i);
      }
    }
    zinit.shield=0;
    for(int i=0; i<3; i++)
    {
      if(init_dlg[i+13].flags & D_SELECTED)
      {
        zinit.shield|=(1<<i);
      }
    }
    zinit.ring=0;
    for(int i=0; i<3; i++)
    {
      if(init_dlg[i+18].flags & D_SELECTED)
      {
        zinit.ring|=(1<<i);
      }
    }
    zinit.bracelet=0;
    for(int i=0; i<2; i++)
    {
      if(init_dlg[i+23].flags & D_SELECTED)
      {
        zinit.bracelet|=(1<<i);
      }
    }
    zinit.amulet=0;
    for(int i=0; i<2; i++)
    {
      if(init_dlg[i+27].flags & D_SELECTED)
      {
        zinit.amulet|=(1<<i);
      }
    }
    zinit.wallet=0;
    for(int i=0; i<2; i++)
    {
      if(init_dlg[i+31].flags & D_SELECTED)
      {
        zinit.wallet|=(1<<i);
      }
    }
    zinit.bow=0;
    for(int i=0; i<2; i++)
    {
      if(init_dlg[i+35].flags & D_SELECTED)
      {
        zinit.bow|=(1<<i);
      }
    }
    zinit.raft=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+39].flags & D_SELECTED)
      {
        zinit.raft|=(1<<i);
      }
    }
    zinit.ladder=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+40].flags & D_SELECTED)
      {
        zinit.ladder|=(1<<i);
      }
    }
    zinit.book=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+41].flags & D_SELECTED)
      {
        zinit.book|=(1<<i);
      }
    }
    zinit.key=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+42].flags & D_SELECTED)
      {
        zinit.key|=(1<<i);
      }
    }
    zinit.flippers=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+43].flags & D_SELECTED)
      {
        zinit.flippers|=(1<<i);
      }
    }
    zinit.boots=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+44].flags & D_SELECTED)
      {
        zinit.boots|=(1<<i);
      }
    }
	zinit.quiver=0;
	for(int i=0;i<3; i++)
	{
		if(init_dlg[i+47].flags & D_SELECTED)
		{
			zinit.quiver |= (1<<i);
		}
	}


    // items
    zinit.boomerang=0;
    for(int i=0; i<3; i++)
    {
      if(init_dlg[i+67].flags & D_SELECTED)
      {
        zinit.boomerang|=(1<<i);
      }
    }
    zinit.arrow=0;
    for(int i=0; i<3; i++)
    {
      if(init_dlg[i+72].flags & D_SELECTED)
      {
        zinit.arrow|=(1<<i);
      }
    }
    zinit.potion=0;
    for(int i=0; i<3; i++)
    {
      if(init_dlg[i+77].flags & D_SELECTED)
      {
        zinit.potion=i;
      }
    }
    zinit.whistle=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+82].flags & D_SELECTED)
      {
        zinit.whistle|=(1<<i);
      }
    }
    zinit.hammer=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+85].flags & D_SELECTED)
      {
        zinit.hammer|=(1<<i);
      }
    }
    zinit.hookshot=0;
    for(int i=0; i<2; i++)
    {
      if(init_dlg[i+88].flags & D_SELECTED)
      {
        zinit.hookshot|=(1<<i);
      }
    }
    zinit.candle=0;
    for(int i=0; i<2; i++)
    {
      if(init_dlg[i+92].flags & D_SELECTED)
      {
        zinit.candle|=(1<<i);
      }
    }
    zinit.bait=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+96].flags & D_SELECTED)
      {
        zinit.bait|=(1<<i);
      }
    }
    zinit.letter=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+97].flags & D_SELECTED)
      {
        zinit.letter|=(1<<i);
      }
    }
    zinit.wand=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+98].flags & D_SELECTED)
      {
        zinit.wand|=(1<<i);
      }
    }
    zinit.lens=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+99].flags & D_SELECTED)
      {
        zinit.lens|=(1<<i);
      }
    }
    zinit.dins_fire=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+100].flags & D_SELECTED)
      {
        zinit.dins_fire|=(1<<i);
      }
    }
    zinit.farores_wind=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+101].flags & D_SELECTED)
      {
        zinit.farores_wind|=(1<<i);
      }
    }
    zinit.nayrus_love=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+102].flags & D_SELECTED)
      {
        zinit.nayrus_love|=(1<<i);
      }
    }
	zinit.cbyrna=0;
	for(int i=0; i<1; i++)
	{
	 if(init_dlg[i+1693].flags & D_SELECTED)
	 {
	  zinit.cbyrna|=(1<<i);
	 }
    }
	
    zinit.bombs=atoi(bombstring);
    zinit.max_bombs=atoi(maxbombstring);
    zinit.super_bombs=atoi(sbombstring);
    zinit.arrows=atoi(arrowstring);
    zinit.max_arrows=atoi(maxarrowstring);

    // dmap items
    for(int i=0; i<256; i++)
    {
      set_bit(zinit.map,i,init_dlg[i+631].flags);
      set_bit(zinit.compass,i,init_dlg[i+887].flags);
      set_bit(zinit.boss_key,i,init_dlg[i+1143].flags);
      int numkeys=atoi(key_list[i]);
	  if(numkeys>255) numkeys=255;
	  if(numkeys<0) numkeys=0;
	  zinit.level_keys[i]=numkeys;
    }


    // misc
    zinit.start_dmap = init_dlg[1656].d1;
    zinit.hc = min(atoi(hcstring),get_bit(quest_rules,qr_24HC)?24:16);
    zinit.start_heart = min(atoi(sheartstring),zinit.hc);
    set_bit(zinit.misc,idM_CONTPERCENT,init_dlg[1667].flags);
    if (get_bit(zinit.misc,idM_CONTPERCENT))
    {
      zinit.cont_heart = min(atoi(cheartstring),100);
    }
    else
    {
      zinit.cont_heart = min(atoi(cheartstring),zinit.hc);
    }
    zinit.keys = atoi(keystring);
    zinit.rupies = atoi(rupiestring);

    zinit.hcp=0;
    for(int i=0; i<4; i++)
    {
      if(init_dlg[i+1670].flags & D_SELECTED)
      {
        zinit.hcp=i;
      }
    }

    // triforce
    for(int i=0; i<8; i++)
    {
      set_bit(&zinit.triforce,i,init_dlg[1676+i].flags);
    }


    set_bit(zinit.misc,idM_CANSLASH,init_dlg[1684].flags);
    zinit.max_magic = min(atoi(maxmagicstring),8);
    zinit.magic = min(atoi(magicstring),zinit.max_magic);
    set_bit(zinit.misc,idM_DOUBLEMAGIC,init_dlg[1691].flags);
  }
  return D_O_K;
}


void center_zq_init_dialog()
{
  jwin_center_dialog(init_dlg);
}
