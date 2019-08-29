//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zq_custom.cc
//
//  Custom item, enemy, etc. for ZQuest.
//
//--------------------------------------------------------

#include "gui.h"
#include "zc_custom.h"
#include "zq_custom.h"
#include "tiles.h"
#include "zq_tiles.h"
#include "zq_misc.h"
#include "zquest.h"
#include "items.h"
#include "zsys.h"
#include "sfx.h"
#include <string.h>
#include <stdio.h>
#include <string.h>

extern int ex;
extern void reset_itembuf(itemdata *item, int id);

int d_ecstile_proc(int msg,DIALOG *d,int c);

int d_cstile_proc(int msg,DIALOG *d,int c)
{
  //these are here to bypass compiler warnings about unused arguments
  c=c;

  switch(msg)
  {
    case MSG_CLICK:
    {
      int f = 0;
      int t = d->d1;
      int cs = d->d2;
      if(select_tile(t,f,1,cs,true))
      {
        d->d1 = t;
        d->d2 = cs;
        return D_REDRAW;
      }
    }
    break;

    case MSG_DRAW:
    BITMAP *buf = create_bitmap_ex(8,20,20);
    if(buf)
    {
      clear_bitmap(buf);
      overtile16(buf,d->d1,2,2,d->d2,0);
      jwin_draw_frame(buf,0,0,20,20,FR_DEEP);
      blit(buf,screen,0,0,d->x,d->y,20,20);
      destroy_bitmap(buf);
    }
    //    text_mode(d->bg);
    textprintf_ex(screen,pfont,d->x+24,d->y+2,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Tile: %d",d->d1);
    textprintf_ex(screen,pfont,d->x+24,d->y+8+3,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"CSet: %d",d->d2);
    break;
  }
  return D_O_K;
}



/*****************************/
/******  onCustomItems  ******/
/*****************************/

static int itemdata_gfx_list[] =
{
  // dialog control number
  2, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, -1
};

static int itemdata_action_list[] =
{
  // dialog control number
  18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, -1
};

static TABPANEL itemdata_tabs[] =
{
  // (text)
  { "GFX",       D_SELECTED,   itemdata_gfx_list, 0, NULL },
  { "Action",    0,            itemdata_action_list, 0, NULL },
  { NULL,        0,            NULL,                 0, NULL }
};

static DIALOG itemdata_dlg[] =
{
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
  { jwin_win_proc,      53,   40,  224,  205,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       NULL, NULL, NULL },
  { jwin_tab_proc,      57,   65,  216,  150,   0,       0,      0,       0,          0,             0,       (void *) itemdata_tabs, NULL, (void *)itemdata_dlg },
  { d_cstile_proc,     198,   84,   20,   20,   vc(11),  vc(1),  0,       0,          0,             6,       NULL, NULL, NULL },
  { jwin_button_proc,   90,  220,   61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK", NULL, NULL },
  { jwin_button_proc,  170,  220,   61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel", NULL, NULL },
  { jwin_button_proc,  202,  140,   53,   21,   vc(14),  vc(1),  't',     D_EXIT,     0,             0,       (void *) "&Test", NULL, NULL },
  { jwin_check_proc,   198,  109,   65,    9,   vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Flash ", NULL, NULL },
  { jwin_check_proc,   198,  120,   65,    9,   vc(14),  vc(1),  0,       0,          1,             0,       (void *) "2-Hand", NULL, NULL },
  { jwin_text_proc,     61,   88,   96,    8,   vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Flash CSet:", NULL, NULL },
  { jwin_text_proc,     61,  106,   96,    8,   vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Animation Frames:", NULL, NULL },
  { jwin_text_proc,     61,  124,   96,    8,   vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Animation Speed:", NULL, NULL },
  { jwin_text_proc,     61,  142,   96,    8,   vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Initial Delay:", NULL, NULL },
  { jwin_text_proc,     61,  160,   96,    8,   vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Link Tile Modification:", NULL, NULL },
  { jwin_edit_proc,    160,   84,   35,   16,   vc(12),  vc(1),  0,       0,          2,             0,       NULL, NULL, NULL },
  { jwin_edit_proc,    160,  102,   35,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
  { jwin_edit_proc,    160,  120,   35,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
  { jwin_edit_proc,    160,  138,   35,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
  { jwin_edit_proc,    160,  156,   35,   16,   vc(12),  vc(1),  0,       0,          6,             0,       NULL, NULL, NULL },
  //18
  { jwin_check_proc,    61,  174,  130,    9,   vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Keep Item When Collected", NULL, NULL },
  { jwin_text_proc,     61,   88,   96,    8,   vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Class Number:", NULL, NULL },
  { jwin_text_proc,     61,  106,   96,    8,   vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Class Level:", NULL, NULL },
  { jwin_text_proc,     61,  124,   96,    8,   vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Increase Amount:", NULL, NULL },
  { jwin_text_proc,     61,  142,   96,    8,   vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Counter Reference:", NULL, NULL },
  { jwin_text_proc,     61,  160,   60,    8,   vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Full Max:", NULL, NULL },
  { jwin_text_proc,    160,  160,   60,    8,   vc(14),  vc(1),  0,       0,          0,             0,       (void *) "+Max:", NULL, NULL },
  //25
  { jwin_edit_proc,    160,   84,   35,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
  { jwin_edit_proc,    160,  102,   35,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
  { jwin_edit_proc,    160,  120,   35,   16,   vc(12),  vc(1),  0,       0,          5,             0,       NULL, NULL, NULL },
  { jwin_edit_proc,    160,  138,   35,   16,   vc(12),  vc(1),  0,       0,          2,             0,       NULL, NULL, NULL },
  { jwin_edit_proc,    121,  156,   35,   16,   vc(12),  vc(1),  0,       0,          5,             0,       NULL, NULL, NULL },
  //30
  { jwin_edit_proc,    220,  156,   35,   16,   vc(12),  vc(1),  0,       0,          5,             0,       NULL, NULL, NULL },
  { jwin_check_proc,   200,  124,   60,    9,   vc(14),  vc(1),  0,       0,          1,             0,       (void *) "DCounter", NULL, NULL },
  { jwin_button_proc,  200,	 174,	61,	  21,	vc(14),  vc(1),	 0,		  D_EXIT,	  0,			 0,		  (void *) "Defaults", NULL, NULL },
  { jwin_text_proc,     61,  196,   30,    8,   vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Script:", NULL, NULL },
  { jwin_edit_proc,     91,  192,   35,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
  { jwin_text_proc,    131,  196,   30,    8,   vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Sound:", NULL, NULL },
  { jwin_edit_proc,    161,  192,   35,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
  { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};



void test_item(itemdata test)
{
  itemdata *hold = itemsbuf;
  itemsbuf = &test;
  BITMAP *buf = create_bitmap_ex(8,16,16);

  item temp((fix)0,(fix)0,(fix)0,0,0,0);
  temp.yofs = 0;
  go();
  scare_mouse();
  itemdata_dlg[0].flags=0;
  jwin_win_proc(MSG_DRAW, itemdata_dlg, 0);
  itemdata_dlg[0].flags=D_EXIT;
  //  d_ctext_proc(MSG_DRAW, itemdata_dlg+1, 0);
  unscare_mouse();

  frame=0;
  jwin_draw_frame(screen,150,110,20,20,FR_DEEP);
  for(;;)
  {
    ++frame;
    clear_bitmap(buf);
    temp.animate(0);
    temp.draw(buf);
    vsync();
    scare_mouse();
    blit(buf,screen,0,0,152,112,16,16);
    unscare_mouse();
    if(keypressed())
    {
      readkey();
      break;
    }
    if(gui_mouse_b())
      break;
  }

  comeback();
  while(gui_mouse_b())
    ;

  destroy_bitmap(buf);
  itemsbuf = hold;
}

void edit_itemdata(int index)
{
  char frm[8], spd[8], fcs[8], dly[8], ltm[8];
  char cls[8], cll[8], cnt[8], amt[8], fmx[8], max[8];
  char scr[8], snd[8];

  itemdata_dlg[0].dp = item_string[index];
  itemdata_dlg[0].dp2 = lfont;
  itemdata_dlg[2].d1 = itemsbuf[index].tile;
  itemdata_dlg[2].d2 = itemsbuf[index].csets&15;

  itemdata_dlg[6].flags = (itemsbuf[index].misc&1) ? D_SELECTED : 0;
  itemdata_dlg[7].flags = (itemsbuf[index].misc&2) ? D_SELECTED : 0;

  itemdata_dlg[18].flags = (itemsbuf[index].set_gamedata) ? D_SELECTED : 0;
  itemdata_dlg[31].flags = (itemsbuf[index].amount & 0x8000)  ? D_SELECTED : 0;

  sprintf(fcs,"%d",itemsbuf[index].csets>>4);
  sprintf(frm,"%d",itemsbuf[index].frames);
  sprintf(spd,"%d",itemsbuf[index].speed);
  sprintf(dly,"%d",itemsbuf[index].delay);
  sprintf(ltm,"%ld",itemsbuf[index].ltm);
  itemdata_dlg[13].dp = fcs;
  itemdata_dlg[14].dp = frm;
  itemdata_dlg[15].dp = spd;
  itemdata_dlg[16].dp = dly;
  itemdata_dlg[17].dp = ltm;

  sprintf(cls,"%d",itemsbuf[index].family);
  sprintf(cll,"%d",itemsbuf[index].fam_type);
  sprintf(amt,"%d",itemsbuf[index].amount&0x4000?-(itemsbuf[index].amount&0x3FFF):itemsbuf[index].amount&0x3FFF);
  sprintf(cnt,"%d",itemsbuf[index].count);
  sprintf(fmx,"%d",itemsbuf[index].max);
  sprintf(max,"%d",itemsbuf[index].setmax);
  sprintf(scr,"%d",itemsbuf[index].script);
  sprintf(snd,"%d",itemsbuf[index].playsound);
  itemdata_dlg[25].dp = cls;
  itemdata_dlg[26].dp = cll;
  itemdata_dlg[27].dp = amt;
  itemdata_dlg[28].dp = cnt;
  itemdata_dlg[29].dp = fmx;
  itemdata_dlg[30].dp = max;
  itemdata_dlg[34].dp = scr;
  itemdata_dlg[36].dp = snd;

  int ret;
  itemdata test;
  memset(&test, 0, sizeof(itemdata));
  test.playsound = 25;

  do
  {
    ret = zc_popup_dialog(itemdata_dlg,3);

    test.tile  = itemdata_dlg[2].d1;
    test.csets = itemdata_dlg[2].d2;

    test.misc  = 0;
	test.set_gamedata = 0;
    if(itemdata_dlg[6].flags & D_SELECTED)
      test.misc |= 1;
    if(itemdata_dlg[7].flags & D_SELECTED)
      test.misc |= 2;
	if(itemdata_dlg[18].flags & D_SELECTED)
	  test.set_gamedata = 1;

    test.csets  |= (atoi(fcs)&15)<<4;
    test.frames = min(atoi(frm),255);
    test.speed  = min(atoi(spd),255);
    test.delay  = min(atoi(dly),255);
    test.ltm    = max(min(atol(ltm),NEWMAXTILES-1),0-(NEWMAXTILES-1));

	test.family = vbound(atoi(cls), 0, 255);
	test.fam_type = vbound(atoi(cll), 1, 8);
	test.count = vbound(atoi(cnt), -1, 31);
	test.amount = atoi(amt)<0?-(vbound(atoi(amt), -0x3FFF, 0))|0x4000:vbound(atoi(amt), 0, 0x3FFF);
	test.amount |= (itemdata_dlg[31].flags & D_SELECTED) ? 0x8000 : 0;
	test.setmax = atoi(max);
	test.max = atoi(fmx);
	test.script = vbound(atoi(scr), 0, 255);
	test.playsound = vbound(atoi(snd), 0, 127);

    if(ret==5)
    {
      test_item(test);
      sprintf(fcs,"%d",test.csets>>4);
      sprintf(frm,"%d",test.frames);
      sprintf(spd,"%d",test.speed);
      sprintf(dly,"%d",test.delay);
      sprintf(ltm,"%ld",test.ltm);

	  sprintf(cls,"%d",itemsbuf[index].family);
      sprintf(cll,"%d",itemsbuf[index].fam_type);
      sprintf(amt,"%d",itemsbuf[index].amount&0x4000?-(itemsbuf[index].amount&0x3FFF):itemsbuf[index].amount&0x3FFF);
      sprintf(cnt,"%d",itemsbuf[index].count);
      sprintf(fmx,"%d",itemsbuf[index].max);
      sprintf(max,"%d",itemsbuf[index].setmax);
    }
	if(ret==32)
	{
		memset(&test,0,sizeof(itemdata));
		test.count=-1;
        test.family=0xFF;
        test.playsound=WAV_SCALE;
		reset_itembuf(&test, index);
		sprintf(cls,"%d",itemsbuf[index].family);
		sprintf(cll,"%d",itemsbuf[index].fam_type);
		sprintf(amt,"%d",itemsbuf[index].amount&0x4000?-(itemsbuf[index].amount&0x3FFF):itemsbuf[index].amount&0x3FFF);
		sprintf(cnt,"%d",itemsbuf[index].count);
		sprintf(fmx,"%d",itemsbuf[index].max);
		sprintf(max,"%d",itemsbuf[index].setmax);
		sprintf(scr,"%d",itemsbuf[index].script);
		sprintf(snd,"%d",itemsbuf[index].playsound);
		itemdata_dlg[25].dp = cls;
		itemdata_dlg[26].dp = cll;
		itemdata_dlg[27].dp = amt;
		itemdata_dlg[28].dp = cnt;
		itemdata_dlg[29].dp = fmx;
		itemdata_dlg[30].dp = max;
		itemdata_dlg[34].dp = scr;
		itemdata_dlg[36].dp = snd;
	}

  } while(ret==5 || ret==32);

  if(ret==3)
  {
    itemsbuf[index] = test;
    saved = false;
  }

}

int onCustomItems()
{
  /*
    char *hold = item_string[0];
    item_string[0] = "rupee (1)";
    */

  build_bii_list(false);

  int index = select_item("Select Item",bii[0].i,1);
  while(index >= 0)
  {
    edit_itemdata(index);
    index = select_item("Select Item",index,1);
  }

  refresh(rMAP+rCOMBOS);
  return D_O_K;
}

/****************************/
/******  onCustomWpns  ******/
/****************************/

static DIALOG wpndata_dlg[] =
{
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
  { jwin_win_proc,     55,   40,   210,  143,  vc(14),  vc(1),  0,       D_EXIT,     0,             0,       NULL, NULL, NULL },
  { d_dummy_proc,      160,  68,   0,    8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL, NULL, NULL },
  { d_cstile_proc,     198,  64,   20,   20,   vc(11),  vc(1),  0,       0,          0,             6,       NULL, NULL, NULL },
  { jwin_button_proc,  90,   156,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK", NULL, NULL },
  { jwin_button_proc,  170,  156,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel", NULL, NULL },
  { jwin_check_proc,   198,  89,   65,   9,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Auto-flash", NULL, NULL },
  { jwin_check_proc,   198,  100,  65,   9,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "2P Flash", NULL, NULL },
  { jwin_check_proc,   198,  111,  65,   9,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "H-Flip", NULL, NULL },
  { jwin_check_proc,   198,  122,  65,   9,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "V-Flip", NULL, NULL },
  { jwin_text_proc,    61,   68,   96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Flash CSet:", NULL, NULL },
  { jwin_text_proc,    61,   86,   96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Animation Frames:", NULL, NULL },
  { jwin_text_proc,    61,   104,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Animation Speed:", NULL, NULL },
  { jwin_text_proc,    61,   122,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Miscellaneous Type:", NULL, NULL },
  { jwin_edit_proc,    160,  64,   35,   16,   vc(12),  vc(1),  0,       0,          2,             0,       NULL, NULL, NULL },
  { jwin_edit_proc,    160,  82,   35,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
  { jwin_edit_proc,    160,  100,  35,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
  { jwin_edit_proc,    160,  118,  35,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
  { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};




void edit_weapondata(int index)
{
  char frm[8], spd[8], fcs[8], typ[8];

  wpndata_dlg[0].dp  = weapon_string[index];
  wpndata_dlg[0].dp2 = lfont;
  wpndata_dlg[2].d1  = wpnsbuf[index].tile;
  wpndata_dlg[2].d2  = wpnsbuf[index].csets&15;

  for(int i=0; i<4; i++)
    wpndata_dlg[i+5].flags = (wpnsbuf[index].misc&(1<<i)) ? D_SELECTED : 0;

  sprintf(fcs,"%d",wpnsbuf[index].csets>>4);
  sprintf(frm,"%d",wpnsbuf[index].frames);
  sprintf(spd,"%d",wpnsbuf[index].speed);
  sprintf(typ,"%d",wpnsbuf[index].type);
  wpndata_dlg[13].dp = fcs;
  wpndata_dlg[14].dp = frm;
  wpndata_dlg[15].dp = spd;
  wpndata_dlg[16].dp = typ;

  int ret;
  wpndata test;

  do
  {
    ret = zc_popup_dialog(wpndata_dlg,3);

    test.tile  = wpndata_dlg[2].d1;
    test.csets = wpndata_dlg[2].d2;

    test.misc  = 0;
    for(int i=0; i<4; i++)
      if(wpndata_dlg[i+5].flags & D_SELECTED)
        test.misc |= 1<<i;

      test.csets  |= (atoi(fcs)&15)<<4;
    test.frames = atoi(frm);
    test.speed  = atoi(spd);
    test.type   = atoi(typ);

  } while(ret==17);

  if(ret==3)
  {
    wpnsbuf[index] = test;
    saved = false;
  }
}

int onCustomWpns()
{
  /*
    char *hold = item_string[0];
    item_string[0] = "rupee (1)";
    */

  build_biw_list();

  int index = select_weapon("Select Weapon",biw[0].i);
  while(index >= 0)
  {
    edit_weapondata(index);
    index = select_weapon("Select Weapon",index);
  }

  refresh(rMAP+rCOMBOS);
  return D_O_K;
}

/****************************/
/***** onCustomEnemies  *****/
/****************************/


static int enedata_data_list[] =
{
	2,3,4,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,
	31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,-1
};

static int enedata_data2_list[] =
{
	54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,138,139,140,141,-1
};

static int enedata_flags_list[] =
{
	74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,-1
};

static int enedata_flags2_list[] =
{
	90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,-1
};

static int enedata_flags3_list[] =
{
	106,107,108,109,110,111,112,113,114,115,116,117,118,119,-1
};

static TABPANEL enedata_tabs[] =
{
	{ "Data",	D_SELECTED,		enedata_data_list, 0, NULL },
	{ "Data 2",	   0,	enedata_data2_list,  0, NULL },
	{ "Flags 1",	   0,			enedata_flags_list,  0, NULL },
	{ "Flags 2",	   0,			enedata_flags2_list,  0, NULL },
	{ "Flags 3",	   0,			enedata_flags3_list,  0, NULL },
	{ NULL,        0,            NULL,                 0, NULL }
};

char *enetypelist(int index, int *list_size)
{
  if(index>=0)
  {
	bound(index,0,eeMAX-1);
    return enetype_string[index];
  }
  *list_size=eeMAX;
  return NULL;
}

char *eneanimlist(int index, int *list_size)
{
  if(index>=0)
  {
	bound(index,0,aMAX-1);
    return eneanim_string[index];
  }
  *list_size=aMAX;
  return NULL;
}

char *itemsetlist(int index, int *list_size)
{
  if(index>=0)
  {
	bound(index,0,isMAX-1);
    return itemset_string[index];
  }
  *list_size=isMAX;
  return NULL;
}

char *eweaponlist(int index, int *list_size)
{
  if(index>=0)
  {
    bound(index,0,wMax-wEnemyWeapons-1);
      return eweapon_string[index];
  }
  *list_size=wMax-wEnemyWeapons;
  return NULL;
}

static DIALOG enedata_dlg[] =
{
	{ jwin_win_proc,     0,   0,   320,  240,  vc(14),  vc(1),  0,       D_EXIT,     0,             0,       NULL, NULL, NULL },
	{ jwin_tab_proc,      4,   24,  312,  192,   0,       0,      0,       0,          0,             0,       (void *) enedata_tabs, NULL, (void *)enedata_dlg },
	//2
	{ d_ecstile_proc,     8,  64,   20,   20,   vc(11),  vc(1),  0,       0,          0,             6,       NULL, NULL, NULL },
	{ d_ecstile_proc,     38,  64,   20,   20,   vc(11),  vc(1),  0,       0,          0,             6,       NULL, NULL, NULL },
	{ d_ecstile_proc,     68,  64,   20,   20,   vc(11),  vc(1),  0,       0,          0,             6,       NULL, NULL, NULL },
	//5
	{ jwin_button_proc,  50,   220,  61,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK", NULL, NULL },
    { jwin_button_proc,  130,  220,  61,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel", NULL, NULL },
	//7
	{ jwin_text_proc,    6,   54,  30,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "O.Tile", NULL, NULL },
	{ jwin_text_proc,    36,   54,  30,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "S.Tile", NULL, NULL },
	{ jwin_text_proc,    66,   54,  30,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "E.Tile", NULL, NULL },
	{ jwin_text_proc,    6,   90,  10,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "W:", NULL, NULL },
	{ jwin_text_proc,    6,   102,  10,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "H:", NULL, NULL },
	//12
	{ jwin_text_proc,    36,   90,  10,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "W:", NULL, NULL },
	{ jwin_text_proc,    36,   108,  10,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "H:", NULL, NULL },
	{ jwin_text_proc,    66,   90,  10,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "W:", NULL, NULL },
	{ jwin_text_proc,    66,   108,  10,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "H:", NULL, NULL },
	//16
	{ jwin_text_proc,    100,   54,  30,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Name:", NULL, NULL },
	{ jwin_text_proc,    100,   72,  30,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "HP:", NULL, NULL },
	{ jwin_text_proc,    100,   90,  50,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Damage:", NULL, NULL },
	{ jwin_text_proc,    100,   108,  50,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "W. Damage:", NULL, NULL },
	{ jwin_text_proc,    140,   126,  35,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Hunger:", NULL, NULL },
	//21
	{ jwin_text_proc,    200,   72,  60,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Random Rate:", NULL, NULL },
	{ jwin_text_proc,    200,   90,  60,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Halt Rate:", NULL, NULL },
	{ jwin_text_proc,    200,   108,  60,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Homing Factor:", NULL, NULL },
	{ jwin_text_proc,    200,   126,  60,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Step Speed:", NULL, NULL },
	//25
	{ jwin_text_proc,    6,   126,  45,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Weapon:", NULL, NULL },
	{ jwin_text_proc,    6,   144,  45,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Type:", NULL, NULL },
	{ jwin_text_proc,    6,   162,  45,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "O. Anim:", NULL, NULL },
	{ jwin_text_proc,    6,   180,  45,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "E. Anim:", NULL, NULL },
	{ jwin_text_proc,    6,   198,  45,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Item Set:", NULL, NULL },
	//30
	{ jwin_edit_proc,    16,  86,   20,   16,   vc(12),  vc(1),  0,       0,          2,             0,       NULL, NULL, NULL },
	{ jwin_edit_proc,    16,  104,   20,   16,   vc(12),  vc(1),  0,       0,          2,             0,       NULL, NULL, NULL },
	{ jwin_edit_proc,    46,  86,   20,   16,   vc(12),  vc(1),  0,       0,          2,             0,       NULL, NULL, NULL },
	{ jwin_edit_proc,    46,  104,   20,   16,   vc(12),  vc(1),  0,       0,          2,             0,       NULL, NULL, NULL },
	{ jwin_edit_proc,    76,  86,   20,   16,   vc(12),  vc(1),  0,       0,          2,             0,       NULL, NULL, NULL },
	{ jwin_edit_proc,    76,  104,   20,   16,   vc(12),  vc(1),  0,       0,          2,             0,       NULL, NULL, NULL },
	//36
	{ jwin_edit_proc,    130,  50,   100,   16,   vc(12),  vc(1),  0,       0,          63,             0,       NULL, NULL, NULL },
	{ jwin_edit_proc,    150,  68,   45,   16,   vc(12),  vc(1),  0,       0,          5,             0,       NULL, NULL, NULL },
	{ jwin_edit_proc,    150,  86,   45,   16,   vc(12),  vc(1),  0,       0,          5,             0,       NULL, NULL, NULL },
	{ jwin_edit_proc,    150,  104,   45,   16,   vc(12),  vc(1),  0,       0,          5,             0,       NULL, NULL, NULL },
	{ jwin_edit_proc,    175,  122,   20,   16,   vc(12),  vc(1),  0,       0,          1,             0,       NULL, NULL, NULL },
	//41
	{ jwin_edit_proc,    260,  68,   30,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
	{ jwin_edit_proc,    260,  86,   30,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
	{ jwin_edit_proc,    260,  104,   30,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
	{ jwin_edit_proc,    260,  122,   30,   16,   vc(12),  vc(1),  0,       0,          4,             0,       NULL, NULL, NULL },
	//45
	{ jwin_droplist_proc, 51,  122,   85,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) eweaponlist, NULL, NULL },
	{ jwin_droplist_proc, 51,  140,   184,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) enetypelist, NULL, NULL },
	{ jwin_droplist_proc, 51,  158,   184,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) eneanimlist, NULL, NULL },
	{ jwin_droplist_proc, 51,  176,   184,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) eneanimlist, NULL, NULL },
	{ jwin_droplist_proc, 51,  194,   184,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) itemsetlist, NULL, NULL },
	//50
	{ jwin_text_proc,    240,   144,  40,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "BG SFX:", NULL, NULL },
	{ jwin_text_proc,    240,   162,  40,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Pal CSet:", NULL, NULL },
	{ jwin_edit_proc,    280,  140,   30,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
	{ jwin_edit_proc,    280,  158,   30,   16,   vc(12),  vc(1),  0,       0,          2,             0,       NULL, NULL, NULL },
	//54
	{ jwin_text_proc,    6,   54,  80,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Misc Attribute 1:", NULL, NULL },
	{ jwin_text_proc,    6,   72,  80,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Misc Attribute 2:", NULL, NULL },
	{ jwin_text_proc,    6,   90,  80,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Misc Attribute 3:", NULL, NULL },
	{ jwin_text_proc,    6,   108,  80,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Misc Attribute 4:", NULL, NULL },
	{ jwin_text_proc,    6,   126,  80,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Misc Attribute 5:", NULL, NULL },
	{ jwin_text_proc,    140,   54,  80,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Misc Attribute 6:", NULL, NULL },
	{ jwin_text_proc,    140,   72,  80,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Misc Attribute 7:", NULL, NULL },
	{ jwin_text_proc,    140,   90,  80,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Misc Attribute 8:", NULL, NULL },
	{ jwin_text_proc,    140,   108,  80,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Misc Attribute 9:", NULL, NULL },
	{ jwin_text_proc,    140,   126,  80,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Misc Attribute 10:", NULL, NULL },
	//64
	{ jwin_edit_proc,    86,  50,   45,   16,   vc(12),  vc(1),  0,       0,          6,             0,       NULL, NULL, NULL },
	{ jwin_edit_proc,    86,  68,   45,   16,   vc(12),  vc(1),  0,       0,          6,             0,       NULL, NULL, NULL },
	{ jwin_edit_proc,    86,  86,   45,   16,   vc(12),  vc(1),  0,       0,          6,             0,       NULL, NULL, NULL },
	{ jwin_edit_proc,    86,  104,   45,   16,   vc(12),  vc(1),  0,       0,          6,             0,       NULL, NULL, NULL },
	{ jwin_edit_proc,    86,  122,   45,   16,   vc(12),  vc(1),  0,       0,          6,             0,       NULL, NULL, NULL },
	{ jwin_edit_proc,    220,  50,   45,   16,   vc(12),  vc(1),  0,       0,          6,             0,       NULL, NULL, NULL },
	{ jwin_edit_proc,    220,  68,   45,   16,   vc(12),  vc(1),  0,       0,          6,             0,       NULL, NULL, NULL },
	{ jwin_edit_proc,    220,  86,   45,   16,   vc(12),  vc(1),  0,       0,          6,             0,       NULL, NULL, NULL },
	{ jwin_edit_proc,    220,  104,   45,   16,   vc(12),  vc(1),  0,       0,          6,             0,       NULL, NULL, NULL },
	{ jwin_edit_proc,    220,  122,   45,   16,   vc(12),  vc(1),  0,       0,          6,             0,       NULL, NULL, NULL },
	//74
	{ jwin_check_proc,   6,    50,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Can Be Stunned By Boomerang", NULL, NULL },
	{ jwin_check_proc,   6,    60,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Is Invisible", NULL, NULL },
	{ jwin_check_proc,   6,    70,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Never Returns After Death (Boss)", NULL, NULL },
	{ jwin_check_proc,   6,    80,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Is Invincible", NULL, NULL },
	{ jwin_check_proc,   6,    90,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Is Vulnerable Only to Super Bombs", NULL, NULL },
	{ jwin_check_proc,   6,    100,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Invulnerable to Boomerang", NULL, NULL },
	{ jwin_check_proc,   6,    110,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Invulnerable to Bombs", NULL, NULL },
	{ jwin_check_proc,   6,    120,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Invulnerable to Super Bombs", NULL, NULL },
	//82
	{ jwin_check_proc,   6,    130,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Invulnerable to Arrows", NULL, NULL },
	{ jwin_check_proc,   6,    140,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Invulnerable to L2 Arrows", NULL, NULL },
	{ jwin_check_proc,   6,    150,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Invulnerable to Fire", NULL, NULL },
	{ jwin_check_proc,   6,    160,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Invulnerable to Wand", NULL, NULL },
	{ jwin_check_proc,   6,    170,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Invulnerable to Magic", NULL, NULL },
	{ jwin_check_proc,   6,    180,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Invulnerable to Hookshot", NULL, NULL },
	{ jwin_check_proc,   6,    190,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Invulnerable to Hammer", NULL, NULL },
	{ jwin_check_proc,   6,    200,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Invulnerable to L3 Boomerang", NULL, NULL },
	//90
	{ jwin_check_proc,   6,    50,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Invulnerable to L1 Sword", NULL, NULL },
	{ jwin_check_proc,   6,    60,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Invulnerable to L3 Sword", NULL, NULL },
	{ jwin_check_proc,   6,    70,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Invulnerable to L1 Sword Beam", NULL, NULL },
	{ jwin_check_proc,   6,    80,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Invulnerable to L3 Sword Beam", NULL, NULL },
	{ jwin_check_proc,   6,    90,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Invulnerable to Reflected Beam", NULL, NULL },
	{ jwin_check_proc,   6,    100,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Invulnerable to Reflected Magic", NULL, NULL },
	{ jwin_check_proc,   6,    110,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Invulnerable to Reflected Fire Ball", NULL, NULL },
	{ jwin_check_proc,   6,    120,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Extra Unused Flag", NULL, NULL },
	//98
	{ jwin_check_proc,   6,    130,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Invulnerable In Front", NULL, NULL },
	{ jwin_check_proc,   6,    140,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Invulnerable In Left", NULL, NULL },
	{ jwin_check_proc,   6,    150,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Invulnerable In Right", NULL, NULL },
	{ jwin_check_proc,   6,    160,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Invulnerable In Back", NULL, NULL },
	{ jwin_check_proc,   6,    170,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Weak To Whistle", NULL, NULL },
	{ jwin_check_proc,   6,    180,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Weak to Arrow", NULL, NULL },
	{ jwin_check_proc,   6,    190,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Weak to L3 Boomerang", NULL, NULL },
	{ jwin_check_proc,   6,    200,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Can Only Be Seen By Lens Of Truth", NULL, NULL },
	//106
	{ jwin_check_proc,   6,    50,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Is Flashing", NULL, NULL },
	{ jwin_check_proc,   6,    60,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Spawned By \"Zora\" Enemy Flag", NULL, NULL },
	{ jwin_check_proc,   6,    70,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Spawned By \"Falling Rocks\" Enemy Flag", NULL, NULL },
	{ jwin_check_proc,   6,    80,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Spawned By \"Corner Traps\" Enemy Flag", NULL, NULL },
	{ jwin_check_proc,   6,    90,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Spawned By \"Horz Trap\" Combo Type", NULL, NULL },
	{ jwin_check_proc,   6,    100,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Spawned By \"Vert Trap\" Combo Type", NULL, NULL },
	{ jwin_check_proc,   6,    110,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Spawned By \"4-Way Trap\" Combo Type", NULL, NULL },
	{ jwin_check_proc,   6,    120,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Spawned By \"LR Trap\" Combo Type", NULL, NULL },
	//114
	{ jwin_check_proc,   6,    130,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Spawned By \"UD Trap\" Combo Type", NULL, NULL },
	{ jwin_check_proc,   6,    140,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Spawned By \"Middle Traps\" Enemy Flag", NULL, NULL },
	{ jwin_check_proc,   6,    150,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Spawned By \"Statue Fire\" Enemy Flag", NULL, NULL },
	{ jwin_check_proc,   6,    160,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Spawned By \"Armos\" Combo Type", NULL, NULL },
	{ jwin_check_proc,   6,    170,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Spawned By \"Grave\" Combo Type", NULL, NULL },
	{ jwin_check_proc,   6,    180,   280,   9,   vc(14),   vc(1),  0,         0,         1,            0,       (void *) "Spawned By \"Ganon\" Room Type", NULL, NULL },
	{ d_dummy_proc,      160,   40,     0,    8,   vc(15),              vc(1),                 0,       0,          0,             0,       NULL, NULL, NULL },
	{ d_dummy_proc,      160,   40,     0,    8,   vc(15),              vc(1),                 0,       0,          0,             0,       NULL, NULL, NULL },
	//122
	{ d_dummy_proc,      160,   40,     0,    8,   vc(15),              vc(1),                 0,       0,          0,             0,       NULL, NULL, NULL },
	{ d_dummy_proc,      160,   40,     0,    8,   vc(15),              vc(1),                 0,       0,          0,             0,       NULL, NULL, NULL },
	{ d_dummy_proc,      160,   40,     0,    8,   vc(15),              vc(1),                 0,       0,          0,             0,       NULL, NULL, NULL },
	{ d_dummy_proc,      160,   40,     0,    8,   vc(15),              vc(1),                 0,       0,          0,             0,       NULL, NULL, NULL },
	{ d_dummy_proc,      160,   40,     0,    8,   vc(15),              vc(1),                 0,       0,          0,             0,       NULL, NULL, NULL },
	{ d_dummy_proc,      160,   40,     0,    8,   vc(15),              vc(1),                 0,       0,          0,             0,       NULL, NULL, NULL },
	{ d_dummy_proc,      160,   40,     0,    8,   vc(15),              vc(1),                 0,       0,          0,             0,       NULL, NULL, NULL },
	{ d_dummy_proc,      160,   40,     0,    8,   vc(15),              vc(1),                 0,       0,          0,             0,       NULL, NULL, NULL },
	//130
	{ d_dummy_proc,      160,   40,     0,    8,   vc(15),              vc(1),                 0,       0,          0,             0,       NULL, NULL, NULL },
	{ d_dummy_proc,      160,   40,     0,    8,   vc(15),              vc(1),                 0,       0,          0,             0,       NULL, NULL, NULL },
	{ d_dummy_proc,      160,   40,     0,    8,   vc(15),              vc(1),                 0,       0,          0,             0,       NULL, NULL, NULL },
	{ d_dummy_proc,      160,   40,     0,    8,   vc(15),              vc(1),                 0,       0,          0,             0,       NULL, NULL, NULL },
	{ d_dummy_proc,      160,   40,     0,    8,   vc(15),              vc(1),                 0,       0,          0,             0,       NULL, NULL, NULL },
	{ d_dummy_proc,      160,   40,     0,    8,   vc(15),              vc(1),                 0,       0,          0,             0,       NULL, NULL, NULL },
	{ d_dummy_proc,      160,   40,     0,    8,   vc(15),              vc(1),                 0,       0,          0,             0,       NULL, NULL, NULL },
	{ d_dummy_proc,      160,   40,     0,    8,   vc(15),              vc(1),                 0,       0,          0,             0,       NULL, NULL, NULL },
	//138
	{ jwin_text_proc,    6,   144,  80,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "O. Frame Rate:", NULL, NULL },
	{ jwin_text_proc,    140,   144,  80,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "E. Frame Rate:", NULL, NULL },
	{ jwin_edit_proc,    86,  140,   45,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
	{ jwin_edit_proc,    220,  140,   45,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
	{ NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

int d_ecstile_proc(int msg,DIALOG *d,int c)
{
  //these are here to bypass compiler warnings about unused arguments
  c=c;

  switch(msg)
  {
    case MSG_CLICK:
    {
      int f = 0;
      int t = d->d1;
      int cs = d->d2;
      if(select_tile(t,f,1,cs,true))
      {
        d->d1 = t;
        enedata_dlg[2].d2 = cs;
		enedata_dlg[3].d2 = cs;
		enedata_dlg[4].d2 = cs;
        return D_REDRAW;
      }
    }
    break;

    case MSG_DRAW:
    BITMAP *buf = create_bitmap_ex(8,20,20);
    if(buf)
    {
      clear_bitmap(buf);
      overtile16(buf,d->d1,2,2,d->d2,0);
      jwin_draw_frame(buf,0,0,20,20,FR_DEEP);
      blit(buf,screen,0,0,d->x,d->y,20,20);
      destroy_bitmap(buf);
    }
    //    text_mode(d->bg);
    break;
  }
  return D_O_K;
}

void edit_enemydata(int index)
{
  char hp[8], dp[8], wdp[8], rat[8], hrt[8], hom[8], grm[8], spd[8],
	  frt[8], efr[8], sfx[8], bsp[8];
  char w[8],h[8],sw[8],sh[8],ew[8],eh[8];
  char name[64];
  char ms1[8], ms2[8], ms3[8], ms4[8], ms5[8], ms6[8], ms7[8], ms8[8], ms9[8], ms10[8];

  enedata_dlg[0].dp = guy_string[index];
  enedata_dlg[0].dp2 = lfont;
  enedata_dlg[2].d1 = guysbuf[index].tile;
  enedata_dlg[2].d2 = guysbuf[index].cset;
  enedata_dlg[3].d1 = guysbuf[index].s_tile;
  enedata_dlg[3].d2 = guysbuf[index].cset;
  enedata_dlg[4].d1 = guysbuf[index].e_tile;
  enedata_dlg[4].d2 = guysbuf[index].cset;

  enedata_dlg[45].d1 = guysbuf[index].weapon != wNone ? guysbuf[index].weapon - wEnemyWeapons : 0;
  enedata_dlg[46].d1 = guysbuf[index].family;
  enedata_dlg[47].d1 = guysbuf[index].anim;
  enedata_dlg[48].d1 = guysbuf[index].e_anim;
  enedata_dlg[49].d1 = guysbuf[index].item_set;


  sprintf(w,"%d",guysbuf[index].width);
  sprintf(h,"%d",guysbuf[index].height);
  sprintf(sw,"%d",guysbuf[index].s_width);
  sprintf(sh,"%d",guysbuf[index].s_height);
  sprintf(ew,"%d",guysbuf[index].e_width);
  sprintf(eh,"%d",guysbuf[index].e_height);
  enedata_dlg[30].dp = w;
  enedata_dlg[31].dp = h;
  enedata_dlg[32].dp = sw;
  enedata_dlg[33].dp = sh;
  enedata_dlg[34].dp = ew;
  enedata_dlg[35].dp = eh;

  sprintf(hp,"%d",guysbuf[index].hp);
  sprintf(dp,"%d",guysbuf[index].dp);
  sprintf(wdp,"%d",guysbuf[index].wdp);
  sprintf(grm,"%d",guysbuf[index].grumble);
  enedata_dlg[37].dp = hp;
  enedata_dlg[38].dp = dp;
  enedata_dlg[39].dp = wdp;
  enedata_dlg[40].dp = grm;

  sprintf(rat,"%d",guysbuf[index].rate);
  sprintf(hrt,"%d",guysbuf[index].hrate);
  sprintf(hom,"%d",guysbuf[index].homing);
  sprintf(spd,"%d",guysbuf[index].step);
  enedata_dlg[41].dp = rat;
  enedata_dlg[42].dp = hrt;
  enedata_dlg[43].dp = hom;
  enedata_dlg[44].dp = spd;

  sprintf(name,"%s",guy_string[index]);
  enedata_dlg[36].dp = name;

  sprintf(ms1,"%d",guysbuf[index].misc1);
  sprintf(ms2,"%d",guysbuf[index].misc2);
  sprintf(ms3,"%d",guysbuf[index].misc3);
  sprintf(ms4,"%d",guysbuf[index].misc4);
  sprintf(ms5,"%d",guysbuf[index].misc5);
  sprintf(ms6,"%d",guysbuf[index].misc6);
  sprintf(ms7,"%d",guysbuf[index].misc7);
  sprintf(ms8,"%d",guysbuf[index].misc8);
  sprintf(ms9,"%d",guysbuf[index].misc9);
  sprintf(ms10,"%d",guysbuf[index].misc10);
  enedata_dlg[64].dp = ms1;
  enedata_dlg[65].dp = ms2;
  enedata_dlg[66].dp = ms3;
  enedata_dlg[67].dp = ms4;
  enedata_dlg[68].dp = ms5;
  enedata_dlg[69].dp = ms6;
  enedata_dlg[70].dp = ms7;
  enedata_dlg[71].dp = ms8;
  enedata_dlg[72].dp = ms9;
  enedata_dlg[73].dp = ms10;

  sprintf(frt,"%d",guysbuf[index].frate);
  sprintf(efr,"%d",guysbuf[index].e_frate);
  enedata_dlg[140].dp = frt;
  enedata_dlg[141].dp = efr;

  sprintf(sfx,"%d",guysbuf[index].bgsfx);
  sprintf(bsp,"%d",guysbuf[index].bosspal);
  enedata_dlg[52].dp = sfx;
  enedata_dlg[53].dp = bsp;

  for(int i=0;i<32;i++)
	  enedata_dlg[74+i].flags = (guysbuf[index].flags & (1<<i)) ? D_SELECTED : 0;

  for(int i=0;i<14;i++)
	  enedata_dlg[106+i].flags = (guysbuf[index].flags2 & (1<<i)) ? D_SELECTED : 0;

  int ret;
  guydata test;
  memset(&test, 0, sizeof(guydata));

  do
  {
    ret = zc_popup_dialog(enedata_dlg,3);

    test.tile  = enedata_dlg[2].d1;
    test.cset = enedata_dlg[2].d2;
	test.s_tile  = enedata_dlg[3].d1;
	test.e_tile  = enedata_dlg[4].d1;

	test.width = vbound(atoi(w),0,20);
	test.height = vbound(atoi(h),0,20);
	test.s_width = vbound(atoi(sw),0,20);
	test.s_height = vbound(atoi(sh),0,20);
	test.e_width = vbound(atoi(ew),0,20);
	test.e_height = vbound(atoi(eh),0,20);

	test.weapon = enedata_dlg[45].d1 != 0 ? enedata_dlg[45].d1 + wEnemyWeapons : wNone;
	test.family = enedata_dlg[46].d1;
	test.anim = enedata_dlg[47].d1;
	test.e_anim = enedata_dlg[48].d1;
	test.item_set = enedata_dlg[49].d1;

	test.hp = vbound(atoi(hp), 0, 32767);
	test.dp = vbound(atoi(dp), 0, 32767);
	test.wdp = vbound(atoi(wdp), 0, 32767);
	test.grumble = vbound(atoi(grm), 0, 4);

	test.rate = vbound(atoi(rat), 0, 16);
	test.hrate = vbound(atoi(hrt), 0, 16);
	test.homing = vbound(atoi(hom), 0, 256);
	test.step = vbound(atoi(spd),0,1000);

	test.frate = vbound(atoi(frt),0,256);
	test.e_frate = vbound(atoi(efr),0,256);
	test.bgsfx = vbound(atoi(sfx),-1,255);
	test.bosspal = vbound(atoi(bsp),-1,15);

	test.misc1 = (short) atoi(ms1);
	test.misc2 = (short) atoi(ms2);
	test.misc3 = (short) atoi(ms3);
	test.misc4 = (short) atoi(ms4);
	test.misc5 = (short) atoi(ms5);
	test.misc6 = (short) atoi(ms6);
	test.misc7 = (short) atoi(ms7);
	test.misc8 = (short) atoi(ms8);
	test.misc9 = (short) atoi(ms9);
	test.misc10 = (short) atoi(ms10);

	strcpy(guy_string[index],name);

	for(int i=0;i<32;i++)
		test.flags |= (enedata_dlg[74+i].flags & D_SELECTED) ? (1<<i) : 0;

	for(int i=0;i<14;i++)
		test.flags2 |= (enedata_dlg[106+i].flags & D_SELECTED) ? (1<<i) : 0;

  } while(false);

  if(ret==5)
  {
    guysbuf[index] = test;
    saved = false;
  }

}

int onCustomEnemies()
{
  /*
    char *hold = item_string[0];
    item_string[0] = "rupee (1)";
    */

  build_bie_list(false);

  int index = select_enemy("Select Enemy",bie[0].i,"Edit","Done");
  while(index >= 0)
  {
    edit_enemydata(index);
    index = select_enemy("Select Enemy",index,"Edit","Done");
  }

  refresh(rMAP+rCOMBOS);
  return D_O_K;
}


int onCustomGuys()
{
  return D_O_K;
}

int d_ltile_proc(int msg,DIALOG *d,int c)
{
  //these are here to bypass compiler warnings about unused arguments
  c=c;

  //d1=dir
  //d2=type (determines how to animate)
  //fg=cset (6)
  enum {lt_clock, lt_tile, lt_flip, lt_extend};
  static int bg=makecol(0, 0, 0);
  int *p=(int*)d->dp3;
  int oldtile=0;
  int oldflip=0;

  switch(msg)
  {
    case MSG_START:
    {
      d->dp3=(int*)malloc(sizeof(int)*4);
      p=(int*)d->dp3;
      p[lt_clock]=0;
      p[lt_tile]=0;
      p[lt_flip]=0;
      p[lt_extend]=0;
      linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], d->d2, d->d1, zinit.linkanimationstyle);
      break;
    }
    case MSG_CLICK:
    {
      int t;
      int f;
      int extend;
      int cs = 6;
      linktile(&t, &f, &extend, d->d2, d->d1, zinit.linkanimationstyle);
      switch(extend)
      {
        case 0:
          if (!isinRect(gui_mouse_x(),gui_mouse_y(),d->x+2+8, d->y+2+16, d->x+33-8, d->y+33))
          {
            return D_O_K;
          }
          break;
        case 1:
          if (!isinRect(gui_mouse_x(),gui_mouse_y(),d->x+2+8, d->y+2, d->x+33-8, d->y+33))
          {
            return D_O_K;
          }
          break;
        case 2:
          if (!isinRect(gui_mouse_x(),gui_mouse_y(),d->x+2, d->y+2, d->x+33, d->y+33))
          {
            return D_O_K;
          }
          break;
      }
      if(select_tile(t,f,2,cs,false,extend))
      {
        extend=ex;
        setlinktile(t,f,extend,d->d2,d->d1);
        return D_REDRAW;
      }
    }
    break;

    case MSG_VSYNC:
    oldtile=p[lt_tile];
    oldflip=p[lt_flip];
    p[lt_clock]++;
    switch (zinit.linkanimationstyle)
    {
      case las_original:                                             //2-frame
        switch (d->d2)
        {
          case ls_charge:
          case ls_walk:
            linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], d->d2, d->d1, zinit.linkanimationstyle);
            if (p[lt_clock]>=6)
            {
              if (d->d1==up&&d->d2==ls_walk)
              {
                p[lt_flip]=1;                //h flip
              }
              else
              {
                p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;               //tile++
              }
            };
            if (p[lt_clock]>=11)
            {
              p[lt_clock]=-1;
            }
            break;
          case ls_jump:
            if (p[lt_clock]>=24)
            {
              linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
              if (p[lt_clock]>=36)
              {
                p[lt_clock]=-1;
              }
            }
            else
            {
              linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_jump, d->d1, zinit.linkanimationstyle);
              p[lt_tile]+=p[lt_extend]==2?((int)p[lt_clock]/8)*2:((int)p[lt_clock]/8);
            }
            break;
          case ls_slash:
            if (p[lt_clock]<6)
            {
              linktile(&p[lt_tile], &p[lt_flip], ls_slash, d->d1, zinit.linkanimationstyle);
            }
            else if (p[lt_clock]<12)
              {
                linktile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.linkanimationstyle);
              }
              else if (p[lt_clock]<13)
                {
                  linktile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.linkanimationstyle);
                }
                else
                {
                  linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
                  p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
                  if (p[lt_clock]>=16)
                  {
                    p[lt_clock]=-1;
                  }
                };
            break;
          case ls_stab:
            if (p[lt_clock]<12)
            {
              linktile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.linkanimationstyle);
            }
            else if (p[lt_clock]<13)
              {
                linktile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.linkanimationstyle);
              }
              else
              {
                linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
                p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
                if (p[lt_clock]>=16)
                {
                  p[lt_clock]=-1;
                }
              };
            break;
          case ls_pound:
            if (p[lt_clock]<12)
            {
              linktile(&p[lt_tile], &p[lt_flip], ls_pound, d->d1, zinit.linkanimationstyle);
            }
            else if (p[lt_clock]<30)
              {
                linktile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.linkanimationstyle);
              }
              else
              {
                linktile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.linkanimationstyle);
                if (p[lt_clock]>=31)
                {
                  p[lt_clock]=-1;
                }
              };
            break;
          case ls_float:
            linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_float, d->d1, zinit.linkanimationstyle);
            if (p[lt_clock]>=12)
            {
              p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
            };
            if (p[lt_clock]>=23)
            {
              p[lt_clock]=-1;
            }
            break;
          case ls_swim:
            linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_swim, d->d1, zinit.linkanimationstyle);
            if (p[lt_clock]>=12)
            {
              p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
            };
            if (p[lt_clock]>=23)
            {
              p[lt_clock]=-1;
            }
            break;
          case ls_dive:
            linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_dive, d->d1, zinit.linkanimationstyle);
            if (p[lt_clock]>=50)
            {
              linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_float, d->d1, zinit.linkanimationstyle);
            };
            if ((p[lt_clock]/12)&1)
            {
              p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
            };
            if (p[lt_clock]>=81)
            {
              p[lt_clock]=-1;
            }
            break;
          case ls_landhold1:
            linktile(&p[lt_tile], &p[lt_flip], ls_landhold1, d->d1, zinit.linkanimationstyle);
            break;
          case ls_landhold2:
            linktile(&p[lt_tile], &p[lt_flip], ls_landhold2, d->d1, zinit.linkanimationstyle);
            break;
          case ls_waterhold1:
            linktile(&p[lt_tile], &p[lt_flip], ls_waterhold1, d->d1, zinit.linkanimationstyle);
            break;
          case ls_waterhold2:
            linktile(&p[lt_tile], &p[lt_flip], ls_waterhold2, d->d1, zinit.linkanimationstyle);
            break;
          case ls_cast:
            linktile(&p[lt_tile], &p[lt_flip], ls_cast, d->d1, zinit.linkanimationstyle);
            if (p[lt_clock]<96)
            {
              linktile(&p[lt_tile], &p[lt_flip], ls_landhold2, d->d1, zinit.linkanimationstyle);
            };
            if (p[lt_clock]>=194)
            {
              p[lt_clock]=-1;
            }
            break;
          default:
            break;
        }
        break;
      case las_bszelda:                                             //3-frame BS
        switch (d->d2)
        {
          case ls_charge:
          case ls_walk:
            linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], d->d2, d->d1, zinit.linkanimationstyle);
            p[lt_tile]+=anim_3_4(p[lt_clock],7)*(p[lt_extend]==2?2:1);
            if (p[lt_clock]>=27)
            {
              p[lt_clock]=-1;
            }
            break;
          case ls_jump:
            if (p[lt_clock]>=24)
            {
              linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
              if (p[lt_clock]>=36)
              {
                p[lt_clock]=-1;
              }
            }
            else
            {
              linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_jump, d->d1, zinit.linkanimationstyle);
              p[lt_tile]+=p[lt_extend]==2?((int)p[lt_clock]/8)*2:((int)p[lt_clock]/8);
            }
            break;
          case ls_slash:
            if (p[lt_clock]<6)
            {
              linktile(&p[lt_tile], &p[lt_flip], ls_slash, d->d1, zinit.linkanimationstyle);
            }
            else if (p[lt_clock]<12)
              {
                linktile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.linkanimationstyle);
              }
              else if (p[lt_clock]<13)
                {
                  linktile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.linkanimationstyle);
                }
                else
                {
                  linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
                  p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
                  if (p[lt_clock]>=16)
                  {
                    p[lt_clock]=-1;
                  }
                };
            break;
          case ls_stab:
            if (p[lt_clock]<12)
            {
              linktile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.linkanimationstyle);
            }
            else if (p[lt_clock]<13)
              {
                linktile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.linkanimationstyle);
              }
              else
              {
                linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
                p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
                if (p[lt_clock]>=16)
                {
                  p[lt_clock]=-1;
                }
              };
            break;
          case ls_pound:
            if (p[lt_clock]<12)
            {
              linktile(&p[lt_tile], &p[lt_flip], ls_pound, d->d1, zinit.linkanimationstyle);
            }
            else if (p[lt_clock]<30)
              {
                linktile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.linkanimationstyle);
              }
              else
              {
                linktile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.linkanimationstyle);
                if (p[lt_clock]>=31)
                {
                  p[lt_clock]=-1;
                }
              };
            break;
          case ls_float:
            linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_float, d->d1, zinit.linkanimationstyle);
            p[lt_tile]+=anim_3_4(p[lt_clock],7)*(p[lt_extend]==2?2:1);
            if (p[lt_clock]>=55)
            {
              p[lt_clock]=-1;
            }
            break;
          case ls_swim:
            linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_swim, d->d1, zinit.linkanimationstyle);
            p[lt_tile]+=anim_3_4(p[lt_clock],7)*(p[lt_extend]==2?2:1);
            if (p[lt_clock]>=55)
            {
              p[lt_clock]=-1;
            }
            break;
          case ls_dive:
            linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_dive, d->d1, zinit.linkanimationstyle);
            if (p[lt_clock]>=50)
            {
              linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_float, d->d1, zinit.linkanimationstyle);
            };
            p[lt_tile]+=anim_3_4(p[lt_clock],7)*(p[lt_extend]==2?2:1);
            if (p[lt_clock]>=81)
            {
              p[lt_clock]=-1;
            }
            break;
          case ls_landhold1:
            linktile(&p[lt_tile], &p[lt_flip], ls_landhold1, d->d1, zinit.linkanimationstyle);
            break;
          case ls_landhold2:
            linktile(&p[lt_tile], &p[lt_flip], ls_landhold2, d->d1, zinit.linkanimationstyle);
            break;
          case ls_waterhold1:
            linktile(&p[lt_tile], &p[lt_flip], ls_waterhold1, d->d1, zinit.linkanimationstyle);
            break;
          case ls_waterhold2:
            linktile(&p[lt_tile], &p[lt_flip], ls_waterhold2, d->d1, zinit.linkanimationstyle);
            break;
          case ls_cast:
            linktile(&p[lt_tile], &p[lt_flip], ls_cast, d->d1, zinit.linkanimationstyle);
            if (p[lt_clock]<96)
            {
              linktile(&p[lt_tile], &p[lt_flip], ls_landhold2, d->d1, zinit.linkanimationstyle);
            };
            if (p[lt_clock]>=194)
            {
              p[lt_clock]=-1;
            }
            break;
          default:
            break;
        }
        break;
      case las_zelda3:                                             //multi-frame Zelda 3
        switch (d->d2)
        {
          case ls_charge:
          case ls_walk:
            linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], d->d2, d->d1, zinit.linkanimationstyle);
            if (p[lt_clock]>=128)
            {
              p[lt_tile]+=(p[lt_extend]==2?2:1);

              int l=((p[lt_clock]>>1)&15);
              l-=((l>3)?1:0)+((l>12)?1:0);
              p[lt_tile]+=(l/2)*(p[lt_extend]==2?2:1);

              //p[lt_tile]+=(((p[lt_clock]>>2)%8)*(p[lt_extend]==2?2:1));
              if (p[lt_clock]>=255)
              {
                p[lt_clock]=-1;
              }
            }
            break;
          case ls_jump:
            if (p[lt_clock]>=24)
            {
              linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
              if (p[lt_clock]>=36)
              {
                p[lt_clock]=-1;
              }
            }
            else
            {
              linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_jump, d->d1, zinit.linkanimationstyle);
              p[lt_tile]+=p[lt_extend]==2?((int)p[lt_clock]/8)*2:((int)p[lt_clock]/8);
            }
            break;
          case ls_slash:
            if (p[lt_clock]>23)
            {
              linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_slash, d->d1, zinit.linkanimationstyle);
              p[lt_tile]+=(((p[lt_clock]>>2)%6)*(p[lt_extend]==2?2:1));
              if (p[lt_clock]>=47)
              {
                p[lt_clock]=-1;
              }
            }
            else
            {
              linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
            }
            break;
          case ls_stab:
            if (p[lt_clock]>35)
            {
              linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_stab, d->d1, zinit.linkanimationstyle);
              p[lt_tile]+=(((p[lt_clock]>>2)%3)*(p[lt_extend]==2?2:1));
              if (p[lt_clock]>=47)
              {
                p[lt_clock]=-1;
              }
            }
            else
            {
              linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
            }
            break;
          case ls_pound:
            if (p[lt_clock]>35)
            {
              linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_pound, d->d1, zinit.linkanimationstyle);
              p[lt_tile]+=(((p[lt_clock]>>2)%3)*(p[lt_extend]==2?2:1));
              if (p[lt_clock]>=47)
              {
                p[lt_clock]=-1;
              }
            }
            else
            {
              linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
            }
            break;
          case ls_float:
            linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_float, d->d1, zinit.linkanimationstyle);
            p[lt_tile]+=((p[lt_clock]/6)%4)<<(p[lt_extend]==2?1:0);
            if (p[lt_clock]>=23)
            {
              p[lt_clock]=-1;
            }
            break;
          case ls_swim:
            linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_swim, d->d1, zinit.linkanimationstyle);
            p[lt_tile]+=((p[lt_clock]/12)%4)<<(p[lt_extend]==2?1:0);
            if (p[lt_clock]>=47)
            {
              p[lt_clock]=-1;
            }
            break;
          case ls_dive:
            linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_dive, d->d1, zinit.linkanimationstyle);
            if (p[lt_clock]>=50)
            {
              linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_float, d->d1, zinit.linkanimationstyle);
            };
            p[lt_tile]+=((p[lt_clock]/6)%4)<<(p[lt_extend]==2?1:0);
            if (p[lt_clock]>=81)
            {
              p[lt_clock]=-1;
            }
            break;
          case ls_landhold1:
            linktile(&p[lt_tile], &p[lt_flip], ls_landhold1, d->d1, zinit.linkanimationstyle);
            break;
          case ls_landhold2:
            linktile(&p[lt_tile], &p[lt_flip], ls_landhold2, d->d1, zinit.linkanimationstyle);
            break;
          case ls_waterhold1:
            linktile(&p[lt_tile], &p[lt_flip], ls_waterhold1, d->d1, zinit.linkanimationstyle);
            break;
          case ls_waterhold2:
            linktile(&p[lt_tile], &p[lt_flip], ls_waterhold2, d->d1, zinit.linkanimationstyle);
            break;
          case ls_cast:
            linktile(&p[lt_tile], &p[lt_flip], ls_cast, d->d1, zinit.linkanimationstyle);
            if (p[lt_clock]<96)
            {
              linktile(&p[lt_tile], &p[lt_flip], ls_landhold2, d->d1, zinit.linkanimationstyle);
            };
            if (p[lt_clock]>=194)
            {
              p[lt_clock]=-1;
            }
            break;
          default:
            break;
        }
        break;
      default:
        break;
    }
    if ((p[lt_tile]!=oldtile)||(p[lt_flip]!=oldflip))
    {
      d->flags|=D_DIRTY;
    }
    break;

    case MSG_DRAW:
    {
      BITMAP *buf=create_bitmap_ex(8,1,1);
      BITMAP *buf2=buf;
      int dummy1, dummy2;
      int extend;
      linktile(&dummy1, &dummy2, &extend, d->d2, d->d1, zinit.linkanimationstyle);
      switch(extend)
      {
        case 0: //16x16
        buf = create_bitmap_ex(8,20,20);
        break;
        case 1: //16x32
        buf = create_bitmap_ex(8,20,36);
        break;
        case 2: //32x32
        buf = create_bitmap_ex(8,36,36);
        break;
        default:
        break;
      }

      if(buf)
      {
        clear_to_color(buf, bg);
        switch(extend)
        {
          case 0:
          overtile16(buf,p[lt_tile],0+2,0+2,6,p[lt_flip]);
          jwin_draw_frame(buf,0, 0, 20, 20, FR_DEEP);
          blit(buf,screen,0,0,d->x+8,d->y+16,20,20);
          break;
          case 1:
          overtile16(buf,p[lt_tile]-TILES_PER_ROW,0+2,0+2,6,p[lt_flip]);
          overtile16(buf,p[lt_tile],0+2,16+2,6,p[lt_flip]);
          jwin_draw_frame(buf,0, 0, 20, 36, FR_DEEP);
          blit(buf,screen,0,0,d->x+8,d->y,20,36);
          break;
          case 2:
          overtile16(buf,p[lt_tile]-TILES_PER_ROW,8+2,0+2,6,p[lt_flip]); //top middle
          overtile16(buf,p[lt_tile]-TILES_PER_ROW-(p[lt_flip]?-1:1),-8+2,0+2,6,p[lt_flip]); //top left
          overtile16(buf,p[lt_tile]-TILES_PER_ROW+(p[lt_flip]?-1:1),8+16+2,0+2,6,p[lt_flip]); //top right
          overtile16(buf,p[lt_tile]-(p[lt_flip]?-1:1),-8+2,18,6,p[lt_flip]); //bottom left
          overtile16(buf,p[lt_tile]+(p[lt_flip]?-1:1),8+16+2,18,6,p[lt_flip]); //bottom right
          overtile16(buf,p[lt_tile],8+2,16+2,6,p[lt_flip]);              //bottom middle
          jwin_draw_frame(buf,0, 0, 36, 36, FR_DEEP);
          //rectfill(buf, 0+2, 0+2, 0+15+2, 0+15+2, vc(1));
          blit(buf,screen,0,0,d->x,d->y,36,36);
          break;
          default:
          break;
        }
        destroy_bitmap(buf);
        destroy_bitmap(buf2);
      }
    }
    break;

    case MSG_END:
    {
      free(d->dp3);
      break;
    }
  }
  return D_O_K;
}

static int linktile_land_walk_list[] =
{
  // dialog control number
  11, 12, 13, 14, 15, 16, 17, 18, -1
};

static int linktile_land_slash_list[] =
{
  // dialog control number
  19, 20, 21, 22, 23, 24, 25, 26, -1
};

static int linktile_land_stab_list[] =
{
  // dialog control number
  27, 28, 29, 30, 31, 32, 33, 34, -1
};

static int linktile_land_pound_list[] =
{
  // dialog control number
  35, 36, 37, 38, 39, 40, 41, 42, -1
};

static int linktile_land_hold_list[] =
{
  // dialog control number
  43, 44, 45, 46, -1
};

static int linktile_land_cast_list[] =
{
  // dialog control number
  47, -1
};

static int linktile_land_jump_list[] =
{
  // dialog control number
  77, 78, 79, 80, 81, 82, 83, 84, -1
};

static int linktile_land_charge_list[] =
{
  // dialog control number
  85, 86, 87, 88, 89, 90, 91, 92, -1
};

static TABPANEL linktile_land_tabs[] =
{
  // (text)
  { "Walk",       D_SELECTED,  linktile_land_walk_list, 0, NULL },
  { "Slash",      0,           linktile_land_slash_list, 0, NULL },
  { "Stab",       0,           linktile_land_stab_list, 0, NULL },
  { "Pound",      0,           linktile_land_pound_list, 0, NULL },
  { "Jump",       0,           linktile_land_jump_list, 0, NULL },
  { "Charge",     0,           linktile_land_charge_list, 0, NULL },
  { "Hold",       0,           linktile_land_hold_list, 0, NULL },
  { "Cast",       0,           linktile_land_cast_list, 0, NULL },
  { NULL,         0,           NULL,                   0, NULL }
};

static int linktile_water_float_list[] =
{
  // dialog control number
  48, 49, 50, 51, 52, 53, 54, 55, -1
};

static int linktile_water_swim_list[] =
{
  // dialog control number
  56, 57, 58, 59, 60, 61, 62, 63, -1
};

static int linktile_water_dive_list[] =
{
  // dialog control number
  64, 65, 66, 67, 68, 69, 70, 71, -1
};

static int linktile_water_hold_list[] =
{
  // dialog control number
  72, 73, 74, 75, -1
};

static TABPANEL linktile_water_tabs[] =
{
  // (text)
  { "Float",      D_SELECTED,  linktile_water_float_list, 0, NULL },
  { "Swim",       0,           linktile_water_swim_list, 0, NULL },
  { "Dive",       0,           linktile_water_dive_list, 0, NULL },
  { "Hold",       0,           linktile_water_hold_list, 0, NULL },
  { NULL,         0,           NULL,                     0, NULL }
};

static int linktile_land_list[] =
{
  // dialog control number
  9, -1
};

static int linktile_water_list[] =
{
  // dialog control number
  10, -1
};

static TABPANEL linktile_tabs[] =
{
  // (text)
  { "Land",       D_SELECTED,   linktile_land_list, 0, NULL },
  { "Water",      0,            linktile_water_list, 0, NULL },
  { NULL,         0,            NULL,                0, NULL }
};

char *animationstyles[]= { "Original", "BS-Zelda", "Zelda 3" };

char *animationstylelist(int index, int *list_size)
{
  if(index>=0)
  {
    return animationstyles[index];
  }
  *list_size=las_max;
  return NULL;
}

int jwin_as_droplist_proc(int msg,DIALOG *d,int c)
{
  int ret = jwin_droplist_proc(msg,d,c);
  switch(msg)
  {
    case MSG_CHAR:
    case MSG_CLICK:
    zinit.linkanimationstyle=d->d1;
  }
  return ret;
}

static DIALOG linktile_dlg[] =
{
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
  { jwin_win_proc,     0,   0,   320,    240,  vc(14),  vc(1),  0,       D_EXIT,      0,             0,       (void *) "Link Sprites", NULL, NULL },
  { d_vsync_proc,         0,   0,    0,    0,   0,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
  { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       (void *) onHelp, NULL, NULL },
  { jwin_button_proc,     90,  215,   61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK", NULL, NULL },
  { jwin_button_proc,     170, 215,   61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel", NULL, NULL },
  // 5
  { jwin_check_proc,   217, 200,   0,   9,   vc(14),   vc(1),  0,                             0,         1,            0,       (void *) "Large Link Hit Box", NULL, NULL },

  { jwin_text_proc,      4,  201,   16+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Animation Style:", NULL, NULL },
  { jwin_as_droplist_proc, 77,  197,   78,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) animationstylelist, NULL, NULL },

  { jwin_tab_proc,     4,  25,  312,  152,   0,       0,      0,       0,             0,       0,       (void *) linktile_tabs, NULL, (void *)linktile_dlg },
  // 9
  { jwin_tab_proc,     7,  46,  305,  127,   0,       0,      0,       0,             0,       0,       (void *) linktile_land_tabs, NULL, (void *)linktile_dlg },
  { jwin_tab_proc,     7,  46,  305,  127,   0,       0,      0,       0,             0,       0,       (void *) linktile_water_tabs, NULL, (void *)linktile_dlg },
  // 11 (walk sprite titles)
  { jwin_rtext_proc,    33,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Up", NULL, NULL },
  { jwin_rtext_proc,   101,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Down", NULL, NULL },
  { jwin_rtext_proc,    33, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Left", NULL, NULL },
  { jwin_rtext_proc,   101, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Right", NULL, NULL },
  // 15 (walk sprites)
  { d_ltile_proc,       36,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,          up,             ls_walk,       NULL, NULL, NULL },
  { d_ltile_proc,      104,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        down,             ls_walk,       NULL, NULL, NULL },
  { d_ltile_proc,       36, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        left,             ls_walk,       NULL, NULL, NULL },
  { d_ltile_proc,      104, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,       right,             ls_walk,       NULL, NULL, NULL },
  // 19 (slash spritetitles)
  { jwin_rtext_proc,    33,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Up", NULL, NULL },
  { jwin_rtext_proc,   101,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Down", NULL, NULL },
  { jwin_rtext_proc,    33, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Left", NULL, NULL },
  { jwin_rtext_proc,   101, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Right", NULL, NULL },
  // 23 (slash sprites)
  { d_ltile_proc,       36,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,          up,             ls_slash,      NULL, NULL, NULL },
  { d_ltile_proc,      104,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        down,             ls_slash,      NULL, NULL, NULL },
  { d_ltile_proc,       36, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        left,             ls_slash,      NULL, NULL, NULL },
  { d_ltile_proc,      104, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,       right,             ls_slash,      NULL, NULL, NULL },
  // 27 (stab sprite titles)
  { jwin_rtext_proc,    33,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Up", NULL, NULL },
  { jwin_rtext_proc,   101,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Down", NULL, NULL },
  { jwin_rtext_proc,    33, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Left", NULL, NULL },
  { jwin_rtext_proc,   101, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Right", NULL, NULL },
  // 31 (stab sprites)
  { d_ltile_proc,       36,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,          up,             ls_stab,       NULL, NULL, NULL },
  { d_ltile_proc,      104,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        down,             ls_stab,       NULL, NULL, NULL },
  { d_ltile_proc,       36, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        left,             ls_stab,       NULL, NULL, NULL },
  { d_ltile_proc,      104, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,       right,             ls_stab,       NULL, NULL, NULL },
  // 35 (pound sprite titles)
  { jwin_rtext_proc,    33,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Up", NULL, NULL },
  { jwin_rtext_proc,   101,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Down", NULL, NULL },
  { jwin_rtext_proc,    33, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Left", NULL, NULL },
  { jwin_rtext_proc,   101, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Right", NULL, NULL },
  // 39 (pound sprites)
  { d_ltile_proc,       36,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,          up,             ls_pound,      NULL, NULL, NULL },
  { d_ltile_proc,      104,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        down,             ls_pound,      NULL, NULL, NULL },
  { d_ltile_proc,       36, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        left,             ls_pound,      NULL, NULL, NULL },
  { d_ltile_proc,      104, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,       right,             ls_pound,      NULL, NULL, NULL },
  // 43 (hold sprite titles)
  { jwin_rtext_proc,    67,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "One Hand", NULL, NULL },
  { jwin_rtext_proc,    67, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Two Hands", NULL, NULL },
  // 45 (hold sprites)
  { d_ltile_proc,       70,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,          up,             ls_landhold1,      NULL, NULL, NULL },
  { d_ltile_proc,       70, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        left,             ls_landhold2,      NULL, NULL, NULL },
  // 47 (casting sprites)
  { d_ltile_proc,       70,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,          up,             ls_cast,      NULL, NULL, NULL },

  // 48 (float sprite titles)
  { jwin_rtext_proc,    33,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Up", NULL, NULL },
  { jwin_rtext_proc,   101,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Down", NULL, NULL },
  { jwin_rtext_proc,    33, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Left", NULL, NULL },
  { jwin_rtext_proc,   101, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Right", NULL, NULL },
  // 52 (float sprites)
  { d_ltile_proc,       36,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,          up,             ls_float,       NULL, NULL, NULL },
  { d_ltile_proc,      104,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        down,             ls_float,       NULL, NULL, NULL },
  { d_ltile_proc,       36, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        left,             ls_float,       NULL, NULL, NULL },
  { d_ltile_proc,      104, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,       right,             ls_float,       NULL, NULL, NULL },
  // 56 (swim sprite titles)
  { jwin_rtext_proc,    33,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Up", NULL, NULL },
  { jwin_rtext_proc,   101,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Down", NULL, NULL },
  { jwin_rtext_proc,    33, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Left", NULL, NULL },
  { jwin_rtext_proc,   101, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Right", NULL, NULL },
  // 60 (swim sprites)
  { d_ltile_proc,       36,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,          up,             ls_swim,        NULL, NULL, NULL },
  { d_ltile_proc,      104,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        down,             ls_swim,        NULL, NULL, NULL },
  { d_ltile_proc,       36, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        left,             ls_swim,        NULL, NULL, NULL },
  { d_ltile_proc,      104, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,       right,             ls_swim,        NULL, NULL, NULL },
  // 64 (dive sprite titles)
  { jwin_rtext_proc,    33,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Up", NULL, NULL },
  { jwin_rtext_proc,   101,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Down", NULL, NULL },
  { jwin_rtext_proc,    33, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Left", NULL, NULL },
  { jwin_rtext_proc,   101, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Right", NULL, NULL },
  // 68 (dive sprites)
  { d_ltile_proc,       36,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,          up,             ls_dive,       NULL, NULL, NULL },
  { d_ltile_proc,      104,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        down,             ls_dive,       NULL, NULL, NULL },
  { d_ltile_proc,       36, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        left,             ls_dive,       NULL, NULL, NULL },
  { d_ltile_proc,      104, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,       right,             ls_dive,       NULL, NULL, NULL },
  // 72 (hold sprite titles)
  { jwin_rtext_proc,    67,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "One Hand", NULL, NULL },
  { jwin_rtext_proc,    67, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Two Hands", NULL, NULL },
  // 74 (hold sprites)
  { d_ltile_proc,       70,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,          up,             ls_waterhold1,      NULL, NULL, NULL },
  { d_ltile_proc,       70, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        left,             ls_waterhold2,      NULL, NULL, NULL },
  { jwin_check_proc,   217, 186,   0,   9,   vc(14),   vc(1),  0,                             0,         1,            0,       (void *) "Z3-style Movement", NULL, NULL },
  // 77 (jump sprite titles)
  { jwin_rtext_proc,    33,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Up", NULL, NULL },
  { jwin_rtext_proc,   101,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Down", NULL, NULL },
  { jwin_rtext_proc,    33, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Left", NULL, NULL },
  { jwin_rtext_proc,   101, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Right", NULL, NULL },
  // 81 (jump sprites)
  { d_ltile_proc,       36,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,          up,             ls_jump,       NULL, NULL, NULL },
  { d_ltile_proc,      104,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        down,             ls_jump,       NULL, NULL, NULL },
  { d_ltile_proc,       36, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        left,             ls_jump,       NULL, NULL, NULL },
  { d_ltile_proc,      104, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,       right,             ls_jump,       NULL, NULL, NULL },
  // 85 (charge sprite titles)
  { jwin_rtext_proc,    33,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Up", NULL, NULL },
  { jwin_rtext_proc,   101,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Down", NULL, NULL },
  { jwin_rtext_proc,    33, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Left", NULL, NULL },
  { jwin_rtext_proc,   101, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Right", NULL, NULL },
  // 89 (charge sprites)
  { d_ltile_proc,       36,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,          up,             ls_charge,       NULL, NULL, NULL },
  { d_ltile_proc,      104,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        down,             ls_charge,       NULL, NULL, NULL },
  { d_ltile_proc,       36, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        left,             ls_charge,       NULL, NULL, NULL },
  { d_ltile_proc,      104, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,       right,             ls_charge,       NULL, NULL, NULL },
  { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};



int onCustomLink()
{
  //setuplinktiles(zinit.linkanimationstyle);
  linktile_dlg[0].dp2=lfont;
  linktile_dlg[5].flags = get_bit(quest_rules, qr_LTTPCOLLISION)? D_SELECTED : 0;
  linktile_dlg[76].flags = get_bit(quest_rules, qr_LTTPWALK)? D_SELECTED : 0;
  linktile_dlg[7].d1=zinit.linkanimationstyle;
  int ret = popup_dialog_through_bitmap(screen2,linktile_dlg,3);
  if(ret==3)
  {
    saved=false;
    set_bit(quest_rules, qr_LTTPCOLLISION, (linktile_dlg[5].flags&D_SELECTED)?1:0);
    set_bit(quest_rules, qr_LTTPWALK, (linktile_dlg[76].flags&D_SELECTED)?1:0);
  }

  ret=ret;
  return D_O_K;
}

void center_zq_custom_dialogs()
{
  jwin_center_dialog(itemdata_dlg);
  jwin_center_dialog(linktile_dlg);
  jwin_center_dialog(wpndata_dlg);
  jwin_center_dialog(enedata_dlg);
}
