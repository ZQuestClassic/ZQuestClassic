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

static DIALOG itemdata_dlg[] =
{
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
  { jwin_win_proc,     55,   40,   210,  143,  vc(14),  vc(1),  0,       D_EXIT,     0,             0,       NULL },
  { d_dummy_proc,      160,  68,   0,    8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { d_cstile_proc,     198,  64,   20,   20,   vc(11),  vc(1),  0,       0,          0,             6,       NULL },
  { jwin_button_proc,  90,   156,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,  170,  156,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { jwin_button_proc,  202,  120,  53,   21,   vc(14),  vc(1),  't',     D_EXIT,     0,             0,       (void *) "&Test" },
  { jwin_check_proc,   198,  89,   65,   9,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Flash " },
  { jwin_check_proc,   198,  100,  65,   9,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "2-Hand" },
  { jwin_text_proc,    61,   68,   96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Flash CSet:" },
  { jwin_text_proc,    61,   86,   96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Animation Frames:" },
  { jwin_text_proc,    61,   104,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Animation Speed:" },
  { jwin_text_proc,    61,   122,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Initial Delay:" },
  { jwin_text_proc,    61,   140,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Link Tile Modification:" },
  { jwin_edit_proc,    160,  64,   35,   16,   vc(12),  vc(1),  0,       0,          2,             0,       NULL },
  { jwin_edit_proc,    160,  82,   35,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { jwin_edit_proc,    160,  100,  35,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { jwin_edit_proc,    160,  118,  35,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { jwin_edit_proc,    160,  136,  35,   16,   vc(12),  vc(1),  0,       0,          6,             0,       NULL },
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

  itemdata_dlg[0].dp = item_string[index];
  itemdata_dlg[0].dp2 = lfont;
  itemdata_dlg[2].d1 = itemsbuf[index].tile;
  itemdata_dlg[2].d2 = itemsbuf[index].csets&15;

  itemdata_dlg[6].flags = (itemsbuf[index].misc&1) ? D_SELECTED : 0;
  itemdata_dlg[7].flags = (itemsbuf[index].misc&2) ? D_SELECTED : 0;

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

  int ret;
  itemdata test;

  do
  {
    ret = zc_popup_dialog(itemdata_dlg,3);

    test.tile  = itemdata_dlg[2].d1;
    test.csets = itemdata_dlg[2].d2;

    test.misc  = 0;
    if(itemdata_dlg[6].flags & D_SELECTED)
      test.misc |= 1;
    if(itemdata_dlg[7].flags & D_SELECTED)
      test.misc |= 2;

    test.csets  |= (atoi(fcs)&15)<<4;
    test.frames = min(atoi(frm),255);
    test.speed  = min(atoi(spd),255);
    test.delay  = min(atoi(dly),255);
    test.ltm    = max(min(atol(ltm),NEWMAXTILES-1),0-(NEWMAXTILES-1));

    if(ret==5)
    {
      test_item(test);
      sprintf(fcs,"%d",test.csets>>4);
      sprintf(frm,"%d",test.frames);
      sprintf(spd,"%d",test.speed);
      sprintf(dly,"%d",test.delay);
      sprintf(ltm,"%ld",test.ltm);
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

  /*
    int index = select_data("Edit Item Data",index,itemslist,"Edit","Done");
    while(index >= 0)
    {
      edit_itemdata(index);
      index = select_data("Edit Item Data",index,itemslist,"Edit","Done");
    }
  */
  //  item_string[0] = hold;
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

  /*
    int index = select_data("Edit Item Data",index,itemslist,"Edit","Done");
    while(index >= 0)
    {
      edit_itemdata(index);
      index = select_data("Edit Item Data",index,itemslist,"Edit","Done");
    }
  */
  //  item_string[0] = hold;
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
  enum {lt_clock, lt_tile, lt_flip};
  static int bg=makecol(0, 0, 0);
  int *p=(int*)d->dp3;
  int oldtile=0;
  int oldflip=0;

  switch(msg)
  {
    case MSG_START:
    {
      d->dp3=(int*)malloc(sizeof(int)*3);
      p=(int*)d->dp3;
      p[lt_clock]=0;
      p[lt_tile]=0;
      p[lt_flip]=0;
      linktile(&p[lt_tile], &p[lt_flip], d->d2, d->d1, zinit.linkwalkstyle);
      break;
    }
    case MSG_CLICK:
      {
        int f  = 0;
        int cs = 6;
        if(select_tile(d->d1,f,1,cs,false))
          return D_REDRAW;
      }
      break;

    case MSG_VSYNC:
      oldtile=p[lt_tile];
      oldflip=p[lt_flip];
      p[lt_clock]++;
      switch (zinit.linkwalkstyle)
      {
        case 0:                                             //2-frame
          switch (d->d2)
          {
            case ls_walk:
              linktile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.linkwalkstyle);
              if (p[lt_clock]>=6)
              {
                if (d->d1==up)
                {
                  p[lt_flip]=1;                //h flip
                }
                else
                {
                  p[lt_tile]++;                //tile++
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
                linktile(&p[lt_tile], &p[lt_flip], ls_slash, d->d1, zinit.linkwalkstyle);
              }
              else if (p[lt_clock]<12)
              {
                linktile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.linkwalkstyle);
              }
              else if (p[lt_clock]<13)
              {
                linktile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.linkwalkstyle);
              }
              else
              {
                linktile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.linkwalkstyle);
                p[lt_tile]++;                  //tile++
                if (p[lt_clock]>=16)
                {
                  p[lt_clock]=-1;
                }
              };
              break;
            case ls_stab:
              if (p[lt_clock]<12)
              {
                linktile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.linkwalkstyle);
              }
              else if (p[lt_clock]<13)
              {
                linktile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.linkwalkstyle);
              }
              else
              {
                linktile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.linkwalkstyle);
                p[lt_tile]++;                  //tile++
                if (p[lt_clock]>=16)
                {
                  p[lt_clock]=-1;
                }
              };
              break;
            case ls_pound:
              if (p[lt_clock]<12)
              {
                linktile(&p[lt_tile], &p[lt_flip], ls_pound, d->d1, zinit.linkwalkstyle);
              }
              else if (p[lt_clock]<30)
              {
                linktile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.linkwalkstyle);
              }
              else
              {
                linktile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.linkwalkstyle);
                if (p[lt_clock]>=31)
                {
                  p[lt_clock]=-1;
                }
              };
              break;
            case ls_swim:
              linktile(&p[lt_tile], &p[lt_flip], ls_swim, d->d1, zinit.linkwalkstyle);
              if (p[lt_clock]>=12)
              {
                p[lt_tile]++;                  //tile++
              };
              if (p[lt_clock]>=23)
              {
                p[lt_clock]=-1;
              }
              break;
            case ls_dive:
              linktile(&p[lt_tile], &p[lt_flip], ls_dive, d->d1, zinit.linkwalkstyle);
              if (p[lt_clock]>=50)
              {
                linktile(&p[lt_tile], &p[lt_flip], ls_swim, d->d1, zinit.linkwalkstyle);
              };
              if ((p[lt_clock]/12)&1)
              {
                p[lt_tile]++;                  //tile++
              };
              if (p[lt_clock]>=81)
              {
                p[lt_clock]=-1;
              }
              break;
            case ls_hold1:
              linktile(&p[lt_tile], &p[lt_flip], ls_hold1, d->d1, zinit.linkwalkstyle);
              break;
            case ls_hold2:
              linktile(&p[lt_tile], &p[lt_flip], ls_hold2, d->d1, zinit.linkwalkstyle);
              break;
            case ls_swimhold1:
              linktile(&p[lt_tile], &p[lt_flip], ls_swimhold1, d->d1, zinit.linkwalkstyle);
              break;
            case ls_swimhold2:
              linktile(&p[lt_tile], &p[lt_flip], ls_swimhold2, d->d1, zinit.linkwalkstyle);
              break;
            case ls_cast:
              linktile(&p[lt_tile], &p[lt_flip], ls_cast, d->d1, zinit.linkwalkstyle);
              if (p[lt_clock]<96)
              {
                linktile(&p[lt_tile], &p[lt_flip], ls_hold2, d->d1, zinit.linkwalkstyle);
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
        case 1:                                             //3-frame BS
          switch (d->d2)
          {
            case ls_walk:
              linktile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.linkwalkstyle);
              p[lt_tile]+=anim_3_4(p[lt_clock],7);
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
                linktile(&p[lt_tile], &p[lt_flip], ls_slash, d->d1, zinit.linkwalkstyle);
              }
              else if (p[lt_clock]<12)
              {
                linktile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.linkwalkstyle);
              }
              else if (p[lt_clock]<13)
              {
                linktile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.linkwalkstyle);
              }
              else
              {
                linktile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.linkwalkstyle);
                p[lt_tile]++;                  //tile++
                if (p[lt_clock]>=16)
                {
                  p[lt_clock]=-1;
                }
              };
              break;
            case ls_stab:
              if (p[lt_clock]<12)
              {
                linktile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.linkwalkstyle);
              }
              else if (p[lt_clock]<13)
              {
                linktile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.linkwalkstyle);
              }
              else
              {
                linktile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.linkwalkstyle);
                p[lt_tile]++;                  //tile++
                if (p[lt_clock]>=16)
                {
                  p[lt_clock]=-1;
                }
              };
              break;
            case ls_pound:
              if (p[lt_clock]<12)
              {
                linktile(&p[lt_tile], &p[lt_flip], ls_pound, d->d1, zinit.linkwalkstyle);
              }
              else if (p[lt_clock]<30)
              {
                linktile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.linkwalkstyle);
              }
              else
              {
                linktile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.linkwalkstyle);
                if (p[lt_clock]>=31)
                {
                  p[lt_clock]=-1;
                }
              };
              break;
            case ls_swim:
              linktile(&p[lt_tile], &p[lt_flip], ls_swim, d->d1, zinit.linkwalkstyle);
              p[lt_tile]+=anim_3_4(p[lt_clock],7);
              if (p[lt_clock]>=55)
              {
                p[lt_clock]=-1;
              }
              break;
            case ls_dive:
              linktile(&p[lt_tile], &p[lt_flip], ls_dive, d->d1, zinit.linkwalkstyle);
              if (p[lt_clock]>=50)
              {
                linktile(&p[lt_tile], &p[lt_flip], ls_swim, d->d1, zinit.linkwalkstyle);
              };
              p[lt_tile]+=anim_3_4(p[lt_clock],7);
              if (p[lt_clock]>=81)
              {
                p[lt_clock]=-1;
              }
              break;
            case ls_hold1:
              linktile(&p[lt_tile], &p[lt_flip], ls_hold1, d->d1, zinit.linkwalkstyle);
              break;
            case ls_hold2:
              linktile(&p[lt_tile], &p[lt_flip], ls_hold2, d->d1, zinit.linkwalkstyle);
              break;
            case ls_swimhold1:
              linktile(&p[lt_tile], &p[lt_flip], ls_swimhold1, d->d1, zinit.linkwalkstyle);
              break;
            case ls_swimhold2:
              linktile(&p[lt_tile], &p[lt_flip], ls_swimhold2, d->d1, zinit.linkwalkstyle);
              break;
            case ls_cast:
              linktile(&p[lt_tile], &p[lt_flip], ls_cast, d->d1, zinit.linkwalkstyle);
              if (p[lt_clock]<96)
              {
                linktile(&p[lt_tile], &p[lt_flip], ls_hold2, d->d1, zinit.linkwalkstyle);
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
      BITMAP *buf = create_bitmap_ex(8,20,20);
      if(buf)
      {
        clear_to_color(buf, bg);
        jwin_draw_frame(buf,0, 0, 20, 20, FR_DEEP);
        overtile16(buf,p[lt_tile],2,2,6,p[lt_flip]);
        blit(buf,screen,0,0,d->x,d->y,20,20);
        destroy_bitmap(buf);
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


char *animationstyles[]= { "Original", "BS-Zelda" };

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
    zinit.linkwalkstyle=d->d1;
  }
  return ret;
}

static DIALOG linktile_dlg[] =
{
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
  { jwin_win_proc,     20,   18,   281,  217,  vc(14),  vc(1),  0,       D_EXIT,      0,             0,       (void *) "Link Tiles" },
  { d_vsync_proc,         0,   0,    0,    0,   0,       0,      0,       0,          0,             0,       NULL },
  { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       (void *) onHelp },
  { jwin_button_proc,     90,  210,   61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,     170, 210,   61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  // 4
  { d_dummy_proc,      160,  23,    0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Link Tiles" },

  { jwin_text_proc,        32,  42,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Walk" },
  { jwin_text_proc,        32,  72,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Jump" },
  { jwin_text_proc,        32, 102,   40,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Slash" },
  { jwin_text_proc,        32, 132,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Stab" },
  { jwin_text_proc,        32, 162,   40,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Pound" },
  { jwin_text_proc,       122,  42,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Swim" },
  { jwin_text_proc,       122,  72,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Dive" },
  { jwin_text_proc,       122, 102,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Hold" },
  { jwin_text_proc,       122, 132,   32,    8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Cast" },
  // walk
  { d_ltile_proc,       32,  50,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          up,             ls_walk,       NULL },
  { d_ltile_proc,       53,  50,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          down,             ls_walk,       NULL },
  { d_ltile_proc,       74,  50,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          left,             ls_walk,       NULL },
  { d_ltile_proc,       95,  50,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          right,             ls_walk,       NULL },
  // jump
  { d_ltile_proc,       32,  80,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          up,             ls_jump,       NULL },
  { d_ltile_proc,       53,  80,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          down,             ls_jump,       NULL },
  { d_ltile_proc,       74,  80,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          left,             ls_jump,       NULL },
  { d_ltile_proc,       95,  80,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          right,             ls_jump,       NULL },
  // slash
  { d_ltile_proc,       32, 110,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          up,             ls_slash,       NULL },
  { d_ltile_proc,       53, 110,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          down,             ls_slash,       NULL },
  { d_ltile_proc,       74, 110,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          left,             ls_slash,       NULL },
  { d_ltile_proc,       95, 110,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          right,             ls_slash,       NULL },
  // stab
  { d_ltile_proc,       32, 140,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          up,             ls_stab,       NULL },
  { d_ltile_proc,       53, 140,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          down,             ls_stab,       NULL },
  { d_ltile_proc,       74, 140,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          left,             ls_stab,       NULL },
  { d_ltile_proc,       95, 140,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          right,             ls_stab,       NULL },
  // swim
  { d_ltile_proc,       32, 170,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          up,             ls_pound,       NULL },
  { d_ltile_proc,       53, 170,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          down,             ls_pound,       NULL },
  { d_ltile_proc,       74, 170,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          left,             ls_pound,       NULL },
  { d_ltile_proc,       95, 170,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          right,             ls_pound,       NULL },
  // dive
  { d_ltile_proc,      122,  50,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          up,             ls_swim,       NULL },
  { d_ltile_proc,      143,  50,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          down,             ls_swim,       NULL },
  { d_ltile_proc,      164,  50,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          left,             ls_swim,       NULL },
  { d_ltile_proc,      185,  50,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          right,             ls_swim,       NULL },
  // hammer
  { d_ltile_proc,       122,  80,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          up,             ls_dive,       NULL },
  { d_ltile_proc,       143,  80,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          down,             ls_dive,       NULL },
  { d_ltile_proc,       164,  80,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          left,             ls_dive,       NULL },
  { d_ltile_proc,       185,  80,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          right,             ls_dive,       NULL },
  // hold
  { d_ltile_proc,       122, 110,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          0,             ls_hold1,       NULL },
  { d_ltile_proc,       143, 110,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          0,             ls_hold2,       NULL },
  { d_ltile_proc,       164, 110,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          0,             ls_swimhold1,       NULL },
  { d_ltile_proc,       185, 110,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          0,             ls_swimhold2,       NULL },
  // casting
  { d_ltile_proc,       122, 140,   20,   20,   6,       jwin_pal[jcBOX],      0,       0,          0,             ls_cast,       NULL },
  { jwin_text_proc,      32,  196,   16+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Animation Style:", NULL, NULL },
  { jwin_as_droplist_proc, 105,  192,   78,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) animationstylelist, NULL, NULL },

  { NULL }
};

int onCustomLink()
{
  setuplinktiles(zinit.linkwalkstyle);
  linktile_dlg[0].dp2=lfont;
  int walkstyle_temp=zinit.linkwalkstyle;
  linktile_dlg[49].d1=walkstyle_temp;
  int ret = popup_dialog_through_bitmap(screen2,linktile_dlg,3);
  if (ret!=3)
  {
    zinit.linkwalkstyle=walkstyle_temp;
  }
  return D_O_K;
}

void center_zq_custom_dialogs()
{
  jwin_center_dialog(itemdata_dlg);
  jwin_center_dialog(linktile_dlg);
  jwin_center_dialog(wpndata_dlg);
}
