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
#include <stdio.h>
#include <string.h>

extern int ex;

int d_cstile_proc(int msg,DIALOG *d,int c)
{
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
  18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, -1
};

static TABPANEL itemdata_tabs[] =
{
  // (text)
  { "GFX",       D_SELECTED,   itemdata_gfx_list },
  { "Action",       0,           itemdata_action_list },
  { NULL }
};

static DIALOG itemdata_dlg[] =
{
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
  { jwin_win_proc,     55,   40,   220,  193,  vc(14),  vc(1),  0,       D_EXIT,     0,             0,       NULL },
  { jwin_tab_proc,     57,  65,  216,  135,   0,       0,      0,       0,             0,       0,       (void *) itemdata_tabs, NULL, (void *)itemdata_dlg },
  { d_cstile_proc,     198,  84,   20,   20,   vc(11),  vc(1),  0,       0,          0,             6,       NULL },
  { jwin_button_proc,  90,   206,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,  170,  206,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { jwin_button_proc,  202,  140,  53,   21,   vc(14),  vc(1),  't',     D_EXIT,     0,             0,       (void *) "&Test" },
  { jwin_check_proc,   198,  109,   65,   9,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Flash " },
  { jwin_check_proc,   198,  120,  65,   9,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "2-Hand" },
  { jwin_text_proc,    61,   88,   96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Flash CSet:" },
  { jwin_text_proc,    61,   106,   96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Animation Frames:" },
  { jwin_text_proc,    61,   124,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Animation Speed:" },
  { jwin_text_proc,    61,   142,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Initial Delay:" },
  { jwin_text_proc,    61,   160,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Link Tile Modification:" },
  { jwin_edit_proc,    160,  84,   35,   16,   vc(12),  vc(1),  0,       0,          2,             0,       NULL },
  { jwin_edit_proc,    160,  102,   35,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { jwin_edit_proc,    160,  120,  35,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { jwin_edit_proc,    160,  138,  35,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { jwin_edit_proc,    160,  156,  35,   16,   vc(12),  vc(1),  0,       0,          6,             0,       NULL },
  //18
  { jwin_check_proc,   61,  174,   130,   9,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Keep Item When Collected" },
  { jwin_text_proc,    61,   88,   96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Class Number:" },
  { jwin_text_proc,    61,   106,   96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Class Level:" },
  { jwin_text_proc,    61,   124,   96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Increase Amount:" },
  { jwin_text_proc,    61,   142,   96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Counter Reference:" },
  { jwin_text_proc,    61,   160,  60,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Full Max:" },
  { jwin_text_proc,    160,   160,  60,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "+Max:" },
  //25
  { jwin_edit_proc,    160,  84,   35,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { jwin_edit_proc,    160,  102,   35,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { jwin_edit_proc,    160,  120,  35,   16,   vc(12),  vc(1),  0,       0,          5,             0,       NULL },
  { jwin_edit_proc,    160,  138,  35,   16,   vc(12),  vc(1),  0,       0,          2,             0,       NULL },
  { jwin_edit_proc,    121,  156,  35,   16,   vc(12),  vc(1),  0,       0,          5,             0,       NULL },
  { jwin_edit_proc,    220,  156,  35,   16,   vc(12),  vc(1),  0,       0,          5,             0,       NULL },
  { jwin_check_proc,   200,  124,   60,   9,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "DCounter" },
  { NULL }
};

void test_item(itemdata test)
{
  itemdata *hold = itemsbuf;
  itemsbuf = &test;
  BITMAP *buf = create_bitmap_ex(8,16,16);

  item temp((fix)0,(fix)0,0,0,0);
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
  while(1)
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
  itemdata_dlg[25].dp = cls;
  itemdata_dlg[26].dp = cll;
  itemdata_dlg[27].dp = amt;
  itemdata_dlg[28].dp = cnt;
  itemdata_dlg[29].dp = fmx;
  itemdata_dlg[30].dp = max;

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

  } while(ret==5);

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
  { jwin_win_proc,     55,   40,   210,  143,  vc(14),  vc(1),  0,       D_EXIT,     0,             0,       NULL },
  { d_dummy_proc,      160,  68,   0,    8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { d_cstile_proc,     198,  64,   20,   20,   vc(11),  vc(1),  0,       0,          0,             6,       NULL },
  { jwin_button_proc,  90,   156,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,  170,  156,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { jwin_check_proc,   198,  89,   65,   9,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Auto-flash" },
  { jwin_check_proc,   198,  100,  65,   9,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "2P Flash" },
  { jwin_check_proc,   198,  111,  65,   9,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "H-Flip" },
  { jwin_check_proc,   198,  122,  65,   9,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "V-Flip" },
  { jwin_text_proc,    61,   68,   96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Flash CSet:" },
  { jwin_text_proc,    61,   86,   96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Animation Frames:" },
  { jwin_text_proc,    61,   104,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Animation Speed:" },
  { jwin_text_proc,    61,   122,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Miscellaneous Type:" },
  { jwin_edit_proc,    160,  64,   35,   16,   vc(12),  vc(1),  0,       0,          2,             0,       NULL },
  { jwin_edit_proc,    160,  82,   35,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { jwin_edit_proc,    160,  100,  35,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { jwin_edit_proc,    160,  118,  35,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { NULL }
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

int onCustomGuys()
{
  return D_O_K;
}

int d_ltile_proc(int msg,DIALOG *d,int c)
{
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
        case ls_walk:
        linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
        if (p[lt_clock]>=6)
        {
          if (d->d1==up)
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
        case ls_walk:
        linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
        p[lt_tile]+=anim_3_4(p[lt_clock],7)*(p[lt_extend]==2?2:1);
        if (p[lt_clock]>=27)
        {
          p[lt_clock]=-1;
        }
        break;
        case ls_jump:
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
        case ls_walk:
        linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
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

static TABPANEL linktile_land_tabs[] =
{
  // (text)
  { "Walk",       D_SELECTED,  linktile_land_walk_list },
  { "Slash",      0,           linktile_land_slash_list },
  { "Stab",       0,           linktile_land_stab_list },
  { "Pound",      0,           linktile_land_pound_list },
  { "Hold",       0,           linktile_land_hold_list },
  { "Cast",       0,           linktile_land_cast_list },
  { NULL }
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
  { "Float",      D_SELECTED,  linktile_water_float_list },
  { "Swim",       0,           linktile_water_swim_list },
  { "Dive",       0,           linktile_water_dive_list },
  { "Hold",       0,           linktile_water_hold_list },
  { NULL }
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
  { "Land",       D_SELECTED,   linktile_land_list },
  { "Water",       0,           linktile_water_list },
  { NULL }
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
  { jwin_win_proc,     0,   0,   320,    240,  vc(14),  vc(1),  0,       D_EXIT,      0,             0,       (void *) "Link Sprites" },
  { d_vsync_proc,         0,   0,    0,    0,   0,       0,      0,       0,          0,             0,       NULL },
  { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       (void *) onHelp },
  { jwin_button_proc,     90,  215,   61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,     170, 215,   61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  // 5
  { jwin_check_proc,   217, 200,   0,   9,   vc(14),   vc(1),  0,                             0,         1,            0,       (void *) "Large Link Hit Box" },

  { jwin_text_proc,      4,  201,   16+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Animation Style:" },
  { jwin_as_droplist_proc, 77,  197,   78,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) animationstylelist },

  { jwin_tab_proc,     4,  25,  312,  152,   0,       0,      0,       0,             0,       0,       (void *) linktile_tabs, NULL, (void *)linktile_dlg },
  // 9
  { jwin_tab_proc,     7,  46,  305,  127,   0,       0,      0,       0,             0,       0,       (void *) linktile_land_tabs, NULL, (void *)linktile_dlg },
  { jwin_tab_proc,     7,  46,  305,  127,   0,       0,      0,       0,             0,       0,       (void *) linktile_water_tabs, NULL, (void *)linktile_dlg },
  // 11 (walk sprite titles)
  { jwin_rtext_proc,    33,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Up" },
  { jwin_rtext_proc,   101,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Down" },
  { jwin_rtext_proc,    33, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Left" },
  { jwin_rtext_proc,   101, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Right" },
  // 15 (walk sprites)
  { d_ltile_proc,       36,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,          up,             ls_walk,       NULL },
  { d_ltile_proc,      104,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        down,             ls_walk,       NULL },
  { d_ltile_proc,       36, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        left,             ls_walk,       NULL },
  { d_ltile_proc,      104, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,       right,             ls_walk,       NULL },
  // 19 (slash spritetitles)
  { jwin_rtext_proc,    33,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Up" },
  { jwin_rtext_proc,   101,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Down" },
  { jwin_rtext_proc,    33, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Left" },
  { jwin_rtext_proc,   101, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Right" },
  // 23 (slash sprites)
  { d_ltile_proc,       36,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,          up,             ls_slash,      NULL },
  { d_ltile_proc,      104,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        down,             ls_slash,      NULL },
  { d_ltile_proc,       36, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        left,             ls_slash,      NULL },
  { d_ltile_proc,      104, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,       right,             ls_slash,      NULL },
  // 27 (stab sprite titles)
  { jwin_rtext_proc,    33,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Up" },
  { jwin_rtext_proc,   101,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Down" },
  { jwin_rtext_proc,    33, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Left" },
  { jwin_rtext_proc,   101, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Right" },
  // 31 (stab sprites)
  { d_ltile_proc,       36,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,          up,             ls_stab,       NULL },
  { d_ltile_proc,      104,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        down,             ls_stab,       NULL },
  { d_ltile_proc,       36, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        left,             ls_stab,       NULL },
  { d_ltile_proc,      104, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,       right,             ls_stab,       NULL },
  // 35 (pound sprite titles)
  { jwin_rtext_proc,    33,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Up" },
  { jwin_rtext_proc,   101,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Down" },
  { jwin_rtext_proc,    33, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Left" },
  { jwin_rtext_proc,   101, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Right" },
  // 39 (pound sprites)
  { d_ltile_proc,       36,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,          up,             ls_pound,      NULL },
  { d_ltile_proc,      104,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        down,             ls_pound,      NULL },
  { d_ltile_proc,       36, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        left,             ls_pound,      NULL },
  { d_ltile_proc,      104, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,       right,             ls_pound,      NULL },
  // 43 (hold sprite titles)
  { jwin_rtext_proc,    67,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "One Hand" },
  { jwin_rtext_proc,    67, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Two Hands" },
  // 45 (hold sprites)
  { d_ltile_proc,       70,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,          up,             ls_landhold1,      NULL },
  { d_ltile_proc,       70, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        left,             ls_landhold2,      NULL },
  // 47 (casting sprites)
  { d_ltile_proc,       70,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,          up,             ls_cast,      NULL },

  // 48 (float sprite titles)
  { jwin_rtext_proc,    33,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Up" },
  { jwin_rtext_proc,   101,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Down" },
  { jwin_rtext_proc,    33, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Left" },
  { jwin_rtext_proc,   101, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Right" },
  // 52 (float sprites)
  { d_ltile_proc,       36,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,          up,             ls_float,       NULL },
  { d_ltile_proc,      104,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        down,             ls_float,       NULL },
  { d_ltile_proc,       36, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        left,             ls_float,       NULL },
  { d_ltile_proc,      104, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,       right,             ls_float,       NULL },
  // 56 (swim sprite titles)
  { jwin_rtext_proc,    33,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Up" },
  { jwin_rtext_proc,   101,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Down" },
  { jwin_rtext_proc,    33, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Left" },
  { jwin_rtext_proc,   101, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Right" },
  // 60 (swim sprites)
  { d_ltile_proc,       36,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,          up,             ls_swim,        NULL },
  { d_ltile_proc,      104,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        down,             ls_swim,        NULL },
  { d_ltile_proc,       36, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        left,             ls_swim,        NULL },
  { d_ltile_proc,      104, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,       right,             ls_swim,        NULL },
  // 64 (dive sprite titles)
  { jwin_rtext_proc,    33,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Up" },
  { jwin_rtext_proc,   101,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Down" },
  { jwin_rtext_proc,    33, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Left" },
  { jwin_rtext_proc,   101, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Right" },
  // 68 (dive sprites)
  { d_ltile_proc,       36,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,          up,             ls_dive,       NULL },
  { d_ltile_proc,      104,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        down,             ls_dive,       NULL },
  { d_ltile_proc,       36, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        left,             ls_dive,       NULL },
  { d_ltile_proc,      104, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,       right,             ls_dive,       NULL },
  // 72 (hold sprite titles)
  { jwin_rtext_proc,    67,  96,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "One Hand" },
  { jwin_rtext_proc,    67, 134,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Two Hands" },
  // 74 (hold sprites)
  { d_ltile_proc,       70,  74,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,          up,             ls_waterhold1,      NULL },
  { d_ltile_proc,       70, 112,   36,   36,   6,       jwin_pal[jcBOX],      0,       0,        left,             ls_waterhold2,      NULL },
  { jwin_check_proc,   217, 186,   0,   9,   vc(14),   vc(1),  0,                             0,         1,            0,       (void *) "Z3-style Movement" },
  { NULL }
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
}
